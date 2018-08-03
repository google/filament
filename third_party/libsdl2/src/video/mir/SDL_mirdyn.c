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
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MIR

#define DEBUG_DYNAMIC_MIR 0

#include "SDL_mirdyn.h"

#if DEBUG_DYNAMIC_MIR
#include "SDL_log.h"
#endif

#ifdef SDL_VIDEO_DRIVER_MIR_DYNAMIC

#include "SDL_name.h"
#include "SDL_loadso.h"

typedef struct
{
    void *lib;
    const char *libname;
} mirdynlib;

#ifndef SDL_VIDEO_DRIVER_MIR_DYNAMIC
#define SDL_VIDEO_DRIVER_MIR_DYNAMIC NULL
#endif
#ifndef SDL_VIDEO_DRIVER_MIR_DYNAMIC_XKBCOMMON
#define SDL_VIDEO_DRIVER_MIR_DYNAMIC_XKBCOMMON NULL
#endif

static mirdynlib mirlibs[] = {
    {NULL, SDL_VIDEO_DRIVER_MIR_DYNAMIC},
    {NULL, SDL_VIDEO_DRIVER_MIR_DYNAMIC_XKBCOMMON}
};

static void *
MIR_GetSym(const char *fnname, int *pHasModule)
{
    int i;
    void *fn = NULL;
    for (i = 0; i < SDL_TABLESIZE(mirlibs); i++) {
        if (mirlibs[i].lib != NULL) {
            fn = SDL_LoadFunction(mirlibs[i].lib, fnname);
            if (fn != NULL)
                break;
        }
    }

#if DEBUG_DYNAMIC_MIR
    if (fn != NULL)
        SDL_Log("MIR: Found '%s' in %s (%p)\n", fnname, mirlibs[i].libname, fn);
    else
        SDL_Log("MIR: Symbol '%s' NOT FOUND!\n", fnname);
#endif

    if (fn == NULL)
        *pHasModule = 0;  /* kill this module. */

    return fn;
}

#endif /* SDL_VIDEO_DRIVER_MIR_DYNAMIC */

/* Define all the function pointers and wrappers... */
#define SDL_MIR_MODULE(modname) int SDL_MIR_HAVE_##modname = 0;
#define SDL_MIR_SYM(rc,fn,params) SDL_DYNMIRFN_##fn MIR_##fn = NULL;
#define SDL_MIR_SYM_CONST(type,name) SDL_DYMMIRCONST_##name MIR_##name = NULL;
#include "SDL_mirsym.h"

static int mir_load_refcount = 0;

void
SDL_MIR_UnloadSymbols(void)
{
    /* Don't actually unload if more than one module is using the libs... */
    if (mir_load_refcount > 0) {
        if (--mir_load_refcount == 0) {
#ifdef SDL_VIDEO_DRIVER_MIR_DYNAMIC            
            int i;
#endif
            
            /* set all the function pointers to NULL. */
#define SDL_MIR_MODULE(modname) SDL_MIR_HAVE_##modname = 0;
#define SDL_MIR_SYM(rc,fn,params) MIR_##fn = NULL;
#define SDL_MIR_SYM_CONST(type,name) MIR_##name = NULL;
#include "SDL_mirsym.h"


#ifdef SDL_VIDEO_DRIVER_MIR_DYNAMIC
            for (i = 0; i < SDL_TABLESIZE(mirlibs); i++) {
                if (mirlibs[i].lib != NULL) {
                    SDL_UnloadObject(mirlibs[i].lib);
                    mirlibs[i].lib = NULL;
                }
            }
#endif
        }
    }
}

/* returns non-zero if all needed symbols were loaded. */
int
SDL_MIR_LoadSymbols(void)
{
    int rc = 1;                 /* always succeed if not using Dynamic MIR stuff. */

    /* deal with multiple modules (dga, wayland, mir, etc) needing these symbols... */
    if (mir_load_refcount++ == 0) {
#ifdef SDL_VIDEO_DRIVER_MIR_DYNAMIC
        int i;
        int *thismod = NULL;
        for (i = 0; i < SDL_TABLESIZE(mirlibs); i++) {
            if (mirlibs[i].libname != NULL) {
                mirlibs[i].lib = SDL_LoadObject(mirlibs[i].libname);
            }
        }

#define SDL_MIR_MODULE(modname) SDL_MIR_HAVE_##modname = 1; /* default yes */
#include "SDL_mirsym.h"

#define SDL_MIR_MODULE(modname) thismod = &SDL_MIR_HAVE_##modname;
#define SDL_MIR_SYM(rc,fn,params) MIR_##fn = (SDL_DYNMIRFN_##fn) MIR_GetSym(#fn,thismod);
#define SDL_MIR_SYM_CONST(type,name) MIR_##name = *(SDL_DYMMIRCONST_##name*) MIR_GetSym(#name,thismod);
#include "SDL_mirsym.h"

        if ((SDL_MIR_HAVE_MIR_CLIENT) && (SDL_MIR_HAVE_XKBCOMMON)) {
            /* all required symbols loaded. */
            SDL_ClearError();
        } else {
            /* in case something got loaded... */
            SDL_MIR_UnloadSymbols();
            rc = 0;
        }

#else  /* no dynamic MIR */

#define SDL_MIR_MODULE(modname) SDL_MIR_HAVE_##modname = 1; /* default yes */
#define SDL_MIR_SYM(rc,fn,params) MIR_##fn = fn;
#define SDL_MIR_SYM_CONST(type,name) MIR_##name = name;
#include "SDL_mirsym.h"

#endif
    }

    return rc;
}

#endif /* SDL_VIDEO_DRIVER_MIR */

/* vi: set ts=4 sw=4 expandtab: */
