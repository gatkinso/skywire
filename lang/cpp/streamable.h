//
////  atkinet - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//

#pragma once

#include <stream.h>

namespace skyloupe
{
  class Streamable
  {
  public:
    Streamable() = delete;

    virtual bool serialize(const stream& str) const = 0;
    virtual bool deserialize(stream& str) = 0;

    virtual unsigned int size() = 0;

    virtual bool serialize_as_json() const { return false; };
  };
}


