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

#include "xcb/xcb.h"
#include <X11/Xlib-xcb.h>
#undef Success

namespace pvr {
namespace platform {

// See array kHardwareKeycodeMap https://chromium.googlesource.com/chromium/src/+/lkgr/ui/events/keycodes/keyboard_code_conversion_x.cc
static Keys XCBKeyCodeToKeys[255] = {
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

class XCBInternalOS : public InternalOS
{
public:
	XCBInternalOS(ShellOS* shellOS) : InternalOS(shellOS), _connection(nullptr), _screen(nullptr), _window(0) {}

	bool initializeWindow(DisplayAttributes& data);
	void releaseWindow();

	xcb_connection_t* getConnection() const { return _connection; }

	Display* getDisplay() const { return _display; }

	xcb_screen_t* getScreen() const { return _screen; }

	const xcb_window_t& getWindow() const { return _window; }

	void setPointerLocation(int32_t x, int32_t y)
	{
		_pointerXY[0] = x;
		_pointerXY[1] = y;
	}

	int32_t getPointerX() const { return _pointerXY[0]; }

	int32_t getPointerY() const { return _pointerXY[1]; }

	virtual bool handleOSEvents(std::unique_ptr<Shell>& shell) override;

private:
	xcb_connection_t* _connection;
	xcb_screen_t* _screen;
	xcb_window_t _window;
	int32_t _pointerXY[2];
	uint32_t _deleteWindowAtom;
	Display* _display;

	inline Keys getKeyCodeFromXCBEvent(xcb_generic_event_t* xcbEvent);
};

bool XCBInternalOS::initializeWindow(DisplayAttributes& data)
{
	// initialize the connection
	_display = XOpenDisplay(nullptr);

	if (!_display)
	{
		Log(LogLevel::Error, "Failed to open XOpenDisplay");
		return false;
	}

	_connection = XGetXCBConnection(_display);

	if (!_connection || xcb_connection_has_error(_connection))
	{
		Log(LogLevel::Error, "Failed to open XCB connection");
		return false;
	}

	// Retrieve data returned by the server when the connection was initialized
	const xcb_setup_t* setup = xcb_get_setup(_connection);

	int screenCount = xcb_setup_roots_length(setup);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

	for (uint32_t i = 0; i < screenCount; ++screenCount)
	{
		// Else retrieve the first valid screen
		if (iter.data)
		{
			_screen = iter.data;
			break;
		}
		xcb_screen_next(&iter);
	}

	if (!_screen)
	{
		Log(LogLevel::Error, "Failed to find a valid XCB screen");
		return false;
	}

	// Allocate an XID for the window
	_window = xcb_generate_id(_connection);

	if (!_window)
	{
		Log(LogLevel::Error, "Failed to allocate an id for an XCB window");
		return false;
	}

	// Resize if the width and height if they're out of bounds
	if (!data.fullscreen)
	{
		if (data.width > static_cast<uint32_t>(_screen->width_in_pixels)) { data.width = _screen->width_in_pixels; }

		if (data.height > static_cast<uint32_t>(_screen->height_in_pixels)) { data.height = _screen->height_in_pixels; }
	}

	if (data.x == static_cast<uint32_t>(DisplayAttributes::PosDefault)) { data.x = 0; }
	if (data.y == static_cast<uint32_t>(DisplayAttributes::PosDefault)) { data.y = 0; }

	// XCB_CW_BACK_PIXEL - A pixemap of undefined size filled with the specified background pixel is used for the background. Range-checking is not performed.
	// XCB_CW_BORDER_PIXMAP - Specifies the pixel color used for the border
	// XCB_CW_EVENT_MASK - The event-mask defines which events the client is interested in for this window
	uint32_t valueMask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXMAP | XCB_CW_EVENT_MASK;
	uint32_t valueList[3] = { _screen->black_pixel, 0,
		XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_POINTER_MOTION };

	xcb_create_window(_connection, XCB_COPY_FROM_PARENT, _window, _screen->root, data.x, data.y, data.width, data.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, _screen->root_visual,
		valueMask, valueList);

	// Setup code that will send a notification when the window is destroyed.
	xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(_connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_cookie_t wm_delete_window_cookie = xcb_intern_atom(_connection, 0, 16, "WM_DELETE_WINDOW");
	xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(_connection, wm_protocols_cookie, 0);
	xcb_intern_atom_reply_t* wm_delete_window_reply = xcb_intern_atom_reply(_connection, wm_delete_window_cookie, 0);

	_deleteWindowAtom = wm_delete_window_reply->atom;
	xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, wm_protocols_reply->atom, XCB_ATOM_ATOM, 32, 1, &wm_delete_window_reply->atom);

	free(wm_protocols_reply);
	free(wm_delete_window_reply);

	// Change the title of the window to match the example title
	const char* title = data.windowTitle.c_str();
	xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

	xcb_map_window(_connection, _window);
	xcb_flush(_connection);
	return true;
}

// Key press events cannot be used directly and must be translated to a KeySym
// This keysym can then be used to lookup the keypress using its ASCII encoding
inline Keys XCBInternalOS::getKeyCodeFromXCBEvent(xcb_generic_event_t* xcbEvent)
{
	xcb_key_press_event_t* keyPressEvent = reinterpret_cast<xcb_key_press_event_t*>(xcbEvent);

	if (keyPressEvent->detail >= ARRAY_SIZE(XCBKeyCodeToKeys)) { return Keys::Unknown; }
	return XCBKeyCodeToKeys[keyPressEvent->detail];
}

bool XCBInternalOS::handleOSEvents(std::unique_ptr<Shell>& shell)
{
	bool result = InternalOS::handleOSEvents(shell);

	xcb_generic_event_t* genericEvent;
	while ((genericEvent = xcb_poll_for_event(_connection)))
	{
		const uint8_t event_code = (genericEvent->response_type & 0x7f);
		if (event_code == XCB_CLIENT_MESSAGE)
		{
			const xcb_client_message_event_t* clientMessageEvent = (const xcb_client_message_event_t*)genericEvent;
			if (clientMessageEvent->data.data32[0] == _deleteWindowAtom)
			{
				shell->onSystemEvent(SystemEvent::SystemEvent_Quit);
				break;
			}
		}
		switch (genericEvent->response_type & ~0x80)
		{
		case XCB_DESTROY_NOTIFY:
		{
			shell->onSystemEvent(SystemEvent::SystemEvent_Quit);
			break;
		}
		case XCB_MOTION_NOTIFY:
		{
			xcb_motion_notify_event_t* motion = reinterpret_cast<xcb_motion_notify_event_t*>(genericEvent);
			setPointerLocation(motion->event_x, motion->event_y);
			break;
		}
		case XCB_BUTTON_PRESS:
		{
			xcb_button_press_event_t* buttonPressEvent = reinterpret_cast<xcb_button_press_event_t*>(genericEvent);
			switch (buttonPressEvent->detail)
			{
			case 1: { shell->onPointingDeviceDown(0);
			}
			break;
			default: break;
			}
			break;
		}
		case XCB_BUTTON_RELEASE:
		{
			xcb_button_press_event_t* buttonPressEvent;
			buttonPressEvent = reinterpret_cast<xcb_button_press_event_t*>(genericEvent);
			switch (buttonPressEvent->detail)
			{
			case 1: { shell->onPointingDeviceUp(0);
			}
			break;
			default: break;
			}
			break;
		}
		case XCB_KEY_PRESS:
		{
			shell->onKeyDown(getKeyCodeFromXCBEvent(genericEvent));
			break;
		}
		case XCB_KEY_RELEASE:
		{
			shell->onKeyUp(getKeyCodeFromXCBEvent(genericEvent));
			break;
		}
		default: break;
		}

		free(genericEvent);
	}

	return result;
}

void XCBInternalOS::releaseWindow()
{
	xcb_destroy_window(_connection, _window);
	xcb_disconnect(_connection);
}

void ShellOS::updatePointingDeviceLocation()
{
	_shell->updatePointerPosition(PointerLocation(static_cast<int16_t>(static_cast<XCBInternalOS*>(_OSImplementation.get())->getPointerX()),
		static_cast<int16_t>(static_cast<XCBInternalOS*>(_OSImplementation.get())->getPointerY())));
}

// Setup the capabilities
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(OSApplication application, OSDATA osdata) : _instance(application) { _OSImplementation = std::make_unique<XCBInternalOS>(this); }

ShellOS::~ShellOS() {}

bool ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation) { return false; }

	return true;
}

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	static_cast<XCBInternalOS*>(_OSImplementation.get())->initializeWindow(data);
	_OSImplementation->setIsInitialized(true);

	return true;
}

void ShellOS::releaseWindow()
{
	static_cast<XCBInternalOS*>(_OSImplementation.get())->releaseWindow();
	_OSImplementation->setIsInitialized(false);
}

OSApplication ShellOS::getApplication() const { return _instance; }

OSConnection ShellOS::getConnection() const { return static_cast<OSConnection>(static_cast<XCBInternalOS*>(_OSImplementation.get())->getConnection()); }

OSDisplay ShellOS::getDisplay() const { return static_cast<OSDisplay>(static_cast<XCBInternalOS*>(_OSImplementation.get())->getDisplay()); }

OSWindow ShellOS::getWindow() const { return reinterpret_cast<void*>(static_cast<XCBInternalOS*>(_OSImplementation.get())->getWindow()); }

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
