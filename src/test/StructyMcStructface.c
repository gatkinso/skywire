// This is the generated file

#include "StructyMcStructface.h"

int serialize(StructyMcStructface* structure, Cstream* str) {
    //str.serialize(structure->a_myfloat100);
    //str.serialize(structure->b_myuint32_200);
    //str.serialize(structure->c_myint64_201);
    //str.serialize(structure->d_myDoubleBubble202);
    return 0;
}
int deserialize(StructyMcStructface* structure, Cstream* str) {
    //str.deserialize(structure->a_myfloat100);
    //str.deserialize(structure->b_myuint32_200);
    //str.deserialize(structure->c_myint64_201);
    //str.deserialize(structure->d_myDoubleBubble202);
    return 0;
}
size_t size() {
    return(
        sizeof(float) +
        sizeof(uint32_t) +
        sizeof(int64_t) +
        sizeof(double)
        );
}
int Get_a_myfloat100(const StructyMcStructface* structure, float* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->a_myfloat100;
    return 0;
}
int Get_b_myuint32_200(const StructyMcStructface* structure, uint32_t* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->b_myuint32_200;
    return 0;
}
int Get_c_myint64_201(const StructyMcStructface* structure, int64_t* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->c_myint64_201;
    return 0;
}
int Get_d_myDoubleBubble202(const StructyMcStructface* structure, double* value) {
    if (NULL == structure || NULL == value)  return 1;
    *value = structure->d_myDoubleBubble202;
    return 0;
}
void Set_a_myfloat100(StructyMcStructface* const structure, const float newval) {
    if (NULL == structure)
        return;
    structure->a_myfloat100 = newval;
}
void Set_b_myuint32_200(StructyMcStructface* const structure, const uint32_t newval) {
    if (NULL == structure)
        return;
    structure->b_myuint32_200 = newval;
}
void Set_c_myint64_201(StructyMcStructface* const structure, const int64_t newval) {
    if (NULL == structure)
        return;
    structure->c_myint64_201 = newval;
}
void Set_d_myDoubleBubble202(StructyMcStructface* const structure, const double newval) {
    if (NULL == structure)
        return;
    structure->d_myDoubleBubble202 = newval;
}

