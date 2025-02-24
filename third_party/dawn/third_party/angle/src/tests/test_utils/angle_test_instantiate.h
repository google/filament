//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angle_test_instantiate.h: Adds support for filtering parameterized
// tests by platform, so we skip unsupported configs.

#ifndef ANGLE_TEST_INSTANTIATE_H_
#define ANGLE_TEST_INSTANTIATE_H_

#include <gtest/gtest.h>

#include "common/platform_helpers.h"

namespace angle
{
struct SystemInfo;
struct PlatformParameters;

// Operating systems
bool IsOzone();

// CPU architectures
bool IsARM64();

// Android devices
bool IsNexus5X();
bool IsNexus9();
bool IsPixelXL();
bool IsPixel2();
bool IsPixel2XL();
bool IsPixel4();
bool IsPixel4XL();
bool IsPixel6();
bool IsGalaxyS22();
bool IsNVIDIAShield();

// Android versions
bool IsAndroid14OrNewer();

// GPU vendors.
bool IsIntel();
bool IsAMD();
bool IsAppleGPU();
bool IsARM();
bool IsNVIDIA();
bool IsQualcomm();

// GPU devices.
bool IsSwiftshaderDevice();
bool IsIntelUHD630Mobile();

bool HasMesa();

bool IsPlatformAvailable(const PlatformParameters &param);

// This functions is used to filter which tests should be registered,
// T must be or inherit from angle::PlatformParameters.
template <typename T>
std::vector<T> FilterTestParams(const T *params, size_t numParams)
{
    std::vector<T> filtered;

    for (size_t i = 0; i < numParams; i++)
    {
        if (IsPlatformAvailable(params[i]))
        {
            filtered.push_back(params[i]);
        }
    }

    return filtered;
}

template <typename T>
std::vector<T> FilterTestParams(const std::vector<T> &params)
{
    return FilterTestParams(params.data(), params.size());
}

// Used to generate valid test names out of testing::PrintToStringParamName used in combined tests.
struct CombinedPrintToStringParamName
{
    template <class ParamType>
    std::string operator()(const testing::TestParamInfo<ParamType> &info) const
    {
        std::string name = testing::PrintToStringParamName()(info);
        std::string sanitized;
        for (const char c : name)
        {
            if (c == ',')
            {
                sanitized += '_';
            }
            else if (isalnum(c) || c == '_')
            {
                sanitized += c;
            }
        }
        return sanitized;
    }
};

#define ANGLE_INSTANTIATE_TEST_PLATFORMS(testName, ...)                        \
    testing::ValuesIn(::angle::FilterTestParams(testName##__VA_ARGS__##params, \
                                                ArraySize(testName##__VA_ARGS__##params)))

// Instantiate the test once for each extra argument. The types of all the
// arguments must match, and getRenderer must be implemented for that type.
#define ANGLE_INSTANTIATE_TEST(testName, first, ...)                                         \
    const std::remove_reference<decltype(first)>::type testName##params[] = {first,          \
                                                                             ##__VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),         \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ARRAY(testName, valuesin)                                         \
    INSTANTIATE_TEST_SUITE_P(, testName, testing::ValuesIn(::angle::FilterTestParams(valuesin)), \
                             testing::PrintToStringParamName())

#if !defined(ANGLE_TEST_ENABLE_SYSTEM_EGL)
#    define ANGLE_TEST_PLATFORMS_ES1_SYSTEM_EGL
#    define ANGLE_TEST_PLATFORMS_ES2_SYSTEM_EGL
#    define ANGLE_TEST_PLATFORMS_ES3_SYSTEM_EGL
#    define ANGLE_TEST_PLATFORMS_ES31_SYSTEM_EGL
#    define ANGLE_TEST_PLATFORMS_ES32_SYSTEM_EGL
#else
#    define ANGLE_TEST_PLATFORMS_ES1_SYSTEM_EGL ES1_EGL(),
#    define ANGLE_TEST_PLATFORMS_ES2_SYSTEM_EGL ES2_EGL(),
#    define ANGLE_TEST_PLATFORMS_ES3_SYSTEM_EGL ES3_EGL(),
#    define ANGLE_TEST_PLATFORMS_ES31_SYSTEM_EGL ES31_EGL(),
#    define ANGLE_TEST_PLATFORMS_ES32_SYSTEM_EGL ES32_EGL(),
#endif

#define ANGLE_ALL_TEST_PLATFORMS_ES1                                      \
    ANGLE_TEST_PLATFORMS_ES1_SYSTEM_EGL                                   \
    ES1_D3D11(), ES1_METAL(), ES1_OPENGL(), ES1_OPENGLES(), ES1_VULKAN(), \
        ES1_VULKAN_SWIFTSHADER(), ES1_VULKAN().enable(Feature::EnableParallelCompileAndLink)

#define ANGLE_ALL_TEST_PLATFORMS_ES2                                                               \
    ANGLE_TEST_PLATFORMS_ES2_SYSTEM_EGL                                                            \
    ES2_D3D9(), ES2_D3D11(), ES2_OPENGL(), ES2_OPENGLES(), ES2_VULKAN(), ES2_VULKAN_SWIFTSHADER(), \
        ES2_METAL(),                                                                               \
        ES2_VULKAN()                                                                               \
            .enable(Feature::EnableParallelCompileAndLink)                                         \
            .enable(Feature::VaryingsRequireMatchingPrecisionInSpirv),                             \
        ES2_VULKAN_SWIFTSHADER()                                                                   \
            .enable(Feature::EnableParallelCompileAndLink)                                         \
            .disable(Feature::SupportsGraphicsPipelineLibrary)

#define ANGLE_ALL_TEST_PLATFORMS_ES3                                                   \
    ANGLE_TEST_PLATFORMS_ES3_SYSTEM_EGL                                                \
    ES3_D3D11(), ES3_OPENGL(), ES3_OPENGLES(), ES3_VULKAN(), ES3_VULKAN_SWIFTSHADER(), \
        ES3_METAL(),                                                                   \
        ES3_VULKAN()                                                                   \
            .enable(Feature::EnableParallelCompileAndLink)                             \
            .enable(Feature::VaryingsRequireMatchingPrecisionInSpirv),                 \
        ES3_VULKAN_SWIFTSHADER()                                                       \
            .enable(Feature::EnableParallelCompileAndLink)                             \
            .disable(Feature::SupportsGraphicsPipelineLibrary)                         \
            .enable(Feature::VaryingsRequireMatchingPrecisionInSpirv)

#define ANGLE_ALL_TEST_PLATFORMS_ES31                                                       \
    ANGLE_TEST_PLATFORMS_ES31_SYSTEM_EGL                                                    \
    ES31_D3D11(), ES31_OPENGL(), ES31_OPENGLES(), ES31_VULKAN(), ES31_VULKAN_SWIFTSHADER(), \
        ES31_VULKAN()                                                                       \
            .enable(Feature::EnableParallelCompileAndLink)                                  \
            .enable(Feature::VaryingsRequireMatchingPrecisionInSpirv),                      \
        ES31_VULKAN_SWIFTSHADER()                                                           \
            .enable(Feature::EnableParallelCompileAndLink)                                  \
            .disable(Feature::SupportsGraphicsPipelineLibrary)                              \
            .enable(Feature::VaryingsRequireMatchingPrecisionInSpirv)

#define ANGLE_ALL_TEST_PLATFORMS_ES32                                 \
    ANGLE_TEST_PLATFORMS_ES32_SYSTEM_EGL                              \
    ES32_VULKAN(), ES32_VULKAN()                                      \
                       .enable(Feature::EnableParallelCompileAndLink) \
                       .enable(Feature::VaryingsRequireMatchingPrecisionInSpirv)

#define ANGLE_ALL_TEST_PLATFORMS_NULL ES2_NULL(), ES3_NULL(), ES31_NULL()

// Instantiate the test once for each GLES1 platform
#define ANGLE_INSTANTIATE_TEST_ES1(testName)                                         \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES1};    \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

// Instantiate the test once for each GLES2 platform
#define ANGLE_INSTANTIATE_TEST_ES2(testName)                                         \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES2};    \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES2_AND(testName, ...)                                          \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES2, __VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),           \
                             testing::PrintToStringParamName())

// Instantiate the test once for each GLES3 platform
#define ANGLE_INSTANTIATE_TEST_ES3(testName)                                         \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES3};    \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES3_AND(testName, ...)                                          \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES3, __VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),           \
                             testing::PrintToStringParamName())

// Instantiate the test once for each GLES31 platform
#define ANGLE_INSTANTIATE_TEST_ES31(testName)                                        \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES31};   \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES31_AND(testName, ...)                                          \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES31, __VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),            \
                             testing::PrintToStringParamName())

// Instantiate the test once for each GLES32 platform
#define ANGLE_INSTANTIATE_TEST_ES32(testName)                                        \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES32};   \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES32_AND(testName, ...)                                          \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES32, __VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),            \
                             testing::PrintToStringParamName())

// Multiple ES Version macros
#define ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(testName)                                 \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES2,     \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES3};    \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(testName, ...)                                  \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES2,               \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES3, __VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),           \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31(testName)                        \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES2,     \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES3,     \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES31};   \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31_AND(testName, ...)                          \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES2,                \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES3,                \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES31, __VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),            \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31_AND_ES32(testName, ...)                        \
    const PlatformParameters testName##params[] = {                                                \
        ANGLE_ALL_TEST_PLATFORMS_ES2, ANGLE_ALL_TEST_PLATFORMS_ES3, ANGLE_ALL_TEST_PLATFORMS_ES31, \
        ANGLE_ALL_TEST_PLATFORMS_ES32, __VA_ARGS__};                                               \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),               \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31_AND_NULL(testName)                             \
    const PlatformParameters testName##params[] = {                                                \
        ANGLE_ALL_TEST_PLATFORMS_ES2, ANGLE_ALL_TEST_PLATFORMS_ES3, ANGLE_ALL_TEST_PLATFORMS_ES31, \
        ANGLE_ALL_TEST_PLATFORMS_NULL};                                                            \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),               \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31_AND_NULL_AND(testName, ...)                    \
    const PlatformParameters testName##params[] = {                                                \
        ANGLE_ALL_TEST_PLATFORMS_ES2, ANGLE_ALL_TEST_PLATFORMS_ES3, ANGLE_ALL_TEST_PLATFORMS_ES31, \
        ANGLE_ALL_TEST_PLATFORMS_NULL, __VA_ARGS__};                                               \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),               \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(testName)                                \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES3,     \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES31};   \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), \
                             testing::PrintToStringParamName())

#define ANGLE_INSTANTIATE_TEST_ES3_AND_ES31_AND(testName, ...)                                  \
    const PlatformParameters testName##params[] = {ANGLE_ALL_TEST_PLATFORMS_ES3,                \
                                                   ANGLE_ALL_TEST_PLATFORMS_ES31, __VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(, testName, ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),            \
                             testing::PrintToStringParamName())

// Instantiate the test for a combination of N parameters and the
// enumeration of platforms in the extra args, similar to
// ANGLE_INSTANTIATE_TEST.  The macros are defined only for the Ns
// currently in use, and can be expanded as necessary.
#define ANGLE_INSTANTIATE_TEST_COMBINE_1(testName, print, combine1, first, ...)              \
    const std::remove_reference<decltype(first)>::type testName##params[] = {first,          \
                                                                             ##__VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(                                                                \
        , testName, testing::Combine(ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), combine1), print)
#define ANGLE_INSTANTIATE_TEST_COMBINE_2(testName, print, combine1, combine2, first, ...)    \
    const std::remove_reference<decltype(first)>::type testName##params[] = {first,          \
                                                                             ##__VA_ARGS__}; \
    INSTANTIATE_TEST_SUITE_P(                                                                \
        , testName,                                                                          \
        testing::Combine(ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), combine1, combine2), print)
#define ANGLE_INSTANTIATE_TEST_COMBINE_3(testName, print, combine1, combine2, combine3, first, \
                                         ...)                                                  \
    const std::remove_reference<decltype(first)>::type testName##params[] = {first,            \
                                                                             ##__VA_ARGS__};   \
    INSTANTIATE_TEST_SUITE_P(, testName,                                                       \
                             testing::Combine(ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),      \
                                              combine1, combine2, combine3),                   \
                             print)
#define ANGLE_INSTANTIATE_TEST_COMBINE_4(testName, print, combine1, combine2, combine3, combine4, \
                                         first, ...)                                              \
    const std::remove_reference<decltype(first)>::type testName##params[] = {first,               \
                                                                             ##__VA_ARGS__};      \
    INSTANTIATE_TEST_SUITE_P(, testName,                                                          \
                             testing::Combine(ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),         \
                                              combine1, combine2, combine3, combine4),            \
                             print)
#define ANGLE_INSTANTIATE_TEST_COMBINE_5(testName, print, combine1, combine2, combine3, combine4, \
                                         combine5, first, ...)                                    \
    const std::remove_reference<decltype(first)>::type testName##params[] = {first,               \
                                                                             ##__VA_ARGS__};      \
    INSTANTIATE_TEST_SUITE_P(, testName,                                                          \
                             testing::Combine(ANGLE_INSTANTIATE_TEST_PLATFORMS(testName),         \
                                              combine1, combine2, combine3, combine4, combine5),  \
                             print)
#define ANGLE_INSTANTIATE_TEST_COMBINE_6(testName, print, combine1, combine2, combine3, combine4,  \
                                         combine5, combine6, first, ...)                           \
    const std::remove_reference<decltype(first)>::type testName##params[] = {first,                \
                                                                             ##__VA_ARGS__};       \
    INSTANTIATE_TEST_SUITE_P(                                                                      \
        , testName,                                                                                \
        testing::Combine(ANGLE_INSTANTIATE_TEST_PLATFORMS(testName), combine1, combine2, combine3, \
                         combine4, combine5, combine6),                                            \
        print)

// Checks if a config is expected to be supported by checking a system-based allow list.
bool IsConfigAllowlisted(const SystemInfo &systemInfo, const PlatformParameters &param);

// Determines if a config is supported by trying to initialize it. Does
// not require SystemInfo.
bool IsConfigSupported(const PlatformParameters &param);

// Returns shared test system information. Can be used globally in the
// tests.
SystemInfo *GetTestSystemInfo();

// Returns a list of all enabled test platform names. For use in
// configuration enumeration.
std::vector<std::string> GetAvailableTestPlatformNames();

// Active config (e.g. ES2_Vulkan).
void SetSelectedConfig(const char *selectedConfig);
bool IsConfigSelected();

// Check whether texture swizzle is natively supported on Metal device.
bool IsMetalTextureSwizzleAvailable();

extern bool gEnableANGLEPerTestCaptureLabel;

// For use with ANGLE_INSTANTIATE_TEST_ARRAY
template <typename ParamsT>
using ModifierFunc = std::function<ParamsT(const ParamsT &)>;

template <typename ParamsT>
std::vector<ParamsT> CombineWithFuncs(const std::vector<ParamsT> &in,
                                      const std::vector<ModifierFunc<ParamsT>> &modifiers)
{
    std::vector<ParamsT> out;
    for (const ParamsT &paramsIn : in)
    {
        for (ModifierFunc<ParamsT> modifier : modifiers)
        {
            out.push_back(modifier(paramsIn));
        }
    }
    return out;
}

template <typename ParamT, typename RangeT, typename ModifierT>
std::vector<ParamT> CombineWithValues(const std::vector<ParamT> &in,
                                      RangeT begin,
                                      RangeT end,
                                      ParamT combine(const ParamT &, ModifierT))
{
    std::vector<ParamT> out;
    for (const ParamT &paramsIn : in)
    {
        for (auto iter = begin; iter != end; ++iter)
        {
            out.push_back(combine(paramsIn, *iter));
        }
    }
    return out;
}

template <typename ParamT, typename ModifierT>
std::vector<ParamT> CombineWithValues(const std::vector<ParamT> &in,
                                      const std::initializer_list<ModifierT> &modifiers,
                                      ParamT combine(const ParamT &, ModifierT))
{
    return CombineWithValues(in, modifiers.begin(), modifiers.end(), combine);
}

template <typename ParamT, typename ModifiersT, typename ModifierT>
std::vector<ParamT> CombineWithValues(const std::vector<ParamT> &in,
                                      const ModifiersT &modifiers,
                                      ParamT combine(const ParamT &, ModifierT))
{
    return CombineWithValues(in, std::begin(modifiers), std::end(modifiers), combine);
}

template <typename ParamT, typename FilterFunc>
std::vector<ParamT> FilterWithFunc(const std::vector<ParamT> &in, FilterFunc filter)
{
    std::vector<ParamT> out;
    for (const ParamT &param : in)
    {
        if (filter(param))
        {
            out.push_back(param);
        }
    }
    return out;
}
}  // namespace angle

#define ANGLE_SKIP_TEST_IF(COND)                        \
    do                                                  \
    {                                                   \
        if (COND)                                       \
        {                                               \
            GTEST_SKIP() << "Test skipped: " #COND "."; \
            return;                                     \
        }                                               \
    } while (0)

#endif  // ANGLE_TEST_INSTANTIATE_H_
