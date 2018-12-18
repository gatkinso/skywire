//
////  Atkinet - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//

#pragma once

#include <stdint.h>
#include <vector>
#include <cstring>
#include "endian.h"

namespace skyloupe
{

enum Endianness : uint32_t { Big = 0, Little = 1, Native = 2 };

class Streamable;

class Stream
{
public:
    Stream(size_t size = 512, size_t expand_size = 512)
    :pos_(0)
    ,expand_size_(expand_size)
    {
        buffer_.resize(size);
    }

    void serialize(char d, Endianness = Big)
    {
        write((char*)&d, sizeof(char));
    }

    void serialize(int16_t d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = htobe16(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = htone16(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = htole16(d);
            break;
        };
        write((char*)&d, sizeof(int16_t));
    }

    void serialize(int32_t d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = htobe32(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = htone32(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = htole32(d);
            break;
        };
        write((char*)&d, sizeof(int32_t));
    }

    void serialize(int64_t d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = htobe64(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = htone64(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = htole64(d);
            break;
        };
        write((char*)&d, sizeof(int64_t));
    }

    void serialize(unsigned char d, Endianness = Big)
    {
        write((char*)&d, sizeof(char));
    }

    void serialize(uint16_t d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = htobe16(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = htone16(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = htole16(d);
            break;
        };
        write((char*)&d, sizeof(uint16_t));
    }

    void serialize(uint32_t d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = htobe32(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = htone32(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = htole32(d);
            break;
        };
        write((char*)&d, sizeof(uint32_t));
    }

    void serialize(uint64_t d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = htobe64(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = htone64(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = htole64(d);
            break;
        };
        write((char*)&d, sizeof(uint64_t));
    }

    void serialize(float d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            hftobef(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            hftonef(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            hftolef(d);
            break;
        };
        write((char*)&d, sizeof(float));
    }

    void serialize(double d, Endianness endian = Big)
    {
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            hdtobed(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            hdtoned(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            hdtoled(d);
            break;
        };
        write((char*)&d, sizeof(double));
    }

    void deserialize(char& d, Endianness = Big)
    {
        read((char*)&d, sizeof(char));
    }

    void deserialize(int16_t& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(int16_t));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = be16toh(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = ne16toh(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = le16toh(d);
            break;
        };
    }

    void deserialize(int32_t& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(int32_t));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = be32toh(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = ne32toh(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = le32toh(d);
            break;
        };
    }

    void deserialize(int64_t& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(int64_t));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = be64toh(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = ne64toh(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = le64toh(d);
            break;
        };
    }

    void deserialize(unsigned char& d, Endianness = Big)
    {
        read((char*)&d, sizeof(char));
    }

    void deserialize(uint16_t& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(uint16_t));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = be16toh(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = ne16toh(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = le16toh(d);
            break;
        };
    }

    void deserialize(uint32_t& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(uint32_t));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = be32toh(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = ne32toh(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = le32toh(d);
            break;
        };
    }

    void deserialize(uint64_t& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(uint64_t));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            d = be64toh(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            d = ne64toh(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            d = le64toh(d);
            break;
        };
    }

    void deserialize(float& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(float));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            beftohf(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            neftohf(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            leftohf(d);
            break;
        };
    }

    void deserialize(double& d, Endianness endian = Big)
    {
        read((char*)&d, sizeof(double));
        switch (endian)
        {
        case SW_BIG_ENDIAN_FLAG:
            bedtohd(d);
            break;
        case SW_NATIVE_ENDIAN_FLAG:
            nedtohd(d);
            break;
        case SW_LITTLE_ENDIAN_FLAG:
        default:
            ledtohd(d);
            break;
        };
    }

    size_t write(const void* data, size_t len)
    {
        if(buffer_.size() < pos_ + len)
        {
            size_t grow = expand_size_;
            if (len > expand_size_)
            {
                grow = len + expand_size_;
            }
            buffer_.resize(buffer_.size() + grow);
        }

        std::memcpy(buffer_.data() + pos_, data, len);
        pos_ += len;

        return len;
    }

    size_t read(void* data, size_t len)
    {
        if(len == 0 || buffer_.size() < len || pos_ < len)
        {
            return 0;
        }

        std::memcpy(data, buffer_.data(), len);
        buffer_.erase(buffer_.begin(), buffer_.begin() + len);

        pos_ -= len;

        return len;
    }

    size_t size() const
    {
        return buffer_.size();
    }

    size_t byte_size() const
    {
        return pos_;
    }

    void clear()
    {
        buffer_.clear();
    }

    void set_expand_size(size_t expand_size)
    {
        if (expand_size_ == 0) return;

        expand_size_ = expand_size;
    }

protected:
    size_t pos_;
    size_t expand_size_;
    std::vector<char> buffer_;
};

}

