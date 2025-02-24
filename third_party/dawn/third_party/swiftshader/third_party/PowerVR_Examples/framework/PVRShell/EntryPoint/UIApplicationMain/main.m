/*!
\brief Main implementation for the iOS UIKit
\file PVRShell/EntryPoint/UIApplicationMain/main.m
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#import <UIKit/UIKit.h>

/// <summary>Application entry point for the iOS UIKit. Forwards to UIApplicationMain</summary>
/// <param name="argc">Number of command line arguments</param>
/// <param name="argv">Array of command line arguments</param>
/// <returns>0 on no error, otherwise 1</returns>
int main(int argc, char *argv[])
{
	//NSAutoreleasePool* pool = [NSAutoreleasePool new];
	@autoreleasepool {
        UIApplicationMain(argc, argv, nil, @"AppController");
    }
	
	
//	[pool release];
	
	return 0;
}

