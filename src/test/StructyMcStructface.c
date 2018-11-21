// This is the generated file

#include "StructyMcStructface.h"

int StructyMcStructface_serialize(StructyMcStructface* structure, Cstream* str) {
    if (NULL == structure || NULL == str) return -1;
    cstream_write(str, &structure->a_myfloat100, sizeof(structure->a_myfloat100));
    cstream_write(str, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200));
    cstream_write(str, &structure->c_myint64_201, sizeof(structure->c_myint64_201));
    cstream_write(str, &structure->d_myDoubleBubble202, sizeof(&structure->d_myDoubleBubble202));
    return 0;
}
int StructyMcStructface_deserialize(StructyMcStructface* structure, Cstream* str) {
    if (NULL == structure || NULL == str) return -1;
    cstream_read(str, &structure->a_myfloat100, sizeof(structure->a_myfloat100));
    cstream_read(str, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200));
    cstream_read(str, &structure->c_myint64_201, sizeof(structure->c_myint64_201));
    cstream_read(str, &structure->d_myDoubleBubble202, sizeof(&structure->d_myDoubleBubble202));
    return 0;
}
size_t StructyMcStructface_size() {
    return(
        sizeof(float) +
        sizeof(uint32_t) +
        sizeof(int64_t) +
        sizeof(double)
        );
}
int StructyMcStructface_Equals(StructyMcStructface* const left, StructyMcStructface* const right) {
    if (NULL == left || NULL == right) return 0;

    if (left->a_myfloat100 == right->a_myfloat100 &&
        left->b_myuint32_200 == right->b_myuint32_200 &&
        left->c_myint64_201 == right->c_myint64_201 &&
        left->d_myDoubleBubble202 == right->d_myDoubleBubble202)
        return 1;
    return 0;
}
int StructyMcStructface_Get_a_myfloat100(const StructyMcStructface* structure, float* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->a_myfloat100;
    return 0;
}
int StructyMcStructface_Get_b_myuint32_200(const StructyMcStructface* structure, uint32_t* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->b_myuint32_200;
    return 0;
}
int StructyMcStructface_Get_c_myint64_201(const StructyMcStructface* structure, int64_t* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->c_myint64_201;
    return 0;
}
int StructyMcStructface_Get_d_myDoubleBubble202(const StructyMcStructface* structure, double* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->d_myDoubleBubble202;
    return 0;
}
void StructyMcStructface_Set_a_myfloat100(StructyMcStructface* const structure, const float newval) {
    if (NULL == structure)
        return;
    structure->a_myfloat100 = newval;
}
void StructyMcStructface_Set_b_myuint32_200(StructyMcStructface* const structure, const uint32_t newval) {
    if (NULL == structure)
        return;
    structure->b_myuint32_200 = newval;
}
void StructyMcStructface_Set_c_myint64_201(StructyMcStructface* const structure, const int64_t newval) {
    if (NULL == structure)
        return;
    structure->c_myint64_201 = newval;
}
void StructyMcStructface_Set_d_myDoubleBubble202(StructyMcStructface* const structure, const double newval) {
    if (NULL == structure)
        return;
    structure->d_myDoubleBubble202 = newval;
}

