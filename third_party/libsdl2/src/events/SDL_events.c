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

/* General event handling code for SDL */

#include "SDL.h"
#include "SDL_events.h"
#include "SDL_thread.h"
#include "SDL_events_c.h"
#include "../timer/SDL_timer_c.h"
#if !SDL_JOYSTICK_DISABLED
#include "../joystick/SDL_joystick_c.h"
#endif
#include "../video/SDL_sysvideo.h"
#include "SDL_syswm.h"

/*#define SDL_DEBUG_EVENTS 1*/

/* An arbitrary limit so we don't have unbounded growth */
#define SDL_MAX_QUEUED_EVENTS   65535

typedef struct SDL_EventWatcher {
    SDL_EventFilter callback;
    void *userdata;
    SDL_bool removed;
} SDL_EventWatcher;

static SDL_mutex *SDL_event_watchers_lock;
static SDL_EventWatcher SDL_EventOK;
static SDL_EventWatcher *SDL_event_watchers = NULL;
static int SDL_event_watchers_count = 0;
static SDL_bool SDL_event_watchers_dispatching = SDL_FALSE;
static SDL_bool SDL_event_watchers_removed = SDL_FALSE;

typedef struct {
    Uint32 bits[8];
} SDL_DisabledEventBlock;

static SDL_DisabledEventBlock *SDL_disabled_events[256];
static Uint32 SDL_userevents = SDL_USEREVENT;

/* Private data -- event queue */
typedef struct _SDL_EventEntry
{
    SDL_Event event;
    SDL_SysWMmsg msg;
    struct _SDL_EventEntry *prev;
    struct _SDL_EventEntry *next;
} SDL_EventEntry;

typedef struct _SDL_SysWMEntry
{
    SDL_SysWMmsg msg;
    struct _SDL_SysWMEntry *next;
} SDL_SysWMEntry;

static struct
{
    SDL_mutex *lock;
    SDL_atomic_t active;
    SDL_atomic_t count;
    int max_events_seen;
    SDL_EventEntry *head;
    SDL_EventEntry *tail;
    SDL_EventEntry *free;
    SDL_SysWMEntry *wmmsg_used;
    SDL_SysWMEntry *wmmsg_free;
} SDL_EventQ = { NULL, { 1 }, { 0 }, 0, NULL, NULL, NULL, NULL, NULL };


#ifdef SDL_DEBUG_EVENTS

/* this is to make printf() calls cleaner. */
#define uint unsigned int

static void
SDL_DebugPrintEvent(const SDL_Event *event)
{
    /* !!! FIXME: This code is kinda ugly, sorry. */
    printf("SDL EVENT: ");

    if ((event->type >= SDL_USEREVENT) && (event->type <= SDL_LASTEVENT)) {
        printf("SDL_USEREVENT");
        if (event->type > SDL_USEREVENT) {
            printf("+%u", ((uint) event->type) - SDL_USEREVENT);
        }
        printf(" (timestamp=%u windowid=%u code=%d data1=%p data2=%p)",
                (uint) event->user.timestamp, (uint) event->user.windowID,
                (int) event->user.code, event->user.data1, event->user.data2);
        return;
    }

    switch (event->type) {
        #define SDL_EVENT_CASE(x) case x: printf("%s", #x);
        SDL_EVENT_CASE(SDL_FIRSTEVENT) printf("(THIS IS PROBABLY A BUG!)"); break;
        SDL_EVENT_CASE(SDL_QUIT) printf("(timestamp=%u)", (uint) event->quit.timestamp); break;
        SDL_EVENT_CASE(SDL_APP_TERMINATING) break;
        SDL_EVENT_CASE(SDL_APP_LOWMEMORY) break;
        SDL_EVENT_CASE(SDL_APP_WILLENTERBACKGROUND) break;
        SDL_EVENT_CASE(SDL_APP_DIDENTERBACKGROUND) break;
        SDL_EVENT_CASE(SDL_APP_WILLENTERFOREGROUND) break;
        SDL_EVENT_CASE(SDL_APP_DIDENTERFOREGROUND) break;
        SDL_EVENT_CASE(SDL_KEYMAPCHANGED) break;
        SDL_EVENT_CASE(SDL_CLIPBOARDUPDATE) break;
        SDL_EVENT_CASE(SDL_RENDER_TARGETS_RESET) break;
        SDL_EVENT_CASE(SDL_RENDER_DEVICE_RESET) break;
        #undef SDL_EVENT_CASE

        #define SDL_EVENT_CASE(x) case x: printf("%s ", #x);

        SDL_EVENT_CASE(SDL_WINDOWEVENT)
            printf("(timestamp=%u windowid=%u event=", (uint) event->window.timestamp, (uint) event->window.windowID);
            switch(event->window.event) {
                case SDL_WINDOWEVENT_NONE: printf("none(THIS IS PROBABLY A BUG!)"); break;
                #define SDL_WINDOWEVENT_CASE(x) case x: printf("%s", #x); break
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_SHOWN);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_HIDDEN);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_EXPOSED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_MOVED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_RESIZED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_SIZE_CHANGED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_MINIMIZED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_MAXIMIZED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_RESTORED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_ENTER);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_LEAVE);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_FOCUS_GAINED);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_FOCUS_LOST);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_CLOSE);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_TAKE_FOCUS);
                SDL_WINDOWEVENT_CASE(SDL_WINDOWEVENT_HIT_TEST);
                #undef SDL_WINDOWEVENT_CASE
                default: printf("UNKNOWN(bug? fixme?)"); break;
            }
            printf(" data1=%d data2=%d)", (int) event->window.data1, (int) event->window.data2);
            break;

        SDL_EVENT_CASE(SDL_SYSWMEVENT)
            printf("(timestamp=%u)", (uint) event->syswm.timestamp);
            /* !!! FIXME: we don't delve further at the moment. */
            break;

        #define PRINT_KEY_EVENT(event) \
            printf("(timestamp=%u windowid=%u state=%s repeat=%s scancode=%u keycode=%u mod=%u)", \
                (uint) event->key.timestamp, (uint) event->key.windowID, \
                event->key.state == SDL_PRESSED ? "pressed" : "released", \
                event->key.repeat ? "true" : "false", \
                (uint) event->key.keysym.scancode, \
                (uint) event->key.keysym.sym, \
                (uint) event->key.keysym.mod)
        SDL_EVENT_CASE(SDL_KEYDOWN) PRINT_KEY_EVENT(event); break;
        SDL_EVENT_CASE(SDL_KEYUP) PRINT_KEY_EVENT(event); break;
        #undef PRINT_KEY_EVENT

        SDL_EVENT_CASE(SDL_TEXTEDITING)
            printf("(timestamp=%u windowid=%u text='%s' start=%d length=%d)",
                (uint) event->edit.timestamp, (uint) event->edit.windowID,
                event->edit.text, (int) event->edit.start, (int) event->edit.length);
            break;

        SDL_EVENT_CASE(SDL_TEXTINPUT)
            printf("(timestamp=%u windowid=%u text='%s')", (uint) event->text.timestamp, (uint) event->text.windowID, event->text.text);
            break;


        SDL_EVENT_CASE(SDL_MOUSEMOTION)
            printf("(timestamp=%u windowid=%u which=%u state=%u x=%d y=%d xrel=%d yrel=%d)",
                    (uint) event->motion.timestamp, (uint) event->motion.windowID,
                    (uint) event->motion.which, (uint) event->motion.state,
                    (int) event->motion.x, (int) event->motion.y,
                    (int) event->motion.xrel, (int) event->motion.yrel);
            break;

        #define PRINT_MBUTTON_EVENT(event) \
            printf("(timestamp=%u windowid=%u which=%u button=%u state=%s clicks=%u x=%d y=%d)", \
                    (uint) event->button.timestamp, (uint) event->button.windowID, \
                    (uint) event->button.which, (uint) event->button.button, \
                    event->button.state == SDL_PRESSED ? "pressed" : "released", \
                    (uint) event->button.clicks, (int) event->button.x, (int) event->button.y)
        SDL_EVENT_CASE(SDL_MOUSEBUTTONDOWN) PRINT_MBUTTON_EVENT(event); break;
        SDL_EVENT_CASE(SDL_MOUSEBUTTONUP) PRINT_MBUTTON_EVENT(event); break;
        #undef PRINT_MBUTTON_EVENT


        SDL_EVENT_CASE(SDL_MOUSEWHEEL)
            printf("(timestamp=%u windowid=%u which=%u x=%d y=%d direction=%s)",
                    (uint) event->wheel.timestamp, (uint) event->wheel.windowID,
                    (uint) event->wheel.which, (int) event->wheel.x, (int) event->wheel.y,
                    event->wheel.direction == SDL_MOUSEWHEEL_NORMAL ? "normal" : "flipped");
            break;

        SDL_EVENT_CASE(SDL_JOYAXISMOTION)
            printf("(timestamp=%u which=%d axis=%u value=%d)",
                (uint) event->jaxis.timestamp, (int) event->jaxis.which,
                (uint) event->jaxis.axis, (int) event->jaxis.value);
            break;

        SDL_EVENT_CASE(SDL_JOYBALLMOTION)
            printf("(timestamp=%u which=%d ball=%u xrel=%d yrel=%d)",
                (uint) event->jball.timestamp, (int) event->jball.which,
                (uint) event->jball.ball, (int) event->jball.xrel, (int) event->jball.yrel);
            break;

        SDL_EVENT_CASE(SDL_JOYHATMOTION)
            printf("(timestamp=%u which=%d hat=%u value=%u)",
                (uint) event->jhat.timestamp, (int) event->jhat.which,
                (uint) event->jhat.hat, (uint) event->jhat.value);
            break;

        #define PRINT_JBUTTON_EVENT(event) \
            printf("(timestamp=%u which=%d button=%u state=%s)", \
                (uint) event->jbutton.timestamp, (int) event->jbutton.which, \
                (uint) event->jbutton.button, event->jbutton.state == SDL_PRESSED ? "pressed" : "released")
        SDL_EVENT_CASE(SDL_JOYBUTTONDOWN) PRINT_JBUTTON_EVENT(event); break;
        SDL_EVENT_CASE(SDL_JOYBUTTONUP) PRINT_JBUTTON_EVENT(event); break;
        #undef PRINT_JBUTTON_EVENT

        #define PRINT_JOYDEV_EVENT(event) printf("(timestamp=%u which=%d)", (uint) event->jdevice.timestamp, (int) event->jdevice.which)
        SDL_EVENT_CASE(SDL_JOYDEVICEADDED) PRINT_JOYDEV_EVENT(event); break;
        SDL_EVENT_CASE(SDL_JOYDEVICEREMOVED) PRINT_JOYDEV_EVENT(event); break;
        #undef PRINT_JOYDEV_EVENT

        SDL_EVENT_CASE(SDL_CONTROLLERAXISMOTION)
            printf("(timestamp=%u which=%d axis=%u value=%d)",
                (uint) event->caxis.timestamp, (int) event->caxis.which,
                (uint) event->caxis.axis, (int) event->caxis.value);
            break;

        #define PRINT_CBUTTON_EVENT(event) \
            printf("(timestamp=%u which=%d button=%u state=%s)", \
                (uint) event->cbutton.timestamp, (int) event->cbutton.which, \
                (uint) event->cbutton.button, event->cbutton.state == SDL_PRESSED ? "pressed" : "released")
        SDL_EVENT_CASE(SDL_CONTROLLERBUTTONDOWN) PRINT_CBUTTON_EVENT(event); break;
        SDL_EVENT_CASE(SDL_CONTROLLERBUTTONUP) PRINT_CBUTTON_EVENT(event); break;
        #undef PRINT_CBUTTON_EVENT

        #define PRINT_CONTROLLERDEV_EVENT(event) printf("(timestamp=%u which=%d)", (uint) event->cdevice.timestamp, (int) event->cdevice.which)
        SDL_EVENT_CASE(SDL_CONTROLLERDEVICEADDED) PRINT_CONTROLLERDEV_EVENT(event); break;
        SDL_EVENT_CASE(SDL_CONTROLLERDEVICEREMOVED) PRINT_CONTROLLERDEV_EVENT(event); break;
        SDL_EVENT_CASE(SDL_CONTROLLERDEVICEREMAPPED) PRINT_CONTROLLERDEV_EVENT(event); break;
        #undef PRINT_CONTROLLERDEV_EVENT

        #define PRINT_FINGER_EVENT(event) \
            printf("(timestamp=%u touchid=%lld fingerid=%lld x=%f y=%f dx=%f dy=%f pressure=%f)", \
                (uint) event->tfinger.timestamp, (long long) event->tfinger.touchId, \
                (long long) event->tfinger.fingerId, event->tfinger.x, event->tfinger.y, \
                event->tfinger.dx, event->tfinger.dy, event->tfinger.pressure)
        SDL_EVENT_CASE(SDL_FINGERDOWN) PRINT_FINGER_EVENT(event); break;
        SDL_EVENT_CASE(SDL_FINGERUP) PRINT_FINGER_EVENT(event); break;
        SDL_EVENT_CASE(SDL_FINGERMOTION) PRINT_FINGER_EVENT(event); break;
        #undef PRINT_FINGER_EVENT

        #define PRINT_DOLLAR_EVENT(event) \
            printf("(timestamp=%u touchid=%lld gestureid=%lld numfingers=%u error=%f x=%f y=%f)", \
                (uint) event->dgesture.timestamp, (long long) event->dgesture.touchId, \
                (long long) event->dgesture.gestureId, (uint) event->dgesture.numFingers, \
                event->dgesture.error, event->dgesture.x, event->dgesture.y);
        SDL_EVENT_CASE(SDL_DOLLARGESTURE) PRINT_DOLLAR_EVENT(event); break;
        SDL_EVENT_CASE(SDL_DOLLARRECORD) PRINT_DOLLAR_EVENT(event); break;
        #undef PRINT_DOLLAR_EVENT

        SDL_EVENT_CASE(SDL_MULTIGESTURE)
            printf("(timestamp=%u touchid=%lld dtheta=%f ddist=%f x=%f y=%f numfingers=%u)",
                (uint) event->mgesture.timestamp, (long long) event->mgesture.touchId,
                event->mgesture.dTheta, event->mgesture.dDist,
                event->mgesture.x, event->mgesture.y, (uint) event->mgesture.numFingers);
            break;

        #define PRINT_DROP_EVENT(event) printf("(file='%s' timestamp=%u windowid=%u)", event->drop.file, (uint) event->drop.timestamp, (uint) event->drop.windowID)
        SDL_EVENT_CASE(SDL_DROPFILE) PRINT_DROP_EVENT(event); break;
        SDL_EVENT_CASE(SDL_DROPTEXT) PRINT_DROP_EVENT(event); break;
        SDL_EVENT_CASE(SDL_DROPBEGIN) PRINT_DROP_EVENT(event); break;
        SDL_EVENT_CASE(SDL_DROPCOMPLETE) PRINT_DROP_EVENT(event); break;
        #undef PRINT_DROP_EVENT

        #define PRINT_AUDIODEV_EVENT(event) printf("(timestamp=%u which=%u iscapture=%s)", (uint) event->adevice.timestamp, (uint) event->adevice.which, event->adevice.iscapture ? "true" : "false");
        SDL_EVENT_CASE(SDL_AUDIODEVICEADDED) PRINT_AUDIODEV_EVENT(event); break;
        SDL_EVENT_CASE(SDL_AUDIODEVICEREMOVED) PRINT_AUDIODEV_EVENT(event); break;
        #undef PRINT_AUDIODEV_EVENT

        #undef SDL_EVENT_CASE

        default:
            printf("UNKNOWN SDL EVENT #%u! (Bug? FIXME?)", (uint) event->type);
            break;
    }

    printf("\n");
}
#undef uint
#endif



/* Public functions */

void
SDL_StopEventLoop(void)
{
    const char *report = SDL_GetHint("SDL_EVENT_QUEUE_STATISTICS");
    int i;
    SDL_EventEntry *entry;
    SDL_SysWMEntry *wmmsg;

    if (SDL_EventQ.lock) {
        SDL_LockMutex(SDL_EventQ.lock);
    }

    SDL_AtomicSet(&SDL_EventQ.active, 0);

    if (report && SDL_atoi(report)) {
        SDL_Log("SDL EVENT QUEUE: Maximum events in-flight: %d\n",
                SDL_EventQ.max_events_seen);
    }

    /* Clean out EventQ */
    for (entry = SDL_EventQ.head; entry; ) {
        SDL_EventEntry *next = entry->next;
        SDL_free(entry);
        entry = next;
    }
    for (entry = SDL_EventQ.free; entry; ) {
        SDL_EventEntry *next = entry->next;
        SDL_free(entry);
        entry = next;
    }
    for (wmmsg = SDL_EventQ.wmmsg_used; wmmsg; ) {
        SDL_SysWMEntry *next = wmmsg->next;
        SDL_free(wmmsg);
        wmmsg = next;
    }
    for (wmmsg = SDL_EventQ.wmmsg_free; wmmsg; ) {
        SDL_SysWMEntry *next = wmmsg->next;
        SDL_free(wmmsg);
        wmmsg = next;
    }

    SDL_AtomicSet(&SDL_EventQ.count, 0);
    SDL_EventQ.max_events_seen = 0;
    SDL_EventQ.head = NULL;
    SDL_EventQ.tail = NULL;
    SDL_EventQ.free = NULL;
    SDL_EventQ.wmmsg_used = NULL;
    SDL_EventQ.wmmsg_free = NULL;

    /* Clear disabled event state */
    for (i = 0; i < SDL_arraysize(SDL_disabled_events); ++i) {
        SDL_free(SDL_disabled_events[i]);
        SDL_disabled_events[i] = NULL;
    }

    if (SDL_event_watchers_lock) {
        SDL_DestroyMutex(SDL_event_watchers_lock);
        SDL_event_watchers_lock = NULL;
    }
    if (SDL_event_watchers) {
        SDL_free(SDL_event_watchers);
        SDL_event_watchers = NULL;
        SDL_event_watchers_count = 0;
    }
    SDL_zero(SDL_EventOK);

    if (SDL_EventQ.lock) {
        SDL_UnlockMutex(SDL_EventQ.lock);
        SDL_DestroyMutex(SDL_EventQ.lock);
        SDL_EventQ.lock = NULL;
    }
}

/* This function (and associated calls) may be called more than once */
int
SDL_StartEventLoop(void)
{
    /* We'll leave the event queue alone, since we might have gotten
       some important events at launch (like SDL_DROPFILE)

       FIXME: Does this introduce any other bugs with events at startup?
     */

    /* Create the lock and set ourselves active */
#if !SDL_THREADS_DISABLED
    if (!SDL_EventQ.lock) {
        SDL_EventQ.lock = SDL_CreateMutex();
        if (SDL_EventQ.lock == NULL) {
            return -1;
        }
    }

    if (!SDL_event_watchers_lock) {
        SDL_event_watchers_lock = SDL_CreateMutex();
        if (SDL_event_watchers_lock == NULL) {
            return -1;
        }
    }
#endif /* !SDL_THREADS_DISABLED */

    /* Process most event types */
    SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
    SDL_EventState(SDL_TEXTEDITING, SDL_DISABLE);
    SDL_EventState(SDL_SYSWMEVENT, SDL_DISABLE);

    SDL_AtomicSet(&SDL_EventQ.active, 1);

    return 0;
}


/* Add an event to the event queue -- called with the queue locked */
static int
SDL_AddEvent(SDL_Event * event)
{
    SDL_EventEntry *entry;
    const int initial_count = SDL_AtomicGet(&SDL_EventQ.count);
    int final_count;

    if (initial_count >= SDL_MAX_QUEUED_EVENTS) {
        SDL_SetError("Event queue is full (%d events)", initial_count);
        return 0;
    }

    if (SDL_EventQ.free == NULL) {
        entry = (SDL_EventEntry *)SDL_malloc(sizeof(*entry));
        if (!entry) {
            return 0;
        }
    } else {
        entry = SDL_EventQ.free;
        SDL_EventQ.free = entry->next;
    }

    #ifdef SDL_DEBUG_EVENTS
    SDL_DebugPrintEvent(event);
    #endif

    entry->event = *event;
    if (event->type == SDL_SYSWMEVENT) {
        entry->msg = *event->syswm.msg;
        entry->event.syswm.msg = &entry->msg;
    }

    if (SDL_EventQ.tail) {
        SDL_EventQ.tail->next = entry;
        entry->prev = SDL_EventQ.tail;
        SDL_EventQ.tail = entry;
        entry->next = NULL;
    } else {
        SDL_assert(!SDL_EventQ.head);
        SDL_EventQ.head = entry;
        SDL_EventQ.tail = entry;
        entry->prev = NULL;
        entry->next = NULL;
    }

    final_count = SDL_AtomicAdd(&SDL_EventQ.count, 1) + 1;
    if (final_count > SDL_EventQ.max_events_seen) {
        SDL_EventQ.max_events_seen = final_count;
    }

    return 1;
}

/* Remove an event from the queue -- called with the queue locked */
static void
SDL_CutEvent(SDL_EventEntry *entry)
{
    if (entry->prev) {
        entry->prev->next = entry->next;
    }
    if (entry->next) {
        entry->next->prev = entry->prev;
    }

    if (entry == SDL_EventQ.head) {
        SDL_assert(entry->prev == NULL);
        SDL_EventQ.head = entry->next;
    }
    if (entry == SDL_EventQ.tail) {
        SDL_assert(entry->next == NULL);
        SDL_EventQ.tail = entry->prev;
    }

    entry->next = SDL_EventQ.free;
    SDL_EventQ.free = entry;
    SDL_assert(SDL_AtomicGet(&SDL_EventQ.count) > 0);
    SDL_AtomicAdd(&SDL_EventQ.count, -1);
}

/* Lock the event queue, take a peep at it, and unlock it */
int
SDL_PeepEvents(SDL_Event * events, int numevents, SDL_eventaction action,
               Uint32 minType, Uint32 maxType)
{
    int i, used;

    /* Don't look after we've quit */
    if (!SDL_AtomicGet(&SDL_EventQ.active)) {
        /* We get a few spurious events at shutdown, so don't warn then */
        if (action != SDL_ADDEVENT) {
            SDL_SetError("The event system has been shut down");
        }
        return (-1);
    }
    /* Lock the event queue */
    used = 0;
    if (!SDL_EventQ.lock || SDL_LockMutex(SDL_EventQ.lock) == 0) {
        if (action == SDL_ADDEVENT) {
            for (i = 0; i < numevents; ++i) {
                used += SDL_AddEvent(&events[i]);
            }
        } else {
            SDL_EventEntry *entry, *next;
            SDL_SysWMEntry *wmmsg, *wmmsg_next;
            Uint32 type;

            if (action == SDL_GETEVENT) {
                /* Clean out any used wmmsg data
                   FIXME: Do we want to retain the data for some period of time?
                 */
                for (wmmsg = SDL_EventQ.wmmsg_used; wmmsg; wmmsg = wmmsg_next) {
                    wmmsg_next = wmmsg->next;
                    wmmsg->next = SDL_EventQ.wmmsg_free;
                    SDL_EventQ.wmmsg_free = wmmsg;
                }
                SDL_EventQ.wmmsg_used = NULL;
            }

            for (entry = SDL_EventQ.head; entry && (!events || used < numevents); entry = next) {
                next = entry->next;
                type = entry->event.type;
                if (minType <= type && type <= maxType) {
                    if (events) {
                        events[used] = entry->event;
                        if (entry->event.type == SDL_SYSWMEVENT) {
                            /* We need to copy the wmmsg somewhere safe.
                               For now we'll guarantee it's valid at least until
                               the next call to SDL_PeepEvents()
                             */
                            if (SDL_EventQ.wmmsg_free) {
                                wmmsg = SDL_EventQ.wmmsg_free;
                                SDL_EventQ.wmmsg_free = wmmsg->next;
                            } else {
                                wmmsg = (SDL_SysWMEntry *)SDL_malloc(sizeof(*wmmsg));
                            }
                            wmmsg->msg = *entry->event.syswm.msg;
                            wmmsg->next = SDL_EventQ.wmmsg_used;
                            SDL_EventQ.wmmsg_used = wmmsg;
                            events[used].syswm.msg = &wmmsg->msg;
                        }

                        if (action == SDL_GETEVENT) {
                            SDL_CutEvent(entry);
                        }
                    }
                    ++used;
                }
            }
        }
        if (SDL_EventQ.lock) {
            SDL_UnlockMutex(SDL_EventQ.lock);
        }
    } else {
        return SDL_SetError("Couldn't lock event queue");
    }
    return (used);
}

SDL_bool
SDL_HasEvent(Uint32 type)
{
    return (SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, type, type) > 0);
}

SDL_bool
SDL_HasEvents(Uint32 minType, Uint32 maxType)
{
    return (SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, minType, maxType) > 0);
}

void
SDL_FlushEvent(Uint32 type)
{
    SDL_FlushEvents(type, type);
}

void
SDL_FlushEvents(Uint32 minType, Uint32 maxType)
{
    /* Don't look after we've quit */
    if (!SDL_AtomicGet(&SDL_EventQ.active)) {
        return;
    }

    /* Make sure the events are current */
#if 0
    /* Actually, we can't do this since we might be flushing while processing
       a resize event, and calling this might trigger further resize events.
    */
    SDL_PumpEvents();
#endif

    /* Lock the event queue */
    if (!SDL_EventQ.lock || SDL_LockMutex(SDL_EventQ.lock) == 0) {
        SDL_EventEntry *entry, *next;
        Uint32 type;
        for (entry = SDL_EventQ.head; entry; entry = next) {
            next = entry->next;
            type = entry->event.type;
            if (minType <= type && type <= maxType) {
                SDL_CutEvent(entry);
            }
        }
        if (SDL_EventQ.lock) {
            SDL_UnlockMutex(SDL_EventQ.lock);
        }
    }
}

/* Run the system dependent event loops */
void
SDL_PumpEvents(void)
{
    SDL_VideoDevice *_this = SDL_GetVideoDevice();

    /* Get events from the video subsystem */
    if (_this) {
        _this->PumpEvents(_this);
    }
#if !SDL_JOYSTICK_DISABLED
    /* Check for joystick state change */
    if ((!SDL_disabled_events[SDL_JOYAXISMOTION >> 8] || SDL_JoystickEventState(SDL_QUERY))) {
        SDL_JoystickUpdate();
    }
#endif

    SDL_SendPendingQuit();  /* in case we had a signal handler fire, etc. */
}

/* Public functions */

int
SDL_PollEvent(SDL_Event * event)
{
    return SDL_WaitEventTimeout(event, 0);
}

int
SDL_WaitEvent(SDL_Event * event)
{
    return SDL_WaitEventTimeout(event, -1);
}

int
SDL_WaitEventTimeout(SDL_Event * event, int timeout)
{
    Uint32 expiration = 0;

    if (timeout > 0)
        expiration = SDL_GetTicks() + timeout;

    for (;;) {
        SDL_PumpEvents();
        switch (SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        case -1:
            return 0;
        case 0:
            if (timeout == 0) {
                /* Polling and no events, just return */
                return 0;
            }
            if (timeout > 0 && SDL_TICKS_PASSED(SDL_GetTicks(), expiration)) {
                /* Timeout expired and no events */
                return 0;
            }
            SDL_Delay(10);
            break;
        default:
            /* Has events */
            return 1;
        }
    }
}

int
SDL_PushEvent(SDL_Event * event)
{
    event->common.timestamp = SDL_GetTicks();

    if (SDL_EventOK.callback || SDL_event_watchers_count > 0) {
        if (!SDL_event_watchers_lock || SDL_LockMutex(SDL_event_watchers_lock) == 0) {
            if (SDL_EventOK.callback && !SDL_EventOK.callback(SDL_EventOK.userdata, event)) {
                if (SDL_event_watchers_lock) {
                    SDL_UnlockMutex(SDL_event_watchers_lock);
                }
                return 0;
            }

            if (SDL_event_watchers_count > 0) {
                /* Make sure we only dispatch the current watcher list */
                int i, event_watchers_count = SDL_event_watchers_count;

                SDL_event_watchers_dispatching = SDL_TRUE;
                for (i = 0; i < event_watchers_count; ++i) {
                    if (!SDL_event_watchers[i].removed) {
                        SDL_event_watchers[i].callback(SDL_event_watchers[i].userdata, event);
                    }
                }
                SDL_event_watchers_dispatching = SDL_FALSE;

                if (SDL_event_watchers_removed) {
                    for (i = SDL_event_watchers_count; i--; ) {
                        if (SDL_event_watchers[i].removed) {
                            --SDL_event_watchers_count;
                            if (i < SDL_event_watchers_count) {
                                SDL_memmove(&SDL_event_watchers[i], &SDL_event_watchers[i+1], (SDL_event_watchers_count - i) * sizeof(SDL_event_watchers[i]));
                            }
                        }
                    }
                    SDL_event_watchers_removed = SDL_FALSE;
                }
            }

            if (SDL_event_watchers_lock) {
                SDL_UnlockMutex(SDL_event_watchers_lock);
            }
        }
    }

    if (SDL_PeepEvents(event, 1, SDL_ADDEVENT, 0, 0) <= 0) {
        return -1;
    }

    SDL_GestureProcessEvent(event);

    return 1;
}

void
SDL_SetEventFilter(SDL_EventFilter filter, void *userdata)
{
    if (!SDL_event_watchers_lock || SDL_LockMutex(SDL_event_watchers_lock) == 0) {
        /* Set filter and discard pending events */
        SDL_EventOK.callback = filter;
        SDL_EventOK.userdata = userdata;
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

        if (SDL_event_watchers_lock) {
            SDL_UnlockMutex(SDL_event_watchers_lock);
        }
    }
}

SDL_bool
SDL_GetEventFilter(SDL_EventFilter * filter, void **userdata)
{
    SDL_EventWatcher event_ok;

    if (!SDL_event_watchers_lock || SDL_LockMutex(SDL_event_watchers_lock) == 0) {
        event_ok = SDL_EventOK;

        if (SDL_event_watchers_lock) {
            SDL_UnlockMutex(SDL_event_watchers_lock);
        }
    } else {
        SDL_zero(event_ok);
    }

    if (filter) {
        *filter = event_ok.callback;
    }
    if (userdata) {
        *userdata = event_ok.userdata;
    }
    return event_ok.callback ? SDL_TRUE : SDL_FALSE;
}

void
SDL_AddEventWatch(SDL_EventFilter filter, void *userdata)
{
    if (!SDL_event_watchers_lock || SDL_LockMutex(SDL_event_watchers_lock) == 0) {
        SDL_EventWatcher *event_watchers;

        event_watchers = SDL_realloc(SDL_event_watchers, (SDL_event_watchers_count + 1) * sizeof(*event_watchers));
        if (event_watchers) {
            SDL_EventWatcher *watcher;

            SDL_event_watchers = event_watchers;
            watcher = &SDL_event_watchers[SDL_event_watchers_count];
            watcher->callback = filter;
            watcher->userdata = userdata;
            watcher->removed = SDL_FALSE;
            ++SDL_event_watchers_count;
        }

        if (SDL_event_watchers_lock) {
            SDL_UnlockMutex(SDL_event_watchers_lock);
        }
    }
}

void
SDL_DelEventWatch(SDL_EventFilter filter, void *userdata)
{
    if (!SDL_event_watchers_lock || SDL_LockMutex(SDL_event_watchers_lock) == 0) {
        int i;

        for (i = 0; i < SDL_event_watchers_count; ++i) {
            if (SDL_event_watchers[i].callback == filter && SDL_event_watchers[i].userdata == userdata) {
                if (SDL_event_watchers_dispatching) {
                    SDL_event_watchers[i].removed = SDL_TRUE;
                    SDL_event_watchers_removed = SDL_TRUE;
                } else {
                    --SDL_event_watchers_count;
                    if (i < SDL_event_watchers_count) {
                        SDL_memmove(&SDL_event_watchers[i], &SDL_event_watchers[i+1], (SDL_event_watchers_count - i) * sizeof(SDL_event_watchers[i]));
                    }
                }
                break;
            }
        }

        if (SDL_event_watchers_lock) {
            SDL_UnlockMutex(SDL_event_watchers_lock);
        }
    }
}

void
SDL_FilterEvents(SDL_EventFilter filter, void *userdata)
{
    if (!SDL_EventQ.lock || SDL_LockMutex(SDL_EventQ.lock) == 0) {
        SDL_EventEntry *entry, *next;
        for (entry = SDL_EventQ.head; entry; entry = next) {
            next = entry->next;
            if (!filter(userdata, &entry->event)) {
                SDL_CutEvent(entry);
            }
        }
        if (SDL_EventQ.lock) {
            SDL_UnlockMutex(SDL_EventQ.lock);
        }
    }
}

Uint8
SDL_EventState(Uint32 type, int state)
{
    Uint8 current_state;
    Uint8 hi = ((type >> 8) & 0xff);
    Uint8 lo = (type & 0xff);

    if (SDL_disabled_events[hi] &&
        (SDL_disabled_events[hi]->bits[lo/32] & (1 << (lo&31)))) {
        current_state = SDL_DISABLE;
    } else {
        current_state = SDL_ENABLE;
    }

    if (state != current_state)
    {
        switch (state) {
        case SDL_DISABLE:
            /* Disable this event type and discard pending events */
            if (!SDL_disabled_events[hi]) {
                SDL_disabled_events[hi] = (SDL_DisabledEventBlock*) SDL_calloc(1, sizeof(SDL_DisabledEventBlock));
                if (!SDL_disabled_events[hi]) {
                    /* Out of memory, nothing we can do... */
                    break;
                }
            }
            SDL_disabled_events[hi]->bits[lo/32] |= (1 << (lo&31));
            SDL_FlushEvent(type);
            break;
        case SDL_ENABLE:
            SDL_disabled_events[hi]->bits[lo/32] &= ~(1 << (lo&31));
            break;
        default:
            /* Querying state... */
            break;
        }
    }

    return current_state;
}

Uint32
SDL_RegisterEvents(int numevents)
{
    Uint32 event_base;

    if ((numevents > 0) && (SDL_userevents+numevents <= SDL_LASTEVENT)) {
        event_base = SDL_userevents;
        SDL_userevents += numevents;
    } else {
        event_base = (Uint32)-1;
    }
    return event_base;
}

int
SDL_SendAppEvent(SDL_EventType eventType)
{
    int posted;

    posted = 0;
    if (SDL_GetEventState(eventType) == SDL_ENABLE) {
        SDL_Event event;
        event.type = eventType;
        posted = (SDL_PushEvent(&event) > 0);
    }
    return (posted);
}

int
SDL_SendSysWMEvent(SDL_SysWMmsg * message)
{
    int posted;

    posted = 0;
    if (SDL_GetEventState(SDL_SYSWMEVENT) == SDL_ENABLE) {
        SDL_Event event;
        SDL_memset(&event, 0, sizeof(event));
        event.type = SDL_SYSWMEVENT;
        event.syswm.msg = message;
        posted = (SDL_PushEvent(&event) > 0);
    }
    /* Update internal event state */
    return (posted);
}

int
SDL_SendKeymapChangedEvent(void)
{
    return SDL_SendAppEvent(SDL_KEYMAPCHANGED);
}

/* vi: set ts=4 sw=4 expandtab: */
