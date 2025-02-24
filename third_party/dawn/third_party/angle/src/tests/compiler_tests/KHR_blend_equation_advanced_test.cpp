//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// KHR_blend_equation_advanced_test.cpp:
//   Test for KHR_blend_equation_advanced and KHR_blend_equation_advanced_coherent
//

#include "tests/test_utils/ShaderExtensionTest.h"

#include "common/PackedEnums.h"

namespace
{
const char EXTPragma[] =
    "#extension GL_KHR_blend_equation_advanced : require\n"
    "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n";

// Use the multiply equation for blending
const char ESSL310_Simple[] =
    R"(
    precision highp float;

    layout (blend_support_multiply) out;
    layout (location = 0) out vec4 oCol;

    uniform vec4 uSrcCol;

    void main (void)
    {
        oCol = uSrcCol;
    })";

const char ESSL310_DeclaredMultiplyScreenSeparately[] =
    R"(
    precision highp float;

    layout (blend_support_multiply) out;
    layout (blend_support_screen) out;
    layout (location = 0) out vec4 oCol;

    uniform vec4 uSrcCol;

    void main (void)
    {
        oCol = uSrcCol;
    })";

const char ESSL310_DeclaredMultiplyScreenSuccessively[] =
    R"(
    precision highp float;

    layout (blend_support_multiply, blend_support_screen) out;
    layout (location = 0) out vec4 oCol;

    uniform vec4 uSrcCol;

    void main (void)
    {
        oCol = uSrcCol;
    })";

const char ESSL310_With_FramebufferFetch[] =
    R"(
    precision highp float;

    layout (blend_support_multiply) out;
    layout (location = 0, noncoherent) inout vec4 oCol;

    uniform vec4 uSrcCol;

    void main (void)
    {
        oCol = mix(oCol, uSrcCol, 0.5f);
    })";

const char ESSL310_With_FramebufferFetchVec3[] =
    R"(
    precision highp float;

    layout (blend_support_multiply) out;
    layout (location = 0, noncoherent) inout vec3 oCol;

    uniform vec3 uSrcCol;

    void main (void)
    {
        oCol = mix(oCol, uSrcCol, 0.5f);
    })";

class KHRBlendEquationAdvancedTest : public sh::ShaderExtensionTest
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

    enum class Emulation
    {
        Disabled,
        Enabled
    };

    testing::AssertionResult TestShaderCompile(ShShaderOutput shaderOutputType,
                                               const char *pragma,
                                               Emulation emulate)
    {
        const char *shaderStrings[] = {testing::get<1>(GetParam()), pragma,
                                       testing::get<2>(GetParam())};

        ShCompileOptions compileFlags = {};
        compileFlags.objectCode       = true;
        if (emulate == Emulation::Enabled)
        {
            compileFlags.addAdvancedBlendEquationsEmulation = true;
        }

        bool success = sh::Compile(mCompilerList[shaderOutputType], shaderStrings, 3, compileFlags);
        if (success)
        {
            return ::testing::AssertionSuccess()
                   << "Compilation success(" << mShaderOutputList[shaderOutputType] << ")";
        }
        return ::testing::AssertionFailure() << sh::GetInfoLog(mCompilerList[shaderOutputType]);
    }

    void TestShaderCompile(bool expectation, const char *pragma, Emulation emulate)
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            if (expectation)
            {
                EXPECT_TRUE(TestShaderCompile(shaderOutputType.first, pragma, emulate));
            }
            else
            {
                EXPECT_FALSE(TestShaderCompile(shaderOutputType.first, pragma, emulate));
            }
        }
    }

    void SetExtensionEnable(bool enable)
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            mResourceList[shaderOutputType.first].KHR_blend_equation_advanced = enable;
            mResourceList[shaderOutputType.first].EXT_shader_framebuffer_fetch_non_coherent =
                enable;
        }
    }

  protected:
    std::map<ShShaderOutput, std::string> mShaderOutputList;
    std::map<ShShaderOutput, ShHandle> mCompilerList;
    std::map<ShShaderOutput, ShBuiltInResources> mResourceList;
};

class KHRBlendEquationAdvancedES310Test : public KHRBlendEquationAdvancedTest
{};

// Extension flag is required to compile properly. Expect failure when it is not present.
TEST_P(KHRBlendEquationAdvancedES310Test, CompileFailsWithoutExtension)
{
    SetExtensionEnable(false);
    InitializeCompiler();
    TestShaderCompile(false, EXTPragma, Emulation::Disabled);
}

// Extension directive is required to compile properly. Expect failure when it is not present.
TEST_P(KHRBlendEquationAdvancedES310Test, CompileFailsWithExtensionWithoutPragma)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(false, "", Emulation::Disabled);
}

INSTANTIATE_TEST_SUITE_P(CorrectESSL310Shaders,
                         KHRBlendEquationAdvancedES310Test,
                         Combine(Values(SH_GLES3_1_SPEC),
                                 Values(sh::ESSLVersion310),
                                 Values(ESSL310_Simple,
                                        ESSL310_With_FramebufferFetch,
                                        ESSL310_With_FramebufferFetchVec3,
                                        ESSL310_DeclaredMultiplyScreenSeparately,
                                        ESSL310_DeclaredMultiplyScreenSuccessively)));

#if defined(ANGLE_ENABLE_VULKAN)

class KHRBlendEquationAdvancedSuccessTest : public KHRBlendEquationAdvancedTest
{
  public:
    void SetUp() override
    {
        std::map<ShShaderOutput, std::string> shaderOutputList = {
            {SH_SPIRV_VULKAN_OUTPUT, "SH_SPIRV_VULKAN_OUTPUT"}};

        Initialize(shaderOutputList);
    }
};

class KHRBlendEquationAdvancedES310SuccessTest : public KHRBlendEquationAdvancedSuccessTest
{};

// With extension flag and extension directive, compiling succeeds.  Also test that the extension
// directive state is reset correctly.
TEST_P(KHRBlendEquationAdvancedES310SuccessTest, CompileSucceedsWithExtensionAndPragma)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(true, EXTPragma, Emulation::Disabled);
    // Test reset functionality.
    TestShaderCompile(false, "", Emulation::Disabled);
    TestShaderCompile(true, EXTPragma, Emulation::Disabled);
}

// Same as CompileSucceedsWithExtensionAndPragma but with emulation.
TEST_P(KHRBlendEquationAdvancedES310SuccessTest, CompileSucceedsWithExtensionAndPragmaWithEmulation)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(true, EXTPragma, Emulation::Enabled);
}

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL310Shaders,
                         KHRBlendEquationAdvancedES310SuccessTest,
                         Combine(Values(SH_GLES3_1_SPEC),
                                 Values(sh::ESSLVersion310),
                                 Values(ESSL310_Simple,
                                        ESSL310_With_FramebufferFetch,
                                        ESSL310_With_FramebufferFetchVec3,
                                        ESSL310_DeclaredMultiplyScreenSeparately,
                                        ESSL310_DeclaredMultiplyScreenSuccessively)));

class KHRBlendEquationAdvancedEnabledListCheckTest : public KHRBlendEquationAdvancedTest
{
  public:
    void SetUp() override
    {
        std::map<ShShaderOutput, std::string> shaderOutputList = {
            {SH_SPIRV_VULKAN_OUTPUT, "SH_SPIRV_VULKAN_OUTPUT"}};

        Initialize(shaderOutputList);
    }

    const ShHandle &GetCompilerHandle(const ShShaderOutput outputType) const
    {
        return mCompilerList.at(outputType);
    }
};

class KHRBlendEquationAdvancedEnabledSeparatelyTest
    : public KHRBlendEquationAdvancedEnabledListCheckTest
{};

// Test for declaring different blend equations in separate layout declarations
TEST_P(KHRBlendEquationAdvancedEnabledSeparatelyTest, DeclaredEquationSeparately)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(true, EXTPragma, Emulation::Disabled);

    const ShHandle compilerHandle = GetCompilerHandle(SH_SPIRV_VULKAN_OUTPUT);
    gl::BlendEquationBitSet enabledBlendEquation(sh::GetAdvancedBlendEquations(compilerHandle));
    EXPECT_TRUE(enabledBlendEquation.test(gl::BlendEquationType::Multiply));
    EXPECT_TRUE(enabledBlendEquation.test(gl::BlendEquationType::Screen));
}

// Same as DeclaredEquationSeparately but with emulation.
TEST_P(KHRBlendEquationAdvancedEnabledSeparatelyTest, DeclaredEquationSeparatelyWithEmulation)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(true, EXTPragma, Emulation::Enabled);
}

INSTANTIATE_TEST_SUITE_P(CorrectESSL310Shaders,
                         KHRBlendEquationAdvancedEnabledSeparatelyTest,
                         Combine(Values(SH_GLES3_1_SPEC),
                                 Values(sh::ESSLVersion310),
                                 Values(ESSL310_DeclaredMultiplyScreenSeparately)));

class KHRBlendEquationAdvancedEnabledSuccessivelyTest
    : public KHRBlendEquationAdvancedEnabledListCheckTest
{};

// Test for declaring different blend equations in the same layout declaration
TEST_P(KHRBlendEquationAdvancedEnabledSuccessivelyTest, DeclaredEquationSuccessively)
{
    SetExtensionEnable(true);
    InitializeCompiler();
    TestShaderCompile(true, EXTPragma, Emulation::Disabled);

    const ShHandle compilerHandle = GetCompilerHandle(SH_SPIRV_VULKAN_OUTPUT);
    gl::BlendEquationBitSet enabledBlendEquation(sh::GetAdvancedBlendEquations(compilerHandle));
    EXPECT_TRUE(enabledBlendEquation.test(gl::BlendEquationType::Multiply));
    EXPECT_TRUE(enabledBlendEquation.test(gl::BlendEquationType::Screen));
}

INSTANTIATE_TEST_SUITE_P(CorrectESSL310Shaders,
                         KHRBlendEquationAdvancedEnabledSuccessivelyTest,
                         Combine(Values(SH_GLES3_1_SPEC),
                                 Values(sh::ESSLVersion310),
                                 Values(ESSL310_DeclaredMultiplyScreenSuccessively)));

#endif

}  // anonymous namespace
