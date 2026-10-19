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

// Fixed-width stores and loads in a given byte order, used by generated
// serialize_sw()/deserialize_sw() once the whole message has been claimed or
// consumed. Nothing here checks bounds. Each returns the pointer just past
// the bytes it handled; with a constant byte order the swap folds away.
namespace wire
{

template <size_t N> struct uint_of;
template <> struct uint_of<1> { typedef uint8_t type; };
template <> struct uint_of<2> { typedef uint16_t type; };
template <> struct uint_of<4> { typedef uint32_t type; };
template <> struct uint_of<8> { typedef uint64_t type; };

inline uint8_t bswap(uint8_t v) { return v; }
inline uint16_t bswap(uint16_t v) { return SW_BSWAP16(v); }
inline uint32_t bswap(uint32_t v) { return SW_BSWAP32(v); }
inline uint64_t bswap(uint64_t v) { return SW_BSWAP64(v); }

// Converts between host order and endian; the same operation both ways.
template <typename U>
inline U order(U v, Endianness endian)
{
    if (SW_HOST_LITTLE_ENDIAN)
    {
        return endian == Big ? bswap(v) : v;
    }
    return endian == Little ? bswap(v) : v;
}

template <typename T>
inline char* put(char* p, T value, Endianness endian)
{
    typedef typename uint_of<sizeof(T)>::type U;
    U u;
    std::memcpy(&u, &value, sizeof(U));
    u = order(u, endian);
    std::memcpy(p, &u, sizeof(U));
    return p + sizeof(U);
}

template <typename T>
inline const char* get(const char* p, T& value, Endianness endian)
{
    typedef typename uint_of<sizeof(T)>::type U;
    U u;
    std::memcpy(&u, p, sizeof(U));
    u = order(u, endian);
    std::memcpy(&value, &u, sizeof(U));
    return p + sizeof(U);
}

// Fixed-length strings (char[N]) travel verbatim.
inline char* put_bytes(char* p, const char* data, size_t len)
{
    std::memcpy(p, data, len);
    return p + len;
}

inline const char* get_bytes(const char* p, char* data, size_t len)
{
    std::memcpy(data, p, len);
    return p + len;
}

} // namespace wire

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

    void serialize(char d, Endianness endian) { put_value(d, endian); }
    // int8_t is signed char, a distinct type from char. Without this overload
    // it promotes to int and serializes as 4 bytes.
    void serialize(signed char d, Endianness endian) { put_value(d, endian); }
    void serialize(unsigned char d, Endianness endian) { put_value(d, endian); }
    void serialize(int16_t d, Endianness endian) { put_value(d, endian); }
    void serialize(int32_t d, Endianness endian) { put_value(d, endian); }
    void serialize(int64_t d, Endianness endian) { put_value(d, endian); }
    void serialize(uint16_t d, Endianness endian) { put_value(d, endian); }
    void serialize(uint32_t d, Endianness endian) { put_value(d, endian); }
    void serialize(uint64_t d, Endianness endian) { put_value(d, endian); }
    void serialize(float d, Endianness endian) { put_value(d, endian); }
    void serialize(double d, Endianness endian) { put_value(d, endian); }

    // Each returns false, consuming nothing, if too few bytes are unread.
    bool deserialize(char& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(signed char& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(unsigned char& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(int16_t& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(int32_t& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(int64_t& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(uint16_t& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(uint32_t& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(uint64_t& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(float& d, Endianness endian) { return get_value(d, endian); }
    bool deserialize(double& d, Endianness endian) { return get_value(d, endian); }

    // Reserves len bytes at the end of the stream and returns where to write
    // them; they count as written. Space already read is reused before the
    // buffer grows. Returns nullptr (nothing reserved) if len is 0 or too large.
    char* claim(size_t len)
    {
        if (len == 0 || len > buffer_.max_size() - pos_)
        {
            return nullptr;
        }

        if (buffer_.size() - pos_ < len)
        {
            grow(len);
        }

        char* p = buffer_.data() + pos_;
        pos_ += len;
        return p;
    }

    // Consumes the next len unread bytes and returns where they start, or
    // nullptr (nothing consumed) if fewer are unread or len is 0. The bytes
    // stay valid until the next write.
    const char* consume(size_t len)
    {
        if (len == 0 || pos_ - read_pos_ < len)
        {
            return nullptr;
        }

        const char* p = buffer_.data() + read_pos_;
        read_pos_ += len;

        // Fully drained: rewind so the buffer is reused from the start.
        if (read_pos_ == pos_)
        {
            pos_ = 0;
            read_pos_ = 0;
        }

        return p;
    }

    size_t write(const void* data, size_t len)
    {
        char* p = claim(len);
        if (p == nullptr)
        {
            return 0;
        }

        std::memcpy(p, data, len);
        return len;
    }

    size_t read(void* data, size_t len)
    {
        const char* p = consume(len);
        if (p == nullptr)
        {
            return 0;
        }

        std::memcpy(data, p, len);
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

    // Drops the unread bytes. The buffer keeps its size, so writes after a
    // clear() reuse it without reallocating or zero-filling.
    void clear()
    {
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

    template <typename T>
    void put_value(T d, Endianness endian)
    {
        if (char* p = claim(sizeof(T)))
        {
            wire::put(p, d, endian);
        }
    }

    template <typename T>
    bool get_value(T& d, Endianness endian)
    {
        const char* p = consume(sizeof(T));
        if (p == nullptr)
        {
            return false;
        }
        wire::get(p, d, endian);
        return true;
    }

    // Makes room for len more bytes at pos_: first by sliding the unread
    // bytes to the front, then by growing the buffer. Out of line, so claim()
    // stays small enough to inline.
#if defined(_MSC_VER)
    __declspec(noinline)
#else
    __attribute__((noinline))
#endif
    void grow(size_t len)
    {
        compact();

        if (buffer_.size() - pos_ < len)
        {
            size_t extra = expand_size_;
            if (len > expand_size_)
            {
                extra = len + expand_size_;
            }
            buffer_.resize(pos_ + extra);
        }
    }

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

// Helpers for the protobuf (PZ) encoding of generated messages.

// proto3 leaves out fields holding their zero value. Floating point values
// compare by bits, so -0.0 is still sent.
template <typename T>
inline bool pb_nonzero(T value) { return value != 0; }

inline bool pb_nonzero(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits != 0;
}

inline bool pb_nonzero(double value)
{
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits != 0;
}

// A fixed-length string travels as bytes without its trailing NULs, so a
// protobuf peer sees "abc", not "abc\0\0\0".
inline size_t pb_bytes_length(const char* data, size_t len)
{
    while (len > 0 && data[len - 1] == '\0')
    {
        len--;
    }
    return len;
}

// Reads a bytes field into a fixed-length string, zero-padding a shorter
// value. A longer one does not fit and fails.
inline bool pb_read_bytes(protozero::pbf_reader& reader, char* data, size_t len)
{
    protozero::data_view view = reader.get_view();
    if (view.size() > len)
    {
        return false;
    }

    std::memset(data, 0, len);
    if (view.size() > 0)
    {
        std::memcpy(data, view.data(), view.size());
    }
    return true;
}

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

    // SW mode only (nullptr in PZ mode): see SWStream::claim and consume.
    // Generated serialize()/deserialize() use them to handle a whole message
    // at once.
    char* claim(size_t len) {
        return mode_ == SW ? swstream_.claim(len) : nullptr;
    }
    const char* consume(size_t len) {
        return mode_ == SW ? swstream_.consume(len) : nullptr;
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

    modetype mode() const {
        return mode_;
    }

    // The protobuf bytes of a PZ-mode stream. Generated messages encode into
    // and decode from it directly, using their member tags as field numbers.
    std::string& pz_buffer() {
        return buffer_;
    }

private:
    // buffer_ must be declared before pzstream_, which binds to it on construction.
    modetype mode_ = SW;
    std::string buffer_;
    SWStream swstream_;
    PZStream pzstream_;
};

} // namespace skyloupe
