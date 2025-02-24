/*!
\brief Implementation of the OS specific part of the PVRShell class for the AppKit macOS implementation
\file PVRShell/OS/AppKit/ShellOS.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/OS/AppKit/ViewMTL.h"
#include "PVRCore/stream/FilePath.h"
#include <mach/mach_time.h>
#include <Foundation/Foundation.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSScreen.h>
#include <AppKit/NSAlert.h>

@interface AppWindow : NSWindow < NSWindowDelegate> // The implementation appears at the bottom of this file
{
pvr::Shell* eventQueue;
}
@property (assign) pvr::Shell * eventQueue;
@end
using namespace ::pvr::platform;
namespace pvr{namespace platform{
class InternalOS
{
public:
	AppWindow* window;
	ViewMTL* view;

	InternalOS() : window(nil), view(nil)
	{
	}
};
}
}

int16_t g_cursorX, g_cursorY;

// Setup the capabilities
const pvr::platform::ShellOS::Capabilities pvr::platform::ShellOS::_capabilities = { pvr::Capability::Immutable, pvr::Capability::Immutable };

ShellOS::ShellOS(/*NSObject<NSApplicationDelegate>*/void* hInstance, OSDATA osdata) : _instance(hInstance)
{
	_OSImplementation = std::make_unique<InternalOS>();
}

ShellOS::~ShellOS() {}

bool ShellOS::init(DisplayAttributes &data)
{
	if(!_OSImplementation)
		return false;
	// Setup read and write paths
	NSString* readPath = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] resourcePath], @"/"];
	//_Strings[eReadPath] = [readPath UTF8String];
    _readPaths.push_back([readPath UTF8String]);
    NSString* writePath = [NSString stringWithFormat:@"%@%@", [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent], @"/"];
    _writePath = [writePath UTF8String];
	// Setup the app name
    NSString* name = [[NSProcessInfo processInfo] processName];
    
    _appName =[name UTF8String];
	return true;
}


void ShellOS::updatePointingDeviceLocation()
{
    _shell->updatePointerPosition(PointerLocation(g_cursorX, g_cursorY));
}

bool ShellOS::initializeWindow(DisplayAttributes &data)
{
	 // Now create our window
    NSRect frame;
    NSUInteger style;
    
    if(data.fullscreen)
    {
        // Set our frame to fill the screen
        frame = [[NSScreen mainScreen] frame];
        
        // Setup our fullscreen style
        style = NSBorderlessWindowMask;
        
        data.x = frame.origin.x;
        data.y = frame.origin.y;
        data.width = frame.size.width;
        data.height = frame.size.height;
    }
    else
    {
        // Get our dimensions and position as requested from the shell
        frame = NSMakeRect(data.x, data.y, data.width, data.height);
        
        // Setup our style
        style = NSMiniaturizableWindowMask | NSTitledWindowMask | NSClosableWindowMask;
    }
    
    _OSImplementation->window = [[AppWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO screen:[NSScreen mainScreen]];
    
    if(!_OSImplementation->window)
    {
        NSLog(@"Failed to allocated the window.");
        return false;
    }
    _OSImplementation->window.eventQueue = _shell.get();
	
    // Don't release our window when closed
    [_OSImplementation->window setReleasedWhenClosed:NO];
    // When the window is closed, terminate the application
    [[NSNotificationCenter defaultCenter] addObserver:(__bridge NSObject*)_instance selector:@selector(terminateApp) name:NSWindowWillCloseNotification object:_OSImplementation->window];
    
    // Set the window title
    [_OSImplementation->window setTitle:[NSString stringWithUTF8String:data.windowTitle.c_str()]];
    
    // Now create the view that we'll render to
    _OSImplementation->view = [[ViewMTL alloc] initWithFrame:frame];
    
    if(!_OSImplementation->view)
    {
        return false;
    }
    
    // Add our view to our window
    [_OSImplementation->window setContentView:_OSImplementation->view];
    
    // Save a pointer to our window and view
    if(data.fullscreen)
    {
        // Set our window's level above the main menu window
        [_OSImplementation->window setLevel:NSMainMenuWindowLevel+1];
    }

    [_OSImplementation->window makeKeyAndOrderFront:nil];
	return true;
}

void ShellOS::releaseWindow()
{
	if(_OSImplementation->window)
	{
		[_OSImplementation->view release];
        _OSImplementation->view = nil;
	}
}

pvr::OSApplication ShellOS::getApplication() const
{
	return NULL;
}

pvr::OSConnection ShellOS::getConnection() const
{
	return nullptr;
}

pvr::OSDisplay ShellOS::getDisplay() const
{
	return NULL;
}

pvr::OSWindow ShellOS::getWindow() const
{
	return (__bridge OSWindow)_OSImplementation->view;
}

bool ShellOS::handleOSEvents()
{
	// Nothing to do
	return true;
}

bool ShellOS::isInitialized()
{
	return _OSImplementation && _OSImplementation->window;
}

bool ShellOS::popUpMessage(const char *const title, const char *const message, ...)const
{
    if(title && message)
    {
        va_list arg;
        
        va_start(arg, message);
        // NSString *fullMessage = [[[NSString alloc] initWithFormat:[NSString stringWithUTF8String:message] arguments:arg] autorelease];
        NSString *fullMessage = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:message] arguments:arg];
        va_end(arg);
        
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:fullMessage];
        [alert runModal];
    }
    
    return true;
}

static pvr::Keys mapNSKeyToPvrKey(unsigned short keyCode)
{
    switch(keyCode){
    
        case 0x31: // kVK_Space
            {
                return pvr::Keys::Space;
            }
            break;
    
        case 0x7B: // kVK_LeftArrow
            {
                return pvr::Keys::Left;
            }
            break;
        case 0x7C: // kVK_RightArrow
            {
                return pvr::Keys::Right;
            }
            break;
        case 0x7D: // kVK_DownArrow
            {
                return pvr::Keys::Down;
            }
            break;
        case 0x7E: // kVK_UpArrow
            {
                return pvr::Keys::Up;
            }
            break;
            
        case 0x35: // kVK_Escape
            {
                return pvr::Keys::Escape;
            }
            break;
        case 0x24:// kVK_Return
            {
                return pvr::Keys::Return;
            }
            break;
               
        case 0x53:// kVK_ANSI_Keypad1
            {
                return pvr::Keys::Key1;
            }
            break;
    }
    return pvr::Keys(0);
}

// CLASS IMPLEMENTATION
@implementation AppWindow

@synthesize eventQueue;

- (void)keyDown:(NSEvent*) event
{
    eventQueue->onKeyDown(mapNSKeyToPvrKey([event keyCode]));
}

- (void)keyUp:(NSEvent*) event
{
    eventQueue->onKeyUp(mapNSKeyToPvrKey([event keyCode]));
}

- (void)mouseDown:(NSEvent*)event
{
    switch([event type])
    {
        case NSLeftMouseDown:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceDown(0);
        }
        break;
        case NSRightMouseDown:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceDown(1);
        }
        break;
        default: break;
    }
}

- (void)mouseUp:(NSEvent*)event
{
    switch([event type])
    {
        case NSLeftMouseUp:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceUp(0);
        }
        break;
        case NSRightMouseUp:
        {
            NSPoint location = [event locationInWindow];
            g_cursorX = location.x;
            g_cursorY = location.y;
            eventQueue->onPointingDeviceUp(1);
        }
        break;
        default: break;
    }
}

- (void)mouseDragged:(NSEvent*)event
{
}

-(BOOL)canBecomeKeyWindow
{
    return YES;
}

@end
//!\endcond
