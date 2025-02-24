//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawCallPerfParams.h:
//   Parametrization for performance tests for ANGLE draw call overhead.
//

#ifndef TESTS_PERF_TESTS_DRAW_CALL_PERF_PARAMS_H_
#define TESTS_PERF_TESTS_DRAW_CALL_PERF_PARAMS_H_

#include <ostream>

#include "ANGLEPerfTest.h"
#include "test_utils/angle_test_configs.h"

struct DrawCallPerfParams : public RenderTestParams
{
    // Common default options
    DrawCallPerfParams();
    ~DrawCallPerfParams() override;

    std::string story() const override;

    double runTimeSeconds;
    int numTris;
};

namespace params
{
template <typename ParamsT>
ParamsT D3D11(const ParamsT &in)
{
    ParamsT out       = in;
    out.eglParameters = angle::egl_platform::D3D11();
    return out;
}

template <typename ParamsT>
ParamsT Metal(const ParamsT &in)
{
    ParamsT out       = in;
    out.eglParameters = angle::egl_platform::METAL();
    return out;
}

template <typename ParamsT>
ParamsT GL(const ParamsT &in)
{
    ParamsT out       = in;
    out.eglParameters = angle::egl_platform::OPENGL_OR_GLES();
    return out;
}

template <typename ParamsT>
ParamsT GL3(const ParamsT &in)
{
    ParamsT out       = in;
    out.eglParameters = angle::egl_platform::OPENGL_OR_GLES(3, 0);
    return out;
}

template <typename ParamsT>
ParamsT Vulkan(const ParamsT &in)
{
    ParamsT out       = in;
    out.eglParameters = angle::egl_platform::VULKAN();
    return out;
}

template <typename ParamsT>
ParamsT VulkanMockICD(const ParamsT &in)
{
    ParamsT out       = in;
    out.eglParameters = angle::egl_platform::VULKAN_NULL();
    return out;
}

template <typename ParamsT>
ParamsT VulkanSwiftShader(const ParamsT &in)
{
    ParamsT out       = in;
    out.eglParameters = angle::egl_platform::VULKAN_SWIFTSHADER();
    return out;
}

template <typename ParamsT>
ParamsT WGL(const ParamsT &in)
{
    ParamsT out = in;
    out.driver  = angle::GLESDriverType::SystemWGL;
    return out;
}

template <typename ParamsT>
ParamsT EGL(const ParamsT &in)
{
    ParamsT out = in;
    out.driver  = angle::GLESDriverType::SystemEGL;
    return out;
}

template <typename ParamsT>
ParamsT Zink(const ParamsT &in)
{
    ParamsT out = in;
    out.driver  = angle::GLESDriverType::ZinkEGL;
    return out;
}

template <typename ParamsT>
ParamsT Native(const ParamsT &in)
{
#if defined(ANGLE_PLATFORM_WINDOWS)
    return WGL(in);
#else
    return EGL(in);
#endif
}
}  // namespace params

#endif  // TESTS_PERF_TESTS_DRAW_CALL_PERF_PARAMS_H_
