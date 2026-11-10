// This is the generated .c file

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstring>
#include "ClassyMcClassface.h"

TEST(ClassyMcClassfaceTest, SerDes)
{
    Cstream cs;

    const int NUM = 1000;
    ClassyMcClassface out[NUM];
    cstream_init(&cs, 128);
    int sign = 1;
    for (uint32_t i = 0; i < NUM; i++)
    {
        out[i].a_mydouble100 = sign*i*12.2;
        out[i].b_myuint32_200 = i*14;
        out[i].c_myint64_201 = sign*i*-30;
        out[i].d_myfloat202 = sign*i*44.4f;
        out[i].e_myint16_203 = sign*i*-30;
        out[i].f_myuint16_204 = i*30;
        out[i].g_myint32_205 = sign*i*-56;
        out[i].h_myuint64_206 = i*72;
        out[i].i_charcoal301 = 9;
        strncpy(out[i].j_mystring302, "skywire", 16);
        out[i].k_uchar303 = i*20;
        out[i].l_myuint8_304 = i*33;
        out[i].m_myint8_305 = sign*i*-48;
        strncpy(out[i].n_shortstring306, "skywire", 4);
        out[i].o_myfloat1000 = sign*i*144.3f;
        out[i].p_myuint32_2000 = i*98;
        out[i].q_myint64_2010 = sign*i*-150;
        out[i].r_mydouble2020 = sign*i*195.2;
        sign *= -1;
        ClassyMcClassface_Serialize(&out[i], &cs);
    }

    for (uint32_t i = 0; i < NUM; i++)
    {
        ClassyMcClassface in;
        ClassyMcClassface_Deserialize(&in, &cs);

        ASSERT_EQ(1, ClassyMcClassface_Equals(&out[i], &in));
    }

    cstream_fini(&cs);
}

TEST(ClassyMcClassfaceTest, Size)
{
    ASSERT_EQ(ClassyMcClassface_Size(),sizeof(double)+sizeof(uint32_t)+sizeof(int64_t)+sizeof(float)+sizeof(int16_t)+sizeof(uint16_t)+sizeof(int32_t)+sizeof(uint64_t)+sizeof(char)+16+sizeof(unsigned char)+sizeof(uint8_t)+sizeof(int8_t)+4+sizeof(float)+sizeof(uint32_t)+sizeof(int64_t)+sizeof(double));
}

TEST(ClassyMcClassfaceTest, ShortBuffer)
{
    ClassyMcClassface in;
    std::memset(&in, 0x5a, sizeof(in));
    Cstream full;
    cstream_init(&full, 128);
    ASSERT_EQ(0, ClassyMcClassface_Serialize(&in, &full));
    ASSERT_EQ(ClassyMcClassface_Size(), cstream_available(&full));

    Cstream cs;
    cstream_init(&cs, 128);
    cstream_write(&cs, full.head_, ClassyMcClassface_Size() - 1, SW_NATIVE_ENDIAN_FLAG);

    ClassyMcClassface out;
    std::memset(&out, 0x11, sizeof(out));
    ClassyMcClassface before = out;

    ASSERT_EQ(-1, ClassyMcClassface_Deserialize(&out, &cs));
    ASSERT_EQ(0, std::memcmp(&out, &before, sizeof(out)));
    ASSERT_EQ(ClassyMcClassface_Size() - 1, cstream_available(&cs));

    cstream_fini(&cs);
    cstream_fini(&full);
}

TEST(ClassyMcClassfaceTest, Wire)
{
    ClassyMcClassface in;
    std::memset(&in, 0, sizeof(in));
    in.a_mydouble100 = 3.14159;
    in.b_myuint32_200 = 3000000000;
    in.c_myint64_201 = -9000000000;
    in.d_myfloat202 = 3.5;
    in.e_myint16_203 = -12345;
    in.f_myuint16_204 = 60000;
    in.g_myint32_205 = -2000000000;
    in.h_myuint64_206 = 12000000000;
    in.i_charcoal301 = 65;
    strncpy(in.j_mystring302, "skywire", 16);
    in.k_uchar303 = 200;
    in.l_myuint8_304 = 201;
    in.m_myint8_305 = -50;
    strncpy(in.n_shortstring306, "skywire", 4);
    in.o_myfloat1000 = 3.5;
    in.p_myuint32_2000 = 3000000000;
    in.q_myint64_2010 = -9000000000;
    in.r_mydouble2020 = 3.14159;

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

    Cstream cs;
    cstream_init(&cs, 128);
    ASSERT_EQ(0, ClassyMcClassface_Serialize(&in, &cs));
    ASSERT_EQ(sizeof(want), cstream_available(&cs));
    ASSERT_EQ(0, std::memcmp(cs.head_, want, sizeof(want)));
    cstream_fini(&cs);
}

