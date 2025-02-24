/*!
\brief Contains the implementation for the pvr::platform::ShellOS class on Null Windowing System platforms (Linux &
QNX).
\file PVRShell/OS/Linux/NullWS/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRShell/OS/Linux/InternalOS.h"
#include "PVRCore/Log.h"

#include <linux/input.h>
#include <iostream>
#include <fstream>

const std::string IdInputSet = "ID_INPUT=1";
const std::string IdKeyboardSet = "ID_INPUT_KEYBOARD=1";
const std::string IdTouchpadSet = "ID_INPUT_TOUCHPAD=1";
const std::string IdTouchScreenSet = "ID_INPUT_TOUCHSCREEN=1";

namespace pvr {
namespace platform {

// File Descriptors used for non-window system event input handling
static int TouchpadDeviceFileDescriptor;
static int TouchScreenDeviceFileDescriptor;
static int KeyboardFileDescriptor;

namespace {
	// Use 'udevadm info --query=property' to determine whether the given device supports input of the given type
	bool deviceHasEventType(const std::string& deviceName, const std::string& eventType)
	{
		// Use popen to run the udevadm info command
		// Take the output and parse it using the given event type string
		const std::string udevadmCommandWithDevice = "udevadm info --query=property --name=" + deviceName + " | grep " + eventType;
		bool hasEventType = false;
		FILE* pipe = popen(udevadmCommandWithDevice.c_str(), "r");
		if (pipe)
		{
			const size_t numCharacters(eventType.length() + 1);
			char buffer[numCharacters];
			auto nowarn = fgets(buffer, numCharacters, pipe);
			(void)nowarn;
			if (strcmp(buffer, eventType.c_str()) == 0) 
			{
				hasEventType = true;
			}
			pclose(pipe);
		}
		return hasEventType;
	}

	// Opens the device as the provided file descriptor
	void openDevice(int& fileDescriptor, const std::string& deviceName, const std::string& deviceDescription)
	{
		if(deviceName.length() > 0)
		{
			fileDescriptor = open(deviceName.c_str(), O_RDONLY | O_NONBLOCK);
			if (fileDescriptor <= 0)
			{
				Log(LogLevel::Warning, "Unable to open input device '%s' as a '%s' device -- (Code : %i - %s). Note that this input device requires 'sudo' privileges",
					deviceName.c_str(), deviceDescription.c_str(), errno, strerror(errno));
			}
			else
			{
				Log(LogLevel::Information, "Opened '%s' for device input as a '%s' device", deviceName.c_str(), deviceDescription.c_str());
			}
		}
	}
}

class NullWSInternalOS : public InternalOS
{
public:
	NullWSInternalOS(ShellOS* shellOS) : InternalOS(shellOS), _display(0), _pointerXY{}
	{
		TouchpadDeviceFileDescriptor = 0;
		TouchScreenDeviceFileDescriptor = 0;
		KeyboardFileDescriptor = 0;

		// Retrieve each of the event devices reported via "ls -l /dev/input/by-path"
		std::string eventsFromDevicesById = "ls -l /dev/input/by-path | grep -Eo event[0-9]+";
		std::vector<std::string> eventDevicePaths;

		// Grep the /dev/input/by-path file matching for event names
		// event1
		// event2
		// event3 etc.
		{
			FILE* pipe = popen(eventsFromDevicesById.c_str(), "r");
			if (pipe)
			{
				// Support events up to 4 digits i.e. event1234
				const size_t numCharacters(9);
				char deviceName[numCharacters];
				const size_t devInputLength(11 + numCharacters);
				char devInputFilePath[devInputLength] = "/dev/input/";

				// Store the full path for each event name returned by the grep
				// /dev/input/event1
				// /dev/input/event2
				// /dev/input/event3 etc.
				while(NULL != fgets(deviceName, static_cast<int>(numCharacters), pipe))
				{
					deviceName[strcspn(deviceName, "\n")] = 0;
					std::string deviceFilePath = std::string(devInputFilePath) + std::string(deviceName);
					eventDevicePaths.emplace_back(deviceFilePath);
				}
				pclose(pipe);
			}
		}

		// Favour uncombined devices i.e. a touch pad that is only a touch pad rather than one which also advertises as a touch screen
		struct UDevEventDevice
		{
			std::string devicePath = "";
			std::string deviceType = "";
			bool isCombined = false;
		};

		UDevEventDevice keyboardDevice;
		UDevEventDevice touchpadDevice;
		UDevEventDevice touchScreenDevice;

		// For each event device retrieved use udevadm info to query properties to search for supported input devices
		for(const std::string& eventDevice : eventDevicePaths) 
		{
			bool isInputDevice = false;

			// ignore the device if ID_INPUT=1 not present or set
			isInputDevice = deviceHasEventType(eventDevice, IdInputSet);

			// Search for supported device types
			if(isInputDevice)
			{
				// Find the first supported keyboard
				if(deviceHasEventType(eventDevice, IdKeyboardSet))
				{
					keyboardDevice.devicePath = eventDevice;
					continue;
				}

				bool isTouchpad = false;
				bool isTouchScreen = false;
				std::string currentTouchPadDevicePath = "";
				std::string currentTouchScreenDevicePath = "";

				// Determine whether the current event device reports as a touchpad
				if((isTouchpad = deviceHasEventType(eventDevice, IdTouchpadSet)))
				{
					currentTouchPadDevicePath = eventDevice;
				}

				// Determine whether the current event device reports as a touch screen
				if((isTouchScreen = deviceHasEventType(eventDevice, IdTouchScreenSet)))
				{
					currentTouchScreenDevicePath = eventDevice;
				}

				// If the current event device reported itself as both a touchpad and a touchscreen then mark it as a combined device
				if(isTouchpad && isTouchScreen)
				{
					touchScreenDevice.isCombined = true;
					touchpadDevice.isCombined = true;

					// Update the touchpad and touchscreen event device to use only if it has not already been set
					if(touchpadDevice.devicePath.length() == 0)
					{
						touchpadDevice.devicePath = currentTouchPadDevicePath;
					}
					if(touchScreenDevice.devicePath.length() == 0)
					{
						touchScreenDevice.devicePath = currentTouchScreenDevicePath;
					}
				}
				else
				{
					// The current event device did not report itself as both a touchpad and touchscreen device
					// Update the touchpad or touch screen device:
					// - Only the first of each type of device is recorded. There is no support for multiple different touchpads or touchscreens
					// - If the current touchpad or touchscreen device to use is a combined device then a non-combined device will be preferred 
					if(isTouchpad)
					{
						if(touchpadDevice.devicePath.length() == 0 || touchpadDevice.isCombined)
						{
							touchpadDevice.devicePath = currentTouchPadDevicePath;
						}
					}
					if(isTouchScreen)
					{
						if(touchScreenDevice.devicePath.length() == 0 || touchScreenDevice.isCombined)
						{
							touchScreenDevice.devicePath = currentTouchScreenDevicePath;
						}
					}
				}
			}
		}

		// Attempt to open the input device file descriptors
		openDevice(KeyboardFileDescriptor, keyboardDevice.devicePath, "Keyboard");
		openDevice(TouchpadDeviceFileDescriptor, touchpadDevice.devicePath, "Touchpad");
		openDevice(TouchScreenDeviceFileDescriptor, touchScreenDevice.devicePath, "Touchscreen");

		if(TouchScreenDeviceFileDescriptor > 0)
		{
			// Retrieve the absoluate min and max values for the touchscreen. These will be used to convert from surface positions to pixel coordinates
			struct input_absinfo abs;
			ioctl(TouchScreenDeviceFileDescriptor, EVIOCGABS(ABS_X), &abs);
			_absX[0] = abs.minimum;
			_absX[1] = abs.maximum;
			ioctl(TouchScreenDeviceFileDescriptor, EVIOCGABS(ABS_Y), &abs);
			_absY[0] = abs.minimum;
			_absY[1] = abs.maximum;
		}
	}

	virtual ~NullWSInternalOS()
	{
		if (KeyboardFileDescriptor) { close(KeyboardFileDescriptor); }
		if (TouchpadDeviceFileDescriptor) { close(TouchpadDeviceFileDescriptor); }
		if (TouchScreenDeviceFileDescriptor) { close(TouchScreenDeviceFileDescriptor); }
	}

	uint32_t getDisplay() const { return _display; }

	virtual bool handleOSEvents(std::unique_ptr<Shell>& shell) override;
	
	void setPointerXLocation(int32_t x)
	{
		_pointerXY[0] = x;
	}

	void setPointerYLocation(int32_t y)
	{
		_pointerXY[1] = y;
	}

	int32_t getPointerX() const { return _pointerXY[0]; }

	int32_t getPointerY() const { return _pointerXY[1]; }

private:
	uint32_t _display;
	std::vector<input_event> _keyboardEvents;
	int32_t _pointerXY[2];
	
	// Min/Max X/Y retrieved from input_absinfo populated using a call to ioctl
	int32_t _absX[2];
	int32_t _absY[2];
};

// Setup the capabilities.
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Unsupported, Capability::Unsupported };

ShellOS::ShellOS(OSApplication application, OSDATA osdata) : _instance(application) { _OSImplementation = std::make_unique<NullWSInternalOS>(this); }

ShellOS::~ShellOS() {}

void ShellOS::updatePointingDeviceLocation()
{
	_shell->updatePointerPosition(PointerLocation(static_cast<int16_t>(static_cast<NullWSInternalOS*>(_OSImplementation.get())->getPointerX()),
		static_cast<int16_t>(static_cast<NullWSInternalOS*>(_OSImplementation.get())->getPointerY())));
}

bool ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation) { return false; }

	return true;
}

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	data.fullscreen = true;
	data.x = data.y = 0;
	data.width = data.height = 0; // no way of getting the monitor resolution.
	_OSImplementation->setIsInitialized(true);
	return true;
}

void ShellOS::releaseWindow() { _OSImplementation->setIsInitialized(false); }

OSApplication ShellOS::getApplication() const { return _instance; }

OSConnection ShellOS::getConnection() const { return nullptr; }

OSDisplay ShellOS::getDisplay() const { return reinterpret_cast<OSDisplay>(static_cast<NullWSInternalOS*>(_OSImplementation.get())->getDisplay()); }

OSWindow ShellOS::getWindow() const { return nullptr; }

bool NullWSInternalOS::handleOSEvents(std::unique_ptr<Shell>& shell)
{
	bool result = InternalOS::handleOSEvents(shell);

	// Check input from the Keyboard
	if (KeyboardFileDescriptor > 0)
	{
		struct input_event keyEvent;

		// Keep reading input while there is new input available
		while (read(KeyboardFileDescriptor, &keyEvent, sizeof(struct input_event)) == sizeof(struct input_event))
		{
			// If the key event is type EV_KEY then add it to a list of key events
			// Don't process it just yet as there may be other events in the report
			if (keyEvent.type == EV_KEY) 
			{
				_keyboardEvents.push_back(keyEvent); 
			}

			// A SYN_REPORT event signals the end of a report, process all the
			// previously accumulated events.
			if (keyEvent.type == EV_SYN && keyEvent.code == SYN_REPORT)
			{
				for (const input_event& currentEvent : _keyboardEvents)
				{
					switch (currentEvent.value)
					{
					case 0:
					{
						// A value of 0 indicates a "key released" event.
						shell->onKeyUp(getKeyFromEVCode(currentEvent.code));
						break;
					}
					case 1:
					{
						// A value of 1 indicates a "key pressed" event.
						shell->onKeyDown(getKeyFromEVCode(currentEvent.code));
						break;
					}
					case 2:
					{
						// A value of 2 indicates a "key repeat" event.
						break;
					}
					default: {
					}
					}
				}

				// Discard all processed events.
				_keyboardEvents.clear();
			}
		}
	}

	// Check input from the Keypad
	if (TouchpadDeviceFileDescriptor > 0)
	{
		struct input_event keyEvent;

		// Keep reading input while there is new input available
		while (read(TouchpadDeviceFileDescriptor, &keyEvent, sizeof(struct input_event)) == sizeof(struct input_event))
		{
			// If the key event is type EV_KEY then add it to a list of key events
			// Don't process it just yet as there may be other events in the report
			if (keyEvent.type == EV_KEY) 
			{
				_keyboardEvents.push_back(keyEvent); 
			}

			// A SYN_REPORT event signals the end of a report, process all the
			// previously accumulated events.
			if (keyEvent.type == EV_SYN && keyEvent.code == SYN_REPORT)
			{
				for (const input_event& currentEvent : _keyboardEvents)
				{
					// BTN_TOUCH should not be considered for onPointingDeviceDown
					if(currentEvent.code == BTN_MOUSE)
					{
						if(currentEvent.value == 1)
						{
							shell->onPointingDeviceDown(0);
						}	
						else if(currentEvent.value == 0)
						{
							shell->onPointingDeviceUp(0);
						}
					}
					else
					{
						Keys key;
						switch (currentEvent.code)
						{
						case KEY_U:
						case KEY_F6:
						case KEY_ESC:
						case KEY_END: key = Keys::Escape; break;
						case KEY_ENTER: key = Keys::Space; break;
						case KEY_C:
						case KEY_1:
						case KEY_F1: key = Keys::Key1; break;
						case KEY_2:
						case KEY_F2: key = Keys::Key2; break;
						case KEY_UP: key = Keys::Up; break;
						case KEY_DOWN: key = Keys::Down; break;
						case KEY_LEFT: key = Keys::Left; break;
						case KEY_RIGHT: key = Keys::Right; break;
						default: key = Keys::Unknown; break;
						}
						if (key != Keys::Unknown) { currentEvent.value == 0 ? shell->onKeyUp(key) : shell->onKeyDown(key); }
					}
				}

				// Discard all processed events.
				_keyboardEvents.clear();
			}
		}
	}

	// Check input from the Touch screen
	if (TouchScreenDeviceFileDescriptor > 0)
	{
		struct input_event keyEvent;

		// Keep reading input while there is new input available
		while (read(TouchScreenDeviceFileDescriptor, &keyEvent, sizeof(struct input_event)) == sizeof(struct input_event))
		{
			// If the key event is type EV_KEY then add it to a list of key events
			// Don't process it just yet as there may be other events in the report
			if (keyEvent.type == EV_KEY) 
			{
				_keyboardEvents.push_back(keyEvent); 
			}
			else if (keyEvent.type == EV_ABS)
			{
				// Convert from absoluate min and max values for the touchscreen to pixel coordinates for the given surface
				if (keyEvent.code == ABS_X)
				{
					uint16_t displayX = (keyEvent.value - _absX[0]) * shell->getWidth() / (_absX[1] - _absX[0] + 1);
					setPointerXLocation(static_cast<int16_t>(displayX));
				}
				else if(keyEvent.code == ABS_Y)
				{
					uint16_t displayY = (keyEvent.value - _absY[0]) * shell->getHeight() / (_absY[1] - _absY[0] + 1);
					setPointerYLocation(static_cast<int16_t>(displayY));
				}
			}

			// A SYN_REPORT event signals the end of a report, process all the
			// previously accumulated events.
			if (keyEvent.type == EV_SYN && keyEvent.code == SYN_REPORT)
			{
				for (const input_event& currentEvent : _keyboardEvents)
				{
					if(currentEvent.code == BTN_TOUCH)
					{
						if(currentEvent.value == 1)
						{
							shell->onPointingDeviceDown(0);
						}	
						else if(currentEvent.value == 0)
						{
							shell->onPointingDeviceUp(0);
						}
					}
				}

				// Discard all processed events.
				_keyboardEvents.clear();
			}
		}
	}
	return result;
}

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
