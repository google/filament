/*
    SDL_windows_main.c, placed in the public domain by Sam Lantinga  4/13/98

    The WinMain function -- calls your program's main() function
*/
#include "SDL_config.h"

#ifdef __WIN32__

/* Include this so we define UNICODE properly */
#include "../../core/windows/SDL_windows.h"

/* Include the SDL main definition header */
#include "SDL.h"
#include "SDL_main.h"

#ifdef main
#  undef main
#endif /* main */

static void
UnEscapeQuotes(char *arg)
{
    char *last = NULL;

    while (*arg) {
        if (*arg == '"' && (last != NULL && *last == '\\')) {
            char *c_curr = arg;
            char *c_last = last;

            while (*c_curr) {
                *c_last = *c_curr;
                c_last = c_curr;
                c_curr++;
            }
            *c_last = '\0';
        }
        last = arg;
        arg++;
    }
}

/* Parse a command line buffer into arguments */
static int
ParseCommandLine(char *cmdline, char **argv)
{
    char *bufp;
    char *lastp = NULL;
    int argc, last_argc;

    argc = last_argc = 0;
    for (bufp = cmdline; *bufp;) {
        /* Skip leading whitespace */
        while (*bufp == ' ' || *bufp == '\t') {
            ++bufp;
        }
        /* Skip over argument */
        if (*bufp == '"') {
            ++bufp;
            if (*bufp) {
                if (argv) {
                    argv[argc] = bufp;
                }
                ++argc;
            }
            /* Skip over word */
            lastp = bufp;
            while (*bufp && (*bufp != '"' || *lastp == '\\')) {
                lastp = bufp;
                ++bufp;
            }
        } else {
            if (*bufp) {
                if (argv) {
                    argv[argc] = bufp;
                }
                ++argc;
            }
            /* Skip over word */
            while (*bufp && (*bufp != ' ' && *bufp != '\t')) {
                ++bufp;
            }
        }
        if (*bufp) {
            if (argv) {
                *bufp = '\0';
            }
            ++bufp;
        }

        /* Strip out \ from \" sequences */
        if (argv && last_argc != argc) {
            UnEscapeQuotes(argv[last_argc]);
        }
        last_argc = argc;
    }
    if (argv) {
        argv[argc] = NULL;
    }
    return (argc);
}

/* Pop up an out of memory message, returns to Windows */
static BOOL
OutOfMemory(void)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", "Out of memory - aborting", NULL);
    return FALSE;
}

#if defined(_MSC_VER)
/* The VC++ compiler needs main/wmain defined */
# define console_ansi_main main
# if UNICODE
#  define console_wmain wmain
# endif
#endif

/* WinMain, main, and wmain eventually call into here. */
static int
main_utf8(int argc, char *argv[])
{
    SDL_SetMainReady();

    /* Run the application main() code */
    return SDL_main(argc, argv);
}

/* Gets the arguments with GetCommandLine, converts them to argc and argv
   and calls main_utf8 */
static int
main_getcmdline()
{
    char **argv;
    int argc;
    char *cmdline;
    int retval = 0;

    /* Grab the command line */
    TCHAR *text = GetCommandLine();
#if UNICODE
    cmdline = WIN_StringToUTF8(text);
#else
    /* !!! FIXME: are these in the system codepage? We need to convert to UTF-8. */
    cmdline = SDL_strdup(text);
#endif
    if (cmdline == NULL) {
        return OutOfMemory();
    }

    /* Parse it into argv and argc */
    argc = ParseCommandLine(cmdline, NULL);
    argv = SDL_stack_alloc(char *, argc + 1);
    if (argv == NULL) {
        return OutOfMemory();
    }
    ParseCommandLine(cmdline, argv);

    retval = main_utf8(argc, argv);

    SDL_stack_free(argv);
    SDL_free(cmdline);

    return retval;
}

/* This is where execution begins [console apps, ansi] */
int
console_ansi_main(int argc, char *argv[])
{
    return main_getcmdline();
}


#if UNICODE
/* This is where execution begins [console apps, unicode] */
int
console_wmain(int argc, wchar_t *wargv[], wchar_t *wenvp)
{
    int retval = 0;
    char **argv = SDL_stack_alloc(char*, argc + 1);
    int i;

    for (i = 0; i < argc; ++i) {
        argv[i] = WIN_StringToUTF8(wargv[i]);
    }
    argv[argc] = NULL;

    retval = main_utf8(argc, argv);

    /* !!! FIXME: we are leaking all the elements of argv we allocated. */
    SDL_stack_free(argv);

    return retval;
}
#endif

/* This is where execution begins [windowed apps] */
int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
    return main_getcmdline();
}

#endif /* __WIN32__ */

/* vi: set ts=4 sw=4 expandtab: */
