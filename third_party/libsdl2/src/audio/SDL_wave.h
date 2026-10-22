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

/* WAVE files are little-endian */

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF            0x46464952      /* "RIFF" */
#define WAVE            0x45564157      /* "WAVE" */
#define FACT            0x74636166      /* "fact" */
#define LIST            0x5453494c      /* "LIST" */
#define BEXT            0x74786562      /* "bext" */
#define JUNK            0x4B4E554A      /* "JUNK" */
#define FMT             0x20746D66      /* "fmt " */
#define DATA            0x61746164      /* "data" */
#define PCM_CODE        0x0001
#define MS_ADPCM_CODE   0x0002
#define IEEE_FLOAT_CODE 0x0003
#define IMA_ADPCM_CODE  0x0011
#define MP3_CODE        0x0055
#define EXTENSIBLE_CODE 0xFFFE
#define WAVE_MONO       1
#define WAVE_STEREO     2

/* Normally, these three chunks come consecutively in a WAVE file */
typedef struct WaveFMT
{
/* Not saved in the chunk we read:
    Uint32  FMTchunk;
    Uint32  fmtlen;
*/
    Uint16 encoding;
    Uint16 channels;            /* 1 = mono, 2 = stereo */
    Uint32 frequency;           /* One of 11025, 22050, or 44100 Hz */
    Uint32 byterate;            /* Average bytes per second */
    Uint16 blockalign;          /* Bytes per sample block */
    Uint16 bitspersample;       /* One of 8, 12, 16, or 4 for ADPCM */
} WaveFMT;

/* The general chunk found in the WAVE file */
typedef struct Chunk
{
    Uint32 magic;
    Uint32 length;
    Uint8 *data;
} Chunk;

typedef struct WaveExtensibleFMT
{
    WaveFMT format;
    Uint16 size;
    Uint16 validbits;
    Uint32 channelmask;
    Uint8 subformat[16];  /* a GUID. */
} WaveExtensibleFMT;

/* vi: set ts=4 sw=4 expandtab: */
