/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "SDL_test_common.h"

#ifdef __MACOS__
#define HAVE_OPENGL
#endif

#ifdef HAVE_OPENGL

#include "SDL_opengl.h"

typedef struct GL_Context
{
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "../src/render/opengl/SDL_glfuncs.h"
#undef SDL_PROC
} GL_Context;


/* Undefine this if you want a flat cube instead of a rainbow cube */
#define SHADED_CUBE

static SDLTest_CommonState *state;
static SDL_GLContext context;
static GL_Context ctx;

static int LoadContext(GL_Context * data)
{
#if SDL_VIDEO_DRIVER_UIKIT
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_ANDROID
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_PANDORA
#define __SDL_NOGETPROCADDR__
#endif

#if defined __SDL_NOGETPROCADDR__
#define SDL_PROC(ret,func,params) data->func=func;
#else
#define SDL_PROC(ret,func,params) \
    do { \
        data->func = SDL_GL_GetProcAddress(#func); \
        if ( ! data->func ) { \
            return SDL_SetError("Couldn't load GL function %s: %s", #func, SDL_GetError()); \
        } \
    } while ( 0 );
#endif /* __SDL_NOGETPROCADDR__ */

#include "../src/render/opengl/SDL_glfuncs.h"
#undef SDL_PROC
    return 0;
}


/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    if (context) {
        /* SDL_GL_MakeCurrent(0, NULL); *//* doesn't do anything */
        SDL_GL_DeleteContext(context);
    }
    SDLTest_CommonQuit(state);
    exit(rc);
}

static void
Render()
{
    static float color[8][3] = {
        {1.0, 1.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        {1.0, 0.0, 1.0},
        {0.0, 0.0, 1.0}
    };
    static float cube[8][3] = {
        {0.5, 0.5, -0.5},
        {0.5, -0.5, -0.5},
        {-0.5, -0.5, -0.5},
        {-0.5, 0.5, -0.5},
        {-0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5},
        {0.5, -0.5, 0.5},
        {-0.5, -0.5, 0.5}
    };

    /* Do our drawing, too. */
    ctx.glClearColor(0.0, 0.0, 0.0, 1.0);
    ctx.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ctx.glBegin(GL_QUADS);

#ifdef SHADED_CUBE
    ctx.glColor3fv(color[0]);
    ctx.glVertex3fv(cube[0]);
    ctx.glColor3fv(color[1]);
    ctx.glVertex3fv(cube[1]);
    ctx.glColor3fv(color[2]);
    ctx.glVertex3fv(cube[2]);
    ctx.glColor3fv(color[3]);
    ctx.glVertex3fv(cube[3]);

    ctx.glColor3fv(color[3]);
    ctx.glVertex3fv(cube[3]);
    ctx.glColor3fv(color[4]);
    ctx.glVertex3fv(cube[4]);
    ctx.glColor3fv(color[7]);
    ctx.glVertex3fv(cube[7]);
    ctx.glColor3fv(color[2]);
    ctx.glVertex3fv(cube[2]);

    ctx.glColor3fv(color[0]);
    ctx.glVertex3fv(cube[0]);
    ctx.glColor3fv(color[5]);
    ctx.glVertex3fv(cube[5]);
    ctx.glColor3fv(color[6]);
    ctx.glVertex3fv(cube[6]);
    ctx.glColor3fv(color[1]);
    ctx.glVertex3fv(cube[1]);

    ctx.glColor3fv(color[5]);
    ctx.glVertex3fv(cube[5]);
    ctx.glColor3fv(color[4]);
    ctx.glVertex3fv(cube[4]);
    ctx.glColor3fv(color[7]);
    ctx.glVertex3fv(cube[7]);
    ctx.glColor3fv(color[6]);
    ctx.glVertex3fv(cube[6]);

    ctx.glColor3fv(color[5]);
    ctx.glVertex3fv(cube[5]);
    ctx.glColor3fv(color[0]);
    ctx.glVertex3fv(cube[0]);
    ctx.glColor3fv(color[3]);
    ctx.glVertex3fv(cube[3]);
    ctx.glColor3fv(color[4]);
    ctx.glVertex3fv(cube[4]);

    ctx.glColor3fv(color[6]);
    ctx.glVertex3fv(cube[6]);
    ctx.glColor3fv(color[1]);
    ctx.glVertex3fv(cube[1]);
    ctx.glColor3fv(color[2]);
    ctx.glVertex3fv(cube[2]);
    ctx.glColor3fv(color[7]);
    ctx.glVertex3fv(cube[7]);
#else /* flat cube */
    ctx.glColor3f(1.0, 0.0, 0.0);
    ctx.glVertex3fv(cube[0]);
    ctx.glVertex3fv(cube[1]);
    ctx.glVertex3fv(cube[2]);
    ctx.glVertex3fv(cube[3]);

    ctx.glColor3f(0.0, 1.0, 0.0);
    ctx.glVertex3fv(cube[3]);
    ctx.glVertex3fv(cube[4]);
    ctx.glVertex3fv(cube[7]);
    ctx.glVertex3fv(cube[2]);

    ctx.glColor3f(0.0, 0.0, 1.0);
    ctx.glVertex3fv(cube[0]);
    ctx.glVertex3fv(cube[5]);
    ctx.glVertex3fv(cube[6]);
    ctx.glVertex3fv(cube[1]);

    ctx.glColor3f(0.0, 1.0, 1.0);
    ctx.glVertex3fv(cube[5]);
    ctx.glVertex3fv(cube[4]);
    ctx.glVertex3fv(cube[7]);
    ctx.glVertex3fv(cube[6]);

    ctx.glColor3f(1.0, 1.0, 0.0);
    ctx.glVertex3fv(cube[5]);
    ctx.glVertex3fv(cube[0]);
    ctx.glVertex3fv(cube[3]);
    ctx.glVertex3fv(cube[4]);

    ctx.glColor3f(1.0, 0.0, 1.0);
    ctx.glVertex3fv(cube[6]);
    ctx.glVertex3fv(cube[1]);
    ctx.glVertex3fv(cube[2]);
    ctx.glVertex3fv(cube[7]);
#endif /* SHADED_CUBE */

    ctx.glEnd();

    ctx.glMatrixMode(GL_MODELVIEW);
    ctx.glRotatef(5.0, 1.0, 1.0, 1.0);
}

int
main(int argc, char *argv[])
{
    int fsaa, accel;
    int value;
    int i, done;
    SDL_DisplayMode mode;
    SDL_Event event;
    Uint32 then, now, frames;
    int status;
    int dw, dh;
    int swap_interval = 0;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize parameters */
    fsaa = 0;
    accel = -1;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }
    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (consumed == 0) {
            if (SDL_strcasecmp(argv[i], "--fsaa") == 0 && i+1 < argc) {
                fsaa = atoi(argv[i+1]);
                consumed = 2;
            } else if (SDL_strcasecmp(argv[i], "--accel") == 0 && i+1 < argc) {
                accel = atoi(argv[i+1]);
                consumed = 2;
            } else {
                consumed = -1;
            }
        }
        if (consumed < 0) {
            static const char *options[] = { "[--fsaa n]", "[--accel n]", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            quit(1);
        }
        i += consumed;
    }

    /* Set OpenGL parameters */
    state->window_flags |= SDL_WINDOW_OPENGL;
    state->gl_red_size = 5;
    state->gl_green_size = 5;
    state->gl_blue_size = 5;
    state->gl_depth_size = 16;
    state->gl_double_buffer = 1;
    if (fsaa) {
        state->gl_multisamplebuffers = 1;
        state->gl_multisamplesamples = fsaa;
    }
    if (accel >= 0) {
        state->gl_accelerated = accel;
    }

    if (!SDLTest_CommonInit(state)) {
        quit(2);
    }

    /* Create OpenGL context */
    context = SDL_GL_CreateContext(state->windows[0]);
    if (!context) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_GL_CreateContext(): %s\n", SDL_GetError());
        quit(2);
    }
    
    /* Important: call this *after* creating the context */
    if (LoadContext(&ctx) < 0) {
        SDL_Log("Could not load GL functions\n");
        quit(2);
        return 0;
    }

    if (state->render_flags & SDL_RENDERER_PRESENTVSYNC) {
        /* try late-swap-tearing first. If not supported, try normal vsync. */
        if (SDL_GL_SetSwapInterval(-1) == 0) {
            swap_interval = -1;
        } else {
            SDL_GL_SetSwapInterval(1);
            swap_interval = 1;
        }
    } else {
        SDL_GL_SetSwapInterval(0);  /* disable vsync. */
        swap_interval = 0;
    }

    SDL_GetCurrentDisplayMode(0, &mode);
    SDL_Log("Screen BPP    : %d\n", SDL_BITSPERPIXEL(mode.format));
    SDL_Log("Swap Interval : %d\n", SDL_GL_GetSwapInterval());
    SDL_GetWindowSize(state->windows[0], &dw, &dh);
    SDL_Log("Window Size   : %d,%d\n", dw, dh);
    SDL_GL_GetDrawableSize(state->windows[0], &dw, &dh);
    SDL_Log("Draw Size     : %d,%d\n", dw, dh);
    SDL_Log("\n");
    SDL_Log("Vendor        : %s\n", ctx.glGetString(GL_VENDOR));
    SDL_Log("Renderer      : %s\n", ctx.glGetString(GL_RENDERER));
    SDL_Log("Version       : %s\n", ctx.glGetString(GL_VERSION));
    SDL_Log("Extensions    : %s\n", ctx.glGetString(GL_EXTENSIONS));
    SDL_Log("\n");

    status = SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_RED_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_RED_SIZE: %s\n", SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_GREEN_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_GREEN_SIZE: %s\n", SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_BLUE_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_BLUE_SIZE: %s\n", SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_DEPTH_SIZE: requested %d, got %d\n", 16, value);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_DEPTH_SIZE: %s\n", SDL_GetError());
    }
    if (fsaa) {
        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value);
        if (!status) {
            SDL_Log("SDL_GL_MULTISAMPLEBUFFERS: requested 1, got %d\n", value);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_MULTISAMPLEBUFFERS: %s\n",
                   SDL_GetError());
        }
        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value);
        if (!status) {
            SDL_Log("SDL_GL_MULTISAMPLESAMPLES: requested %d, got %d\n", fsaa,
                   value);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_MULTISAMPLESAMPLES: %s\n",
                   SDL_GetError());
        }
    }
    if (accel >= 0) {
        status = SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &value);
        if (!status) {
            SDL_Log("SDL_GL_ACCELERATED_VISUAL: requested %d, got %d\n", accel,
                   value);
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get SDL_GL_ACCELERATED_VISUAL: %s\n",
                   SDL_GetError());
        }
    }

    /* Set rendering settings */
    ctx.glMatrixMode(GL_PROJECTION);
    ctx.glLoadIdentity();
    ctx.glOrtho(-2.0, 2.0, -2.0, 2.0, -20.0, 20.0);
    ctx.glMatrixMode(GL_MODELVIEW);
    ctx.glLoadIdentity();
    ctx.glEnable(GL_DEPTH_TEST);
    ctx.glDepthFunc(GL_LESS);
    ctx.glShadeModel(GL_SMOOTH);
    
    /* Main render loop */
    frames = 0;
    then = SDL_GetTicks();
    done = 0;
    while (!done) {
        SDL_bool update_swap_interval = SDL_FALSE;

        /* Check for events */
        ++frames;
        while (SDL_PollEvent(&event)) {
            SDLTest_CommonEvent(state, &event, &done);
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_o) {
                    swap_interval--;
                    update_swap_interval = SDL_TRUE;
                } else if (event.key.keysym.sym == SDLK_p) {
                    swap_interval++;
                    update_swap_interval = SDL_TRUE;
                }
            }
        }

        if (update_swap_interval) {
            SDL_Log("Swap interval to be set to %d\n", swap_interval);
        }

        for (i = 0; i < state->num_windows; ++i) {
            int w, h;
            if (state->windows[i] == NULL)
                continue;
            SDL_GL_MakeCurrent(state->windows[i], context);
            if (update_swap_interval) {
                SDL_GL_SetSwapInterval(swap_interval);
            }
            SDL_GL_GetDrawableSize(state->windows[i], &w, &h);
            ctx.glViewport(0, 0, w, h);
            Render();
            SDL_GL_SwapWindow(state->windows[i]);
        }
    }

    /* Print out some timing information */
    now = SDL_GetTicks();
    if (now > then) {
        SDL_Log("%2.2f frames per second\n",
               ((double) frames * 1000) / (now - then));
    }
    quit(0);
    return 0;
}

#else /* HAVE_OPENGL */

int
main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No OpenGL support on this system\n");
    return 1;
}

#endif /* HAVE_OPENGL */
