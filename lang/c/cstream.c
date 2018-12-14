//
////  atkinet - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//
#include <stdio.h>
#include "cstream.h"
#include "endian.h"

void __swap(char* l, char* r)
{
    char t = *l;
    *l = *r;
    *r = t;
}

void __swap16(char* d)
{
    if (!d) return;

    __swap(&d[0], &d[1]);
}

void __swap32(char* d)
{
    if (!d) return;

    __swap(&d[0], &d[3]);
    __swap(&d[1], &d[2]);
}

void __swap64(char* d)
{
    if (!d) return;

    __swap(&d[0], &d[7]);
    __swap(&d[1], &d[6]);
    __swap(&d[2], &d[5]);
    __swap(&d[3], &d[4]);
}

void cstream_init(Cstream* streamer, size_t size)
{
    streamer->buffer_ = (char*)malloc(size);
    streamer->size_ = size;
    streamer->head_ = streamer->buffer_;
    streamer->tail_ = streamer->head_;
}

void cstream_fini(Cstream* streamer)
{
    if (streamer != NULL)
        free(streamer->buffer_);

    streamer->size_ = 0;
    streamer->head_ = NULL;
    streamer->tail_ = NULL;
}

void cstream_write(Cstream* streamer, const void* d, size_t sz, unsigned int endian)
{
    size_t tsize = 0;
    size_t hsize = 0;
    size_t grow = 0;

    if (sz == 0)
    {
        return;
    }

    if (streamer == NULL)
    {
        cstream_init(streamer, DEF_CSTREAM_SIZE);
        return;
    }

    if (streamer->tail_ + sz > streamer->buffer_ + streamer->size_)
    {
        hsize = streamer->head_ - streamer->buffer_;
        tsize = streamer->tail_ - streamer->buffer_;

        grow = streamer->size_ * 2;
        if (sz >= grow)
        {
            grow = sz * 2;
        }

        streamer->size_ = streamer->size_ + grow;
        char* newbuffer = (char*)realloc(streamer->buffer_, streamer->size_);
        streamer->buffer_ = newbuffer;
        streamer->head_ = streamer->buffer_ + hsize;
        streamer->tail_ = streamer->buffer_ + tsize;
    }

    unsigned int system_endian = SW_NATIVE_ENDIAN_FLAG;
#if SW_LITTLE_ENDIAN
    system_endian = SW_LITTLE_ENDIAN_FLAG;
#else
    system_endian = SW_BIG_ENDIAN_FLAG;
#endif
    char buf[8];
    memcpy(buf, d, sz);
    if (endian != SW_NATIVE_ENDIAN_FLAG && system_endian != endian)
    {
        switch (sz) 
        {
        case 2:
            __swap16(buf);
            break;
        case 4:
            __swap32(buf);
            break;
        case 8:
            __swap64(buf);
            break;
        default:
            break;
        }
    }

    memcpy(streamer->tail_, buf, sz);

    streamer->tail_ += sz;
}

void cstream_read(Cstream* streamer, void* d, size_t sz, unsigned int endian)
{
    size_t tsize = 0;
    size_t hsize = 0;
    size_t grow = 0;

    if (sz == 0 || streamer == NULL || streamer->head_ + sz > streamer->tail_)
    {
        return;
    }

    if((streamer->head_ - streamer->buffer_) > (0.8) * streamer->size_)
    {
        tsize = streamer->tail_ - streamer->head_;

        char* newbuffer = (char*)malloc(streamer->size_);        
        memcpy(newbuffer, streamer->head_, tsize);
        streamer->buffer_ = newbuffer;
        streamer->head_ = streamer->buffer_;
        streamer->tail_ = streamer->buffer_ + tsize;
    }

    memcpy(d, streamer->head_, sz);

    unsigned int system_endian = SW_NATIVE_ENDIAN_FLAG;
#if SW_LITTLE_ENDIAN
    system_endian = SW_LITTLE_ENDIAN_FLAG;
#else
    system_endian = SW_BIG_ENDIAN_FLAG;
#endif
    
    if (endian != SW_NATIVE_ENDIAN_FLAG && system_endian != endian)
    {
        switch (sz)
        {
        case 2:
            __swap16(d);
            break;
        case 4:
            __swap32(d);
            break;
        case 8:
            __swap64(d);
            break;
        default:
            break;
        }
    }

    streamer->head_ += sz;
}

