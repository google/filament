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
#ifndef SDL_BAPP_H
#define SDL_BAPP_H

#include <InterfaceKit.h>
#if SDL_VIDEO_OPENGL
#include <OpenGLKit.h>
#endif

#include "../../video/haiku/SDL_bkeyboard.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "../../SDL_internal.h"

#include "SDL_video.h"

/* Local includes */
#include "../../events/SDL_events_c.h"
#include "../../video/haiku/SDL_bframebuffer.h"

#ifdef __cplusplus
}
#endif

#include <vector>




/* Forward declarations */
class SDL_BWin;

/* Message constants */
enum ToSDL {
    /* Intercepted by BWindow on its way to BView */
    BAPP_MOUSE_MOVED,
    BAPP_MOUSE_BUTTON,
    BAPP_MOUSE_WHEEL,
    BAPP_KEY,
    BAPP_REPAINT,           /* from _UPDATE_ */
    /* From BWindow */
    BAPP_MAXIMIZE,          /* from B_ZOOM */
    BAPP_MINIMIZE,
    BAPP_RESTORE,           /* TODO: IMPLEMENT! */
    BAPP_SHOW,
    BAPP_HIDE,
    BAPP_MOUSE_FOCUS,       /* caused by MOUSE_MOVE */
    BAPP_KEYBOARD_FOCUS,    /* from WINDOW_ACTIVATED */
    BAPP_WINDOW_CLOSE_REQUESTED,
    BAPP_WINDOW_MOVED,
    BAPP_WINDOW_RESIZED,
    BAPP_SCREEN_CHANGED
};



/* Create a descendant of BApplication */
class SDL_BApp : public BApplication {
public:
    SDL_BApp(const char* signature) :
        BApplication(signature) {
#if SDL_VIDEO_OPENGL
        _current_context = NULL;
#endif
    }


    virtual ~SDL_BApp() {
    }



        /* Event-handling functions */
    virtual void MessageReceived(BMessage* message) {
        /* Sort out SDL-related messages */
        switch ( message->what ) {
        case BAPP_MOUSE_MOVED:
            _HandleMouseMove(message);
            break;

        case BAPP_MOUSE_BUTTON:
            _HandleMouseButton(message);
            break;

        case BAPP_MOUSE_WHEEL:
            _HandleMouseWheel(message);
            break;

        case BAPP_KEY:
            _HandleKey(message);
            break;

        case BAPP_REPAINT:
            _HandleBasicWindowEvent(message, SDL_WINDOWEVENT_EXPOSED);
            break;

        case BAPP_MAXIMIZE:
            _HandleBasicWindowEvent(message, SDL_WINDOWEVENT_MAXIMIZED);
            break;

        case BAPP_MINIMIZE:
            _HandleBasicWindowEvent(message, SDL_WINDOWEVENT_MINIMIZED);
            break;

        case BAPP_SHOW:
            _HandleBasicWindowEvent(message, SDL_WINDOWEVENT_SHOWN);
            break;

        case BAPP_HIDE:
            _HandleBasicWindowEvent(message, SDL_WINDOWEVENT_HIDDEN);
            break;

        case BAPP_MOUSE_FOCUS:
            _HandleMouseFocus(message);
            break;

        case BAPP_KEYBOARD_FOCUS:
            _HandleKeyboardFocus(message);
            break;

        case BAPP_WINDOW_CLOSE_REQUESTED:
            _HandleBasicWindowEvent(message, SDL_WINDOWEVENT_CLOSE);
            break;

        case BAPP_WINDOW_MOVED:
            _HandleWindowMoved(message);
            break;

        case BAPP_WINDOW_RESIZED:
            _HandleWindowResized(message);
            break;

        case BAPP_SCREEN_CHANGED:
            /* TODO: Handle screen resize or workspace change */
            break;

        default:
           BApplication::MessageReceived(message);
           break;
        }
    }

    /* Window creation/destruction methods */
    int32 GetID(SDL_Window *win) {
        int32 i;
        for(i = 0; i < _GetNumWindowSlots(); ++i) {
            if( GetSDLWindow(i) == NULL ) {
                _SetSDLWindow(win, i);
                return i;
            }
        }

        /* Expand the vector if all slots are full */
        if( i == _GetNumWindowSlots() ) {
            _PushBackWindow(win);
            return i;
        }

        /* TODO: error handling */
        return 0;
    }

    /* FIXME: Bad coding practice, but I can't include SDL_BWin.h here.  Is
       there another way to do this? */
    void ClearID(SDL_BWin *bwin); /* Defined in SDL_BeApp.cc */


    SDL_Window *GetSDLWindow(int32 winID) {
        return _window_map[winID];
    }

#if SDL_VIDEO_OPENGL
    void SetCurrentContext(BGLView *newContext) {
        if(_current_context)
            _current_context->UnlockGL();
        _current_context = newContext;
        if (_current_context)
	        _current_context->LockGL();
    }
#endif

private:
    /* Event management */
    void _HandleBasicWindowEvent(BMessage *msg, int32 sdlEventType) {
        SDL_Window *win;
        int32 winID;
        if(
            !_GetWinID(msg, &winID)
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        SDL_SendWindowEvent(win, sdlEventType, 0, 0);
    }

    void _HandleMouseMove(BMessage *msg) {
        SDL_Window *win;
        int32 winID;
        int32 x = 0, y = 0;
        if(
            !_GetWinID(msg, &winID) ||
            msg->FindInt32("x", &x) != B_OK || /* x movement */
            msg->FindInt32("y", &y) != B_OK    /* y movement */
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        SDL_SendMouseMotion(win, 0, 0, x, y);

        /* Tell the application that the mouse passed over, redraw needed */
        BE_UpdateWindowFramebuffer(NULL,win,NULL,-1);
    }

    void _HandleMouseButton(BMessage *msg) {
        SDL_Window *win;
        int32 winID;
        int32 button, state;    /* left/middle/right, pressed/released */
        if(
            !_GetWinID(msg, &winID) ||
            msg->FindInt32("button-id", &button) != B_OK ||
            msg->FindInt32("button-state", &state) != B_OK
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        SDL_SendMouseButton(win, 0, state, button);
    }

    void _HandleMouseWheel(BMessage *msg) {
        SDL_Window *win;
        int32 winID;
        int32 xTicks, yTicks;
        if(
            !_GetWinID(msg, &winID) ||
            msg->FindInt32("xticks", &xTicks) != B_OK ||
            msg->FindInt32("yticks", &yTicks) != B_OK
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        SDL_SendMouseWheel(win, 0, xTicks, yTicks, SDL_MOUSEWHEEL_NORMAL);
    }

    void _HandleKey(BMessage *msg) {
        int32 scancode, state;  /* scancode, pressed/released */
        if(
            msg->FindInt32("key-state", &state) != B_OK ||
            msg->FindInt32("key-scancode", &scancode) != B_OK
        ) {
            return;
        }

        /* Make sure this isn't a repeated event (key pressed and held) */
        if(state == SDL_PRESSED && BE_GetKeyState(scancode) == SDL_PRESSED) {
            return;
        }
        BE_SetKeyState(scancode, state);
        SDL_SendKeyboardKey(state, BE_GetScancodeFromBeKey(scancode));
        
        if (state == SDL_PRESSED && SDL_EventState(SDL_TEXTINPUT, SDL_QUERY)) {
            const int8 *keyUtf8;
            ssize_t count;
            if (msg->FindData("key-utf8", B_INT8_TYPE, (const void**)&keyUtf8, &count) == B_OK) {
                char text[SDL_TEXTINPUTEVENT_TEXT_SIZE];
                SDL_zero(text);
                SDL_memcpy(text, keyUtf8, count);
                SDL_SendKeyboardText(text);
            }
        }
    }

    void _HandleMouseFocus(BMessage *msg) {
        SDL_Window *win;
        int32 winID;
        bool bSetFocus; /* If false, lose focus */
        if(
            !_GetWinID(msg, &winID) ||
            msg->FindBool("focusGained", &bSetFocus) != B_OK
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        if(bSetFocus) {
            SDL_SetMouseFocus(win);
        } else if(SDL_GetMouseFocus() == win) {
            /* Only lose all focus if this window was the current focus */
            SDL_SetMouseFocus(NULL);
        }
    }

    void _HandleKeyboardFocus(BMessage *msg) {
        SDL_Window *win;
        int32 winID;
        bool bSetFocus; /* If false, lose focus */
        if(
            !_GetWinID(msg, &winID) ||
            msg->FindBool("focusGained", &bSetFocus) != B_OK
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        if(bSetFocus) {
            SDL_SetKeyboardFocus(win);
        } else if(SDL_GetKeyboardFocus() == win) {
            /* Only lose all focus if this window was the current focus */
            SDL_SetKeyboardFocus(NULL);
        }
    }

    void _HandleWindowMoved(BMessage *msg) {
        SDL_Window *win;
        int32 winID;
        int32 xPos, yPos;
        /* Get the window id and new x/y position of the window */
        if(
            !_GetWinID(msg, &winID) ||
            msg->FindInt32("window-x", &xPos) != B_OK ||
            msg->FindInt32("window-y", &yPos) != B_OK
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        SDL_SendWindowEvent(win, SDL_WINDOWEVENT_MOVED, xPos, yPos);
    }

    void _HandleWindowResized(BMessage *msg) {
        SDL_Window *win;
        int32 winID;
        int32 w, h;
        /* Get the window id ]and new x/y position of the window */
        if(
            !_GetWinID(msg, &winID) ||
            msg->FindInt32("window-w", &w) != B_OK ||
            msg->FindInt32("window-h", &h) != B_OK
        ) {
            return;
        }
        win = GetSDLWindow(winID);
        SDL_SendWindowEvent(win, SDL_WINDOWEVENT_RESIZED, w, h);
    }

    bool _GetWinID(BMessage *msg, int32 *winID) {
        return msg->FindInt32("window-id", winID) == B_OK;
    }



    /* Vector functions: Wraps vector stuff in case we need to change
       implementation */
    void _SetSDLWindow(SDL_Window *win, int32 winID) {
        _window_map[winID] = win;
    }

    int32 _GetNumWindowSlots() {
        return _window_map.size();
    }


    void _PopBackWindow() {
        _window_map.pop_back();
    }

    void _PushBackWindow(SDL_Window *win) {
        _window_map.push_back(win);
    }


    /* Members */
    std::vector<SDL_Window*> _window_map; /* Keeps track of SDL_Windows by index-id */

#if SDL_VIDEO_OPENGL
    BGLView      *_current_context;
#endif
};

#endif
