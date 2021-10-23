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

#include "../SDL_sysurl.h"
#include "../../core/windows/SDL_windows.h"

#include <shellapi.h>

/* https://msdn.microsoft.com/en-us/library/windows/desktop/bb762153%28v=vs.85%29.aspx */
int
SDL_SYS_OpenURL(const char *url)
{
    WCHAR* wurl;
    HINSTANCE rc;

    /* MSDN says for safety's sake, make sure COM is initialized. */
    const HRESULT hr = WIN_CoInitialize();
    if (FAILED(hr)) {
        return WIN_SetErrorFromHRESULT("CoInitialize failed", hr);
    }

    wurl = WIN_UTF8ToStringW(url);
    if (wurl == NULL) {
        WIN_CoUninitialize();
        return SDL_OutOfMemory();
    }

    /* Success returns value greater than 32. Less is an error. */
    rc = ShellExecuteW(NULL, L"open", wurl, NULL, NULL, SW_SHOWNORMAL);
    SDL_free(wurl);
    WIN_CoUninitialize();
    return (rc > ((HINSTANCE) 32)) ? 0 : WIN_SetError("Couldn't open given URL.");
}

/* vi: set ts=4 sw=4 expandtab: */

