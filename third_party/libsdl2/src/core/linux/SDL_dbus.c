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
#include "SDL_dbus.h"
#include "SDL_atomic.h"

#if SDL_USE_LIBDBUS
/* we never link directly to libdbus. */
#include "SDL_loadso.h"
static const char *dbus_library = "libdbus-1.so.3";
static void *dbus_handle = NULL;
static unsigned int screensaver_cookie = 0;
static SDL_DBusContext dbus;

static int
LoadDBUSSyms(void)
{
    #define SDL_DBUS_SYM2(x, y) \
        if (!(dbus.x = SDL_LoadFunction(dbus_handle, #y))) return -1
        
    #define SDL_DBUS_SYM(x) \
        SDL_DBUS_SYM2(x, dbus_##x)

    SDL_DBUS_SYM(bus_get_private);
    SDL_DBUS_SYM(bus_register);
    SDL_DBUS_SYM(bus_add_match);
    SDL_DBUS_SYM(connection_open_private);
    SDL_DBUS_SYM(connection_set_exit_on_disconnect);
    SDL_DBUS_SYM(connection_get_is_connected);
    SDL_DBUS_SYM(connection_add_filter);
    SDL_DBUS_SYM(connection_try_register_object_path);
    SDL_DBUS_SYM(connection_send);
    SDL_DBUS_SYM(connection_send_with_reply_and_block);
    SDL_DBUS_SYM(connection_close);
    SDL_DBUS_SYM(connection_unref);
    SDL_DBUS_SYM(connection_flush);
    SDL_DBUS_SYM(connection_read_write);
    SDL_DBUS_SYM(connection_dispatch);
    SDL_DBUS_SYM(message_is_signal);
    SDL_DBUS_SYM(message_new_method_call);
    SDL_DBUS_SYM(message_append_args);
    SDL_DBUS_SYM(message_append_args_valist);
    SDL_DBUS_SYM(message_iter_init_append);
    SDL_DBUS_SYM(message_iter_open_container);
    SDL_DBUS_SYM(message_iter_append_basic);
    SDL_DBUS_SYM(message_iter_close_container);
    SDL_DBUS_SYM(message_get_args);
    SDL_DBUS_SYM(message_get_args_valist);
    SDL_DBUS_SYM(message_iter_init);
    SDL_DBUS_SYM(message_iter_next);
    SDL_DBUS_SYM(message_iter_get_basic);
    SDL_DBUS_SYM(message_iter_get_arg_type);
    SDL_DBUS_SYM(message_iter_recurse);
    SDL_DBUS_SYM(message_unref);
    SDL_DBUS_SYM(threads_init_default);
    SDL_DBUS_SYM(error_init);
    SDL_DBUS_SYM(error_is_set);
    SDL_DBUS_SYM(error_free);
    SDL_DBUS_SYM(get_local_machine_id);
    SDL_DBUS_SYM(free);
    SDL_DBUS_SYM(free_string_array);
    SDL_DBUS_SYM(shutdown);

    #undef SDL_DBUS_SYM
    #undef SDL_DBUS_SYM2

    return 0;
}

static void
UnloadDBUSLibrary(void)
{
    if (dbus_handle != NULL) {
        SDL_UnloadObject(dbus_handle);
        dbus_handle = NULL;
    }
}

static int
LoadDBUSLibrary(void)
{
    int retval = 0;
    if (dbus_handle == NULL) {
        dbus_handle = SDL_LoadObject(dbus_library);
        if (dbus_handle == NULL) {
            retval = -1;
            /* Don't call SDL_SetError(): SDL_LoadObject already did. */
        } else {
            retval = LoadDBUSSyms();
            if (retval < 0) {
                UnloadDBUSLibrary();
            }
        }
    }

    return retval;
}


static SDL_SpinLock spinlock_dbus_init = 0;

/* you must hold spinlock_dbus_init before calling this! */
static void
SDL_DBus_Init_Spinlocked(void)
{
    static SDL_bool is_dbus_available = SDL_TRUE;
    if (!is_dbus_available) {
        return;  /* don't keep trying if this fails. */
    }

    if (!dbus.session_conn) {
        DBusError err;

        if (LoadDBUSLibrary() == -1) {
            is_dbus_available = SDL_FALSE;  /* can't load at all? Don't keep trying. */
            return;  /* oh well */
        }

        if (!dbus.threads_init_default()) {
            is_dbus_available = SDL_FALSE;
            return;
        }

        dbus.error_init(&err);
        /* session bus is required */

        dbus.session_conn = dbus.bus_get_private(DBUS_BUS_SESSION, &err);
        if (dbus.error_is_set(&err)) {
            dbus.error_free(&err);
            SDL_DBus_Quit();
            is_dbus_available = SDL_FALSE;
            return;  /* oh well */
        }
        dbus.connection_set_exit_on_disconnect(dbus.session_conn, 0);

        /* system bus is optional */
        dbus.system_conn = dbus.bus_get_private(DBUS_BUS_SYSTEM, &err);
        if (!dbus.error_is_set(&err)) {
            dbus.connection_set_exit_on_disconnect(dbus.system_conn, 0);
        }

        dbus.error_free(&err);
    }
}

void
SDL_DBus_Init(void)
{
    SDL_AtomicLock(&spinlock_dbus_init);  /* make sure two threads can't init at same time, since this can happen before SDL_Init. */
    SDL_DBus_Init_Spinlocked();
    SDL_AtomicUnlock(&spinlock_dbus_init);
}

void
SDL_DBus_Quit(void)
{
    if (dbus.system_conn) {
        dbus.connection_close(dbus.system_conn);
        dbus.connection_unref(dbus.system_conn);
    }
    if (dbus.session_conn) {
        dbus.connection_close(dbus.session_conn);
        dbus.connection_unref(dbus.session_conn);
    }
/* Don't do this - bug 3950
   dbus_shutdown() is a debug feature which closes all global resources in the dbus library. Calling this should be done by the app, not a library, because if there are multiple users of dbus in the process then SDL could shut it down even though another part is using it.
*/
#if 0
    if (dbus.shutdown) {
        dbus.shutdown();
    }
#endif
    SDL_zero(dbus);
    UnloadDBUSLibrary();
}

SDL_DBusContext *
SDL_DBus_GetContext(void)
{
    if (!dbus_handle || !dbus.session_conn) {
        SDL_DBus_Init();
    }
    
    return (dbus_handle && dbus.session_conn) ? &dbus : NULL;
}

static SDL_bool
SDL_DBus_CallMethodInternal(DBusConnection *conn, const char *node, const char *path, const char *interface, const char *method, va_list ap)
{
    SDL_bool retval = SDL_FALSE;

    if (conn) {
        DBusMessage *msg = dbus.message_new_method_call(node, path, interface, method);
        if (msg) {
            int firstarg;
            va_list ap_reply;
            va_copy(ap_reply, ap);  /* copy the arg list so we don't compete with D-Bus for it */
            firstarg = va_arg(ap, int);
            if ((firstarg == DBUS_TYPE_INVALID) || dbus.message_append_args_valist(msg, firstarg, ap)) {
                DBusMessage *reply = dbus.connection_send_with_reply_and_block(conn, msg, 300, NULL);
                if (reply) {
                    /* skip any input args, get to output args. */
                    while ((firstarg = va_arg(ap_reply, int)) != DBUS_TYPE_INVALID) {
                        /* we assume D-Bus already validated all this. */
                        { void *dumpptr = va_arg(ap_reply, void*); (void) dumpptr; }
                        if (firstarg == DBUS_TYPE_ARRAY) {
                            { const int dumpint = va_arg(ap_reply, int); (void) dumpint; }
                        }
                    }
                    firstarg = va_arg(ap_reply, int);
                    if ((firstarg == DBUS_TYPE_INVALID) || dbus.message_get_args_valist(reply, NULL, firstarg, ap_reply)) {
                        retval = SDL_TRUE;
                    }
                    dbus.message_unref(reply);
                }
            }
            va_end(ap_reply);
            dbus.message_unref(msg);
        }
    }

    return retval;
}

SDL_bool
SDL_DBus_CallMethodOnConnection(DBusConnection *conn, const char *node, const char *path, const char *interface, const char *method, ...)
{
    SDL_bool retval;
    va_list ap;
    va_start(ap, method);
    retval = SDL_DBus_CallMethodInternal(conn, node, path, interface, method, ap);
    va_end(ap);
    return retval;
}

SDL_bool
SDL_DBus_CallMethod(const char *node, const char *path, const char *interface, const char *method, ...)
{
    SDL_bool retval;
    va_list ap;
    va_start(ap, method);
    retval = SDL_DBus_CallMethodInternal(dbus.session_conn, node, path, interface, method, ap);
    va_end(ap);
    return retval;
}

static SDL_bool
SDL_DBus_CallVoidMethodInternal(DBusConnection *conn, const char *node, const char *path, const char *interface, const char *method, va_list ap)
{
    SDL_bool retval = SDL_FALSE;

    if (conn) {
        DBusMessage *msg = dbus.message_new_method_call(node, path, interface, method);
        if (msg) {
            int firstarg = va_arg(ap, int);
            if ((firstarg == DBUS_TYPE_INVALID) || dbus.message_append_args_valist(msg, firstarg, ap)) {
                if (dbus.connection_send(conn, msg, NULL)) {
                    dbus.connection_flush(conn);
                    retval = SDL_TRUE;
                }
            }

            dbus.message_unref(msg);
        }
    }

    return retval;
}

SDL_bool
SDL_DBus_CallVoidMethodOnConnection(DBusConnection *conn, const char *node, const char *path, const char *interface, const char *method, ...)
{
    SDL_bool retval;
    va_list ap;
    va_start(ap, method);
    retval = SDL_DBus_CallVoidMethodInternal(conn, node, path, interface, method, ap);
    va_end(ap);
    return retval;
}

SDL_bool
SDL_DBus_CallVoidMethod(const char *node, const char *path, const char *interface, const char *method, ...)
{
    SDL_bool retval;
    va_list ap;
    va_start(ap, method);
    retval = SDL_DBus_CallVoidMethodInternal(dbus.session_conn, node, path, interface, method, ap);
    va_end(ap);
    return retval;
}

SDL_bool
SDL_DBus_QueryPropertyOnConnection(DBusConnection *conn, const char *node, const char *path, const char *interface, const char *property, const int expectedtype, void *result)
{
    SDL_bool retval = SDL_FALSE;

    if (conn) {
        DBusMessage *msg = dbus.message_new_method_call(node, path, "org.freedesktop.DBus.Properties", "Get");
        if (msg) {
            if (dbus.message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) {
                DBusMessage *reply = dbus.connection_send_with_reply_and_block(conn, msg, 300, NULL);
                if (reply) {
                    DBusMessageIter iter, sub;
                    dbus.message_iter_init(reply, &iter);
                    if (dbus.message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT) {
                        dbus.message_iter_recurse(&iter, &sub);
                        if (dbus.message_iter_get_arg_type(&sub) == expectedtype) {
                            dbus.message_iter_get_basic(&sub, result);
                            retval = SDL_TRUE;
                        }
                    }
                    dbus.message_unref(reply);
                }
            }
            dbus.message_unref(msg);
        }
    }

    return retval;
}

SDL_bool
SDL_DBus_QueryProperty(const char *node, const char *path, const char *interface, const char *property, const int expectedtype, void *result)
{
    return SDL_DBus_QueryPropertyOnConnection(dbus.session_conn, node, path, interface, property, expectedtype, result);
}


void
SDL_DBus_ScreensaverTickle(void)
{
    if (screensaver_cookie == 0) {  /* no need to tickle if we're inhibiting. */
        /* org.gnome.ScreenSaver is the legacy interface, but it'll either do nothing or just be a second harmless tickle on newer systems, so we leave it for now. */
        SDL_DBus_CallVoidMethod("org.gnome.ScreenSaver", "/org/gnome/ScreenSaver", "org.gnome.ScreenSaver", "SimulateUserActivity", DBUS_TYPE_INVALID);
        SDL_DBus_CallVoidMethod("org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver", "SimulateUserActivity", DBUS_TYPE_INVALID);
    }
}

SDL_bool
SDL_DBus_ScreensaverInhibit(SDL_bool inhibit)
{
    if ( (inhibit && (screensaver_cookie != 0)) || (!inhibit && (screensaver_cookie == 0)) ) {
        return SDL_TRUE;
    } else {
        const char *node = "org.freedesktop.ScreenSaver";
        const char *path = "/org/freedesktop/ScreenSaver";
        const char *interface = "org.freedesktop.ScreenSaver";

        if (inhibit) {
            const char *app = "My SDL application";
            const char *reason = "Playing a game";
            if (!SDL_DBus_CallMethod(node, path, interface, "Inhibit",
                    DBUS_TYPE_STRING, &app, DBUS_TYPE_STRING, &reason, DBUS_TYPE_INVALID,
                    DBUS_TYPE_UINT32, &screensaver_cookie, DBUS_TYPE_INVALID)) {
                return SDL_FALSE;
            }
            return (screensaver_cookie != 0) ? SDL_TRUE : SDL_FALSE;
        } else {
            if (!SDL_DBus_CallVoidMethod(node, path, interface, "UnInhibit", DBUS_TYPE_UINT32, &screensaver_cookie, DBUS_TYPE_INVALID)) {
                return SDL_FALSE;
            }
            screensaver_cookie = 0;
        }
    }

    return SDL_TRUE;
}
#endif

/* vi: set ts=4 sw=4 expandtab: */
