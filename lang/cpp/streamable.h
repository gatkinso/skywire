//
////  atkinet - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//

#pragma once

#include "stream.h"

namespace skyloupe
{

class Streamable
{
public:
    virtual bool serialize(skyloupe::Stream& str) const = 0;
    virtual bool deserialize(skyloupe::Stream& str) = 0;
    virtual size_t size() const = 0;
};

}


