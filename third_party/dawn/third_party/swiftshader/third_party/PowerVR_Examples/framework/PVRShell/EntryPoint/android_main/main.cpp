/*!
\brief Entry point for Android systems (android_main).
\file PVRShell/EntryPoint/android_main/main.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/StateMachine.h"
#include "PVRCore/commandline/CommandLine.h"
#include "PVRCore/Log.h"
#include <android_native_app_glue.h>

const char* to_cmd_string(int32_t cmd)
{
#define case_statement_stringify(WORD) \
	case WORD: return #WORD;
	switch (cmd)
	{
		case_statement_stringify(APP_CMD_INPUT_CHANGED);
		case_statement_stringify(APP_CMD_INIT_WINDOW);
		case_statement_stringify(APP_CMD_TERM_WINDOW);
		case_statement_stringify(APP_CMD_WINDOW_RESIZED);
		case_statement_stringify(APP_CMD_WINDOW_REDRAW_NEEDED);
		case_statement_stringify(APP_CMD_CONTENT_RECT_CHANGED);
		case_statement_stringify(APP_CMD_GAINED_FOCUS);
		case_statement_stringify(APP_CMD_LOST_FOCUS);
		case_statement_stringify(APP_CMD_CONFIG_CHANGED);
		case_statement_stringify(APP_CMD_LOW_MEMORY);
		case_statement_stringify(APP_CMD_START);
		case_statement_stringify(APP_CMD_RESUME);
		case_statement_stringify(APP_CMD_SAVE_STATE);
		case_statement_stringify(APP_CMD_PAUSE);
		case_statement_stringify(APP_CMD_STOP);
		case_statement_stringify(APP_CMD_DESTROY);
	}
	return "UNKNOWN";
#undef case_statement_stringify
}

static void checkState(android_app* app, pvr::platform::StateMachine* stateMachine, bool& has_error, bool windowInitialised, bool started, bool paused)
{
#ifdef DEBUG
	Log(LogLevel::Debug, "CheckState: - started:%s - windowInitialised:%s - paused:%s", started ? "true" : "false", windowInitialised ? "true" : "false", paused ? "true" : "false");
#endif
	pvr::Result result = pvr::Result::Success;
	pvr::Result resulttmp = pvr::Result::Success; // Ensure, that if winding down, we do not actually overwrite a failed result.
	// clang-format off
	using pvr::platform::StateMachine;
	if (started)                                 { 	resulttmp = stateMachine->executeUpTo(StateMachine::StateAppInitialised); }
	if (resulttmp != pvr::Result::Success && result == pvr::Result::Success) { result = resulttmp; started = false; paused = true; windowInitialised = false; has_error = true; }
	
	//if (started && windowInitialised)            { result = stateMachine->executeUpTo(StateMachine::StateWindowInitialised); }
	
	if (started && windowInitialised && !paused) { resulttmp = stateMachine->executeUpTo(StateMachine::StateReady); }
	if (resulttmp != pvr::Result::Success && result == pvr::Result::Success) { result = resulttmp; started = false; paused = true; windowInitialised = false; has_error = true; }
	
	if (!windowInitialised || paused)            { resulttmp = stateMachine->executeDownTo(StateMachine::StateAppInitialised); }
	if (resulttmp != pvr::Result::Success && result == pvr::Result::Success) {  result = resulttmp; started = false; paused = true; windowInitialised = false; has_error = true; }
	
	if (!started)                                { resulttmp = stateMachine->executeDownTo(StateMachine::StateInitialised); }
	if (resulttmp != pvr::Result::Success && result == pvr::Result::Success) {  result = resulttmp; started = false; paused = true; windowInitialised = false; has_error = true; }
	
	if (result != pvr::Result::Success) { has_error = true; Log(LogLevel::Debug, "checkState: Requesting Native Activity finish."); ANativeActivity_finish(app->activity); }
	// clang-format on
}

/// <summary>A function to handle the android OS system messages. Used for lifecycle management.</summary>
/// <param name="app">The android application.</param>
/// <param name="cmd">The command type (android predetermined).</param>
static void handle_cmd(struct android_app* app, int32_t cmd)
{
#ifdef DEBUG
	Log(LogLevel::Debug, "[MAIN]: handle_cmd %s !", to_cmd_string(cmd));
#endif
	pvr::platform::StateMachine* stateMachinePtr = static_cast<pvr::platform::StateMachine*>(app->userData);

	static bool windowInitialised = false;
	static bool started = false;
	static bool paused = true;
	static bool has_error = false;

	if (has_error)
	{
		started = false;
		paused = true;
		windowInitialised = false;
	}
	switch (cmd)
	{
	case APP_CMD_START:
		started = true;
		checkState(app, stateMachinePtr, has_error, windowInitialised, started, paused);
		break;
	case APP_CMD_PAUSE:
	case APP_CMD_LOST_FOCUS:
		paused = true;
		checkState(app, stateMachinePtr, has_error, windowInitialised, started, paused);
		break;
	case APP_CMD_RESUME:
	case APP_CMD_GAINED_FOCUS:
		paused = false;
		checkState(app, stateMachinePtr, has_error, windowInitialised, started, paused);
		break;
	case APP_CMD_WINDOW_RESIZED:
		if (windowInitialised && !paused) // will only cause release/init if everything was ready - otherwise it will happen naturally
		{
			checkState(app, stateMachinePtr, has_error, false, started, paused);
			if (!has_error) (checkState(app, stateMachinePtr, has_error, true, started, paused));
		}
		break;
	case APP_CMD_WINDOW_REDRAW_NEEDED: break;
	case APP_CMD_CONTENT_RECT_CHANGED:
		if (windowInitialised && !paused) // will only cause release/init if everything was ready - otherwise it will happen naturally
		{
			checkState(app, stateMachinePtr, has_error, false, started, paused);
			if (!has_error) (checkState(app, stateMachinePtr, has_error, true, started, paused));
		}
		break;
	case APP_CMD_INIT_WINDOW:
		windowInitialised = true;
		checkState(app, stateMachinePtr, has_error, windowInitialised, started, paused);
		break;
	case APP_CMD_TERM_WINDOW:
	case APP_CMD_STOP:
		windowInitialised = false;
		checkState(app, stateMachinePtr, has_error, windowInitialised, started, paused);
		break;
	case APP_CMD_DESTROY:
		started = false;
		windowInitialised = false;
		checkState(app, stateMachinePtr, has_error, windowInitialised, started, paused);
		break;
	default: return;
	};
}

/// <summary>Main function: Entry point for the Android platform</summary>
/// <param name="state">the android app state</param>
/// <remarks>This Main function is the Entry point for a NativeActivity- style android NDK main app</remarks>
void android_main(struct android_app* state)
{
	// Make sure glue isn't stripped.
	pvr::platform::CommandLineParser commandLine;

	{
		// Handle command-line
		/*
		How to launch an app from an adb shell with command-line options, e.g.

			am start -a android.intent.action.MAIN -n com.powervr.OGLESIntroducingPOD/.OGLESIntroducingPOD --es args "-info"
		*/
		ANativeActivity* activity = state->activity;

		JNIEnv* env;
		activity->vm->AttachCurrentThread(&env, 0);

		jobject me = activity->clazz;

		jclass acl = env->GetObjectClass(me); // class pointer of NativeActivity
		jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");

		jobject intent = env->CallObjectMethod(me, giid); // Got our intent
		jclass icl = env->GetObjectClass(intent); // class pointer of Intent
		jmethodID gseid = env->GetMethodID(icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

		jstring jsArgs = (jstring)env->CallObjectMethod(intent, gseid, env->NewStringUTF("args"));

		if (jsArgs != NULL)
		{
			const char* args = env->GetStringUTFChars(jsArgs, 0);

			if (args != NULL)
			{
				commandLine.set(args);

				// Tidy up the args std::string
				env->ReleaseStringUTFChars(jsArgs, args);
			}
		}

		activity->vm->DetachCurrentThread();
	}

	Log(LogLevel::Debug, "MAIN: Initializing state machine.");
	pvr::platform::StateMachine stateMachine(state, commandLine, NULL);
	if (stateMachine.init() != pvr::Result::Success)
	{
		Log(LogLevel::Error, "MAIN: Failed to initialise the StateMachine. Exiting application.");
		return;
	}

	state->userData = &stateMachine;
	state->onAppCmd = &handle_cmd;

	// Handle our events until we have a valid window or destroy has been requested
	int events;
	struct android_poll_source* source;

	Log(LogLevel::Debug, "MAIN: Entering main loop state machine.");

	//	Initialize our window/run/shutdown
	while (true)
	{
		while (ALooper_pollAll((state->destroyRequested || (stateMachine.getState() == pvr::platform::StateMachine::StateReady && !stateMachine.isPaused())) ? 0 : 10000, NULL,
				   &events, (void**)&source) >= 0)
		{
			// Process this event.
			if (source != NULL) { source->process(state, source); }
			else
			{
				Log(LogLevel::Debug, "[MAIN]: Event handling: Null Source.");
			}
		}

		// Check if we are exiting.
		if (state->destroyRequested != 0)
		{
			Log(LogLevel::Debug, "[MAIN]: android_main - Destroy requested. Deinitialising application.");
			stateMachine.executeDownTo(pvr::platform::StateMachine::StateInitialised);
			Log(LogLevel::Debug, "[MAIN]: android_main - App deinitialise complete. Exiting native thread.");
			return;
		}

		// Avoid infinite loop:
		// Render our scene
		if (stateMachine.getState() == pvr::platform::StateMachine::StateReady && !stateMachine.isPaused())
		{
			// Log(LogLevel::Debug, "[MAIN]: Executing main loop render frame.");
			pvr::Result result;
			do
			{
				result = stateMachine.executeNext();
			} while (result == pvr::Result::Success && stateMachine.getState() != pvr::platform::StateMachine::StateReady);

			if (result != pvr::Result::Success)
			{
				if (result != pvr::Result::ExitRenderFrame) { Log(LogLevel::Debug, "[MAIN]: android_main - RenderFrame execution result was %d. Requesting app finish.", result); }
				else
				{
					Log(LogLevel::Debug, "[MAIN]: android_main - ExitRenderFrame requested...");
				}
				stateMachine.pause();
				ANativeActivity_finish(state->activity);
			}
		}
		else
		{
			Log(LogLevel::Debug, "[MAIN]: android_main - Skipped execution. Current state is: %d", stateMachine.getState());
		}
	}
	Log(LogLevel::Debug, "[MAIN]: android_main - MAIN RETURNING - Ndk thread exiting.");
}
