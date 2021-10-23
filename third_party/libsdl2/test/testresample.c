/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include "SDL.h"

int
main(int argc, char **argv)
{
    SDL_AudioSpec spec;
    SDL_AudioCVT cvt;
    Uint32 len = 0;
    Uint8 *data = NULL;
    int cvtfreq = 0;
    int cvtchans = 0;
    int bitsize = 0;
    int blockalign = 0;
    int avgbytes = 0;
    SDL_RWops *io = NULL;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (argc != 5) {
        SDL_Log("USAGE: %s in.wav out.wav newfreq newchans\n", argv[0]);
        return 1;
    }

    cvtfreq = SDL_atoi(argv[3]);
    cvtchans = SDL_atoi(argv[4]);

    if (SDL_Init(SDL_INIT_AUDIO) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init() failed: %s\n", SDL_GetError());
        return 2;
    }

    if (SDL_LoadWAV(argv[1], &spec, &data, &len) == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "failed to load %s: %s\n", argv[1], SDL_GetError());
        SDL_Quit();
        return 3;
    }

    if (SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
                          spec.format, cvtchans, cvtfreq) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "failed to build CVT: %s\n", SDL_GetError());
        SDL_FreeWAV(data);
        SDL_Quit();
        return 4;
    }

    cvt.len = len;
    cvt.buf = (Uint8 *) SDL_malloc(len * cvt.len_mult);
    if (cvt.buf == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory.\n");
        SDL_FreeWAV(data);
        SDL_Quit();
        return 5;
    }
    SDL_memcpy(cvt.buf, data, len);

    if (SDL_ConvertAudio(&cvt) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Conversion failed: %s\n", SDL_GetError());
        SDL_free(cvt.buf);
        SDL_FreeWAV(data);
        SDL_Quit();
        return 6;
    }

    /* write out a WAV header... */
    io = SDL_RWFromFile(argv[2], "wb");
    if (io == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "fopen('%s') failed: %s\n", argv[2], SDL_GetError());
        SDL_free(cvt.buf);
        SDL_FreeWAV(data);
        SDL_Quit();
        return 7;
    }

    bitsize = SDL_AUDIO_BITSIZE(spec.format);
    blockalign = (bitsize / 8) * cvtchans;
    avgbytes = cvtfreq * blockalign;

    SDL_WriteLE32(io, 0x46464952);      /* RIFF */
    SDL_WriteLE32(io, cvt.len_cvt + 36);
    SDL_WriteLE32(io, 0x45564157);      /* WAVE */
    SDL_WriteLE32(io, 0x20746D66);      /* fmt */
    SDL_WriteLE32(io, 16);      /* chunk size */
    SDL_WriteLE16(io, SDL_AUDIO_ISFLOAT(spec.format) ? 3 : 1);       /* uncompressed */
    SDL_WriteLE16(io, cvtchans);   /* channels */
    SDL_WriteLE32(io, cvtfreq); /* sample rate */
    SDL_WriteLE32(io, avgbytes);        /* average bytes per second */
    SDL_WriteLE16(io, blockalign);      /* block align */
    SDL_WriteLE16(io, bitsize); /* significant bits per sample */
    SDL_WriteLE32(io, 0x61746164);      /* data */
    SDL_WriteLE32(io, cvt.len_cvt);     /* size */
    SDL_RWwrite(io, cvt.buf, cvt.len_cvt, 1);

    if (SDL_RWclose(io) == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "fclose('%s') failed: %s\n", argv[2], SDL_GetError());
        SDL_free(cvt.buf);
        SDL_FreeWAV(data);
        SDL_Quit();
        return 8;
    }                           /* if */

    SDL_free(cvt.buf);
    SDL_FreeWAV(data);
    SDL_Quit();
    return 0;
}                               /* main */

/* end of testresample.c ... */
