// This is the generated .h file

#ifndef _ClassyMcClassface_h
#define _ClassyMcClassface_h

#include "cstream.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct ClassyMcClassfaceTag {
        double a_mydouble100;
        uint32_t b_myuint32_200;
        int64_t c_myint64_201;
        float d_myfloat202;
        int16_t e_myint16_203;
        uint16_t f_myuint16_204;
        int32_t g_myint32_205;
        uint64_t h_myuint64_206;
        char i_charcoal301;
        char j_mystring302[16];
        unsigned char k_uchar303;
        uint8_t l_myuint8_304;
        int8_t m_myint8_305;
        char n_shortstring306[4];
        float o_myfloat1000;
        uint32_t p_myuint32_2000;
        int64_t q_myint64_2010;
        double r_mydouble2020;
    } ClassyMcClassface;

    int ClassyMcClassface_Serialize(ClassyMcClassface * structure, Cstream * str);
    char* ClassyMcClassface_SerializeSw(const ClassyMcClassface * structure, char * p);
    int ClassyMcClassface_Deserialize(ClassyMcClassface * structure, Cstream * str);
    const char* ClassyMcClassface_DeserializeSw(ClassyMcClassface * structure, const char * p);
    size_t ClassyMcClassface_Size();
    int ClassyMcClassface_Equals(ClassyMcClassface * const left, ClassyMcClassface * const right);
    int Get_ClassyMcClassface_a_mydouble100(ClassyMcClassface * structure, double * value);
    int Get_ClassyMcClassface_b_myuint32_200(ClassyMcClassface * structure, uint32_t * value);
    int Get_ClassyMcClassface_c_myint64_201(ClassyMcClassface * structure, int64_t * value);
    int Get_ClassyMcClassface_d_myfloat202(ClassyMcClassface * structure, float * value);
    int Get_ClassyMcClassface_e_myint16_203(ClassyMcClassface * structure, int16_t * value);
    int Get_ClassyMcClassface_f_myuint16_204(ClassyMcClassface * structure, uint16_t * value);
    int Get_ClassyMcClassface_g_myint32_205(ClassyMcClassface * structure, int32_t * value);
    int Get_ClassyMcClassface_h_myuint64_206(ClassyMcClassface * structure, uint64_t * value);
    int Get_ClassyMcClassface_i_charcoal301(ClassyMcClassface * structure, char * value);
    int Get_ClassyMcClassface_j_mystring302(ClassyMcClassface * structure, char * value);
    int Get_ClassyMcClassface_k_uchar303(ClassyMcClassface * structure, unsigned char * value);
    int Get_ClassyMcClassface_l_myuint8_304(ClassyMcClassface * structure, uint8_t * value);
    int Get_ClassyMcClassface_m_myint8_305(ClassyMcClassface * structure, int8_t * value);
    int Get_ClassyMcClassface_n_shortstring306(ClassyMcClassface * structure, char * value);
    int Get_ClassyMcClassface_o_myfloat1000(ClassyMcClassface * structure, float * value);
    int Get_ClassyMcClassface_p_myuint32_2000(ClassyMcClassface * structure, uint32_t * value);
    int Get_ClassyMcClassface_q_myint64_2010(ClassyMcClassface * structure, int64_t * value);
    int Get_ClassyMcClassface_r_mydouble2020(ClassyMcClassface * structure, double * value);
    int Set_ClassyMcClassface_a_mydouble100(ClassyMcClassface * const structure, double * const value);
    int Set_ClassyMcClassface_b_myuint32_200(ClassyMcClassface * const structure, uint32_t * const value);
    int Set_ClassyMcClassface_c_myint64_201(ClassyMcClassface * const structure, int64_t * const value);
    int Set_ClassyMcClassface_d_myfloat202(ClassyMcClassface * const structure, float * const value);
    int Set_ClassyMcClassface_e_myint16_203(ClassyMcClassface * const structure, int16_t * const value);
    int Set_ClassyMcClassface_f_myuint16_204(ClassyMcClassface * const structure, uint16_t * const value);
    int Set_ClassyMcClassface_g_myint32_205(ClassyMcClassface * const structure, int32_t * const value);
    int Set_ClassyMcClassface_h_myuint64_206(ClassyMcClassface * const structure, uint64_t * const value);
    int Set_ClassyMcClassface_i_charcoal301(ClassyMcClassface * const structure, char * const value);
    int Set_ClassyMcClassface_j_mystring302(ClassyMcClassface * const structure, const char * value);
    int Set_ClassyMcClassface_k_uchar303(ClassyMcClassface * const structure, unsigned char * const value);
    int Set_ClassyMcClassface_l_myuint8_304(ClassyMcClassface * const structure, uint8_t * const value);
    int Set_ClassyMcClassface_m_myint8_305(ClassyMcClassface * const structure, int8_t * const value);
    int Set_ClassyMcClassface_n_shortstring306(ClassyMcClassface * const structure, const char * value);
    int Set_ClassyMcClassface_o_myfloat1000(ClassyMcClassface * const structure, float * const value);
    int Set_ClassyMcClassface_p_myuint32_2000(ClassyMcClassface * const structure, uint32_t * const value);
    int Set_ClassyMcClassface_q_myint64_2010(ClassyMcClassface * const structure, int64_t * const value);
    int Set_ClassyMcClassface_r_mydouble2020(ClassyMcClassface * const structure, double * const value);

#ifdef __cplusplus
}
#endif

    #endif //_ClassyMcClassface_h
