/* See LICENSE.txt for the full license governing this code. */
/**
 * \file variator_random.c
 *
 * Source file for the variator that tests the SUT with random variations to the
 * input parameters.
 */

#include <time.h>
#include <SDL_test.h>
#include "SDL_visualtest_random_variator.h"

int
SDLVisualTest_InitRandomVariator(SDLVisualTest_RandomVariator* variator,
                                 SDLVisualTest_SUTConfig* config, Uint64 seed)
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

    if(seed)
        SDLTest_FuzzerInit(seed);
    else
        SDLTest_FuzzerInit(time(NULL));

    variator->config = *config;

    if(!SDLVisualTest_InitVariation(&variator->variation, &variator->config))
    {
        SDLTest_LogError("SDLVisualTest_InitVariation() failed");
        return 0;
    }

    return 1;
}

char*
SDLVisualTest_GetNextRandomVariation(SDLVisualTest_RandomVariator* variator)
{
    SDLVisualTest_SUTOptionValue* vars;
    SDLVisualTest_SUTOption* options;
    int i;
    if(!variator)
    {
        SDLTest_LogError("variator argument cannot be NULL");
        return NULL;
    }
 
    /* to save typing */
    vars = variator->variation.vars;
    options = variator->config.options;

    /* generate a random variation */
    for(i = 0; i < variator->variation.num_vars; i++)
    {
        switch(options[i].type)
        {
            case SDL_SUT_OPTIONTYPE_BOOL:
                vars[i].bool_value = SDLTest_RandomIntegerInRange(0, 1) ? SDL_FALSE :
                                                                          SDL_TRUE;
            break;

            case SDL_SUT_OPTIONTYPE_ENUM:
            {
                int emx = 0;
                while(options[i].data.enum_values[emx])
                    emx++;
                vars[i].enumerated.index = SDLTest_RandomIntegerInRange(0, emx - 1);
            }
            break;
            
            case SDL_SUT_OPTIONTYPE_INT:
                vars[i].integer.value = SDLTest_RandomIntegerInRange(
                                        options[i].data.range.min,
                                        options[i].data.range.max);
            break;

            case SDL_SUT_OPTIONTYPE_STRING:
                // String values are left unchanged
            break;
        }
    }

    /* convert variation to an arguments string */
    if(!SDLVisualTest_MakeStrFromVariation(&variator->variation, &variator->config,
                                           variator->buffer, MAX_SUT_ARGS_LEN))
    {
        SDLTest_LogError("SDLVisualTest_MakeStrFromVariation() failed");
        return NULL;
    }

    return variator->buffer;
}

void SDLVisualTest_FreeRandomVariator(SDLVisualTest_RandomVariator* variator)
{
    if(!variator)
    {
        SDLTest_LogError("variator argument cannot be NULL");
        return;
    }
    SDL_free(variator->variation.vars);
    variator->variation.vars = NULL;
}