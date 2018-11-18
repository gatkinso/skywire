//
////  skyloupe - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//

#pragma once

#include "stdint.h"
#include <vector>

namespace skyloupe
{

class Streamable;

class Stream
{
public:
    Stream(size_t size = 512, size_t expand_size = 512)
    :expand_size_(expand_size)
    ,pos_(0)
    {
        buffer_.resize(size);
    }

    void serialize(char d)
    {
        write((char*)&d, sizeof(char));
    }

    void serialize(int16_t d)
    {
        write((char*)&d, sizeof(int16_t));
    }

    void serialize(int32_t d)
    {
        write((char*)&d, sizeof(int32_t));
    }

    void serialize(int64_t d)
    {
        write((char*)&d, sizeof(int64_t));
    }

    void serialize(unsigned char d)
    {
        write((char*)&d, sizeof(char));
    }

    void serialize(uint16_t d)
    {
        write((char*)&d, sizeof(uint16_t));
    }

    void serialize(uint32_t d)
    {
        write((char*)&d, sizeof(uint32_t));
    }

    void serialize(uint64_t d)
    {
        write((char*)&d, sizeof(uint64_t));
    }

    void serialize(float d)
    {
        write((char*)&d, sizeof(float));
    }

    void serialize(double d)
    {
        write((char*)&d, sizeof(double));
    }

    void deserialize(char& d)
    {
        read((char*)&d, sizeof(char));
    }

    void deserialize(int16_t& d)
    {
        read((char*)&d, sizeof(int16_t));
    }

    void deserialize(int32_t& d)
    {
        read((char*)&d, sizeof(int32_t));
    }

    void deserialize(int64_t& d)
    {
        read((char*)&d, sizeof(int64_t));
    }

    void deserialize(unsigned char& d)
    {
        read((char*)&d, sizeof(char));
    }

    void deserialize(uint16_t& d)
    {
        read((char*)&d, sizeof(uint16_t));
    }

    void deserialize(uint32_t& d)
    {
        read((char*)&d, sizeof(uint32_t));
    }

    void deserialize(uint64_t& d)
    {
        read((char*)&d, sizeof(uint64_t));
    }

    void deserialize(float& d)
    {
        read((char*)&d, sizeof(float));
    }

    void deserialize(double& d)
    {
        read((char*)&d, sizeof(double));
    }

    size_t write(const void* data, size_t len)
    {
        if(buffer_.size() < pos_+len)
        {
            buffer_.resize(buffer_.size()+ expand_size_);
        }

        std::memcpy(buffer_.data()+pos_, data, len);
        pos_ += len;

        return len;
    }

    size_t read(void* data, size_t len)
    {
        if(len == 0 || buffer_.size() < len)
        {
            return 0;
        }

        std::memcpy(data, buffer_.data(), len);
        buffer_.erase(buffer_.begin(), buffer_.begin() + len);

        return len;
    }

    size_t size() const
    {
        return buffer_.size();
    }

    void clear()
    {
        buffer_.clear();
    }

    protected:
    size_t pos_;
    size_t expand_size_;
    std::vector<char> buffer_;
};

}

