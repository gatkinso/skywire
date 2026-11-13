// This is the generated file

#pragma once

#include "streamable.h"
#include <stdint.h>
#include "text.h"
#include "bson.h"

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
        std::string to_json() const {
            skyloupe::text::JsonWriter w;
            to_json(w);
            return w.take();
        }
        void to_json(skyloupe::text::JsonWriter& w) const {
            w.begin_object();
            w.key("a_mydouble100"); w.value(this->a_mydouble100);
            w.key("b_myuint32_200"); w.value(this->b_myuint32_200);
            w.key("c_myint64_201"); w.value(this->c_myint64_201);
            w.key("d_myfloat202"); w.value(this->d_myfloat202);
            w.key("e_myint16_203"); w.value(this->e_myint16_203);
            w.key("f_myuint16_204"); w.value(this->f_myuint16_204);
            w.key("g_myint32_205"); w.value(this->g_myint32_205);
            w.key("h_myuint64_206"); w.value(this->h_myuint64_206);
            w.key("i_charcoal301"); w.value(this->i_charcoal301);
            w.key("j_mystring302"); w.bytes(this->j_mystring302, 16);
            w.key("k_uchar303"); w.value(this->k_uchar303);
            w.key("l_myuint8_304"); w.value(this->l_myuint8_304);
            w.key("m_myint8_305"); w.value(this->m_myint8_305);
            w.key("n_shortstring306"); w.bytes(this->n_shortstring306, 4);
            w.key("o_myfloat1000"); w.value(this->o_myfloat1000);
            w.key("p_myuint32_2000"); w.value(this->p_myuint32_2000);
            w.key("q_myint64_2010"); w.value(this->q_myint64_2010);
            w.key("r_mydouble2020"); w.value(this->r_mydouble2020);
            w.end_object();
        }
        bool from_json(std::string_view text) {
            skyloupe::text::JsonValue doc;
            if (false == skyloupe::text::parse_json(text, doc)) return false;
            return from_json(doc);
        }
        bool from_json(const skyloupe::text::JsonValue& value) {
            if (value.kind != skyloupe::text::JsonValue::Object) return false;
            ClassyMcClassface decoded;
            for (const auto& field : value.members) {
                if (field.name == "a_mydouble100") { if (false == skyloupe::text::read(field.value, decoded.a_mydouble100)) return false; continue; }
                if (field.name == "b_myuint32_200") { if (false == skyloupe::text::read(field.value, decoded.b_myuint32_200)) return false; continue; }
                if (field.name == "c_myint64_201") { if (false == skyloupe::text::read(field.value, decoded.c_myint64_201)) return false; continue; }
                if (field.name == "d_myfloat202") { if (false == skyloupe::text::read(field.value, decoded.d_myfloat202)) return false; continue; }
                if (field.name == "e_myint16_203") { if (false == skyloupe::text::read(field.value, decoded.e_myint16_203)) return false; continue; }
                if (field.name == "f_myuint16_204") { if (false == skyloupe::text::read(field.value, decoded.f_myuint16_204)) return false; continue; }
                if (field.name == "g_myint32_205") { if (false == skyloupe::text::read(field.value, decoded.g_myint32_205)) return false; continue; }
                if (field.name == "h_myuint64_206") { if (false == skyloupe::text::read(field.value, decoded.h_myuint64_206)) return false; continue; }
                if (field.name == "i_charcoal301") { if (false == skyloupe::text::read(field.value, decoded.i_charcoal301)) return false; continue; }
                if (field.name == "j_mystring302") { if (false == skyloupe::text::read_bytes(field.value, decoded.j_mystring302, 16)) return false; continue; }
                if (field.name == "k_uchar303") { if (false == skyloupe::text::read(field.value, decoded.k_uchar303)) return false; continue; }
                if (field.name == "l_myuint8_304") { if (false == skyloupe::text::read(field.value, decoded.l_myuint8_304)) return false; continue; }
                if (field.name == "m_myint8_305") { if (false == skyloupe::text::read(field.value, decoded.m_myint8_305)) return false; continue; }
                if (field.name == "n_shortstring306") { if (false == skyloupe::text::read_bytes(field.value, decoded.n_shortstring306, 4)) return false; continue; }
                if (field.name == "o_myfloat1000") { if (false == skyloupe::text::read(field.value, decoded.o_myfloat1000)) return false; continue; }
                if (field.name == "p_myuint32_2000") { if (false == skyloupe::text::read(field.value, decoded.p_myuint32_2000)) return false; continue; }
                if (field.name == "q_myint64_2010") { if (false == skyloupe::text::read(field.value, decoded.q_myint64_2010)) return false; continue; }
                if (field.name == "r_mydouble2020") { if (false == skyloupe::text::read(field.value, decoded.r_mydouble2020)) return false; continue; }
            }
            *this = decoded;
            return true;
        }
        std::string to_xml() const {
            skyloupe::text::XmlWriter w;
            to_xml(w, "ClassyMcClassface");
            return w.take();
        }
        void to_xml(skyloupe::text::XmlWriter& w, std::string_view element) const {
            w.open(element);
            w.element("a_mydouble100", this->a_mydouble100);
            w.element("b_myuint32_200", this->b_myuint32_200);
            w.element("c_myint64_201", this->c_myint64_201);
            w.element("d_myfloat202", this->d_myfloat202);
            w.element("e_myint16_203", this->e_myint16_203);
            w.element("f_myuint16_204", this->f_myuint16_204);
            w.element("g_myint32_205", this->g_myint32_205);
            w.element("h_myuint64_206", this->h_myuint64_206);
            w.element("i_charcoal301", this->i_charcoal301);
            w.bytes_element("j_mystring302", this->j_mystring302, 16);
            w.element("k_uchar303", this->k_uchar303);
            w.element("l_myuint8_304", this->l_myuint8_304);
            w.element("m_myint8_305", this->m_myint8_305);
            w.bytes_element("n_shortstring306", this->n_shortstring306, 4);
            w.element("o_myfloat1000", this->o_myfloat1000);
            w.element("p_myuint32_2000", this->p_myuint32_2000);
            w.element("q_myint64_2010", this->q_myint64_2010);
            w.element("r_mydouble2020", this->r_mydouble2020);
            w.close(element);
        }
        bool from_xml(std::string_view text) {
            skyloupe::text::XmlNode doc;
            if (false == skyloupe::text::parse_xml(text, doc) || doc.name != "ClassyMcClassface") return false;
            return from_xml(doc);
        }
        bool from_xml(const skyloupe::text::XmlNode& node) {
            if (false == skyloupe::text::is_blank(node)) return false;
            ClassyMcClassface decoded;
            for (const auto& child : node.children) {
                if (child.name == "a_mydouble100") { if (false == skyloupe::text::read(child, decoded.a_mydouble100)) return false; continue; }
                if (child.name == "b_myuint32_200") { if (false == skyloupe::text::read(child, decoded.b_myuint32_200)) return false; continue; }
                if (child.name == "c_myint64_201") { if (false == skyloupe::text::read(child, decoded.c_myint64_201)) return false; continue; }
                if (child.name == "d_myfloat202") { if (false == skyloupe::text::read(child, decoded.d_myfloat202)) return false; continue; }
                if (child.name == "e_myint16_203") { if (false == skyloupe::text::read(child, decoded.e_myint16_203)) return false; continue; }
                if (child.name == "f_myuint16_204") { if (false == skyloupe::text::read(child, decoded.f_myuint16_204)) return false; continue; }
                if (child.name == "g_myint32_205") { if (false == skyloupe::text::read(child, decoded.g_myint32_205)) return false; continue; }
                if (child.name == "h_myuint64_206") { if (false == skyloupe::text::read(child, decoded.h_myuint64_206)) return false; continue; }
                if (child.name == "i_charcoal301") { if (false == skyloupe::text::read(child, decoded.i_charcoal301)) return false; continue; }
                if (child.name == "j_mystring302") { if (false == skyloupe::text::read_bytes(child, decoded.j_mystring302, 16)) return false; continue; }
                if (child.name == "k_uchar303") { if (false == skyloupe::text::read(child, decoded.k_uchar303)) return false; continue; }
                if (child.name == "l_myuint8_304") { if (false == skyloupe::text::read(child, decoded.l_myuint8_304)) return false; continue; }
                if (child.name == "m_myint8_305") { if (false == skyloupe::text::read(child, decoded.m_myint8_305)) return false; continue; }
                if (child.name == "n_shortstring306") { if (false == skyloupe::text::read_bytes(child, decoded.n_shortstring306, 4)) return false; continue; }
                if (child.name == "o_myfloat1000") { if (false == skyloupe::text::read(child, decoded.o_myfloat1000)) return false; continue; }
                if (child.name == "p_myuint32_2000") { if (false == skyloupe::text::read(child, decoded.p_myuint32_2000)) return false; continue; }
                if (child.name == "q_myint64_2010") { if (false == skyloupe::text::read(child, decoded.q_myint64_2010)) return false; continue; }
                if (child.name == "r_mydouble2020") { if (false == skyloupe::text::read(child, decoded.r_mydouble2020)) return false; continue; }
            }
            *this = decoded;
            return true;
        }
        std::string to_bson() const {
            skyloupe::bson::Writer w;
            to_bson(w);
            return w.take();
        }
        void to_bson(skyloupe::bson::Writer& w) const {
            w.begin_document();
            w.value("a_mydouble100", this->a_mydouble100);
            w.value("b_myuint32_200", this->b_myuint32_200);
            w.value("c_myint64_201", this->c_myint64_201);
            w.value("d_myfloat202", this->d_myfloat202);
            w.value("e_myint16_203", this->e_myint16_203);
            w.value("f_myuint16_204", this->f_myuint16_204);
            w.value("g_myint32_205", this->g_myint32_205);
            w.value("h_myuint64_206", this->h_myuint64_206);
            w.value("i_charcoal301", this->i_charcoal301);
            w.bytes("j_mystring302", this->j_mystring302, 16);
            w.value("k_uchar303", this->k_uchar303);
            w.value("l_myuint8_304", this->l_myuint8_304);
            w.value("m_myint8_305", this->m_myint8_305);
            w.bytes("n_shortstring306", this->n_shortstring306, 4);
            w.value("o_myfloat1000", this->o_myfloat1000);
            w.value("p_myuint32_2000", this->p_myuint32_2000);
            w.value("q_myint64_2010", this->q_myint64_2010);
            w.value("r_mydouble2020", this->r_mydouble2020);
            w.end_document();
        }
        bool from_bson(std::string_view bytes) {
            skyloupe::bson::Document doc;
            if (false == skyloupe::bson::parse(bytes, doc)) return false;
            return from_bson(doc);
        }
        bool from_bson(const skyloupe::bson::Document& doc) {
            ClassyMcClassface decoded;
            for (const auto& field : doc.elements) {
                if (field.name == "a_mydouble100") { if (false == skyloupe::bson::read(field, decoded.a_mydouble100)) return false; continue; }
                if (field.name == "b_myuint32_200") { if (false == skyloupe::bson::read(field, decoded.b_myuint32_200)) return false; continue; }
                if (field.name == "c_myint64_201") { if (false == skyloupe::bson::read(field, decoded.c_myint64_201)) return false; continue; }
                if (field.name == "d_myfloat202") { if (false == skyloupe::bson::read(field, decoded.d_myfloat202)) return false; continue; }
                if (field.name == "e_myint16_203") { if (false == skyloupe::bson::read(field, decoded.e_myint16_203)) return false; continue; }
                if (field.name == "f_myuint16_204") { if (false == skyloupe::bson::read(field, decoded.f_myuint16_204)) return false; continue; }
                if (field.name == "g_myint32_205") { if (false == skyloupe::bson::read(field, decoded.g_myint32_205)) return false; continue; }
                if (field.name == "h_myuint64_206") { if (false == skyloupe::bson::read(field, decoded.h_myuint64_206)) return false; continue; }
                if (field.name == "i_charcoal301") { if (false == skyloupe::bson::read(field, decoded.i_charcoal301)) return false; continue; }
                if (field.name == "j_mystring302") { if (false == skyloupe::bson::read_bytes(field, decoded.j_mystring302, 16)) return false; continue; }
                if (field.name == "k_uchar303") { if (false == skyloupe::bson::read(field, decoded.k_uchar303)) return false; continue; }
                if (field.name == "l_myuint8_304") { if (false == skyloupe::bson::read(field, decoded.l_myuint8_304)) return false; continue; }
                if (field.name == "m_myint8_305") { if (false == skyloupe::bson::read(field, decoded.m_myint8_305)) return false; continue; }
                if (field.name == "n_shortstring306") { if (false == skyloupe::bson::read_bytes(field, decoded.n_shortstring306, 4)) return false; continue; }
                if (field.name == "o_myfloat1000") { if (false == skyloupe::bson::read(field, decoded.o_myfloat1000)) return false; continue; }
                if (field.name == "p_myuint32_2000") { if (false == skyloupe::bson::read(field, decoded.p_myuint32_2000)) return false; continue; }
                if (field.name == "q_myint64_2010") { if (false == skyloupe::bson::read(field, decoded.q_myint64_2010)) return false; continue; }
                if (field.name == "r_mydouble2020") { if (false == skyloupe::bson::read(field, decoded.r_mydouble2020)) return false; continue; }
            }
            *this = decoded;
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
