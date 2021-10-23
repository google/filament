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

#if SDL_VIDEO_DRIVER_WAYLAND

#include "SDL.h"
#include <stdlib.h> /* fgets */
#include <stdio.h> /* FILE, STDOUT_FILENO, fdopen, fclose */
#include <unistd.h> /* pid_t, pipe, fork, close, dup2, execvp, _exit */
#include <sys/wait.h> /* waitpid, WIFEXITED, WEXITSTATUS */
#include <string.h> /* strerr */
#include <errno.h>

#define MAX_BUTTONS             8       /* Maximum number of buttons supported */

int
Wayland_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
    int fd_pipe[2]; /* fd_pipe[0]: read end of pipe, fd_pipe[1]: write end of pipe */
    pid_t pid1;

    if (messageboxdata->numbuttons > MAX_BUTTONS) {
        return SDL_SetError("Too many buttons (%d max allowed)", MAX_BUTTONS);
    }

    if (pipe(fd_pipe) != 0) { /* create a pipe */
        return SDL_SetError("pipe() failed: %s", strerror(errno));
    }

    pid1 = fork();
    if (pid1 == 0) {  /* child process */
        int argc = 5, i;
        const char* argv[5 + 2/* icon name */ + 2/* title */ + 2/* message */ + 2*MAX_BUTTONS + 1/* NULL */] = {
            "zenity", "--question", "--switch", "--no-wrap", "--no-markup"
        };

        close(fd_pipe[0]); /* no reading from pipe */
        /* write stdout in pipe */
        if (dup2(fd_pipe[1], STDOUT_FILENO) == -1) {
            _exit(128);
        }

        argv[argc++] = "--icon-name";
        switch (messageboxdata->flags) {
        case SDL_MESSAGEBOX_ERROR:
            argv[argc++] = "dialog-error";
            break;
        case SDL_MESSAGEBOX_WARNING:
            argv[argc++] = "dialog-warning";
            break;
        case SDL_MESSAGEBOX_INFORMATION:
        default:
            argv[argc++] = "dialog-information";
            break;
        }

        if (messageboxdata->title && messageboxdata->title[0]) {
            argv[argc++] = "--title";
            argv[argc++] = messageboxdata->title;
        } else {
            argv[argc++] = "--title=\"\"";
        }

        if (messageboxdata->message && messageboxdata->message[0]) {
            argv[argc++] = "--text";
            argv[argc++] = messageboxdata->message;
        } else {
            argv[argc++] = "--text=\"\"";
        }

        for (i = 0; i < messageboxdata->numbuttons; ++i) {
            if (messageboxdata->buttons[i].text && messageboxdata->buttons[i].text[0]) {
                argv[argc++] = "--extra-button";
                argv[argc++] = messageboxdata->buttons[i].text;
            } else {
                argv[argc++] = "--extra-button=\"\"";
            }
        }
        argv[argc] = NULL;

        /* const casting argv is fine:
         * https://pubs.opengroup.org/onlinepubs/9699919799/functions/fexecve.html -> rational
         */
        execvp("zenity", (char **)argv);
        _exit(129);
    } else if (pid1 < 0) {
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        return SDL_SetError("fork() failed: %s", strerror(errno));
    } else {
        int status;
        if (waitpid(pid1, &status, 0) == pid1) {
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) < 128) {
                    int i;
                    size_t output_len = 1;
                    char* output = NULL;
                    char* tmp = NULL;
                    FILE* stdout = NULL;

                    close(fd_pipe[1]); /* no writing to pipe */
                    /* At this point, if no button ID is needed, we can just bail as soon as the
                     * process has completed.
                     */
                    if (buttonid == NULL) {
                        close(fd_pipe[0]);
                        return 0;
                    }
                    *buttonid = -1;

                    /* find button with longest text */
                    for (i = 0; i < messageboxdata->numbuttons; ++i) {
                        if (messageboxdata->buttons[i].text != NULL) {
                            const size_t button_len = SDL_strlen(messageboxdata->buttons[i].text);
                            if (button_len > output_len) {
                                output_len = button_len;
                            }
                        }
                    }
                    output = SDL_malloc(output_len + 1);
                    if (!output) {
                        close(fd_pipe[0]);
                        return SDL_OutOfMemory();
                    }
                    output[0] = '\0';

                    stdout = fdopen(fd_pipe[0], "r");
                    if (!stdout) {
                        SDL_free(output);
                        close(fd_pipe[0]);
                        return SDL_SetError("Couldn't open pipe for reading: %s", strerror(errno));
                    }
                    tmp = fgets(output, output_len + 1, stdout);
                    fclose(stdout);

                    if ((tmp == NULL) || (*tmp == '\0') || (*tmp == '\n')) {
                        SDL_free(output);
                        return 0; /* User simply closed the dialog */
                    }

                    /* It likes to add a newline... */
                    tmp = SDL_strrchr(output, '\n');
                    if (tmp != NULL) {
                        *tmp = '\0';
                    }

                    /* Check which button got pressed */
                    for (i = 0; i < messageboxdata->numbuttons; i += 1) {
                        if (messageboxdata->buttons[i].text != NULL) {
                            if (SDL_strcmp(output, messageboxdata->buttons[i].text) == 0) {
                                *buttonid = messageboxdata->buttons[i].buttonid;
                                break;
                            }
                        }
                    }

                    SDL_free(output);
                    return 0;  /* success! */
                } else {
                    return SDL_SetError("zenity reported error or failed to launch: %d", WEXITSTATUS(status));
                }
            } else {
                return SDL_SetError("zenity failed for some reason");
            }
        } else {
            return SDL_SetError("Waiting on zenity failed: %s", strerror(errno));
        }
    }
    return 0;
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND */

/* vi: set ts=4 sw=4 expandtab: */
