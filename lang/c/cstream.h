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

#ifdef __cplusplus
}
#endif

#endif //__wiregen_stream_h


