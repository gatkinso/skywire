//
//   Copyright (C) 2026 Geoffrey Atkinson
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

// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//   example_c telemetry.bin

#include <stdio.h>
#include <string.h>

#include "cstream.h"
#include "Telemetry.h"

/* Writes n bytes to path; 0 on success. */
static int write_file(const char* path, const void* data, size_t n)
{
    FILE* f = fopen(path, "wb");
    if (f == NULL)
    {
        return -1;
    }
    if (fwrite(data, 1, n, f) != n)
    {
        fclose(f);
        return -1;
    }
    return fclose(f) == 0 ? 0 : -1;
}

/* Reads up to cap bytes from path into data; the count, or 0 on failure. */
static size_t read_file(const char* path, void* data, size_t cap)
{
    size_t n;
    FILE* f = fopen(path, "rb");
    if (f == NULL)
    {
        return 0;
    }
    n = fread(data, 1, cap, f);
    fclose(f);
    return n;
}

int main(int argc, char** argv)
{
    Telemetry sent, received;
    Position position;
    Cstream stream;
    char bytes[64];
    size_t n;
    FILE* f;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s FILE\n", argv[0]);
        return 2;
    }

    // Start from zeros: the setters only change the member they name.
    memset(&sent, 0, sizeof sent);
    memset(&received, 0, sizeof received);
    memset(&position, 0, sizeof position);

    // The setters take pointers to their values.
    double latitude = 47.6062, longitude = -122.3321;
    float altitude_m = 56.5f;
    Set_Position_latitude(&position, &latitude);
    Set_Position_longitude(&position, &longitude);
    Set_Position_altitude_m(&position, &altitude_m);

    uint32_t device_id = 4207;
    uint64_t timestamp_us = 1760959800123456ULL;
    uint8_t mode = 5, status = 11, armed = 1; // status has 3 bits: 11 is stored as 3
    uint16_t battery_mv = 3870;
    Set_Telemetry_device_id(&sent, &device_id);
    Set_Telemetry_timestamp_us(&sent, &timestamp_us);
    Set_Telemetry_mode(&sent, &mode);
    Set_Telemetry_status(&sent, &status);
    Set_Telemetry_armed(&sent, &armed);
    Set_Telemetry_callsign(&sent, "SKY1"); // NUL-padded to 8 bytes
    Set_Telemetry_position(&sent, &position);
    Set_Telemetry_battery_mv(&sent, &battery_mv);

    // Serialize into a stream, then write the stream's bytes to the file.
    if (cstream_init(&stream, 0) != 0 || Telemetry_Serialize(&sent, &stream) != 0)
    {
        fprintf(stderr, "serialize failed\n");
        return 1;
    }
    n = cstream_available(&stream);
    if (n > sizeof bytes || cstream_read(&stream, bytes, n, SW_NATIVE_ENDIAN_FLAG) != 0)
    {
        fprintf(stderr, "reading the stream failed\n");
        return 1;
    }
    f = fopen(argv[1], "wb");
    if (f == NULL || fwrite(bytes, 1, n, f) != n || fclose(f) != 0)
    {
        fprintf(stderr, "cannot write %s\n", argv[1]);
        return 1;
    }

    // Read the file back into the stream and deserialize a second message.
    f = fopen(argv[1], "rb");
    if (f == NULL)
    {
        fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    n = fread(bytes, 1, sizeof bytes, f);
    fclose(f);
    if (cstream_write(&stream, bytes, n, SW_NATIVE_ENDIAN_FLAG) != 0 ||
        Telemetry_Deserialize(&received, &stream) != 0)
    {
        fprintf(stderr, "deserialize failed\n");
        return 1;
    }
    cstream_fini(&stream);

    if (Telemetry_Equals(&sent, &received) != 1)
    {
        fprintf(stderr, "the message read back is different\n");
        return 1;
    }

    printf("wrote and read back %zu bytes (Telemetry_Size() = %zu)\n", n, Telemetry_Size());
    printf("device_id %u, callsign %.8s, status %u, position %.4f, %.4f, %.1f m\n",
           (unsigned)received.device_id, received.callsign, (unsigned)received.status,
           received.position.latitude, received.position.longitude,
           (double)received.position.altitude_m);

    /* The same message as JSON, XML and BSON (docs/text-formats.md): each is
     * written beside the file, read back and checked. The To functions follow
     * snprintf (they return the length the whole form needs); a From function
     * returns 0, or -1, leaving the message unchanged, on input it refuses. */
    {
        char base[1024];
        char path[1040];
        char text[1024];
        uint8_t bson[512];
        Telemetry copy;
        size_t len = strlen(argv[1]);
        if (len >= sizeof base)
        {
            fprintf(stderr, "the file name is too long\n");
            return 1;
        }
        memcpy(base, argv[1], len + 1);
        if (len > 4 && strcmp(base + len - 4, ".bin") == 0)
        {
            base[len - 4] = '\0';
        }

        n = Telemetry_ToJson(&sent, text, sizeof text);
        snprintf(path, sizeof path, "%s.json", base);
        memset(&copy, 0, sizeof copy);
        if (n >= sizeof text || write_file(path, text, n) != 0 ||
            Telemetry_FromJson(&copy, text, read_file(path, text, sizeof text)) != 0 ||
            Telemetry_Equals(&sent, &copy) != 1)
        {
            fprintf(stderr, "the JSON copy is different\n");
            return 1;
        }

        n = Telemetry_ToXml(&sent, text, sizeof text);
        snprintf(path, sizeof path, "%s.xml", base);
        memset(&copy, 0, sizeof copy);
        if (n >= sizeof text || write_file(path, text, n) != 0 ||
            Telemetry_FromXml(&copy, text, read_file(path, text, sizeof text)) != 0 ||
            Telemetry_Equals(&sent, &copy) != 1)
        {
            fprintf(stderr, "the XML copy is different\n");
            return 1;
        }

        n = Telemetry_ToBson(&sent, bson, sizeof bson);
        snprintf(path, sizeof path, "%s.bson", base);
        memset(&copy, 0, sizeof copy);
        if (n > sizeof bson || write_file(path, bson, n) != 0 ||
            Telemetry_FromBson(&copy, bson, read_file(path, bson, sizeof bson)) != 0 ||
            Telemetry_Equals(&sent, &copy) != 1)
        {
            fprintf(stderr, "the BSON copy is different\n");
            return 1;
        }

        Telemetry_ToJson(&sent, text, sizeof text);
        printf("%s\n", text);
    }
    return 0;
}
