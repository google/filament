//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// APPLE_clip_distance_test.cpp:
//   Test for APPLE_clip_distance
//

#include "tests/test_utils/ShaderExtensionTest.h"

namespace
{
const char EXTPragma[] = "#extension GL_APPLE_clip_distance : require\n";

// Shader using gl_ClipDistance
const char ESSL100_APPLEClipDistanceShader1[] =
    R"(
    uniform vec4 uPlane;

    attribute vec4 aPosition;

    void main()
    {
        gl_Position = aPosition;
        gl_ClipDistance[1] = dot(aPosition, uPlane);
    })";

// Shader redeclares gl_ClipDistance
const char ESSL100_APPLEClipDistanceShader2[] =
    R"(
    uniform vec4 uPlane;

    attribute vec4 aPosition;

    varying highp float gl_ClipDistance[4];

    void main()
    {
        gl_Position = aPosition;
        gl_ClipDistance[gl_MaxClipDistances - 6 + 1] = dot(aPosition, uPlane);
        gl_ClipDistance[gl_MaxClipDistances - int(aPosition.x)] = dot(aPosition, uPlane);
    })";

// ESSL 3.00 Shader using gl_ClipDistance
const char ESSL300_APPLEClipDistanceShader1[] =
    R"(
    uniform vec4 uPlane;

    in vec4 aPosition;

    void main()
    {
        gl_Position = aPosition;
        gl_ClipDistance[1] = dot(aPosition, uPlane);
    })";

// ESSL 3.00 Shader redeclares gl_ClipDistance
const char ESSL300_APPLEClipDistanceShader2[] =
    R"(
    uniform vec4 uPlane;

    in vec4 aPosition;

    out highp float gl_ClipDistance[4];

    void main()
    {
        gl_Position = aPosition;
        gl_ClipDistance[gl_MaxClipDistances - 6 + 1] = dot(aPosition, uPlane);
        gl_ClipDistance[gl_MaxClipDistances - int(aPosition.x)] = dot(aPosition, uPlane);
    })";

class APPLEClipDistanceTest : public sh::ShaderExtensionTest
{
  public:
    void InitializeCompiler() { InitializeCompiler(SH_GLSL_130_OUTPUT); }
    void InitializeCompiler(ShShaderOutput shaderOutputType)
    {
        DestroyCompiler();

        mCompiler = sh::ConstructCompiler(GL_VERTEX_SHADER, testing::get<0>(GetParam()),
                                          shaderOutputType, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }

    testing::AssertionResult TestShaderCompile(const char *pragma)
    {
        const char *shaderStrings[] = {testing::get<1>(GetParam()), pragma,
                                       testing::get<2>(GetParam())};

        ShCompileOptions compileOptions = {};
        compileOptions.objectCode       = true;

        bool success = sh::Compile(mCompiler, shaderStrings, 3, compileOptions);
        if (success)
        {
            return ::testing::AssertionSuccess() << "Compilation success";
        }
        return ::testing::AssertionFailure() << sh::GetInfoLog(mCompiler);
    }
};

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(APPLEClipDistanceTest, CompileFailsWithoutExtension)
{
    mResources.APPLE_clip_distance = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(APPLEClipDistanceTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.APPLE_clip_distance = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(APPLEClipDistanceTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.APPLE_clip_distance = 1;
    mResources.MaxClipDistances    = 8;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}

#if defined(ANGLE_ENABLE_VULKAN)
// With extension flag and extension directive, compiling using TranslatorVulkan succeeds.
TEST_P(APPLEClipDistanceTest, CompileSucceedsVulkan)
{
    mResources.APPLE_clip_distance = 1;
    mResources.MaxClipDistances    = 8;

    InitializeCompiler(SH_SPIRV_VULKAN_OUTPUT);
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}

// Test that the SPIR-V gen path can compile a shader when this extension is not supported.
TEST_P(APPLEClipDistanceTest, CompileSucceedsWithoutExtSupportVulkan)
{
    mResources.APPLE_clip_distance = 0;
    mResources.MaxClipDistances    = 0;
    mResources.MaxCullDistances    = 0;

    InitializeCompiler(SH_SPIRV_VULKAN_OUTPUT);

    constexpr char kNoClipCull[] = R"(
    void main()
    {
        gl_Position = vec4(0);
    })";
    const char *shaderStrings[]  = {kNoClipCull};

    ShCompileOptions compileOptions = {};
    compileOptions.objectCode       = true;

    bool success = sh::Compile(mCompiler, shaderStrings, 1, compileOptions);
    if (success)
    {
        ::testing::AssertionSuccess() << "Compilation success";
    }
    else
    {
        ::testing::AssertionFailure() << sh::GetInfoLog(mCompiler);
    }

    EXPECT_TRUE(success);
}
#endif

#if defined(ANGLE_ENABLE_METAL)
// With extension flag and extension directive, compiling using TranslatorMSL succeeds.
TEST_P(APPLEClipDistanceTest, CompileSucceedsMetal)
{
    mResources.APPLE_clip_distance = 1;
    mResources.MaxClipDistances    = 8;

    InitializeCompiler(SH_MSL_METAL_OUTPUT);
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}
#endif

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         APPLEClipDistanceTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_APPLEClipDistanceShader1,
                                        ESSL100_APPLEClipDistanceShader2)));

INSTANTIATE_TEST_SUITE_P(CorrectESSL300Shaders,
                         APPLEClipDistanceTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_APPLEClipDistanceShader1,
                                        ESSL300_APPLEClipDistanceShader2)));

}  // anonymous namespace
