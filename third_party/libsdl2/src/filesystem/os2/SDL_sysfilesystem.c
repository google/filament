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

#ifdef SDL_FILESYSTEM_OS2

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* System dependent filesystem routines                                */

#include "SDL_error.h"
#include "SDL_filesystem.h"
#include "../../core/os2/SDL_os2.h"

#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>


char *
SDL_GetBasePath(void)
{
    PTIB    tib;
    PPIB    pib;
    ULONG   ulRC = DosGetInfoBlocks(&tib, &pib);
    PCHAR   pcEnd;
    ULONG   cbResult;
    CHAR    acBuf[_MAX_PATH];

    if (ulRC != NO_ERROR) {
        debug_os2("DosGetInfoBlocks() failed, rc = %u", ulRC);
        return NULL;
    }

    pcEnd = SDL_strrchr(pib->pib_pchcmd, '\\');
    if (pcEnd != NULL)
        pcEnd++;
    else {
        if (pib->pib_pchcmd[1] == ':')
            pcEnd = &pib->pib_pchcmd[2];
        else {
            SDL_SetError("No path in pib->pib_pchcmd");
            return NULL;
        }
    }

    cbResult = pcEnd - pib->pib_pchcmd;
    SDL_memcpy(acBuf, pib->pib_pchcmd, cbResult);
    acBuf[cbResult] = '\0';

    return OS2_SysToUTF8(acBuf);
}

char *
SDL_GetPrefPath(const char *org, const char *app)
{
    PSZ     pszPath;
    CHAR    acBuf[_MAX_PATH];
    int     lPosApp, lPosOrg;
    PSZ     pszApp, pszOrg;

    if (!app) {
        SDL_InvalidParamError("app");
        return NULL;
    }

    pszPath = SDL_getenv("HOME");
    if (!pszPath) {
        pszPath = SDL_getenv("ETC");
        if (!pszPath) {
            SDL_SetError("HOME or ETC environment not set");
            return NULL;
        }
    }

    if (!org) {
        lPosApp = SDL_snprintf(acBuf, sizeof(acBuf) - 1, "%s", pszPath);
    } else {
        pszOrg = OS2_UTF8ToSys(org);
        if (!pszOrg) {
            SDL_OutOfMemory();
            return NULL;
        }
        lPosApp = SDL_snprintf(acBuf, sizeof(acBuf) - 1, "%s\\%s", pszPath, pszOrg);
        SDL_free(pszOrg);
    }
    if (lPosApp < 0)
        return NULL;

    DosCreateDir(acBuf, NULL);

    pszApp = OS2_UTF8ToSys(app);
    if (!pszApp) {
        SDL_OutOfMemory();
        return NULL;
    }

    lPosOrg = SDL_snprintf(&acBuf[lPosApp], sizeof(acBuf) - lPosApp - 1, "\\%s", pszApp);
    SDL_free(pszApp);
    if (lPosOrg < 0)
        return NULL;

    DosCreateDir(acBuf, NULL);
    *((PUSHORT)&acBuf[lPosApp + lPosOrg]) = (USHORT)'\0\\';

    return OS2_SysToUTF8(acBuf);
}

#endif /* SDL_FILESYSTEM_OS2 */

/* vi: set ts=4 sw=4 expandtab: */
