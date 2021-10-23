/* See LICENSE.txt for the full license governing this code. */
/**
 * \file variator_common.c
 *
 * Source file for some common functionality used by variators.
 */

#include <SDL_test.h>
#include "SDL_visualtest_variator_common.h"

int
SDLVisualTest_NextValue(SDLVisualTest_SUTOptionValue* var,
                        SDLVisualTest_SUTOption* opt)
{
    if(!var)
    {
        SDLTest_LogError("var argument cannot be NULL");
        return -1;
    }
    if(!opt)
    {
        SDLTest_LogError("opt argument cannot be NULL");
        return -1;
    }

    switch(opt->type)
    {
        case SDL_SUT_OPTIONTYPE_BOOL:
            if(var->bool_value)
            {
                var->bool_value = SDL_FALSE;
                return 1;
            }
            else
            {
                var->bool_value = SDL_TRUE;
                return 0;
            }
        break;

        case SDL_SUT_OPTIONTYPE_ENUM:
            var->enumerated.index++;
            if(!opt->data.enum_values[var->enumerated.index])
            {
                var->enumerated.index = 0;
                return 1;
            }
            return 0;
        break;

        case SDL_SUT_OPTIONTYPE_INT:
        {
            int increment = (opt->data.range.max - opt->data.range.min) /
                            SDL_SUT_INTEGER_OPTION_TEST_STEPS;
            /* prevents infinite loop when rounding */
            if(increment == 0)
                increment = 1;
            var->integer.value += increment;
            if(var->integer.value > opt->data.range.max)
            {
                var->integer.value = opt->data.range.min;
                return 1;
            }
            return 0;
        }
        break;

        case SDL_SUT_OPTIONTYPE_STRING:
            return 1;
        break;
    }
    return -1;
}

int
SDLVisualTest_MakeStrFromVariation(SDLVisualTest_Variation* variation,
                                   SDLVisualTest_SUTConfig* config,
                                   char* buffer, int size)
{
    int i, index;
    SDLVisualTest_SUTOptionValue* vars;
    SDLVisualTest_SUTOption* options;
    if(!variation)
    {
        SDLTest_LogError("variation argument cannot be NULL");
        return 0;
    }
    if(!config)
    {
        SDLTest_LogError("config argument cannot be NULL");
        return 0;
    }
    if(!buffer)
    {
        SDLTest_LogError("buffer argument cannot be NULL");
        return 0;
    }
    if(size <= 0)
    {
        SDLTest_LogError("size argument should be positive");
        return 0;
    }

    index = 0;
    buffer[0] = '\0';
    options = config->options;
    vars = variation->vars;
    for(i = 0; i < variation->num_vars; i++)
    {
        int n, enum_index;
        if(index >= size - 1)
        {
            SDLTest_LogError("String did not fit in buffer size");
            return 0;
        }
        switch(options[i].type)
        {
            case SDL_SUT_OPTIONTYPE_BOOL:
                if(vars[i].bool_value)
                {
                    n = SDL_snprintf(buffer + index, size - index, "%s ",
                                     options[i].name);
                    if(n <= 0)
                    {
                        SDLTest_LogError("SDL_snprintf() failed");
                        return 0;
                    }
                    index += n;
                }
            break;

            case SDL_SUT_OPTIONTYPE_ENUM:
                if(vars[i].enumerated.on)
                {
                    enum_index = vars[i].enumerated.index;
                    n = SDL_snprintf(buffer + index, size - index, "%s %s ",
                        options[i].name, options[i].data.enum_values[enum_index]);
                    index += n;
                }
            break;

            case SDL_SUT_OPTIONTYPE_INT:
                if(vars[i].integer.on)
                {
                    n = SDL_snprintf(buffer + index, size - index, "%s %d ",
                                     options[i].name, vars[i].integer.value);
                    index += n;
                }
            break;

            case SDL_SUT_OPTIONTYPE_STRING:
                if(vars[i].string.on)
                {
                    n = SDL_snprintf(buffer + index, size - index, "%s %s ",
                                     options[i].name, vars[i].string.value);
                    index += n;
                }
            break;
        }
    }
    return 1;
}

int
SDLVisualTest_InitVariation(SDLVisualTest_Variation* variation,
                            SDLVisualTest_SUTConfig* config)
{
    int i;
    SDLVisualTest_SUTOptionValue* vars;
    SDLVisualTest_SUTOption* options;
    if(!variation)
    {
        SDLTest_LogError("variation argument cannot be NULL");
        return 0;
    }
    if(!config)
    {
        SDLTest_LogError("config argument cannot be NULL");
        return 0;
    }

    /* initialize the first variation */
    if(config->num_options <= 0)
    {
        SDLTest_LogError("config->num_options must be positive");
        return 0;
    }
    variation->vars = (SDLVisualTest_SUTOptionValue*)SDL_malloc(config->num_options *
                     sizeof(SDLVisualTest_SUTOptionValue));
    if(!variation->vars)
    {
        SDLTest_LogError("malloc() failed");
        return 0;
    }
    variation->num_vars = config->num_options;
    vars = variation->vars;
    options = config->options;
    for(i = 0; i < variation->num_vars; i++)
    {
        switch(options[i].type)
        {
            case SDL_SUT_OPTIONTYPE_BOOL:
                vars[i].bool_value = SDL_FALSE;
            break;

            case SDL_SUT_OPTIONTYPE_ENUM:
                vars[i].enumerated.on = SDL_TRUE;
                vars[i].enumerated.index = 0;
            break;

            case SDL_SUT_OPTIONTYPE_INT:
            {
                vars[i].integer.on = SDL_TRUE;
                vars[i].integer.value = options[i].data.range.min;
            }
            break;

            case SDL_SUT_OPTIONTYPE_STRING:
                vars[i].string.on = SDL_TRUE;
                vars[i].string.value = options[i].name;
            break;
        }
    }
    return 1;
}