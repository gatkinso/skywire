// This is the generated .h file

#ifndef _Telemetry_h
#define _Telemetry_h

#include "cstream.h"
#include "ctext.h"
#include "cbson.h"
#include "Position.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TelemetryTag {
        uint32_t device_id;
        uint64_t timestamp_us;
        uint8_t mode;
        uint8_t status;
        uint8_t armed;
        char callsign[8];
        Position position;
        uint16_t battery_mv;
    } Telemetry;

    int Telemetry_Serialize(Telemetry * structure, Cstream * str);
    char* Telemetry_SerializeSw(const Telemetry * structure, char * p);
    int Telemetry_Deserialize(Telemetry * structure, Cstream * str);
    const char* Telemetry_DeserializeSw(Telemetry * structure, const char * p);
    size_t Telemetry_Size();
    int Telemetry_Equals(Telemetry * const left, Telemetry * const right);
    int Get_Telemetry_device_id(Telemetry * structure, uint32_t * value);
    int Get_Telemetry_timestamp_us(Telemetry * structure, uint64_t * value);
    int Get_Telemetry_mode(Telemetry * structure, uint8_t * value);
    int Get_Telemetry_status(Telemetry * structure, uint8_t * value);
    int Get_Telemetry_armed(Telemetry * structure, uint8_t * value);
    int Get_Telemetry_callsign(Telemetry * structure, char * value);
    int Get_Telemetry_position(Telemetry * structure, Position * value);
    int Get_Telemetry_battery_mv(Telemetry * structure, uint16_t * value);
    int Set_Telemetry_device_id(Telemetry * const structure, uint32_t * const value);
    int Set_Telemetry_timestamp_us(Telemetry * const structure, uint64_t * const value);
    int Set_Telemetry_mode(Telemetry * const structure, uint8_t * const value);
    int Set_Telemetry_status(Telemetry * const structure, uint8_t * const value);
    int Set_Telemetry_armed(Telemetry * const structure, uint8_t * const value);
    int Set_Telemetry_callsign(Telemetry * const structure, const char * value);
    int Set_Telemetry_position(Telemetry * const structure, Position * const value);
    int Set_Telemetry_battery_mv(Telemetry * const structure, uint16_t * const value);
    size_t Telemetry_ToJson(const Telemetry * structure, char * buf, size_t cap);
    int Telemetry_FromJson(Telemetry * structure, const char * text, size_t len);
    void Telemetry_WriteJson(const Telemetry * structure, sw_json_writer * w);
    int Telemetry_ReadJson(Telemetry * structure, sw_json * p);
    size_t Telemetry_ToXml(const Telemetry * structure, char * buf, size_t cap);
    int Telemetry_FromXml(Telemetry * structure, const char * text, size_t len);
    void Telemetry_WriteXml(const Telemetry * structure, sw_out * o, const char * element);
    int Telemetry_ReadXml(Telemetry * structure, sw_xml * p, const sw_xml_tag * tag);
    size_t Telemetry_ToBson(const Telemetry * structure, uint8_t * buf, size_t cap);
    int Telemetry_FromBson(Telemetry * structure, const uint8_t * bytes, size_t len);
    void Telemetry_WriteBson(const Telemetry * structure, sw_out * o);
    int Telemetry_ReadBson(Telemetry * structure, const sw_bson_doc * doc);

#ifdef __cplusplus
}
#endif

    #endif //_Telemetry_h
