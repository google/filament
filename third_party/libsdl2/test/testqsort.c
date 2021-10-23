/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include "SDL_test.h"

static int
num_compare(const void *_a, const void *_b)
{
    const int a = *((const int *) _a);
    const int b = *((const int *) _b);
    return (a < b) ? -1 : ((a > b) ? 1 : 0);
}

static void
test_sort(const char *desc, int *nums, const int arraylen)
{
    int i;
    int prev;

    SDL_Log("test: %s arraylen=%d", desc, arraylen);

    SDL_qsort(nums, arraylen, sizeof (nums[0]), num_compare);

    prev = nums[0];
    for (i = 1; i < arraylen; i++) {
        const int val = nums[i];
        if (val < prev) {
            SDL_Log("sort is broken!");
            return;
        }
        prev = val;
    }
}

int
main(int argc, char *argv[])
{
    static int nums[1024 * 100];
    static const int itervals[] = { SDL_arraysize(nums), 12 };
    int iteration;
    SDLTest_RandomContext rndctx;

    if (argc > 1)
    {
        int success;
        Uint64 seed = 0;
        if (argv[1][0] == '0' && argv[1][1] == 'x')
            success = SDL_sscanf(argv[1] + 2, "%"SDL_PRIx64, &seed);
        else
            success = SDL_sscanf(argv[1], "%"SDL_PRIu64, &seed);
        if (!success) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid seed. Use a decimal or hexadecimal number.\n");
            return 1;
        }
        if (seed <= ((Uint64)0xffffffff)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Seed must be equal or greater than 0x100000000.\n");
            return 1;
        }
        SDLTest_RandomInit(&rndctx, (unsigned int)(seed >> 32), (unsigned int)(seed & 0xffffffff));
    }
    else
    {
        SDLTest_RandomInitTime(&rndctx);
    }
    SDL_Log("Using random seed 0x%08x%08x\n", rndctx.x, rndctx.c);

    for (iteration = 0; iteration < SDL_arraysize(itervals); iteration++) {
        const int arraylen = itervals[iteration];
        int i;

        for (i = 0; i < arraylen; i++) {
            nums[i] = i;
        }
        test_sort("already sorted", nums, arraylen);

        for (i = 0; i < arraylen; i++) {
            nums[i] = i;
        }
        nums[arraylen-1] = -1;
        test_sort("already sorted except last element", nums, arraylen);

        for (i = 0; i < arraylen; i++) {
            nums[i] = (arraylen-1) - i;
        }
        test_sort("reverse sorted", nums, arraylen);

        for (i = 0; i < arraylen; i++) {
            nums[i] = SDLTest_RandomInt(&rndctx);
        }
        test_sort("random sorted", nums, arraylen);
    }

    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */

