// This is the generated .c file

#include "Telemetry.h"

int Telemetry_Serialize(Telemetry * structure, Cstream * str) {
    char* p;
    if (NULL == structure || NULL == str) return -1;
    p = cstream_claim(str, Telemetry_Size());
    if (NULL == p) return -1;
    Telemetry_SerializeSw(structure, p);
    return 0;
}
// Writes the wire form, Telemetry_Size() bytes, at p; returns the end.
char* Telemetry_SerializeSw(const Telemetry * structure, char * p) {
    p = sw_put(p, &structure->device_id, sizeof(structure->device_id), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->timestamp_us, sizeof(structure->timestamp_us), SW_LITTLE_ENDIAN_FLAG);
    uint8_t unit_0 = 0;
    unit_0 |= (uint8_t)(((uint8_t)structure->mode & 0xfu) << 0);
    unit_0 |= (uint8_t)(((uint8_t)structure->status & 0x7u) << 4);
    unit_0 |= (uint8_t)(((uint8_t)structure->armed & 0x1u) << 7);
    p = sw_put(p, &unit_0, sizeof(unit_0), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->callsign, sizeof(structure->callsign), SW_NATIVE_ENDIAN_FLAG);
    p = Position_SerializeSw(&structure->position, p);
    p = sw_put(p, &structure->battery_mv, sizeof(structure->battery_mv), SW_LITTLE_ENDIAN_FLAG);
    return p;
}
int Telemetry_Deserialize(Telemetry * structure, Cstream * str) {
    const char* p;
    if (NULL == structure || NULL == str) return -1;
    p = cstream_consume(str, Telemetry_Size());
    if (NULL == p) return -1;
    Telemetry_DeserializeSw(structure, p);
    return 0;
}
// Reads the wire form, Telemetry_Size() bytes, from p; returns the end.
const char* Telemetry_DeserializeSw(Telemetry * structure, const char * p) {
    p = sw_get(p, &structure->device_id, sizeof(structure->device_id), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->timestamp_us, sizeof(structure->timestamp_us), SW_LITTLE_ENDIAN_FLAG);
    uint8_t unit_0;
    p = sw_get(p, &unit_0, sizeof(unit_0), SW_LITTLE_ENDIAN_FLAG);
    structure->mode = (uint8_t)((uint8_t)(unit_0 >> 0) & 0xfu);
    structure->status = (uint8_t)((uint8_t)(unit_0 >> 4) & 0x7u);
    structure->armed = (uint8_t)((uint8_t)(unit_0 >> 7) & 0x1u);
    p = sw_get(p, &structure->callsign, sizeof(structure->callsign), SW_NATIVE_ENDIAN_FLAG);
    p = Position_DeserializeSw(&structure->position, p);
    p = sw_get(p, &structure->battery_mv, sizeof(structure->battery_mv), SW_LITTLE_ENDIAN_FLAG);
    return p;
}
size_t Telemetry_Size() {
    return (
        sizeof(uint32_t)+
        sizeof(uint64_t)+
        sizeof(uint8_t)+
        8+
        Position_Size()+
        sizeof(uint16_t)
    );
}
int Telemetry_Equals(Telemetry * const left, Telemetry * const right) {
    if (NULL == left || NULL == right) return -1;
    if(
        left->device_id == right->device_id &&
        left->timestamp_us == right->timestamp_us &&
        left->mode == right->mode &&
        left->status == right->status &&
        left->armed == right->armed &&
        memcmp(left->callsign, right->callsign, 8) == 0 &&
        Position_Equals(&left->position, &right->position) == 1 &&
        left->battery_mv == right->battery_mv 
    )
        return 1;
    return 0;
}
int Get_Telemetry_device_id(Telemetry * structure, uint32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->device_id;
    return 0;
}
int Get_Telemetry_timestamp_us(Telemetry * structure, uint64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->timestamp_us;
    return 0;
}
int Get_Telemetry_mode(Telemetry * structure, uint8_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->mode;
    return 0;
}
int Get_Telemetry_status(Telemetry * structure, uint8_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->status;
    return 0;
}
int Get_Telemetry_armed(Telemetry * structure, uint8_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->armed;
    return 0;
}
int Get_Telemetry_callsign(Telemetry * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    memcpy(value, structure->callsign, 8);
    return 0;
}
int Get_Telemetry_position(Telemetry * structure, Position * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->position;
    return 0;
}
int Get_Telemetry_battery_mv(Telemetry * structure, uint16_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->battery_mv;
    return 0;
}
int Set_Telemetry_device_id(Telemetry * const structure, uint32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->device_id = *value;
    return 0;
}
int Set_Telemetry_timestamp_us(Telemetry * const structure, uint64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->timestamp_us = *value;
    return 0;
}
int Set_Telemetry_mode(Telemetry * const structure, uint8_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->mode = (uint8_t)((uint8_t)(*value) & 0xfu);
    return 0;
}
int Set_Telemetry_status(Telemetry * const structure, uint8_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->status = (uint8_t)((uint8_t)(*value) & 0x7u);
    return 0;
}
int Set_Telemetry_armed(Telemetry * const structure, uint8_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->armed = (uint8_t)((uint8_t)(*value) & 0x1u);
    return 0;
}
int Set_Telemetry_callsign(Telemetry * const structure, const char * value) {
    if (NULL == structure || NULL == value) return -1;
    strncpy(structure->callsign, value, 8);
    return 0;
}
int Set_Telemetry_position(Telemetry * const structure, Position * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->position = *value;
    return 0;
}
int Set_Telemetry_battery_mv(Telemetry * const structure, uint16_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->battery_mv = *value;
    return 0;
}
// The canonical JSON form into buf, NUL-terminated, at most cap bytes with the NUL; returns its length (snprintf's contract).
size_t Telemetry_ToJson(const Telemetry * structure, char * buf, size_t cap) {
    sw_json_writer w;
    sw_json_writer_init(&w, buf, cap);
    if (NULL != structure) Telemetry_WriteJson(structure, &w);
    return sw_out_text(&w.out);
}
void Telemetry_WriteJson(const Telemetry * structure, sw_json_writer * w) {
    sw_json_begin(w);
    sw_json_put_key(w, "device_id"); sw_json_put_uint32_t(w, structure->device_id);
    sw_json_put_key(w, "timestamp_us"); sw_json_put_uint64_t(w, structure->timestamp_us);
    sw_json_put_key(w, "mode"); sw_json_put_uint8_t(w, structure->mode);
    sw_json_put_key(w, "status"); sw_json_put_uint8_t(w, structure->status);
    sw_json_put_key(w, "armed"); sw_json_put_uint8_t(w, structure->armed);
    sw_json_put_key(w, "callsign"); sw_json_put_bytes(w, structure->callsign, 8);
    sw_json_put_key(w, "position"); Position_WriteJson(&structure->position, w);
    sw_json_put_key(w, "battery_mv"); sw_json_put_uint16_t(w, structure->battery_mv);
    sw_json_end(w);
}
// Reads the JSON form of len bytes at text: 0, or -1 leaving structure unchanged.
int Telemetry_FromJson(Telemetry * structure, const char * text, size_t len) {
    Telemetry decoded;
    sw_json p;
    if (NULL == structure || NULL == text) return -1;
    sw_json_init(&p, text, len);
    if (0 != Telemetry_ReadJson(&decoded, &p) || 0 != sw_json_finish(&p)) return -1;
    *structure = decoded;
    return 0;
}
int Telemetry_ReadJson(Telemetry * structure, sw_json * p) {
    Telemetry decoded;
    size_t key;
    int first = 1, r;
    memset(&decoded, 0, sizeof(decoded));
    if (0 != sw_json_object(p)) return -1;
    while ((r = sw_json_next_key(p, &first, &key)) > 0) {
        if (sw_json_key_is(p, key, "device_id")) { if (0 != sw_json_read_uint32_t(p, &decoded.device_id, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "timestamp_us")) { if (0 != sw_json_read_uint64_t(p, &decoded.timestamp_us, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "mode")) { if (0 != sw_json_read_uint8_t(p, &decoded.mode, 4)) return -1; continue; }
        if (sw_json_key_is(p, key, "status")) { if (0 != sw_json_read_uint8_t(p, &decoded.status, 3)) return -1; continue; }
        if (sw_json_key_is(p, key, "armed")) { if (0 != sw_json_read_uint8_t(p, &decoded.armed, 1)) return -1; continue; }
        if (sw_json_key_is(p, key, "callsign")) { if (0 != sw_json_read_bytes(p, decoded.callsign, 8)) return -1; continue; }
        if (sw_json_key_is(p, key, "position")) { if (0 != Position_ReadJson(&decoded.position, p)) return -1; continue; }
        if (sw_json_key_is(p, key, "battery_mv")) { if (0 != sw_json_read_uint16_t(p, &decoded.battery_mv, 0)) return -1; continue; }
        if (0 != sw_json_skip(p)) return -1;
    }
    if (r < 0) return -1;
    *structure = decoded;
    return 0;
}
// The canonical XML form into buf, NUL-terminated, at most cap bytes with the NUL; returns its length (snprintf's contract).
size_t Telemetry_ToXml(const Telemetry * structure, char * buf, size_t cap) {
    sw_out o;
    sw_out_init(&o, buf, cap);
    if (NULL != structure) Telemetry_WriteXml(structure, &o, "Telemetry");
    return sw_out_text(&o);
}
void Telemetry_WriteXml(const Telemetry * structure, sw_out * o, const char * element) {
    sw_xml_open(o, element);
    sw_xml_put_uint32_t(o, "device_id", structure->device_id);
    sw_xml_put_uint64_t(o, "timestamp_us", structure->timestamp_us);
    sw_xml_put_uint8_t(o, "mode", structure->mode);
    sw_xml_put_uint8_t(o, "status", structure->status);
    sw_xml_put_uint8_t(o, "armed", structure->armed);
    sw_xml_put_bytes(o, "callsign", structure->callsign, 8);
    Position_WriteXml(&structure->position, o, "position");
    sw_xml_put_uint16_t(o, "battery_mv", structure->battery_mv);
    sw_xml_close(o, element);
}
// Reads the XML form of len bytes at text: 0, or -1 leaving structure unchanged.
int Telemetry_FromXml(Telemetry * structure, const char * text, size_t len) {
    Telemetry decoded;
    sw_xml p;
    sw_xml_tag root;
    if (NULL == structure || NULL == text) return -1;
    if (0 != sw_xml_begin(&p, text, len, &root) || 0 == sw_xml_is(&p, &root, "Telemetry")) return -1;
    if (0 != Telemetry_ReadXml(&decoded, &p, &root) || 0 != sw_xml_finish(&p)) return -1;
    *structure = decoded;
    return 0;
}
int Telemetry_ReadXml(Telemetry * structure, sw_xml * p, const sw_xml_tag * tag) {
    Telemetry decoded;
    sw_xml_tag child;
    uint32_t cp;
    int r;
    memset(&decoded, 0, sizeof(decoded));
    while ((r = sw_xml_next(p, tag, &cp)) > 0) {
        if (r == 1) { if (0 == sw_xml_is_space(cp)) return -1; continue; }
        if (0 != sw_xml_start(p, &child)) return -1;
        if (sw_xml_is(p, &child, "device_id")) { if (0 != sw_xml_read_uint32_t(p, &child, &decoded.device_id, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "timestamp_us")) { if (0 != sw_xml_read_uint64_t(p, &child, &decoded.timestamp_us, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "mode")) { if (0 != sw_xml_read_uint8_t(p, &child, &decoded.mode, 4)) return -1; continue; }
        if (sw_xml_is(p, &child, "status")) { if (0 != sw_xml_read_uint8_t(p, &child, &decoded.status, 3)) return -1; continue; }
        if (sw_xml_is(p, &child, "armed")) { if (0 != sw_xml_read_uint8_t(p, &child, &decoded.armed, 1)) return -1; continue; }
        if (sw_xml_is(p, &child, "callsign")) { if (0 != sw_xml_read_bytes(p, &child, decoded.callsign, 8)) return -1; continue; }
        if (sw_xml_is(p, &child, "position")) { if (0 != Position_ReadXml(&decoded.position, p, &child)) return -1; continue; }
        if (sw_xml_is(p, &child, "battery_mv")) { if (0 != sw_xml_read_uint16_t(p, &child, &decoded.battery_mv, 0)) return -1; continue; }
        if (0 != sw_xml_skip(p, &child)) return -1;
    }
    if (r < 0) return -1;
    *structure = decoded;
    return 0;
}
// The BSON document into buf if it fits in cap bytes; returns its length either way.
size_t Telemetry_ToBson(const Telemetry * structure, uint8_t * buf, size_t cap) {
    sw_out o;
    sw_out_init(&o, buf, cap);
    if (NULL != structure) Telemetry_WriteBson(structure, &o);
    return o.len;
}
void Telemetry_WriteBson(const Telemetry * structure, sw_out * o) {
    const size_t start = sw_bson_begin(o);
    sw_bson_put_uint32_t(o, "device_id", structure->device_id);
    sw_bson_put_uint64_t(o, "timestamp_us", structure->timestamp_us);
    sw_bson_put_uint8_t(o, "mode", structure->mode);
    sw_bson_put_uint8_t(o, "status", structure->status);
    sw_bson_put_uint8_t(o, "armed", structure->armed);
    sw_bson_put_bytes(o, "callsign", structure->callsign, 8);
    sw_bson_document(o, "position"); Position_WriteBson(&structure->position, o);
    sw_bson_put_uint16_t(o, "battery_mv", structure->battery_mv);
    sw_bson_end(o, start);
}
// Reads a BSON document of len bytes: 0, or -1 leaving structure unchanged.
int Telemetry_FromBson(Telemetry * structure, const uint8_t * bytes, size_t len) {
    Telemetry decoded;
    sw_bson_doc doc;
    if (NULL == structure || 0 != sw_bson_parse(bytes, len, &doc)) return -1;
    if (0 != Telemetry_ReadBson(&decoded, &doc)) return -1;
    *structure = decoded;
    return 0;
}
int Telemetry_ReadBson(Telemetry * structure, const sw_bson_doc * doc) {
    Telemetry decoded;
    sw_bson_iter it;
    sw_bson_element e;
    memset(&decoded, 0, sizeof(decoded));
    sw_bson_iter_init(&it, doc);
    while (sw_bson_next(&it, &e)) {
        if (sw_bson_is(&e, "device_id")) { if (0 != sw_bson_read_uint32_t(&e, &decoded.device_id, 0)) return -1; continue; }
        if (sw_bson_is(&e, "timestamp_us")) { if (0 != sw_bson_read_uint64_t(&e, &decoded.timestamp_us, 0)) return -1; continue; }
        if (sw_bson_is(&e, "mode")) { if (0 != sw_bson_read_uint8_t(&e, &decoded.mode, 4)) return -1; continue; }
        if (sw_bson_is(&e, "status")) { if (0 != sw_bson_read_uint8_t(&e, &decoded.status, 3)) return -1; continue; }
        if (sw_bson_is(&e, "armed")) { if (0 != sw_bson_read_uint8_t(&e, &decoded.armed, 1)) return -1; continue; }
        if (sw_bson_is(&e, "callsign")) { if (0 != sw_bson_read_bytes(&e, decoded.callsign, 8)) return -1; continue; }
        if (sw_bson_is(&e, "position")) { if (e.type != SW_BSON_DOCUMENT || 0 != Position_ReadBson(&decoded.position, &e.doc)) return -1; continue; }
        if (sw_bson_is(&e, "battery_mv")) { if (0 != sw_bson_read_uint16_t(&e, &decoded.battery_mv, 0)) return -1; continue; }
    }
    *structure = decoded;
    return 0;
}
