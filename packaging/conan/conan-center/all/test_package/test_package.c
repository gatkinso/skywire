#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Point.h"

int main(void)
{
    /* x big-endian (endian 1), y little-endian (endian 0) */
    static const unsigned char expected[4] = {0x12, 0x34, 0x78, 0x56};
    Point p;
    char wire[4];
    char json[64];
    uint16_t x = 0x1234, y = 0x5678;

    memset(&p, 0, sizeof p);
    Set_Point_x(&p, &x);
    Set_Point_y(&p, &y);

    Point_SerializeSw(&p, wire);
    Point_ToJson(&p, json, sizeof json);
    printf("%s\n", json);
    return memcmp(wire, expected, 4) != 0 || strcmp(json, "{\"x\":4660,\"y\":22136}") != 0;
}
