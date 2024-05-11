/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

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

#include "../SDL_internal.h"
#include "SDL_audio.h"
#include "SDL_audio_c.h"
#include "SDL_cpuinfo.h"
#include "SDL_assert.h"

/* !!! FIXME: write NEON code. */
#define HAVE_NEON_INTRINSICS 0

#ifdef __SSE2__
#define HAVE_SSE2_INTRINSICS 1
#endif

#if defined(__x86_64__) && HAVE_SSE2_INTRINSICS
#define NEED_SCALAR_CONVERTER_FALLBACKS 0  /* x86_64 guarantees SSE2. */
#elif __MACOSX__ && HAVE_SSE2_INTRINSICS
#define NEED_SCALAR_CONVERTER_FALLBACKS 0  /* Mac OS X/Intel guarantees SSE2. */
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 8) && HAVE_NEON_INTRINSICS
#define NEED_SCALAR_CONVERTER_FALLBACKS 0  /* ARMv8+ promise NEON. */
#elif defined(__APPLE__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7) && HAVE_NEON_INTRINSICS
#define NEED_SCALAR_CONVERTER_FALLBACKS 0  /* All Apple ARMv7 chips promise NEON support. */
#endif

/* Set to zero if platform is guaranteed to use a SIMD codepath here. */
#ifndef NEED_SCALAR_CONVERTER_FALLBACKS
#define NEED_SCALAR_CONVERTER_FALLBACKS 1
#endif

/* Function pointers set to a CPU-specific implementation. */
SDL_AudioFilter SDL_Convert_S8_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_U8_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_S16_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_U16_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_S32_to_F32 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_S8 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_U8 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_S16 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_U16 = NULL;
SDL_AudioFilter SDL_Convert_F32_to_S32 = NULL;


#define DIVBY128 0.0078125f
#define DIVBY32768 0.000030517578125f
#define DIVBY2147483648 0.00000000046566128730773926


#if NEED_SCALAR_CONVERTER_FALLBACKS
static void SDLCALL
SDL_Convert_S8_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint8 *src = ((const Sint8 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 4)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S8", "AUDIO_F32");

    for (i = cvt->len_cvt; i; --i, --src, --dst) {
        *dst = ((float) *src) * DIVBY128;
    }

    cvt->len_cvt *= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_U8_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Uint8 *src = ((const Uint8 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 4)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_U8", "AUDIO_F32");

    for (i = cvt->len_cvt; i; --i, --src, --dst) {
        *dst = (((float) *src) * DIVBY128) - 1.0f;
    }

    cvt->len_cvt *= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_S16_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint16 *src = ((const Sint16 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 2)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S16", "AUDIO_F32");

    for (i = cvt->len_cvt / sizeof (Sint16); i; --i, --src, --dst) {
        *dst = ((float) *src) * DIVBY32768;
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_U16_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Uint16 *src = ((const Uint16 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 2)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_U16", "AUDIO_F32");

    for (i = cvt->len_cvt / sizeof (Uint16); i; --i, --src, --dst) {
        *dst = (((float) *src) * DIVBY32768) - 1.0f;
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_S32_to_F32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint32 *src = (const Sint32 *) cvt->buf;
    float *dst = (float *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S32", "AUDIO_F32");

    for (i = cvt->len_cvt / sizeof (Sint32); i; --i, ++src, ++dst) {
        *dst = (float) (((double) *src) * DIVBY2147483648);
    }

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S8_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint8 *dst = (Sint8 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S8");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample > 1.0f) {
            *dst = 127;
        } else if (sample < -1.0f) {
            *dst = -127;
        } else {
            *dst = (Sint8)(sample * 127.0f);
        }
    }

    cvt->len_cvt /= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S8);
    }
}

static void SDLCALL
SDL_Convert_F32_to_U8_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Uint8 *dst = (Uint8 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_U8");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample > 1.0f) {
            *dst = 255;
        } else if (sample < -1.0f) {
            *dst = 0;
        } else {
            *dst = (Uint8)((sample + 1.0f) * 127.0f);
        }
    }

    cvt->len_cvt /= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_U8);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S16_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint16 *dst = (Sint16 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S16");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample > 1.0f) {
            *dst = 32767;
        } else if (sample < -1.0f) {
            *dst = -32767;
        } else {
            *dst = (Sint16)(sample * 32767.0f);
        }
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S16SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_U16_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Uint16 *dst = (Uint16 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_U16");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample > 1.0f) {
            *dst = 65534;
        } else if (sample < -1.0f) {
            *dst = 0;
        } else {
            *dst = (Uint16)((sample + 1.0f) * 32767.0f);
        }
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_U16SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S32_Scalar(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint32 *dst = (Sint32 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S32");

    for (i = cvt->len_cvt / sizeof (float); i; --i, ++src, ++dst) {
        const float sample = *src;
        if (sample > 1.0f) {
            *dst = 2147483647;
        } else if (sample < -1.0f) {
            *dst = -2147483647;
        } else {
            *dst = (Sint32)((double)sample * 2147483647.0);
        }
    }

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S32SYS);
    }
}
#endif


#if HAVE_SSE2_INTRINSICS
static void SDLCALL
SDL_Convert_S8_to_F32_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint8 *src = ((const Sint8 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 4)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S8", "AUDIO_F32 (using SSE2)");

    /* Get dst aligned to 16 bytes (since buffer is growing, we don't have to worry about overreading from src) */
    for (i = cvt->len_cvt; i && (((size_t) (dst-15)) & 15); --i, --src, --dst) {
        *dst = ((float) *src) * DIVBY128;
    }

    src -= 15; dst -= 15;  /* adjust to read SSE blocks from the start. */
    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128i *mmsrc = (const __m128i *) src;
        const __m128i zero = _mm_setzero_si128();
        const __m128 divby128 = _mm_set1_ps(DIVBY128);
        while (i >= 16) {   /* 16 * 8-bit */
            const __m128i bytes = _mm_load_si128(mmsrc);  /* get 16 sint8 into an XMM register. */
            /* treat as int16, shift left to clear every other sint16, then back right with sign-extend. Now sint16. */
            const __m128i shorts1 = _mm_srai_epi16(_mm_slli_epi16(bytes, 8), 8);
            /* right-shift-sign-extend gets us sint16 with the other set of values. */
            const __m128i shorts2 = _mm_srai_epi16(bytes, 8);
            /* unpack against zero to make these int32, shift to make them sign-extend, convert to float, multiply. Whew! */
            const __m128 floats1 = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(shorts1, zero), 16), 16)), divby128);
            const __m128 floats2 = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpacklo_epi16(shorts2, zero), 16), 16)), divby128);
            const __m128 floats3 = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(shorts1, zero), 16), 16)), divby128);
            const __m128 floats4 = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_slli_epi32(_mm_unpackhi_epi16(shorts2, zero), 16), 16)), divby128);
            /* Interleave back into correct order, store. */
            _mm_store_ps(dst, _mm_unpacklo_ps(floats1, floats2));
            _mm_store_ps(dst+4, _mm_unpackhi_ps(floats1, floats2));
            _mm_store_ps(dst+8, _mm_unpacklo_ps(floats3, floats4));
            _mm_store_ps(dst+12, _mm_unpackhi_ps(floats3, floats4));
            i -= 16; mmsrc--; dst -= 16;
        }

        src = (const Sint8 *) mmsrc;
    }

    src += 15; dst += 15;  /* adjust for any scalar finishing. */

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = ((float) *src) * DIVBY128;
        i--; src--; dst--;
    }

    cvt->len_cvt *= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_U8_to_F32_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Uint8 *src = ((const Uint8 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 4)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_U8", "AUDIO_F32 (using SSE2)");

    /* Get dst aligned to 16 bytes (since buffer is growing, we don't have to worry about overreading from src) */
    for (i = cvt->len_cvt; i && (((size_t) (dst-15)) & 15); --i, --src, --dst) {
        *dst = (((float) *src) * DIVBY128) - 1.0f;
    }

    src -= 15; dst -= 15;  /* adjust to read SSE blocks from the start. */
    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128i *mmsrc = (const __m128i *) src;
        const __m128i zero = _mm_setzero_si128();
        const __m128 divby128 = _mm_set1_ps(DIVBY128);
        const __m128 minus1 = _mm_set1_ps(-1.0f);
        while (i >= 16) {   /* 16 * 8-bit */
            const __m128i bytes = _mm_load_si128(mmsrc);  /* get 16 uint8 into an XMM register. */
            /* treat as int16, shift left to clear every other sint16, then back right with zero-extend. Now uint16. */
            const __m128i shorts1 = _mm_srli_epi16(_mm_slli_epi16(bytes, 8), 8);
            /* right-shift-zero-extend gets us uint16 with the other set of values. */
            const __m128i shorts2 = _mm_srli_epi16(bytes, 8);
            /* unpack against zero to make these int32, convert to float, multiply, add. Whew! */
            /* Note that AVX2 can do floating point multiply+add in one instruction, fwiw. SSE2 cannot. */
            const __m128 floats1 = _mm_add_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(shorts1, zero)), divby128), minus1);
            const __m128 floats2 = _mm_add_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi16(shorts2, zero)), divby128), minus1);
            const __m128 floats3 = _mm_add_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_unpackhi_epi16(shorts1, zero)), divby128), minus1);
            const __m128 floats4 = _mm_add_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_unpackhi_epi16(shorts2, zero)), divby128), minus1);
            /* Interleave back into correct order, store. */
            _mm_store_ps(dst, _mm_unpacklo_ps(floats1, floats2));
            _mm_store_ps(dst+4, _mm_unpackhi_ps(floats1, floats2));
            _mm_store_ps(dst+8, _mm_unpacklo_ps(floats3, floats4));
            _mm_store_ps(dst+12, _mm_unpackhi_ps(floats3, floats4));
            i -= 16; mmsrc--; dst -= 16;
        }

        src = (const Uint8 *) mmsrc;
    }

    src += 15; dst += 15;  /* adjust for any scalar finishing. */

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (((float) *src) * DIVBY128) - 1.0f;
        i--; src--; dst--;
    }

    cvt->len_cvt *= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_S16_to_F32_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint16 *src = ((const Sint16 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 2)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S16", "AUDIO_F32 (using SSE2)");

    /* Get dst aligned to 16 bytes (since buffer is growing, we don't have to worry about overreading from src) */
    for (i = cvt->len_cvt / sizeof (Sint16); i && (((size_t) (dst-7)) & 15); --i, --src, --dst) {
        *dst = ((float) *src) * DIVBY32768;
    }

    src -= 7; dst -= 7;  /* adjust to read SSE blocks from the start. */
    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128 divby32768 = _mm_set1_ps(DIVBY32768);
        while (i >= 8) {   /* 8 * 16-bit */
            const __m128i ints = _mm_load_si128((__m128i const *) src);  /* get 8 sint16 into an XMM register. */
            /* treat as int32, shift left to clear every other sint16, then back right with sign-extend. Now sint32. */
            const __m128i a = _mm_srai_epi32(_mm_slli_epi32(ints, 16), 16);
            /* right-shift-sign-extend gets us sint32 with the other set of values. */
            const __m128i b = _mm_srai_epi32(ints, 16);
            /* Interleave these back into the right order, convert to float, multiply, store. */
            _mm_store_ps(dst, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi32(a, b)), divby32768));
            _mm_store_ps(dst+4, _mm_mul_ps(_mm_cvtepi32_ps(_mm_unpackhi_epi32(a, b)), divby32768));
            i -= 8; src -= 8; dst -= 8;
        }
    }

    src += 7; dst += 7;  /* adjust for any scalar finishing. */

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = ((float) *src) * DIVBY32768;
        i--; src--; dst--;
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_U16_to_F32_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Uint16 *src = ((const Uint16 *) (cvt->buf + cvt->len_cvt)) - 1;
    float *dst = ((float *) (cvt->buf + cvt->len_cvt * 2)) - 1;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_U16", "AUDIO_F32 (using SSE2)");

    /* Get dst aligned to 16 bytes (since buffer is growing, we don't have to worry about overreading from src) */
    for (i = cvt->len_cvt / sizeof (Sint16); i && (((size_t) (dst-7)) & 15); --i, --src, --dst) {
        *dst = (((float) *src) * DIVBY32768) - 1.0f;
    }

    src -= 7; dst -= 7;  /* adjust to read SSE blocks from the start. */
    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128 divby32768 = _mm_set1_ps(DIVBY32768);
        const __m128 minus1 = _mm_set1_ps(1.0f);
        while (i >= 8) {   /* 8 * 16-bit */
            const __m128i ints = _mm_load_si128((__m128i const *) src);  /* get 8 sint16 into an XMM register. */
            /* treat as int32, shift left to clear every other sint16, then back right with zero-extend. Now sint32. */
            const __m128i a = _mm_srli_epi32(_mm_slli_epi32(ints, 16), 16);
            /* right-shift-sign-extend gets us sint32 with the other set of values. */
            const __m128i b = _mm_srli_epi32(ints, 16);
            /* Interleave these back into the right order, convert to float, multiply, store. */
            _mm_store_ps(dst, _mm_add_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_unpacklo_epi32(a, b)), divby32768), minus1));
            _mm_store_ps(dst+4, _mm_add_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_unpackhi_epi32(a, b)), divby32768), minus1));
            i -= 8; src -= 8; dst -= 8;
        }
    }

    src += 7; dst += 7;  /* adjust for any scalar finishing. */

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (((float) *src) * DIVBY32768) - 1.0f;
        i--; src--; dst--;
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

#if defined(__GNUC__) && (__GNUC__ < 4)
/* these were added as of gcc-4.0: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=19418 */
static inline __m128 _mm_castsi128_ps(__m128i __A) {
  return (__m128) __A;
}
static inline __m128i _mm_castps_si128(__m128 __A) {
  return (__m128i) __A;
}
#endif

static void SDLCALL
SDL_Convert_S32_to_F32_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const Sint32 *src = (const Sint32 *) cvt->buf;
    float *dst = (float *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_S32", "AUDIO_F32 (using SSE2)");

    /* Get dst aligned to 16 bytes */
    for (i = cvt->len_cvt / sizeof (Sint32); i && (((size_t) dst) & 15); --i, ++src, ++dst) {
        *dst = (float) (((double) *src) * DIVBY2147483648);
    }

    SDL_assert(!i || ((((size_t) dst) & 15) == 0));
    SDL_assert(!i || ((((size_t) src) & 15) == 0));

    {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128d divby2147483648 = _mm_set1_pd(DIVBY2147483648);
        const __m128i *mmsrc = (const __m128i *) src;
        while (i >= 4) {   /* 4 * sint32 */
            const __m128i ints = _mm_load_si128(mmsrc);
            /* bitshift the whole register over, so _mm_cvtepi32_pd can read the top ints in the bottom of the vector. */
            const __m128d doubles1 = _mm_mul_pd(_mm_cvtepi32_pd(_mm_srli_si128(ints, 8)), divby2147483648);
            const __m128d doubles2 = _mm_mul_pd(_mm_cvtepi32_pd(ints), divby2147483648);
            /* convert to float32, bitshift/or to get these into a vector to store. */
            _mm_store_ps(dst, _mm_castsi128_ps(_mm_or_si128(_mm_slli_si128(_mm_castps_si128(_mm_cvtpd_ps(doubles1)), 8), _mm_castps_si128(_mm_cvtpd_ps(doubles2)))));
            i -= 4; mmsrc++; dst += 4;
        }
        src = (const Sint32 *) mmsrc;
    }

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (float) (((double) *src) * DIVBY2147483648);
        i--; src++; dst++;
    }

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_F32SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S8_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint8 *dst = (Sint8 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S8 (using SSE2)");

    /* Get dst aligned to 16 bytes */
    for (i = cvt->len_cvt / sizeof (float); i && (((size_t) dst) & 15); --i, ++src, ++dst) {
        *dst = (Sint8) (*src * 127.0f);
    }

    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128 mulby127 = _mm_set1_ps(127.0f);
        __m128i *mmdst = (__m128i *) dst;
        while (i >= 16) {   /* 16 * float32 */
            const __m128i ints1 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src), mulby127));  /* load 4 floats, convert to sint32 */
            const __m128i ints2 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src+4), mulby127));  /* load 4 floats, convert to sint32 */
            const __m128i ints3 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src+8), mulby127));  /* load 4 floats, convert to sint32 */
            const __m128i ints4 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src+12), mulby127));  /* load 4 floats, convert to sint32 */
            _mm_store_si128(mmdst, _mm_packs_epi16(_mm_packs_epi32(ints1, ints2), _mm_packs_epi32(ints3, ints4)));  /* pack down, store out. */
            i -= 16; src += 16; mmdst++;
        }
        dst = (Sint8 *) mmdst;
    }

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (Sint8) (*src * 127.0f);
        i--; src++; dst++;
    }

    cvt->len_cvt /= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S8);
    }
}

static void SDLCALL
SDL_Convert_F32_to_U8_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Uint8 *dst = (Uint8 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_U8 (using SSE2)");

    /* Get dst aligned to 16 bytes */
    for (i = cvt->len_cvt / sizeof (float); i && (((size_t) dst) & 15); --i, ++src, ++dst) {
        *dst = (Uint8) ((*src + 1.0f) * 127.0f);
    }

    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128 add1 = _mm_set1_ps(1.0f);
        const __m128 mulby127 = _mm_set1_ps(127.0f);
        __m128i *mmdst = (__m128i *) dst;
        while (i >= 16) {   /* 16 * float32 */
            const __m128i ints1 = _mm_cvtps_epi32(_mm_mul_ps(_mm_add_ps(_mm_load_ps(src), add1), mulby127));  /* load 4 floats, convert to sint32 */
            const __m128i ints2 = _mm_cvtps_epi32(_mm_mul_ps(_mm_add_ps(_mm_load_ps(src+4), add1), mulby127));  /* load 4 floats, convert to sint32 */
            const __m128i ints3 = _mm_cvtps_epi32(_mm_mul_ps(_mm_add_ps(_mm_load_ps(src+8), add1), mulby127));  /* load 4 floats, convert to sint32 */
            const __m128i ints4 = _mm_cvtps_epi32(_mm_mul_ps(_mm_add_ps(_mm_load_ps(src+12), add1), mulby127));  /* load 4 floats, convert to sint32 */
            _mm_store_si128(mmdst, _mm_packus_epi16(_mm_packs_epi32(ints1, ints2), _mm_packs_epi32(ints3, ints4)));  /* pack down, store out. */
            i -= 16; src += 16; mmdst++;
        }
        dst = (Uint8 *) mmdst;
    }

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (Uint8) ((*src + 1.0f) * 127.0f);
        i--; src++; dst++;
    }

    cvt->len_cvt /= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_U8);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S16_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint16 *dst = (Sint16 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S16 (using SSE2)");

    /* Get dst aligned to 16 bytes */
    for (i = cvt->len_cvt / sizeof (float); i && (((size_t) dst) & 15); --i, ++src, ++dst) {
        *dst = (Sint16) (*src * 32767.0f);
    }

    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128 mulby32767 = _mm_set1_ps(32767.0f);
        __m128i *mmdst = (__m128i *) dst;
        while (i >= 8) {   /* 8 * float32 */
            const __m128i ints1 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src), mulby32767));  /* load 4 floats, convert to sint32 */
            const __m128i ints2 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src+4), mulby32767));  /* load 4 floats, convert to sint32 */
            _mm_store_si128(mmdst, _mm_packs_epi32(ints1, ints2));  /* pack to sint16, store out. */
            i -= 8; src += 8; mmdst++;
        }
        dst = (Sint16 *) mmdst;
    }

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (Sint16) (*src * 32767.0f);
        i--; src++; dst++;
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S16SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_U16_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Uint16 *dst = (Uint16 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_U16 (using SSE2)");

    /* Get dst aligned to 16 bytes */
    for (i = cvt->len_cvt / sizeof (float); i && (((size_t) dst) & 15); --i, ++src, ++dst) {
        *dst = (Uint16) ((*src + 1.0f) * 32767.0f);
    }

    SDL_assert(!i || ((((size_t) dst) & 15) == 0));

    /* Make sure src is aligned too. */
    if ((((size_t) src) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        /* This calculates differently than the scalar path because SSE2 can't
           pack int32 data down to unsigned int16. _mm_packs_epi32 does signed
           saturation, so that would corrupt our data. _mm_packus_epi32 exists,
           but not before SSE 4.1. So we convert from float to sint16, packing
           that down with legit signed saturation, and then xor the top bit
           against 1. This results in the correct unsigned 16-bit value, even
           though it looks like dark magic. */
        const __m128 mulby32767 = _mm_set1_ps(32767.0f);
        const __m128i topbit = _mm_set1_epi16(-32768);
        __m128i *mmdst = (__m128i *) dst;
        while (i >= 8) {   /* 8 * float32 */
            const __m128i ints1 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src), mulby32767));  /* load 4 floats, convert to sint32 */
            const __m128i ints2 = _mm_cvtps_epi32(_mm_mul_ps(_mm_load_ps(src+4), mulby32767));  /* load 4 floats, convert to sint32 */
            _mm_store_si128(mmdst, _mm_xor_si128(_mm_packs_epi32(ints1, ints2), topbit));  /* pack to sint16, xor top bit, store out. */
            i -= 8; src += 8; mmdst++;
        }
        dst = (Uint16 *) mmdst;
    }

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (Uint16) ((*src + 1.0f) * 32767.0f);
        i--; src++; dst++;
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_U16SYS);
    }
}

static void SDLCALL
SDL_Convert_F32_to_S32_SSE2(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) cvt->buf;
    Sint32 *dst = (Sint32 *) cvt->buf;
    int i;

    LOG_DEBUG_CONVERT("AUDIO_F32", "AUDIO_S32 (using SSE2)");

    /* Get dst aligned to 16 bytes */
    for (i = cvt->len_cvt / sizeof (float); i && (((size_t) dst) & 15); --i, ++src, ++dst) {
        *dst = (Sint32) (((double) *src) * 2147483647.0);
    }

    SDL_assert(!i || ((((size_t) dst) & 15) == 0));
    SDL_assert(!i || ((((size_t) src) & 15) == 0));

    {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128d mulby2147483647 = _mm_set1_pd(2147483647.0);
        __m128i *mmdst = (__m128i *) dst;
        while (i >= 4) {   /* 4 * float32 */
            const __m128 floats = _mm_load_ps(src);
            /* bitshift the whole register over, so _mm_cvtps_pd can read the top floats in the bottom of the vector. */
            const __m128d doubles1 = _mm_mul_pd(_mm_cvtps_pd(_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(floats), 8))), mulby2147483647);
            const __m128d doubles2 = _mm_mul_pd(_mm_cvtps_pd(floats), mulby2147483647);
            _mm_store_si128(mmdst, _mm_or_si128(_mm_slli_si128(_mm_cvtpd_epi32(doubles1), 8), _mm_cvtpd_epi32(doubles2)));
            i -= 4; src += 4; mmdst++;
        }
        dst = (Sint32 *) mmdst;
    }

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (Sint32) (((double) *src) * 2147483647.0);
        i--; src++; dst++;
    }

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, AUDIO_S32SYS);
    }
}
#endif


void SDL_ChooseAudioConverters(void)
{
    static SDL_bool converters_chosen = SDL_FALSE;

    if (converters_chosen) {
        return;
    }

#define SET_CONVERTER_FUNCS(fntype) \
        SDL_Convert_S8_to_F32 = SDL_Convert_S8_to_F32_##fntype; \
        SDL_Convert_U8_to_F32 = SDL_Convert_U8_to_F32_##fntype; \
        SDL_Convert_S16_to_F32 = SDL_Convert_S16_to_F32_##fntype; \
        SDL_Convert_U16_to_F32 = SDL_Convert_U16_to_F32_##fntype; \
        SDL_Convert_S32_to_F32 = SDL_Convert_S32_to_F32_##fntype; \
        SDL_Convert_F32_to_S8 = SDL_Convert_F32_to_S8_##fntype; \
        SDL_Convert_F32_to_U8 = SDL_Convert_F32_to_U8_##fntype; \
        SDL_Convert_F32_to_S16 = SDL_Convert_F32_to_S16_##fntype; \
        SDL_Convert_F32_to_U16 = SDL_Convert_F32_to_U16_##fntype; \
        SDL_Convert_F32_to_S32 = SDL_Convert_F32_to_S32_##fntype; \
        converters_chosen = SDL_TRUE

#if HAVE_SSE2_INTRINSICS
    if (SDL_HasSSE2()) {
        SET_CONVERTER_FUNCS(SSE2);
        return;
    }
#endif

#if NEED_SCALAR_CONVERTER_FALLBACKS
    SET_CONVERTER_FUNCS(Scalar);
#endif

#undef SET_CONVERTER_FUNCS

    SDL_assert(converters_chosen == SDL_TRUE);
}

/* vi: set ts=4 sw=4 expandtab: */
