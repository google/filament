//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GPUTestExpectationsTest.cpp : Tests of the test_expectations library.

#include "test_expectations/GPUTestConfig.h"
#include "test_expectations/GPUTestExpectationsParser.h"
#include "test_utils/ANGLETest.h"

namespace angle
{

class GPUTestConfigTest : public ANGLETest<>
{
  protected:
    GPUTestConfigTest() {}

    // todo(jonahr): Eventually could add support for all conditions/operating
    // systems, but these are the ones in use for now
    void validateConfigBase(const GPUTestConfig &config)
    {
        EXPECT_EQ(IsWindows(), config.getConditions()[GPUTestConfig::kConditionWin]);
        EXPECT_EQ(IsMac(), config.getConditions()[GPUTestConfig::kConditionMac]);
        EXPECT_EQ(IsIOS(), config.getConditions()[GPUTestConfig::kConditionIOS]);
        EXPECT_EQ(IsLinux(), config.getConditions()[GPUTestConfig::kConditionLinux]);
        EXPECT_EQ(IsAndroid(), config.getConditions()[GPUTestConfig::kConditionAndroid]);
        EXPECT_EQ(IsNexus5X(), config.getConditions()[GPUTestConfig::kConditionNexus5X]);
        EXPECT_EQ((IsPixel2() || IsPixel2XL()),
                  config.getConditions()[GPUTestConfig::kConditionPixel2OrXL]);
        EXPECT_EQ(IsIntel(), config.getConditions()[GPUTestConfig::kConditionIntel]);
        EXPECT_EQ(IsAMD(), config.getConditions()[GPUTestConfig::kConditionAMD]);
        EXPECT_EQ(IsNVIDIA(), config.getConditions()[GPUTestConfig::kConditionNVIDIA]);
        EXPECT_EQ(IsDebug(), config.getConditions()[GPUTestConfig::kConditionDebug]);
        EXPECT_EQ(IsRelease(), config.getConditions()[GPUTestConfig::kConditionRelease]);
        EXPECT_EQ(IsASan(), config.getConditions()[GPUTestConfig::kConditionASan]);
        EXPECT_EQ(IsTSan(), config.getConditions()[GPUTestConfig::kConditionTSan]);
        EXPECT_EQ(IsUBSan(), config.getConditions()[GPUTestConfig::kConditionUBSan]);
    }

    void validateConfigAPI(const GPUTestConfig &config,
                           const GPUTestConfig::API &api,
                           uint32_t preRotation)
    {
        bool D3D9      = false;
        bool D3D11     = false;
        bool GLDesktop = false;
        bool GLES      = false;
        bool Vulkan    = false;
        bool Metal     = false;
        bool Wgpu      = false;
        switch (api)
        {
            case GPUTestConfig::kAPID3D9:
                D3D9 = true;
                break;
            case GPUTestConfig::kAPID3D11:
                D3D11 = true;
                break;
            case GPUTestConfig::kAPIGLDesktop:
                GLDesktop = true;
                break;
            case GPUTestConfig::kAPIGLES:
                GLES = true;
                break;
            case GPUTestConfig::kAPIVulkan:
                Vulkan = true;
                break;
            case GPUTestConfig::kAPIMetal:
                Metal = true;
                break;
            case GPUTestConfig::kAPIWgpu:
                Wgpu = true;
                break;
            case GPUTestConfig::kAPIUnknown:
            default:
                break;
        }
        EXPECT_EQ(D3D9, config.getConditions()[GPUTestConfig::kConditionD3D9]);
        EXPECT_EQ(D3D11, config.getConditions()[GPUTestConfig::kConditionD3D11]);
        EXPECT_EQ(GLDesktop, config.getConditions()[GPUTestConfig::kConditionGLDesktop]);
        EXPECT_EQ(GLES, config.getConditions()[GPUTestConfig::kConditionGLES]);
        EXPECT_EQ(Vulkan, config.getConditions()[GPUTestConfig::kConditionVulkan]);
        EXPECT_EQ(Metal, config.getConditions()[GPUTestConfig::kConditionMetal]);
        EXPECT_EQ(Wgpu, config.getConditions()[GPUTestConfig::kConditionWgpu]);

        switch (preRotation)
        {
            case 90:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionPreRotation]);
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionPreRotation90]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation180]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation270]);
                break;
            case 180:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionPreRotation]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation90]);
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionPreRotation180]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation270]);
                break;
            case 270:
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionPreRotation]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation90]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation180]);
                EXPECT_TRUE(config.getConditions()[GPUTestConfig::kConditionPreRotation270]);
                break;
            default:
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation90]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation180]);
                EXPECT_FALSE(config.getConditions()[GPUTestConfig::kConditionPreRotation270]);
                break;
        }
    }
};

// Create a new GPUTestConfig and make sure all the condition flags were set
// correctly based on the hardware.
TEST_P(GPUTestConfigTest, GPUTestConfigConditions)
{
    GPUTestConfig config;
    validateConfigBase(config);
}

// Create a new GPUTestConfig with each backend specified and validate the
// condition flags are set correctly.
TEST_P(GPUTestConfigTest, GPUTestConfigConditions_D3D9)
{
    GPUTestConfig config(GPUTestConfig::kAPID3D9, 0);
    validateConfigAPI(config, GPUTestConfig::kAPID3D9, 0);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_D3D11)
{
    GPUTestConfig config(GPUTestConfig::kAPID3D11, 0);
    validateConfigAPI(config, GPUTestConfig::kAPID3D11, 0);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_Metal)
{
    GPUTestConfig config(GPUTestConfig::kAPIMetal, 0);
    validateConfigAPI(config, GPUTestConfig::kAPIMetal, 0);
}

// Create a new GPUTestConfig with webgpu and validate the
// condition flags are set correctly.
TEST_P(GPUTestConfigTest, GPUTestConfigConditions_Wgpu)
{
    GPUTestConfig config(GPUTestConfig::kAPIWgpu, 0);
    validateConfigAPI(config, GPUTestConfig::kAPIWgpu, 0);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_GLDesktop)
{
    GPUTestConfig config(GPUTestConfig::kAPIGLDesktop, 0);
    validateConfigAPI(config, GPUTestConfig::kAPIGLDesktop, 0);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_GLES)
{
    GPUTestConfig config(GPUTestConfig::kAPIGLES, 0);
    validateConfigAPI(config, GPUTestConfig::kAPIGLES, 0);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_Vulkan)
{
    GPUTestConfig config(GPUTestConfig::kAPIVulkan, 0);
    validateConfigAPI(config, GPUTestConfig::kAPIVulkan, 0);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_Vulkan_PreRotation90)
{
    GPUTestConfig config(GPUTestConfig::kAPIVulkan, 90);
    validateConfigAPI(config, GPUTestConfig::kAPIVulkan, 90);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_Vulkan_PreRotation180)
{
    GPUTestConfig config(GPUTestConfig::kAPIVulkan, 180);
    validateConfigAPI(config, GPUTestConfig::kAPIVulkan, 180);
}

TEST_P(GPUTestConfigTest, GPUTestConfigConditions_Vulkan_PreRotation270)
{
    GPUTestConfig config(GPUTestConfig::kAPIVulkan, 270);
    validateConfigAPI(config, GPUTestConfig::kAPIVulkan, 270);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(GPUTestConfigTest);

}  // namespace angle
