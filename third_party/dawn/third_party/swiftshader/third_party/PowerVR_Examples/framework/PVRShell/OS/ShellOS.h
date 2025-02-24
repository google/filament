/*!
\brief Contains the declaration of the ShellOS class. Most of the functionality is platform-specific, and as such is
delegated to platform-specific ShellOS.cpp files. Do not access or use directly.
\file PVRShell/OS/ShellOS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/ShellData.h"
#include "PVRShell/Shell.h"
#include <memory>
namespace pvr {
namespace platform {
// Forward declaration of internal implementation.
class InternalOS;

/// <summary>Internal class Implements a lot of the functionality and forwards to the platform from PVRShell. Don't use directly,
/// instead use the pvr::Shell class.</summary>
class ShellOS
{
public:
	/// <summary>Capabilities that may be different between platforms.</summary>
	struct Capabilities
	{
		Capability resizable; //!< A window with this capability can be resized while the program is running (e.g windows, X11, but not Android)
		Capability movable; //!< A window with this capability can be moved while the program is running (e.g windows and X11, but not Android)
	};

	ShellData _shellData; //!< Platform specific shell data

	/// <summary>Constructor.</summary>
	/// <param name="instance">An OS Specific instance.</param>
	/// <param name="osdata">OS Specific data.</param>
	ShellOS(OSApplication instance, OSDATA osdata);

	/// <summary>Destructor.</summary>
	virtual ~ShellOS();

	/// <summary>Initializes the platform specific shell. Accepts a data struct so it can overide default values.</summary>
	/// <param name="data">A set of display attributes which will be set within this function.</param>
	/// <returns>True on success</returns>
	bool init(DisplayAttributes& data);

	/// <summary>Initializes the platform specific window.</summary>
	/// <param name="data">A set of display attributes which will be set within this function.</param>
	/// <returns>True on success</returns>
	bool initializeWindow(DisplayAttributes& data);

	/// <summary>Indicates whether the platform specific shell has been initiailised.</summary>
	/// <returns>True on success</returns>
	bool isInitialized();

	/// <summary>Releases the platform specific window.</summary>
	void releaseWindow();

	/// <summary>Retrieves the platform specific application.</summary>
	/// <returns>The OS specific application</returns>
	OSApplication getApplication() const;

	/// <summary>Retrieves the window specific connection.</summary>
	/// <returns>The window system specific connection</returns>
	OSConnection getConnection() const;

	/// <summary>Retrieves the platform specific display.</summary>
	/// <returns>The OS specific display</returns>
	OSDisplay getDisplay() const;

	/// <summary>Retrieves the platform specific window.</summary>
	/// <returns>The OS specific window</returns>
	OSWindow getWindow() const;

	/// <summary>Retrieves the default read path.</summary>
	/// <returns>The default read path</returns>
	const std::string& getDefaultReadPath() const;

	/// <summary>Retrieves the read paths.</summary>
	/// <returns>The read paths</returns>
	const std::vector<std::string>& getReadPaths() const;

	/// <summary>Retrieves the read paths.</summary>
	/// <param name="readPath">A new read path to add to the list of read paths.</param>
	void addReadPath(const std::string& readPath) { _readPaths.emplace_back(readPath); }

	/// <summary>Clears the read paths.</summary>
	void clearReadPaths() { _readPaths.clear(); }

	/// <summary>Retrieves the current write path.</summary>
	/// <returns>The write path</returns>
	const std::string& getWritePath() const;

	/// <summary>Sets the current write path.</summary>
	/// <param name="writePath">A new write path to use as the current write path.</param>
	void setWritePath(const std::string& writePath) { _writePath = writePath; }

	/// <summary>Retrieves the application name.</summary>
	/// <returns>The application name</returns>
	const std::string& getApplicationName() const;

	/// <summary>Sets the application name.</summary>
	/// <param name="applicationName">An application name.</param>
	void setApplicationName(const std::string& applicationName);

	/// <summary>Handles os specific events.</summary>
	/// <returns>True on success</returns>
	bool handleOSEvents();

	/// <summary>Retrieves OS specific capabilities.</summary>
	/// <returns>OS Specific capabilities</returns>
	static const Capabilities& getCapabilities() { return _capabilities; }

	/// <summary>Pops up a message.</summary>
	/// <param name="title">Specifies the title.</param>
	/// <param name="message">Specifies the message.</param>
	/// <param name="...">Variable arguments for the format std::string. Printf-style rules</param>
	/// <returns>True on success</returns>
	bool popUpMessage(const char* const title, const char* const message, ...) const;

	/// <summary>Retrieves the shell.</summary>
	/// <returns>A pointer to the shell</returns>
	Shell* getShell() { return _shell.get(); }

	/// <summary>Updates the location of the pointing device.</summary>
	void updatePointingDeviceLocation();

protected:
	std::unique_ptr<Shell> _shell; //!< A unique pointer to the shell
	std::string _appName; //!< The application name
	std::vector<std::string> _readPaths; //!< A list of read paths
	std::string _writePath; //!< The current write path

private:
	OSApplication _instance;
	std::unique_ptr<InternalOS> _OSImplementation;
	static const Capabilities _capabilities;
};
inline const std::string& ShellOS::getApplicationName() const { return _appName; }
inline void ShellOS::setApplicationName(const std::string& applicationName) { _appName = applicationName; }
inline const std::string& ShellOS::getDefaultReadPath() const { return _readPaths[0]; }
inline const std::vector<std::string>& ShellOS::getReadPaths() const { return _readPaths; }
inline const std::string& ShellOS::getWritePath() const { return _writePath; }
} // namespace platform
} // namespace pvr
