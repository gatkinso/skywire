//
//   Copyright (C) 2017 Geoffrey Atkinson
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

// The C++ runtime for the JSON and XML text forms, lang/cpp/text.h.

#include <gtest/gtest.h>
#include <clocale>
#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include "text.h"

using namespace skyloupe::text;

namespace
{

JsonValue json(const std::string& text)
{
    JsonValue v;
    EXPECT_TRUE(parse_json(text, v)) << text;
    return v;
}

template <typename T>
bool json_reads(const std::string& text, T& out, unsigned bits = 0)
{
    JsonValue v;
    return parse_json(text, v) && read(v, out, bits);
}

}

TEST(TextUnitTest, FloatTextIsShortestInJavaScriptLayout)
{
    EXPECT_EQ("3.14159", number_text(3.14159));
    EXPECT_EQ("0.1", number_text(0.1));
    EXPECT_EQ("0.1", number_text(0.1f));
    EXPECT_EQ("100", number_text(100.0));
    EXPECT_EQ("123456789012345680000", number_text(123456789012345680000.0));
    EXPECT_EQ("1e+21", number_text(1e21));
    EXPECT_EQ("0.0000015", number_text(1.5e-6));
    EXPECT_EQ("1e-7", number_text(1e-7));
    EXPECT_EQ("5e-324", number_text(5e-324));
    EXPECT_EQ("1.7976931348623157e+308", number_text(std::numeric_limits<double>::max()));
    EXPECT_EQ("3.4028235e+38", number_text(std::numeric_limits<float>::max()));
    EXPECT_EQ("1e-45", number_text(std::numeric_limits<float>::denorm_min()));
    EXPECT_EQ("16777216", number_text(16777216.0f));
    EXPECT_EQ("0", number_text(0.0));
    EXPECT_EQ("-0", number_text(-0.0));
    EXPECT_EQ("-0", number_text(-0.0f));
    EXPECT_EQ("NaN", number_text(std::nan("")));
    EXPECT_EQ("Infinity", number_text(std::numeric_limits<double>::infinity()));
    EXPECT_EQ("-Infinity", number_text(-std::numeric_limits<float>::infinity()));
}

// Of two equally near shortest forms, the one ending in an even digit.
TEST(TextUnitTest, FloatTiesGoToEven)
{
    EXPECT_EQ("0.00024414062", number_text(0.000244140625f));
    EXPECT_EQ("-1954271.2", number_text(-1954271.25f));
}

TEST(TextUnitTest, FloatTextReadsBackExactly)
{
    std::mt19937_64 rng(1);
    for (int i = 0; i < 20000; i++)
    {
        const uint64_t bits = rng();
        double d;
        std::memcpy(&d, &bits, sizeof(d));
        if (std::isfinite(d))
        {
            double back;
            ASSERT_TRUE(json_reads(number_text(d), back)) << number_text(d);
            ASSERT_EQ(0, std::memcmp(&d, &back, sizeof(d))) << number_text(d);
        }

        const uint32_t bits32 = static_cast<uint32_t>(bits >> 32);
        float f;
        std::memcpy(&f, &bits32, sizeof(f));
        if (std::isfinite(f))
        {
            float back;
            ASSERT_TRUE(json_reads(number_text(f), back)) << number_text(f);
            ASSERT_EQ(0, std::memcmp(&f, &back, sizeof(f))) << number_text(f);
        }
    }
}

TEST(TextUnitTest, NumbersIgnoreTheLocale)
{
    const char* old = std::setlocale(LC_NUMERIC, nullptr);
    const std::string saved = old ? old : "C";
    if (std::setlocale(LC_NUMERIC, "de_DE.UTF-8") == nullptr && std::setlocale(LC_NUMERIC, "de_DE") == nullptr)
    {
        GTEST_SKIP() << "no German locale";
    }
    EXPECT_EQ("1.25", number_text(1.25));
    double d = 0;
    EXPECT_TRUE(json_reads("1.25", d));
    EXPECT_EQ(1.25, d);
    std::setlocale(LC_NUMERIC, saved.c_str());
}

TEST(TextUnitTest, IntegerText)
{
    EXPECT_EQ("-56", number_text(static_cast<char>(-56)));
    EXPECT_EQ("200", number_text(static_cast<unsigned char>(200)));
    EXPECT_EQ("-9223372036854775808", number_text(std::numeric_limits<int64_t>::min()));
    EXPECT_EQ("18446744073709551615", number_text(std::numeric_limits<uint64_t>::max()));
}

TEST(TextUnitTest, IntegersMustBeWholeAndInRange)
{
    int32_t i32 = 0;
    EXPECT_TRUE(json_reads("100", i32));
    EXPECT_EQ(100, i32);
    EXPECT_TRUE(json_reads("1.0e2", i32));
    EXPECT_EQ(100, i32);
    EXPECT_TRUE(json_reads("12000e-2", i32));
    EXPECT_EQ(120, i32);
    EXPECT_TRUE(json_reads("\"-7\"", i32));
    EXPECT_EQ(-7, i32);
    EXPECT_TRUE(json_reads("-0", i32));
    EXPECT_EQ(0, i32);
    EXPECT_FALSE(json_reads("1.5", i32));
    EXPECT_FALSE(json_reads("1e-1", i32));
    EXPECT_FALSE(json_reads("2147483648", i32));
    EXPECT_FALSE(json_reads("1e400", i32));
    EXPECT_FALSE(json_reads("\" 7\"", i32));
    EXPECT_FALSE(json_reads("\"0x10\"", i32));
    EXPECT_FALSE(json_reads("true", i32));
    EXPECT_FALSE(json_reads("null", i32));

    int8_t i8 = 0;
    EXPECT_TRUE(json_reads("-128", i8));
    EXPECT_EQ(-128, i8);
    EXPECT_FALSE(json_reads("128", i8));

    uint64_t u64 = 0;
    EXPECT_TRUE(json_reads("\"18446744073709551615\"", u64));
    EXPECT_EQ(std::numeric_limits<uint64_t>::max(), u64);
    EXPECT_FALSE(json_reads("18446744073709551616", u64));
    EXPECT_FALSE(json_reads("-1", u64));

    int64_t i64 = 0;
    EXPECT_TRUE(json_reads("\"-9223372036854775808\"", i64));
    EXPECT_EQ(std::numeric_limits<int64_t>::min(), i64);
    EXPECT_FALSE(json_reads("-9223372036854775809", i64));

    // A char takes either signedness.
    char c = 0;
    EXPECT_TRUE(json_reads("-128", c));
    EXPECT_TRUE(json_reads("255", c));
    EXPECT_FALSE(json_reads("256", c));
}

TEST(TextUnitTest, BitfieldsMustFit)
{
    uint8_t u = 0;
    EXPECT_TRUE(json_reads("7", u, 3));
    EXPECT_FALSE(json_reads("8", u, 3));
    int16_t s = 0;
    EXPECT_TRUE(json_reads("-4", s, 3));
    EXPECT_EQ(-4, s);
    EXPECT_TRUE(json_reads("3", s, 3));
    EXPECT_FALSE(json_reads("-5", s, 3));
    EXPECT_FALSE(json_reads("4", s, 3));
    int64_t one = 0;
    EXPECT_TRUE(json_reads("-1", one, 1));
    EXPECT_FALSE(json_reads("1", one, 1));
}

TEST(TextUnitTest, Floats)
{
    float f = 0;
    EXPECT_TRUE(json_reads("3.5", f));
    EXPECT_EQ(3.5f, f);
    EXPECT_TRUE(json_reads("\"NaN\"", f));
    EXPECT_TRUE(std::isnan(f));
    EXPECT_TRUE(json_reads("\"-Infinity\"", f));
    EXPECT_EQ(-std::numeric_limits<float>::infinity(), f);
    EXPECT_FALSE(json_reads("1e39", f));
    EXPECT_FALSE(json_reads("\"nan\"", f));
    EXPECT_TRUE(json_reads("1e-50", f));
    EXPECT_EQ(0.0f, f);

    double d = 0;
    EXPECT_TRUE(json_reads("1e39", d));
    EXPECT_FALSE(json_reads("1e309", d));
    EXPECT_TRUE(json_reads("-0", d));
    EXPECT_TRUE(std::signbit(d));
}

TEST(TextUnitTest, JsonStringsAreBytes)
{
    char buf[4];
    EXPECT_TRUE(read_bytes(json("\"a\\u00e9\""), buf, sizeof(buf)));
    EXPECT_EQ(0, std::memcmp(buf, "a\xe9\0\0", 4));
    // A raw é (UTF-8) is the same character.
    EXPECT_TRUE(read_bytes(json("\"a\xc3\xa9\""), buf, sizeof(buf)));
    EXPECT_EQ(0, std::memcmp(buf, "a\xe9\0\0", 4));
    EXPECT_TRUE(read_bytes(json("\"\\u0000\\u0001\""), buf, sizeof(buf)));
    EXPECT_EQ(0, std::memcmp(buf, "\0\x01\0\0", 4));
    EXPECT_FALSE(read_bytes(json("\"\\u0100\""), buf, sizeof(buf)));
    EXPECT_FALSE(read_bytes(json("\"abcde\""), buf, sizeof(buf)));
    EXPECT_FALSE(read_bytes(json("1"), buf, sizeof(buf)));

    JsonWriter w;
    const char bytes[8] = { 'a', '"', '\\', '\x01', '\xff', 'z', 0, 0 };
    w.bytes(bytes, sizeof(bytes));
    EXPECT_EQ("\"a\\\"\\\\\\u0001\\u00ffz\"", w.str());
}

TEST(TextUnitTest, JsonWriter)
{
    JsonWriter w;
    w.begin_object();
    w.key("a");
    w.value(static_cast<int64_t>(-5));
    w.key("b");
    w.begin_object();
    w.key("c");
    w.value(1.5f);
    w.end_object();
    w.key("d");
    w.value(std::nan(""));
    w.key("e");
    w.value(static_cast<uint16_t>(7));
    w.end_object();
    EXPECT_EQ("{\"a\":\"-5\",\"b\":{\"c\":1.5},\"d\":\"NaN\",\"e\":7}", w.str());
}

TEST(TextUnitTest, JsonParser)
{
    JsonValue v = json(" {\"a\" : [1, -2.5e3, true, false, null, \"\\ud83d\\ude00\"], \"b\":{}} ");
    ASSERT_EQ(JsonValue::Object, v.kind);
    ASSERT_EQ(2u, v.members.size());
    ASSERT_EQ(6u, v.members[0].value.items.size());
    EXPECT_EQ("-2.5e3", v.members[0].value.items[1].text);
    EXPECT_EQ("\xf0\x9f\x98\x80", v.members[0].value.items[5].text);

    JsonValue bad;
    for (const char* text : { "", "{", "{\"a\":01}", "[1,]", "{\"a\" 1}", "\"\\ud800\"", "\"\\udc00\"", "\"\x01\"",
                              "\"\xc0\xaf\"", "\"\xff\"", "1 2", "+1", ".5", "1.", "tru", "\"\\x\"", "{a:1}" })
    {
        EXPECT_FALSE(parse_json(text, bad)) << text;
    }

    std::string deep(max_depth + 1, '[');
    deep += std::string(max_depth + 1, ']');
    EXPECT_FALSE(parse_json(deep, bad));
    std::string ok(max_depth, '[');
    ok += std::string(max_depth, ']');
    EXPECT_TRUE(parse_json(ok, bad));
}

TEST(TextUnitTest, XmlParser)
{
    XmlNode root;
    ASSERT_TRUE(parse_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!-- hi -->\n<M a='1' b=\"2\">\r\n"
                          "  <x> 5 </x><s>a&amp;&#x41;&#66;&lt;&gt;&quot;&apos;</s><?pi?><c><![CDATA[<z>&]]></c><e/>"
                          "</M>\n<!-- bye -->\n",
                          root));
    EXPECT_EQ("M", root.name);
    ASSERT_EQ(4u, root.children.size());
    EXPECT_TRUE(is_blank(root));
    ASSERT_NE(nullptr, root.attribute("b"));
    EXPECT_EQ("2", root.attribute("b")->value);
    EXPECT_EQ("a&AB<>\"'", root.children[1].text);
    EXPECT_EQ("<z>&", root.children[2].text);

    int x = 0;
    EXPECT_TRUE(read(root.children[0], x));
    EXPECT_EQ(5, x);
    EXPECT_FALSE(read(root, x));

    for (const char* text : { "", "<M>", "<M></N>", "<M/><N/>", "<!DOCTYPE M><M/>",
                              "<!DOCTYPE M [<!ENTITY e \"x\">]><M>&e;</M>", "<M>&unknown;</M>", "<M>&#0;</M>",
                              "<M>\x01</M>", "<M a=1/>", "<M a='1' a='2'/>", "<M a='<'/>", "<M>\xff</M>", "<1/>",
                              "<M a='1'b='2'/>", "<M><!-- x</M>", "<M><![CDATA[x</M>" })
    {
        EXPECT_FALSE(parse_xml(text, root)) << text;
    }

    std::string deep;
    for (unsigned i = 0; i <= max_depth; i++)
        deep += "<a>";
    for (unsigned i = 0; i <= max_depth; i++)
        deep += "</a>";
    EXPECT_FALSE(parse_xml(deep, root));
}

TEST(TextUnitTest, XmlStrings)
{
    XmlNode root;
    char buf[4];
    ASSERT_TRUE(parse_xml("<s>a\xc3\xa9</s>", root));
    EXPECT_TRUE(read_bytes(root, buf, sizeof(buf)));
    EXPECT_EQ(0, std::memcmp(buf, "a\xe9\0\0", 4));

    ASSERT_TRUE(parse_xml("<s encoding=\"hex\"> 00FF01 </s>", root));
    EXPECT_TRUE(read_bytes(root, buf, sizeof(buf)));
    EXPECT_EQ(0, std::memcmp(buf, "\0\xff\x01\0", 4));

    ASSERT_TRUE(parse_xml("<s encoding=\"hex\">0</s>", root));
    EXPECT_FALSE(read_bytes(root, buf, sizeof(buf)));
    ASSERT_TRUE(parse_xml("<s encoding=\"hex\">0102030405</s>", root));
    EXPECT_FALSE(read_bytes(root, buf, sizeof(buf)));
    ASSERT_TRUE(parse_xml("<s encoding=\"base64\">AA==</s>", root));
    EXPECT_FALSE(read_bytes(root, buf, sizeof(buf)));
    ASSERT_TRUE(parse_xml("<s>abcde</s>", root));
    EXPECT_FALSE(read_bytes(root, buf, sizeof(buf)));
    ASSERT_TRUE(parse_xml("<s>&#x100;</s>", root));
    EXPECT_FALSE(read_bytes(root, buf, sizeof(buf)));
}

TEST(TextUnitTest, XmlWriter)
{
    XmlWriter w;
    w.open("M");
    w.element("i", static_cast<int64_t>(-5));
    w.element("f", 0.1f);
    w.element("n", -std::numeric_limits<double>::infinity());
    w.bytes_element("s", "<a&'\">\0", 7);
    w.bytes_element("h", "a\x01\xff\0", 4);
    w.bytes_element("e", "\0\0", 2);
    w.close("M");
    EXPECT_EQ("<M><i>-5</i><f>0.1</f><n>-Infinity</n><s>&lt;a&amp;&apos;&quot;&gt;</s>"
              "<h encoding=\"hex\">6101ff</h><e></e></M>",
              w.str());
}
