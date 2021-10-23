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

#if SDL_VIDEO_DRIVER_X11

#include "SDL_x11video.h"
#include "SDL_x11shape.h"
#include "SDL_x11window.h"
#include "../SDL_shape_internals.h"

SDL_WindowShaper*
X11_CreateShaper(SDL_Window* window) {
    SDL_WindowShaper* result = NULL;
    SDL_ShapeData* data = NULL;
    int resized_properly;

#if SDL_VIDEO_DRIVER_X11_XSHAPE
    if (SDL_X11_HAVE_XSHAPE) {  /* Make sure X server supports it. */
        result = malloc(sizeof(SDL_WindowShaper));
        result->window = window;
        result->mode.mode = ShapeModeDefault;
        result->mode.parameters.binarizationCutoff = 1;
        result->userx = result->usery = 0;
        data = SDL_malloc(sizeof(SDL_ShapeData));
        result->driverdata = data;
        data->bitmapsize = 0;
        data->bitmap = NULL;
        window->shaper = result;
        resized_properly = X11_ResizeWindowShape(window);
        SDL_assert(resized_properly == 0);
    }
#endif

    return result;
}

int
X11_ResizeWindowShape(SDL_Window* window) {
    SDL_ShapeData* data = window->shaper->driverdata;
    unsigned int bitmapsize = window->w / 8;
    SDL_assert(data != NULL);

    if(window->w % 8 > 0)
        bitmapsize += 1;
    bitmapsize *= window->h;
    if(data->bitmapsize != bitmapsize || data->bitmap == NULL) {
        data->bitmapsize = bitmapsize;
        if(data->bitmap != NULL)
            free(data->bitmap);
        data->bitmap = malloc(data->bitmapsize);
        if(data->bitmap == NULL) {
            return SDL_SetError("Could not allocate memory for shaped-window bitmap.");
        }
    }
    memset(data->bitmap,0,data->bitmapsize);

    window->shaper->userx = window->x;
    window->shaper->usery = window->y;
    SDL_SetWindowPosition(window,-1000,-1000);

    return 0;
}

int
X11_SetWindowShape(SDL_WindowShaper *shaper,SDL_Surface *shape,SDL_WindowShapeMode *shape_mode) {
    SDL_ShapeData *data = NULL;
    SDL_WindowData *windowdata = NULL;
    Pixmap shapemask;
    
    if(shaper == NULL || shape == NULL || shaper->driverdata == NULL)
        return -1;

#if SDL_VIDEO_DRIVER_X11_XSHAPE
    if(shape->format->Amask == 0 && SDL_SHAPEMODEALPHA(shape_mode->mode))
        return -2;
    if(shape->w != shaper->window->w || shape->h != shaper->window->h)
        return -3;
    data = shaper->driverdata;

    /* Assume that shaper->alphacutoff already has a value, because SDL_SetWindowShape() should have given it one. */
    SDL_CalculateShapeBitmap(shaper->mode,shape,data->bitmap,8);

    windowdata = (SDL_WindowData*)(shaper->window->driverdata);
    shapemask = X11_XCreateBitmapFromData(windowdata->videodata->display,windowdata->xwindow,data->bitmap,shaper->window->w,shaper->window->h);

    X11_XShapeCombineMask(windowdata->videodata->display,windowdata->xwindow, ShapeBounding, 0, 0,shapemask, ShapeSet);
    X11_XSync(windowdata->videodata->display,False);

    X11_XFreePixmap(windowdata->videodata->display,shapemask);
#endif

    return 0;
}

#endif /* SDL_VIDEO_DRIVER_X11 */
