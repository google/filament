/* See LICENSE.txt for the full license governing this code. */
/**
 * \file variator_exhaustive.c
 *
 * Source file for the variator that tests the SUT with all the different
 * variations of input parameters that are valid.
 */

#include <time.h>
#include <SDL_test.h>
#include "SDL_visualtest_sut_configparser.h"
#include "SDL_visualtest_exhaustive_variator.h"

static int
NextVariation(SDLVisualTest_Variation* variation,
              SDLVisualTest_SUTConfig* config)
{
    int i, carry;
    if(!variation)
    {
        SDLTest_LogError("variation argument cannot be NULL");
        return -1;
    }
    if(!config)
    {
        SDLTest_LogError("config argument cannot be NULL");
        return -1;
    }

    carry = 1;
    for(i = 0; i < variation->num_vars; i++)
    {
        carry = SDLVisualTest_NextValue(&variation->vars[i], &config->options[i]);
        if(carry != 1)
            break;
    }

    if(carry == 1) /* we're done, we've tried all possible variations */
        return 0;
    if(carry == 0)
        return 1;
    SDLTest_LogError("NextVariation() failed");
    return -1;
}

int
SDLVisualTest_InitExhaustiveVariator(SDLVisualTest_ExhaustiveVariator* variator,
                                     SDLVisualTest_SUTConfig* config)
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

    SDLTest_FuzzerInit(time(NULL));

    variator->config = *config;
    variator->variation.num_vars = 0;
    variator->variation.vars = NULL;

    return 1;
}

/* TODO: Right now variations where an option is not specified at all are not
   tested for. This can be implemented by switching the on attribute for integer,
   enum and string options to true and false. */
char*
SDLVisualTest_GetNextExhaustiveVariation(SDLVisualTest_ExhaustiveVariator* variator)
{
    int success;
    if(!variator)
    {
        SDLTest_LogError("variator argument cannot be NULL");
        return NULL;
    }

    if(!variator->variation.vars) /* the first time this function is called */
    {
        success = SDLVisualTest_InitVariation(&variator->variation,
                                              &variator->config);
        if(!success)
        {
            SDLTest_LogError("SDLVisualTest_InitVariation() failed");
            return NULL;
        }
        success = SDLVisualTest_MakeStrFromVariation(&variator->variation,
                  &variator->config, variator->buffer, MAX_SUT_ARGS_LEN);
        if(!success)
        {
            SDLTest_LogError("SDLVisualTest_MakeStrFromVariation() failed");
            return NULL;
        }
        return variator->buffer;
    }
    else
    {
        success = NextVariation(&variator->variation, &variator->config);
        if(success == 1)
        {
            success = SDLVisualTest_MakeStrFromVariation(&variator->variation,
                      &variator->config, variator->buffer, MAX_SUT_ARGS_LEN);
            if(!success)
            {
                SDLTest_LogError("SDLVisualTest_MakeStrFromVariation() failed");
                return NULL;
            }
            return variator->buffer;
        }
        else if(success == -1)
            SDLTest_LogError("NextVariation() failed.");
        return NULL;
    }
    return NULL;
}

void
SDLVisualTest_FreeExhaustiveVariator(SDLVisualTest_ExhaustiveVariator* variator)
{
    if(!variator)
    {
        SDLTest_LogError("variator argument cannot be NULL");
        return;
    }
    SDL_free(variator->variation.vars);
    variator->variation.vars = NULL;
}