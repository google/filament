/**
 * SysWM test suite
 */

#include <stdio.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_test.h"

/* Test case functions */

/**
 * @brief Call to SDL_GetWindowWMInfo
 */
int
syswm_getWindowWMInfo(void *arg)
{
  SDL_bool result;
  SDL_Window *window;
  SDL_SysWMinfo info;

  window = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_HIDDEN);
  SDLTest_AssertPass("Call to SDL_CreateWindow()");
  SDLTest_AssertCheck(window != NULL, "Check that value returned from SDL_CreateWindow is not NULL");
  if (window == NULL) {
     return TEST_ABORTED;
  }

  /* Initialize info structure with SDL version info */
  SDL_VERSION(&info.version);

  /* Make call */
  result = SDL_GetWindowWMInfo(window, &info);
  SDLTest_AssertPass("Call to SDL_GetWindowWMInfo()");
  SDLTest_Log((result == SDL_TRUE) ? "Got window information" : "Couldn't get window information");

  SDL_DestroyWindow(window);
  SDLTest_AssertPass("Call to SDL_DestroyWindow()");

  return TEST_COMPLETED;
}

/* ================= Test References ================== */

/* SysWM test cases */
static const SDLTest_TestCaseReference syswmTest1 =
        { (SDLTest_TestCaseFp)syswm_getWindowWMInfo, "syswm_getWindowWMInfo", "Call to SDL_GetWindowWMInfo", TEST_ENABLED };

/* Sequence of SysWM test cases */
static const SDLTest_TestCaseReference *syswmTests[] =  {
    &syswmTest1, NULL
};

/* SysWM test suite (global) */
SDLTest_TestSuiteReference syswmTestSuite = {
    "SysWM",
    NULL,
    syswmTests,
    NULL
};
