/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Game controller mapping generator */
/* Gabriel Jacobo <gabomdq@gmail.com> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#ifndef SDL_JOYSTICK_DISABLED

/* Define this for verbose output while mapping controllers */
#define DEBUG_CONTROLLERMAP

#define SCREEN_WIDTH    512
#define SCREEN_HEIGHT   320

#define MARKER_BUTTON 1
#define MARKER_AXIS 2

enum
{
    SDL_CONTROLLER_BINDING_AXIS_LEFTX_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_LEFTX_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_LEFTY_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_LEFTY_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTX_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTX_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTY_NEGATIVE,
    SDL_CONTROLLER_BINDING_AXIS_RIGHTY_POSITIVE,
    SDL_CONTROLLER_BINDING_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_BINDING_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_BINDING_AXIS_MAX,
};

#define BINDING_COUNT (SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_MAX)

static struct 
{
    int x, y;
    double angle;
    int marker;

} s_arrBindingDisplay[] = {
    { 387, 167, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_A */
    { 431, 132, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_B */
    { 342, 132, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_X */
    { 389, 101, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_Y */
    { 174, 132, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_BACK */
    { 232, 128, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_GUIDE */
    { 289, 132, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_START */
    {  75, 154, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_LEFTSTICK */
    { 305, 230, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_RIGHTSTICK */
    {  77,  40, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_LEFTSHOULDER */
    { 396,  36, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_RIGHTSHOULDER */
    { 154, 188, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_DPAD_UP */
    { 154, 249, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_DPAD_DOWN */
    { 116, 217, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_DPAD_LEFT */
    { 186, 217, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_DPAD_RIGHT */
    { 232, 174, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_MISC1 */
    { 132, 135, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_PADDLE1 */
    { 330, 135, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_PADDLE2 */
    { 132, 175, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_PADDLE3 */
    { 330, 175, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_PADDLE4 */
    {   0,   0, 0.0, MARKER_BUTTON }, /* SDL_CONTROLLER_BUTTON_TOUCHPAD */
    {  74, 153, 270.0, MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_LEFTX_NEGATIVE */
    {  74, 153, 90.0,  MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_LEFTX_POSITIVE */
    {  74, 153, 0.0,   MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_LEFTY_NEGATIVE */
    {  74, 153, 180.0, MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_LEFTY_POSITIVE */
    { 306, 231, 270.0, MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_RIGHTX_NEGATIVE */
    { 306, 231, 90.0,  MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_RIGHTX_POSITIVE */
    { 306, 231, 0.0,   MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_RIGHTY_NEGATIVE */
    { 306, 231, 180.0, MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_RIGHTY_POSITIVE */
    {  91, -20, 180.0, MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_TRIGGERLEFT */
    { 375, -20, 180.0, MARKER_AXIS }, /* SDL_CONTROLLER_BINDING_AXIS_TRIGGERRIGHT */
};
SDL_COMPILE_TIME_ASSERT(s_arrBindingDisplay, SDL_arraysize(s_arrBindingDisplay) == BINDING_COUNT);

static int s_arrBindingOrder[] = {
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTX_NEGATIVE,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTX_POSITIVE,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTY_NEGATIVE,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_LEFTY_POSITIVE,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTX_NEGATIVE,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTX_POSITIVE,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTY_NEGATIVE,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_RIGHTY_POSITIVE,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_MAX + SDL_CONTROLLER_BINDING_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_MISC1,
    SDL_CONTROLLER_BUTTON_PADDLE1,
    SDL_CONTROLLER_BUTTON_PADDLE2,
    SDL_CONTROLLER_BUTTON_PADDLE3,
    SDL_CONTROLLER_BUTTON_PADDLE4,
    -1,
};
SDL_COMPILE_TIME_ASSERT(s_arrBindingOrder, SDL_arraysize(s_arrBindingOrder) == BINDING_COUNT);

typedef struct
{
    SDL_GameControllerBindType bindType;
    union
    {
        int button;

        struct {
            int axis;
            int axis_min;
            int axis_max;
        } axis;

        struct {
            int hat;
            int hat_mask;
        } hat;

    } value;

    SDL_bool committed;

} SDL_GameControllerExtendedBind;

static SDL_GameControllerExtendedBind s_arrBindings[BINDING_COUNT];

typedef struct
{
    SDL_bool m_bMoving;
    int m_nLastValue;
    int m_nStartingValue;
    int m_nFarthestValue;
} AxisState;

static int s_nNumAxes;
static AxisState *s_arrAxisState;
    
static int s_iCurrentBinding;
static Uint32 s_unPendingAdvanceTime;
static SDL_bool s_bBindingComplete;

static SDL_Window *window;
static SDL_Renderer *screen;
static SDL_bool done = SDL_FALSE;

SDL_Texture *
LoadTexture(SDL_Renderer *renderer, const char *file, SDL_bool transparent)
{
    SDL_Surface *temp;
    SDL_Texture *texture;

    /* Load the sprite image */
    temp = SDL_LoadBMP(file);
    if (temp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s", file, SDL_GetError());
        return NULL;
    }

    /* Set transparent pixel as the pixel at (0,0) */
    if (transparent) {
        if (temp->format->palette) {
            SDL_SetColorKey(temp, SDL_TRUE, *(Uint8 *) temp->pixels);
        }
    }

    /* Create textures from the image */
    texture = SDL_CreateTextureFromSurface(renderer, temp);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(temp);
        return NULL;
    }
    SDL_FreeSurface(temp);

    /* We're ready to roll. :) */
    return texture;
}

static int
StandardizeAxisValue(int nValue)
{
    if (nValue > SDL_JOYSTICK_AXIS_MAX/2) {
        return SDL_JOYSTICK_AXIS_MAX;
    } else if (nValue < SDL_JOYSTICK_AXIS_MIN/2) {
        return SDL_JOYSTICK_AXIS_MIN;
    } else {
        return 0;
    }
}

static void
SetCurrentBinding(int iBinding)
{
    int iIndex;
    SDL_GameControllerExtendedBind *pBinding;

    if (iBinding < 0) {
        return;
    }

    if (iBinding == BINDING_COUNT) {
        s_bBindingComplete = SDL_TRUE;
        return;
    }

    if (s_arrBindingOrder[iBinding] == -1)
    {
        SetCurrentBinding(iBinding + 1);
        return;
    }

    s_iCurrentBinding = iBinding;

    pBinding = &s_arrBindings[s_arrBindingOrder[s_iCurrentBinding]];
    SDL_zerop(pBinding);

    for (iIndex = 0; iIndex < s_nNumAxes; ++iIndex) {
        s_arrAxisState[iIndex].m_nFarthestValue = s_arrAxisState[iIndex].m_nStartingValue;
    }

    s_unPendingAdvanceTime = 0;
}

static SDL_bool
BBindingContainsBinding(const SDL_GameControllerExtendedBind *pBindingA, const SDL_GameControllerExtendedBind *pBindingB)
{
    if (pBindingA->bindType != pBindingB->bindType)
    {
        return SDL_FALSE;
    }
    switch (pBindingA->bindType)
    {
    case SDL_CONTROLLER_BINDTYPE_AXIS:
        if (pBindingA->value.axis.axis != pBindingB->value.axis.axis) {
            return SDL_FALSE;
        }
        if (!pBindingA->committed) {
            return SDL_FALSE;
        }
        {
            int minA = SDL_min(pBindingA->value.axis.axis_min, pBindingA->value.axis.axis_max);
            int maxA = SDL_max(pBindingA->value.axis.axis_min, pBindingA->value.axis.axis_max);
            int minB = SDL_min(pBindingB->value.axis.axis_min, pBindingB->value.axis.axis_max);
            int maxB = SDL_max(pBindingB->value.axis.axis_min, pBindingB->value.axis.axis_max);
            return (minA <= minB && maxA >= maxB);
        }
        /* Not reached */
    default:
        return SDL_memcmp(pBindingA, pBindingB, sizeof(*pBindingA)) == 0;
    }
}

static void
ConfigureBinding(const SDL_GameControllerExtendedBind *pBinding)
{
    SDL_GameControllerExtendedBind *pCurrent;
    int iIndex;
    int iCurrentElement = s_arrBindingOrder[s_iCurrentBinding];

    /* Do we already have this binding? */
    for (iIndex = 0; iIndex < SDL_arraysize(s_arrBindings); ++iIndex) {
        pCurrent = &s_arrBindings[iIndex];
        if (BBindingContainsBinding(pCurrent, pBinding)) {
            if (iIndex == SDL_CONTROLLER_BUTTON_A && iCurrentElement != SDL_CONTROLLER_BUTTON_B) {
                /* Skip to the next binding */
                SetCurrentBinding(s_iCurrentBinding + 1);
                return;
            }

            if (iIndex == SDL_CONTROLLER_BUTTON_B) {
                /* Go back to the previous binding */
                SetCurrentBinding(s_iCurrentBinding - 1);
                return;
            }

            /* Already have this binding, ignore it */
            return;
        }
    }

#ifdef DEBUG_CONTROLLERMAP
    switch ( pBinding->bindType )
    {
    case SDL_CONTROLLER_BINDTYPE_NONE:
            break;
    case SDL_CONTROLLER_BINDTYPE_BUTTON:
            SDL_Log("Configuring button binding for button %d\n", pBinding->value.button);
            break;
    case SDL_CONTROLLER_BINDTYPE_AXIS:
            SDL_Log("Configuring axis binding for axis %d %d/%d committed = %s\n", pBinding->value.axis.axis, pBinding->value.axis.axis_min, pBinding->value.axis.axis_max, pBinding->committed ? "true" : "false");
            break;
    case SDL_CONTROLLER_BINDTYPE_HAT:
            SDL_Log("Configuring hat binding for hat %d %d\n", pBinding->value.hat.hat, pBinding->value.hat.hat_mask);
            break;
    }
#endif /* DEBUG_CONTROLLERMAP */

    /* Should the new binding override the existing one? */
    pCurrent = &s_arrBindings[iCurrentElement];
    if (pCurrent->bindType != SDL_CONTROLLER_BINDTYPE_NONE) {
        SDL_bool bNativeDPad, bCurrentDPad;
        SDL_bool bNativeAxis, bCurrentAxis;
        
        bNativeDPad = (iCurrentElement == SDL_CONTROLLER_BUTTON_DPAD_UP ||
                       iCurrentElement == SDL_CONTROLLER_BUTTON_DPAD_DOWN ||
                       iCurrentElement == SDL_CONTROLLER_BUTTON_DPAD_LEFT ||
                       iCurrentElement == SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        bCurrentDPad = (pCurrent->bindType == SDL_CONTROLLER_BINDTYPE_HAT);
        if (bNativeDPad && bCurrentDPad) {
            /* We already have a binding of the type we want, ignore the new one */
            return;
        }

        bNativeAxis = (iCurrentElement >= SDL_CONTROLLER_BUTTON_MAX);
        bCurrentAxis = (pCurrent->bindType == SDL_CONTROLLER_BINDTYPE_AXIS);
        if (bNativeAxis == bCurrentAxis &&
            (pBinding->bindType != SDL_CONTROLLER_BINDTYPE_AXIS ||
             pBinding->value.axis.axis != pCurrent->value.axis.axis)) {
            /* We already have a binding of the type we want, ignore the new one */
            return;
        }
    }

    *pCurrent = *pBinding;

    if (pBinding->committed) {
        s_unPendingAdvanceTime = SDL_GetTicks();
    } else {
        s_unPendingAdvanceTime = 0;
    }
}

static SDL_bool
BMergeAxisBindings(int iIndex)
{
    SDL_GameControllerExtendedBind *pBindingA = &s_arrBindings[iIndex];
    SDL_GameControllerExtendedBind *pBindingB = &s_arrBindings[iIndex+1];
    if (pBindingA->bindType == SDL_CONTROLLER_BINDTYPE_AXIS &&
        pBindingB->bindType == SDL_CONTROLLER_BINDTYPE_AXIS &&
        pBindingA->value.axis.axis == pBindingB->value.axis.axis) {
        if (pBindingA->value.axis.axis_min == pBindingB->value.axis.axis_min) {
            pBindingA->value.axis.axis_min = pBindingA->value.axis.axis_max;
            pBindingA->value.axis.axis_max = pBindingB->value.axis.axis_max;
            pBindingB->bindType = SDL_CONTROLLER_BINDTYPE_NONE;
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static void
WatchJoystick(SDL_Joystick * joystick)
{
    SDL_Texture *background_front, *background_back, *button, *axis, *marker;
    const char *name = NULL;
    SDL_Event event;
    SDL_Rect dst;
    Uint8 alpha=200, alpha_step = -1;
    Uint32 alpha_ticks = 0;
    SDL_JoystickID nJoystickID;

    background_front = LoadTexture(screen, "controllermap.bmp", SDL_FALSE);
    background_back = LoadTexture(screen, "controllermap_back.bmp", SDL_FALSE);
    button = LoadTexture(screen, "button.bmp", SDL_TRUE);
    axis = LoadTexture(screen, "axis.bmp", SDL_TRUE);
    SDL_RaiseWindow(window);

    /* scale for platforms that don't give you the window size you asked for. */
    SDL_RenderSetLogicalSize(screen, SCREEN_WIDTH, SCREEN_HEIGHT);

    /* Print info about the joystick we are watching */
    name = SDL_JoystickName(joystick);
    SDL_Log("Watching joystick %d: (%s)\n", SDL_JoystickInstanceID(joystick),
           name ? name : "Unknown Joystick");
    SDL_Log("Joystick has %d axes, %d hats, %d balls, and %d buttons\n",
           SDL_JoystickNumAxes(joystick), SDL_JoystickNumHats(joystick),
           SDL_JoystickNumBalls(joystick), SDL_JoystickNumButtons(joystick));
    
    SDL_Log("\n\n\
    ====================================================================================\n\
    Press the buttons on your controller when indicated\n\
    (Your controller may look different than the picture)\n\
    If you want to correct a mistake, press backspace or the back button on your device\n\
    To skip a button, press SPACE or click/touch the screen\n\
    To exit, press ESC\n\
    ====================================================================================\n");

    nJoystickID = SDL_JoystickInstanceID(joystick);

    s_nNumAxes = SDL_JoystickNumAxes(joystick);
    s_arrAxisState = (AxisState *)SDL_calloc(s_nNumAxes, sizeof(*s_arrAxisState));

    /* Skip any spurious events at start */
    while (SDL_PollEvent(&event) > 0) {
        continue;
    }

    /* Loop, getting joystick events! */
    while (!done && !s_bBindingComplete) {
        int iElement = s_arrBindingOrder[s_iCurrentBinding];

        switch (s_arrBindingDisplay[iElement].marker) {
            case MARKER_AXIS:
                marker = axis;
                break;
            case MARKER_BUTTON:
                marker = button;
                break;
            default:
                break;
        }
        
        dst.x = s_arrBindingDisplay[iElement].x;
        dst.y = s_arrBindingDisplay[iElement].y;
        SDL_QueryTexture(marker, NULL, NULL, &dst.w, &dst.h);

        if (SDL_GetTicks() - alpha_ticks > 5) {
            alpha_ticks = SDL_GetTicks();
            alpha += alpha_step;
            if (alpha == 255) {
                alpha_step = -1;
            }
            if (alpha < 128) {
                alpha_step = 1;
            }
        }

        SDL_SetRenderDrawColor(screen, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(screen);
        if (s_arrBindingOrder[s_iCurrentBinding] >= SDL_CONTROLLER_BUTTON_PADDLE1 &&
            s_arrBindingOrder[s_iCurrentBinding] <= SDL_CONTROLLER_BUTTON_PADDLE4) {
            SDL_RenderCopy(screen, background_back, NULL, NULL);
        } else {
            SDL_RenderCopy(screen, background_front, NULL, NULL);
        }
        SDL_SetTextureAlphaMod(marker, alpha);
        SDL_SetTextureColorMod(marker, 10, 255, 21);
        SDL_RenderCopyEx(screen, marker, NULL, &dst, s_arrBindingDisplay[iElement].angle, NULL, SDL_FLIP_NONE);
        SDL_RenderPresent(screen);
            
        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
            case SDL_JOYDEVICEREMOVED:
                if (event.jaxis.which == nJoystickID) {
                    done = SDL_TRUE;
                }
                break;
            case SDL_JOYAXISMOTION:
                if (event.jaxis.which == nJoystickID) {
                    const int MAX_ALLOWED_JITTER = SDL_JOYSTICK_AXIS_MAX / 80;  /* ShanWan PS3 controller needed 96 */
                    AxisState *pAxisState = &s_arrAxisState[event.jaxis.axis];
                    int nValue = event.jaxis.value;
                    int nCurrentDistance, nFarthestDistance;
                    if (!pAxisState->m_bMoving) {
                        Sint16 nInitialValue;
                        pAxisState->m_bMoving = SDL_JoystickGetAxisInitialState(joystick, event.jaxis.axis, &nInitialValue);
                        pAxisState->m_nLastValue = nValue;
                        pAxisState->m_nStartingValue = nInitialValue;
                        pAxisState->m_nFarthestValue = nInitialValue;
                    } else if (SDL_abs(nValue - pAxisState->m_nLastValue) <= MAX_ALLOWED_JITTER) {
                        break;
                    } else {
                        pAxisState->m_nLastValue = nValue;
                    }
                    nCurrentDistance = SDL_abs(nValue - pAxisState->m_nStartingValue);
                    nFarthestDistance = SDL_abs(pAxisState->m_nFarthestValue - pAxisState->m_nStartingValue);
                    if (nCurrentDistance > nFarthestDistance) {
                        pAxisState->m_nFarthestValue = nValue;
                        nFarthestDistance = SDL_abs(pAxisState->m_nFarthestValue - pAxisState->m_nStartingValue);
                    }

#ifdef DEBUG_CONTROLLERMAP
                    SDL_Log("AXIS %d nValue %d nCurrentDistance %d nFarthestDistance %d\n", event.jaxis.axis, nValue, nCurrentDistance, nFarthestDistance);
#endif
                    if (nFarthestDistance >= 16000) {
                        /* If we've gone out far enough and started to come back, let's bind this axis */
                        SDL_bool bCommitBinding = (nCurrentDistance <= 10000) ? SDL_TRUE : SDL_FALSE;
                        SDL_GameControllerExtendedBind binding;
                        SDL_zero(binding);
                        binding.bindType = SDL_CONTROLLER_BINDTYPE_AXIS;
                        binding.value.axis.axis = event.jaxis.axis;
                        binding.value.axis.axis_min = StandardizeAxisValue(pAxisState->m_nStartingValue);
                        binding.value.axis.axis_max = StandardizeAxisValue(pAxisState->m_nFarthestValue);
                        binding.committed = bCommitBinding;
                        ConfigureBinding(&binding);
                    }
                }
                break;
            case SDL_JOYHATMOTION:
                if (event.jhat.which == nJoystickID) {
                    if (event.jhat.value != SDL_HAT_CENTERED) {
                        SDL_GameControllerExtendedBind binding;

#ifdef DEBUG_CONTROLLERMAP
                        SDL_Log("HAT %d %d\n", event.jhat.hat, event.jhat.value);
#endif
                        SDL_zero(binding);
                        binding.bindType = SDL_CONTROLLER_BINDTYPE_HAT;
                        binding.value.hat.hat = event.jhat.hat;
                        binding.value.hat.hat_mask = event.jhat.value;
                        binding.committed = SDL_TRUE;
                        ConfigureBinding(&binding);
                    }
                }
                break;
            case SDL_JOYBALLMOTION:
                break;
            case SDL_JOYBUTTONDOWN:
                if (event.jbutton.which == nJoystickID) {
                    SDL_GameControllerExtendedBind binding;

#ifdef DEBUG_CONTROLLERMAP
                    SDL_Log("BUTTON %d\n", event.jbutton.button);
#endif
                    SDL_zero(binding);
                    binding.bindType = SDL_CONTROLLER_BINDTYPE_BUTTON;
                    binding.value.button = event.jbutton.button;
                    binding.committed = SDL_TRUE;
                    ConfigureBinding(&binding);
                }
                break;
            case SDL_FINGERDOWN:
            case SDL_MOUSEBUTTONDOWN:
                /* Skip this step */
                SetCurrentBinding(s_iCurrentBinding + 1);
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_AC_BACK) {
                    SetCurrentBinding(s_iCurrentBinding - 1);
                    break;
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    SetCurrentBinding(s_iCurrentBinding + 1);
                    break;
                }

                if ((event.key.keysym.sym != SDLK_ESCAPE)) {
                    break;
                }
                /* Fall through to signal quit */
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
            default:
                break;
            }
        }

        SDL_Delay(15);

        /* Wait 100 ms for joystick events to stop coming in,
           in case a controller sends multiple events for a single control (e.g. axis and button for trigger)
        */
        if (s_unPendingAdvanceTime && SDL_GetTicks() - s_unPendingAdvanceTime >= 100) {
            SetCurrentBinding(s_iCurrentBinding + 1);
        }
    }

    if (s_bBindingComplete) {
        char mapping[1024];
        char trimmed_name[128];
        char *spot;
        int iIndex;
        char pszElement[12];

        SDL_strlcpy(trimmed_name, name, SDL_arraysize(trimmed_name));
        while (SDL_isspace(trimmed_name[0])) {
            SDL_memmove(&trimmed_name[0], &trimmed_name[1], SDL_strlen(trimmed_name));
        }
        while (trimmed_name[0] && SDL_isspace(trimmed_name[SDL_strlen(trimmed_name) - 1])) {
            trimmed_name[SDL_strlen(trimmed_name) - 1] = '\0';
        }
        while ((spot = SDL_strchr(trimmed_name, ',')) != NULL) {
            SDL_memmove(spot, spot + 1, SDL_strlen(spot));
        }

        /* Initialize mapping with GUID and name */
        SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), mapping, SDL_arraysize(mapping));
        SDL_strlcat(mapping, ",", SDL_arraysize(mapping));
        SDL_strlcat(mapping, trimmed_name, SDL_arraysize(mapping));
        SDL_strlcat(mapping, ",", SDL_arraysize(mapping));
        SDL_strlcat(mapping, "platform:", SDL_arraysize(mapping));
        SDL_strlcat(mapping, SDL_GetPlatform(), SDL_arraysize(mapping));
        SDL_strlcat(mapping, ",", SDL_arraysize(mapping));

        for (iIndex = 0; iIndex < SDL_arraysize(s_arrBindings); ++iIndex) {
            SDL_GameControllerExtendedBind *pBinding = &s_arrBindings[iIndex];
            if (pBinding->bindType == SDL_CONTROLLER_BINDTYPE_NONE) {
                continue;
            }

            if (iIndex < SDL_CONTROLLER_BUTTON_MAX) {
                SDL_GameControllerButton eButton = (SDL_GameControllerButton)iIndex;
                SDL_strlcat(mapping, SDL_GameControllerGetStringForButton(eButton), SDL_arraysize(mapping));
            } else {
                const char *pszAxisName;
                switch (iIndex - SDL_CONTROLLER_BUTTON_MAX) {
                case SDL_CONTROLLER_BINDING_AXIS_LEFTX_NEGATIVE:
                    if (!BMergeAxisBindings(iIndex)) {
                        SDL_strlcat(mapping, "-", SDL_arraysize(mapping));
                    }
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_LEFTX);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_LEFTX_POSITIVE:
                    SDL_strlcat(mapping, "+", SDL_arraysize(mapping));
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_LEFTX);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_LEFTY_NEGATIVE:
                    if (!BMergeAxisBindings(iIndex)) {
                        SDL_strlcat(mapping, "-", SDL_arraysize(mapping));
                    }
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_LEFTY);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_LEFTY_POSITIVE:
                    SDL_strlcat(mapping, "+", SDL_arraysize(mapping));
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_LEFTY);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_RIGHTX_NEGATIVE:
                    if (!BMergeAxisBindings(iIndex)) {
                        SDL_strlcat(mapping, "-", SDL_arraysize(mapping));
                    }
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_RIGHTX);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_RIGHTX_POSITIVE:
                    SDL_strlcat(mapping, "+", SDL_arraysize(mapping));
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_RIGHTX);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_RIGHTY_NEGATIVE:
                    if (!BMergeAxisBindings(iIndex)) {
                        SDL_strlcat(mapping, "-", SDL_arraysize(mapping));
                    }
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_RIGHTY);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_RIGHTY_POSITIVE:
                    SDL_strlcat(mapping, "+", SDL_arraysize(mapping));
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_RIGHTY);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_TRIGGERLEFT:
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
                    break;
                case SDL_CONTROLLER_BINDING_AXIS_TRIGGERRIGHT:
                    pszAxisName = SDL_GameControllerGetStringForAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
                    break;
                }
                SDL_strlcat(mapping, pszAxisName, SDL_arraysize(mapping));
            }
            SDL_strlcat(mapping, ":", SDL_arraysize(mapping));

            pszElement[0] = '\0';
            switch (pBinding->bindType) {
            case SDL_CONTROLLER_BINDTYPE_BUTTON:
                SDL_snprintf(pszElement, sizeof(pszElement), "b%d", pBinding->value.button);
                break;
            case SDL_CONTROLLER_BINDTYPE_AXIS:
                if (pBinding->value.axis.axis_min == 0 && pBinding->value.axis.axis_max == SDL_JOYSTICK_AXIS_MIN) {
                    /* The negative half axis */
                    SDL_snprintf(pszElement, sizeof(pszElement), "-a%d", pBinding->value.axis.axis);
                } else if (pBinding->value.axis.axis_min == 0 && pBinding->value.axis.axis_max == SDL_JOYSTICK_AXIS_MAX) {
                    /* The positive half axis */
                    SDL_snprintf(pszElement, sizeof(pszElement), "+a%d", pBinding->value.axis.axis);
                } else {
                    SDL_snprintf(pszElement, sizeof(pszElement), "a%d", pBinding->value.axis.axis);
                    if (pBinding->value.axis.axis_min > pBinding->value.axis.axis_max) {
                        /* Invert the axis */
                        SDL_strlcat(pszElement, "~", SDL_arraysize(pszElement));
                    }
                }
                break;
            case SDL_CONTROLLER_BINDTYPE_HAT:
                SDL_snprintf(pszElement, sizeof(pszElement), "h%d.%d", pBinding->value.hat.hat, pBinding->value.hat.hat_mask);
                break;
            default:
                SDL_assert(!"Unknown bind type");
                break;
            }
            SDL_strlcat(mapping, pszElement, SDL_arraysize(mapping));
            SDL_strlcat(mapping, ",", SDL_arraysize(mapping));
        }

        SDL_Log("Mapping:\n\n%s\n\n", mapping);
        /* Print to stdout as well so the user can cat the output somewhere */
        printf("%s\n", mapping);
    }

    SDL_free(s_arrAxisState);
    s_arrAxisState = NULL;
    
    SDL_DestroyRenderer(screen);
}

int
main(int argc, char *argv[])
{
    const char *name;
    int i;
    SDL_Joystick *joystick;

    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize SDL (Note: video is required to start event loop) */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /* Create a window to display joystick axis position */
    window = SDL_CreateWindow("Game Controller Map", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, 0);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s\n", SDL_GetError());
        return 2;
    }

    screen = SDL_CreateRenderer(window, -1, 0);
    if (screen == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s\n", SDL_GetError());
        return 2;
    }

    while (!done && SDL_NumJoysticks() == 0) {
        SDL_Event event;

        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if ((event.key.keysym.sym != SDLK_ESCAPE)) {
                    break;
                }
                /* Fall through to signal quit */
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
            default:
                break;
            }
        }
        SDL_RenderPresent(screen);
    }

    /* Print information about the joysticks */
    SDL_Log("There are %d joysticks attached\n", SDL_NumJoysticks());
    for (i = 0; i < SDL_NumJoysticks(); ++i) {
        name = SDL_JoystickNameForIndex(i);
        SDL_Log("Joystick %d: %s\n", i, name ? name : "Unknown Joystick");
        joystick = SDL_JoystickOpen(i);
        if (joystick == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_JoystickOpen(%d) failed: %s\n", i,
                    SDL_GetError());
        } else {
            char guid[64];
            SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick),
                                      guid, sizeof (guid));
            SDL_Log("       axes: %d\n", SDL_JoystickNumAxes(joystick));
            SDL_Log("      balls: %d\n", SDL_JoystickNumBalls(joystick));
            SDL_Log("       hats: %d\n", SDL_JoystickNumHats(joystick));
            SDL_Log("    buttons: %d\n", SDL_JoystickNumButtons(joystick));
            SDL_Log("instance id: %d\n", SDL_JoystickInstanceID(joystick));
            SDL_Log("       guid: %s\n", guid);
            SDL_Log("    VID/PID: 0x%.4x/0x%.4x\n", SDL_JoystickGetVendor(joystick), SDL_JoystickGetProduct(joystick));
            SDL_JoystickClose(joystick);
        }
    }

    joystick = SDL_JoystickOpen(0);
    if (joystick == NULL) {
        SDL_Log("Couldn't open joystick 0: %s\n", SDL_GetError());
    } else {
        WatchJoystick(joystick);
        SDL_JoystickClose(joystick);
    }

    SDL_DestroyWindow(window);

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    return 0;
}

#else

int
main(int argc, char *argv[])
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL compiled without Joystick support.\n");
    return 1;
}

#endif

/* vi: set ts=4 sw=4 expandtab: */
