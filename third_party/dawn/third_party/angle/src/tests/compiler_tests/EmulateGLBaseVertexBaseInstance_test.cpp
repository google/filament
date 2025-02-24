//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLE_base_vertex_base_instance.cpp:
//   Test for ANGLE_base_vertex_base_instance extension
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/tree_ops/EmulateMultiDrawShaderBuiltins.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class EmulateGLBaseVertexBaseInstanceTest : public MatchOutputCodeTest
{
  public:
    EmulateGLBaseVertexBaseInstanceTest()
        : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_GLSL_COMPATIBILITY_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions = {};
        setDefaultCompileOptions(defaultCompileOptions);

        getResources()->ANGLE_base_vertex_base_instance_shader_builtin = 1;
    }

  protected:
    void CheckCompileFailure(const std::string &shaderString,
                             const char *expectedError        = nullptr,
                             ShCompileOptions *compileOptions = nullptr)
    {
        ShCompileOptions options = {};
        if (compileOptions != nullptr)
        {
            options = *compileOptions;
        }

        std::string translatedCode;
        std::string infoLog;
        bool success =
            compileTestShader(GL_VERTEX_SHADER, SH_GLES3_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT,
                              shaderString, getResources(), options, &translatedCode, &infoLog);
        EXPECT_FALSE(success);
        if (expectedError)
        {
            EXPECT_TRUE(infoLog.find(expectedError) != std::string::npos);
        }
    }
};

// Check that compilation fails if the compile option to emulate gl_BaseVertex and gl_BaseInstance
// is not set
TEST_F(EmulateGLBaseVertexBaseInstanceTest, RequiresEmulation)
{
    CheckCompileFailure(
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseVertex), float(gl_BaseInstance), 0.0, 1.0);\n"
        "}\n",
        "extension is not supported");
}

// Check that compiling with emulation with gl_BaseVertex and gl_BaseInstance works
TEST_F(EmulateGLBaseVertexBaseInstanceTest, CheckCompile)
{
    const std::string shaderString =
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseVertex), float(gl_BaseInstance), 0.0, 1.0);\n"
        "}\n";

    ShCompileOptions compileOptions                = {};
    compileOptions.objectCode                      = true;
    compileOptions.emulateGLBaseVertexBaseInstance = true;

    compile(shaderString, compileOptions);
}

// Check that compiling with the old extension doesn't work
TEST_F(EmulateGLBaseVertexBaseInstanceTest, CheckCompileOldExtension)
{
    const std::string shaderString =
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseVertex), float(gl_BaseInstance), 0.0, 1.0);\n"
        "}\n";

    ShCompileOptions compileOptions                = {};
    compileOptions.objectCode                      = true;
    compileOptions.emulateGLBaseVertexBaseInstance = true;

    CheckCompileFailure(shaderString, "extension is not supported", &compileOptions);
}

// Check that gl_BaseVertex and gl_BaseInstance is properly emulated
TEST_F(EmulateGLBaseVertexBaseInstanceTest, EmulatesUniform)
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
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseVertex), float(gl_BaseInstance), 0.0, 1.0);\n"
        "}\n";

    ShCompileOptions compileOptions                = {};
    compileOptions.objectCode                      = true;
    compileOptions.emulateGLBaseVertexBaseInstance = true;

    compile(shaderString, compileOptions);

    EXPECT_TRUE(notFoundInCode("gl_BaseVertex"));
    EXPECT_TRUE(foundInCode("angle_BaseVertex"));
    EXPECT_TRUE(notFoundInCode("gl_BaseInstance"));
    EXPECT_TRUE(foundInCode("angle_BaseInstance"));
    EXPECT_TRUE(notFoundInCode("GL_ANGLE_base_vertex_base_instance_shader_builtin"));

    EXPECT_TRUE(foundInCode(SH_GLSL_COMPATIBILITY_OUTPUT, "uniform int angle_BaseVertex"));
    EXPECT_TRUE(foundInCode(SH_GLSL_COMPATIBILITY_OUTPUT, "uniform int angle_BaseInstance"));
    EXPECT_TRUE(foundInCode(SH_ESSL_OUTPUT, "uniform highp int angle_BaseVertex"));
    EXPECT_TRUE(foundInCode(SH_ESSL_OUTPUT, "uniform highp int angle_BaseInstance"));

#ifdef ANGLE_ENABLE_HLSL
    EXPECT_TRUE(foundInCode(SH_HLSL_3_0_OUTPUT, "uniform int angle_BaseVertex : register"));
    EXPECT_TRUE(foundInCode(SH_HLSL_3_0_OUTPUT, "uniform int angle_BaseInstance : register"));
#endif
}

// Check that a user-defined "gl_BaseVertex" or "gl_BaseInstance" is not permitted
TEST_F(EmulateGLBaseVertexBaseInstanceTest, DisallowsUserDefinedGLDrawID)
{
    // Check that it is not permitted without the extension
    CheckCompileFailure(
        "#version 300 es\n"
        "uniform int gl_BaseVertex;\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseVertex), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "#version 300 es\n"
        "uniform int gl_BaseInstance;\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseInstance), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "#version 300 es\n"
        "void main() {\n"
        "   int gl_BaseVertex = 0;\n"
        "   gl_Position = vec4(float(gl_BaseVertex), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "#version 300 es\n"
        "void main() {\n"
        "   int gl_BaseInstance = 0;\n"
        "   gl_Position = vec4(float(gl_BaseInstance), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    // Check that it is not permitted with the extension
    CheckCompileFailure(
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "uniform int gl_BaseVertex;\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseVertex), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "uniform int gl_BaseInstance;\n"
        "void main() {\n"
        "   gl_Position = vec4(float(gl_BaseInstance), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "void main() {\n"
        "   int gl_BaseVertex = 0;\n"
        "   gl_Position = vec4(float(gl_BaseVertex), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");

    CheckCompileFailure(
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "void main() {\n"
        "   int gl_BaseInstance = 0;\n"
        "   gl_Position = vec4(float(gl_BaseInstance), 0.0, 0.0, 1.0);\n"
        "}\n",
        "reserved built-in name");
}

// gl_BaseVertex and gl_BaseInstance are translated to angle_BaseVertex and angle_BaseInstance
// internally. Check that a user-defined angle_BaseVertex or angle_BaseInstance is permitted
TEST_F(EmulateGLBaseVertexBaseInstanceTest, AllowsUserDefinedANGLEDrawID)
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
        "#version 300 es\n"
        "#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n"
        "uniform int angle_BaseVertex;\n"
        "uniform int angle_BaseInstance;\n"
        "void main() {\n"
        "   gl_Position = vec4(\n"
        "           float(angle_BaseVertex + gl_BaseVertex),\n"
        "           float(angle_BaseInstance + gl_BaseInstance),\n"
        "           0.0, 1.0);\n"
        "}\n";

    ShCompileOptions compileOptions                = {};
    compileOptions.objectCode                      = true;
    compileOptions.emulateGLBaseVertexBaseInstance = true;

    compile(shaderString, compileOptions);

    // " angle_BaseVertex" (note the space) should appear exactly twice:
    //    once in the declaration and once in the body.
    // The user-defined angle_BaseVertex should be decorated
    EXPECT_TRUE(foundInCode(" angle_BaseVertex", 2));
    EXPECT_TRUE(foundInCode(" angle_BaseInstance", 2));
}
