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
#include "../SDL_internal.h"

/* Useful functions and variables from SDL_events.c */
#include "SDL_events.h"
#include "SDL_thread.h"
#include "SDL_clipboardevents_c.h"
#include "SDL_dropevents_c.h"
#include "SDL_gesture_c.h"
#include "SDL_keyboard_c.h"
#include "SDL_mouse_c.h"
#include "SDL_touch_c.h"
#include "SDL_windowevents_c.h"

/* Start and stop the event processing loop */
extern int SDL_StartEventLoop(void);
extern void SDL_StopEventLoop(void);
extern void SDL_QuitInterrupt(void);

extern int SDL_SendAppEvent(SDL_EventType eventType);
extern int SDL_SendSysWMEvent(SDL_SysWMmsg * message);
extern int SDL_SendKeymapChangedEvent(void);

extern int SDL_QuitInit(void);
extern int SDL_SendQuit(void);
extern void SDL_QuitQuit(void);

extern void SDL_SendPendingQuit(void);

/* vi: set ts=4 sw=4 expandtab: */
