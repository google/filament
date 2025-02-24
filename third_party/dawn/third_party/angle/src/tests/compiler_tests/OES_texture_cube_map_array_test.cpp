//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OES_texture_cube_map_array_test.cpp:
//   Test for the [OES/EXT]_texture_cube_map_array extension
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class TextureCubeMapArrayTestNoExt : public ShaderCompileTreeTest
{
  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

class OESTextureCubeMapArrayTest : public TextureCubeMapArrayTestNoExt
{
  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->OES_texture_cube_map_array = 1;
    }
};

class EXTTextureCubeMapArrayTest : public TextureCubeMapArrayTestNoExt
{
  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->EXT_texture_cube_map_array = 1;
    }
};

// Check that if the extension is not supported, trying to use the features without having an
// extension directive fails.
TEST_F(TextureCubeMapArrayTestNoExt, MissingExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension is not supported, trying to use the features without having an
// extension directive fails.
TEST_F(OESTextureCubeMapArrayTest, MissingExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension is not supported, trying to use the features without having an
// extension directive fails.
TEST_F(EXTTextureCubeMapArrayTest, MissingExtensionDirective)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension is enabled, trying to use the features without the extension
// enabled fails.
TEST_F(TextureCubeMapArrayTestNoExt, ExtensionEnabledOES)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_texture_cube_map_array : enable
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension supported and enabled, using the features succeeds.
TEST_F(OESTextureCubeMapArrayTest, ExtensionEnabledOES)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_texture_cube_map_array : enable
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Check that if the extension is enabled, trying to use the features without the extension
// enabled fails.
TEST_F(EXTTextureCubeMapArrayTest, ExtensionEnabledOES)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_texture_cube_map_array : enable
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension is enabled, trying to use the features without the extension
// enabled fails.
TEST_F(TextureCubeMapArrayTestNoExt, ExtensionEnabledEXT)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_texture_cube_map_array : enable
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension is enabled, trying to use the features without the extension
// enabled fails.
TEST_F(OESTextureCubeMapArrayTest, ExtensionEnabledEXT)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_texture_cube_map_array : enable
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that if the extension supported and enabled, using the features succeeds.
TEST_F(EXTTextureCubeMapArrayTest, ExtensionEnabledEXT)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_texture_cube_map_array : enable
        precision mediump float;
        uniform highp isamplerCubeArray u_sampler;
        void main()
        {
            vec4 color = vec4(texture(u_sampler, vec4(0, 0, 0, 0)));
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}
