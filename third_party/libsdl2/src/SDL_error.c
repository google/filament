/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "./SDL_internal.h"

/* Simple error handling in SDL */

#include "SDL_log.h"
#include "SDL_error.h"
#include "SDL_error_c.h"


/* Routine to get the thread-specific error variable */
#if SDL_THREADS_DISABLED
/* The default (non-thread-safe) global error variable */
static SDL_error SDL_global_error;
#define SDL_GetErrBuf() (&SDL_global_error)
#else
extern SDL_error *SDL_GetErrBuf(void);
#endif /* SDL_THREADS_DISABLED */

#define SDL_ERRBUFIZE   1024

/* Private functions */

static const char *
SDL_LookupString(const char *key)
{
    /* FIXME: Add code to lookup key in language string hash-table */
    return key;
}

/* Public functions */

static char *SDL_GetErrorMsg(char *errstr, int maxlen);

int
SDL_SetError(SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    va_list ap;
    SDL_error *error;

    /* Ignore call if invalid format pointer was passed */
    if (fmt == NULL) return -1;

    /* Copy in the key, mark error as valid */
    error = SDL_GetErrBuf();
    error->error = 1;
    SDL_strlcpy((char *) error->key, fmt, sizeof(error->key));

    va_start(ap, fmt);
    error->argc = 0;
    while (*fmt) {
        if (*fmt++ == '%') {
            while (*fmt == '.' || (*fmt >= '0' && *fmt <= '9')) {
                ++fmt;
            }
            switch (*fmt++) {
            case 0:            /* Malformed format string.. */
                --fmt;
                break;
            case 'l':
                switch (*fmt++) {
                case 0:        /* Malformed format string.. */
                    --fmt;
                    break;
                case 'i': case 'd': case 'u':
                    error->args[error->argc++].value_l = va_arg(ap, long);
                    break;
                }
                break;
            case 'c':
            case 'i':
            case 'd':
            case 'u':
            case 'o':
            case 'x':
            case 'X':
                error->args[error->argc++].value_i = va_arg(ap, int);
                break;
            case 'f':
                error->args[error->argc++].value_f = va_arg(ap, double);
                break;
            case 'p':
                error->args[error->argc++].value_ptr = va_arg(ap, void *);
                break;
            case 's':
                {
                    int i = error->argc;
                    const char *str = va_arg(ap, const char *);
                    if (str == NULL)
                        str = "(null)";
                    SDL_strlcpy((char *) error->args[i].buf, str,
                                ERR_MAX_STRLEN);
                    error->argc++;
                }
                break;
            default:
                break;
            }
            if (error->argc >= ERR_MAX_ARGS) {
                break;
            }
        }
    }
    va_end(ap);

    if (SDL_LogGetPriority(SDL_LOG_CATEGORY_ERROR) <= SDL_LOG_PRIORITY_DEBUG) {
        /* If we are in debug mode, print out an error message
         * Avoid stomping on the static buffer in GetError, just
         * in case this is called while processing a ShowMessageBox to
         * show an error already in that static buffer.
         */
        char errmsg[SDL_ERRBUFIZE];
        SDL_GetErrorMsg(errmsg, sizeof(errmsg));
        SDL_LogDebug(SDL_LOG_CATEGORY_ERROR, "%s", errmsg);
    }
    return -1;
}

/* Available for backwards compatibility */
const char *
SDL_GetError(void)
{
    static char errmsg[SDL_ERRBUFIZE];

    return SDL_GetErrorMsg(errmsg, SDL_ERRBUFIZE);
}

void
SDL_ClearError(void)
{
    SDL_error *error;

    error = SDL_GetErrBuf();
    error->error = 0;
}

/* Very common errors go here */
int
SDL_Error(SDL_errorcode code)
{
    switch (code) {
    case SDL_ENOMEM:
        return SDL_SetError("Out of memory");
    case SDL_EFREAD:
        return SDL_SetError("Error reading from datastream");
    case SDL_EFWRITE:
        return SDL_SetError("Error writing to datastream");
    case SDL_EFSEEK:
        return SDL_SetError("Error seeking in datastream");
    case SDL_UNSUPPORTED:
        return SDL_SetError("That operation is not supported");
    default:
        return SDL_SetError("Unknown SDL error");
    }
}

#ifdef TEST_ERROR
int
main(int argc, char *argv[])
{
    char buffer[BUFSIZ + 1];

    SDL_SetError("Hi there!");
    printf("Error 1: %s\n", SDL_GetError());
    SDL_ClearError();
    SDL_memset(buffer, '1', BUFSIZ);
    buffer[BUFSIZ] = 0;
    SDL_SetError("This is the error: %s (%f)", buffer, 1.0);
    printf("Error 2: %s\n", SDL_GetError());
    exit(0);
}
#endif


/* keep this at the end of the file so it works with GCC builds that don't
   support "#pragma GCC diagnostic push" ... we'll just leave the warning
   disabled after this. */
/* this pragma arrived in GCC 4.2 and causes a warning on older GCCs! Sigh. */
#if defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && (__GNUC_MINOR__ >= 2))))
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

/* This function has a bit more overhead than most error functions
   so that it supports internationalization and thread-safe errors.
*/
static char *
SDL_GetErrorMsg(char *errstr, int maxlen)
{
    SDL_error *error;

    /* Clear the error string */
    *errstr = '\0';
    --maxlen;

    /* Get the thread-safe error, and print it out */
    error = SDL_GetErrBuf();
    if (error->error) {
        const char *fmt;
        char *msg = errstr;
        int len;
        int argi;

        fmt = SDL_LookupString(error->key);
        argi = 0;
        while (*fmt && (maxlen > 0)) {
            if (*fmt == '%') {
                char tmp[32], *spot = tmp;
                *spot++ = *fmt++;
                while ((*fmt == '.' || (*fmt >= '0' && *fmt <= '9'))
                       && spot < (tmp + SDL_arraysize(tmp) - 2)) {
                    *spot++ = *fmt++;
                }
                if (*fmt == 'l') {
                    *spot++ = *fmt++;
                    *spot++ = *fmt++;
                    *spot++ = '\0';
                    switch (spot[-2]) {
                    case 'i': case 'd': case 'u':
                      len = SDL_snprintf(msg, maxlen, tmp,
                                         error->args[argi++].value_l);
                      if (len > 0) {
                          msg += len;
                          maxlen -= len;
                      }
                      break;
                    }
                    continue;
                }
                *spot++ = *fmt++;
                *spot++ = '\0';
                switch (spot[-2]) {
                case '%':
                    *msg++ = '%';
                    maxlen -= 1;
                    break;
                case 'c':
                case 'i':
                case 'd':
                case 'u':
                case 'o':
                case 'x':
                case 'X':
                    len =
                        SDL_snprintf(msg, maxlen, tmp,
                                     error->args[argi++].value_i);
                    if (len > 0) {
                        msg += len;
                        maxlen -= len;
                    }
                    break;

                case 'f':
                    len =
                        SDL_snprintf(msg, maxlen, tmp,
                                     error->args[argi++].value_f);
                    if (len > 0) {
                        msg += len;
                        maxlen -= len;
                    }
                    break;

                case 'p':
                    len =
                        SDL_snprintf(msg, maxlen, tmp,
                                     error->args[argi++].value_ptr);
                    if (len > 0) {
                        msg += len;
                        maxlen -= len;
                    }
                    break;

                case 's':
                    len =
                        SDL_snprintf(msg, maxlen, tmp,
                                     SDL_LookupString(error->args[argi++].
                                                      buf));
                    if (len > 0) {
                        msg += len;
                        maxlen -= len;
                    }
                    break;

                }
            } else {
                *msg++ = *fmt++;
                maxlen -= 1;
            }
        }

        /* slide back if we've overshot the end of our buffer. */
        if (maxlen < 0) {
            msg -= (-maxlen) + 1;
        }

        *msg = 0;               /* NULL terminate the string */
    }
    return (errstr);
}

/* vi: set ts=4 sw=4 expandtab: */
