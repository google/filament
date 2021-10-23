/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_parsehelper.h
 *
 * Header with some helper functions for parsing strings.
 */

#ifndef SDL_visualtest_parsehelper_h_
#define SDL_visualtest_parsehelper_h_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Takes an string of command line arguments and breaks them up into an array
 * based on whitespace.
 *
 * \param args The string of arguments.
 *
 * \return NULL on failure, an array of strings on success. The last element
 *         of the array is NULL. The first element of the array is NULL and should
 *         be set to the path of the executable by the caller.
 */
char** SDLVisualTest_ParseArgsToArgv(char* args);

/**
 * Takes a string and breaks it into tokens by splitting on whitespace.
 *
 * \param str The string to be split.
 * \param max_token_len Length of each element in the array to be returned.
 *
 * \return NULL on failure; an array of strings with the tokens on success. The
 *         last element of the array is NULL.
 */
char** SDLVisualTest_Tokenize(char* str, int max_token_len);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_parsehelper_h_ */

/* vi: set ts=4 sw=4 expandtab: */
