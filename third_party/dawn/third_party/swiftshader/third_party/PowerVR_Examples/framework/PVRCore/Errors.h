#pragma once
#include <stdexcept>
#include <string>
#include <assert.h>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cstdio>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#define vsnprintf _vsnprintf
#endif

#if defined(__APPLE__)
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <signal.h>
#endif

#if defined(__linux__)
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#endif

/// <summary>Checks whether a debugger can be found for the current running process (on Windows and Linux only).
/// The prescene of a debugger can be used to provide additional helpful functionality for debugging application issues one of which could be to break in the
/// debugger when an exception is thrown. Being able to have the debugger break on such a thrown exception provides by far the most seamless and constructive environment for
/// fixing an issue causing the exception to be thrown due to the full state and stack trace being present at the point in which the issue has occurred rather
/// than relying on error logic handling.</summary>
/// <returns>True if a debugger can be found for the current running process else False.</returns>
inline static bool isDebuggerPresent()
{
	// only check once for whether the debugger is present as this may not be efficient to determine
	static bool isUsingDebugger = false;
	static bool haveCheckedForDebugger = false;
	if (!haveCheckedForDebugger)
	{
#if defined(_MSC_VER)
		if (IsDebuggerPresent()) { isUsingDebugger = true; }
#elif defined(__APPLE__)
		// reference implementation taken from: https: // developer.apple.com/library/archive/qa/qa1361/_index.html
		int junk;
		int mib[4];
		struct kinfo_proc info;
		size_t size;

		// Initialize the flags so that, if sysctl fails for some bizarre
		// reason, we get a predictable result.

		info.kp_proc.p_flag = 0;

		// Initialize mib, which tells sysctl the info we want, in this case
		// we're looking for information about a specific process ID.

		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PID;
		mib[3] = getpid();

		// Call sysctl.

		size = sizeof(info);
		junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
		assert(junk == 0);

		// We're being debugged if the P_TRACED flag is set.

		if ((info.kp_proc.p_flag & P_TRACED) != 0) { isUsingDebugger = true; }
#elif defined(__linux__)
		// reference implementation taken from: https://stackoverflow.com/a/24969863
		char buf[1024];

		int status_fd = open("/proc/self/status", O_RDONLY);
		if (status_fd == -1) { isUsingDebugger = false; }
		else
		{
			ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
			if (num_read > 0)
			{
				static const char TracerPid[] = "TracerPid:";
				char* tracer_pid;

				buf[num_read] = 0;
				tracer_pid = strstr(buf, TracerPid);
				if (tracer_pid) { isUsingDebugger = !!atoi(tracer_pid + sizeof(TracerPid) - 1); }
			}
		}
#endif
		haveCheckedForDebugger = true;
	}

	return isUsingDebugger;
}

/// <summary>If supported on the platform, makes the debugger break at this line. Used for Assertions on Visual Studio</summary>
inline void debuggerBreak()
{
	if (isDebuggerPresent())
	{
#if defined(__APPLE__)
		raise(SIGTRAP);
#elif defined(__linux__)
		{
			raise(SIGTRAP);
		}
#elif defined(_MSC_VER)
		__debugbreak();
#endif
	}
}

namespace pvr {

/// <summary>A simple std::runtime_error wrapper for throwing generic PVR exceptions</summary>
class PvrError : public std::runtime_error
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	PvrError(std::string message) : std::runtime_error(message) { debuggerBreak(); }
};
/// <summary>A simple std::runtime_error wrapper for throwing exceptions when invalid arguments are provided</summary>
class InvalidArgumentError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="argument">The invalid argument.</param>
	/// <param name="message">A message to log.</param>
	InvalidArgumentError(std::string argument, std::string message) : PvrError("Invalid Argument error:[" + argument + "] : " + message) {}
	/// <summary>Constructor.</summary>
	/// <param name="argument">The invalid argument.</param>
	InvalidArgumentError(std::string argument) : PvrError("Invalid Argument error:[" + argument + "]") {}
};
/// <summary>A simple std::runtime_error wrapper for throwing exceptions when unsupported operations are attempted</summary>
class UnsupportedOperationError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	UnsupportedOperationError(std::string message) : PvrError("UnsupportedOperationOerror (Operation not supported on this system) : " + message) {}
	/// <summary>Constructor.</summary>
	UnsupportedOperationError() : PvrError("UnsupportedOperationOerror (Operation not supported on this system)") {}
};
/// <summary>A simple std::runtime_error wrapper for throwing exceptions when invalid operations are attempted</summary>
class InvalidOperationError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	InvalidOperationError(std::string message) : PvrError("InvalidOperationError (Specified operation could not be performed) : " + message) {}
	/// <summary>Constructor.</summary>
	InvalidOperationError() : PvrError("Specified operation could not be performed on this object.") {}
};

/// <summary>A simple std::runtime_error wrapper for throwing exceptions when invalid operations are attempted</summary>
class TextureDecompressionError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	/// <param name="format">The source format of the texture decompression.</param>
	TextureDecompressionError(const std::string& message, const std::string& format) : PvrError("Texture Decompression to format [" + format + "] Failed:" + message) {}
	/// <summary>Constructor.</summary>
	/// <param name="format">The source format of the texture decompression.</param>
	TextureDecompressionError(const std::string& format) : PvrError("Texture Decompression to format [" + format + "] Failed") {}
};

/// <summary>A simple std::runtime_error wrapper for throwing exceptions when operations fail</summary>
class OperationFailedError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	OperationFailedError(std::string message) : PvrError("OperationFailedError (The requested operation failed to execute) : " + message) {}
	/// <summary>Constructor.</summary>
	OperationFailedError() : PvrError("OperationFailedError (The requested operation failed to execute).") {}
};
/// <summary>A simple std::runtime_error wrapper for throwing exceptions when invalid data is provided</summary>
class InvalidDataError : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	InvalidDataError(std::string message) : PvrError("[Invalid data provided]: " + message) {}
	/// <summary>Constructor.</summary>
	InvalidDataError() : PvrError("[Invalid data provided]") {}
};
/// <summary>A simple std::runtime_error wrapper for throwing exceptions when attempting to use index out of range</summary>
class IndexOutOfRange : public PvrError
{
public:
	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	IndexOutOfRange(std::string message) : PvrError("[Index was out of range]: " + message) {}

	/// <summary>Constructor.</summary>
	/// <param name="message">A message to log.</param>
	/// <param name="index">The index which was out of range.</param>
	/// <param name="maxIndex">The maximum index supported.</param>
	IndexOutOfRange(std::string message, size_t index, size_t maxIndex)
		: PvrError("[Index was out of range]: Index was [" + std::to_string(index) + "] while max index was [" + std::to_string(maxIndex) + "] - " + message)
	{}
	/// <summary>Constructor.</summary>
	IndexOutOfRange() : PvrError("[Index was out of range]") {}
};
} // namespace pvr
