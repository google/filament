/*!*********************************************************************************************************************
\File         MainMacOS.mm
\Title        Main MacOS
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a MacOS surface platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSView.h>
#include <Foundation/NSTimer.h>

@interface ViewMTL : NSView
{
    id<MTLDevice>  m_device; // Metal device
}

-(ViewMTL*) initWithFrame:(NSRect) frame;

@end

@implementation ViewMTL

- (ViewMTL*) initWithFrame:(NSRect) frame
{
    m_device = MTLCreateSystemDefaultDevice();
    [m_device retain];
    [super initWithFrame:frame];
    self.wantsLayer = YES;
    return self;
}

- (void)dealloc
{
    [m_device release];
    [super dealloc];
}

- (BOOL) wantsUpdateLayer { return YES; }

+ (Class) layerClass { return [CAMetalLayer class]; }

- (CALayer*) makeBackingLayer
{
    CAMetalLayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1,1)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    layer.device = m_device;
    layer.backgroundColor = CGColorCreateGenericRGB(0.0, 0.0, 0.0, 1.0);
    return layer;
}

@end

@class AppController;

@interface AppController : NSObject <NSApplicationDelegate>
{
@private
    NSWindow*      m_window; // Our window
    ViewMTL*       m_view;   // Our view
    NSTimer*       m_timer;  // Our view

    VulkanHelloAPI m_vulkanExample;
}
@end

@implementation AppController

- (void) mainLoop
{
    m_vulkanExample.drawFrame();
}

- (void) applicationDidFinishLaunching:(NSNotification *)notification
{
    m_vulkanExample.surfaceData.width = 1280.0f;
    m_vulkanExample.surfaceData.height = 800.0f;
    
    // Create our window
    NSRect frame = NSMakeRect(0, 0, m_vulkanExample.surfaceData.width, m_vulkanExample.surfaceData.height);
    m_window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSMiniaturizableWindowMask | NSTitledWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];
    
    if(!m_window)
    {
        NSLog(@"Failed to allocated the window.");
        [self terminateApp];
    }
    
    [m_window setTitle:@"VulkanHelloAPI"];
    
    // Create our view
    m_view = [[ViewMTL alloc] initWithFrame:frame];
    m_vulkanExample.surfaceData.view = m_view;
    m_vulkanExample.initialize();
    m_vulkanExample.recordCommandBuffer();
    
    // Now we have a view, add it to our window
    [m_window setContentView:m_view];
    [m_window makeKeyAndOrderFront:nil];
    
    // Add an observer so when our window is closed we terminate the app
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(terminateApp) name:NSWindowWillCloseNotification object:m_window];
    
    m_timer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / 60.0) target:self selector:@selector(mainLoop) userInfo:nil repeats:YES];
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
    m_vulkanExample.deinitialize();
    m_vulkanExample.surfaceData.view = nullptr;
    
    // Release our view and window
    [m_view release];
    m_view = nil;
    
    [m_window release];
    m_window = nil;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

- (void) terminateApp
{
    [NSApp terminate:nil];
}

@end

int main(int argc, char** argv)
{
    return NSApplicationMain(argc, (const char **)argv);
}