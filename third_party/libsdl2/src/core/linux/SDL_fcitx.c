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

#ifdef HAVE_FCITX_FRONTEND_H

#include <fcitx/frontend.h>
#include <unistd.h>

#include "SDL_fcitx.h"
#include "SDL_keycode.h"
#include "SDL_keyboard.h"
#include "../../events/SDL_keyboard_c.h"
#include "SDL_dbus.h"
#include "SDL_syswm.h"
#if SDL_VIDEO_DRIVER_X11
#  include "../../video/x11/SDL_x11video.h"
#endif
#include "SDL_hints.h"

#define FCITX_DBUS_SERVICE "org.fcitx.Fcitx"

#define FCITX_IM_DBUS_PATH "/inputmethod"
#define FCITX_IC_DBUS_PATH "/inputcontext_%d"

#define FCITX_IM_DBUS_INTERFACE "org.fcitx.Fcitx.InputMethod"
#define FCITX_IC_DBUS_INTERFACE "org.fcitx.Fcitx.InputContext"

#define IC_NAME_MAX 64
#define DBUS_TIMEOUT 500

typedef struct _FcitxClient
{
    SDL_DBusContext *dbus;

    char servicename[IC_NAME_MAX];
    char icname[IC_NAME_MAX];

    int id;

    SDL_Rect cursor_rect;
} FcitxClient;

static FcitxClient fcitx_client;

static int
GetDisplayNumber()
{
    const char *display = SDL_getenv("DISPLAY");
    const char *p = NULL;
    int number = 0;

    if (display == NULL)
        return 0;

    display = SDL_strchr(display, ':');
    if (display == NULL)
        return 0;

    display++;
    p = SDL_strchr(display, '.');
    if (p == NULL && display != NULL) {
        number = SDL_strtod(display, NULL);
    } else {
        char *buffer = SDL_strdup(display);
        buffer[p - display] = '\0';
        number = SDL_strtod(buffer, NULL);
        SDL_free(buffer);
    }

    return number;
}

static char*
GetAppName()
{
#if defined(__LINUX__) || defined(__FREEBSD__)
    char *spot;
    char procfile[1024];
    char linkfile[1024];
    int linksize;

#if defined(__LINUX__)
    SDL_snprintf(procfile, sizeof(procfile), "/proc/%d/exe", getpid());
#elif defined(__FREEBSD__)
    SDL_snprintf(procfile, sizeof(procfile), "/proc/%d/file", getpid());
#endif
    linksize = readlink(procfile, linkfile, sizeof(linkfile) - 1);
    if (linksize > 0) {
        linkfile[linksize] = '\0';
        spot = SDL_strrchr(linkfile, '/');
        if (spot) {
            return SDL_strdup(spot + 1);
        } else {
            return SDL_strdup(linkfile);
        }
    }
#endif /* __LINUX__ || __FREEBSD__ */

    return SDL_strdup("SDL_App");
}

static DBusHandlerResult
DBus_MessageFilter(DBusConnection *conn, DBusMessage *msg, void *data)
{
    SDL_DBusContext *dbus = (SDL_DBusContext *)data;

    if (dbus->message_is_signal(msg, FCITX_IC_DBUS_INTERFACE, "CommitString")) {
        DBusMessageIter iter;
        const char *text = NULL;

        dbus->message_iter_init(msg, &iter);
        dbus->message_iter_get_basic(&iter, &text);

        if (text)
            SDL_SendKeyboardText(text);

        return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (dbus->message_is_signal(msg, FCITX_IC_DBUS_INTERFACE, "UpdatePreedit")) {
        DBusMessageIter iter;
        const char *text;

        dbus->message_iter_init(msg, &iter);
        dbus->message_iter_get_basic(&iter, &text);

        if (text && *text) {
            char buf[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
            size_t text_bytes = SDL_strlen(text), i = 0;
            size_t cursor = 0;

            while (i < text_bytes) {
                const size_t sz = SDL_utf8strlcpy(buf, text + i, sizeof(buf));
                const size_t chars = SDL_utf8strlen(buf);

                SDL_SendEditingText(buf, cursor, chars);

                i += sz;
                cursor += chars;
            }
        }

        SDL_Fcitx_UpdateTextRect(NULL);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
FcitxClientICCallMethod(FcitxClient *client, const char *method)
{
    SDL_DBus_CallVoidMethod(client->servicename, client->icname, FCITX_IC_DBUS_INTERFACE, method, DBUS_TYPE_INVALID);
}

static void SDLCALL
Fcitx_SetCapabilities(void *data,
        const char *name,
        const char *old_val,
        const char *internal_editing)
{
    FcitxClient *client = (FcitxClient *)data;
    Uint32 caps = CAPACITY_NONE;

    if (!(internal_editing && *internal_editing == '1')) {
        caps |= CAPACITY_PREEDIT;
    }

    SDL_DBus_CallVoidMethod(client->servicename, client->icname, FCITX_IC_DBUS_INTERFACE, "SetCapacity", DBUS_TYPE_UINT32, &caps, DBUS_TYPE_INVALID);
}

static SDL_bool
FcitxClientCreateIC(FcitxClient *client)
{
    char *appname = GetAppName();
    pid_t pid = getpid();
    int id = -1;
    Uint32 enable, arg1, arg2, arg3, arg4;

    if (!SDL_DBus_CallMethod(client->servicename, FCITX_IM_DBUS_PATH, FCITX_IM_DBUS_INTERFACE, "CreateICv3",
            DBUS_TYPE_STRING, &appname, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INVALID,
            DBUS_TYPE_INT32, &id, DBUS_TYPE_BOOLEAN, &enable, DBUS_TYPE_UINT32, &arg1, DBUS_TYPE_UINT32, &arg2, DBUS_TYPE_UINT32, &arg3, DBUS_TYPE_UINT32, &arg4, DBUS_TYPE_INVALID)) {
        id = -1;  /* just in case. */
    }

    SDL_free(appname);

    if (id >= 0) {
        SDL_DBusContext *dbus = client->dbus;

        client->id = id;

        SDL_snprintf(client->icname, IC_NAME_MAX, FCITX_IC_DBUS_PATH, client->id);

        dbus->bus_add_match(dbus->session_conn,
                "type='signal', interface='org.fcitx.Fcitx.InputContext'",
                NULL);
        dbus->connection_add_filter(dbus->session_conn,
                &DBus_MessageFilter, dbus,
                NULL);
        dbus->connection_flush(dbus->session_conn);

        SDL_AddHintCallback(SDL_HINT_IME_INTERNAL_EDITING, Fcitx_SetCapabilities, client);
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

static Uint32
Fcitx_ModState(void)
{
    Uint32 fcitx_mods = 0;
    SDL_Keymod sdl_mods = SDL_GetModState();

    if (sdl_mods & KMOD_SHIFT) fcitx_mods |= FcitxKeyState_Shift;
    if (sdl_mods & KMOD_CAPS)   fcitx_mods |= FcitxKeyState_CapsLock;
    if (sdl_mods & KMOD_CTRL)  fcitx_mods |= FcitxKeyState_Ctrl;
    if (sdl_mods & KMOD_ALT)   fcitx_mods |= FcitxKeyState_Alt;
    if (sdl_mods & KMOD_NUM)    fcitx_mods |= FcitxKeyState_NumLock;
    if (sdl_mods & KMOD_LGUI)   fcitx_mods |= FcitxKeyState_Super;
    if (sdl_mods & KMOD_RGUI)   fcitx_mods |= FcitxKeyState_Meta;

    return fcitx_mods;
}

SDL_bool
SDL_Fcitx_Init()
{
    fcitx_client.dbus = SDL_DBus_GetContext();

    fcitx_client.cursor_rect.x = -1;
    fcitx_client.cursor_rect.y = -1;
    fcitx_client.cursor_rect.w = 0;
    fcitx_client.cursor_rect.h = 0;

    SDL_snprintf(fcitx_client.servicename, IC_NAME_MAX,
            "%s-%d",
            FCITX_DBUS_SERVICE, GetDisplayNumber());

    return FcitxClientCreateIC(&fcitx_client);
}

void
SDL_Fcitx_Quit()
{
    FcitxClientICCallMethod(&fcitx_client, "DestroyIC");
}

void
SDL_Fcitx_SetFocus(SDL_bool focused)
{
    if (focused) {
        FcitxClientICCallMethod(&fcitx_client, "FocusIn");
    } else {
        FcitxClientICCallMethod(&fcitx_client, "FocusOut");
    }
}

void
SDL_Fcitx_Reset(void)
{
    FcitxClientICCallMethod(&fcitx_client, "Reset");
    FcitxClientICCallMethod(&fcitx_client, "CloseIC");
}

SDL_bool
SDL_Fcitx_ProcessKeyEvent(Uint32 keysym, Uint32 keycode)
{
    Uint32 state = Fcitx_ModState();
    Uint32 handled = SDL_FALSE;
    int type = FCITX_PRESS_KEY;
    Uint32 event_time = 0;

    if (SDL_DBus_CallMethod(fcitx_client.servicename, fcitx_client.icname, FCITX_IC_DBUS_INTERFACE, "ProcessKeyEvent",
            DBUS_TYPE_UINT32, &keysym, DBUS_TYPE_UINT32, &keycode, DBUS_TYPE_UINT32, &state, DBUS_TYPE_INT32, &type, DBUS_TYPE_UINT32, &event_time, DBUS_TYPE_INVALID,
            DBUS_TYPE_INT32, &handled, DBUS_TYPE_INVALID)) {
        if (handled) {
            SDL_Fcitx_UpdateTextRect(NULL);
            return SDL_TRUE;
        }
    }

    return SDL_FALSE;
}

void
SDL_Fcitx_UpdateTextRect(SDL_Rect *rect)
{
    SDL_Window *focused_win = NULL;
    SDL_SysWMinfo info;
    int x = 0, y = 0;
    SDL_Rect *cursor = &fcitx_client.cursor_rect;

    if (rect) {
        SDL_memcpy(cursor, rect, sizeof(SDL_Rect));
    }

    focused_win = SDL_GetKeyboardFocus();
    if (!focused_win) {
        return ;
    }

    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(focused_win, &info)) {
        return;
    }

    SDL_GetWindowPosition(focused_win, &x, &y);

#if SDL_VIDEO_DRIVER_X11
    if (info.subsystem == SDL_SYSWM_X11) {
        SDL_DisplayData *displaydata = (SDL_DisplayData *) SDL_GetDisplayForWindow(focused_win)->driverdata;

        Display *x_disp = info.info.x11.display;
        Window x_win = info.info.x11.window;
        int x_screen = displaydata->screen;
        Window unused;
        X11_XTranslateCoordinates(x_disp, x_win, RootWindow(x_disp, x_screen), 0, 0, &x, &y, &unused);
    }
#endif

    if (cursor->x == -1 && cursor->y == -1 && cursor->w == 0 && cursor->h == 0) {
        /* move to bottom left */
        int w = 0, h = 0;
        SDL_GetWindowSize(focused_win, &w, &h);
        cursor->x = 0;
        cursor->y = h;
    }

    x += cursor->x;
    y += cursor->y;

    SDL_DBus_CallVoidMethod(fcitx_client.servicename, fcitx_client.icname, FCITX_IC_DBUS_INTERFACE, "SetCursorRect",
        DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, DBUS_TYPE_INT32, &cursor->w, DBUS_TYPE_INT32, &cursor->h, DBUS_TYPE_INVALID);
}

void
SDL_Fcitx_PumpEvents(void)
{
    SDL_DBusContext *dbus = fcitx_client.dbus;
    DBusConnection *conn = dbus->session_conn;

    dbus->connection_read_write(conn, 0);

    while (dbus->connection_dispatch(conn) == DBUS_DISPATCH_DATA_REMAINS) {
        /* Do nothing, actual work happens in DBus_MessageFilter */
        usleep(10);
    }
}

#endif /* HAVE_FCITX_FRONTEND_H */

/* vi: set ts=4 sw=4 expandtab: */
