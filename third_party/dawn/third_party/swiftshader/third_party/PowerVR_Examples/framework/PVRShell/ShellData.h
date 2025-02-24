/*!
\brief Class containing internal data of the PowerVR Shell.
\file PVRShell/ShellData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/commandline/CommandLine.h"
#include "PVRCore/texture/PixelFormat.h"
#include "PVRCore/types/Types.h"
#include "PVRCore/Time_.h"

/*! This file simply defines a version std::string. It can be commented out. */
#include "sdkver.h"
#if !defined(PVRSDK_BUILD)
#define PVRSDK_BUILD "n.n@nnnnnnn"
#endif

/*! Define the txt file that we can load command-line options from. */
#if !defined(PVRSHELL_COMMANDLINE_TXT_FILE)
#define PVRSHELL_COMMANDLINE_TXT_FILE "PVRShellCL.txt"
#endif

namespace pvr {
namespace platform {
class ShellOS;

/// <summary>Internal. Contains and tracks internal data necessary to power the pvr::Shell.</summary>
struct ShellData
{
	Time timer; //!< A timer
	uint64_t timeAtInitApplication; //!< The time when initApplication is called
	uint64_t lastFrameTime; //!< The time take for the last frame
	uint64_t currentFrameTime; //!< The time taken for the current frame
	std::string exitMessage; //!< A message to print upon exitting the application

	ShellOS* os; //!< An abstraction of the OS specific shell
	DisplayAttributes attributes; //!< A set of display attributes

	CommandLineParser* commandLine; //!< A Command line parser

	int32_t captureFrameStart; //!< The frame at which to start capturing frames
	int32_t captureFrameStop; //!< The frame at which to stop capturing frames
	uint32_t captureFrameScale; //!< A scaling factor to apply to each captured frame

	bool trapPointerOnDrag; //!< Whether to trap the pointer when dragging
	bool forceFrameTime; //!< Indicates whether frame time animation should be used
	uint32_t fakeFrameTime; //!< The fake time used for each frame

	bool exiting; //!< Indicates that the application is exiting

	uint32_t frameNo; //!< The current frame number

	bool forceReleaseInitWindow; //!< Forces a release cycle to happen, calling ReleaseView and then recreating the window as well
	bool forceReleaseInitView; //!< Forces a release cycle to happen, calling ReleaseView. The window is not recreated
	int32_t dieAfterFrame; //!< Specifies a frame after which the application will exit
	float dieAfterTime; //!< Specifies a time after which the application will exit
	int64_t startTime; //!< Indicates the time at which the application was started

	bool outputInfo; //!< Indicates that the output information should be printed

	bool weAreDone; //!< Indicates that the application is finished

	float FPS; //!< The current frames per second
	bool showFPS; //!< Indicates whether the current fps should be printed

	Api contextType; //!< The API used
	Api minContextType; //!< The minimum API supported

	/// <summary>Default constructor.</summary>
	ShellData()
		: os(0), commandLine(0), captureFrameStart(-1), captureFrameStop(-1), captureFrameScale(1), trapPointerOnDrag(true), forceFrameTime(false), fakeFrameTime(16),
		  exiting(false), frameNo(0), forceReleaseInitWindow(false), forceReleaseInitView(false), dieAfterFrame(-1), dieAfterTime(-1), startTime(0), outputInfo(false),
		  weAreDone(false), FPS(0.0f), showFPS(false), contextType(Api::Unspecified), minContextType(Api::Unspecified), currentFrameTime(static_cast<uint64_t>(-1)),
		  lastFrameTime(static_cast<uint64_t>(-1)), timeAtInitApplication(static_cast<uint64_t>(-1)){};
};
} // namespace platform
} // namespace pvr
