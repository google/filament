#pragma once
#include <string>
#include <cstring>
#include <cstdio>

#if defined(__linux__) || defined(__ANDROID__) || defined(__QNXNTO__) || defined(__APPLE__)
#include <unistd.h>
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <tchar.h>
#include <Winbase.h>
#endif

#if defined(_PVR_LOG_H)
#define Log_Info(...) ((void)Log(LogLevel::Information, __VA_ARGS__))
#define Log_Warning(...) ((void)Log(LogLevel::Warning, __VA_ARGS__))
#define Log_Error(...) ((void)Log(LogLevel::Error, __VA_ARGS__))
#else
#if defined(__ANDROID__)
#define _ANDROID 1
#include <android/log.h>
#define Log_Info(...) ((void)__android_log_print(ANDROID_LOG_INFO, "com.imgtec", __VA_ARGS__))
#define Log_Warning(...) ((void)__android_log_print(ANDROID_LOG_WARN, "com.imgtec", __VA_ARGS__))
#define Log_Error(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "com.imgtec", __VA_ARGS__))
#elif defined(_WIN32)
static const char* procAddressMessageTypes[] = {
	"INFORMATION: ",
	"WARNING:",
	"ERROR: ",
};

inline void logOutput(const int logLevel, const char* const formatString, va_list argumentList)
{
	static char buffer[4096];
	va_list tempList;
	memset(buffer, 0, sizeof(buffer));

#if (defined _MSC_VER) // Pre VS2013
	tempList = argumentList;
#else
	va_copy(tempList, argumentList);
#endif

	vsnprintf(buffer, 4095, formatString, argumentList);

#if defined(_WIN32) && !defined(_CONSOLE)
	if (IsDebuggerPresent())
	{
		OutputDebugString(procAddressMessageTypes[logLevel]);
		OutputDebugString(buffer);
		OutputDebugString("\n");
	}
#else
	printf("%s", procAddressMessageTypes[logLevel]);
	vprintf(formatString, tempList);
	printf("\n");
#endif
}

inline void Log_Info(const char* const formatString, ...)
{
	va_list argumentList;
	va_start(argumentList, formatString);
	logOutput(0, formatString, argumentList);
	va_end(argumentList);
}
inline void Log_Warning(const char* const formatString, ...)
{
	va_list argumentList;
	va_start(argumentList, formatString);
	logOutput(1, formatString, argumentList);
	va_end(argumentList);
}
inline void Log_Error(const char* const formatString, ...)
{
	va_list argumentList;
	va_start(argumentList, formatString);
	logOutput(2, formatString, argumentList);
	va_end(argumentList);
}
#else
#define Log_Info(...) ((void)printf(__VA_ARGS__))
#define Log_Warning(...) ((void)fprintf(stderr, __VA_ARGS__))
#define Log_Error(...) ((void)fprintf(stderr, __VA_ARGS__))
#endif
#endif

/** ABSTRACT THE PLATFORM SPEFCIFIC LIBRARY LOADING FUNCTIONS **/
#if defined(_WIN32)

namespace pvr {
namespace lib {
typedef HINSTANCE LIBTYPE;
}
} // namespace pvr

namespace pvr {
namespace internal {
inline pvr::lib::LIBTYPE OpenLibrary(const char* pszPath)
{
#if defined(_UNICODE) // UNDER_CE
	if (!pszPath)
	{
		Log_Error("Path must be valid '%s'", pszPath);
		return nullptr;
	}

	// Get full path of executable
	wchar_t pszPathW[_MAX_PATH];

	// Convert char to wchar
	DWORD i = 0;

	for (i = 0; i <= strlen(pszPath); ++i) { pszPathW[i] = static_cast<wchar_t>(pszPath[i]); }

	pszPathW[i] = ' ';
	pvr::lib::LIBTYPE hostLib = LoadLibraryW(pszPathW);
#else
	pvr::lib::LIBTYPE hostLib = LoadLibraryA(pszPath);
#endif
	if (!hostLib) { Log_Error("Could not load host library '%s'", pszPath); }
	Log_Info("Host library '%s' loaded", pszPath);
	return hostLib;
}

inline void CloseLibrary(pvr::lib::LIBTYPE hostLib) { FreeLibrary(hostLib); }

inline void* getLibraryFunction(pvr::lib::LIBTYPE hostLib, const char* pszName)
{
	if (hostLib)
	{
		return reinterpret_cast<void*>(GetProcAddress(hostLib, pszName));
	}
	return nullptr;
}
} // namespace internal
} // namespace pvr
#elif defined(__linux__) || defined(__QNXNTO__) || defined(__APPLE__)
#if defined(__APPLE__)
#if !TARGET_OS_IPHONE
#include "CoreFoundation/CoreFoundation.h"
static const char* g_pszEnvVar = "PVRTRACE_LIB_PATH";
inline void* OpenFramework(const char* pszPath)
{
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourceURL = CFBundleCopyPrivateFrameworksURL(mainBundle);
	char path[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourceURL, TRUE, (UInt8*)path, PATH_MAX)) { return 0; }
	CFRelease(resourceURL);

	{
		void* lib = NULL;

		// --- Set a global environment variable to point to this path (for VFrame usage)
		const char* slash = strrchr(pszPath, '/');
		if (slash)
		{
			char szPath[FILENAME_MAX];
			memset(szPath, 0, sizeof(szPath));
			strncpy(szPath, pszPath, slash - pszPath);
			setenv(g_pszEnvVar, szPath, 1);
		}
		else
		{
			// Use the current bundle path
			std::string framework = std::string(path) + "/../Frameworks/";
			setenv(g_pszEnvVar, framework.c_str(), 1);
		}

		// --- Make a temp symlink
		char szTempFile[FILENAME_MAX];
		memset(szTempFile, 0, sizeof(szTempFile));

		char tmpdir[PATH_MAX];
		size_t n = confstr(_CS_DARWIN_USER_TEMP_DIR, tmpdir, sizeof(tmpdir));
		if ((n <= 0) || (n >= sizeof(tmpdir))) { strlcpy(tmpdir, getenv("TMPDIR"), sizeof(tmpdir)); }

		strcat(szTempFile, tmpdir);
		strcat(szTempFile, "tmp.XXXXXX");

		if (mkstemp(szTempFile))
		{
			if (symlink(pszPath, szTempFile) == 0)
			{
				lib = dlopen(szTempFile, RTLD_LAZY | RTLD_GLOBAL);
				remove(szTempFile);
			}
		}

		// --- Can't find the lib? Check the application framework folder instead.
		if (!lib)
		{
			std::string framework = std::string(path) + std::string("/") + pszPath;
			lib = dlopen(framework.c_str(), RTLD_LAZY | RTLD_GLOBAL);

			if (!lib)
			{
				const char* err = dlerror();
				if (err)
				{
					// NSLog(@"dlopen failed with error: %s => %@", err, framework);
				}
			}
		}

		return lib;
	}
}
#endif
#endif

namespace pvr {
namespace lib {
typedef void* LIBTYPE;
}
} // namespace pvr

namespace pvr {
namespace internal {
#if defined(__APPLE__) && !TARGET_OS_IPHONE
inline pvr::lib::LIBTYPE OpenLibrary(const char* pszPath)
{
	// An objective-C function that uses dlopen
	pvr::lib::LIBTYPE hostLib = OpenFramework(pszPath);
	if (!hostLib) { Log_Error("Could not load host library '%s'", pszPath); }
	Log_Info("Host library '%s' loaded", pszPath);
	return hostLib;
}
#else
namespace {
inline pvr::lib::LIBTYPE OpenLibrary_Helper(const char* pszPath)
{
	pvr::lib::LIBTYPE hostLib = dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);
	if (!hostLib)
	{
		char pathMod[256];
		strcpy(pathMod, "./");
		strcat(pathMod, pszPath);

		hostLib = dlopen(pathMod, RTLD_LAZY | RTLD_GLOBAL);
	}
	return hostLib;
}
} // namespace

inline pvr::lib::LIBTYPE OpenLibrary(const char* pszPath)
{
	size_t start = 0;
	std::string tmp;
	std::string LibPath(pszPath);
	pvr::lib::LIBTYPE hostLib = nullptr;

	while (!hostLib)
	{
		size_t end = LibPath.find_first_of(';', start);

		if (end == std::string::npos) { tmp = LibPath.substr(start, LibPath.length() - start); }
		else
		{
			tmp = LibPath.substr(start, end - start);
		}

		if (!tmp.empty())
		{
			hostLib = OpenLibrary_Helper(tmp.c_str());

			if (!hostLib)
			{
				// Remove the last character in case a new line character snuck in
				tmp = tmp.substr(0, tmp.size() - 1);
				hostLib = OpenLibrary_Helper(tmp.c_str());
			}
		}

		if (end == std::string::npos) { break; }

		start = end + 1;
	}
	if (!hostLib)
	{
		const char* err = dlerror();
		if (err)
		{
			Log_Error("Could not load host library '%s'", pszPath);
			Log_Error("dlopen failed with error '%s'", err);
		}
	}
	Log_Info("Host library '%s' loaded", pszPath);

	return hostLib;
}

#endif

inline void CloseLibrary(pvr::lib::LIBTYPE hostLib) { dlclose(hostLib); }

inline void* getLibraryFunction(pvr::lib::LIBTYPE hostLib, const char* pszName)
{
	if (hostLib)
	{
		void* func = dlsym(hostLib, pszName);
		return func;
	}
	return nullptr;
}
} // namespace internal
} // namespace pvr
#elif defined(ANDROID)

namespace pvr {
namespace internal {
inline pvr::lib::LIBTYPE OpenLibrary(const char* pszPath)
{
	pvr::lib::LIBTYPE hostLib = dlopen(pszPath, RTLD_LAZY | RTLD_GLOBAL);
	if (!hostLib)
	{
		const char* err = dlerror();
		if (err)
		{
			Log_Error("Could not load host library '%s'", pszPath);
			Log_Error("dlopen failed with error '%s'", err);
		}
	}
	Log_Info("Host library '%s' loaded", pszPath);
	return hostLib;
}

inline void CloseLibrary(pvr::lib::LIBTYPE hostLib) { dlclose(hostLib); }

inline void* getLibraryFunction(pvr::lib::LIBTYPE hostLib, const char* pszName)
{
	void* fnct = dlsym(hostLib, pszName);
	return fnct;
}
} // namespace internal
} // namespace pvr
#else
#error Unsupported platform
#endif

namespace pvr {
namespace lib {
static pvr::lib::LIBTYPE hostLib = nullptr;
static std::string libraryName;

static inline pvr::lib::LIBTYPE openlib(const std::string& libName)
{
	hostLib = pvr::internal::OpenLibrary(libName.c_str());
	libraryName = libName;
	return hostLib;
}

static inline void closelib(pvr::lib::LIBTYPE lib) { pvr::internal::CloseLibrary(lib); }

template<typename PtrType_>
PtrType_ inline getLibFunction(pvr::lib::LIBTYPE hostLib, const std::string& functionName)
{
	return reinterpret_cast<PtrType_>(pvr::internal::getLibraryFunction(hostLib, functionName.c_str()));
}

template<typename PtrType_>
PtrType_ inline getLibFunctionChecked(pvr::lib::LIBTYPE hostLib, const std::string& functionName)
{
	PtrType_ func = getLibFunction<PtrType_>(hostLib, functionName);
	if (!func) { Log_Warning("Failed to load function [%s] from library '%s'.\n", functionName.c_str(), libraryName.c_str()); }
	return func;
}
} // namespace lib
} // namespace pvr
