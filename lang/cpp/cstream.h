//
////  atkinet - A library for cross platform development in C++
//
//    (c)2003 - 2006 Geoffrey John Atkinson
//
//    This software is the property of Geoffrey John Atkinson.
//

#ifndef __wiregen_stream_h
#define __wiregen_stream_h

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#define DEF_CSTREAM_SIZE 1024

typedef struct CStreamTag {
    char* buffer_;
    size_t size_;
    char *head_, *tail_;
} Cstream;

void cstream_init(Cstream* streamer, size_t size)
{
    if (streamer != NULL)
        free(streamer->buffer_);

    streamer->buffer_ = (char*)malloc(size);
    streamer->size_ = size;
    streamer->head_ = streamer->buffer_;
    streamer->tail_ = streamer->head_;
}


void cstream_fini(Cstream* streamer, size_t size)
{
    if (streamer != NULL)
        free(streamer->buffer_);

    streamer->size_ = 0;
    streamer->head_ = NULL;
    streamer->tail_ = NULL;
}

void write(Cstream* streamer, const void* d, size_t sz)
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

    if (streamer->tail_+sz >= streamer->buffer_+streamer->size_)
    {
        hsize = streamer->head_ - streamer->buffer_;
        tsize = streamer->tail_ - streamer->buffer_;

        grow = streamer->size_ * 2;
        if (sz >= grow)
        {
            grow = sz * 2;
        }
        streamer->buffer_ = (char*)realloc(streamer->buffer_, streamer->size_+grow);

        streamer->head_ = streamer->buffer_ + hsize;
        streamer->tail_ = streamer->buffer_ + tsize;
    }

    memcpy(streamer->tail_, d, sizeof(char));

    streamer->tail_ += sz;
}

void read(Cstream* streamer, void* d, size_t sz)
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
        return;
    }

    if (streamer->head_+sz > streamer->tail_ ||
        streamer->head_- streamer->buffer_ > (0.8)*streamer->size_)
    {
        tsize = streamer->tail_ - streamer->head_;

        grow = streamer->size_ * 2;
        if (sz >= grow)
        {
            grow = sz * 2;
        }
        char* newbuffer = (char*)malloc(streamer->size_ + grow);        
        memcpy(newbuffer, streamer->head_, tsize);

        free(streamer->buffer_);
        streamer->buffer_ = newbuffer;

        streamer->head_ = streamer->buffer_;
        streamer->tail_ = streamer->buffer_ + tsize;
    }

    memcpy(d, streamer->head_, sz);

    streamer->head_ += sz;
}

#endif


