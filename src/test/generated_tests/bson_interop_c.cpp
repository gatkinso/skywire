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

// The C half of skywire_generated_tests:BsonInteropC (bson_interop.py): as
// bson_interop.cpp, for the generated C (StructyMcStructface, whose members
// are ClassyMcClassface's), so pymongo checks the C runtime too.
//
//   bson_interop_c write <dir> writes StructyMcStructface as BSON: values.bson
//                              (the shared test values) and extremes.bson
//                              (limits, NaN, -0, a binary string, Decimal128)
//   bson_interop_c read <file> reads a StructyMcStructface from BSON and
//                              prints it as JSON; exits 1 if _FromBson fails

#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "StructyMcStructface.h"

namespace
{

// Every member at a limit or an awkward value, given as JSON so the strings
// can hold any bytes.
const char* const extremes = R"({
    "a_mydouble100": "NaN",
    "b_myuint32_200": 4294967295,
    "c_myint64_201": "-9223372036854775808",
    "d_myfloat202": "-Infinity",
    "e_myint16_203": -32768,
    "f_myuint16_204": 65535,
    "g_myint32_205": -2147483648,
    "h_myuint64_206": "18446744073709551615",
    "i_charcoal301": -128,
    "j_mystring302": "\u0000ÿab",
    "k_uchar303": 255,
    "l_myuint8_304": 0,
    "m_myint8_305": 127,
    "n_shortstring306": "Ã©",
    "o_myfloat1000": 1e-45,
    "p_myuint32_2000": 0,
    "q_myint64_2010": "9223372036854775807",
    "r_mydouble2020": -0
})";

bool write_file(const std::string& path, const StructyMcStructface& message)
{
    std::vector<uint8_t> bytes(StructyMcStructface_ToBson(&message, nullptr, 0));
    StructyMcStructface_ToBson(&message, bytes.data(), bytes.size());
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(out);
}

}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "usage: bson_interop_c write <dir> | read <file>" << std::endl;
        return 2;
    }
    const std::string mode = argv[1];

    if (mode == "write")
    {
        const std::string dir = argv[2];
        StructyMcStructface values;
        std::memset(&values, 0, sizeof(values));
        values.a_mydouble100 = 3.14159;
        values.b_myuint32_200 = 3000000000u;
        values.c_myint64_201 = -9000000000;
        values.d_myfloat202 = 3.5f;
        values.e_myint16_203 = -12345;
        values.f_myuint16_204 = 60000;
        values.g_myint32_205 = -2000000000;
        values.h_myuint64_206 = 12000000000u;
        values.i_charcoal301 = 65;
        std::strncpy(values.j_mystring302, "skywire", sizeof(values.j_mystring302));
        values.k_uchar303 = 200;
        values.l_myuint8_304 = 201;
        values.m_myint8_305 = -50;
        std::strncpy(values.n_shortstring306, "skywire", sizeof(values.n_shortstring306));
        values.o_myfloat1000 = 3.5f;
        values.p_myuint32_2000 = 3000000000u;
        values.q_myint64_2010 = -9000000000;
        values.r_mydouble2020 = 3.14159;

        StructyMcStructface limits;
        if (0 != StructyMcStructface_FromJson(&limits, extremes, std::strlen(extremes)))
        {
            std::cerr << "the extremes are not valid JSON for StructyMcStructface" << std::endl;
            return 1;
        }

        if (false == write_file(dir + "/values.bson", values) || false == write_file(dir + "/extremes.bson", limits))
        {
            std::cerr << "cannot write to " << dir << std::endl;
            return 1;
        }
        return 0;
    }

    if (mode == "read")
    {
        std::ifstream in(argv[2], std::ios::binary);
        const std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        StructyMcStructface message;
        if (0 != StructyMcStructface_FromBson(&message, reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()))
        {
            std::cout << "_FromBson failed" << std::endl;
            return 1;
        }
        std::vector<char> json(StructyMcStructface_ToJson(&message, nullptr, 0) + 1);
        StructyMcStructface_ToJson(&message, json.data(), json.size());
        std::cout << json.data() << std::endl;
        return 0;
    }

    std::cerr << "unknown mode " << mode << std::endl;
    return 2;
}
