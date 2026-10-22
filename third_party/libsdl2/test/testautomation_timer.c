/**
 * Timer test suite
 */

#include <stdio.h>

#include "SDL.h"
#include "SDL_test.h"

/* Flag indicating if the param should be checked */
int _paramCheck = 0;

/* Userdata value to check */
int _paramValue = 0;

/* Flag indicating that the callback was called */
int _timerCallbackCalled = 0;

/* Fixture */

void
_timerSetUp(void *arg)
{
    /* Start SDL timer subsystem */
    int ret = SDL_InitSubSystem( SDL_INIT_TIMER );
        SDLTest_AssertPass("Call to SDL_InitSubSystem(SDL_INIT_TIMER)");
    SDLTest_AssertCheck(ret==0, "Check result from SDL_InitSubSystem(SDL_INIT_TIMER)");
    if (ret != 0) {
           SDLTest_LogError("%s", SDL_GetError());
        }
}

/* Test case functions */

/**
 * @brief Call to SDL_GetPerformanceCounter
 */
int
timer_getPerformanceCounter(void *arg)
{
  Uint64 result;

  result = SDL_GetPerformanceCounter();
  SDLTest_AssertPass("Call to SDL_GetPerformanceCounter()");
  SDLTest_AssertCheck(result > 0, "Check result value, expected: >0, got: %"SDL_PRIu64, result);

  return TEST_COMPLETED;
}

/**
 * @brief Call to SDL_GetPerformanceFrequency
 */
int
timer_getPerformanceFrequency(void *arg)
{
  Uint64 result;

  result = SDL_GetPerformanceFrequency();
  SDLTest_AssertPass("Call to SDL_GetPerformanceFrequency()");
  SDLTest_AssertCheck(result > 0, "Check result value, expected: >0, got: %"SDL_PRIu64, result);

  return TEST_COMPLETED;
}

/**
 * @brief Call to SDL_Delay and SDL_GetTicks
 */
int
timer_delayAndGetTicks(void *arg)
{
  const Uint32 testDelay = 100;
  const Uint32 marginOfError = 25;
  Uint32 result;
  Uint32 result2;
  Uint32 difference;

  /* Zero delay */
  SDL_Delay(0);
  SDLTest_AssertPass("Call to SDL_Delay(0)");

  /* Non-zero delay */
  SDL_Delay(1);
  SDLTest_AssertPass("Call to SDL_Delay(1)");

  SDL_Delay(SDLTest_RandomIntegerInRange(5, 15));
  SDLTest_AssertPass("Call to SDL_Delay()");

  /* Get ticks count - should be non-zero by now */
  result = SDL_GetTicks();
  SDLTest_AssertPass("Call to SDL_GetTicks()");
  SDLTest_AssertCheck(result > 0, "Check result value, expected: >0, got: %d", result);

  /* Delay a bit longer and measure ticks and verify difference */
  SDL_Delay(testDelay);
  SDLTest_AssertPass("Call to SDL_Delay(%d)", testDelay);
  result2 = SDL_GetTicks();
  SDLTest_AssertPass("Call to SDL_GetTicks()");
  SDLTest_AssertCheck(result2 > 0, "Check result value, expected: >0, got: %d", result2);
  difference = result2 - result;
  SDLTest_AssertCheck(difference > (testDelay - marginOfError), "Check difference, expected: >%d, got: %d", testDelay - marginOfError, difference);
  SDLTest_AssertCheck(difference < (testDelay + marginOfError), "Check difference, expected: <%d, got: %d", testDelay + marginOfError, difference);

  return TEST_COMPLETED;
}

/* Test callback */
Uint32 SDLCALL _timerTestCallback(Uint32 interval, void *param)
{
   _timerCallbackCalled = 1;

   if (_paramCheck != 0) {
       SDLTest_AssertCheck(param != NULL, "Check param pointer, expected: non-NULL, got: %s", (param != NULL) ? "non-NULL" : "NULL");
       if (param != NULL) {
          SDLTest_AssertCheck(*(int *)param == _paramValue, "Check param value, expected: %i, got: %i", _paramValue, *(int *)param);
       }
   }

   return 0;
}

/**
 * @brief Call to SDL_AddTimer and SDL_RemoveTimer
 */
int
timer_addRemoveTimer(void *arg)
{
  SDL_TimerID id;
  SDL_bool result;
  int param;

  /* Reset state */
  _paramCheck = 0;
  _timerCallbackCalled = 0;

  /* Set timer with a long delay */
  id = SDL_AddTimer(10000, _timerTestCallback, NULL);
  SDLTest_AssertPass("Call to SDL_AddTimer(10000,...)");
  SDLTest_AssertCheck(id > 0, "Check result value, expected: >0, got: %d", id);

  /* Remove timer again and check that callback was not called */
  result = SDL_RemoveTimer(id);
  SDLTest_AssertPass("Call to SDL_RemoveTimer()");
  SDLTest_AssertCheck(result == SDL_TRUE, "Check result value, expected: %i, got: %i", SDL_TRUE, result);
  SDLTest_AssertCheck(_timerCallbackCalled == 0, "Check callback WAS NOT called, expected: 0, got: %i", _timerCallbackCalled);

  /* Try to remove timer again (should be a NOOP) */
  result = SDL_RemoveTimer(id);
  SDLTest_AssertPass("Call to SDL_RemoveTimer()");
  SDLTest_AssertCheck(result == SDL_FALSE, "Check result value, expected: %i, got: %i", SDL_FALSE, result);

  /* Reset state */
  param = SDLTest_RandomIntegerInRange(-1024, 1024);
  _paramCheck = 1;
  _paramValue = param;
  _timerCallbackCalled = 0;

  /* Set timer with a short delay */
  id = SDL_AddTimer(10, _timerTestCallback, (void *)&param);
  SDLTest_AssertPass("Call to SDL_AddTimer(10, param)");
  SDLTest_AssertCheck(id > 0, "Check result value, expected: >0, got: %d", id);

  /* Wait to let timer trigger callback */
  SDL_Delay(100);
  SDLTest_AssertPass("Call to SDL_Delay(100)");

  /* Remove timer again and check that callback was called */
  result = SDL_RemoveTimer(id);
  SDLTest_AssertPass("Call to SDL_RemoveTimer()");
  SDLTest_AssertCheck(result == SDL_FALSE, "Check result value, expected: %i, got: %i", SDL_FALSE, result);
  SDLTest_AssertCheck(_timerCallbackCalled == 1, "Check callback WAS called, expected: 1, got: %i", _timerCallbackCalled);

  return TEST_COMPLETED;
}

/* ================= Test References ================== */

/* Timer test cases */
static const SDLTest_TestCaseReference timerTest1 =
        { (SDLTest_TestCaseFp)timer_getPerformanceCounter, "timer_getPerformanceCounter", "Call to SDL_GetPerformanceCounter", TEST_ENABLED };

static const SDLTest_TestCaseReference timerTest2 =
        { (SDLTest_TestCaseFp)timer_getPerformanceFrequency, "timer_getPerformanceFrequency", "Call to SDL_GetPerformanceFrequency", TEST_ENABLED };

static const SDLTest_TestCaseReference timerTest3 =
        { (SDLTest_TestCaseFp)timer_delayAndGetTicks, "timer_delayAndGetTicks", "Call to SDL_Delay and SDL_GetTicks", TEST_ENABLED };

static const SDLTest_TestCaseReference timerTest4 =
        { (SDLTest_TestCaseFp)timer_addRemoveTimer, "timer_addRemoveTimer", "Call to SDL_AddTimer and SDL_RemoveTimer", TEST_ENABLED };

/* Sequence of Timer test cases */
static const SDLTest_TestCaseReference *timerTests[] =  {
    &timerTest1, &timerTest2, &timerTest3, &timerTest4, NULL
};

/* Timer test suite (global) */
SDLTest_TestSuiteReference timerTestSuite = {
    "Timer",
    _timerSetUp,
    timerTests,
    NULL
};
