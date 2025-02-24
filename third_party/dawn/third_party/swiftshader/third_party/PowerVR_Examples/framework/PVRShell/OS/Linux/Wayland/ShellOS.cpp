/*!
\brief Contains an implementation of pvr::platform::ShellOS for Linux Wayland based windowing systems.
\file PVRShell/OS/Linux/Wayland/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/OS/Linux/InternalOS.h"
#include "PVRCore/Log.h"

#include <linux/input.h>
#include <wayland-client.h>
#include <wayland-server.h>

namespace pvr {
namespace platform {
class WaylandInternalOS : public InternalOS
{
public:
	WaylandInternalOS(ShellOS* shellOS)
		: InternalOS(shellOS), _display(0), _registry(nullptr), _compositor(nullptr), _shell(nullptr), _seat(nullptr), _pointer(nullptr), _keyboard(nullptr),
		  _waylandSurface(nullptr), _shellSurface(nullptr), _pointerXY{}
	{}

	bool initWaylandConnection();
	bool initializeWindow(DisplayAttributes& data);
	void releaseWindow();
	void releaseWaylandConnection();

	void setDisplay(wl_display* display) { _display = display; }

	wl_display* getDisplay() const { return _display; }

	void setRegistry(wl_registry* registry) { _registry = registry; }

	wl_registry* getRegistry() const { return _registry; }

	void setCompositor(wl_compositor* compositor) { _compositor = compositor; }

	wl_compositor* getCompositor() const { return _compositor; }

	void setShell(wl_shell* shell) { _shell = shell; }

	wl_shell* getShell() const { return _shell; }

	void setSeat(wl_seat* seat) { _seat = seat; }

	wl_seat* getSeat() const { return _seat; }

	void setPointer(wl_pointer* pointer) { _pointer = pointer; }

	wl_pointer* getPointer() const { return _pointer; }

	void setKeyboard(wl_keyboard* keyboard) { _keyboard = keyboard; }

	wl_keyboard* getKeyboard() const { return _keyboard; }

	wl_surface* getSurface() const { return _waylandSurface; }

	wl_shell_surface* getShellSurface() const { return _shellSurface; }

	void setPointerLocation(int32_t x, int32_t y)
	{
		_pointerXY[0] = x;
		_pointerXY[1] = y;
	}

	int32_t getPointerX() const { return _pointerXY[0]; }

	int32_t getPointerY() const { return _pointerXY[1]; }

	virtual bool handleOSEvents(std::unique_ptr<Shell>& shell) override;

private:
	wl_display* _display;
	wl_registry* _registry;
	wl_compositor* _compositor;
	wl_shell* _shell;
	wl_seat* _seat;
	wl_pointer* _pointer;
	wl_keyboard* _keyboard;
	wl_surface* _waylandSurface;
	wl_shell_surface* _shellSurface;
	int32_t _pointerXY[2];
};

// Pointer specific listener callbacks
// Notification when the seat's pointer enters a surface
static void pointerHandleEnter(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy) {}

// Notification when the seat's pointer leaves a surface
static void pointerHandleLeave(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface) {}

// Handle the mouse motion
static void pointerHandleMotion(void* data, struct wl_pointer* pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	pvr::platform::WaylandInternalOS* internalOS = reinterpret_cast<pvr::platform::WaylandInternalOS*>(data);
	internalOS->setPointerLocation(static_cast<int16_t>(surface_x), static_cast<int16_t>(surface_y));
}

// Handle the mouse buttons
static void pointerHandleButton(void* data, struct wl_pointer* wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	pvr::platform::WaylandInternalOS* internalOS = reinterpret_cast<pvr::platform::WaylandInternalOS*>(data);

	if (internalOS)
	{
		if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) { internalOS->getShellOS()->getShell()->onPointingDeviceDown(0); }
		else if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_RELEASED)
		{
			internalOS->getShellOS()->getShell()->onPointingDeviceUp(0);
		}
		else if (button == BTN_RIGHT && state == WL_POINTER_BUTTON_STATE_PRESSED)
		{
			internalOS->getShellOS()->getShell()->onPointingDeviceDown(1);
		}
		else if (button == BTN_RIGHT && state == WL_POINTER_BUTTON_STATE_RELEASED)
		{
			internalOS->getShellOS()->getShell()->onPointingDeviceUp(1);
		}
	}
}

static void pointerHandleAxis(void* data, struct wl_pointer* wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {}

static const struct wl_pointer_listener pointerListener = {
	pointerHandleEnter,
	pointerHandleLeave,
	pointerHandleMotion,
	pointerHandleButton,
	pointerHandleAxis,
};

// Seat input listener callbacks
// Keyboard specific listener callbacks
static void keyboardHandleKeymap(void* data, struct wl_keyboard* keyboard, uint32_t format, int fd, uint32_t size) {}

// Handle keyboard entering the surface
static void keyboardHandleEnter(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {}

// Handle keyboard leaving the surface
static void keyboardHandleLeave(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface) {}

// Handle keyboard leaving the surface
static void keyboardHandleKey(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	pvr::platform::WaylandInternalOS* internalOS = reinterpret_cast<pvr::platform::WaylandInternalOS*>(data);
	if (internalOS)
	{
		uint32_t keyState = (state == WL_POINTER_BUTTON_STATE_PRESSED) ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED;

		if (keyState == WL_POINTER_BUTTON_STATE_PRESSED) { internalOS->getShellOS()->getShell()->onKeyDown(internalOS->getKeyFromEVCode(key)); }
		else
		{
			internalOS->getShellOS()->getShell()->onKeyUp(internalOS->getKeyFromEVCode(key));
		}
	}
}

static void keyboardHandleModifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{}

static const struct wl_keyboard_listener keyboardListener = {
	keyboardHandleKeymap,
	keyboardHandleEnter,
	keyboardHandleLeave,
	keyboardHandleKey,
	keyboardHandleModifiers,
};

// Handles input devices. Seats represent a group of input devices including mice, keyboards and touchscreens
static void seatHandleCapabilities(void* data, struct wl_seat* seat, uint32_t caps)
{
	pvr::platform::WaylandInternalOS* internalOS = reinterpret_cast<pvr::platform::WaylandInternalOS*>(data);

	if (internalOS)
	{
		// Handle pointer input devices
		if ((caps & WL_SEAT_CAPABILITY_POINTER) && !internalOS->getPointer())
		{
			internalOS->setPointer(wl_seat_get_pointer(seat));
			wl_pointer_add_listener(internalOS->getPointer(), &pointerListener, data);
			Log(LogLevel::Debug, "Added a pointer listener for Wayland");
		}
		if (!(caps & WL_SEAT_CAPABILITY_POINTER) && internalOS->getPointer())
		{
			wl_pointer_release(internalOS->getPointer());
			internalOS->setPointer(nullptr);
			Log(LogLevel::Debug, "Destroyed a pointer listener for Wayland");
		}

		// Handle keyboard input devices
		if (caps & WL_SEAT_CAPABILITY_KEYBOARD && (!internalOS->getKeyboard()))
		{
			internalOS->setKeyboard(wl_seat_get_keyboard(seat));
			wl_keyboard_add_listener(internalOS->getKeyboard(), &keyboardListener, data);
			Log(LogLevel::Debug, "Added a keyboard listener for Wayland");
		}
		if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && internalOS->getKeyboard())
		{
			wl_keyboard_destroy(internalOS->getKeyboard());
			internalOS->setKeyboard(nullptr);
			Log(LogLevel::Debug, "Destroyed a keyboard listener for Wayland");
		}
	}
}
static void seatHandleName(void* data, struct wl_seat* seat, const char* name) { Log(LogLevel::Debug, "Seat Handle name: %s", name); }

static const struct wl_seat_listener seatListener = { seatHandleCapabilities, seatHandleName };

// Global registry callbacks. Handle the creation and destruction of proxy objects
static void globalRegistryCallback(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
	pvr::platform::WaylandInternalOS* internalOS = reinterpret_cast<pvr::platform::WaylandInternalOS*>(data);

	if (internalOS)
	{
		if (strcmp(interface, "wl_compositor") == 0) { internalOS->setCompositor(reinterpret_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1))); }
		else if (strcmp(interface, "wl_shell") == 0)
		{
			internalOS->setShell(reinterpret_cast<wl_shell*>(wl_registry_bind(registry, id, &wl_shell_interface, 1)));
		}
		else if (strcmp(interface, "wl_seat") == 0)
		{
			internalOS->setSeat(reinterpret_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 1)));
			wl_seat_add_listener(internalOS->getSeat(), &seatListener, data);
		}
	}
}

static void globalRegistryRemover(void* data, struct wl_registry* wl_registry, uint32_t id) { Log(LogLevel::Debug, "Removing registry event for: %d", id); }

static const wl_registry_listener registryListener = { globalRegistryCallback, globalRegistryRemover };

// Surface specific registry callbacks
// Ping a client to check if it is receiving events and sending requests. A client is expected to reply with a pong request.
static void pingCallback(void* data, struct wl_shell_surface* shell_surface, uint32_t serial) { wl_shell_surface_pong(shell_surface, serial); }
// The configure event asks the client to resize its surface. We do not support resize events.
static void configureCallback(void* /*data*/, struct wl_shell_surface* /*shell_surface*/, uint32_t /*edges*/, int32_t /*width*/, int32_t /*height*/) {}

// The popup_done event is sent out when a popup grab is broken, that is, when the user clicks a surface that doesn't belong to the client owning the popup surface.
static void popupDoneCallback(void* /*data*/, struct wl_shell_surface* /*shell_surface*/) {}

static const struct wl_shell_surface_listener shellSurfaceListeners = { pingCallback, configureCallback, popupDoneCallback };

bool WaylandInternalOS::handleOSEvents(std::unique_ptr<Shell>& shell)
{
	bool result = InternalOS::handleOSEvents(shell);

	// Dispatches default queue events without reading from the display fd
	int waylandResult = wl_display_dispatch_pending(_display);
	if (waylandResult == -1) { result = false; }
	return result;
}

// Setup the capabilities
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Immutable, Capability::Immutable };

ShellOS::ShellOS(OSApplication application, OSDATA osdata) : _instance(application) { _OSImplementation = std::make_unique<WaylandInternalOS>(this); }

ShellOS::~ShellOS() {}

void ShellOS::updatePointingDeviceLocation()
{
	_shell->updatePointerPosition(PointerLocation(static_cast<int16_t>(static_cast<WaylandInternalOS*>(_OSImplementation.get())->getPointerX()),
		static_cast<int16_t>(static_cast<WaylandInternalOS*>(_OSImplementation.get())->getPointerY())));
}

bool ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation) { return false; }

	return true;
}

bool WaylandInternalOS::initWaylandConnection()
{
	// Connect to the Wayland display/server
	if ((_display = wl_display_connect(nullptr)) == nullptr)
	{
		Log(LogLevel::Error, "Failed to connect to Wayland display");
		return false;
	}
	else
	{
		Log(LogLevel::Information, "Successfully connected the Wayland display");
	}

	// Connect to the Wayland registry which will allow access to various high level proxy objects
	if ((_registry = wl_display_get_registry(getDisplay())) == nullptr)
	{
		Log(LogLevel::Error, "Failed to get Wayland registry");
		return false;
	}
	else
	{
		Log(LogLevel::Information, "Successfully retrieved the Wayland registry");
	}

	// Add registry listeners
	// 1. New proxy objects
	// 2. To remove proxy objects
	wl_registry_add_listener(_registry, &registryListener, this);

	// Make the above a blocking synchronous call
	wl_display_dispatch(getDisplay());
	wl_display_roundtrip(getDisplay());

	// Validate that the required proxy objects were created appropriately
	if (!_compositor)
	{
		Log(LogLevel::Error, "Could not find Wayland compositor");
		return false;
	}
	else
	{
		Log(LogLevel::Information, "Successfully retrieved the Wayland compositor");
	}

	if (!_shell) { Log(LogLevel::Warning, "Could not find Wayland shell"); }
	else
	{
		Log(LogLevel::Debug, "Successfully retrieved the Wayland shell");
	}

	if (!_seat) { Log(LogLevel::Warning, "Could not find Wayland seat"); }
	else
	{
		Log(LogLevel::Debug, "Successfully retrieved the Wayland seat");
	}

	return true;
}

bool WaylandInternalOS::initializeWindow(DisplayAttributes& data)
{
	// Initialize the Wayland connection and create the required proxy objects
	if (!initWaylandConnection()) { return false; }

	// Create the Wayland surface
	_waylandSurface = wl_compositor_create_surface(_compositor);
	if (_waylandSurface == nullptr)
	{
		Log("Failed to create Wayland surface");
		return false;
	}
	else
	{
		Log(LogLevel::Information, "Successfully create the Wayland surface");
	}

	_shellSurface = wl_shell_get_shell_surface(_shell, _waylandSurface);
	if (_shellSurface == nullptr)
	{
		Log("Failed to get Wayland shell surface");
		return false;
	}
	else
	{
		Log(LogLevel::Information, "Successfully create the Wayland shell surface");
	}
	wl_shell_surface_set_toplevel(_shellSurface);

	wl_shell_surface_add_listener(_shellSurface, &shellSurfaceListeners, this);
	wl_shell_surface_set_title(_shellSurface, data.windowTitle.c_str());

	return true;
}

void WaylandInternalOS::releaseWindow()
{
	wl_shell_surface_destroy(_shellSurface);
	wl_surface_destroy(_waylandSurface);

	// Release the Wayland connection
	releaseWaylandConnection();
}

void WaylandInternalOS::releaseWaylandConnection()
{
	if (getKeyboard()) { wl_keyboard_destroy(getKeyboard()); }
	if (getPointer()) { wl_pointer_destroy(getPointer()); }
	wl_seat_destroy(getSeat());
	wl_shell_destroy(getShell());
	wl_compositor_destroy(getCompositor());
	wl_registry_destroy(getRegistry());
	wl_display_disconnect(getDisplay());
}

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	if (!static_cast<WaylandInternalOS*>(_OSImplementation.get())->initializeWindow(data)) { return false; }
	_OSImplementation->setIsInitialized(true);

	return true;
}

void ShellOS::releaseWindow()
{
	static_cast<WaylandInternalOS*>(_OSImplementation.get())->releaseWindow();
	_OSImplementation->setIsInitialized(false);
}

OSApplication ShellOS::getApplication() const { return _instance; }

OSConnection ShellOS::getConnection() const { return nullptr; }

OSDisplay ShellOS::getDisplay() const { return static_cast<OSDisplay>(static_cast<WaylandInternalOS*>(_OSImplementation.get())->getDisplay()); }

OSWindow ShellOS::getWindow() const { return reinterpret_cast<void*>(static_cast<WaylandInternalOS*>(_OSImplementation.get())->getSurface()); }

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
