/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include <stdio.h>

#include "SDL.h"

static size_t
widelen(char *data)
{
    size_t len = 0;
    Uint32 *p = (Uint32 *) data;
    while (*p++) {
        ++len;
    }
    return len;
}

int
main(int argc, char *argv[])
{
    const char *formats[] = {
        "UTF8",
        "UTF-8",
        "UTF16BE",
        "UTF-16BE",
        "UTF16LE",
        "UTF-16LE",
        "UTF32BE",
        "UTF-32BE",
        "UTF32LE",
        "UTF-32LE",
        "UCS4",
        "UCS-4",
    };
    char buffer[BUFSIZ];
    char *ucs4;
    char *test[2];
    int i;
    FILE *file;
    int errors = 0;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (!argv[1]) {
        argv[1] = "utf8.txt";
    }
    file = fopen(argv[1], "rb");
    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to open %s\n", argv[1]);
        return (1);
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        /* Convert to UCS-4 */
        size_t len;
        ucs4 =
            SDL_iconv_string("UCS-4", "UTF-8", buffer,
                             SDL_strlen(buffer) + 1);
        len = (widelen(ucs4) + 1) * 4;
        for (i = 0; i < SDL_arraysize(formats); ++i) {
            test[0] = SDL_iconv_string(formats[i], "UCS-4", ucs4, len);
            test[1] = SDL_iconv_string("UCS-4", formats[i], test[0], len);
            if (!test[1] || SDL_memcmp(test[1], ucs4, len) != 0) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FAIL: %s\n", formats[i]);
                ++errors;
            }
            SDL_free(test[0]);
            SDL_free(test[1]);
        }
        test[0] = SDL_iconv_string("UTF-8", "UCS-4", ucs4, len);
        SDL_free(ucs4);
        fputs(test[0], stdout);
        SDL_free(test[0]);
    }
    fclose(file);
    return (errors ? errors + 1 : 0);
}
