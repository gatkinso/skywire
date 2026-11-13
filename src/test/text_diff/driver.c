/*
 *   Copyright (C) 2026 Geoffrey Atkinson
 *
 *   Licensed under the Apache License, Version 2.0 (the "License").
 *   You may not use this file except in compliance with the License.
 *   A copy of the License is located at
 *
 *   https://www.apache.org/licenses/LICENSE-2.0.html
 *
 *   or in the "license" file accompanying this file. This file is distributed
 *   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *   express or implied. See the License for the specific language governing
 *   permissions and limitations under the License.
 */

/*
 * The C driver for skywire_text_diff: prints what the generated C ring
 * message makes of each input, as oracle.cpp does for C++. It builds with the
 * project, from the C the fuzz targets generate.
 *
 *     text_diff_c_driver <inputs>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_text/RingyMcRingface.h"

static void line(char tag, const uint8_t* data, size_t n)
{
    static const char digits[] = "0123456789abcdef";
    size_t i;
    putchar(tag);
    if (data == NULL)
    {
        fputs(" -\n", stdout);
        return;
    }
    putchar(' ');
    for (i = 0; i < n; i++)
    {
        putchar(digits[data[i] >> 4]);
        putchar(digits[data[i] & 15]);
    }
    putchar('\n');
}

/* Writes what one of the To functions makes of m, sized by a first call
 * with no buffer (the snprintf contract), or "-" if read failed. */
#define WRITE_BACK(tag, ok, to, m, type)                                     \
    do                                                                       \
    {                                                                        \
        if (!(ok))                                                           \
        {                                                                    \
            line(tag, NULL, 0);                                              \
            break;                                                           \
        }                                                                    \
        {                                                                    \
            const size_t n = to(&(m), NULL, 0);                              \
            type* buf = (type*)malloc(n + 1);                                \
            if (buf == NULL || to(&(m), buf, n + 1) != n)                    \
            {                                                                \
                fprintf(stderr, #to " did not keep the snprintf contract\n"); \
                exit(1);                                                     \
            }                                                                \
            line(tag, (const uint8_t*)buf, n);                               \
            free(buf);                                                       \
        }                                                                    \
    } while (0)

int main(int argc, char* argv[])
{
    FILE* in;
    unsigned char prefix[4];
    if (argc != 2)
    {
        fprintf(stderr, "usage: text_diff_c_driver <inputs>\n");
        return 2;
    }
    in = fopen(argv[1], "rb");
    if (in == NULL)
    {
        perror(argv[1]);
        return 2;
    }
    while (fread(prefix, 1, 4, in) == 4)
    {
        const uint32_t n = prefix[0] | (prefix[1] << 8) | (prefix[2] << 16) | ((uint32_t)prefix[3] << 24);
        uint8_t* d = (uint8_t*)malloc(n ? n : 1);
        RingyMcRingface json, xml, bson;
        int ok;
        if (d == NULL || fread(d, 1, n, in) != n)
        {
            fprintf(stderr, "short input file\n");
            return 2;
        }
        memset(&json, 0, sizeof(json));
        memset(&xml, 0, sizeof(xml));
        memset(&bson, 0, sizeof(bson));

        ok = RingyMcRingface_FromJson(&json, (const char*)d, n) == 0;
        WRITE_BACK('J', ok, RingyMcRingface_ToJson, json, char);
        ok = RingyMcRingface_FromXml(&xml, (const char*)d, n) == 0;
        WRITE_BACK('X', ok, RingyMcRingface_ToXml, xml, char);
        ok = RingyMcRingface_FromBson(&bson, d, n) == 0;
        WRITE_BACK('B', ok, RingyMcRingface_ToBson, bson, uint8_t);
        free(d);
    }
    fclose(in);
    return 0;
}
