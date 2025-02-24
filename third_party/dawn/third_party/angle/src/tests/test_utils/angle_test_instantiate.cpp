//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angle_test_instantiate.cpp: Adds support for filtering parameterized
// tests by platform, so we skip unsupported configs.

#include "test_utils/angle_test_instantiate.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>

#include "angle_gl.h"
#include "common/base/anglebase/no_destructor.h"
#include "common/debug.h"
#include "common/platform.h"
#include "common/system_utils.h"
#include "gpu_info_util/SystemInfo.h"
#include "test_utils/angle_test_configs.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/test_utils.h"

#if defined(ANGLE_PLATFORM_WINDOWS)
#    include <VersionHelpers.h>
#    include "util/windows/WGLWindow.h"
#endif  // defined(ANGLE_PLATFORM_WINDOWS)

#if defined(ANGLE_PLATFORM_APPLE)
#    include "test_utils/angle_test_instantiate_apple.h"
#endif

namespace angle
{
namespace
{
bool IsEGLConfigSupported(const PlatformParameters &param,
                          OSWindow *osWindow,
                          const char *eglLibraryName)
{
    std::unique_ptr<angle::Library> eglLibrary;

#if defined(ANGLE_USE_UTIL_LOADER)
    eglLibrary.reset(
        angle::OpenSharedLibrary(ANGLE_EGL_LIBRARY_NAME, angle::SearchType::ModuleDir));
#endif

    EGLWindow *eglWindow = EGLWindow::New(param.majorVersion, param.minorVersion);
    ConfigParameters configParams;
    bool result =
        eglWindow->initializeGL(osWindow, eglLibrary.get(), angle::GLESDriverType::AngleEGL,
                                param.eglParameters, configParams);
    eglWindow->destroyGL();
    EGLWindow::Delete(&eglWindow);
    return result;
}

bool IsAngleEGLConfigSupported(const PlatformParameters &param, OSWindow *osWindow)
{
    return IsEGLConfigSupported(param, osWindow, ANGLE_EGL_LIBRARY_NAME);
}

bool IsAngleVulkanSecondariesEGLConfigSupported(const PlatformParameters &param, OSWindow *osWindow)
{
    return IsEGLConfigSupported(param, osWindow, ANGLE_VULKAN_SECONDARIES_EGL_LIBRARY_NAME);
}

bool IsSystemWGLConfigSupported(const PlatformParameters &param, OSWindow *osWindow)
{
#if defined(ANGLE_PLATFORM_WINDOWS) && defined(ANGLE_USE_UTIL_LOADER)
    std::unique_ptr<angle::Library> openglLibrary(
        angle::OpenSharedLibrary("opengl32", angle::SearchType::SystemDir));

    WGLWindow *wglWindow = WGLWindow::New(param.majorVersion, param.minorVersion);
    ConfigParameters configParams;
    bool result =
        wglWindow->initializeGL(osWindow, openglLibrary.get(), angle::GLESDriverType::SystemWGL,
                                param.eglParameters, configParams);
    wglWindow->destroyGL();
    WGLWindow::Delete(&wglWindow);
    return result;
#else
    return false;
#endif  // defined(ANGLE_PLATFORM_WINDOWS) && defined(ANGLE_USE_UTIL_LOADER)
}

bool IsSystemEGLConfigSupported(const PlatformParameters &param, OSWindow *osWindow)
{
#if defined(ANGLE_USE_UTIL_LOADER)
    std::unique_ptr<angle::Library> eglLibrary;

    eglLibrary.reset(OpenSharedLibraryWithExtension(GetNativeEGLLibraryNameWithExtension(),
                                                    SearchType::SystemDir));

    EGLWindow *eglWindow = EGLWindow::New(param.majorVersion, param.minorVersion);
    ConfigParameters configParams;
    bool result =
        eglWindow->initializeGL(osWindow, eglLibrary.get(), angle::GLESDriverType::SystemEGL,
                                param.eglParameters, configParams);
    eglWindow->destroyGL();
    EGLWindow::Delete(&eglWindow);
    return result;
#else
    return false;
#endif
}

bool IsZinkEGLConfigSupported(const PlatformParameters &param, OSWindow *osWindow)
{
    return IsEGLConfigSupported(param, osWindow, ANGLE_MESA_EGL_LIBRARY_NAME);
}

bool IsAndroidDevice(const std::string &deviceName)
{
    if (!IsAndroid())
    {
        return false;
    }
    SystemInfo *systemInfo = GetTestSystemInfo();
    if (systemInfo->machineModelName == deviceName)
    {
        return true;
    }
    return false;
}

bool IsAndroidSdkLevelOrNewer(int level)
{
    if (!IsAndroid())
    {
        return false;
    }
    SystemInfo *systemInfo = GetTestSystemInfo();
    if (systemInfo->androidSdkLevel >= level)
    {
        return true;
    }
    return false;
}

bool IsAndroid9OrNewer()
{
    return IsAndroidSdkLevelOrNewer(28);
}

GPUDeviceInfo *GetActiveGPUDeviceInfo()
{
    SystemInfo *systemInfo = GetTestSystemInfo();
    // Unfortunately sometimes GPU info collection can fail.
    if (systemInfo->gpus.empty())
    {
        return nullptr;
    }
    return &systemInfo->gpus[systemInfo->activeGPUIndex];
}

bool HasSystemVendorID(VendorID vendorID)
{
    GPUDeviceInfo *gpuInfo = GetActiveGPUDeviceInfo();

    return gpuInfo && gpuInfo->vendorId == vendorID;
}

bool HasSystemDeviceID(VendorID vendorID, DeviceID deviceID)
{
    GPUDeviceInfo *gpuInfo = GetActiveGPUDeviceInfo();

    return gpuInfo && gpuInfo->vendorId == vendorID && gpuInfo->deviceId == deviceID;
}

using ParamAvailabilityCache = std::map<PlatformParameters, bool>;

ParamAvailabilityCache &GetAvailabilityCache()
{
    static angle::base::NoDestructor<std::unique_ptr<ParamAvailabilityCache>>
        sParamAvailabilityCache(new ParamAvailabilityCache());
    return **sParamAvailabilityCache;
}

constexpr size_t kMaxConfigNameLen = 100;
std::array<char, kMaxConfigNameLen> gSelectedConfig;
}  // namespace

bool gEnableANGLEPerTestCaptureLabel = false;

bool IsConfigSelected()
{
    return gSelectedConfig[0] != 0;
}

#if !defined(ANGLE_PLATFORM_APPLE)
// For Apple platform, see angle_test_instantiate_apple.mm
bool IsMetalTextureSwizzleAvailable()
{
    return false;
}
#endif

SystemInfo *GetTestSystemInfo()
{
    static SystemInfo *sSystemInfo = nullptr;
    if (sSystemInfo == nullptr)
    {
        sSystemInfo = new SystemInfo;
        if (!GetSystemInfo(sSystemInfo))
        {
            std::cerr << "Warning: incomplete system info collection.\n";
        }

        // On dual-GPU Macs we want the active GPU to always appear to be the
        // high-performance GPU for tests.
        // We can call the generic GPU info collector which selects the
        // non-Intel GPU as the active one on dual-GPU machines.
        if (IsMac())
        {
            GetDualGPUInfo(sSystemInfo);
        }

        // Print complete system info when available.
        // Seems to trip up Android test expectation parsing.
        // Also don't print info when a config is selected to prevent test spam.
        if (!IsAndroid() && !IsConfigSelected())
        {
            PrintSystemInfo(*sSystemInfo);
        }
    }
    return sSystemInfo;
}

bool IsARM64()
{
// _M_ARM64 is Windows-specific, while __aarch64__ is for other platforms.
#if defined(_M_ARM64) || defined(__aarch64__)
    return true;
#else
    return false;
#endif
}

bool IsOzone()
{
#if defined(USE_OZONE) && (defined(USE_X11) || defined(ANGLE_USE_VULKAN_DISPLAY))
    // We do not have a proper support for Ozone/Linux yet. Still, we need to figure out how to
    // properly initialize tests and differentiate between X11 and Wayland. Probably, passing a
    // command line argument could be sufficient. At the moment, run tests only for X11 backend
    // as we don't have Wayland support in Angle. Yes, this is a bit weird to return false, but
    // it makes it possible to continue angle tests with X11 regardless of the Chromium config
    // for linux, which is use_x11 && use_ozone.  Also, IsOzone is a bit vague now. It was only
    // expected that angle could run with ozone/drm backend for ChromeOS. And returning true
    // for desktop Linux when USE_OZONE && USE_X11 are both defined results in incorrect tests'
    // expectations. We should also rework them and make IsOzone less vague.
    //
    // TODO(crbug.com/angleproject/4977): make it possible to switch between X11 and Wayland on
    // Ozone/Linux builds. Probably, it's possible to identify the WAYLAND backend by checking
    // the WAYLAND_DISPLAY or XDG_SESSION_TYPE env vars. And also make the IsOzone method less
    // vague (read the comment above).
    return false;
#elif defined(USE_OZONE)
    return true;
#else
    return false;
#endif
}

bool IsNexus5X()
{
    return IsAndroidDevice("Nexus 5X");
}

bool IsNexus9()
{
    return IsAndroidDevice("Nexus 9");
}

bool IsPixelXL()
{
    return IsAndroidDevice("Pixel XL");
}

bool IsPixel2()
{
    return IsAndroidDevice("Pixel 2");
}

bool IsPixel2XL()
{
    return IsAndroidDevice("Pixel 2 XL");
}

bool IsPixel4()
{
    return IsAndroidDevice("Pixel 4");
}

bool IsPixel4XL()
{
    return IsAndroidDevice("Pixel 4 XL");
}

bool IsPixel6()
{
    return IsAndroidDevice("Pixel 6");
}

bool IsGalaxyS22()
{
    return IsAndroidDevice("SM-S901B");
}

bool IsNVIDIAShield()
{
    return IsAndroidDevice("SHIELD Android TV");
}

bool IsAndroid14OrNewer()
{
    return IsAndroidSdkLevelOrNewer(34);
}

bool IsIntel()
{
    return HasSystemVendorID(kVendorID_Intel);
}

bool IsIntelUHD630Mobile()
{
    return HasSystemDeviceID(kVendorID_Intel, kDeviceID_UHD630Mobile);
}

bool IsAMD()
{
    return HasSystemVendorID(kVendorID_AMD);
}

bool IsAppleGPU()
{
    return HasSystemVendorID(kVendorID_Apple);
}

bool IsARM()
{
    return HasSystemVendorID(kVendorID_ARM);
}

bool IsSwiftshaderDevice()
{
    return HasSystemDeviceID(kVendorID_GOOGLE, kDeviceID_Swiftshader);
}

bool IsSwiftShaderSupported()
{
#if defined(ANGLE_ENABLE_SWIFTSHADER)
    return true;
#else
    return false;
#endif
}

bool IsNVIDIA()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    // NVIDIA Shield cannot detect vendor ID (http://anglebug.com/42262205)
    if (IsNVIDIAShield())
    {
        return true;
    }
#endif
    return HasSystemVendorID(kVendorID_NVIDIA);
}

bool IsQualcomm()
{
    return HasSystemVendorID(kVendorID_Qualcomm) || HasSystemVendorID(kVendorID_Qualcomm_DXGI) ||
           IsNexus5X() || IsNexus9() || IsPixelXL() || IsPixel2() || IsPixel2XL() || IsPixel4() ||
           IsPixel4XL();
}

bool HasMesa()
{
#if defined(ANGLE_HAS_MESA)
    return true;
#else
    return false;
#endif  // defined(ANGLE_HAS_MESA)
}

bool IsConfigAllowlisted(const SystemInfo &systemInfo, const PlatformParameters &param)
{
    VendorID vendorID =
        systemInfo.gpus.empty() ? 0 : systemInfo.gpus[systemInfo.activeGPUIndex].vendorId;

    // We support the default and null back-ends on every platform.
    if (param.driver == GLESDriverType::AngleEGL)
    {
        if (param.getRenderer() == EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE)
            return true;
        if (param.getRenderer() == EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE)
            return true;
    }

    if (param.isSwiftshader() || IsSwiftshaderDevice())
    {
        if (!IsSwiftShaderSupported())
        {
            return false;
        }

        // TODO: http://crbug.com/swiftshader/145
        // Swiftshader does not currently have all the robustness features
        // we need for ANGLE. In particular, it is unable to detect and recover
        // from infinitely looping shaders. That bug is the tracker for fixing
        // that and when resolved we can remove the following code.
        // This test will disable tests marked with the config WithRobustness
        // when run with the swiftshader Vulkan driver and on Android.
        if (param.eglParameters.robustness == EGL_TRUE)
        {
            return false;
        }
    }

    if (param.driver == GLESDriverType::AngleVulkanSecondariesEGL)
    {
        if (param.getRenderer() != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
        {
            return false;
        }
        if (IsAndroid() &&
            param.getDeviceType() == EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE)
        {
            return false;
        }
        return true;
    }

    if (IsWindows())
    {
        switch (param.driver)
        {
            case GLESDriverType::AngleEGL:
                switch (param.getRenderer())
                {
                    case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
                    case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
                        return true;
                    case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
                        // Note we disable AMD OpenGL testing on Windows due to using a very old and
                        // outdated card with many driver bugs. See http://anglebug.com/42263687
                        return !IsAMD();
                    case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
                        if (IsARM64())
                        {
                            return param.getDeviceType() ==
                                   EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE;
                        }
                        return true;
                    case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
                        // ES 3.1+ back-end is not supported properly.
                        if (param.eglParameters.majorVersion == 3 &&
                            param.eglParameters.minorVersion > 0)
                        {
                            return false;
                        }

                        // Win ES emulation is currently only supported on NVIDIA.
                        return IsNVIDIA(vendorID);
                    case EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE:
                        return true;
                    default:
                        return false;
                }
            case GLESDriverType::SystemWGL:
                // AMD does not support the ES compatibility extensions.
                return !IsAMD(vendorID);
            default:
                return false;
        }
    }

#if defined(ANGLE_PLATFORM_APPLE)
    if (IsMac() || IsIOS())
    {
        // We do not support non-ANGLE bindings on OSX.
        if (param.driver != GLESDriverType::AngleEGL)
        {
            return false;
        }

        switch (param.getRenderer())
        {
            case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
                if (IsIOS())
                {
                    // OpenGL backend has been deprecated on iOS.
                    return false;
                }
                // ES 3.1+ back-end is not supported properly.
                if (param.majorVersion == 3 && param.minorVersion > 0)
                {
                    return false;
                }
                return true;
            case EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE:
                return true;
            case EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE:
                if (!IsMetalRendererAvailable())
                {
                    return false;
                }
                return true;
            case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
                // OSX does not support native vulkan
                return param.getDeviceType() == EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE;
            default:
                return false;
        }
    }
#endif  // #if defined(ANGLE_PLATFORM_APPLE)

    if (IsFuchsia())
    {
        // We do not support non-ANGLE bindings on Fuchsia.
        if (param.driver != GLESDriverType::AngleEGL)
        {
            return false;
        }

        // ES 3 configs do not work properly on Fuchsia ARM.
        // TODO(anglebug.com/42262979): Investigate missing features.
        if (param.majorVersion > 2 && IsARM())
            return false;

        // Loading swiftshader is not brought up on Fuchsia.
        // TODO(anglebug.com/42262980): Support loading swiftshader vulkan ICD.
        if (param.getDeviceType() == EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE)
            return false;

        // Currently we only support the Vulkan back-end on Fuchsia.
        return (param.getRenderer() == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
    }

    if (IsOzone())
    {
        // We do not support non-ANGLE bindings on Ozone.
        if (param.driver != GLESDriverType::AngleEGL)
            return false;

        // ES 3 configs do not work properly on Ozone.
        if (param.majorVersion > 2)
            return false;

        // Currently we only support the GLES back-end on Ozone.
        return (param.getRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
    }

    if (IsLinux() || IsAndroid())
    {
        // We do not support WGL bindings on Linux/Android. We do support system EGL.
        switch (param.driver)
        {
            case GLESDriverType::SystemEGL:
                return param.getRenderer() == EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
            case GLESDriverType::SystemWGL:
                return false;
            case GLESDriverType::ZinkEGL:
                return HasMesa();
            default:
                break;
        }
    }

    if (IsLinux())
    {
        ASSERT(param.driver == GLESDriverType::AngleEGL);

        // Currently we support the OpenGL, Vulkan and WebGPU back-ends on Linux.
        switch (param.getRenderer())
        {
            case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
            case EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE:
            case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
                return true;
            default:
                return false;
        }
    }

    if (IsAndroid())
    {
        ASSERT(param.driver == GLESDriverType::AngleEGL);

        // Nexus Android devices don't support backing 3.2 contexts
        if (param.eglParameters.majorVersion == 3 && param.eglParameters.minorVersion == 2)
        {
            if (IsNexus5X())
            {
                return false;
            }
        }

        // Currently we support the GLES and Vulkan back-ends on Android.
        switch (param.getRenderer())
        {
            case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
                return true;
            case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
                // Swiftshader's vulkan frontend doesn't build on Android.
                if (param.getDeviceType() == EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE)
                {
                    return false;
                }
                if (!IsAndroid9OrNewer())
                {
                    return false;
                }
                return true;
            default:
                return false;
        }
    }

    // Unknown platform.
    return false;
}

bool IsConfigSupported(const PlatformParameters &param)
{
    OSWindow *osWindow = OSWindow::New();
    bool result        = false;
    if (osWindow->initialize("CONFIG_TESTER", 1, 1))
    {
        switch (param.driver)
        {
            case GLESDriverType::AngleEGL:
                result = IsAngleEGLConfigSupported(param, osWindow);
                break;
            case GLESDriverType::AngleVulkanSecondariesEGL:
                result = IsAngleVulkanSecondariesEGLConfigSupported(param, osWindow);
                break;
            case GLESDriverType::SystemEGL:
                result = IsSystemEGLConfigSupported(param, osWindow);
                break;
            case GLESDriverType::SystemWGL:
                result = IsSystemWGLConfigSupported(param, osWindow);
                break;
            case GLESDriverType::ZinkEGL:
                result = IsZinkEGLConfigSupported(param, osWindow);
                break;
        }

        osWindow->destroy();
    }

    OSWindow::Delete(&osWindow);
    return result;
}

bool IsPlatformAvailable(const PlatformParameters &param)
{
    // Disable "null" device when not on ANGLE or in D3D9.
    if (param.getDeviceType() == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
    {
        if (!IsANGLE(param.driver))
            return false;
        if (param.getRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
            return false;
    }

    switch (param.getRenderer())
    {
        case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
            break;

        case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
#if !defined(ANGLE_ENABLE_D3D9)
            return false;
#else
            break;
#endif

        case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
#if !defined(ANGLE_ENABLE_D3D11)
            return false;
#else
            break;
#endif

        case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
        case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
#if !defined(ANGLE_ENABLE_OPENGL)
            return false;
#else
            break;
#endif

        case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
#if !defined(ANGLE_ENABLE_VULKAN)
            return false;
#else
            break;
#endif

        case EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE:
#if !defined(ANGLE_ENABLE_METAL)
            return false;
#else
            break;
#endif

        case EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE:
#if !defined(ANGLE_ENABLE_NULL)
            return false;
#else
            break;
#endif
        case EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE:
#if !defined(ANGLE_ENABLE_WGPU)
            return false;
#else
            break;
#endif

        default:
            std::cout << "Unknown test platform: " << param << std::endl;
            return false;
    }

    bool result = false;

    auto iter = GetAvailabilityCache().find(param);
    if (iter != GetAvailabilityCache().end())
    {
        result = iter->second;
    }
    else
    {
        if (IsConfigSelected())
        {
            std::stringstream strstr;
            strstr << param;
            if (strstr.str() == std::string(gSelectedConfig.data()))
            {
                result = true;
            }
        }
        else
        {
            const SystemInfo *systemInfo = GetTestSystemInfo();

            if (systemInfo)
            {
                result = IsConfigAllowlisted(*systemInfo, param);
            }
            else
            {
                result = IsConfigSupported(param);
            }
        }

        GetAvailabilityCache()[param] = result;

        // Enable this unconditionally to print available platforms.
        if (IsConfigSelected())
        {
            if (result)
            {
                std::cout << "Test Config: " << param << "\n";
            }
        }
        else if (!result)
        {
            std::cout << "Skipping tests using configuration " << param
                      << " because it is not available.\n";
        }
    }
    return result;
}

std::vector<std::string> GetAvailableTestPlatformNames()
{
    std::vector<std::string> platformNames;

    for (const auto &iter : GetAvailabilityCache())
    {
        if (iter.second)
        {
            std::stringstream strstr;
            strstr << iter.first;
            platformNames.push_back(strstr.str());
        }
    }

    // Keep the list sorted.
    std::sort(platformNames.begin(), platformNames.end());

    return platformNames;
}

void SetSelectedConfig(const char *selectedConfig)
{
    gSelectedConfig.fill(0);
    strncpy(gSelectedConfig.data(), selectedConfig, kMaxConfigNameLen - 1);
}

GLESDriverType GetDriverTypeFromString(const char *driverName, GLESDriverType defaultDriverType)
{
    if (!driverName)
    {
        return defaultDriverType;
    }

    if (strcmp(driverName, "angle") == 0)
    {
        return GLESDriverType::AngleEGL;
    }

    if (strcmp(driverName, "angle-vulkan-secondaries") == 0)
    {
        return GLESDriverType::AngleVulkanSecondariesEGL;
    }

    if (strcmp(driverName, "zink") == 0)
    {
        return GLESDriverType::ZinkEGL;
    }

    if (strcmp(driverName, "native") == 0 || strcmp(driverName, "system") == 0)
    {
        if (IsWindows())
        {
            return GLESDriverType::SystemWGL;
        }
        else
        {
            return GLESDriverType::SystemEGL;
        }
    }

    printf("Unknown driver type: %s\n", driverName);
    exit(EXIT_FAILURE);
}
}  // namespace angle
