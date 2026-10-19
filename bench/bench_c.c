//
//   Copyright (C) 2026 Skyloupe, LLC
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

// Times the generated C code for the ring message. See README.md.
//
//   bench_c RingyMcRingface.bin

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "RingyMcRingface.h"

#define ROUNDS 5
#define ITERS 2000000L

static volatile long sink;

static double now_ns(void)
{
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    return (double)t.tv_sec * 1e9 + (double)t.tv_nsec;
}

// Empties the stream without freeing its buffer.
static void reset(Cstream* s)
{
    s->head_ = s->tail_ = s->buffer_;
}

int main(int argc, char** argv)
{
    char bin[512];
    size_t n;
    FILE* f;
    Cstream s;
    RingyMcRingface m, m2;
    double best_ser = 1e30, best_de = 1e30;
    int r;
    long i;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s RingyMcRingface.bin\n", argv[0]);
        return 2;
    }
    f = fopen(argv[1], "rb");
    if (f == NULL)
    {
        fprintf(stderr, "cannot open %s\n", argv[1]);
        return 2;
    }
    n = fread(bin, 1, sizeof bin, f);
    fclose(f);

    if (cstream_init(&s, 4096) != 0)
    {
        return 1;
    }

    // The message must decode and re-encode to the same bytes.
    memset(&m, 0, sizeof m);
    memset(&m2, 0, sizeof m2);
    cstream_write(&s, bin, n, SW_NATIVE_ENDIAN_FLAG);
    if (RingyMcRingface_Deserialize(&m, &s) != 0 || cstream_available(&s) != 0)
    {
        fprintf(stderr, "deserialize failed\n");
        return 1;
    }
    reset(&s);
    RingyMcRingface_Serialize(&m, &s);
    if (cstream_available(&s) != n || memcmp(s.head_, bin, n) != 0)
    {
        fprintf(stderr, "serialize does not reproduce the input\n");
        return 1;
    }

    for (r = 0; r < ROUNDS; r++)
    {
        double t = now_ns();
        for (i = 0; i < ITERS; i++)
        {
            reset(&s);
            RingyMcRingface_Serialize(&m, &s);
            sink += s.tail_[-1];
        }
        t = (now_ns() - t) / ITERS;
        if (t < best_ser) best_ser = t;

        t = now_ns();
        for (i = 0; i < ITERS; i++)
        {
            reset(&s);
            cstream_write(&s, bin, n, SW_NATIVE_ENDIAN_FLAG);
            RingyMcRingface_Deserialize(&m2, &s);
            sink += m2.u8;
        }
        t = (now_ns() - t) / ITERS;
        if (t < best_de) best_de = t;
    }

    printf("RESULT serialize %.1f\n", best_ser);
    printf("RESULT deserialize %.1f\n", best_de);
    cstream_fini(&s);
    return 0;
}
