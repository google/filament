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

/* Microsoft WAVE file loading routines */

#include "SDL_audio.h"
#include "SDL_wave.h"


static int ReadChunk(SDL_RWops * src, Chunk * chunk);

struct MS_ADPCM_decodestate
{
    Uint8 hPredictor;
    Uint16 iDelta;
    Sint16 iSamp1;
    Sint16 iSamp2;
};
static struct MS_ADPCM_decoder
{
    WaveFMT wavefmt;
    Uint16 wSamplesPerBlock;
    Uint16 wNumCoef;
    Sint16 aCoeff[7][2];
    /* * * */
    struct MS_ADPCM_decodestate state[2];
} MS_ADPCM_state;

static int
InitMS_ADPCM(WaveFMT * format)
{
    Uint8 *rogue_feel;
    int i;

    /* Set the rogue pointer to the MS_ADPCM specific data */
    MS_ADPCM_state.wavefmt.encoding = SDL_SwapLE16(format->encoding);
    MS_ADPCM_state.wavefmt.channels = SDL_SwapLE16(format->channels);
    MS_ADPCM_state.wavefmt.frequency = SDL_SwapLE32(format->frequency);
    MS_ADPCM_state.wavefmt.byterate = SDL_SwapLE32(format->byterate);
    MS_ADPCM_state.wavefmt.blockalign = SDL_SwapLE16(format->blockalign);
    MS_ADPCM_state.wavefmt.bitspersample =
        SDL_SwapLE16(format->bitspersample);
    rogue_feel = (Uint8 *) format + sizeof(*format);
    if (sizeof(*format) == 16) {
        /* const Uint16 extra_info = ((rogue_feel[1] << 8) | rogue_feel[0]); */
        rogue_feel += sizeof(Uint16);
    }
    MS_ADPCM_state.wSamplesPerBlock = ((rogue_feel[1] << 8) | rogue_feel[0]);
    rogue_feel += sizeof(Uint16);
    MS_ADPCM_state.wNumCoef = ((rogue_feel[1] << 8) | rogue_feel[0]);
    rogue_feel += sizeof(Uint16);
    if (MS_ADPCM_state.wNumCoef != 7) {
        SDL_SetError("Unknown set of MS_ADPCM coefficients");
        return (-1);
    }
    for (i = 0; i < MS_ADPCM_state.wNumCoef; ++i) {
        MS_ADPCM_state.aCoeff[i][0] = ((rogue_feel[1] << 8) | rogue_feel[0]);
        rogue_feel += sizeof(Uint16);
        MS_ADPCM_state.aCoeff[i][1] = ((rogue_feel[1] << 8) | rogue_feel[0]);
        rogue_feel += sizeof(Uint16);
    }
    return (0);
}

static Sint32
MS_ADPCM_nibble(struct MS_ADPCM_decodestate *state,
                Uint8 nybble, Sint16 * coeff)
{
    const Sint32 max_audioval = ((1 << (16 - 1)) - 1);
    const Sint32 min_audioval = -(1 << (16 - 1));
    const Sint32 adaptive[] = {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
    };
    Sint32 new_sample, delta;

    new_sample = ((state->iSamp1 * coeff[0]) +
                  (state->iSamp2 * coeff[1])) / 256;
    if (nybble & 0x08) {
        new_sample += state->iDelta * (nybble - 0x10);
    } else {
        new_sample += state->iDelta * nybble;
    }
    if (new_sample < min_audioval) {
        new_sample = min_audioval;
    } else if (new_sample > max_audioval) {
        new_sample = max_audioval;
    }
    delta = ((Sint32) state->iDelta * adaptive[nybble]) / 256;
    if (delta < 16) {
        delta = 16;
    }
    state->iDelta = (Uint16) delta;
    state->iSamp2 = state->iSamp1;
    state->iSamp1 = (Sint16) new_sample;
    return (new_sample);
}

static int
MS_ADPCM_decode(Uint8 ** audio_buf, Uint32 * audio_len)
{
    struct MS_ADPCM_decodestate *state[2];
    Uint8 *freeable, *encoded, *decoded;
    Sint32 encoded_len, samplesleft;
    Sint8 nybble;
    Uint8 stereo;
    Sint16 *coeff[2];
    Sint32 new_sample;

    /* Allocate the proper sized output buffer */
    encoded_len = *audio_len;
    encoded = *audio_buf;
    freeable = *audio_buf;
    *audio_len = (encoded_len / MS_ADPCM_state.wavefmt.blockalign) *
        MS_ADPCM_state.wSamplesPerBlock *
        MS_ADPCM_state.wavefmt.channels * sizeof(Sint16);
    *audio_buf = (Uint8 *) SDL_malloc(*audio_len);
    if (*audio_buf == NULL) {
        return SDL_OutOfMemory();
    }
    decoded = *audio_buf;

    /* Get ready... Go! */
    stereo = (MS_ADPCM_state.wavefmt.channels == 2);
    state[0] = &MS_ADPCM_state.state[0];
    state[1] = &MS_ADPCM_state.state[stereo];
    while (encoded_len >= MS_ADPCM_state.wavefmt.blockalign) {
        /* Grab the initial information for this block */
        state[0]->hPredictor = *encoded++;
        if (stereo) {
            state[1]->hPredictor = *encoded++;
        }
        state[0]->iDelta = ((encoded[1] << 8) | encoded[0]);
        encoded += sizeof(Sint16);
        if (stereo) {
            state[1]->iDelta = ((encoded[1] << 8) | encoded[0]);
            encoded += sizeof(Sint16);
        }
        state[0]->iSamp1 = ((encoded[1] << 8) | encoded[0]);
        encoded += sizeof(Sint16);
        if (stereo) {
            state[1]->iSamp1 = ((encoded[1] << 8) | encoded[0]);
            encoded += sizeof(Sint16);
        }
        state[0]->iSamp2 = ((encoded[1] << 8) | encoded[0]);
        encoded += sizeof(Sint16);
        if (stereo) {
            state[1]->iSamp2 = ((encoded[1] << 8) | encoded[0]);
            encoded += sizeof(Sint16);
        }
        coeff[0] = MS_ADPCM_state.aCoeff[state[0]->hPredictor];
        coeff[1] = MS_ADPCM_state.aCoeff[state[1]->hPredictor];

        /* Store the two initial samples we start with */
        decoded[0] = state[0]->iSamp2 & 0xFF;
        decoded[1] = state[0]->iSamp2 >> 8;
        decoded += 2;
        if (stereo) {
            decoded[0] = state[1]->iSamp2 & 0xFF;
            decoded[1] = state[1]->iSamp2 >> 8;
            decoded += 2;
        }
        decoded[0] = state[0]->iSamp1 & 0xFF;
        decoded[1] = state[0]->iSamp1 >> 8;
        decoded += 2;
        if (stereo) {
            decoded[0] = state[1]->iSamp1 & 0xFF;
            decoded[1] = state[1]->iSamp1 >> 8;
            decoded += 2;
        }

        /* Decode and store the other samples in this block */
        samplesleft = (MS_ADPCM_state.wSamplesPerBlock - 2) *
            MS_ADPCM_state.wavefmt.channels;
        while (samplesleft > 0) {
            nybble = (*encoded) >> 4;
            new_sample = MS_ADPCM_nibble(state[0], nybble, coeff[0]);
            decoded[0] = new_sample & 0xFF;
            new_sample >>= 8;
            decoded[1] = new_sample & 0xFF;
            decoded += 2;

            nybble = (*encoded) & 0x0F;
            new_sample = MS_ADPCM_nibble(state[1], nybble, coeff[1]);
            decoded[0] = new_sample & 0xFF;
            new_sample >>= 8;
            decoded[1] = new_sample & 0xFF;
            decoded += 2;

            ++encoded;
            samplesleft -= 2;
        }
        encoded_len -= MS_ADPCM_state.wavefmt.blockalign;
    }
    SDL_free(freeable);
    return (0);
}

struct IMA_ADPCM_decodestate
{
    Sint32 sample;
    Sint8 index;
};
static struct IMA_ADPCM_decoder
{
    WaveFMT wavefmt;
    Uint16 wSamplesPerBlock;
    /* * * */
    struct IMA_ADPCM_decodestate state[2];
} IMA_ADPCM_state;

static int
InitIMA_ADPCM(WaveFMT * format)
{
    Uint8 *rogue_feel;

    /* Set the rogue pointer to the IMA_ADPCM specific data */
    IMA_ADPCM_state.wavefmt.encoding = SDL_SwapLE16(format->encoding);
    IMA_ADPCM_state.wavefmt.channels = SDL_SwapLE16(format->channels);
    IMA_ADPCM_state.wavefmt.frequency = SDL_SwapLE32(format->frequency);
    IMA_ADPCM_state.wavefmt.byterate = SDL_SwapLE32(format->byterate);
    IMA_ADPCM_state.wavefmt.blockalign = SDL_SwapLE16(format->blockalign);
    IMA_ADPCM_state.wavefmt.bitspersample =
        SDL_SwapLE16(format->bitspersample);
    rogue_feel = (Uint8 *) format + sizeof(*format);
    if (sizeof(*format) == 16) {
        /* const Uint16 extra_info = ((rogue_feel[1] << 8) | rogue_feel[0]); */
        rogue_feel += sizeof(Uint16);
    }
    IMA_ADPCM_state.wSamplesPerBlock = ((rogue_feel[1] << 8) | rogue_feel[0]);
    return (0);
}

static Sint32
IMA_ADPCM_nibble(struct IMA_ADPCM_decodestate *state, Uint8 nybble)
{
    const Sint32 max_audioval = ((1 << (16 - 1)) - 1);
    const Sint32 min_audioval = -(1 << (16 - 1));
    const int index_table[16] = {
        -1, -1, -1, -1,
        2, 4, 6, 8,
        -1, -1, -1, -1,
        2, 4, 6, 8
    };
    const Sint32 step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
        34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
        143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
        449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
        1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
        3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
        9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
        22385, 24623, 27086, 29794, 32767
    };
    Sint32 delta, step;

    /* Compute difference and new sample value */
    if (state->index > 88) {
        state->index = 88;
    } else if (state->index < 0) {
        state->index = 0;
    }
    /* explicit cast to avoid gcc warning about using 'char' as array index */
    step = step_table[(int)state->index];
    delta = step >> 3;
    if (nybble & 0x04)
        delta += step;
    if (nybble & 0x02)
        delta += (step >> 1);
    if (nybble & 0x01)
        delta += (step >> 2);
    if (nybble & 0x08)
        delta = -delta;
    state->sample += delta;

    /* Update index value */
    state->index += index_table[nybble];

    /* Clamp output sample */
    if (state->sample > max_audioval) {
        state->sample = max_audioval;
    } else if (state->sample < min_audioval) {
        state->sample = min_audioval;
    }
    return (state->sample);
}

/* Fill the decode buffer with a channel block of data (8 samples) */
static void
Fill_IMA_ADPCM_block(Uint8 * decoded, Uint8 * encoded,
                     int channel, int numchannels,
                     struct IMA_ADPCM_decodestate *state)
{
    int i;
    Sint8 nybble;
    Sint32 new_sample;

    decoded += (channel * 2);
    for (i = 0; i < 4; ++i) {
        nybble = (*encoded) & 0x0F;
        new_sample = IMA_ADPCM_nibble(state, nybble);
        decoded[0] = new_sample & 0xFF;
        new_sample >>= 8;
        decoded[1] = new_sample & 0xFF;
        decoded += 2 * numchannels;

        nybble = (*encoded) >> 4;
        new_sample = IMA_ADPCM_nibble(state, nybble);
        decoded[0] = new_sample & 0xFF;
        new_sample >>= 8;
        decoded[1] = new_sample & 0xFF;
        decoded += 2 * numchannels;

        ++encoded;
    }
}

static int
IMA_ADPCM_decode(Uint8 ** audio_buf, Uint32 * audio_len)
{
    struct IMA_ADPCM_decodestate *state;
    Uint8 *freeable, *encoded, *decoded;
    Sint32 encoded_len, samplesleft;
    unsigned int c, channels;

    /* Check to make sure we have enough variables in the state array */
    channels = IMA_ADPCM_state.wavefmt.channels;
    if (channels > SDL_arraysize(IMA_ADPCM_state.state)) {
        SDL_SetError("IMA ADPCM decoder can only handle %u channels",
                     (unsigned int)SDL_arraysize(IMA_ADPCM_state.state));
        return (-1);
    }
    state = IMA_ADPCM_state.state;

    /* Allocate the proper sized output buffer */
    encoded_len = *audio_len;
    encoded = *audio_buf;
    freeable = *audio_buf;
    *audio_len = (encoded_len / IMA_ADPCM_state.wavefmt.blockalign) *
        IMA_ADPCM_state.wSamplesPerBlock *
        IMA_ADPCM_state.wavefmt.channels * sizeof(Sint16);
    *audio_buf = (Uint8 *) SDL_malloc(*audio_len);
    if (*audio_buf == NULL) {
        return SDL_OutOfMemory();
    }
    decoded = *audio_buf;

    /* Get ready... Go! */
    while (encoded_len >= IMA_ADPCM_state.wavefmt.blockalign) {
        /* Grab the initial information for this block */
        for (c = 0; c < channels; ++c) {
            /* Fill the state information for this block */
            state[c].sample = ((encoded[1] << 8) | encoded[0]);
            encoded += 2;
            if (state[c].sample & 0x8000) {
                state[c].sample -= 0x10000;
            }
            state[c].index = *encoded++;
            /* Reserved byte in buffer header, should be 0 */
            if (*encoded++ != 0) {
                /* Uh oh, corrupt data?  Buggy code? */ ;
            }

            /* Store the initial sample we start with */
            decoded[0] = (Uint8) (state[c].sample & 0xFF);
            decoded[1] = (Uint8) (state[c].sample >> 8);
            decoded += 2;
        }

        /* Decode and store the other samples in this block */
        samplesleft = (IMA_ADPCM_state.wSamplesPerBlock - 1) * channels;
        while (samplesleft > 0) {
            for (c = 0; c < channels; ++c) {
                Fill_IMA_ADPCM_block(decoded, encoded,
                                     c, channels, &state[c]);
                encoded += 4;
                samplesleft -= 8;
            }
            decoded += (channels * 8 * 2);
        }
        encoded_len -= IMA_ADPCM_state.wavefmt.blockalign;
    }
    SDL_free(freeable);
    return (0);
}


static int
ConvertSint24ToSint32(Uint8 ** audio_buf, Uint32 * audio_len)
{
    const double DIVBY8388608 = 0.00000011920928955078125;
    const Uint32 original_len = *audio_len;
    const Uint32 samples = original_len / 3;
    const Uint32 expanded_len = samples * sizeof (Uint32);
    Uint8 *ptr = (Uint8 *) SDL_realloc(*audio_buf, expanded_len);
    const Uint8 *src;
    Uint32 *dst;
    Uint32 i;

    if (!ptr) {
        return SDL_OutOfMemory();
    }

    *audio_buf = ptr;
    *audio_len = expanded_len;

    /* work from end to start, since we're expanding in-place. */
    src = (ptr + original_len) - 3;
    dst = ((Uint32 *) (ptr + expanded_len)) - 1;
    for (i = 0; i < samples; i++) {
        /* There's probably a faster way to do all this. */
        const Sint32 converted = ((Sint32) ( (((Uint32) src[2]) << 24) |
                                             (((Uint32) src[1]) << 16) |
                                             (((Uint32) src[0]) << 8) )) >> 8;
        const double scaled = (((double) converted) * DIVBY8388608);
        src -= 3;
        *(dst--) = (Sint32) (scaled * 2147483647.0);
    }

    return 0;
}


/* GUIDs that are used by WAVE_FORMAT_EXTENSIBLE */
static const Uint8 extensible_pcm_guid[16] = { 1, 0, 0, 0, 0, 0, 16, 0, 128, 0, 0, 170, 0, 56, 155, 113 };
static const Uint8 extensible_ieee_guid[16] = { 3, 0, 0, 0, 0, 0, 16, 0, 128, 0, 0, 170, 0, 56, 155, 113 };

SDL_AudioSpec *
SDL_LoadWAV_RW(SDL_RWops * src, int freesrc,
               SDL_AudioSpec * spec, Uint8 ** audio_buf, Uint32 * audio_len)
{
    int was_error;
    Chunk chunk;
    int lenread;
    int IEEE_float_encoded, MS_ADPCM_encoded, IMA_ADPCM_encoded;
    int samplesize;

    /* WAV magic header */
    Uint32 RIFFchunk;
    Uint32 wavelen = 0;
    Uint32 WAVEmagic;
    Uint32 headerDiff = 0;

    /* FMT chunk */
    WaveFMT *format = NULL;
    WaveExtensibleFMT *ext = NULL;

    SDL_zero(chunk);

    /* Make sure we are passed a valid data source */
    was_error = 0;
    if (src == NULL) {
        was_error = 1;
        goto done;
    }

    /* Check the magic header */
    RIFFchunk = SDL_ReadLE32(src);
    wavelen = SDL_ReadLE32(src);
    if (wavelen == WAVE) {      /* The RIFFchunk has already been read */
        WAVEmagic = wavelen;
        wavelen = RIFFchunk;
        RIFFchunk = RIFF;
    } else {
        WAVEmagic = SDL_ReadLE32(src);
    }
    if ((RIFFchunk != RIFF) || (WAVEmagic != WAVE)) {
        SDL_SetError("Unrecognized file type (not WAVE)");
        was_error = 1;
        goto done;
    }
    headerDiff += sizeof(Uint32);       /* for WAVE */

    /* Read the audio data format chunk */
    chunk.data = NULL;
    do {
        SDL_free(chunk.data);
        chunk.data = NULL;
        lenread = ReadChunk(src, &chunk);
        if (lenread < 0) {
            was_error = 1;
            goto done;
        }
        /* 2 Uint32's for chunk header+len, plus the lenread */
        headerDiff += lenread + 2 * sizeof(Uint32);
    } while ((chunk.magic == FACT) || (chunk.magic == LIST) || (chunk.magic == BEXT) || (chunk.magic == JUNK));

    /* Decode the audio data format */
    format = (WaveFMT *) chunk.data;
    if (chunk.magic != FMT) {
        SDL_SetError("Complex WAVE files not supported");
        was_error = 1;
        goto done;
    }
    IEEE_float_encoded = MS_ADPCM_encoded = IMA_ADPCM_encoded = 0;
    switch (SDL_SwapLE16(format->encoding)) {
    case PCM_CODE:
        /* We can understand this */
        break;
    case IEEE_FLOAT_CODE:
        IEEE_float_encoded = 1;
        /* We can understand this */
        break;
    case MS_ADPCM_CODE:
        /* Try to understand this */
        if (InitMS_ADPCM(format) < 0) {
            was_error = 1;
            goto done;
        }
        MS_ADPCM_encoded = 1;
        break;
    case IMA_ADPCM_CODE:
        /* Try to understand this */
        if (InitIMA_ADPCM(format) < 0) {
            was_error = 1;
            goto done;
        }
        IMA_ADPCM_encoded = 1;
        break;
    case EXTENSIBLE_CODE:
        /* note that this ignores channel masks, smaller valid bit counts
           inside a larger container, and most subtypes. This is just enough
           to get things that didn't really _need_ WAVE_FORMAT_EXTENSIBLE
           to be useful working when they use this format flag. */
        ext = (WaveExtensibleFMT *) format;
        if (SDL_SwapLE16(ext->size) < 22) {
            SDL_SetError("bogus extended .wav header");
            was_error = 1;
            goto done;
        }
        if (SDL_memcmp(ext->subformat, extensible_pcm_guid, 16) == 0) {
            break;  /* cool. */
        } else if (SDL_memcmp(ext->subformat, extensible_ieee_guid, 16) == 0) {
            IEEE_float_encoded = 1;
            break;
        }
        break;
    case MP3_CODE:
        SDL_SetError("MPEG Layer 3 data not supported");
        was_error = 1;
        goto done;
    default:
        SDL_SetError("Unknown WAVE data format: 0x%.4x",
                     SDL_SwapLE16(format->encoding));
        was_error = 1;
        goto done;
    }
    SDL_zerop(spec);
    spec->freq = SDL_SwapLE32(format->frequency);

    if (IEEE_float_encoded) {
        if ((SDL_SwapLE16(format->bitspersample)) != 32) {
            was_error = 1;
        } else {
            spec->format = AUDIO_F32;
        }
    } else {
        switch (SDL_SwapLE16(format->bitspersample)) {
        case 4:
            if (MS_ADPCM_encoded || IMA_ADPCM_encoded) {
                spec->format = AUDIO_S16;
            } else {
                was_error = 1;
            }
            break;
        case 8:
            spec->format = AUDIO_U8;
            break;
        case 16:
            spec->format = AUDIO_S16;
            break;
        case 24:  /* convert this. */
            spec->format = AUDIO_S32;
            break;
        case 32:
            spec->format = AUDIO_S32;
            break;
        default:
            was_error = 1;
            break;
        }
    }

    if (was_error) {
        SDL_SetError("Unknown %d-bit PCM data format",
                     SDL_SwapLE16(format->bitspersample));
        goto done;
    }
    spec->channels = (Uint8) SDL_SwapLE16(format->channels);
    spec->samples = 4096;       /* Good default buffer size */

    /* Read the audio data chunk */
    *audio_buf = NULL;
    do {
        SDL_free(*audio_buf);
        *audio_buf = NULL;
        lenread = ReadChunk(src, &chunk);
        if (lenread < 0) {
            was_error = 1;
            goto done;
        }
        *audio_len = lenread;
        *audio_buf = chunk.data;
        if (chunk.magic != DATA)
            headerDiff += lenread + 2 * sizeof(Uint32);
    } while (chunk.magic != DATA);
    headerDiff += 2 * sizeof(Uint32);   /* for the data chunk and len */

    if (MS_ADPCM_encoded) {
        if (MS_ADPCM_decode(audio_buf, audio_len) < 0) {
            was_error = 1;
            goto done;
        }
    }
    if (IMA_ADPCM_encoded) {
        if (IMA_ADPCM_decode(audio_buf, audio_len) < 0) {
            was_error = 1;
            goto done;
        }
    }

    if (SDL_SwapLE16(format->bitspersample) == 24) {
        if (ConvertSint24ToSint32(audio_buf, audio_len) < 0) {
            was_error = 1;
            goto done;
        }
    }

    /* Don't return a buffer that isn't a multiple of samplesize */
    samplesize = ((SDL_AUDIO_BITSIZE(spec->format)) / 8) * spec->channels;
    *audio_len &= ~(samplesize - 1);

  done:
    SDL_free(format);
    if (src) {
        if (freesrc) {
            SDL_RWclose(src);
        } else {
            /* seek to the end of the file (given by the RIFF chunk) */
            SDL_RWseek(src, wavelen - chunk.length - headerDiff, RW_SEEK_CUR);
        }
    }
    if (was_error) {
        spec = NULL;
    }
    return (spec);
}

/* Since the WAV memory is allocated in the shared library, it must also
   be freed here.  (Necessary under Win32, VC++)
 */
void
SDL_FreeWAV(Uint8 * audio_buf)
{
    SDL_free(audio_buf);
}

static int
ReadChunk(SDL_RWops * src, Chunk * chunk)
{
    chunk->magic = SDL_ReadLE32(src);
    chunk->length = SDL_ReadLE32(src);
    chunk->data = (Uint8 *) SDL_malloc(chunk->length);
    if (chunk->data == NULL) {
        return SDL_OutOfMemory();
    }
    if (SDL_RWread(src, chunk->data, chunk->length, 1) != 1) {
        SDL_free(chunk->data);
        chunk->data = NULL;
        return SDL_Error(SDL_EFREAD);
    }
    return (chunk->length);
}

/* vi: set ts=4 sw=4 expandtab: */
