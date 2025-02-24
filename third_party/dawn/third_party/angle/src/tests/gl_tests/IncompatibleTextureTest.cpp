//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// These tests assert that textures incompatible with samplers do not cause any runtime errors.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

enum class SamplerType
{
    Float,
    SignedInteger,
    UnsignedInteger,
    Shadow,
};

enum class TextureType
{
    UnsignedNormalized,
    SignedNormalized,
    Float,
    UnsignedInteger,
    SignedInteger,
    Depth,
    DepthStencilDepthMode,
    DepthStencilStencilMode,
    Stencil,
};

enum class TextureCompareMode
{
    None,
    Ref,
};

// Variations corresponding to enums above.
using IncompatibleTextureVariationsTestParams =
    std::tuple<angle::PlatformParameters, SamplerType, TextureType, TextureCompareMode>;

std::ostream &operator<<(std::ostream &out, SamplerType samplerType)
{
    switch (samplerType)
    {
        case SamplerType::Float:
            out << "Float";
            break;
        case SamplerType::SignedInteger:
            out << "SignedInteger";
            break;
        case SamplerType::UnsignedInteger:
            out << "UnsignedInteger";
            break;
        case SamplerType::Shadow:
            out << "Shadow";
            break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::UnsignedNormalized:
            out << "UnsignedNormalized";
            break;
        case TextureType::SignedNormalized:
            out << "SignedNormalized";
            break;
        case TextureType::Float:
            out << "Float";
            break;
        case TextureType::UnsignedInteger:
            out << "UnsignedInteger";
            break;
        case TextureType::SignedInteger:
            out << "SignedInteger";
            break;
        case TextureType::Depth:
            out << "Depth";
            break;
        case TextureType::DepthStencilDepthMode:
            out << "DepthStencilDepthMode";
            break;
        case TextureType::DepthStencilStencilMode:
            out << "DepthStencilStencilMode";
            break;
        case TextureType::Stencil:
            out << "Stencil";
            break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, TextureCompareMode textureCompareMode)
{
    switch (textureCompareMode)
    {
        case TextureCompareMode::None:
            out << "None";
            break;
        case TextureCompareMode::Ref:
            out << "Ref";
            break;
    }

    return out;
}

void ParseIncompatibleTextureTestParams(const IncompatibleTextureVariationsTestParams &params,
                                        SamplerType *samplerTypeOut,
                                        TextureType *textureTypeOut,
                                        TextureCompareMode *textureCompareModeOut)
{
    *samplerTypeOut        = std::get<1>(params);
    *textureTypeOut        = std::get<2>(params);
    *textureCompareModeOut = std::get<3>(params);
}

std::string IncompatibleTextureVariationsTestPrint(
    const ::testing::TestParamInfo<IncompatibleTextureVariationsTestParams> &paramsInfo)
{
    const IncompatibleTextureVariationsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    SamplerType samplerType;
    TextureType textureType;
    TextureCompareMode textureCompareMode;
    ParseIncompatibleTextureTestParams(params, &samplerType, &textureType, &textureCompareMode);

    out << "__" << "SamplerType" << samplerType << "_" << "TextureType" << textureType << "_"
        << "TextureCompareMode" << textureCompareMode;
    return out.str();
}

class IncompatibleTextureTest : public ANGLETest<IncompatibleTextureVariationsTestParams>
{};

// Test that no errors are generated
TEST_P(IncompatibleTextureTest, Test)
{
    SamplerType samplerType;
    TextureType textureType;
    TextureCompareMode textureCompareMode;
    ParseIncompatibleTextureTestParams(GetParam(), &samplerType, &textureType, &textureCompareMode);

    std::stringstream fragmentSource;
    fragmentSource << "#version 300 es\n" << "out mediump vec4 my_FragColor;\n";
    switch (samplerType)
    {
        case SamplerType::Float:
            fragmentSource << "uniform mediump sampler2D tex;\n";
            break;
        case SamplerType::SignedInteger:
            fragmentSource << "uniform mediump isampler2D tex;\n";
            break;
        case SamplerType::UnsignedInteger:
            fragmentSource << "uniform mediump usampler2D tex;\n";
            break;
        case SamplerType::Shadow:
            fragmentSource << "uniform mediump sampler2DShadow tex;\n";
            break;
    }
    fragmentSource << "void main()\n" << "{\n";
    fragmentSource << "    my_FragColor = vec4(texture(tex, "
                   << (samplerType == SamplerType::Shadow ? "vec3" : "vec2") << "(0)));\n";
    fragmentSource << "}";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), fragmentSource.str().c_str());
    ASSERT_GL_NO_ERROR();

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    switch (textureType)
    {
        case TextureType::UnsignedNormalized:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            break;
        case TextureType::SignedNormalized:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, 1, 1, 0, GL_RGBA, GL_BYTE, nullptr);
            break;
        case TextureType::Float:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
            break;
        case TextureType::UnsignedInteger:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, 1, 1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,
                         nullptr);
            break;
        case TextureType::SignedInteger:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8I, 1, 1, 0, GL_RGBA_INTEGER, GL_BYTE, nullptr);
            break;
        case TextureType::Depth:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1, 1, 0, GL_DEPTH_COMPONENT,
                         GL_UNSIGNED_SHORT, nullptr);
            break;
        case TextureType::DepthStencilDepthMode:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, 1, 1, 0, GL_DEPTH_STENCIL,
                         GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);
            break;
        case TextureType::DepthStencilStencilMode:
            ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_stencil_texturing"));
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, 1, 1, 0, GL_DEPTH_STENCIL,
                         GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE_ANGLE, GL_STENCIL_INDEX);
            break;
        case TextureType::Stencil:
            ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_stencil8"));
            glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, 1, 1, 0, GL_STENCIL_INDEX,
                         GL_UNSIGNED_BYTE, nullptr);
            break;
    }
    ASSERT_GL_NO_ERROR();

    switch (textureCompareMode)
    {
        case TextureCompareMode::None:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
            break;
        case TextureCompareMode::Ref:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            break;
    }
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.0f);
    EXPECT_GL_NO_ERROR();
}

constexpr SamplerType kSamplerTypes[] = {
    SamplerType::Float,
    SamplerType::SignedInteger,
    SamplerType::UnsignedInteger,
    SamplerType::Shadow,
};
constexpr TextureType kTextureTypes[] = {
    TextureType::UnsignedNormalized,    TextureType::SignedNormalized,        TextureType::Float,
    TextureType::UnsignedInteger,       TextureType::SignedInteger,           TextureType::Depth,
    TextureType::DepthStencilDepthMode, TextureType::DepthStencilStencilMode, TextureType::Stencil,
};
constexpr TextureCompareMode kTextureCompareModes[] = {
    TextureCompareMode::None,
    TextureCompareMode::Ref,
};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(IncompatibleTextureTest);
ANGLE_INSTANTIATE_TEST_COMBINE_3(IncompatibleTextureTest,
                                 IncompatibleTextureVariationsTestPrint,
                                 testing::ValuesIn(kSamplerTypes),
                                 testing::ValuesIn(kTextureTypes),
                                 testing::ValuesIn(kTextureCompareModes),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

}  // anonymous namespace
