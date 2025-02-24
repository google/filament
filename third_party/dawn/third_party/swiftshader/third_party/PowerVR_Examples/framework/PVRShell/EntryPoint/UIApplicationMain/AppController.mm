/*!
\brief Implementation of the AppController for the UIKit based (iOS) implementation of PVRShell.
\file PVRShell/EntryPoint/UIApplicationMain/AppController.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#import "AppController.h"

//!\cond NO_DOXYGEN
const int kFPS = 60.0;
//!\endcond

// CLASS IMPLEMENTATION
@implementation AppController

- (void) mainLoop
{
	if(stateMachine->executeNext() != pvr::Result::Success)
	{
		[mainLoopTimer invalidate];
		mainLoopTimer = nil;
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
	
	stateMachine = new pvr::platform::StateMachine((__bridge pvr::OSApplication)self, commandLine,NULL);

	if(!stateMachine)
	{
		NSLog(@"Failed to allocate stateMachine.\n");
		return;
	}
	
	if(stateMachine->init() != pvr::Result::Success)
	{
		NSLog(@"Failed to initialize stateMachine.\n");
		delete stateMachine;
		stateMachine = NULL;
		return;
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

@end
