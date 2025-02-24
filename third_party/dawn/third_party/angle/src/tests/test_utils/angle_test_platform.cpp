//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "angle_test_platform.h"

#include "common/platform.h"
#include "gpu_info_util/SystemInfo.h"

#if defined(ANGLE_PLATFORM_WINDOWS)
#    include <VersionHelpers.h>
#endif  // defined(ANGLE_PLATFORM_WINDOWS)

using namespace angle;

bool IsAdreno()
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("Adreno") != std::string::npos);
}

bool IsD3D11()
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("Direct3D11 vs_5_0") != std::string::npos);
}

bool IsD3D9()
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("Direct3D9") != std::string::npos);
}

bool IsDesktopOpenGL()
{
    return IsOpenGL() && !IsOpenGLES();
}

bool IsOpenGLES()
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("OpenGL ES") != std::string::npos);
}

bool IsOpenGL()
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("OpenGL") != std::string::npos);
}

bool IsNULL()
{
    std::string rendererString(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    return (rendererString.find("NULL") != std::string::npos);
}

bool IsVulkan()
{
    const char *renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    std::string rendererString(renderer);
    return (rendererString.find("Vulkan") != std::string::npos);
}

bool IsMetal()
{
    const char *renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    std::string rendererString(renderer);
    return (rendererString.find("ANGLE Metal") != std::string::npos);
}

bool IsD3D()
{
    return IsD3D9() || IsD3D11();
}

bool IsDebug()
{
#if !defined(NDEBUG)
    return true;
#else
    return false;
#endif
}

bool IsRelease()
{
    return !IsDebug();
}

bool EnsureGLExtensionEnabled(const std::string &extName)
{
    if (IsGLExtensionEnabled("GL_ANGLE_request_extension") && IsGLExtensionRequestable(extName))
    {
        glRequestExtensionANGLE(extName.c_str());
    }

    return IsGLExtensionEnabled(extName);
}

bool IsEGLClientExtensionEnabled(const std::string &extName)
{
    return CheckExtensionExists(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS), extName);
}

bool IsEGLDeviceExtensionEnabled(EGLDeviceEXT device, const std::string &extName)
{
    return CheckExtensionExists(eglQueryDeviceStringEXT(device, EGL_EXTENSIONS), extName);
}

bool IsEGLDisplayExtensionEnabled(EGLDisplay display, const std::string &extName)
{
    return CheckExtensionExists(eglQueryString(display, EGL_EXTENSIONS), extName);
}

bool IsGLExtensionEnabled(const std::string &extName)
{
    return CheckExtensionExists(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)),
                                extName);
}

bool IsGLExtensionRequestable(const std::string &extName)
{
    return CheckExtensionExists(
        reinterpret_cast<const char *>(glGetString(GL_REQUESTABLE_EXTENSIONS_ANGLE)), extName);
}
