//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureFunction_test.cpp:
//   Tests that malformed texture function calls don't pass compilation.
//

#include <array>

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class TextureFunctionTest : public ShaderCompileTreeTest
{
  public:
    TextureFunctionTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

// Test that none of the texture offset functions can take a non-constant parameter.
TEST_F(TextureFunctionTest, NonConstantOffset)
{
    const std::string &shaderTemplate =
        R"(#version 310 es
        precision mediump float;
        out vec4 my_FragColor;
        uniform sampler2D s;
        uniform vec4 samplePos;

        void main()
        {
            my_FragColor = {sample};
        })";

    const std::array<const std::string, 10> sampleVariants(
        {"texelFetchOffset(s, ivec2(samplePos.xy), 0, {offset})",
         "textureLodOffset(s, samplePos.xy, 0.0, {offset})",
         "textureProjLodOffset(s, samplePos.xyz, 0.0, {offset})",
         "textureGradOffset(s, samplePos.xy, vec2(1.0), vec2(1.0), {offset})",
         "textureProjGradOffset(s, samplePos.xyz, vec2(1.0), vec2(1.0), {offset})",
         "textureOffset(s, samplePos.xy, {offset})",
         "textureOffset(s, samplePos.xy, {offset}, 1.0)",
         "textureProjOffset(s, samplePos.xyz, {offset})",
         "textureProjOffset(s, samplePos.xyz, {offset}, 1.0)",
         "textureGatherOffset(s, samplePos.xy, {offset})"});

    size_t sampleReplacePos = shaderTemplate.find("{sample}");

    for (auto &variantTemplate : sampleVariants)
    {
        size_t offsetReplacePos = variantTemplate.find("{offset}");

        std::string variantConst = variantTemplate;
        variantConst.replace(offsetReplacePos, 8, "ivec2(0, 0)");

        std::string shaderValid = shaderTemplate;
        shaderValid.replace(sampleReplacePos, 8, variantConst);
        if (!compile(shaderValid))
        {
            FAIL() << "Shader compilation failed with sample function " << variantConst
                   << ", expecting success:\n"
                   << mInfoLog;
        }

        std::string variantNonConst = variantTemplate;
        variantNonConst.replace(offsetReplacePos, 8, "ivec2(samplePos.xy)");

        std::string shaderNonConst = shaderTemplate;
        shaderNonConst.replace(sampleReplacePos, 8, variantNonConst);
        if (compile(shaderNonConst))
        {
            FAIL() << "Shader compilation succeeded with sample function " << variantNonConst
                   << ", expecting failure:\n"
                   << mInfoLog;
        }

        std::string variantOutOfBounds = variantTemplate;
        variantOutOfBounds.replace(offsetReplacePos, 8, "ivec2(1000, 1000)");

        std::string shaderOutOfBounds = shaderTemplate;
        shaderOutOfBounds.replace(sampleReplacePos, 8, variantOutOfBounds);
        if (compile(shaderOutOfBounds))
        {
            FAIL() << "Shader compilation succeeded with sample function " << variantOutOfBounds
                   << ", expecting failure:\n"
                   << mInfoLog;
        }
    }
}
