/*!
\brief Contains the implementation for the pvr::platform::ShellOS class on Null Windowing System platforms (QNX).
\file PVRShell/OS/QNXNullWS/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/procfs.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <termios.h>

#if !defined(CONNAME)
#define CONNAME "/dev/tty"
#endif

#if !defined(KEYPAD_INPUT)
#define KEYPAD_INPUT "/dev/con1"
#endif

#if !defined(FBNAME)
#define FBNAME "/dev/fb0"
#endif

#if !defined(REMOTE)
#define REMOTE "/dev/ttyS1"
#endif

namespace pvr {
namespace platform {
class InternalOS
{
public:
	bool isInitialized;
	uint32_t display;

	int devfd;
	struct termios termio;
	struct termios termio_orig;
	int keypad_fd;
	int keyboard_fd;
	bool keyboardShiftHeld; // Is one of the shift keys on the keyboard being held down? (Keyboard device only - not terminal).

	InternalOS() : isInitialized(false), display(0), devfd(0), keypad_fd(0), keyboard_fd(0), keyboardShiftHeld(false) {}

	~InternalOS()
	{
		//#if defined(__linux__)
		// Recover tty state.
		tcsetattr(devfd, TCSANOW, &termio_orig);
		//#endif
	}

	//#if defined(__linux__)
	Keys getSpecialKey(Keys firstCharacter) const;
	//#endif
};

// Setup the capabilities.
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Unsupported, Capability::Unsupported };

ShellOS::ShellOS(OSApplication application, OSDATA osdata) : _instance(application) { _OSImplementation = std::make_unique<InternalOS>(); }

ShellOS::~ShellOS() {}

void ShellOS::updatePointingDeviceLocation()
{
	static bool runOnlyOnce = true;
	if (runOnlyOnce)
	{
		_shell->updatePointerPosition(PointerLocation(0, 0));
		runOnlyOnce = false;
	}
}

bool ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation) { return false; }

	//#if defined(__linux__)
	// In case we're in the background ignore SIGTTIN and SIGTTOU.
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	// Keyboard handling.
	if ((_OSImplementation->devfd = open(CONNAME, O_RDWR | O_NDELAY)) <= 0) { Log(LogLevel::Warning, "Can't open tty '" CONNAME "'"); }
	else
	{
		tcgetattr(_OSImplementation->devfd, &_OSImplementation->termio_orig);
		tcgetattr(_OSImplementation->devfd, &_OSImplementation->termio);
		cfmakeraw(&_OSImplementation->termio);
		_OSImplementation->termio.c_oflag |= OPOST | ONLCR; // Turn back on cr-lf expansion on output
		_OSImplementation->termio.c_cc[VMIN] = 1;
		_OSImplementation->termio.c_cc[VTIME] = 0;

		if (tcsetattr(_OSImplementation->devfd, TCSANOW, &_OSImplementation->termio) == -1) { Log(LogLevel::Warning, "Can't set tty attributes for '" CONNAME "'"); }
	}

	// Keypad handling.
	if ((_OSImplementation->keypad_fd = open(KEYPAD_INPUT, O_RDONLY | O_NDELAY)) <= 0) { Log(LogLevel::Warning, "Can't open keypad input device (%s)\n", KEYPAD_INPUT); }

	// Keyboard handling.
	// Get keyboard device file name.
	// Uses the fact that all device information is shown in /proc/bus/input/devices and
	// the keyboard device file should always have an EV of 120013.
	static const char* command = "grep -E 'Handlers|EV' /proc/bus/input/devices |"
								 "grep -B1 120013 |"
								 "grep -Eo event[0-9]+ |"
								 "tr '\\n' '\\0'";

	FILE* pipe = popen(command, "r");
	if (pipe == NULL) { Log(LogLevel::Warning, "Can't find keyboard input device\n"); }

	char devFilePath[20] = "/dev/input/";
	char temp[9];
	if (fgets(temp, 9, pipe))
		;
	pclose(pipe);

	static char* keyboardDeviceFileName = strdup(strcat(devFilePath, temp));

	_OSImplementation->keyboard_fd = open(keyboardDeviceFileName, O_RDONLY | O_NDELAY);
	if (_OSImplementation->keyboard_fd <= 0)
	{ Log(LogLevel::Warning, "Can't open keyboard input device (%s)  -- (Code : %i - %s)\n", keyboardDeviceFileName, errno, strerror(errno)); } // Construct our read and write path.
	pid_t ourPid = getpid();
	char *exePath, srcLink[64];
	int len = 256;
	int res;
	FILE* fp;

	sprintf(srcLink, "/proc/%d/exefile", ourPid);
	exePath = 0;

	do
	{
		len *= 2;
		delete[] exePath;
		exePath = new char[len];
		fp = fopen(srcLink, "r");
		fgets(exePath, len, fp);
		res = strlen(exePath);
		fclose(fp);

		if (res < 0)
		{
			Log(LogLevel::Warning, "Readlink %s failed. The application name, read path and write path have not been set.\n", exePath);
			break;
		}
	} while (res >= len);

	if (res >= 0)
	{
		exePath[res] = '\0'; // Null-terminate readlink's result.
		FilePath filepath(exePath);
		setApplicationName(filepath.getFilenameNoExtension());
		_writePath = filepath.getDirectory() + FilePath::getDirectorySeparator();
		_readPaths.clear();
		_readPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator());
		_readPaths.push_back(std::string(".") + FilePath::getDirectorySeparator());
		_readPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets" + FilePath::getDirectorySeparator());
		_readPaths.push_back(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets_" + filepath.getFilenameNoExtension() + FilePath::getDirectorySeparator());
	}

	delete[] exePath;

	/*
	 Get rid of the blinking cursor on a screen.

	 It's an equivalent of:
	 echo -n -e "\033[?25l" > /dev/tty0

	 if you do the above command then you can undo it with:
	 echo -n -e "\033[?25h" > /dev/tty0
	 */
	FILE* tty = 0;
	tty = fopen("/dev/tty0", "w");
	if (tty != 0)
	{
		const char txt[] = { 27 /* the ESCAPE ASCII character */
			,
			'[', '?', '2', '5', 'l', 0 };

		fprintf(tty, "%s", txt);
		fclose(tty);
	}
	//#endif
	return true;
}

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	_OSImplementation->isInitialized = true;
	data.fullscreen = true;
	data.x = data.y = 0;
	data.width = data.height = 0; // no way of getting the monitor resolution.
	return true;
}

void ShellOS::releaseWindow()
{
	_OSImplementation->isInitialized = false;

	close(_OSImplementation->keyboard_fd);
	close(_OSImplementation->keypad_fd);
}

OSConnection ShellOS::getConnection() const { return nullptr; }

OSApplication ShellOS::getApplication() const { return _instance; }

OSDisplay ShellOS::getDisplay() const { return reinterpret_cast<OSDisplay>(_OSImplementation->display); }

OSWindow ShellOS::getWindow() const { return NULL; }

static Keys terminalStandardKeyMap[128] = {
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 0   */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Backspace, Keys::Tab, /* 5   */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Return, Keys::Unknown, /* 10  */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 15  */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 20  */
	Keys::Unknown, Keys::Unknown, Keys::Escape, Keys::Unknown, Keys::Unknown, /* 25  */
	Keys::Unknown, Keys::Unknown, Keys::Space, Keys::Key1, Keys::Quote, /* 30  */
	Keys::Backslash, Keys::Key4, Keys::Key5, Keys::Key7, Keys::Quote, /* 35  */
	Keys::Key9, Keys::Key0, Keys::NumMul, Keys::NumAdd, Keys::Comma, /* 40  */
	Keys::Minus, Keys::Period, Keys::Slash, Keys::Key0, Keys::Key1, /* 45  */
	Keys::Key2, Keys::Key3, Keys::Key4, Keys::Key5, Keys::Key6, /* 50  */
	Keys::Key7, Keys::Key8, Keys::Key9, Keys::Semicolon, Keys::Semicolon, /* 55  */
	Keys::Comma, Keys::Equals, Keys::Period, Keys::Slash, Keys::Key2, /* 60  */
	Keys::A, Keys::B, Keys::C, Keys::D, Keys::E, /* upper case */ /* 65  */
	Keys::F, Keys::G, Keys::H, Keys::I, Keys::J, /* 70  */
	Keys::K, Keys::L, Keys::M, Keys::N, Keys::O, /* 75  */
	Keys::P, Keys::Q, Keys::R, Keys::S, Keys::T, /* 80  */
	Keys::U, Keys::V, Keys::W, Keys::X, Keys::Y, /* 85  */
	Keys::Z, Keys::SquareBracketLeft, Keys::Backslash, Keys::SquareBracketRight, Keys::Key6, /* 90  */
	Keys::Minus, Keys::Backquote, Keys::A, Keys::B, Keys::C, /* 95  */
	Keys::D, Keys::E, Keys::F, Keys::G, Keys::H, /* lower case */ /* 100 */
	Keys::I, Keys::J, Keys::K, Keys::L, Keys::M, /* 105 */
	Keys::N, Keys::O, Keys::P, Keys::Q, Keys::R, /* 110 */
	Keys::S, Keys::T, Keys::U, Keys::V, Keys::W, /* 115 */
	Keys::X, Keys::Y, Keys::Z, Keys::SquareBracketLeft, Keys::Backslash, /* 120 */
	Keys::SquareBracketRight, Keys::Backquote, Keys::Backspace /* 125 */
};

struct SpecialKeyCode
{
	const char* str;
	Keys key;
};

// Some codes for F-keys can differ, depending on whether we are reading a
// /dev/tty from within X or from a text console.
// Some keys (e.g. Home, Delete) have multiple codes
// one for the standard version and one for the numpad version.
SpecialKeyCode terminalSpecialKeyMap[] = { { "[A", Keys::Up }, { "[B", Keys::Down }, { "[C", Keys::Right }, { "[D", Keys::Left },

	{ "[E", Keys::Key5 }, // Numpad 5 has no second function - do this to avoid the code being interpreted as Escape.

	{ "OP", Keys::F1 }, /* Within X */
	{ "[[A", Keys::F1 }, /* Text console */
	{ "OQ", Keys::F2 }, /* Within X */
	{ "[[B", Keys::F2 }, /* Text console */
	{ "OR", Keys::F3 }, /* Within X */
	{ "[[C", Keys::F3 }, /* Text console */
	{ "OS", Keys::F4 }, /* Within X */
	{ "[[D", Keys::F4 }, /* Text console */
	{ "[15~", Keys::F5 }, /* Within X */
	{ "[[E", Keys::F5 }, /* Text console */
	{ "[17~", Keys::F6 }, { "[18~", Keys::F7 }, { "[19~", Keys::F8 }, { "[20~", Keys::F9 }, { "[21~", Keys::F10 }, { "[23~", Keys::F11 }, { "[24~", Keys::F12 },

	{ "[1~", Keys::Home }, { "OH", Keys::Home }, { "[2~", Keys::Insert }, { "[3~", Keys::Delete }, { "[4~", Keys::End }, { "OF", Keys::End }, { "[5~", Keys::PageUp },
	{ "[6~", Keys::PageDown }, { NULL, Keys::Unknown } };

static const char* keyboardEventTypes[] = { "released", "pressed", "held" };

static Keys keyboardKeyMap[] = { Keys::Unknown, Keys::Escape, Keys::Key1, Keys::Key2, Keys::Key3, Keys::Key4, Keys::Key5, Keys::Key6, Keys::Key7, Keys::Key8, Keys::Key9,
	Keys::Key0, Keys::Minus, Keys::Equals, Keys::Backspace, Keys::Tab, Keys::Q, Keys::W, Keys::E, Keys::R, Keys::T, Keys::Y, Keys::U, Keys::I, Keys::O, Keys::P,
	Keys::SquareBracketLeft, Keys::SquareBracketRight, Keys::Return, Keys::Control, Keys::A, Keys::S, Keys::D, Keys::F, Keys::G, Keys::H, Keys::J, Keys::K, Keys::L,
	Keys::Semicolon, Keys::Quote, Keys::Backquote, Keys::Shift, Keys::Backslash, Keys::Z, Keys::X, Keys::C, Keys::V, Keys::B, Keys::N, Keys::M, Keys::Comma, Keys::Period,
	Keys::Slash, Keys::Shift, Keys::NumMul, Keys::Alt, Keys::Space, Keys::CapsLock, Keys::F1, Keys::F2, Keys::F3, Keys::F4, Keys::F5, Keys::F6, Keys::F7, Keys::F8, Keys::F9,
	Keys::F10, Keys::NumLock, Keys::ScrollLock, Keys::Num7, Keys::Num8, Keys::Num9, Keys::NumSub, Keys::Num4, Keys::Num5, Keys::Num6, Keys::NumAdd, Keys::Num1, Keys::Num2,
	Keys::Num3, Keys::Num0, Keys::NumPeriod, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::F11, Keys::F12, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown,
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Return, Keys::Control, Keys::NumDiv, Keys::PrintScreen, Keys::Alt, Keys::Unknown, Keys::Home, Keys::Up, Keys::PageUp,
	Keys::Left, Keys::Right, Keys::End, Keys::Down, Keys::PageDown, Keys::Insert, Keys::Delete };

static Keys keyboardShiftedKeyMap[] = { Keys::Unknown, Keys::Escape, Keys::Key1, Keys::Key2, Keys::Backslash, Keys::Key4, Keys::Key5, Keys::Key6, Keys::Key7, Keys::Key8,
	Keys::Key9, Keys::Key0, Keys::Minus, Keys::Equals, Keys::Backspace, Keys::Tab, Keys::Q, Keys::W, Keys::E, Keys::R, Keys::T, Keys::Y, Keys::U, Keys::I, Keys::O, Keys::P,
	Keys::SquareBracketLeft, Keys::SquareBracketRight, Keys::Return, Keys::Control, Keys::A, Keys::S, Keys::D, Keys::F, Keys::G, Keys::H, Keys::J, Keys::K, Keys::L,
	Keys::Semicolon, Keys::Quote, Keys::Backquote, Keys::Shift, Keys::Backslash, Keys::Z, Keys::X, Keys::C, Keys::V, Keys::B, Keys::N, Keys::M, Keys::Comma, Keys::Period,
	Keys::Slash, Keys::Shift, Keys::NumMul, Keys::Alt, Keys::Space, Keys::CapsLock, Keys::F1, Keys::F2, Keys::F3, Keys::F4, Keys::F5, Keys::F6, Keys::F7, Keys::F8, Keys::F9,
	Keys::F10, Keys::NumLock, Keys::ScrollLock, Keys::Num7, Keys::Num8, Keys::Num9, Keys::NumSub, Keys::Num4, Keys::Num5, Keys::Num6, Keys::NumAdd, Keys::Num1, Keys::Num2,
	Keys::Num3, Keys::Num0, Keys::NumPeriod, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::F11, Keys::F12, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown,
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Return, Keys::Control, Keys::NumDiv, Keys::PrintScreen, Keys::Alt, Keys::Unknown, Keys::Home, Keys::Up, Keys::PageUp,
	Keys::Left, Keys::Right, Keys::End, Keys::Down, Keys::PageDown, Keys::Insert, Keys::Delete };

Keys InternalOS::getSpecialKey(Keys firstCharacter) const
{
	int len = 0, pos = 0;
	unsigned char key, buf[6];

	// Read until we have the full key.
	while ((read(devfd, &key, 1) == 1) && len < 6) { buf[len++] = key; }
	buf[len] = '\0';

	// Find the matching special key from the map.
	while (terminalSpecialKeyMap[pos].str != NULL)
	{
		if (!strcmp(terminalSpecialKeyMap[pos].str, (const char*)buf)) { return (terminalSpecialKeyMap[pos].key); }
		pos++;
	}

	if (len > 0)
	// Unrecognised special key read.
	{
		return (Keys::Unknown);
	}
	else
	// No special key read, so must just be the first character.
	{
		return (firstCharacter);
	}
}

bool ShellOS::handleOSEvents()
{
	// Check user input from the available input devices.

	//#if defined(__linux__)
	// Terminal
	if (_OSImplementation->devfd > 0)
	{
		unsigned char initialKey;
		int bytesRead = read(_OSImplementation->devfd, &initialKey, 1);
		Keys key = Keys::Unknown;
		if ((bytesRead > 0) && initialKey)
		{
			// Check for special multi-character key - (first character is Escape or F7).
			if ((terminalStandardKeyMap[initialKey] == Keys::Escape) || (terminalStandardKeyMap[initialKey] == Keys::F7))
			{ key = _OSImplementation->getSpecialKey(terminalStandardKeyMap[initialKey]); } else
			{
				key = terminalStandardKeyMap[initialKey];
			}
		}
		_shell->onKeyDown(key);
		_shell->onKeyUp(key);
	}

	// Keyboard
	if (_OSImplementation->keyboard_fd > 0)
	{
		struct input_event
		{
			struct timeval time;
			unsigned short type;
			unsigned short code;
			unsigned int value;
		} keyinfo;

		int bytes = read(_OSImplementation->keyboard_fd, &keyinfo, sizeof(struct input_event));

		if ((bytes == sizeof(struct input_event)) && (keyinfo.type == 0x01))
		{
			// Update shift key status.
			if (keyboardKeyMap[keyinfo.code] == Keys::Shift) { _OSImplementation->keyboardShiftHeld = (keyinfo.value > 0); }

			// Select standard or shifted key map.
			Keys* keyMap = _OSImplementation->keyboardShiftHeld ? keyboardShiftedKeyMap : keyboardKeyMap;

			if (keyinfo.value == 0) { _shell->onKeyUp(keyMap[keyinfo.code]); }
			else
			{
				_shell->onKeyDown(keyMap[keyinfo.code]);
			}
		}
	}

	// Keypad
	if (_OSImplementation->keypad_fd > 0)
	{
		struct input_event
		{
			struct timeval time;
			unsigned short type;
			unsigned short code;
			unsigned int value;
		} keyinfo;

		int bytes = read(_OSImplementation->keypad_fd, &keyinfo, sizeof(struct input_event));

		if (bytes == sizeof(struct input_event) && keyinfo.type == 0x01)
		{
			Keys key;
			switch (keyinfo.code)
			{
			case 22:
			case 64:
			case 107: key = Keys::Escape; break; // End call button on Zoom2
			case 28: key = Keys::Space; break; // Old Select
			case 46:
			case 59: key = Keys::Key1; break;
			case 60: key = Keys::Key2; break;
			case 103: key = Keys::Up; break;
			case 108: key = Keys::Down; break;
			case 105: key = Keys::Left; break;
			case 106: key = Keys::Right; break;
			default: key = Keys::Unknown; break;
			}
			if (key != Keys::Unknown) { keyinfo.value == 0 ? _shell->onKeyUp(key) : _shell->onKeyDown(key); }
		}
	}

	return true;
}

bool ShellOS::isInitialized() { return _OSImplementation && _OSImplementation->isInitialized; }

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