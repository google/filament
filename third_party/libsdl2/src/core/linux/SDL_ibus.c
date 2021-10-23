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

#ifdef HAVE_IBUS_IBUS_H
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_ibus.h"
#include "SDL_dbus.h"
#include "../../video/SDL_sysvideo.h"
#include "../../events/SDL_keyboard_c.h"

#if SDL_VIDEO_DRIVER_X11
    #include "../../video/x11/SDL_x11video.h"
#endif

#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

static const char IBUS_SERVICE[]         = "org.freedesktop.IBus";
static const char IBUS_PATH[]            = "/org/freedesktop/IBus";
static const char IBUS_INTERFACE[]       = "org.freedesktop.IBus";
static const char IBUS_INPUT_INTERFACE[] = "org.freedesktop.IBus.InputContext";

static char *input_ctx_path = NULL;
static SDL_Rect ibus_cursor_rect = { 0, 0, 0, 0 };
static DBusConnection *ibus_conn = NULL;
static char *ibus_addr_file = NULL;
static int inotify_fd = -1, inotify_wd = -1;

static Uint32
IBus_ModState(void)
{
    Uint32 ibus_mods = 0;
    SDL_Keymod sdl_mods = SDL_GetModState();
    
    /* Not sure about MOD3, MOD4 and HYPER mappings */
    if (sdl_mods & KMOD_LSHIFT) ibus_mods |= IBUS_SHIFT_MASK;
    if (sdl_mods & KMOD_CAPS)   ibus_mods |= IBUS_LOCK_MASK;
    if (sdl_mods & KMOD_LCTRL)  ibus_mods |= IBUS_CONTROL_MASK;
    if (sdl_mods & KMOD_LALT)   ibus_mods |= IBUS_MOD1_MASK;
    if (sdl_mods & KMOD_NUM)    ibus_mods |= IBUS_MOD2_MASK;
    if (sdl_mods & KMOD_MODE)   ibus_mods |= IBUS_MOD5_MASK;
    if (sdl_mods & KMOD_LGUI)   ibus_mods |= IBUS_SUPER_MASK;
    if (sdl_mods & KMOD_RGUI)   ibus_mods |= IBUS_META_MASK;

    return ibus_mods;
}

static const char *
IBus_GetVariantText(DBusConnection *conn, DBusMessageIter *iter, SDL_DBusContext *dbus)
{
    /* The text we need is nested weirdly, use dbus-monitor to see the structure better */
    const char *text = NULL;
    const char *struct_id = NULL;
    DBusMessageIter sub1, sub2;

    if (dbus->message_iter_get_arg_type(iter) != DBUS_TYPE_VARIANT) {
        return NULL;
    }
    
    dbus->message_iter_recurse(iter, &sub1);
    
    if (dbus->message_iter_get_arg_type(&sub1) != DBUS_TYPE_STRUCT) {
        return NULL;
    }
    
    dbus->message_iter_recurse(&sub1, &sub2);
    
    if (dbus->message_iter_get_arg_type(&sub2) != DBUS_TYPE_STRING) {
        return NULL;
    }
    
    dbus->message_iter_get_basic(&sub2, &struct_id);
    if (!struct_id || SDL_strncmp(struct_id, "IBusText", sizeof("IBusText")) != 0) {
        return NULL;
    }
    
    dbus->message_iter_next(&sub2);
    dbus->message_iter_next(&sub2);
    
    if (dbus->message_iter_get_arg_type(&sub2) != DBUS_TYPE_STRING) {
        return NULL;
    }
    
    dbus->message_iter_get_basic(&sub2, &text);
    
    return text;
}

static DBusHandlerResult
IBus_MessageHandler(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    SDL_DBusContext *dbus = (SDL_DBusContext *)user_data;
        
    if (dbus->message_is_signal(msg, IBUS_INPUT_INTERFACE, "CommitText")) {
        DBusMessageIter iter;
        const char *text;

        dbus->message_iter_init(msg, &iter);
        
        text = IBus_GetVariantText(conn, &iter, dbus);
        if (text && *text) {
            char buf[SDL_TEXTINPUTEVENT_TEXT_SIZE];
            size_t text_bytes = SDL_strlen(text), i = 0;
            
            while (i < text_bytes) {
                size_t sz = SDL_utf8strlcpy(buf, text+i, sizeof(buf));
                SDL_SendKeyboardText(buf);
                
                i += sz;
            }
        }
        
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    if (dbus->message_is_signal(msg, IBUS_INPUT_INTERFACE, "UpdatePreeditText")) {
        DBusMessageIter iter;
        const char *text;

        dbus->message_iter_init(msg, &iter);
        text = IBus_GetVariantText(conn, &iter, dbus);
        
        if (text) {
            char buf[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
            size_t text_bytes = SDL_strlen(text), i = 0;
            size_t cursor = 0;
            
            do {
                const size_t sz = SDL_utf8strlcpy(buf, text+i, sizeof(buf));
                const size_t chars = SDL_utf8strlen(buf);
                
                SDL_SendEditingText(buf, cursor, chars);

                i += sz;
                cursor += chars;
            } while (i < text_bytes);
        }
        
        SDL_IBus_UpdateTextRect(NULL);
        
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    if (dbus->message_is_signal(msg, IBUS_INPUT_INTERFACE, "HidePreeditText")) {
        SDL_SendEditingText("", 0, 0);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static char *
IBus_ReadAddressFromFile(const char *file_path)
{
    char addr_buf[1024];
    SDL_bool success = SDL_FALSE;
    FILE *addr_file;

    addr_file = fopen(file_path, "r");
    if (!addr_file) {
        return NULL;
    }

    while (fgets(addr_buf, sizeof(addr_buf), addr_file)) {
        if (SDL_strncmp(addr_buf, "IBUS_ADDRESS=", sizeof("IBUS_ADDRESS=")-1) == 0) {
            size_t sz = SDL_strlen(addr_buf);
            if (addr_buf[sz-1] == '\n') addr_buf[sz-1] = 0;
            if (addr_buf[sz-2] == '\r') addr_buf[sz-2] = 0;
            success = SDL_TRUE;
            break;
        }
    }

    fclose(addr_file);

    if (success) {
        return SDL_strdup(addr_buf + (sizeof("IBUS_ADDRESS=") - 1));
    } else {
        return NULL;
    }
}

static char *
IBus_GetDBusAddressFilename(void)
{
    SDL_DBusContext *dbus;
    const char *disp_env;
    char config_dir[PATH_MAX];
    char *display = NULL;
    const char *addr;
    const char *conf_env;
    char *key;
    char file_path[PATH_MAX];
    const char *host;
    char *disp_num, *screen_num;

    if (ibus_addr_file) {
        return SDL_strdup(ibus_addr_file);
    }
    
    dbus = SDL_DBus_GetContext();
    if (!dbus) {
        return NULL;
    }
    
    /* Use this environment variable if it exists. */
    addr = SDL_getenv("IBUS_ADDRESS");
    if (addr && *addr) {
        return SDL_strdup(addr);
    }
    
    /* Otherwise, we have to get the hostname, display, machine id, config dir
       and look up the address from a filepath using all those bits, eek. */
    disp_env = SDL_getenv("DISPLAY");

    if (!disp_env || !*disp_env) {
        display = SDL_strdup(":0.0");
    } else {
        display = SDL_strdup(disp_env);
    }
    
    host = display;
    disp_num   = SDL_strrchr(display, ':');
    screen_num = SDL_strrchr(display, '.');
    
    if (!disp_num) {
        SDL_free(display);
        return NULL;
    }
    
    *disp_num = 0;
    disp_num++;
    
    if (screen_num) {
        *screen_num = 0;
    }
    
    if (!*host) {
        const char *session = SDL_getenv("XDG_SESSION_TYPE");
        if (session != NULL && SDL_strcmp(session, "wayland") == 0) {
            host = "unix-wayland";
        } else {
            host = "unix";
        }
    }
        
    SDL_memset(config_dir, 0, sizeof(config_dir));
    
    conf_env = SDL_getenv("XDG_CONFIG_HOME");
    if (conf_env && *conf_env) {
        SDL_strlcpy(config_dir, conf_env, sizeof(config_dir));
    } else {
        const char *home_env = SDL_getenv("HOME");
        if (!home_env || !*home_env) {
            SDL_free(display);
            return NULL;
        }
        SDL_snprintf(config_dir, sizeof(config_dir), "%s/.config", home_env);
    }
    
    key = dbus->get_local_machine_id();

    SDL_memset(file_path, 0, sizeof(file_path));
    SDL_snprintf(file_path, sizeof(file_path), "%s/ibus/bus/%s-%s-%s", 
                                               config_dir, key, host, disp_num);
    dbus->free(key);
    SDL_free(display);
    
    return SDL_strdup(file_path);
}

static SDL_bool IBus_CheckConnection(SDL_DBusContext *dbus);

static void SDLCALL
IBus_SetCapabilities(void *data, const char *name, const char *old_val,
                                                   const char *internal_editing)
{
    SDL_DBusContext *dbus = SDL_DBus_GetContext();
    
    if (IBus_CheckConnection(dbus)) {
        Uint32 caps = IBUS_CAP_FOCUS;
        if (!(internal_editing && *internal_editing == '1')) {
            caps |= IBUS_CAP_PREEDIT_TEXT;
        }

        SDL_DBus_CallVoidMethodOnConnection(ibus_conn, IBUS_SERVICE, input_ctx_path, IBUS_INPUT_INTERFACE, "SetCapabilities",
                                DBUS_TYPE_UINT32, &caps, DBUS_TYPE_INVALID);
    }
}


static SDL_bool
IBus_SetupConnection(SDL_DBusContext *dbus, const char* addr)
{
    const char *client_name = "SDL2_Application";
    const char *path = NULL;
    SDL_bool result = SDL_FALSE;
    DBusObjectPathVTable ibus_vtable;

    SDL_zero(ibus_vtable);
    ibus_vtable.message_function = &IBus_MessageHandler;

    ibus_conn = dbus->connection_open_private(addr, NULL);

    if (!ibus_conn) {
        return SDL_FALSE;
    }

    dbus->connection_flush(ibus_conn);
    
    if (!dbus->bus_register(ibus_conn, NULL)) {
        ibus_conn = NULL;
        return SDL_FALSE;
    }
    
    dbus->connection_flush(ibus_conn);

    if (SDL_DBus_CallMethodOnConnection(ibus_conn, IBUS_SERVICE, IBUS_PATH, IBUS_INTERFACE, "CreateInputContext",
            DBUS_TYPE_STRING, &client_name, DBUS_TYPE_INVALID,
            DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID)) {
        SDL_free(input_ctx_path);
        input_ctx_path = SDL_strdup(path);
        SDL_AddHintCallback(SDL_HINT_IME_INTERNAL_EDITING, IBus_SetCapabilities, NULL);
        
        dbus->bus_add_match(ibus_conn, "type='signal',interface='org.freedesktop.IBus.InputContext'", NULL);
        dbus->connection_try_register_object_path(ibus_conn, input_ctx_path, &ibus_vtable, dbus, NULL);
        dbus->connection_flush(ibus_conn);
        result = SDL_TRUE;
    }

    SDL_IBus_SetFocus(SDL_GetKeyboardFocus() != NULL);
    SDL_IBus_UpdateTextRect(NULL);
    
    return result;
}

static SDL_bool
IBus_CheckConnection(SDL_DBusContext *dbus)
{
    if (!dbus) return SDL_FALSE;
    
    if (ibus_conn && dbus->connection_get_is_connected(ibus_conn)) {
        return SDL_TRUE;
    }
    
    if (inotify_fd > 0 && inotify_wd > 0) {
        char buf[1024];
        ssize_t readsize = read(inotify_fd, buf, sizeof(buf));
        if (readsize > 0) {
        
            char *p;
            SDL_bool file_updated = SDL_FALSE;
            
            for (p = buf; p < buf + readsize; /**/) {
                struct inotify_event *event = (struct inotify_event*) p;
                if (event->len > 0) {
                    char *addr_file_no_path = SDL_strrchr(ibus_addr_file, '/');
                    if (!addr_file_no_path) return SDL_FALSE;
                 
                    if (SDL_strcmp(addr_file_no_path + 1, event->name) == 0) {
                        file_updated = SDL_TRUE;
                        break;
                    }
                }
                
                p += sizeof(struct inotify_event) + event->len;
            }
            
            if (file_updated) {
                char *addr = IBus_ReadAddressFromFile(ibus_addr_file);
                if (addr) {
                    SDL_bool result = IBus_SetupConnection(dbus, addr);
                    SDL_free(addr);
                    return result;
                }
            }
        }
    }
    
    return SDL_FALSE;
}

SDL_bool
SDL_IBus_Init(void)
{
    SDL_bool result = SDL_FALSE;
    SDL_DBusContext *dbus = SDL_DBus_GetContext();
    
    if (dbus) {
        char *addr_file = IBus_GetDBusAddressFilename();
        char *addr;
        char *addr_file_dir;

        if (!addr_file) {
            return SDL_FALSE;
        }
        
        /* !!! FIXME: if ibus_addr_file != NULL, this will overwrite it and leak (twice!) */
        ibus_addr_file = SDL_strdup(addr_file);
        
        addr = IBus_ReadAddressFromFile(addr_file);
        if (!addr) {
            SDL_free(addr_file);
            return SDL_FALSE;
        }
        
        if (inotify_fd < 0) {
            inotify_fd = inotify_init();
            fcntl(inotify_fd, F_SETFL, O_NONBLOCK);
        }
        
        addr_file_dir = SDL_strrchr(addr_file, '/');
        if (addr_file_dir) {
            *addr_file_dir = 0;
        }
        
        inotify_wd = inotify_add_watch(inotify_fd, addr_file, IN_CREATE | IN_MODIFY);
        SDL_free(addr_file);
        
        if (addr) {
            result = IBus_SetupConnection(dbus, addr);
            SDL_free(addr);
        }
    }
    
    return result;
}

void
SDL_IBus_Quit(void)
{   
    SDL_DBusContext *dbus;

    if (input_ctx_path) {
        SDL_free(input_ctx_path);
        input_ctx_path = NULL;
    }
    
    if (ibus_addr_file) {
        SDL_free(ibus_addr_file);
        ibus_addr_file = NULL;
    }
    
    dbus = SDL_DBus_GetContext();
    
    if (dbus && ibus_conn) {
        dbus->connection_close(ibus_conn);
        dbus->connection_unref(ibus_conn);
    }
    
    if (inotify_fd > 0 && inotify_wd > 0) {
        inotify_rm_watch(inotify_fd, inotify_wd);
        inotify_wd = -1;
    }
    
    SDL_DelHintCallback(SDL_HINT_IME_INTERNAL_EDITING, IBus_SetCapabilities, NULL);
    
    SDL_memset(&ibus_cursor_rect, 0, sizeof(ibus_cursor_rect));
}

static void
IBus_SimpleMessage(const char *method)
{   
    SDL_DBusContext *dbus = SDL_DBus_GetContext();
    
    if ((input_ctx_path != NULL) && (IBus_CheckConnection(dbus))) {
        SDL_DBus_CallVoidMethodOnConnection(ibus_conn, IBUS_SERVICE, input_ctx_path, IBUS_INPUT_INTERFACE, method, DBUS_TYPE_INVALID);
    }
}

void
SDL_IBus_SetFocus(SDL_bool focused)
{ 
    const char *method = focused ? "FocusIn" : "FocusOut";
    IBus_SimpleMessage(method);
}

void
SDL_IBus_Reset(void)
{
    IBus_SimpleMessage("Reset");
}

SDL_bool
SDL_IBus_ProcessKeyEvent(Uint32 keysym, Uint32 keycode)
{ 
    Uint32 result = 0;
    SDL_DBusContext *dbus = SDL_DBus_GetContext();
    
    if (IBus_CheckConnection(dbus)) {
        Uint32 mods = IBus_ModState();
        if (!SDL_DBus_CallMethodOnConnection(ibus_conn, IBUS_SERVICE, input_ctx_path, IBUS_INPUT_INTERFACE, "ProcessKeyEvent",
                DBUS_TYPE_UINT32, &keysym, DBUS_TYPE_UINT32, &keycode, DBUS_TYPE_UINT32, &mods, DBUS_TYPE_INVALID,
                DBUS_TYPE_BOOLEAN, &result, DBUS_TYPE_INVALID)) {
            result = 0;
        }
    }
    
    SDL_IBus_UpdateTextRect(NULL);

    return result ? SDL_TRUE : SDL_FALSE;
}

void
SDL_IBus_UpdateTextRect(SDL_Rect *rect)
{
    SDL_Window *focused_win;
    SDL_SysWMinfo info;
    int x = 0, y = 0;
    SDL_DBusContext *dbus;

    if (rect) {
        SDL_memcpy(&ibus_cursor_rect, rect, sizeof(ibus_cursor_rect));
    }

    focused_win = SDL_GetKeyboardFocus();
    if (!focused_win) {
        return;
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

    x += ibus_cursor_rect.x;
    y += ibus_cursor_rect.y;
        
    dbus = SDL_DBus_GetContext();
    
    if (IBus_CheckConnection(dbus)) {
        SDL_DBus_CallVoidMethodOnConnection(ibus_conn, IBUS_SERVICE, input_ctx_path, IBUS_INPUT_INTERFACE, "SetCursorLocation",
                DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, DBUS_TYPE_INT32, &ibus_cursor_rect.w, DBUS_TYPE_INT32, &ibus_cursor_rect.h, DBUS_TYPE_INVALID);
    }
}

void
SDL_IBus_PumpEvents(void)
{
    SDL_DBusContext *dbus = SDL_DBus_GetContext();
    
    if (IBus_CheckConnection(dbus)) {
        dbus->connection_read_write(ibus_conn, 0);
    
        while (dbus->connection_dispatch(ibus_conn) == DBUS_DISPATCH_DATA_REMAINS) {
            /* Do nothing, actual work happens in IBus_MessageHandler */
        }
    }
}

#endif

/* vi: set ts=4 sw=4 expandtab: */
