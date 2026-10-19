//
//   Copyright (C) 2018 Skyloupe, LLC
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

#ifndef __wiregen_stream_h
#define __wiregen_stream_h

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include "sw_endian.h"

#define DEF_CSTREAM_SIZE 1024

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct CStreamTag {
        char* buffer_;
        size_t size_;
        char *head_, *tail_;
    } Cstream;

    // Returns 0 on success, -1 if streamer is NULL or the allocation fails.
    // A size of 0 uses DEF_CSTREAM_SIZE.
    int cstream_init(Cstream* streamer, size_t size);

    // Frees the buffer; safe to call on a NULL or already-finished stream.
    void cstream_fini(Cstream* streamer);

    // Returns 0 on success, -1 on a NULL argument or failed allocation (the
    // stream is left unchanged). Space already read is reused before growing.
    int cstream_write(Cstream* streamer, const void* d, size_t sz, unsigned int endian);

    // Returns 0 on success, -1 if fewer than sz bytes are unread (nothing is consumed).
    int cstream_read(Cstream* streamer, void* d, size_t sz, unsigned int endian);

    // Number of unread bytes.
    size_t cstream_available(const Cstream* streamer);

    // Reserves sz bytes after the unread ones and returns where to write
    // them; they count as written. Space already read is reused before the
    // buffer grows. Returns NULL (stream unchanged) on a NULL stream, a sz of
    // 0 or a failed allocation. Generated _Serialize functions claim a whole
    // message at once.
    char* cstream_claim(Cstream* streamer, size_t sz);

    // Consumes the next sz unread bytes and returns where they start, or NULL
    // (nothing consumed) on a NULL stream, a sz of 0 or fewer than sz unread
    // bytes. The bytes stay valid until the next write.
    const char* cstream_consume(Cstream* streamer, size_t sz);

// Stores and loads of one value in a given byte order, for code that has
// already claimed or consumed the space; nothing is bounds-checked. sz is the
// value's size: 2, 4 and 8 byte values are swapped when endian is not the
// host's order, anything else (bytes, char[N]) is copied as is. Each returns
// the pointer just past the value. With constant sz and endian, as in
// generated code, each compiles to a load or store and at most one swap.
#if defined(_MSC_VER) && !defined(__cplusplus)
    #define SW_INLINE static __inline
#else
    #define SW_INLINE static inline
#endif

    SW_INLINE int sw_needs_swap(unsigned int endian)
    {
        return endian != SW_NATIVE_ENDIAN_FLAG &&
               endian != (SW_HOST_LITTLE_ENDIAN ? SW_LITTLE_ENDIAN_FLAG : SW_BIG_ENDIAN_FLAG);
    }

    SW_INLINE char* sw_put(char* p, const void* d, size_t sz, unsigned int endian)
    {
        if (sw_needs_swap(endian))
        {
            switch (sz)
            {
            case 2: { uint16_t v; memcpy(&v, d, 2); v = SW_BSWAP16(v); memcpy(p, &v, 2); return p + 2; }
            case 4: { uint32_t v; memcpy(&v, d, 4); v = SW_BSWAP32(v); memcpy(p, &v, 4); return p + 4; }
            case 8: { uint64_t v; memcpy(&v, d, 8); v = SW_BSWAP64(v); memcpy(p, &v, 8); return p + 8; }
            default: break;
            }
        }
        memcpy(p, d, sz);
        return p + sz;
    }

    SW_INLINE const char* sw_get(const char* p, void* d, size_t sz, unsigned int endian)
    {
        if (sw_needs_swap(endian))
        {
            switch (sz)
            {
            case 2: { uint16_t v; memcpy(&v, p, 2); v = SW_BSWAP16(v); memcpy(d, &v, 2); return p + 2; }
            case 4: { uint32_t v; memcpy(&v, p, 4); v = SW_BSWAP32(v); memcpy(d, &v, 4); return p + 4; }
            case 8: { uint64_t v; memcpy(&v, p, 8); v = SW_BSWAP64(v); memcpy(d, &v, 8); return p + 8; }
            default: break;
            }
        }
        memcpy(d, p, sz);
        return p + sz;
    }

#ifdef __cplusplus
}
#endif

#endif //__wiregen_stream_h


