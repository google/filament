/* See LICENSE.txt for the full license governing this code. */
/**
 * \file variators.c
 *
 * Source file for the operations that act on variators.
 */

#include <SDL_test.h>
#include "SDL_visualtest_variators.h"

int
SDLVisualTest_InitVariator(SDLVisualTest_Variator* variator,
                           SDLVisualTest_SUTConfig* config,
                           SDLVisualTest_VariatorType type,
                           Uint64 seed)
{
    if(!variator)
    {
        SDLTest_LogError("variator argument cannot be NULL");
        return 0;
    }
    if(!config)
    {
        SDLTest_LogError("config argument cannot be NULL");
        return 0;
    }

    variator->type = type;
    switch(type)
    {
        case SDL_VARIATOR_EXHAUSTIVE:
            return SDLVisualTest_InitExhaustiveVariator(&variator->data.exhaustive,
                                                        config);
        break;

        case SDL_VARIATOR_RANDOM:
            return SDLVisualTest_InitRandomVariator(&variator->data.random,
                                                    config, seed);
        break;

        default:
            SDLTest_LogError("Invalid value for variator type");
            return 0;
    }
    return 0;
}

char*
SDLVisualTest_GetNextVariation(SDLVisualTest_Variator* variator)
{
    if(!variator)
    {
        SDLTest_LogError("variator argument cannot be NULL");
        return NULL;
    }
    switch(variator->type)
    {
        case SDL_VARIATOR_EXHAUSTIVE:
            return SDLVisualTest_GetNextExhaustiveVariation(&variator->data.exhaustive);
        break;

        case SDL_VARIATOR_RANDOM:
            return SDLVisualTest_GetNextRandomVariation(&variator->data.random);
        break;

        default:
            SDLTest_LogError("Invalid value for variator type");
            return NULL;
    }
    return NULL;
}

void SDLVisualTest_FreeVariator(SDLVisualTest_Variator* variator)
{
    if(!variator)
    {
        SDLTest_LogError("variator argument cannot be NULL");
        return;
    }
    switch(variator->type)
    {
        case SDL_VARIATOR_EXHAUSTIVE:
            SDLVisualTest_FreeExhaustiveVariator(&variator->data.exhaustive);
        break;

        case SDL_VARIATOR_RANDOM:
            SDLVisualTest_FreeRandomVariator(&variator->data.random);
        break;

        default:
            SDLTest_LogError("Invalid value for variator type");
    }
}