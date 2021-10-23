/* See LICENSE.txt for the full license governing this code. */
/**
 * \file rwhelper.c
 *
 * Header file with some helper functions for working with SDL_RWops.
 */

#include <SDL_rwops.h>

#ifndef SDL_visualtest_rwhelper_h_
#define SDL_visualtest_rwhelper_h_

/** Length of the buffer in SDLVisualTest_RWHelperBuffer */
#define RWOPS_BUFFER_LEN 256

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct that is used as a buffer by the RW helper functions. Should be initialized by calling
 * SDLVisualTest_RWHelperResetBuffer() before being used.
 */
typedef struct SDLVisualTest_RWHelperBuffer
{
    /*! Character buffer that data is read into */
    char buffer[RWOPS_BUFFER_LEN];
    /*! buffer[buffer_pos] is the next character to be read from the buffer */
    int buffer_pos;
    /*! Number of character read into the buffer */
    int buffer_width;
} SDLVisualTest_RWHelperBuffer;

/**
 * Resets the buffer pointed to by \c buffer used by some of the helper functions.
 * This function should be called when you're using one of the helper functions 
 * with a new SDL_RWops object.
 */
void SDLVisualTest_RWHelperResetBuffer(SDLVisualTest_RWHelperBuffer* buffer);

/**
 * Reads a single character using the SDL_RWops object pointed to by \c rw.
 * This function reads data in blocks and stores them in the buffer pointed to by
 * \c buffer, so other SDL_RWops functions should not be used in conjunction 
 * with this function.
 *
 * \return The character that was read.
 */
char SDLVisualTest_RWHelperReadChar(SDL_RWops* rw,
                                    SDLVisualTest_RWHelperBuffer* buffer);

/**
 * Reads characters using the SDL_RWops object pointed to by \c rw into the
 * character array pointed to by \c str (of size \c size) until either the 
 * array is full or a new line is encountered. If \c comment_char is encountered,
 * all characters from that position till the end of the line are ignored. The new line
 * is not included as part of the buffer. Lines with only whitespace and comments
 * are ignored. This function reads data in blocks and stores them in the buffer
 * pointed to by \c buffer, so other SDL_RWops functions should not be used in
 * conjunction with this function.
 * 
 * \return pointer to the string on success, NULL on failure or EOF.
 */
char* SDLVisualTest_RWHelperReadLine(SDL_RWops* rw, char* str, int size,
                                     SDLVisualTest_RWHelperBuffer* buffer,
                                     char comment_char);

/**
 * Counts the number of lines that are not all whitespace and comments using the
 * SDL_RWops object pointed to by \c rw. \c comment_char indicates the character
 * used for comments. Uses the buffer pointed to by \c buffer to read data in blocks.
 *
 * \return Number of lines on success, -1 on failure.
 */
int SDLVisualTest_RWHelperCountNonEmptyLines(SDL_RWops* rw,
                                             SDLVisualTest_RWHelperBuffer* buffer,
                                             char comment_char);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* SDL_visualtest_rwhelper_h_ */

/* vi: set ts=4 sw=4 expandtab: */
