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
#include "SDL_timer.h"

#include "SDL_cocoavideo.h"
#include "../../events/SDL_events_c.h"
#include "SDL_assert.h"
#include "SDL_hints.h"

/* This define was added in the 10.9 SDK. */
#ifndef kIOPMAssertPreventUserIdleDisplaySleep
#define kIOPMAssertPreventUserIdleDisplaySleep kIOPMAssertionTypePreventUserIdleDisplaySleep
#endif

@interface SDLApplication : NSApplication

- (void)terminate:(id)sender;
- (void)sendEvent:(NSEvent *)theEvent;

+ (void)registerUserDefaults;

@end

@implementation SDLApplication

// Override terminate to handle Quit and System Shutdown smoothly.
- (void)terminate:(id)sender
{
    SDL_SendQuit();
}

static SDL_bool s_bShouldHandleEventsInSDLApplication = SDL_FALSE;

static void Cocoa_DispatchEvent(NSEvent *theEvent)
{
    SDL_VideoDevice *_this = SDL_GetVideoDevice();

    switch ([theEvent type]) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeOtherMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeLeftMouseUp:
        case NSEventTypeOtherMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged: /* usually middle mouse dragged */
        case NSEventTypeMouseMoved:
        case NSEventTypeScrollWheel:
            Cocoa_HandleMouseEvent(_this, theEvent);
            break;
        case NSEventTypeKeyDown:
        case NSEventTypeKeyUp:
        case NSEventTypeFlagsChanged:
            Cocoa_HandleKeyEvent(_this, theEvent);
            break;
        default:
            break;
    }
}

// Dispatch events here so that we can handle events caught by
// nextEventMatchingMask in SDL, as well as events caught by other
// processes (such as CEF) that are passed down to NSApp.
- (void)sendEvent:(NSEvent *)theEvent
{
    if (s_bShouldHandleEventsInSDLApplication) {
        Cocoa_DispatchEvent(theEvent);
    }

    [super sendEvent:theEvent];
}

+ (void)registerUserDefaults
{
    NSDictionary *appDefaults = [[NSDictionary alloc] initWithObjectsAndKeys:
                                 [NSNumber numberWithBool:NO], @"AppleMomentumScrollSupported",
                                 [NSNumber numberWithBool:NO], @"ApplePressAndHoldEnabled",
                                 [NSNumber numberWithBool:YES], @"ApplePersistenceIgnoreState",
                                 nil];
    [[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
    [appDefaults release];
}

@end // SDLApplication

/* setAppleMenu disappeared from the headers in 10.4 */
@interface NSApplication(NSAppleMenu)
- (void)setAppleMenu:(NSMenu *)menu;
@end

@interface SDLAppDelegate : NSObject <NSApplicationDelegate> {
@public
    BOOL seenFirstActivate;
}

- (id)init;
@end

@implementation SDLAppDelegate : NSObject
- (id)init
{
    self = [super init];
    if (self) {
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

        seenFirstActivate = NO;

        [center addObserver:self
                   selector:@selector(windowWillClose:)
                       name:NSWindowWillCloseNotification
                     object:nil];

        [center addObserver:self
                   selector:@selector(focusSomeWindow:)
                       name:NSApplicationDidBecomeActiveNotification
                     object:nil];
    }

    return self;
}

- (void)dealloc
{
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

    [center removeObserver:self name:NSWindowWillCloseNotification object:nil];
    [center removeObserver:self name:NSApplicationDidBecomeActiveNotification object:nil];

    [super dealloc];
}

- (void)windowWillClose:(NSNotification *)notification;
{
    NSWindow *win = (NSWindow*)[notification object];

    if (![win isKeyWindow]) {
        return;
    }

    /* HACK: Make the next window in the z-order key when the key window is
     * closed. The custom event loop and/or windowing code we have seems to
     * prevent the normal behavior: https://bugzilla.libsdl.org/show_bug.cgi?id=1825
     */

    /* +[NSApp orderedWindows] never includes the 'About' window, but we still
     * want to try its list first since the behavior in other apps is to only
     * make the 'About' window key if no other windows are on-screen.
     */
    for (NSWindow *window in [NSApp orderedWindows]) {
        if (window != win && [window canBecomeKeyWindow]) {
            if (![window isOnActiveSpace]) {
                continue;
            }
            [window makeKeyAndOrderFront:self];
            return;
        }
    }

    /* If a window wasn't found above, iterate through all visible windows in
     * the active Space in z-order (including the 'About' window, if it's shown)
     * and make the first one key.
     */
    for (NSNumber *num in [NSWindow windowNumbersWithOptions:0]) {
        NSWindow *window = [NSApp windowWithWindowNumber:[num integerValue]];
        if (window && window != win && [window canBecomeKeyWindow]) {
            [window makeKeyAndOrderFront:self];
            return;
        }
    }
}

- (void)focusSomeWindow:(NSNotification *)aNotification
{
    /* HACK: Ignore the first call. The application gets a
     * applicationDidBecomeActive: a little bit after the first window is
     * created, and if we don't ignore it, a window that has been created with
     * SDL_WINDOW_MINIMIZED will ~immediately be restored.
     */
    if (!seenFirstActivate) {
        seenFirstActivate = YES;
        return;
    }

    SDL_VideoDevice *device = SDL_GetVideoDevice();
    if (device && device->windows) {
        SDL_Window *window = device->windows;
        int i;
        for (i = 0; i < device->num_displays; ++i) {
            SDL_Window *fullscreen_window = device->displays[i].fullscreen_window;
            if (fullscreen_window) {
                if (fullscreen_window->flags & SDL_WINDOW_MINIMIZED) {
                    SDL_RestoreWindow(fullscreen_window);
                }
                return;
            }
        }

        if (window->flags & SDL_WINDOW_MINIMIZED) {
            SDL_RestoreWindow(window);
        } else {
            SDL_RaiseWindow(window);
        }
    }
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    return (BOOL)SDL_SendDropFile(NULL, [filename UTF8String]) && SDL_SendDropComplete(NULL);
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    /* The menu bar of SDL apps which don't have the typical .app bundle
     * structure fails to work the first time a window is created (until it's
     * de-focused and re-focused), if this call is in Cocoa_RegisterApp instead
     * of here. https://bugzilla.libsdl.org/show_bug.cgi?id=3051
     */
    if (!SDL_GetHintBoolean(SDL_HINT_MAC_BACKGROUND_APP, SDL_FALSE)) {
        [NSApp activateIgnoringOtherApps:YES];
    }

    /* If we call this before NSApp activation, macOS might print a complaint
     * about ApplePersistenceIgnoreState. */
    [SDLApplication registerUserDefaults];
}
@end

static SDLAppDelegate *appDelegate = nil;

static NSString *
GetApplicationName(void)
{
    NSString *appName;

    /* Determine the application name */
    appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"];
    if (!appName) {
        appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    }

    if (![appName length]) {
        appName = [[NSProcessInfo processInfo] processName];
    }

    return appName;
}

static void
CreateApplicationMenus(void)
{
    NSString *appName;
    NSString *title;
    NSMenu *appleMenu;
    NSMenu *serviceMenu;
    NSMenu *windowMenu;
    NSMenu *viewMenu;
    NSMenuItem *menuItem;
    NSMenu *mainMenu;

    if (NSApp == nil) {
        return;
    }

    mainMenu = [[NSMenu alloc] init];

    /* Create the main menu bar */
    [NSApp setMainMenu:mainMenu];

    [mainMenu release];  /* we're done with it, let NSApp own it. */
    mainMenu = nil;

    /* Create the application menu */
    appName = GetApplicationName();
    appleMenu = [[NSMenu alloc] initWithTitle:@""];

    /* Add menu items */
    title = [@"About " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    [appleMenu addItemWithTitle:@"Preferences…" action:nil keyEquivalent:@","];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    serviceMenu = [[NSMenu alloc] initWithTitle:@""];
    menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Services" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:serviceMenu];

    [NSApp setServicesMenu:serviceMenu];
    [serviceMenu release];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Hide " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

    menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];

    [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Quit " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];

    /* Put menu into the menubar */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:appleMenu];
    [[NSApp mainMenu] addItem:menuItem];
    [menuItem release];

    /* Tell the application object that this is now the application menu */
    [NSApp setAppleMenu:appleMenu];
    [appleMenu release];


    /* Create the window menu */
    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];

    /* Add menu items */
    [windowMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];

    [windowMenu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];

    /* Put menu into the menubar */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:windowMenu];
    [[NSApp mainMenu] addItem:menuItem];
    [menuItem release];

    /* Tell the application object that this is now the window menu */
    [NSApp setWindowsMenu:windowMenu];
    [windowMenu release];


    /* Add the fullscreen view toggle menu option, if supported */
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6) {
        /* Create the view menu */
        viewMenu = [[NSMenu alloc] initWithTitle:@"View"];

        /* Add menu items */
        menuItem = [viewMenu addItemWithTitle:@"Toggle Full Screen" action:@selector(toggleFullScreen:) keyEquivalent:@"f"];
        [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];

        /* Put menu into the menubar */
        menuItem = [[NSMenuItem alloc] initWithTitle:@"View" action:nil keyEquivalent:@""];
        [menuItem setSubmenu:viewMenu];
        [[NSApp mainMenu] addItem:menuItem];
        [menuItem release];

        [viewMenu release];
    }
}

void
Cocoa_RegisterApp(void)
{ @autoreleasepool
{
    /* This can get called more than once! Be careful what you initialize! */

    if (NSApp == nil) {
        [SDLApplication sharedApplication];
        SDL_assert(NSApp != nil);

        s_bShouldHandleEventsInSDLApplication = SDL_TRUE;

        if (!SDL_GetHintBoolean(SDL_HINT_MAC_BACKGROUND_APP, SDL_FALSE)) {
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		}
		
        if ([NSApp mainMenu] == nil) {
            CreateApplicationMenus();
        }
        [NSApp finishLaunching];
        if ([NSApp delegate]) {
            /* The SDL app delegate calls this in didFinishLaunching if it's
             * attached to the NSApp, otherwise we need to call it manually.
             */
            [SDLApplication registerUserDefaults];
        }
    }
    if (NSApp && !appDelegate) {
        appDelegate = [[SDLAppDelegate alloc] init];

        /* If someone else has an app delegate, it means we can't turn a
         * termination into SDL_Quit, and we can't handle application:openFile:
         */
        if (![NSApp delegate]) {
            [(NSApplication *)NSApp setDelegate:appDelegate];
        } else {
            appDelegate->seenFirstActivate = YES;
        }
    }
}}

void
Cocoa_PumpEvents(_THIS)
{ @autoreleasepool
{
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1070
    /* Update activity every 30 seconds to prevent screensaver */
    SDL_VideoData *data = (SDL_VideoData *)_this->driverdata;
    if (_this->suspend_screensaver && !data->screensaver_use_iopm) {
        Uint32 now = SDL_GetTicks();
        if (!data->screensaver_activity ||
            SDL_TICKS_PASSED(now, data->screensaver_activity + 30000)) {
            UpdateSystemActivity(UsrActivity);
            data->screensaver_activity = now;
        }
    }
#endif

    for ( ; ; ) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES ];
        if ( event == nil ) {
            break;
        }

        if (!s_bShouldHandleEventsInSDLApplication) {
            Cocoa_DispatchEvent(event);
        }

        // Pass events down to SDLApplication to be handled in sendEvent:
        [NSApp sendEvent:event];
    }
}}

void
Cocoa_SuspendScreenSaver(_THIS)
{ @autoreleasepool
{
    SDL_VideoData *data = (SDL_VideoData *)_this->driverdata;

    if (!data->screensaver_use_iopm) {
        return;
    }

    if (data->screensaver_assertion) {
        IOPMAssertionRelease(data->screensaver_assertion);
        data->screensaver_assertion = 0;
    }

    if (_this->suspend_screensaver) {
        /* FIXME: this should ideally describe the real reason why the game
         * called SDL_DisableScreenSaver. Note that the name is only meant to be
         * seen by OS X power users. there's an additional optional human-readable
         * (localized) reason parameter which we don't set.
         */
        NSString *name = [GetApplicationName() stringByAppendingString:@" using SDL_DisableScreenSaver"];
        IOPMAssertionCreateWithDescription(kIOPMAssertPreventUserIdleDisplaySleep,
                                           (CFStringRef) name,
                                           NULL, NULL, NULL, 0, NULL,
                                           &data->screensaver_assertion);
    }
}}

#endif /* SDL_VIDEO_DRIVER_COCOA */

/* vi: set ts=4 sw=4 expandtab: */
