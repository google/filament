/*!
\brief Contains the implementation of pvr::platform::ShellOS for Linux X11 systems.
\file PVRShell/OS/X11/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/OS/Linux/InternalOS.h"
#include "PVRCore/Log.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#undef Success

namespace pvr {
namespace platform {

static Keys X11_To_Keycode[255] = {
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Escape,

	Keys::Key1, // 10
	Keys::Key2,
	Keys::Key3,
	Keys::Key4,
	Keys::Key5,
	Keys::Key6,
	Keys::Key7,
	Keys::Key8,
	Keys::Key9,
	Keys::Key0,

	Keys::Minus, // 20
	Keys::Equals,
	Keys::Backspace,
	Keys::Tab,
	Keys::Q,
	Keys::W,
	Keys::E,
	Keys::R,
	Keys::T,
	Keys::Y,

	Keys::U, // 30
	Keys::I,
	Keys::O,
	Keys::P,
	Keys::SquareBracketLeft,
	Keys::SquareBracketRight,
	Keys::Return,
	Keys::Control,
	Keys::A,
	Keys::S,

	Keys::D, // 40
	Keys::F,
	Keys::G,
	Keys::H,
	Keys::J,
	Keys::K,
	Keys::L,
	Keys::Semicolon,
	Keys::Quote,
	Keys::Backquote,
	Keys::Shift, // 50

	Keys::Backslash,
	Keys::Z,
	Keys::X,
	Keys::C,
	Keys::V,
	Keys::B,
	Keys::N,
	Keys::M,
	Keys::Comma,

	Keys::Period, // 60
	Keys::Slash,
	Keys::Shift,
	Keys::NumMul,
	Keys::Alt,
	Keys::Space,
	Keys::CapsLock,
	Keys::F1,
	Keys::F2,
	Keys::F3,

	Keys::F4, // 70
	Keys::F5,
	Keys::F6,
	Keys::F7,
	Keys::F8,
	Keys::F9,
	Keys::F10,
	Keys::NumLock,
	Keys::ScrollLock,
	Keys::Num7,

	Keys::Num8, // 80
	Keys::Num9,
	Keys::NumSub,
	Keys::Num4,
	Keys::Num5,
	Keys::Num6,
	Keys::NumAdd,
	Keys::Num1,
	Keys::Num2,
	Keys::Num3,

	Keys::Num0, // 90
	Keys::NumPeriod,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Backslash,
	Keys::F11,
	Keys::F12,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,

	Keys::Unknown, // 100
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Return,
	Keys::Control,
	Keys::NumDiv,
	Keys::PrintScreen,
	Keys::Alt,
	Keys::Unknown,

	Keys::Home, // 110
	Keys::Up,
	Keys::PageUp,
	Keys::Left,
	Keys::Right,
	Keys::End,
	Keys::Down,
	Keys::PageDown,
	Keys::Insert,
	Keys::Delete,

	Keys::Unknown, // 120
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Pause,
	Keys::Unknown,
	Keys::Unknown,

	Keys::Unknown, // 130
	Keys::Unknown,
	Keys::Unknown,
	Keys::SystemKey1,
	Keys::SystemKey1,
	Keys::SystemKey2,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
	Keys::Unknown,
};

class X11InternalOS : public InternalOS
{
public:
	X11InternalOS(ShellOS* shellOS) : InternalOS(shellOS), _display(nullptr), _screen(0), _window(0), _colorMap(0) { _visualInfo = std::make_unique<XVisualInfo>(); }

	bool initializeWindow(DisplayAttributes& data);
	void releaseWindow();

	Display* getDisplay() const { return _display; }

	int getScreen() const { return _screen; }

	const std::unique_ptr<XVisualInfo>& getVisualInfo() const { return _visualInfo; }

	Colormap getColorMap() const { return _colorMap; }

	const Window& getWindow() const { return _window; }

	void setPointerLocation(int32_t x, int32_t y)
	{
		_pointerXY[0] = x;
		_pointerXY[1] = y;
	}

	int32_t getPointerX() const { return _pointerXY[0]; }

	int32_t getPointerY() const { return _pointerXY[1]; }

	virtual bool handleOSEvents(std::unique_ptr<Shell>& shell) override;

	void updatePointingDeviceLocation();

private:
	Display* _display;
	int _screen;
	std::unique_ptr<XVisualInfo> _visualInfo;
	Colormap _colorMap;
	Window _window;
	int32_t _pointerXY[2];

	Keys getKeyCodeFromXEvent(XEvent& xEvent);
};

static Bool waitForMapNotify(Display* /*display*/, XEvent* e, char* arg) { return (e->type == MapNotify) && (e->xmap.window == (Window)arg); }

bool X11InternalOS::initializeWindow(DisplayAttributes& data)
{
	// Open the X Display
	_display = XOpenDisplay(nullptr);

	if (!_display)
	{
		Log(LogLevel::Error, "Failed to open X display");
		return false;
	}

	// Create a default X screen using the XDisplay
	_screen = XDefaultScreen(_display);

	// Retrieve the X display width and height
	int displayWidth = XDisplayWidth(_display, _screen);
	int displayHeight = XDisplayHeight(_display, _screen);

	// Resize if the width and height if they're out of bounds
	if (!data.fullscreen)
	{
		if (data.width > static_cast<uint32_t>(displayWidth)) { data.width = displayWidth; }

		if (data.height > static_cast<uint32_t>(displayHeight)) { data.height = displayHeight; }
	}

	if (data.x == static_cast<uint32_t>(DisplayAttributes::PosDefault)) { data.x = 0; }
	if (data.y == static_cast<uint32_t>(DisplayAttributes::PosDefault)) { data.y = 0; }

	// Create the window
	XSetWindowAttributes WinAttibutes;
	XSizeHints sh;
	XEvent xEvent;

	// Return the number of planes of the default root window for the given screen
	int depth = DefaultDepth(_display, _screen);

	// Retrieves the visual information for a visual that matches the specified depth and class for the screen
	XMatchVisualInfo(_display, _screen, depth, TrueColor, getVisualInfo().get());

	if (!getVisualInfo())
	{
		Log(LogLevel::Error, "Unable to acquire visual info");
		return false;
	}

	// Creates a color map of the specified visual type for the screen on which the specified window resides
	_colorMap = XCreateColormap(_display, RootWindow(_display, _screen), getVisualInfo()->visual, AllocNone);

	// Create the window
	{
		WinAttibutes.colormap = _colorMap;
		WinAttibutes.background_pixel = 0xFFFFFFFF;
		WinAttibutes.border_pixel = 0;

		// add to these for handling other events
		WinAttibutes.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

		// The attribute mask
		unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

		// Create the X window
		_window = XCreateWindow(_display, // Display
			RootWindow(_display, _screen), // Parent
			data.x, // X position of window
			data.y, // Y position of window
			data.width, // Window width
			data.height, // Window height
			0, // Border width
			CopyFromParent, // Depth (taken from parent)
			InputOutput, // Window class
			CopyFromParent, // Visual type (taken from parent)
			mask, // Attributes mask
			&WinAttibutes); // Attributes

		// Set the window position
		sh.flags = USPosition | PMinSize | PMaxSize;
		sh.x = data.x;
		sh.y = data.y;
		sh.min_width = sh.max_width = data.width;
		sh.min_height = sh.max_height = data.height;
		XSetStandardProperties(_display, _window, data.windowTitle.c_str(), getShellOS()->getApplicationName().c_str(), 0, 0, 0, &sh);

		// Map and then wait till mapped
		XMapWindow(_display, _window);
		XIfEvent(_display, &xEvent, waitForMapNotify, reinterpret_cast<char*>(_window));

		// An attempt to hide a border for fullscreen on non OpenGL apis (OpenGLES, Vulkan)
		if (data.fullscreen)
		{
			XEvent xev;
			Atom wmState = XInternAtom(_display, "_NET_WM_STATE", False);
			Atom wmStateFullscreen = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", False);

			memset(&xev, 0, sizeof(XEvent));
			xev.type = ClientMessage;
			xev.xclient.window = getWindow();
			xev.xclient.message_type = wmState;
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 1;
			xev.xclient.data.l[1] = wmStateFullscreen;
			xev.xclient.data.l[2] = 0;
			XSendEvent(_display, RootWindow(_display, _screen), False, SubstructureNotifyMask, &xev);
		}

		Atom wmDelete = XInternAtom(_display, "WM_DELETE_WINDOW", True);
		XSetWMProtocols(_display, getWindow(), &wmDelete, 1);
		XSetWMColormapWindows(_display, getWindow(), &_window, 1);
	}

	XFlush(_display);
	return true;
}

bool X11InternalOS::handleOSEvents(std::unique_ptr<Shell>& shell)
{
	bool result = InternalOS::handleOSEvents(shell);

	XEvent xEvent;
	char* atoms;

	// Check if there are pending events which have been received but not removed from the event queue
	int numMessages = XPending(_display);

	for (int i = 0; i < numMessages; ++i)
	{
		XNextEvent(_display, &xEvent);

		switch (xEvent.type)
		{
		case ClientMessage:
		{
			atoms = XGetAtomName(_display, xEvent.xclient.message_type);
			if (*atoms == *"WM_PROTOCOLS") { shell->onSystemEvent(SystemEvent::SystemEvent_Quit); }
			XFree(atoms);
			break;
		}
		case ButtonPress:
		{
			XButtonEvent* buttonEvent = reinterpret_cast<XButtonEvent*>(&xEvent);
			switch (buttonEvent->button)
			{
			case 1:
			{
				shell->onPointingDeviceDown(0);
			}
			break;
			default: break;
			}
			break;
		}
		case ButtonRelease:
		{
			XButtonEvent* buttonEvent = reinterpret_cast<XButtonEvent*>(&xEvent);
			switch (buttonEvent->button)
			{
			case 1:
			{
				shell->onPointingDeviceUp(0);
			}
			break;
			default: break;
			}
			break;
		}
		case KeyPress:
		{
			shell->onKeyDown(X11_To_Keycode[xEvent.xkey.keycode]);
		}
		break;
		case KeyRelease:
		{
			shell->onKeyUp(X11_To_Keycode[xEvent.xkey.keycode]);
		}
		break;
		case ConfigureNotify:
		{
			XConfigureEvent* configureEvent = reinterpret_cast<XConfigureEvent*>(&xEvent);
			shell->onConfigureEvent(ConfigureEvent{ configureEvent->x, configureEvent->y, configureEvent->width, configureEvent->height, configureEvent->border_width });
		}
		case MappingNotify:
		{
			// Notifies if the keyboard mapping has changed
			// Call XRefreshKeyboardMapping to refresh our keyboard mapping
			XMappingEvent* mappingEvent = reinterpret_cast<XMappingEvent*>(&xEvent);
			XRefreshKeyboardMapping(mappingEvent);
			break;
		}
		default: break;
		}
	}

	return result;
}

void X11InternalOS::updatePointingDeviceLocation()
{
	int x, y, dummy0, dummy1;
	uint dummy2;
	Window child_return, root_return;
	if (XQueryPointer(_display, getWindow(), &root_return, &child_return, &x, &y, &dummy0, &dummy1, &dummy2))
	{ setPointerLocation(static_cast<int16_t>(x), static_cast<int16_t>(y)); }
}

void X11InternalOS::releaseWindow()
{
	XDestroyWindow(_display, getWindow());
	XFreeColormap(_display, getColorMap());
	XCloseDisplay(_display);
}

void ShellOS::updatePointingDeviceLocation()
{
	static_cast<X11InternalOS*>(_OSImplementation.get())->updatePointingDeviceLocation();

	_shell->updatePointerPosition(PointerLocation(static_cast<int16_t>(static_cast<X11InternalOS*>(_OSImplementation.get())->getPointerX()),
		static_cast<int16_t>(static_cast<X11InternalOS*>(_OSImplementation.get())->getPointerY())));
}

// Setup the capabilities
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(OSApplication application, OSDATA osdata) : _instance(application) { _OSImplementation = std::make_unique<X11InternalOS>(this); }

ShellOS::~ShellOS() {}

bool ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation) { return false; }

	return true;
}

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	static_cast<X11InternalOS*>(_OSImplementation.get())->initializeWindow(data);
	_OSImplementation->setIsInitialized(true);

	return true;
}

void ShellOS::releaseWindow()
{
	static_cast<X11InternalOS*>(_OSImplementation.get())->releaseWindow();
	_OSImplementation->setIsInitialized(false);
}

OSApplication ShellOS::getApplication() const { return _instance; }

OSConnection ShellOS::getConnection() const { return nullptr; }

OSDisplay ShellOS::getDisplay() const { return static_cast<OSDisplay>(static_cast<X11InternalOS*>(_OSImplementation.get())->getDisplay()); }

OSWindow ShellOS::getWindow() const { return reinterpret_cast<void*>(static_cast<X11InternalOS*>(_OSImplementation.get())->getWindow()); }

bool ShellOS::handleOSEvents()
{
	// Check user input from the available input devices.

	// Use the common handleOSEvents implementation
	return _OSImplementation->handleOSEvents(_shell);
}

bool ShellOS::isInitialized() { return _OSImplementation && _OSImplementation->isInitialized(); }

bool ShellOS::popUpMessage(const char* title, const char* message, ...) const
{
	if (!message) { return false; }

	va_list arg;
	va_start(arg, message);
	Log(LogLevel::Information, message, arg);
	va_end(arg);

	return true;
}
} // namespace platform
} // namespace pvr
//!\endcond
