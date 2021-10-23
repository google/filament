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

#include "../../SDL_internal.h"
#include "../../core/windows/SDL_windows.h"
#include "../SDL_syslocale.h"

typedef BOOL (WINAPI *pfnGetUserPreferredUILanguages)(DWORD,PULONG,WCHAR*,PULONG);
#ifndef MUI_LANGUAGE_NAME
#define MUI_LANGUAGE_NAME 0x8
#endif

static pfnGetUserPreferredUILanguages pGetUserPreferredUILanguages = NULL;
static HMODULE kernel32 = 0;


/* this is the fallback for WinXP...one language, not a list. */
static void
SDL_SYS_GetPreferredLocales_winxp(char *buf, size_t buflen)
{
    char lang[16];
    char country[16];

    const int langrc = GetLocaleInfoA(LOCALE_USER_DEFAULT,
                                      LOCALE_SISO639LANGNAME,
                                      lang, sizeof (lang));

    const int ctryrc =  GetLocaleInfoA(LOCALE_USER_DEFAULT,
                                       LOCALE_SISO3166CTRYNAME,
                                       country, sizeof (country));

    /* Win95 systems will fail, because they don't have LOCALE_SISO*NAME ... */
    if (langrc == 0) {
        SDL_SetError("Couldn't obtain language info");
    } else {
        SDL_snprintf(buf, buflen, "%s%s%s", lang, ctryrc ? "_" : "", ctryrc ? country : "");
    }
}

/* this works on Windows Vista and later. */
static void
SDL_SYS_GetPreferredLocales_vista(char *buf, size_t buflen)
{
    ULONG numlangs = 0;
    WCHAR *wbuf = NULL;
    ULONG wbuflen = 0;
    SDL_bool isstack;

    SDL_assert(pGetUserPreferredUILanguages != NULL);
    pGetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numlangs, NULL, &wbuflen);

    wbuf = SDL_small_alloc(WCHAR, wbuflen, &isstack);
    if (!wbuf) {
        SDL_OutOfMemory();
        return;
    }

    if (!pGetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &numlangs, wbuf, &wbuflen)) {
        SDL_SYS_GetPreferredLocales_winxp(buf, buflen);  /* oh well, try the fallback. */
    } else {
        const ULONG endidx = (ULONG) SDL_min(buflen, wbuflen - 1);
        ULONG str_start = 0;
        ULONG i;
        for (i = 0; i < endidx; i++) {
            const char ch = (char) wbuf[i];  /* these should all be low-ASCII, safe to cast */
            if (ch == '\0') {
                buf[i] = ',';  /* change null separators to commas */
                str_start = i;
            } else if (ch == '-') {
                buf[i] = '_';  /* change '-' to '_' */
            } else {
                buf[i] = ch;   /* copy through as-is. */
            }
        }
        buf[str_start] = '\0';  /* terminate string, chop off final ',' */
    }

    SDL_small_free(wbuf, isstack);
}

void
SDL_SYS_GetPreferredLocales(char *buf, size_t buflen)
{
    if (!kernel32) {
        kernel32 = GetModuleHandle(TEXT("kernel32.dll"));
        if (kernel32) {
            pGetUserPreferredUILanguages = (pfnGetUserPreferredUILanguages) GetProcAddress(kernel32, "GetUserPreferredUILanguages");
        }
    }

    if (pGetUserPreferredUILanguages == NULL) {
        SDL_SYS_GetPreferredLocales_winxp(buf, buflen);  /* this is always available */
    } else {
        SDL_SYS_GetPreferredLocales_vista(buf, buflen);  /* available on Vista and later. */
    }
}

/* vi: set ts=4 sw=4 expandtab: */

