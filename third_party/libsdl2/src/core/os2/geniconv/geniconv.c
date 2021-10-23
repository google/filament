/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

/*
  Universal iconv implementation for OS/2.

  Andrey Vasilkin, 2016.
*/

#define INCL_DOSMODULEMGR   /* Module Manager values */
#define INCL_DOSERRORS      /* Error values */
#include <os2.h>

#include "geniconv.h"

/*#define DEBUG*/

#ifdef DEBUG
# include <stdio.h>
# define debug(s,...) printf(__func__"(): "##s"\n" ,##__VA_ARGS__)
#else
# define debug(s,...) do {} while (0)
#endif

/* Exports from os2iconv.c */
extern iconv_t _System os2_iconv_open(const char* tocode, const char* fromcode);
extern size_t _System os2_iconv(iconv_t cd, char* * inbuf,
                                size_t *inbytesleft, char* * outbuf,
                                size_t *outbytesleft);
extern int _System os2_iconv_close(iconv_t cd);

/* Functions pointers types */
typedef iconv_t _System (*FNICONV_OPEN)(const char* tocode, const char* fromcode);
typedef size_t _System (*FNICONV)(iconv_t cd, char* * inbuf,
                                  size_t *inbytesleft, char* * outbuf,
                                  size_t *outbytesleft);
typedef int _System (*FNICONV_CLOSE)(iconv_t cd);

/* Used DLL module handle */
static HMODULE         hmIconv = NULLHANDLE;
/* Functions pointers */
static FNICONV_OPEN    fn_iconv_open = NULL;
static FNICONV         fn_iconv = NULL;
static FNICONV_CLOSE   fn_iconv_close = NULL;


static BOOL _loadDLL(PSZ pszName, PSZ pszIconvOpen, PSZ pszIconv,
                     PSZ pszIconvClose)
{
    ULONG ulRC;
    CHAR  acError[256];

    ulRC = DosLoadModule(acError, sizeof(acError), pszName, &hmIconv);
    if (ulRC != NO_ERROR) {
        debug("DLL not loaded: %s", &acError);
        return FALSE;
    }

    do {
        ulRC = DosQueryProcAddr(hmIconv, 0, pszIconvOpen, (PFN *)&fn_iconv_open);
        if (ulRC != NO_ERROR) {
            debug("Error: cannot find entry %s in %s", pszIconvOpen, pszName);
            break;
        }

        ulRC = DosQueryProcAddr(hmIconv, 0, pszIconv, (PFN *)&fn_iconv);
        if (ulRC != NO_ERROR) {
            debug("Error: cannot find entry %s in %s", pszIconv, pszName);
            break;
        }

        ulRC = DosQueryProcAddr(hmIconv, 0, pszIconvClose, (PFN *)&fn_iconv_close);
        if (ulRC != NO_ERROR) {
            debug("Error: cannot find entry %s in %s", pszIconvClose, pszName);
          break;
        }

        debug("DLL %s used", pszName);
        return TRUE;
    } while (FALSE);

    DosFreeModule(hmIconv);
    hmIconv = NULLHANDLE;
    return FALSE;
}

static void _init(void)
{
    if (fn_iconv_open != NULL) /* Already was initialized */
        return;

    /* Try to load kiconv.dll, iconv2.dll or iconv.dll */
    if (!_loadDLL("KICONV", "_libiconv_open", "_libiconv", "_libiconv_close") &&
        !_loadDLL("ICONV2", "_libiconv_open", "_libiconv", "_libiconv_close") &&
        !_loadDLL("ICONV",  "_iconv_open",    "_iconv",    "_iconv_close") ) {
        /* No DLL was loaded - use OS/2 conversion objects API */
        debug("Uni*() API used");
        fn_iconv_open  = os2_iconv_open;
        fn_iconv       = os2_iconv;
        fn_iconv_close = os2_iconv_close;
    }
}


/* Public routines.
 * ----------------
 */

/* Non-standard function for iconv to unload the used dynamic library */
void libiconv_clean(void)
{
    if (hmIconv != NULLHANDLE) {
        DosFreeModule(hmIconv);
        hmIconv = NULLHANDLE;

        fn_iconv_open  = NULL;
        fn_iconv       = NULL;
        fn_iconv_close = NULL;
    }
}

iconv_t libiconv_open(const char* tocode, const char* fromcode)
{
    _init();
    return fn_iconv_open(tocode, fromcode);
}

size_t libiconv(iconv_t cd, char* * inbuf, size_t *inbytesleft,
                char* * outbuf, size_t *outbytesleft)
{
    return fn_iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
}

int libiconv_close(iconv_t cd)
{
    return fn_iconv_close(cd);
}

/* vi: set ts=4 sw=4 expandtab: */
