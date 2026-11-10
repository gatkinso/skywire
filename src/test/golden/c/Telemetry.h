// This is the generated .h file

#ifndef _Telemetry_h
#define _Telemetry_h

#include "cstream.h"
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

#ifdef __cplusplus
}
#endif

    #endif //_Telemetry_h
