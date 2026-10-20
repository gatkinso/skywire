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

// The C runtime: header-only. Every function is static inline, so a program
// just includes this header; there is nothing to compile or link.

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

// Stores and loads of one value in a given byte order, for code that has
// already claimed or consumed the space; nothing is bounds-checked. sz is the
// value's size: 2, 4 and 8 byte values are swapped when endian is not the
// host's order, anything else (bytes, char[N]) is copied as is. Each returns
// the pointer just past the value. With constant sz and endian, as in
// generated code, each compiles to a load or store and at most one swap.

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

    // Returns 0 on success, -1 if streamer is NULL or the allocation fails.
    // A size of 0 uses DEF_CSTREAM_SIZE.
    SW_INLINE int cstream_init(Cstream* streamer, size_t size)
    {
        if (streamer == NULL)
        {
            return -1;
        }

        if (size == 0)
        {
            size = DEF_CSTREAM_SIZE;
        }

        streamer->buffer_ = (char*)malloc(size);
        streamer->size_ = (streamer->buffer_ != NULL) ? size : 0;
        streamer->head_ = streamer->buffer_;
        streamer->tail_ = streamer->buffer_;

        return (streamer->buffer_ != NULL) ? 0 : -1;
    }

    // Frees the buffer; safe to call on a NULL or already-finished stream.
    SW_INLINE void cstream_fini(Cstream* streamer)
    {
        if (streamer == NULL)
        {
            return;
        }

        free(streamer->buffer_);
        streamer->buffer_ = NULL;
        streamer->size_ = 0;
        streamer->head_ = NULL;
        streamer->tail_ = NULL;
    }

    // Number of unread bytes.
    SW_INLINE size_t cstream_available(const Cstream* streamer)
    {
        if (streamer == NULL || streamer->head_ == NULL)
        {
            return 0;
        }

        return (size_t)(streamer->tail_ - streamer->head_);
    }

    // Makes room for sz more bytes after tail_: first reclaims space already
    // read, then grows. On failure the stream is left unchanged. Used by
    // cstream_claim; not part of the interface.
    SW_INLINE int cstream_reserve_(Cstream* streamer, size_t sz)
    {
        size_t unread = 0;
        size_t needed = 0;
        size_t newsize = 0;
        char* newbuffer = NULL;

        if (streamer->buffer_ == NULL && cstream_init(streamer, DEF_CSTREAM_SIZE) != 0)
        {
            return -1;
        }

        if ((size_t)(streamer->buffer_ + streamer->size_ - streamer->tail_) >= sz)
        {
            return 0;
        }

        unread = (size_t)(streamer->tail_ - streamer->head_);
        if (sz > SIZE_MAX - unread)
        {
            return -1;
        }
        needed = unread + sz;

        if (streamer->head_ != streamer->buffer_)
        {
            memmove(streamer->buffer_, streamer->head_, unread);
            streamer->head_ = streamer->buffer_;
            streamer->tail_ = streamer->buffer_ + unread;
        }

        if (streamer->size_ >= needed)
        {
            return 0;
        }

        newsize = (streamer->size_ <= SIZE_MAX / 2) ? streamer->size_ * 2 : SIZE_MAX;
        if (newsize < needed)
        {
            newsize = needed;
        }

        newbuffer = (char*)realloc(streamer->buffer_, newsize);
        if (newbuffer == NULL)
        {
            return -1;
        }

        streamer->buffer_ = newbuffer;
        streamer->size_ = newsize;
        streamer->head_ = newbuffer;
        streamer->tail_ = newbuffer + unread;
        return 0;
    }

    // Reserves sz bytes after the unread ones and returns where to write
    // them; they count as written. Space already read is reused before the
    // buffer grows. Returns NULL (stream unchanged) on a NULL stream, a sz of
    // 0 or a failed allocation. Generated _Serialize functions claim a whole
    // message at once.
    SW_INLINE char* cstream_claim(Cstream* streamer, size_t sz)
    {
        char* p = NULL;

        if (streamer == NULL || sz == 0 || cstream_reserve_(streamer, sz) != 0)
        {
            return NULL;
        }

        p = streamer->tail_;
        streamer->tail_ += sz;
        return p;
    }

    // Consumes the next sz unread bytes and returns where they start, or NULL
    // (nothing consumed) on a NULL stream, a sz of 0 or fewer than sz unread
    // bytes. The bytes stay valid until the next write.
    SW_INLINE const char* cstream_consume(Cstream* streamer, size_t sz)
    {
        const char* p = NULL;

        if (sz == 0 || cstream_available(streamer) < sz)
        {
            return NULL;
        }

        p = streamer->head_;
        streamer->head_ += sz;
        return p;
    }

    // Returns 0 on success, -1 on a NULL argument or failed allocation (the
    // stream is left unchanged). Space already read is reused before growing.
    SW_INLINE int cstream_write(Cstream* streamer, const void* d, size_t sz, unsigned int endian)
    {
        char* p = NULL;

        if (sz == 0)
        {
            return 0;
        }

        if (d == NULL || (p = cstream_claim(streamer, sz)) == NULL)
        {
            return -1;
        }

        sw_put(p, d, sz, endian);
        return 0;
    }

    // Returns 0 on success, -1 if fewer than sz bytes are unread (nothing is consumed).
    SW_INLINE int cstream_read(Cstream* streamer, void* d, size_t sz, unsigned int endian)
    {
        const char* p = NULL;

        if (sz == 0)
        {
            return 0;
        }

        if (d == NULL || (p = cstream_consume(streamer, sz)) == NULL)
        {
            return -1;
        }

        sw_get(p, d, sz, endian);
        return 0;
    }

#ifdef __cplusplus
}
#endif

#endif //__wiregen_stream_h
