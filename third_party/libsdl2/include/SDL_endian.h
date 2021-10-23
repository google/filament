/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 *  \file SDL_endian.h
 *
 *  Functions for reading and writing endian-specific values
 */

#ifndef SDL_endian_h_
#define SDL_endian_h_

#include "SDL_stdinc.h"

#ifdef _MSC_VER
/* As of Clang 11, '_m_prefetchw' is conflicting with the winnt.h's version,
   so we define the needed '_m_prefetch' here as a pseudo-header, until the issue is fixed. */

#ifdef __clang__
#ifndef __PRFCHWINTRIN_H
#define __PRFCHWINTRIN_H

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_m_prefetch(void *__P)
{
  __builtin_prefetch (__P, 0, 3 /* _MM_HINT_T0 */);
}

#endif /* __PRFCHWINTRIN_H */
#endif /* __clang__ */

#include <intrin.h>
#endif

/**
 *  \name The two types of endianness
 */
/* @{ */
#define SDL_LIL_ENDIAN  1234
#define SDL_BIG_ENDIAN  4321
/* @} */

#ifndef SDL_BYTEORDER           /* Not defined in SDL_config.h? */
#ifdef __linux__
#include <endian.h>
#define SDL_BYTEORDER  __BYTE_ORDER
#elif defined(__OpenBSD__)
#include <endian.h>
#define SDL_BYTEORDER  BYTE_ORDER
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/endian.h>
#define SDL_BYTEORDER  BYTE_ORDER
#else
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__sparc__)
#define SDL_BYTEORDER   SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER   SDL_LIL_ENDIAN
#endif
#endif /* __linux__ */
#endif /* !SDL_BYTEORDER */


#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \file SDL_endian.h
 */
#if (defined(__clang__) && (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 2))) || \
    (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
#define SDL_Swap16(x) __builtin_bswap16(x)
#elif defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
  __asm__("xchgb %b0,%h0": "=q"(x):"0"(x));
    return x;
}
#elif defined(__GNUC__) && defined(__x86_64__)
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
  __asm__("xchgb %b0,%h0": "=Q"(x):"0"(x));
    return x;
}
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
    int result;

  __asm__("rlwimi %0,%2,8,16,23": "=&r"(result):"0"(x >> 8), "r"(x));
    return (Uint16)result;
}
#elif defined(__GNUC__) && defined(__aarch64__)
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
  __asm__("rev16 %w1, %w0" : "=r"(x) : "r"(x));
  return x;
}
#elif defined(__GNUC__) && (defined(__m68k__) && !defined(__mcoldfire__))
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
  __asm__("rorw #8,%0": "=d"(x): "0"(x):"cc");
    return x;
}
#elif defined(_MSC_VER)
#pragma intrinsic(_byteswap_ushort)
#define SDL_Swap16(x) _byteswap_ushort(x)
#elif defined(__WATCOMC__) && defined(__386__)
extern _inline Uint16 SDL_Swap16(Uint16);
#pragma aux SDL_Swap16 = \
  "xchg al, ah" \
  parm   [ax]   \
  modify [ax];
#else
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
    return SDL_static_cast(Uint16, ((x << 8) | (x >> 8)));
}
#endif

#if (defined(__clang__) && (__clang_major__ > 2 || (__clang_major__ == 2 && __clang_minor__ >= 6))) || \
    (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
#define SDL_Swap32(x) __builtin_bswap32(x)
#elif defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
  __asm__("bswap %0": "=r"(x):"0"(x));
    return x;
}
#elif defined(__GNUC__) && defined(__x86_64__)
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
  __asm__("bswapl %0": "=r"(x):"0"(x));
    return x;
}
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
    Uint32 result;

  __asm__("rlwimi %0,%2,24,16,23": "=&r"(result): "0" (x>>24),  "r"(x));
  __asm__("rlwimi %0,%2,8,8,15"  : "=&r"(result): "0" (result), "r"(x));
  __asm__("rlwimi %0,%2,24,0,7"  : "=&r"(result): "0" (result), "r"(x));
    return result;
}
#elif defined(__GNUC__) && defined(__aarch64__)
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
  __asm__("rev %w1, %w0": "=r"(x):"r"(x));
  return x;
}
#elif defined(__GNUC__) && (defined(__m68k__) && !defined(__mcoldfire__))
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
  __asm__("rorw #8,%0\n\tswap %0\n\trorw #8,%0": "=d"(x): "0"(x):"cc");
    return x;
}
#elif defined(__WATCOMC__) && defined(__386__)
extern _inline Uint32 SDL_Swap32(Uint32);
#pragma aux SDL_Swap32 = \
  "bswap eax"  \
  parm   [eax] \
  modify [eax];
#elif defined(_MSC_VER)
#pragma intrinsic(_byteswap_ulong)
#define SDL_Swap32(x) _byteswap_ulong(x)
#else
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
    return SDL_static_cast(Uint32, ((x << 24) | ((x << 8) & 0x00FF0000) |
                                    ((x >> 8) & 0x0000FF00) | (x >> 24)));
}
#endif

#if (defined(__clang__) && (__clang_major__ > 2 || (__clang_major__ == 2 && __clang_minor__ >= 6))) || \
    (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
#define SDL_Swap64(x) __builtin_bswap64(x)
#elif defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
SDL_FORCE_INLINE Uint64
SDL_Swap64(Uint64 x)
{
    union {
        struct {
            Uint32 a, b;
        } s;
        Uint64 u;
    } v;
    v.u = x;
  __asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1"
          : "=r"(v.s.a), "=r"(v.s.b)
          : "0" (v.s.a),  "1"(v.s.b));
    return v.u;
}
#elif defined(__GNUC__) && defined(__x86_64__)
SDL_FORCE_INLINE Uint64
SDL_Swap64(Uint64 x)
{
  __asm__("bswapq %0": "=r"(x):"0"(x));
    return x;
}
#elif defined(__WATCOMC__) && defined(__386__)
extern _inline Uint64 SDL_Swap64(Uint64);
#pragma aux SDL_Swap64 = \
  "bswap eax"     \
  "bswap edx"     \
  "xchg eax,edx"  \
  parm [eax edx]  \
  modify [eax edx];
#elif defined(_MSC_VER)
#pragma intrinsic(_byteswap_uint64)
#define SDL_Swap64(x) _byteswap_uint64(x)
#else
SDL_FORCE_INLINE Uint64
SDL_Swap64(Uint64 x)
{
    Uint32 hi, lo;

    /* Separate into high and low 32-bit values and swap them */
    lo = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
    x >>= 32;
    hi = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
    x = SDL_Swap32(lo);
    x <<= 32;
    x |= SDL_Swap32(hi);
    return (x);
}
#endif


SDL_FORCE_INLINE float
SDL_SwapFloat(float x)
{
    union {
        float f;
        Uint32 ui32;
    } swapper;
    swapper.f = x;
    swapper.ui32 = SDL_Swap32(swapper.ui32);
    return swapper.f;
}


/**
 *  \name Swap to native
 *  Byteswap item from the specified endianness to the native endianness.
 */
/* @{ */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SDL_SwapLE16(X)     (X)
#define SDL_SwapLE32(X)     (X)
#define SDL_SwapLE64(X)     (X)
#define SDL_SwapFloatLE(X)  (X)
#define SDL_SwapBE16(X)     SDL_Swap16(X)
#define SDL_SwapBE32(X)     SDL_Swap32(X)
#define SDL_SwapBE64(X)     SDL_Swap64(X)
#define SDL_SwapFloatBE(X)  SDL_SwapFloat(X)
#else
#define SDL_SwapLE16(X)     SDL_Swap16(X)
#define SDL_SwapLE32(X)     SDL_Swap32(X)
#define SDL_SwapLE64(X)     SDL_Swap64(X)
#define SDL_SwapFloatLE(X)  SDL_SwapFloat(X)
#define SDL_SwapBE16(X)     (X)
#define SDL_SwapBE32(X)     (X)
#define SDL_SwapBE64(X)     (X)
#define SDL_SwapFloatBE(X)  (X)
#endif
/* @} *//* Swap to native */

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_endian_h_ */

/* vi: set ts=4 sw=4 expandtab: */
