//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// samplerVideoWEBGL_test.cpp:
// Tests compiling shaders that use samplerVideoWEBGL types
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class SamplerVideoWEBGLTest : public ShaderCompileTreeTest
{
  public:
    SamplerVideoWEBGLTest() {}

    void initResources(ShBuiltInResources *resources) override
    {
        resources->WEBGL_video_texture = 1;
    }

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

// Checks whether compiler returns error when extension isn't enabled but samplerVideoWEBGL is
// used in shader.
TEST_F(SamplerVideoWEBGLTest, UsingSamplerVideoWEBGLWithoutWEBGLVideoTextureExtensionRequired)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform mediump samplerVideoWEBGL s;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = textureVideoWEBGL(s, vec2(0.0, 0.0));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation passed, expecting fail:\n" << mInfoLog;
    }
}

// Checks whether compiler returns error when extension isn't enabled but use samplerVideoWEBGL is
// used in ES300 shader.
TEST_F(SamplerVideoWEBGLTest,
       UsingSamplerVideoWEBGLWithoutWEBGLVideoTextureExtensionRequiredInES300)
{
    const std::string &shaderString =
        "#version 300 es"
        "precision mediump float;\n"
        "uniform mediump samplerVideoWEBGL s;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "   my_FragColor = texture(s, vec2(0.0, 0.0));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation passed, expecting fail:\n" << mInfoLog;
    }
}

// Checks whether compiler can support samplerVideoWEBGL as texture2D parameter.
TEST_F(SamplerVideoWEBGLTest, SamplerVideoWEBGLCanBeSupportedInTexture2D)
{
    const std::string &shaderString =
        "#extension GL_WEBGL_video_texture : require\n"
        "precision mediump float;\n"
        "uniform mediump samplerVideoWEBGL s;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = textureVideoWEBGL(s, vec2(0.0, 0.0));\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Checks whether compiler can support samplerVideoWEBGL as texture parameter in ES300.
TEST_F(SamplerVideoWEBGLTest, SamplerVideoWEBGLCanBeSupportedInTextureInES300)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_WEBGL_video_texture : require\n"
        "precision mediump float;\n"
        "uniform mediump samplerVideoWEBGL s;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = texture(s, vec2(0.0, 0.0));\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}