/*!
\brief Implementation of the StateMachine class powering the pvr::Shell.
\file PVRShell/StateMachine.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/StateMachine.h"
#include "PVRShell/Shell.h"
#include "PVRCore/stream/FileStream.h"
#include "PVRCore/Log.h"
#include "PVRCore/Time_.h"
#include <map>
#include <cstdlib>
#include <cmath>
#include <sstream>
#pragma warning(disable : 4127)

#if defined(_WIN32)
#define strcasecmp(a, b) _stricmp(a, b)
#endif

using std::string;

namespace pvr {
namespace platform {

StateMachine::StateMachine(OSApplication instance, platform::CommandLineParser& commandLine, OSDATA osdata)
	: ShellOS(instance, osdata), _currentState(StateUninitialised), _pause(false)
{
	_shellData.os = this;
	_shellData.commandLine = &commandLine;
}

StateMachine::~StateMachine() { LogClose(); }

void StateMachine::readApiFromCommandLine()
{
	const char *arg, *val;
	const platform::CommandLineParser::ParsedCommandLine& options = _shellData.commandLine->getParsedCommandLine();

	for (uint32_t i = 0; i < options.getOptionsList().size(); ++i)
	{
		arg = options.getOptionsList()[i].arg;
		val = options.getOptionsList()[i].val;

		if (!arg) { continue; }

		if (strcasecmp(arg, "-apiversion") == 0)
		{
			if (!strcasecmp(val, "vulkan"))
			{
				_shellData.minContextType = Api::Vulkan;
				_shellData.contextType = Api::Vulkan;
			}
			else if (!strcasecmp(val, "ogles31") || !strcasecmp(val, "gles31") || !strcasecmp(val, "gl31") || !strcasecmp(val, "es31"))
			{
				_shellData.minContextType = Api::OpenGLES31;
				_shellData.contextType = Api::OpenGLES31;
			}
			else if (!strcasecmp(val, "ogles3") || !strcasecmp(val, "gles3") || !strcasecmp(val, "gl3") || !strcasecmp(val, "es3"))
			{
				_shellData.minContextType = Api::OpenGLES3;
				_shellData.contextType = Api::OpenGLES3;
			}
			else if (!strcasecmp(val, "ogles2") || !strcasecmp(val, "gles2") || !strcasecmp(val, "gl2") || !strcasecmp(val, "es2"))
			{
				_shellData.minContextType = Api::OpenGLES2;
				_shellData.contextType = Api::OpenGLES2;
			}
			else
			{
				Log(LogLevel::Error, "Unrecognized command line value '%s' for command line argument '-apiversion'", val);
			}
		}
		else if (strcasecmp(arg, "-minapiversion") == 0 || strcasecmp(arg, "-minapi") == 0)
		{
			if (!strcasecmp(val, "vulkan")) { _shellData.minContextType = Api::Vulkan; }
			else if (!strcasecmp(val, "ogles31") || !strcasecmp(val, "gles31") || !strcasecmp(val, "gl31") || !strcasecmp(val, "es31"))
			{
				_shellData.minContextType = Api::OpenGLES31;
			}
			else if (!strcasecmp(val, "ogles3") || !strcasecmp(val, "gles3") || !strcasecmp(val, "gl3") || !strcasecmp(val, "es3"))
			{
				_shellData.minContextType = Api::OpenGLES3;
			}
			else if (!strcasecmp(val, "ogles2") || !strcasecmp(val, "gles2") || !strcasecmp(val, "gl2") || !strcasecmp(val, "es2"))
			{
				_shellData.minContextType = Api::OpenGLES2;
			}
			else
			{
				Log(LogLevel::Error, "Unrecognized command line value '%s' for command line argument '-minapiversion'", val);
			}
		}
	}
}

// == Functions to call for corresponding command line parameters ==
namespace {
typedef void (*SetShellParameterPtr)(Shell& shell, const char* arg, const char* val);
#define WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val) \
	if (!val) \
	{ \
		Log(LogLevel::Warning, "PVRShell recognised command-line option '%s' is supported, but no parameter has been provided.", arg); \
		return; \
	}
#define WARNING_UNSUPPORTED_OPTION(x) \
	{ \
		Log(LogLevel::Warning, "PVRShell recognised command-line option '" x "' is unsupported by PVRShell and has been ignored."); \
	}
void setWidth(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setDimensions(static_cast<uint32_t>(atoi(val)), shell.getHeight());
}
void setHeight(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setDimensions(shell.getWidth(), static_cast<uint32_t>(atoi(val)));
}
void setAASamples(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setAASamples(static_cast<uint32_t>(atoi(val)));
}
void setFullScreen(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setFullscreen(static_cast<uint32_t>(atoi(val)) != 0);
}
void setQuitAfterFrame(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setQuitAfterFrame(static_cast<uint32_t>(atoi(val)));
}
void setQuitAfterTime(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setQuitAfterTime(static_cast<float>(atof(val)));
}
void setPosx(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	if (shell.setPosition(static_cast<uint32_t>(atoi(val)), shell.getPositionY()) != Result::Success) { WARNING_UNSUPPORTED_OPTION("posx") }
}
void setPosy(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	if (shell.setPosition(shell.getPositionX(), static_cast<uint32_t>(atoi(val))) != Result::Success) { WARNING_UNSUPPORTED_OPTION("posy") }
}
void setSwapLength(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setPreferredSwapChainLength(static_cast<uint32_t>(atoi(val)));
}
void setVsync(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	if (!strcasecmp(val, "on"))
	{
		shell.setVsyncMode(VsyncMode::On);
		Log("On");
	}
	else if (!strcasecmp(val, "off"))
	{
		shell.setVsyncMode(VsyncMode::Off);
		Log("Off");
	}
	else if (!strcasecmp(val, "relaxed"))
	{
		shell.setVsyncMode(VsyncMode::Relaxed);
		Log("Relaxed");
	}
	else if (!strcasecmp(val, "mailbox"))
	{
		shell.setVsyncMode(VsyncMode::Mailbox);
		Log("Mailbox");
	}
	else if (!strcasecmp(val, "half"))
	{
		shell.setVsyncMode(VsyncMode::Half);
		Log("Half");
	}
	else
	{
		Log("Trying number");
		char* converted;
		int value = strtol(val, &converted, 0);
		if (!*converted)
		{
			switch (value)
			{
			case 0: shell.setVsyncMode(VsyncMode::Off); break;
			case 1: shell.setVsyncMode(VsyncMode::On); break;
			case 2: shell.setVsyncMode(VsyncMode::Half); break;
			case -1: shell.setVsyncMode(VsyncMode::Relaxed); break;
			case -2: shell.setVsyncMode(VsyncMode::Mailbox); break;
			default: break;
			}
		}
	}
	Log("%d", shell.getVsyncMode());
}
void setLogLevel(Shell& shell, const char* arg, const char* val)
{
	(void)(shell);
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	if (!strcasecmp(val, "critical")) { DefaultLogger().setVerbosity(LogLevel::Critical); }
	else if (!strcasecmp(val, "error"))
	{
		DefaultLogger().setVerbosity(LogLevel::Error);
	}
	else if (!strcasecmp(val, "warning"))
	{
		DefaultLogger().setVerbosity(LogLevel::Warning);
	}
	else if (!strcasecmp(val, "information"))
	{
		DefaultLogger().setVerbosity(LogLevel::Information);
	}
	else if (!strcasecmp(val, "info"))
	{
		DefaultLogger().setVerbosity(LogLevel::Information);
	}
	else if (!strcasecmp(val, "verbose"))
	{
		DefaultLogger().setVerbosity(LogLevel::Verbose);
	}
	else if (!strcasecmp(val, "debug"))
	{
		DefaultLogger().setVerbosity(LogLevel::Debug);
	}
	else
	{
		Log(LogLevel::Warning,
			"Unrecognized threshold '%s' for '-loglevel' command "
			"line parameter. Accepted values: [critical, error, warning, "
			"information(default for release build), debug(default for debug build), verbose");
	}
}
void setColorBpp(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	switch (atoi(val))
	{
	case 16: shell.setColorBitsPerPixel(5, 6, 5, 0); break;
	case 24: shell.setColorBitsPerPixel(8, 8, 8, 0); break;
	case 32: shell.setColorBitsPerPixel(8, 8, 8, 8); break;
	default: Log(LogLevel::Warning, "PVRShell recognised command-line option 'set color bpp' set to unsupported value %hs. Supported values are (16, 24 and 32).", val); break;
	}
}
void setDepthBpp(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setDepthBitsPerPixel(static_cast<uint32_t>(atoi(val)));
}
void setStencilBpp(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setStencilBitsPerPixel(static_cast<uint32_t>(atoi(val)));
}
void setCaptureFrames(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	const char* pDash = strchr(val, '-');
	uint32_t start = static_cast<uint32_t>(atoi(val));
	uint32_t stop = (pDash ? static_cast<uint32_t>(atoi(pDash + 1)) : start);
	shell.setCaptureFrames(start, stop);
}
void setScreenshotScale(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setCaptureFrameScale(static_cast<uint32_t>(atoi(val)));
}
void setContextPriority(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setContextPriority(static_cast<uint32_t>(atoi(val)));
}
void setDesiredCconfigId(Shell& shell, const char* arg, const char* val)
{
	WARN_AND_QUIT_IF_PARAMETER_NOT_PROVIDED(arg, val);
	shell.setDesiredConfig(static_cast<uint32_t>(atoi(val)));
}
void setForceFrameTime(Shell& shell, const char* arg, const char* val)
{
	(void)(arg);
	shell.setForceFrameTime(true);
	if (val)
	{
		int value = atoi(val);
		if (value) { shell.setFakeFrameTime(std::max(1u, static_cast<uint32_t>(value))); }
	}
}
void showVersion(Shell& shell, const char* /*arg*/, const char* /*val*/) { Log(LogLevel::Information, "Version: '%hs'", shell.getSDKVersion()); }
void setShowFps(Shell& shell, const char* /*arg*/, const char* /*val*/) { shell.setShowFPS(true); }
void showInfo(Shell& shell, const char* /*arg*/, const char* /*val*/) { shell.getOS()._shellData.outputInfo = true; }
void showCommandLineOptions(Shell& shell, const char* arg, const char* val);
} // namespace

#undef WARNING_UNSUPPORTED_OPTION

const std::map<std::string, SetShellParameterPtr> supportedCommandLineOptions{ std::make_pair("-width", &setWidth), std::make_pair("-height", &setHeight),
	std::make_pair("-aasamples", &setAASamples), std::make_pair("-fullscreen", &setFullScreen), std::make_pair("-quitafterframe", &setQuitAfterFrame),
	std::make_pair("-qaf", &setQuitAfterFrame), std::make_pair("-quitaftertime", &setQuitAfterTime), std::make_pair("-qat", &setQuitAfterTime), std::make_pair("-posx", &setPosx),
	std::make_pair("-posy", &setPosy), std::make_pair("-swaplength", &setSwapLength), std::make_pair("-preferredswaplength", &setSwapLength), std::make_pair("-vsync", &setVsync),
	std::make_pair("-loglevel", &setLogLevel), std::make_pair("-colorbpp", &setColorBpp), std::make_pair("-colourbpp", &setColorBpp), std::make_pair("-cbpp", &setColorBpp),
	std::make_pair("-depthbpp", &setDepthBpp), std::make_pair("-dbpp", &setDepthBpp), std::make_pair("-stencilbpp", &setStencilBpp), std::make_pair("-dbpp", &setStencilBpp),
	std::make_pair("-c", &setCaptureFrames), std::make_pair("-screenshotscale", &setScreenshotScale), std::make_pair("-priority", &setContextPriority),
	std::make_pair("-config", &setDesiredCconfigId), std::make_pair("-forceframetime", &setForceFrameTime), std::make_pair("-fft", &setForceFrameTime),
	std::make_pair("-version", &showVersion), std::make_pair("-fps", &setShowFps), std::make_pair("-info", &showInfo), std::make_pair("-h", &showCommandLineOptions),
	std::make_pair("-help", &showCommandLineOptions), std::make_pair("--help", &showCommandLineOptions) };

namespace {
void showCommandLineOptions(Shell& /*shell*/, const char* /*arg*/, const char* /*val*/)
{
	std::stringstream sstream;
	sstream << "Supported Command-line options:";
	auto it = supportedCommandLineOptions.begin(), end = supportedCommandLineOptions.end();
	sstream << it->first;
	++it;
	for (; it != end; ++it) { sstream << ", " << it->first; }
	Log(LogLevel::Information, "%s", sstream.str().c_str());
}
} // namespace

Result StateMachine::init()
{
	if (ShellOS::init(_shellData.attributes))
	{
		// Check for the existence of PVRShellCL.txt and load from it if it exists
		std::string filepath;

		for (size_t i = 0; i < ShellOS::getReadPaths().size(); ++i)
		{
			filepath = ShellOS::getReadPaths()[i] + PVRSHELL_COMMANDLINE_TXT_FILE;
			FileStream file(filepath, "r", false);

			if (file.isReadable())
			{
				auto str = file.readString();
				_shellData.commandLine->prefix(str.c_str());
				Log(LogLevel::Information, ("Command-line options have been loaded from file " + filepath).c_str());
				break;
			}
		}

		// Build a window title using the application name and version of the SDK
		_shellData.attributes.windowTitle = getApplicationName() + " - Build " + std::string(Shell::getSDKVersion());

		// setup our state
		_currentState = StateInitialised;
		return Result::Success;
	}

	return Result::InitializationError;
}
void StateMachine::applyCommandLine()
{
#define WARNING_UNKNOWN_OPTION(x) \
	{ \
		Log(LogLevel::Warning, "PVRShell recognised command-line option '%s' is unsupported by PVRShell and has been ignored.", x); \
	}
	const char *arg, *val;
	const platform::CommandLineParser::ParsedCommandLine& options = _shellData.commandLine->getParsedCommandLine();

	bool has_unknown_options = false;
	for (uint32_t i = 0; i < options.getOptionsList().size(); ++i)
	{
		arg = options.getOptionsList()[i].arg;
		val = options.getOptionsList()[i].val;
		if (!arg) { continue; }
		auto it = supportedCommandLineOptions.find(arg);
		if (it != supportedCommandLineOptions.end()) { it->second(*_shell, arg, val); }
		else
		{
			has_unknown_options = true;
			WARNING_UNKNOWN_OPTION(arg);
		}
#undef WARNING_UNKNOWN_OPTION
	}
	if (has_unknown_options) { showCommandLineOptions(*_shell, "-help", nullptr); }
}

Result StateMachine::execute()
{
	Result finalresult = pvr::Result::Success;
	if (_currentState != StateInitialised) { Log(LogLevel::Warning, "The state machine was not in its initialised state when execute was called."); }
	for (;;)
	{
		Result result;
		result = executeNext();
		if (result != pvr::Result::Success && finalresult == pvr::Result::Success) { finalresult = result; }
		if (_currentState == StateInitialised) { return finalresult; }
	}
}

Result StateMachine::executeTo(NewState state)
{
	Result result = Result::Success;

	NewState initialCurrentState = _currentState;

	while ((result == Result::Success) && state > _currentState && state != StateReady) { result = executeUp(); }
	while ((result == Result::Success) && state < _currentState && state != StateReady) { result = executeDown(); }
	if ((result == Result::Success) && state == StateReady) { result = executeUp(); }

	if (result != Result::Success)
	{
		Log(LogLevel::Debug, "StateMachine::executeTo %s from %s (current state: %s) exits with Error %s.", to_string(state).c_str(), to_string(initialCurrentState).c_str(),
			to_string(_currentState).c_str(), getResultCodeString(result));
	}

	return result;
}

Result StateMachine::executeUpTo(NewState state)
{
	if (state > _currentState) return executeTo(state);
// IFDEF to avoid to_string being called.
#ifdef DEBUG
	Log(LogLevel::Debug, "StateMachine::executeUpTo skipped as requested state (%s) is not later current state (%s)", to_string(state).c_str(), to_string(_currentState).c_str());
#endif
	return Result::Success;
}

Result StateMachine::executeDownTo(NewState state)
{
	if (state < _currentState) return executeTo(state);
#ifdef DEBUG
	Log(LogLevel::Debug, "StateMachine::executeDownTo skipped as requested state (%s) is not earlier than current state (%s)", to_string(state).c_str(),
		to_string(_currentState).c_str());
#endif
	return Result::Success;
}

Result StateMachine::executeInitApplication()
{
	Log(LogLevel::Debug, "StateMachine::executeInitApplication executing");
	if (_shellData.commandLine->getParsedCommandLine().hasOption("-h") || _shellData.commandLine->getParsedCommandLine().hasOption("-help") ||
		_shellData.commandLine->getParsedCommandLine().hasOption("--help"))
	{
		applyCommandLine();
		_currentState = StateInitialised;
		_shellData.weAreDone = true;
		return Result::Success;
	} // ELSE

	_shell = newDemo();
	{
		readApiFromCommandLine();
	} // End linking dynamically
	if (!_shell->init(&_shellData))
	{
		_shell.reset();

		_currentState = StateUninitialised; // If we have reached this point, then _shell has already been initialized.
		_shellData.weAreDone = true;
		Log(LogLevel::Error, "State Machine initialisation failed : Unable to initialise the main Application Class instance");
		return Result::InitializationError;
	}

	Result result = _shell->shellInitApplication();
	if (result != Result::Success)
	{
		_shellData.weAreDone = true;
		_shell.reset();
		preExit();
		std::string error = std::string("InitApplication() failed with pvr error '") + getResultCodeString(result) + std::string("'\n");
		Log(LogLevel::Error, error.c_str());
		_currentState = StateInitialised;
		return result;
	}
	applyCommandLine();
	_currentState = StateAppInitialised;
	return Result::Success;
}
Result StateMachine::executeQuitApplication()
{
	Log(LogLevel::Debug, "StateMachine::executeQuitApplication executing");
	Result result = _shell->shellQuitApplication();

	if (result != Result::Success)
	{
		std::string error = std::string("QuitApplication() failed with pvr error '") + getResultCodeString(result) + std::string("'\n");
		Log(LogLevel::Error, error.c_str());
	}

	_shell.reset();
	preExit();
	Log(LogLevel::Debug, "StateExit");
	_currentState = StateInitialised;
	return result;
}

void StateMachine::preExit()
{
	Log(LogLevel::Debug, "StateMachine::preExit executing");
	if (!_shellData.exitMessage.empty()) { ShellOS::popUpMessage(ShellOS::getApplicationName().c_str(), _shellData.exitMessage.c_str()); }
}

Result StateMachine::executeInitWindow()
{
	Log(LogLevel::Debug, "StateMachine::executeInitWindow entered");
	if (_shellData.weAreDone) return Result::Success;
	Log(LogLevel::Debug, "StateMachine::executeInitWindow executing");
	if (!ShellOS::initializeWindow(_shellData.attributes))
	{
		_shellData.forceReleaseInitView = false;
		_shellData.forceReleaseInitWindow = false;
		_shellData.weAreDone = true;
		return Result::InitializationError;
	}
	_currentState = StateWindowInitialised;
	return Result::Success;
}

Result StateMachine::executeReleaseWindow()
{
	Log(LogLevel::Debug, "StateMachine::executeReleaseWindow executing");
	ShellOS::releaseWindow();

	_shellData.forceReleaseInitWindow = false;
	_currentState = StateAppInitialised;
	return Result::Success;
}

Result StateMachine::executeInitView()
{
	Log(LogLevel::Debug, "StateMachine::executeInitView executing");

	Result result = _shell->shellInitView();

	if (result != Result::Success)
	{
		_shellData.weAreDone = true;
		_shellData.forceReleaseInitView = false;
		_shellData.forceReleaseInitWindow = false;

		std::string error = std::string("InitView() failed with pvr error '") + getResultCodeString(result) + std::string("'\n");
		Log(LogLevel::Error, error.c_str());
		return result;
	}
	if (_shellData.outputInfo) { _shell->showOutputInfo(); }
	_currentState = StateReady;
	_shellData.startTime = _shellData.timer.getCurrentTimeMilliSecs();
	return result;
}

Result StateMachine::executeReleaseView()
{
	Log(LogLevel::Debug, "StateMachine::executeReleaseView executing");
	Result result = _shell->shellReleaseView();
	_shellData.forceReleaseInitView = false;

	if (result != Result::Success)
	{
		_shellData.forceReleaseInitWindow = false;
		_shellData.weAreDone = true;
		std::string error = std::string("ReleaseView() failed with pvr error '") + getResultCodeString(result) + std::string("'\n");
		Log(LogLevel::Error, error.c_str());
	}
	_currentState = StateWindowInitialised;
	return result;
}

Result StateMachine::executeFrame()
{
	// Exit already requested
	if (_shellData.weAreDone || _shellData.forceReleaseInitWindow || _shellData.forceReleaseInitView) { return Result::ExitRenderFrame; }
	if (_pause) { return Result::Success; }

	// May set we are done;
	ShellOS::handleOSEvents();

	// Call RenderScene
	Result result = _shell->shellRenderFrame();

	if (_shellData.weAreDone && result == Result::Success) { result = Result::ExitRenderFrame; }

	if (result != Result::Success)
	{
		if (result != Result::Success && result != Result::ExitRenderFrame)
		{
			std::string error = std::string("renderFrame() failed with pvr error '") + getResultCodeString(result) + std::string("'\n");
			Log(LogLevel::Error, error.c_str());
		}
		_shellData.weAreDone = true;
	}

	// Calculate our FPS
	{
		static uint64_t prevTime(_shellData.timer.getCurrentTimeMilliSecs()), NumFPSFrames(0);
		uint64_t time(_shellData.timer.getCurrentTimeMilliSecs()), delta(time - prevTime);

		++NumFPSFrames;

		if (delta >= 1000)
		{
			_shellData.FPS = 1000.0f * NumFPSFrames / static_cast<float>(delta);

			NumFPSFrames = 0;
			prevTime = time;

			if (_shellData.showFPS) { Log(LogLevel::Information, "Frame %i, FPS %.2f", _shellData.frameNo, _shellData.FPS); }
		}
	}

	// Have we reached the point where we need to die?
	if ((_shellData.dieAfterFrame >= 0 && _shellData.frameNo >= static_cast<uint32_t>(_shellData.dieAfterFrame)) ||
		(_shellData.dieAfterTime >= 0 && ((_shellData.timer.getCurrentTimeMilliSecs() - _shellData.startTime) * 0.001f) > _shellData.dieAfterTime))
	{ _shellData.weAreDone = true; }
	if (_shellData.forceReleaseInitWindow || _shellData.forceReleaseInitView)
	{
		Log(LogLevel::Information,
			_shellData.forceReleaseInitWindow
				? "Reinit Window+View requested: starting Reinitialization cycle. ReleaseView will be called next, then the Window will be recreated, then InitView will be "
				  "called."
				: "Reinit View requested: starting Reinitialization cycle. ReleaseView will be called next, then InitView. Window will not be recreated.");
	}

	// Increment our frame number
	++_shellData.frameNo;
	if (_shellData.weAreDone) { Log(LogLevel::Debug, "[StateMachine]: We Are Done"); }
	return result;
}

Result StateMachine::executeNext()
{
	if (_currentState == StateUninitialised)
	{
		Log(LogLevel::Error, "[StateMachine] Attemted to execute while StateMachine was uninitialised");
		return Result::UnknownError;
	}

	if (_shellData.weAreDone || _shellData.forceReleaseInitView || _shellData.forceReleaseInitWindow) { return executeDown(); }
	return executeUp();
}

Result StateMachine::executeUp()
{
	// Handle our state
	switch (_currentState)
	{
	case StateUninitialised: Log(LogLevel::Error, "[StateMachine] Attemted to execute while StateMachine was uninitialised"); return Result::UnknownError;
	case StateInitialised: return executeInitApplication();
	case StateAppInitialised: return executeInitWindow();
	case StateWindowInitialised: return executeInitView();
	case StateReady: return executeFrame();
	default: return Result::UnknownError;
	}
}

Result StateMachine::executeDown()
{
	// Handle our state
	switch (_currentState)
	{
	case StateUninitialised: Log(LogLevel::Error, "[StateMachine] Attemted to tear down step while StateMachine was uninitialised"); return Result::UnsupportedRequest;
	case StateInitialised:
		Log(LogLevel::Warning, "[StateMachine] Attemted to tear down step while StateMachine was at the Initialised state. A StateMachine object cannot be deinitialised further.");
		return Result::UnsupportedRequest; // Cannot un-initialise.
	case StateAppInitialised: return executeQuitApplication();
	case StateWindowInitialised: return executeReleaseWindow();
	case StateReady: return executeReleaseView();
	default: return Result::UnknownError;
	}
}

} // namespace platform
} // namespace pvr
//!\endcond
