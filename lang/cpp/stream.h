//
//   Copyright (C) 2018 Skyloupe, LLC
//
//   Licensed under the Apache License, Version 2.0 (the "License").
//   You may not use this file except in compliance with the License.
//   A copy of the License is located at
//
//   https://www.apache.org/licenses/LICENSE-2.0.html
//
//   or in the "license" file accompanying this file. This file is distributed
//   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
//   express or implied. See the License for the specific language governing
//   permissions and limitations under the License.
//

#pragma once

#include <stdint.h>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <string>
#include "sw_endian.h"
#include <protozero/pbf_writer.hpp>
#include <protozero/pbf_reader.hpp>

namespace skyloupe
{

enum Endianness : uint32_t { Big = 0, Little = 1, Native = 2 };

class Streamable;

class SWStream
{
public:
    SWStream(size_t size = 512, size_t expand_size = 512)
    :pos_(0)
    ,read_pos_(0)
    ,expand_size_(expand_size)
    {
        buffer_.resize(size);
    }

    void serialize(char d, Endianness)
    {
        write((char*)&d, sizeof(char));
    }

    // int8_t is signed char, a distinct type from char. Without this overload
    // it promotes to int and serializes as 4 bytes.
    void serialize(signed char d, Endianness)
    {
        write((char*)&d, sizeof(signed char));
    }

    void serialize(int16_t d, Endianness endian)
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

    void serialize(int32_t d, Endianness endian)
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

    void serialize(int64_t d, Endianness endian)
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

    void serialize(unsigned char d, Endianness)
    {
        write((char*)&d, sizeof(char));
    }

    void serialize(uint16_t d, Endianness endian)
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

    void serialize(uint32_t d, Endianness endian)
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

    void serialize(uint64_t d, Endianness endian)
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

    void serialize(float d, Endianness endian)
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

    void serialize(double d, Endianness endian)
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

    bool deserialize(char& d, Endianness)
    {
        if (read((char*)&d, sizeof(char)) != sizeof(char))
        {
            return false;
        }
        return true;
    }

    bool deserialize(signed char& d, Endianness)
    {
        if (read((char*)&d, sizeof(signed char)) != sizeof(signed char))
        {
            return false;
        }
        return true;
    }

    bool deserialize(int16_t& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(int16_t)) != sizeof(int16_t))
        {
            return false;
        }
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
        return true;
    }

    bool deserialize(int32_t& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(int32_t)) != sizeof(int32_t))
        {
            return false;
        }
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
        return true;
    }

    bool deserialize(int64_t& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(int64_t)) != sizeof(int64_t))
        {
            return false;
        }
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
        return true;
    }

    bool deserialize(unsigned char& d, Endianness)
    {
        if (read((char*)&d, sizeof(char)) != sizeof(char))
        {
            return false;
        }
        return true;
    }

    bool deserialize(uint16_t& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(uint16_t)) != sizeof(uint16_t))
        {
            return false;
        }
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
        return true;
    }

    bool deserialize(uint32_t& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(uint32_t)) != sizeof(uint32_t))
        {
            return false;
        }
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
        return true;
    }

    bool deserialize(uint64_t& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(uint64_t)) != sizeof(uint64_t))
        {
            return false;
        }
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
        return true;
    }

    bool deserialize(float& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(float)) != sizeof(float))
        {
            return false;
        }
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
        return true;
    }

    bool deserialize(double& d, Endianness endian)
    {
        if (read((char*)&d, sizeof(double)) != sizeof(double))
        {
            return false;
        }
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
        return true;
    }

    size_t write(const void* data, size_t len)
    {
        if (len == 0 || len > buffer_.max_size() - pos_)
        {
            return 0;
        }

        if (buffer_.size() < pos_ + len)
        {
            compact();
        }

        if (buffer_.size() < pos_ + len)
        {
            size_t grow = expand_size_;
            if (len > expand_size_)
            {
                grow = len + expand_size_;
            }
            buffer_.resize(pos_ + grow);
        }

        std::memcpy(buffer_.data() + pos_, data, len);
        pos_ += len;

        return len;
    }

    size_t read(void* data, size_t len)
    {
        if (len == 0 || pos_ - read_pos_ < len)
        {
            return 0;
        }

        std::memcpy(data, buffer_.data() + read_pos_, len);
        read_pos_ += len;

        // Fully drained: rewind so the buffer is reused from the start.
        if (read_pos_ == pos_)
        {
            pos_ = 0;
            read_pos_ = 0;
        }

        return len;
    }

    size_t size() const
    {
        return buffer_.size();
    }

    size_t byte_size() const
    {
        return pos_ - read_pos_;
    }

    void clear()
    {
        buffer_.clear();
        pos_ = 0;
        read_pos_ = 0;
    }

    const char* data() const
    {
        return buffer_.data() + read_pos_;
    }

    void set_expand_size(size_t expand_size)
    {
        if (expand_size == 0) return;

        expand_size_ = expand_size;
    }

    void reset_tag()
    {
    }

    void prepare()
    {

    }

protected:
    // Unread bytes are [read_pos_, pos_); pos_ is where the next write lands.
    size_t pos_;
    size_t read_pos_;
    size_t expand_size_;
    std::vector<char> buffer_;

    // Slide unread bytes to the front so consumed space is reused before growing.
    void compact()
    {
        if (read_pos_ == 0)
        {
            return;
        }

        std::memmove(buffer_.data(), buffer_.data() + read_pos_, pos_ - read_pos_);
        pos_ -= read_pos_;
        read_pos_ = 0;
    }
};

class PZStream
{
public:
    PZStream(std::string& buffer)
    :buffer_(buffer)
    {
        pos_ = 0;
        expand_size_ = 0;
    }

    // Holds a reference to its owner's buffer; a copy would alias it.
    PZStream(const PZStream&) = delete;
    PZStream& operator=(const PZStream&) = delete;

    // Each value is one protobuf field, numbered from 1 in call order.
    // Protobuf has no 8- or 16-bit scalars, so those travel as int32/uint32.
    void serialize(char d, Endianness)
    {
        writer_.add_int32(next_tag(), d);
    }

    void serialize(signed char d, Endianness)
    {
        writer_.add_int32(next_tag(), d);
    }

    void serialize(int16_t d, Endianness)
    {
        writer_.add_int32(next_tag(), d);
    }

    void serialize(int32_t d, Endianness)
    {
        writer_.add_int32(next_tag(), d);
    }

    void serialize(int64_t d, Endianness)
    {
        writer_.add_int64(next_tag(), d);
    }

    void serialize(unsigned char d, Endianness)
    {
        writer_.add_uint32(next_tag(), d);
    }

    void serialize(uint16_t d, Endianness)
    {
        writer_.add_uint32(next_tag(), d);
    }

    void serialize(uint32_t d, Endianness)
    {
        writer_.add_uint32(next_tag(), d);
    }

    void serialize(uint64_t d, Endianness)
    {
        writer_.add_uint64(next_tag(), d);
    }

    void serialize(float d, Endianness)
    {
        writer_.add_float(next_tag(), d);
    }

    void serialize(double d, Endianness)
    {
        writer_.add_double(next_tag(), d);
    }

    // Each returns false, leaving d unchanged, when no field is left to read.
    bool deserialize(char& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = static_cast<char>(reader_.get_int32());
        return true;
    }

    bool deserialize(signed char& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = static_cast<signed char>(reader_.get_int32());
        return true;
    }

    bool deserialize(int16_t& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = static_cast<int16_t>(reader_.get_int32());
        return true;
    }

    bool deserialize(int32_t& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = reader_.get_int32();
        return true;
    }

    bool deserialize(int64_t& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = reader_.get_int64();
        return true;
    }

    bool deserialize(unsigned char& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = static_cast<unsigned char>(reader_.get_uint32());
        return true;
    }

    bool deserialize(uint16_t& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = static_cast<uint16_t>(reader_.get_uint32());
        return true;
    }

    bool deserialize(uint32_t& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = reader_.get_uint32();
        return true;
    }

    bool deserialize(uint64_t& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = reader_.get_uint64();
        return true;
    }

    bool deserialize(float& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = reader_.get_float();
        return true;
    }

    bool deserialize(double& d, Endianness)
    {
        if (!reader_.next()) return false;
        d = reader_.get_double();
        return true;
    }

    // Raw bytes (fixed-length strings) travel as one bytes field.
    size_t write(const void* data, size_t len)
    {
        if (len == 0)
        {
            return 0;
        }

        writer_.add_bytes(next_tag(), static_cast<const char*>(data), len);
        return len;
    }

    // Reads one bytes field; it must be exactly len bytes long.
    size_t read(void* data, size_t len)
    {
        if (len == 0 || !reader_.next())
        {
            return 0;
        }

        protozero::data_view view = reader_.get_view();
        if (view.size() != len)
        {
            return 0;
        }

        std::memcpy(data, view.data(), len);
        return len;
    }

    size_t size() const
    {
        return buffer_.size();
    }

    size_t byte_size() const
    {
        return size();
    }

    void clear()
    {
        buffer_.clear();
        pos_ = 0;
        reader_ = protozero::pbf_reader{buffer_};
    }

    const char* data() const
    {
        return buffer_.data();
    }

    void set_expand_size(size_t expand_size)
    {
        if (expand_size == 0) return;

        expand_size_ = expand_size;
    }

    void reset_tag()
    {
        current_tag_ = 1;
    }

    void prepare()
    {
        reader_ = protozero::pbf_reader{buffer_};
        pos_ = 0;
    }

protected:
    // Field numbers run 1, 2, 3... in call order, skipping the range protobuf
    // reserves (19000-19999). Throws std::out_of_range past the largest one.
    protozero::pbf_tag_type next_tag()
    {
        if (current_tag_ > max_field_number)
        {
            throw std::out_of_range("PZStream: too many fields for one protobuf message");
        }

        protozero::pbf_tag_type tag = current_tag_++;
        if (current_tag_ == first_reserved_field_number)
        {
            current_tag_ = last_reserved_field_number + 1;
        }
        return tag;
    }

    static constexpr protozero::pbf_tag_type max_field_number = (1u << 29) - 1;
    static constexpr protozero::pbf_tag_type first_reserved_field_number = 19000;
    static constexpr protozero::pbf_tag_type last_reserved_field_number = 19999;

    size_t pos_;
    size_t expand_size_;
    std::string& buffer_;

    protozero::pbf_tag_type current_tag_ = 1;
    protozero::pbf_writer writer_{buffer_};
    protozero::pbf_reader reader_{buffer_};
};

class Stream {
public:
    typedef enum mode { SW, PZ } modetype;

    Stream(size_t size = 512, size_t expand_size = 512, modetype mode = SW)
    :mode_(mode)
    ,swstream_(size, expand_size)
    ,pzstream_(buffer_){
    }
    ~Stream() {}
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    bool serialize(char d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(signed char d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(int16_t d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(int32_t d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(int64_t d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(unsigned char d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(uint16_t d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(uint32_t d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(uint64_t d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(float d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool serialize(double d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            swstream_.serialize(d, endian);
        } else {
            pzstream_.serialize(d, endian);
        }
        return true;
    }
    bool deserialize(char& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(signed char& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(int16_t& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(int32_t& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(int64_t& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(unsigned char& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(uint16_t& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(uint32_t& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(uint64_t& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(float& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }
    bool deserialize(double& d, Endianness endian = Endianness::Big) {
        if (mode_ == SW) {
            return swstream_.deserialize(d, endian);
        } else {
            return pzstream_.deserialize(d, endian);
        }
    }

    size_t write(const void* data, size_t len)
    {
        if (mode_ == SW) {
            return swstream_.write(data, len);
        } else {
            return pzstream_.write(data, len);
        }
    }

    size_t read(void* data, size_t len)
    {
        if (mode_ == SW) {
            return swstream_.read(data, len);
        } else {
            return pzstream_.read(data, len);
        }
    }

    size_t size() const {
        if (mode_ == SW) {
            return swstream_.size();
        } else {
            return pzstream_.size();
        }
    }
    // True if len more bytes can be read. PZ messages are variable-length, so
    // the check only applies to the fixed-size SW wire format.
    bool can_read(size_t len) const {
        return mode_ != SW || swstream_.byte_size() >= len;
    }
    size_t byte_size() const {
        if (mode_ == SW) {
            return swstream_.byte_size();
        } else {
            return pzstream_.byte_size();
        }
    }
    void clear() {
        if (mode_ == SW) {
            swstream_.clear();
        } else {
            pzstream_.clear();
        }
    }
    const char* data() const {
        if (mode_ == SW) {
            return swstream_.data();
        } else {
            return pzstream_.data();
        }
    }
    void set_expand_size(size_t expand_size) {
        if (mode_ == SW) {
            swstream_.set_expand_size(expand_size);
        } else {
            pzstream_.set_expand_size(expand_size);
        }
    }
    void reset_tag() {
        if (mode_ == SW) {
            swstream_.reset_tag();
        } else {
            pzstream_.reset_tag();
        }
    }

    void prepare() {
        if (mode_ == PZ) {
            pzstream_.prepare();
        }
    }

private:
    // buffer_ must be declared before pzstream_, which binds to it on construction.
    modetype mode_ = SW;
    std::string buffer_;
    SWStream swstream_;
    PZStream pzstream_;
};

} // namespace skyloupe
