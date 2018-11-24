//
////  atkinet - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//
#include <stdio.h>
#include "cstream.h"

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

void cstream_write(Cstream* streamer, const void* d, size_t sz)
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

    memcpy(streamer->tail_, d, sz);

    streamer->tail_ += sz;
}

void cstream_read(Cstream* streamer, void* d, size_t sz)
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

    streamer->head_ += sz;
}
