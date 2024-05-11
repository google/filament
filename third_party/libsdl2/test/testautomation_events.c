/**
 * Events test suite
 */

#include <stdio.h>

#include "SDL.h"
#include "SDL_test.h"

/* ================= Test Case Implementation ================== */

/* Test case functions */

/* Flag indicating if the userdata should be checked */
int _userdataCheck = 0;

/* Userdata value to check */
int _userdataValue = 0;

/* Flag indicating that the filter was called */
int _eventFilterCalled = 0;

/* Userdata values for event */
int _userdataValue1 = 1;
int _userdataValue2 = 2;

/* Event filter that sets some flags and optionally checks userdata */
int SDLCALL _events_sampleNullEventFilter(void *userdata, SDL_Event *event)
{
   _eventFilterCalled = 1;

   if (_userdataCheck != 0) {
       SDLTest_AssertCheck(userdata != NULL, "Check userdata pointer, expected: non-NULL, got: %s", (userdata != NULL) ? "non-NULL" : "NULL");
       if (userdata != NULL) {
          SDLTest_AssertCheck(*(int *)userdata == _userdataValue, "Check userdata value, expected: %i, got: %i", _userdataValue, *(int *)userdata);
       }
   }

   return 0;
}

/**
 * @brief Test pumping and peeking events.
 *
 * @sa http://wiki.libsdl.org/moin.cgi/SDL_PumpEvents
 * @sa http://wiki.libsdl.org/moin.cgi/SDL_PollEvent
 */
int
events_pushPumpAndPollUserevent(void *arg)
{
   SDL_Event event1;
   SDL_Event event2;
   int result;

   /* Create user event */
   event1.type = SDL_USEREVENT;
   event1.user.code = SDLTest_RandomSint32();
   event1.user.data1 = (void *)&_userdataValue1;
   event1.user.data2 = (void *)&_userdataValue2;

   /* Push a user event onto the queue and force queue update */
   SDL_PushEvent(&event1);
   SDLTest_AssertPass("Call to SDL_PushEvent()");
   SDL_PumpEvents();
   SDLTest_AssertPass("Call to SDL_PumpEvents()");

   /* Poll for user event */
   result = SDL_PollEvent(&event2);
   SDLTest_AssertPass("Call to SDL_PollEvent()");
   SDLTest_AssertCheck(result == 1, "Check result from SDL_PollEvent, expected: 1, got: %d", result);

   return TEST_COMPLETED;
}


/**
 * @brief Adds and deletes an event watch function with NULL userdata
 *
 * @sa http://wiki.libsdl.org/moin.cgi/SDL_AddEventWatch
 * @sa http://wiki.libsdl.org/moin.cgi/SDL_DelEventWatch
 *
 */
int
events_addDelEventWatch(void *arg)
{
   SDL_Event event;

   /* Create user event */
   event.type = SDL_USEREVENT;
   event.user.code = SDLTest_RandomSint32();
   event.user.data1 = (void *)&_userdataValue1;
   event.user.data2 = (void *)&_userdataValue2;

   /* Disable userdata check */
   _userdataCheck = 0;

   /* Reset event filter call tracker */
   _eventFilterCalled = 0;

   /* Add watch */
   SDL_AddEventWatch(_events_sampleNullEventFilter, NULL);
   SDLTest_AssertPass("Call to SDL_AddEventWatch()");

   /* Push a user event onto the queue and force queue update */
   SDL_PushEvent(&event);
   SDLTest_AssertPass("Call to SDL_PushEvent()");
   SDL_PumpEvents();
   SDLTest_AssertPass("Call to SDL_PumpEvents()");
   SDLTest_AssertCheck(_eventFilterCalled == 1, "Check that event filter was called");

   /* Delete watch */
   SDL_DelEventWatch(_events_sampleNullEventFilter, NULL);
   SDLTest_AssertPass("Call to SDL_DelEventWatch()");

   /* Push a user event onto the queue and force queue update */
   _eventFilterCalled = 0;
   SDL_PushEvent(&event);
   SDLTest_AssertPass("Call to SDL_PushEvent()");
   SDL_PumpEvents();
   SDLTest_AssertPass("Call to SDL_PumpEvents()");
   SDLTest_AssertCheck(_eventFilterCalled == 0, "Check that event filter was NOT called");

   return TEST_COMPLETED;
}

/**
 * @brief Adds and deletes an event watch function with userdata
 *
 * @sa http://wiki.libsdl.org/moin.cgi/SDL_AddEventWatch
 * @sa http://wiki.libsdl.org/moin.cgi/SDL_DelEventWatch
 *
 */
int
events_addDelEventWatchWithUserdata(void *arg)
{
   SDL_Event event;

   /* Create user event */
   event.type = SDL_USEREVENT;
   event.user.code = SDLTest_RandomSint32();
   event.user.data1 = (void *)&_userdataValue1;
   event.user.data2 = (void *)&_userdataValue2;

   /* Enable userdata check and set a value to check */
   _userdataCheck = 1;
   _userdataValue = SDLTest_RandomIntegerInRange(-1024, 1024);

   /* Reset event filter call tracker */
   _eventFilterCalled = 0;

   /* Add watch */
   SDL_AddEventWatch(_events_sampleNullEventFilter, (void *)&_userdataValue);
   SDLTest_AssertPass("Call to SDL_AddEventWatch()");

   /* Push a user event onto the queue and force queue update */
   SDL_PushEvent(&event);
   SDLTest_AssertPass("Call to SDL_PushEvent()");
   SDL_PumpEvents();
   SDLTest_AssertPass("Call to SDL_PumpEvents()");
   SDLTest_AssertCheck(_eventFilterCalled == 1, "Check that event filter was called");

   /* Delete watch */
   SDL_DelEventWatch(_events_sampleNullEventFilter, (void *)&_userdataValue);
   SDLTest_AssertPass("Call to SDL_DelEventWatch()");

   /* Push a user event onto the queue and force queue update */
   _eventFilterCalled = 0;
   SDL_PushEvent(&event);
   SDLTest_AssertPass("Call to SDL_PushEvent()");
   SDL_PumpEvents();
   SDLTest_AssertPass("Call to SDL_PumpEvents()");
   SDLTest_AssertCheck(_eventFilterCalled == 0, "Check that event filter was NOT called");

   return TEST_COMPLETED;
}


/* ================= Test References ================== */

/* Events test cases */
static const SDLTest_TestCaseReference eventsTest1 =
        { (SDLTest_TestCaseFp)events_pushPumpAndPollUserevent, "events_pushPumpAndPollUserevent", "Pushes, pumps and polls a user event", TEST_ENABLED };

static const SDLTest_TestCaseReference eventsTest2 =
        { (SDLTest_TestCaseFp)events_addDelEventWatch, "events_addDelEventWatch", "Adds and deletes an event watch function with NULL userdata", TEST_ENABLED };

static const SDLTest_TestCaseReference eventsTest3 =
        { (SDLTest_TestCaseFp)events_addDelEventWatchWithUserdata, "events_addDelEventWatchWithUserdata", "Adds and deletes an event watch function with userdata", TEST_ENABLED };

/* Sequence of Events test cases */
static const SDLTest_TestCaseReference *eventsTests[] =  {
    &eventsTest1, &eventsTest2, &eventsTest3, NULL
};

/* Events test suite (global) */
SDLTest_TestSuiteReference eventsTestSuite = {
    "Events",
    NULL,
    eventsTests,
    NULL
};
