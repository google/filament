/* See LICENSE.txt for the full license governing this code. */
/**
 * \file rwhelper.c
 *
 * Source file with some helper functions for working with SDL_RWops.
 */

#include <SDL_test.h>
#include "SDL_visualtest_sut_configparser.h"
#include "SDL_visualtest_rwhelper.h"

void
SDLVisualTest_RWHelperResetBuffer(SDLVisualTest_RWHelperBuffer* buffer)
{
    if(!buffer)
    {
        SDLTest_LogError("buffer argument cannot be NULL");
        return;
    }
    buffer->buffer_pos = 0;
    buffer->buffer_width = 0;
}

char
SDLVisualTest_RWHelperReadChar(SDL_RWops* rw, SDLVisualTest_RWHelperBuffer* buffer)
{
    if(!rw || !buffer)
        return 0;
    /* if the buffer has been consumed, we fill it up again */
    if(buffer->buffer_pos == buffer->buffer_width)
    {
        buffer->buffer_width = SDL_RWread(rw, buffer->buffer, 1, RWOPS_BUFFER_LEN);
        buffer->buffer_pos = 0;
        if(buffer->buffer_width == 0)
            return 0;
    }
    buffer->buffer_pos++;
    return buffer->buffer[buffer->buffer_pos - 1];
}

/* does not include new lines in the buffer and adds a trailing null character */
char*
SDLVisualTest_RWHelperReadLine(SDL_RWops* rw, char* str, int size,
                               SDLVisualTest_RWHelperBuffer* buffer,
                               char comment_char)
{
    char ch;
    int current_pos, done;
    if(!rw)
    {
        SDLTest_LogError("rw argument cannot be NULL");
        return NULL;
    }
    if(!str)
    {
        SDLTest_LogError("str argument cannot be NULL");
        return NULL;
    }
    if(!buffer)
    {
        SDLTest_LogError("buffer argument cannot be NULL");
        return NULL;
    }
    if(size <= 0)
    {
        SDLTest_LogError("size argument should be positive");
        return NULL;
    }

    done = 0;
    while(!done)
    {
        /* ignore leading whitespace */
        for(ch = SDLVisualTest_RWHelperReadChar(rw, buffer); ch && SDL_isspace(ch);
            ch = SDLVisualTest_RWHelperReadChar(rw, buffer));

        for(current_pos = 0;
            ch && ch != '\n' && ch != '\r' && ch != comment_char;
            current_pos++)
        {
            str[current_pos] = ch;
            if(current_pos >= size - 2)
            {
                current_pos++;
                break;
            }
            ch = SDLVisualTest_RWHelperReadChar(rw, buffer);
        }

        done = 1;
        if(ch == comment_char) /* discard all characters until the next line */
        {
            do
            {
                ch = SDLVisualTest_RWHelperReadChar(rw, buffer);
            }while(ch && ch != '\n' && ch != '\r');

            if(current_pos == 0)
                done = 0;
        }
    }
    if(current_pos == 0)
        return NULL;

    str[current_pos] = '\0';
    return str;
}

/* Lines with all whitespace are ignored */
int
SDLVisualTest_RWHelperCountNonEmptyLines(SDL_RWops* rw,
                                         SDLVisualTest_RWHelperBuffer* buffer,
                                         char comment_char)
{
    int num_lines = 0;
    char str[MAX_SUTOPTION_LINE_LENGTH];
    if(!rw)
    {
        SDLTest_LogError("rw argument cannot be NULL");
        return -1;
    }
    if(!buffer)
    {
        SDLTest_LogError("buffer argument cannot be NULL");
        return -1;
    }
    while(SDLVisualTest_RWHelperReadLine(rw, str, MAX_SUTOPTION_LINE_LENGTH,
                                         buffer, comment_char))
        num_lines++;
    return num_lines;
}