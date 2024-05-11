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

/*

  Data generators for fuzzing test data in a reproducible way.

*/

#include "SDL_config.h"

#include <limits.h>
/* Visual Studio 2008 doesn't have stdint.h */
#if defined(_MSC_VER) && _MSC_VER <= 1500
#define UINT8_MAX   _UI8_MAX
#define UINT16_MAX  _UI16_MAX
#define UINT32_MAX  _UI32_MAX
#define INT64_MIN    _I64_MIN
#define INT64_MAX    _I64_MAX
#define UINT64_MAX  _UI64_MAX
#else
#include <stdint.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "SDL_test.h"

/**
 * Counter for fuzzer invocations
 */
static int fuzzerInvocationCounter = 0;

/**
 * Context for shared random number generator
 */
static SDLTest_RandomContext rndContext;

/*
 * Note: doxygen documentation markup for functions is in the header file.
 */

void
SDLTest_FuzzerInit(Uint64 execKey)
{
    Uint32 a = (execKey >> 32) & 0x00000000FFFFFFFF;
    Uint32 b = execKey & 0x00000000FFFFFFFF;
    SDL_memset((void *)&rndContext, 0, sizeof(SDLTest_RandomContext));
    SDLTest_RandomInit(&rndContext, a, b);
    fuzzerInvocationCounter = 0;
}

int
SDLTest_GetFuzzerInvocationCount()
{
    return fuzzerInvocationCounter;
}

Uint8
SDLTest_RandomUint8()
{
    fuzzerInvocationCounter++;

    return (Uint8) SDLTest_RandomInt(&rndContext) & 0x000000FF;
}

Sint8
SDLTest_RandomSint8()
{
    fuzzerInvocationCounter++;

    return (Sint8) SDLTest_RandomInt(&rndContext) & 0x000000FF;
}

Uint16
SDLTest_RandomUint16()
{
    fuzzerInvocationCounter++;

    return (Uint16) SDLTest_RandomInt(&rndContext) & 0x0000FFFF;
}

Sint16
SDLTest_RandomSint16()
{
    fuzzerInvocationCounter++;

    return (Sint16) SDLTest_RandomInt(&rndContext) & 0x0000FFFF;
}

Sint32
SDLTest_RandomSint32()
{
    fuzzerInvocationCounter++;

    return (Sint32) SDLTest_RandomInt(&rndContext);
}

Uint32
SDLTest_RandomUint32()
{
    fuzzerInvocationCounter++;

    return (Uint32) SDLTest_RandomInt(&rndContext);
}

Uint64
SDLTest_RandomUint64()
{
    union {
        Uint64 v64;
        Uint32 v32[2];
    } value;
    value.v64 = 0;

    fuzzerInvocationCounter++;

    value.v32[0] = SDLTest_RandomSint32();
    value.v32[1] = SDLTest_RandomSint32();

    return value.v64;
}

Sint64
SDLTest_RandomSint64()
{
    union {
        Uint64 v64;
        Uint32 v32[2];
    } value;
    value.v64 = 0;

    fuzzerInvocationCounter++;

    value.v32[0] = SDLTest_RandomSint32();
    value.v32[1] = SDLTest_RandomSint32();

    return (Sint64)value.v64;
}



Sint32
SDLTest_RandomIntegerInRange(Sint32 pMin, Sint32 pMax)
{
    Sint64 min = pMin;
    Sint64 max = pMax;
    Sint64 temp;
    Sint64 number;

    if(pMin > pMax) {
        temp = min;
        min = max;
        max = temp;
    } else if(pMin == pMax) {
        return (Sint32)min;
    }

    number = SDLTest_RandomUint32();
    /* invocation count increment in preceeding call */

    return (Sint32)((number % ((max + 1) - min)) + min);
}

/* !
 * Generates a unsigned boundary value between the given boundaries.
 * Boundary values are inclusive. See the examples below.
 * If boundary2 < boundary1, the values are swapped.
 * If boundary1 == boundary2, value of boundary1 will be returned
 *
 * Generating boundary values for Uint8:
 * BoundaryValues(UINT8_MAX, 10, 20, True) -> [10,11,19,20]
 * BoundaryValues(UINT8_MAX, 10, 20, False) -> [9,21]
 * BoundaryValues(UINT8_MAX, 0, 15, True) -> [0, 1, 14, 15]
 * BoundaryValues(UINT8_MAX, 0, 15, False) -> [16]
 * BoundaryValues(UINT8_MAX, 0, 0xFF, False) -> [0], error set
 *
 * Generator works the same for other types of unsigned integers.
 *
 * \param maxValue The biggest value that is acceptable for this data type.
 *                  For instance, for Uint8 -> 255, Uint16 -> 65536 etc.
 * \param boundary1 defines lower boundary
 * \param boundary2 defines upper boundary
 * \param validDomain Generate only for valid domain (for the data type)
 *
 * \returns Returns a random boundary value for the domain or 0 in case of error
 */
static Uint64
SDLTest_GenerateUnsignedBoundaryValues(const Uint64 maxValue, Uint64 boundary1, Uint64 boundary2, SDL_bool validDomain)
{
        Uint64 b1, b2;
    Uint64 delta;
    Uint64 tempBuf[4];
    Uint8 index;

        /* Maybe swap */
    if (boundary1 > boundary2) {
        b1 = boundary2;
        b2 = boundary1;
    } else {
        b1 = boundary1;
        b2 = boundary2;
        }

    index = 0;
    if (validDomain == SDL_TRUE) {
            if (b1 == b2) {
                return b1;
            }

            /* Generate up to 4 values within bounds */
            delta = b2 - b1;
            if (delta < 4) {
                do {
                tempBuf[index] = b1 + index;
                index++;
                    } while (index < delta);
            } else {
          tempBuf[index] = b1;
          index++;
          tempBuf[index] = b1 + 1;
          index++;
          tempBuf[index] = b2 - 1;
          index++;
          tempBuf[index] = b2;
          index++;
            }
        } else {
            /* Generate up to 2 values outside of bounds */
        if (b1 > 0) {
            tempBuf[index] = b1 - 1;
            index++;
        }

        if (b2 < maxValue) {
            tempBuf[index] = b2 + 1;
            index++;
        }
    }

    if (index == 0) {
        /* There are no valid boundaries */
        SDL_Unsupported();
        return 0;
    }

    return tempBuf[SDLTest_RandomUint8() % index];
}


Uint8
SDLTest_RandomUint8BoundaryValue(Uint8 boundary1, Uint8 boundary2, SDL_bool validDomain)
{
    /* max value for Uint8 */
    const Uint64 maxValue = UCHAR_MAX;
    return (Uint8)SDLTest_GenerateUnsignedBoundaryValues(maxValue,
                (Uint64) boundary1, (Uint64) boundary2,
                validDomain);
}

Uint16
SDLTest_RandomUint16BoundaryValue(Uint16 boundary1, Uint16 boundary2, SDL_bool validDomain)
{
    /* max value for Uint16 */
    const Uint64 maxValue = USHRT_MAX;
    return (Uint16)SDLTest_GenerateUnsignedBoundaryValues(maxValue,
                (Uint64) boundary1, (Uint64) boundary2,
                validDomain);
}

Uint32
SDLTest_RandomUint32BoundaryValue(Uint32 boundary1, Uint32 boundary2, SDL_bool validDomain)
{
    /* max value for Uint32 */
    #if ((ULONG_MAX) == (UINT_MAX))
      const Uint64 maxValue = ULONG_MAX;
        #else
      const Uint64 maxValue = UINT_MAX;
        #endif
    return (Uint32)SDLTest_GenerateUnsignedBoundaryValues(maxValue,
                (Uint64) boundary1, (Uint64) boundary2,
                validDomain);
}

Uint64
SDLTest_RandomUint64BoundaryValue(Uint64 boundary1, Uint64 boundary2, SDL_bool validDomain)
{
    /* max value for Uint64 */
    const Uint64 maxValue = UINT64_MAX;
    return SDLTest_GenerateUnsignedBoundaryValues(maxValue,
                (Uint64) boundary1, (Uint64) boundary2,
                validDomain);
}

/* !
 * Generates a signed boundary value between the given boundaries.
 * Boundary values are inclusive. See the examples below.
 * If boundary2 < boundary1, the values are swapped.
 * If boundary1 == boundary2, value of boundary1 will be returned
 *
 * Generating boundary values for Sint8:
 * SignedBoundaryValues(SCHAR_MIN, SCHAR_MAX, -10, 20, True) -> [-10,-9,19,20]
 * SignedBoundaryValues(SCHAR_MIN, SCHAR_MAX, -10, 20, False) -> [-11,21]
 * SignedBoundaryValues(SCHAR_MIN, SCHAR_MAX, -30, -15, True) -> [-30, -29, -16, -15]
 * SignedBoundaryValues(SCHAR_MIN, SCHAR_MAX, -127, 15, False) -> [16]
 * SignedBoundaryValues(SCHAR_MIN, SCHAR_MAX, -127, 127, False) -> [0], error set
 *
 * Generator works the same for other types of signed integers.
 *
 * \param minValue The smallest value that is acceptable for this data type.
 *                  For instance, for Uint8 -> -127, etc.
 * \param maxValue The biggest value that is acceptable for this data type.
 *                  For instance, for Uint8 -> 127, etc.
 * \param boundary1 defines lower boundary
 * \param boundary2 defines upper boundary
 * \param validDomain Generate only for valid domain (for the data type)
 *
 * \returns Returns a random boundary value for the domain or 0 in case of error
 */
static Sint64
SDLTest_GenerateSignedBoundaryValues(const Sint64 minValue, const Sint64 maxValue, Sint64 boundary1, Sint64 boundary2, SDL_bool validDomain)
{
        Sint64 b1, b2;
    Sint64 delta;
    Sint64 tempBuf[4];
    Uint8 index;

        /* Maybe swap */
    if (boundary1 > boundary2) {
        b1 = boundary2;
        b2 = boundary1;
    } else {
        b1 = boundary1;
        b2 = boundary2;
        }

    index = 0;
    if (validDomain == SDL_TRUE) {
            if (b1 == b2) {
                return b1;
            }

            /* Generate up to 4 values within bounds */
            delta = b2 - b1;
            if (delta < 4) {
                do {
                tempBuf[index] = b1 + index;
                index++;
                    } while (index < delta);
            } else {
          tempBuf[index] = b1;
          index++;
          tempBuf[index] = b1 + 1;
          index++;
          tempBuf[index] = b2 - 1;
          index++;
          tempBuf[index] = b2;
          index++;
            }
        } else {
            /* Generate up to 2 values outside of bounds */
        if (b1 > minValue) {
            tempBuf[index] = b1 - 1;
            index++;
        }

        if (b2 < maxValue) {
            tempBuf[index] = b2 + 1;
            index++;
        }
    }

    if (index == 0) {
        /* There are no valid boundaries */
        SDL_Unsupported();
        return minValue;
    }

    return tempBuf[SDLTest_RandomUint8() % index];
}


Sint8
SDLTest_RandomSint8BoundaryValue(Sint8 boundary1, Sint8 boundary2, SDL_bool validDomain)
{
    /* min & max values for Sint8 */
    const Sint64 maxValue = SCHAR_MAX;
    const Sint64 minValue = SCHAR_MIN;
    return (Sint8)SDLTest_GenerateSignedBoundaryValues(minValue, maxValue,
                (Sint64) boundary1, (Sint64) boundary2,
                validDomain);
}

Sint16
SDLTest_RandomSint16BoundaryValue(Sint16 boundary1, Sint16 boundary2, SDL_bool validDomain)
{
    /* min & max values for Sint16 */
    const Sint64 maxValue = SHRT_MAX;
    const Sint64 minValue = SHRT_MIN;
    return (Sint16)SDLTest_GenerateSignedBoundaryValues(minValue, maxValue,
                (Sint64) boundary1, (Sint64) boundary2,
                validDomain);
}

Sint32
SDLTest_RandomSint32BoundaryValue(Sint32 boundary1, Sint32 boundary2, SDL_bool validDomain)
{
    /* min & max values for Sint32 */
    #if ((ULONG_MAX) == (UINT_MAX))
      const Sint64 maxValue = LONG_MAX;
      const Sint64 minValue = LONG_MIN;
        #else
      const Sint64 maxValue = INT_MAX;
      const Sint64 minValue = INT_MIN;
        #endif
    return (Sint32)SDLTest_GenerateSignedBoundaryValues(minValue, maxValue,
                (Sint64) boundary1, (Sint64) boundary2,
                validDomain);
}

Sint64
SDLTest_RandomSint64BoundaryValue(Sint64 boundary1, Sint64 boundary2, SDL_bool validDomain)
{
    /* min & max values for Sint64 */
    const Sint64 maxValue = INT64_MAX;
    const Sint64 minValue = INT64_MIN;
    return SDLTest_GenerateSignedBoundaryValues(minValue, maxValue,
                boundary1, boundary2,
                validDomain);
}

float
SDLTest_RandomUnitFloat()
{
    return SDLTest_RandomUint32() / (float) UINT_MAX;
}

float
SDLTest_RandomFloat()
{
        return (float) (SDLTest_RandomUnitDouble() * (double)2.0 * (double)FLT_MAX - (double)(FLT_MAX));
}

double
SDLTest_RandomUnitDouble()
{
    return (double) (SDLTest_RandomUint64() >> 11) * (1.0/9007199254740992.0);
}

double
SDLTest_RandomDouble()
{
    double r = 0.0;
    double s = 1.0;
    do {
      s /= UINT_MAX + 1.0;
      r += (double)SDLTest_RandomInt(&rndContext) * s;
    } while (s > DBL_EPSILON);

    fuzzerInvocationCounter++;

    return r;
}


char *
SDLTest_RandomAsciiString()
{
    return SDLTest_RandomAsciiStringWithMaximumLength(255);
}

char *
SDLTest_RandomAsciiStringWithMaximumLength(int maxLength)
{
    int size;

    if(maxLength < 1) {
                SDL_InvalidParamError("maxLength");
        return NULL;
    }

    size = (SDLTest_RandomUint32() % (maxLength + 1));

    return SDLTest_RandomAsciiStringOfSize(size);
}

char *
SDLTest_RandomAsciiStringOfSize(int size)
{
    char *string;
    int counter;


    if(size < 1) {
                SDL_InvalidParamError("size");
        return NULL;
    }

    string = (char *)SDL_malloc((size + 1) * sizeof(char));
    if (string==NULL) {
      return NULL;
        }

    for(counter = 0; counter < size; ++counter) {
        string[counter] = (char)SDLTest_RandomIntegerInRange(32, 126);
    }

    string[counter] = '\0';

    fuzzerInvocationCounter++;

    return string;
}

/* vi: set ts=4 sw=4 expandtab: */
