/**
 * Reference to all test suites.
 *
 */

#ifndef _testsuites_h
#define _testsuites_h

#include "SDL_test.h"

/* Test collections */
extern SDLTest_TestSuiteReference audioTestSuite;
extern SDLTest_TestSuiteReference clipboardTestSuite;
extern SDLTest_TestSuiteReference eventsTestSuite;
extern SDLTest_TestSuiteReference keyboardTestSuite;
extern SDLTest_TestSuiteReference mainTestSuite;
extern SDLTest_TestSuiteReference mouseTestSuite;
extern SDLTest_TestSuiteReference pixelsTestSuite;
extern SDLTest_TestSuiteReference platformTestSuite;
extern SDLTest_TestSuiteReference rectTestSuite;
extern SDLTest_TestSuiteReference renderTestSuite;
extern SDLTest_TestSuiteReference rwopsTestSuite;
extern SDLTest_TestSuiteReference sdltestTestSuite;
extern SDLTest_TestSuiteReference stdlibTestSuite;
extern SDLTest_TestSuiteReference surfaceTestSuite;
extern SDLTest_TestSuiteReference syswmTestSuite;
extern SDLTest_TestSuiteReference timerTestSuite;
extern SDLTest_TestSuiteReference videoTestSuite;
extern SDLTest_TestSuiteReference hintsTestSuite;

/* All test suites */
SDLTest_TestSuiteReference *testSuites[] =  {
    &audioTestSuite,
    &clipboardTestSuite,
    &eventsTestSuite,
    &keyboardTestSuite,
    &mainTestSuite,
    &mouseTestSuite,
    &pixelsTestSuite,
    &platformTestSuite,
    &rectTestSuite,
    &renderTestSuite,
    &rwopsTestSuite,
    &sdltestTestSuite,
    &stdlibTestSuite,
    &surfaceTestSuite,
    &syswmTestSuite,
    &timerTestSuite,
    &videoTestSuite,
    &hintsTestSuite,
    NULL
};

#endif
