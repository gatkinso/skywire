// This is the generated .h file

#ifndef _StructyMcStructface_h
#define _StructyMcStructface_h

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
        char c_charcoal;
    } StructyMcStructface;

    int StructyMcStructface_Serialize(StructyMcStructface * structure, Cstream * str);
    int StructyMcStructface_Deserialize(StructyMcStructface * structure, Cstream * str);
    size_t StructyMcStructface_Size();
    int StructyMcStructface_Equals(StructyMcStructface * const left, StructyMcStructface * const right);
    int Get_StructyMcStructface_a_myfloat100(StructyMcStructface * structure, float * value);
    int Get_StructyMcStructface_b_myuint32_200(StructyMcStructface * structure, uint32_t * value);
    int Get_StructyMcStructface_c_myint64_201(StructyMcStructface * structure, int64_t * value);
    int Get_StructyMcStructface_d_myDoubleBubble202(StructyMcStructface * structure, double * value);
    int Get_StructyMcStructface_c_charcoal(StructyMcStructface * structure, char * value);
    int Set_StructyMcStructface_a_myfloat100(StructyMcStructface * const structure, float * const value);
    int Set_StructyMcStructface_b_myuint32_200(StructyMcStructface * const structure, uint32_t * const value);
    int Set_StructyMcStructface_c_myint64_201(StructyMcStructface * const structure, int64_t * const value);
    int Set_StructyMcStructface_d_myDoubleBubble202(StructyMcStructface * const structure, double * const value);
    int Set_StructyMcStructface_c_charcoal(StructyMcStructface * const structure, char * const value);

#ifdef __cplusplus
}
#endif

    #endif //_StructyMcStructface_h
