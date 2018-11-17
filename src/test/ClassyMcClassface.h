// This is the generated file
#pragma once
#include "streamable.h"
#include <stdint.h>
#include <iostream>
namespace testspace {
    class ClassyMcClassface : public skyloupe::Streamable {
    public:
        ClassyMcClassface() {}
        virtual ~ClassyMcClassface() {}

        bool serialize(skyloupe::Stream& str) const { return true; }
        bool deserialize(skyloupe::Stream& str) { return true; }
        size_t size() const { return 0; }
        float Get_a_myfloat100() const { return a_myfloat100; }
        int Get_b_myint200() const { return b_myint200; }
        int Get_c_myint201() const { return c_myint201; }
        double Get_d_myDoubleBubble202() const { return d_myDoubleBubble202; }

        void Set_a_myfloat100(const float newval) { a_myfloat100 = newval; }
        void Set_b_myint200(const int newval) { b_myint200 = newval; }
        void Set_c_myint201(const int newval) { c_myint201 = newval; }
        void Set_d_myDoubleBubble202(const double newval) { d_myDoubleBubble202 = newval; }

    private:
        float a_myfloat100;
        int b_myint200;
        int c_myint201;
        double d_myDoubleBubble202;
    };
}
