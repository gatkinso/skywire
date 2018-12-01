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

#ifndef __wiregen_endian_h
#define __wiregen_endian_h

#include <stdint.h>

#if defined WIN32
    #include <Winsock.h>
#endif

#if defined LINUX || defined APPLE
    #include <netinet/in.h>
#endif

#if defined LINUX || defined WIN32
    #define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
    #define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

#if SW_LITTLE_ENDIAN  //Top level CMakeLists.txt
    #ifndef htole16
    #define htole16(x) (x)
    #endif

    #ifndef htole32
    #define htole32(x) (x)
    #endif

    #ifndef htole64
    #define htole64(x) (x)
    #endif

    #ifndef le16toh
    #define le16toh(x) (x)
    #endif

    #ifndef le32toh
    #define le32toh(x) (x)
    #endif

    #ifndef le64toh
    #define le64toh(x) (x)
    #endif

    #ifndef htobe16
    #define htobe16(x) htons(x)
    #endif

    #ifndef htobe32
    #define htobe32(x) htonl(x)
    #endif

    #ifndef htobe64
    #define htobe64(x) htonll(x)
    #endif

    #ifndef be16toh
    #define be16toh(x) ntohs(x)
    #endif

    #ifndef be32toh
    #define be32toh(x) ntohl(x)
    #endif

    #ifndef be64toh
    #define be64toh(x) ntohll(x)
    #endif
#else
    #ifndef htole16
    #define htole16(x) htons(x)
    #endif

    #ifndef htole32
    #define htole32(x) htonl(x)
    #endif

    #ifndef htole64
    #define htole64(x) htonll(x)
    #endif

    #ifndef le16toh
    #define le16toh(x) ntohs(x)
    #endif

    #ifndef le32toh
    #define le32toh(x) ntohl(x)
    #endif

    #ifndef le64toh
    #define le64toh(x) ntohll(x)
    #endif

    #ifndef htobe16
    #define htobe16(x) (x)
    #endif

    #ifndef htobe32
    #define htobe32(x) (x)
    #endif

    #ifndef htobe64
    #define htobe64(x) (x)
    #endif

    #ifndef be16toh
    #define be16toh(x) (x)
    #endif

    #ifndef be32toh
    #define be32toh(x) (x)
    #endif

    #ifndef be64toh
    #define be64toh(x) (x)
    #endif
#endif

#endif // __wiregen_endian_h

