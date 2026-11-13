// This is the generated file

#pragma once

#include "streamable.h"
#include <stdint.h>

namespace testspace {
    class ClassyMcClassface : public skyloupe::Streamable {
    public:
        ClassyMcClassface() {}
        virtual ~ClassyMcClassface() {}
        bool serialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) const {
            if (str.mode() == skyloupe::Stream::PZ) {
                protozero::pbf_writer pbf{str.pz_buffer()};
                return serialize_pb(pbf);
            }
            char* p = str.claim(size());
            if (p == nullptr) return false;
            serialize_sw(p);
            return true;
        }
        // Writes the SW wire form, size() bytes, at p; returns the end.
        char* serialize_sw(char* p) const {
            p = skyloupe::wire::put(p, a_mydouble100, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, b_myuint32_200, skyloupe::Endianness::Big);
            p = skyloupe::wire::put(p, c_myint64_201, skyloupe::Endianness::Big);
            p = skyloupe::wire::put(p, d_myfloat202, skyloupe::Endianness::Big);
            p = skyloupe::wire::put(p, e_myint16_203, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, f_myuint16_204, skyloupe::Endianness::Big);
            p = skyloupe::wire::put(p, g_myint32_205, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, h_myuint64_206, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, i_charcoal301, skyloupe::Endianness::Little);
            p = skyloupe::wire::put_bytes(p, j_mystring302, 16);
            p = skyloupe::wire::put(p, k_uchar303, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, l_myuint8_304, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, m_myint8_305, skyloupe::Endianness::Little);
            p = skyloupe::wire::put_bytes(p, n_shortstring306, 4);
            p = skyloupe::wire::put(p, o_myfloat1000, skyloupe::Endianness::Native);
            p = skyloupe::wire::put(p, p_myuint32_2000, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, q_myint64_2010, skyloupe::Endianness::Native);
            p = skyloupe::wire::put(p, r_mydouble2020, skyloupe::Endianness::Big);
            return p;
        }
        bool deserialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) {
            if (str.mode() == skyloupe::Stream::PZ) {
                ClassyMcClassface decoded;
                try {
                    if (false == decoded.deserialize_pb(protozero::pbf_reader{str.pz_buffer()})) return false;
                } catch (const protozero::exception&) {
                    return false;
                }
                *this = decoded;
                str.clear();
                return true;
            }
            const char* p = str.consume(size());
            if (p == nullptr) return false;
            deserialize_sw(p);
            return true;
        }
        // Reads the SW wire form, size() bytes, from p; returns the end.
        const char* deserialize_sw(const char* p) {
            p = skyloupe::wire::get(p, a_mydouble100, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, b_myuint32_200, skyloupe::Endianness::Big);
            p = skyloupe::wire::get(p, c_myint64_201, skyloupe::Endianness::Big);
            p = skyloupe::wire::get(p, d_myfloat202, skyloupe::Endianness::Big);
            p = skyloupe::wire::get(p, e_myint16_203, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, f_myuint16_204, skyloupe::Endianness::Big);
            p = skyloupe::wire::get(p, g_myint32_205, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, h_myuint64_206, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, i_charcoal301, skyloupe::Endianness::Little);
            p = skyloupe::wire::get_bytes(p, j_mystring302, 16);
            p = skyloupe::wire::get(p, k_uchar303, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, l_myuint8_304, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, m_myint8_305, skyloupe::Endianness::Little);
            p = skyloupe::wire::get_bytes(p, n_shortstring306, 4);
            p = skyloupe::wire::get(p, o_myfloat1000, skyloupe::Endianness::Native);
            p = skyloupe::wire::get(p, p_myuint32_2000, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, q_myint64_2010, skyloupe::Endianness::Native);
            p = skyloupe::wire::get(p, r_mydouble2020, skyloupe::Endianness::Big);
            return p;
        }
        bool serialize_pb(protozero::pbf_writer& pbf) const {
            if (skyloupe::pb_nonzero(a_mydouble100)) pbf.add_double(100, a_mydouble100);
            if (skyloupe::pb_nonzero(b_myuint32_200)) pbf.add_uint32(200, b_myuint32_200);
            if (skyloupe::pb_nonzero(c_myint64_201)) pbf.add_int64(201, c_myint64_201);
            if (skyloupe::pb_nonzero(d_myfloat202)) pbf.add_float(202, d_myfloat202);
            if (skyloupe::pb_nonzero(e_myint16_203)) pbf.add_int32(203, e_myint16_203);
            if (skyloupe::pb_nonzero(f_myuint16_204)) pbf.add_uint32(204, f_myuint16_204);
            if (skyloupe::pb_nonzero(g_myint32_205)) pbf.add_int32(205, g_myint32_205);
            if (skyloupe::pb_nonzero(h_myuint64_206)) pbf.add_uint64(206, h_myuint64_206);
            if (skyloupe::pb_nonzero(i_charcoal301)) pbf.add_int32(301, i_charcoal301);
            if (size_t len = skyloupe::pb_bytes_length(j_mystring302, 16)) pbf.add_bytes(302, j_mystring302, len);
            if (skyloupe::pb_nonzero(k_uchar303)) pbf.add_uint32(303, k_uchar303);
            if (skyloupe::pb_nonzero(l_myuint8_304)) pbf.add_uint32(304, l_myuint8_304);
            if (skyloupe::pb_nonzero(m_myint8_305)) pbf.add_int32(305, m_myint8_305);
            if (size_t len = skyloupe::pb_bytes_length(n_shortstring306, 4)) pbf.add_bytes(306, n_shortstring306, len);
            if (skyloupe::pb_nonzero(o_myfloat1000)) pbf.add_float(1000, o_myfloat1000);
            if (skyloupe::pb_nonzero(p_myuint32_2000)) pbf.add_uint32(2000, p_myuint32_2000);
            if (skyloupe::pb_nonzero(q_myint64_2010)) pbf.add_int64(2010, q_myint64_2010);
            if (skyloupe::pb_nonzero(r_mydouble2020)) pbf.add_double(2020, r_mydouble2020);
            return true;
        }
        bool deserialize_pb(protozero::pbf_reader pbf) {
            while (pbf.next()) {
                switch (pbf.tag_and_type()) {
                case protozero::tag_and_type(100, protozero::pbf_wire_type::fixed64): a_mydouble100 = static_cast<double>(pbf.get_double()); break;
                case protozero::tag_and_type(200, protozero::pbf_wire_type::varint): b_myuint32_200 = static_cast<uint32_t>(pbf.get_uint32()); break;
                case protozero::tag_and_type(201, protozero::pbf_wire_type::varint): c_myint64_201 = static_cast<int64_t>(pbf.get_int64()); break;
                case protozero::tag_and_type(202, protozero::pbf_wire_type::fixed32): d_myfloat202 = static_cast<float>(pbf.get_float()); break;
                case protozero::tag_and_type(203, protozero::pbf_wire_type::varint): e_myint16_203 = static_cast<int16_t>(pbf.get_int32()); break;
                case protozero::tag_and_type(204, protozero::pbf_wire_type::varint): f_myuint16_204 = static_cast<uint16_t>(pbf.get_uint32()); break;
                case protozero::tag_and_type(205, protozero::pbf_wire_type::varint): g_myint32_205 = static_cast<int32_t>(pbf.get_int32()); break;
                case protozero::tag_and_type(206, protozero::pbf_wire_type::varint): h_myuint64_206 = static_cast<uint64_t>(pbf.get_uint64()); break;
                case protozero::tag_and_type(301, protozero::pbf_wire_type::varint): i_charcoal301 = static_cast<char>(pbf.get_int32()); break;
                case protozero::tag_and_type(302, protozero::pbf_wire_type::length_delimited): if (false == skyloupe::pb_read_bytes(pbf, j_mystring302, 16)) return false; break;
                case protozero::tag_and_type(303, protozero::pbf_wire_type::varint): k_uchar303 = static_cast<unsigned char>(pbf.get_uint32()); break;
                case protozero::tag_and_type(304, protozero::pbf_wire_type::varint): l_myuint8_304 = static_cast<uint8_t>(pbf.get_uint32()); break;
                case protozero::tag_and_type(305, protozero::pbf_wire_type::varint): m_myint8_305 = static_cast<int8_t>(pbf.get_int32()); break;
                case protozero::tag_and_type(306, protozero::pbf_wire_type::length_delimited): if (false == skyloupe::pb_read_bytes(pbf, n_shortstring306, 4)) return false; break;
                case protozero::tag_and_type(1000, protozero::pbf_wire_type::fixed32): o_myfloat1000 = static_cast<float>(pbf.get_float()); break;
                case protozero::tag_and_type(2000, protozero::pbf_wire_type::varint): p_myuint32_2000 = static_cast<uint32_t>(pbf.get_uint32()); break;
                case protozero::tag_and_type(2010, protozero::pbf_wire_type::varint): q_myint64_2010 = static_cast<int64_t>(pbf.get_int64()); break;
                case protozero::tag_and_type(2020, protozero::pbf_wire_type::fixed64): r_mydouble2020 = static_cast<double>(pbf.get_double()); break;
                default: pbf.skip(); break;
                }
            }
            return true;
        }
        static size_t size() {
            return(
                sizeof(double) +
                sizeof(uint32_t) +
                sizeof(int64_t) +
                sizeof(float) +
                sizeof(int16_t) +
                sizeof(uint16_t) +
                sizeof(int32_t) +
                sizeof(uint64_t) +
                sizeof(char) +
                16 +
                sizeof(unsigned char) +
                sizeof(uint8_t) +
                sizeof(int8_t) +
                4 +
                sizeof(float) +
                sizeof(uint32_t) +
                sizeof(int64_t) +
                sizeof(double)
            );
        }
        bool operator==(const ClassyMcClassface& other) const {
            if(this == &other) return true;
            return(
                a_mydouble100 == other.a_mydouble100 &&
                b_myuint32_200 == other.b_myuint32_200 &&
                c_myint64_201 == other.c_myint64_201 &&
                d_myfloat202 == other.d_myfloat202 &&
                e_myint16_203 == other.e_myint16_203 &&
                f_myuint16_204 == other.f_myuint16_204 &&
                g_myint32_205 == other.g_myint32_205 &&
                h_myuint64_206 == other.h_myuint64_206 &&
                i_charcoal301 == other.i_charcoal301 &&
                memcmp(j_mystring302, other.j_mystring302, 16) == 0 &&
                k_uchar303 == other.k_uchar303 &&
                l_myuint8_304 == other.l_myuint8_304 &&
                m_myint8_305 == other.m_myint8_305 &&
                memcmp(n_shortstring306, other.n_shortstring306, 4) == 0 &&
                o_myfloat1000 == other.o_myfloat1000 &&
                p_myuint32_2000 == other.p_myuint32_2000 &&
                q_myint64_2010 == other.q_myint64_2010 &&
                r_mydouble2020 == other.r_mydouble2020
            );
        }
        double Get_a_mydouble100() const { return a_mydouble100; }
        uint32_t Get_b_myuint32_200() const { return b_myuint32_200; }
        int64_t Get_c_myint64_201() const { return c_myint64_201; }
        float Get_d_myfloat202() const { return d_myfloat202; }
        int16_t Get_e_myint16_203() const { return e_myint16_203; }
        uint16_t Get_f_myuint16_204() const { return f_myuint16_204; }
        int32_t Get_g_myint32_205() const { return g_myint32_205; }
        uint64_t Get_h_myuint64_206() const { return h_myuint64_206; }
        char Get_i_charcoal301() const { return i_charcoal301; }
        const char* Get_j_mystring302() const { return j_mystring302; } // 16 bytes; not NUL-terminated when full
        unsigned char Get_k_uchar303() const { return k_uchar303; }
        uint8_t Get_l_myuint8_304() const { return l_myuint8_304; }
        int8_t Get_m_myint8_305() const { return m_myint8_305; }
        const char* Get_n_shortstring306() const { return n_shortstring306; } // 4 bytes; not NUL-terminated when full
        float Get_o_myfloat1000() const { return o_myfloat1000; }
        uint32_t Get_p_myuint32_2000() const { return p_myuint32_2000; }
        int64_t Get_q_myint64_2010() const { return q_myint64_2010; }
        double Get_r_mydouble2020() const { return r_mydouble2020; }
        void Set_a_mydouble100(const double newval) { a_mydouble100 = newval; }
        void Set_b_myuint32_200(const uint32_t newval) { b_myuint32_200 = newval; }
        void Set_c_myint64_201(const int64_t newval) { c_myint64_201 = newval; }
        void Set_d_myfloat202(const float newval) { d_myfloat202 = newval; }
        void Set_e_myint16_203(const int16_t newval) { e_myint16_203 = newval; }
        void Set_f_myuint16_204(const uint16_t newval) { f_myuint16_204 = newval; }
        void Set_g_myint32_205(const int32_t newval) { g_myint32_205 = newval; }
        void Set_h_myuint64_206(const uint64_t newval) { h_myuint64_206 = newval; }
        void Set_i_charcoal301(const char newval) { i_charcoal301 = newval; }
        void Set_j_mystring302(const char* newval) { strncpy(j_mystring302, newval, 16); }
        void Set_k_uchar303(const unsigned char newval) { k_uchar303 = newval; }
        void Set_l_myuint8_304(const uint8_t newval) { l_myuint8_304 = newval; }
        void Set_m_myint8_305(const int8_t newval) { m_myint8_305 = newval; }
        void Set_n_shortstring306(const char* newval) { strncpy(n_shortstring306, newval, 4); }
        void Set_o_myfloat1000(const float newval) { o_myfloat1000 = newval; }
        void Set_p_myuint32_2000(const uint32_t newval) { p_myuint32_2000 = newval; }
        void Set_q_myint64_2010(const int64_t newval) { q_myint64_2010 = newval; }
        void Set_r_mydouble2020(const double newval) { r_mydouble2020 = newval; }
    private:
        double a_mydouble100{};
        uint32_t b_myuint32_200{};
        int64_t c_myint64_201{};
        float d_myfloat202{};
        int16_t e_myint16_203{};
        uint16_t f_myuint16_204{};
        int32_t g_myint32_205{};
        uint64_t h_myuint64_206{};
        char i_charcoal301{};
        char j_mystring302[16]{};
        unsigned char k_uchar303{};
        uint8_t l_myuint8_304{};
        int8_t m_myint8_305{};
        char n_shortstring306[4]{};
        float o_myfloat1000{};
        uint32_t p_myuint32_2000{};
        int64_t q_myint64_2010{};
        double r_mydouble2020{};
    };
}
