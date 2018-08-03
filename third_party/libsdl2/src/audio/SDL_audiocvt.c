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

/* Functions for audio drivers to perform runtime conversion of audio format */

/* FIXME: Channel weights when converting from more channels to fewer may need to be adjusted, see https://msdn.microsoft.com/en-us/library/windows/desktop/ff819070(v=vs.85).aspx
*/

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_audio_c.h"

#include "SDL_loadso.h"
#include "SDL_assert.h"
#include "../SDL_dataqueue.h"
#include "SDL_cpuinfo.h"

#define DEBUG_AUDIOSTREAM 0

#ifdef __SSE3__
#define HAVE_SSE3_INTRINSICS 1
#endif

#if HAVE_SSE3_INTRINSICS
/* Convert from stereo to mono. Average left and right. */
static void SDLCALL
SDL_ConvertStereoToMono_SSE3(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    float *dst = (float *) cvt->buf;
    const float *src = dst;
    int i = cvt->len_cvt / 8;

    LOG_DEBUG_CONVERT("stereo", "mono (using SSE3)");
    SDL_assert(format == AUDIO_F32SYS);

    /* We can only do this if dst is aligned to 16 bytes; since src is the
       same pointer and it moves by 2, it can't be forcibly aligned. */
    if ((((size_t) dst) & 15) == 0) {
        /* Aligned! Do SSE blocks as long as we have 16 bytes available. */
        const __m128 divby2 = _mm_set1_ps(0.5f);
        while (i >= 4) {   /* 4 * float32 */
            _mm_store_ps(dst, _mm_mul_ps(_mm_hadd_ps(_mm_load_ps(src), _mm_load_ps(src+4)), divby2));
            i -= 4; src += 8; dst += 4;
        }
    }

    /* Finish off any leftovers with scalar operations. */
    while (i) {
        *dst = (src[0] + src[1]) * 0.5f;
        dst++; i--; src += 2;
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}
#endif

/* Convert from stereo to mono. Average left and right. */
static void SDLCALL
SDL_ConvertStereoToMono(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    float *dst = (float *) cvt->buf;
    const float *src = dst;
    int i;

    LOG_DEBUG_CONVERT("stereo", "mono");
    SDL_assert(format == AUDIO_F32SYS);

    for (i = cvt->len_cvt / 8; i; --i, src += 2) {
        *(dst++) = (src[0] + src[1]) * 0.5f;
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Convert from 5.1 to stereo. Average left and right, distribute center, discard LFE. */
static void SDLCALL
SDL_Convert51ToStereo(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    float *dst = (float *) cvt->buf;
    const float *src = dst;
    int i;

    LOG_DEBUG_CONVERT("5.1", "stereo");
    SDL_assert(format == AUDIO_F32SYS);

    /* SDL's 5.1 layout: FL+FR+FC+LFE+BL+BR */
    for (i = cvt->len_cvt / (sizeof (float) * 6); i; --i, src += 6, dst += 2) {
        const float front_center_distributed = src[2] * 0.5f;
        dst[0] = (src[0] + front_center_distributed + src[4]) / 2.5f;  /* left */
        dst[1] = (src[1] + front_center_distributed + src[5]) / 2.5f;  /* right */
    }

    cvt->len_cvt /= 3;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Convert from quad to stereo. Average left and right. */
static void SDLCALL
SDL_ConvertQuadToStereo(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    float *dst = (float *) cvt->buf;
    const float *src = dst;
    int i;

    LOG_DEBUG_CONVERT("quad", "stereo");
    SDL_assert(format == AUDIO_F32SYS);

    for (i = cvt->len_cvt / (sizeof (float) * 4); i; --i, src += 4, dst += 2) {
        dst[0] = (src[0] + src[2]) * 0.5f; /* left */
        dst[1] = (src[1] + src[3]) * 0.5f; /* right */
    }

    cvt->len_cvt /= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Convert from 7.1 to 5.1. Distribute sides across front and back. */
static void SDLCALL
SDL_Convert71To51(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    float *dst = (float *) cvt->buf;
    const float *src = dst;
    int i;

    LOG_DEBUG_CONVERT("7.1", "5.1");
    SDL_assert(format == AUDIO_F32SYS);

    for (i = cvt->len_cvt / (sizeof (float) * 8); i; --i, src += 8, dst += 6) {
        const float surround_left_distributed = src[6] * 0.5f;
        const float surround_right_distributed = src[7] * 0.5f;
        dst[0] = (src[0] + surround_left_distributed) / 1.5f;  /* FL */
        dst[1] = (src[1] + surround_right_distributed) / 1.5f;  /* FR */
        dst[2] = src[2] / 1.5f; /* CC */
        dst[3] = src[3] / 1.5f; /* LFE */
        dst[4] = (src[4] + surround_left_distributed) / 1.5f;  /* BL */
        dst[5] = (src[5] + surround_right_distributed) / 1.5f;  /* BR */
    }

    cvt->len_cvt /= 8;
    cvt->len_cvt *= 6;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Convert from 5.1 to quad. Distribute center across front, discard LFE. */
static void SDLCALL
SDL_Convert51ToQuad(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    float *dst = (float *) cvt->buf;
    const float *src = dst;
    int i;

    LOG_DEBUG_CONVERT("5.1", "quad");
    SDL_assert(format == AUDIO_F32SYS);

    /* SDL's 4.0 layout: FL+FR+BL+BR */
    /* SDL's 5.1 layout: FL+FR+FC+LFE+BL+BR */
    for (i = cvt->len_cvt / (sizeof (float) * 6); i; --i, src += 6, dst += 4) {
        const float front_center_distributed = src[2] * 0.5f;
        dst[0] = (src[0] + front_center_distributed) / 1.5f;  /* FL */
        dst[1] = (src[1] + front_center_distributed) / 1.5f;  /* FR */
        dst[2] = src[4] / 1.5f;  /* BL */
        dst[3] = src[5] / 1.5f;  /* BR */
    }

    cvt->len_cvt /= 6;
    cvt->len_cvt *= 4;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Upmix mono to stereo (by duplication) */
static void SDLCALL
SDL_ConvertMonoToStereo(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) (cvt->buf + cvt->len_cvt);
    float *dst = (float *) (cvt->buf + cvt->len_cvt * 2);
    int i;

    LOG_DEBUG_CONVERT("mono", "stereo");
    SDL_assert(format == AUDIO_F32SYS);

    for (i = cvt->len_cvt / sizeof (float); i; --i) {
        src--;
        dst -= 2;
        dst[0] = dst[1] = *src;
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Upmix stereo to a pseudo-5.1 stream */
static void SDLCALL
SDL_ConvertStereoTo51(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    int i;
    float lf, rf, ce;
    const float *src = (const float *) (cvt->buf + cvt->len_cvt);
    float *dst = (float *) (cvt->buf + cvt->len_cvt * 3);

    LOG_DEBUG_CONVERT("stereo", "5.1");
    SDL_assert(format == AUDIO_F32SYS);

    for (i = cvt->len_cvt / (sizeof(float) * 2); i; --i) {
        dst -= 6;
        src -= 2;
        lf = src[0];
        rf = src[1];
        ce = (lf + rf) * 0.5f;
        /* !!! FIXME: FL and FR may clip */
        dst[0] = lf + (lf - ce);  /* FL */
        dst[1] = rf + (rf - ce);  /* FR */
        dst[2] = ce;  /* FC */
        dst[3] = 0;   /* LFE (only meant for special LFE effects) */
        dst[4] = lf;  /* BL */
        dst[5] = rf;  /* BR */
    }

    cvt->len_cvt *= 3;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Upmix quad to a pseudo-5.1 stream */
static void SDLCALL
SDL_ConvertQuadTo51(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    int i;
    float lf, rf, lb, rb, ce;
    const float *src = (const float *) (cvt->buf + cvt->len_cvt);
    float *dst = (float *) (cvt->buf + cvt->len_cvt * 3 / 2);

    LOG_DEBUG_CONVERT("quad", "5.1");
    SDL_assert(format == AUDIO_F32SYS);
    SDL_assert(cvt->len_cvt % (sizeof(float) * 4) == 0);

    for (i = cvt->len_cvt / (sizeof(float) * 4); i; --i) {
        dst -= 6;
        src -= 4;
        lf = src[0];
        rf = src[1];
        lb = src[2];
        rb = src[3];
        ce = (lf + rf) * 0.5f;
        /* !!! FIXME: FL and FR may clip */
        dst[0] = lf + (lf - ce);  /* FL */
        dst[1] = rf + (rf - ce);  /* FR */
        dst[2] = ce;  /* FC */
        dst[3] = 0;   /* LFE (only meant for special LFE effects) */
        dst[4] = lb;  /* BL */
        dst[5] = rb;  /* BR */
    }

    cvt->len_cvt = cvt->len_cvt * 3 / 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Upmix stereo to a pseudo-4.0 stream (by duplication) */
static void SDLCALL
SDL_ConvertStereoToQuad(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    const float *src = (const float *) (cvt->buf + cvt->len_cvt);
    float *dst = (float *) (cvt->buf + cvt->len_cvt * 2);
    float lf, rf;
    int i;

    LOG_DEBUG_CONVERT("stereo", "quad");
    SDL_assert(format == AUDIO_F32SYS);

    for (i = cvt->len_cvt / (sizeof(float) * 2); i; --i) {
        dst -= 4;
        src -= 2;
        lf = src[0];
        rf = src[1];
        dst[0] = lf;  /* FL */
        dst[1] = rf;  /* FR */
        dst[2] = lf;  /* BL */
        dst[3] = rf;  /* BR */
    }

    cvt->len_cvt *= 2;
    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}


/* Upmix 5.1 to 7.1 */
static void SDLCALL
SDL_Convert51To71(SDL_AudioCVT * cvt, SDL_AudioFormat format)
{
    float lf, rf, lb, rb, ls, rs;
    int i;
    const float *src = (const float *) (cvt->buf + cvt->len_cvt);
    float *dst = (float *) (cvt->buf + cvt->len_cvt * 4 / 3);

    LOG_DEBUG_CONVERT("5.1", "7.1");
    SDL_assert(format == AUDIO_F32SYS);
    SDL_assert(cvt->len_cvt % (sizeof(float) * 6) == 0);

    for (i = cvt->len_cvt / (sizeof(float) * 6); i; --i) {
        dst -= 8;
        src -= 6;
        lf = src[0];
        rf = src[1];
        lb = src[4];
        rb = src[5];
        ls = (lf + lb) * 0.5f;
        rs = (rf + rb) * 0.5f;
        /* !!! FIXME: these four may clip */
        lf += lf - ls;
        rf += rf - ls;
        lb += lb - ls;
        rb += rb - ls;
        dst[3] = src[3];  /* LFE */
        dst[2] = src[2];  /* FC */
        dst[7] = rs; /* SR */
        dst[6] = ls; /* SL */
        dst[5] = rb;  /* BR */
        dst[4] = lb;  /* BL */
        dst[1] = rf;  /* FR */
        dst[0] = lf;  /* FL */
    }

    cvt->len_cvt = cvt->len_cvt * 4 / 3;

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index] (cvt, format);
    }
}

/* SDL's resampler uses a "bandlimited interpolation" algorithm:
     https://ccrma.stanford.edu/~jos/resample/ */

#define RESAMPLER_ZERO_CROSSINGS 5
#define RESAMPLER_BITS_PER_SAMPLE 16
#define RESAMPLER_SAMPLES_PER_ZERO_CROSSING  (1 << ((RESAMPLER_BITS_PER_SAMPLE / 2) + 1))
#define RESAMPLER_FILTER_SIZE ((RESAMPLER_SAMPLES_PER_ZERO_CROSSING * RESAMPLER_ZERO_CROSSINGS) + 1)

/* This is a "modified" bessel function, so you can't use POSIX j0() */
static double
bessel(const double x)
{
    const double xdiv2 = x / 2.0;
    double i0 = 1.0f;
    double f = 1.0f;
    int i = 1;

    while (SDL_TRUE) {
        const double diff = SDL_pow(xdiv2, i * 2) / SDL_pow(f, 2);
        if (diff < 1.0e-21f) {
            break;
        }
        i0 += diff;
        i++;
        f *= (double) i;
    }

    return i0;
}

/* build kaiser table with cardinal sine applied to it, and array of differences between elements. */
static void
kaiser_and_sinc(float *table, float *diffs, const int tablelen, const double beta)
{
    const int lenm1 = tablelen - 1;
    const int lenm1div2 = lenm1 / 2;
    int i;

    table[0] = 1.0f;
    for (i = 1; i < tablelen; i++) {
        const double kaiser = bessel(beta * SDL_sqrt(1.0 - SDL_pow(((i - lenm1) / 2.0) / lenm1div2, 2.0))) / bessel(beta);
        table[tablelen - i] = (float) kaiser;
    }

    for (i = 1; i < tablelen; i++) {
        const float x = (((float) i) / ((float) RESAMPLER_SAMPLES_PER_ZERO_CROSSING)) * ((float) M_PI);
        table[i] *= SDL_sinf(x) / x;
        diffs[i - 1] = table[i] - table[i - 1];
    }
    diffs[lenm1] = 0.0f;
}


static SDL_SpinLock ResampleFilterSpinlock = 0;
static float *ResamplerFilter = NULL;
static float *ResamplerFilterDifference = NULL;

int
SDL_PrepareResampleFilter(void)
{
    SDL_AtomicLock(&ResampleFilterSpinlock);
    if (!ResamplerFilter) {
        /* if dB > 50, beta=(0.1102 * (dB - 8.7)), according to Matlab. */
        const double dB = 80.0;
        const double beta = 0.1102 * (dB - 8.7);
        const size_t alloclen = RESAMPLER_FILTER_SIZE * sizeof (float);

        ResamplerFilter = (float *) SDL_malloc(alloclen);
        if (!ResamplerFilter) {
            SDL_AtomicUnlock(&ResampleFilterSpinlock);
            return SDL_OutOfMemory();
        }

        ResamplerFilterDifference = (float *) SDL_malloc(alloclen);
        if (!ResamplerFilterDifference) {
            SDL_free(ResamplerFilter);
            ResamplerFilter = NULL;
            SDL_AtomicUnlock(&ResampleFilterSpinlock);
            return SDL_OutOfMemory();
        }
        kaiser_and_sinc(ResamplerFilter, ResamplerFilterDifference, RESAMPLER_FILTER_SIZE, beta);
    }
    SDL_AtomicUnlock(&ResampleFilterSpinlock);
    return 0;
}

void
SDL_FreeResampleFilter(void)
{
    SDL_free(ResamplerFilter);
    SDL_free(ResamplerFilterDifference);
    ResamplerFilter = NULL;
    ResamplerFilterDifference = NULL;
}

static int
ResamplerPadding(const int inrate, const int outrate)
{
    if (inrate == outrate) {
        return 0;
    } else if (inrate > outrate) {
        return (int) SDL_ceil(((float) (RESAMPLER_SAMPLES_PER_ZERO_CROSSING * inrate) / ((float) outrate)));
    }
    return RESAMPLER_SAMPLES_PER_ZERO_CROSSING;
}

/* lpadding and rpadding are expected to be buffers of (ResamplePadding(inrate, outrate) * chans * sizeof (float)) bytes. */
static int
SDL_ResampleAudio(const int chans, const int inrate, const int outrate,
                        const float *lpadding, const float *rpadding,
                        const float *inbuf, const int inbuflen,
                        float *outbuf, const int outbuflen)
{
    const double finrate = (double) inrate;
    const double outtimeincr = 1.0 / ((float) outrate);
    const double  ratio = ((float) outrate) / ((float) inrate);
    const int paddinglen = ResamplerPadding(inrate, outrate);
    const int framelen = chans * (int)sizeof (float);
    const int inframes = inbuflen / framelen;
    const int wantedoutframes = (int) ((inbuflen / framelen) * ratio);  /* outbuflen isn't total to write, it's total available. */
    const int maxoutframes = outbuflen / framelen;
    const int outframes = SDL_min(wantedoutframes, maxoutframes);
    float *dst = outbuf;
    double outtime = 0.0;
    int i, j, chan;

    for (i = 0; i < outframes; i++) {
        const int srcindex = (int) (outtime * inrate);
        const double intime = ((double) srcindex) / finrate;
        const double innexttime = ((double) (srcindex + 1)) / finrate;
        const double interpolation1 = 1.0 - ((innexttime - outtime) / (innexttime - intime));
        const int filterindex1 = (int) (interpolation1 * RESAMPLER_SAMPLES_PER_ZERO_CROSSING);
        const double interpolation2 = 1.0 - interpolation1;
        const int filterindex2 = (int) (interpolation2 * RESAMPLER_SAMPLES_PER_ZERO_CROSSING);

        for (chan = 0; chan < chans; chan++) {
            float outsample = 0.0f;

            /* do this twice to calculate the sample, once for the "left wing" and then same for the right. */
            /* !!! FIXME: do both wings in one loop */
            for (j = 0; (filterindex1 + (j * RESAMPLER_SAMPLES_PER_ZERO_CROSSING)) < RESAMPLER_FILTER_SIZE; j++) {
                const int srcframe = srcindex - j;
                /* !!! FIXME: we can bubble this conditional out of here by doing a pre loop. */
                const float insample = (srcframe < 0) ? lpadding[((paddinglen + srcframe) * chans) + chan] : inbuf[(srcframe * chans) + chan];
                outsample += (float)(insample * (ResamplerFilter[filterindex1 + (j * RESAMPLER_SAMPLES_PER_ZERO_CROSSING)] + (interpolation1 * ResamplerFilterDifference[filterindex1 + (j * RESAMPLER_SAMPLES_PER_ZERO_CROSSING)])));
            }

            for (j = 0; (filterindex2 + (j * RESAMPLER_SAMPLES_PER_ZERO_CROSSING)) < RESAMPLER_FILTER_SIZE; j++) {
                const int srcframe = srcindex + 1 + j;
                /* !!! FIXME: we can bubble this conditional out of here by doing a post loop. */
                const float insample = (srcframe >= inframes) ? rpadding[((srcframe - inframes) * chans) + chan] : inbuf[(srcframe * chans) + chan];
                outsample += (float)(insample * (ResamplerFilter[filterindex2 + (j * RESAMPLER_SAMPLES_PER_ZERO_CROSSING)] + (interpolation2 * ResamplerFilterDifference[filterindex2 + (j * RESAMPLER_SAMPLES_PER_ZERO_CROSSING)])));
            }
            *(dst++) = outsample;
        }

        outtime += outtimeincr;
    }

    return outframes * chans * sizeof (float);
}

int
SDL_ConvertAudio(SDL_AudioCVT * cvt)
{
    /* !!! FIXME: (cvt) should be const; stack-copy it here. */
    /* !!! FIXME: (actually, we can't...len_cvt needs to be updated. Grr.) */

    /* Make sure there's data to convert */
    if (cvt->buf == NULL) {
        return SDL_SetError("No buffer allocated for conversion");
    }

    /* Return okay if no conversion is necessary */
    cvt->len_cvt = cvt->len;
    if (cvt->filters[0] == NULL) {
        return 0;
    }

    /* Set up the conversion and go! */
    cvt->filter_index = 0;
    cvt->filters[0] (cvt, cvt->src_format);
    return 0;
}

static void SDLCALL
SDL_Convert_Byteswap(SDL_AudioCVT *cvt, SDL_AudioFormat format)
{
#if DEBUG_CONVERT
    printf("Converting byte order\n");
#endif

    switch (SDL_AUDIO_BITSIZE(format)) {
        #define CASESWAP(b) \
            case b: { \
                Uint##b *ptr = (Uint##b *) cvt->buf; \
                int i; \
                for (i = cvt->len_cvt / sizeof (*ptr); i; --i, ++ptr) { \
                    *ptr = SDL_Swap##b(*ptr); \
                } \
                break; \
            }

        CASESWAP(16);
        CASESWAP(32);
        CASESWAP(64);

        #undef CASESWAP

        default: SDL_assert(!"unhandled byteswap datatype!"); break;
    }

    if (cvt->filters[++cvt->filter_index]) {
        /* flip endian flag for data. */
        if (format & SDL_AUDIO_MASK_ENDIAN) {
            format &= ~SDL_AUDIO_MASK_ENDIAN;
        } else {
            format |= SDL_AUDIO_MASK_ENDIAN;
        }
        cvt->filters[cvt->filter_index](cvt, format);
    }
}

static int
SDL_AddAudioCVTFilter(SDL_AudioCVT *cvt, const SDL_AudioFilter filter)
{
    if (cvt->filter_index >= SDL_AUDIOCVT_MAX_FILTERS) {
        return SDL_SetError("Too many filters needed for conversion, exceeded maximum of %d", SDL_AUDIOCVT_MAX_FILTERS);
    }
    if (filter == NULL) {
        return SDL_SetError("Audio filter pointer is NULL");
    }
    cvt->filters[cvt->filter_index++] = filter;
    cvt->filters[cvt->filter_index] = NULL; /* Moving terminator */
    return 0;
}

static int
SDL_BuildAudioTypeCVTToFloat(SDL_AudioCVT *cvt, const SDL_AudioFormat src_fmt)
{
    int retval = 0;  /* 0 == no conversion necessary. */

    if ((SDL_AUDIO_ISBIGENDIAN(src_fmt) != 0) == (SDL_BYTEORDER == SDL_LIL_ENDIAN)) {
        if (SDL_AddAudioCVTFilter(cvt, SDL_Convert_Byteswap) < 0) {
            return -1;
        }
        retval = 1;  /* added a converter. */
    }

    if (!SDL_AUDIO_ISFLOAT(src_fmt)) {
        const Uint16 src_bitsize = SDL_AUDIO_BITSIZE(src_fmt);
        const Uint16 dst_bitsize = 32;
        SDL_AudioFilter filter = NULL;

        switch (src_fmt & ~SDL_AUDIO_MASK_ENDIAN) {
            case AUDIO_S8: filter = SDL_Convert_S8_to_F32; break;
            case AUDIO_U8: filter = SDL_Convert_U8_to_F32; break;
            case AUDIO_S16: filter = SDL_Convert_S16_to_F32; break;
            case AUDIO_U16: filter = SDL_Convert_U16_to_F32; break;
            case AUDIO_S32: filter = SDL_Convert_S32_to_F32; break;
            default: SDL_assert(!"Unexpected audio format!"); break;
        }

        if (!filter) {
            return SDL_SetError("No conversion from source format to float available");
        }

        if (SDL_AddAudioCVTFilter(cvt, filter) < 0) {
            return -1;
        }
        if (src_bitsize < dst_bitsize) {
            const int mult = (dst_bitsize / src_bitsize);
            cvt->len_mult *= mult;
            cvt->len_ratio *= mult;
        } else if (src_bitsize > dst_bitsize) {
            cvt->len_ratio /= (src_bitsize / dst_bitsize);
        }

        retval = 1;  /* added a converter. */
    }

    return retval;
}

static int
SDL_BuildAudioTypeCVTFromFloat(SDL_AudioCVT *cvt, const SDL_AudioFormat dst_fmt)
{
    int retval = 0;  /* 0 == no conversion necessary. */

    if (!SDL_AUDIO_ISFLOAT(dst_fmt)) {
        const Uint16 dst_bitsize = SDL_AUDIO_BITSIZE(dst_fmt);
        const Uint16 src_bitsize = 32;
        SDL_AudioFilter filter = NULL;
        switch (dst_fmt & ~SDL_AUDIO_MASK_ENDIAN) {
            case AUDIO_S8: filter = SDL_Convert_F32_to_S8; break;
            case AUDIO_U8: filter = SDL_Convert_F32_to_U8; break;
            case AUDIO_S16: filter = SDL_Convert_F32_to_S16; break;
            case AUDIO_U16: filter = SDL_Convert_F32_to_U16; break;
            case AUDIO_S32: filter = SDL_Convert_F32_to_S32; break;
            default: SDL_assert(!"Unexpected audio format!"); break;
        }

        if (!filter) {
            return SDL_SetError("No conversion from float to destination format available");
        }

        if (SDL_AddAudioCVTFilter(cvt, filter) < 0) {
            return -1;
        }
        if (src_bitsize < dst_bitsize) {
            const int mult = (dst_bitsize / src_bitsize);
            cvt->len_mult *= mult;
            cvt->len_ratio *= mult;
        } else if (src_bitsize > dst_bitsize) {
            cvt->len_ratio /= (src_bitsize / dst_bitsize);
        }
        retval = 1;  /* added a converter. */
    }

    if ((SDL_AUDIO_ISBIGENDIAN(dst_fmt) != 0) == (SDL_BYTEORDER == SDL_LIL_ENDIAN)) {
        if (SDL_AddAudioCVTFilter(cvt, SDL_Convert_Byteswap) < 0) {
            return -1;
        }
        retval = 1;  /* added a converter. */
    }

    return retval;
}

static void
SDL_ResampleCVT(SDL_AudioCVT *cvt, const int chans, const SDL_AudioFormat format)
{
    /* !!! FIXME in 2.1: there are ten slots in the filter list, and the theoretical maximum we use is six (seven with NULL terminator).
       !!! FIXME in 2.1:   We need to store data for this resampler, because the cvt structure doesn't store the original sample rates,
       !!! FIXME in 2.1:   so we steal the ninth and tenth slot.  :( */
    const int inrate = (int) (size_t) cvt->filters[SDL_AUDIOCVT_MAX_FILTERS-1];
    const int outrate = (int) (size_t) cvt->filters[SDL_AUDIOCVT_MAX_FILTERS];
    const float *src = (const float *) cvt->buf;
    const int srclen = cvt->len_cvt;
    /*float *dst = (float *) cvt->buf;
    const int dstlen = (cvt->len * cvt->len_mult);*/
    /* !!! FIXME: remove this if we can get the resampler to work in-place again. */
    float *dst = (float *) (cvt->buf + srclen);
    const int dstlen = (cvt->len * cvt->len_mult) - srclen;
    const int paddingsamples = (ResamplerPadding(inrate, outrate) * chans);
    float *padding;

    SDL_assert(format == AUDIO_F32SYS);

    /* we keep no streaming state here, so pad with silence on both ends. */
    padding = (float *) SDL_calloc(paddingsamples, sizeof (float));
    if (!padding) {
        SDL_OutOfMemory();
        return;
    }

    cvt->len_cvt = SDL_ResampleAudio(chans, inrate, outrate, padding, padding, src, srclen, dst, dstlen);

    SDL_free(padding);

    SDL_memmove(cvt->buf, dst, cvt->len_cvt);  /* !!! FIXME: remove this if we can get the resampler to work in-place again. */

    if (cvt->filters[++cvt->filter_index]) {
        cvt->filters[cvt->filter_index](cvt, format);
    }
}

/* !!! FIXME: We only have this macro salsa because SDL_AudioCVT doesn't
   !!! FIXME:  store channel info, so we have to have function entry
   !!! FIXME:  points for each supported channel count and multiple
   !!! FIXME:  vs arbitrary. When we rev the ABI, clean this up. */
#define RESAMPLER_FUNCS(chans) \
    static void SDLCALL \
    SDL_ResampleCVT_c##chans(SDL_AudioCVT *cvt, SDL_AudioFormat format) { \
        SDL_ResampleCVT(cvt, chans, format); \
    }
RESAMPLER_FUNCS(1)
RESAMPLER_FUNCS(2)
RESAMPLER_FUNCS(4)
RESAMPLER_FUNCS(6)
RESAMPLER_FUNCS(8)
#undef RESAMPLER_FUNCS

static SDL_AudioFilter
ChooseCVTResampler(const int dst_channels)
{
    switch (dst_channels) {
        case 1: return SDL_ResampleCVT_c1;
        case 2: return SDL_ResampleCVT_c2;
        case 4: return SDL_ResampleCVT_c4;
        case 6: return SDL_ResampleCVT_c6;
        case 8: return SDL_ResampleCVT_c8;
        default: break;
    }

    return NULL;
}

static int
SDL_BuildAudioResampleCVT(SDL_AudioCVT * cvt, const int dst_channels,
                          const int src_rate, const int dst_rate)
{
    SDL_AudioFilter filter;

    if (src_rate == dst_rate) {
        return 0;  /* no conversion necessary. */
    }

    filter = ChooseCVTResampler(dst_channels);
    if (filter == NULL) {
        return SDL_SetError("No conversion available for these rates");
    }

    if (SDL_PrepareResampleFilter() < 0) {
        return -1;
    }

    /* Update (cvt) with filter details... */
    if (SDL_AddAudioCVTFilter(cvt, filter) < 0) {
        return -1;
    }

    /* !!! FIXME in 2.1: there are ten slots in the filter list, and the theoretical maximum we use is six (seven with NULL terminator).
       !!! FIXME in 2.1:   We need to store data for this resampler, because the cvt structure doesn't store the original sample rates,
       !!! FIXME in 2.1:   so we steal the ninth and tenth slot.  :( */
    if (cvt->filter_index >= (SDL_AUDIOCVT_MAX_FILTERS-2)) {
        return SDL_SetError("Too many filters needed for conversion, exceeded maximum of %d", SDL_AUDIOCVT_MAX_FILTERS-2);
    }
    cvt->filters[SDL_AUDIOCVT_MAX_FILTERS-1] = (SDL_AudioFilter) (size_t) src_rate;
    cvt->filters[SDL_AUDIOCVT_MAX_FILTERS] = (SDL_AudioFilter) (size_t) dst_rate;

    if (src_rate < dst_rate) {
        const double mult = ((double) dst_rate) / ((double) src_rate);
        cvt->len_mult *= (int) SDL_ceil(mult);
        cvt->len_ratio *= mult;
    } else {
        cvt->len_ratio /= ((double) src_rate) / ((double) dst_rate);
    }

    /* !!! FIXME: remove this if we can get the resampler to work in-place again. */
    /* the buffer is big enough to hold the destination now, but
       we need it large enough to hold a separate scratch buffer. */
    cvt->len_mult *= 2;

    return 1;               /* added a converter. */
}

static SDL_bool
SDL_SupportedAudioFormat(const SDL_AudioFormat fmt)
{
    switch (fmt) {
        case AUDIO_U8:
        case AUDIO_S8:
        case AUDIO_U16LSB:
        case AUDIO_S16LSB:
        case AUDIO_U16MSB:
        case AUDIO_S16MSB:
        case AUDIO_S32LSB:
        case AUDIO_S32MSB:
        case AUDIO_F32LSB:
        case AUDIO_F32MSB:
            return SDL_TRUE;  /* supported. */

        default:
            break;
    }

    return SDL_FALSE;  /* unsupported. */
}

static SDL_bool
SDL_SupportedChannelCount(const int channels)
{
    switch (channels) {
        case 1:  /* mono */
        case 2:  /* stereo */
        case 4:  /* quad */
        case 6:  /* 5.1 */
        case 8:  /* 7.1 */
          return SDL_TRUE;  /* supported. */

        default:
            break;
    }

    return SDL_FALSE;  /* unsupported. */
}


/* Creates a set of audio filters to convert from one format to another.
   Returns 0 if no conversion is needed, 1 if the audio filter is set up,
   or -1 if an error like invalid parameter, unsupported format, etc. occurred.
*/

int
SDL_BuildAudioCVT(SDL_AudioCVT * cvt,
                  SDL_AudioFormat src_fmt, Uint8 src_channels, int src_rate,
                  SDL_AudioFormat dst_fmt, Uint8 dst_channels, int dst_rate)
{
    /* Sanity check target pointer */
    if (cvt == NULL) {
        return SDL_InvalidParamError("cvt");
    }

    /* Make sure we zero out the audio conversion before error checking */
    SDL_zerop(cvt);

    if (!SDL_SupportedAudioFormat(src_fmt)) {
        return SDL_SetError("Invalid source format");
    } else if (!SDL_SupportedAudioFormat(dst_fmt)) {
        return SDL_SetError("Invalid destination format");
    } else if (!SDL_SupportedChannelCount(src_channels)) {
        return SDL_SetError("Invalid source channels");
    } else if (!SDL_SupportedChannelCount(dst_channels)) {
        return SDL_SetError("Invalid destination channels");
    } else if (src_rate == 0) {
        return SDL_SetError("Source rate is zero");
    } else if (dst_rate == 0) {
        return SDL_SetError("Destination rate is zero");
    }

#if DEBUG_CONVERT
    printf("Build format %04x->%04x, channels %u->%u, rate %d->%d\n",
           src_fmt, dst_fmt, src_channels, dst_channels, src_rate, dst_rate);
#endif

    /* Start off with no conversion necessary */
    cvt->src_format = src_fmt;
    cvt->dst_format = dst_fmt;
    cvt->needed = 0;
    cvt->filter_index = 0;
    SDL_zero(cvt->filters);
    cvt->len_mult = 1;
    cvt->len_ratio = 1.0;
    cvt->rate_incr = ((double) dst_rate) / ((double) src_rate);

    /* Make sure we've chosen audio conversion functions (MMX, scalar, etc.) */
    SDL_ChooseAudioConverters();

    /* Type conversion goes like this now:
        - byteswap to CPU native format first if necessary.
        - convert to native Float32 if necessary.
        - resample and change channel count if necessary.
        - convert back to native format.
        - byteswap back to foreign format if necessary.

       The expectation is we can process data faster in float32
       (possibly with SIMD), and making several passes over the same
       buffer is likely to be CPU cache-friendly, avoiding the
       biggest performance hit in modern times. Previously we had
       (script-generated) custom converters for every data type and
       it was a bloat on SDL compile times and final library size. */

    /* see if we can skip float conversion entirely. */
    if (src_rate == dst_rate && src_channels == dst_channels) {
        if (src_fmt == dst_fmt) {
            return 0;
        }

        /* just a byteswap needed? */
        if ((src_fmt & ~SDL_AUDIO_MASK_ENDIAN) == (dst_fmt & ~SDL_AUDIO_MASK_ENDIAN)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_Convert_Byteswap) < 0) {
                return -1;
            }
            cvt->needed = 1;
            return 1;
        }
    }

    /* Convert data types, if necessary. Updates (cvt). */
    if (SDL_BuildAudioTypeCVTToFloat(cvt, src_fmt) < 0) {
        return -1;              /* shouldn't happen, but just in case... */
    }

    /* Channel conversion */
    if (src_channels < dst_channels) {
        /* Upmixing */
        /* Mono -> Stereo [-> ...] */
        if ((src_channels == 1) && (dst_channels > 1)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_ConvertMonoToStereo) < 0) {
                return -1;
            }
            cvt->len_mult *= 2;
            src_channels = 2;
            cvt->len_ratio *= 2;
        }
        /* [Mono ->] Stereo -> 5.1 [-> 7.1] */
        if ((src_channels == 2) && (dst_channels >= 6)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_ConvertStereoTo51) < 0) {
                return -1;
            }
            src_channels = 6;
            cvt->len_mult *= 3;
            cvt->len_ratio *= 3;
        }
        /* Quad -> 5.1 [-> 7.1] */
        if ((src_channels == 4) && (dst_channels >= 6)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_ConvertQuadTo51) < 0) {
                return -1;
            }
            src_channels = 6;
            cvt->len_mult = (cvt->len_mult * 3 + 1) / 2;
            cvt->len_ratio *= 1.5;
        }
        /* [[Mono ->] Stereo ->] 5.1 -> 7.1 */
        if ((src_channels == 6) && (dst_channels == 8)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_Convert51To71) < 0) {
                return -1;
            }
            src_channels = 8;
            cvt->len_mult = (cvt->len_mult * 4 + 2) / 3;
            /* Should be numerically exact with every valid input to this
               function */
            cvt->len_ratio = cvt->len_ratio * 4 / 3;
        }
        /* [Mono ->] Stereo -> Quad */
        if ((src_channels == 2) && (dst_channels == 4)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_ConvertStereoToQuad) < 0) {
                return -1;
            }
            src_channels = 4;
            cvt->len_mult *= 2;
            cvt->len_ratio *= 2;
        }
    } else if (src_channels > dst_channels) {
        /* Downmixing */
        /* 7.1 -> 5.1 [-> Stereo [-> Mono]] */
        /* 7.1 -> 5.1 [-> Quad] */
        if ((src_channels == 8) && (dst_channels <= 6)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_Convert71To51) < 0) {
                return -1;
            }
            src_channels = 6;
            cvt->len_ratio *= 0.75;
        }
        /* [7.1 ->] 5.1 -> Stereo [-> Mono] */
        if ((src_channels == 6) && (dst_channels <= 2)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_Convert51ToStereo) < 0) {
                return -1;
            }
            src_channels = 2;
            cvt->len_ratio /= 3;
        }
        /* 5.1 -> Quad */
        if ((src_channels == 6) && (dst_channels == 4)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_Convert51ToQuad) < 0) {
                return -1;
            }
            src_channels = 4;
            cvt->len_ratio = cvt->len_ratio * 2 / 3;
        }
        /* Quad -> Stereo [-> Mono] */
        if ((src_channels == 4) && (dst_channels <= 2)) {
            if (SDL_AddAudioCVTFilter(cvt, SDL_ConvertQuadToStereo) < 0) {
                return -1;
            }
            src_channels = 2;
            cvt->len_ratio /= 2;
        }
        /* [... ->] Stereo -> Mono */
        if ((src_channels == 2) && (dst_channels == 1)) {
            SDL_AudioFilter filter = NULL;

            #if HAVE_SSE3_INTRINSICS
            if (SDL_HasSSE3()) {
                filter = SDL_ConvertStereoToMono_SSE3;
            }
            #endif

            if (!filter) {
                filter = SDL_ConvertStereoToMono;
            }

            if (SDL_AddAudioCVTFilter(cvt, filter) < 0) {
                return -1;
            }

            src_channels = 1;
            cvt->len_ratio /= 2;
        }
    }

    if (src_channels != dst_channels) {
        /* All combinations of supported channel counts should have been
           handled by now, but let's be defensive */
      return SDL_SetError("Invalid channel combination");
    }
    
    /* Do rate conversion, if necessary. Updates (cvt). */
    if (SDL_BuildAudioResampleCVT(cvt, dst_channels, src_rate, dst_rate) < 0) {
        return -1;              /* shouldn't happen, but just in case... */
    }

    /* Move to final data type. */
    if (SDL_BuildAudioTypeCVTFromFloat(cvt, dst_fmt) < 0) {
        return -1;              /* shouldn't happen, but just in case... */
    }

    cvt->needed = (cvt->filter_index != 0);
    return (cvt->needed);
}

typedef int (*SDL_ResampleAudioStreamFunc)(SDL_AudioStream *stream, const void *inbuf, const int inbuflen, void *outbuf, const int outbuflen);
typedef void (*SDL_ResetAudioStreamResamplerFunc)(SDL_AudioStream *stream);
typedef void (*SDL_CleanupAudioStreamResamplerFunc)(SDL_AudioStream *stream);

struct _SDL_AudioStream
{
    SDL_AudioCVT cvt_before_resampling;
    SDL_AudioCVT cvt_after_resampling;
    SDL_DataQueue *queue;
    SDL_bool first_run;
    Uint8 *staging_buffer;
    int staging_buffer_size;
    int staging_buffer_filled;
    Uint8 *work_buffer_base;  /* maybe unaligned pointer from SDL_realloc(). */
    int work_buffer_len;
    int src_sample_frame_size;
    SDL_AudioFormat src_format;
    Uint8 src_channels;
    int src_rate;
    int dst_sample_frame_size;
    SDL_AudioFormat dst_format;
    Uint8 dst_channels;
    int dst_rate;
    double rate_incr;
    Uint8 pre_resample_channels;
    int packetlen;
    int resampler_padding_samples;
    float *resampler_padding;
    void *resampler_state;
    SDL_ResampleAudioStreamFunc resampler_func;
    SDL_ResetAudioStreamResamplerFunc reset_resampler_func;
    SDL_CleanupAudioStreamResamplerFunc cleanup_resampler_func;
};

static Uint8 *
EnsureStreamBufferSize(SDL_AudioStream *stream, const int newlen)
{
    Uint8 *ptr;
    size_t offset;

    if (stream->work_buffer_len >= newlen) {
        ptr = stream->work_buffer_base;
    } else {
        ptr = (Uint8 *) SDL_realloc(stream->work_buffer_base, newlen + 32);
        if (!ptr) {
            SDL_OutOfMemory();
            return NULL;
        }
        /* Make sure we're aligned to 16 bytes for SIMD code. */
        stream->work_buffer_base = ptr;
        stream->work_buffer_len = newlen;
    }

    offset = ((size_t) ptr) & 15;
    return offset ? ptr + (16 - offset) : ptr;
}

#ifdef HAVE_LIBSAMPLERATE_H
static int
SDL_ResampleAudioStream_SRC(SDL_AudioStream *stream, const void *_inbuf, const int inbuflen, void *_outbuf, const int outbuflen)
{
    const float *inbuf = (const float *) _inbuf;
    float *outbuf = (float *) _outbuf;
    const int framelen = sizeof(float) * stream->pre_resample_channels;
    SRC_STATE *state = (SRC_STATE *)stream->resampler_state;
    SRC_DATA data;
    int result;

    SDL_assert(inbuf != ((const float *) outbuf));  /* SDL_AudioStreamPut() shouldn't allow in-place resamples. */

    data.data_in = (float *)inbuf; /* Older versions of libsamplerate had a non-const pointer, but didn't write to it */
    data.input_frames = inbuflen / framelen;
    data.input_frames_used = 0;

    data.data_out = outbuf;
    data.output_frames = outbuflen / framelen;

    data.end_of_input = 0;
    data.src_ratio = stream->rate_incr;

    result = SRC_src_process(state, &data);
    if (result != 0) {
        SDL_SetError("src_process() failed: %s", SRC_src_strerror(result));
        return 0;
    }

    /* If this fails, we need to store them off somewhere */
    SDL_assert(data.input_frames_used == data.input_frames);

    return data.output_frames_gen * (sizeof(float) * stream->pre_resample_channels);
}

static void
SDL_ResetAudioStreamResampler_SRC(SDL_AudioStream *stream)
{
    SRC_src_reset((SRC_STATE *)stream->resampler_state);
}

static void
SDL_CleanupAudioStreamResampler_SRC(SDL_AudioStream *stream)
{
    SRC_STATE *state = (SRC_STATE *)stream->resampler_state;
    if (state) {
        SRC_src_delete(state);
    }

    stream->resampler_state = NULL;
    stream->resampler_func = NULL;
    stream->reset_resampler_func = NULL;
    stream->cleanup_resampler_func = NULL;
}

static SDL_bool
SetupLibSampleRateResampling(SDL_AudioStream *stream)
{
    int result = 0;
    SRC_STATE *state = NULL;

    if (SRC_available) {
        state = SRC_src_new(SRC_converter, stream->pre_resample_channels, &result);
        if (!state) {
            SDL_SetError("src_new() failed: %s", SRC_src_strerror(result));
        }
    }

    if (!state) {
        SDL_CleanupAudioStreamResampler_SRC(stream);
        return SDL_FALSE;
    }

    stream->resampler_state = state;
    stream->resampler_func = SDL_ResampleAudioStream_SRC;
    stream->reset_resampler_func = SDL_ResetAudioStreamResampler_SRC;
    stream->cleanup_resampler_func = SDL_CleanupAudioStreamResampler_SRC;

    return SDL_TRUE;
}
#endif /* HAVE_LIBSAMPLERATE_H */


static int
SDL_ResampleAudioStream(SDL_AudioStream *stream, const void *_inbuf, const int inbuflen, void *_outbuf, const int outbuflen)
{
    const Uint8 *inbufend = ((const Uint8 *) _inbuf) + inbuflen;
    const float *inbuf = (const float *) _inbuf;
    float *outbuf = (float *) _outbuf;
    const int chans = (int) stream->pre_resample_channels;
    const int inrate = stream->src_rate;
    const int outrate = stream->dst_rate;
    const int paddingsamples = stream->resampler_padding_samples;
    const int paddingbytes = paddingsamples * sizeof (float);
    float *lpadding = (float *) stream->resampler_state;
    const float *rpadding = (const float *) inbufend; /* we set this up so there are valid padding samples at the end of the input buffer. */
    const int cpy = SDL_min(inbuflen, paddingbytes);
    int retval;

    SDL_assert(inbuf != ((const float *) outbuf));  /* SDL_AudioStreamPut() shouldn't allow in-place resamples. */

    retval = SDL_ResampleAudio(chans, inrate, outrate, lpadding, rpadding, inbuf, inbuflen, outbuf, outbuflen);

    /* update our left padding with end of current input, for next run. */
    SDL_memcpy((lpadding + paddingsamples) - (cpy / sizeof (float)), inbufend - cpy, cpy);
    return retval;
}

static void
SDL_ResetAudioStreamResampler(SDL_AudioStream *stream)
{
    /* set all the padding to silence. */
    const int len = stream->resampler_padding_samples;
    SDL_memset(stream->resampler_state, '\0', len * sizeof (float));
}

static void
SDL_CleanupAudioStreamResampler(SDL_AudioStream *stream)
{
    SDL_free(stream->resampler_state);
}

SDL_AudioStream *
SDL_NewAudioStream(const SDL_AudioFormat src_format,
                   const Uint8 src_channels,
                   const int src_rate,
                   const SDL_AudioFormat dst_format,
                   const Uint8 dst_channels,
                   const int dst_rate)
{
    const int packetlen = 4096;  /* !!! FIXME: good enough for now. */
    Uint8 pre_resample_channels;
    SDL_AudioStream *retval;

    retval = (SDL_AudioStream *) SDL_calloc(1, sizeof (SDL_AudioStream));
    if (!retval) {
        return NULL;
    }

    /* If increasing channels, do it after resampling, since we'd just
       do more work to resample duplicate channels. If we're decreasing, do
       it first so we resample the interpolated data instead of interpolating
       the resampled data (!!! FIXME: decide if that works in practice, though!). */
    pre_resample_channels = SDL_min(src_channels, dst_channels);

    retval->first_run = SDL_TRUE;
    retval->src_sample_frame_size = (SDL_AUDIO_BITSIZE(src_format) / 8) * src_channels;
    retval->src_format = src_format;
    retval->src_channels = src_channels;
    retval->src_rate = src_rate;
    retval->dst_sample_frame_size = (SDL_AUDIO_BITSIZE(dst_format) / 8) * dst_channels;
    retval->dst_format = dst_format;
    retval->dst_channels = dst_channels;
    retval->dst_rate = dst_rate;
    retval->pre_resample_channels = pre_resample_channels;
    retval->packetlen = packetlen;
    retval->rate_incr = ((double) dst_rate) / ((double) src_rate);
    retval->resampler_padding_samples = ResamplerPadding(retval->src_rate, retval->dst_rate) * pre_resample_channels;
    retval->resampler_padding = (float *) SDL_calloc(retval->resampler_padding_samples, sizeof (float));

    if (retval->resampler_padding == NULL) {
        SDL_FreeAudioStream(retval);
        SDL_OutOfMemory();
        return NULL;
    }

    retval->staging_buffer_size = ((retval->resampler_padding_samples / retval->pre_resample_channels) * retval->src_sample_frame_size);
    if (retval->staging_buffer_size > 0) {
        retval->staging_buffer = (Uint8 *) SDL_malloc(retval->staging_buffer_size);
        if (retval->staging_buffer == NULL) {
            SDL_FreeAudioStream(retval);
            SDL_OutOfMemory();
            return NULL;
        }
    }

    /* Not resampling? It's an easy conversion (and maybe not even that!) */
    if (src_rate == dst_rate) {
        retval->cvt_before_resampling.needed = SDL_FALSE;
        if (SDL_BuildAudioCVT(&retval->cvt_after_resampling, src_format, src_channels, dst_rate, dst_format, dst_channels, dst_rate) < 0) {
            SDL_FreeAudioStream(retval);
            return NULL;  /* SDL_BuildAudioCVT should have called SDL_SetError. */
        }
    } else {
        /* Don't resample at first. Just get us to Float32 format. */
        /* !!! FIXME: convert to int32 on devices without hardware float. */
        if (SDL_BuildAudioCVT(&retval->cvt_before_resampling, src_format, src_channels, src_rate, AUDIO_F32SYS, pre_resample_channels, src_rate) < 0) {
            SDL_FreeAudioStream(retval);
            return NULL;  /* SDL_BuildAudioCVT should have called SDL_SetError. */
        }

#ifdef HAVE_LIBSAMPLERATE_H
        SetupLibSampleRateResampling(retval);
#endif

        if (!retval->resampler_func) {
            retval->resampler_state = SDL_calloc(retval->resampler_padding_samples, sizeof (float));
            if (!retval->resampler_state) {
                SDL_FreeAudioStream(retval);
                SDL_OutOfMemory();
                return NULL;
            }

            if (SDL_PrepareResampleFilter() < 0) {
                SDL_free(retval->resampler_state);
                retval->resampler_state = NULL;
                SDL_FreeAudioStream(retval);
                return NULL;
            }

            retval->resampler_func = SDL_ResampleAudioStream;
            retval->reset_resampler_func = SDL_ResetAudioStreamResampler;
            retval->cleanup_resampler_func = SDL_CleanupAudioStreamResampler;
        }

        /* Convert us to the final format after resampling. */
        if (SDL_BuildAudioCVT(&retval->cvt_after_resampling, AUDIO_F32SYS, pre_resample_channels, dst_rate, dst_format, dst_channels, dst_rate) < 0) {
            SDL_FreeAudioStream(retval);
            return NULL;  /* SDL_BuildAudioCVT should have called SDL_SetError. */
        }
    }

    retval->queue = SDL_NewDataQueue(packetlen, packetlen * 2);
    if (!retval->queue) {
        SDL_FreeAudioStream(retval);
        return NULL;  /* SDL_NewDataQueue should have called SDL_SetError. */
    }

    return retval;
}

static int
SDL_AudioStreamPutInternal(SDL_AudioStream *stream, const void *buf, int len, int *maxputbytes)
{
    int buflen = len;
    int workbuflen;
    Uint8 *workbuf;
    Uint8 *resamplebuf = NULL;
    int resamplebuflen = 0;
    int neededpaddingbytes;
    int paddingbytes;

    /* !!! FIXME: several converters can take advantage of SIMD, but only
       !!! FIXME:  if the data is aligned to 16 bytes. EnsureStreamBufferSize()
       !!! FIXME:  guarantees the buffer will align, but the
       !!! FIXME:  converters will iterate over the data backwards if
       !!! FIXME:  the output grows, and this means we won't align if buflen
       !!! FIXME:  isn't a multiple of 16. In these cases, we should chop off
       !!! FIXME:  a few samples at the end and convert them separately. */

    /* no padding prepended on first run. */
    neededpaddingbytes = stream->resampler_padding_samples * sizeof (float);
    paddingbytes = stream->first_run ? 0 : neededpaddingbytes;
    stream->first_run = SDL_FALSE;

    /* Make sure the work buffer can hold all the data we need at once... */
    workbuflen = buflen;
    if (stream->cvt_before_resampling.needed) {
        workbuflen *= stream->cvt_before_resampling.len_mult;
    }

    if (stream->dst_rate != stream->src_rate) {
        /* resamples can't happen in place, so make space for second buf. */
        const int framesize = stream->pre_resample_channels * sizeof (float);
        const int frames = workbuflen / framesize;
        resamplebuflen = ((int) SDL_ceil(frames * stream->rate_incr)) * framesize;
        #if DEBUG_AUDIOSTREAM
        printf("AUDIOSTREAM: will resample %d bytes to %d (ratio=%.6f)\n", workbuflen, resamplebuflen, stream->rate_incr);
        #endif
        workbuflen += resamplebuflen;
    }

    if (stream->cvt_after_resampling.needed) {
        /* !!! FIXME: buffer might be big enough already? */
        workbuflen *= stream->cvt_after_resampling.len_mult;
    }

    workbuflen += neededpaddingbytes;

    #if DEBUG_AUDIOSTREAM
    printf("AUDIOSTREAM: Putting %d bytes of preconverted audio, need %d byte work buffer\n", buflen, workbuflen);
    #endif

    workbuf = EnsureStreamBufferSize(stream, workbuflen);
    if (!workbuf) {
        return -1;  /* probably out of memory. */
    }

    resamplebuf = workbuf;  /* default if not resampling. */

    SDL_memcpy(workbuf + paddingbytes, buf, buflen);

    if (stream->cvt_before_resampling.needed) {
        stream->cvt_before_resampling.buf = workbuf + paddingbytes;
        stream->cvt_before_resampling.len = buflen;
        if (SDL_ConvertAudio(&stream->cvt_before_resampling) == -1) {
            return -1;   /* uhoh! */
        }
        buflen = stream->cvt_before_resampling.len_cvt;

        #if DEBUG_AUDIOSTREAM
        printf("AUDIOSTREAM: After initial conversion we have %d bytes\n", buflen);
        #endif
    }

    if (stream->dst_rate != stream->src_rate) {
        /* save off some samples at the end; they are used for padding now so
           the resampler is coherent and then used at the start of the next
           put operation. Prepend last put operation's padding, too. */

        /* prepend prior put's padding. :P */
        if (paddingbytes) {
            SDL_memcpy(workbuf, stream->resampler_padding, paddingbytes);
            buflen += paddingbytes;
        }

        /* save off the data at the end for the next run. */
        SDL_memcpy(stream->resampler_padding, workbuf + (buflen - neededpaddingbytes), neededpaddingbytes);

        resamplebuf = workbuf + buflen;  /* skip to second piece of workbuf. */
        SDL_assert(buflen >= neededpaddingbytes);
        if (buflen > neededpaddingbytes) {
            buflen = stream->resampler_func(stream, workbuf, buflen - neededpaddingbytes, resamplebuf, resamplebuflen);
        } else {
            buflen = 0;
        }

        #if DEBUG_AUDIOSTREAM
        printf("AUDIOSTREAM: After resampling we have %d bytes\n", buflen);
        #endif
    }

    if (stream->cvt_after_resampling.needed && (buflen > 0)) {
        stream->cvt_after_resampling.buf = resamplebuf;
        stream->cvt_after_resampling.len = buflen;
        if (SDL_ConvertAudio(&stream->cvt_after_resampling) == -1) {
            return -1;   /* uhoh! */
        }
        buflen = stream->cvt_after_resampling.len_cvt;

        #if DEBUG_AUDIOSTREAM
        printf("AUDIOSTREAM: After final conversion we have %d bytes\n", buflen);
        #endif
    }

    #if DEBUG_AUDIOSTREAM
    printf("AUDIOSTREAM: Final output is %d bytes\n", buflen);
    #endif

    if (maxputbytes) {
        const int maxbytes = *maxputbytes;
        if (buflen > maxbytes)
            buflen = maxbytes;
        *maxputbytes -= buflen;
    }

    /* resamplebuf holds the final output, even if we didn't resample. */
    return buflen ? SDL_WriteToDataQueue(stream->queue, resamplebuf, buflen) : 0;
}

int
SDL_AudioStreamPut(SDL_AudioStream *stream, const void *buf, int len)
{
    /* !!! FIXME: several converters can take advantage of SIMD, but only
       !!! FIXME:  if the data is aligned to 16 bytes. EnsureStreamBufferSize()
       !!! FIXME:  guarantees the buffer will align, but the
       !!! FIXME:  converters will iterate over the data backwards if
       !!! FIXME:  the output grows, and this means we won't align if buflen
       !!! FIXME:  isn't a multiple of 16. In these cases, we should chop off
       !!! FIXME:  a few samples at the end and convert them separately. */

    #if DEBUG_AUDIOSTREAM
    printf("AUDIOSTREAM: wants to put %d preconverted bytes\n", buflen);
    #endif

    if (!stream) {
        return SDL_InvalidParamError("stream");
    } else if (!buf) {
        return SDL_InvalidParamError("buf");
    } else if (len == 0) {
        return 0;  /* nothing to do. */
    } else if ((len % stream->src_sample_frame_size) != 0) {
        return SDL_SetError("Can't add partial sample frames");
    }

    if (!stream->cvt_before_resampling.needed &&
        (stream->dst_rate == stream->src_rate) &&
        !stream->cvt_after_resampling.needed) {
        #if DEBUG_AUDIOSTREAM
        printf("AUDIOSTREAM: no conversion needed at all, queueing %d bytes.\n", len);
        #endif
        return SDL_WriteToDataQueue(stream->queue, buf, len);
    }

    while (len > 0) {
        int amount;

        /* If we don't have a staging buffer or we're given enough data that
           we don't need to store it for later, skip the staging process.
         */
        if (!stream->staging_buffer_filled && len >= stream->staging_buffer_size) {
            return SDL_AudioStreamPutInternal(stream, buf, len, NULL);
        }

        /* If there's not enough data to fill the staging buffer, just save it */
        if ((stream->staging_buffer_filled + len) < stream->staging_buffer_size) {
            SDL_memcpy(stream->staging_buffer + stream->staging_buffer_filled, buf, len);
            stream->staging_buffer_filled += len;
            return 0;
        }
 
        /* Fill the staging buffer, process it, and continue */
        amount = (stream->staging_buffer_size - stream->staging_buffer_filled);
        SDL_assert(amount > 0);
        SDL_memcpy(stream->staging_buffer + stream->staging_buffer_filled, buf, amount);
        stream->staging_buffer_filled = 0;
        if (SDL_AudioStreamPutInternal(stream, stream->staging_buffer, stream->staging_buffer_size, NULL) < 0) {
            return -1;
        }
        buf = (void *)((Uint8 *)buf + amount);
        len -= amount;
    }
    return 0;
}

int SDL_AudioStreamFlush(SDL_AudioStream *stream)
{
    if (!stream) {
        return SDL_InvalidParamError("stream");
    }

    #if DEBUG_AUDIOSTREAM
    printf("AUDIOSTREAM: flushing! staging_buffer_filled=%d bytes\n", stream->staging_buffer_filled);
    #endif

    /* shouldn't use a staging buffer if we're not resampling. */
    SDL_assert((stream->dst_rate != stream->src_rate) || (stream->staging_buffer_filled == 0));

    if (stream->staging_buffer_filled > 0) {
        /* push the staging buffer + silence. We need to flush out not just
           the staging buffer, but the piece that the stream was saving off
           for right-side resampler padding. */
        const SDL_bool first_run = stream->first_run;
        const int filled = stream->staging_buffer_filled;
        int actual_input_frames = filled / stream->src_sample_frame_size;
        if (!first_run)
            actual_input_frames += stream->resampler_padding_samples / stream->pre_resample_channels;

        if (actual_input_frames > 0) {  /* don't bother if nothing to flush. */
            /* This is how many bytes we're expecting without silence appended. */
            int flush_remaining = ((int) SDL_ceil(actual_input_frames * stream->rate_incr)) * stream->dst_sample_frame_size;

            #if DEBUG_AUDIOSTREAM
            printf("AUDIOSTREAM: flushing with padding to get max %d bytes!\n", flush_remaining);
            #endif

            SDL_memset(stream->staging_buffer + filled, '\0', stream->staging_buffer_size - filled);
            if (SDL_AudioStreamPutInternal(stream, stream->staging_buffer, stream->staging_buffer_size, &flush_remaining) < 0) {
                return -1;
            }

            /* we have flushed out (or initially filled) the pending right-side
               resampler padding, but we need to push more silence to guarantee
               the staging buffer is fully flushed out, too. */
            SDL_memset(stream->staging_buffer, '\0', filled);
            if (SDL_AudioStreamPutInternal(stream, stream->staging_buffer, stream->staging_buffer_size, &flush_remaining) < 0) {
                return -1;
            }
        }
    }

    stream->staging_buffer_filled = 0;
    stream->first_run = SDL_TRUE;

    return 0;
}

/* get converted/resampled data from the stream */
int
SDL_AudioStreamGet(SDL_AudioStream *stream, void *buf, int len)
{
    #if DEBUG_AUDIOSTREAM
    printf("AUDIOSTREAM: want to get %d converted bytes\n", len);
    #endif

    if (!stream) {
        return SDL_InvalidParamError("stream");
    } else if (!buf) {
        return SDL_InvalidParamError("buf");
    } else if (len <= 0) {
        return 0;  /* nothing to do. */
    } else if ((len % stream->dst_sample_frame_size) != 0) {
        return SDL_SetError("Can't request partial sample frames");
    }

    return (int) SDL_ReadFromDataQueue(stream->queue, buf, len);
}

/* number of converted/resampled bytes available */
int
SDL_AudioStreamAvailable(SDL_AudioStream *stream)
{
    return stream ? (int) SDL_CountDataQueue(stream->queue) : 0;
}

void
SDL_AudioStreamClear(SDL_AudioStream *stream)
{
    if (!stream) {
        SDL_InvalidParamError("stream");
    } else {
        SDL_ClearDataQueue(stream->queue, stream->packetlen * 2);
        if (stream->reset_resampler_func) {
            stream->reset_resampler_func(stream);
        }
        stream->first_run = SDL_TRUE;
        stream->staging_buffer_filled = 0;
    }
}

/* dispose of a stream */
void
SDL_FreeAudioStream(SDL_AudioStream *stream)
{
    if (stream) {
        if (stream->cleanup_resampler_func) {
            stream->cleanup_resampler_func(stream);
        }
        SDL_FreeDataQueue(stream->queue);
        SDL_free(stream->staging_buffer);
        SDL_free(stream->work_buffer_base);
        SDL_free(stream->resampler_padding);
        SDL_free(stream);
    }
}

/* vi: set ts=4 sw=4 expandtab: */

