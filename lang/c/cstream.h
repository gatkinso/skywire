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

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct CStreamTag {
        char* buffer_;
        size_t size_;
        char *head_, *tail_;
    } Cstream;

    void cstream_init(Cstream* streamer, size_t size);

    void cstream_fini(Cstream* streamer);

    void cstream_write(Cstream* streamer, const void* d, size_t sz);

    void cstream_read(Cstream* streamer, void* d, size_t sz);

#ifdef __cplusplus
}
#endif

#endif //__wiregen_stream_h


