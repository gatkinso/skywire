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

// The C++ half of skywire_generated_tests:BsonInterop (bson_interop.py),
// which checks the generated BSON against pymongo's own implementation.
//
//   bson_interop write <dir>   writes ClassyMcClassface as BSON: values.bson
//                              (the shared test values) and extremes.bson
//                              (limits, NaN, -0, a binary string, Decimal128)
//   bson_interop read <file>   reads a ClassyMcClassface from BSON and prints
//                              it as JSON; exits 1 if from_bson fails

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "ClassyMcClassface.h"

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

bool write_file(const std::string& path, const std::string& bytes)
{
    std::ofstream out(path, std::ios::binary);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(out);
}

}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "usage: bson_interop write <dir> | read <file>" << std::endl;
        return 2;
    }
    const std::string mode = argv[1];

    if (mode == "write")
    {
        const std::string dir = argv[2];
        testspace::ClassyMcClassface values;
        values.Set_a_mydouble100(3.14159);
        values.Set_b_myuint32_200(3000000000);
        values.Set_c_myint64_201(-9000000000);
        values.Set_d_myfloat202(3.5);
        values.Set_e_myint16_203(-12345);
        values.Set_f_myuint16_204(60000);
        values.Set_g_myint32_205(-2000000000);
        values.Set_h_myuint64_206(12000000000);
        values.Set_i_charcoal301(65);
        values.Set_j_mystring302("skywire");
        values.Set_k_uchar303(200);
        values.Set_l_myuint8_304(201);
        values.Set_m_myint8_305(-50);
        values.Set_n_shortstring306("skywire");
        values.Set_o_myfloat1000(3.5);
        values.Set_p_myuint32_2000(3000000000);
        values.Set_q_myint64_2010(-9000000000);
        values.Set_r_mydouble2020(3.14159);

        testspace::ClassyMcClassface limits;
        if (false == limits.from_json(extremes))
        {
            std::cerr << "the extremes are not valid JSON for ClassyMcClassface" << std::endl;
            return 1;
        }

        if (false == write_file(dir + "/values.bson", values.to_bson()) ||
            false == write_file(dir + "/extremes.bson", limits.to_bson()))
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
        testspace::ClassyMcClassface message;
        if (false == message.from_bson(bytes))
        {
            std::cout << "from_bson failed" << std::endl;
            return 1;
        }
        std::cout << message.to_json() << std::endl;
        return 0;
    }

    std::cerr << "unknown mode " << mode << std::endl;
    return 2;
}
