/**
 * Pixels test suite
 */

#include <stdio.h>

#include "SDL.h"
#include "SDL_test.h"

/* Test case functions */

/* Definition of all RGB formats used to test pixel conversions */
const int _numRGBPixelFormats = 30;
Uint32 _RGBPixelFormats[] =
  {
    SDL_PIXELFORMAT_INDEX1LSB,
    SDL_PIXELFORMAT_INDEX1MSB,
    SDL_PIXELFORMAT_INDEX4LSB,
    SDL_PIXELFORMAT_INDEX4MSB,
    SDL_PIXELFORMAT_INDEX8,
    SDL_PIXELFORMAT_RGB332,
    SDL_PIXELFORMAT_RGB444,
    SDL_PIXELFORMAT_RGB555,
    SDL_PIXELFORMAT_BGR555,
    SDL_PIXELFORMAT_ARGB4444,
    SDL_PIXELFORMAT_RGBA4444,
    SDL_PIXELFORMAT_ABGR4444,
    SDL_PIXELFORMAT_BGRA4444,
    SDL_PIXELFORMAT_ARGB1555,
    SDL_PIXELFORMAT_RGBA5551,
    SDL_PIXELFORMAT_ABGR1555,
    SDL_PIXELFORMAT_BGRA5551,
    SDL_PIXELFORMAT_RGB565,
    SDL_PIXELFORMAT_BGR565,
    SDL_PIXELFORMAT_RGB24,
    SDL_PIXELFORMAT_BGR24,
    SDL_PIXELFORMAT_RGB888,
    SDL_PIXELFORMAT_RGBX8888,
    SDL_PIXELFORMAT_BGR888,
    SDL_PIXELFORMAT_BGRX8888,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_ARGB2101010
  };
char* _RGBPixelFormatsVerbose[] =
  {
    "SDL_PIXELFORMAT_INDEX1LSB",
    "SDL_PIXELFORMAT_INDEX1MSB",
    "SDL_PIXELFORMAT_INDEX4LSB",
    "SDL_PIXELFORMAT_INDEX4MSB",
    "SDL_PIXELFORMAT_INDEX8",
    "SDL_PIXELFORMAT_RGB332",
    "SDL_PIXELFORMAT_RGB444",
    "SDL_PIXELFORMAT_RGB555",
    "SDL_PIXELFORMAT_BGR555",
    "SDL_PIXELFORMAT_ARGB4444",
    "SDL_PIXELFORMAT_RGBA4444",
    "SDL_PIXELFORMAT_ABGR4444",
    "SDL_PIXELFORMAT_BGRA4444",
    "SDL_PIXELFORMAT_ARGB1555",
    "SDL_PIXELFORMAT_RGBA5551",
    "SDL_PIXELFORMAT_ABGR1555",
    "SDL_PIXELFORMAT_BGRA5551",
    "SDL_PIXELFORMAT_RGB565",
    "SDL_PIXELFORMAT_BGR565",
    "SDL_PIXELFORMAT_RGB24",
    "SDL_PIXELFORMAT_BGR24",
    "SDL_PIXELFORMAT_RGB888",
    "SDL_PIXELFORMAT_RGBX8888",
    "SDL_PIXELFORMAT_BGR888",
    "SDL_PIXELFORMAT_BGRX8888",
    "SDL_PIXELFORMAT_ARGB8888",
    "SDL_PIXELFORMAT_RGBA8888",
    "SDL_PIXELFORMAT_ABGR8888",
    "SDL_PIXELFORMAT_BGRA8888",
    "SDL_PIXELFORMAT_ARGB2101010"
  };

/* Definition of all Non-RGB formats used to test pixel conversions */
const int _numNonRGBPixelFormats = 7;
Uint32 _nonRGBPixelFormats[] =
  {
    SDL_PIXELFORMAT_YV12,
    SDL_PIXELFORMAT_IYUV,
    SDL_PIXELFORMAT_YUY2,
    SDL_PIXELFORMAT_UYVY,
    SDL_PIXELFORMAT_YVYU,
    SDL_PIXELFORMAT_NV12,
    SDL_PIXELFORMAT_NV21
  };
char* _nonRGBPixelFormatsVerbose[] =
  {
    "SDL_PIXELFORMAT_YV12",
    "SDL_PIXELFORMAT_IYUV",
    "SDL_PIXELFORMAT_YUY2",
    "SDL_PIXELFORMAT_UYVY",
    "SDL_PIXELFORMAT_YVYU",
    "SDL_PIXELFORMAT_NV12",
    "SDL_PIXELFORMAT_NV21"
  };

/* Definition of some invalid formats for negative tests */
const int _numInvalidPixelFormats = 2;
Uint32 _invalidPixelFormats[] =
  {
    0xfffffffe,
    0xffffffff
  };
char* _invalidPixelFormatsVerbose[] =
  {
    "SDL_PIXELFORMAT_UNKNOWN",
    "SDL_PIXELFORMAT_UNKNOWN"
  };

/* Test case functions */

/**
 * @brief Call to SDL_AllocFormat and SDL_FreeFormat
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_AllocFormat
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_FreeFormat
 */
int
pixels_allocFreeFormat(void *arg)
{
  const char *unknownFormat = "SDL_PIXELFORMAT_UNKNOWN";
  const char *expectedError = "Parameter 'format' is invalid";
  const char *error;
  int i;
  Uint32 format;
  Uint32 masks;
  SDL_PixelFormat* result;

  /* Blank/unknown format */
  format = 0;
  SDLTest_Log("RGB Format: %s (%u)", unknownFormat, format);

  /* Allocate format */
  result = SDL_AllocFormat(format);
  SDLTest_AssertPass("Call to SDL_AllocFormat()");
  SDLTest_AssertCheck(result != NULL, "Verify result is not NULL");
  if (result != NULL) {
    SDLTest_AssertCheck(result->format == format, "Verify value of result.format; expected: %u, got %u", format, result->format);
    SDLTest_AssertCheck(result->BitsPerPixel == 0, "Verify value of result.BitsPerPixel; expected: 0, got %u", result->BitsPerPixel);
    SDLTest_AssertCheck(result->BytesPerPixel == 0, "Verify value of result.BytesPerPixel; expected: 0, got %u", result->BytesPerPixel);
    masks = result->Rmask | result->Gmask | result->Bmask | result->Amask;
    SDLTest_AssertCheck(masks == 0, "Verify value of result.[RGBA]mask combined; expected: 0, got %u", masks);

    /* Deallocate again */
    SDL_FreeFormat(result);
    SDLTest_AssertPass("Call to SDL_FreeFormat()");
  }

  /* RGB formats */
  for (i = 0; i < _numRGBPixelFormats; i++) {
    format = _RGBPixelFormats[i];
    SDLTest_Log("RGB Format: %s (%u)", _RGBPixelFormatsVerbose[i], format);

    /* Allocate format */
    result = SDL_AllocFormat(format);
    SDLTest_AssertPass("Call to SDL_AllocFormat()");
    SDLTest_AssertCheck(result != NULL, "Verify result is not NULL");
    if (result != NULL) {
      SDLTest_AssertCheck(result->format == format, "Verify value of result.format; expected: %u, got %u", format, result->format);
      SDLTest_AssertCheck(result->BitsPerPixel > 0, "Verify value of result.BitsPerPixel; expected: >0, got %u", result->BitsPerPixel);
      SDLTest_AssertCheck(result->BytesPerPixel > 0, "Verify value of result.BytesPerPixel; expected: >0, got %u", result->BytesPerPixel);
      if (result->palette != NULL) {
         masks = result->Rmask | result->Gmask | result->Bmask | result->Amask;
         SDLTest_AssertCheck(masks > 0, "Verify value of result.[RGBA]mask combined; expected: >0, got %u", masks);
      }

      /* Deallocate again */
      SDL_FreeFormat(result);
      SDLTest_AssertPass("Call to SDL_FreeFormat()");
    }
  }

  /* Non-RGB formats */
  for (i = 0; i < _numNonRGBPixelFormats; i++) {
    format = _nonRGBPixelFormats[i];
    SDLTest_Log("non-RGB Format: %s (%u)", _nonRGBPixelFormatsVerbose[i], format);

    /* Try to allocate format */
    result = SDL_AllocFormat(format);
    SDLTest_AssertPass("Call to SDL_AllocFormat()");
    SDLTest_AssertCheck(result == NULL, "Verify result is NULL");
  }

  /* Negative cases */

  /* Invalid Formats */
  for (i = 0; i < _numInvalidPixelFormats; i++) {
    SDL_ClearError();
    SDLTest_AssertPass("Call to SDL_ClearError()");
    format = _invalidPixelFormats[i];
    result = SDL_AllocFormat(format);
    SDLTest_AssertPass("Call to SDL_AllocFormat(%u)", format);
    SDLTest_AssertCheck(result == NULL, "Verify result is NULL");
    error = SDL_GetError();
    SDLTest_AssertPass("Call to SDL_GetError()");
    SDLTest_AssertCheck(error != NULL, "Validate that error message was not NULL");
    if (error != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(error, expectedError) == 0,
          "Validate error message, expected: '%s', got: '%s'", expectedError, error);
    }
  }

  /* Invalid free pointer */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  SDL_FreeFormat(NULL);
  SDLTest_AssertPass("Call to SDL_FreeFormat(NULL)");
  error = SDL_GetError();
  SDLTest_AssertPass("Call to SDL_GetError()");
  SDLTest_AssertCheck(error != NULL, "Validate that error message was not NULL");
  if (error != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(error, expectedError) == 0,
          "Validate error message, expected: '%s', got: '%s'", expectedError, error);
  }

  return TEST_COMPLETED;
}

/**
 * @brief Call to SDL_GetPixelFormatName
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetPixelFormatName
 */
int
pixels_getPixelFormatName(void *arg)
{
  const char *unknownFormat = "SDL_PIXELFORMAT_UNKNOWN";
  const char *error;
  int i;
  Uint32 format;
  char* result;

  /* Blank/undefined format */
  format = 0;
  SDLTest_Log("RGB Format: %s (%u)", unknownFormat, format);

  /* Get name of format */
  result = (char *)SDL_GetPixelFormatName(format);
  SDLTest_AssertPass("Call to SDL_GetPixelFormatName()");
  SDLTest_AssertCheck(result != NULL, "Verify result is not NULL");
  if (result != NULL) {
      SDLTest_AssertCheck(result[0] != '\0', "Verify result is non-empty");
      SDLTest_AssertCheck(SDL_strcmp(result, unknownFormat) == 0,
        "Verify result text; expected: %s, got %s", unknownFormat, result);
  }

  /* RGB formats */
  for (i = 0; i < _numRGBPixelFormats; i++) {
    format = _RGBPixelFormats[i];
    SDLTest_Log("RGB Format: %s (%u)", _RGBPixelFormatsVerbose[i], format);

    /* Get name of format */
    result = (char *)SDL_GetPixelFormatName(format);
    SDLTest_AssertPass("Call to SDL_GetPixelFormatName()");
    SDLTest_AssertCheck(result != NULL, "Verify result is not NULL");
    if (result != NULL) {
      SDLTest_AssertCheck(result[0] != '\0', "Verify result is non-empty");
      SDLTest_AssertCheck(SDL_strcmp(result, _RGBPixelFormatsVerbose[i]) == 0,
        "Verify result text; expected: %s, got %s", _RGBPixelFormatsVerbose[i], result);
    }
  }

  /* Non-RGB formats */
  for (i = 0; i < _numNonRGBPixelFormats; i++) {
    format = _nonRGBPixelFormats[i];
    SDLTest_Log("non-RGB Format: %s (%u)", _nonRGBPixelFormatsVerbose[i], format);

    /* Get name of format */
    result = (char *)SDL_GetPixelFormatName(format);
    SDLTest_AssertPass("Call to SDL_GetPixelFormatName()");
    SDLTest_AssertCheck(result != NULL, "Verify result is not NULL");
    if (result != NULL) {
      SDLTest_AssertCheck(result[0] != '\0', "Verify result is non-empty");
      SDLTest_AssertCheck(SDL_strcmp(result, _nonRGBPixelFormatsVerbose[i]) == 0,
        "Verify result text; expected: %s, got %s", _nonRGBPixelFormatsVerbose[i], result);
    }
  }

  /* Negative cases */

  /* Invalid Formats */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  for (i = 0; i < _numInvalidPixelFormats; i++) {
    format = _invalidPixelFormats[i];
    result = (char *)SDL_GetPixelFormatName(format);
    SDLTest_AssertPass("Call to SDL_GetPixelFormatName(%u)", format);
    SDLTest_AssertCheck(result != NULL, "Verify result is not NULL");
    if (result != NULL) {
      SDLTest_AssertCheck(result[0] != '\0',
        "Verify result is non-empty; got: %s", result);
      SDLTest_AssertCheck(SDL_strcmp(result, _invalidPixelFormatsVerbose[i]) == 0,
        "Validate name is UNKNOWN, expected: '%s', got: '%s'", _invalidPixelFormatsVerbose[i], result);
    }
    error = SDL_GetError();
    SDLTest_AssertPass("Call to SDL_GetError()");
    SDLTest_AssertCheck(error == NULL || error[0] == '\0', "Validate that error message is empty");
  }

  return TEST_COMPLETED;
}

/**
 * @brief Call to SDL_AllocPalette and SDL_FreePalette
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_AllocPalette
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_FreePalette
 */
int
pixels_allocFreePalette(void *arg)
{
  const char *expectedError1 = "Parameter 'ncolors' is invalid";
  const char *expectedError2 = "Parameter 'palette' is invalid";
  const char *error;
  int variation;
  int i;
  int ncolors;
  SDL_Palette* result;

  /* Allocate palette */
  for (variation = 1; variation <= 3; variation++) {
    switch (variation) {
      /* Just one color */
      case 1:
        ncolors = 1;
        break;
      /* Two colors */
      case 2:
        ncolors = 2;
        break;
      /* More than two colors */
      case 3:
        ncolors = SDLTest_RandomIntegerInRange(8, 16);
        break;
    }

    result = SDL_AllocPalette(ncolors);
    SDLTest_AssertPass("Call to SDL_AllocPalette(%d)", ncolors);
    SDLTest_AssertCheck(result != NULL, "Verify result is not NULL");
    if (result != NULL) {
      SDLTest_AssertCheck(result->ncolors == ncolors, "Verify value of result.ncolors; expected: %u, got %u", ncolors, result->ncolors);
      if (result->ncolors > 0) {
        SDLTest_AssertCheck(result->colors != NULL, "Verify value of result.colors is not NULL");
        if (result->colors != NULL) {
          for(i = 0; i < result->ncolors; i++) {
            SDLTest_AssertCheck(result->colors[i].r == 255, "Verify value of result.colors[%d].r; expected: 255, got %u", i, result->colors[i].r);
            SDLTest_AssertCheck(result->colors[i].g == 255, "Verify value of result.colors[%d].g; expected: 255, got %u", i, result->colors[i].g);
            SDLTest_AssertCheck(result->colors[i].b == 255, "Verify value of result.colors[%d].b; expected: 255, got %u", i, result->colors[i].b);
           }
         }
      }

      /* Deallocate again */
      SDL_FreePalette(result);
      SDLTest_AssertPass("Call to SDL_FreePalette()");
    }
  }

  /* Negative cases */

  /* Invalid number of colors */
  for (ncolors = 0; ncolors > -3; ncolors--) {
    SDL_ClearError();
    SDLTest_AssertPass("Call to SDL_ClearError()");
    result = SDL_AllocPalette(ncolors);
    SDLTest_AssertPass("Call to SDL_AllocPalette(%d)", ncolors);
    SDLTest_AssertCheck(result == NULL, "Verify result is NULL");
    error = SDL_GetError();
    SDLTest_AssertPass("Call to SDL_GetError()");
    SDLTest_AssertCheck(error != NULL, "Validate that error message was not NULL");
    if (error != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(error, expectedError1) == 0,
          "Validate error message, expected: '%s', got: '%s'", expectedError1, error);
    }
  }

  /* Invalid free pointer */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  SDL_FreePalette(NULL);
  SDLTest_AssertPass("Call to SDL_FreePalette(NULL)");
  error = SDL_GetError();
  SDLTest_AssertPass("Call to SDL_GetError()");
  SDLTest_AssertCheck(error != NULL, "Validate that error message was not NULL");
  if (error != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(error, expectedError2) == 0,
          "Validate error message, expected: '%s', got: '%s'", expectedError2, error);
  }

  return TEST_COMPLETED;
}

/**
 * @brief Call to SDL_CalculateGammaRamp
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_CalculateGammaRamp
 */
int
pixels_calcGammaRamp(void *arg)
{
  const char *expectedError1 = "Parameter 'gamma' is invalid";
  const char *expectedError2 = "Parameter 'ramp' is invalid";
  const char *error;
  float gamma;
  Uint16 *ramp;
  int variation;
  int i;
  int changed;
  Uint16 magic = 0xbeef;

  /* Allocate temp ramp array and fill with some value */
  ramp = (Uint16 *)SDL_malloc(256 * sizeof(Uint16));
  SDLTest_AssertCheck(ramp != NULL, "Validate temp ramp array could be allocated");
  if (ramp == NULL) return TEST_ABORTED;

  /* Make call with different gamma values */
  for (variation = 0; variation < 4; variation++) {
    switch (variation) {
      /* gamma = 0 all black */
      case 0:
        gamma = 0.0f;
        break;
      /* gamma = 1 identity */
      case 1:
        gamma = 1.0f;
        break;
      /* gamma = [0.2,0.8] normal range */
      case 2:
        gamma = 0.2f + 0.8f * SDLTest_RandomUnitFloat();
        break;
      /* gamma = >1.1 non-standard range */
      case 3:
        gamma = 1.1f + SDLTest_RandomUnitFloat();
        break;
    }

    /* Make call and check that values were updated */
    for (i = 0; i < 256; i++) ramp[i] = magic;
    SDL_CalculateGammaRamp(gamma, ramp);
    SDLTest_AssertPass("Call to SDL_CalculateGammaRamp(%f)", gamma);
    changed = 0;
    for (i = 0; i < 256; i++) if (ramp[i] != magic) changed++;
    SDLTest_AssertCheck(changed > 250, "Validate that ramp was calculated; expected: >250 values changed, got: %d values changed", changed);

    /* Additional value checks for some cases */
    i = SDLTest_RandomIntegerInRange(64,192);
    switch (variation) {
      case 0:
        SDLTest_AssertCheck(ramp[i] == 0, "Validate value at position %d; expected: 0, got: %d", i, ramp[i]);
        break;
      case 1:
        SDLTest_AssertCheck(ramp[i] == ((i << 8) | i), "Validate value at position %d; expected: %d, got: %d", i, (i << 8) | i, ramp[i]);
        break;
      case 2:
      case 3:
        SDLTest_AssertCheck(ramp[i] > 0, "Validate value at position %d; expected: >0, got: %d", i, ramp[i]);
        break;
    }
  }

  /* Negative cases */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  gamma = -1;
  for (i=0; i<256; i++) ramp[i] = magic;
  SDL_CalculateGammaRamp(gamma, ramp);
  SDLTest_AssertPass("Call to SDL_CalculateGammaRamp(%f)", gamma);
  error = SDL_GetError();
  SDLTest_AssertPass("Call to SDL_GetError()");
  SDLTest_AssertCheck(error != NULL, "Validate that error message was not NULL");
  if (error != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(error, expectedError1) == 0,
          "Validate error message, expected: '%s', got: '%s'", expectedError1, error);
  }
  changed = 0;
  for (i = 0; i < 256; i++) if (ramp[i] != magic) changed++;
  SDLTest_AssertCheck(changed ==0, "Validate that ramp unchanged; expected: 0 values changed got: %d values changed", changed);

  SDL_CalculateGammaRamp(0.5f, NULL);
  SDLTest_AssertPass("Call to SDL_CalculateGammaRamp(0.5,NULL)");
  error = SDL_GetError();
  SDLTest_AssertPass("Call to SDL_GetError()");
  SDLTest_AssertCheck(error != NULL, "Validate that error message was not NULL");
  if (error != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(error, expectedError2) == 0,
          "Validate error message, expected: '%s', got: '%s'", expectedError2, error);
  }

  /* Cleanup */
  SDL_free(ramp);


  return TEST_COMPLETED;
}

/* ================= Test References ================== */

/* Pixels test cases */
static const SDLTest_TestCaseReference pixelsTest1 =
        { (SDLTest_TestCaseFp)pixels_allocFreeFormat, "pixels_allocFreeFormat", "Call to SDL_AllocFormat and SDL_FreeFormat", TEST_ENABLED };

static const SDLTest_TestCaseReference pixelsTest2 =
        { (SDLTest_TestCaseFp)pixels_allocFreePalette, "pixels_allocFreePalette", "Call to SDL_AllocPalette and SDL_FreePalette", TEST_ENABLED };

static const SDLTest_TestCaseReference pixelsTest3 =
        { (SDLTest_TestCaseFp)pixels_calcGammaRamp, "pixels_calcGammaRamp", "Call to SDL_CalculateGammaRamp", TEST_ENABLED };

static const SDLTest_TestCaseReference pixelsTest4 =
        { (SDLTest_TestCaseFp)pixels_getPixelFormatName, "pixels_getPixelFormatName", "Call to SDL_GetPixelFormatName", TEST_ENABLED };

/* Sequence of Pixels test cases */
static const SDLTest_TestCaseReference *pixelsTests[] =  {
    &pixelsTest1, &pixelsTest2, &pixelsTest3, &pixelsTest4, NULL
};

/* Pixels test suite (global) */
SDLTest_TestSuiteReference pixelsTestSuite = {
    "Pixels",
    NULL,
    pixelsTests,
    NULL
};
