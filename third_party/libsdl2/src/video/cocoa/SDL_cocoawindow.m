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

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
# error SDL for Mac OS X must be built with a 10.7 SDK or above.
#endif /* MAC_OS_X_VERSION_MAX_ALLOWED < 1070 */

#include "SDL_syswm.h"
#include "SDL_timer.h"  /* For SDL_GetTicks() */
#include "SDL_hints.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_touch_c.h"
#include "../../events/SDL_windowevents_c.h"
#include "../../events/SDL_dropevents_c.h"
#include "SDL_cocoavideo.h"
#include "SDL_cocoashape.h"
#include "SDL_cocoamouse.h"
#include "SDL_cocoamousetap.h"
#include "SDL_cocoaopengl.h"
#include "SDL_cocoaopengles.h"
#include "SDL_assert.h"

/* #define DEBUG_COCOAWINDOW */

#ifdef DEBUG_COCOAWINDOW
#define DLog(fmt, ...) printf("%s: " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define DLog(...) do { } while (0)
#endif


#define FULLSCREEN_MASK (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)


@interface SDLWindow : NSWindow <NSDraggingDestination>
/* These are needed for borderless/fullscreen windows */
- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
- (void)sendEvent:(NSEvent *)event;
- (void)doCommandBySelector:(SEL)aSelector;

/* Handle drag-and-drop of files onto the SDL window. */
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (BOOL)wantsPeriodicDraggingUpdates;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;

- (SDL_Window*)findSDLWindow;
@end

@implementation SDLWindow

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    /* Only allow using the macOS native fullscreen toggle menubar item if the
     * window is resizable and not in a SDL fullscreen mode.
     */
    if ([menuItem action] == @selector(toggleFullScreen:)) {
        SDL_Window *window = [self findSDLWindow];
        if (window == NULL) {
            return NO;
        } else if ((window->flags & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0) {
            return NO;
        } else if ((window->flags & SDL_WINDOW_RESIZABLE) == 0) {
            return NO;
        }
    }
    return [super validateMenuItem:menuItem];
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

- (void)sendEvent:(NSEvent *)event
{
    [super sendEvent:event];

    if ([event type] != NSEventTypeLeftMouseUp) {
        return;
    }

    id delegate = [self delegate];
    if (![delegate isKindOfClass:[Cocoa_WindowListener class]]) {
        return;
    }

    if ([delegate isMoving]) {
        [delegate windowDidFinishMoving];
    }
}

/* We'll respond to selectors by doing nothing so we don't beep.
 * The escape key gets converted to a "cancel" selector, etc.
 */
- (void)doCommandBySelector:(SEL)aSelector
{
    /*NSLog(@"doCommandBySelector: %@\n", NSStringFromSelector(aSelector));*/
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    if (([sender draggingSourceOperationMask] & NSDragOperationGeneric) == NSDragOperationGeneric) {
        return NSDragOperationGeneric;
    }

    return NSDragOperationNone; /* no idea what to do with this, reject it. */
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{ @autoreleasepool
{
    NSPasteboard *pasteboard = [sender draggingPasteboard];
    NSArray *types = [NSArray arrayWithObject:NSFilenamesPboardType];
    NSString *desiredType = [pasteboard availableTypeFromArray:types];
    SDL_Window *sdlwindow = [self findSDLWindow];

    if (desiredType == nil) {
        return NO;  /* can't accept anything that's being dropped here. */
    }

    NSData *data = [pasteboard dataForType:desiredType];
    if (data == nil) {
        return NO;
    }

    SDL_assert([desiredType isEqualToString:NSFilenamesPboardType]);
    NSArray *array = [pasteboard propertyListForType:@"NSFilenamesPboardType"];

    for (NSString *path in array) {
        NSURL *fileURL = [NSURL fileURLWithPath:path];
        NSNumber *isAlias = nil;

        [fileURL getResourceValue:&isAlias forKey:NSURLIsAliasFileKey error:nil];

        /* If the URL is an alias, resolve it. */
        if ([isAlias boolValue]) {
            NSURLBookmarkResolutionOptions opts = NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithoutUI;
            NSData *bookmark = [NSURL bookmarkDataWithContentsOfURL:fileURL error:nil];
            if (bookmark != nil) {
                NSURL *resolvedURL = [NSURL URLByResolvingBookmarkData:bookmark
                                                               options:opts
                                                         relativeToURL:nil
                                                   bookmarkDataIsStale:nil
                                                                 error:nil];

                if (resolvedURL != nil) {
                    fileURL = resolvedURL;
                }
            }
        }

        if (!SDL_SendDropFile(sdlwindow, [[fileURL path] UTF8String])) {
            return NO;
        }
    }

    SDL_SendDropComplete(sdlwindow);
    return YES;
}}

- (BOOL)wantsPeriodicDraggingUpdates
{
    return NO;
}

- (SDL_Window*)findSDLWindow
{
    SDL_Window *sdlwindow = NULL;
    SDL_VideoDevice *_this = SDL_GetVideoDevice();

    /* !!! FIXME: is there a better way to do this? */
    if (_this) {
        for (sdlwindow = _this->windows; sdlwindow; sdlwindow = sdlwindow->next) {
            NSWindow *nswindow = ((SDL_WindowData *) sdlwindow->driverdata)->nswindow;
            if (nswindow == self) {
                break;
            }
        }
    }

    return sdlwindow;
}

@end


static Uint32 s_moveHack;

static void ConvertNSRect(NSScreen *screen, BOOL fullscreen, NSRect *r)
{
    r->origin.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - r->origin.y - r->size.height;
}

static void
ScheduleContextUpdates(SDL_WindowData *data)
{
    NSOpenGLContext *currentContext = [NSOpenGLContext currentContext];
    NSMutableArray *contexts = data->nscontexts;
    @synchronized (contexts) {
        for (SDLOpenGLContext *context in contexts) {
            if (context == currentContext) {
                [context update];
            } else {
                [context scheduleUpdate];
            }
        }
    }
}

/* !!! FIXME: this should use a hint callback. */
static int
GetHintCtrlClickEmulateRightClick()
{
	return SDL_GetHintBoolean(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, SDL_FALSE);
}

static NSUInteger
GetWindowStyle(SDL_Window * window)
{
    NSUInteger style = 0;

    if (window->flags & SDL_WINDOW_FULLSCREEN) {
        style = NSWindowStyleMaskBorderless;
    } else {
        if (window->flags & SDL_WINDOW_BORDERLESS) {
            style = NSWindowStyleMaskBorderless;
        } else {
            style = (NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable);
        }
        if (window->flags & SDL_WINDOW_RESIZABLE) {
            style |= NSWindowStyleMaskResizable;
        }
    }
    return style;
}

static SDL_bool
SetWindowStyle(SDL_Window * window, NSUInteger style)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    NSWindow *nswindow = data->nswindow;

    /* The view responder chain gets messed with during setStyleMask */
    if ([[nswindow contentView] nextResponder] == data->listener) {
        [[nswindow contentView] setNextResponder:nil];
    }

    [nswindow setStyleMask:style];

    /* The view responder chain gets messed with during setStyleMask */
    if ([[nswindow contentView] nextResponder] != data->listener) {
        [[nswindow contentView] setNextResponder:data->listener];
    }

    return SDL_TRUE;
}


@implementation Cocoa_WindowListener

- (void)listen:(SDL_WindowData *)data
{
    NSNotificationCenter *center;
    NSWindow *window = data->nswindow;
    NSView *view = [window contentView];

    _data = data;
    observingVisible = YES;
    wasCtrlLeft = NO;
    wasVisible = [window isVisible];
    isFullscreenSpace = NO;
    inFullscreenTransition = NO;
    pendingWindowOperation = PENDING_OPERATION_NONE;
    isMoving = NO;
    isDragAreaRunning = NO;

    center = [NSNotificationCenter defaultCenter];

    if ([window delegate] != nil) {
        [center addObserver:self selector:@selector(windowDidExpose:) name:NSWindowDidExposeNotification object:window];
        [center addObserver:self selector:@selector(windowDidMove:) name:NSWindowDidMoveNotification object:window];
        [center addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:window];
        [center addObserver:self selector:@selector(windowDidMiniaturize:) name:NSWindowDidMiniaturizeNotification object:window];
        [center addObserver:self selector:@selector(windowDidDeminiaturize:) name:NSWindowDidDeminiaturizeNotification object:window];
        [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:window];
        [center addObserver:self selector:@selector(windowDidResignKey:) name:NSWindowDidResignKeyNotification object:window];
        [center addObserver:self selector:@selector(windowDidChangeBackingProperties:) name:NSWindowDidChangeBackingPropertiesNotification object:window];
        [center addObserver:self selector:@selector(windowWillEnterFullScreen:) name:NSWindowWillEnterFullScreenNotification object:window];
        [center addObserver:self selector:@selector(windowDidEnterFullScreen:) name:NSWindowDidEnterFullScreenNotification object:window];
        [center addObserver:self selector:@selector(windowWillExitFullScreen:) name:NSWindowWillExitFullScreenNotification object:window];
        [center addObserver:self selector:@selector(windowDidExitFullScreen:) name:NSWindowDidExitFullScreenNotification object:window];
        [center addObserver:self selector:@selector(windowDidFailToEnterFullScreen:) name:@"NSWindowDidFailToEnterFullScreenNotification" object:window];
        [center addObserver:self selector:@selector(windowDidFailToExitFullScreen:) name:@"NSWindowDidFailToExitFullScreenNotification" object:window];
    } else {
        [window setDelegate:self];
    }

    /* Haven't found a delegate / notification that triggers when the window is
     * ordered out (is not visible any more). You can be ordered out without
     * minimizing, so DidMiniaturize doesn't work. (e.g. -[NSWindow orderOut:])
     */
    [window addObserver:self
             forKeyPath:@"visible"
                options:NSKeyValueObservingOptionNew
                context:NULL];

    [window setNextResponder:self];
    [window setAcceptsMouseMovedEvents:YES];

    [view setNextResponder:self];

    [view setAcceptsTouchEvents:YES];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if (!observingVisible) {
        return;
    }

    if (object == _data->nswindow && [keyPath isEqualToString:@"visible"]) {
        int newVisibility = [[change objectForKey:@"new"] intValue];
        if (newVisibility) {
            SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_SHOWN, 0, 0);
        } else {
            SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_HIDDEN, 0, 0);
        }
    }
}

-(void) pauseVisibleObservation
{
    observingVisible = NO;
    wasVisible = [_data->nswindow isVisible];
}

-(void) resumeVisibleObservation
{
    BOOL isVisible = [_data->nswindow isVisible];
    observingVisible = YES;
    if (wasVisible != isVisible) {
        if (isVisible) {
            SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_SHOWN, 0, 0);
        } else {
            SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_HIDDEN, 0, 0);
        }

        wasVisible = isVisible;
    }
}

-(BOOL) setFullscreenSpace:(BOOL) state
{
    SDL_Window *window = _data->window;
    NSWindow *nswindow = _data->nswindow;
    SDL_VideoData *videodata = ((SDL_WindowData *) window->driverdata)->videodata;

    if (!videodata->allow_spaces) {
        return NO;  /* Spaces are forcibly disabled. */
    } else if (state && ((window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        return NO;  /* we only allow you to make a Space on FULLSCREEN_DESKTOP windows. */
    } else if (!state && ((window->last_fullscreen_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        return NO;  /* we only handle leaving the Space on windows that were previously FULLSCREEN_DESKTOP. */
    } else if (state == isFullscreenSpace) {
        return YES;  /* already there. */
    }

    if (inFullscreenTransition) {
        if (state) {
            [self addPendingWindowOperation:PENDING_OPERATION_ENTER_FULLSCREEN];
        } else {
            [self addPendingWindowOperation:PENDING_OPERATION_LEAVE_FULLSCREEN];
        }
        return YES;
    }
    inFullscreenTransition = YES;

    /* you need to be FullScreenPrimary, or toggleFullScreen doesn't work. Unset it again in windowDidExitFullScreen. */
    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [nswindow performSelectorOnMainThread: @selector(toggleFullScreen:) withObject:nswindow waitUntilDone:NO];
    return YES;
}

-(BOOL) isInFullscreenSpace
{
    return isFullscreenSpace;
}

-(BOOL) isInFullscreenSpaceTransition
{
    return inFullscreenTransition;
}

-(void) addPendingWindowOperation:(PendingWindowOperation) operation
{
    pendingWindowOperation = operation;
}

- (void)close
{
    NSNotificationCenter *center;
    NSWindow *window = _data->nswindow;
    NSView *view = [window contentView];

    center = [NSNotificationCenter defaultCenter];

    if ([window delegate] != self) {
        [center removeObserver:self name:NSWindowDidExposeNotification object:window];
        [center removeObserver:self name:NSWindowDidMoveNotification object:window];
        [center removeObserver:self name:NSWindowDidResizeNotification object:window];
        [center removeObserver:self name:NSWindowDidMiniaturizeNotification object:window];
        [center removeObserver:self name:NSWindowDidDeminiaturizeNotification object:window];
        [center removeObserver:self name:NSWindowDidBecomeKeyNotification object:window];
        [center removeObserver:self name:NSWindowDidResignKeyNotification object:window];
        [center removeObserver:self name:NSWindowDidChangeBackingPropertiesNotification object:window];
        [center removeObserver:self name:NSWindowWillEnterFullScreenNotification object:window];
        [center removeObserver:self name:NSWindowDidEnterFullScreenNotification object:window];
        [center removeObserver:self name:NSWindowWillExitFullScreenNotification object:window];
        [center removeObserver:self name:NSWindowDidExitFullScreenNotification object:window];
        [center removeObserver:self name:@"NSWindowDidFailToEnterFullScreenNotification" object:window];
        [center removeObserver:self name:@"NSWindowDidFailToExitFullScreenNotification" object:window];
    } else {
        [window setDelegate:nil];
    }

    [window removeObserver:self forKeyPath:@"visible"];

    if ([window nextResponder] == self) {
        [window setNextResponder:nil];
    }
    if ([view nextResponder] == self) {
        [view setNextResponder:nil];
    }
}

- (BOOL)isMoving
{
    return isMoving;
}

-(void) setPendingMoveX:(int)x Y:(int)y
{
    pendingWindowWarpX = x;
    pendingWindowWarpY = y;
}

- (void)windowDidFinishMoving
{
    if ([self isMoving]) {
        isMoving = NO;

        SDL_Mouse *mouse = SDL_GetMouse();
        if (pendingWindowWarpX != INT_MAX && pendingWindowWarpY != INT_MAX) {
            mouse->WarpMouseGlobal(pendingWindowWarpX, pendingWindowWarpY);
            pendingWindowWarpX = pendingWindowWarpY = INT_MAX;
        }
        if (mouse->relative_mode && !mouse->relative_mode_warp && mouse->focus == _data->window) {
            mouse->SetRelativeMouseMode(SDL_TRUE);
        }
    }
}

- (BOOL)windowShouldClose:(id)sender
{
    SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_CLOSE, 0, 0);
    return NO;
}

- (void)windowDidExpose:(NSNotification *)aNotification
{
    SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_EXPOSED, 0, 0);
}

- (void)windowWillMove:(NSNotification *)aNotification
{
    if ([_data->nswindow isKindOfClass:[SDLWindow class]]) {
        pendingWindowWarpX = pendingWindowWarpY = INT_MAX;
        isMoving = YES;
    }
}

- (void)windowDidMove:(NSNotification *)aNotification
{
    int x, y;
    SDL_Window *window = _data->window;
    NSWindow *nswindow = _data->nswindow;
    BOOL fullscreen = window->flags & FULLSCREEN_MASK;
    NSRect rect = [nswindow contentRectForFrameRect:[nswindow frame]];
    ConvertNSRect([nswindow screen], fullscreen, &rect);

    if (inFullscreenTransition) {
        /* We'll take care of this at the end of the transition */
        return;
    }

    if (s_moveHack) {
        SDL_bool blockMove = ((SDL_GetTicks() - s_moveHack) < 500);

        s_moveHack = 0;

        if (blockMove) {
            /* Cocoa is adjusting the window in response to a mode change */
            rect.origin.x = window->x;
            rect.origin.y = window->y;
            ConvertNSRect([nswindow screen], fullscreen, &rect);
            [nswindow setFrameOrigin:rect.origin];
            return;
        }
    }

    x = (int)rect.origin.x;
    y = (int)rect.origin.y;

    ScheduleContextUpdates(_data);

    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_MOVED, x, y);
}

- (void)windowDidResize:(NSNotification *)aNotification
{
    if (inFullscreenTransition) {
        /* We'll take care of this at the end of the transition */
        return;
    }

    SDL_Window *window = _data->window;
    NSWindow *nswindow = _data->nswindow;
    int x, y, w, h;
    NSRect rect = [nswindow contentRectForFrameRect:[nswindow frame]];
    ConvertNSRect([nswindow screen], (window->flags & FULLSCREEN_MASK), &rect);
    x = (int)rect.origin.x;
    y = (int)rect.origin.y;
    w = (int)rect.size.width;
    h = (int)rect.size.height;

    if (SDL_IsShapedWindow(window)) {
        Cocoa_ResizeWindowShape(window);
    }

    ScheduleContextUpdates(_data);

    /* The window can move during a resize event, such as when maximizing
       or resizing from a corner */
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_MOVED, x, y);
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, w, h);

    const BOOL zoomed = [nswindow isZoomed];
    if (!zoomed) {
        SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESTORED, 0, 0);
    } else if (zoomed) {
        SDL_SendWindowEvent(window, SDL_WINDOWEVENT_MAXIMIZED, 0, 0);
    }
}

- (void)windowDidMiniaturize:(NSNotification *)aNotification
{
    SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
}

- (void)windowDidDeminiaturize:(NSNotification *)aNotification
{
    SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_RESTORED, 0, 0);
}

- (void)windowDidBecomeKey:(NSNotification *)aNotification
{
    SDL_Window *window = _data->window;
    SDL_Mouse *mouse = SDL_GetMouse();

    /* We're going to get keyboard events, since we're key. */
    /* This needs to be done before restoring the relative mouse mode. */
    SDL_SetKeyboardFocus(window);

    if (mouse->relative_mode && !mouse->relative_mode_warp && ![self isMoving]) {
        mouse->SetRelativeMouseMode(SDL_TRUE);
    }

    /* If we just gained focus we need the updated mouse position */
    if (!mouse->relative_mode) {
        NSPoint point;
        int x, y;

        point = [_data->nswindow mouseLocationOutsideOfEventStream];
        x = (int)point.x;
        y = (int)(window->h - point.y);

        if (x >= 0 && x < window->w && y >= 0 && y < window->h) {
            SDL_SendMouseMotion(window, 0, 0, x, y);
        }
    }

    /* Check to see if someone updated the clipboard */
    Cocoa_CheckClipboardUpdate(_data->videodata);

    if ((isFullscreenSpace) && ((window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        [NSMenu setMenuBarVisible:NO];
    }

    const unsigned int newflags = [NSEvent modifierFlags] & NSEventModifierFlagCapsLock;
    _data->videodata->modifierFlags = (_data->videodata->modifierFlags & ~NSEventModifierFlagCapsLock) | newflags;
    SDL_ToggleModState(KMOD_CAPS, newflags != 0);
}

- (void)windowDidResignKey:(NSNotification *)aNotification
{
    SDL_Mouse *mouse = SDL_GetMouse();
    if (mouse->relative_mode && !mouse->relative_mode_warp) {
        mouse->SetRelativeMouseMode(SDL_FALSE);
    }

    /* Some other window will get mouse events, since we're not key. */
    if (SDL_GetMouseFocus() == _data->window) {
        SDL_SetMouseFocus(NULL);
    }

    /* Some other window will get keyboard events, since we're not key. */
    if (SDL_GetKeyboardFocus() == _data->window) {
        SDL_SetKeyboardFocus(NULL);
    }

    if (isFullscreenSpace) {
        [NSMenu setMenuBarVisible:YES];
    }
}

- (void)windowDidChangeBackingProperties:(NSNotification *)aNotification
{
    NSNumber *oldscale = [[aNotification userInfo] objectForKey:NSBackingPropertyOldScaleFactorKey];

    if (inFullscreenTransition) {
        return;
    }

    if ([oldscale doubleValue] != [_data->nswindow backingScaleFactor]) {
        /* Force a resize event when the backing scale factor changes. */
        _data->window->w = 0;
        _data->window->h = 0;
        [self windowDidResize:aNotification];
    }
}

- (void)windowWillEnterFullScreen:(NSNotification *)aNotification
{
    SDL_Window *window = _data->window;

    SetWindowStyle(window, (NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable));

    isFullscreenSpace = YES;
    inFullscreenTransition = YES;
}

- (void)windowDidFailToEnterFullScreen:(NSNotification *)aNotification
{
    SDL_Window *window = _data->window;

    if (window->is_destroying) {
        return;
    }

    SetWindowStyle(window, GetWindowStyle(window));

    isFullscreenSpace = NO;
    inFullscreenTransition = NO;
    
    [self windowDidExitFullScreen:nil];
}

- (void)windowDidEnterFullScreen:(NSNotification *)aNotification
{
    SDL_Window *window = _data->window;
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    NSWindow *nswindow = data->nswindow;

    inFullscreenTransition = NO;

    if (pendingWindowOperation == PENDING_OPERATION_LEAVE_FULLSCREEN) {
        pendingWindowOperation = PENDING_OPERATION_NONE;
        [self setFullscreenSpace:NO];
    } else {
        /* Unset the resizable flag. 
           This is a workaround for https://bugzilla.libsdl.org/show_bug.cgi?id=3697
         */
        SetWindowStyle(window, [nswindow styleMask] & (~NSWindowStyleMaskResizable));

        if ((window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP) {
            [NSMenu setMenuBarVisible:NO];
        }

        pendingWindowOperation = PENDING_OPERATION_NONE;
        /* Force the size change event in case it was delivered earlier
           while the window was still animating into place.
         */
        window->w = 0;
        window->h = 0;
        [self windowDidMove:aNotification];
        [self windowDidResize:aNotification];
    }
}

- (void)windowWillExitFullScreen:(NSNotification *)aNotification
{
    SDL_Window *window = _data->window;

    isFullscreenSpace = NO;
    inFullscreenTransition = YES;

    /* As of OS X 10.11, the window seems to need to be resizable when exiting
       a Space, in order for it to resize back to its windowed-mode size.
     */
    SetWindowStyle(window, GetWindowStyle(window) | NSWindowStyleMaskResizable);
}

- (void)windowDidFailToExitFullScreen:(NSNotification *)aNotification
{
    SDL_Window *window = _data->window;
    
    if (window->is_destroying) {
        return;
    }

    SetWindowStyle(window, (NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable));
    
    isFullscreenSpace = YES;
    inFullscreenTransition = NO;
    
    [self windowDidEnterFullScreen:nil];
}

- (void)windowDidExitFullScreen:(NSNotification *)aNotification
{
    SDL_Window *window = _data->window;
    NSWindow *nswindow = _data->nswindow;

    inFullscreenTransition = NO;

    SetWindowStyle(window, GetWindowStyle(window));

    [nswindow setLevel:kCGNormalWindowLevel];

    if (pendingWindowOperation == PENDING_OPERATION_ENTER_FULLSCREEN) {
        pendingWindowOperation = PENDING_OPERATION_NONE;
        [self setFullscreenSpace:YES];
    } else if (pendingWindowOperation == PENDING_OPERATION_MINIMIZE) {
        pendingWindowOperation = PENDING_OPERATION_NONE;
        [nswindow miniaturize:nil];
    } else {
        /* Adjust the fullscreen toggle button and readd menu now that we're here. */
        if (window->flags & SDL_WINDOW_RESIZABLE) {
            /* resizable windows are Spaces-friendly: they get the "go fullscreen" toggle button on their titlebar. */
            [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        } else {
            [nswindow setCollectionBehavior:NSWindowCollectionBehaviorManaged];
        }
        [NSMenu setMenuBarVisible:YES];

        pendingWindowOperation = PENDING_OPERATION_NONE;

#if 0
/* This fixed bug 3719, which is that changing window size while fullscreen
   doesn't take effect when leaving fullscreen, but introduces bug 3809,
   which is that a maximized window doesn't go back to normal size when
   restored, so this code is disabled until we can properly handle the
   beginning and end of maximize and restore.
 */
        /* Restore windowed size and position in case it changed while fullscreen */
        {
            NSRect rect;
            rect.origin.x = window->windowed.x;
            rect.origin.y = window->windowed.y;
            rect.size.width = window->windowed.w;
            rect.size.height = window->windowed.h;
            ConvertNSRect([nswindow screen], NO, &rect);

            s_moveHack = 0;
            [nswindow setContentSize:rect.size];
            [nswindow setFrameOrigin:rect.origin];
            s_moveHack = SDL_GetTicks();
        }
#endif /* 0 */

        /* Force the size change event in case it was delivered earlier
           while the window was still animating into place.
         */
        window->w = 0;
        window->h = 0;
        [self windowDidMove:aNotification];
        [self windowDidResize:aNotification];

        /* FIXME: Why does the window get hidden? */
        if (window->flags & SDL_WINDOW_SHOWN) {
            Cocoa_ShowWindow(SDL_GetVideoDevice(), window);
        }
    }
}

-(NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions
{
    if ((_data->window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP) {
        return NSApplicationPresentationFullScreen | NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar;
    } else {
        return proposedOptions;
    }
}


/* We'll respond to key events by doing nothing so we don't beep.
 * We could handle key messages here, but we lose some in the NSApp dispatch,
 * where they get converted to action messages, etc.
 */
- (void)flagsChanged:(NSEvent *)theEvent
{
    /*Cocoa_HandleKeyEvent(SDL_GetVideoDevice(), theEvent);*/
}
- (void)keyDown:(NSEvent *)theEvent
{
    /*Cocoa_HandleKeyEvent(SDL_GetVideoDevice(), theEvent);*/
}
- (void)keyUp:(NSEvent *)theEvent
{
    /*Cocoa_HandleKeyEvent(SDL_GetVideoDevice(), theEvent);*/
}

/* We'll respond to selectors by doing nothing so we don't beep.
 * The escape key gets converted to a "cancel" selector, etc.
 */
- (void)doCommandBySelector:(SEL)aSelector
{
    /*NSLog(@"doCommandBySelector: %@\n", NSStringFromSelector(aSelector));*/
}

- (BOOL)processHitTest:(NSEvent *)theEvent
{
    SDL_assert(isDragAreaRunning == [_data->nswindow isMovableByWindowBackground]);

    if (_data->window->hit_test) {  /* if no hit-test, skip this. */
        const NSPoint location = [theEvent locationInWindow];
        const SDL_Point point = { (int) location.x, _data->window->h - (((int) location.y)-1) };
        const SDL_HitTestResult rc = _data->window->hit_test(_data->window, &point, _data->window->hit_test_data);
        if (rc == SDL_HITTEST_DRAGGABLE) {
            if (!isDragAreaRunning) {
                isDragAreaRunning = YES;
                [_data->nswindow setMovableByWindowBackground:YES];
            }
            return YES;  /* dragging! */
        }
    }

    if (isDragAreaRunning) {
        isDragAreaRunning = NO;
        [_data->nswindow setMovableByWindowBackground:NO];
        return YES;  /* was dragging, drop event. */
    }

    return NO;  /* not a special area, carry on. */
}

- (void)mouseDown:(NSEvent *)theEvent
{
    int button;
    int clicks;

    /* Ignore events that aren't inside the client area (i.e. title bar.) */
    if ([theEvent window]) {
        NSRect windowRect = [[[theEvent window] contentView] frame];
        if (!NSMouseInRect([theEvent locationInWindow], windowRect, NO)) {
            return;
        }
    }

    if ([self processHitTest:theEvent]) {
        SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_HIT_TEST, 0, 0);
        return;  /* dragging, drop event. */
    }

    switch ([theEvent buttonNumber]) {
    case 0:
        if (([theEvent modifierFlags] & NSEventModifierFlagControl) &&
		    GetHintCtrlClickEmulateRightClick()) {
            wasCtrlLeft = YES;
            button = SDL_BUTTON_RIGHT;
        } else {
            wasCtrlLeft = NO;
            button = SDL_BUTTON_LEFT;
        }
        break;
    case 1:
        button = SDL_BUTTON_RIGHT;
        break;
    case 2:
        button = SDL_BUTTON_MIDDLE;
        break;
    default:
        button = (int) [theEvent buttonNumber] + 1;
        break;
    }

    clicks = (int) [theEvent clickCount];
    SDL_SendMouseButtonClicks(_data->window, 0, SDL_PRESSED, button, clicks);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    [self mouseDown:theEvent];
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
    [self mouseDown:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    int button;
    int clicks;

    if ([self processHitTest:theEvent]) {
        SDL_SendWindowEvent(_data->window, SDL_WINDOWEVENT_HIT_TEST, 0, 0);
        return;  /* stopped dragging, drop event. */
    }

    switch ([theEvent buttonNumber]) {
    case 0:
        if (wasCtrlLeft) {
            button = SDL_BUTTON_RIGHT;
            wasCtrlLeft = NO;
        } else {
            button = SDL_BUTTON_LEFT;
        }
        break;
    case 1:
        button = SDL_BUTTON_RIGHT;
        break;
    case 2:
        button = SDL_BUTTON_MIDDLE;
        break;
    default:
        button = (int) [theEvent buttonNumber] + 1;
        break;
    }

    clicks = (int) [theEvent clickCount];
    SDL_SendMouseButtonClicks(_data->window, 0, SDL_RELEASED, button, clicks);
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
    [self mouseUp:theEvent];
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
    [self mouseUp:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_Window *window = _data->window;
    NSPoint point;
    int x, y;

    if ([self processHitTest:theEvent]) {
        SDL_SendWindowEvent(window, SDL_WINDOWEVENT_HIT_TEST, 0, 0);
        return;  /* dragging, drop event. */
    }

    if (mouse->relative_mode) {
        return;
    }

    point = [theEvent locationInWindow];
    x = (int)point.x;
    y = (int)(window->h - point.y);

    if (window->flags & SDL_WINDOW_INPUT_GRABBED) {
        if (x < 0 || x >= window->w || y < 0 || y >= window->h) {
            if (x < 0) {
                x = 0;
            } else if (x >= window->w) {
                x = window->w - 1;
            }
            if (y < 0) {
                y = 0;
            } else if (y >= window->h) {
                y = window->h - 1;
            }

#if !SDL_MAC_NO_SANDBOX
            CGPoint cgpoint;

            /* When SDL_MAC_NO_SANDBOX is set, this is handled by
             * SDL_cocoamousetap.m.
             */

            cgpoint.x = window->x + x;
            cgpoint.y = window->y + y;

            CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, cgpoint);
            CGAssociateMouseAndMouseCursorPosition(YES);

            Cocoa_HandleMouseWarp(cgpoint.x, cgpoint.y);
#endif
        }
    }
    SDL_SendMouseMotion(window, 0, 0, x, y);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    [self mouseMoved:theEvent];
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
    [self mouseMoved:theEvent];
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
    [self mouseMoved:theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    Cocoa_HandleMouseWheel(_data->window, theEvent);
}

- (void)touchesBeganWithEvent:(NSEvent *) theEvent
{
    NSSet *touches = [theEvent touchesMatchingPhase:NSTouchPhaseAny inView:nil];
    int existingTouchCount = 0;

    for (NSTouch* touch in touches) {
        if ([touch phase] != NSTouchPhaseBegan) {
            existingTouchCount++;
        }
    }
    if (existingTouchCount == 0) {
        SDL_TouchID touchID = (SDL_TouchID)(intptr_t)[[touches anyObject] device];
        int numFingers = SDL_GetNumTouchFingers(touchID);
        DLog("Reset Lost Fingers: %d", numFingers);
        for (--numFingers; numFingers >= 0; --numFingers) {
            SDL_Finger* finger = SDL_GetTouchFinger(touchID, numFingers);
            SDL_SendTouch(touchID, finger->id, SDL_FALSE, 0, 0, 0);
        }
    }

    DLog("Began Fingers: %lu .. existing: %d", (unsigned long)[touches count], existingTouchCount);
    [self handleTouches:NSTouchPhaseBegan withEvent:theEvent];
}

- (void)touchesMovedWithEvent:(NSEvent *) theEvent
{
    [self handleTouches:NSTouchPhaseMoved withEvent:theEvent];
}

- (void)touchesEndedWithEvent:(NSEvent *) theEvent
{
    [self handleTouches:NSTouchPhaseEnded withEvent:theEvent];
}

- (void)touchesCancelledWithEvent:(NSEvent *) theEvent
{
    [self handleTouches:NSTouchPhaseCancelled withEvent:theEvent];
}

- (void)handleTouches:(NSTouchPhase) phase withEvent:(NSEvent *) theEvent
{
    NSSet *touches = [theEvent touchesMatchingPhase:phase inView:nil];

    for (NSTouch *touch in touches) {
        const SDL_TouchID touchId = (SDL_TouchID)(intptr_t)[touch device];
        if (SDL_AddTouch(touchId, "") < 0) {
            return;
        }

        const SDL_FingerID fingerId = (SDL_FingerID)(intptr_t)[touch identity];
        float x = [touch normalizedPosition].x;
        float y = [touch normalizedPosition].y;
        /* Make the origin the upper left instead of the lower left */
        y = 1.0f - y;

        switch (phase) {
        case NSTouchPhaseBegan:
            SDL_SendTouch(touchId, fingerId, SDL_TRUE, x, y, 1.0f);
            break;
        case NSTouchPhaseEnded:
        case NSTouchPhaseCancelled:
            SDL_SendTouch(touchId, fingerId, SDL_FALSE, x, y, 1.0f);
            break;
        case NSTouchPhaseMoved:
            SDL_SendTouchMotion(touchId, fingerId, x, y, 1.0f);
            break;
        default:
            break;
        }
    }
}

@end

@interface SDLView : NSView {
    SDL_Window *_sdlWindow;
}

- (void)setSDLWindow:(SDL_Window*)window;

/* The default implementation doesn't pass rightMouseDown to responder chain */
- (void)rightMouseDown:(NSEvent *)theEvent;
- (BOOL)mouseDownCanMoveWindow;
- (void)drawRect:(NSRect)dirtyRect;
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent;
@end

@implementation SDLView
- (void)setSDLWindow:(SDL_Window*)window
{
    _sdlWindow = window;
}

- (void)drawRect:(NSRect)dirtyRect
{
    /* Force the graphics context to clear to black so we don't get a flash of
       white until the app is ready to draw. In practice on modern macOS, this
       only gets called for window creation and other extraordinary events. */
    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);
    SDL_SendWindowEvent(_sdlWindow, SDL_WINDOWEVENT_EXPOSED, 0, 0);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    [[self nextResponder] rightMouseDown:theEvent];
}

- (BOOL)mouseDownCanMoveWindow
{
    /* Always say YES, but this doesn't do anything until we call
       -[NSWindow setMovableByWindowBackground:YES], which we ninja-toggle
       during mouse events when we're using a drag area. */
    return YES;
}

- (void)resetCursorRects
{
    [super resetCursorRects];
    SDL_Mouse *mouse = SDL_GetMouse();

    if (mouse->cursor_shown && mouse->cur_cursor && !mouse->relative_mode) {
        [self addCursorRect:[self bounds]
                     cursor:mouse->cur_cursor->driverdata];
    } else {
        [self addCursorRect:[self bounds]
                     cursor:[NSCursor invisibleCursor]];
    }
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    if (SDL_GetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH)) {
        return SDL_GetHintBoolean(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, SDL_FALSE);
    } else {
        return SDL_GetHintBoolean("SDL_MAC_MOUSE_FOCUS_CLICKTHROUGH", SDL_FALSE);
    }
}
@end

static int
SetupWindowData(_THIS, SDL_Window * window, NSWindow *nswindow, SDL_bool created)
{ @autoreleasepool
{
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    SDL_WindowData *data;

    /* Allocate the window data */
    window->driverdata = data = (SDL_WindowData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        return SDL_OutOfMemory();
    }
    data->window = window;
    data->nswindow = nswindow;
    data->created = created;
    data->videodata = videodata;
    data->nscontexts = [[NSMutableArray alloc] init];

    /* Create an event listener for the window */
    data->listener = [[Cocoa_WindowListener alloc] init];

    /* Fill in the SDL window with the window data */
    {
        NSRect rect = [nswindow contentRectForFrameRect:[nswindow frame]];
        ConvertNSRect([nswindow screen], (window->flags & FULLSCREEN_MASK), &rect);
        window->x = (int)rect.origin.x;
        window->y = (int)rect.origin.y;
        window->w = (int)rect.size.width;
        window->h = (int)rect.size.height;
    }

    /* Set up the listener after we create the view */
    [data->listener listen:data];

    if ([nswindow isVisible]) {
        window->flags |= SDL_WINDOW_SHOWN;
    } else {
        window->flags &= ~SDL_WINDOW_SHOWN;
    }

    {
        unsigned long style = [nswindow styleMask];

        if (style == NSWindowStyleMaskBorderless) {
            window->flags |= SDL_WINDOW_BORDERLESS;
        } else {
            window->flags &= ~SDL_WINDOW_BORDERLESS;
        }
        if (style & NSWindowStyleMaskResizable) {
            window->flags |= SDL_WINDOW_RESIZABLE;
        } else {
            window->flags &= ~SDL_WINDOW_RESIZABLE;
        }
    }

    /* isZoomed always returns true if the window is not resizable */
    if ((window->flags & SDL_WINDOW_RESIZABLE) && [nswindow isZoomed]) {
        window->flags |= SDL_WINDOW_MAXIMIZED;
    } else {
        window->flags &= ~SDL_WINDOW_MAXIMIZED;
    }

    if ([nswindow isMiniaturized]) {
        window->flags |= SDL_WINDOW_MINIMIZED;
    } else {
        window->flags &= ~SDL_WINDOW_MINIMIZED;
    }

    if ([nswindow isKeyWindow]) {
        window->flags |= SDL_WINDOW_INPUT_FOCUS;
        SDL_SetKeyboardFocus(data->window);
    }

    /* Prevents the window's "window device" from being destroyed when it is
     * hidden. See http://www.mikeash.com/pyblog/nsopenglcontext-and-one-shot.html
     */
    [nswindow setOneShot:NO];

    /* All done! */
    window->driverdata = data;
    return 0;
}}

int
Cocoa_CreateWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    NSWindow *nswindow;
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
    NSRect rect;
    SDL_Rect bounds;
    NSUInteger style;
    NSArray *screens = [NSScreen screens];

    Cocoa_GetDisplayBounds(_this, display, &bounds);
    rect.origin.x = window->x;
    rect.origin.y = window->y;
    rect.size.width = window->w;
    rect.size.height = window->h;
    ConvertNSRect([screens objectAtIndex:0], (window->flags & FULLSCREEN_MASK), &rect);

    style = GetWindowStyle(window);

    /* Figure out which screen to place this window */
    NSScreen *screen = nil;
    for (NSScreen *candidate in screens) {
        NSRect screenRect = [candidate frame];
        if (rect.origin.x >= screenRect.origin.x &&
            rect.origin.x < screenRect.origin.x + screenRect.size.width &&
            rect.origin.y >= screenRect.origin.y &&
            rect.origin.y < screenRect.origin.y + screenRect.size.height) {
            screen = candidate;
            rect.origin.x -= screenRect.origin.x;
            rect.origin.y -= screenRect.origin.y;
        }
    }

    @try {
        nswindow = [[SDLWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO screen:screen];
    }
    @catch (NSException *e) {
        return SDL_SetError("%s", [[e reason] UTF8String]);
    }

    if (videodata->allow_spaces) {
        SDL_assert(floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6);
        SDL_assert([nswindow respondsToSelector:@selector(toggleFullScreen:)]);
        /* we put FULLSCREEN_DESKTOP windows in their own Space, without a toggle button or menubar, later */
        if (window->flags & SDL_WINDOW_RESIZABLE) {
            /* resizable windows are Spaces-friendly: they get the "go fullscreen" toggle button on their titlebar. */
            [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        }
    }

    /* Create a default view for this window */
    rect = [nswindow contentRectForFrameRect:[nswindow frame]];
    SDLView *contentView = [[SDLView alloc] initWithFrame:rect];
    [contentView setSDLWindow:window];

    if (window->flags & SDL_WINDOW_ALLOW_HIGHDPI) {
        if ([contentView respondsToSelector:@selector(setWantsBestResolutionOpenGLSurface:)]) {
            [contentView setWantsBestResolutionOpenGLSurface:YES];
        }
    }
#if SDL_VIDEO_OPENGL_ES2
#if SDL_VIDEO_OPENGL_EGL
    if ((window->flags & SDL_WINDOW_OPENGL) &&
        _this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
        [contentView setWantsLayer:TRUE];
    }
#endif /* SDL_VIDEO_OPENGL_EGL */
#endif /* SDL_VIDEO_OPENGL_ES2 */
    [nswindow setContentView:contentView];
    [contentView release];

    /* Allow files and folders to be dragged onto the window by users */
    [nswindow registerForDraggedTypes:[NSArray arrayWithObject:(NSString *)kUTTypeFileURL]];

    if (SetupWindowData(_this, window, nswindow, SDL_TRUE) < 0) {
        [nswindow release];
        return -1;
    }

    if (!(window->flags & SDL_WINDOW_OPENGL)) {
        return 0;
    }
    
    /* The rest of this macro mess is for OpenGL or OpenGL ES windows */
#if SDL_VIDEO_OPENGL_ES2
    if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
#if SDL_VIDEO_OPENGL_EGL
        if (Cocoa_GLES_SetupWindow(_this, window) < 0) {
            Cocoa_DestroyWindow(_this, window);
            return -1;
        }
        return 0;
#else
        return SDL_SetError("Could not create GLES window surface (EGL support not configured)");
#endif /* SDL_VIDEO_OPENGL_EGL */
    }
#endif /* SDL_VIDEO_OPENGL_ES2 */
    return 0;
}}

int
Cocoa_CreateWindowFrom(_THIS, SDL_Window * window, const void *data)
{ @autoreleasepool
{
    NSWindow *nswindow = (NSWindow *) data;
    NSString *title;

    /* Query the title from the existing window */
    title = [nswindow title];
    if (title) {
        window->title = SDL_strdup([title UTF8String]);
    }

    return SetupWindowData(_this, window, nswindow, SDL_FALSE);
}}

void
Cocoa_SetWindowTitle(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    const char *title = window->title ? window->title : "";
    NSWindow *nswindow = ((SDL_WindowData *) window->driverdata)->nswindow;
    NSString *string = [[NSString alloc] initWithUTF8String:title];
    [nswindow setTitle:string];
    [string release];
}}

void
Cocoa_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon)
{ @autoreleasepool
{
    NSImage *nsimage = Cocoa_CreateImage(icon);

    if (nsimage) {
        [NSApp setApplicationIconImage:nsimage];
    }
}}

void
Cocoa_SetWindowPosition(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *windata = (SDL_WindowData *) window->driverdata;
    NSWindow *nswindow = windata->nswindow;
    NSRect rect;
    Uint32 moveHack;

    rect.origin.x = window->x;
    rect.origin.y = window->y;
    rect.size.width = window->w;
    rect.size.height = window->h;
    ConvertNSRect([nswindow screen], (window->flags & FULLSCREEN_MASK), &rect);

    moveHack = s_moveHack;
    s_moveHack = 0;
    [nswindow setFrameOrigin:rect.origin];
    s_moveHack = moveHack;

    ScheduleContextUpdates(windata);
}}

void
Cocoa_SetWindowSize(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *windata = (SDL_WindowData *) window->driverdata;
    NSWindow *nswindow = windata->nswindow;
    NSRect rect;
    Uint32 moveHack;

    /* Cocoa will resize the window from the bottom-left rather than the
     * top-left when -[nswindow setContentSize:] is used, so we must set the
     * entire frame based on the new size, in order to preserve the position.
     */
    rect.origin.x = window->x;
    rect.origin.y = window->y;
    rect.size.width = window->w;
    rect.size.height = window->h;
    ConvertNSRect([nswindow screen], (window->flags & FULLSCREEN_MASK), &rect);

    moveHack = s_moveHack;
    s_moveHack = 0;
    [nswindow setFrame:[nswindow frameRectForContentRect:rect] display:YES];
    s_moveHack = moveHack;

    ScheduleContextUpdates(windata);
}}

void
Cocoa_SetWindowMinimumSize(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *windata = (SDL_WindowData *) window->driverdata;

    NSSize minSize;
    minSize.width = window->min_w;
    minSize.height = window->min_h;

    [windata->nswindow setContentMinSize:minSize];
}}

void
Cocoa_SetWindowMaximumSize(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *windata = (SDL_WindowData *) window->driverdata;

    NSSize maxSize;
    maxSize.width = window->max_w;
    maxSize.height = window->max_h;

    [windata->nswindow setContentMaxSize:maxSize];
}}

void
Cocoa_ShowWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *windowData = ((SDL_WindowData *) window->driverdata);
    NSWindow *nswindow = windowData->nswindow;

    if (![nswindow isMiniaturized]) {
        [windowData->listener pauseVisibleObservation];
        [nswindow makeKeyAndOrderFront:nil];
        [windowData->listener resumeVisibleObservation];
    }
}}

void
Cocoa_HideWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    NSWindow *nswindow = ((SDL_WindowData *) window->driverdata)->nswindow;

    [nswindow orderOut:nil];
}}

void
Cocoa_RaiseWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *windowData = ((SDL_WindowData *) window->driverdata);
    NSWindow *nswindow = windowData->nswindow;

    /* makeKeyAndOrderFront: has the side-effect of deminiaturizing and showing
       a minimized or hidden window, so check for that before showing it.
     */
    [windowData->listener pauseVisibleObservation];
    if (![nswindow isMiniaturized] && [nswindow isVisible]) {
        [NSApp activateIgnoringOtherApps:YES];
        [nswindow makeKeyAndOrderFront:nil];
    }
    [windowData->listener resumeVisibleObservation];
}}

void
Cocoa_MaximizeWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *windata = (SDL_WindowData *) window->driverdata;
    NSWindow *nswindow = windata->nswindow;

    [nswindow zoom:nil];

    ScheduleContextUpdates(windata);
}}

void
Cocoa_MinimizeWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    NSWindow *nswindow = data->nswindow;

    if ([data->listener isInFullscreenSpaceTransition]) {
        [data->listener addPendingWindowOperation:PENDING_OPERATION_MINIMIZE];
    } else {
        [nswindow miniaturize:nil];
    }
}}

void
Cocoa_RestoreWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    NSWindow *nswindow = ((SDL_WindowData *) window->driverdata)->nswindow;

    if ([nswindow isMiniaturized]) {
        [nswindow deminiaturize:nil];
    } else if ((window->flags & SDL_WINDOW_RESIZABLE) && [nswindow isZoomed]) {
        [nswindow zoom:nil];
    }
}}

void
Cocoa_SetWindowBordered(_THIS, SDL_Window * window, SDL_bool bordered)
{ @autoreleasepool
{
    if (SetWindowStyle(window, GetWindowStyle(window))) {
        if (bordered) {
            Cocoa_SetWindowTitle(_this, window);  /* this got blanked out. */
        }
    }
}}

void
Cocoa_SetWindowResizable(_THIS, SDL_Window * window, SDL_bool resizable)
{ @autoreleasepool
{
    /* Don't set this if we're in a space!
     * The window will get permanently stuck if resizable is false.
     * -flibit
     */
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Cocoa_WindowListener *listener = data->listener;
    if (![listener isInFullscreenSpace]) {
        SetWindowStyle(window, GetWindowStyle(window));
    }
}}

void
Cocoa_SetWindowFullscreen(_THIS, SDL_Window * window, SDL_VideoDisplay * display, SDL_bool fullscreen)
{ @autoreleasepool
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    NSWindow *nswindow = data->nswindow;
    NSRect rect;

    /* The view responder chain gets messed with during setStyleMask */
    if ([[nswindow contentView] nextResponder] == data->listener) {
        [[nswindow contentView] setNextResponder:nil];
    }

    if (fullscreen) {
        SDL_Rect bounds;

        Cocoa_GetDisplayBounds(_this, display, &bounds);
        rect.origin.x = bounds.x;
        rect.origin.y = bounds.y;
        rect.size.width = bounds.w;
        rect.size.height = bounds.h;
        ConvertNSRect([nswindow screen], fullscreen, &rect);

        /* Hack to fix origin on Mac OS X 10.4 */
        NSRect screenRect = [[nswindow screen] frame];
        if (screenRect.size.height >= 1.0f) {
            rect.origin.y += (screenRect.size.height - rect.size.height);
        }

        [nswindow setStyleMask:NSWindowStyleMaskBorderless];
    } else {
        rect.origin.x = window->windowed.x;
        rect.origin.y = window->windowed.y;
        rect.size.width = window->windowed.w;
        rect.size.height = window->windowed.h;
        ConvertNSRect([nswindow screen], fullscreen, &rect);

        [nswindow setStyleMask:GetWindowStyle(window)];

        /* Hack to restore window decorations on Mac OS X 10.10 */
        NSRect frameRect = [nswindow frame];
        [nswindow setFrame:NSMakeRect(frameRect.origin.x, frameRect.origin.y, frameRect.size.width + 1, frameRect.size.height) display:NO];
        [nswindow setFrame:frameRect display:NO];
    }

    /* The view responder chain gets messed with during setStyleMask */
    if ([[nswindow contentView] nextResponder] != data->listener) {
        [[nswindow contentView] setNextResponder:data->listener];
    }

    s_moveHack = 0;
    [nswindow setContentSize:rect.size];
    [nswindow setFrameOrigin:rect.origin];
    s_moveHack = SDL_GetTicks();

    /* When the window style changes the title is cleared */
    if (!fullscreen) {
        Cocoa_SetWindowTitle(_this, window);
    }

    if (SDL_ShouldAllowTopmost() && fullscreen) {
        /* OpenGL is rendering to the window, so make it visible! */
        [nswindow setLevel:CGShieldingWindowLevel()];
    } else {
        [nswindow setLevel:kCGNormalWindowLevel];
    }

    if ([nswindow isVisible] || fullscreen) {
        [data->listener pauseVisibleObservation];
        [nswindow makeKeyAndOrderFront:nil];
        [data->listener resumeVisibleObservation];
    }

    ScheduleContextUpdates(data);
}}

int
Cocoa_SetWindowGammaRamp(_THIS, SDL_Window * window, const Uint16 * ramp)
{
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
    CGDirectDisplayID display_id = ((SDL_DisplayData *)display->driverdata)->display;
    const uint32_t tableSize = 256;
    CGGammaValue redTable[tableSize];
    CGGammaValue greenTable[tableSize];
    CGGammaValue blueTable[tableSize];
    uint32_t i;
    float inv65535 = 1.0f / 65535.0f;

    /* Extract gamma values into separate tables, convert to floats between 0.0 and 1.0 */
    for (i = 0; i < 256; i++) {
        redTable[i] = ramp[0*256+i] * inv65535;
        greenTable[i] = ramp[1*256+i] * inv65535;
        blueTable[i] = ramp[2*256+i] * inv65535;
    }

    if (CGSetDisplayTransferByTable(display_id, tableSize,
                                    redTable, greenTable, blueTable) != CGDisplayNoErr) {
        return SDL_SetError("CGSetDisplayTransferByTable()");
    }
    return 0;
}

int
Cocoa_GetWindowGammaRamp(_THIS, SDL_Window * window, Uint16 * ramp)
{
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
    CGDirectDisplayID display_id = ((SDL_DisplayData *)display->driverdata)->display;
    const uint32_t tableSize = 256;
    CGGammaValue redTable[tableSize];
    CGGammaValue greenTable[tableSize];
    CGGammaValue blueTable[tableSize];
    uint32_t i, tableCopied;

    if (CGGetDisplayTransferByTable(display_id, tableSize,
                                    redTable, greenTable, blueTable, &tableCopied) != CGDisplayNoErr) {
        return SDL_SetError("CGGetDisplayTransferByTable()");
    }

    for (i = 0; i < tableCopied; i++) {
        ramp[0*256+i] = (Uint16)(redTable[i] * 65535.0f);
        ramp[1*256+i] = (Uint16)(greenTable[i] * 65535.0f);
        ramp[2*256+i] = (Uint16)(blueTable[i] * 65535.0f);
    }
    return 0;
}

void
Cocoa_SetWindowGrab(_THIS, SDL_Window * window, SDL_bool grabbed)
{
    SDL_Mouse *mouse = SDL_GetMouse();
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;

    /* Enable or disable the event tap as necessary */
    Cocoa_EnableMouseEventTap(mouse->driverdata, grabbed);

    /* Move the cursor to the nearest point in the window */
    if (grabbed && data && ![data->listener isMoving]) {
        int x, y;
        CGPoint cgpoint;

        SDL_GetMouseState(&x, &y);
        cgpoint.x = window->x + x;
        cgpoint.y = window->y + y;

        Cocoa_HandleMouseWarp(cgpoint.x, cgpoint.y);

        DLog("Returning cursor to (%g, %g)", cgpoint.x, cgpoint.y);
        CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, cgpoint);
    }

    if ( data && (window->flags & SDL_WINDOW_FULLSCREEN) ) {
        if (SDL_ShouldAllowTopmost() && (window->flags & SDL_WINDOW_INPUT_FOCUS)
            && ![data->listener isInFullscreenSpace]) {
            /* OpenGL is rendering to the window, so make it visible! */
            /* Doing this in 10.11 while in a Space breaks things (bug #3152) */
            [data->nswindow setLevel:CGShieldingWindowLevel()];
        } else {
            [data->nswindow setLevel:kCGNormalWindowLevel];
        }
    }
}

void
Cocoa_DestroyWindow(_THIS, SDL_Window * window)
{ @autoreleasepool
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;

    if (data) {
        if ([data->listener isInFullscreenSpace]) {
            [NSMenu setMenuBarVisible:YES];
        }
        [data->listener close];
        [data->listener release];
        if (data->created) {
            [data->nswindow close];
        }

        NSArray *contexts = [[data->nscontexts copy] autorelease];
        for (SDLOpenGLContext *context in contexts) {
            /* Calling setWindow:NULL causes the context to remove itself from the context list. */            
            [context setWindow:NULL];
        }
        [data->nscontexts release];

        SDL_free(data);
    }
    window->driverdata = NULL;
}}

SDL_bool
Cocoa_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info)
{
    NSWindow *nswindow = ((SDL_WindowData *) window->driverdata)->nswindow;

    if (info->version.major <= SDL_MAJOR_VERSION) {
        info->subsystem = SDL_SYSWM_COCOA;
        info->info.cocoa.window = nswindow;
        return SDL_TRUE;
    } else {
        SDL_SetError("Application not compiled with SDL %d.%d",
                     SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
        return SDL_FALSE;
    }
}

SDL_bool
Cocoa_IsWindowInFullscreenSpace(SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;

    if ([data->listener isInFullscreenSpace]) {
        return SDL_TRUE;
    } else {
        return SDL_FALSE;
    }
}

SDL_bool
Cocoa_SetWindowFullscreenSpace(SDL_Window * window, SDL_bool state)
{ @autoreleasepool
{
    SDL_bool succeeded = SDL_FALSE;
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;

    if ([data->listener setFullscreenSpace:(state ? YES : NO)]) {
        const int maxattempts = 3;
        int attempt = 0;
        while (++attempt <= maxattempts) {
            /* Wait for the transition to complete, so application changes
             take effect properly (e.g. setting the window size, etc.)
             */
            const int limit = 10000;
            int count = 0;
            while ([data->listener isInFullscreenSpaceTransition]) {
                if ( ++count == limit ) {
                    /* Uh oh, transition isn't completing. Should we assert? */
                    break;
                }
                SDL_Delay(1);
                SDL_PumpEvents();
            }
            if ([data->listener isInFullscreenSpace] == (state ? YES : NO))
                break;
            /* Try again, the last attempt was interrupted by user gestures */
            if (![data->listener setFullscreenSpace:(state ? YES : NO)])
                break; /* ??? */
        }
        /* Return TRUE to prevent non-space fullscreen logic from running */
        succeeded = SDL_TRUE;
    }

    return succeeded;
}}

int
Cocoa_SetWindowHitTest(SDL_Window * window, SDL_bool enabled)
{
    return 0;  /* just succeed, the real work is done elsewhere. */
}

int
Cocoa_SetWindowOpacity(_THIS, SDL_Window * window, float opacity)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    [data->nswindow setAlphaValue:opacity];
    return 0;
}

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vi: set ts=4 sw=4 expandtab: */
