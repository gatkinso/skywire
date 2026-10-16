//
//   Copyright (C) 2019 Skyloupe, LLC
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

#include <map>
#include <cstdio>
#include "swifttestgenerator.h"
#include "test_values.h"

namespace skyloupe
{

bool SwiftTestGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    if (false == do_test_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

bool SwiftTestGenerator::do_test_implementation(int& errorcode)
{
    std::string& test_string = results_["unittest"];

    std::string classname = msg_spec_.get_value_string("classname");

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    ss_ << "// This is the generated Swift round-trip test\n\n";
    ss_ << "import Foundation\n\n";
    ss_ << "@main\n";
    ss_ << "struct " << classname << "Test {\n";
    ss_ << "    static func check(_ cond: Bool, _ msg: String) {\n";
    ss_ << "        if !cond {\n";
    ss_ << "            print(\"FAIL: \\(msg)\")\n";
    ss_ << "            exit(1)\n";
    ss_ << "        }\n";
    ss_ << "    }\n\n";
    ss_ << "    static func main() {\n";
    ss_ << "        let input = " << classname << "()\n";

    do_sets(errorcode);

    ss_ << "\n";
    ss_ << "        let stream = SWStream()\n";
    ss_ << "        check(input.serialize(stream), \"serialize failed\")\n\n";
    ss_ << "        let output = " << classname << "()\n";
    ss_ << "        check(output.deserialize(stream), \"deserialize failed\")\n\n";
    ss_ << "        check(input.Equals(output), \"round-trip value mismatch\")\n";
    ss_ << "        check(input.size() == output.size(), \"size mismatch\")\n\n";
    // One byte short of a full message: deserialize must fail, consume
    // nothing, and leave the destination untouched.
    ss_ << "        let full = SWStream()\n";
    ss_ << "        check(input.serialize(full), \"serialize failed\")\n";
    ss_ << "        let short = SWStream()\n";
    ss_ << "        short.write(full.read(input.size() - 1)!)\n";
    ss_ << "        let untouched = " << classname << "()\n";
    ss_ << "        check(!untouched.deserialize(short), \"short buffer accepted\")\n";
    ss_ << "        check(untouched.Equals(" << classname << "()), \"short buffer modified the destination\")\n";
    ss_ << "        check(short.remaining() == input.size() - 1, \"short buffer consumed bytes\")\n";

    // The expected bytes are computed by wiregen, shared with every language.
    std::vector<uint8_t> wire;
    bool have_wire = true;
    for (auto& member : ordered_members_)
    {
        have_wire = have_wire && test_values::encode(std::get<member_type>(member.second),
                                                     std::get<member_endianness>(member.second),
                                                     std::get<member_length>(member.second), wire);
    }
    if (have_wire)
    {
        ss_ << "\n";
        ss_ << "        let want: [UInt8] = [";
        for (size_t i = 0; i < wire.size(); i++)
        {
            ss_ << ((i % 8 == 0) ? "\n            " : " ");
            char hex[8];
            std::snprintf(hex, sizeof(hex), "0x%02x,", wire[i]);
            ss_ << hex;
        }
        ss_ << "\n        ]\n";
        ss_ << "        let wire = SWStream()\n";
        ss_ << "        check(input.serialize(wire), \"serialize failed\")\n";
        ss_ << "        check(wire.read(wire.remaining())! == want, \"wire mismatch\")\n";
    }
    ss_ << "        print(\"PASS " << classname << " round-trip\")\n";
    ss_ << "    }\n";
    ss_ << "}\n";

    test_string.append(ss_.str());

    return true;
}

// Every member gets the value shared by all the test generators.
bool SwiftTestGenerator::do_sets(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        ss_ << set_line("input", member.second);
    }

    return true;
}

// The statement that sets one member of target to its shared test value, or
// nothing for a member with no test value (a nested message).
std::string SwiftTestGenerator::set_line(const std::string& target, const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const std::string& name = std::get<member_name>(member);

    if (is_string_type(type))
    {
        return "        " + target + ".Set_" + name + "(Array(\"" + test_values::string_value() + "\".utf8))\n";
    }
    if (test_values::literal(type))
    {
        return "        " + target + ".Set_" + name + "(" + test_values::literal(type) + ")\n";
    }
    return "";
}

}
