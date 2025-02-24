//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_YUV_target.cpp:
//   Test for EXT_YUV_target implementation.
//

#include "tests/test_utils/ShaderExtensionTest.h"

namespace
{
class EXTYUVTargetTest : public sh::ShaderExtensionTest
{
  public:
    void InitializeCompiler() { InitializeCompiler(SH_GLSL_450_CORE_OUTPUT); }
    void InitializeCompiler(ShShaderOutput shaderOutputType)
    {
        DestroyCompiler();

        mCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, testing::get<0>(GetParam()),
                                          shaderOutputType, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }
};

const char EXTYTPragma[] = "#extension GL_EXT_YUV_target : require\n";

const char ESSL300_SimpleShader[] =
    R"(precision mediump float;
    uniform __samplerExternal2DY2YEXT uSampler;
    out vec4 fragColor;
    void main() {
        fragColor = vec4(1.0);
    })";

// Shader that samples the texture and writes to FragColor.
const char ESSL300_FragColorShader[] =
    R"(precision mediump float;
    uniform __samplerExternal2DY2YEXT uSampler;
    layout(yuv) out vec4 fragColor;
    void main() {
        fragColor = texture(uSampler, vec2(0.0));
    })";

// Shader that samples the texture and swizzle and writes to FragColor.
const char ESSL300_textureSwizzleFragColorShader[] =
    R"(precision mediump float;
    uniform __samplerExternal2DY2YEXT uSampler;
    layout(yuv) out vec4 fragColor;
    void main() {
        fragColor = vec4(texture(uSampler, vec2(0.0)).zyx, 1.0);
    })";

// Shader that specifies yuv layout qualifier multiple times.
const char ESSL300_YUVQualifierMultipleTimesShader[] =
    R"(precision mediump float;
    layout(yuv, yuv, yuv) out vec4 fragColor;
    void main() {
    })";

// Shader that specifies yuv layout qualifier for not output fails to compile.
const char ESSL300_YUVQualifierFailureShader1[] =
    R"(precision mediump float;
    layout(yuv) in vec4 fragColor;
    void main() {
    })";

const char ESSL300_YUVQualifierFailureShader2[] =
    R"(precision mediump float;
    layout(yuv) uniform;
    layout(yuv) uniform Transform {
         mat4 M1;
    };
    void main() {
    })";

// Shader that specifies yuv layout qualifier with location fails to compile.
const char ESSL300_LocationAndYUVFailureShader[] =
    R"(precision mediump float;
    layout(location = 0, yuv) out vec4 fragColor;
    void main() {
    })";

// Shader that specifies yuv layout qualifier with multiple color outputs fails to compile.
const char ESSL300_MultipleColorAndYUVOutputsFailureShader1[] =
    R"(precision mediump float;
    layout(yuv) out vec4 fragColor;
    layout out vec4 fragColor1;
    void main() {
    })";

const char ESSL300_MultipleColorAndYUVOutputsFailureShader2[] =
    R"(precision mediump float;
    layout(yuv) out vec4 fragColor;
    layout(location = 1) out vec4 fragColor1;
    void main() {
    })";

// Shader that specifies yuv layout qualifier with depth output fails to compile.
const char ESSL300_DepthAndYUVOutputsFailureShader[] =
    R"(precision mediump float;
    layout(yuv) out vec4 fragColor;
    void main() {
        gl_FragDepth = 1.0f;
    })";

// Shader that specifies yuv layout qualifier with multiple outputs fails to compile.
const char ESSL300_MultipleYUVOutputsFailureShader[] =
    R"(precision mediump float;
    layout(yuv) out vec4 fragColor;
    layout(yuv) out vec4 fragColor1;
    void main() {
    })";

// Shader that specifies yuvCscStandartEXT type and associated values.
const char ESSL300_YuvCscStandardEXTShader[] =
    R"(precision mediump float;
    yuvCscStandardEXT;
    yuvCscStandardEXT conv;
    yuvCscStandardEXT conv1 = itu_601;
    yuvCscStandardEXT conv2 = itu_601_full_range;
    yuvCscStandardEXT conv3 = itu_709;
    const yuvCscStandardEXT conv4 = itu_709;

    uniform int u;
    out vec4 my_color;

    yuvCscStandardEXT conv_standard()
    {
        switch(u)
        {
            case 1:
                return conv1;
            case 2:
                return conv2;
            case 3:
                return conv3;
            default:
                return conv;
        }
    }
    bool is_itu_601(inout yuvCscStandardEXT csc)
    {
        csc = itu_601;
        return csc == itu_601;
    }
    bool is_itu_709(yuvCscStandardEXT csc)
    {
        return csc == itu_709;
    }
    void main()
    {
        yuvCscStandardEXT conv = conv_standard();
        bool csc_check1 = is_itu_601(conv);
        bool csc_check2 = is_itu_709(itu_709);
        if (csc_check1 && csc_check2) {
            my_color = vec4(0, 1, 0, 1);
        }
    })";

// Shader that specifies yuvCscStandardEXT type constructor fails to compile.
const char ESSL300_YuvCscStandardEXTConstructFailureShader1[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = yuvCscStandardEXT();
    void main() {
    })";

const char ESSL300_YuvCscStandardEXTConstructFailureShader2[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = yuvCscStandardEXT(itu_601);
    void main() {
    })";

// Shader that specifies yuvCscStandardEXT type conversion fails to compile.
const char ESSL300_YuvCscStandardEXTConversionFailureShader1[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = false;
    void main() {
    })";

const char ESSL300_YuvCscStandardEXTConversionFailureShader2[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = 0;
    void main() {
    })";

const char ESSL300_YuvCscStandardEXTConversionFailureShader3[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = 2.0f;
    void main() {
    })";

const char ESSL300_YuvCscStandardEXTConversionFailureShader4[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_601 | itu_709;
    void main() {
    })";

const char ESSL300_YuvCscStandardEXTConversionFailureShader5[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_601 & 3.0f;
    void main() {
    })";

// Shader that specifies yuvCscStandardEXT type qualifiers fails to compile.
const char ESSL300_YuvCscStandardEXTQualifiersFailureShader1[] =
    R"(precision mediump float;
    in yuvCscStandardEXT conv = itu_601;
    void main() {
    })";

const char ESSL300_YuvCscStandardEXTQualifiersFailureShader2[] =
    R"(precision mediump float;
    out yuvCscStandardEXT conv = itu_601;
    void main() {
    })";

const char ESSL300_YuvCscStandardEXTQualifiersFailureShader3[] =
    R"(precision mediump float;
    uniform yuvCscStandardEXT conv = itu_601;
    void main() {
    })";

// Shader that specifies yuv_to_rgb() and rgb_to_yuv() built-in functions.
const char ESSL300_BuiltInFunctionsShader[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_601;

    out vec4 my_color;

    void main()
    {
        vec3 yuv = rgb_2_yuv(vec3(0.0f), conv);
        vec3 rgb = yuv_2_rgb(yuv, itu_601);
        my_color = vec4(rgb, 1.0);
    })";

// Shader with nested yuv_to_rgb() and rgb_to_yuv() built-in functions.
const char ESSL300_BuiltInFunctionsNested1Shader[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_601;

    out vec4 my_color;

    void main()
    {
        vec3 rgb = yuv_2_rgb(rgb_2_yuv(vec3(0.0f), conv), conv);
        my_color = vec4(rgb, 1.0);
    })";
const char ESSL300_BuiltInFunctionsNested2Shader[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_601_full_range;

    out vec4 my_color;

    void main()
    {
        vec3 rgb = yuv_2_rgb(vec3(0.1) + rgb_2_yuv(vec3(0.0f), conv), conv);
        my_color = vec4(rgb, 1.0);
    })";
const char ESSL300_BuiltInFunctionsNested3Shader[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_709;

    out vec4 my_color;

    vec3 f(vec3 v) { return v + vec3(0.1); }

    void main()
    {
        vec3 rgb = yuv_2_rgb(f(rgb_2_yuv(vec3(0.0f), conv)), conv);
        my_color = vec4(rgb, 1.0);
    })";
const char ESSL300_BuiltInFunctionsNested4Shader[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_601;

    out vec4 my_color;

    vec3 f(vec3 v) { return v + vec3(0.1); }

    void main()
    {
        vec3 rgb = f(yuv_2_rgb(f(rgb_2_yuv(vec3(0.0f), conv)), conv));
        my_color = vec4(rgb, 1.0);
    })";

// Shader with yuv_to_rgb() and rgb_to_yuv() built-in functions with different precision.
const char ESSL300_BuiltInFunctionsPrecisionShader[] =
    R"(precision mediump float;
    yuvCscStandardEXT conv = itu_601;

    out vec4 my_color;

    void main()
    {
        lowp vec3 rgbLow = vec3(0.1);
        mediump vec3 rgbMedium = vec3(0.2);
        highp vec3 rgbHigh = vec3(0.3);

        lowp vec3 yuvLow = vec3(0.4);
        mediump vec3 yuvMedium = vec3(0.5);
        highp vec3 yuvHigh = vec3(0.6);

        my_color = vec4(
                rgb_2_yuv(rgbLow, conv) - rgb_2_yuv(rgbMedium, conv) + rgb_2_yuv(rgbHigh, conv) -
                yuv_2_rgb(yuvLow, conv) - yuv_2_rgb(yuvMedium, conv) + yuv_2_rgb(yuvHigh, conv),
            1.0);
    })";

const char ESSL300_OverloadRgb2Yuv[] =
    R"(precision mediump float;
    float rgb_2_yuv(float x) { return x + 1.0; }

    in float i;
    out float o;

    void main()
    {
        o = rgb_2_yuv(i);
    })";

const char ESSL300_OverloadYuv2Rgb[] =
    R"(precision mediump float;
    float yuv_2_rgb(float x) { return x + 1.0; }

    in float i;
    out float o;

    void main()
    {
        o = yuv_2_rgb(i);
    })";

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTYUVTargetTest, CompileFailsWithoutExtension)
{
    mResources.EXT_YUV_target = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTYTPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTYUVTargetTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_YUV_target = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTYUVTargetTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_YUV_target = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTYTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTYTPragma));
}

INSTANTIATE_TEST_SUITE_P(CorrectVariantsWithExtensionAndPragma,
                         EXTYUVTargetTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_SimpleShader, ESSL300_FragColorShader)));

class EXTYUVTargetCompileSuccessTest : public EXTYUVTargetTest
{};

TEST_P(EXTYUVTargetCompileSuccessTest, CompileSucceeds)
{
    // Expect compile success.
    mResources.EXT_YUV_target = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTYTPragma));
}

#ifdef ANGLE_ENABLE_VULKAN
// Test that YUV built-in emulation works on Vulkan
TEST_P(EXTYUVTargetCompileSuccessTest, CompileSucceedsWithExtensionAndPragmaOnVulkan)
{
    mResources.EXT_YUV_target   = 1;
    mCompileOptions.validateAST = 1;
    InitializeCompiler(SH_SPIRV_VULKAN_OUTPUT);
    EXPECT_TRUE(TestShaderCompile(EXTYTPragma));
}
#endif

INSTANTIATE_TEST_SUITE_P(CorrectESSL300Shaders,
                         EXTYUVTargetCompileSuccessTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_FragColorShader,
                                        ESSL300_textureSwizzleFragColorShader,
                                        ESSL300_YUVQualifierMultipleTimesShader,
                                        ESSL300_YuvCscStandardEXTShader,
                                        ESSL300_BuiltInFunctionsShader,
                                        ESSL300_BuiltInFunctionsNested1Shader,
                                        ESSL300_BuiltInFunctionsNested2Shader,
                                        ESSL300_BuiltInFunctionsNested3Shader,
                                        ESSL300_BuiltInFunctionsNested4Shader,
                                        ESSL300_BuiltInFunctionsPrecisionShader)));

class EXTYUVTargetCompileFailureTest : public EXTYUVTargetTest
{};

TEST_P(EXTYUVTargetCompileFailureTest, CompileFails)
{
    // Expect compile failure due to shader error, with shader having correct pragma.
    mResources.EXT_YUV_target = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTYTPragma));
}

INSTANTIATE_TEST_SUITE_P(IncorrectESSL300Shaders,
                         EXTYUVTargetCompileFailureTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_YUVQualifierFailureShader1,
                                        ESSL300_YUVQualifierFailureShader2,
                                        ESSL300_LocationAndYUVFailureShader,
                                        ESSL300_MultipleColorAndYUVOutputsFailureShader1,
                                        ESSL300_MultipleColorAndYUVOutputsFailureShader2,
                                        ESSL300_DepthAndYUVOutputsFailureShader,
                                        ESSL300_MultipleYUVOutputsFailureShader,
                                        ESSL300_YuvCscStandardEXTConstructFailureShader1,
                                        ESSL300_YuvCscStandardEXTConstructFailureShader2,
                                        ESSL300_YuvCscStandardEXTConversionFailureShader1,
                                        ESSL300_YuvCscStandardEXTConversionFailureShader2,
                                        ESSL300_YuvCscStandardEXTConversionFailureShader3,
                                        ESSL300_YuvCscStandardEXTConversionFailureShader4,
                                        ESSL300_YuvCscStandardEXTConversionFailureShader5,
                                        ESSL300_YuvCscStandardEXTQualifiersFailureShader1,
                                        ESSL300_YuvCscStandardEXTQualifiersFailureShader2,
                                        ESSL300_YuvCscStandardEXTQualifiersFailureShader3)));

class EXTYUVNotEnabledTest : public EXTYUVTargetTest
{};

TEST_P(EXTYUVNotEnabledTest, CanOverloadConversions)
{
    // Expect compile success with a shader that overloads functions in the EXT_YUV_target
    // extension.
    mResources.EXT_YUV_target = 0;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(""));
}

INSTANTIATE_TEST_SUITE_P(CoreESSL300Shaders,
                         EXTYUVNotEnabledTest,
                         Combine(Values(SH_GLES3_SPEC),
                                 Values(sh::ESSLVersion300),
                                 Values(ESSL300_OverloadRgb2Yuv, ESSL300_OverloadYuv2Rgb)));

}  // namespace
