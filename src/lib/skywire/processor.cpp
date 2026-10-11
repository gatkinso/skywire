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

#include <regex>
#include <set>
#include "file_writer.h"
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

bool Processor::process_string(const std::string& input_msg_str, const std::string& input_lang_str, results_t& output_str_map, const std::string& outputdir, int& errorcode, bool generate_tests, bool write_to_file)
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

    bool ret = process(output_str_map, errorcode, generate_tests);

    if (true == ret && true == write_to_file && false == output_str_map.empty())
    {
        ret = write_results(outputdir, output_str_map, errorcode);
    }

    return ret;
}

bool Processor::process_file(const std::string& msgpathname, const std::string& langpathname, const std::string& outputdir, int& errorcode, bool generate_tests, bool write_to_file)
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

    if (false == msg_spec_.read_file(msgpathname))
    {
        errorcode |= SW_FILEREADERROR;
        std::cerr << "Error reading msg file: " << msgpathname << std::endl;
        return success;
    }

    if (false == lang_spec_.read_file(langpathname))
    {
        errorcode |= SW_FILEREADERROR;
        std::cerr << "Error reading land file: " << langpathname << std::endl;
        return success;
    }

    results_t output_str_map;
    success = process(output_str_map, errorcode, generate_tests);
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

bool Processor::process(results_t& output_str_map, int& errorcode, bool generate_tests)
{
    bool success = false;

    if (false == validate_spec(errorcode))
    {
        return success;
    }

    std::string lang = msg_spec_.get_value_string(language_key_);

    std::unique_ptr<skyloupe::Generator> genny;
    std::unique_ptr<skyloupe::Generator> test_genny;
    std::unique_ptr<skyloupe::Generator> relay_genny;

    // A spec that names a relay file also gets the cross-language relay program
    // (see relaygenerators.h) whenever tests are generated.
    const bool generate_relay = generate_tests && msg_spec_.get_proto().string_values().count("outputrelayfilename") != 0;

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
    else
    {
        errorcode |= SW_UNKNOWNLANG;
        std::cerr << "Unsupported language: " << lang << std::endl;
        return success;
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