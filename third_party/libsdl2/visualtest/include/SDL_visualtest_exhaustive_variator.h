/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_exhaustive_variator.h
 *
 * Header for the exhaustive variator.
 */

#include "SDL_visualtest_harness_argparser.h"
#include "SDL_visualtest_variator_common.h"

#ifndef SDL_visualtest_exhaustive_variator_h_
#define SDL_visualtest_exhaustive_variator_h_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct for the variator that exhaustively iterates through all variations of
 * command line arguments to the SUT.
 */
typedef struct SDLVisualTest_ExhaustiveVariator
{
    /*! The current variation. */
    SDLVisualTest_Variation variation;
    /*! Configuration object for the SUT that the variator is running for. */
    SDLVisualTest_SUTConfig config;
    /*! Buffer to store the arguments string built from the variation */
    char buffer[MAX_SUT_ARGS_LEN];
} SDLVisualTest_ExhaustiveVariator;

/**
 * Initializes the variator.
 *
 * \return 1 on success, 0 on failure
 */
int SDLVisualTest_InitExhaustiveVariator(SDLVisualTest_ExhaustiveVariator* variator,
                                         SDLVisualTest_SUTConfig* config);

/**
 * Gets the arguments string for the next variation using the variator and updates
 * the variator's current variation object to the next variation.
 *
 * \return The arguments string representing the next variation on success, and
 *         NULL on failure or if we have iterated through all possible variations.
 *         In the latter case subsequent calls will start the variations again from
 *         the very beginning. The pointer returned should not be freed.
 */
char* SDLVisualTest_GetNextExhaustiveVariation(SDLVisualTest_ExhaustiveVariator* variator);

/**
 * Frees any resources associated with the variator.
 */
void SDLVisualTest_FreeExhaustiveVariator(SDLVisualTest_ExhaustiveVariator* variator);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_exhaustive_variator_h_ */

/* vi: set ts=4 sw=4 expandtab: */
