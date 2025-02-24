/*!
\brief Provides a class representing a Filepath and common manipulating functions
\file PVRCore/stream/FilePath.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <cstdint>
#include <string>
#include <limits>

namespace pvr {
/// <summary>Filepath represents a Path + Filename + Extension.</summary>
class FilePath : public std::string
{
public:
	/// <summary>Creates an empty Filepath object.</summary>
	FilePath() {}

	/// <summary>Creates an Filepath object from a filename.</summary>
	/// <param name="str">Creates a filepath object of this path.</param>
	FilePath(const std::string& str) : std::string(str) {}

	/// <summary>Get a std::string containing the Extension of the filepath.</summary>
	/// <returns>The file extension of the path</returns>
	std::string getFileExtension() const
	{
		size_t index = find_last_of(c_extensionSeparator, length());

		if (index != std::string::npos) { return substr((index + 1), length() - (index + 1)); }

		return std::string();
	}

	/// <summary>Get a std::string containing the Directory of the filepath.</summary>
	/// <returns>The directory part of the path</returns>
	std::string getDirectory() const
	{
		const std::string::size_type c_objectNotFound = std::numeric_limits<std::string::size_type>::max();

		std::string::size_type index = static_cast<std::string::size_type>(find_last_of(c_unixDirectorySeparator, length()));

#if defined(_WIN32)
		if (index == std::string::npos) { index = static_cast<std::string::size_type>(find_last_of(c_windowsDirectorySeparator, length())); }
#endif

		if (index != c_objectNotFound) { return substr(0, index); }

		return std::string();
	}

	/// <summary>Get a std::string containing the Filename+Extension of the filepath.</summary>
	/// <returns>The filename part of the path (including the extension)</returns>
	std::string getFilename() const
	{
		std::string::size_type index = static_cast<std::string::size_type>(find_last_of(c_unixDirectorySeparator, length()));

#if defined(_WIN32)
		if (index == std::string::npos) { index = static_cast<std::string::size_type>(find_last_of(c_windowsDirectorySeparator, length())); }
#endif

		if (index != std::string::npos) { return substr((index + 1), length() - (index + 1)); }

		return *this;
	}

	/// <summary>Get a std::string containing the Filename (without extension) of the filepath.</summary>
	/// <returns>The filename part of the path (without the extension)</returns>
	std::string getFilenameNoExtension() const
	{
		std::string::size_type extensionIndex = static_cast<std::string::size_type>(getFilename().find_last_of(c_extensionSeparator, length()));

		if (extensionIndex != std::string::npos) { return getFilename().substr(0, extensionIndex); }

		return getFilename();
	}

	/// <summary>Get the directory separator used by the current platform.</summary>
	/// <returns>The platform specific directory separator (usually "\\" or "/"</returns>
	static char getDirectorySeparator()
	{
#if defined(WIN32) || defined(_WIN32)
		return c_windowsDirectorySeparator;
#else
		return c_unixDirectorySeparator;
#endif
	}

private:
	static const char c_unixDirectorySeparator = '/';
	static const char c_windowsDirectorySeparator = '\\';
	static const char c_extensionSeparator = '.';
};
} // namespace pvr
