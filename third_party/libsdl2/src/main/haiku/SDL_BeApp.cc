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

#if defined(__HAIKU__)

/* Handle the BeApp specific portions of the application */

#include <AppKit.h>
#include <storage/AppFileInfo.h>
#include <storage/Path.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <unistd.h>

#include "SDL_BApp.h"	/* SDL_BApp class definition */
#include "SDL_BeApp.h"
#include "SDL_timer.h"
#include "SDL_error.h"

#include "../../video/haiku/SDL_BWin.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../thread/SDL_systhread.h"

/* Flag to tell whether or not the Be application is active or not */
static int SDL_BeAppActive = 0;
static SDL_Thread *SDL_AppThread = NULL;

static int
StartBeApp(void *unused)
{
    BApplication *App;

	// default application signature
	const char *signature = "application/x-SDL-executable";
	// dig resources for correct signature
	image_info info;
	int32 cookie = 0;
	if (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
		BFile f(info.name, O_RDONLY);
		if (f.InitCheck() == B_OK) {
			BAppFileInfo app_info(&f);
			if (app_info.InitCheck() == B_OK) {
				char sig[B_MIME_TYPE_LENGTH];
				if (app_info.GetSignature(sig) == B_OK)
					signature = strndup(sig, B_MIME_TYPE_LENGTH);
			}
		}
	}

	App = new SDL_BApp(signature);

    App->Run();
    delete App;
    return (0);
}

/* Initialize the Be Application, if it's not already started */
int
SDL_InitBeApp(void)
{
    /* Create the BApplication that handles appserver interaction */
    if (SDL_BeAppActive <= 0) {
        SDL_AppThread = SDL_CreateThreadInternal(StartBeApp, "SDLApplication", 0, NULL);
        if (SDL_AppThread == NULL) {
            return SDL_SetError("Couldn't create BApplication thread");
        }

        /* Change working directory to that of executable */
        app_info info;
        if (B_OK == be_app->GetAppInfo(&info)) {
            entry_ref ref = info.ref;
            BEntry entry;
            if (B_OK == entry.SetTo(&ref)) {
                BPath path;
                if (B_OK == path.SetTo(&entry)) {
                    if (B_OK == path.GetParent(&path)) {
                        chdir(path.Path());
                    }
                }
            }
        }

        do {
            SDL_Delay(10);
        } while ((be_app == NULL) || be_app->IsLaunching());

        /* Mark the application active */
        SDL_BeAppActive = 0;
    }

    /* Increment the application reference count */
    ++SDL_BeAppActive;

    /* The app is running, and we're ready to go */
    return (0);
}

/* Quit the Be Application, if there's nothing left to do */
void
SDL_QuitBeApp(void)
{
    /* Decrement the application reference count */
    --SDL_BeAppActive;

    /* If the reference count reached zero, clean up the app */
    if (SDL_BeAppActive == 0) {
        if (SDL_AppThread != NULL) {
            if (be_app != NULL) {       /* Not tested */
                be_app->PostMessage(B_QUIT_REQUESTED);
            }
            SDL_WaitThread(SDL_AppThread, NULL);
            SDL_AppThread = NULL;
        }
        /* be_app should now be NULL since be_app has quit */
    }
}

#ifdef __cplusplus
}
#endif

/* SDL_BApp functions */
void SDL_BApp::ClearID(SDL_BWin *bwin) {
	_SetSDLWindow(NULL, bwin->GetID());
	int32 i = _GetNumWindowSlots() - 1;
	while(i >= 0 && GetSDLWindow(i) == NULL) {
		_PopBackWindow();
		--i;
	}
}

#endif /* __HAIKU__ */

/* vi: set ts=4 sw=4 expandtab: */
