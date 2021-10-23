/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_screenshot.h
 *
 * Header for the screenshot API.
 */

#include "SDL_visualtest_process.h"

#ifndef SDL_visualtest_screenshot_h_
#define SDL_visualtest_screenshot_h_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Takes a screenshot of each window owned by the process \c pinfo and saves 
 * it in a file \c prefix-i.png where \c prefix is the full path to the file 
 * along with a prefix given to each screenshot.
 *
 * \return 1 on success, 0 on failure.
 */
int SDLVisualTest_ScreenshotProcess(SDL_ProcessInfo* pinfo, char* prefix);

/**
 * Takes a screenshot of the desktop and saves it into the file with path
 * \c filename.
 *
 * \return 1 on success, 0 on failure.
 */
int SDLVisualTest_ScreenshotDesktop(char* filename);

/**
 * Compare a screenshot taken previously with SUT arguments \c args that is
 * located in \c test_dir with a verification image that is located in 
 * \c verify_dir.
 *
 * \return -1 on failure, 0 if the images were not equal, 1 if the images are equal
 *         and 2 if the verification image is not present.
 */
int SDLVisualTest_VerifyScreenshots(char* args, char* test_dir, char* verify_dir);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_screenshot_h_ */

/* vi: set ts=4 sw=4 expandtab: */
