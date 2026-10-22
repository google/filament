
#include "testnative.h"

#ifdef TEST_NATIVE_COCOA

#include <Cocoa/Cocoa.h>

static void *CreateWindowCocoa(int w, int h);
static void DestroyWindowCocoa(void *window);

NativeWindowFactory CocoaWindowFactory = {
    "cocoa",
    CreateWindowCocoa,
    DestroyWindowCocoa
};

static void *CreateWindowCocoa(int w, int h)
{
    NSAutoreleasePool *pool;
    NSWindow *nswindow;
    NSRect rect;
    unsigned int style;

    pool = [[NSAutoreleasePool alloc] init];

    rect.origin.x = 0;
    rect.origin.y = 0;
    rect.size.width = w;
    rect.size.height = h;
    rect.origin.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - rect.origin.y - rect.size.height;

    style = (NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask);

    nswindow = [[NSWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:FALSE];
    [nswindow makeKeyAndOrderFront:nil];

    [pool release];

    return nswindow;
}

static void DestroyWindowCocoa(void *window)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSWindow *nswindow = (NSWindow *)window;

    [nswindow close];
    [pool release];
}

#endif
