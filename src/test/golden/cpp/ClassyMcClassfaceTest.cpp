// This is the generated file

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include "ClassyMcClassface.h"

TEST(ClassyMcClassfaceTest, SerDes)
{
    testspace::ClassyMcClassface in, out;
    skyloupe::Stream stream;

    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);
}

TEST(ClassyMcClassfaceTest, Size)
{
    testspace::ClassyMcClassface msg;

    ASSERT_EQ(msg.size(), sizeof(double)+sizeof(uint32_t)+sizeof(int64_t)+sizeof(float)+sizeof(int16_t)+sizeof(uint16_t)+sizeof(int32_t)+sizeof(uint64_t)+sizeof(char)+16+sizeof(unsigned char)+sizeof(uint8_t)+sizeof(int8_t)+4+sizeof(float)+sizeof(uint32_t)+sizeof(int64_t)+sizeof(double));
}

TEST(ClassyMcClassfaceTest, EqualityOperator)
{
    testspace::ClassyMcClassface in, out;
    skyloupe::Stream stream;

    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);

    ASSERT_EQ(in, out);
}

TEST(ClassyMcClassfaceTest, ShortBuffer)
{
    testspace::ClassyMcClassface in;
    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    skyloupe::Stream full;
    ASSERT_TRUE(in.serialize(full));
    ASSERT_EQ(full.byte_size(), in.size());

    skyloupe::Stream stream;
    stream.write(full.data(), full.byte_size() - 1);

    testspace::ClassyMcClassface out = in;
    ASSERT_FALSE(out.deserialize(stream));
    ASSERT_EQ(out, in);
    ASSERT_EQ(stream.byte_size(), in.size() - 1);
}

TEST(ClassyMcClassfaceTest, Wire)
{
    testspace::ClassyMcClassface in;
    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    static const uint8_t want[] = {
        0x6e, 0x86, 0x1b, 0xf0, 0xf9, 0x21, 0x09, 0x40,
        0xb2, 0xd0, 0x5e, 0x00, 0xff, 0xff, 0xff, 0xfd,
        0xe7, 0x8e, 0xe6, 0x00, 0x40, 0x60, 0x00, 0x00,
        0xc7, 0xcf, 0xea, 0x60, 0x00, 0x6c, 0xca, 0x88,
        0x00, 0x78, 0x41, 0xcb, 0x02, 0x00, 0x00, 0x00,
        0x41, 0x73, 0x6b, 0x79, 0x77, 0x69, 0x72, 0x65,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xc8, 0xc9, 0xce, 0x73, 0x6b, 0x79, 0x77,
        0x00, 0x00, 0x60, 0x40, 0x00, 0x5e, 0xd0, 0xb2,
        0x00, 0xe6, 0x8e, 0xe7, 0xfd, 0xff, 0xff, 0xff,
        0x40, 0x09, 0x21, 0xf9, 0xf0, 0x1b, 0x86, 0x6e,
    };

    skyloupe::Stream stream;
    ASSERT_TRUE(in.serialize(stream));
    ASSERT_EQ(sizeof(want), stream.byte_size());
    ASSERT_EQ(0, std::memcmp(stream.data(), want, sizeof(want)));
}

TEST(ClassyMcClassfaceTest, PZRoundTrip)
{
    testspace::ClassyMcClassface in, out;
    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(stream));
    ASSERT_TRUE(out.deserialize(stream));
    ASSERT_EQ(in, out);
    ASSERT_EQ(0u, stream.byte_size());
}

TEST(ClassyMcClassfaceTest, PZWire)
{
    testspace::ClassyMcClassface in;
    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    static const uint8_t want[] = {
        0xa1, 0x06, 0x6e, 0x86, 0x1b, 0xf0, 0xf9, 0x21,
        0x09, 0x40, 0xc0, 0x0c, 0x80, 0xbc, 0xc1, 0x96,
        0x0b, 0xc8, 0x0c, 0x80, 0xcc, 0xbb, 0xbc, 0xde,
        0xff, 0xff, 0xff, 0xff, 0x01, 0xd5, 0x0c, 0x00,
        0x00, 0x60, 0x40, 0xd8, 0x0c, 0xc7, 0x9f, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xe0,
        0x0c, 0xe0, 0xd4, 0x03, 0xe8, 0x0c, 0x80, 0xd8,
        0xa9, 0xc6, 0xf8, 0xff, 0xff, 0xff, 0xff, 0x01,
        0xf0, 0x0c, 0x80, 0xf0, 0x85, 0xda, 0x2c, 0xe8,
        0x12, 0x41, 0xf2, 0x12, 0x07, 0x73, 0x6b, 0x79,
        0x77, 0x69, 0x72, 0x65, 0xf8, 0x12, 0xc8, 0x01,
        0x80, 0x13, 0xc9, 0x01, 0x88, 0x13, 0xce, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
        0x92, 0x13, 0x04, 0x73, 0x6b, 0x79, 0x77, 0xc5,
        0x3e, 0x00, 0x00, 0x60, 0x40, 0x80, 0x7d, 0x80,
        0xbc, 0xc1, 0x96, 0x0b, 0xd0, 0x7d, 0x80, 0xcc,
        0xbb, 0xbc, 0xde, 0xff, 0xff, 0xff, 0xff, 0x01,
        0xa1, 0x7e, 0x6e, 0x86, 0x1b, 0xf0, 0xf9, 0x21,
        0x09, 0x40,
    };

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(stream));
    ASSERT_EQ(sizeof(want), stream.byte_size());
    ASSERT_EQ(0, std::memcmp(stream.data(), want, sizeof(want)));
}

TEST(ClassyMcClassfaceTest, PZEmptyIsDefault)
{
    testspace::ClassyMcClassface zero, in, out;
    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(zero.serialize(stream));
    ASSERT_EQ(0u, stream.byte_size());

    out = in;
    ASSERT_TRUE(out.deserialize(stream));
    ASSERT_EQ(zero, out);
}

TEST(ClassyMcClassfaceTest, PZShortBuffer)
{
    testspace::ClassyMcClassface in;
    in.Set_a_mydouble100(3.14159);
    in.Set_b_myuint32_200(3000000000);
    in.Set_c_myint64_201(-9000000000);
    in.Set_d_myfloat202(3.5);
    in.Set_e_myint16_203(-12345);
    in.Set_f_myuint16_204(60000);
    in.Set_g_myint32_205(-2000000000);
    in.Set_h_myuint64_206(12000000000);
    in.Set_i_charcoal301(65);
    in.Set_j_mystring302("skywire");
    in.Set_k_uchar303(200);
    in.Set_l_myuint8_304(201);
    in.Set_m_myint8_305(-50);
    in.Set_n_shortstring306("skywire");
    in.Set_o_myfloat1000(3.5);
    in.Set_p_myuint32_2000(3000000000);
    in.Set_q_myint64_2010(-9000000000);
    in.Set_r_mydouble2020(3.14159);

    skyloupe::Stream full(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(full));
    ASSERT_LT(0u, full.byte_size());

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    stream.pz_buffer() = full.pz_buffer().substr(0, full.byte_size() - 1);

    testspace::ClassyMcClassface out;
    ASSERT_FALSE(out.deserialize(stream));
    ASSERT_EQ(out, testspace::ClassyMcClassface());
    ASSERT_EQ(stream.byte_size(), full.byte_size() - 1);
}

