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
#include <string.h>

#define SW_BIG_ENDIAN_FLAG    0
#define SW_LITTLE_ENDIAN_FLAG 1
#define SW_NATIVE_ENDIAN_FLAG 2


#if defined LINUX || defined APPLE
    #include <netinet/in.h>
#endif

// Host byte order. A build may pass SW_LITTLE_ENDIAN or SW_BIG_ENDIAN
// (bare or =0/1); otherwise it comes from the compiler. A guess is never
// made: picking the wrong branch silently byte-swaps everything on the wire.
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define SW_DETECTED_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define SW_DETECTED_LITTLE_ENDIAN 0
#elif defined(_WIN32)
    #define SW_DETECTED_LITTLE_ENDIAN 1
#endif

#if defined(SW_LITTLE_ENDIAN) && defined(SW_BIG_ENDIAN)
    #if !!(SW_LITTLE_ENDIAN) == !!(SW_BIG_ENDIAN)
        #error "sw_endian.h: SW_LITTLE_ENDIAN and SW_BIG_ENDIAN disagree"
    #endif
#endif

#if defined(SW_LITTLE_ENDIAN)
    #define SW_HOST_LITTLE_ENDIAN (!!(SW_LITTLE_ENDIAN))
#elif defined(SW_BIG_ENDIAN)
    #define SW_HOST_LITTLE_ENDIAN (!(SW_BIG_ENDIAN))
#elif defined(SW_DETECTED_LITTLE_ENDIAN)
    #define SW_HOST_LITTLE_ENDIAN SW_DETECTED_LITTLE_ENDIAN
#else
    #error "sw_endian.h: cannot determine host byte order; define SW_LITTLE_ENDIAN=1 or SW_BIG_ENDIAN=1"
#endif

#if defined(SW_DETECTED_LITTLE_ENDIAN) && SW_HOST_LITTLE_ENDIAN != SW_DETECTED_LITTLE_ENDIAN
    #error "sw_endian.h: SW_LITTLE_ENDIAN/SW_BIG_ENDIAN contradicts the compiler's byte order"
#endif

// Keep the legacy names usable in #if for code that still tests them.
#ifndef SW_LITTLE_ENDIAN
    #define SW_LITTLE_ENDIAN SW_HOST_LITTLE_ENDIAN
#endif
#ifndef SW_BIG_ENDIAN
    #define SW_BIG_ENDIAN (!SW_HOST_LITTLE_ENDIAN)
#endif

#if defined(_MSC_VER)
    #include <stdlib.h>
    #define SW_BSWAP16(x) _byteswap_ushort((uint16_t)(x))
    #define SW_BSWAP32(x) _byteswap_ulong((uint32_t)(x))
    #define SW_BSWAP64(x) _byteswap_uint64((uint64_t)(x))
#else
    #define SW_BSWAP16(x) __builtin_bswap16((uint16_t)(x))
    #define SW_BSWAP32(x) __builtin_bswap32((uint32_t)(x))
    #define SW_BSWAP64(x) __builtin_bswap64((uint64_t)(x))
#endif

// The runtime is header-only: its functions are static inline.
#ifndef SW_INLINE
    #if defined(_MSC_VER) && !defined(__cplusplus)
        #define SW_INLINE static __inline
    #else
        #define SW_INLINE static inline
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Reverse the 2, 4 or 8 bytes at d in place; NULL is ignored.
SW_INLINE void sw_swap16(char* d)
{
    uint16_t v;
    if (!d) return;
    memcpy(&v, d, sizeof(v));
    v = SW_BSWAP16(v);
    memcpy(d, &v, sizeof(v));
}

SW_INLINE void sw_swap32(char* d)
{
    uint32_t v;
    if (!d) return;
    memcpy(&v, d, sizeof(v));
    v = SW_BSWAP32(v);
    memcpy(d, &v, sizeof(v));
}

SW_INLINE void sw_swap64(char* d)
{
    uint64_t v;
    if (!d) return;
    memcpy(&v, d, sizeof(v));
    v = SW_BSWAP64(v);
    memcpy(d, &v, sizeof(v));
}

#if SW_HOST_LITTLE_ENDIAN
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
    #define htobe16(x) SW_BSWAP16(x)
    #endif

    #ifndef htobe32
    #define htobe32(x) SW_BSWAP32(x)
    #endif

    #ifndef htobe64
    #define htobe64(x) SW_BSWAP64(x)
    #endif

    #ifndef be16toh
    #define be16toh(x) SW_BSWAP16(x)
    #endif

    #ifndef be32toh
    #define be32toh(x) SW_BSWAP32(x)
    #endif

    #ifndef be64toh
    #define be64toh(x) SW_BSWAP64(x)
    #endif

    #ifndef htone16
    #define htone16(x) (x)
    #endif

    #ifndef htone32
    #define htone32(x) (x)
    #endif

    #ifndef htone64
    #define htone64(x) (x)
    #endif

    #ifndef ne16toh
    #define ne16toh(x) (x)
    #endif

    #ifndef ne32toh
    #define ne32toh(x) (x)
    #endif

    #ifndef ne64toh
    #define ne64toh(x) (x)
    #endif

    #ifndef neftohf
    #define neftohf(x) ((void)(x))
    #endif

    #ifndef nedtohd
    #define nedtohd(x) ((void)(x))
    #endif

    #ifndef beftohf
    #define beftohf(x) (sw_swap32((char*)&(x)))
    #endif

    #ifndef bedtohd
    #define bedtohd(x) (sw_swap64((char*)&(x)))
    #endif

    #ifndef leftohf
    #define leftohf(x) ((void)(x))
    #endif

    #ifndef ledtohd
    #define ledtohd(x) ((void)(x))
    #endif

    #ifndef hftonef
    #define hftonef(x) ((void)(x))
    #endif

    #ifndef hdtoned
    #define hdtoned(x) ((void)(x))
    #endif

    #ifndef hftobef
    #define hftobef(x) (sw_swap32((char*)&(x)))
    #endif

    #ifndef hdtobed
    #define hdtobed(x) (sw_swap64((char*)&(x)))
    #endif

    #ifndef hftolef
    #define hftolef(x) ((void)(x))
    #endif

    #ifndef hdtoled
    #define hdtoled(x) ((void)(x))
    #endif
#else
    #ifndef htole16
    #define htole16(x) SW_BSWAP16(x)
    #endif

    #ifndef htole32
    #define htole32(x) SW_BSWAP32(x)
    #endif

    #ifndef htole64
    #define htole64(x) SW_BSWAP64(x)
    #endif

    #ifndef le16toh
    #define le16toh(x) SW_BSWAP16(x)
    #endif

    #ifndef le32toh
    #define le32toh(x) SW_BSWAP32(x)
    #endif

    #ifndef le64toh
    #define le64toh(x) SW_BSWAP64(x)
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

    #ifndef htone16
    #define htone16(x) (x)
    #endif

    #ifndef htone32
    #define htone32(x) (x)
    #endif

    #ifndef htone64
    #define htone64(x) (x)
    #endif

    #ifndef ne16toh
    #define ne16toh(x) (x)
    #endif

    #ifndef ne32toh
    #define ne32toh(x) (x)
    #endif

    #ifndef ne64toh
    #define ne64toh(x) (x)
    #endif

    #ifndef neftohf
    #define neftohf(x) ((void)(x))
    #endif

    #ifndef nedtohd
    #define nedtohd(x) ((void)(x))
    #endif

    #ifndef beftohf
    #define beftohf(x) ((void)(x))
    #endif

    #ifndef bedtohd
    #define bedtohd(x) ((void)(x))
    #endif

    #ifndef leftohf
    #define leftohf(x) (sw_swap32((char*)&(x)))
    #endif

    #ifndef ledtohd
    #define ledtohd(x) (sw_swap64((char*)&(x)))
    #endif

    #ifndef hftonef
    #define hftonef(x) ((void)(x))
    #endif

    #ifndef hdtoned
    #define hdtoned(x) ((void)(x))
    #endif

    #ifndef hftobef
    #define hftobef(x) ((void)(x))
    #endif

    #ifndef hdtobed
    #define hdtobed(x) ((void)(x))
    #endif

    #ifndef hftolef
    #define hftolef(x) (sw_swap32((char*)&(x)))
    #endif

    #ifndef hdtoled
    #define hdtoled(x) (sw_swap64((char*)&(x)))
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif // __wiregen_endian_h

