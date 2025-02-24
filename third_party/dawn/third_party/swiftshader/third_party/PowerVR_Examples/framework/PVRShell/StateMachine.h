/*!
\brief The StateMachine controlling the PowerVR Shell.
\file PVRShell/StateMachine.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/OS/ShellOS.h"
namespace pvr {
namespace platform {
class Shell;

/// <summary>The StateMachine controlling the PowerVR Shell. Provides the application main loop and callbacks.</summary>
class StateMachine : public ShellOS
{
public:
	enum NewState
	{
		StateUninitialised, // Initial State
		//--v Init       --^ Exit
		StateInitialised, // Initialised. Need to initalise App.
		//--v InitApplication       --^ QuitApplication
		StateAppInitialised, // Initialised. Need to initalise App.
		//--v InitWindow            --^ ReleaseWindow
		StateWindowInitialised,
		//--v InitView            --^ ReleaseView
		StateReady,
	};

public:
	/// <summary>Constructor. Called by the application's entry point (main).</summary>
	/// <param name="instance">Platform-specific object containing pointer/s to the application instance.</param>
	/// <param name="commandLine">The command line arguments passed by the user.</param>
	/// <param name="osdata">Platform specific data passed in by the system</param>
	StateMachine(OSApplication instance, platform::CommandLineParser& commandLine, OSDATA osdata);

	~StateMachine();

	/// <summary>Called by the application's entry point (main).</summary>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result init();

	/// <summary>Called by the application's entry point (main).</summary>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result execute();

	/// <summary>Called internally by the state machine.</summary>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result executeNext();

	/// <summary>Called internally by the state machine. Executes all code paths between current state
	/// and the state requested (naturally reaching that state). For example, executeTo(QuitApplication)
	/// when current state is RenderFrame, will execute ReleaseView, ReleaseWindow, QuitApplication</summary>
	/// <param name="state">The state to execute</param>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result executeTo(NewState state);

	/// <summary>If the state machine is before (less initialised) than the parameter state, execute initialisation steps until that state is reached.
	/// Does nothing if it is after (more initialised) than the parameter state.</summary>
	/// <param name="state">The state to execute</param>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result executeUpTo(NewState state);

	/// <summary>If the state machine is after (more initialised) than the parameter state, execute teardown steps until that state is reached.
	/// Does nothing if it is before (less initialised) than the parameter state.</summary>
	/// <param name="state">The state to execute</param>
	/// <returns>pvr::Result::Success if successful, otherwise error code.</returns>
	Result executeDownTo(NewState state);

	/// <summary>Get the current state of the StateMachine.</summary>
	/// <returns>The current state of the StateMachine.</returns>
	NewState getState() const { return _currentState; }

	/// <summary>Check if the StateMachine is paused.</summary>
	/// <returns>True if the StateMachine is paused.</returns>
	bool isPaused() const { return _pause; }

	/// <summary>Pauses the state machine.</summary>
	void pause() { _pause = true; }

	/// <summary>Resumes (exits pause state for) the state machine.</summary>
	void resume() { _pause = false; }

private:
	Result executeInitApplication();
	Result executeQuitApplication();
	Result executeInitWindow();
	Result executeReleaseWindow();
	Result executeInitView();
	Result executeReleaseView();
	Result executeFrame();

	Result executeUp();
	Result executeDown();

	void preExit();

	void applyCommandLine();
	void readApiFromCommandLine();

	NewState _currentState;
	bool _pause;
};

inline std::string to_string(StateMachine::NewState state)
{
	static const std::string STATE_MACHINE_UNINITIALISED("STATE_MACHINE_UNINITIALISED");
	static const std::string STATE_MACHINE_INITIALISED("STATE_MACHINE_INITIALISED");
	static const std::string APP_INITIALISED("APP_INITIALISED");
	static const std::string WINDOW_INITIALISED("WINDOW_INITIALISED");
	static const std::string READY("READY");
	static const std::string UNKNOWN("UNKNOWN");
	switch (state)
	{
	case StateMachine::StateUninitialised: return STATE_MACHINE_UNINITIALISED;
	case StateMachine::StateInitialised: return STATE_MACHINE_INITIALISED;
	case StateMachine::StateAppInitialised: return APP_INITIALISED;
	case StateMachine::StateWindowInitialised: return WINDOW_INITIALISED;
	case StateMachine::StateReady: return READY;
	default: return UNKNOWN;
	}
}

} // namespace platform
} // namespace pvr
