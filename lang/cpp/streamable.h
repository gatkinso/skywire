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
    virtual bool serialize(skyloupe::Stream& str, Endianness endian = Big) const = 0;
    virtual bool deserialize(skyloupe::Stream& str, Endianness endian = Big) = 0;
};

}


