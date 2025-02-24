//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLPlatformParameters: Basic description of an EGL device.

#ifndef UTIL_EGLPLATFORMPARAMETERS_H_
#define UTIL_EGLPLATFORMPARAMETERS_H_

#include "util/util_gl.h"

#include "autogen/angle_features_autogen.h"

#include <string>
#include <tuple>
#include <vector>

namespace angle
{
struct PlatformMethods;

// The GLES driver type determines what shared object we use to load the GLES entry points.
// AngleEGL loads from ANGLE's version of libEGL, libGLESv2, and libGLESv1_CM.
// SystemEGL uses the system copies of libEGL, libGLESv2, and libGLESv1_CM.
// SystemWGL loads Windows GL with the GLES compatibility extensions. See util/WGLWindow.h.
enum class GLESDriverType
{
    AngleEGL,
    AngleVulkanSecondariesEGL,
    SystemEGL,
    SystemWGL,
    ZinkEGL,
};

inline bool IsANGLE(angle::GLESDriverType driverType)
{
    return driverType == angle::GLESDriverType::AngleEGL ||
           driverType == angle::GLESDriverType::AngleVulkanSecondariesEGL;
}

GLESDriverType GetDriverTypeFromString(const char *driverName, GLESDriverType defaultDriverType);
}  // namespace angle

struct EGLPlatformParameters
{
    EGLPlatformParameters() = default;

    explicit EGLPlatformParameters(EGLint renderer) : renderer(renderer) {}

    EGLPlatformParameters(EGLint renderer,
                          EGLint majorVersion,
                          EGLint minorVersion,
                          EGLint deviceType)
        : renderer(renderer),
          majorVersion(majorVersion),
          minorVersion(minorVersion),
          deviceType(deviceType)
    {}

    EGLPlatformParameters(EGLint renderer,
                          EGLint majorVersion,
                          EGLint minorVersion,
                          EGLint deviceType,
                          EGLint presentPath)
        : renderer(renderer),
          majorVersion(majorVersion),
          minorVersion(minorVersion),
          deviceType(deviceType),
          presentPath(presentPath)
    {}

    auto tie() const
    {
        return std::tie(renderer, majorVersion, minorVersion, deviceType, presentPath,
                        debugLayersEnabled, robustness, displayPowerPreference,
                        disabledFeatureOverrides, enabledFeatureOverrides, platformMethods);
    }

    // Helpers to enable and disable ANGLE features.  Expects a kFeature* value from
    // angle_features_autogen.h.
    EGLPlatformParameters &enable(angle::Feature feature)
    {
        enabledFeatureOverrides.push_back(feature);
        return *this;
    }
    EGLPlatformParameters &disable(angle::Feature feature)
    {
        disabledFeatureOverrides.push_back(feature);
        return *this;
    }

    EGLint renderer               = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
    EGLint majorVersion           = EGL_DONT_CARE;
    EGLint minorVersion           = EGL_DONT_CARE;
    EGLint deviceType             = EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE;
    EGLint presentPath            = EGL_DONT_CARE;
    EGLint debugLayersEnabled     = EGL_DONT_CARE;
    EGLint robustness             = EGL_DONT_CARE;
    EGLint displayPowerPreference = EGL_DONT_CARE;

    std::vector<angle::Feature> enabledFeatureOverrides;
    std::vector<angle::Feature> disabledFeatureOverrides;

    angle::PlatformMethods *platformMethods = nullptr;
};

inline bool operator<(const EGLPlatformParameters &a, const EGLPlatformParameters &b)
{
    return a.tie() < b.tie();
}

inline bool operator==(const EGLPlatformParameters &a, const EGLPlatformParameters &b)
{
    return a.tie() == b.tie();
}

inline bool operator!=(const EGLPlatformParameters &a, const EGLPlatformParameters &b)
{
    return a.tie() != b.tie();
}

#endif  // UTIL_EGLPLATFORMPARAMETERS_H_
