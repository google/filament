//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_shader_framebuffer_fetch_test.cpp:
//   Test for EXT_shader_framebuffer_fetch and EXT_shader_framebuffer_fetch_non_coherent
//

#include "tests/test_utils/ShaderExtensionTest.h"

namespace
{
const char EXTPragma[] = "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n";

// Redeclare gl_LastFragData with noncoherent qualifier
const char ESSL100_LastFragDataRedeclared1[] =
    R"(
    uniform highp vec4 u_color;
    layout(noncoherent) highp vec4 gl_LastFragData[gl_MaxDrawBuffers];

    void main (void)
    {
        gl_FragColor = u_color + gl_LastFragData[0] + gl_LastFragData[2];
    })";

// Use inout variable with noncoherent qualifier
const char ESSL300_InOut[] =
    R"(
    layout(noncoherent, location = 0) inout highp vec4 o_color;
    uniform highp vec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, vec4(0.0f), vec4(1.0f));
    })";

// Use inout variable with noncoherent qualifier and 3-components vector
const char ESSL300_InOut2[] =
    R"(
    layout(noncoherent, location = 0) inout highp vec3 o_color;
    uniform highp vec3 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, vec3(0.0f), vec3(1.0f));
    })";

// Use inout variable with noncoherent qualifier and integer type qualifier
const char ESSL300_InOut3[] =
    R"(
    layout(noncoherent, location = 0) inout highp ivec4 o_color;
    uniform highp ivec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, ivec4(0), ivec4(1));
    })";

// Use inout variable with noncoherent qualifier and unsigned integer type qualifier
const char ESSL300_InOut4[] =
    R"(
    layout(noncoherent, location = 0) inout highp uvec4 o_color;
    uniform highp uvec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, uvec4(0), uvec4(1));
    })";

// Use inout variable with noncoherent qualifier and inout function parameter
const char ESSL300_InOut5[] =
    R"(
    layout(noncoherent, location = 0) inout highp vec4 o_color;
    uniform highp vec4 u_color;

    void getClampValue(inout highp mat4 io_color, highp vec4 i_color)
    {
        io_color[0] = clamp(io_color[0] + i_color, vec4(0.0f), vec4(1.0f));
    }

    void main (void)
    {
        highp mat4 o_color_mat = mat4(0);
        o_color_mat[0] = o_color;
        getClampValue(o_color_mat, u_color);
        o_color = o_color_mat[0];
    })";

// Use multiple inout variables with noncoherent qualifier
const char ESSL300_InOut6[] =
    R"(
    layout(noncoherent, location = 0) inout highp vec4 o_color0;
    layout(noncoherent, location = 1) inout highp vec4 o_color1;
    layout(noncoherent, location = 2) inout highp vec4 o_color2;
    layout(noncoherent, location = 3) inout highp vec4 o_color3;
    uniform highp vec4 u_color;

    void main (void)
    {
        o_color0 = clamp(o_color0 + u_color, vec4(0.0f), vec4(1.0f));
        o_color1 = clamp(o_color1 + u_color, vec4(0.0f), vec4(1.0f));
        o_color2 = clamp(o_color2 + u_color, vec4(0.0f), vec4(1.0f));
        o_color3 = clamp(o_color3 + u_color, vec4(0.0f), vec4(1.0f));
    })";

class EXTShaderFramebufferFetchNoncoherentTest : public sh::ShaderExtensionTest
{
  public:
    void SetUp() override
    {
        std::map<ShShaderOutput, std::string> shaderOutputList = {
            {SH_GLSL_450_CORE_OUTPUT, "SH_GLSL_450_CORE_OUTPUT"},
#if defined(ANGLE_ENABLE_VULKAN)
            {SH_SPIRV_VULKAN_OUTPUT, "SH_SPIRV_VULKAN_OUTPUT"}
#endif
        };

        Initialize(shaderOutputList);
    }

    void TearDown() override
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            DestroyCompiler(shaderOutputType.first);
        }
    }

    void Initialize(std::map<ShShaderOutput, std::string> &shaderOutputList)
    {
        mShaderOutputList = std::move(shaderOutputList);

        for (auto shaderOutputType : mShaderOutputList)
        {
            sh::InitBuiltInResources(&mResourceList[shaderOutputType.first]);
            mCompilerList[shaderOutputType.first] = nullptr;
        }
    }

    void DestroyCompiler(ShShaderOutput shaderOutputType)
    {
        if (mCompilerList[shaderOutputType])
        {
            sh::Destruct(mCompilerList[shaderOutputType]);
            mCompilerList[shaderOutputType] = nullptr;
        }
    }

    void InitializeCompiler()
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            InitializeCompiler(shaderOutputType.first);
        }
    }

    void InitializeCompiler(ShShaderOutput shaderOutputType)
    {
        DestroyCompiler(shaderOutputType);

        mCompilerList[shaderOutputType] =
            sh::ConstructCompiler(GL_FRAGMENT_SHADER, testing::get<0>(GetParam()), shaderOutputType,
                                  &mResourceList[shaderOutputType]);
        ASSERT_TRUE(mCompilerList[shaderOutputType] != nullptr)
            << "Compiler for " << mShaderOutputList[shaderOutputType]
            << " could not be constructed.";
    }

    testing::AssertionResult TestShaderCompile(ShShaderOutput shaderOutputType, const char *pragma)
    {
        ShCompileOptions compileOptions = {};
        compileOptions.objectCode       = true;

        const char *shaderStrings[] = {testing::get<1>(GetParam()), pragma,
                                       testing::get<2>(GetParam())};

        bool success =
            sh::Compile(mCompilerList[shaderOutputType], shaderStrings, 3, compileOptions);
        if (success)
        {
            return ::testing::AssertionSuccess()
                   << "Compilation success(" << mShaderOutputList[shaderOutputType] << ")";
        }
        return ::testing::AssertionFailure() << sh::GetInfoLog(mCompilerList[shaderOutputType]);
    }

    void TestShaderCompile(bool expectation, const char *pragma)
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            if (expectation)
            {
                EXPECT_TRUE(TestShaderCompile(shaderOutputType.first, pragma));
            }
            else
            {
                EXPECT_FALSE(TestShaderCompile(shaderOutputType.first, pragma));
            }
        }
    }

    void SetExtensionEnable(bool enable)
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            mResourceList[shaderOutputType.first].MaxDrawBuffers = 8;
            mResourceList[shaderOutputType.first].EXT_shader_framebuffer_fetch_non_coherent =
                enable;
        }
    }

  private:
    std::map<ShShaderOutput, std::string> mShaderOutputList;
    std::map<ShShaderOutput, ShHandle> mCompilerList;
    std::map<ShShaderOutput, ShBuiltInResources> mResourceList;
};

class EXTShaderFramebufferFetchNoncoherentES100Test
    : public EXTShaderFramebufferFetchNoncoherentTest
{};

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTShaderFramebufferFetchNoncoherentES100Test, CompileFailsWithoutExtension)
{
    SetExtensionEnable(false);
    InitializeCompiler();
    TestShaderCompile(false, EXTPragma);
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTShaderFramebufferFetchNoncoherentES100Test, CompileFailsWithExtensionWithoutPragma)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(false, "");
}

class EXTShaderFramebufferFetchNoncoherentES300Test
    : public EXTShaderFramebufferFetchNoncoherentTest
{};

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTShaderFramebufferFetchNoncoherentES300Test, CompileFailsWithoutExtension)
{
    SetExtensionEnable(false);
    InitializeCompiler();
    TestShaderCompile(false, EXTPragma);
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTShaderFramebufferFetchNoncoherentES300Test, CompileFailsWithExtensionWithoutPragma)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(false, "");
}

INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTShaderFramebufferFetchNoncoherentES100Test,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_LastFragDataRedeclared1)));

INSTANTIATE_TEST_SUITE_P(CorrectESSL300Shaders,
                         EXTShaderFramebufferFetchNoncoherentES300Test,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_InOut,
                                        ESSL300_InOut2,
                                        ESSL300_InOut3,
                                        ESSL300_InOut4,
                                        ESSL300_InOut5,
                                        ESSL300_InOut6)));

#if defined(ANGLE_ENABLE_VULKAN)

// Use gl_LastFragData without redeclaration of gl_LastFragData with noncoherent qualifier
const char ESSL100_LastFragDataWithoutRedeclaration[] =
    R"(
    uniform highp vec4 u_color;

    void main (void)
    {
        gl_FragColor = u_color + gl_LastFragData[0];
    })";

// Redeclare gl_LastFragData without noncoherent qualifier
const char ESSL100_LastFragDataRedeclaredWithoutNoncoherent[] =
    R"(
    uniform highp vec4 u_color;
    highp vec4 gl_LastFragData[gl_MaxDrawBuffers];

    void main (void)
    {
        gl_FragColor = u_color + gl_LastFragData[0];
    })";

// Use inout variable without noncoherent qualifier
const char ESSL300_InOutWithoutNoncoherent[] =
    R"(
    layout(location = 0) inout highp vec4 o_color;
    uniform highp vec4 u_color;

    void main (void)
    {
        o_color = clamp(o_color + u_color, vec4(0.0f), vec4(1.0f));
    })";

class EXTShaderFramebufferFetchNoncoherentSuccessTest
    : public EXTShaderFramebufferFetchNoncoherentTest
{
  public:
    void SetUp() override
    {
        std::map<ShShaderOutput, std::string> shaderOutputList = {
            {SH_SPIRV_VULKAN_OUTPUT, "SH_SPIRV_VULKAN_OUTPUT"}};

        Initialize(shaderOutputList);
    }
};

class EXTShaderFramebufferFetchNoncoherentFailureTest
    : public EXTShaderFramebufferFetchNoncoherentSuccessTest
{};

class EXTShaderFramebufferFetchNoncoherentES100SuccessTest
    : public EXTShaderFramebufferFetchNoncoherentSuccessTest
{};

class EXTShaderFramebufferFetchNoncoherentES100FailureTest
    : public EXTShaderFramebufferFetchNoncoherentFailureTest
{};

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTShaderFramebufferFetchNoncoherentES100SuccessTest, CompileSucceedsWithExtensionAndPragma)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(true, EXTPragma);
    // Test reset functionality.
    TestShaderCompile(false, "");
    TestShaderCompile(true, EXTPragma);
}

//
TEST_P(EXTShaderFramebufferFetchNoncoherentES100FailureTest, CompileFailsWithoutNoncoherent)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(false, EXTPragma);
}

class EXTShaderFramebufferFetchNoncoherentES300SuccessTest
    : public EXTShaderFramebufferFetchNoncoherentSuccessTest
{};

class EXTShaderFramebufferFetchNoncoherentES300FailureTest
    : public EXTShaderFramebufferFetchNoncoherentFailureTest
{};

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTShaderFramebufferFetchNoncoherentES300SuccessTest, CompileSucceedsWithExtensionAndPragma)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(true, EXTPragma);
    // Test reset functionality.
    TestShaderCompile(false, "");
    TestShaderCompile(true, EXTPragma);
}

//
TEST_P(EXTShaderFramebufferFetchNoncoherentES300FailureTest, CompileFailsWithoutNoncoherent)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(false, EXTPragma);
}

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTShaderFramebufferFetchNoncoherentES100SuccessTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_LastFragDataRedeclared1)));

INSTANTIATE_TEST_SUITE_P(IncorrectESSL100Shaders,
                         EXTShaderFramebufferFetchNoncoherentES100FailureTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_LastFragDataWithoutRedeclaration,
                                        ESSL100_LastFragDataRedeclaredWithoutNoncoherent)));

INSTANTIATE_TEST_SUITE_P(CorrectESSL300Shaders,
                         EXTShaderFramebufferFetchNoncoherentES300SuccessTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_InOut,
                                        ESSL300_InOut2,
                                        ESSL300_InOut3,
                                        ESSL300_InOut4,
                                        ESSL300_InOut5,
                                        ESSL300_InOut6)));

INSTANTIATE_TEST_SUITE_P(IncorrectESSL300Shaders,
                         EXTShaderFramebufferFetchNoncoherentES300FailureTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_InOutWithoutNoncoherent)));
#endif

}  // anonymous namespace
