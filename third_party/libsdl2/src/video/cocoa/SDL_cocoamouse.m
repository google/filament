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

#if SDL_VIDEO_DRIVER_COCOA

#include "SDL_events.h"
#include "SDL_cocoamouse.h"
#include "SDL_cocoavideo.h"

#include "../../events/SDL_mouse_c.h"

/* #define DEBUG_COCOAMOUSE */

#ifdef DEBUG_COCOAMOUSE
#define DLog(fmt, ...) printf("%s: " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define DLog(...) do { } while (0)
#endif

@implementation NSCursor (InvisibleCursor)
+ (NSCursor *)invisibleCursor
{
    static NSCursor *invisibleCursor = NULL;
    if (!invisibleCursor) {
        /* RAW 16x16 transparent GIF */
        static unsigned char cursorBytes[] = {
            0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xF9, 0x04,
            0x01, 0x00, 0x00, 0x01, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x10,
            0x00, 0x10, 0x00, 0x00, 0x02, 0x0E, 0x8C, 0x8F, 0xA9, 0xCB, 0xED,
            0x0F, 0xA3, 0x9C, 0xB4, 0xDA, 0x8B, 0xB3, 0x3E, 0x05, 0x00, 0x3B
        };

        NSData *cursorData = [NSData dataWithBytesNoCopy:&cursorBytes[0]
                                                  length:sizeof(cursorBytes)
                                            freeWhenDone:NO];
        NSImage *cursorImage = [[[NSImage alloc] initWithData:cursorData] autorelease];
        invisibleCursor = [[NSCursor alloc] initWithImage:cursorImage
                                                  hotSpot:NSZeroPoint];
    }

    return invisibleCursor;
}
@end


static SDL_Cursor *
Cocoa_CreateDefaultCursor()
{ @autoreleasepool
{
    NSCursor *nscursor;
    SDL_Cursor *cursor = NULL;

    nscursor = [NSCursor arrowCursor];

    if (nscursor) {
        cursor = SDL_calloc(1, sizeof(*cursor));
        if (cursor) {
            cursor->driverdata = nscursor;
            [nscursor retain];
        }
    }

    return cursor;
}}

static SDL_Cursor *
Cocoa_CreateCursor(SDL_Surface * surface, int hot_x, int hot_y)
{ @autoreleasepool
{
    NSImage *nsimage;
    NSCursor *nscursor = NULL;
    SDL_Cursor *cursor = NULL;

    nsimage = Cocoa_CreateImage(surface);
    if (nsimage) {
        nscursor = [[NSCursor alloc] initWithImage: nsimage hotSpot: NSMakePoint(hot_x, hot_y)];
    }

    if (nscursor) {
        cursor = SDL_calloc(1, sizeof(*cursor));
        if (cursor) {
            cursor->driverdata = nscursor;
        } else {
            [nscursor release];
        }
    }

    return cursor;
}}

static SDL_Cursor *
Cocoa_CreateSystemCursor(SDL_SystemCursor id)
{ @autoreleasepool
{
    NSCursor *nscursor = NULL;
    SDL_Cursor *cursor = NULL;

    switch(id) {
    case SDL_SYSTEM_CURSOR_ARROW:
        nscursor = [NSCursor arrowCursor];
        break;
    case SDL_SYSTEM_CURSOR_IBEAM:
        nscursor = [NSCursor IBeamCursor];
        break;
    case SDL_SYSTEM_CURSOR_WAIT:
        nscursor = [NSCursor arrowCursor];
        break;
    case SDL_SYSTEM_CURSOR_CROSSHAIR:
        nscursor = [NSCursor crosshairCursor];
        break;
    case SDL_SYSTEM_CURSOR_WAITARROW:
        nscursor = [NSCursor arrowCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZENWSE:
    case SDL_SYSTEM_CURSOR_SIZENESW:
        nscursor = [NSCursor closedHandCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZEWE:
        nscursor = [NSCursor resizeLeftRightCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZENS:
        nscursor = [NSCursor resizeUpDownCursor];
        break;
    case SDL_SYSTEM_CURSOR_SIZEALL:
        nscursor = [NSCursor closedHandCursor];
        break;
    case SDL_SYSTEM_CURSOR_NO:
        nscursor = [NSCursor operationNotAllowedCursor];
        break;
    case SDL_SYSTEM_CURSOR_HAND:
        nscursor = [NSCursor pointingHandCursor];
        break;
    default:
        SDL_assert(!"Unknown system cursor");
        return NULL;
    }

    if (nscursor) {
        cursor = SDL_calloc(1, sizeof(*cursor));
        if (cursor) {
            /* We'll free it later, so retain it here */
            [nscursor retain];
            cursor->driverdata = nscursor;
        }
    }

    return cursor;
}}

static void
Cocoa_FreeCursor(SDL_Cursor * cursor)
{ @autoreleasepool
{
    NSCursor *nscursor = (NSCursor *)cursor->driverdata;

    [nscursor release];
    SDL_free(cursor);
}}

static int
Cocoa_ShowCursor(SDL_Cursor * cursor)
{ @autoreleasepool
{
    SDL_VideoDevice *device = SDL_GetVideoDevice();
    SDL_Window *window = (device ? device->windows : NULL);
    for (; window != NULL; window = window->next) {
        SDL_WindowData *driverdata = (SDL_WindowData *)window->driverdata;
        if (driverdata) {
            [driverdata->nswindow performSelectorOnMainThread:@selector(invalidateCursorRectsForView:)
                                                   withObject:[driverdata->nswindow contentView]
                                                waitUntilDone:NO];
        }
    }
    return 0;
}}

static SDL_Window *
SDL_FindWindowAtPoint(const int x, const int y)
{
    const SDL_Point pt = { x, y };
    SDL_Window *i;
    for (i = SDL_GetVideoDevice()->windows; i; i = i->next) {
        const SDL_Rect r = { i->x, i->y, i->w, i->h };
        if (SDL_PointInRect(&pt, &r)) {
            return i;
        }
    }

    return NULL;
}

static int
Cocoa_WarpMouseGlobal(int x, int y)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    if (mouse->focus) {
        SDL_WindowData *data = (SDL_WindowData *) mouse->focus->driverdata;
        if ([data->listener isMovingOrFocusClickPending]) {
            DLog("Postponing warp, window being moved or focused.");
            [data->listener setPendingMoveX:x Y:y];
            return 0;
        }
    }
    const CGPoint point = CGPointMake((float)x, (float)y);

    Cocoa_HandleMouseWarp(point.x, point.y);

    CGWarpMouseCursorPosition(point);

    /* CGWarpMouse causes a short delay by default, which is preventable by
     * Calling this directly after. CGSetLocalEventsSuppressionInterval can also
     * prevent it, but it's deprecated as of OS X 10.6.
     */
    if (!mouse->relative_mode) {
        CGAssociateMouseAndMouseCursorPosition(YES);
    }

    /* CGWarpMouseCursorPosition doesn't generate a window event, unlike our
     * other implementations' APIs. Send what's appropriate.
     */
    if (!mouse->relative_mode) {
        SDL_Window *win = SDL_FindWindowAtPoint(x, y);
        SDL_SetMouseFocus(win);
        if (win) {
            SDL_assert(win == mouse->focus);
            SDL_SendMouseMotion(win, mouse->mouseID, 0, x - win->x, y - win->y);
        }
    }

    return 0;
}

static void
Cocoa_WarpMouse(SDL_Window * window, int x, int y)
{
    Cocoa_WarpMouseGlobal(x + window->x, y + window->y);
}

static int
Cocoa_SetRelativeMouseMode(SDL_bool enabled)
{
    /* We will re-apply the relative mode when the window gets focus, if it
     * doesn't have focus right now.
     */
    SDL_Window *window = SDL_GetMouseFocus();
    if (!window) {
      return 0;
    }

    /* We will re-apply the relative mode when the window finishes being moved,
     * if it is being moved right now.
     */
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    if ([data->listener isMovingOrFocusClickPending]) {
        return 0;
    }

    CGError result;
    if (enabled) {
        DLog("Turning on.");
        result = CGAssociateMouseAndMouseCursorPosition(NO);
    } else {
        DLog("Turning off.");
        result = CGAssociateMouseAndMouseCursorPosition(YES);
    }
    if (result != kCGErrorSuccess) {
        return SDL_SetError("CGAssociateMouseAndMouseCursorPosition() failed");
    }

    /* The hide/unhide calls are redundant most of the time, but they fix
     * https://bugzilla.libsdl.org/show_bug.cgi?id=2550
     */
    if (enabled) {
        [NSCursor hide];
    } else {
        [NSCursor unhide];
    }
    return 0;
}

static int
Cocoa_CaptureMouse(SDL_Window *window)
{
    /* our Cocoa event code already tracks the mouse outside the window,
        so all we have to do here is say "okay" and do what we always do. */
    return 0;
}

static Uint32
Cocoa_GetGlobalMouseState(int *x, int *y)
{
    const NSUInteger cocoaButtons = [NSEvent pressedMouseButtons];
    const NSPoint cocoaLocation = [NSEvent mouseLocation];
    Uint32 retval = 0;

    *x = (int) cocoaLocation.x;
    *y = (int) (CGDisplayPixelsHigh(kCGDirectMainDisplay) - cocoaLocation.y);

    retval |= (cocoaButtons & (1 << 0)) ? SDL_BUTTON_LMASK : 0;
    retval |= (cocoaButtons & (1 << 1)) ? SDL_BUTTON_RMASK : 0;
    retval |= (cocoaButtons & (1 << 2)) ? SDL_BUTTON_MMASK : 0;
    retval |= (cocoaButtons & (1 << 3)) ? SDL_BUTTON_X1MASK : 0;
    retval |= (cocoaButtons & (1 << 4)) ? SDL_BUTTON_X2MASK : 0;

    return retval;
}

int
Cocoa_InitMouse(_THIS)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_MouseData *driverdata = (SDL_MouseData*) SDL_calloc(1, sizeof(SDL_MouseData));
    if (driverdata == NULL) {
        return SDL_OutOfMemory();
    }

    mouse->driverdata = driverdata;
    mouse->CreateCursor = Cocoa_CreateCursor;
    mouse->CreateSystemCursor = Cocoa_CreateSystemCursor;
    mouse->ShowCursor = Cocoa_ShowCursor;
    mouse->FreeCursor = Cocoa_FreeCursor;
    mouse->WarpMouse = Cocoa_WarpMouse;
    mouse->WarpMouseGlobal = Cocoa_WarpMouseGlobal;
    mouse->SetRelativeMouseMode = Cocoa_SetRelativeMouseMode;
    mouse->CaptureMouse = Cocoa_CaptureMouse;
    mouse->GetGlobalMouseState = Cocoa_GetGlobalMouseState;

    SDL_SetDefaultCursor(Cocoa_CreateDefaultCursor());

    const NSPoint location =  [NSEvent mouseLocation];
    driverdata->lastMoveX = location.x;
    driverdata->lastMoveY = location.y;
    return 0;
}

static void
Cocoa_HandleTitleButtonEvent(_THIS, NSEvent *event)
{
    SDL_Window *window;
    NSWindow *nswindow = [event window];

    for (window = _this->windows; window; window = window->next) {
        SDL_WindowData *data = (SDL_WindowData *)window->driverdata;
        if (data && data->nswindow == nswindow) {
            switch ([event type]) {
            case NSEventTypeLeftMouseDown:
            case NSEventTypeRightMouseDown:
            case NSEventTypeOtherMouseDown:
                [data->listener setFocusClickPending:[event buttonNumber]];
                break;
            case NSEventTypeLeftMouseUp:
            case NSEventTypeRightMouseUp:
            case NSEventTypeOtherMouseUp:
                [data->listener clearFocusClickPending:[event buttonNumber]];
                break;
            default:
                break;
            }
            break;
        }
    }
}

void
Cocoa_HandleMouseEvent(_THIS, NSEvent *event)
{
    switch ([event type]) {
        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
            break;

        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseUp:
            if ([event window]) {
                NSRect windowRect = [[[event window] contentView] frame];
                if (!NSMouseInRect([event locationInWindow], windowRect, NO)) {
                    Cocoa_HandleTitleButtonEvent(_this, event);
                    return;
                }
            }
            return;

        default:
            /* Ignore any other events. */
            return;
    }

    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_MouseData *driverdata = (SDL_MouseData*)mouse->driverdata;
    if (!driverdata) {
        return;  /* can happen when returning from fullscreen Space on shutdown */
    }

    SDL_MouseID mouseID = mouse ? mouse->mouseID : 0;
    const SDL_bool seenWarp = driverdata->seenWarp;
    driverdata->seenWarp = NO;

    const NSPoint location =  [NSEvent mouseLocation];
    const CGFloat lastMoveX = driverdata->lastMoveX;
    const CGFloat lastMoveY = driverdata->lastMoveY;
    driverdata->lastMoveX = location.x;
    driverdata->lastMoveY = location.y;
    DLog("Last seen mouse: (%g, %g)", location.x, location.y);

    /* Non-relative movement is handled in -[Cocoa_WindowListener mouseMoved:] */
    if (!mouse->relative_mode) {
        return;
    }

    /* Ignore events that aren't inside the client area (i.e. title bar.) */
    if ([event window]) {
        NSRect windowRect = [[[event window] contentView] frame];
        if (!NSMouseInRect([event locationInWindow], windowRect, NO)) {
            return;
        }
    }

    float deltaX = [event deltaX];
    float deltaY = [event deltaY];

    if (seenWarp) {
        deltaX += (lastMoveX - driverdata->lastWarpX);
        deltaY += ((CGDisplayPixelsHigh(kCGDirectMainDisplay) - lastMoveY) - driverdata->lastWarpY);

        DLog("Motion was (%g, %g), offset to (%g, %g)", [event deltaX], [event deltaY], deltaX, deltaY);
    }

    SDL_SendMouseMotion(mouse->focus, mouseID, 1, (int)deltaX, (int)deltaY);
}

void
Cocoa_HandleMouseWheel(SDL_Window *window, NSEvent *event)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    if (!mouse) {
        return;
    }

    SDL_MouseID mouseID = mouse->mouseID;
    CGFloat x = -[event deltaX];
    CGFloat y = [event deltaY];
    SDL_MouseWheelDirection direction = SDL_MOUSEWHEEL_NORMAL;

    if ([event respondsToSelector:@selector(isDirectionInvertedFromDevice)]) {
        if ([event isDirectionInvertedFromDevice] == YES) {
            direction = SDL_MOUSEWHEEL_FLIPPED;
        }
    }

    if (x > 0) {
        x = SDL_ceil(x);
    } else if (x < 0) {
        x = SDL_floor(x);
    }
    if (y > 0) {
        y = SDL_ceil(y);
    } else if (y < 0) {
        y = SDL_floor(y);
    }

    SDL_SendMouseWheel(window, mouseID, x, y, direction);
}

void
Cocoa_HandleMouseWarp(CGFloat x, CGFloat y)
{
    /* This makes Cocoa_HandleMouseEvent ignore the delta caused by the warp,
     * since it gets included in the next movement event.
     */
    SDL_MouseData *driverdata = (SDL_MouseData*)SDL_GetMouse()->driverdata;
    driverdata->lastWarpX = x;
    driverdata->lastWarpY = y;
    driverdata->seenWarp = SDL_TRUE;

    DLog("(%g, %g)", x, y);
}

void
Cocoa_QuitMouse(_THIS)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    if (mouse) {
        if (mouse->driverdata) {
            SDL_free(mouse->driverdata);
            mouse->driverdata = NULL;
        }
    }
}

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vi: set ts=4 sw=4 expandtab: */
