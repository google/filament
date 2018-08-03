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

#if SDL_VIDEO_DRIVER_COCOA

#include "SDL_cocoavideo.h"
#include "SDL_shape.h"
#include "SDL_cocoashape.h"
#include "../SDL_sysvideo.h"
#include "SDL_assert.h"

SDL_WindowShaper*
Cocoa_CreateShaper(SDL_Window* window)
{
    SDL_WindowData* windata = (SDL_WindowData*)window->driverdata;
    [windata->nswindow setOpaque:NO];

    [windata->nswindow setStyleMask:NSWindowStyleMaskBorderless];

    SDL_WindowShaper* result = result = malloc(sizeof(SDL_WindowShaper));
    result->window = window;
    result->mode.mode = ShapeModeDefault;
    result->mode.parameters.binarizationCutoff = 1;
    result->userx = result->usery = 0;
    window->shaper = result;

    SDL_ShapeData* data = malloc(sizeof(SDL_ShapeData));
    result->driverdata = data;
    data->context = [windata->nswindow graphicsContext];
    data->saved = SDL_FALSE;
    data->shape = NULL;

    int resized_properly = Cocoa_ResizeWindowShape(window);
    SDL_assert(resized_properly == 0);
    return result;
}

typedef struct {
    NSView* view;
    NSBezierPath* path;
    SDL_Window* window;
} SDL_CocoaClosure;

void
ConvertRects(SDL_ShapeTree* tree, void* closure)
{
    SDL_CocoaClosure* data = (SDL_CocoaClosure*)closure;
    if(tree->kind == OpaqueShape) {
        NSRect rect = NSMakeRect(tree->data.shape.x,data->window->h - tree->data.shape.y,tree->data.shape.w,tree->data.shape.h);
        [data->path appendBezierPathWithRect:[data->view convertRect:rect toView:nil]];
    }
}

int
Cocoa_SetWindowShape(SDL_WindowShaper *shaper, SDL_Surface *shape, SDL_WindowShapeMode *shape_mode)
{ @autoreleasepool
{
    SDL_ShapeData* data = (SDL_ShapeData*)shaper->driverdata;
    SDL_WindowData* windata = (SDL_WindowData*)shaper->window->driverdata;
    SDL_CocoaClosure closure;
    if(data->saved == SDL_TRUE) {
        [data->context restoreGraphicsState];
        data->saved = SDL_FALSE;
    }

    /*[data->context saveGraphicsState];*/
    /*data->saved = SDL_TRUE;*/
    [NSGraphicsContext setCurrentContext:data->context];

    [[NSColor clearColor] set];
    NSRectFill([[windata->nswindow contentView] frame]);
    data->shape = SDL_CalculateShapeTree(*shape_mode,shape);

    closure.view = [windata->nswindow contentView];
    closure.path = [NSBezierPath bezierPath];
    closure.window = shaper->window;
    SDL_TraverseShapeTree(data->shape,&ConvertRects,&closure);
    [closure.path addClip];

    return 0;
}}

int
Cocoa_ResizeWindowShape(SDL_Window *window)
{
    SDL_ShapeData* data = window->shaper->driverdata;
    SDL_assert(data != NULL);
    return 0;
}

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vi: set ts=4 sw=4 expandtab: */
