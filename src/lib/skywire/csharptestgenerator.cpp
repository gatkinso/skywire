//
//   Copyright (C) 2026 Geoffrey Atkinson
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

#include <algorithm>
#include <cstdio>
#include "csharpgenerator.h"
#include "csharptestgenerator.h"
#include "test_values.h"

namespace skyloupe
{

bool CSharpTestGenerator::generate(int& errorcode)
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

// The generated test needs no test framework, so it runs with only a C#
// compiler. Run() prints a PASS or FAIL line per test and returns the number
// of failures; Main(), left out when SKYWIRE_NO_MAIN is defined so that
// several tests can build into one program, exits non-zero if any failed.
bool CSharpTestGenerator::do_test_implementation(int& errorcode)
{
    std::string& test_string = results_["unittest"];

    const std::string classname = msg_spec_.get_value_string("classname");
    const std::string ns = msg_spec_.get_value_string("namespace", "@NOTFOUND");

    // Named by the test file, {classname}Test.cs.
    std::string testclass = msg_spec_.get_value_string("outputtestfilename", classname + "Test.cs");
    testclass = testclass.substr(0, testclass.rfind(".cs"));

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    std::vector<uint8_t> wire;
    const bool have_wire = expected_wire(wire);
    bool have_string = false;
    for (auto& member : ordered_members_)
    {
        have_string = have_string || is_string_type(std::get<member_type>(member.second));
    }

    std::stringstream body;

    body << "public static class " << testclass << "\n";
    body << "{\n";
    body << "    private static " << classname << " MakeMessage()\n";
    body << "    {\n";
    body << "        " << classname << " msg = new " << classname << "();\n";
    for (auto& member : ordered_members_)
    {
        body << set_line("msg", member.second);
    }
    body << "        return msg;\n";
    body << "    }\n\n";

    body << "    private static void Check(bool cond, string message)\n";
    body << "    {\n";
    body << "        if (!cond)\n";
    body << "        {\n";
    body << "            throw new InvalidOperationException(message);\n";
    body << "        }\n";
    body << "    }\n\n";

    body << "    private static void TestRoundTrip()\n";
    body << "    {\n";
    body << "        " << classname << " input = MakeMessage();\n";
    body << "        byte[] wire = input.Serialize();\n";
    body << "        Check(wire.Length == input.Size(), \"serialized \" + wire.Length + \" bytes, Size() is \" + input.Size());\n";
    body << "        " << classname << " output = new " << classname << "();\n";
    body << "        output.Deserialize(wire);\n";
    body << "        Check(input.Equals(output), \"round-trip value mismatch\\n in \" + input + \"\\nout \" + output);\n";
    body << "        Check(input.Equals((object)output), \"Equals(object) differs from Equals\");\n";
    body << "        Check(input.GetHashCode() == output.GetHashCode(), \"GetHashCode differs after round-trip\");\n";
    body << "        Check(!input.Equals(null), \"Equals(null) is true\");\n";
    body << "    }\n\n";

    // Each member takes part in Equals: clearing one makes the messages differ.
    body << "    private static void TestEquals()\n";
    body << "    {\n";
    body << "        " << classname << " msg;\n";
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        const std::string& name = std::get<member_name>(member.second);
        std::string clear;
        if (is_string_type(type))
            clear = "msg.Set_" + name + "(new byte[0]);";
        else if (test_values::literal(type))
            clear = "msg." + CSharpGenerator::identifier(name) + " = 0;";
        else
            continue;

        body << "        msg = MakeMessage();\n";
        body << "        " << clear << "\n";
        body << "        Check(!msg.Equals(MakeMessage()), \"Equals ignores " << name << "\");\n";
    }
    body << "    }\n\n";

    body << "    private static void ExpectArgumentException(Action action, string what)\n";
    body << "    {\n";
    body << "        try\n";
    body << "        {\n";
    body << "            action();\n";
    body << "        }\n";
    body << "        catch (ArgumentException)\n";
    body << "        {\n";
    body << "            return;\n";
    body << "        }\n";
    body << "        throw new InvalidOperationException(\"expected an ArgumentException from \" + what);\n";
    body << "    }\n\n";

    body << "    private static void TestShortBuffer()\n";
    body << "    {\n";
    body << "        " << classname << " msg = MakeMessage();\n";
    body << "        byte[] wire = msg.Serialize();\n";
    body << "        byte[] shorter = new byte[wire.Length - 1];\n";
    body << "        Array.Copy(wire, shorter, shorter.Length);\n";
    body << "        ExpectArgumentException(() => new " << classname << "().Deserialize(shorter), \"Deserialize\");\n";
    body << "        ExpectArgumentException(() => { int offset = 0; msg.Serialize(shorter, ref offset); }, \"Serialize\");\n";
    body << "        ExpectArgumentException(() => { int offset = -1; msg.Serialize(wire, ref offset); }, \"Serialize at offset -1\");\n";
    body << "    }\n\n";

    // The offset overloads are what nested messages use: they must start at
    // the offset, write or read exactly Size() bytes, and advance past them.
    body << "    private static void TestOffset()\n";
    body << "    {\n";
    body << "        " << classname << " input = MakeMessage();\n";
    body << "        byte[] buf = new byte[3 + input.Size() + 2];\n";
    body << "        int offset = 3;\n";
    body << "        input.Serialize(buf, ref offset);\n";
    body << "        Check(offset == 3 + input.Size(), \"Serialize(buf, ref offset) wrote \" + (offset - 3) + \" bytes, Size() is \" + input.Size());\n";
    body << "        byte[] wire = input.Serialize();\n";
    body << "        for (int i = 0; i < buf.Length; i++)\n";
    body << "        {\n";
    body << "            byte want = i >= 3 && i < 3 + wire.Length ? wire[i - 3] : (byte)0;\n";
    body << "            Check(buf[i] == want, \"Serialize(buf, ref offset) differs from Serialize() at byte \" + i);\n";
    body << "        }\n";
    body << "        offset = 3;\n";
    body << "        " << classname << " output = new " << classname << "();\n";
    body << "        output.Deserialize(buf, ref offset);\n";
    body << "        Check(offset == 3 + input.Size(), \"Deserialize(buf, ref offset) read \" + (offset - 3) + \" bytes, Size() is \" + input.Size());\n";
    body << "        Check(input.Equals(output), \"offset round-trip value mismatch\\n in \" + input + \"\\nout \" + output);\n";
    body << "    }\n";

    // A bitfield setter keeps only the field's bits: a value with a bit set
    // above the field reads back as the field's value.
    std::string setters;
    for (auto& member : ordered_members_)
    {
        const std::string wide = out_of_range_hex(member.second);
        if (wide.empty())
        {
            continue;
        }
        const std::string property = "msg." + CSharpGenerator::identifier(std::get<member_name>(member.second));
        const std::string& type = std::get<member_type>(member.second);
        setters += "        " + property + " = " + csharp_literal(type, wide) + ";\n";
        setters += "        Check(" + property + " == " + csharp_literal(type, test_literal(member.second)) +
                   ", \"" + std::get<member_name>(member.second) + ": got \" + " + property + ");\n";
    }
    if (false == setters.empty())
    {
        body << "\n    private static void TestBitfieldSetters()\n";
        body << "    {\n";
        body << "        " << classname << " msg = new " << classname << "();\n";
        body << setters;
        body << "    }\n";
    }

    if (have_wire)
    {
        body << "\n"
             << expected_wire_test(wire);
    }

    std::vector<std::string> text_tests;
    body << text_tests_code(classname, text_tests);

    body << "\n";
    body << "    private static int RunTest(string name, Action test)\n";
    body << "    {\n";
    body << "        try\n";
    body << "        {\n";
    body << "            test();\n";
    body << "        }\n";
    body << "        catch (Exception e)\n";
    body << "        {\n";
    body << "            Console.WriteLine(\"FAIL " << classname << " \" + name + \": \" + e.Message);\n";
    body << "            return 1;\n";
    body << "        }\n";
    body << "        Console.WriteLine(\"PASS " << classname << " \" + name);\n";
    body << "        return 0;\n";
    body << "    }\n\n";

    body << "    /// <summary>Runs every test, and returns the number that failed.</summary>\n";
    body << "    public static int Run()\n";
    body << "    {\n";
    body << "        int failures = 0;\n";
    body << "        failures += RunTest(\"RoundTrip\", TestRoundTrip);\n";
    body << "        failures += RunTest(\"Equals\", TestEquals);\n";
    body << "        failures += RunTest(\"ShortBuffer\", TestShortBuffer);\n";
    body << "        failures += RunTest(\"Offset\", TestOffset);\n";
    if (have_wire)
    {
        body << "        failures += RunTest(\"Wire\", TestWire);\n";
    }
    if (false == setters.empty())
    {
        body << "        failures += RunTest(\"BitfieldSetters\", TestBitfieldSetters);\n";
    }
    for (auto& test : text_tests)
    {
        body << "        failures += RunTest(\"" << test << "\", Test" << test << ");\n";
    }
    body << "        return failures;\n";
    body << "    }\n";
    body << "#if !SKYWIRE_NO_MAIN\n\n";
    body << "    public static int Main()\n";
    body << "    {\n";
    body << "        return Run() == 0 ? 0 : 1;\n";
    body << "    }\n";
    body << "#endif\n";
    body << "}\n";

    ss_ << "// Code generated by wiregen. DO NOT EDIT.\n\n";
    ss_ << "using System;\n";
    if (have_string || wants_json() || wants_xml() || wants_bson())
    {
        ss_ << "using System.Text;\n";
    }
    ss_ << "\n";

    if (false == ns.empty() && ns != "@NOTFOUND")
    {
        ss_ << "namespace " << ns << "\n{\n";
        std::string line;
        while (std::getline(body, line))
        {
            ss_ << (line.empty() || line[0] == '#' ? "" : "    ") << line << "\n";
        }
        ss_ << "}\n";
    }
    else
    {
        ss_ << body.str();
    }

    test_string.append(ss_.str());

    return true;
}

namespace
{

// bytes as a C# string literal, one char per byte: printable ASCII as is,
// every other byte as a \u00xx escape. Latin1() in the test turns it back
// into exactly these bytes.
std::string cs_str(const std::string& bytes)
{
    std::string out = "\"";
    for (unsigned char c : bytes)
    {
        if (c == '\\' || c == '"')
        {
            out += '\\';
            out += static_cast<char>(c);
        }
        else if (c < 0x20 || c > 0x7E)
        {
            char hex[8];
            std::snprintf(hex, sizeof(hex), "\\u%04x", c);
            out += hex;
        }
        else
        {
            out += static_cast<char>(c);
        }
    }
    return out + "\"";
}

std::string hex(const std::vector<uint8_t>& bytes)
{
    std::string out;
    for (uint8_t b : bytes)
    {
        char h[4];
        std::snprintf(h, sizeof(h), "%02x", b);
        out += h;
    }
    return out;
}

}

// The JSON, XML and BSON tests, and the names of their Test methods: the
// exact text or bytes (the same in every language, from test_values), a
// round trip, an empty object, element or document reading as zero, uint64_t
// values above 2^63-1 as Decimal128 in BSON, and malformed or out-of-range
// input failing without changing the message.
std::string CSharpTestGenerator::text_tests_code(const std::string& classname, std::vector<std::string>& tests)
{
    std::string out;
    if (false == wants_json() && false == wants_xml() && false == wants_bson())
    {
        return out;
    }

    out += "\n";
    out += "    private static byte[] Latin1(string text)\n";
    out += "    {\n";
    out += "        byte[] bytes = new byte[text.Length];\n";
    out += "        for (int i = 0; i < text.Length; i++)\n";
    out += "        {\n";
    out += "            bytes[i] = (byte)text[i];\n";
    out += "        }\n";
    out += "        return bytes;\n";
    out += "    }\n\n";
    out += "    private static string Hex(byte[] bytes)\n";
    out += "    {\n";
    out += "        return BitConverter.ToString(bytes).Replace(\"-\", \"\").ToLowerInvariant();\n";
    out += "    }\n\n";
    out += "    private static byte[] FromHex(string hex)\n";
    out += "    {\n";
    out += "        byte[] bytes = new byte[hex.Length / 2];\n";
    out += "        for (int i = 0; i < bytes.Length; i++)\n";
    out += "        {\n";
    out += "            bytes[i] = Convert.ToByte(hex.Substring(2 * i, 2), 16);\n";
    out += "        }\n";
    out += "        return bytes;\n";
    out += "    }\n";

    for (const std::string format : { "Json", "Xml" })
    {
        const bool json = format == "Json";
        if ((json && false == wants_json()) || (false == json && false == wants_xml()))
        {
            continue;
        }
        std::string expected;
        const bool have_expected = json ? expected_json(expected) : expected_xml(expected);

        tests.push_back(format);
        out += "\n    private static void Test" + format + "()\n";
        out += "    {\n";
        out += "        " + classname + " msg = MakeMessage();\n";
        out += "        string text = msg.To" + format + "();\n";
        if (have_expected)
        {
            out += "        string want = " + cs_str(expected) + ";\n";
            out += "        Check(text == want, \"To" + format + "\\n got \" + text + \"\\nwant \" + want);\n";
        }
        out += "        " + classname + " output = new " + classname + "();\n";
        out += "        Check(output.From" + format + "(text), \"From" + format + " refused \" + text);\n";
        out += "        Check(msg.Equals(output) && output.To" + format + "() == text, \"round trip\\n in \" + msg + \"\\nout \" + output);\n";
        out += "        output = new " + classname + "();\n";
        out += "        Check(output.From" + format + "(Encoding.UTF8.GetBytes(text)) && msg.Equals(output), \"From" + format + "(byte[])\\n in \" + msg + \"\\nout \" + output);\n";
        out += "    }\n";

        tests.push_back(format + "EmptyIsDefault");
        out += "\n    private static void Test" + format + "EmptyIsDefault()\n";
        out += "    {\n";
        out += "        " + classname + " output = MakeMessage();\n";
        out += "        Check(output.From" + format + "(" + cs_str(json ? "{}" : "<" + classname + "/>") + "), \"empty refused\");\n";
        out += "        Check(output.Equals(new " + classname + "()), \"got \" + output);\n";
        out += "    }\n";

        tests.push_back(format + "Rejects");
        out += "\n    private static void Test" + format + "Rejects()\n";
        out += "    {\n";
        out += "        " + classname + " output = MakeMessage();\n";
        out += "        foreach (string text in new string[]\n";
        out += "        {\n";
        for (const auto& text : json ? rejected_json() : rejected_xml())
        {
            out += "            " + cs_str(text) + ",\n";
        }
        out += "        })\n";
        out += "        {\n";
        out += "            Check(!output.From" + format + "(Latin1(text)), \"accepted \" + text);\n";
        out += "        }\n";
        out += "        Check(output.Equals(MakeMessage()), \"changed to \" + output);\n";
        out += "    }\n";
    }

    if (false == wants_bson())
    {
        return out;
    }

    std::vector<uint8_t> expected;
    const bool have_expected = expected_bson(expected);

    tests.push_back("Bson");
    out += "\n    private static void TestBson()\n";
    out += "    {\n";
    out += "        " + classname + " msg = MakeMessage();\n";
    out += "        byte[] data = msg.ToBson();\n";
    if (have_expected)
    {
        out += "        string want = \"" + hex(expected) + "\";\n";
        out += "        Check(Hex(data) == want, \"ToBson\\n got \" + Hex(data) + \"\\nwant \" + want);\n";
    }
    out += "        " + classname + " output = new " + classname + "();\n";
    out += "        Check(output.FromBson(data), \"FromBson refused \" + Hex(data));\n";
    out += "        Check(msg.Equals(output) && Hex(output.ToBson()) == Hex(data), \"round trip\\n in \" + msg + \"\\nout \" + output);\n";
    out += "    }\n";

    tests.push_back("BsonEmptyIsDefault");
    out += "\n    private static void TestBsonEmptyIsDefault()\n";
    out += "    {\n";
    out += "        " + classname + " output = MakeMessage();\n";
    out += "        Check(output.FromBson(new byte[] { 5, 0, 0, 0, 0 }), \"empty refused\");\n";
    out += "        Check(output.Equals(new " + classname + "()), \"got \" + output);\n";
    out += "    }\n";

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
        const std::string property = "msg." + CSharpGenerator::identifier(name);
        tests.push_back("BsonLargeUint64_" + name);
        out += "\n    private static void TestBsonLargeUint64_" + name + "()\n";
        out += "    {\n";
        out += "        ulong[] values = { ulong.MaxValue, long.MaxValue };\n";
        out += "        string[] elements = { \"" + hex(big) + "\", \"" + hex(edge) + "\" };\n";
        out += "        for (int i = 0; i < values.Length; i++)\n";
        out += "        {\n";
        out += "            " + classname + " msg = new " + classname + "();\n";
        out += "            " + property + " = values[i];\n";
        out += "            byte[] data = msg.ToBson();\n";
        out += "            Check(Hex(data).Contains(elements[i]), Hex(data) + \" does not hold \" + elements[i]);\n";
        out += "            " + classname + " output = new " + classname + "();\n";
        out += "            Check(output.FromBson(data) && msg.Equals(output), \"round trip of \" + values[i]);\n";
        out += "        }\n";
        out += "    }\n";
    }

    tests.push_back("BsonRejects");
    out += "\n    private static void TestBsonRejects()\n";
    out += "    {\n";
    out += "        byte[] good = new " + classname + "().ToBson();\n";
    out += "        byte[] shorter = new byte[good.Length - 1];\n";
    out += "        Array.Copy(good, shorter, shorter.Length);\n";
    out += "        byte[] longer = new byte[good.Length + 1];\n";
    out += "        Array.Copy(good, longer, good.Length);\n";
    out += "        byte[] miscounted = (byte[])good.Clone();\n";
    out += "        miscounted[0]++;\n";
    out += "        " + classname + " output = MakeMessage();\n";
    out += "        foreach (byte[] doc in new byte[][] { shorter, longer, miscounted })\n";
    out += "        {\n";
    out += "            Check(!output.FromBson(doc), \"accepted \" + Hex(doc));\n";
    out += "        }\n";
    out += "        foreach (string doc in new string[]\n";
    out += "        {\n";
    for (const auto& doc : rejected_bson())
    {
        out += "            \"" + hex(doc) + "\",\n";
    }
    out += "        })\n";
    out += "        {\n";
    out += "            Check(!output.FromBson(FromHex(doc)), \"accepted \" + doc);\n";
    out += "        }\n";
    out += "        Check(output.Equals(MakeMessage()), \"changed to \" + output);\n";
    out += "    }\n";

    return out;
}

std::string CSharpTestGenerator::csharp_literal(const std::string& type, const std::string& literal)
{
    const std::string cstype = lang_spec_.get_value_string(type);

    // A bit pattern, which can be out of range for a signed type.
    if (literal.compare(0, 2, "0x") == 0)
    {
        return "unchecked((" + cstype + ")" + literal + "UL)";
    }
    if (cstype == "float")
        return literal + "f";
    if (cstype == "uint")
        return literal + "U";
    if (cstype == "ulong")
        return literal + "UL";
    if (cstype == "long")
        return literal + "L";
    // In range constants convert to byte, sbyte, short and ushort.
    return literal;
}

// The statement that sets one member of target to its shared test value, or
// nothing for a member with no test value (a nested message).
std::string CSharpTestGenerator::set_line(const std::string& target, const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const std::string& name = std::get<member_name>(member);

    if (is_string_type(type))
    {
        return "        " + target + ".Set_" + name + "(Encoding.ASCII.GetBytes(\"" + test_values::string_value() + "\"));\n";
    }
    if (test_values::literal(type))
    {
        return "        " + target + "." + CSharpGenerator::identifier(name) + " = " + csharp_literal(type, test_literal(member)) + ";\n";
    }
    return "";
}

// The expected bytes come from the generator, not from the generated C#, so
// this catches wire-format bugs (byte order, padding, width) that a self
// round-trip cannot.
std::string CSharpTestGenerator::expected_wire_test(const std::vector<uint8_t>& wire)
{
    std::string out;
    out += "    private static void TestWire()\n";
    out += "    {\n";
    out += "        string want = \"\"";
    for (size_t i = 0; i < wire.size(); i += 8)
    {
        out += "\n            + \"";
        for (size_t j = i; j < std::min(i + 8, wire.size()); j++)
        {
            char hex[4];
            std::snprintf(hex, sizeof(hex), "%02x", wire[j]);
            out += (j > i ? " " : "") + std::string(hex);
        }
        out += std::string(i + 8 < wire.size() ? " " : "") + "\"";
    }
    out += ";\n";
    out += "        string got = BitConverter.ToString(MakeMessage().Serialize()).Replace('-', ' ').ToLowerInvariant();\n";
    out += "        Check(got == want, \"wire mismatch\\n got \" + got + \"\\nwant \" + want);\n";
    out += "    }\n";

    return out;
}

}
