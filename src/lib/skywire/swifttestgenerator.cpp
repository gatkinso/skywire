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
#include "swifttestgenerator.h"

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
    ss_ << "        print(\"PASS " << classname << " round-trip\")\n";
    ss_ << "    }\n";
    ss_ << "}\n";

    test_string.append(ss_.str());

    return true;
}

bool SwiftTestGenerator::do_sets(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        const std::string& name = std::get<member_name>(member.second);

        ss_ << "        input.Set_" << name << "(";

             if (type == "char")          ss_ << "65";
        else if (type == "unsigned char") ss_ << "200";
        else if (type == "uint8_t")       ss_ << "201";
        else if (type == "int8_t")        ss_ << "-50";
        else if (type == "uint16_t")      ss_ << "60000";
        else if (type == "int16_t")       ss_ << "-12345";
        else if (type == "uint32_t")      ss_ << "3000000000";
        else if (type == "int32_t")       ss_ << "-2000000000";
        else if (type == "uint64_t")      ss_ << "12000000000";
        else if (type == "int64_t")       ss_ << "-9000000000";
        else if (type == "float")         ss_ << "3.5";
        else if (type == "double")        ss_ << "3.14159";
        else if (type == "string")        ss_ << "Array(\"skywire\".utf8)";

        ss_ << ")\n";
    }

    return true;
}

}
