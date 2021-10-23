/* See LICENSE.txt for the full license governing this code. */
/**
 * \file SDL_visualtest_variators.h
 *
 * Header for all the variators that vary input parameters to a SUT application.
 */

#include "SDL_visualtest_exhaustive_variator.h"
#include "SDL_visualtest_random_variator.h"

#ifndef SDL_visualtest_variators_h_
#define SDL_visualtest_variators_h_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct that acts like a wrapper around the different types of variators
 * available.
 */
typedef struct SDLVisualTest_Variator
{
    /*! Type of the variator */
    SDLVisualTest_VariatorType type;
    /*! union object that stores the variator */
    union
    {
        SDLVisualTest_ExhaustiveVariator exhaustive;
        SDLVisualTest_RandomVariator random;
    } data;
} SDLVisualTest_Variator;

/**
 * Initializes the variator object pointed to by \c variator of type \c type
 * with information from the config object pointed to by \c config.
 *
 * \return 1 on success, 0 on failure
 */
int SDLVisualTest_InitVariator(SDLVisualTest_Variator* variator,
                               SDLVisualTest_SUTConfig* config,
                               SDLVisualTest_VariatorType type,
                               Uint64 seed);

/**
 * Gets the next variation using the variator.
 *
 * \return The arguments string representing the variation on success, and
 *         NULL on failure. The pointer returned should not be freed.
 */
char* SDLVisualTest_GetNextVariation(SDLVisualTest_Variator* variator);

/**
 * Frees any resources associated with the variator.
 */
void SDLVisualTest_FreeVariator(SDLVisualTest_Variator* variator);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_variators_h_ */

/* vi: set ts=4 sw=4 expandtab: */
