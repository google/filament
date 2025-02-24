//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// These tests verify shadow sampler functions and their options.

#include "common/gl_enum_utils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

enum class FunctionType
{
    Texture,
    TextureBias,
    TextureOffset,
    TextureOffsetBias,
    TextureLod,
    TextureLodOffset,
    TextureGrad,
    TextureGradOffset,
    TextureProj,
    TextureProjBias,
    TextureProjOffset,
    TextureProjOffsetBias,
    TextureProjLod,
    TextureProjLodOffset,
    TextureProjGrad,
    TextureProjGradOffset,
};

const char *FunctionName(FunctionType function)
{
    switch (function)
    {
        case FunctionType::Texture:
        case FunctionType::TextureBias:
            return "texture";
        case FunctionType::TextureOffset:
        case FunctionType::TextureOffsetBias:
            return "textureOffset";
        case FunctionType::TextureLod:
            return "textureLod";
        case FunctionType::TextureLodOffset:
            return "textureLodOffset";
        case FunctionType::TextureGrad:
            return "textureGrad";
        case FunctionType::TextureGradOffset:
            return "textureGradOffset";
        case FunctionType::TextureProj:
        case FunctionType::TextureProjBias:
            return "textureProj";
        case FunctionType::TextureProjOffset:
        case FunctionType::TextureProjOffsetBias:
            return "textureProjOffset";
        case FunctionType::TextureProjLod:
            return "textureProjLod";
        case FunctionType::TextureProjLodOffset:
            return "textureProjLodOffset";
        case FunctionType::TextureProjGrad:
            return "textureProjGrad";
        case FunctionType::TextureProjGradOffset:
            return "textureProjGradOffset";
    }
}

constexpr bool IsProj(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureProj:
        case FunctionType::TextureProjBias:
        case FunctionType::TextureProjOffset:
        case FunctionType::TextureProjOffsetBias:
        case FunctionType::TextureProjLod:
        case FunctionType::TextureProjLodOffset:
        case FunctionType::TextureProjGrad:
        case FunctionType::TextureProjGradOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool HasBias(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureBias:
        case FunctionType::TextureOffsetBias:
        case FunctionType::TextureProjBias:
        case FunctionType::TextureProjOffsetBias:
            return true;
        default:
            return false;
    }
}

constexpr bool HasLOD(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureLod:
        case FunctionType::TextureLodOffset:
        case FunctionType::TextureProjLod:
        case FunctionType::TextureProjLodOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool HasGrad(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureGrad:
        case FunctionType::TextureGradOffset:
        case FunctionType::TextureProjGrad:
        case FunctionType::TextureProjGradOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool HasOffset(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureOffset:
        case FunctionType::TextureOffsetBias:
        case FunctionType::TextureLodOffset:
        case FunctionType::TextureGradOffset:
        case FunctionType::TextureProjOffset:
        case FunctionType::TextureProjOffsetBias:
        case FunctionType::TextureProjLodOffset:
        case FunctionType::TextureProjGradOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool RequiresExtensionFor2DArray(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureBias:
        case FunctionType::TextureOffset:
        case FunctionType::TextureOffsetBias:
        case FunctionType::TextureLod:
        case FunctionType::TextureLodOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool RequiresExtensionForCube(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureLod:
            return true;
        default:
            return false;
    }
}

constexpr bool RequiresExtensionForCubeArray(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureBias:
        case FunctionType::TextureLod:
            return true;
        default:
            return false;
    }
}

bool Compare(float reference, float sampled, GLenum op)
{
    switch (op)
    {
        case GL_NEVER:
            return false;
        case GL_LESS:
            return reference < sampled;
        case GL_EQUAL:
            return reference == sampled;
        case GL_LEQUAL:
            return reference <= sampled;
        case GL_GREATER:
            return reference > sampled;
        case GL_NOTEQUAL:
            return reference != sampled;
        case GL_GEQUAL:
            return reference >= sampled;
        case GL_ALWAYS:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
}

// Variations corresponding to enums above.
using ShadowSamplerFunctionVariationsTestParams =
    std::tuple<angle::PlatformParameters, FunctionType, bool>;

std::ostream &operator<<(std::ostream &out, FunctionType function)
{
    switch (function)
    {
        case FunctionType::Texture:
            out << "Texture";
            break;
        case FunctionType::TextureBias:
            out << "TextureBias";
            break;
        case FunctionType::TextureOffset:
            out << "TextureOffset";
            break;
        case FunctionType::TextureOffsetBias:
            out << "TextureOffsetBias";
            break;
        case FunctionType::TextureLod:
            out << "TextureLod";
            break;
        case FunctionType::TextureLodOffset:
            out << "TextureLodOffset";
            break;
        case FunctionType::TextureGrad:
            out << "TextureGrad";
            break;
        case FunctionType::TextureGradOffset:
            out << "TextureGradOffset";
            break;
        case FunctionType::TextureProj:
            out << "TextureProj";
            break;
        case FunctionType::TextureProjBias:
            out << "TextureProjBias";
            break;
        case FunctionType::TextureProjOffset:
            out << "TextureProjOffset";
            break;
        case FunctionType::TextureProjOffsetBias:
            out << "TextureProjOffsetBias";
            break;
        case FunctionType::TextureProjLod:
            out << "TextureProjLod";
            break;
        case FunctionType::TextureProjLodOffset:
            out << "TextureProjLodOffset";
            break;
        case FunctionType::TextureProjGrad:
            out << "TextureProjGrad";
            break;
        case FunctionType::TextureProjGradOffset:
            out << "TextureProjGradOffset";
            break;
    }

    return out;
}

void ParseShadowSamplerFunctionVariationsTestParams(
    const ShadowSamplerFunctionVariationsTestParams &params,
    FunctionType *functionOut,
    bool *mipmappedOut)
{
    *functionOut  = std::get<1>(params);
    *mipmappedOut = std::get<2>(params);
}

std::string ShadowSamplerFunctionVariationsTestPrint(
    const ::testing::TestParamInfo<ShadowSamplerFunctionVariationsTestParams> &paramsInfo)
{
    const ShadowSamplerFunctionVariationsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    FunctionType function;
    bool mipmapped;
    ParseShadowSamplerFunctionVariationsTestParams(params, &function, &mipmapped);

    out << "__" << function << "_" << (mipmapped ? "Mipmapped" : "NonMipmapped");
    return out.str();
}

class ShadowSamplerFunctionTestBase : public ANGLETest<ShadowSamplerFunctionVariationsTestParams>
{
  protected:
    ShadowSamplerFunctionTestBase()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    GLuint mPrg = 0;
};

class ShadowSamplerFunctionTexture2DTest : public ShadowSamplerFunctionTestBase
{
  protected:
    void setupProgram2D(FunctionType function, bool useShadowSampler)
    {
        std::stringstream fragmentSource;
        fragmentSource << "#version 300 es\n"
                       << "precision mediump float;\n"
                       << "precision mediump sampler2D;\n"
                       << "precision mediump sampler2DShadow;\n"
                       << "uniform float dRef;\n"
                       << "uniform sampler2D" << (useShadowSampler ? "Shadow" : "") << " tex;\n"
                       << "in vec4 v_position;\n"
                       << "out vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    vec2 texcoord = v_position.xy * 0.5 + 0.5;\n"
                       << "    float r = " << FunctionName(function) << "(tex, ";
        if (IsProj(function))
        {
            if (useShadowSampler)
            {
                fragmentSource << "vec4(texcoord * 2.0, dRef * 2.0, 2.0)";
            }
            else
            {
                fragmentSource << "vec3(texcoord * 2.0, 2.0)";
            }
        }
        else
        {
            if (useShadowSampler)
            {
                fragmentSource << "vec3(texcoord, dRef)";
            }
            else
            {
                fragmentSource << "vec2(texcoord)";
            }
        }

        if (HasLOD(function))
        {
            fragmentSource << ", 2.0";
        }
        else if (HasGrad(function))
        {
            fragmentSource << ", vec2(0.17), vec2(0.17)";
        }

        if (HasOffset(function))
        {
            // Does not affect LOD selection, added to try all overloads.
            fragmentSource << ", ivec2(1, 1)";
        }

        if (HasBias(function))
        {
            fragmentSource << ", 3.0";
        }

        fragmentSource << ")" << (useShadowSampler ? "" : ".r") << ";\n";
        if (useShadowSampler)
        {
            fragmentSource << "    my_FragColor = vec4(0.0, r, 1.0 - r, 1.0);\n";
        }
        else
        {
            fragmentSource << "    my_FragColor = vec4(r, 0.0, 0.0, 1.0);\n";
        }
        fragmentSource << "}";

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Passthrough(), fragmentSource.str().c_str());
        glUseProgram(program);
        mPrg = program;
    }
};

class ShadowSamplerFunctionTexture2DArrayTest : public ShadowSamplerFunctionTestBase
{
  protected:
    void setupProgram2DArray(FunctionType function, bool useShadowSampler)
    {
        ASSERT_FALSE(IsProj(function));
        std::stringstream fragmentSource;
        fragmentSource << "#version 300 es\n"
                       << "#extension GL_EXT_texture_shadow_lod : enable\n"
                       << "precision mediump float;\n"
                       << "precision mediump sampler2DArray;\n"
                       << "precision mediump sampler2DArrayShadow;\n"
                       << "uniform float dRef;\n"
                       << "uniform sampler2DArray" << (useShadowSampler ? "Shadow" : "")
                       << " tex;\n"
                       << "in vec4 v_position;\n"
                       << "out vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    vec2 texcoord = v_position.xy * 0.5 + 0.5;\n"
                       << "    float r = " << FunctionName(function) << "(tex, ";
        if (useShadowSampler)
        {
            fragmentSource << "vec4(texcoord, 1.0, dRef)";
        }
        else
        {
            fragmentSource << "vec3(texcoord, 1.0)";
        }

        if (HasLOD(function))
        {
            fragmentSource << ", 2.0";
        }
        else if (HasGrad(function))
        {
            fragmentSource << ", vec2(0.17), vec2(0.17)";
        }

        if (HasOffset(function))
        {
            // Does not affect LOD selection, added to try all overloads.
            fragmentSource << ", ivec2(1, 1)";
        }

        if (HasBias(function))
        {
            fragmentSource << ", 3.0";
        }

        fragmentSource << ")" << (useShadowSampler ? "" : ".r") << ";\n";
        if (useShadowSampler)
        {
            fragmentSource << "    my_FragColor = vec4(0.0, r, 1.0 - r, 1.0);\n";
        }
        else
        {
            fragmentSource << "    my_FragColor = vec4(r, 0.0, 0.0, 1.0);\n";
        }
        fragmentSource << "}";

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Passthrough(), fragmentSource.str().c_str());
        glUseProgram(program);
        mPrg = program;
    }
};

class ShadowSamplerFunctionTextureCubeTest : public ShadowSamplerFunctionTestBase
{
  protected:
    void setupProgramCube(FunctionType function, bool useShadowSampler)
    {
        ASSERT_FALSE(IsProj(function));
        ASSERT_FALSE(HasOffset(function));
        std::stringstream fragmentSource;
        fragmentSource << "#version 300 es\n"
                       << "#extension GL_EXT_texture_shadow_lod : enable\n"
                       << "precision mediump float;\n"
                       << "precision mediump samplerCube;\n"
                       << "precision mediump samplerCubeShadow;\n"
                       << "uniform float dRef;\n"
                       << "uniform samplerCube" << (useShadowSampler ? "Shadow" : "") << " tex;\n"
                       << "in vec4 v_position;\n"
                       << "out vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    vec3 texcoord = vec3(1.0, v_position.xy);\n"
                       << "    float r = " << FunctionName(function) << "(tex, ";
        if (useShadowSampler)
        {
            fragmentSource << "vec4(texcoord, dRef)";
        }
        else
        {
            fragmentSource << "vec3(texcoord)";
        }

        if (HasLOD(function))
        {
            fragmentSource << ", 2.0";
        }
        else if (HasGrad(function))
        {
            fragmentSource << ", vec3(0.0, 0.34, 0.34), vec3(0.0)";
        }

        if (HasBias(function))
        {
            fragmentSource << ", 3.0";
        }

        fragmentSource << ")" << (useShadowSampler ? "" : ".r") << ";\n";
        if (useShadowSampler)
        {
            fragmentSource << "    my_FragColor = vec4(0.0, r, 1.0 - r, 1.0);\n";
        }
        else
        {
            fragmentSource << "    my_FragColor = vec4(r, 0.0, 0.0, 1.0);\n";
        }
        fragmentSource << "}";

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Passthrough(), fragmentSource.str().c_str());
        glUseProgram(program);
        mPrg = program;
    }
};

class ShadowSamplerFunctionTextureCubeArrayTest : public ShadowSamplerFunctionTestBase
{
  protected:
    void setupProgramCubeArray(FunctionType function, bool useShadowSampler)
    {
        ASSERT_FALSE(IsProj(function));
        ASSERT_FALSE(HasOffset(function));
        ASSERT_FALSE(HasGrad(function));
        std::stringstream fragmentSource;
        fragmentSource << "#version 310 es\n"
                       << "#extension GL_EXT_texture_cube_map_array : require\n"
                       << "#extension GL_EXT_texture_shadow_lod : enable\n"
                       << "precision mediump float;\n"
                       << "precision mediump samplerCubeArray;\n"
                       << "precision mediump samplerCubeArrayShadow;\n"
                       << "uniform float dRef;\n"
                       << "uniform samplerCubeArray" << (useShadowSampler ? "Shadow" : "")
                       << " tex;\n"
                       << "in vec4 v_position;\n"
                       << "out vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    vec4 texcoord = vec4(1.0, v_position.xy, 1.0);\n"
                       << "    float r = " << FunctionName(function) << "(tex, texcoord";
        if (useShadowSampler)
        {
            fragmentSource << ", dRef";
        }

        if (HasLOD(function))
        {
            fragmentSource << ", 2.0";
        }
        else if (HasBias(function))
        {
            fragmentSource << ", 3.0";
        }

        fragmentSource << ")" << (useShadowSampler ? "" : ".r") << ";\n";
        if (useShadowSampler)
        {
            fragmentSource << "    my_FragColor = vec4(0.0, r, 1.0 - r, 1.0);\n";
        }
        else
        {
            fragmentSource << "    my_FragColor = vec4(r, 0.0, 0.0, 1.0);\n";
        }
        fragmentSource << "}";

        ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Passthrough(), fragmentSource.str().c_str());
        glUseProgram(program);
        mPrg = program;
    }
};

constexpr GLenum kCompareFuncs[] = {
    GL_LEQUAL, GL_GEQUAL, GL_LESS, GL_GREATER, GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS, GL_NEVER,
};
constexpr float kRefValues[] = {
    0.0f, 0.25f, 0.5f, 0.75f, 1.0f,
};

// Test TEXTURE_2D with shadow samplers
TEST_P(ShadowSamplerFunctionTexture2DTest, Test)
{
    FunctionType function;
    bool mipmapped;
    ParseShadowSamplerFunctionVariationsTestParams(GetParam(), &function, &mipmapped);

    GLTexture tex;
    const std::vector<GLfloat> level0(64, 0.125f);
    const std::vector<GLfloat> level1(16, 0.5f);
    const std::vector<GLfloat> level2(4, 0.25f);
    const std::vector<GLfloat> level3(1, 0.75f);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH_COMPONENT32F, 8, 8);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_DEPTH_COMPONENT, GL_FLOAT, level0.data());
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 4, 4, GL_DEPTH_COMPONENT, GL_FLOAT, level1.data());
    glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 2, 2, GL_DEPTH_COMPONENT, GL_FLOAT, level2.data());
    glTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, level3.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    float expectedSample;
    if (mipmapped)
    {
        if (HasBias(function))
        {
            // Base level 8x8, viewport 8x8, bias 3.0
            expectedSample = level3[0];
        }
        else if (HasLOD(function))
        {
            // Explicitly requested level 2
            expectedSample = level2[0];
        }
        else if (HasGrad(function))
        {
            // Screen space derivatives of 0.17 for a 8x8 texture should resolve to level 1
            expectedSample = level1[0];
        }
        else  // implicit LOD
        {
            // Base level 8x8, viewport 8x8, no bias
            expectedSample = level0[0];
        }
    }
    else
    {
        // LOD options must have no effect when the texture is not mipmapped.
        expectedSample = level0[0];
    }

    glViewport(0, 0, 8, 8);
    glClearColor(1.0, 0.0, 1.0, 1.0);

    // First sample the texture directly for easier debugging
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    setupProgram2D(function, false);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(expectedSample * 255.0, 0, 0, 255), 1);

    // Try shadow samplers
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    setupProgram2D(function, true);
    const GLint loc = glGetUniformLocation(mPrg, "dRef");
    for (const float refValue : kRefValues)
    {
        glUniform1f(loc, refValue);
        for (const GLenum compareFunc : kCompareFuncs)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, compareFunc);
            ASSERT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
            if (Compare(refValue, expectedSample, compareFunc))
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::green)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::blue)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
        }
    }
}

// Test TEXTURE_2D_ARRAY with shadow samplers
TEST_P(ShadowSamplerFunctionTexture2DArrayTest, Test)
{
    FunctionType function;
    bool mipmapped;
    ParseShadowSamplerFunctionVariationsTestParams(GetParam(), &function, &mipmapped);

    if (RequiresExtensionFor2DArray(function))
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_shadow_lod"));
    }

    GLTexture tex;
    const std::vector<GLfloat> unused(64, 0.875);
    const std::vector<GLfloat> level0(64, 0.125f);
    const std::vector<GLfloat> level1(16, 0.5f);
    const std::vector<GLfloat> level2(4, 0.25f);
    const std::vector<GLfloat> level3(1, 0.75f);

    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_DEPTH_COMPONENT32F, 8, 8, 2);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 8, 8, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    unused.data());
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 0, 4, 4, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    unused.data());
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, 0, 2, 2, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    unused.data());
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 3, 0, 0, 0, 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    unused.data());
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, 8, 8, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    level0.data());
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 1, 4, 4, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    level1.data());
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, 1, 2, 2, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    level2.data());
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 3, 0, 0, 1, 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                    level3.data());
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
                    mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    float expectedSample;
    if (mipmapped)
    {
        if (HasBias(function))
        {
            // Base level 8x8, viewport 8x8, bias 3.0
            expectedSample = level3[0];
        }
        else if (HasLOD(function))
        {
            // Explicitly requested level 2
            expectedSample = level2[0];
        }
        else if (HasGrad(function))
        {
            // Screen space derivatives of 0.17 for a 8x8 texture should resolve to level 1
            expectedSample = level1[0];
        }
        else  // implicit LOD
        {
            // Base level 8x8, viewport 8x8, no bias
            expectedSample = level0[0];
        }
    }
    else
    {
        // LOD options must have no effect when the texture is not mipmapped.
        expectedSample = level0[0];
    }

    glViewport(0, 0, 8, 8);
    glClearColor(1.0, 0.0, 1.0, 1.0);

    // First sample the texture directly for easier debugging
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    setupProgram2DArray(function, false);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(expectedSample * 255.0, 0, 0, 255), 1);

    // Try shadow samplers
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    setupProgram2DArray(function, true);
    const GLint loc = glGetUniformLocation(mPrg, "dRef");
    for (const float refValue : kRefValues)
    {
        glUniform1f(loc, refValue);
        for (const GLenum compareFunc : kCompareFuncs)
        {
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, compareFunc);
            ASSERT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
            if (Compare(refValue, expectedSample, compareFunc))
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::green)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::blue)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
        }
    }
}

// Test TEXTURE_CUBE_MAP with shadow samplers
TEST_P(ShadowSamplerFunctionTextureCubeTest, Test)
{
    FunctionType function;
    bool mipmapped;
    ParseShadowSamplerFunctionVariationsTestParams(GetParam(), &function, &mipmapped);

    if (RequiresExtensionForCube(function))
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_shadow_lod"));
    }

    GLTexture tex;
    const std::vector<GLfloat> level0(64, 0.125f);
    const std::vector<GLfloat> level1(16, 0.5f);
    const std::vector<GLfloat> level2(4, 0.25f);
    const std::vector<GLfloat> level3(1, 0.75f);

    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 4, GL_DEPTH_COMPONENT32F, 8, 8);
    for (const GLenum face : {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                              GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                              GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z})
    {
        glTexSubImage2D(face, 0, 0, 0, 8, 8, GL_DEPTH_COMPONENT, GL_FLOAT, level0.data());
        glTexSubImage2D(face, 1, 0, 0, 4, 4, GL_DEPTH_COMPONENT, GL_FLOAT, level1.data());
        glTexSubImage2D(face, 2, 0, 0, 2, 2, GL_DEPTH_COMPONENT, GL_FLOAT, level2.data());
        glTexSubImage2D(face, 3, 0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, level3.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    float expectedSample;
    if (mipmapped)
    {
        if (HasBias(function))
        {
            // Base level 8x8, viewport 8x8, bias 3.0
            expectedSample = level3[0];
        }
        else if (HasLOD(function))
        {
            // Explicitly requested level 2
            expectedSample = level2[0];
        }
        else if (HasGrad(function))
        {
            // Cube screen space derivatives should be projected as 0.17
            // on the +X face and resolved to level 1 for a 8x8 texture
            expectedSample = level1[0];
        }
        else  // implicit LOD
        {
            // Base level 8x8, viewport 8x8, no bias
            expectedSample = level0[0];
        }
    }
    else
    {
        // LOD options must have no effect when the texture is not mipmapped.
        expectedSample = level0[0];
    }

    glViewport(0, 0, 8, 8);
    glClearColor(1.0, 0.0, 1.0, 1.0);

    // First sample the texture directly for easier debugging
    // This step is skipped on Apple GPUs when the PreTransformTextureCubeGradDerivatives feature
    // is disabled for testing because native cubemap sampling with explicit derivatives does not
    // work on that platform without this feature.
    // Since the AllowSamplerCompareGradient feature is also disabled in this case, the next steps,
    // which use shadow samplers, rely on emulation and thus they are not affected.
    const auto &disabledFeatures = std::get<0>(GetParam()).eglParameters.disabledFeatureOverrides;
    if (!IsAppleGPU() ||
        std::find(disabledFeatures.begin(), disabledFeatures.end(),
                  Feature::PreTransformTextureCubeGradDerivatives) == disabledFeatures.end())
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        setupProgramCube(function, false);
        ASSERT_GL_NO_ERROR();

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(expectedSample * 255.0, 0, 0, 255), 1);
    }

    // Try shadow samplers
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    setupProgramCube(function, true);
    const GLint loc = glGetUniformLocation(mPrg, "dRef");
    for (const float refValue : kRefValues)
    {
        glUniform1f(loc, refValue);
        for (const GLenum compareFunc : kCompareFuncs)
        {
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, compareFunc);
            ASSERT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
            if (Compare(refValue, expectedSample, compareFunc))
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::green)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::blue)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
        }
    }
}

// Test TEXTURE_CUBE_MAP_ARRAY with shadow samplers
TEST_P(ShadowSamplerFunctionTextureCubeArrayTest, Test)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_cube_map_array"));

    FunctionType function;
    bool mipmapped;
    ParseShadowSamplerFunctionVariationsTestParams(GetParam(), &function, &mipmapped);

    if (RequiresExtensionForCubeArray(function))
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_shadow_lod"));
    }

    GLTexture tex;
    const std::vector<GLfloat> unused(64, 0.875);
    const std::vector<GLfloat> level0(64, 0.125f);
    const std::vector<GLfloat> level1(16, 0.5f);
    const std::vector<GLfloat> level2(4, 0.25f);
    const std::vector<GLfloat> level3(1, 0.75f);

    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, tex);
    glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, 4, GL_DEPTH_COMPONENT32F, 8, 8, 12);
    for (size_t k = 0; k < 6 * 2; ++k)
    {
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, 0, 0, 0, k, 8, 8, 1, GL_DEPTH_COMPONENT,
                        GL_FLOAT, k > 5 ? level0.data() : unused.data());
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, 1, 0, 0, k, 4, 4, 1, GL_DEPTH_COMPONENT,
                        GL_FLOAT, k > 5 ? level1.data() : unused.data());
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, 2, 0, 0, k, 2, 2, 1, GL_DEPTH_COMPONENT,
                        GL_FLOAT, k > 5 ? level2.data() : unused.data());
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, 3, 0, 0, k, 1, 1, 1, GL_DEPTH_COMPONENT,
                        GL_FLOAT, k > 5 ? level3.data() : unused.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, GL_TEXTURE_MIN_FILTER,
                    mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    float expectedSample;
    if (mipmapped)
    {
        if (HasBias(function))
        {
            // Base level 8x8, viewport 8x8, bias 3.0
            expectedSample = level3[0];
        }
        else if (HasLOD(function))
        {
            // Explicitly requested level 2
            expectedSample = level2[0];
        }
        else  // implicit LOD
        {
            // Base level 8x8, viewport 8x8, no bias
            expectedSample = level0[0];
        }
    }
    else
    {
        // LOD options must have no effect when the texture is not mipmapped.
        expectedSample = level0[0];
    }

    glViewport(0, 0, 8, 8);
    glClearColor(1.0, 0.0, 1.0, 1.0);

    // First sample the texture directly for easier debugging
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    setupProgramCubeArray(function, false);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mPrg, essl31_shaders::PositionAttrib(), 0.0f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(expectedSample * 255.0, 0, 0, 255), 1);

    // Try shadow samplers
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE,
                    GL_COMPARE_REF_TO_TEXTURE);
    setupProgramCubeArray(function, true);
    const GLint loc = glGetUniformLocation(mPrg, "dRef");
    for (const float refValue : kRefValues)
    {
        glUniform1f(loc, refValue);
        for (const GLenum compareFunc : kCompareFuncs)
        {
            glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_EXT, GL_TEXTURE_COMPARE_FUNC, compareFunc);
            ASSERT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(mPrg, essl31_shaders::PositionAttrib(), 0.0f);
            if (Compare(refValue, expectedSample, compareFunc))
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::green)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(4, 4, GLColor::blue)
                    << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
                    << ", reference " << refValue << ", expected sample " << expectedSample;
            }
        }
    }
}

constexpr FunctionType kTexture2DFunctionTypes[] = {
    FunctionType::Texture,           FunctionType::TextureBias,
    FunctionType::TextureOffset,     FunctionType::TextureOffsetBias,
    FunctionType::TextureLod,        FunctionType::TextureLodOffset,
    FunctionType::TextureGrad,       FunctionType::TextureGradOffset,
    FunctionType::TextureProj,       FunctionType::TextureProjBias,
    FunctionType::TextureProjOffset, FunctionType::TextureProjOffsetBias,
    FunctionType::TextureProjLod,    FunctionType::TextureProjLodOffset,
    FunctionType::TextureProjGrad,   FunctionType::TextureProjGradOffset,
};

constexpr FunctionType kTexture2DArrayFunctionTypes[] = {
    FunctionType::Texture,       FunctionType::TextureBias,
    FunctionType::TextureOffset, FunctionType::TextureOffsetBias,
    FunctionType::TextureLod,    FunctionType::TextureLodOffset,
    FunctionType::TextureGrad,   FunctionType::TextureGradOffset,
};

constexpr FunctionType kTextureCubeFunctionTypes[] = {
    FunctionType::Texture,
    FunctionType::TextureBias,
    FunctionType::TextureLod,
    FunctionType::TextureGrad,
};

constexpr FunctionType kTextureCubeArrayFunctionTypes[] = {
    FunctionType::Texture,
    FunctionType::TextureBias,
    FunctionType::TextureLod,
};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShadowSamplerFunctionTexture2DTest);
ANGLE_INSTANTIATE_TEST_COMBINE_2(ShadowSamplerFunctionTexture2DTest,
                                 ShadowSamplerFunctionVariationsTestPrint,
                                 testing::ValuesIn(kTexture2DFunctionTypes),
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3,
                                 ES3_METAL()
                                     .disable(Feature::AllowSamplerCompareGradient)
                                     .disable(Feature::PreTransformTextureCubeGradDerivatives));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShadowSamplerFunctionTexture2DArrayTest);
ANGLE_INSTANTIATE_TEST_COMBINE_2(ShadowSamplerFunctionTexture2DArrayTest,
                                 ShadowSamplerFunctionVariationsTestPrint,
                                 testing::ValuesIn(kTexture2DArrayFunctionTypes),
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3,
                                 ES3_METAL()
                                     .disable(Feature::AllowSamplerCompareGradient)
                                     .disable(Feature::PreTransformTextureCubeGradDerivatives));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShadowSamplerFunctionTextureCubeTest);
ANGLE_INSTANTIATE_TEST_COMBINE_2(ShadowSamplerFunctionTextureCubeTest,
                                 ShadowSamplerFunctionVariationsTestPrint,
                                 testing::ValuesIn(kTextureCubeFunctionTypes),
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3,
                                 ES3_METAL()
                                     .disable(Feature::AllowSamplerCompareGradient)
                                     .disable(Feature::PreTransformTextureCubeGradDerivatives));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShadowSamplerFunctionTextureCubeArrayTest);
ANGLE_INSTANTIATE_TEST_COMBINE_2(ShadowSamplerFunctionTextureCubeArrayTest,
                                 ShadowSamplerFunctionVariationsTestPrint,
                                 testing::ValuesIn(kTextureCubeArrayFunctionTypes),
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES31);

}  // anonymous namespace
