//
//   Copyright (C) 2019 Geoffrey Atkinson
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
    bool have_wire = expected_wire(wire);
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
        // clear() drops what was written; the stream is then reused.
        ss_ << "        check(input.serialize(wire), \"serialize failed\")\n";
        ss_ << "        wire.clear()\n";
        ss_ << "        check(wire.remaining() == 0, \"clear() left bytes\")\n";
        ss_ << "        check(input.serialize(wire), \"serialize after clear() failed\")\n";
        ss_ << "        check(wire.read(wire.remaining())! == want, \"wire mismatch after clear()\")\n";
    }

    // One stream reused for many messages, reading two of every three it
    // writes: unread bytes build up, so growing slides them to the front, and
    // draining the rest empties the stream.
    ss_ << "\n";
    ss_ << "        let reused = SWStream()\n";
    ss_ << "        var pending = 0\n";
    ss_ << "        for i in 0..<1000 {\n";
    ss_ << "            check(input.serialize(reused), \"reused stream serialize failed\")\n";
    ss_ << "            pending += 1\n";
    ss_ << "            if i % 3 != 2 {\n";
    ss_ << "                let got = " << classname << "()\n";
    ss_ << "                check(got.deserialize(reused) && input.Equals(got), \"reused stream round-trip mismatch\")\n";
    ss_ << "                pending -= 1\n";
    ss_ << "            }\n";
    ss_ << "            check(reused.remaining() == pending * input.size(), \"reused stream byte count\")\n";
    ss_ << "        }\n";
    ss_ << "        check(reused.byteSize() == reused.remaining(), \"byteSize() is not the unread byte count\")\n";
    ss_ << "        while pending > 0 {\n";
    ss_ << "            let got = " << classname << "()\n";
    ss_ << "            check(got.deserialize(reused) && input.Equals(got), \"reused stream drain mismatch\")\n";
    ss_ << "            pending -= 1\n";
    ss_ << "        }\n";
    ss_ << "        check(reused.remaining() == 0, \"reused stream not drained\")\n";

    do_text_tests(classname);
    do_bson_tests(classname);

    // A bitfield setter keeps only the field's bits: a value with a bit set
    // above the field reads back as the field's value.
    for (auto& member : ordered_members_)
    {
        const std::string wide = out_of_range_hex(member.second);
        if (wide.empty())
        {
            continue;
        }
        const std::string& name = std::get<member_name>(member.second);
        const std::string type = lang_spec_.get_value_string(std::get<member_type>(member.second));
        ss_ << "        input.Set_" << name << "(" << type << "(truncatingIfNeeded: UInt64(" << wide << ")))\n";
        ss_ << "        check(input.Get_" << name << "() == " << test_literal(member.second) << ", \"" << name << " setter kept bits outside the field\")\n";
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
        return "        " + target + ".Set_" + name + "(" + test_literal(member) + ")\n";
    }
    return "";
}

namespace
{

// bytes as a Swift [UInt8] literal, eight to a line, at the given indent.
std::string byte_array(const std::string& bytes, const std::string& indent)
{
    if (bytes.empty())
    {
        return "[]";
    }
    std::string out = "[";
    for (size_t i = 0; i < bytes.size(); i++)
    {
        out += (i % 8 == 0 ? "\n" + indent + "    " : " ");
        char hex[8];
        std::snprintf(hex, sizeof(hex), "0x%02x,", static_cast<unsigned char>(bytes[i]));
        out += hex;
    }
    return out + "\n" + indent + "]";
}

std::string byte_array(const std::vector<uint8_t>& bytes, const std::string& indent)
{
    return byte_array(std::string(bytes.begin(), bytes.end()), indent);
}

}

// The JSON and XML checks: the exact text (the same in every language, from
// test_values), a round trip, from bytes too, an empty object or element
// reading as the default, and malformed or out-of-range input failing.
void SwiftTestGenerator::do_text_tests(const std::string& classname)
{
    for (const std::string format : { "json", "xml" })
    {
        if ((format == "json" && false == wants_json()) || (format == "xml" && false == wants_xml()))
        {
            continue;
        }
        const bool json = format == "json";
        const std::string name = json ? "Json" : "Xml";
        const std::string label = json ? "JSON" : "XML";
        std::string expected;
        const bool have_expected = json ? expected_json(expected) : expected_xml(expected);

        ss_ << "\n";
        ss_ << "        let " << format << "Text = input.to" << name << "()\n";
        if (have_expected)
        {
            ss_ << "        let " << format << "Want: [UInt8] = " << byte_array(expected, "        ") << "\n";
            ss_ << "        check(Array(" << format << "Text.utf8) == " << format << "Want, \"" << label << " mismatch: \\(" << format
                << "Text)\")\n";
        }
        ss_ << "        do {\n";
        ss_ << "            let got = try " << classname << "(" << format << ": " << format << "Text)\n";
        ss_ << "            check(got.Equals(input), \"" << label << " round-trip value mismatch\")\n";
        ss_ << "            check(got.to" << name << "() == " << format << "Text, \"" << label << " written again differs\")\n";
        ss_ << "            let fromBytes = try " << classname << "(" << format << ": Array(" << format << "Text.utf8))\n";
        ss_ << "            check(fromBytes.Equals(input), \"" << label << " from bytes differs\")\n";
        ss_ << "            let empty = try " << classname << "(" << format << ": \""
            << (json ? "{}" : "<" + classname + "/>") << "\")\n";
        ss_ << "            check(empty.Equals(" << classname << "()), \"an empty " << label << " message is not the default\")\n";
        if (json)
        {
            // Swift's String equality would match a name that is only
            // canonically equivalent to a member's (KELVIN SIGN for K).
            ss_ << "            let kelvin = try SkywireText.parseJson(\"{\\\"\\u{212A}\\\":1}\").members()\n";
            ss_ << "            check(kelvin[0].name != \"K\", \"a name only canonically equivalent to K matched it\")\n";
        }
        ss_ << "        } catch {\n";
        ss_ << "            check(false, \"" << label << " read failed: \\(error)\")\n";
        ss_ << "        }\n";
        ss_ << "        var " << format << "Rejects: [[UInt8]] = []\n";
        for (const auto& text : json ? rejected_json() : rejected_xml())
        {
            ss_ << "        " << format << "Rejects.append(" << byte_array(text, "        ") << ")\n";
        }
        ss_ << "        for text in " << format << "Rejects {\n";
        ss_ << "            check((try? " << classname << "(" << format << ": text)) == nil, \"" << label
            << " accepted \\(String(decoding: text, as: UTF8.self))\")\n";
        ss_ << "        }\n";
    }
}

// The BSON checks: the exact bytes (from test_values), a round trip, an
// empty document reading as the default, uint64_t values above 2^63-1 as
// Decimal128, and malformed or out-of-range documents failing.
void SwiftTestGenerator::do_bson_tests(const std::string& classname)
{
    if (false == wants_bson())
    {
        return;
    }
    std::vector<uint8_t> expected;
    const bool have_expected = expected_bson(expected);

    ss_ << "\n";
    ss_ << "        let bson = input.toBson()\n";
    if (have_expected)
    {
        ss_ << "        let bsonWant: [UInt8] = " << byte_array(expected, "        ") << "\n";
        ss_ << "        check(bson == bsonWant, \"BSON mismatch: \\(bson)\")\n";
    }
    ss_ << "        do {\n";
    ss_ << "            let got = try " << classname << "(bson: bson)\n";
    ss_ << "            check(got.Equals(input), \"BSON round-trip value mismatch\")\n";
    ss_ << "            check(got.toBson() == bson, \"BSON written again differs\")\n";
    ss_ << "            let empty = try " << classname << "(bson: [5, 0, 0, 0, 0])\n";
    ss_ << "            check(empty.Equals(" << classname << "()), \"an empty BSON document is not the default\")\n";

    // A uint64_t above 2^63-1 is a Decimal128, so no tool sees it negative;
    // 2^63-1 itself is still an int64.
    for (auto& member : ordered_members_)
    {
        const std::string& name = std::get<member_name>(member.second);
        if (std::get<member_type>(member.second) != "uint64_t" || is_bitfield(member.second))
            continue;
        std::vector<uint8_t> big, edge;
        test_values::bson_decimal128(name, 0, 0xFFFFFFFFFFFFFFFFull, false, big);
        test_values::bson_int64(name, 0x7FFFFFFFFFFFFFFFll, edge);
        ss_ << "            let " << name << "Big: [UInt8] = " << byte_array(big, "            ") << "\n";
        ss_ << "            let " << name << "Edge: [UInt8] = " << byte_array(edge, "            ") << "\n";
        ss_ << "            for (value, element) in [(UInt64.max, " << name << "Big), (UInt64.max >> 1, " << name << "Edge)] {\n";
        ss_ << "                let msg = " << classname << "()\n";
        ss_ << "                msg.Set_" << name << "(value)\n";
        ss_ << "                let data = msg.toBson()\n";
        ss_ << "                check((0...(data.count - element.count)).contains { Array(data[$0..<($0 + element.count)]) == element },\n";
        ss_ << "                      \"" << name << " = \\(value) is not \\(element)\")\n";
        ss_ << "                check(try " << classname << "(bson: data).Equals(msg), \"" << name << " did not read back\")\n";
        ss_ << "            }\n";
    }
    ss_ << "        } catch {\n";
    ss_ << "            check(false, \"BSON read failed: \\(error)\")\n";
    ss_ << "        }\n";

    ss_ << "        let good = " << classname << "().toBson()\n";
    ss_ << "        var miscounted = good\n";
    ss_ << "        miscounted[0] += 1\n";
    ss_ << "        var bsonRejects: [[UInt8]] = [Array(good.dropLast()), good + [0], miscounted]\n";
    for (const auto& doc : rejected_bson())
    {
        ss_ << "        bsonRejects.append(" << byte_array(doc, "        ") << ")\n";
    }
    ss_ << "        for doc in bsonRejects {\n";
    ss_ << "            check((try? " << classname << "(bson: doc)) == nil, \"BSON accepted \\(doc)\")\n";
    ss_ << "        }\n";
}

}
