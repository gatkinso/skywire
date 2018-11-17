//
////  skyloupe - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//

#pragma once

#include <vector>

namespace skyloupe
{

class Streamable;

class Stream
{
public:
    Stream(std::vector<char>& buffer, size_t size, size_t expand_size = 512)
    :expand_size_(expand_size)
    ,pos_(0)
    ,buffer_(buffer)
    {
        buffer_.resize(size);
    }

    void serialize(char d)
    {
        write((char*)&d, sizeof(char));
    }

    void serialize(short d)
    {
        write((char*)&d, sizeof(short));
    }

    void serialize(int d)
    {
        write((char*)&d, sizeof(int));
    }

    void serialize(long d)
    {
        write((char*)&d, sizeof(long));
    }

    void serialize(unsigned char d)
    {
        write((char*)&d, sizeof(char));
    }

    void serialize(unsigned short d)
    {
        write((char*)&d, sizeof(short));
    }

    void serialize(unsigned int d)
    {
        write((char*)&d, sizeof(int));
    }

    void serialize(unsigned long d)
    {
        write((char*)&d, sizeof(long));
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

    void deserialize(short& d)
    {
        read((char*)&d, sizeof(short));
    }

    void deserialize(int& d)
    {
        read((char*)&d, sizeof(int));
    }

    void deserialize(long& d)
    {
        read((char*)&d, sizeof(int));
    }

    void deserialize(unsigned char& d)
    {
        read((char*)&d, sizeof(char));
    }

    void deserialize(unsigned short& d)
    {
        read((char*)&d, sizeof(short));
    }

    void deserialize(unsigned int& d)
    {
        read((char*)&d, sizeof(int));
    }

    void deserialize(unsigned long& d)
    {
        read((char*)&d, sizeof(long));
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
    std::vector<char>& buffer_;
};

}

