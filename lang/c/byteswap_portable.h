#ifndef BYTESWAP_PORTABLE_H
#define BYTESWAP_PORTABLE_H

#include <stdint.h>

#if defined(__APPLE__)
// macOS / iOS
#include <libkern/OSByteOrder.h>
#define bswap16(x) OSSwapInt16(x)
#define bswap32(x) OSSwapInt32(x)
#define bswap64(x) OSSwapInt64(x)

#elif defined(__linux__)
// Linux / Android
#include <byteswap.h>
#define bswap16(x) bswap_16(x)
#define bswap32(x) bswap_32(x)
#define bswap64(x) bswap_64(x)

#elif defined(_WIN32)
// Windows (MSVC / MinGW)
#include <stdlib.h>
#define bswap16(x) _byteswap_ushort(x)
#define bswap32(x) _byteswap_ulong(x)
#define bswap64(x) _byteswap_uint64(x)

#else
// Fallback using GCC/Clang builtins
static inline uint16_t bswap16(uint16_t x) { return __builtin_bswap16(x); }
static inline uint32_t bswap32(uint32_t x) { return __builtin_bswap32(x); }
static inline uint64_t bswap64(uint64_t x) { return __builtin_bswap64(x); }
#endif

#endif // BYTESWAP_PORTABLE_H
