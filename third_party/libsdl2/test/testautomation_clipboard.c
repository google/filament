/**
 * New/updated tests: aschiffler at ferzkopp dot net
 */

#include <stdio.h>
#include <string.h>

#include "SDL.h"
#include "SDL_test.h"

/* ================= Test Case Implementation ================== */

/* Test case functions */

/**
 * \brief Check call to SDL_HasClipboardText
 *
 * \sa
 * http://wiki.libsdl.org/SDL_HasClipboardText
 */
int
clipboard_testHasClipboardText(void *arg)
{
    SDL_HasClipboardText();
    SDLTest_AssertPass("Call to SDL_HasClipboardText succeeded");

    return TEST_COMPLETED;
}

/**
 * \brief Check call to SDL_GetClipboardText
 *
 * \sa
 * http://wiki.libsdl.org/SDL_GetClipboardText
 */
int
clipboard_testGetClipboardText(void *arg)
{
    char *charResult;
    charResult = SDL_GetClipboardText();
    SDLTest_AssertPass("Call to SDL_GetClipboardText succeeded");

    SDL_free(charResult);

    return TEST_COMPLETED;
}

/**
 * \brief Check call to SDL_SetClipboardText
 * \sa
 * http://wiki.libsdl.org/SDL_SetClipboardText
 */
int
clipboard_testSetClipboardText(void *arg)
{
    char *textRef = SDLTest_RandomAsciiString();
    char *text = SDL_strdup(textRef);
    int result;
    result = SDL_SetClipboardText((const char *)text);
    SDLTest_AssertPass("Call to SDL_SetClipboardText succeeded");
    SDLTest_AssertCheck(
        result == 0,
        "Validate SDL_SetClipboardText result, expected 0, got %i",
        result);
    SDLTest_AssertCheck(
        SDL_strcmp(textRef, text) == 0,
        "Verify SDL_SetClipboardText did not modify input string, expected '%s', got '%s'",
        textRef, text);

    /* Cleanup */
    SDL_free(textRef);
    SDL_free(text);

   return TEST_COMPLETED;
}

/**
 * \brief End-to-end test of SDL_xyzClipboardText functions
 * \sa
 * http://wiki.libsdl.org/SDL_HasClipboardText
 * http://wiki.libsdl.org/SDL_GetClipboardText
 * http://wiki.libsdl.org/SDL_SetClipboardText
 */
int
clipboard_testClipboardTextFunctions(void *arg)
{
    char *textRef = SDLTest_RandomAsciiString();
    char *text = SDL_strdup(textRef);
    SDL_bool boolResult;
    int intResult;
    char *charResult;

    /* Clear clipboard text state */
    boolResult = SDL_HasClipboardText();
    SDLTest_AssertPass("Call to SDL_HasClipboardText succeeded");
    if (boolResult == SDL_TRUE) {
        intResult = SDL_SetClipboardText((const char *)NULL);
        SDLTest_AssertPass("Call to SDL_SetClipboardText(NULL) succeeded");
        SDLTest_AssertCheck(
            intResult == 0,
            "Verify result from SDL_SetClipboardText(NULL), expected 0, got %i",
            intResult);
        charResult = SDL_GetClipboardText();
        SDLTest_AssertPass("Call to SDL_GetClipboardText succeeded");
        SDL_free(charResult);
        boolResult = SDL_HasClipboardText();
        SDLTest_AssertPass("Call to SDL_HasClipboardText succeeded");
        SDLTest_AssertCheck(
            boolResult == SDL_FALSE,
            "Verify SDL_HasClipboardText returned SDL_FALSE, got %s",
            (boolResult) ? "SDL_TRUE" : "SDL_FALSE");
    }

    /* Empty clipboard  */
    charResult = SDL_GetClipboardText();
    SDLTest_AssertPass("Call to SDL_GetClipboardText succeeded");
    SDLTest_AssertCheck(
        charResult != NULL,
        "Verify SDL_GetClipboardText did not return NULL");
    SDLTest_AssertCheck(
        charResult[0] == '\0',
        "Verify SDL_GetClipboardText returned string with length 0, got length %i",
        (int) SDL_strlen(charResult));
    intResult = SDL_SetClipboardText((const char *)text);
    SDLTest_AssertPass("Call to SDL_SetClipboardText succeeded");
    SDLTest_AssertCheck(
        intResult == 0,
        "Verify result from SDL_SetClipboardText(NULL), expected 0, got %i",
        intResult);
    SDLTest_AssertCheck(
        SDL_strcmp(textRef, text) == 0,
        "Verify SDL_SetClipboardText did not modify input string, expected '%s', got '%s'",
        textRef, text);
    boolResult = SDL_HasClipboardText();
    SDLTest_AssertPass("Call to SDL_HasClipboardText succeeded");
    SDLTest_AssertCheck(
        boolResult == SDL_TRUE,
        "Verify SDL_HasClipboardText returned SDL_TRUE, got %s",
        (boolResult) ? "SDL_TRUE" : "SDL_FALSE");
    SDL_free(charResult);
    charResult = SDL_GetClipboardText();
    SDLTest_AssertPass("Call to SDL_GetClipboardText succeeded");
    SDLTest_AssertCheck(
        SDL_strcmp(textRef, charResult) == 0,
        "Verify SDL_GetClipboardText returned correct string, expected '%s', got '%s'",
        textRef, charResult);

    /* Cleanup */
    SDL_free(textRef);
    SDL_free(text);
    SDL_free(charResult);

   return TEST_COMPLETED;
}


/* ================= Test References ================== */

/* Clipboard test cases */
static const SDLTest_TestCaseReference clipboardTest1 =
        { (SDLTest_TestCaseFp)clipboard_testHasClipboardText, "clipboard_testHasClipboardText", "Check call to SDL_HasClipboardText", TEST_ENABLED };

static const SDLTest_TestCaseReference clipboardTest2 =
        { (SDLTest_TestCaseFp)clipboard_testGetClipboardText, "clipboard_testGetClipboardText", "Check call to SDL_GetClipboardText", TEST_ENABLED };

static const SDLTest_TestCaseReference clipboardTest3 =
        { (SDLTest_TestCaseFp)clipboard_testSetClipboardText, "clipboard_testSetClipboardText", "Check call to SDL_SetClipboardText", TEST_ENABLED };

static const SDLTest_TestCaseReference clipboardTest4 =
        { (SDLTest_TestCaseFp)clipboard_testClipboardTextFunctions, "clipboard_testClipboardTextFunctions", "End-to-end test of SDL_xyzClipboardText functions", TEST_ENABLED };

/* Sequence of Clipboard test cases */
static const SDLTest_TestCaseReference *clipboardTests[] =  {
    &clipboardTest1, &clipboardTest2, &clipboardTest3, &clipboardTest4, NULL
};

/* Clipboard test suite (global) */
SDLTest_TestSuiteReference clipboardTestSuite = {
    "Clipboard",
    NULL,
    clipboardTests,
    NULL
};
