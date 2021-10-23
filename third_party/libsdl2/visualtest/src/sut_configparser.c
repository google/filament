/* See LICENSE.txt for the full license governing this code. */
/**
 * \file sut_configparser.c
 *
 * Source file for the parser for SUT config files.
 */

#include <limits.h>
#include <string.h>
#include <SDL_test.h>
#include <SDL_rwops.h>
#include "SDL_visualtest_sut_configparser.h"
#include "SDL_visualtest_parsehelper.h"
#include "SDL_visualtest_rwhelper.h"

int
SDLVisualTest_ParseSUTConfig(char* file, SDLVisualTest_SUTConfig* config)
{
    char line[MAX_SUTOPTION_LINE_LENGTH];
    SDLVisualTest_RWHelperBuffer buffer;
    char* token_ptr;
    char* token_end;
    int num_lines, i, token_len;
    SDL_RWops* rw;

    if(!file)
    {
        SDLTest_LogError("file argument cannot be NULL");
        return 0;
    }
    if(!config)
    {
        SDLTest_LogError("config argument cannot be NULL");
        return 0;
    }

    /* count the number of lines */
    rw = SDL_RWFromFile(file, "r");
    if(!rw)
    {
        SDLTest_LogError("SDL_RWFromFile() failed");
        return 0;
    }
    SDLVisualTest_RWHelperResetBuffer(&buffer);
    num_lines = SDLVisualTest_RWHelperCountNonEmptyLines(rw, &buffer, '#');
    if(num_lines == -1)
        return 0;
    else if(num_lines == 0)
    {
        config->options = NULL;
        config->num_options = 0;
        SDL_RWclose(rw);
        return 1;
    }

    /* allocate memory */
    SDL_RWseek(rw, 0, RW_SEEK_SET);
    SDLVisualTest_RWHelperResetBuffer(&buffer);
    config->num_options = num_lines;
    config->options = (SDLVisualTest_SUTOption*)SDL_malloc(num_lines * 
                      sizeof(SDLVisualTest_SUTOption));
    if(!config->options)
    {
        SDLTest_LogError("malloc() failed");
        SDL_RWclose(rw);
        return 0;
    }

    /* actually parse the options */
    for(i = 0; i < num_lines; i++)
    {
        if(!SDLVisualTest_RWHelperReadLine(rw, line, MAX_SUTOPTION_LINE_LENGTH,
                                           &buffer, '#'))
        {
            SDLTest_LogError("SDLVisualTest_RWHelperReadLine() failed");
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }

        /* parse name */
        token_ptr = strtok(line, ", ");
        if(!token_ptr)
        {
            SDLTest_LogError("Could not parse line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }
        token_len = SDL_strlen(token_ptr) + 1;
        SDL_strlcpy(config->options[i].name, token_ptr, token_len);

        /* parse type */
        token_ptr = strtok(NULL, ", ");
        if(!token_ptr)
        {
            SDLTest_LogError("Could not parse line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }
        if(SDL_strcmp(token_ptr, "string") == 0)
            config->options[i].type = SDL_SUT_OPTIONTYPE_STRING;
        else if(SDL_strcmp(token_ptr, "integer") == 0)
            config->options[i].type = SDL_SUT_OPTIONTYPE_INT;
        else if(SDL_strcmp(token_ptr, "enum") == 0)
            config->options[i].type = SDL_SUT_OPTIONTYPE_ENUM;
        else if(SDL_strcmp(token_ptr, "boolean") == 0)
            config->options[i].type = SDL_SUT_OPTIONTYPE_BOOL;
        else
        {
            SDLTest_LogError("Could not parse type token at line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }

        /* parse values */
        token_ptr = strtok(NULL, "]");
        if(!token_ptr)
        {
            SDLTest_LogError("Could not parse line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }
        token_ptr = SDL_strchr(token_ptr, '[');
        if(!token_ptr)
        {
            SDLTest_LogError("Could not parse enum token at line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }
        token_ptr++;
        if(config->options[i].type == SDL_SUT_OPTIONTYPE_INT)
        {
            if(SDL_sscanf(token_ptr, "%d %d", &config->options[i].data.range.min,
                          &config->options[i].data.range.max) != 2)
            {
                config->options[i].data.range.min = INT_MIN;
                config->options[i].data.range.max = INT_MAX;
            }
        }
        else if(config->options[i].type == SDL_SUT_OPTIONTYPE_ENUM)
        {
            config->options[i].data.enum_values = SDLVisualTest_Tokenize(token_ptr,
                                                  MAX_SUTOPTION_ENUMVAL_LEN);
            if(!config->options[i].data.enum_values)
            {
                SDLTest_LogError("Could not parse enum token at line %d", i + 1);
                SDL_free(config->options);
                SDL_RWclose(rw);
                return 0;
            }
        }

        /* parse required */
        token_ptr = strtok(NULL, ", ");
        if(!token_ptr)
        {
            SDLTest_LogError("Could not parse line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }

        if(SDL_strcmp(token_ptr, "true") == 0)
            config->options[i].required = SDL_TRUE;
        else if(SDL_strcmp(token_ptr, "false") == 0)
            config->options[i].required = SDL_FALSE;
        else
        {
            SDLTest_LogError("Could not parse required token at line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }

        /* parse categories */
        token_ptr = strtok(NULL, ",");
        if(!token_ptr)
        {
            SDLTest_LogError("Could not parse line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }
        token_ptr = SDL_strchr(token_ptr, '[');
        if(!token_ptr)
        {
            SDLTest_LogError("Could not parse enum token at line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }
        token_ptr++;
        token_end = SDL_strchr(token_ptr, ']');
        *token_end = '\0';
        if(!token_end)
        {
            SDLTest_LogError("Could not parse enum token at line %d", i + 1);
            SDL_free(config->options);
            SDL_RWclose(rw);
            return 0;
        }
        config->options[i].categories = SDLVisualTest_Tokenize(token_ptr,
                                        MAX_SUTOPTION_CATEGORY_LEN);
    }
    SDL_RWclose(rw);
    return 1;
}

void
SDLVisualTest_FreeSUTConfig(SDLVisualTest_SUTConfig* config)
{
    if(config && config->options)
    {
        SDLVisualTest_SUTOption* option;
        for(option = config->options;
            option != config->options + config->num_options; option++)
        {
            if(option->categories)
                SDL_free(option->categories);
            if(option->type == SDL_SUT_OPTIONTYPE_ENUM && option->data.enum_values)
                SDL_free(option->data.enum_values);
        }
        SDL_free(config->options);
        config->options = NULL;
        config->num_options = 0;
    }
}
