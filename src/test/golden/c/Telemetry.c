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
