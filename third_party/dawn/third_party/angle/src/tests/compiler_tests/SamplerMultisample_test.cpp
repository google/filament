//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerMultisample_test.cpp:
// Tests compiling shaders that use gsampler2DMS types
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class SamplerMultisampleTest : public ShaderCompileTreeTest
{
  public:
    SamplerMultisampleTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

class SamplerMultisampleArrayTest : public ShaderCompileTreeTest
{
  public:
    SamplerMultisampleArrayTest() {}

    void initResources(ShBuiltInResources *resources) override
    {
        resources->OES_texture_storage_multisample_2d_array = 1;
    }

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

// Checks whether compiler has parsed the gsampler2DMS, texelfetch correctly.
TEST_F(SamplerMultisampleTest, TexelFetchSampler2DMS)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            vec4 tex1 = texelFetch(s, ivec2(0, 0), 0);
            ivec4 tex2 = texelFetch(is, ivec2(0, 0), 0);
            uvec4 tex3 = texelFetch(us, ivec2(0, 0), 0);
        })";

    if (!compile(kShaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Checks whether compiler has parsed the gsampler2DMS, textureSize correctly.
TEST_F(SamplerMultisampleTest, TextureSizeSampler2DMS)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            ivec2 size = textureSize(s);
            size = textureSize(is);
            size = textureSize(us);
        })";

    if (!compile(kShaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Check that sampler2DMS has no default precision.
TEST_F(SamplerMultisampleTest, NoPrecisionSampler2DMS)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform sampler2DMS s;

        void main() {})";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that isampler2DMS has no default precision.
TEST_F(SamplerMultisampleTest, NoPrecisionISampler2DMS)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform isampler2DMS s;

        void main() {})";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that usampler2DMS has no default precision.
TEST_F(SamplerMultisampleTest, NoPrecisionUSampler2DMS)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform usampler2DMS s;

        void main() {})";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Negative test: checks that sampler2DMS is not usable in ESSL 3.00.
TEST_F(SamplerMultisampleTest, Sampler2DMSESSL300)
{
    constexpr char kShaderString[] =
        R"(#version 300 es
        precision highp float;
        uniform highp sampler2DMS s;

        void main() {
        })";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Negative test: checks that isampler2DMS is not usable in ESSL 3.00.
TEST_F(SamplerMultisampleTest, ISampler2DMSESSL300)
{
    constexpr char kShaderString[] =
        R"(#version 300 es
        precision highp float;
        uniform highp isampler2DMS s;

        void main() {
        })";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Negative test: checks that usampler2DMS is not usable in ESSL 3.00.
TEST_F(SamplerMultisampleTest, USampler2DMSESSL300)
{
    constexpr char kShaderString[] =
        R"(#version 300 es
        precision highp float;
        uniform highp usampler2DMS s;

        void main() {
        })";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Negative test: checks that sampler2DMSArray is not usable in ESSL 3.10 without extensions.
TEST_F(SamplerMultisampleTest, Sampler2DMSArrayNotSupported)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform highp sampler2DMSArray s;

        void main() {
        })";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Negative test: checks that isampler2DMSArray is not usable in ESSL 3.10 without extensions.
TEST_F(SamplerMultisampleTest, ISampler2DMSArrayNotSupported)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform highp isampler2DMSArray s;

        void main() {
        })";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Negative test: checks that usampler2DMSArray is not usable in ESSL 3.10 without extensions.
TEST_F(SamplerMultisampleTest, USampler2DMSArrayNotSupported)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision highp float;
        uniform highp usampler2DMSArray s;

        void main() {
        })";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Checks whether compiler has parsed the gsampler2DMSArray, texelfetch correctly.
TEST_F(SamplerMultisampleArrayTest, TexelFetchSampler2DMSArray)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        #extension GL_OES_texture_storage_multisample_2d_array : require
        precision highp float;
        uniform highp sampler2DMSArray s;
        uniform highp isampler2DMSArray is;
        uniform highp usampler2DMSArray us;

        void main() {
            vec4 tex1 = texelFetch(s, ivec3(0, 0, 0), 0);
            ivec4 tex2 = texelFetch(is, ivec3(0, 0, 0), 0);
            uvec4 tex3 = texelFetch(us, ivec3(0, 0, 0), 0);
        })";

    if (!compile(kShaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Checks whether compiler has parsed the gsampler2DMSArray, textureSize correctly.
TEST_F(SamplerMultisampleArrayTest, TextureSizeSampler2DMSArray)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        #extension GL_OES_texture_storage_multisample_2d_array : require
        precision highp float;
        uniform highp sampler2DMSArray s;
        uniform highp isampler2DMSArray is;
        uniform highp usampler2DMSArray us;

        void main() {
            ivec3 size = textureSize(s);
            size = textureSize(is);
            size = textureSize(us);
        })";

    if (!compile(kShaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Check that sampler2DMSArray has no default precision.
TEST_F(SamplerMultisampleArrayTest, NoPrecisionSampler2DMSArray)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        #extension GL_OES_texture_storage_multisample_2d_array : require
        precision highp float;
        uniform sampler2DMSArray s;

        void main() {})";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that isampler2DMSArray has no default precision.
TEST_F(SamplerMultisampleArrayTest, NoPrecisionISampler2DMSArray)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        #extension GL_OES_texture_storage_multisample_2d_array : require
        precision highp float;
        uniform isampler2DMSArray s;

        void main() {})";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check that usampler2DMSArray has no default precision.
TEST_F(SamplerMultisampleArrayTest, NoPrecisionUSampler2DMSArray)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        #extension GL_OES_texture_storage_multisample_2d_array : require
        precision highp float;
        uniform usampler2DMSArray s;

        void main() {})";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

class SamplerMultisampleEXTTest : public SamplerMultisampleTest
{
  public:
    SamplerMultisampleEXTTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->ANGLE_texture_multisample = 1;
    }

    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_SPEC; }
};

// checks ANGLE_texture_multisample is supported in es 3.0
TEST_F(SamplerMultisampleEXTTest, TextureMultisampleEXTEnabled)
{
    constexpr char kShaderString[] =
        R"(#version 300 es
        #extension GL_ANGLE_texture_multisample : require
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            ivec2 size = textureSize(s);
            size = textureSize(is);
            size = textureSize(us);
            vec4 tex1 = texelFetch(s, ivec2(0, 0), 0);
            ivec4 tex2 = texelFetch(is, ivec2(0, 0), 0);
            uvec4 tex3 = texelFetch(us, ivec2(0, 0), 0);
        })";

    if (!compile(kShaderString))
    {
        FAIL() << "Shader compilation failure, expecting success:\n" << mInfoLog;
    }
}

// checks that multisample texture is not supported without ANGLE_texture_multisample in es 3.0
TEST_F(SamplerMultisampleEXTTest, TextureMultisampleEXTDisabled)
{
    constexpr char kShaderString[] =
        R"(#version 300 es
        precision highp float;
        uniform highp sampler2DMS s;
        uniform highp isampler2DMS is;
        uniform highp usampler2DMS us;

        void main() {
            ivec2 size = textureSize(s);
            size = textureSize(is);
            size = textureSize(us);
            vec4 tex1 = texelFetch(s, ivec2(0, 0), 0);
            ivec4 tex2 = texelFetch(is, ivec2(0, 0), 0);
            uvec4 tex3 = texelFetch(us, ivec2(0, 0), 0);
        })";

    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation success, expecting failure:\n" << mInfoLog;
    }
}
