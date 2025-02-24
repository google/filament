/*!
\brief Contains the implementation for a common pvr::platform::InternalOS class which will be used for Linux window systems.
\file PVRShell/OS/Linux/InternalOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "InternalOS.h"

#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Log.h"

#include <linux/input.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <signal.h>

#if !defined(CONNAME)
#define CONNAME "/dev/tty"
#endif

namespace pvr {
namespace platform {

static struct termios OriginalTermio;
static int TtyFileDescriptor;

static struct sigaction OldSIGSEGVAction;
static struct sigaction OldSIGINTAction;
static struct sigaction OldSIGTERMAction;

// Forward declare functions used by the signal handler
static void restoreTtyState();
static void uninstallSignalHandlers();

// Provides a callback for particular signals on Linux platforms. This function is used specifically for resetting modified state
static void signalHandler(int sig, siginfo_t* si, void* ucontext)
{
	// Restore the tty to its original state
	restoreTtyState();

	// Uninstall the signal handlers (sigactions) and revert them back to their original state
	uninstallSignalHandlers();
}

static void restoreTtyState()
{
	// Reopen the tty so that we can restore the state
	if (!TtyFileDescriptor)
	{
		if ((TtyFileDescriptor = open(CONNAME, O_RDWR | O_NONBLOCK)) <= 0) { Log(LogLevel::Warning, "Unable to open '%s' for resetting attributes", CONNAME); }
	}

	// Recover tty state.
	if (tcsetattr(TtyFileDescriptor, TCSAFLUSH, &OriginalTermio) == -1) { Log(LogLevel::Error, "Unable to reset attributes for '%s'. Unable to recover the tty state", CONNAME); }
}

static void uninstallSignalHandlers()
{
	struct sigaction currentSignalAction;

	// SIGSEGV
	if (sigaction(SIGSEGV, nullptr, &currentSignalAction) == 0)
	{
		if (currentSignalAction.sa_sigaction == &signalHandler)
		{
			sigaction(SIGSEGV, &OldSIGSEGVAction, nullptr);
			memset(&OldSIGSEGVAction, 0, sizeof(OldSIGSEGVAction));
		}
	}

	// SIGINT
	if (sigaction(SIGINT, nullptr, &currentSignalAction) == 0)
	{
		if (currentSignalAction.sa_sigaction == &signalHandler)
		{
			sigaction(SIGINT, &OldSIGINTAction, nullptr);
			memset(&OldSIGINTAction, 0, sizeof(OldSIGINTAction));
		}
	}

	// SIGTERM
	if (sigaction(SIGTERM, nullptr, &currentSignalAction) == 0)
	{
		if (currentSignalAction.sa_sigaction == &signalHandler)
		{
			sigaction(SIGTERM, &OldSIGTERMAction, nullptr);
			memset(&OldSIGTERMAction, 0, sizeof(OldSIGTERMAction));
		}
	}
}

static void installSignalHandler()
{
	struct sigaction signalAction;
	struct sigaction currentSignalAction;

	memset(&signalAction, 0, sizeof(signalAction));

	signalAction.sa_sigaction = &signalHandler;
	signalAction.sa_flags = SA_RESETHAND;

	// SIGSEGV
	if (sigaction(SIGSEGV, nullptr, &currentSignalAction) != 0 || currentSignalAction.sa_sigaction != &signalHandler)
	{
		signalAction.sa_sigaction = &signalHandler;
		sigaction(SIGSEGV, &signalAction, &OldSIGSEGVAction);
	}

	// SIGINT
	if (sigaction(SIGINT, nullptr, &currentSignalAction) != 0 || currentSignalAction.sa_sigaction != &signalHandler)
	{
		sigdelset(&signalAction.sa_mask, SIGINT);
		signalAction.sa_sigaction = &signalHandler;
		sigaction(SIGINT, &signalAction, &OldSIGINTAction);
	}

	// SIGTERM
	if (sigaction(SIGTERM, nullptr, &currentSignalAction) != 0 || currentSignalAction.sa_sigaction != &signalHandler)
	{
		sigdelset(&signalAction.sa_mask, SIGTERM);
		signalAction.sa_sigaction = &signalHandler;
		sigaction(SIGTERM, &signalAction, &OldSIGTERMAction);
	}
}

InternalOS::InternalOS(ShellOS* shellOS) : _isInitialized(false), _shellOS(shellOS)
{
	OriginalTermio = { 0 };
	TtyFileDescriptor = 0;

	// Attempt to open the tty (the terminal connected to standard input) as read/write
	// Note that because O_NONBLOCK has been used termio.c_cc[VTIME] will be ignored so is not set
	if ((TtyFileDescriptor = open(CONNAME, O_RDWR | O_NONBLOCK)) <= 0) { Log(LogLevel::Warning, "Unable to open '%s'", CONNAME); }
	else
	{
		// Reads the current set of terminal attributes to the struct
		tcgetattr(TtyFileDescriptor, &OriginalTermio);

		// Ensure that on-exit the terminal state is restored as per the original termios structure retrieved above
		atexit(restoreTtyState);

		// Used as a modified set of termios attriutes
		struct termios termio;

		// Take a copy of the original termios structure.
		// The original set attributes will be modified and used as the source termios structure in a tcsetattr call to update the terminal attributes
		termio = OriginalTermio;

		// Enables raw mode (input is available character per character, echoing is disabled and special processing of terminal input and output characters is disabled)
		cfmakeraw(&termio);

		// Re-enable the use of Ctrl-C for sending a SIGINT signal to the current process causing it to terminate
		// Also re-enables Ctrl-Z which can be used to send a SIGTSTP signal.
		termio.c_lflag |= ISIG;

		// re-enable NL -> CR-NL expansion on output
		termio.c_oflag |= OPOST | ONLCR;

		// Set the minimum number of characters to read in bytes
		termio.c_cc[VMIN] = 1;

		// Update the attributes of the current terminal
		if (tcsetattr(TtyFileDescriptor, TCSANOW, &termio) == -1) { Log(LogLevel::Error, "Unable to set attributes for '%s'", CONNAME); }

		Log(LogLevel::Information, "Opened '%s' for input", CONNAME);
	}

	// Restore the terminal console on SIGINT, SIGSEGV and SIGABRT
	installSignalHandler();

	{
		// Construct our read paths and write path
		std::string selfProc = "/proc/self/exe";

		ssize_t exePathLength(PATH_MAX);
		std::string exePath(exePathLength, 0);
		exePathLength = readlink(selfProc.c_str(), &exePath[0], exePathLength - 1);
		if (exePathLength == -1) { Log(LogLevel::Warning, "Readlink %s failed. The application name, read path and write path have not been set", selfProc.c_str()); }
		else
		{
			// Resize the executable path based on the result of readlink
			exePath.resize(exePathLength);

			Log(LogLevel::Debug, "Found executable path: '%s'", exePath.c_str());

			FilePath filepath(exePath);
			_shellOS->setApplicationName(filepath.getFilenameNoExtension());
			_shellOS->setWritePath(filepath.getDirectory() + FilePath::getDirectorySeparator());
			_shellOS->clearReadPaths();
			_shellOS->addReadPath(filepath.getDirectory() + FilePath::getDirectorySeparator());
			_shellOS->addReadPath(std::string(".") + FilePath::getDirectorySeparator());
			_shellOS->addReadPath(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets" + FilePath::getDirectorySeparator());
			_shellOS->addReadPath(filepath.getDirectory() + FilePath::getDirectorySeparator() + "Assets_" + filepath.getFilenameNoExtension() + FilePath::getDirectorySeparator());
		}
	}
}

InternalOS::~InternalOS()
{
	if (TtyFileDescriptor)
	{
		close(TtyFileDescriptor);
		TtyFileDescriptor = 0;
	}

	uninstallSignalHandlers();
}

Keys InternalOS::getSpecialKey(unsigned char firstCharacter) const
{
	uint32_t numCharacters = 0;
	unsigned char currentKey = 0;
	// Escape keys will input up to 4 characters including the first character to the terminal i.e. 3 additonal character bytes
	// rfc3629 states that the maximum number of bytes per character is 4: https://tools.ietf.org/html/rfc3629
	const unsigned int maxNumExtraCharacterBytes = 3;
	// Leave room at the end of the buffer for a null terminator
	unsigned char buf[maxNumExtraCharacterBytes + 1];

	// Read until we have the full key.
	while ((read(TtyFileDescriptor, &currentKey, 1) == 1) && numCharacters < maxNumExtraCharacterBytes) { buf[numCharacters++] = currentKey; }
	buf[numCharacters] = '\0';

	uint32_t pos = 0;

	// Find the matching special key from the map.
	while (ASCIISpecialKeyMap[pos].str != NULL)
	{
		if (!strcmp(ASCIISpecialKeyMap[pos].str, (const char*)buf)) { return ASCIISpecialKeyMap[pos].key; }
		pos++;
	}

	// If additional character bytes have been retrieved and a matching key has not been found there has been an unrecognised special key read.
	if (numCharacters > 0) { return Keys::Unknown; }
	else
	// No additional character bytes have been retrieved meaning it must just be the first character - a key corresponding to an escape key
	{
		return ASCIIStandardKeyMap[firstCharacter];
	}
}

Keys InternalOS::getKeyFromAscii(unsigned char initialKey)
{
	Keys key = Keys::Unknown;

	if (initialKey < ARRAY_SIZE(KeyboardKeyMap))
	{
		// Check for special multi-character keys
		// CHeck for escape sequences (they start with a '27' byte which matches the escape key)
		if ((ASCIIStandardKeyMap[initialKey] == Keys::Escape) || (ASCIIStandardKeyMap[initialKey] == Keys::F7)) { key = getSpecialKey(initialKey); }
		else
		{
			key = ASCIIStandardKeyMap[initialKey];
		}
	}

	return key;
}

Keys InternalOS::getKeyFromEVCode(uint32_t keycode)
{
	if (keycode >= ARRAY_SIZE(KeyboardKeyMap)) { return Keys::Unknown; }
	return KeyboardKeyMap[keycode];
}

bool InternalOS::handleOSEvents(std::unique_ptr<Shell>& shell)
{
	// Check user input from the available input devices.

	// Check input from tty
	if (TtyFileDescriptor > 0)
	{
		unsigned char initialKey = 0;

		// Read a single byte using the TTY file descriptor
		int bytesRead = read(TtyFileDescriptor, &initialKey, 1);
		Keys key = Keys::Unknown;

		// If the number of bytes is not 0 and the initial key has been set then determine the corresponding key
		if ((bytesRead > 0) && initialKey) { key = getKeyFromAscii(initialKey); }
		shell->onKeyDown(key);
		shell->onKeyUp(key);
	}

	return true;
}
} // namespace platform
} // namespace pvr
//!\endcond
