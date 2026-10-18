//
//   Copyright (C) 2018 Skyloupe, LLC
//
//   Licensed under the Apache License, Version 2.0 (the "License").
//   You may not use this file except in compliance with the License.
//   A copy of the License is located at
//
//   https://www.apache.org/licenses/LICENSE-2.0.html
//
//   or in the "license" file accompanying this file. This file is distributed
//   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
//   express or implied. See the License for the specific language governing
//   permissions and limitations under the License.
//

#include <filesystem>
#include <algorithm>
#include <regex>
#include <set>
#include "bitfields.h"
#include "file_writer.h"
#include "naming.h"
#include "processor.h"
#include "sw_endian.h"

namespace skyloupe
{

namespace
{

// Everything from the message spec is pasted into generated source or used as
// a path, so it is restricted to characters that cannot break out of either.
const std::regex identifier_re("[A-Za-z_][A-Za-z0-9_]*");
const std::regex qualified_re("[A-Za-z_][A-Za-z0-9_]*((\\.|::)[A-Za-z_][A-Za-z0-9_]*)*");
const std::regex include_re("<[A-Za-z0-9_./+-]+>|\"[A-Za-z0-9_./+-]+\"|[A-Za-z0-9_./+-]+");
const std::regex filename_re("[A-Za-z0-9_.+-]+");

// Member tags are also protobuf field numbers (proto3 output, PZ streams), so
// they must be valid ones: 1 to 2^29-1, outside the range protobuf reserves.
const uint32_t max_field_number = (1u << 29) - 1;
const uint32_t first_reserved_field_number = 19000;
const uint32_t last_reserved_field_number = 19999;

// Output slot produced by a generator -> spec key naming its file.
const std::pair<const char*, const char*> output_slots[] = {
    { "header", "outputheaderfilename" },
    { "implementation", "outputimplfilename" },
    { "unittest", "outputtestfilename" },
    { "relay", "outputrelayfilename" },
};

// Output file names may contain these, which are replaced by the spec's
// classname, as is or in snake_case (Rust module files).
const std::string classname_token = "{classname}";
const std::string classname_snake_token = "{classname_snake}";

std::string replace_token(std::string pattern, const std::string& token, const std::string& value)
{
    for (auto pos = pattern.find(token); pos != std::string::npos;
         pos = pattern.find(token, pos + value.size()))
    {
        pattern.replace(pos, token.size(), value);
    }
    return pattern;
}

std::string expand_filename(const std::string& pattern, const std::string& classname)
{
    return replace_token(replace_token(pattern, classname_token, classname), classname_snake_token, snake_case(classname));
}

std::string printable(const std::string& value)
{
    std::string out(value);
    for (auto& c : out)
    {
        if (c < 0x20 || c == 0x7f)
        {
            c = '?';
        }
    }
    return out;
}

bool spec_error(int& errorcode, const std::string& what)
{
    errorcode |= SW_SPEC_INVALID;
    std::cerr << "Invalid specification: " << what << std::endl;
    return false;
}

}

bool Processor::process_string(const std::string& input_msg_str, const std::string& input_lang_str, results_t& output_str_map, const std::string& outputdir, int& errorcode, bool generate_tests, bool write_to_file, bool generate_relay)
{
    errorcode = SW_SUCCESS;
    bool success = false;

    if (input_msg_str.empty())
    {
        errorcode |= SW_BLANKINPUTSTRING;
        std::cerr << "Blank input lang string" << std::endl;
        return success;
    }

    if (input_lang_str.empty())
    {
        errorcode |= SW_BLANKINPUTSTRING;
        std::cerr << "Blank input lang string" << std::endl;
        return success;
    }

    spec_dir_.clear();

    if (false == msg_spec_.read_string(input_msg_str))
    {
        errorcode |= SW_STRINGREADERROR;
        std::cerr << "Error reading msg string: " << input_msg_str << std::endl;
        return success;
    }

    if (false == lang_spec_.read_string(input_lang_str))
    {
        errorcode |= SW_STRINGREADERROR;
        std::cerr << "Error reading lang string: " << input_lang_str << std::endl;
        return success;
    }

    bool ret = process(output_str_map, errorcode, generate_tests, generate_relay);

    if (true == ret && true == write_to_file && false == output_str_map.empty())
    {
        ret = write_results(outputdir, output_str_map, errorcode);
    }

    return ret;
}

bool Processor::process_file(const std::string& msgpathname, const std::string& langpathname, results_t& output_str_map, const std::string& outputdir, int& errorcode, bool generate_tests, bool write_to_file, bool generate_relay)
{
    errorcode = SW_SUCCESS;
    bool success = false;

    if (msgpathname.empty())
    {
        errorcode |= SW_BLANKINPUTFILENAME;
        std::cerr << "Blank input file name" << std::endl;
        return success;
    }

    if (langpathname.empty())
    {
        errorcode |= SW_BLANKINPUTFILENAME;
        std::cerr << "Blank input file name" << std::endl;
        return success;
    }

    spec_dir_ = std::filesystem::path(msgpathname).parent_path().string();
    if (spec_dir_.empty())
    {
        spec_dir_ = ".";
    }

    if (false == msg_spec_.read_file(msgpathname))
    {
        errorcode |= SW_FILEREADERROR;
        std::cerr << "Error reading msg file: " << msgpathname << std::endl;
        return success;
    }

    if (false == lang_spec_.read_file(langpathname))
    {
        errorcode |= SW_FILEREADERROR;
        std::cerr << "Error reading lang file: " << langpathname << std::endl;
        return success;
    }

    success = process(output_str_map, errorcode, generate_tests, generate_relay);
    if (false == success)
    {
        return success;
    }

    if (true == write_to_file && false == output_str_map.empty())
    {
        success = write_results(outputdir, output_str_map, errorcode);
    }

    return success;
}

bool Processor::process(results_t& output_str_map, int& errorcode, bool generate_tests, bool generate_relay)
{
    bool success = false;

    if (false == namespace_.empty())
    {
        (*msg_spec_.get_proto_ref().mutable_string_values())["namespace"] = namespace_;
    }

    resolve_output_filenames();
    resolve_includes();

    if (false == validate_spec(errorcode))
    {
        return success;
    }

    // The language belongs to the lang file, so one message spec serves every language.
    std::string lang = lang_spec_.get_value_string(language_key_);

    if (lang.empty())
    {
        errorcode |= SW_UNKNOWNLANG;
        std::cerr << "The lang file does not name a language (its \"" << language_key_ << "\" value)" << std::endl;
        return success;
    }

    if (false == expected_language_.empty() && lang != expected_language_)
    {
        errorcode |= SW_UNKNOWNLANG;
        std::cerr << "The lang file is for " << lang << ", not " << expected_language_ << std::endl;
        return success;
    }

    if (msg_spec_.get_proto().string_values().count(language_key_) != 0)
    {
        std::cerr << "Warning: the specification's \"" << language_key_ << "\" value is ignored; "
                  << "the language comes from the lang file" << std::endl;
    }

    std::unique_ptr<skyloupe::Generator> genny;
    std::unique_ptr<skyloupe::Generator> test_genny;
    std::unique_ptr<skyloupe::Generator> relay_genny;

    // temporary - replace with factory, dynamic language definition via json
    //             maybe a plugin for each language?
    if (lang == swift_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::SwiftGenerator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::SwiftTestGenerator(msg_spec_, lang_spec_));
        }

        if (true == generate_relay)
        {
            relay_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::SwiftRelayGenerator(msg_spec_, lang_spec_));
        }
    }
    else if (lang == c_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CGenerator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CTestGenerator(msg_spec_, lang_spec_));
        }

        if (true == generate_relay)
        {
            relay_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CRelayGenerator(msg_spec_, lang_spec_));
        }
    }
    else if (lang == cpp_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppGenerator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppTestGenerator(msg_spec_, lang_spec_));
        }

        if (true == generate_relay)
        {
            relay_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppRelayGenerator(msg_spec_, lang_spec_));
        }
    }
    else if (lang == go_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::GoGenerator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::GoTestGenerator(msg_spec_, lang_spec_));
        }

        if (true == generate_relay)
        {
            relay_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::GoRelayGenerator(msg_spec_, lang_spec_));
        }
    }
    else if (lang == java_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::JavaGenerator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::JavaTestGenerator(msg_spec_, lang_spec_));
        }

        if (true == generate_relay)
        {
            relay_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::JavaRelayGenerator(msg_spec_, lang_spec_));
        }
    }
    else if (lang == proto3_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::Proto3Generator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::Proto3TestGenerator(msg_spec_, lang_spec_));
        }
    }
    else if (lang == py3_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::Py3Generator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::Py3TestGenerator(msg_spec_, lang_spec_));
        }

        if (true == generate_relay)
        {
            relay_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::Py3RelayGenerator(msg_spec_, lang_spec_));
        }
    }
    else if (lang == rust_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::RustGenerator(msg_spec_, lang_spec_));

        if (true == generate_tests)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::RustTestGenerator(msg_spec_, lang_spec_));
        }
    }
    else
    {
        errorcode |= SW_UNKNOWNLANG;
        std::cerr << "Unsupported language: " << lang << std::endl;
        return success;
    }

    // A relay checks the values inside nested messages, so it needs their specs.
    if (relay_genny)
    {
        std::map<std::string, Specification> nested_specs;
        std::vector<std::string> stack = { msg_spec_.get_value_string("classname") };
        if (false == load_nested_specs(msg_spec_, nested_specs, stack, errorcode))
        {
            return success;
        }
        relay_genny->set_nested_specs(nested_specs);
    }

    success = genny->generate(errorcode);
    if (true == success && true == generate_tests && test_genny)
    {
        success = test_genny->generate(errorcode);
    }
    if (true == success && relay_genny)
    {
        success = relay_genny->generate(errorcode);
    }

    if (success)
    {
        auto t1 = genny->get_results(); // inefficient, refactor
        output_str_map.insert(t1.begin(), t1.end());
        if (true == generate_tests && test_genny)
        {
            auto t2 = test_genny->get_results();
            output_str_map.insert(t2.begin(), t2.end());
        }
        if (relay_genny)
        {
            auto t3 = relay_genny->get_results();
            output_str_map.insert(t3.begin(), t3.end());
        }
    }
    // end temporary

    return success;
}

void Processor::resolve_output_filenames()
{
    // The lang file gives each output a file name pattern (".h", "_test.go",
    // ...); a name in the spec takes its place. Either may use {classname}
    // or {classname_snake}.
    auto& values = *msg_spec_.get_proto_ref().mutable_string_values();
    const auto& patterns = lang_spec_.get_proto().string_values();

    auto classname_value = values.find("classname");
    const std::string classname = classname_value == values.end() ? "" : classname_value->second;

    for (const auto& slot : output_slots)
    {
        auto value = values.find(slot.second);
        if (value != values.end())
        {
            value->second = expand_filename(value->second, classname);
            continue;
        }

        auto pattern = patterns.find(slot.second);
        if (pattern != patterns.end())
        {
            values[slot.second] = expand_filename(pattern->second, classname);
        }
    }
}

void Processor::resolve_includes()
{
    // The lang file lists what generated code always includes (its runtime
    // headers); the spec's own includes follow, without repeats.
    const auto& defaults = lang_spec_.get_proto().includes();
    if (defaults.empty())
    {
        return;
    }

    auto& includes = *msg_spec_.get_proto_ref().mutable_includes();
    std::vector<std::string> merged(defaults.begin(), defaults.end());
    for (const auto& include : includes)
    {
        if (std::find(merged.begin(), merged.end(), include) == merged.end())
        {
            merged.push_back(include);
        }
    }

    includes.Clear();
    for (const auto& include : merged)
    {
        includes.Add()->assign(include);
    }
}

// Reads the spec of every nested message type that spec uses, recursively,
// from spec_dir_/<type>.json. stack holds the messages being loaded, to
// reject a message that contains itself.
bool Processor::load_nested_specs(Specification& spec, std::map<std::string, Specification>& specs, std::vector<std::string>& stack, int& errorcode)
{
    std::set<std::string> native_types(lang_spec_.get_proto().native_types().begin(), lang_spec_.get_proto().native_types().end());

    for (const auto& member : spec.get_proto().members())
    {
        const std::string& type = member.second.type();
        if (type == "string" || native_types.count(type) != 0)
        {
            continue;
        }

        if (std::find(stack.begin(), stack.end(), type) != stack.end())
        {
            errorcode |= SW_SPEC_INVALID;
            std::cerr << "nested type '" << type << "' contains itself" << std::endl;
            return false;
        }

        if (specs.count(type) != 0)
        {
            continue;
        }

        const std::string pathname = (std::filesystem::path(spec_dir_) / (type + ".json")).string();
        Specification nested;
        if (spec_dir_.empty() || false == nested.read_file(pathname))
        {
            errorcode |= SW_FILEREADERROR;
            std::cerr << "nested type '" << type << "': cannot read its specification "
                      << (spec_dir_.empty() ? type + ".json (no spec directory)" : pathname) << std::endl;
            return false;
        }

        if (nested.get_value_string("classname") != type)
        {
            errorcode |= SW_SPEC_INVALID;
            std::cerr << pathname << ": classname is not '" << type << "'" << std::endl;
            return false;
        }

        specs[type] = nested;
        stack.push_back(type);
        bool ok = load_nested_specs(specs[type], specs, stack, errorcode);
        stack.pop_back();
        if (false == ok)
        {
            return false;
        }
    }

    return true;
}

bool Processor::validate_spec(int& errorcode)
{
    bool valid = true;
    const auto& proto = msg_spec_.get_proto();
    const auto& values = proto.string_values();

    auto classname = values.find("classname");
    if (classname == values.end() || false == std::regex_match(classname->second, identifier_re))
    {
        valid = spec_error(errorcode, "classname must be present and an identifier");
    }

    auto name_space = values.find("namespace");
    if (name_space != values.end() && false == std::regex_match(name_space->second, qualified_re))
    {
        valid = spec_error(errorcode, "namespace '" + printable(name_space->second) + "' must be identifiers joined by '.' or '::'");
    }

    auto bitorder = values.find("bitorder");
    if (bitorder != values.end() && bitorder->second != "lsb" && bitorder->second != "msb")
    {
        valid = spec_error(errorcode, "bitorder '" + printable(bitorder->second) + "' must be lsb or msb");
    }

    for (const auto& slot : output_slots)
    {
        auto file = values.find(slot.second);
        if (file != values.end() &&
            (false == std::regex_match(file->second, filename_re) || file->second == "." || file->second == ".."))
        {
            valid = spec_error(errorcode, std::string(slot.second) + " '" + printable(file->second) + "' must be a plain file name");
        }
    }

    for (const auto& include : proto.includes())
    {
        if (false == std::regex_match(include, include_re))
        {
            valid = spec_error(errorcode, "include '" + printable(include) + "' is not a plain path, <path> or \"path\"");
        }
    }

    if (proto.members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        valid = spec_error(errorcode, "no members");
    }

    std::set<std::string> native_types(lang_spec_.get_proto().native_types().begin(), lang_spec_.get_proto().native_types().end());
    std::set<std::string> names;

    for (const auto& member : proto.members())
    {
        const auto& m = member.second;
        const std::string where = "member " + std::to_string(member.first) + ": ";

        if (member.first == 0 || member.first > max_field_number)
        {
            valid = spec_error(errorcode, where + "tag must be from 1 to " + std::to_string(max_field_number));
        }
        else if (member.first >= first_reserved_field_number && member.first <= last_reserved_field_number)
        {
            valid = spec_error(errorcode, where + "tags " + std::to_string(first_reserved_field_number) + "-" +
                               std::to_string(last_reserved_field_number) + " are reserved by protobuf");
        }

        if (false == std::regex_match(m.name(), identifier_re))
        {
            valid = spec_error(errorcode, where + "name '" + printable(m.name()) + "' must be an identifier");
        }
        else if (false == names.insert(m.name()).second)
        {
            valid = spec_error(errorcode, where + "name '" + m.name() + "' is used by another member");
        }

        if (m.type() != "string" && native_types.count(m.type()) == 0 && false == std::regex_match(m.type(), identifier_re))
        {
            valid = spec_error(errorcode, where + "type '" + printable(m.type()) + "' is neither a native type nor an identifier");
        }

        if (m.type() == "string" && m.length() == 0)
        {
            valid = spec_error(errorcode, where + "string needs a length of at least 1");
        }

        if (m.bits() != 0 && bitfields::width(m.type()) == 0)
        {
            valid = spec_error(errorcode, where + "bits needs a type from uint8_t to int64_t, not '" + printable(m.type()) + "'");
        }
        else if (m.bits() > bitfields::width(m.type()))
        {
            valid = spec_error(errorcode, where + "bits " + std::to_string(m.bits()) + " is wider than " + m.type());
        }

        if (m.endian() > SW_NATIVE_ENDIAN_FLAG)
        {
            valid = spec_error(errorcode, where + "endian " + std::to_string(m.endian()) + " is not 0 (big), 1 (little) or 2 (native)");
        }
    }

    return valid;
}

bool Processor::write_results(const std::string& outputdir, const results_t& output_str_map, int& errorcode)
{
    const auto& values = msg_spec_.get_proto().string_values();

    // Check every destination before writing any, so a bad spec leaves no partial output.
    for (const auto& slot : output_slots)
    {
        if (output_str_map.count(slot.first) != 0 && values.find(slot.second) == values.end())
        {
            return spec_error(errorcode, std::string(slot.second) + " is required to write the " + slot.first + " output");
        }
    }

    bool success = true;

    for (const auto& slot : output_slots)
    {
        auto result = output_str_map.find(slot.first);
        if (result == output_str_map.end())
        {
            continue;
        }

        std::string pathname_str = outputdir + "/" + values.at(slot.second);

        std::unique_ptr<skyloupe::ResultPresenter> presenter(new skyloupe::FileWriter(pathname_str, result->second));
        if (false == presenter->present(errorcode))
        {
            errorcode |= SW_FILEWRITEERROR;
            success = false;
        }
    }

    return success;
}

}