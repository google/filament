//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLContextCompatibilityTest.cpp:
//   This test will try to use all combinations of context configs and
//   surface configs. If the configs are compatible, it checks that simple
//   rendering works, otherwise it checks an error is generated one MakeCurrent.
//

#include <gtest/gtest.h>

#include <unordered_set>
#include <vector>

#include "common/debug.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"
#include "util/OSWindow.h"
#include "util/random_utils.h"

using namespace angle;

namespace
{
// The only configs with 16-bits for each of red, green, blue, and alpha is GL_RGBA16F
bool IsRGBA16FConfig(EGLDisplay display, EGLConfig config)
{
    EGLint red, green, blue, alpha;
    eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red);
    eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue);
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alpha);
    return ((red == 16) && (green == 16) && (blue == 16) && (alpha == 16));
}

bool IsRGB10_A2Config(EGLDisplay display, EGLConfig config)
{
    EGLint red, green, blue, alpha;
    eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red);
    eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue);
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alpha);
    return ((red == 10) && (green == 10) && (blue == 10) && (alpha == 2));
}

// Queries EGL config to determine if multisampled or not
bool IsMultisampledConfig(EGLDisplay display, EGLConfig config)
{
    EGLint samples = 0;
    eglGetConfigAttrib(display, config, EGL_SAMPLES, &samples);
    return (samples > 1);
}

bool ShouldSkipConfig(EGLDisplay display, EGLConfig config, bool windowSurfaceTest)
{
    // Skip multisampled configurations due to test instability.
    if (IsMultisampledConfig(display, config))
        return true;

    // Disable RGBA16F/RGB10_A2 on Android due to OSWindow on Android not providing compatible
    // windows (http://anglebug.com/42261830)
    if (IsAndroid())
    {
        if (IsRGB10_A2Config(display, config))
            return true;

        if (IsRGBA16FConfig(display, config))
            return windowSurfaceTest;
    }

    return false;
}

std::vector<EGLConfig> GetConfigs(EGLDisplay display)
{
    int nConfigs = 0;
    if (eglGetConfigs(display, nullptr, 0, &nConfigs) != EGL_TRUE)
    {
        std::cerr << "EGLContextCompatibilityTest: eglGetConfigs error\n";
        return {};
    }
    if (nConfigs == 0)
    {
        std::cerr << "EGLContextCompatibilityTest: no configs\n";
        return {};
    }

    std::vector<EGLConfig> configs;

    int nReturnedConfigs = 0;
    configs.resize(nConfigs);
    if (eglGetConfigs(display, configs.data(), nConfigs, &nReturnedConfigs) != EGL_TRUE)
    {
        std::cerr << "EGLContextCompatibilityTest: eglGetConfigs error\n";
        return {};
    }
    if (nConfigs != nReturnedConfigs)
    {
        std::cerr << "EGLContextCompatibilityTest: eglGetConfigs returned wrong count\n";
        return {};
    }

    return configs;
}

PlatformParameters FromRenderer(EGLint renderer)
{
    return WithNoFixture(PlatformParameters(2, 0, EGLPlatformParameters(renderer)));
}

std::string EGLConfigName(EGLDisplay display, EGLConfig config)
{
    EGLint red;
    eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red);
    EGLint green;
    eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green);
    EGLint blue;
    eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue);
    EGLint alpha;
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alpha);
    EGLint depth;
    eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth);
    EGLint stencil;
    eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &stencil);
    EGLint samples;
    eglGetConfigAttrib(display, config, EGL_SAMPLES, &samples);

    std::stringstream strstr;
    if (red > 0)
    {
        strstr << "R" << red;
    }
    if (green > 0)
    {
        strstr << "G" << green;
    }
    if (blue > 0)
    {
        strstr << "B" << blue;
    }
    if (alpha > 0)
    {
        strstr << "A" << alpha;
    }
    if (depth > 0)
    {
        strstr << "D" << depth;
    }
    if (stencil > 0)
    {
        strstr << "S" << stencil;
    }
    if (samples > 0)
    {
        strstr << "MS" << samples;
    }
    return strstr.str();
}

const std::array<EGLint, 3> kContextAttribs = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

class EGLContextCompatibilityTest : public ANGLETestBase, public testing::Test
{
  public:
    EGLContextCompatibilityTest(EGLint renderer)
        : ANGLETestBase(FromRenderer(renderer)), mRenderer(renderer)
    {}

    void SetUp() final
    {
        ANGLETestBase::ANGLETestSetUp();
        ASSERT_TRUE(eglGetPlatformDisplayEXT != nullptr);

        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, mRenderer, EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_TRUE(eglInitialize(mDisplay, nullptr, nullptr) == EGL_TRUE);

        int nConfigs = 0;
        ASSERT_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &nConfigs) == EGL_TRUE);
        ASSERT_TRUE(nConfigs != 0);

        int nReturnedConfigs = 0;
        mConfigs.resize(nConfigs);
        ASSERT_TRUE(eglGetConfigs(mDisplay, mConfigs.data(), nConfigs, &nReturnedConfigs) ==
                    EGL_TRUE);
        ASSERT_TRUE(nConfigs == nReturnedConfigs);
    }

    void TearDown() final
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
        ANGLETestBase::ANGLETestTearDown();
    }

  protected:
    bool areConfigsCompatible(EGLConfig c1, EGLConfig c2, EGLint surfaceBit)
    {
        EGLint colorBufferType1, colorBufferType2;
        EGLint red1, red2, green1, green2, blue1, blue2, alpha1, alpha2;
        EGLint depth1, depth2, stencil1, stencil2;
        EGLint surfaceType1, surfaceType2;

        eglGetConfigAttrib(mDisplay, c1, EGL_COLOR_BUFFER_TYPE, &colorBufferType1);
        eglGetConfigAttrib(mDisplay, c2, EGL_COLOR_BUFFER_TYPE, &colorBufferType2);

        eglGetConfigAttrib(mDisplay, c1, EGL_RED_SIZE, &red1);
        eglGetConfigAttrib(mDisplay, c2, EGL_RED_SIZE, &red2);
        eglGetConfigAttrib(mDisplay, c1, EGL_GREEN_SIZE, &green1);
        eglGetConfigAttrib(mDisplay, c2, EGL_GREEN_SIZE, &green2);
        eglGetConfigAttrib(mDisplay, c1, EGL_BLUE_SIZE, &blue1);
        eglGetConfigAttrib(mDisplay, c2, EGL_BLUE_SIZE, &blue2);
        eglGetConfigAttrib(mDisplay, c1, EGL_ALPHA_SIZE, &alpha1);
        eglGetConfigAttrib(mDisplay, c2, EGL_ALPHA_SIZE, &alpha2);

        eglGetConfigAttrib(mDisplay, c1, EGL_DEPTH_SIZE, &depth1);
        eglGetConfigAttrib(mDisplay, c2, EGL_DEPTH_SIZE, &depth2);
        eglGetConfigAttrib(mDisplay, c1, EGL_STENCIL_SIZE, &stencil1);
        eglGetConfigAttrib(mDisplay, c2, EGL_STENCIL_SIZE, &stencil2);

        eglGetConfigAttrib(mDisplay, c1, EGL_SURFACE_TYPE, &surfaceType1);
        eglGetConfigAttrib(mDisplay, c2, EGL_SURFACE_TYPE, &surfaceType2);

        EGLint colorComponentType1 = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
        EGLint colorComponentType2 = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(mDisplay, c1, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType1);
            eglGetConfigAttrib(mDisplay, c2, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType2);
        }

        EXPECT_EGL_SUCCESS();

        return colorBufferType1 == colorBufferType2 && red1 == red2 && green1 == green2 &&
               blue1 == blue2 && alpha1 == alpha2 && colorComponentType1 == colorComponentType2 &&
               depth1 == depth2 && stencil1 == stencil2 && (surfaceType1 & surfaceBit) != 0 &&
               (surfaceType2 & surfaceBit) != 0;
    }

    void testWindowCompatibility(EGLConfig windowConfig,
                                 EGLConfig contextConfig,
                                 bool compatible) const
    {
        OSWindow *osWindow = OSWindow::New();
        ASSERT_TRUE(osWindow != nullptr);
        osWindow->initialize("EGLContextCompatibilityTest", 500, 500);

        EGLContext context =
            eglCreateContext(mDisplay, contextConfig, EGL_NO_CONTEXT, kContextAttribs.data());
        ASSERT_TRUE(context != EGL_NO_CONTEXT);

        EGLSurface window =
            eglCreateWindowSurface(mDisplay, windowConfig, osWindow->getNativeWindow(), nullptr);
        ASSERT_EGL_SUCCESS();

        if (compatible)
        {
            testClearSurface(window, windowConfig, context);
        }
        else
        {
            testMakeCurrentFails(window, context);
        }

        eglDestroySurface(mDisplay, window);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(mDisplay, context);
        ASSERT_EGL_SUCCESS();

        OSWindow::Delete(&osWindow);
    }

    void testPbufferCompatibility(EGLConfig pbufferConfig,
                                  EGLConfig contextConfig,
                                  bool compatible) const
    {
        EGLContext context =
            eglCreateContext(mDisplay, contextConfig, EGL_NO_CONTEXT, kContextAttribs.data());
        ASSERT_TRUE(context != EGL_NO_CONTEXT);

        const EGLint pBufferAttribs[] = {
            EGL_WIDTH, 500, EGL_HEIGHT, 500, EGL_NONE,
        };
        EGLSurface pbuffer = eglCreatePbufferSurface(mDisplay, pbufferConfig, pBufferAttribs);
        ASSERT_TRUE(pbuffer != EGL_NO_SURFACE);

        if (compatible)
        {
            testClearSurface(pbuffer, pbufferConfig, context);
        }
        else
        {
            testMakeCurrentFails(pbuffer, context);
        }

        eglDestroySurface(mDisplay, pbuffer);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(mDisplay, context);
        ASSERT_EGL_SUCCESS();
    }

    std::vector<EGLConfig> mConfigs;
    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLint mRenderer    = 0;

  private:
    void testClearSurface(EGLSurface surface, EGLConfig surfaceConfig, EGLContext context) const
    {
        eglMakeCurrent(mDisplay, surface, surface, context);
        ASSERT_EGL_SUCCESS();

        glViewport(0, 0, 500, 500);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        EGLint surfaceCompontentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
        if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(mDisplay, surfaceConfig, EGL_COLOR_COMPONENT_TYPE_EXT,
                               &surfaceCompontentType);
        }

        if (surfaceCompontentType == EGL_COLOR_COMPONENT_TYPE_FIXED_EXT)
        {
            EXPECT_PIXEL_EQ(250, 250, 0, 0, 255, 255);
        }
        else
        {
            EXPECT_PIXEL_32F_EQ(250, 250, 0, 0, 1.0f, 1.0f);
        }

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        ASSERT_EGL_SUCCESS();
    }

    void testMakeCurrentFails(EGLSurface surface, EGLContext context) const
    {
        eglMakeCurrent(mDisplay, surface, surface, context);
        EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    }
};

// The test is split in several subtest so that simple cases
// are tested separately. Also each surface types are not tested
// together.

// Basic test checking contexts and windows created with the
// same config can render.
class EGLContextCompatibilityTest_WindowSameConfig : public EGLContextCompatibilityTest
{
  public:
    EGLContextCompatibilityTest_WindowSameConfig(EGLint renderer, size_t configIndex)
        : EGLContextCompatibilityTest(renderer), mConfigIndex(configIndex)
    {}

    void TestBody() override
    {
        EGLConfig config = mConfigs[mConfigIndex];

        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        ANGLE_SKIP_TEST_IF((surfaceType & EGL_WINDOW_BIT) == 0);

        testWindowCompatibility(config, config, true);
    }

    EGLint mConfigIndex;
};

// Basic test checking contexts and pbuffers created with the
// same config can render.
class EGLContextCompatibilityTest_PbufferSameConfig : public EGLContextCompatibilityTest
{
  public:
    EGLContextCompatibilityTest_PbufferSameConfig(EGLint renderer, size_t configIndex)
        : EGLContextCompatibilityTest(renderer), mConfigIndex(configIndex)
    {}

    void TestBody() override
    {
        EGLConfig config = mConfigs[mConfigIndex];

        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        ANGLE_SKIP_TEST_IF((surfaceType & EGL_PBUFFER_BIT) == 0);

        testPbufferCompatibility(config, config, true);
    }

    EGLint mConfigIndex;
};

// Check that a context rendering to a window with a different
// config works or errors according to the EGL compatibility rules
class EGLContextCompatibilityTest_WindowDifferentConfig : public EGLContextCompatibilityTest
{
  public:
    EGLContextCompatibilityTest_WindowDifferentConfig(EGLint renderer,
                                                      size_t configIndexA,
                                                      size_t configIndexB)
        : EGLContextCompatibilityTest(renderer),
          mConfigIndexA(configIndexA),
          mConfigIndexB(configIndexB)
    {}

    void TestBody() override
    {
        EGLConfig config1 = mConfigs[mConfigIndexA];
        EGLConfig config2 = mConfigs[mConfigIndexB];

        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config1, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        ANGLE_SKIP_TEST_IF((surfaceType & EGL_WINDOW_BIT) == 0);

        testWindowCompatibility(config1, config2,
                                areConfigsCompatible(config1, config2, EGL_WINDOW_BIT));
    }

    EGLint mConfigIndexA;
    EGLint mConfigIndexB;
};

// Check that a context rendering to a pbuffer with a different
// config works or errors according to the EGL compatibility rules
class EGLContextCompatibilityTest_PbufferDifferentConfig : public EGLContextCompatibilityTest
{
  public:
    EGLContextCompatibilityTest_PbufferDifferentConfig(EGLint renderer,
                                                       size_t configIndexA,
                                                       size_t configIndexB)
        : EGLContextCompatibilityTest(renderer),
          mConfigIndexA(configIndexA),
          mConfigIndexB(configIndexB)
    {}

    void TestBody() override
    {
        EGLConfig config1 = mConfigs[mConfigIndexA];
        EGLConfig config2 = mConfigs[mConfigIndexB];

        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config1, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        ANGLE_SKIP_TEST_IF((surfaceType & EGL_PBUFFER_BIT) == 0);

        testPbufferCompatibility(config1, config2,
                                 areConfigsCompatible(config1, config2, EGL_PBUFFER_BIT));
    }

    EGLint mConfigIndexA;
    EGLint mConfigIndexB;
};
}  // namespace

void RegisterContextCompatibilityTests()
{
    // Linux failures: http://anglebug.com/42263563
    // Also wrong drivers loaded under xvfb due to egl* calls: https://anglebug.com/42266535
    if (IsLinux())
    {
        std::cerr << "EGLContextCompatibilityTest: skipped on Linux\n";
        return;
    }

    std::vector<EGLint> renderers = {{
        EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE,
    }};

    LoadEntryPointsWithUtilLoader(angle::GLESDriverType::AngleEGL);

    if (eglGetPlatformDisplayEXT == nullptr)
    {
        std::cerr << "EGLContextCompatibilityTest: missing eglGetPlatformDisplayEXT\n";
        return;
    }

    for (EGLint renderer : renderers)
    {
        PlatformParameters params = FromRenderer(renderer);
        if (!IsPlatformAvailable(params))
            continue;

        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, renderer, EGL_NONE};
        EGLDisplay display = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        if (display == EGL_NO_DISPLAY)
        {
            std::cerr << "EGLContextCompatibilityTest: eglGetPlatformDisplayEXT error\n";
            return;
        }

        if (eglInitialize(display, nullptr, nullptr) != EGL_TRUE)
        {
            std::cerr << "EGLContextCompatibilityTest: eglInitialize error\n";
            return;
        }

        std::vector<EGLConfig> configs;
        std::vector<std::string> configNames;
        std::string rendererName = GetRendererName(renderer);

        {
            std::unordered_set<std::string> configNameSet;

            for (EGLConfig config : GetConfigs(display))
            {
                std::string configName = EGLConfigName(display, config);
                // Skip configs with duplicate names
                if (configNameSet.count(configName) == 0)
                {
                    configNames.push_back(configName);
                    configNameSet.insert(configName);
                    configs.push_back(config);
                }
            }
        }

        for (size_t configIndex = 0; configIndex < configs.size(); ++configIndex)
        {
            if (ShouldSkipConfig(display, configs[configIndex], true))
                continue;

            std::stringstream nameStr;
            nameStr << "WindowSameConfig/" << rendererName << "_" << configNames[configIndex];
            std::string name = nameStr.str();

            testing::RegisterTest(
                "EGLContextCompatibilityTest", name.c_str(), nullptr, nullptr, __FILE__, __LINE__,
                [renderer, configIndex]() -> EGLContextCompatibilityTest * {
                    return new EGLContextCompatibilityTest_WindowSameConfig(renderer, configIndex);
                });
        }

        for (size_t configIndex = 0; configIndex < configs.size(); ++configIndex)
        {
            if (ShouldSkipConfig(display, configs[configIndex], false))
                continue;

            std::stringstream nameStr;
            nameStr << "PbufferSameConfig/" << rendererName << "_" << configNames[configIndex];
            std::string name = nameStr.str();

            testing::RegisterTest(
                "EGLContextCompatibilityTest", name.c_str(), nullptr, nullptr, __FILE__, __LINE__,
                [renderer, configIndex]() -> EGLContextCompatibilityTest * {
                    return new EGLContextCompatibilityTest_PbufferSameConfig(renderer, configIndex);
                });
        }

        // Because there are so many permutations, we skip some configs randomly.
        // Attempt to run at most 100 tests per renderer.
        RNG rng(0);
        constexpr uint32_t kMaximumTestsPerRenderer = 100;
        const uint32_t kTestCount = static_cast<uint32_t>(configs.size() * configs.size());
        const float kSkipP =
            1.0f - (static_cast<float>(std::min(kMaximumTestsPerRenderer, kTestCount)) /
                    static_cast<float>(kTestCount));

        for (size_t configIndexA = 0; configIndexA < configs.size(); ++configIndexA)
        {
            if (ShouldSkipConfig(display, configs[configIndexA], true))
                continue;

            std::string configNameA = configNames[configIndexA];

            for (size_t configIndexB = 0; configIndexB < configs.size(); ++configIndexB)
            {
                if (ShouldSkipConfig(display, configs[configIndexB], true))
                    continue;

                if (rng.randomFloat() < kSkipP)
                    continue;

                std::string configNameB = configNames[configIndexB];

                std::stringstream nameStr;
                nameStr << "WindowDifferentConfig/" << rendererName << "_" << configNameA << "_"
                        << configNameB;
                std::string name = nameStr.str();

                testing::RegisterTest(
                    "EGLContextCompatibilityTest", name.c_str(), nullptr, nullptr, __FILE__,
                    __LINE__,
                    [renderer, configIndexA, configIndexB]() -> EGLContextCompatibilityTest * {
                        return new EGLContextCompatibilityTest_WindowDifferentConfig(
                            renderer, configIndexA, configIndexB);
                    });
            }
        }

        for (size_t configIndexA = 0; configIndexA < configs.size(); ++configIndexA)
        {
            if (ShouldSkipConfig(display, configs[configIndexA], false))
                continue;

            std::string configNameA = configNames[configIndexA];

            for (size_t configIndexB = 0; configIndexB < configs.size(); ++configIndexB)
            {
                if (ShouldSkipConfig(display, configs[configIndexB], false))
                    continue;

                if (rng.randomFloat() < kSkipP)
                    continue;

                std::string configNameB = configNames[configIndexB];

                std::stringstream nameStr;
                nameStr << "PbufferDifferentConfig/" << rendererName << "_" << configNameA << "_"
                        << configNameB;
                std::string name = nameStr.str();

                testing::RegisterTest(
                    "EGLContextCompatibilityTest", name.c_str(), nullptr, nullptr, __FILE__,
                    __LINE__,
                    [renderer, configIndexA, configIndexB]() -> EGLContextCompatibilityTest * {
                        return new EGLContextCompatibilityTest_PbufferDifferentConfig(
                            renderer, configIndexA, configIndexB);
                    });
            }
        }

        if (eglTerminate(display) == EGL_FALSE)
        {
            std::cerr << "EGLContextCompatibilityTest: eglTerminate error\n";
            return;
        }
    }
}
