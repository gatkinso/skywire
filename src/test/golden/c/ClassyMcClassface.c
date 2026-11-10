// This is the generated .c file

#include "ClassyMcClassface.h"

int ClassyMcClassface_Serialize(ClassyMcClassface * structure, Cstream * str) {
    char* p;
    if (NULL == structure || NULL == str) return -1;
    p = cstream_claim(str, ClassyMcClassface_Size());
    if (NULL == p) return -1;
    ClassyMcClassface_SerializeSw(structure, p);
    return 0;
}
// Writes the wire form, ClassyMcClassface_Size() bytes, at p; returns the end.
char* ClassyMcClassface_SerializeSw(const ClassyMcClassface * structure, char * p) {
    p = sw_put(p, &structure->a_mydouble100, sizeof(structure->a_mydouble100), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->c_myint64_201, sizeof(structure->c_myint64_201), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->d_myfloat202, sizeof(structure->d_myfloat202), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->e_myint16_203, sizeof(structure->e_myint16_203), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->f_myuint16_204, sizeof(structure->f_myuint16_204), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->g_myint32_205, sizeof(structure->g_myint32_205), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->h_myuint64_206, sizeof(structure->h_myuint64_206), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->i_charcoal301, sizeof(structure->i_charcoal301), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->j_mystring302, sizeof(structure->j_mystring302), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->k_uchar303, sizeof(structure->k_uchar303), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->l_myuint8_304, sizeof(structure->l_myuint8_304), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->m_myint8_305, sizeof(structure->m_myint8_305), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->n_shortstring306, sizeof(structure->n_shortstring306), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->o_myfloat1000, sizeof(structure->o_myfloat1000), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->p_myuint32_2000, sizeof(structure->p_myuint32_2000), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->q_myint64_2010, sizeof(structure->q_myint64_2010), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->r_mydouble2020, sizeof(structure->r_mydouble2020), SW_BIG_ENDIAN_FLAG);
    return p;
}
int ClassyMcClassface_Deserialize(ClassyMcClassface * structure, Cstream * str) {
    const char* p;
    if (NULL == structure || NULL == str) return -1;
    p = cstream_consume(str, ClassyMcClassface_Size());
    if (NULL == p) return -1;
    ClassyMcClassface_DeserializeSw(structure, p);
    return 0;
}
// Reads the wire form, ClassyMcClassface_Size() bytes, from p; returns the end.
const char* ClassyMcClassface_DeserializeSw(ClassyMcClassface * structure, const char * p) {
    p = sw_get(p, &structure->a_mydouble100, sizeof(structure->a_mydouble100), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->c_myint64_201, sizeof(structure->c_myint64_201), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->d_myfloat202, sizeof(structure->d_myfloat202), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->e_myint16_203, sizeof(structure->e_myint16_203), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->f_myuint16_204, sizeof(structure->f_myuint16_204), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->g_myint32_205, sizeof(structure->g_myint32_205), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->h_myuint64_206, sizeof(structure->h_myuint64_206), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->i_charcoal301, sizeof(structure->i_charcoal301), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->j_mystring302, sizeof(structure->j_mystring302), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->k_uchar303, sizeof(structure->k_uchar303), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->l_myuint8_304, sizeof(structure->l_myuint8_304), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->m_myint8_305, sizeof(structure->m_myint8_305), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->n_shortstring306, sizeof(structure->n_shortstring306), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->o_myfloat1000, sizeof(structure->o_myfloat1000), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->p_myuint32_2000, sizeof(structure->p_myuint32_2000), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->q_myint64_2010, sizeof(structure->q_myint64_2010), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->r_mydouble2020, sizeof(structure->r_mydouble2020), SW_BIG_ENDIAN_FLAG);
    return p;
}
size_t ClassyMcClassface_Size() {
    return (
        sizeof(double)+
        sizeof(uint32_t)+
        sizeof(int64_t)+
        sizeof(float)+
        sizeof(int16_t)+
        sizeof(uint16_t)+
        sizeof(int32_t)+
        sizeof(uint64_t)+
        sizeof(char)+
        16+
        sizeof(unsigned char)+
        sizeof(uint8_t)+
        sizeof(int8_t)+
        4+
        sizeof(float)+
        sizeof(uint32_t)+
        sizeof(int64_t)+
        sizeof(double)
    );
}
int ClassyMcClassface_Equals(ClassyMcClassface * const left, ClassyMcClassface * const right) {
    if (NULL == left || NULL == right) return -1;
    if(
        left->a_mydouble100 == right->a_mydouble100 &&
        left->b_myuint32_200 == right->b_myuint32_200 &&
        left->c_myint64_201 == right->c_myint64_201 &&
        left->d_myfloat202 == right->d_myfloat202 &&
        left->e_myint16_203 == right->e_myint16_203 &&
        left->f_myuint16_204 == right->f_myuint16_204 &&
        left->g_myint32_205 == right->g_myint32_205 &&
        left->h_myuint64_206 == right->h_myuint64_206 &&
        left->i_charcoal301 == right->i_charcoal301 &&
        memcmp(left->j_mystring302, right->j_mystring302, 16) == 0 &&
        left->k_uchar303 == right->k_uchar303 &&
        left->l_myuint8_304 == right->l_myuint8_304 &&
        left->m_myint8_305 == right->m_myint8_305 &&
        memcmp(left->n_shortstring306, right->n_shortstring306, 4) == 0 &&
        left->o_myfloat1000 == right->o_myfloat1000 &&
        left->p_myuint32_2000 == right->p_myuint32_2000 &&
        left->q_myint64_2010 == right->q_myint64_2010 &&
        left->r_mydouble2020 == right->r_mydouble2020 
    )
        return 1;
    return 0;
}
int Get_ClassyMcClassface_a_mydouble100(ClassyMcClassface * structure, double * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->a_mydouble100;
    return 0;
}
int Get_ClassyMcClassface_b_myuint32_200(ClassyMcClassface * structure, uint32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->b_myuint32_200;
    return 0;
}
int Get_ClassyMcClassface_c_myint64_201(ClassyMcClassface * structure, int64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->c_myint64_201;
    return 0;
}
int Get_ClassyMcClassface_d_myfloat202(ClassyMcClassface * structure, float * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->d_myfloat202;
    return 0;
}
int Get_ClassyMcClassface_e_myint16_203(ClassyMcClassface * structure, int16_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->e_myint16_203;
    return 0;
}
int Get_ClassyMcClassface_f_myuint16_204(ClassyMcClassface * structure, uint16_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->f_myuint16_204;
    return 0;
}
int Get_ClassyMcClassface_g_myint32_205(ClassyMcClassface * structure, int32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->g_myint32_205;
    return 0;
}
int Get_ClassyMcClassface_h_myuint64_206(ClassyMcClassface * structure, uint64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->h_myuint64_206;
    return 0;
}
int Get_ClassyMcClassface_i_charcoal301(ClassyMcClassface * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->i_charcoal301;
    return 0;
}
int Get_ClassyMcClassface_j_mystring302(ClassyMcClassface * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    memcpy(value, structure->j_mystring302, 16);
    return 0;
}
int Get_ClassyMcClassface_k_uchar303(ClassyMcClassface * structure, unsigned char * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->k_uchar303;
    return 0;
}
int Get_ClassyMcClassface_l_myuint8_304(ClassyMcClassface * structure, uint8_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->l_myuint8_304;
    return 0;
}
int Get_ClassyMcClassface_m_myint8_305(ClassyMcClassface * structure, int8_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->m_myint8_305;
    return 0;
}
int Get_ClassyMcClassface_n_shortstring306(ClassyMcClassface * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    memcpy(value, structure->n_shortstring306, 4);
    return 0;
}
int Get_ClassyMcClassface_o_myfloat1000(ClassyMcClassface * structure, float * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->o_myfloat1000;
    return 0;
}
int Get_ClassyMcClassface_p_myuint32_2000(ClassyMcClassface * structure, uint32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->p_myuint32_2000;
    return 0;
}
int Get_ClassyMcClassface_q_myint64_2010(ClassyMcClassface * structure, int64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->q_myint64_2010;
    return 0;
}
int Get_ClassyMcClassface_r_mydouble2020(ClassyMcClassface * structure, double * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->r_mydouble2020;
    return 0;
}
int Set_ClassyMcClassface_a_mydouble100(ClassyMcClassface * const structure, double * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->a_mydouble100 = *value;
    return 0;
}
int Set_ClassyMcClassface_b_myuint32_200(ClassyMcClassface * const structure, uint32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->b_myuint32_200 = *value;
    return 0;
}
int Set_ClassyMcClassface_c_myint64_201(ClassyMcClassface * const structure, int64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->c_myint64_201 = *value;
    return 0;
}
int Set_ClassyMcClassface_d_myfloat202(ClassyMcClassface * const structure, float * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->d_myfloat202 = *value;
    return 0;
}
int Set_ClassyMcClassface_e_myint16_203(ClassyMcClassface * const structure, int16_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->e_myint16_203 = *value;
    return 0;
}
int Set_ClassyMcClassface_f_myuint16_204(ClassyMcClassface * const structure, uint16_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->f_myuint16_204 = *value;
    return 0;
}
int Set_ClassyMcClassface_g_myint32_205(ClassyMcClassface * const structure, int32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->g_myint32_205 = *value;
    return 0;
}
int Set_ClassyMcClassface_h_myuint64_206(ClassyMcClassface * const structure, uint64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->h_myuint64_206 = *value;
    return 0;
}
int Set_ClassyMcClassface_i_charcoal301(ClassyMcClassface * const structure, char * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->i_charcoal301 = *value;
    return 0;
}
int Set_ClassyMcClassface_j_mystring302(ClassyMcClassface * const structure, const char * value) {
    if (NULL == structure || NULL == value) return -1;
    strncpy(structure->j_mystring302, value, 16);
    return 0;
}
int Set_ClassyMcClassface_k_uchar303(ClassyMcClassface * const structure, unsigned char * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->k_uchar303 = *value;
    return 0;
}
int Set_ClassyMcClassface_l_myuint8_304(ClassyMcClassface * const structure, uint8_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->l_myuint8_304 = *value;
    return 0;
}
int Set_ClassyMcClassface_m_myint8_305(ClassyMcClassface * const structure, int8_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->m_myint8_305 = *value;
    return 0;
}
int Set_ClassyMcClassface_n_shortstring306(ClassyMcClassface * const structure, const char * value) {
    if (NULL == structure || NULL == value) return -1;
    strncpy(structure->n_shortstring306, value, 4);
    return 0;
}
int Set_ClassyMcClassface_o_myfloat1000(ClassyMcClassface * const structure, float * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->o_myfloat1000 = *value;
    return 0;
}
int Set_ClassyMcClassface_p_myuint32_2000(ClassyMcClassface * const structure, uint32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->p_myuint32_2000 = *value;
    return 0;
}
int Set_ClassyMcClassface_q_myint64_2010(ClassyMcClassface * const structure, int64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->q_myint64_2010 = *value;
    return 0;
}
int Set_ClassyMcClassface_r_mydouble2020(ClassyMcClassface * const structure, double * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->r_mydouble2020 = *value;
    return 0;
}
