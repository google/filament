//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_TEST_CONFIGS_H_
#define ANGLE_TEST_CONFIGS_H_

// On Linux EGL/egl.h includes X.h which does defines for some very common
// names that are used by gtest (like None and Bool) and causes a lot of
// compilation errors. To work around this, even if this file doesn't use it,
// we include gtest before EGL so that it compiles fine in other files that
// want to use gtest.
#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "angle_test_instantiate.h"
#include "util/EGLPlatformParameters.h"

namespace angle
{

struct PlatformParameters
{
    PlatformParameters();
    PlatformParameters(EGLint majorVersion,
                       EGLint minorVersion,
                       const EGLPlatformParameters &eglPlatformParameters);
    PlatformParameters(EGLint majorVersion, EGLint minorVersion, GLESDriverType driver);

    EGLint getRenderer() const;
    EGLint getDeviceType() const;
    bool isSwiftshader() const;
    bool isVulkan() const;
    bool isANGLE() const;
    bool isMetal() const;
    bool isWebGPU() const;

    void initDefaultParameters();

    auto tie() const
    {
        return std::tie(driver, noFixture, eglParameters, majorVersion, minorVersion);
    }

    // Helpers to enable and disable ANGLE features.  Expects a Feature::* value from
    // angle_features_autogen.h.
    PlatformParameters &enable(Feature feature)
    {
        eglParameters.enable(feature);
        return *this;
    }
    PlatformParameters &disable(Feature feature)
    {
        eglParameters.disable(feature);
        return *this;
    }
    bool isEnableRequested(Feature feature) const;
    bool isDisableRequested(Feature feature) const;

    GLESDriverType driver;
    bool noFixture;
    EGLPlatformParameters eglParameters;
    EGLint majorVersion;
    EGLint minorVersion;
};

const char *GetRendererName(EGLint renderer);

bool operator<(const PlatformParameters &a, const PlatformParameters &b);
bool operator==(const PlatformParameters &a, const PlatformParameters &b);
std::ostream &operator<<(std::ostream &stream, const PlatformParameters &pp);

// EGL platforms
namespace egl_platform
{

EGLPlatformParameters DEFAULT();
EGLPlatformParameters DEFAULT_NULL();

EGLPlatformParameters D3D9();
EGLPlatformParameters D3D9_NULL();
EGLPlatformParameters D3D9_REFERENCE();

EGLPlatformParameters D3D11();
EGLPlatformParameters D3D11_PRESENT_PATH_FAST();
EGLPlatformParameters D3D11_FL11_1();
EGLPlatformParameters D3D11_FL11_0();
EGLPlatformParameters D3D11_FL10_1();
EGLPlatformParameters D3D11_FL10_0();

EGLPlatformParameters D3D11_NULL();

EGLPlatformParameters D3D11_WARP();
EGLPlatformParameters D3D11_FL11_1_WARP();
EGLPlatformParameters D3D11_FL11_0_WARP();
EGLPlatformParameters D3D11_FL10_1_WARP();
EGLPlatformParameters D3D11_FL10_0_WARP();

EGLPlatformParameters D3D11_REFERENCE();
EGLPlatformParameters D3D11_FL11_1_REFERENCE();
EGLPlatformParameters D3D11_FL11_0_REFERENCE();
EGLPlatformParameters D3D11_FL10_1_REFERENCE();
EGLPlatformParameters D3D11_FL10_0_REFERENCE();

EGLPlatformParameters METAL();

EGLPlatformParameters OPENGL();
EGLPlatformParameters OPENGL(EGLint major, EGLint minor);
EGLPlatformParameters OPENGL_NULL();

EGLPlatformParameters OPENGLES();
EGLPlatformParameters OPENGLES(EGLint major, EGLint minor);
EGLPlatformParameters OPENGLES_NULL();

EGLPlatformParameters OPENGL_OR_GLES();
EGLPlatformParameters OPENGL_OR_GLES(EGLint major, EGLint minor);
EGLPlatformParameters OPENGL_OR_GLES_NULL();

EGLPlatformParameters VULKAN();
EGLPlatformParameters VULKAN_NULL();
EGLPlatformParameters VULKAN_SWIFTSHADER();

EGLPlatformParameters WEBGPU();

}  // namespace egl_platform

// ANGLE tests platforms
PlatformParameters ES1_D3D9();
PlatformParameters ES2_D3D9();

PlatformParameters ES1_D3D11();
PlatformParameters ES2_D3D11();
PlatformParameters ES2_D3D11_PRESENT_PATH_FAST();
PlatformParameters ES2_D3D11_FL11_0();
PlatformParameters ES2_D3D11_FL10_1();
PlatformParameters ES2_D3D11_FL10_0();

PlatformParameters ES2_D3D11_WARP();
PlatformParameters ES2_D3D11_FL11_0_WARP();
PlatformParameters ES2_D3D11_FL10_1_WARP();
PlatformParameters ES2_D3D11_FL10_0_WARP();

PlatformParameters ES2_D3D11_REFERENCE();
PlatformParameters ES2_D3D11_FL11_0_REFERENCE();
PlatformParameters ES2_D3D11_FL10_1_REFERENCE();
PlatformParameters ES2_D3D11_FL10_0_REFERENCE();

PlatformParameters ES3_D3D11();
PlatformParameters ES3_D3D11_FL11_1();
PlatformParameters ES3_D3D11_FL11_0();
PlatformParameters ES3_D3D11_FL10_1();
PlatformParameters ES31_D3D11();
PlatformParameters ES31_D3D11_FL11_1();
PlatformParameters ES31_D3D11_FL11_0();

PlatformParameters ES3_D3D11_WARP();
PlatformParameters ES3_D3D11_FL11_1_WARP();
PlatformParameters ES3_D3D11_FL11_0_WARP();
PlatformParameters ES3_D3D11_FL10_1_WARP();

PlatformParameters ES1_OPENGL();
PlatformParameters ES2_OPENGL();
PlatformParameters ES2_OPENGL(EGLint major, EGLint minor);
PlatformParameters ES3_OPENGL();
PlatformParameters ES3_OPENGL(EGLint major, EGLint minor);
PlatformParameters ES31_OPENGL();
PlatformParameters ES31_OPENGL(EGLint major, EGLint minor);

PlatformParameters ES1_OPENGLES();
PlatformParameters ES2_OPENGLES();
PlatformParameters ES2_OPENGLES(EGLint major, EGLint minor);
PlatformParameters ES3_OPENGLES();
PlatformParameters ES3_OPENGLES(EGLint major, EGLint minor);
PlatformParameters ES31_OPENGLES();
PlatformParameters ES31_OPENGLES(EGLint major, EGLint minor);

PlatformParameters ES1_NULL();
PlatformParameters ES2_NULL();
PlatformParameters ES3_NULL();
PlatformParameters ES31_NULL();

PlatformParameters ES1_VULKAN();
PlatformParameters ES1_VULKAN_NULL();
PlatformParameters ES1_VULKAN_SWIFTSHADER();
PlatformParameters ES2_VULKAN();
PlatformParameters ES2_VULKAN_NULL();
PlatformParameters ES2_VULKAN_SWIFTSHADER();
PlatformParameters ES3_VULKAN();
PlatformParameters ES3_VULKAN_NULL();
PlatformParameters ES3_VULKAN_SWIFTSHADER();
PlatformParameters ES31_VULKAN();
PlatformParameters ES31_VULKAN_NULL();
PlatformParameters ES31_VULKAN_SWIFTSHADER();
PlatformParameters ES32_VULKAN();
PlatformParameters ES32_VULKAN_NULL();
PlatformParameters ES32_VULKAN_SWIFTSHADER();

PlatformParameters ES1_METAL();
PlatformParameters ES2_METAL();
PlatformParameters ES3_METAL();

PlatformParameters ES2_WGL();
PlatformParameters ES3_WGL();

PlatformParameters ES1_EGL();
PlatformParameters ES2_EGL();
PlatformParameters ES3_EGL();
PlatformParameters ES31_EGL();
PlatformParameters ES32_EGL();

PlatformParameters ES1_ANGLE_Vulkan_Secondaries();
PlatformParameters ES2_ANGLE_Vulkan_Secondaries();
PlatformParameters ES3_ANGLE_Vulkan_Secondaries();
PlatformParameters ES31_ANGLE_Vulkan_Secondaries();
PlatformParameters ES32_ANGLE_Vulkan_Secondaries();

PlatformParameters ES2_WEBGPU();
PlatformParameters ES3_WEBGPU();

PlatformParameters ES1_Zink();
PlatformParameters ES2_Zink();
PlatformParameters ES3_Zink();
PlatformParameters ES31_Zink();
PlatformParameters ES32_Zink();

const char *GetNativeEGLLibraryNameWithExtension();

inline PlatformParameters WithNoFixture(const PlatformParameters &params)
{
    PlatformParameters withNoFixture = params;
    withNoFixture.noFixture          = true;
    return withNoFixture;
}

inline PlatformParameters WithRobustness(const PlatformParameters &params)
{
    PlatformParameters withRobustness       = params;
    withRobustness.eglParameters.robustness = EGL_TRUE;
    return withRobustness;
}

inline PlatformParameters WithLowPowerGPU(const PlatformParameters &paramsIn)
{
    PlatformParameters paramsOut                   = paramsIn;
    paramsOut.eglParameters.displayPowerPreference = EGL_LOW_POWER_ANGLE;
    return paramsOut;
}

inline PlatformParameters WithHighPowerGPU(const PlatformParameters &paramsIn)
{
    PlatformParameters paramsOut                   = paramsIn;
    paramsOut.eglParameters.displayPowerPreference = EGL_HIGH_POWER_ANGLE;
    return paramsOut;
}

inline PlatformParameters WithVulkanSecondaries(const PlatformParameters &params)
{
    PlatformParameters paramsOut = params;
    paramsOut.driver             = GLESDriverType::AngleVulkanSecondariesEGL;
    return paramsOut;
}
}  // namespace angle

#endif  // ANGLE_TEST_CONFIGS_H_
