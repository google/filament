/*!
\brief UIApplication delegate functioning as the application controller for UIKit implementation of Shell(iOS)
\file PVRShell/EntryPoint/UIApplicationMain/AppController.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRShell/StateMachine.h"
#include "PVRCore/commandline/CommandLine.h"

#import <UIKit/UIApplication.h>
#import <Foundation/NSTimer.h>

@interface AppController : NSObject<UIApplicationDelegate>
{
	NSTimer* mainLoopTimer; //!< timer for the main loop
	pvr::platform::StateMachine* stateMachine; //!< The StateMachine powering the pvr::Shell
	pvr::platform::CommandLineParser commandLine; //!< The command line options passed on app launch
}

@end