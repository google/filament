/*!
\brief Implementation for the PowerVR Shell (pvr::Shell).
\file PVRShell/Shell.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/Shell.h"
#include "PVRShell/ShellData.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/stream/FileStream.h"
#include "PVRCore/types/Types.h"
#include "PVRCore/Log.h"
#include <cstdlib>
#include <fstream>
#if defined(_WIN32)
#include "PVRCore/Windows/WindowsResourceStream.h"
#elif defined(__ANDROID__)
#include <android_native_app_glue.h>
#include "PVRCore/Android/AndroidAssetStream.h"
#endif

#define EPSILON_PIXEL_SQUARE 100
namespace pvr {
namespace platform {
Shell::Shell() : _dragging(false), _data(0) {}

Shell::~Shell() {}

void Shell::implSystemEvent(SystemEvent systemEvent)
{
	switch (systemEvent)
	{
	case SystemEvent::SystemEvent_Quit:
		Log(LogLevel::Information, "SystemEvent::Quit");
		exitShell();
		break;
	default: break;
	}
}

void Shell::implPointingDeviceUp(uint8_t buttonIdx)
{
	if (!_pointerState.isPressed(static_cast<int8_t>(buttonIdx))) { return; }
	_pointerState.setButton(static_cast<int8_t>(buttonIdx), false);
	if (buttonIdx == 0) // NO buttons pressed - start drag
	{ _pointerState.endDragging(); }
	eventButtonUp(buttonIdx); // send the ButtonUp event

	bool drag = (_dragging && buttonIdx == 0); // Detecting drag for first button only pointer
	if (drag) // Drag button was release - Detect Drag!
	{
		_dragging = false;
		eventDragFinished(_pointerState.position());

		int16_t dx = _pointerState.position().x - _pointerState.dragStartPosition().x;
		int16_t dy = _pointerState.position().y - _pointerState.dragStartPosition().y;
		drag = (dx * dx + dy * dy > EPSILON_PIXEL_SQUARE);

		//////// MAPPING SWIPES - TOUCHES TO MAIN INPUT /////////
		// Swiping motion -> Left/Right/Up/Down
		// Touching : Center: Action1, Left part = Action2, Right part = Action3
		float dist = static_cast<float>(dx * dx + dy * dy);
		if (dist > 10 * EPSILON_PIXEL_SQUARE) // SWIPE -- needs a slightly bigger gesture than drag, but otherwise it's the same...
		{
			SimplifiedInput act = (dy * dy > dx * dx) ? (dy < 0 ? SimplifiedInput::Up : SimplifiedInput::Down) : (dx > 0 ? SimplifiedInput::Right : SimplifiedInput::Left);
			eventMappedInput(act);
		}
	}
	if (!drag) // Not a drag, then a click...
	{
		eventClick(buttonIdx, _pointerState.position());

		if (buttonIdx == 0) // First button, so map to other actions as well...
		{
			SimplifiedInput act = getPointerNormalisedPosition().x < .25 ? SimplifiedInput::Action2 : // Left
				getPointerNormalisedPosition().x > .75 ? SimplifiedInput::Action3 : // Right
					SimplifiedInput::Action1; // Center
			eventMappedInput(act);
		}
		else // For mouses, map mouse actions to other actions as well
		{
			SimplifiedInput action = MapPointingDeviceButtonToSimpleInput(buttonIdx);
			if (action != SimplifiedInput::NONE) { eventMappedInput(action); }
		}
	}
}

void Shell::implPointingDeviceDown(uint8_t buttonIdx)
{
	if (!_pointerState.isPressed(buttonIdx))
	{
		_pointerState.setButton(buttonIdx, true);
		if (buttonIdx == 0) // NO buttons pressed - start drag
		{ _pointerState.startDragging(); }
		eventButtonDown(buttonIdx);
	}
}

void Shell::updatePointerPosition(PointerLocation location)
{
	_pointerState.setPointerLocation(location);
	if (!_dragging && _pointerState.isDragging())
	{
		int16_t dx = _pointerState.position().x - _pointerState.dragStartPosition().x;
		int16_t dy = _pointerState.position().y - _pointerState.dragStartPosition().y;
		_dragging = (dx * dx + dy * dy > EPSILON_PIXEL_SQUARE);
		if (_dragging) { eventDragStart(0, _pointerState.dragStartPosition()); }
	}
}

void Shell::implKeyDown(Keys key)
{
	if (!_keystate[static_cast<uint32_t>(key)]) // Swallow event on repeat.
	{
		_keystate[static_cast<uint32_t>(key)] = true;
		eventKeyDown(key);
	}
	eventKeyStroke(key);
}

void Shell::implKeyUp(Keys key)
{
	if (_keystate[static_cast<uint32_t>(key)])
	{
		_keystate[static_cast<uint32_t>(key)] = false;
		eventKeyUp(key);
		SimplifiedInput action = MapKeyToMainInput(key);
		if (action != SimplifiedInput::NONE) { eventMappedInput(action); }
	}
}

Result Shell::shellInitApplication()
{
	assertion(_data != NULL);
	pvr::Result res;

	_data->timeAtInitApplication = getTime();
	_data->lastFrameTime = _data->timeAtInitApplication;
	_data->currentFrameTime = _data->timeAtInitApplication;

	// if a debugger is present then let it break on exceptions rather than catching them manually. This provides a much improved experience
#ifdef DEBUG
	if (isDebuggerPresent())
	{
		res = initApplication();
		return res;
	}
#endif

	try
	{
		res = initApplication();
	}
	catch (const std::runtime_error& e)
	{
		setExitMessage("InitApplication threw a runtime exception with message: '%s'", e.what());
		res = pvr::Result::InitializationError;
	}
	return res;
}

Result Shell::shellQuitApplication()
{
	pvr::Result res;

	// if a debugger is present then let it break on exceptions rather than catching them manually. This provides a much improved experience
#ifdef DEBUG
	if (isDebuggerPresent())
	{
		res = quitApplication();
		return res;
	}
#endif

	try
	{
		res = quitApplication();
	}
	catch (const std::runtime_error& e)
	{
		setExitMessage("QuitAplication threw a runtime exception with message: '%s'", e.what());
		res = pvr::Result::InitializationError;
	}

	return res;
}

Result Shell::shellInitView()
{
	pvr::Result res;

	// if a debugger is present then let it break on exceptions rather than catching them manually. This provides a much improved experience
#ifdef DEBUG
	if (isDebuggerPresent())
	{
		res = initView();
		return res;
	}
#endif

	try
	{
		res = initView();
	}
	catch (const std::runtime_error& e)
	{
		setExitMessage("InitView threw a runtime exception with message: '%s'", e.what());
		res = pvr::Result::InitializationError;
	}

	_data->currentFrameTime = getTime() - 17; // Avoid first frame huge times
	_data->lastFrameTime = getTime() - 32;
	return res;
}

Result Shell::shellReleaseView()
{
	pvr::Result res;

	// if a debugger is present then let it break on exceptions rather than catching them manually. This provides a much improved experience
#ifdef DEBUG
	if (isDebuggerPresent())
	{
		res = releaseView();
		return res;
	}
#endif

	try
	{
		res = releaseView();
	}
	catch (const std::runtime_error& e)
	{
		setExitMessage("ReleaseView threw a runtime exception with message: '%s'", e.what());
		res = pvr::Result::UnknownError;
	}

	return res;
}

Result Shell::shellRenderFrame()
{
	Result res = Result::Success;
	getOS().updatePointingDeviceLocation();

	bool processedEvents = false;

	// if a debugger is present then let it break on exceptions rather than catching them manually. This provides a much improved experience
#ifdef DEBUG
	if (isDebuggerPresent())
	{
		processedEvents = true;
		processShellEvents();
	}
#endif

	if (!processedEvents)
	{
		try
		{
			processShellEvents();
		}
		catch (const std::runtime_error& e)
		{
			setExitMessage("runtime exception during processing shell events, with message: '%s'", e.what());
			return pvr::Result::UnknownError;
		}
	}

	_data->lastFrameTime = _data->currentFrameTime;
	_data->currentFrameTime = getTime();
	if (!_data->weAreDone)
	{
		// if a debugger is present then let it break on exceptions rather than catching them manually. This provides a much improved experience
#ifdef DEBUG
		if (isDebuggerPresent())
		{
			res = renderFrame();
			return res;
		}
#endif

		try
		{
			res = renderFrame();
		}
		catch (const std::runtime_error& e)
		{
			setExitMessage("RenderFrame threw a runtime exception with message: '%s'", e.what());
			res = pvr::Result::UnknownError;
		}
	}
	//_data->weAreDone can very well be changed DURING renderFrame.
	if (_data->weAreDone) { res = Result::ExitRenderFrame; }
	return res;
}

void Shell::processShellEvents()
{
	while (!eventQueue.empty())
	{
		ShellEvent event = eventQueue.front();
		eventQueue.pop();
		switch (event.type)
		{
		case ShellEvent::SystemEvent: implSystemEvent(event.systemEvent); break;
		case ShellEvent::PointingDeviceDown: implPointingDeviceDown(event.buttonIdx); break;
		case ShellEvent::PointingDeviceUp: implPointingDeviceUp(event.buttonIdx); break;
		case ShellEvent::KeyDown: implKeyDown(event.key); break;
		case ShellEvent::KeyUp: implKeyUp(event.key); break;
		case ShellEvent::PointingDeviceMove: break;
		}
	}
}

uint64_t Shell::getFrameTime() const { return _data->currentFrameTime - _data->lastFrameTime; }

uint64_t Shell::getTime() const
{
	ShellData& data = *_data;

	if (data.forceFrameTime) { return data.frameNo * data.fakeFrameTime; }

	return data.timer.getCurrentTimeMilliSecs();
}

uint64_t Shell::getTimeAtInitApplication() const { return _data->timeAtInitApplication; }

bool Shell::init(struct ShellData* data)
{
	if (!_data)
	{
		_data = data;
		return true;
	}

	return false;
}

const platform::CommandLineParser::ParsedCommandLine& Shell::getCommandLine() const { return _data->commandLine->getParsedCommandLine(); }

void Shell::setFullscreen(const bool fullscreen)
{
	if (ShellOS::getCapabilities().resizable != Capability::Unsupported) { _data->attributes.fullscreen = fullscreen; }
}

bool Shell::isFullScreen() const { return _data->attributes.fullscreen; }

Result Shell::setDimensions(uint32_t w, uint32_t h)
{
	if (ShellOS::getCapabilities().resizable != Capability::Unsupported)
	{
		_data->attributes.width = w;
		_data->attributes.height = h;
		return Result::Success;
	}

	return Result::UnsupportedRequest;
}

uint32_t Shell::getWidth() const { return _data->attributes.width; }

uint32_t Shell::getHeight() const { return _data->attributes.height; }

uint32_t Shell::getCaptureFrameScale() const { return _data->captureFrameScale; }

Api Shell::getMaxApi() const { return this->_data->contextType; }

Api Shell::getMinApi() const { return this->_data->minContextType; }

Result Shell::setPosition(uint32_t x, uint32_t y)
{
	if (ShellOS::getCapabilities().resizable != Capability::Unsupported)
	{
		_data->attributes.x = x;
		_data->attributes.y = y;
		return Result::Success;
	}

	return Result::UnsupportedRequest;
}

uint32_t Shell::getPositionX() const { return _data->attributes.x; }

uint32_t Shell::getPositionY() const { return _data->attributes.y; }

int32_t Shell::getQuitAfterFrame() const { return _data->dieAfterFrame; }

float Shell::getQuitAfterTime() const { return _data->dieAfterTime; }

VsyncMode Shell::getVsyncMode() const { return _data->attributes.vsyncMode; }

uint32_t Shell::getAASamples() const { return _data->attributes.aaSamples; }

uint32_t Shell::getColorBitsPerPixel() const { return _data->attributes.redBits + _data->attributes.blueBits + _data->attributes.greenBits + _data->attributes.alphaBits; }

uint32_t Shell::getDepthBitsPerPixel() const { return _data->attributes.depthBPP; }

uint32_t Shell::getStencilBitsPerPixel() const { return _data->attributes.stencilBPP; }

void Shell::setQuitAfterFrame(uint32_t value) { _data->dieAfterFrame = value; }

void Shell::setQuitAfterTime(float value) { _data->dieAfterTime = value; }

void Shell::setVsyncMode(VsyncMode value) { _data->attributes.vsyncMode = value; }

void Shell::setPreferredSwapChainLength(uint32_t swapChainLength) { _data->attributes.swapLength = swapChainLength; }

void Shell::forceReleaseInitView() { _data->forceReleaseInitView = true; }

void Shell::forceReleaseInitWindow() { _data->forceReleaseInitWindow = true; }

void Shell::setAASamples(uint32_t value) { _data->attributes.aaSamples = value; }

void Shell::setColorBitsPerPixel(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	_data->attributes.redBits = r;
	_data->attributes.greenBits = g;
	_data->attributes.blueBits = b;
	_data->attributes.alphaBits = a;
}

void Shell::setBackBufferColorspace(ColorSpace colorSpace) { _data->attributes.frameBufferSrgb = (colorSpace == ColorSpace::sRGB); }

ColorSpace Shell::getBackBufferColorspace() const { return _data->attributes.frameBufferSrgb ? ColorSpace::sRGB : ColorSpace::lRGB; }

void Shell::setDepthBitsPerPixel(uint32_t value) { _data->attributes.depthBPP = value; }

void Shell::setStencilBitsPerPixel(uint32_t value) { _data->attributes.stencilBPP = value; }

void Shell::setCaptureFrames(uint32_t start, uint32_t stop)
{
	_data->captureFrameStart = start;
	_data->captureFrameStop = stop;
}

void Shell::setCaptureFrameScale(uint32_t value)
{
	if (value >= 1) { _data->captureFrameScale = value; }
}

uint32_t Shell::getCaptureFrameStart() const { return _data->captureFrameStart; }

uint32_t Shell::getCaptureFrameStop() const { return _data->captureFrameStop; }

uint32_t Shell::getFrameNumber() const { return _data->frameNo; }

void Shell::setContextPriority(uint32_t value) { _data->attributes.contextPriority = value; }

uint32_t Shell::getContextPriority() const { return _data->attributes.contextPriority; }

void Shell::setDesiredConfig(uint32_t value) { _data->attributes.configID = value; }

uint32_t Shell::getDesiredConfig() const { return _data->attributes.configID; }

void Shell::setFakeFrameTime(uint32_t value) { _data->fakeFrameTime = value; }

uint32_t Shell::getFakeFrameTime() const { return _data->fakeFrameTime; }

std::unique_ptr<Stream> Shell::getWriteAssetStream(const std::string& filename, bool allowRead, bool truncateIfExists) const
{
	std::unique_ptr<Stream> stream;
	std::string mode;

	try
	{
		// the file open mode will depend on the flags possible combinations are: wb, wb+, ab, ab+.
		mode.append((truncateIfExists ? "a" : "w")).append("b").append((allowRead ? "+" : ""));

		// The files will be written to the WritePath which is platform specific.
		stream = std::make_unique<FileStream>(getWritePath() + filename, mode, false);

		if (stream.get())
		{
			if (stream->isWritable()) { return stream; }
		}
	}
	catch (...)
	{
		stream.reset();
	}

	return std::unique_ptr<Stream>();
}

std::unique_ptr<Stream> Shell::getAssetStream(const std::string& filename, bool errorIfFileNotFound) const
{
	// The shell will first attempt to open a file in your readpath with the same name.
	// This allows you to override any built-in assets
	std::unique_ptr<Stream> stream;
	// Try absolute path first:

	stream = std::make_unique<FileStream>(filename, "rb", false);
	if (stream->isReadable()) { return stream; }
	stream.reset(0);

	// Then relative to the search paths:
	const std::vector<std::string>& paths = getOS().getReadPaths();
	for (size_t i = 0; i < paths.size(); ++i)
	{
		std::string filepath(paths[i]);
		filepath += filename;

		stream = std::make_unique<FileStream>(filepath, "rb", false);
		if (stream->isReadable()) { return stream; }

		stream.reset(0);
	}

	// Now we attempt to load assets using the OS defined method
#if defined(_WIN32) // On windows, the filename also matches the resource id in our examples, which is fortunate
	try
	{
		stream = std::make_unique<WindowsResourceStream>(filename.c_str());
	}
	catch (FileNotFoundError)
	{}
#elif defined(__ANDROID__) // On android, external files are packaged in the .apk as assets
	struct android_app* app = static_cast<android_app*>(_data->os->getApplication());

	try
	{
		if (app && app->activity && app->activity->assetManager) { stream = std::make_unique<AndroidAssetStream>(app->activity->assetManager, filename.c_str()); }
		else
		{
			Log(LogLevel::Debug, "Could not request android asset stream %s -- Application, Activity or Assetmanager was null", filename.c_str());
		}
	}
	catch (FileNotFoundError)
	{}
#endif // On the rest of the files, the filesystem is either sandboxed (iOS) or we use it directly (Linux)
	try
	{
		if (stream.get())
		{
			if (stream->isReadable()) { return stream; }
		}
	}
	catch (...)
	{
		stream.reset();
	}
	if (errorIfFileNotFound) { throw FileNotFoundError(filename, "[pvr::Shell::getAssetStream]"); }
	return std::unique_ptr<Stream>();
}

template<typename... Args>
void Shell::setApplicationName(const char* const format, Args... args)
{
	getOS().setApplicationName(strings::createFormatted(format, args...).c_str());
}

const std::string& Shell::getExitMessage() const { return _data->exitMessage; }

const std::string& Shell::getApplicationName() const { return getOS().getApplicationName(); }

const std::string& Shell::getDefaultReadPath() const { return getOS().getDefaultReadPath(); }

const std::vector<std::string>& Shell::getReadPaths() const { return getOS().getReadPaths(); }

void Shell::addReadPath(const std::string& readPaths) { getOS().addReadPath(readPaths); }

const std::string& Shell::getWritePath() const { return getOS().getWritePath(); }

ShellOS& Shell::getOS() const { return *_data->os; }

std::string Shell::getScreenshotFileName() const
{
	// determine the screenshot filename
	std::string filename;
	std::string prefix(getWritePath().c_str());
	std::string suffix = strings::createFormatted("_f%u.tga", getFrameNumber());

	prefix += getApplicationName();
	filename = prefix + suffix;
	bool fileExists;
	bool foundValidFile = false;
	{
		std::ifstream f(filename.c_str());
		fileExists = f.good();
	}

	// if the file already exists then add an integer identifier to the filename
	if (fileExists)
	{
		for (uint32_t i = 1; i < 10000; ++i)
		{
			suffix = strings::createFormatted("_f%u_%u.tga", getFrameNumber(), i);
			filename = prefix + suffix;
			{
				std::ifstream f(filename.c_str());
				if (!f.good())
				{
					// found a filename which doesn't already exist
					foundValidFile = true;
					break;
				}
			}
		}
		if (!foundValidFile) { throw std::runtime_error("Could not create a screenshot file"); }
	}

	return filename;
}

void Shell::showOutputInfo() const
{
	std::string attributesInfo, tmp;
	attributesInfo.reserve(2048);
	tmp.reserve(1024);

	tmp = std::string("\nApplication name:\t") + getApplicationName() + "\n\n";
	attributesInfo.append(tmp);

	tmp = std::string("SDK version:\t") + std::string(getSDKVersion()) + "\n\n";
	attributesInfo.append(tmp);

	tmp = std::string("Read path:\t") + getDefaultReadPath() + "\n\n";
	attributesInfo.append(tmp);

	tmp = std::string("Write path:\t") + getWritePath() + "\n\n";
	attributesInfo.append(tmp);

	attributesInfo.append("Command-line:");

	const platform::CommandLineParser::ParsedCommandLine::Options& options = getCommandLine().getOptionsList();

	for (uint32_t i = 0; i < options.size(); ++i)
	{
		if (options[i].val) { tmp = strings::createFormatted(" %hs=%hs", options[i].arg, options[i].val); }
		else
		{
			tmp = strings::createFormatted(" %hs", options[i].arg);
		}

		attributesInfo.append(tmp);
	}

	attributesInfo.append("\n");

	int32_t frame = getQuitAfterFrame();
	if (frame != -1)
	{
		tmp = strings::createFormatted("Quit after frame:\t%i\n", frame);
		attributesInfo.append(tmp);
	}

	float time = getQuitAfterTime();
	if (time != -1)
	{
		tmp = strings::createFormatted("Quit after time:\t%f\n", time);
		attributesInfo.append(tmp);
	}

#if defined(__ANDROID__)
	// Android's logging output truncates long strings. Therefore, output our info in blocks
	int32_t size = static_cast<int32_t>(attributesInfo.size());
	const char* const ptr = attributesInfo.c_str();

	for (uint32_t offset = 0; size > 0; offset += 1024, size -= 1024)
	{
		std::string chunk(&ptr[offset], std::min(size, 1024));
		Log(LogLevel::Information, chunk.c_str());
	}

#else
	Log(LogLevel::Information, attributesInfo.c_str());
#endif
}

void Shell::setForceFrameTime(const bool value)
{
	_data->forceFrameTime = value;
	if (value)
	{
		_data->timeAtInitApplication = 0;
		_data->lastFrameTime = 0;
		_data->currentFrameTime = 0;
	}
}

bool Shell::isForcingFrameTime() const { return _data->forceFrameTime; }

void Shell::setShowFPS(const bool showFPS) { _data->showFPS = showFPS; }

bool Shell::isShowingFPS() const { return _data->showFPS; }

float Shell::getFPS() const { return _data->FPS; }

bool Shell::isScreenRotated() const { return _data->attributes.isDisplayPortrait() && isFullScreen(); }

bool Shell::isScreenPortrait() const { return _data->attributes.isDisplayPortrait(); }

bool Shell::isScreenLandscape() const { return !_data->attributes.isDisplayPortrait(); }

void Shell::exitShell() { _data->weAreDone = true; }

const DisplayAttributes& Shell::getDisplayAttributes() const { return _data->attributes; }
DisplayAttributes& Shell::getDisplayAttributes() { return _data->attributes; }
OSConnection Shell::getConnection() const { return _data->os->getConnection(); }
OSDisplay Shell::getDisplay() const { return _data->os->getDisplay(); }
OSWindow Shell::getWindow() const { return _data->os->getWindow(); }
} // namespace platform
} // namespace pvr
#undef EPSILON_PIXEL_SQUARE
//!\endcond
