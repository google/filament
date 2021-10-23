/**
 * SDL_test test suite
 */

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
#include <float.h>
#include <ctype.h>

#include "SDL.h"
#include "SDL_test.h"

/* Test case functions */

/* Forward declarations for internal harness functions */
extern char *SDLTest_GenerateRunSeed(const int length);

/**
 * @brief Calls to SDLTest_GenerateRunSeed()
 */
int
sdltest_generateRunSeed(void *arg)
{
  char* result;
  size_t i, l;
  int j;
  
  for (i = 1; i <= 10; i += 3) {   
     result = SDLTest_GenerateRunSeed((const int)i);
     SDLTest_AssertPass("Call to SDLTest_GenerateRunSeed()");
     SDLTest_AssertCheck(result != NULL, "Verify returned value is not NULL");
     if (result != NULL) {
       l = SDL_strlen(result);
       SDLTest_AssertCheck(l == i, "Verify length of returned value is %d, got: %d", (int) i, (int) l);
       SDL_free(result);
     }
  }

  /* Negative cases */
  for (j = -2; j <= 0; j++) {
     result = SDLTest_GenerateRunSeed((const int)j);
     SDLTest_AssertPass("Call to SDLTest_GenerateRunSeed()");
     SDLTest_AssertCheck(result == NULL, "Verify returned value is not NULL");
  }
  
  return TEST_COMPLETED;
}

/**
 * @brief Calls to SDLTest_GetFuzzerInvocationCount()
 */
int
sdltest_getFuzzerInvocationCount(void *arg)
{
  Uint8 result;
  int fuzzerCount1, fuzzerCount2;

  fuzzerCount1 = SDLTest_GetFuzzerInvocationCount();
  SDLTest_AssertPass("Call to SDLTest_GetFuzzerInvocationCount()");
  SDLTest_AssertCheck(fuzzerCount1 >= 0, "Verify returned value, expected: >=0, got: %d", fuzzerCount1);

  result = SDLTest_RandomUint8();
  SDLTest_AssertPass("Call to SDLTest_RandomUint8(), returned %d", result);

  fuzzerCount2 = SDLTest_GetFuzzerInvocationCount();
  SDLTest_AssertPass("Call to SDLTest_GetFuzzerInvocationCount()");
  SDLTest_AssertCheck(fuzzerCount2 > fuzzerCount1, "Verify returned value, expected: >%d, got: %d", fuzzerCount1, fuzzerCount2);

  return TEST_COMPLETED;
}


/**
 * @brief Calls to random number generators
 */
int
sdltest_randomNumber(void *arg)
{
  Sint64 result;
  double dresult;
  Uint64 umax;
  Sint64 min, max;

  result = (Sint64)SDLTest_RandomUint8();
  umax = (1 << 8) - 1;
  SDLTest_AssertPass("Call to SDLTest_RandomUint8");
  SDLTest_AssertCheck(result >= 0 && result <= (Sint64)umax, "Verify result value, expected: [0,%"SDL_PRIu64"], got: %"SDL_PRIs64, umax, result);

  result = (Sint64)SDLTest_RandomSint8();
  min = 0 - (1 << 7);
  max =     (1 << 7) - 1;
  SDLTest_AssertPass("Call to SDLTest_RandomSint8");
  SDLTest_AssertCheck(result >= min && result <= max, "Verify result value, expected: [%"SDL_PRIs64",%"SDL_PRIs64"], got: %"SDL_PRIs64, min, max, result);

  result = (Sint64)SDLTest_RandomUint16();
  umax = (1 << 16) - 1;
  SDLTest_AssertPass("Call to SDLTest_RandomUint16");
  SDLTest_AssertCheck(result >= 0 && result <= (Sint64)umax, "Verify result value, expected: [0,%"SDL_PRIu64"], got: %"SDL_PRIs64, umax, result);

  result = (Sint64)SDLTest_RandomSint16();
  min = 0 - (1 << 15);
  max =     (1 << 15) - 1;
  SDLTest_AssertPass("Call to SDLTest_RandomSint16");
  SDLTest_AssertCheck(result >= min && result <= max, "Verify result value, expected: [%"SDL_PRIs64",%"SDL_PRIs64"], got: %"SDL_PRIs64, min, max, result);

  result = (Sint64)SDLTest_RandomUint32();
  umax = ((Uint64)1 << 32) - 1;
  SDLTest_AssertPass("Call to SDLTest_RandomUint32");
  SDLTest_AssertCheck(result >= 0 && result <= (Sint64)umax, "Verify result value, expected: [0,%"SDL_PRIu64"], got: %"SDL_PRIs64, umax, result);

  result = (Sint64)SDLTest_RandomSint32();
  min = 0 - ((Sint64)1 << 31);
  max =     ((Sint64)1 << 31) - 1;
  SDLTest_AssertPass("Call to SDLTest_RandomSint32");
  SDLTest_AssertCheck(result >= min && result <= max, "Verify result value, expected: [%"SDL_PRIs64",%"SDL_PRIs64"], got: %"SDL_PRIs64, min, max, result);

  SDLTest_RandomUint64();
  SDLTest_AssertPass("Call to SDLTest_RandomUint64");

  result = SDLTest_RandomSint64();
  SDLTest_AssertPass("Call to SDLTest_RandomSint64");

  dresult = (double)SDLTest_RandomUnitFloat();
  SDLTest_AssertPass("Call to SDLTest_RandomUnitFloat");
  SDLTest_AssertCheck(dresult >= 0.0 && dresult < 1.0, "Verify result value, expected: [0.0,1.0[, got: %e", dresult);

  dresult = (double)SDLTest_RandomFloat();
  SDLTest_AssertPass("Call to SDLTest_RandomFloat");
  SDLTest_AssertCheck(dresult >= (double)(-FLT_MAX) && dresult <= (double)FLT_MAX, "Verify result value, expected: [%e,%e], got: %e", (double)(-FLT_MAX), (double)FLT_MAX, dresult);

  dresult = (double)SDLTest_RandomUnitDouble();
  SDLTest_AssertPass("Call to SDLTest_RandomUnitDouble");
  SDLTest_AssertCheck(dresult >= 0.0 && dresult < 1.0, "Verify result value, expected: [0.0,1.0[, got: %e", dresult);

  dresult = SDLTest_RandomDouble();
  SDLTest_AssertPass("Call to SDLTest_RandomDouble");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Uint8
 */
int
sdltest_randomBoundaryNumberUint8(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Uint64 uresult;

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomUintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12 || uresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0 || uresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(0, 99, SDL_FALSE) returns 100 */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(0, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 100,
    "Validate result value for parameters (0,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 0xff, SDL_FALSE) returns 0 (no error) */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(1, 255, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters (1,255,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xfe, SDL_FALSE) returns 0xff (no error) */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(0, 254, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0xff,
    "Validate result value for parameters (0,254,SDL_FALSE); expected: 0xff, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xff, SDL_FALSE) returns 0 (sets error) */
  uresult = (Uint64)SDLTest_RandomUint8BoundaryValue(0, 255, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint8BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters(0,255,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Uint16
 */
int
sdltest_randomBoundaryNumberUint16(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Uint64 uresult;

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomUintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12 || uresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0 || uresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(0, 99, SDL_FALSE) returns 100 */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(0, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 100,
    "Validate result value for parameters (0,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 0xffff, SDL_FALSE) returns 0 (no error) */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(1, 0xffff, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters (1,0xffff,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xfffe, SDL_FALSE) returns 0xffff (no error) */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(0, 0xfffe, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0xffff,
    "Validate result value for parameters (0,0xfffe,SDL_FALSE); expected: 0xffff, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xffff, SDL_FALSE) returns 0 (sets error) */
  uresult = (Uint64)SDLTest_RandomUint16BoundaryValue(0, 0xffff, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint16BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters(0,0xffff,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Uint32
 */
int
sdltest_randomBoundaryNumberUint32(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Uint64 uresult;

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomUintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12 || uresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0 || uresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(0, 99, SDL_FALSE) returns 100 */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(0, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 100,
    "Validate result value for parameters (0,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 0xffffffff, SDL_FALSE) returns 0 (no error) */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(1, 0xffffffff, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters (1,0xffffffff,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xfffffffe, SDL_FALSE) returns 0xffffffff (no error) */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(0, 0xfffffffe, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0xffffffff,
    "Validate result value for parameters (0,0xfffffffe,SDL_FALSE); expected: 0xffffffff, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xffffffff, SDL_FALSE) returns 0 (sets error) */
  uresult = (Uint64)SDLTest_RandomUint32BoundaryValue(0, 0xffffffff, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint32BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters(0,0xffffffff,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Uint64
 */
int
sdltest_randomBoundaryNumberUint64(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Uint64 uresult;

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomUintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 12 || uresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 10 || uresult == 11 || uresult == 19 || uresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0 || uresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(0, 99, SDL_FALSE) returns 100 */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(0, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 100,
    "Validate result value for parameters (0,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, uresult);

  /* RandomUintXBoundaryValue(1, 0xffffffffffffffff, SDL_FALSE) returns 0 (no error) */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(1, (Uint64)0xffffffffffffffffULL, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters (1,0xffffffffffffffff,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xfffffffffffffffe, SDL_FALSE) returns 0xffffffffffffffff (no error) */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(0, (Uint64)0xfffffffffffffffeULL, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == (Uint64)0xffffffffffffffffULL,
    "Validate result value for parameters (0,0xfffffffffffffffe,SDL_FALSE); expected: 0xffffffffffffffff, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomUintXBoundaryValue(0, 0xffffffffffffffff, SDL_FALSE) returns 0 (sets error) */
  uresult = (Uint64)SDLTest_RandomUint64BoundaryValue(0, (Uint64)0xffffffffffffffffULL, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomUint64BoundaryValue");
  SDLTest_AssertCheck(
    uresult == 0,
    "Validate result value for parameters(0,0xffffffffffffffff,SDL_FALSE); expected: 0, got: %"SDL_PRIs64, uresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Sint8
 */
int
sdltest_randomBoundaryNumberSint8(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Sint64 sresult;

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomSintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12 || sresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 0 || sresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(SCHAR_MIN, 99, SDL_FALSE) returns 100 */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(SCHAR_MIN, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 100,
    "Validate result value for parameters (SCHAR_MIN,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(SCHAR_MIN + 1, SCHAR_MAX, SDL_FALSE) returns SCHAR_MIN (no error) */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(SCHAR_MIN + 1, SCHAR_MAX, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == SCHAR_MIN,
    "Validate result value for parameters (SCHAR_MIN + 1,SCHAR_MAX,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, SCHAR_MIN, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(SCHAR_MIN, SCHAR_MAX - 1, SDL_FALSE) returns SCHAR_MAX (no error) */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(SCHAR_MIN, SCHAR_MAX -1, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == SCHAR_MAX,
    "Validate result value for parameters (SCHAR_MIN,SCHAR_MAX - 1,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, SCHAR_MAX, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(SCHAR_MIN, SCHAR_MAX, SDL_FALSE) returns SCHAR_MIN (sets error) */
  sresult = (Sint64)SDLTest_RandomSint8BoundaryValue(SCHAR_MIN, SCHAR_MAX, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint8BoundaryValue");
  SDLTest_AssertCheck(
    sresult == SCHAR_MIN,
    "Validate result value for parameters(SCHAR_MIN,SCHAR_MAX,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, SCHAR_MIN, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Sint16
 */
int
sdltest_randomBoundaryNumberSint16(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Sint64 sresult;

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomSintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12 || sresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 0 || sresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(SHRT_MIN, 99, SDL_FALSE) returns 100 */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(SHRT_MIN, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 100,
    "Validate result value for parameters (SHRT_MIN,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(SHRT_MIN + 1, SHRT_MAX, SDL_FALSE) returns SHRT_MIN (no error) */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(SHRT_MIN + 1, SHRT_MAX, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == SHRT_MIN,
    "Validate result value for parameters (SHRT_MIN+1,SHRT_MAX,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, SHRT_MIN, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(SHRT_MIN, SHRT_MAX - 1, SDL_FALSE) returns SHRT_MAX (no error) */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(SHRT_MIN, SHRT_MAX - 1, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == SHRT_MAX,
    "Validate result value for parameters (SHRT_MIN,SHRT_MAX - 1,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, SHRT_MAX, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(SHRT_MIN, SHRT_MAX, SDL_FALSE) returns 0 (sets error) */
  sresult = (Sint64)SDLTest_RandomSint16BoundaryValue(SHRT_MIN, SHRT_MAX, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint16BoundaryValue");
  SDLTest_AssertCheck(
    sresult == SHRT_MIN,
    "Validate result value for parameters(SHRT_MIN,SHRT_MAX,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, SHRT_MIN, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Sint32
 */
int
sdltest_randomBoundaryNumberSint32(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Sint64 sresult;
#if ((ULONG_MAX) == (UINT_MAX))
  Sint32 long_min = LONG_MIN;
  Sint32 long_max = LONG_MAX;
#else
  Sint32 long_min = INT_MIN;
  Sint32 long_max = INT_MAX;
#endif

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomSintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12 || sresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 0 || sresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(LONG_MIN, 99, SDL_FALSE) returns 100 */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(long_min, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 100,
    "Validate result value for parameters (LONG_MIN,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(LONG_MIN + 1, LONG_MAX, SDL_FALSE) returns LONG_MIN (no error) */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(long_min + 1, long_max, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == long_min,
    "Validate result value for parameters (LONG_MIN+1,LONG_MAX,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, long_min, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(LONG_MIN, LONG_MAX - 1, SDL_FALSE) returns LONG_MAX (no error) */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(long_min, long_max - 1, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == long_max,
    "Validate result value for parameters (LONG_MIN,LONG_MAX - 1,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, long_max, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(LONG_MIN, LONG_MAX, SDL_FALSE) returns 0 (sets error) */
  sresult = (Sint64)SDLTest_RandomSint32BoundaryValue(long_min, long_max, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint32BoundaryValue");
  SDLTest_AssertCheck(
    sresult == long_min,
    "Validate result value for parameters(LONG_MIN,LONG_MAX,SDL_FALSE); expected: %d, got: %"SDL_PRIs64, long_min, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/*
 * @brief Calls to random boundary number generators for Sint64
 */
int
sdltest_randomBoundaryNumberSint64(void *arg)
{
  const char *expectedError = "That operation is not supported";
  char *lastError;
  Sint64 sresult;

  /* Clean error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  /* RandomSintXBoundaryValue(10, 10, SDL_TRUE) returns 10 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(10, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10,
    "Validate result value for parameters (10,10,SDL_TRUE); expected: 10, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 11, SDL_TRUE) returns 10, 11 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(10, 11, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11,
    "Validate result value for parameters (10,11,SDL_TRUE); expected: 10|11, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 12, SDL_TRUE) returns 10, 11, 12 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(10, 12, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12,
    "Validate result value for parameters (10,12,SDL_TRUE); expected: 10|11|12, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 13, SDL_TRUE) returns 10, 11, 12, 13 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(10, 13, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 12 || sresult == 13,
    "Validate result value for parameters (10,13,SDL_TRUE); expected: 10|11|12|13, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(10, 20, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(10, 20, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (10,20,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(20, 10, SDL_TRUE) returns 10, 11, 19 or 20 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(20, 10, SDL_TRUE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 10 || sresult == 11 || sresult == 19 || sresult == 20,
    "Validate result value for parameters (20,10,SDL_TRUE); expected: 10|11|19|20, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(1, 20, SDL_FALSE) returns 0, 21 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(1, 20, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 0 || sresult == 21,
    "Validate result value for parameters (1,20,SDL_FALSE); expected: 0|21, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(LLONG_MIN, 99, SDL_FALSE) returns 100 */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(INT64_MIN, 99, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == 100,
    "Validate result value for parameters (LLONG_MIN,99,SDL_FALSE); expected: 100, got: %"SDL_PRIs64, sresult);

  /* RandomSintXBoundaryValue(LLONG_MIN + 1, LLONG_MAX, SDL_FALSE) returns LLONG_MIN (no error) */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(INT64_MIN + 1, INT64_MAX, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == INT64_MIN,
    "Validate result value for parameters (LLONG_MIN+1,LLONG_MAX,SDL_FALSE); expected: %"SDL_PRIs64", got: %"SDL_PRIs64, INT64_MIN, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(LLONG_MIN, LLONG_MAX - 1, SDL_FALSE) returns LLONG_MAX (no error) */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(INT64_MIN, INT64_MAX - 1, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == INT64_MAX,
    "Validate result value for parameters (LLONG_MIN,LLONG_MAX - 1,SDL_FALSE); expected: %"SDL_PRIs64", got: %"SDL_PRIs64, INT64_MAX, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError == NULL || lastError[0] == '\0', "Validate no error message was set");

  /* RandomSintXBoundaryValue(LLONG_MIN, LLONG_MAX, SDL_FALSE) returns 0 (sets error) */
  sresult = (Sint64)SDLTest_RandomSint64BoundaryValue(INT64_MIN, INT64_MAX, SDL_FALSE);
  SDLTest_AssertPass("Call to SDLTest_RandomSint64BoundaryValue");
  SDLTest_AssertCheck(
    sresult == INT64_MIN,
    "Validate result value for parameters(LLONG_MIN,LLONG_MAX,SDL_FALSE); expected: %"SDL_PRIs64", got: %"SDL_PRIs64, INT64_MIN, sresult);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/**
 * @brief Calls to SDLTest_RandomIntegerInRange
 */
int
sdltest_randomIntegerInRange(void *arg)
{
  Sint32 min, max;
  Sint32 result;
#if ((ULONG_MAX) == (UINT_MAX))
  Sint32 long_min = LONG_MIN;
  Sint32 long_max = LONG_MAX;
#else
  Sint32 long_min = INT_MIN;
  Sint32 long_max = INT_MAX;
#endif

  /* Standard range */
  min = (Sint32)SDLTest_RandomSint16();
  max = min + (Sint32)SDLTest_RandomUint8() + 2;
  result = SDLTest_RandomIntegerInRange(min, max);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(min,max)");
  SDLTest_AssertCheck(min <= result && result <= max, "Validated returned value; expected: [%d,%d], got: %d", min, max, result);

  /* One Range */
  min = (Sint32)SDLTest_RandomSint16();
  max = min + 1;
  result = SDLTest_RandomIntegerInRange(min, max);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(min,min+1)");
  SDLTest_AssertCheck(min <= result && result <= max, "Validated returned value; expected: [%d,%d], got: %d", min, max, result);

  /* Zero range */
  min = (Sint32)SDLTest_RandomSint16();
  max = min;
  result = SDLTest_RandomIntegerInRange(min, max);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(min,min)");
  SDLTest_AssertCheck(min == result, "Validated returned value; expected: %d, got: %d", min, result);

  /* Zero range at zero */
  min = 0;
  max = 0;
  result = SDLTest_RandomIntegerInRange(min, max);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(0,0)");
  SDLTest_AssertCheck(result == 0, "Validated returned value; expected: 0, got: %d", result);

  /* Swapped min-max */
  min = (Sint32)SDLTest_RandomSint16();
  max = min + (Sint32)SDLTest_RandomUint8() + 2;
  result = SDLTest_RandomIntegerInRange(max, min);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(max,min)");
  SDLTest_AssertCheck(min <= result && result <= max, "Validated returned value; expected: [%d,%d], got: %d", min, max, result);

  /* Range with min at integer limit */
  min = long_min;
  max = long_max + (Sint32)SDLTest_RandomSint16();
  result = SDLTest_RandomIntegerInRange(min, max);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(SINT32_MIN,...)");
  SDLTest_AssertCheck(min <= result && result <= max, "Validated returned value; expected: [%d,%d], got: %d", min, max, result);

  /* Range with max at integer limit */
  min = long_min - (Sint32)SDLTest_RandomSint16();
  max = long_max;
  result = SDLTest_RandomIntegerInRange(min, max);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(...,SINT32_MAX)");
  SDLTest_AssertCheck(min <= result && result <= max, "Validated returned value; expected: [%d,%d], got: %d", min, max, result);

  /* Full integer range */
  min = long_min;
  max = long_max;
  result = SDLTest_RandomIntegerInRange(min, max);
  SDLTest_AssertPass("Call to SDLTest_RandomIntegerInRange(SINT32_MIN,SINT32_MAX)");
  SDLTest_AssertCheck(min <= result && result <= max, "Validated returned value; expected: [%d,%d], got: %d", min, max, result);

  return TEST_COMPLETED;
}

/**
 * @brief Calls to SDLTest_RandomAsciiString
 */
int
sdltest_randomAsciiString(void *arg)
{
  char* result;
  size_t len;
  int nonAsciiCharacters;
  size_t i;

  result = SDLTest_RandomAsciiString();
  SDLTest_AssertPass("Call to SDLTest_RandomAsciiString()");
  SDLTest_AssertCheck(result != NULL, "Validate that result is not NULL");
  if (result != NULL) {
     len = SDL_strlen(result);
     SDLTest_AssertCheck(len >= 1 && len <= 255, "Validate that result length; expected: len=[1,255], got: %d", (int) len);
     nonAsciiCharacters = 0;
     for (i=0; i<len; i++) {
       if (iscntrl(result[i])) {
         nonAsciiCharacters++;
       }
     }
     SDLTest_AssertCheck(nonAsciiCharacters == 0, "Validate that result does not contain non-Ascii characters, got: %d", nonAsciiCharacters);
     if (nonAsciiCharacters) {
        SDLTest_LogError("Invalid result from generator: '%s'", result);
     }
     SDL_free(result);
  }

  return TEST_COMPLETED;
}


/**
 * @brief Calls to SDLTest_RandomAsciiStringWithMaximumLength
 */
int
sdltest_randomAsciiStringWithMaximumLength(void *arg)
{
  const char* expectedError = "Parameter 'maxLength' is invalid";
  char* lastError;
  char* result;
  size_t targetLen;
  size_t len;
  int nonAsciiCharacters;
  size_t i;

  targetLen = 16 + SDLTest_RandomUint8();
  result = SDLTest_RandomAsciiStringWithMaximumLength((int) targetLen);
  SDLTest_AssertPass("Call to SDLTest_RandomAsciiStringWithMaximumLength(%d)", (int) targetLen);
  SDLTest_AssertCheck(result != NULL, "Validate that result is not NULL");
  if (result != NULL) {
     len = SDL_strlen(result);
     SDLTest_AssertCheck(len >= 1 && len <= targetLen, "Validate that result length; expected: len=[1,%d], got: %d", (int) targetLen, (int) len);
     nonAsciiCharacters = 0;
     for (i=0; i<len; i++) {
       if (iscntrl(result[i])) {
         nonAsciiCharacters++;
       }
     }
     SDLTest_AssertCheck(nonAsciiCharacters == 0, "Validate that result does not contain non-Ascii characters, got: %d", nonAsciiCharacters);
     if (nonAsciiCharacters) {
        SDLTest_LogError("Invalid result from generator: '%s'", result);
     }
     SDL_free(result);
  }

  /* Negative test */
  targetLen = 0;
  result = SDLTest_RandomAsciiStringWithMaximumLength((int) targetLen);
  SDLTest_AssertPass("Call to SDLTest_RandomAsciiStringWithMaximumLength(%d)", (int) targetLen);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}

/**
 * @brief Calls to SDLTest_RandomAsciiStringOfSize
 */
int
sdltest_randomAsciiStringOfSize(void *arg)
{
  const char* expectedError = "Parameter 'size' is invalid";
  char* lastError;
  char* result;
  size_t targetLen;
  size_t len;
  int nonAsciiCharacters;
  size_t i;

  /* Positive test */
  targetLen = 16 + SDLTest_RandomUint8();
  result = SDLTest_RandomAsciiStringOfSize((int) targetLen);
  SDLTest_AssertPass("Call to SDLTest_RandomAsciiStringOfSize(%d)", (int) targetLen);
  SDLTest_AssertCheck(result != NULL, "Validate that result is not NULL");
  if (result != NULL) {
     len = SDL_strlen(result);
     SDLTest_AssertCheck(len == targetLen, "Validate that result length; expected: len=%d, got: %d", (int) targetLen, (int) len);
     nonAsciiCharacters = 0;
     for (i=0; i<len; i++) {
       if (iscntrl(result[i])) {
         nonAsciiCharacters++;
       }
     }
     SDLTest_AssertCheck(nonAsciiCharacters == 0, "Validate that result does not contain non-ASCII characters, got: %d", nonAsciiCharacters);
     if (nonAsciiCharacters) {
        SDLTest_LogError("Invalid result from generator: '%s'", result);
     }
     SDL_free(result);
  }

  /* Negative test */
  targetLen = 0;
  result = SDLTest_RandomAsciiStringOfSize((int) targetLen);
  SDLTest_AssertPass("Call to SDLTest_RandomAsciiStringOfSize(%d)", (int) targetLen);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL && SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);

  /* Clear error messages */
  SDL_ClearError();
  SDLTest_AssertPass("SDL_ClearError()");

  return TEST_COMPLETED;
}


/* ================= Test References ================== */

/* SDL_test test cases */
static const SDLTest_TestCaseReference sdltestTest1 =
        { (SDLTest_TestCaseFp)sdltest_getFuzzerInvocationCount, "sdltest_getFuzzerInvocationCount", "Call to sdltest_GetFuzzerInvocationCount", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest2 =
        { (SDLTest_TestCaseFp)sdltest_randomNumber, "sdltest_randomNumber", "Calls to random number generators", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest3 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberUint8, "sdltest_randomBoundaryNumberUint8", "Calls to random boundary number generators for Uint8", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest4 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberUint16, "sdltest_randomBoundaryNumberUint16", "Calls to random boundary number generators for Uint16", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest5 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberUint32, "sdltest_randomBoundaryNumberUint32", "Calls to random boundary number generators for Uint32", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest6 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberUint64, "sdltest_randomBoundaryNumberUint64", "Calls to random boundary number generators for Uint64", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest7 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberSint8, "sdltest_randomBoundaryNumberSint8", "Calls to random boundary number generators for Sint8", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest8 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberSint16, "sdltest_randomBoundaryNumberSint16", "Calls to random boundary number generators for Sint16", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest9 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberSint32, "sdltest_randomBoundaryNumberSint32", "Calls to random boundary number generators for Sint32", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest10 =
        { (SDLTest_TestCaseFp)sdltest_randomBoundaryNumberSint64, "sdltest_randomBoundaryNumberSint64", "Calls to random boundary number generators for Sint64", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest11 =
        { (SDLTest_TestCaseFp)sdltest_randomIntegerInRange, "sdltest_randomIntegerInRange", "Calls to ranged random number generator", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest12 =
        { (SDLTest_TestCaseFp)sdltest_randomAsciiString, "sdltest_randomAsciiString", "Calls to default ASCII string generator", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest13 =
        { (SDLTest_TestCaseFp)sdltest_randomAsciiStringWithMaximumLength, "sdltest_randomAsciiStringWithMaximumLength", "Calls to random maximum length ASCII string generator", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest14 =
        { (SDLTest_TestCaseFp)sdltest_randomAsciiStringOfSize, "sdltest_randomAsciiStringOfSize", "Calls to fixed size ASCII string generator", TEST_ENABLED };

static const SDLTest_TestCaseReference sdltestTest15 =
        { (SDLTest_TestCaseFp)sdltest_generateRunSeed, "sdltest_generateRunSeed", "Checks internal harness function SDLTest_GenerateRunSeed", TEST_ENABLED };

/* Sequence of SDL_test test cases */
static const SDLTest_TestCaseReference *sdltestTests[] =  {
    &sdltestTest1, &sdltestTest2, &sdltestTest3, &sdltestTest4, &sdltestTest5, &sdltestTest6,
    &sdltestTest7, &sdltestTest8, &sdltestTest9, &sdltestTest10, &sdltestTest11, &sdltestTest12,
    &sdltestTest13, &sdltestTest14, &sdltestTest15, NULL
};

/* SDL_test test suite (global) */
SDLTest_TestSuiteReference sdltestTestSuite = {
    "SDLtest",
    NULL,
    sdltestTests,
    NULL
};
