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

    int serialize(StructyMcStructface* structure, Cstream* str);
    int deserialize(StructyMcStructface* structure, Cstream* str);
    size_t size();

    int Get_a_myfloat100(const StructyMcStructface* structure, float* value);
    int Get_b_myuint32_200(const StructyMcStructface* structure, uint32_t* value);
    int Get_c_myint64_201(const StructyMcStructface* structure, int64_t* value);
    int Get_d_myDoubleBubble202(const StructyMcStructface* structure, double* value);

    void Set_a_myfloat100(StructyMcStructface* const structure, const float newval);
    void Set_b_myuint32_200(StructyMcStructface* const structure, const uint32_t newval);
    void Set_c_myint64_201(StructyMcStructface* const structure, const int64_t newval);
    void Set_d_myDoubleBubble202(StructyMcStructface* const structure, const double newval);

#ifdef __cplusplus
}
#endif

#endif //__StructyMcStructface_h
