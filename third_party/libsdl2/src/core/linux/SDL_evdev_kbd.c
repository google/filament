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

#include "SDL_evdev_kbd.h"
#include "SDL_hints.h"

#ifdef SDL_INPUT_LINUXKD

/* This logic is adapted from drivers/tty/vt/keyboard.c in the Linux kernel source */

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <linux/vt.h>
#include <linux/tiocl.h> /* for TIOCL_GETSHIFTSTATE */

#include <signal.h>

#include "../../events/SDL_events_c.h"
#include "SDL_evdev_kbd_default_accents.h"
#include "SDL_evdev_kbd_default_keymap.h"

/* These are not defined in older Linux kernel headers */
#ifndef K_UNICODE
#define K_UNICODE 0x03
#endif
#ifndef K_OFF
#define K_OFF 0x04
#endif

/*
 * Handler Tables.
 */

#define K_HANDLERS\
    k_self,     k_fn,       k_spec,       k_pad,\
    k_dead,     k_cons,     k_cur,        k_shift,\
    k_meta,     k_ascii,    k_lock,       k_lowercase,\
    k_slock,    k_dead2,    k_brl,        k_ignore

typedef void (k_handler_fn)(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag);
static k_handler_fn K_HANDLERS;
static k_handler_fn *k_handler[16] = { K_HANDLERS };

typedef void (fn_handler_fn)(SDL_EVDEV_keyboard_state *kbd);
static void fn_enter(SDL_EVDEV_keyboard_state *kbd);
static void fn_caps_toggle(SDL_EVDEV_keyboard_state *kbd);
static void fn_caps_on(SDL_EVDEV_keyboard_state *kbd);
static void fn_num(SDL_EVDEV_keyboard_state *kbd);
static void fn_compose(SDL_EVDEV_keyboard_state *kbd);

static fn_handler_fn *fn_handler[] =
{
    NULL,       fn_enter,   NULL,       NULL,
    NULL,       NULL,       NULL,       fn_caps_toggle,
    fn_num,     NULL,       NULL,       NULL,
    NULL,       fn_caps_on, fn_compose, NULL,
    NULL,       NULL,       NULL,       fn_num
};


/*
 * Keyboard State
 */

struct SDL_EVDEV_keyboard_state
{
    int console_fd;
    int old_kbd_mode;
    unsigned short **key_maps;
    unsigned char shift_down[NR_SHIFT];        /* shift state counters.. */
    SDL_bool dead_key_next;
    int npadch;                    /* -1 or number assembled on pad */
    struct kbdiacrs *accents;
    unsigned int diacr;
    SDL_bool rep;                    /* flag telling character repeat */
    unsigned char lockstate;
    unsigned char slockstate;
    unsigned char ledflagstate;
    char shift_state;
    char text[128];
    unsigned int text_len;
};

#ifdef DUMP_ACCENTS
static void SDL_EVDEV_dump_accents(SDL_EVDEV_keyboard_state *kbd)
{
    unsigned int i;

    printf("static struct kbdiacrs default_accents = {\n");
    printf("    %d,\n", kbd->accents->kb_cnt);
    printf("    {\n");
    for (i = 0; i < kbd->accents->kb_cnt; ++i) {
        struct kbdiacr *diacr = &kbd->accents->kbdiacr[i];
        printf("        { 0x%.2x, 0x%.2x, 0x%.2x },\n",
            diacr->diacr, diacr->base, diacr->result);
    }
    while (i < 256) {
        printf("        { 0x00, 0x00, 0x00 },\n");
        ++i;
    }
    printf("    }\n");
    printf("};\n");
}
#endif /* DUMP_ACCENTS */

#ifdef DUMP_KEYMAP
static void SDL_EVDEV_dump_keymap(SDL_EVDEV_keyboard_state *kbd)
{
    int i, j;

    for (i = 0; i < MAX_NR_KEYMAPS; ++i) {
        if (kbd->key_maps[i]) {
            printf("static unsigned short default_key_map_%d[NR_KEYS] = {", i);
            for (j = 0; j < NR_KEYS; ++j) {
                if ((j%8) == 0) {
                    printf("\n    ");
                }
                printf("0x%.4x, ", kbd->key_maps[i][j]);
            }
            printf("\n};\n");
        }
    }
    printf("\n");
    printf("static unsigned short *default_key_maps[MAX_NR_KEYMAPS] = {\n");
    for (i = 0; i < MAX_NR_KEYMAPS; ++i) {
        if (kbd->key_maps[i]) {
            printf("    default_key_map_%d,\n", i);
        } else {
            printf("    NULL,\n");
        }
    }
    printf("};\n");
}
#endif /* DUMP_KEYMAP */

static int SDL_EVDEV_kbd_load_keymaps(SDL_EVDEV_keyboard_state *kbd)
{
    int i, j;

    kbd->key_maps = (unsigned short **)SDL_calloc(MAX_NR_KEYMAPS, sizeof(unsigned short *));
    if (!kbd->key_maps) {
        return -1;
    }

    for (i = 0; i < MAX_NR_KEYMAPS; ++i) {
        struct kbentry kbe;

        kbe.kb_table = i;
        kbe.kb_index = 0;
        if (ioctl(kbd->console_fd, KDGKBENT, &kbe) < 0) {
            return -1;
        }

        if (kbe.kb_value == K_NOSUCHMAP) {
            continue;
        }

        kbd->key_maps[i] = (unsigned short *)SDL_malloc(NR_KEYS * sizeof(unsigned short));
        if (!kbd->key_maps[i]) {
            return -1;
        }

        for (j = 0; j < NR_KEYS; ++j) {
            kbe.kb_table = i;
            kbe.kb_index = j;
            if (ioctl(kbd->console_fd, KDGKBENT, &kbe) < 0) {
                return -1;
            }
            kbd->key_maps[i][j] = (kbe.kb_value ^ 0xf000);
        }
    }
    return 0;
}

static SDL_EVDEV_keyboard_state * kbd_cleanup_state = NULL;
static int kbd_cleanup_sigactions_installed = 0;
static int kbd_cleanup_atexit_installed = 0;

static struct sigaction old_sigaction[NSIG];

static int fatal_signals[] =
{
    /* Handlers for SIGTERM and SIGINT are installed in SDL_QuitInit. */
    SIGHUP,  SIGQUIT, SIGILL,  SIGABRT,
    SIGFPE,  SIGSEGV, SIGPIPE, SIGBUS,
    SIGSYS
};

static void kbd_cleanup(void)
{
    SDL_EVDEV_keyboard_state* kbd = kbd_cleanup_state;
    if (kbd == NULL) {
        return;
    }
    kbd_cleanup_state = NULL;

    ioctl(kbd->console_fd, KDSKBMODE, kbd->old_kbd_mode);
}

void
SDL_EVDEV_kbd_reraise_signal(int sig)
{
    raise(sig);
}

siginfo_t* SDL_EVDEV_kdb_cleanup_siginfo = NULL;
void*      SDL_EVDEV_kdb_cleanup_ucontext = NULL;

static void kbd_cleanup_signal_action(int signum, siginfo_t* info, void* ucontext)
{
    struct sigaction* old_action_p = &(old_sigaction[signum]);
    sigset_t sigset;

    /* Restore original signal handler before going any further. */
    sigaction(signum, old_action_p, NULL);

    /* Unmask current signal. */
    sigemptyset(&sigset);
    sigaddset(&sigset, signum);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);

    /* Save original signal info and context for archeologists. */
    SDL_EVDEV_kdb_cleanup_siginfo = info;
    SDL_EVDEV_kdb_cleanup_ucontext = ucontext;

    /* Restore keyboard. */
    kbd_cleanup();

    /* Reraise signal. */
    SDL_EVDEV_kbd_reraise_signal(signum);
}

static void kbd_unregister_emerg_cleanup()
{
    int tabidx, signum;

    kbd_cleanup_state = NULL;

    if (!kbd_cleanup_sigactions_installed) {
        return;
    }
    kbd_cleanup_sigactions_installed = 0;

    for (tabidx = 0; tabidx < sizeof(fatal_signals) / sizeof(fatal_signals[0]); ++tabidx) {
        struct sigaction* old_action_p;
        struct sigaction cur_action;
        signum = fatal_signals[tabidx];
        old_action_p = &(old_sigaction[signum]);

        /* Examine current signal action */
        if (sigaction(signum, NULL, &cur_action))
            continue;

        /* Check if action installed and not modifed */
        if (!(cur_action.sa_flags & SA_SIGINFO)
                || cur_action.sa_sigaction != &kbd_cleanup_signal_action)
            continue;

        /* Restore original action */
        sigaction(signum, old_action_p, NULL);
    }
}

static void kbd_cleanup_atexit(void)
{
    /* Restore keyboard. */
    kbd_cleanup();

    /* Try to restore signal handlers in case shared library is being unloaded */
    kbd_unregister_emerg_cleanup();
}

static void kbd_register_emerg_cleanup(SDL_EVDEV_keyboard_state * kbd)
{
    int tabidx, signum;

    if (kbd_cleanup_state != NULL) {
        return;
    }
    kbd_cleanup_state = kbd;

    if (!kbd_cleanup_atexit_installed) {
        /* Since glibc 2.2.3, atexit() (and on_exit(3)) can be used within a shared library to establish
         * functions that are called when the shared library is unloaded.
         * -- man atexit(3)
         */
        atexit(kbd_cleanup_atexit);
        kbd_cleanup_atexit_installed = 1;
    }

    if (kbd_cleanup_sigactions_installed) {
        return;
    }
    kbd_cleanup_sigactions_installed = 1;

    for (tabidx = 0; tabidx < sizeof(fatal_signals) / sizeof(fatal_signals[0]); ++tabidx) {
        struct sigaction* old_action_p;
        struct sigaction new_action;
        signum = fatal_signals[tabidx];   
        old_action_p = &(old_sigaction[signum]);
        if (sigaction(signum, NULL, old_action_p))
            continue;

        /* Skip SIGHUP and SIGPIPE if handler is already installed
         * - assume the handler will do the cleanup
         */
        if ((signum == SIGHUP || signum == SIGPIPE)
                && (old_action_p->sa_handler != SIG_DFL 
                    || (void (*)(int))old_action_p->sa_sigaction != SIG_DFL))
            continue;

        new_action = *old_action_p;
        new_action.sa_flags |= SA_SIGINFO;
        new_action.sa_sigaction = &kbd_cleanup_signal_action;
        sigaction(signum, &new_action, NULL);
    }
}

SDL_EVDEV_keyboard_state *
SDL_EVDEV_kbd_init(void)
{
    SDL_EVDEV_keyboard_state *kbd;
    int i;
    char flag_state;
    char shift_state[ sizeof (long) ] = {TIOCL_GETSHIFTSTATE, 0};

    kbd = (SDL_EVDEV_keyboard_state *)SDL_calloc(1, sizeof(*kbd));
    if (!kbd) {
        return NULL;
    }

    kbd->npadch = -1;

    /* This might fail if we're not connected to a tty (e.g. on the Steam Link) */
    kbd->console_fd = open("/dev/tty", O_RDONLY);

    if (ioctl(kbd->console_fd, TIOCLINUX, shift_state) == 0) {
        kbd->shift_state = *shift_state;
    }

    if (ioctl(kbd->console_fd, KDGKBLED, &flag_state) == 0) {
        kbd->ledflagstate = flag_state;
    }

    kbd->accents = &default_accents;
    if (ioctl(kbd->console_fd, KDGKBDIACR, kbd->accents) < 0) {
        /* No worries, we'll use the default accent table */
    }

    kbd->key_maps = default_key_maps;
    if (ioctl(kbd->console_fd, KDGKBMODE, &kbd->old_kbd_mode) == 0) {
        /* Set the keyboard in UNICODE mode and load the keymaps */
        ioctl(kbd->console_fd, KDSKBMODE, K_UNICODE);

        if (SDL_EVDEV_kbd_load_keymaps(kbd) < 0) {
            for (i = 0; i < MAX_NR_KEYMAPS; ++i) {
                if (kbd->key_maps[i]) {
                    SDL_free(kbd->key_maps[i]);
                }
            }
            SDL_free(kbd->key_maps);

            kbd->key_maps = default_key_maps;
        }

        /* Allow inhibiting keyboard mute with env. variable for debugging etc. */
        if (getenv("SDL_INPUT_LINUX_KEEP_KBD") == NULL) {
            /* Mute the keyboard so keystrokes only generate evdev events
             * and do not leak through to the console
             */
            ioctl(kbd->console_fd, KDSKBMODE, K_OFF);

            /* Make sure to restore keyboard if application fails to call
             * SDL_Quit before exit or fatal signal is raised.
             */
            if (!SDL_GetHintBoolean(SDL_HINT_NO_SIGNAL_HANDLERS, SDL_FALSE)) {
                kbd_register_emerg_cleanup(kbd);
            }
        }
    }

#ifdef DUMP_ACCENTS
    SDL_EVDEV_dump_accents(kbd);
#endif
#ifdef DUMP_KEYMAP
    SDL_EVDEV_dump_keymap(kbd);
#endif
    return kbd;
}

void
SDL_EVDEV_kbd_quit(SDL_EVDEV_keyboard_state *kbd)
{
    if (!kbd) {
        return;
    }

    kbd_unregister_emerg_cleanup();

    if (kbd->console_fd >= 0) {
        /* Restore the original keyboard mode */
        ioctl(kbd->console_fd, KDSKBMODE, kbd->old_kbd_mode);

        close(kbd->console_fd);
        kbd->console_fd = -1;
    }

    if (kbd->key_maps && kbd->key_maps != default_key_maps) {
        int i;
        for (i = 0; i < MAX_NR_KEYMAPS; ++i) {
            if (kbd->key_maps[i]) {
                SDL_free(kbd->key_maps[i]);
            }
        }
        SDL_free(kbd->key_maps);
    }

    SDL_free(kbd);
}

/*
 * Helper Functions.
 */
static void put_queue(SDL_EVDEV_keyboard_state *kbd, uint c)
{
    /* c is already part of a UTF-8 sequence and safe to add as a character */
    if (kbd->text_len < (sizeof(kbd->text)-1)) {
        kbd->text[kbd->text_len++] = (char)c;
    }
}

static void put_utf8(SDL_EVDEV_keyboard_state *kbd, uint c)
{
    if (c < 0x80)
        /*  0******* */
        put_queue(kbd, c);
    else if (c < 0x800) {
        /* 110***** 10****** */
        put_queue(kbd, 0xc0 | (c >> 6));
        put_queue(kbd, 0x80 | (c & 0x3f));
    } else if (c < 0x10000) {
        if (c >= 0xD800 && c < 0xE000)
            return;
        if (c == 0xFFFF)
            return;
        /* 1110**** 10****** 10****** */
        put_queue(kbd, 0xe0 | (c >> 12));
        put_queue(kbd, 0x80 | ((c >> 6) & 0x3f));
        put_queue(kbd, 0x80 | (c & 0x3f));
    } else if (c < 0x110000) {
        /* 11110*** 10****** 10****** 10****** */
        put_queue(kbd, 0xf0 | (c >> 18));
        put_queue(kbd, 0x80 | ((c >> 12) & 0x3f));
        put_queue(kbd, 0x80 | ((c >> 6) & 0x3f));
        put_queue(kbd, 0x80 | (c & 0x3f));
    }
}

/*
 * We have a combining character DIACR here, followed by the character CH.
 * If the combination occurs in the table, return the corresponding value.
 * Otherwise, if CH is a space or equals DIACR, return DIACR.
 * Otherwise, conclude that DIACR was not combining after all,
 * queue it and return CH.
 */
static unsigned int handle_diacr(SDL_EVDEV_keyboard_state *kbd, unsigned int ch)
{
    unsigned int d = kbd->diacr;
    unsigned int i;

    kbd->diacr = 0;

    for (i = 0; i < kbd->accents->kb_cnt; i++) {
        if (kbd->accents->kbdiacr[i].diacr == d &&
            kbd->accents->kbdiacr[i].base == ch) {
            return kbd->accents->kbdiacr[i].result;
        }
    }

    if (ch == ' ' || ch == d)
        return d;

    put_utf8(kbd, d);

    return ch;
}

static int vc_kbd_led(SDL_EVDEV_keyboard_state *kbd, int flag)
{
    return (kbd->ledflagstate & flag) != 0;
}

static void set_vc_kbd_led(SDL_EVDEV_keyboard_state *kbd, int flag)
{
    kbd->ledflagstate |= flag;
    ioctl(kbd->console_fd, KDSETLED, (unsigned long)(kbd->ledflagstate));
}

static void clr_vc_kbd_led(SDL_EVDEV_keyboard_state *kbd, int flag)
{
    kbd->ledflagstate &= ~flag;
    ioctl(kbd->console_fd, KDSETLED, (unsigned long)(kbd->ledflagstate));
}

static void chg_vc_kbd_lock(SDL_EVDEV_keyboard_state *kbd, int flag)
{
    kbd->lockstate ^= 1 << flag;
}

static void chg_vc_kbd_slock(SDL_EVDEV_keyboard_state *kbd, int flag)
{
    kbd->slockstate ^= 1 << flag;
}

static void chg_vc_kbd_led(SDL_EVDEV_keyboard_state *kbd, int flag)
{
    kbd->ledflagstate ^= flag;
    ioctl(kbd->console_fd, KDSETLED, (unsigned long)(kbd->ledflagstate));
}

/*
 * Special function handlers
 */

static void fn_enter(SDL_EVDEV_keyboard_state *kbd)
{
    if (kbd->diacr) {
        put_utf8(kbd, kbd->diacr);
        kbd->diacr = 0;
    }
}

static void fn_caps_toggle(SDL_EVDEV_keyboard_state *kbd)
{
    if (kbd->rep)
        return;

    chg_vc_kbd_led(kbd, K_CAPSLOCK);
}

static void fn_caps_on(SDL_EVDEV_keyboard_state *kbd)
{
    if (kbd->rep)
        return;

    set_vc_kbd_led(kbd, K_CAPSLOCK);
}

static void fn_num(SDL_EVDEV_keyboard_state *kbd)
{
    if (!kbd->rep)
        chg_vc_kbd_led(kbd, K_NUMLOCK);
}

static void fn_compose(SDL_EVDEV_keyboard_state *kbd)
{
    kbd->dead_key_next = SDL_TRUE;
}

/*
 * Special key handlers
 */

static void k_ignore(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
}

static void k_spec(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    if (up_flag)
        return;
    if (value >= SDL_arraysize(fn_handler))
        return;
    if (fn_handler[value])
        fn_handler[value](kbd);
}

static void k_lowercase(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
}

static void k_self(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    if (up_flag)
        return;        /* no action, if this is a key release */

    if (kbd->diacr)
        value = handle_diacr(kbd, value);

    if (kbd->dead_key_next) {
        kbd->dead_key_next = SDL_FALSE;
        kbd->diacr = value;
        return;
    }
    put_utf8(kbd, value);
}

static void k_deadunicode(SDL_EVDEV_keyboard_state *kbd, unsigned int value, char up_flag)
{
    if (up_flag)
        return;

    kbd->diacr = (kbd->diacr ? handle_diacr(kbd, value) : value);
}

static void k_dead(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    const unsigned char ret_diacr[NR_DEAD] = {'`', '\'', '^', '~', '"', ',' };

    k_deadunicode(kbd, ret_diacr[value], up_flag);
}

static void k_dead2(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    k_deadunicode(kbd, value, up_flag);
}

static void k_cons(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
}

static void k_fn(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
}

static void k_cur(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
}

static void k_pad(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    static const char pad_chars[] = "0123456789+-*/\015,.?()#";

    if (up_flag)
        return;        /* no action, if this is a key release */

    if (!vc_kbd_led(kbd, K_NUMLOCK)) {
        /* unprintable action */
        return;
    }

    put_queue(kbd, pad_chars[value]);
}

static void k_shift(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    int old_state = kbd->shift_state;

    if (kbd->rep)
        return;
    /*
     * Mimic typewriter:
     * a CapsShift key acts like Shift but undoes CapsLock
     */
    if (value == KVAL(K_CAPSSHIFT)) {
        value = KVAL(K_SHIFT);
        if (!up_flag)
            clr_vc_kbd_led(kbd, K_CAPSLOCK);
    }

    if (up_flag) {
        /*
         * handle the case that two shift or control
         * keys are depressed simultaneously
         */
        if (kbd->shift_down[value])
            kbd->shift_down[value]--;
    } else
        kbd->shift_down[value]++;

    if (kbd->shift_down[value])
        kbd->shift_state |= (1 << value);
    else
        kbd->shift_state &= ~(1 << value);

    /* kludge */
    if (up_flag && kbd->shift_state != old_state && kbd->npadch != -1) {
        put_utf8(kbd, kbd->npadch);
        kbd->npadch = -1;
    }
}

static void k_meta(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
}

static void k_ascii(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    int base;

    if (up_flag)
        return;

    if (value < 10) {
        /* decimal input of code, while Alt depressed */
        base = 10;
    } else {
        /* hexadecimal input of code, while AltGr depressed */
        value -= 10;
        base = 16;
    }

    if (kbd->npadch == -1)
        kbd->npadch = value;
    else
        kbd->npadch = kbd->npadch * base + value;
}

static void k_lock(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    if (up_flag || kbd->rep)
        return;

    chg_vc_kbd_lock(kbd, value);
}

static void k_slock(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
    k_shift(kbd, value, up_flag);
    if (up_flag || kbd->rep)
        return;

    chg_vc_kbd_slock(kbd, value);
    /* try to make Alt, oops, AltGr and such work */
    if (!kbd->key_maps[kbd->lockstate ^ kbd->slockstate]) {
        kbd->slockstate = 0;
        chg_vc_kbd_slock(kbd, value);
    }
}

static void k_brl(SDL_EVDEV_keyboard_state *kbd, unsigned char value, char up_flag)
{
}

void
SDL_EVDEV_kbd_keycode(SDL_EVDEV_keyboard_state *kbd, unsigned int keycode, int down)
{
    unsigned char shift_final;
    unsigned char type;
    unsigned short *key_map;
    unsigned short keysym;

    if (!kbd) {
        return;
    }

    kbd->rep = (down == 2);

    shift_final = (kbd->shift_state | kbd->slockstate) ^ kbd->lockstate;
    key_map = kbd->key_maps[shift_final];
    if (!key_map) {
        /* Unsupported shift state (e.g. ctrl = 4, alt = 8), just reset to the default state */
        kbd->shift_state = 0;
        kbd->slockstate = 0;
        kbd->lockstate = 0;
        return;
    }

    if (keycode < NR_KEYS) {
        keysym = key_map[keycode];
    } else {
        return;
    }

    type = KTYP(keysym);

    if (type < 0xf0) {
        if (down) {
            put_utf8(kbd, keysym);
        }
    } else {
        type -= 0xf0;

        /* if type is KT_LETTER then it can be affected by Caps Lock */
        if (type == KT_LETTER) {
            type = KT_LATIN;

            if (vc_kbd_led(kbd, K_CAPSLOCK)) {
                key_map = kbd->key_maps[shift_final ^ (1 << KG_SHIFT)];
                if (key_map) {
                    keysym = key_map[keycode];
                }
            }
        }

        (*k_handler[type])(kbd, keysym & 0xff, !down);

        if (type != KT_SLOCK) {
            kbd->slockstate = 0;
        }
    }

    if (kbd->text_len > 0) {
        kbd->text[kbd->text_len] = '\0';
        SDL_SendKeyboardText(kbd->text);
        kbd->text_len = 0;
    }
}

#elif !defined(SDL_INPUT_FBSDKBIO) /* !SDL_INPUT_LINUXKD */

SDL_EVDEV_keyboard_state *
SDL_EVDEV_kbd_init(void)
{
    return NULL;
}

void
SDL_EVDEV_kbd_keycode(SDL_EVDEV_keyboard_state *state, unsigned int keycode, int down)
{
}

void
SDL_EVDEV_kbd_quit(SDL_EVDEV_keyboard_state *state)
{
}

#endif /* SDL_INPUT_LINUXKD */

/* vi: set ts=4 sw=4 expandtab: */
