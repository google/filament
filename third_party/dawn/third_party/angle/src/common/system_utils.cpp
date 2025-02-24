//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils.cpp: Implementation of common functions

#include "common/system_utils.h"
#include "common/debug.h"

#include <stdlib.h>
#include <atomic>

#if defined(ANGLE_PLATFORM_ANDROID)
#    include <sys/system_properties.h>
#endif

#if defined(ANGLE_PLATFORM_APPLE)
#    include <dispatch/dispatch.h>
#    include <pthread.h>
#endif

namespace angle
{
std::string GetExecutableName()
{
#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 21
    // Support for "getprogname" function in bionic was introduced in L (API level 21)
    const char *executableName = getprogname();
    return (executableName) ? std::string(executableName) : "ANGLE";
#else
    std::string executableName = GetExecutablePath();
    size_t lastPathSepLoc      = executableName.find_last_of(GetPathSeparator());
    return (lastPathSepLoc > 0 ? executableName.substr(lastPathSepLoc + 1, executableName.length())
                               : "ANGLE");
#endif  // ANGLE_PLATFORM_ANDROID
}

// On Android return value cached in the process environment, if none, call
// GetEnvironmentVarOrUnCachedAndroidProperty if not in environment.
std::string GetEnvironmentVarOrAndroidProperty(const char *variableName, const char *propertyName)
{
#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 21
    // Can't use GetEnvironmentVar here because that won't allow us to distinguish between the
    // environment being set to an empty string vs. not set at all.
    const char *variableValue = getenv(variableName);
    if (variableValue != nullptr)
    {
        std::string value(variableValue);
        return value;
    }
#endif
    return GetEnvironmentVarOrUnCachedAndroidProperty(variableName, propertyName);
}

// On Android call out to 'getprop' on a shell to get an Android property.  On desktop, return
// the value of the environment variable.
std::string GetEnvironmentVarOrUnCachedAndroidProperty(const char *variableName,
                                                       const char *propertyName)
{
#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 26
    std::string propertyValue;

    const prop_info *propertyInfo = __system_property_find(propertyName);
    if (propertyInfo != nullptr)
    {
        __system_property_read_callback(
            propertyInfo,
            [](void *cookie, const char *, const char *value, unsigned) {
                auto propertyValue = reinterpret_cast<std::string *>(cookie);
                *propertyValue     = value;
            },
            &propertyValue);
    }

    return propertyValue;
#else
    // Return the environment variable's value.
    return GetEnvironmentVar(variableName);
#endif  // ANGLE_PLATFORM_ANDROID
}

// Look up a property and add it to the application's environment.
// Adding to the env is a performance optimization, as getting properties is expensive.
// This should only be used in non-Release paths, i.e. when using FrameCapture or DebugUtils.
// It can cause race conditions in stress testing. See http://anglebug.com/42265318
std::string GetAndSetEnvironmentVarOrUnCachedAndroidProperty(const char *variableName,
                                                             const char *propertyName)
{
    std::string value = GetEnvironmentVarOrUnCachedAndroidProperty(variableName, propertyName);

#if defined(ANGLE_PLATFORM_ANDROID)
    if (!value.empty())
    {
        // Set the environment variable with the value to improve future lookups (avoids
        SetEnvironmentVar(variableName, value.c_str());
    }
#endif

    return value;
}

bool GetBoolEnvironmentVar(const char *variableName)
{
    std::string envVarString = GetEnvironmentVar(variableName);
    return (!envVarString.empty() && envVarString == "1");
}

bool PrependPathToEnvironmentVar(const char *variableName, const char *path)
{
    std::string oldValue = GetEnvironmentVar(variableName);
    const char *newValue = nullptr;
    std::string buf;
    if (oldValue.empty())
    {
        newValue = path;
    }
    else
    {
        buf = path;
        buf += GetPathSeparatorForEnvironmentVar();
        buf += oldValue;
        newValue = buf.c_str();
    }
    return SetEnvironmentVar(variableName, newValue);
}

bool IsFullPath(std::string dirName)
{
    if (dirName.find(GetRootDirectory()) == 0)
    {
        return true;
    }
    return false;
}

std::string ConcatenatePath(std::string first, std::string second)
{
    if (first.empty())
    {
        return second;
    }
    if (second.empty())
    {
        return first;
    }
    if (IsFullPath(second))
    {
        return second;
    }
    bool firstRedundantPathSeparator = first.find_last_of(GetPathSeparator()) == first.length() - 1;
    bool secondRedundantPathSeparator = second.find(GetPathSeparator()) == 0;
    if (firstRedundantPathSeparator && secondRedundantPathSeparator)
    {
        return first + second.substr(1);
    }
    else if (firstRedundantPathSeparator || secondRedundantPathSeparator)
    {
        return first + second;
    }
    return first + GetPathSeparator() + second;
}

Optional<std::string> CreateTemporaryFile()
{
    const Optional<std::string> tempDir = GetTempDirectory();
    if (!tempDir.valid())
        return Optional<std::string>::Invalid();

    return CreateTemporaryFileInDirectory(tempDir.value());
}

PageFaultHandler::PageFaultHandler(PageFaultCallback callback) : mCallback(callback) {}
PageFaultHandler::~PageFaultHandler() {}

Library *OpenSharedLibrary(const char *libraryName, SearchType searchType)
{
    void *libraryHandle = OpenSystemLibraryAndGetError(libraryName, searchType, nullptr);
    return new Library(libraryHandle);
}

Library *OpenSharedLibraryWithExtension(const char *libraryName, SearchType searchType)
{
    void *libraryHandle =
        OpenSystemLibraryWithExtensionAndGetError(libraryName, searchType, nullptr);
    return new Library(libraryHandle);
}

Library *OpenSharedLibraryAndGetError(const char *libraryName,
                                      SearchType searchType,
                                      std::string *errorOut)
{
    void *libraryHandle = OpenSystemLibraryAndGetError(libraryName, searchType, errorOut);
    return new Library(libraryHandle);
}

Library *OpenSharedLibraryWithExtensionAndGetError(const char *libraryName,
                                                   SearchType searchType,
                                                   std::string *errorOut)
{
    void *libraryHandle =
        OpenSystemLibraryWithExtensionAndGetError(libraryName, searchType, errorOut);
    return new Library(libraryHandle);
}

void *OpenSystemLibrary(const char *libraryName, SearchType searchType)
{
    return OpenSystemLibraryAndGetError(libraryName, searchType, nullptr);
}

void *OpenSystemLibraryWithExtension(const char *libraryName, SearchType searchType)
{
    return OpenSystemLibraryWithExtensionAndGetError(libraryName, searchType, nullptr);
}

void *OpenSystemLibraryAndGetError(const char *libraryName,
                                   SearchType searchType,
                                   std::string *errorOut)
{
    std::string libraryWithExtension = std::string(libraryName);
    std::string dotExtension         = std::string(".") + GetSharedLibraryExtension();
    // Only append the extension if it's not already present. This enables building libEGL.so.1
    // and libGLESv2.so.2 by setting these as ANGLE_EGL_LIBRARY_NAME and ANGLE_GLESV2_LIBRARY_NAME.
    if (libraryWithExtension.find(dotExtension) == std::string::npos)
    {
        libraryWithExtension += dotExtension;
    }
#if ANGLE_PLATFORM_IOS_FAMILY
    // On iOS, libraryWithExtension is a directory in which the library resides.
    // The actual library name doesn't have an extension at all.
    // E.g. "libEGL.framework/libEGL"
    libraryWithExtension = libraryWithExtension + "/" + libraryName;
#endif
    return OpenSystemLibraryWithExtensionAndGetError(libraryWithExtension.c_str(), searchType,
                                                     errorOut);
}

std::string StripFilenameFromPath(const std::string &path)
{
    size_t lastPathSepLoc = path.find_last_of("\\/");
    return (lastPathSepLoc != std::string::npos) ? path.substr(0, lastPathSepLoc) : "";
}

#if defined(ANGLE_PLATFORM_APPLE)
// https://anglebug.com/42264979, similar to egl::GetCurrentThread() in libGLESv2/global_state.cpp
uint64_t GetCurrentThreadUniqueId()
{
    static std::atomic<uint64_t> globalThreadSerial;
    static pthread_key_t tlsIndex;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
      auto result = pthread_key_create(&tlsIndex, nullptr);
      ASSERT(result == 0);
    });
    void *tlsValue = pthread_getspecific(tlsIndex);
    if (ANGLE_UNLIKELY(tlsValue == nullptr))
    {
        uint64_t threadId = ++globalThreadSerial;
        auto result       = pthread_setspecific(tlsIndex, reinterpret_cast<void *>(threadId));
        ASSERT(result == 0);
        return threadId;
    }
    return reinterpret_cast<uint64_t>(tlsValue);
}
#else
uint64_t GetCurrentThreadUniqueId()
{
    static std::atomic<uint64_t> globalThreadSerial;
    thread_local uint64_t threadId(++globalThreadSerial);
    return threadId;
}
#endif

}  // namespace angle
