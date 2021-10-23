/* See LICENSE.txt for the full license governing this code. */
/**
 * \file parsehelper.c
 *
 * Source file with some helper functions for parsing strings.
 */

#include <SDL_test.h>
#include "SDL_visualtest_harness_argparser.h"

/* this function uses a DFA to count the number of tokens in an agruments string.
   state 0 is taken to be the start and end state. State 1 handles a double quoted
   argument and state 2 handles unquoted arguments. */
static int
CountTokens(char* args)
{
    int index, num_tokens;
    int state; /* current state of the DFA */

    if(!args)
    	return -1;

    index = 0;
    state = 0;
    num_tokens = 0;
    while(args[index])
    {
        char ch = args[index];
        switch(state)
        {
            case 0:
            if(ch == '\"')
            {
                state = 1;
                num_tokens++;
            }
            else if(!SDL_isspace(ch))
            {
                state = 2;
                num_tokens++;
            }
            break;

            case 1:
            if(ch == '\"')
            {
                state = 0;
            }
            break;

            case 2:
            if(SDL_isspace(ch))
            {
                state = 0;
            }
            break;
        }
        index++;
    }
    return num_tokens;
}

/* - size of tokens is num_tokens + 1
- uses the same DFA used in CountTokens() to split args into an array of strings */
static int
TokenizeHelper(char* str, char** tokens, int num_tokens, int max_token_len)
{
    int index, state, done, st_index, token_index;

    if(!str)
    {
        SDLTest_LogError("str argument cannot be NULL");
        return 0;
    }
    if(!tokens)
    {
        SDLTest_LogError("tokens argument cannot be NULL");
        return 0;
    }
    if(num_tokens <= 0)
    {
        SDLTest_LogError("num_tokens argument must be positive");
        return 0;
    }
    if(max_token_len <= 0)
    {
        SDLTest_LogError("max_token_len argument must be positive");
        return 0;
    }

    /* allocate memory for the tokens */
    tokens[num_tokens] = NULL;
    for(index = 0; index < num_tokens; index++)
    {
        tokens[index] = (char*)SDL_malloc(max_token_len);
        if(!tokens[index])
        {
            int i;
            SDLTest_LogError("malloc() failed.");
            for(i = 0; i < index; i++)
                SDL_free(tokens[i]);
            return 0;
        }
        tokens[index][0] = '\0';
    }

    /* copy the tokens into the array */
    st_index = 0;
    index = 0;
    token_index = 0;
    state = 0;
    done = 0;
    while(!done)
    {
        char ch = str[index];
        switch(state)
        {
            case 0:
            if(ch == '\"')
            {
                state = 1;
                st_index = index + 1;
            }
            else if(!ch)
                done = 1;
            else if(ch && !SDL_isspace(ch))
            {
                state = 2;
                st_index = index;
            }
            break;

            case 1:
            if(ch == '\"')
            {
                int i;
                state = 0;
                for(i = st_index; i < index; i++)
                {
                    tokens[token_index][i - st_index] = str[i];
                }
                tokens[token_index][i - st_index] = '\0';
                token_index++;
            }
            else if(!ch)
            {
                SDLTest_LogError("Parsing Error!");
                done = 1;
            }
            break;

            case 2:
            if(!ch)
                done = 1;
            if(SDL_isspace(ch) || !ch)
            {
                int i;
                state = 0;
                for(i = st_index; i < index; i++)
                {
                    tokens[token_index][i - st_index] = str[i];
                }
                tokens[token_index][i - st_index] = '\0';
                token_index++;
            }
            break;
        }
        index++;
    }
    return 1;
}

char**
SDLVisualTest_Tokenize(char* str, int max_token_len)
{
    int num_tokens;
    char** tokens;

    if(!str)
    {
        SDLTest_LogError("str argument cannot be NULL");
        return NULL;
    }
    if(max_token_len <= 0)
    {
        SDLTest_LogError("max_token_len argument must be positive");
        return NULL;
    }

    num_tokens = CountTokens(str);
    if(num_tokens == 0)
        return NULL;

    tokens = (char**)SDL_malloc(sizeof(char*) * (num_tokens + 1));
    if(!TokenizeHelper(str, tokens, num_tokens, max_token_len))
    {
        SDLTest_LogError("TokenizeHelper() failed");
        SDL_free(tokens);
        return NULL;
    }
    return tokens;
}

char**
SDLVisualTest_ParseArgsToArgv(char* args)
{
    char** argv;
    int num_tokens;

    num_tokens = CountTokens(args);
    if(num_tokens == 0)
        return NULL;

    /* allocate space for arguments */
    argv = (char**)SDL_malloc((num_tokens + 2) * sizeof(char*));
    if(!argv)
    {
        SDLTest_LogError("malloc() failed.");
        return NULL;
    }

    /* tokenize */
    if(!TokenizeHelper(args, argv + 1, num_tokens, MAX_SUT_ARGS_LEN))
    {
        SDLTest_LogError("TokenizeHelper() failed");
        SDL_free(argv);
        return NULL;
    }
    argv[0] = NULL;
    return argv;
}
