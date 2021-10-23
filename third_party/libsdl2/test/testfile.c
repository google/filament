/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* sanity tests on SDL_rwops.c (usefull for alternative implementations of stdio rwops) */

/* quiet windows compiler warnings */
#define _CRT_NONSTDC_NO_WARNINGS

#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "SDL.h"


#include <stdio.h>

/* WARNING ! those 2 files will be destroyed by this test program */

#ifdef __IPHONEOS__
#define FBASENAME1  "../Documents/sdldata1" /* this file will be created during tests */
#define FBASENAME2  "../Documents/sdldata2"     /* this file should not exist before starting test */
#else
#define FBASENAME1  "sdldata1"      /* this file will be created during tests */
#define FBASENAME2  "sdldata2"      /* this file should not exist before starting test */
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

static void
cleanup(void)
{
    unlink(FBASENAME1);
    unlink(FBASENAME2);
}

static void
rwops_error_quit(unsigned line, SDL_RWops * rwops)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "testfile.c(%d): failed\n", line);
    if (rwops) {
        rwops->close(rwops);    /* This calls SDL_FreeRW(rwops); */
    }
    cleanup();
    exit(1);                    /* quit with rwops error (test failed) */
}

#define RWOP_ERR_QUIT(x)    rwops_error_quit( __LINE__, (x) )



int
main(int argc, char *argv[])
{
    SDL_RWops *rwops = NULL;
    char test_buf[30];

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    cleanup();

/* test 1 : basic argument test: all those calls to SDL_RWFromFile should fail */

    rwops = SDL_RWFromFile(NULL, NULL);
    if (rwops)
        RWOP_ERR_QUIT(rwops);
    rwops = SDL_RWFromFile(NULL, "ab+");
    if (rwops)
        RWOP_ERR_QUIT(rwops);
    rwops = SDL_RWFromFile(NULL, "sldfkjsldkfj");
    if (rwops)
        RWOP_ERR_QUIT(rwops);
    rwops = SDL_RWFromFile("something", "");
    if (rwops)
        RWOP_ERR_QUIT(rwops);
    rwops = SDL_RWFromFile("something", NULL);
    if (rwops)
        RWOP_ERR_QUIT(rwops);
    SDL_Log("test1 OK\n");

/* test 2 : check that inexistent file is not successfully opened/created when required */
/* modes : r, r+ imply that file MUST exist
   modes : a, a+, w, w+ checks that it succeeds (file may not exists)

 */
    rwops = SDL_RWFromFile(FBASENAME2, "rb");   /* this file doesn't exist that call must fail */
    if (rwops)
        RWOP_ERR_QUIT(rwops);
    rwops = SDL_RWFromFile(FBASENAME2, "rb+");  /* this file doesn't exist that call must fail */
    if (rwops)
        RWOP_ERR_QUIT(rwops);
    rwops = SDL_RWFromFile(FBASENAME2, "wb");
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    rwops->close(rwops);
    unlink(FBASENAME2);
    rwops = SDL_RWFromFile(FBASENAME2, "wb+");
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    rwops->close(rwops);
    unlink(FBASENAME2);
    rwops = SDL_RWFromFile(FBASENAME2, "ab");
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    rwops->close(rwops);
    unlink(FBASENAME2);
    rwops = SDL_RWFromFile(FBASENAME2, "ab+");
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    rwops->close(rwops);
    unlink(FBASENAME2);
    SDL_Log("test2 OK\n");

/* test 3 : creation, writing , reading, seeking,
            test : w mode, r mode, w+ mode
 */
    rwops = SDL_RWFromFile(FBASENAME1, "wb");   /* write only */
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    if (1 != rwops->write(rwops, "1234567890", 10, 1))
        RWOP_ERR_QUIT(rwops);
    if (10 != rwops->write(rwops, "1234567890", 1, 10))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->write(rwops, "1234567", 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);   /* we are in write only mode */
    rwops->close(rwops);

    rwops = SDL_RWFromFile(FBASENAME1, "rb");   /* read mode, file must exists */
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);
    if (20 != rwops->seek(rwops, -7, RW_SEEK_END))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->read(rwops, test_buf, 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "1234567", 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 10, 100))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, -27, RW_SEEK_CUR))
        RWOP_ERR_QUIT(rwops);
    if (2 != rwops->read(rwops, test_buf, 10, 3))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "12345678901234567890", 20))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->write(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);   /* readonly mode */
    rwops->close(rwops);

/* test 3: same with w+ mode */
    rwops = SDL_RWFromFile(FBASENAME1, "wb+");  /* write + read + truncation */
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    if (1 != rwops->write(rwops, "1234567890", 10, 1))
        RWOP_ERR_QUIT(rwops);
    if (10 != rwops->write(rwops, "1234567890", 1, 10))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->write(rwops, "1234567", 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);
    if (1 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);   /* we are in read/write mode */
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);
    if (20 != rwops->seek(rwops, -7, RW_SEEK_END))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->read(rwops, test_buf, 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "1234567", 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 10, 100))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, -27, RW_SEEK_CUR))
        RWOP_ERR_QUIT(rwops);
    if (2 != rwops->read(rwops, test_buf, 10, 3))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "12345678901234567890", 20))
        RWOP_ERR_QUIT(rwops);
    rwops->close(rwops);
    SDL_Log("test3 OK\n");

/* test 4: same in r+ mode */
    rwops = SDL_RWFromFile(FBASENAME1, "rb+");  /* write + read + file must exists, no truncation */
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    if (1 != rwops->write(rwops, "1234567890", 10, 1))
        RWOP_ERR_QUIT(rwops);
    if (10 != rwops->write(rwops, "1234567890", 1, 10))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->write(rwops, "1234567", 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);
    if (1 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);   /* we are in read/write mode */
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);
    if (20 != rwops->seek(rwops, -7, RW_SEEK_END))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->read(rwops, test_buf, 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "1234567", 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 10, 100))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, -27, RW_SEEK_CUR))
        RWOP_ERR_QUIT(rwops);
    if (2 != rwops->read(rwops, test_buf, 10, 3))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "12345678901234567890", 20))
        RWOP_ERR_QUIT(rwops);
    rwops->close(rwops);
    SDL_Log("test4 OK\n");

/* test5 : append mode */
    rwops = SDL_RWFromFile(FBASENAME1, "ab+");  /* write + read + append */
    if (!rwops)
        RWOP_ERR_QUIT(rwops);
    if (1 != rwops->write(rwops, "1234567890", 10, 1))
        RWOP_ERR_QUIT(rwops);
    if (10 != rwops->write(rwops, "1234567890", 1, 10))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->write(rwops, "1234567", 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);

    if (1 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);

    if (20 + 27 != rwops->seek(rwops, -7, RW_SEEK_END))
        RWOP_ERR_QUIT(rwops);
    if (7 != rwops->read(rwops, test_buf, 1, 7))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "1234567", 7))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 1, 1))
        RWOP_ERR_QUIT(rwops);
    if (0 != rwops->read(rwops, test_buf, 10, 100))
        RWOP_ERR_QUIT(rwops);

    if (27 != rwops->seek(rwops, -27, RW_SEEK_CUR))
        RWOP_ERR_QUIT(rwops);

    if (0 != rwops->seek(rwops, 0L, RW_SEEK_SET))
        RWOP_ERR_QUIT(rwops);
    if (3 != rwops->read(rwops, test_buf, 10, 3))
        RWOP_ERR_QUIT(rwops);
    if (SDL_memcmp(test_buf, "123456789012345678901234567123", 30))
        RWOP_ERR_QUIT(rwops);
    rwops->close(rwops);
    SDL_Log("test5 OK\n");
    cleanup();
    return 0;                   /* all ok */
}
