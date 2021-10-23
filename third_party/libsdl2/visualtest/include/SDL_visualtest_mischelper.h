/**
 * \file mischelper.c 
 *
 * Header with miscellaneous helper functions.
 */

#ifndef SDL_visualtest_mischelper_h_
#define SDL_visualtest_mischelper_h_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Stores a 32 digit hexadecimal string representing the MD5 hash of the
 * string \c str in \c hash.
 */
void SDLVisualTest_HashString(char* str, char hash[33]);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_mischelper_h_ */

/* vi: set ts=4 sw=4 expandtab: */
