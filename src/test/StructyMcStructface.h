// This is the generated file

#ifndef __StructyMcStructface_h
#define __StructyMcStructface_h

#include "cstream.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct StructyMcStructfaceTag {
        float a_myfloat100;
        uint32_t b_myuint32_200;
        int64_t c_myint64_201;
        double d_myDoubleBubble202;
    } StructyMcStructface;

    int StructyMcStructface_serialize(StructyMcStructface* structure, Cstream* str);
    int StructyMcStructface_deserialize(StructyMcStructface* structure, Cstream* str);
    size_t StructyMcStructface_size();

    int StructyMcStructface_Get_a_myfloat100(const StructyMcStructface* structure, float* value);
    int StructyMcStructface_Get_b_myuint32_200(const StructyMcStructface* structure, uint32_t* value);
    int StructyMcStructface_Get_c_myint64_201(const StructyMcStructface* structure, int64_t* value);
    int StructyMcStructface_Get_d_myDoubleBubble202(const StructyMcStructface* structure, double* value);

    void StructyMcStructface_Set_a_myfloat100(StructyMcStructface* const structure, const float newval);
    void StructyMcStructface_Set_b_myuint32_200(StructyMcStructface* const structure, const uint32_t newval);
    void StructyMcStructface_Set_c_myint64_201(StructyMcStructface* const structure, const int64_t newval);
    void StructyMcStructface_Set_d_myDoubleBubble202(StructyMcStructface* const structure, const double newval);

    int StructyMcStructface_Equals(StructyMcStructface* const left, StructyMcStructface* const right);

#ifdef __cplusplus
}
#endif

#endif //__StructyMcStructface_h
