/* See LICENSE.txt for the full license governing this code. */
/**
 * \file harness_argparser.c
 *
 * Source file for functions to parse arguments to the test harness.
 */

#include <SDL_test.h>
#include <stdio.h>
#include <string.h>

#include "SDL_visualtest_harness_argparser.h"
#include "SDL_visualtest_rwhelper.h"

/** Maximum length of one line in the config file */
#define MAX_CONFIG_LINE_LEN 400
/** Default value for the timeout after which the SUT is forcefully killed */
#define DEFAULT_SUT_TIMEOUT (60 * 1000)

/* String compare s1 and s2 ignoring leading hyphens */
static int
StrCaseCmpIgnoreHyphen(char* s1, char* s2)
{
    /* treat NULL pointer as empty strings */
    if(!s1)
        s1 = "";
    if(!s2)
        s2 = "";

    while(*s1 == '-')
        s1++;
    while(*s2 == '-')
        s2++;

    return SDL_strcasecmp(s1, s2);
}

/* parser an argument, updates the state object and returns the number of
   arguments processed; returns -1 on failure */
static int
ParseArg(char** argv, int index, SDLVisualTest_HarnessState* state)
{
    if(!argv || !argv[index] || !state)
        return 0;

    if(StrCaseCmpIgnoreHyphen("sutapp", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for sutapp.");
            return -1;
        }
        SDL_strlcpy(state->sutapp, argv[index], MAX_PATH_LEN);
        SDLTest_Log("SUT Application: %s", state->sutapp);
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("output-dir", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for output-dir.");
            return -1;
        }
        SDL_strlcpy(state->output_dir, argv[index], MAX_PATH_LEN);
        SDLTest_Log("Screenshot Output Directory: %s", state->output_dir);
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("verify-dir", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for verify-dir.");
            return -1;
        }
        SDL_strlcpy(state->verify_dir, argv[index], MAX_PATH_LEN);
        SDLTest_Log("Screenshot Verification Directory: %s", state->verify_dir);
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("sutargs", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for sutargs.");
            return -1;
        }
        SDL_strlcpy(state->sutargs, argv[index], MAX_SUT_ARGS_LEN);
        SDLTest_Log("SUT Arguments: %s", state->sutargs);
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("timeout", argv[index]) == 0)
    {
        int hr, min, sec;
        index++;
        if(!argv[index] || SDL_sscanf(argv[index], "%d:%d:%d", &hr, &min, &sec) != 3)
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for timeout.");
            return -1;
        }
        state->timeout = (((hr * 60) + min) * 60 + sec) * 1000;
        SDLTest_Log("Maximum Timeout for each SUT run: %d milliseconds",
                    state->timeout);
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("parameter-config", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for parameter-config.");
            return -1;
        }
        SDLTest_Log("SUT Parameters file: %s", argv[index]);
        SDLVisualTest_FreeSUTConfig(&state->sut_config);
        if(!SDLVisualTest_ParseSUTConfig(argv[index], &state->sut_config))
        {
            SDLTest_LogError("Failed to parse SUT parameters file");
            return -1;
        }
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("variator", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for variator.");
            return -1;
        }
        SDLTest_Log("Variator: %s", argv[index]);
        if(SDL_strcasecmp("exhaustive", argv[index]) == 0)
            state->variator_type = SDL_VARIATOR_EXHAUSTIVE;
        else if(SDL_strcasecmp("random", argv[index]) == 0)
            state->variator_type = SDL_VARIATOR_RANDOM;
        else
        {
            SDLTest_LogError("Arguments parsing error: Invalid variator name.");
            return -1;
        }
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("num-variations", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: Invalid argument for num-variations.");
            return -1;
        }
        state->num_variations = SDL_atoi(argv[index]);
        SDLTest_Log("Number of variations to run: %d", state->num_variations);
        if(state->num_variations <= 0)
        {
            SDLTest_LogError("Arguments parsing error: num-variations must be positive.");
            return -1;
        }
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("no-launch", argv[index]) == 0)
    {
        state->no_launch = SDL_TRUE;
        SDLTest_Log("SUT will not be launched.");
        return 1;
    }
    else if(StrCaseCmpIgnoreHyphen("action-config", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: invalid argument for action-config");
            return -1;
        }
        SDLTest_Log("Action Config file: %s", argv[index]);
        SDLVisualTest_EmptyActionQueue(&state->action_queue);
        if(!SDLVisualTest_ParseActionConfig(argv[index], &state->action_queue))
        {
            SDLTest_LogError("SDLVisualTest_ParseActionConfig() failed");
            return -1;
        }
        return 2;
    }
    else if(StrCaseCmpIgnoreHyphen("config", argv[index]) == 0)
    {
        index++;
        if(!argv[index])
        {
            SDLTest_LogError("Arguments parsing error: invalid argument for config");
            return -1;
        }

        /* do nothing, this option has already been handled */
        return 2;
    }
    return 0;
}

/* TODO: Trailing/leading spaces and spaces between equals sign not supported. */
static int
ParseConfig(char* file, SDLVisualTest_HarnessState* state)
{
    SDL_RWops* rw;
    SDLVisualTest_RWHelperBuffer buffer;
    char line[MAX_CONFIG_LINE_LEN];

    rw = SDL_RWFromFile(file, "r");
    if(!rw)
    {
        SDLTest_LogError("SDL_RWFromFile() failed");
        return 0;
    }

    SDLVisualTest_RWHelperResetBuffer(&buffer);
    while(SDLVisualTest_RWHelperReadLine(rw, line, MAX_CONFIG_LINE_LEN,
                                         &buffer, '#'))
    {
        char** argv;
        int i, num_params;

        /* count number of parameters and replace the trailing newline with 0 */
        num_params = 1;
        for(i = 0; line[i]; i++)
        {
            if(line[i] == '=')
            {
                num_params = 2;
                break;
            }
        }

        /* populate argv */
        argv = (char**)SDL_malloc((num_params + 1) * sizeof(char*));
        if(!argv)
        {
            SDLTest_LogError("malloc() failed.");
            SDL_RWclose(rw);
            return 0;
        }

        argv[num_params] = NULL;
        for(i = 0; i < num_params; i++)
        {
            argv[i] = strtok(i == 0 ? line : NULL, "=");
        }

        if(ParseArg(argv, 0, state) == -1)
        {
            SDLTest_LogError("ParseArg() failed");
            SDL_free(argv);
            SDL_RWclose(rw);
            return 0;
        }
        SDL_free(argv);
    }
    SDL_RWclose(rw);

    if(!state->sutapp[0])
        return 0;
    return 1;
}

int
SDLVisualTest_ParseHarnessArgs(char** argv, SDLVisualTest_HarnessState* state)
{
    int i;

    SDLTest_Log("Parsing commandline arguments..");

    if(!argv)
    {
        SDLTest_LogError("argv is NULL");
        return 0;
    }
    if(!state)
    {
        SDLTest_LogError("state is NULL");
        return 0;
    }

    /* initialize the state object */
    state->sutargs[0] = '\0';
    state->sutapp[0] = '\0';
    state->output_dir[0] = '\0';
    state->verify_dir[0] = '\0';
    state->timeout = DEFAULT_SUT_TIMEOUT;
    SDL_memset(&state->sut_config, 0, sizeof(SDLVisualTest_SUTConfig));
    SDL_memset(&state->action_queue, 0, sizeof(SDLVisualTest_ActionQueue));
    state->variator_type = SDL_VARIATOR_RANDOM;
    state->num_variations = -1;
    state->no_launch = SDL_FALSE;

    /* parse config file if passed */
    for(i = 0; argv[i]; i++)
    {
        if(StrCaseCmpIgnoreHyphen("config", argv[i]) == 0)
        {
            if(!argv[i + 1])
            {
                SDLTest_Log("Arguments parsing error: invalid argument for config.");
                return 0;
            }
            if(!ParseConfig(argv[i + 1], state))
            {
                SDLTest_LogError("ParseConfig() failed");
                return 0;
            }
        }
    }

    /* parse the arguments */
    for(i = 0; argv[i];)
    {
        int consumed = ParseArg(argv, i, state);
        if(consumed == -1 || consumed == 0)
        {
            SDLTest_LogError("ParseArg() failed");
            return 0;
        }
        i += consumed;
    }

    if(state->variator_type == SDL_VARIATOR_RANDOM && state->num_variations == -1)
        state->num_variations = 1;

    /* check to see if required options have been passed */
    if(!state->sutapp[0])
    {
        SDLTest_LogError("sutapp must be passed.");
        return 0;
    }
    if(!state->sutargs[0] && !state->sut_config.options)
    {
        SDLTest_LogError("Either sutargs or parameter-config must be passed.");
        return 0;
    }
    if(!state->output_dir[0])
    {
        SDL_strlcpy(state->output_dir, "./output", MAX_PATH_LEN);
    }
    if(!state->verify_dir[0])
    {
        SDL_strlcpy(state->verify_dir, "./verify", MAX_PATH_LEN);
    }

    return 1;
}

void
SDLVisualTest_FreeHarnessState(SDLVisualTest_HarnessState* state)
{
    if(state)
    {
        SDLVisualTest_EmptyActionQueue(&state->action_queue);
        SDLVisualTest_FreeSUTConfig(&state->sut_config);
    }
}