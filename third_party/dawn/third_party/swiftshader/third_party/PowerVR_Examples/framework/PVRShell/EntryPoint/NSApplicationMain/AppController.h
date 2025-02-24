/*!
\brief Class necessary for the entry point of the AppKit based implementation of PVRShell.
\file PVRShell/EntryPoint/NSApplicationMain/AppController.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#ifndef _APPCONTROLLER_H_
#define _APPCONTROLLER_H_

#include "PVRShell/StateMachine.h"
#include "PVRCore/commandline/CommandLine.h"
#import <AppKit/NSApplication.h>
#import <Foundation/NSTimer.h>

/// <summary>iOS entry point implementation</summary>
@interface AppController : NSObject<NSApplicationDelegate>
{
	NSTimer* mainLoopTimer; //!< timer for the main loop
	pvr::platform::StateMachine* stateMachine; //!< The State Machine powering the pvr::Shell
	pvr::platform::CommandLineParser commandLine; //!< Command line options passed on app launch
}

/// <summary>Terminates the application</summary>
- (void)terminateApp;

@end

#endif //_APPCONTROLLER_H_