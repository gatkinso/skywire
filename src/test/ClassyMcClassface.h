// This is the generated file

#pragma once

#include "streamable.h"
#include <stdint.h>

namespace testspace {
    class ClassyMcClassface : public skyloupe::Streamable {
    public:
        ClassyMcClassface() {}
        virtual ~ClassyMcClassface() {}

        bool serialize(skyloupe::Stream& str) const { 
            str.serialize(a_myfloat100);
            str.serialize(b_myuint32_200);
            str.serialize(c_myint64_201);
            str.serialize(d_myDoubleBubble202);
            return true;
        }
        bool deserialize(skyloupe::Stream& str) { 
            str.deserialize(a_myfloat100);
            str.deserialize(b_myuint32_200);
            str.deserialize(c_myint64_201);
            str.deserialize(d_myDoubleBubble202);
            return true;
        }
        size_t size() const { 
            return(
                sizeof(a_myfloat100)+
                sizeof(b_myuint32_200)+
                sizeof(c_myint64_201)+
                sizeof(d_myDoubleBubble202)
            );
        }

        float Get_a_myfloat100() const { return a_myfloat100; }
        uint32_t Get_b_myuint32_200() const { return b_myuint32_200; }
        int64_t Get_c_myint64_201() const { return c_myint64_201; }
        double Get_d_myDoubleBubble202() const { return d_myDoubleBubble202; }

        void Set_a_myfloat100(const float newval) { a_myfloat100 = newval; }
        void Set_b_myuint32_200(const uint32_t newval) { b_myuint32_200 = newval; }
        void Set_c_myint64_201(const int64_t newval) { c_myint64_201 = newval; }
        void Set_d_myDoubleBubble202(const double newval) { d_myDoubleBubble202 = newval; }

    private:
        float a_myfloat100;
        uint32_t b_myuint32_200;
        int64_t c_myint64_201;
        double d_myDoubleBubble202;
    };
}
