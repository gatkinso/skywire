// This is the generated .c file

#include "StructyMcStructface.h"

int StructyMcStructface_Serialize(StructyMcStructface * structure, Cstream * str) {
    if (NULL == structure || NULL == str) return -1;
    cstream_write(str, &structure->a_myfloat100, sizeof(structure->a_myfloat100));
    cstream_write(str, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200));
    cstream_write(str, &structure->c_myint64_201, sizeof(structure->c_myint64_201));
    cstream_write(str, &structure->d_myDoubleBubble202, sizeof(structure->d_myDoubleBubble202));
    cstream_write(str, &structure->c_charcoal, sizeof(structure->c_charcoal));
    return 0;
}
int StructyMcStructface_Deserialize(StructyMcStructface * structure, Cstream * str) {
    if (NULL == structure || NULL == str) return -1;
    cstream_read(str, &structure->a_myfloat100, sizeof(structure->a_myfloat100));
    cstream_read(str, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200));
    cstream_read(str, &structure->c_myint64_201, sizeof(structure->c_myint64_201));
    cstream_read(str, &structure->d_myDoubleBubble202, sizeof(structure->d_myDoubleBubble202));
    cstream_read(str, &structure->c_charcoal, sizeof(structure->c_charcoal));
    return 0;
}
size_t StructyMcStructface_Size() {
    return (
        sizeof(float)+
        sizeof(uint32_t)+
        sizeof(int64_t)+
        sizeof(double)+
        sizeof(char)
    );
}
int StructyMcStructface_Equals(StructyMcStructface * const left, StructyMcStructface * const right) {
    if (NULL == left || NULL == right) return -1;
    if(
        left->a_myfloat100 == right->a_myfloat100&&
        left->b_myuint32_200 == right->b_myuint32_200&&
        left->c_myint64_201 == right->c_myint64_201&&
        left->d_myDoubleBubble202 == right->d_myDoubleBubble202&&
        left->c_charcoal == right->c_charcoal
    )
        return 1;
    return 0;
}
int Get_StructyMcStructface_a_myfloat100(StructyMcStructface * structure, float * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->a_myfloat100;
    return 0;
}
int Get_StructyMcStructface_b_myuint32_200(StructyMcStructface * structure, uint32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->b_myuint32_200;
    return 0;
}
int Get_StructyMcStructface_c_myint64_201(StructyMcStructface * structure, int64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->c_myint64_201;
    return 0;
}
int Get_StructyMcStructface_d_myDoubleBubble202(StructyMcStructface * structure, double * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->d_myDoubleBubble202;
    return 0;
}
int Get_StructyMcStructface_c_charcoal(StructyMcStructface * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->c_charcoal;
    return 0;
}
int Set_StructyMcStructface_a_myfloat100(StructyMcStructface * const structure, float * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->a_myfloat100 = *value;
    return 0;
}
int Set_StructyMcStructface_b_myuint32_200(StructyMcStructface * const structure, uint32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->b_myuint32_200 = *value;
    return 0;
}
int Set_StructyMcStructface_c_myint64_201(StructyMcStructface * const structure, int64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->c_myint64_201 = *value;
    return 0;
}
int Set_StructyMcStructface_d_myDoubleBubble202(StructyMcStructface * const structure, double * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->d_myDoubleBubble202 = *value;
    return 0;
}
int Set_StructyMcStructface_c_charcoal(StructyMcStructface * const structure, char * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->c_charcoal = *value;
    return 0;
}
