//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLE_draw_id.cpp:
//   Test for ANGLE_draw_id extension
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/tree_ops/EmulateMultiDrawShaderBuiltins.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class EmulateGLDrawIDTest : public MatchOutputCodeTest
{
  public:
    EmulateGLDrawIDTest() : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_GLSL_COMPATIBILITY_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions = {};
        setDefaultCompileOptions(defaultCompileOptions);

        getResources()->ANGLE_multi_draw = 1;
    }

  protected:
    void CheckCompileFailure(const std::string &shaderString, const char *expectedError = nullptr)
    {
        ShCompileOptions compileOptions = {};

        std::string translatedCode;
        std::string infoLog;
        bool success = compileTestShader(GL_VERTEX_SHADER, SH_GLES2_SPEC,
                                         SH_GLSL_COMPATIBILITY_OUTPUT, shaderString, getResources(),
                                         compileOptions, &translatedCode, &infoLog);
        EXPECT_FALSE(success);
        if (expectedError)
        {
            EXPECT_TRUE(infoLog.find(expectedError) != std::string::npos);
        }
    }
};

// Check that compilation fails if the compile option to emulate gl_DrawID
// is not set
TEST_F(EmulateGLDrawIDTest, RequiresEmulation)
{
    CheckCompileFailure(
        "#extension GL_ANGLE_multi_draw : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n",
        "extension is not supported");
}

// Check that compiling with emulation with gl_DrawID works with different shader versions
TEST_F(EmulateGLDrawIDTest, CheckCompile)
{
    const std::string shaderString =
        "#extension GL_ANGLE_multi_draw : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n";

    ShCompileOptions compileOptions = {};
    compileOptions.objectCode       = true;
    compileOptions.emulateGLDrawID  = true;

    compile(shaderString, compileOptions);
    compile("#version 100\n" + shaderString, compileOptions);
    compile("#version 300 es\n" + shaderString, compileOptions);
}

// Check that gl_DrawID is properly emulated
TEST_F(EmulateGLDrawIDTest, EmulatesUniform)
{
    addOutputType(SH_GLSL_COMPATIBILITY_OUTPUT);
    addOutputType(SH_ESSL_OUTPUT);
#ifdef ANGLE_ENABLE_VULKAN
    addOutputType(SH_SPIRV_VULKAN_OUTPUT);
#endif
#ifdef ANGLE_ENABLE_HLSL
    addOutputType(SH_HLSL_3_0_OUTPUT);
    addOutputType(SH_HLSL_3_0_OUTPUT);
#endif

    const std::string &shaderString =
        "#extension GL_ANGLE_multi_draw : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n";

    ShCompileOptions compileOptions = {};
    compileOptions.objectCode       = true;
    compileOptions.emulateGLDrawID  = true;
    compile(shaderString, compileOptions);

    EXPECT_TRUE(notFoundInCode("gl_DrawID"));
    EXPECT_TRUE(foundInCode("angle_DrawID"));
    EXPECT_TRUE(notFoundInCode("GL_ANGLE_multi_draw"));

    EXPECT_TRUE(foundInCode(SH_GLSL_COMPATIBILITY_OUTPUT, "uniform int angle_DrawID"));
    EXPECT_TRUE(foundInCode(SH_ESSL_OUTPUT, "uniform highp int angle_DrawID"));

#ifdef ANGLE_ENABLE_HLSL
    EXPECT_TRUE(foundInCode(SH_HLSL_3_0_OUTPUT, "uniform int angle_DrawID : register"));
    EXPECT_TRUE(foundInCode(SH_HLSL_3_0_OUTPUT, "uniform int angle_DrawID : register"));
#endif
}

// Check that a user-defined "gl_DrawID" is not permitted
TEST_F(EmulateGLDrawIDTest, DisallowsUserDefinedGLDrawID)
{
    // Check that it is not permitted without the extension
    CheckCompileFailure(
        "uniform int gl_DrawID;\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "void main() {\n"
        "   int gl_DrawID = 0;\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    // Check that it is not permitted with the extension
    CheckCompileFailure(
        "#extension GL_ANGLE_multi_draw : require\n"
        "uniform int gl_DrawID;\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "#extension GL_ANGLE_multi_draw : require\n"
        "void main() {\n"
        "   int gl_DrawID = 0;\n"
        "   gl_Position = vec4(float(gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");
}

// gl_DrawID is translated to angle_DrawID internally. Check that a user-defined
// angle_DrawID is permitted
TEST_F(EmulateGLDrawIDTest, AllowsUserDefinedANGLEDrawID)
{
    addOutputType(SH_GLSL_COMPATIBILITY_OUTPUT);
    addOutputType(SH_ESSL_OUTPUT);
#ifdef ANGLE_ENABLE_VULKAN
    addOutputType(SH_SPIRV_VULKAN_OUTPUT);
#endif
#ifdef ANGLE_ENABLE_HLSL
    addOutputType(SH_HLSL_3_0_OUTPUT);
    addOutputType(SH_HLSL_3_0_OUTPUT);
#endif

    const std::string &shaderString =
        "#extension GL_ANGLE_multi_draw : require\n"
        "uniform int angle_DrawID;\n"
        "void main() {\n"
        "   gl_Position = vec4(float(angle_DrawID + gl_DrawID), 0.0, 0.0, 1.0);\n"
        "}\n";

    ShCompileOptions compileOptions = {};
    compileOptions.objectCode       = true;
    compileOptions.emulateGLDrawID  = true;
    compile(shaderString, compileOptions);

    // " angle_DrawID" (note the space) should appear exactly twice:
    //    once in the declaration and once in the body.
    // The user-defined angle_DrawID should be decorated
    EXPECT_TRUE(foundInCode(" angle_DrawID", 2));
}
