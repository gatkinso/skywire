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

// The C++ runtime for the BSON form, lang/cpp/bson.h.

#include <gtest/gtest.h>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include <vector>
#include "bson.h"
#include "skywire/test_values.h"

using namespace skyloupe::bson;
namespace tv = skyloupe::test_values;

namespace
{

std::string str(const std::vector<uint8_t>& bytes)
{
    return std::string(bytes.begin(), bytes.end());
}

// A one-element document, as test_values builds it.
template <typename F>
std::string doc_of(F element)
{
    std::vector<uint8_t> e;
    element(e);
    return str(tv::bson_document(e));
}

// The document the Writer makes holding one value named "v".
template <typename T>
std::string written(T v)
{
    Writer w;
    w.begin_document();
    w.value("v", v);
    w.end_document();
    return w.take();
}

std::string written_bytes(const std::string& s)
{
    Writer w;
    w.begin_document();
    w.bytes("v", s.data(), s.size());
    w.end_document();
    return w.take();
}

// Reads the element named "v" of bytes into out.
template <typename T>
bool reads(const std::string& bytes, T& out, unsigned bits = 0)
{
    Document doc;
    if (false == parse(bytes, doc) || doc.elements.size() != 1)
        return false;
    return read(doc.elements[0], out, bits);
}

bool parses(const std::string& bytes)
{
    Document doc;
    return parse(bytes, doc);
}

std::string decimal(uint64_t high, uint64_t low)
{
    // A Decimal128 element "v" with these exact words.
    std::string out("\x13v\0", 3);
    for (int i = 0; i < 8; i++)
        out += static_cast<char>(low >> (8 * i));
    for (int i = 0; i < 8; i++)
        out += static_cast<char>(high >> (8 * i));
    const uint32_t length = static_cast<uint32_t>(out.size() + 5);
    std::string doc;
    for (int i = 0; i < 4; i++)
        doc += static_cast<char>(length >> (8 * i));
    return doc + out + '\0';
}

// A Decimal128 with coefficient (at most 64 bits) and exponent.
std::string decimal(bool negative, uint64_t coefficient, int exponent)
{
    const uint64_t high = (negative ? 0x8000000000000000ull : 0) | (static_cast<uint64_t>(exponent + 6176) << 49);
    return decimal(high, coefficient);
}

}

TEST(BsonUnitTest, IntegerTypes)
{
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", -50, e); }), written(int8_t(-50)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", 200, e); }), written(uint8_t(200)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", 200, e); }), written(static_cast<unsigned char>(200)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", 60000, e); }), written(uint16_t(60000)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", -12345, e); }), written(int16_t(-12345)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", -2000000000, e); }), written(int32_t(-2000000000)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int64("v", 3000000000, e); }), written(uint32_t(3000000000u)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int64("v", 0, e); }), written(uint32_t(0)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int64("v", std::numeric_limits<int64_t>::min(), e); }),
              written(std::numeric_limits<int64_t>::min()));
}

TEST(BsonUnitTest, CharIsSigned)
{
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", -56, e); }), written(static_cast<char>(200)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_int32("v", 65, e); }), written('A'));

    char c = 0;
    EXPECT_TRUE(reads(written(static_cast<char>(-128)), c));
    EXPECT_EQ(static_cast<char>(-128), c);
    EXPECT_TRUE(reads(doc_of([](auto& e) { tv::bson_int32("v", 255, e); }), c));
    EXPECT_EQ(static_cast<char>(255), c);
    EXPECT_FALSE(reads(doc_of([](auto& e) { tv::bson_int32("v", 256, e); }), c));
    EXPECT_FALSE(reads(doc_of([](auto& e) { tv::bson_int32("v", -129, e); }), c));
}

TEST(BsonUnitTest, Uint64AboveInt64IsDecimal128)
{
    const uint64_t edge = std::numeric_limits<int64_t>::max();
    EXPECT_EQ(doc_of([&](auto& e) { tv::bson_int64("v", static_cast<int64_t>(edge), e); }), written(edge));
    EXPECT_EQ(doc_of([&](auto& e) { tv::bson_decimal128("v", 0, edge + 1, false, e); }), written(edge + 1));
    EXPECT_EQ(decimal(0x3040000000000000ull, ~uint64_t(0)), written(~uint64_t(0)));

    for (uint64_t v : { uint64_t(0), edge, edge + 1, ~uint64_t(0) })
    {
        uint64_t out = 1;
        EXPECT_TRUE(reads(written(v), out)) << v;
        EXPECT_EQ(v, out);
    }

    // An int64 is never read as its bits: negative fails.
    uint64_t out = 7;
    EXPECT_FALSE(reads(doc_of([](auto& e) { tv::bson_int64("v", -1, e); }), out));
    EXPECT_FALSE(reads(decimal(true, 1, 0), out));
    EXPECT_EQ(7u, out);
}

TEST(BsonUnitTest, Decimal128Integers)
{
    int64_t i = 0;
    uint64_t u = 0;

    EXPECT_TRUE(reads(decimal(false, 65, 3), i));
    EXPECT_EQ(65000, i);
    EXPECT_TRUE(reads(decimal(true, 9000000000ull * 10, -1), i));
    EXPECT_EQ(-9000000000, i);
    EXPECT_TRUE(reads(decimal(true, 0, 5), i));
    EXPECT_EQ(0, i);
    EXPECT_TRUE(reads(decimal(true, 9223372036854775808ull, 0), i));
    EXPECT_EQ(std::numeric_limits<int64_t>::min(), i);
    EXPECT_TRUE(reads(decimal(false, 1844674407370955161ull, 1), u));
    EXPECT_EQ(18446744073709551610ull, u);
    // A coefficient wider than 64 bits that divides down: 12000000000 x 10^15 x 10^-15.
    EXPECT_TRUE(reads(decimal(0x3040000000000000ull - (uint64_t(15) << 49) + 0x9ED19ull, 0x4DB19B238C000000ull), u));
    EXPECT_EQ(12000000000ull, u);
    // (10^34 - 1) x 10^-15 is not whole.
    EXPECT_FALSE(reads(decimal(0x3040000000000000ull - (uint64_t(15) << 49) + 0x1ED09BEAD87C0ull, 0x378D8E63FFFFFFFFull), u));

    EXPECT_FALSE(reads(decimal(false, 15, -1), i));                    // 1.5
    EXPECT_FALSE(reads(decimal(false, 9223372036854775808ull, 0), i)); // 2^63
    EXPECT_FALSE(reads(decimal(false, 1844674407370955162ull, 1), u)); // 2^64 + 4
    EXPECT_FALSE(reads(decimal(0x3040000000000001ull, 0), u));         // 2^64
    EXPECT_FALSE(reads(decimal(0x7C00000000000000ull, 0), i));         // NaN
    EXPECT_FALSE(reads(decimal(0x7800000000000000ull, 0), i));         // Infinity
    EXPECT_FALSE(reads(decimal(0x6000000000000000ull, 1), i));         // the large-coefficient form
    // A coefficient of 10^34 is not canonical.
    EXPECT_FALSE(reads(decimal(0x3040000000000000ull | 0x1ED09BEAD87C0ull, 0x378D8E6400000000ull), u));
}

TEST(BsonUnitTest, Floats)
{
    // A float is widened exactly.
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_double("v", static_cast<double>(0.1f), e); }), written(0.1f));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_double("v", -0.0, e); }), written(-0.0));

    // Every NaN is written as the one quiet NaN.
    const std::string nan = doc_of([](auto& e) { tv::bson_double("v", std::nan(""), e); });
    EXPECT_NE(std::string::npos, nan.find(std::string("\0\0\0\0\0\0\xf8\x7f", 8)));
    EXPECT_EQ(nan, written(std::numeric_limits<double>::quiet_NaN()));
    EXPECT_EQ(nan, written(-std::numeric_limits<double>::quiet_NaN()));
    EXPECT_EQ(nan, written(std::numeric_limits<double>::signaling_NaN()));
    EXPECT_EQ(nan, written(std::numeric_limits<float>::quiet_NaN()));

    double d = 0;
    EXPECT_TRUE(reads(written(-0.0), d));
    EXPECT_TRUE(std::signbit(d));
    EXPECT_TRUE(reads(written(-std::numeric_limits<double>::infinity()), d));
    EXPECT_EQ(-std::numeric_limits<double>::infinity(), d);
    EXPECT_TRUE(reads(doc_of([](auto& e) { tv::bson_int64("v", -9000000000, e); }), d));
    EXPECT_EQ(-9000000000.0, d);

    float f = 0;
    EXPECT_TRUE(reads(written(std::numeric_limits<float>::max()), f));
    EXPECT_EQ(std::numeric_limits<float>::max(), f);
    // Just below halfway to 2^128 rounds to the largest float; halfway and
    // above would round to infinity, so they are too large.
    EXPECT_TRUE(reads(written(std::nextafter(0x1.ffffffp+127, 0.0)), f));
    EXPECT_EQ(std::numeric_limits<float>::max(), f);
    EXPECT_FALSE(reads(written(0x1.ffffffp+127), f));
    EXPECT_FALSE(reads(written(-1e39), f));
    EXPECT_TRUE(reads(written(std::numeric_limits<double>::infinity()), f));
    EXPECT_TRUE(std::isinf(f));
    EXPECT_TRUE(reads(written(std::nan("")), f));
    EXPECT_TRUE(std::isnan(f));
    EXPECT_TRUE(reads(written(0.1), f));
    EXPECT_EQ(0.1f, f);

    EXPECT_FALSE(reads(decimal(false, 1, 0), d)); // Decimal128 is for integers
}

TEST(BsonUnitTest, IntegersFromDoubles)
{
    int16_t i = 0;
    EXPECT_TRUE(reads(written(-5.0), i));
    EXPECT_EQ(-5, i);
    EXPECT_TRUE(reads(written(-0.0), i));
    EXPECT_EQ(0, i);
    EXPECT_FALSE(reads(written(1.5), i));
    EXPECT_FALSE(reads(written(32768.0), i));
    EXPECT_FALSE(reads(written(std::nan("")), i));
    EXPECT_FALSE(reads(written(std::numeric_limits<double>::infinity()), i));

    uint64_t u = 0;
    EXPECT_TRUE(reads(written(18446744073709549568.0), u)); // the largest double below 2^64
    EXPECT_EQ(18446744073709549568ull, u);
    EXPECT_FALSE(reads(written(18446744073709551616.0), u));
    int64_t s = 0;
    EXPECT_TRUE(reads(written(-9223372036854775808.0), s));
    EXPECT_EQ(std::numeric_limits<int64_t>::min(), s);
    EXPECT_FALSE(reads(written(9223372036854775808.0), s));
}

TEST(BsonUnitTest, RangesAndBitfields)
{
    uint8_t u8 = 0;
    EXPECT_TRUE(reads(written(int32_t(255)), u8));
    EXPECT_FALSE(reads(written(int32_t(256)), u8));
    EXPECT_FALSE(reads(written(int32_t(-1)), u8));

    uint32_t u32 = 0;
    EXPECT_TRUE(reads(written(int32_t(5)), u32)); // any integer type will do
    EXPECT_FALSE(reads(written(int64_t(4294967296)), u32));

    uint8_t field = 0;
    EXPECT_TRUE(reads(written(int32_t(7)), field, 3));
    EXPECT_FALSE(reads(written(int32_t(8)), field, 3));
    int8_t sfield = 0;
    EXPECT_TRUE(reads(written(int32_t(-4)), sfield, 3));
    EXPECT_FALSE(reads(written(int32_t(-5)), sfield, 3));
    EXPECT_FALSE(reads(written(int32_t(4)), sfield, 3));

    uint32_t wrong = 9;
    EXPECT_FALSE(reads(doc_of([](auto& e) { tv::bson_bool("v", true, e); }), wrong));
    EXPECT_FALSE(reads(doc_of([](auto& e) { tv::bson_string("v", "1", e); }), wrong));
    EXPECT_EQ(9u, wrong);
}

TEST(BsonUnitTest, StringOrBinary)
{
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_string("v", "skywire", e); }), written_bytes(std::string("skywire\0\0\0", 10)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_string("v", "", e); }), written_bytes(std::string("\0\0", 2)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_string("v", "\xc3\xa9", e); }), written_bytes("\xc3\xa9"));
    // Not UTF-8, a NUL inside, a surrogate, an overlong form: binary.
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_binary("v", "\xff", e); }), written_bytes("\xff"));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_binary("v", std::string("a\0b", 3), e); }),
              written_bytes(std::string("a\0b\0", 4)));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_binary("v", "\xed\xa0\x80", e); }), written_bytes("\xed\xa0\x80"));
    EXPECT_EQ(doc_of([](auto& e) { tv::bson_binary("v", "\xc0\x80", e); }), written_bytes("\xc0\x80"));

    Document doc;
    char out[4];
    ASSERT_TRUE(parse(doc_of([](auto& e) { tv::bson_binary("v", std::string("\0\xff", 2), e); }), doc));
    ASSERT_TRUE(read_bytes(doc.elements[0], out, sizeof(out)));
    EXPECT_EQ(0, std::memcmp(out, "\0\xff\0\0", 4));

    doc = Document();
    ASSERT_TRUE(parse(doc_of([](auto& e) { tv::bson_string("v", "abcd", e); }), doc));
    EXPECT_TRUE(read_bytes(doc.elements[0], out, 4));
    EXPECT_FALSE(read_bytes(doc.elements[0], out, 3));

    // Binary of another subtype (here a UUID) is not a string.
    doc = Document();
    std::string uuid = doc_of([](auto& e) { tv::bson_binary("v", "0123456789abcdef", e); });
    uuid[4 + 3 + 4] = 4;
    ASSERT_TRUE(parse(uuid, doc));
    EXPECT_FALSE(read_bytes(doc.elements[0], out, 4));
}

TEST(BsonUnitTest, StructureIsChecked)
{
    EXPECT_TRUE(parses(std::string("\x05\0\0\0\0", 5)));
    EXPECT_FALSE(parses(""));
    EXPECT_FALSE(parses(std::string("\x05\0\0\0", 4)));
    EXPECT_FALSE(parses(std::string("\x04\0\0\0\0", 5)));             // shorter than possible
    EXPECT_FALSE(parses(std::string("\x06\0\0\0\0", 5)));             // longer than the bytes
    EXPECT_FALSE(parses(std::string("\x05\0\0\0\0\0", 6)));           // bytes after it
    EXPECT_FALSE(parses(std::string("\x05\0\0\0\x01", 5)));           // no terminating NUL
    EXPECT_FALSE(parses(std::string("\x05\0\0\x80\0", 5)));           // negative length
    EXPECT_FALSE(parses(std::string("\x08\0\0\0\x10v\0\0", 8)));      // int32 cut short
    EXPECT_FALSE(parses(std::string("\x07\0\0\0\x10vv", 7)));         // name not terminated
    EXPECT_FALSE(parses(std::string("\x08\0\0\0\x14v\0\0", 8)));      // unknown type
    EXPECT_FALSE(parses(std::string("\x09\0\0\0\x08v\0\x02\0", 9)));  // bool other than 0 or 1
    EXPECT_TRUE(parses(std::string("\x09\0\0\0\x08v\0\x01\0", 9)));
    EXPECT_FALSE(parses(std::string("\x08\0\0\0\x10\xff\0\0", 8)));   // name not UTF-8

    // Strings: length counts the NUL, which must be there; UTF-8.
    EXPECT_TRUE(parses(std::string("\x0e\0\0\0\x02v\0\x02\0\0\0a\0\0", 14)));
    EXPECT_FALSE(parses(std::string("\x0e\0\0\0\x02v\0\x02\0\0\0ab\0", 14)));
    EXPECT_FALSE(parses(std::string("\x0d\0\0\0\x02v\0\0\0\0\0\0", 12)));
    EXPECT_FALSE(parses(std::string("\x0e\0\0\0\x02v\0\x03\0\0\0a\0\0", 14)));
    EXPECT_FALSE(parses(std::string("\x0e\0\0\0\x02v\0\x02\0\0\0\xff\0\0", 14)));

    // Binary: a negative length or one beyond the document.
    EXPECT_FALSE(parses(std::string("\x0d\0\0\0\x05v\0\xff\xff\xff\xff\0\0", 13)));
    EXPECT_FALSE(parses(std::string("\x0d\0\0\0\x05v\0\x01\0\0\0\0\0", 13)));

    // An embedded document cannot run past its parent's end.
    EXPECT_FALSE(parses(std::string("\x0c\0\0\0\x03v\0\x06\0\0\0\0", 12)));
    EXPECT_TRUE(parses(std::string("\x0d\0\0\0\x03v\0\x05\0\0\0\0\0", 13)));
}

TEST(BsonUnitTest, NestingIsLimited)
{
    auto nested = [](unsigned depth) {
        std::vector<uint8_t> doc = tv::bson_document({});
        for (unsigned i = 1; i < depth; i++)
        {
            std::vector<uint8_t> e = { 0x03, 'x', 0x00 };
            e.insert(e.end(), doc.begin(), doc.end());
            doc = tv::bson_document(e);
        }
        return str(doc);
    };
    EXPECT_TRUE(parses(nested(max_depth)));
    EXPECT_FALSE(parses(nested(max_depth + 1)));
}

TEST(BsonUnitTest, EveryKnownTypeIsSkipped)
{
    std::vector<uint8_t> e;
    tv::bson_double("double", 1.5, e);
    tv::bson_string("string", "s", e);
    const std::vector<uint8_t> inner = tv::bson_document({});
    for (uint8_t type : { 0x03, 0x04 })
    {
        e.push_back(type);
        e.insert(e.end(), { 'd', 0 });
        e.insert(e.end(), inner.begin(), inner.end());
    }
    tv::bson_binary("binary", "b", e);
    e.insert(e.end(), { 0x06, 'u', 0 });
    e.insert(e.end(), { 0x07, 'o', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 });
    tv::bson_bool("bool", false, e);
    e.insert(e.end(), { 0x09, 't', 0, 1, 2, 3, 4, 5, 6, 7, 8 });
    e.insert(e.end(), { 0x0A, 'n', 0 });
    e.insert(e.end(), { 0x0B, 'r', 0, 'a', 0, 'i', 0 });
    e.insert(e.end(), { 0x0C, 'p', 0, 2, 0, 0, 0, 'c', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 });
    e.insert(e.end(), { 0x0D, 'j', 0, 2, 0, 0, 0, 'f', 0 });
    e.insert(e.end(), { 0x0E, 's', 0, 2, 0, 0, 0, 'f', 0 });
    // Code with scope: total length 4 + 6 + 5.
    e.insert(e.end(), { 0x0F, 'w', 0, 15, 0, 0, 0, 2, 0, 0, 0, 'f', 0, 5, 0, 0, 0, 0 });
    tv::bson_int32("int32", 1, e);
    e.insert(e.end(), { 0x11, 't', 0, 1, 2, 3, 4, 5, 6, 7, 8 });
    tv::bson_int64("int64", 1, e);
    tv::bson_decimal128("dec", 0, 1, false, e);
    e.insert(e.end(), { 0xFF, 'm', 0 });
    e.insert(e.end(), { 0x7F, 'M', 0 });

    Document doc;
    ASSERT_TRUE(parse(str(tv::bson_document(e)), doc));
    EXPECT_EQ(21u, doc.elements.size()); // every type in bsonspec.org 1.1

    // Code with scope whose total length is wrong.
    std::vector<uint8_t> bad = { 0x0F, 'w', 0, 16, 0, 0, 0, 2, 0, 0, 0, 'f', 0, 5, 0, 0, 0, 0, 0 };
    EXPECT_FALSE(parses(str(tv::bson_document(bad))));
}

TEST(BsonUnitTest, NestedDocumentsAreWritten)
{
    Writer w;
    w.begin_document();
    w.value("a", int32_t(1));
    w.document("b");
    w.begin_document();
    w.value("c", int32_t(2));
    w.end_document();
    w.end_document();

    std::vector<uint8_t> c;
    tv::bson_int32("c", 2, c);
    const std::vector<uint8_t> inner = tv::bson_document(c);
    std::vector<uint8_t> outer;
    tv::bson_int32("a", 1, outer);
    outer.insert(outer.end(), { 0x03, 'b', 0 });
    outer.insert(outer.end(), inner.begin(), inner.end());
    EXPECT_EQ(str(tv::bson_document(outer)), w.str());

    Document doc;
    ASSERT_TRUE(parse(w.str(), doc));
    ASSERT_EQ(2u, doc.elements.size());
    EXPECT_EQ(type_document, doc.elements[1].type);
    ASSERT_EQ(1u, doc.elements[1].document.elements.size());
    EXPECT_EQ("c", doc.elements[1].document.elements[0].name);
}
