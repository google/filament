/*!
\brief Implementation for the AppKit version of AppController
\file PVRShell/EntryPoint/NSApplicationMain/AppController.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#import "AppController.h"
#import <AppKit/NSAlert.h>
#import <Foundation/NSString.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSArray.h>

//!\cond NO_DOXYGEN
const int kFPS = 60.0;
//!\endcond

// CLASS IMPLEMENTATION
@implementation AppController

- (void) mainLoop
{
	if(stateMachine->executeNext() != pvr::Result::Success)
	{
		[self terminateApp];
	}
}

- (void) applicationDidFinishLaunching:(NSNotification *)notification
{
	// Parse the command-line
	NSMutableString *cl = [[NSMutableString alloc] init];
	NSArray *args = [[NSProcessInfo processInfo] arguments];

	for(NSUInteger i = 1;i < [args count]; ++i)
	{
		[cl appendString:[args objectAtIndex:i]];
		[cl appendString:@" "];
	}

	commandLine.set([cl UTF8String]);
	//[cl release];

	stateMachine = new pvr::platform::StateMachine((__bridge pvr::OSApplication)self, commandLine, NULL);

	if(!stateMachine)
	{
		NSLog(@"Failed to allocate stateMachine.\n");
		[self terminateApp];
	}

	if(stateMachine->init() != pvr::Result::Success)
	{
		NSLog(@"Failed to initialize stateMachine.\n");
		[self terminateApp];
	}

	mainLoopTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / kFPS) target:self selector:@selector(mainLoop) userInfo:nil repeats:YES];	
}

- (void) applicationWillTerminate:(NSNotification *)notification
{
	[mainLoopTimer invalidate];
	mainLoopTimer = nil;

	stateMachine->executeDownTo(pvr::platform::StateMachine::StateInitialised);

	delete stateMachine;
	stateMachine = NULL;
}

- (void) terminateApp
{
	[NSApp terminate:nil];
}

@end
