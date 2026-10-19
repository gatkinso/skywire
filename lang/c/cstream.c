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

#include <stdint.h>
#include "cstream.h"

void sw_swap16(char* d)
{
    uint16_t v;
    if (!d) return;

    memcpy(&v, d, sizeof(v));
    v = SW_BSWAP16(v);
    memcpy(d, &v, sizeof(v));
}

void sw_swap32(char* d)
{
    uint32_t v;
    if (!d) return;

    memcpy(&v, d, sizeof(v));
    v = SW_BSWAP32(v);
    memcpy(d, &v, sizeof(v));
}

void sw_swap64(char* d)
{
    uint64_t v;
    if (!d) return;

    memcpy(&v, d, sizeof(v));
    v = SW_BSWAP64(v);
    memcpy(d, &v, sizeof(v));
}

int cstream_init(Cstream* streamer, size_t size)
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

void cstream_fini(Cstream* streamer)
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

// Make room for sz more bytes after tail_: first reclaim space already read,
// then grow. On failure the stream is left unchanged.
static int cstream_reserve(Cstream* streamer, size_t sz)
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

char* cstream_claim(Cstream* streamer, size_t sz)
{
    char* p = NULL;

    if (streamer == NULL || sz == 0 || cstream_reserve(streamer, sz) != 0)
    {
        return NULL;
    }

    p = streamer->tail_;
    streamer->tail_ += sz;
    return p;
}

const char* cstream_consume(Cstream* streamer, size_t sz)
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

int cstream_write(Cstream* streamer, const void* d, size_t sz, unsigned int endian)
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

size_t cstream_available(const Cstream* streamer)
{
    if (streamer == NULL || streamer->head_ == NULL)
    {
        return 0;
    }

    return (size_t)(streamer->tail_ - streamer->head_);
}

int cstream_read(Cstream* streamer, void* d, size_t sz, unsigned int endian)
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
