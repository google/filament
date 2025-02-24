/*!
\brief Implementation of the ShellOS class for the UIKit
\file PVRShell/OS/UIKit/ShellOS.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/stream/FilePath.h"
#include <mach/mach_time.h>
#import <UIKit/UIKit.h>

@interface AppWindow : UIWindow // The implementation appears at the bottom of this file
{
	pvr::Shell* eventQueue;
	CGFloat screenScale;
}

@property (assign) pvr::Shell * eventQueue;
@property (assign) CGFloat screenScale;
@end
namespace pvr{
namespace platform{
class InternalOS
{
public:
	AppWindow* window;

	InternalOS() : window(nil)
	{
	}
};
}
}
int16_t g_cursorX, g_cursorY;

namespace pvr{
namespace platform{
// Setup the capabilities
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(/*NSObject<NSApplicationDelegate>*/void* hInstance, OSDATA osdata) : _instance(hInstance)
{
	_OSImplementation = std::make_unique<InternalOS>();
}

ShellOS::~ShellOS() {}

void ShellOS::updatePointingDeviceLocation()
{
	_shell->updatePointerPosition(PointerLocation(g_cursorX, g_cursorY));
}

bool ShellOS::init(DisplayAttributes &data)
{
	if(!_OSImplementation)
		return false;

	// Setup read and write paths
	NSString* readPath = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] resourcePath], @"/"];
	_readPaths.push_back([readPath UTF8String]);

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* writePath = [NSString stringWithFormat:@"%@%@", [paths objectAtIndex:0], @"/"];
	_writePath = [writePath UTF8String];
	
	// Setup the app name
	NSString* name = [[NSProcessInfo processInfo] processName];
	_appName =[name UTF8String];
	
	return true;
}

bool ShellOS::initializeWindow(DisplayAttributes &data)
{
	CGFloat scale = 1.0;
	
	// Now create our window
	data.fullscreen = true;
 
	UIScreen* screen = [UIScreen mainScreen];
	if([UIScreen instancesRespondToSelector:@selector(scale)])
	{
		scale = [screen scale];
	}
	
	// Set our frame to fill the screen
	CGRect frame = [screen applicationFrame];

	data.x = frame.origin.x;
	data.y = frame.origin.y;
	data.width = frame.size.width * scale;
	data.height = frame.size.height * scale;

	_OSImplementation->window = [[AppWindow alloc] initWithFrame:frame];
	
	if(!_OSImplementation->window) {  return false;   }
	
	// pass the shell as the event queue
	_OSImplementation->window.eventQueue = _shell.get();
	
	// Give the window a copy of the eventQueue so it can pass on the keyboard/mouse events
	[_OSImplementation->window setScreenScale:scale];
	[_OSImplementation->window makeKeyAndVisible];
	
	return true;
}

void ShellOS::releaseWindow()
{
	if(_OSImplementation->window)
	{
	//	[_OSImplementation->window release];
		_OSImplementation->window = nil;
	}
}

OSApplication ShellOS::getApplication() const
{
	return NULL;
}

OSConnection ShellOS::getConnection() const
{
    return nullptr;
}

OSDisplay ShellOS::getDisplay() const
{
	return NULL;
}

OSWindow ShellOS::getWindow() const
{
	return (__bridge OSWindow)_OSImplementation->window;
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

bool ShellOS::popUpMessage(const char * const title, const char * const message, ...) const
{
	if(title && message)
	{
		va_list arg;

		va_start(arg, message);

		NSString *fullMessage = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:message] arguments:arg];
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:title]
													message:fullMessage
													delegate:nil
													cancelButtonTitle:@"OK"
													otherButtonTitles:nil];
		
		va_end(arg);
		
		[alert show];
	  //  [alert release];
	}
	
	return true;
}
}
}


// CLASS IMPLEMENTATION
@implementation AppWindow

@synthesize eventQueue;
@synthesize screenScale;


- (void)sendEvent:(UIEvent *)event 
{
	if(event.type == UIEventTypeTouches) 
	{
		for(UITouch * t in [event allTouches]) 
		{
			switch(t.phase)
			{
				case UITouchPhaseBegan:
				{
					CGPoint location = [t locationInView:self];
					g_cursorX =(location.x * screenScale);
					g_cursorY = (location.y) * screenScale;
					if(eventQueue->isScreenRotated() && eventQueue->isFullScreen())
					{
						std::swap(g_cursorX,g_cursorY);
					}
					eventQueue->onPointingDeviceDown(0);
					
				
				}
				break;
				case UITouchPhaseMoved:
				{
				}
				break;
				case UITouchPhaseEnded:
				case UITouchPhaseCancelled:
				{
					CGPoint location = [t locationInView:self];
				   
					g_cursorX =(location.x * screenScale);
					g_cursorY =(location.y * screenScale);
					if(eventQueue->isScreenRotated() && eventQueue->isFullScreen())
					{
						std::swap(g_cursorX,g_cursorY);
					}
					eventQueue->onPointingDeviceUp(0);
					
				}
				break;
			}
		}
	}

	[super sendEvent:event];
}

@end
//!\endcond
