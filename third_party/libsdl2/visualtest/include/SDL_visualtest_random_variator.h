/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_random_variator.h
 *
 * Header for the random variator.
 */

#include "SDL_visualtest_harness_argparser.h"
#include "SDL_visualtest_variator_common.h"

#ifndef SDL_visualtest_random_variator_h_
#define SDL_visualtest_random_variator_h_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct for the variator that randomly generates variations of command line
 * arguments to the SUT.
 */
typedef struct SDLVisualTest_RandomVariator
{
    /*! The current variation. */
    SDLVisualTest_Variation variation;
    /*! Configuration object for the SUT that the variator is running for. */
    SDLVisualTest_SUTConfig config;
    /*! Buffer to store the arguments string built from the variation */
    char buffer[MAX_SUT_ARGS_LEN];
} SDLVisualTest_RandomVariator;

/**
 * Initializes the variator.
 *
 * \return 1 on success, 0 on failure
 */
int SDLVisualTest_InitRandomVariator(SDLVisualTest_RandomVariator* variator,
                                     SDLVisualTest_SUTConfig* config, Uint64 seed);

/**
 * Generates a new random variation.
 *
 * \return The arguments string representing the random variation on success, and
 *         NULL on failure. The pointer returned should not be freed.
 */
char* SDLVisualTest_GetNextRandomVariation(SDLVisualTest_RandomVariator* variator);

/**
 * Frees any resources associated with the variator.
 */
void SDLVisualTest_FreeRandomVariator(SDLVisualTest_RandomVariator* variator);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_random_variator_h_ */

/* vi: set ts=4 sw=4 expandtab: */
