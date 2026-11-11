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

// The C runtime for the JSON, XML and BSON forms, lang/c/ctext.h and
// lang/c/cbson.h, against the C++ runtime they must match byte for byte.
// The generated tests and fuzz_c_cpp_text check whole messages; these check
// the number text, which a fuzzer reaches only through the values it decodes.

#include <gtest/gtest.h>
#include <clocale>
#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include "cbson.h"
#include "ctext.h"
#include "text.h"

namespace
{

std::string c_real(double v, bool is_float)
{
    char out[48];
    const size_t len = sw_text_real(out, v, is_float ? 1 : 0);
    EXPECT_EQ(std::strlen(out), len);
    return std::string(out, len);
}

bool c_reads(const std::string& text, double& d)
{
    float unused;
    return sw_text_to_real(text.data(), text.size(), 0, &d, &unused) == 1;
}

bool c_integer(const std::string& text, int& negative, uint64_t& magnitude)
{
    return sw_text_integer(text.data(), text.size(), &negative, &magnitude) == 1;
}

}

// Every double and float, written the C way and the C++ way: the same text,
// which reads back as the same value.
TEST(CTextUnitTest, RealTextMatchesCpp)
{
    // A million agree too; this many keeps the suite quick.
    std::mt19937_64 rng(20261111);
    for (int i = 0; i < 50000; i++)
    {
        const uint64_t bits = rng();
        double d;
        std::memcpy(&d, &bits, sizeof(d));
        ASSERT_EQ(skyloupe::text::number_text(d), c_real(d, false)) << std::hex << bits;

        const uint32_t fbits = static_cast<uint32_t>(bits >> 17);
        float f;
        std::memcpy(&f, &fbits, sizeof(f));
        ASSERT_EQ(skyloupe::text::number_text(f), c_real(f, true)) << std::hex << fbits;
    }

    // Edges: powers of two, the smallest and largest, and the layout limits.
    const double edges[] = { 0.0, -0.0, 1.0, -1.0, 0.1, 1e21, 1e20, 123456789012345680000.0, 1e-6, 1e-7, 5e-324,
                             std::numeric_limits<double>::max(), std::numeric_limits<double>::min(), 9007199254740993.0,
                             std::ldexp(1.0, 1023), std::ldexp(1.0, -1022), 0.3, 2.5e-7 };
    for (double d : edges)
    {
        EXPECT_EQ(skyloupe::text::number_text(d), c_real(d, false)) << d;
        EXPECT_EQ(skyloupe::text::number_text(static_cast<float>(d)), c_real(static_cast<float>(d), true)) << d;
    }
    EXPECT_EQ("NaN", c_real(std::nan(""), false));
    EXPECT_EQ("-Infinity", c_real(-HUGE_VAL, true));
}

TEST(CTextUnitTest, NumbersIgnoreTheLocale)
{
    const char* old = std::setlocale(LC_NUMERIC, nullptr);
    const std::string saved = old ? old : "C";
    if (std::setlocale(LC_NUMERIC, "de_DE.UTF-8") == nullptr && std::setlocale(LC_NUMERIC, "de_DE") == nullptr)
    {
        GTEST_SKIP() << "no German locale";
    }
    EXPECT_EQ("1.25", c_real(1.25, false));
    EXPECT_EQ("0.1", c_real(0.1f, true));
    double d = 0;
    EXPECT_TRUE(c_reads("1.25", d));
    EXPECT_EQ(1.25, d);
    EXPECT_FALSE(c_reads("1,25", d));
    std::setlocale(LC_NUMERIC, saved.c_str());
}

TEST(CTextUnitTest, RealsReadAsCppReadsThem)
{
    const char* texts[] = { "0", "-0", "1e400", "-1e400", "1e-400", "0.1", "1.", ".5", "01", "1e", "NaN", "nan",
                            "Infinity", "-Infinity", "+1", "3.4028235e38", "3.4028236e38", "1E5", "123456789.123456789" };
    for (const char* text : texts)
    {
        double c = 0, cpp = 0;
        float cf = 0, cppf = 0, unused_f;
        double unused_d;
        const bool c_ok = sw_text_to_real(text, std::strlen(text), 0, &c, &unused_f) == 1;
        const bool cpp_ok = skyloupe::text::detail::to_float(text, cpp);
        ASSERT_EQ(cpp_ok, c_ok) << text;
        if (c_ok && false == std::isnan(c))
            EXPECT_EQ(std::signbit(cpp), std::signbit(c)) << text;
        if (c_ok && false == std::isnan(c))
            EXPECT_EQ(cpp, c) << text;

        const bool cf_ok = sw_text_to_real(text, std::strlen(text), 1, &unused_d, &cf) == 1;
        ASSERT_EQ(skyloupe::text::detail::to_float(text, cppf), cf_ok) << text;
        if (cf_ok && false == std::isnan(cf))
            EXPECT_EQ(cppf, cf) << text;
    }
}

TEST(CTextUnitTest, IntegersReadAsCppReadsThem)
{
    const char* texts[] = { "0", "-0", "12", "1.2e1", "120e-1", "1.5", "0.0", "10.0", "1e19", "1e20",
                            "18446744073709551615", "18446744073709551616", "-9223372036854775808",
                            "0.000000000000000000000000000001e30", "1000000000000000000000e-3", "1e-0", "1e+0",
                            "-", "1e", "01", "1.", "00", "184467440737095516150e-1", "184467440737095516151e-1" };
    for (const char* text : texts)
    {
        int c_negative = 0;
        uint64_t c_magnitude = 0;
        bool cpp_negative = false;
        uint64_t cpp_magnitude = 0;
        const bool c_ok = c_integer(text, c_negative, c_magnitude);
        ASSERT_EQ(skyloupe::text::detail::integer_value(text, cpp_negative, cpp_magnitude), c_ok) << text;
        if (c_ok)
        {
            EXPECT_EQ(cpp_negative, c_negative != 0) << text;
            EXPECT_EQ(cpp_magnitude, c_magnitude) << text;
        }
    }

    // Random digit strings with points and exponents.
    std::mt19937 rng(7);
    for (int i = 0; i < 200000; i++)
    {
        std::string text;
        if (rng() % 4 == 0)
            text += '-';
        const int digits = 1 + static_cast<int>(rng() % 25);
        for (int k = 0; k < digits; k++)
            text += static_cast<char>('0' + (rng() % 3 == 0 ? 0 : rng() % 10));
        if (rng() % 3 == 0)
        {
            text += '.';
            const int fraction = 1 + static_cast<int>(rng() % 8);
            for (int k = 0; k < fraction; k++)
                text += static_cast<char>('0' + (rng() % 2 == 0 ? 0 : rng() % 10));
        }
        if (rng() % 3 == 0)
            text += "e" + std::string(rng() % 2 ? "-" : "") + std::to_string(rng() % 30);

        int c_negative = 0;
        uint64_t c_magnitude = 0;
        bool cpp_negative = false;
        uint64_t cpp_magnitude = 0;
        const bool c_ok = c_integer(text, c_negative, c_magnitude);
        ASSERT_EQ(skyloupe::text::detail::integer_value(text, cpp_negative, cpp_magnitude), c_ok) << text;
        if (c_ok)
        {
            ASSERT_EQ(cpp_magnitude, c_magnitude) << text;
        }
    }
}

// The snprintf contract: the length is always the whole text's, and what
// fits is NUL-terminated.
TEST(CTextUnitTest, ShortBuffers)
{
    char buf[4];
    sw_out o;
    sw_out_init(&o, buf, sizeof(buf));
    sw_out_str(&o, "abcdef");
    EXPECT_EQ(6u, sw_out_text(&o));
    EXPECT_STREQ("abc", buf);

    sw_out_init(&o, nullptr, 0);
    sw_out_str(&o, "abcdef");
    EXPECT_EQ(6u, sw_out_text(&o));

    // A BSON length written back into the part that fits.
    uint8_t bytes[2];
    sw_out_init(&o, bytes, sizeof(bytes));
    const size_t start = sw_bson_begin(&o);
    sw_bson_put_int32(&o, "a", 1);
    sw_bson_end(&o, start);
    EXPECT_EQ(12u, o.len);
    EXPECT_EQ(12, bytes[0]);
    EXPECT_EQ(0, bytes[1]);
}

TEST(CTextUnitTest, ScratchBufferGrows)
{
    sw_buf b;
    sw_buf_init(&b);
    std::string want;
    for (int i = 0; i < 1000; i++)
    {
        sw_buf_byte(&b, static_cast<char>('a' + i % 26));
        want += static_cast<char>('a' + i % 26);
    }
    EXPECT_EQ(want, std::string(b.p, b.len));
    EXPECT_EQ('\0', b.p[b.len]);
    sw_buf_free(&b);
    EXPECT_EQ(0u, b.len);
}
