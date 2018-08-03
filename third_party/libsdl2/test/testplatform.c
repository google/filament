/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include <stdio.h>

#include "SDL.h"

/*
 * Watcom C flags these as Warning 201: "Unreachable code" if you just
 *  compare them directly, so we push it through a function to keep the
 *  compiler quiet.  --ryan.
 */
static int
badsize(size_t sizeoftype, size_t hardcodetype)
{
    return sizeoftype != hardcodetype;
}

int
TestTypes(SDL_bool verbose)
{
    int error = 0;

	SDL_COMPILE_TIME_ASSERT(SDL_MAX_SINT8, SDL_MAX_SINT8 == 127);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_SINT8, SDL_MIN_SINT8 == -128);
	SDL_COMPILE_TIME_ASSERT(SDL_MAX_UINT8, SDL_MAX_UINT8 == 255);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_UINT8, SDL_MIN_UINT8 == 0);

	SDL_COMPILE_TIME_ASSERT(SDL_MAX_SINT16, SDL_MAX_SINT16 == 32767);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_SINT16, SDL_MIN_SINT16 == -32768);
	SDL_COMPILE_TIME_ASSERT(SDL_MAX_UINT16, SDL_MAX_UINT16 == 65535);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_UINT16, SDL_MIN_UINT16 == 0);

	SDL_COMPILE_TIME_ASSERT(SDL_MAX_SINT32, SDL_MAX_SINT32 == 2147483647);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_SINT32, SDL_MIN_SINT32 == ~0x7fffffff); /* Instead of -2147483648, which is treated as unsigned by some compilers */
	SDL_COMPILE_TIME_ASSERT(SDL_MAX_UINT32, SDL_MAX_UINT32 == 4294967295u);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_UINT32, SDL_MIN_UINT32 == 0);

	SDL_COMPILE_TIME_ASSERT(SDL_MAX_SINT64, SDL_MAX_SINT64 == 9223372036854775807ll);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_SINT64, SDL_MIN_SINT64 == ~0x7fffffffffffffffll); /* Instead of -9223372036854775808, which is treated as unsigned by compilers */
	SDL_COMPILE_TIME_ASSERT(SDL_MAX_UINT64, SDL_MAX_UINT64 == 18446744073709551615ull);
	SDL_COMPILE_TIME_ASSERT(SDL_MIN_UINT64, SDL_MIN_UINT64 == 0);

    if (badsize(sizeof(Uint8), 1)) {
        if (verbose)
            SDL_Log("sizeof(Uint8) != 1, instead = %u\n",
                   (unsigned int)sizeof(Uint8));
        ++error;
    }
    if (badsize(sizeof(Uint16), 2)) {
        if (verbose)
            SDL_Log("sizeof(Uint16) != 2, instead = %u\n",
                   (unsigned int)sizeof(Uint16));
        ++error;
    }
    if (badsize(sizeof(Uint32), 4)) {
        if (verbose)
            SDL_Log("sizeof(Uint32) != 4, instead = %u\n",
                   (unsigned int)sizeof(Uint32));
        ++error;
    }
    if (badsize(sizeof(Uint64), 8)) {
        if (verbose)
            SDL_Log("sizeof(Uint64) != 8, instead = %u\n",
                   (unsigned int)sizeof(Uint64));
        ++error;
    }
    if (verbose && !error)
        SDL_Log("All data types are the expected size.\n");

    return (error ? 1 : 0);
}

int
TestEndian(SDL_bool verbose)
{
    int error = 0;
    Uint16 value = 0x1234;
    int real_byteorder;
    Uint16 value16 = 0xCDAB;
    Uint16 swapped16 = 0xABCD;
    Uint32 value32 = 0xEFBEADDE;
    Uint32 swapped32 = 0xDEADBEEF;
    Uint64 value64, swapped64;

    value64 = 0xEFBEADDE;
    value64 <<= 32;
    value64 |= 0xCDAB3412;
    swapped64 = 0x1234ABCD;
    swapped64 <<= 32;
    swapped64 |= 0xDEADBEEF;

    if (verbose) {
        SDL_Log("Detected a %s endian machine.\n",
               (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? "little" : "big");
    }
    if ((*((char *) &value) >> 4) == 0x1) {
        real_byteorder = SDL_BIG_ENDIAN;
    } else {
        real_byteorder = SDL_LIL_ENDIAN;
    }
    if (real_byteorder != SDL_BYTEORDER) {
        if (verbose) {
            SDL_Log("Actually a %s endian machine!\n",
                   (real_byteorder == SDL_LIL_ENDIAN) ? "little" : "big");
        }
        ++error;
    }
    if (verbose) {
        SDL_Log("Value 16 = 0x%X, swapped = 0x%X\n", value16,
               SDL_Swap16(value16));
    }
    if (SDL_Swap16(value16) != swapped16) {
        if (verbose) {
            SDL_Log("16 bit value swapped incorrectly!\n");
        }
        ++error;
    }
    if (verbose) {
        SDL_Log("Value 32 = 0x%X, swapped = 0x%X\n", value32,
               SDL_Swap32(value32));
    }
    if (SDL_Swap32(value32) != swapped32) {
        if (verbose) {
            SDL_Log("32 bit value swapped incorrectly!\n");
        }
        ++error;
    }
    if (verbose) {
        SDL_Log("Value 64 = 0x%"SDL_PRIX64", swapped = 0x%"SDL_PRIX64"\n", value64,
               SDL_Swap64(value64));
    }
    if (SDL_Swap64(value64) != swapped64) {
        if (verbose) {
            SDL_Log("64 bit value swapped incorrectly!\n");
        }
        ++error;
    }
    return (error ? 1 : 0);
}

static int TST_allmul (void *a, void *b, int arg, void *result, void *expected)
{
    (*(long long *)result) = ((*(long long *)a) * (*(long long *)b));
    return (*(long long *)result) == (*(long long *)expected);
}

static int TST_alldiv (void *a, void *b, int arg, void *result, void *expected)
{
    (*(long long *)result) = ((*(long long *)a) / (*(long long *)b));
    return (*(long long *)result) == (*(long long *)expected);
}

static int TST_allrem (void *a, void *b, int arg, void *result, void *expected)
{
    (*(long long *)result) = ((*(long long *)a) % (*(long long *)b));
    return (*(long long *)result) == (*(long long *)expected);
}

static int TST_ualldiv (void *a, void *b, int arg, void *result, void *expected)
{
    (*(unsigned long long *)result) = ((*(unsigned long long *)a) / (*(unsigned long long *)b));
    return (*(unsigned long long *)result) == (*(unsigned long long *)expected);
}

static int TST_uallrem (void *a, void *b, int arg, void *result, void *expected)
{
    (*(unsigned long long *)result) = ((*(unsigned long long *)a) % (*(unsigned long long *)b));
    return (*(unsigned long long *)result) == (*(unsigned long long *)expected);
}

static int TST_allshl (void *a, void *b, int arg, void *result, void *expected)
{
    (*(long long *)result) = (*(long long *)a) << arg;
    return (*(long long *)result) == (*(long long *)expected);
}

static int TST_aullshl (void *a, void *b, int arg, void *result, void *expected)
{
    (*(unsigned long long *)result) = (*(unsigned long long *)a) << arg;
    return (*(unsigned long long *)result) == (*(unsigned long long *)expected);
}

static int TST_allshr (void *a, void *b, int arg, void *result, void *expected)
{
    (*(long long *)result) = (*(long long *)a) >> arg;
    return (*(long long *)result) == (*(long long *)expected);
}

static int TST_aullshr (void *a, void *b, int arg, void *result, void *expected)
{
    (*(unsigned long long *)result) = (*(unsigned long long *)a) >> arg;
    return (*(unsigned long long *)result) == (*(unsigned long long *)expected);
}


typedef int (*LL_Intrinsic)(void *a, void *b, int arg, void *result, void *expected);

typedef struct {
    const char *operation;
    LL_Intrinsic routine;
    unsigned long long a, b;
    int arg;
    unsigned long long expected_result;
} LL_Test;

static LL_Test LL_Tests[] = 
{
    /* UNDEFINED {"_allshl",   &TST_allshl,   0xFFFFFFFFFFFFFFFFll,                  0ll, 65, 0x0000000000000000ll}, */
    {"_allshl",   &TST_allshl,   0xFFFFFFFFFFFFFFFFll,                  0ll,  1, 0xFFFFFFFFFFFFFFFEll},
    {"_allshl",   &TST_allshl,   0xFFFFFFFFFFFFFFFFll,                  0ll, 32, 0xFFFFFFFF00000000ll},
    {"_allshl",   &TST_allshl,   0xFFFFFFFFFFFFFFFFll,                  0ll, 33, 0xFFFFFFFE00000000ll},
    {"_allshl",   &TST_allshl,   0xFFFFFFFFFFFFFFFFll,                  0ll,  0, 0xFFFFFFFFFFFFFFFFll},

    {"_allshr",   &TST_allshr,   0xAAAAAAAA55555555ll,                  0ll, 63, 0xFFFFFFFFFFFFFFFFll},
    /* UNDEFINED {"_allshr",   &TST_allshr,   0xFFFFFFFFFFFFFFFFll,                  0ll, 65, 0xFFFFFFFFFFFFFFFFll}, */
    {"_allshr",   &TST_allshr,   0xFFFFFFFFFFFFFFFFll,                  0ll,  1, 0xFFFFFFFFFFFFFFFFll},
    {"_allshr",   &TST_allshr,   0xFFFFFFFFFFFFFFFFll,                  0ll, 32, 0xFFFFFFFFFFFFFFFFll},
    {"_allshr",   &TST_allshr,   0xFFFFFFFFFFFFFFFFll,                  0ll, 33, 0xFFFFFFFFFFFFFFFFll},
    {"_allshr",   &TST_allshr,   0xFFFFFFFFFFFFFFFFll,                  0ll,  0, 0xFFFFFFFFFFFFFFFFll},
    /* UNDEFINED {"_allshr",   &TST_allshr,   0x5F5F5F5F5F5F5F5Fll,                  0ll, 65, 0x0000000000000000ll}, */
    {"_allshr",   &TST_allshr,   0x5F5F5F5F5F5F5F5Fll,                  0ll,  1, 0x2FAFAFAFAFAFAFAFll},
    {"_allshr",   &TST_allshr,   0x5F5F5F5F5F5F5F5Fll,                  0ll, 32, 0x000000005F5F5F5Fll},
    {"_allshr",   &TST_allshr,   0x5F5F5F5F5F5F5F5Fll,                  0ll, 33, 0x000000002FAFAFAFll},

    /* UNDEFINED {"_aullshl",  &TST_aullshl,  0xFFFFFFFFFFFFFFFFll,                  0ll, 65, 0x0000000000000000ll}, */
    {"_aullshl",  &TST_aullshl,  0xFFFFFFFFFFFFFFFFll,                  0ll,  1, 0xFFFFFFFFFFFFFFFEll},
    {"_aullshl",  &TST_aullshl,  0xFFFFFFFFFFFFFFFFll,                  0ll, 32, 0xFFFFFFFF00000000ll},
    {"_aullshl",  &TST_aullshl,  0xFFFFFFFFFFFFFFFFll,                  0ll, 33, 0xFFFFFFFE00000000ll},
    {"_aullshl",  &TST_aullshl,  0xFFFFFFFFFFFFFFFFll,                  0ll,  0, 0xFFFFFFFFFFFFFFFFll},

    /* UNDEFINED {"_aullshr",  &TST_aullshr,  0xFFFFFFFFFFFFFFFFll,                  0ll, 65, 0x0000000000000000ll}, */
    {"_aullshr",  &TST_aullshr,  0xFFFFFFFFFFFFFFFFll,                  0ll,  1, 0x7FFFFFFFFFFFFFFFll},
    {"_aullshr",  &TST_aullshr,  0xFFFFFFFFFFFFFFFFll,                  0ll, 32, 0x00000000FFFFFFFFll},
    {"_aullshr",  &TST_aullshr,  0xFFFFFFFFFFFFFFFFll,                  0ll, 33, 0x000000007FFFFFFFll},
    {"_aullshr",  &TST_aullshr,  0xFFFFFFFFFFFFFFFFll,                  0ll,  0, 0xFFFFFFFFFFFFFFFFll},

    {"_allmul",   &TST_allmul,   0xFFFFFFFFFFFFFFFFll, 0x0000000000000000ll,  0, 0x0000000000000000ll},
    {"_allmul",   &TST_allmul,   0x0000000000000000ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_allmul",   &TST_allmul,   0x000000000FFFFFFFll, 0x0000000000000001ll,  0, 0x000000000FFFFFFFll},
    {"_allmul",   &TST_allmul,   0x0000000000000001ll, 0x000000000FFFFFFFll,  0, 0x000000000FFFFFFFll},
    {"_allmul",   &TST_allmul,   0x000000000FFFFFFFll, 0x0000000000000010ll,  0, 0x00000000FFFFFFF0ll},
    {"_allmul",   &TST_allmul,   0x0000000000000010ll, 0x000000000FFFFFFFll,  0, 0x00000000FFFFFFF0ll},
    {"_allmul",   &TST_allmul,   0x000000000FFFFFFFll, 0x0000000000000100ll,  0, 0x0000000FFFFFFF00ll},
    {"_allmul",   &TST_allmul,   0x0000000000000100ll, 0x000000000FFFFFFFll,  0, 0x0000000FFFFFFF00ll},
    {"_allmul",   &TST_allmul,   0x000000000FFFFFFFll, 0x0000000010000000ll,  0, 0x00FFFFFFF0000000ll},
    {"_allmul",   &TST_allmul,   0x0000000010000000ll, 0x000000000FFFFFFFll,  0, 0x00FFFFFFF0000000ll},
    {"_allmul",   &TST_allmul,   0x000000000FFFFFFFll, 0x0000000080000000ll,  0, 0x07FFFFFF80000000ll},
    {"_allmul",   &TST_allmul,   0x0000000080000000ll, 0x000000000FFFFFFFll,  0, 0x07FFFFFF80000000ll},
    {"_allmul",   &TST_allmul,   0xFFFFFFFFFFFFFFFEll, 0x0000000080000000ll,  0, 0xFFFFFFFF00000000ll},
    {"_allmul",   &TST_allmul,   0x0000000080000000ll, 0xFFFFFFFFFFFFFFFEll,  0, 0xFFFFFFFF00000000ll},
    {"_allmul",   &TST_allmul,   0xFFFFFFFFFFFFFFFEll, 0x0000000080000008ll,  0, 0xFFFFFFFEFFFFFFF0ll},
    {"_allmul",   &TST_allmul,   0x0000000080000008ll, 0xFFFFFFFFFFFFFFFEll,  0, 0xFFFFFFFEFFFFFFF0ll},
    {"_allmul",   &TST_allmul,   0x00000000FFFFFFFFll, 0x00000000FFFFFFFFll,  0, 0xFFFFFFFE00000001ll},

    {"_alldiv",   &TST_alldiv,   0x0000000000000000ll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_alldiv",   &TST_alldiv,   0x0000000000000000ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_alldiv",   &TST_alldiv,   0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0xFFFFFFFFFFFFFFFFll},
    {"_alldiv",   &TST_alldiv,   0xFFFFFFFFFFFFFFFFll, 0x0000000000000001ll,  0, 0xFFFFFFFFFFFFFFFFll},
    {"_alldiv",   &TST_alldiv,   0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0xFFFFFFFFFFFFFFFFll},
    {"_alldiv",   &TST_alldiv,   0x0000000000000001ll, 0x0000000000000001ll,  0, 0x0000000000000001ll},
    {"_alldiv",   &TST_alldiv,   0xFFFFFFFFFFFFFFFFll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000001ll},
    {"_alldiv",   &TST_alldiv,   0x000000000FFFFFFFll, 0x0000000000000001ll,  0, 0x000000000FFFFFFFll},
    {"_alldiv",   &TST_alldiv,   0x0000000FFFFFFFFFll, 0x0000000000000010ll,  0, 0x00000000FFFFFFFFll},
    {"_alldiv",   &TST_alldiv,   0x0000000000000100ll, 0x000000000FFFFFFFll,  0, 0x0000000000000000ll},
    {"_alldiv",   &TST_alldiv,   0x00FFFFFFF0000000ll, 0x0000000010000000ll,  0, 0x000000000FFFFFFFll},
    {"_alldiv",   &TST_alldiv,   0x07FFFFFF80000000ll, 0x0000000080000000ll,  0, 0x000000000FFFFFFFll},
    {"_alldiv",   &TST_alldiv,   0xFFFFFFFFFFFFFFFEll, 0x0000000080000000ll,  0, 0x0000000000000000ll},
    {"_alldiv",   &TST_alldiv,   0xFFFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0x0000000080000008ll},
    {"_alldiv",   &TST_alldiv,   0x7FFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0xC000000080000008ll},
    {"_alldiv",   &TST_alldiv,   0x7FFFFFFEFFFFFFF0ll, 0x0000FFFFFFFFFFFEll,  0, 0x0000000000007FFFll},
    {"_alldiv",   &TST_alldiv,   0x7FFFFFFEFFFFFFF0ll, 0x7FFFFFFEFFFFFFF0ll,  0, 0x0000000000000001ll},

    {"_allrem",   &TST_allrem,   0x0000000000000000ll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x0000000000000000ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0xFFFFFFFFFFFFFFFFll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x0000000000000001ll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0xFFFFFFFFFFFFFFFFll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x000000000FFFFFFFll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x0000000FFFFFFFFFll, 0x0000000000000010ll,  0, 0x000000000000000Fll},
    {"_allrem",   &TST_allrem,   0x0000000000000100ll, 0x000000000FFFFFFFll,  0, 0x0000000000000100ll},
    {"_allrem",   &TST_allrem,   0x00FFFFFFF0000000ll, 0x0000000010000000ll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x07FFFFFF80000000ll, 0x0000000080000000ll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0xFFFFFFFFFFFFFFFEll, 0x0000000080000000ll,  0, 0xFFFFFFFFFFFFFFFEll},
    {"_allrem",   &TST_allrem,   0xFFFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x7FFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0x0000000000000000ll},
    {"_allrem",   &TST_allrem,   0x7FFFFFFEFFFFFFF0ll, 0x0000FFFFFFFFFFFEll,  0, 0x0000FFFF0000FFEEll},
    {"_allrem",   &TST_allrem,   0x7FFFFFFEFFFFFFF0ll, 0x7FFFFFFEFFFFFFF0ll,  0, 0x0000000000000000ll},


    {"_ualldiv",  &TST_ualldiv,  0x0000000000000000ll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_ualldiv",  &TST_ualldiv,  0x0000000000000000ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_ualldiv",  &TST_ualldiv,  0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_ualldiv",  &TST_ualldiv,  0xFFFFFFFFFFFFFFFFll, 0x0000000000000001ll,  0, 0xFFFFFFFFFFFFFFFFll},
    {"_ualldiv",  &TST_ualldiv,  0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_ualldiv",  &TST_ualldiv,  0x0000000000000001ll, 0x0000000000000001ll,  0, 0x0000000000000001ll},
    {"_ualldiv",  &TST_ualldiv,  0xFFFFFFFFFFFFFFFFll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000001ll},
    {"_ualldiv",  &TST_ualldiv,  0x000000000FFFFFFFll, 0x0000000000000001ll,  0, 0x000000000FFFFFFFll},
    {"_ualldiv",  &TST_ualldiv,  0x0000000FFFFFFFFFll, 0x0000000000000010ll,  0, 0x00000000FFFFFFFFll},
    {"_ualldiv",  &TST_ualldiv,  0x0000000000000100ll, 0x000000000FFFFFFFll,  0, 0x0000000000000000ll},
    {"_ualldiv",  &TST_ualldiv,  0x00FFFFFFF0000000ll, 0x0000000010000000ll,  0, 0x000000000FFFFFFFll},
    {"_ualldiv",  &TST_ualldiv,  0x07FFFFFF80000000ll, 0x0000000080000000ll,  0, 0x000000000FFFFFFFll},
    {"_ualldiv",  &TST_ualldiv,  0xFFFFFFFFFFFFFFFEll, 0x0000000080000000ll,  0, 0x00000001FFFFFFFFll},
    {"_ualldiv",  &TST_ualldiv,  0xFFFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0x0000000000000000ll},
    {"_ualldiv",  &TST_ualldiv,  0x7FFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0x0000000000000000ll},
    {"_ualldiv",  &TST_ualldiv,  0x7FFFFFFEFFFFFFF0ll, 0x0000FFFFFFFFFFFEll,  0, 0x0000000000007FFFll},
    {"_ualldiv",  &TST_ualldiv,  0x7FFFFFFEFFFFFFF0ll, 0x7FFFFFFEFFFFFFF0ll,  0, 0x0000000000000001ll},

    {"_uallrem",  &TST_uallrem,  0x0000000000000000ll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0x0000000000000000ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000001ll},
    {"_uallrem",  &TST_uallrem,  0xFFFFFFFFFFFFFFFFll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0x0000000000000001ll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000001ll},
    {"_uallrem",  &TST_uallrem,  0x0000000000000001ll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0xFFFFFFFFFFFFFFFFll, 0xFFFFFFFFFFFFFFFFll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0x000000000FFFFFFFll, 0x0000000000000001ll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0x0000000FFFFFFFFFll, 0x0000000000000010ll,  0, 0x000000000000000Fll},
    {"_uallrem",  &TST_uallrem,  0x0000000000000100ll, 0x000000000FFFFFFFll,  0, 0x0000000000000100ll},
    {"_uallrem",  &TST_uallrem,  0x00FFFFFFF0000000ll, 0x0000000010000000ll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0x07FFFFFF80000000ll, 0x0000000080000000ll,  0, 0x0000000000000000ll},
    {"_uallrem",  &TST_uallrem,  0xFFFFFFFFFFFFFFFEll, 0x0000000080000000ll,  0, 0x000000007FFFFFFEll},
    {"_uallrem",  &TST_uallrem,  0xFFFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0xFFFFFFFEFFFFFFF0ll},
    {"_uallrem",  &TST_uallrem,  0x7FFFFFFEFFFFFFF0ll, 0xFFFFFFFFFFFFFFFEll,  0, 0x7FFFFFFEFFFFFFF0ll},
    {"_uallrem",  &TST_uallrem,  0x7FFFFFFEFFFFFFF0ll, 0x0000FFFFFFFFFFFEll,  0, 0x0000FFFF0000FFEEll},
    {"_uallrem",  &TST_uallrem,  0x7FFFFFFEFFFFFFF0ll, 0x7FFFFFFEFFFFFFF0ll,  0, 0x0000000000000000ll},

    {NULL}
};

int
Test64Bit (SDL_bool verbose)
{
    LL_Test *t;
    int failed = 0;

    for (t = LL_Tests; t->routine != NULL; t++) {
        unsigned long long result = 0;
        unsigned int *al = (unsigned int *)&t->a;
        unsigned int *bl = (unsigned int *)&t->b;
        unsigned int *el = (unsigned int *)&t->expected_result;
        unsigned int *rl = (unsigned int *)&result;

        if (!t->routine(&t->a, &t->b, t->arg, &result, &t->expected_result)) {
            if (verbose)
                SDL_Log("%s(0x%08X%08X, 0x%08X%08X, %3d, produced: 0x%08X%08X, expected: 0x%08X%08X\n",
                        t->operation, al[1], al[0], bl[1], bl[0], t->arg, rl[1], rl[0], el[1], el[0]);
            ++failed;
        }
    }
    if (verbose && (failed == 0))
        SDL_Log("All 64bit instrinsic tests passed\n");
    return (failed ? 1 : 0);
}

int
TestCPUInfo(SDL_bool verbose)
{
    if (verbose) {
        SDL_Log("CPU count: %d\n", SDL_GetCPUCount());
        SDL_Log("CPU cache line size: %d\n", SDL_GetCPUCacheLineSize());
        SDL_Log("RDTSC %s\n", SDL_HasRDTSC()? "detected" : "not detected");
        SDL_Log("AltiVec %s\n", SDL_HasAltiVec()? "detected" : "not detected");
        SDL_Log("MMX %s\n", SDL_HasMMX()? "detected" : "not detected");
        SDL_Log("3DNow! %s\n", SDL_Has3DNow()? "detected" : "not detected");
        SDL_Log("SSE %s\n", SDL_HasSSE()? "detected" : "not detected");
        SDL_Log("SSE2 %s\n", SDL_HasSSE2()? "detected" : "not detected");
        SDL_Log("SSE3 %s\n", SDL_HasSSE3()? "detected" : "not detected");
        SDL_Log("SSE4.1 %s\n", SDL_HasSSE41()? "detected" : "not detected");
        SDL_Log("SSE4.2 %s\n", SDL_HasSSE42()? "detected" : "not detected");
        SDL_Log("AVX %s\n", SDL_HasAVX()? "detected" : "not detected");
        SDL_Log("AVX2 %s\n", SDL_HasAVX2()? "detected" : "not detected");
        SDL_Log("NEON %s\n", SDL_HasNEON()? "detected" : "not detected");
        SDL_Log("System RAM %d MB\n", SDL_GetSystemRAM());
    }
    return (0);
}

int
TestAssertions(SDL_bool verbose)
{
    SDL_assert(1);
    SDL_assert_release(1);
    SDL_assert_paranoid(1);
    SDL_assert(0 || 1);
    SDL_assert_release(0 || 1);
    SDL_assert_paranoid(0 || 1);

#if 0   /* enable this to test assertion failures. */
    SDL_assert_release(1 == 2);
    SDL_assert_release(5 < 4);
    SDL_assert_release(0 && "This is a test");
#endif

    {
        const SDL_AssertData *item = SDL_GetAssertionReport();
        while (item) {
            SDL_Log("'%s', %s (%s:%d), triggered %u times, always ignore: %s.\n",
                item->condition, item->function, item->filename,
                item->linenum, item->trigger_count,
                item->always_ignore ? "yes" : "no");
            item = item->next;
        }
    }
    return (0);
}

int
main(int argc, char *argv[])
{
    SDL_bool verbose = SDL_TRUE;
    int status = 0;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (argv[1] && (SDL_strcmp(argv[1], "-q") == 0)) {
        verbose = SDL_FALSE;
    }
    if (verbose) {
        SDL_Log("This system is running %s\n", SDL_GetPlatform());
    }

    status += TestTypes(verbose);
    status += TestEndian(verbose);
    status += Test64Bit(verbose);
    status += TestCPUInfo(verbose);
    status += TestAssertions(verbose);

    return status;
}
