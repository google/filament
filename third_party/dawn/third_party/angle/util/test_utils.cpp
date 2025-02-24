//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils: Defines common utility functions

#include "util/test_utils.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <cstring>
#include <fstream>
#include <iostream>

namespace angle
{

namespace
{
void DeleteArg(int *argc, char **argv, int argIndex)
{
    // Shift the remainder of the argv list left by one.  Note that argv has (*argc + 1) elements,
    // the last one always being NULL.  The following loop moves the trailing NULL element as well.
    for (int index = argIndex; index < *argc; ++index)
    {
        argv[index] = argv[index + 1];
    }
    (*argc)--;
}

const char *GetSingleArg(const char *flag,
                         int *argc,
                         char **argv,
                         int argIndex,
                         ArgHandling handling)
{
    if (strstr(argv[argIndex], flag) == argv[argIndex])
    {
        const char *ptr = argv[argIndex] + strlen(flag);

        if (*ptr == '=')
        {
            if (handling == ArgHandling::Delete)
            {
                DeleteArg(argc, argv, argIndex);
            }
            return ptr + 1;
        }

        if (*ptr == '\0' && argIndex < *argc - 1)
        {
            ptr = argv[argIndex + 1];
            if (handling == ArgHandling::Delete)
            {
                DeleteArg(argc, argv, argIndex);
                DeleteArg(argc, argv, argIndex);
            }
            return ptr;
        }
    }

    return nullptr;
}

using DisplayTypeInfo = std::pair<const char *, EGLint>;

const DisplayTypeInfo kDisplayTypes[] = {
    {"d3d9", EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE},
    {"d3d11", EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE},
    {"gl", EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE},
    {"gles", EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE},
    {"metal", EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE},
    {"null", EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE},
    {"swiftshader", EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE},
    {"vulkan", EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE},
    {"vulkan-null", EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE},
    {"webgpu", EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE},
};
}  // anonymous namespace

bool GetFileSize(const char *filePath, uint32_t *sizeOut)
{
    std::ifstream stream(filePath);
    if (!stream)
    {
        return false;
    }

    stream.seekg(0, std::ios::end);
    *sizeOut = static_cast<uint32_t>(stream.tellg());
    return true;
}

bool ReadEntireFileToString(const char *filePath, std::string *contentsOut)
{
    std::ifstream stream(filePath);
    if (!stream)
    {
        return false;
    }

    stream.seekg(0, std::ios::end);
    contentsOut->reserve(static_cast<unsigned int>(stream.tellg()));
    stream.seekg(0, std::ios::beg);

    contentsOut->assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    return true;
}

// static
Process::~Process() = default;

ProcessHandle::ProcessHandle() : mProcess(nullptr) {}

ProcessHandle::ProcessHandle(Process *process) : mProcess(process) {}

ProcessHandle::ProcessHandle(const std::vector<const char *> &args,
                             ProcessOutputCapture captureOutput)
    : mProcess(LaunchProcess(args, captureOutput))
{}

ProcessHandle::~ProcessHandle()
{
    reset();
}

ProcessHandle::ProcessHandle(ProcessHandle &&other) : mProcess(other.mProcess)
{
    other.mProcess = nullptr;
}

ProcessHandle &ProcessHandle::operator=(ProcessHandle &&rhs)
{
    std::swap(mProcess, rhs.mProcess);
    return *this;
}

void ProcessHandle::reset()
{
    if (mProcess)
    {
        delete mProcess;
        mProcess = nullptr;
    }
}

bool ParseIntArgWithHandling(const char *flag,
                             int *argc,
                             char **argv,
                             int argIndex,
                             int *valueOut,
                             ArgHandling handling)
{
    const char *value = GetSingleArg(flag, argc, argv, argIndex, handling);
    if (!value)
    {
        return false;
    }

    char *end            = nullptr;
    const long longValue = strtol(value, &end, 10);

    if (*end != '\0')
    {
        printf("Error parsing integer flag value.\n");
        exit(EXIT_FAILURE);
    }

    if (longValue == LONG_MAX || longValue == LONG_MIN || static_cast<int>(longValue) != longValue)
    {
        printf("Overflow when parsing integer flag value.\n");
        exit(EXIT_FAILURE);
    }

    *valueOut = static_cast<int>(longValue);
    // Note: return value is always false with ArgHandling::Preserve handling
    return handling == ArgHandling::Delete;
}

bool ParseIntArg(const char *flag, int *argc, char **argv, int argIndex, int *valueOut)
{
    return ParseIntArgWithHandling(flag, argc, argv, argIndex, valueOut, ArgHandling::Delete);
}

bool ParseFlag(const char *flag, int *argc, char **argv, int argIndex, bool *flagOut)
{
    if (strcmp(flag, argv[argIndex]) == 0)
    {
        *flagOut = true;
        DeleteArg(argc, argv, argIndex);
        return true;
    }
    return false;
}

bool ParseStringArg(const char *flag, int *argc, char **argv, int argIndex, std::string *valueOut)
{
    const char *value = GetSingleArg(flag, argc, argv, argIndex, ArgHandling::Delete);
    if (!value)
    {
        return false;
    }

    *valueOut = value;
    return true;
}

bool ParseCStringArgWithHandling(const char *flag,
                                 int *argc,
                                 char **argv,
                                 int argIndex,
                                 const char **valueOut,
                                 ArgHandling handling)
{
    const char *value = GetSingleArg(flag, argc, argv, argIndex, handling);
    if (!value)
    {
        return false;
    }

    *valueOut = value;
    // Note: return value is always false with ArgHandling::Preserve handling
    return handling == ArgHandling::Delete;
}

bool ParseCStringArg(const char *flag, int *argc, char **argv, int argIndex, const char **valueOut)
{
    return ParseCStringArgWithHandling(flag, argc, argv, argIndex, valueOut, ArgHandling::Delete);
}

void AddArg(int *argc, char **argv, const char *arg)
{
    // This unsafe const_cast is necessary to work around gtest limitations.
    argv[*argc]     = const_cast<char *>(arg);
    argv[*argc + 1] = nullptr;
    (*argc)++;
}

uint32_t GetPlatformANGLETypeFromArg(const char *useANGLEArg, uint32_t defaultPlatformType)
{
    if (!useANGLEArg)
    {
        return defaultPlatformType;
    }

    for (const DisplayTypeInfo &displayTypeInfo : kDisplayTypes)
    {
        if (strcmp(displayTypeInfo.first, useANGLEArg) == 0)
        {
            std::cout << "Using ANGLE back-end API: " << displayTypeInfo.first << std::endl;
            return displayTypeInfo.second;
        }
    }

    std::cout << "Unknown ANGLE back-end API: " << useANGLEArg << std::endl;
    exit(EXIT_FAILURE);
}

uint32_t GetANGLEDeviceTypeFromArg(const char *useANGLEArg, uint32_t defaultDeviceType)
{
    if (useANGLEArg)
    {
        if (strcmp(useANGLEArg, "swiftshader") == 0)
        {
            return EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE;
        }
        if (strstr(useANGLEArg, "null") != 0)
        {
            return EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE;
        }
    }
    return defaultDeviceType;
}
}  // namespace angle
