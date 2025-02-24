/*!
\brief Generic entry point, normally used for Linux based systems.
\file PVRShell/EntryPoint/main/main.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/StateMachine.h"
#include "PVRCore/commandline/CommandLine.h"

/// <summary>The entry point for "typical" operating systems (e.g. linux based).
/// Follows C/C++ standards.
/// <param name="argc">The number of command line arguments passed + 1</param>
/// <param name="argv">0: The executable name  1+:The command line arguments</param>
/// <returns>0 on no error, otherwise 1</returns>
int main(int argc, char** argv)
{
	pvr::platform::CommandLineParser commandLine;
	commandLine.set((argc - 1), &argv[1]);

	pvr::platform::StateMachine stateMachine(nullptr, commandLine, nullptr);

	stateMachine.init();

	// Main loop of the application.
	return (stateMachine.execute() == pvr::Result::Success) ? 0 : 1;
}