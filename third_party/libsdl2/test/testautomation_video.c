/**
 * Video test suite
 */

#include <stdio.h>
#include <string.h>

/* Visual Studio 2008 doesn't have stdint.h */
#if defined(_MSC_VER) && _MSC_VER <= 1500
#define UINT8_MAX   ~(Uint8)0
#define UINT16_MAX  ~(Uint16)0
#define UINT32_MAX  ~(Uint32)0
#define UINT64_MAX  ~(Uint64)0
#else
#include <stdint.h>
#endif

#include "SDL.h"
#include "SDL_test.h"

/* Private helpers */

/*
 * Create a test window
 */
SDL_Window *_createVideoSuiteTestWindow(const char *title)
{
  SDL_Window* window;
  int x, y, w, h;
  SDL_WindowFlags flags;

  /* Standard window */
  x = SDLTest_RandomIntegerInRange(1, 100);
  y = SDLTest_RandomIntegerInRange(1, 100);
  w = SDLTest_RandomIntegerInRange(320, 1024);
  h = SDLTest_RandomIntegerInRange(320, 768);
  flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;

  window = SDL_CreateWindow(title, x, y, w, h, flags);
  SDLTest_AssertPass("Call to SDL_CreateWindow('Title',%d,%d,%d,%d,%d)", x, y, w, h, flags);
  SDLTest_AssertCheck(window != NULL, "Validate that returned window struct is not NULL");

  return window;
}

/*
 * Destroy test window
 */
void _destroyVideoSuiteTestWindow(SDL_Window *window)
{
  if (window != NULL) {
     SDL_DestroyWindow(window);
     window = NULL;
     SDLTest_AssertPass("Call to SDL_DestroyWindow()");
  }
}

/* Test case functions */

/**
 * @brief Enable and disable screensaver while checking state
 */
int
video_enableDisableScreensaver(void *arg)
{
    SDL_bool initialResult;
    SDL_bool result;

    /* Get current state and proceed according to current state */
    initialResult = SDL_IsScreenSaverEnabled();
    SDLTest_AssertPass("Call to SDL_IsScreenSaverEnabled()");
    if (initialResult == SDL_TRUE) {

      /* Currently enabled: disable first, then enable again */

      /* Disable screensaver and check */
      SDL_DisableScreenSaver();
      SDLTest_AssertPass("Call to SDL_DisableScreenSaver()");
      result = SDL_IsScreenSaverEnabled();
      SDLTest_AssertPass("Call to SDL_IsScreenSaverEnabled()");
      SDLTest_AssertCheck(result == SDL_FALSE, "Verify result from SDL_IsScreenSaverEnabled, expected: %i, got: %i", SDL_FALSE, result);

      /* Enable screensaver and check */
      SDL_EnableScreenSaver();
      SDLTest_AssertPass("Call to SDL_EnableScreenSaver()");
      result = SDL_IsScreenSaverEnabled();
      SDLTest_AssertPass("Call to SDL_IsScreenSaverEnabled()");
      SDLTest_AssertCheck(result == SDL_TRUE, "Verify result from SDL_IsScreenSaverEnabled, expected: %i, got: %i", SDL_TRUE, result);

    } else {

      /* Currently disabled: enable first, then disable again */

      /* Enable screensaver and check */
      SDL_EnableScreenSaver();
      SDLTest_AssertPass("Call to SDL_EnableScreenSaver()");
      result = SDL_IsScreenSaverEnabled();
      SDLTest_AssertPass("Call to SDL_IsScreenSaverEnabled()");
      SDLTest_AssertCheck(result == SDL_TRUE, "Verify result from SDL_IsScreenSaverEnabled, expected: %i, got: %i", SDL_TRUE, result);

      /* Disable screensaver and check */
      SDL_DisableScreenSaver();
      SDLTest_AssertPass("Call to SDL_DisableScreenSaver()");
      result = SDL_IsScreenSaverEnabled();
      SDLTest_AssertPass("Call to SDL_IsScreenSaverEnabled()");
      SDLTest_AssertCheck(result == SDL_FALSE, "Verify result from SDL_IsScreenSaverEnabled, expected: %i, got: %i", SDL_FALSE, result);
    }

    return TEST_COMPLETED;
}

/**
 * @brief Tests the functionality of the SDL_CreateWindow function using different positions
 */
int
video_createWindowVariousPositions(void *arg)
{
  SDL_Window* window;
  const char* title = "video_createWindowVariousPositions Test Window";
  int x, y, w, h;
  int xVariation, yVariation;

  for (xVariation = 0; xVariation < 6; xVariation++) {
   for (yVariation = 0; yVariation < 6; yVariation++) {
    switch(xVariation) {
     case 0:
      /* Zero X Position */
      x = 0;
      break;
     case 1:
      /* Random X position inside screen */
      x = SDLTest_RandomIntegerInRange(1, 100);
      break;
     case 2:
      /* Random X position outside screen (positive) */
      x = SDLTest_RandomIntegerInRange(10000, 11000);
      break;
     case 3:
      /* Random X position outside screen (negative) */
      x = SDLTest_RandomIntegerInRange(-1000, -100);
      break;
     case 4:
      /* Centered X position */
      x = SDL_WINDOWPOS_CENTERED;
      break;
     case 5:
      /* Undefined X position */
      x = SDL_WINDOWPOS_UNDEFINED;
      break;
    }

    switch(yVariation) {
     case 0:
      /* Zero X Position */
      y = 0;
      break;
     case 1:
      /* Random X position inside screen */
      y = SDLTest_RandomIntegerInRange(1, 100);
      break;
     case 2:
      /* Random X position outside screen (positive) */
      y = SDLTest_RandomIntegerInRange(10000, 11000);
      break;
     case 3:
      /* Random Y position outside screen (negative) */
      y = SDLTest_RandomIntegerInRange(-1000, -100);
      break;
     case 4:
      /* Centered Y position */
      y = SDL_WINDOWPOS_CENTERED;
      break;
     case 5:
      /* Undefined Y position */
      y = SDL_WINDOWPOS_UNDEFINED;
      break;
    }

    w = SDLTest_RandomIntegerInRange(32, 96);
    h = SDLTest_RandomIntegerInRange(32, 96);
    window = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN);
    SDLTest_AssertPass("Call to SDL_CreateWindow('Title',%d,%d,%d,%d,SHOWN)", x, y, w, h);
    SDLTest_AssertCheck(window != NULL, "Validate that returned window struct is not NULL");

    /* Clean up */
    _destroyVideoSuiteTestWindow(window);
   }
  }

  return TEST_COMPLETED;
}

/**
 * @brief Tests the functionality of the SDL_CreateWindow function using different sizes
 */
int
video_createWindowVariousSizes(void *arg)
{
  SDL_Window* window;
  const char* title = "video_createWindowVariousSizes Test Window";
  int x, y, w, h;
  int wVariation, hVariation;

  x = SDLTest_RandomIntegerInRange(1, 100);
  y = SDLTest_RandomIntegerInRange(1, 100);
  for (wVariation = 0; wVariation < 3; wVariation++) {
   for (hVariation = 0; hVariation < 3; hVariation++) {
    switch(wVariation) {
     case 0:
      /* Width of 1 */
      w = 1;
      break;
     case 1:
      /* Random "normal" width */
      w = SDLTest_RandomIntegerInRange(320, 1920);
      break;
     case 2:
      /* Random "large" width */
      w = SDLTest_RandomIntegerInRange(2048, 4095);
      break;
    }

    switch(hVariation) {
     case 0:
      /* Height of 1 */
      h = 1;
      break;
     case 1:
      /* Random "normal" height */
      h = SDLTest_RandomIntegerInRange(320, 1080);
      break;
     case 2:
      /* Random "large" height */
      h = SDLTest_RandomIntegerInRange(2048, 4095);
      break;
     }

    window = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN);
    SDLTest_AssertPass("Call to SDL_CreateWindow('Title',%d,%d,%d,%d,SHOWN)", x, y, w, h);
    SDLTest_AssertCheck(window != NULL, "Validate that returned window struct is not NULL");

    /* Clean up */
    _destroyVideoSuiteTestWindow(window);
   }
  }

  return TEST_COMPLETED;
}

/**
 * @brief Tests the functionality of the SDL_CreateWindow function using different flags
 */
int
video_createWindowVariousFlags(void *arg)
{
  SDL_Window* window;
  const char* title = "video_createWindowVariousFlags Test Window";
  int x, y, w, h;
  int fVariation;
  SDL_WindowFlags flags;

  /* Standard window */
  x = SDLTest_RandomIntegerInRange(1, 100);
  y = SDLTest_RandomIntegerInRange(1, 100);
  w = SDLTest_RandomIntegerInRange(320, 1024);
  h = SDLTest_RandomIntegerInRange(320, 768);

  for (fVariation = 0; fVariation < 13; fVariation++) {
    switch(fVariation) {
     case 0:
      flags = SDL_WINDOW_FULLSCREEN;
      /* Skip - blanks screen; comment out next line to run test */
      continue;
      break;
     case 1:
      flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
      /* Skip - blanks screen; comment out next line to run test */
      continue;
      break;
     case 2:
      flags = SDL_WINDOW_OPENGL;
      break;
     case 3:
      flags = SDL_WINDOW_SHOWN;
      break;
     case 4:
      flags = SDL_WINDOW_HIDDEN;
      break;
     case 5:
      flags = SDL_WINDOW_BORDERLESS;
      break;
     case 6:
      flags = SDL_WINDOW_RESIZABLE;
      break;
     case 7:
      flags = SDL_WINDOW_MINIMIZED;
      break;
     case 8:
      flags = SDL_WINDOW_MAXIMIZED;
      break;
     case 9:
      flags = SDL_WINDOW_INPUT_GRABBED;
      break;
     case 10:
      flags = SDL_WINDOW_INPUT_FOCUS;
      break;
     case 11:
      flags = SDL_WINDOW_MOUSE_FOCUS;
      break;
     case 12:
      flags = SDL_WINDOW_FOREIGN;
      break;
    }

    window = SDL_CreateWindow(title, x, y, w, h, flags);
    SDLTest_AssertPass("Call to SDL_CreateWindow('Title',%d,%d,%d,%d,%d)", x, y, w, h, flags);
    SDLTest_AssertCheck(window != NULL, "Validate that returned window struct is not NULL");

    /* Clean up */
    _destroyVideoSuiteTestWindow(window);
  }

  return TEST_COMPLETED;
}


/**
 * @brief Tests the functionality of the SDL_GetWindowFlags function
 */
int
video_getWindowFlags(void *arg)
{
  SDL_Window* window;
  const char* title = "video_getWindowFlags Test Window";
  SDL_WindowFlags flags;
  Uint32 actualFlags;

  /* Reliable flag set always set in test window */
  flags = SDL_WINDOW_SHOWN;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window != NULL) {
      actualFlags = SDL_GetWindowFlags(window);
      SDLTest_AssertPass("Call to SDL_GetWindowFlags()");
      SDLTest_AssertCheck((flags & actualFlags) == flags, "Verify returned value has flags %d set, got: %d", flags, actualFlags);
  }

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  return TEST_COMPLETED;
}

/**
 * @brief Tests the functionality of the SDL_GetNumDisplayModes function
 */
int
video_getNumDisplayModes(void *arg)
{
  int result;
  int displayNum;
  int i;

  /* Get number of displays */
  displayNum = SDL_GetNumVideoDisplays();
  SDLTest_AssertPass("Call to SDL_GetNumVideoDisplays()");

  /* Make call for each display */
  for (i=0; i<displayNum; i++) {
    result = SDL_GetNumDisplayModes(i);
    SDLTest_AssertPass("Call to SDL_GetNumDisplayModes(%d)", i);
    SDLTest_AssertCheck(result >= 1, "Validate returned value from function; expected: >=1; got: %d", result);
  }

  return TEST_COMPLETED;
}

/**
 * @brief Tests negative call to SDL_GetNumDisplayModes function
 */
int
video_getNumDisplayModesNegative(void *arg)
{
  int result;
  int displayNum;
  int displayIndex;

  /* Get number of displays */
  displayNum = SDL_GetNumVideoDisplays();
  SDLTest_AssertPass("Call to SDL_GetNumVideoDisplays()");

  /* Invalid boundary values */
  displayIndex =  SDLTest_RandomSint32BoundaryValue(0, displayNum, SDL_FALSE);
  result = SDL_GetNumDisplayModes(displayIndex);
  SDLTest_AssertPass("Call to SDL_GetNumDisplayModes(%d=out-of-bounds/boundary)", displayIndex);
  SDLTest_AssertCheck(result < 0, "Validate returned value from function; expected: <0; got: %d", result);

  /* Large (out-of-bounds) display index */
  displayIndex = SDLTest_RandomIntegerInRange(-2000, -1000);
  result = SDL_GetNumDisplayModes(displayIndex);
  SDLTest_AssertPass("Call to SDL_GetNumDisplayModes(%d=out-of-bounds/large negative)", displayIndex);
  SDLTest_AssertCheck(result < 0, "Validate returned value from function; expected: <0; got: %d", result);

  displayIndex = SDLTest_RandomIntegerInRange(1000, 2000);
  result = SDL_GetNumDisplayModes(displayIndex);
  SDLTest_AssertPass("Call to SDL_GetNumDisplayModes(%d=out-of-bounds/large positive)", displayIndex);
  SDLTest_AssertCheck(result < 0, "Validate returned value from function; expected: <0; got: %d", result);

  return TEST_COMPLETED;
}

/**
 * @brief Tests the functionality of the SDL_GetClosestDisplayMode function against current resolution
 */
int
video_getClosestDisplayModeCurrentResolution(void *arg)
{
  int result;
  SDL_DisplayMode current;
  SDL_DisplayMode target;
  SDL_DisplayMode closest;
  SDL_DisplayMode* dResult;
  int displayNum;
  int i;
  int variation;

  /* Get number of displays */
  displayNum = SDL_GetNumVideoDisplays();
  SDLTest_AssertPass("Call to SDL_GetNumVideoDisplays()");

  /* Make calls for each display */
  for (i=0; i<displayNum; i++) {
    SDLTest_Log("Testing against display: %d", i);

    /* Get first display mode to get a sane resolution; this should always work */
    result = SDL_GetDisplayMode(i, 0, &current);
    SDLTest_AssertPass("Call to SDL_GetDisplayMode()");
    SDLTest_AssertCheck(result == 0, "Verify return value, expected: 0, got: %d", result);
    if (result != 0) {
      return TEST_ABORTED;
    }

    /* Set the desired resolution equals to current resolution */
    target.w = current.w;
    target.h = current.h;
    for (variation = 0; variation < 8; variation ++) {
      /* Vary constraints on other query parameters */
      target.format = (variation & 1) ? current.format : 0;
      target.refresh_rate = (variation & 2) ? current.refresh_rate : 0;
      target.driverdata = (variation & 4) ? current.driverdata : 0;

      /* Make call */
      dResult = SDL_GetClosestDisplayMode(i, &target, &closest);
      SDLTest_AssertPass("Call to SDL_GetClosestDisplayMode(target=current/variation%d)", variation);
      SDLTest_AssertCheck(dResult != NULL, "Verify returned value is not NULL");

      /* Check that one gets the current resolution back again */
      SDLTest_AssertCheck(closest.w == current.w, "Verify returned width matches current width; expected: %d, got: %d", current.w, closest.w);
      SDLTest_AssertCheck(closest.h == current.h, "Verify returned height matches current height; expected: %d, got: %d", current.h, closest.h);
      SDLTest_AssertCheck(closest.w == dResult->w, "Verify return value matches assigned value; expected: %d, got: %d", closest.w, dResult->w);
      SDLTest_AssertCheck(closest.h == dResult->h, "Verify return value matches assigned value; expected: %d, got: %d", closest.h, dResult->h);
    }
  }

  return TEST_COMPLETED;
}

/**
 * @brief Tests the functionality of the SDL_GetClosestDisplayMode function against random resolution
 */
int
video_getClosestDisplayModeRandomResolution(void *arg)
{
  SDL_DisplayMode target;
  SDL_DisplayMode closest;
  SDL_DisplayMode* dResult;
  int displayNum;
  int i;
  int variation;

  /* Get number of displays */
  displayNum = SDL_GetNumVideoDisplays();
  SDLTest_AssertPass("Call to SDL_GetNumVideoDisplays()");

  /* Make calls for each display */
  for (i=0; i<displayNum; i++) {
    SDLTest_Log("Testing against display: %d", i);

    for (variation = 0; variation < 16; variation ++) {

      /* Set random constraints */
      target.w = (variation & 1) ? SDLTest_RandomIntegerInRange(1, 4096) : 0;
      target.h = (variation & 2) ? SDLTest_RandomIntegerInRange(1, 4096) : 0;
      target.format = (variation & 4) ? SDLTest_RandomIntegerInRange(1, 10) : 0;
      target.refresh_rate = (variation & 8) ? SDLTest_RandomIntegerInRange(25, 120) : 0;
      target.driverdata = 0;

      /* Make call; may or may not find anything, so don't validate any further */
      dResult = SDL_GetClosestDisplayMode(i, &target, &closest);
      SDLTest_AssertPass("Call to SDL_GetClosestDisplayMode(target=random/variation%d)", variation);
    }
  }

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowBrightness
 *
* @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowBrightness
 */
int
video_getWindowBrightness(void *arg)
{
  SDL_Window* window;
  const char* title = "video_getWindowBrightness Test Window";
  float result;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window != NULL) {
      result = SDL_GetWindowBrightness(window);
      SDLTest_AssertPass("Call to SDL_GetWindowBrightness()");
      SDLTest_AssertCheck(result >= 0.0 && result <= 1.0, "Validate range of result value; expected: [0.0, 1.0], got: %f", result);
  }

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowBrightness with invalid input
 *
* @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowBrightness
 */
int
video_getWindowBrightnessNegative(void *arg)
{
  const char *invalidWindowError = "Invalid window";
  char *lastError;
  float result;

  /* Call against invalid window */
  result = SDL_GetWindowBrightness(NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowBrightness(window=NULL)");
  SDLTest_AssertCheck(result == 1.0, "Validate result value; expected: 1.0, got: %f", result);
  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL, "Verify error message is not NULL");
  if (lastError != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(lastError, invalidWindowError) == 0,
         "SDL_GetError(): expected message '%s', was message: '%s'",
         invalidWindowError,
         lastError);
  }

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowDisplayMode
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowDisplayMode
 */
int
video_getWindowDisplayMode(void *arg)
{
  SDL_Window* window;
  const char* title = "video_getWindowDisplayMode Test Window";
  SDL_DisplayMode mode;
  int result;

  /* Invalidate part of the mode content so we can check values later */
  mode.w = -1;
  mode.h = -1;
  mode.refresh_rate = -1;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window != NULL) {
      result = SDL_GetWindowDisplayMode(window, &mode);
      SDLTest_AssertPass("Call to SDL_GetWindowDisplayMode()");
      SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);
      SDLTest_AssertCheck(mode.w > 0, "Validate mode.w content; expected: >0, got: %d", mode.w);
      SDLTest_AssertCheck(mode.h > 0, "Validate mode.h content; expected: >0, got: %d", mode.h);
      SDLTest_AssertCheck(mode.refresh_rate > 0, "Validate mode.refresh_rate content; expected: >0, got: %d", mode.refresh_rate);
  }

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  return TEST_COMPLETED;
}

/* Helper function that checks for an 'Invalid window' error */
void _checkInvalidWindowError()
{
  const char *invalidWindowError = "Invalid window";
  char *lastError;

  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL, "Verify error message is not NULL");
  if (lastError != NULL) {
      SDLTest_AssertCheck(SDL_strcmp(lastError, invalidWindowError) == 0,
         "SDL_GetError(): expected message '%s', was message: '%s'",
         invalidWindowError,
         lastError);
      SDL_ClearError();
      SDLTest_AssertPass("Call to SDL_ClearError()");
  }
}

/**
 * @brief Tests call to SDL_GetWindowDisplayMode with invalid input
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowDisplayMode
 */
int
video_getWindowDisplayModeNegative(void *arg)
{
  const char *expectedError = "Parameter 'mode' is invalid";
  char *lastError;
  SDL_Window* window;
  const char* title = "video_getWindowDisplayModeNegative Test Window";
  SDL_DisplayMode mode;
  int result;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window != NULL) {
      result = SDL_GetWindowDisplayMode(window, NULL);
      SDLTest_AssertPass("Call to SDL_GetWindowDisplayMode(...,mode=NULL)");
      SDLTest_AssertCheck(result == -1, "Validate result value; expected: -1, got: %d", result);
      lastError = (char *)SDL_GetError();
      SDLTest_AssertPass("SDL_GetError()");
      SDLTest_AssertCheck(lastError != NULL, "Verify error message is not NULL");
      if (lastError != NULL) {
        SDLTest_AssertCheck(SDL_strcmp(lastError, expectedError) == 0,
             "SDL_GetError(): expected message '%s', was message: '%s'",
             expectedError,
             lastError);
      }
  }

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  /* Call against invalid window */
  result = SDL_GetWindowDisplayMode(NULL, &mode);
  SDLTest_AssertPass("Call to SDL_GetWindowDisplayMode(window=NULL,...)");
  SDLTest_AssertCheck(result == -1, "Validate result value; expected: -1, got: %d", result);
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowGammaRamp
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowGammaRamp
 */
int
video_getWindowGammaRamp(void *arg)
{
  SDL_Window* window;
  const char* title = "video_getWindowGammaRamp Test Window";
  Uint16 red[256];
  Uint16 green[256];
  Uint16 blue[256];
  int result;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  /* Retrieve no channel */
  result = SDL_GetWindowGammaRamp(window, NULL, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(all NULL)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  /* Retrieve single channel */
  result = SDL_GetWindowGammaRamp(window, red, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(r)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  result = SDL_GetWindowGammaRamp(window, NULL, green, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(g)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  result = SDL_GetWindowGammaRamp(window, NULL, NULL, blue);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(b)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  /* Retrieve two channels */
  result = SDL_GetWindowGammaRamp(window, red, green, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(r, g)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  result = SDL_GetWindowGammaRamp(window, NULL, green, blue);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(g,b)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  result = SDL_GetWindowGammaRamp(window, red, NULL, blue);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(r,b)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  /* Retrieve all channels */
  result = SDL_GetWindowGammaRamp(window, red, green, blue);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(r,g,b)");
  SDLTest_AssertCheck(result == 0, "Validate result value; expected: 0, got: %d", result);

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowGammaRamp with invalid input
 *
* @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowGammaRamp
 */
int
video_getWindowGammaRampNegative(void *arg)
{
  Uint16 red[256];
  Uint16 green[256];
  Uint16 blue[256];
  int result;

  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");

  /* Call against invalid window */
  result = SDL_GetWindowGammaRamp(NULL, red, green, blue);
  SDLTest_AssertPass("Call to SDL_GetWindowGammaRamp(window=NULL,r,g,b)");
  SDLTest_AssertCheck(result == -1, "Validate result value; expected: -1, got: %i", result);
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}

/* Helper for setting and checking the window grab state */
void
_setAndCheckWindowGrabState(SDL_Window* window, SDL_bool desiredState)
{
  SDL_bool currentState;

  /* Set state */
  SDL_SetWindowGrab(window, desiredState);
  SDLTest_AssertPass("Call to SDL_SetWindowGrab(%s)", (desiredState == SDL_FALSE) ? "SDL_FALSE" : "SDL_TRUE");

  /* Get and check state */
  currentState = SDL_GetWindowGrab(window);
  SDLTest_AssertPass("Call to SDL_GetWindowGrab()");
  SDLTest_AssertCheck(
      currentState == desiredState,
      "Validate returned state; expected: %s, got: %s",
      (desiredState == SDL_FALSE) ? "SDL_FALSE" : "SDL_TRUE",
      (currentState == SDL_FALSE) ? "SDL_FALSE" : "SDL_TRUE");
}

/**
 * @brief Tests call to SDL_GetWindowGrab and SDL_SetWindowGrab
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowGrab
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_SetWindowGrab
 */
int
video_getSetWindowGrab(void *arg)
{
  const char* title = "video_getSetWindowGrab Test Window";
  SDL_Window* window;
  SDL_bool originalState, dummyState, currentState, desiredState;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  /* Get state */
  originalState = SDL_GetWindowGrab(window);
  SDLTest_AssertPass("Call to SDL_GetWindowGrab()");

  /* F */
  _setAndCheckWindowGrabState(window, SDL_FALSE);

  /* F --> F */
  _setAndCheckWindowGrabState(window, SDL_FALSE);

  /* F --> T */
  _setAndCheckWindowGrabState(window, SDL_TRUE);

  /* T --> T */
  _setAndCheckWindowGrabState(window, SDL_TRUE);

  /* T --> F */
  _setAndCheckWindowGrabState(window, SDL_FALSE);

  /* Negative tests */
  dummyState = SDL_GetWindowGrab(NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowGrab(window=NULL)");
  _checkInvalidWindowError();

  SDL_SetWindowGrab(NULL, SDL_FALSE);
  SDLTest_AssertPass("Call to SDL_SetWindowGrab(window=NULL,SDL_FALSE)");
  _checkInvalidWindowError();

  SDL_SetWindowGrab(NULL, SDL_TRUE);
  SDLTest_AssertPass("Call to SDL_SetWindowGrab(window=NULL,SDL_FALSE)");
  _checkInvalidWindowError();

  /* State should still be F */
  desiredState = SDL_FALSE;
  currentState = SDL_GetWindowGrab(window);
  SDLTest_AssertPass("Call to SDL_GetWindowGrab()");
  SDLTest_AssertCheck(
      currentState == desiredState,
      "Validate returned state; expected: %s, got: %s",
      (desiredState == SDL_FALSE) ? "SDL_FALSE" : "SDL_TRUE",
      (currentState == SDL_FALSE) ? "SDL_FALSE" : "SDL_TRUE");

  /* Restore state */
  _setAndCheckWindowGrabState(window, originalState);

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  return TEST_COMPLETED;
}


/**
 * @brief Tests call to SDL_GetWindowID and SDL_GetWindowFromID
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowID
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_SetWindowFromID
 */
int
video_getWindowId(void *arg)
{
  const char* title = "video_getWindowId Test Window";
  SDL_Window* window;
  SDL_Window* result;
  Uint32 id, randomId;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  /* Get ID */
  id = SDL_GetWindowID(window);
  SDLTest_AssertPass("Call to SDL_GetWindowID()");

  /* Get window from ID */
  result = SDL_GetWindowFromID(id);
  SDLTest_AssertPass("Call to SDL_GetWindowID(%d)", id);
  SDLTest_AssertCheck(result == window, "Verify result matches window pointer");

  /* Get window from random large ID, no result check */
  randomId = SDLTest_RandomIntegerInRange(UINT8_MAX,UINT16_MAX);
  result = SDL_GetWindowFromID(randomId);
  SDLTest_AssertPass("Call to SDL_GetWindowID(%d/random_large)", randomId);

  /* Get window from 0 and Uint32 max ID, no result check */
  result = SDL_GetWindowFromID(0);
  SDLTest_AssertPass("Call to SDL_GetWindowID(0)");
  result = SDL_GetWindowFromID(UINT32_MAX);
  SDLTest_AssertPass("Call to SDL_GetWindowID(UINT32_MAX)");

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  /* Get window from ID for closed window */
  result = SDL_GetWindowFromID(id);
  SDLTest_AssertPass("Call to SDL_GetWindowID(%d/closed_window)", id);
  SDLTest_AssertCheck(result == NULL, "Verify result is NULL");

  /* Negative test */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  id = SDL_GetWindowID(NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowID(window=NULL)");
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowPixelFormat
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowPixelFormat
 */
int
video_getWindowPixelFormat(void *arg)
{
  const char* title = "video_getWindowPixelFormat Test Window";
  SDL_Window* window;
  Uint32 format;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  /* Get format */
  format = SDL_GetWindowPixelFormat(window);
  SDLTest_AssertPass("Call to SDL_GetWindowPixelFormat()");
  SDLTest_AssertCheck(format != SDL_PIXELFORMAT_UNKNOWN, "Verify that returned format is valid; expected: != %d, got: %d", SDL_PIXELFORMAT_UNKNOWN, format);

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  /* Negative test */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  format = SDL_GetWindowPixelFormat(NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowPixelFormat(window=NULL)");
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowPosition and SDL_SetWindowPosition
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowPosition
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_SetWindowPosition
 */
int
video_getSetWindowPosition(void *arg)
{
  const char* title = "video_getSetWindowPosition Test Window";
  SDL_Window* window;
  int xVariation, yVariation;
  int referenceX, referenceY;
  int currentX, currentY;
  int desiredX, desiredY;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  for (xVariation = 0; xVariation < 4; xVariation++) {
   for (yVariation = 0; yVariation < 4; yVariation++) {
    switch(xVariation) {
     case 0:
      /* Zero X Position */
      desiredX = 0;
      break;
     case 1:
      /* Random X position inside screen */
      desiredX = SDLTest_RandomIntegerInRange(1, 100);
      break;
     case 2:
      /* Random X position outside screen (positive) */
      desiredX = SDLTest_RandomIntegerInRange(10000, 11000);
      break;
     case 3:
      /* Random X position outside screen (negative) */
      desiredX = SDLTest_RandomIntegerInRange(-1000, -100);
      break;
    }

    switch(yVariation) {
     case 0:
      /* Zero X Position */
      desiredY = 0;
      break;
     case 1:
      /* Random X position inside screen */
      desiredY = SDLTest_RandomIntegerInRange(1, 100);
      break;
     case 2:
      /* Random X position outside screen (positive) */
      desiredY = SDLTest_RandomIntegerInRange(10000, 11000);
      break;
     case 3:
      /* Random Y position outside screen (negative) */
      desiredY = SDLTest_RandomIntegerInRange(-1000, -100);
      break;
    }

    /* Set position */
    SDL_SetWindowPosition(window, desiredX, desiredY);
    SDLTest_AssertPass("Call to SDL_SetWindowPosition(...,%d,%d)", desiredX, desiredY);

    /* Get position */
    currentX = desiredX + 1;
    currentY = desiredY + 1;
    SDL_GetWindowPosition(window, &currentX, &currentY);
    SDLTest_AssertPass("Call to SDL_GetWindowPosition()");
    SDLTest_AssertCheck(desiredX == currentX, "Verify returned X position; expected: %d, got: %d", desiredX, currentX);
    SDLTest_AssertCheck(desiredY == currentY, "Verify returned Y position; expected: %d, got: %d", desiredY, currentY);

    /* Get position X */
    currentX = desiredX + 1;
    SDL_GetWindowPosition(window, &currentX, NULL);
    SDLTest_AssertPass("Call to SDL_GetWindowPosition(&y=NULL)");
    SDLTest_AssertCheck(desiredX == currentX, "Verify returned X position; expected: %d, got: %d", desiredX, currentX);

    /* Get position Y */
    currentY = desiredY + 1;
    SDL_GetWindowPosition(window, NULL, &currentY);
    SDLTest_AssertPass("Call to SDL_GetWindowPosition(&x=NULL)");
    SDLTest_AssertCheck(desiredY == currentY, "Verify returned Y position; expected: %d, got: %d", desiredY, currentY);
   }
  }

  /* Dummy call with both pointers NULL */
  SDL_GetWindowPosition(window, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowPosition(&x=NULL,&y=NULL)");

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  /* Set some 'magic' value for later check that nothing was changed */
  referenceX = SDLTest_RandomSint32();
  referenceY = SDLTest_RandomSint32();
  currentX = referenceX;
  currentY = referenceY;
  desiredX = SDLTest_RandomSint32();
  desiredY = SDLTest_RandomSint32();

  /* Negative tests */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  SDL_GetWindowPosition(NULL, &currentX, &currentY);
  SDLTest_AssertPass("Call to SDL_GetWindowPosition(window=NULL)");
  SDLTest_AssertCheck(
    currentX == referenceX && currentY == referenceY,
    "Verify that content of X and Y pointers has not been modified; expected: %d,%d; got: %d,%d",
    referenceX, referenceY,
    currentX, currentY);
  _checkInvalidWindowError();

  SDL_GetWindowPosition(NULL, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowPosition(NULL, NULL, NULL)");
  _checkInvalidWindowError();

  SDL_SetWindowPosition(NULL, desiredX, desiredY);
  SDLTest_AssertPass("Call to SDL_SetWindowPosition(window=NULL)");
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}

/* Helper function that checks for an 'Invalid parameter' error */
void _checkInvalidParameterError()
{
  const char *invalidParameterError = "Parameter";
  char *lastError;

  lastError = (char *)SDL_GetError();
  SDLTest_AssertPass("SDL_GetError()");
  SDLTest_AssertCheck(lastError != NULL, "Verify error message is not NULL");
  if (lastError != NULL) {
      SDLTest_AssertCheck(SDL_strncmp(lastError, invalidParameterError, SDL_strlen(invalidParameterError)) == 0,
         "SDL_GetError(): expected message starts with '%s', was message: '%s'",
         invalidParameterError,
         lastError);
      SDL_ClearError();
      SDLTest_AssertPass("Call to SDL_ClearError()");
  }
}

/**
 * @brief Tests call to SDL_GetWindowSize and SDL_SetWindowSize
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowSize
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_SetWindowSize
 */
int
video_getSetWindowSize(void *arg)
{
  const char* title = "video_getSetWindowSize Test Window";
  SDL_Window* window;
  int result;
  SDL_Rect display;
  int maxwVariation, maxhVariation;
  int wVariation, hVariation;
  int referenceW, referenceH;
  int currentW, currentH;
  int desiredW, desiredH;

  /* Get display bounds for size range */
  result = SDL_GetDisplayBounds(0, &display);
  SDLTest_AssertPass("SDL_GetDisplayBounds()");
  SDLTest_AssertCheck(result == 0, "Verify return value; expected: 0, got: %d", result);
  if (result != 0) return TEST_ABORTED;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

#ifdef __WIN32__
  /* Platform clips window size to screen size */
  maxwVariation = 4;
  maxhVariation = 4;
#else
  /* Platform allows window size >= screen size */
  maxwVariation = 5;
  maxhVariation = 5;
#endif
  
  for (wVariation = 0; wVariation < maxwVariation; wVariation++) {
   for (hVariation = 0; hVariation < maxhVariation; hVariation++) {
    switch(wVariation) {
     case 0:
      /* 1 Pixel Wide */
      desiredW = 1;
      break;
     case 1:
      /* Random width inside screen */
      desiredW = SDLTest_RandomIntegerInRange(1, 100);
      break;
     case 2:
      /* Width 1 pixel smaller than screen */
      desiredW = display.w - 1;
      break;
     case 3:
      /* Width at screen size */
      desiredW = display.w;
      break;
     case 4:
      /* Width 1 pixel larger than screen */
      desiredW = display.w + 1;
      break;
    }

    switch(hVariation) {
     case 0:
      /* 1 Pixel High */
      desiredH = 1;
      break;
     case 1:
      /* Random height inside screen */
      desiredH = SDLTest_RandomIntegerInRange(1, 100);
      break;
     case 2:
      /* Height 1 pixel smaller than screen */
      desiredH = display.h - 1;
      break;
     case 3:
      /* Height at screen size */
      desiredH = display.h;
      break;
     case 4:
      /* Height 1 pixel larger than screen */
      desiredH = display.h + 1;
      break;
    }

    /* Set size */
    SDL_SetWindowSize(window, desiredW, desiredH);
    SDLTest_AssertPass("Call to SDL_SetWindowSize(...,%d,%d)", desiredW, desiredH);

    /* Get size */
    currentW = desiredW + 1;
    currentH = desiredH + 1;
    SDL_GetWindowSize(window, &currentW, &currentH);
    SDLTest_AssertPass("Call to SDL_GetWindowSize()");
    SDLTest_AssertCheck(desiredW == currentW, "Verify returned width; expected: %d, got: %d", desiredW, currentW);
    SDLTest_AssertCheck(desiredH == currentH, "Verify returned height; expected: %d, got: %d", desiredH, currentH);

    /* Get just width */
    currentW = desiredW + 1;
    SDL_GetWindowSize(window, &currentW, NULL);
    SDLTest_AssertPass("Call to SDL_GetWindowSize(&h=NULL)");
    SDLTest_AssertCheck(desiredW == currentW, "Verify returned width; expected: %d, got: %d", desiredW, currentW);

    /* Get just height */
    currentH = desiredH + 1;
    SDL_GetWindowSize(window, NULL, &currentH);
    SDLTest_AssertPass("Call to SDL_GetWindowSize(&w=NULL)");
    SDLTest_AssertCheck(desiredH == currentH, "Verify returned height; expected: %d, got: %d", desiredH, currentH);
   }
  }

  /* Dummy call with both pointers NULL */
  SDL_GetWindowSize(window, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowSize(&w=NULL,&h=NULL)");

  /* Negative tests for parameter input */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  for (desiredH = -2; desiredH < 2; desiredH++) {
    for (desiredW = -2; desiredW < 2; desiredW++) {
      if (desiredW <= 0 || desiredH <= 0) {
        SDL_SetWindowSize(window, desiredW, desiredH);
        SDLTest_AssertPass("Call to SDL_SetWindowSize(...,%d,%d)", desiredW, desiredH);
        _checkInvalidParameterError();
      }
    }
  }

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  /* Set some 'magic' value for later check that nothing was changed */
  referenceW = SDLTest_RandomSint32();
  referenceH = SDLTest_RandomSint32();
  currentW = referenceW;
  currentH = referenceH;
  desiredW = SDLTest_RandomSint32();
  desiredH = SDLTest_RandomSint32();

  /* Negative tests for window input */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  SDL_GetWindowSize(NULL, &currentW, &currentH);
  SDLTest_AssertPass("Call to SDL_GetWindowSize(window=NULL)");
  SDLTest_AssertCheck(
    currentW == referenceW && currentH == referenceH,
    "Verify that content of W and H pointers has not been modified; expected: %d,%d; got: %d,%d",
    referenceW, referenceH,
    currentW, currentH);
  _checkInvalidWindowError();

  SDL_GetWindowSize(NULL, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowSize(NULL, NULL, NULL)");
  _checkInvalidWindowError();

  SDL_SetWindowSize(NULL, desiredW, desiredH);
  SDLTest_AssertPass("Call to SDL_SetWindowSize(window=NULL)");
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowMinimumSize and SDL_SetWindowMinimumSize
 *
 */
int
video_getSetWindowMinimumSize(void *arg)
{
  const char* title = "video_getSetWindowMinimumSize Test Window";
  SDL_Window* window;
  int result;
  SDL_Rect display;
  int wVariation, hVariation;
  int referenceW, referenceH;
  int currentW, currentH;
  int desiredW, desiredH;

  /* Get display bounds for size range */
  result = SDL_GetDisplayBounds(0, &display);
  SDLTest_AssertPass("SDL_GetDisplayBounds()");
  SDLTest_AssertCheck(result == 0, "Verify return value; expected: 0, got: %d", result);
  if (result != 0) return TEST_ABORTED;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  for (wVariation = 0; wVariation < 5; wVariation++) {
   for (hVariation = 0; hVariation < 5; hVariation++) {
    switch(wVariation) {
     case 0:
      /* 1 Pixel Wide */
      desiredW = 1;
      break;
     case 1:
      /* Random width inside screen */
      desiredW = SDLTest_RandomIntegerInRange(2, display.w - 1);
      break;
     case 2:
      /* Width at screen size */
      desiredW = display.w;
      break;
    }

    switch(hVariation) {
     case 0:
      /* 1 Pixel High */
      desiredH = 1;
      break;
     case 1:
      /* Random height inside screen */
      desiredH = SDLTest_RandomIntegerInRange(2, display.h - 1);
      break;
     case 2:
      /* Height at screen size */
      desiredH = display.h;
      break;
     case 4:
      /* Height 1 pixel larger than screen */
      desiredH = display.h + 1;
      break;
    }

    /* Set size */
    SDL_SetWindowMinimumSize(window, desiredW, desiredH);
    SDLTest_AssertPass("Call to SDL_SetWindowMinimumSize(...,%d,%d)", desiredW, desiredH);

    /* Get size */
    currentW = desiredW + 1;
    currentH = desiredH + 1;
    SDL_GetWindowMinimumSize(window, &currentW, &currentH);
    SDLTest_AssertPass("Call to SDL_GetWindowMinimumSize()");
    SDLTest_AssertCheck(desiredW == currentW, "Verify returned width; expected: %d, got: %d", desiredW, currentW);
    SDLTest_AssertCheck(desiredH == currentH, "Verify returned height; expected: %d, got: %d", desiredH, currentH);

    /* Get just width */
    currentW = desiredW + 1;
    SDL_GetWindowMinimumSize(window, &currentW, NULL);
    SDLTest_AssertPass("Call to SDL_GetWindowMinimumSize(&h=NULL)");
    SDLTest_AssertCheck(desiredW == currentW, "Verify returned width; expected: %d, got: %d", desiredW, currentH);

    /* Get just height */
    currentH = desiredH + 1;
    SDL_GetWindowMinimumSize(window, NULL, &currentH);
    SDLTest_AssertPass("Call to SDL_GetWindowMinimumSize(&w=NULL)");
    SDLTest_AssertCheck(desiredH == currentH, "Verify returned height; expected: %d, got: %d", desiredW, currentH);
   }
  }

  /* Dummy call with both pointers NULL */
  SDL_GetWindowMinimumSize(window, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowMinimumSize(&w=NULL,&h=NULL)");

  /* Negative tests for parameter input */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  for (desiredH = -2; desiredH < 2; desiredH++) {
    for (desiredW = -2; desiredW < 2; desiredW++) {
      if (desiredW <= 0 || desiredH <= 0) {
        SDL_SetWindowMinimumSize(window, desiredW, desiredH);
        SDLTest_AssertPass("Call to SDL_SetWindowMinimumSize(...,%d,%d)", desiredW, desiredH);
        _checkInvalidParameterError();
      }
    }
  }

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  /* Set some 'magic' value for later check that nothing was changed */
  referenceW = SDLTest_RandomSint32();
  referenceH = SDLTest_RandomSint32();
  currentW = referenceW;
  currentH = referenceH;
  desiredW = SDLTest_RandomSint32();
  desiredH = SDLTest_RandomSint32();

  /* Negative tests for window input */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  SDL_GetWindowMinimumSize(NULL, &currentW, &currentH);
  SDLTest_AssertPass("Call to SDL_GetWindowMinimumSize(window=NULL)");
  SDLTest_AssertCheck(
    currentW == referenceW && currentH == referenceH,
    "Verify that content of W and H pointers has not been modified; expected: %d,%d; got: %d,%d",
    referenceW, referenceH,
    currentW, currentH);
  _checkInvalidWindowError();

  SDL_GetWindowMinimumSize(NULL, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowMinimumSize(NULL, NULL, NULL)");
  _checkInvalidWindowError();

  SDL_SetWindowMinimumSize(NULL, desiredW, desiredH);
  SDLTest_AssertPass("Call to SDL_SetWindowMinimumSize(window=NULL)");
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}

/**
 * @brief Tests call to SDL_GetWindowMaximumSize and SDL_SetWindowMaximumSize
 *
 */
int
video_getSetWindowMaximumSize(void *arg)
{
  const char* title = "video_getSetWindowMaximumSize Test Window";
  SDL_Window* window;
  int result;
  SDL_Rect display;
  int wVariation, hVariation;
  int referenceW, referenceH;
  int currentW, currentH;
  int desiredW, desiredH;

  /* Get display bounds for size range */
  result = SDL_GetDisplayBounds(0, &display);
  SDLTest_AssertPass("SDL_GetDisplayBounds()");
  SDLTest_AssertCheck(result == 0, "Verify return value; expected: 0, got: %d", result);
  if (result != 0) return TEST_ABORTED;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  for (wVariation = 0; wVariation < 3; wVariation++) {
   for (hVariation = 0; hVariation < 3; hVariation++) {
    switch(wVariation) {
     case 0:
      /* 1 Pixel Wide */
      desiredW = 1;
      break;
     case 1:
      /* Random width inside screen */
      desiredW = SDLTest_RandomIntegerInRange(2, display.w - 1);
      break;
     case 2:
      /* Width at screen size */
      desiredW = display.w;
      break;
    }

    switch(hVariation) {
     case 0:
      /* 1 Pixel High */
      desiredH = 1;
      break;
     case 1:
      /* Random height inside screen */
      desiredH = SDLTest_RandomIntegerInRange(2, display.h - 1);
      break;
     case 2:
      /* Height at screen size */
      desiredH = display.h;
      break;
    }

    /* Set size */
    SDL_SetWindowMaximumSize(window, desiredW, desiredH);
    SDLTest_AssertPass("Call to SDL_SetWindowMaximumSize(...,%d,%d)", desiredW, desiredH);

    /* Get size */
    currentW = desiredW + 1;
    currentH = desiredH + 1;
    SDL_GetWindowMaximumSize(window, &currentW, &currentH);
    SDLTest_AssertPass("Call to SDL_GetWindowMaximumSize()");
    SDLTest_AssertCheck(desiredW == currentW, "Verify returned width; expected: %d, got: %d", desiredW, currentW);
    SDLTest_AssertCheck(desiredH == currentH, "Verify returned height; expected: %d, got: %d", desiredH, currentH);

    /* Get just width */
    currentW = desiredW + 1;
    SDL_GetWindowMaximumSize(window, &currentW, NULL);
    SDLTest_AssertPass("Call to SDL_GetWindowMaximumSize(&h=NULL)");
    SDLTest_AssertCheck(desiredW == currentW, "Verify returned width; expected: %d, got: %d", desiredW, currentH);

    /* Get just height */
    currentH = desiredH + 1;
    SDL_GetWindowMaximumSize(window, NULL, &currentH);
    SDLTest_AssertPass("Call to SDL_GetWindowMaximumSize(&w=NULL)");
    SDLTest_AssertCheck(desiredH == currentH, "Verify returned height; expected: %d, got: %d", desiredW, currentH);
   }
  }

  /* Dummy call with both pointers NULL */
  SDL_GetWindowMaximumSize(window, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowMaximumSize(&w=NULL,&h=NULL)");

  /* Negative tests for parameter input */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  for (desiredH = -2; desiredH < 2; desiredH++) {
    for (desiredW = -2; desiredW < 2; desiredW++) {
      if (desiredW <= 0 || desiredH <= 0) {
        SDL_SetWindowMaximumSize(window, desiredW, desiredH);
        SDLTest_AssertPass("Call to SDL_SetWindowMaximumSize(...,%d,%d)", desiredW, desiredH);
        _checkInvalidParameterError();
      }
    }
  }

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  /* Set some 'magic' value for later check that nothing was changed */
  referenceW = SDLTest_RandomSint32();
  referenceH = SDLTest_RandomSint32();
  currentW = referenceW;
  currentH = referenceH;
  desiredW = SDLTest_RandomSint32();
  desiredH = SDLTest_RandomSint32();

  /* Negative tests */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");
  SDL_GetWindowMaximumSize(NULL, &currentW, &currentH);
  SDLTest_AssertPass("Call to SDL_GetWindowMaximumSize(window=NULL)");
  SDLTest_AssertCheck(
    currentW == referenceW && currentH == referenceH,
    "Verify that content of W and H pointers has not been modified; expected: %d,%d; got: %d,%d",
    referenceW, referenceH,
    currentW, currentH);
  _checkInvalidWindowError();

  SDL_GetWindowMaximumSize(NULL, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowMaximumSize(NULL, NULL, NULL)");
  _checkInvalidWindowError();

  SDL_SetWindowMaximumSize(NULL, desiredW, desiredH);
  SDLTest_AssertPass("Call to SDL_SetWindowMaximumSize(window=NULL)");
  _checkInvalidWindowError();

  return TEST_COMPLETED;
}


/**
 * @brief Tests call to SDL_SetWindowData and SDL_GetWindowData
 *
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_SetWindowData
 * @sa http://wiki.libsdl.org/moin.fcg/SDL_GetWindowData
 */
int
video_getSetWindowData(void *arg)
{
  int returnValue = TEST_COMPLETED;
  const char* title = "video_setGetWindowData Test Window";
  SDL_Window* window;
  const char *referenceName = "TestName";
  const char *name = "TestName";
  const char *referenceName2 = "TestName2";
  const char *name2 = "TestName2";
  int datasize;
  char *referenceUserdata = NULL;
  char *userdata = NULL;
  char *referenceUserdata2 = NULL;
  char *userdata2 = NULL;
  char *result;
  int iteration;

  /* Call against new test window */
  window = _createVideoSuiteTestWindow(title);
  if (window == NULL) return TEST_ABORTED;

  /* Create testdata */
  datasize = SDLTest_RandomIntegerInRange(1, 32);
  referenceUserdata =  SDLTest_RandomAsciiStringOfSize(datasize);
  if (referenceUserdata == NULL) {
    returnValue = TEST_ABORTED;
    goto cleanup;
  }
  userdata = SDL_strdup(referenceUserdata);
  if (userdata == NULL) {
    returnValue = TEST_ABORTED;
    goto cleanup;
  }
  datasize = SDLTest_RandomIntegerInRange(1, 32);
  referenceUserdata2 =  SDLTest_RandomAsciiStringOfSize(datasize);
  if (referenceUserdata2 == NULL) {
    returnValue = TEST_ABORTED;
    goto cleanup;
  }
  userdata2 = (char *)SDL_strdup(referenceUserdata2);
  if (userdata2 == NULL) {
    returnValue = TEST_ABORTED;
    goto cleanup;
  }

  /* Get non-existent data */
  result = (char *)SDL_GetWindowData(window, name);
  SDLTest_AssertPass("Call to SDL_GetWindowData(..,%s)", name);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);

  /* Set data */
  result = (char *)SDL_SetWindowData(window, name, userdata);
  SDLTest_AssertPass("Call to SDL_SetWindowData(...%s,%s)", name, userdata);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, userdata) == 0, "Validate that userdata was not changed, expected: %s, got: %s", referenceUserdata, userdata);

  /* Get data (twice) */
  for (iteration = 1; iteration <= 2; iteration++) {
    result = (char *)SDL_GetWindowData(window, name);
    SDLTest_AssertPass("Call to SDL_GetWindowData(..,%s) [iteration %d]", name, iteration);
    SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata, result);
    SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
  }

  /* Set data again twice */
  for (iteration = 1; iteration <= 2; iteration++) {
    result = (char *)SDL_SetWindowData(window, name, userdata);
    SDLTest_AssertPass("Call to SDL_SetWindowData(...%s,%s) [iteration %d]", name, userdata, iteration);
    SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata, result);
    SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
    SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, userdata) == 0, "Validate that userdata was not changed, expected: %s, got: %s", referenceUserdata, userdata);
  }

  /* Get data again */
  result = (char *)SDL_GetWindowData(window, name);
  SDLTest_AssertPass("Call to SDL_GetWindowData(..,%s) [again]", name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata, result);
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);

  /* Set data with new data */
  result = (char *)SDL_SetWindowData(window, name, userdata2);
  SDLTest_AssertPass("Call to SDL_SetWindowData(...%s,%s) [new userdata]", name, userdata2);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata, result);
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, userdata) == 0, "Validate that userdata was not changed, expected: %s, got: %s", referenceUserdata, userdata);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata2, userdata2) == 0, "Validate that userdata2 was not changed, expected: %s, got: %s", referenceUserdata2, userdata2);

  /* Set data with new data again */
  result = (char *)SDL_SetWindowData(window, name, userdata2);
  SDLTest_AssertPass("Call to SDL_SetWindowData(...%s,%s) [new userdata again]", name, userdata2);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata2, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata2, result);
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, userdata) == 0, "Validate that userdata was not changed, expected: %s, got: %s", referenceUserdata, userdata);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata2, userdata2) == 0, "Validate that userdata2 was not changed, expected: %s, got: %s", referenceUserdata2, userdata2);

  /* Get new data */
  result = (char *)SDL_GetWindowData(window, name);
  SDLTest_AssertPass("Call to SDL_GetWindowData(..,%s)", name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata2, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata2, result);
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);

  /* Set data with NULL to clear */
  result = (char *)SDL_SetWindowData(window, name, NULL);
  SDLTest_AssertPass("Call to SDL_SetWindowData(...%s,NULL)", name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata2, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata2, result);
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, userdata) == 0, "Validate that userdata was not changed, expected: %s, got: %s", referenceUserdata, userdata);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata2, userdata2) == 0, "Validate that userdata2 was not changed, expected: %s, got: %s", referenceUserdata2, userdata2);

  /* Set data with NULL to clear again */
  result = (char *)SDL_SetWindowData(window, name, NULL);
  SDLTest_AssertPass("Call to SDL_SetWindowData(...%s,NULL) [again]", name);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, userdata) == 0, "Validate that userdata was not changed, expected: %s, got: %s", referenceUserdata, userdata);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata2, userdata2) == 0, "Validate that userdata2 was not changed, expected: %s, got: %s", referenceUserdata2, userdata2);

  /* Get non-existent data */
  result = (char *)SDL_GetWindowData(window, name);
  SDLTest_AssertPass("Call to SDL_GetWindowData(..,%s)", name);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);

  /* Get non-existent data new name */
  result = (char *)SDL_GetWindowData(window, name2);
  SDLTest_AssertPass("Call to SDL_GetWindowData(..,%s)", name2);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  SDLTest_AssertCheck(SDL_strcmp(referenceName2, name2) == 0, "Validate that name2 was not changed, expected: %s, got: %s", referenceName2, name2);

  /* Set data (again) */
  result = (char *)SDL_SetWindowData(window, name, userdata);
  SDLTest_AssertPass("Call to SDL_SetWindowData(...%s,%s) [again, after clear]", name, userdata);
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, userdata) == 0, "Validate that userdata was not changed, expected: %s, got: %s", referenceUserdata, userdata);

  /* Get data (again) */
  result = (char *)SDL_GetWindowData(window, name);
  SDLTest_AssertPass("Call to SDL_GetWindowData(..,%s) [again, after clear]", name);
  SDLTest_AssertCheck(SDL_strcmp(referenceUserdata, result) == 0, "Validate that correct result was returned; expected: %s, got: %s", referenceUserdata, result);
  SDLTest_AssertCheck(SDL_strcmp(referenceName, name) == 0, "Validate that name was not changed, expected: %s, got: %s", referenceName, name);

  /* Negative test */
  SDL_ClearError();
  SDLTest_AssertPass("Call to SDL_ClearError()");

  /* Set with invalid window */
  result = (char *)SDL_SetWindowData(NULL, name, userdata);
  SDLTest_AssertPass("Call to SDL_SetWindowData(window=NULL)");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidWindowError();

  /* Set data with NULL name, valid userdata */
  result = (char *)SDL_SetWindowData(window, NULL, userdata);
  SDLTest_AssertPass("Call to SDL_SetWindowData(name=NULL)");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidParameterError();

  /* Set data with empty name, valid userdata */
  result = (char *)SDL_SetWindowData(window, "", userdata);
  SDLTest_AssertPass("Call to SDL_SetWindowData(name='')");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidParameterError();

  /* Set data with NULL name, NULL userdata */
  result = (char *)SDL_SetWindowData(window, NULL, NULL);
  SDLTest_AssertPass("Call to SDL_SetWindowData(name=NULL,userdata=NULL)");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidParameterError();

  /* Set data with empty name, NULL userdata */
  result = (char *)SDL_SetWindowData(window, "", NULL);
  SDLTest_AssertPass("Call to SDL_SetWindowData(name='',userdata=NULL)");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidParameterError();

  /* Get with invalid window */
  result = (char *)SDL_GetWindowData(NULL, name);
  SDLTest_AssertPass("Call to SDL_GetWindowData(window=NULL)");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidWindowError();

  /* Get data with NULL name */
  result = (char *)SDL_GetWindowData(window, NULL);
  SDLTest_AssertPass("Call to SDL_GetWindowData(name=NULL)");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidParameterError();

  /* Get data with empty name */
  result = (char *)SDL_GetWindowData(window, "");
  SDLTest_AssertPass("Call to SDL_GetWindowData(name='')");
  SDLTest_AssertCheck(result == NULL, "Validate that result is NULL");
  _checkInvalidParameterError();

  /* Clean up */
  _destroyVideoSuiteTestWindow(window);

  cleanup:
  SDL_free(referenceUserdata);
  SDL_free(referenceUserdata2);
  SDL_free(userdata);
  SDL_free(userdata2);

  return returnValue;
}


/* ================= Test References ================== */

/* Video test cases */
static const SDLTest_TestCaseReference videoTest1 =
        { (SDLTest_TestCaseFp)video_enableDisableScreensaver, "video_enableDisableScreensaver",  "Enable and disable screenaver while checking state", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest2 =
        { (SDLTest_TestCaseFp)video_createWindowVariousPositions, "video_createWindowVariousPositions",  "Create windows at various locations", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest3 =
        { (SDLTest_TestCaseFp)video_createWindowVariousSizes, "video_createWindowVariousSizes",  "Create windows with various sizes", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest4 =
        { (SDLTest_TestCaseFp)video_createWindowVariousFlags, "video_createWindowVariousFlags",  "Create windows using various flags", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest5 =
        { (SDLTest_TestCaseFp)video_getWindowFlags, "video_getWindowFlags",  "Get window flags set during SDL_CreateWindow", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest6 =
        { (SDLTest_TestCaseFp)video_getNumDisplayModes, "video_getNumDisplayModes",  "Use SDL_GetNumDisplayModes function to get number of display modes", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest7 =
        { (SDLTest_TestCaseFp)video_getNumDisplayModesNegative, "video_getNumDisplayModesNegative",  "Negative tests for SDL_GetNumDisplayModes", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest8 =
        { (SDLTest_TestCaseFp)video_getClosestDisplayModeCurrentResolution, "video_getClosestDisplayModeCurrentResolution",  "Use function to get closes match to requested display mode for current resolution", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest9 =
        { (SDLTest_TestCaseFp)video_getClosestDisplayModeRandomResolution, "video_getClosestDisplayModeRandomResolution",  "Use function to get closes match to requested display mode for random resolution", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest10 =
        { (SDLTest_TestCaseFp)video_getWindowBrightness, "video_getWindowBrightness",  "Get window brightness", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest11 =
        { (SDLTest_TestCaseFp)video_getWindowBrightnessNegative, "video_getWindowBrightnessNegative",  "Get window brightness with invalid input", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest12 =
        { (SDLTest_TestCaseFp)video_getWindowDisplayMode, "video_getWindowDisplayMode",  "Get window display mode", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest13 =
        { (SDLTest_TestCaseFp)video_getWindowDisplayModeNegative, "video_getWindowDisplayModeNegative",  "Get window display mode with invalid input", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest14 =
        { (SDLTest_TestCaseFp)video_getWindowGammaRamp, "video_getWindowGammaRamp",  "Get window gamma ramp", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest15 =
        { (SDLTest_TestCaseFp)video_getWindowGammaRampNegative, "video_getWindowGammaRampNegative",  "Get window gamma ramp against invalid input", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest16 =
        { (SDLTest_TestCaseFp)video_getSetWindowGrab, "video_getSetWindowGrab",  "Checks SDL_GetWindowGrab and SDL_SetWindowGrab positive and negative cases", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest17 =
        { (SDLTest_TestCaseFp)video_getWindowId, "video_getWindowId",  "Checks SDL_GetWindowID and SDL_GetWindowFromID", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest18 =
        { (SDLTest_TestCaseFp)video_getWindowPixelFormat, "video_getWindowPixelFormat",  "Checks SDL_GetWindowPixelFormat", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest19 =
        { (SDLTest_TestCaseFp)video_getSetWindowPosition, "video_getSetWindowPosition",  "Checks SDL_GetWindowPosition and SDL_SetWindowPosition positive and negative cases", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest20 =
        { (SDLTest_TestCaseFp)video_getSetWindowSize, "video_getSetWindowSize",  "Checks SDL_GetWindowSize and SDL_SetWindowSize positive and negative cases", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest21 =
        { (SDLTest_TestCaseFp)video_getSetWindowMinimumSize, "video_getSetWindowMinimumSize",  "Checks SDL_GetWindowMinimumSize and SDL_SetWindowMinimumSize positive and negative cases", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest22 =
        { (SDLTest_TestCaseFp)video_getSetWindowMaximumSize, "video_getSetWindowMaximumSize",  "Checks SDL_GetWindowMaximumSize and SDL_SetWindowMaximumSize positive and negative cases", TEST_ENABLED };

static const SDLTest_TestCaseReference videoTest23 =
        { (SDLTest_TestCaseFp)video_getSetWindowData, "video_getSetWindowData",  "Checks SDL_SetWindowData and SDL_GetWindowData positive and negative cases", TEST_ENABLED };

/* Sequence of Video test cases */
static const SDLTest_TestCaseReference *videoTests[] =  {
    &videoTest1, &videoTest2, &videoTest3, &videoTest4, &videoTest5, &videoTest6,
    &videoTest7, &videoTest8, &videoTest9, &videoTest10, &videoTest11, &videoTest12,
    &videoTest13, &videoTest14, &videoTest15, &videoTest16, &videoTest17,
    &videoTest18, &videoTest19, &videoTest20, &videoTest21, &videoTest22,
    &videoTest23, NULL
};

/* Video test suite (global) */
SDLTest_TestSuiteReference videoTestSuite = {
    "Video",
    NULL,
    videoTests,
    NULL
};
