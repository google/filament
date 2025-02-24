//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ARB_texture_rectangle_test.cpp:
//   Test for the ARB_texture_rectangle extension
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class ARBTextureRectangleTestNoExt : public ShaderCompileTreeTest
{
  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_SPEC; }
};

class ARBTextureRectangleTest : public ARBTextureRectangleTestNoExt
{
  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->ARB_texture_rectangle = 1;
    }
};

// Check that if the extension is not supported, trying to use the features without having an
// extension directive fails.
TEST_F(ARBTextureRectangleTestNoExt, NewTypeAndBuiltinsWithoutExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        uniform sampler2DRect tex;
        void main()
        {
            vec4 color = texture2DRect(tex, vec2(1.0));
            color = texture2DRectProj(tex, vec3(1.0));
            color = texture2DRectProj(tex, vec4(1.0));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension is not supported, trying to use the features fails.
TEST_F(ARBTextureRectangleTestNoExt, NewTypeAndBuiltinsWithExtensionDirective)
{
    const std::string &shaderString =
        R"(
        #extension GL_ARB_texture_rectangle : enable
        precision mediump float;
        uniform sampler2DRect tex;
        void main()
        {
            vec4 color = texture2DRect(tex, vec2(1.0));
            color = texture2DRectProj(tex, vec3(1.0));
            color = texture2DRectProj(tex, vec4(1.0));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that new types and builtins are usable even with the #extension directive
// Issue #15 of ARB_texture_rectangle explains that the extension was specified before the
// #extension mechanism was in place so it doesn't require explicit enabling.
TEST_F(ARBTextureRectangleTest, NewTypeAndBuiltinsWithoutExtensionDirective)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform sampler2DRect tex;\n"
        "void main() {\n"
        "    vec4 color = texture2DRect(tex, vec2(1.0));"
        "    color = texture2DRectProj(tex, vec3(1.0));"
        "    color = texture2DRectProj(tex, vec4(1.0));"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test valid usage of the new types and builtins
TEST_F(ARBTextureRectangleTest, NewTypeAndBuiltingsWithExtensionDirective)
{
    const std::string &shaderString =
        "#extension GL_ARB_texture_rectangle : require\n"
        "precision mediump float;\n"
        "uniform sampler2DRect tex;\n"
        "void main() {\n"
        "    vec4 color = texture2DRect(tex, vec2(1.0));"
        "    color = texture2DRectProj(tex, vec3(1.0));"
        "    color = texture2DRectProj(tex, vec4(1.0));"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Check that it is not possible to pass a sampler2DRect where sampler2D is expected, and vice versa
TEST_F(ARBTextureRectangleTest, Rect2DVs2DMismatch)
{
    const std::string &shaderString1 =
        "#extension GL_ARB_texture_rectangle : require\n"
        "precision mediump float;\n"
        "uniform sampler2DRect tex;\n"
        "void main() {\n"
        "    vec4 color = texture2D(tex, vec2(1.0));"
        "}\n";
    if (compile(shaderString1))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }

    const std::string &shaderString2 =
        "#extension GL_ARB_texture_rectangle : require\n"
        "precision mediump float;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "    vec4 color = texture2DRect(tex, vec2(1.0));"
        "}\n";
    if (compile(shaderString2))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Disabling ARB_texture_rectangle in GLSL should work, even if it is enabled by default.
// See ARB_texture_rectangle spec: "a shader can still include all variations of #extension
// GL_ARB_texture_rectangle in its source code"
TEST_F(ARBTextureRectangleTest, DisableARBTextureRectangle)
{
    const std::string &shaderString =
        R"(
        #extension GL_ARB_texture_rectangle : disable

        precision mediump float;

        uniform sampler2DRect s;
        void main()
        {})";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The compiler option to disable ARB_texture_rectangle should prevent shaders from
// enabling it.
TEST_F(ARBTextureRectangleTest, CompilerOption)
{
    const std::string &shaderString =
        R"(
        #extension GL_ARB_texture_rectangle : enable
        precision mediump float;
        uniform sampler2DRect s;
        void main() {})";
    mCompileOptions.disableARBTextureRectangle = true;
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The compiler option to disable ARB_texture_rectangle should be toggleable.
TEST_F(ARBTextureRectangleTest, ToggleCompilerOption)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        uniform sampler2DRect s;
        void main() {})";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
    mCompileOptions.disableARBTextureRectangle = true;
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
    mCompileOptions.disableARBTextureRectangle = false;
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}
