/*
* Copyright (C) 2022 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <gtest/gtest.h>

#include "MetalArgumentBuffer.h"

using namespace filamat;
using namespace filament::backend;

// -------------------------------------------------------------------------------------------------

TEST(ArgBufferFixup, Empty) {
    auto argBuffer =
            MetalArgumentBuffer::Builder()
                    .name("spvDescriptorSet0")
                    .build();
    auto argBufferStr = argBuffer->getMsl();

    const std::string expected =
            "struct spvDescriptorSet0 {\n"
            "}";

    EXPECT_EQ(argBuffer->getMsl(), expected);

    MetalArgumentBuffer::destroy(&argBuffer);
}

TEST(ArgBufferFixup, NoName) {
    // A valid arg buffer name must be provided to MetalArgumentBuffer.
    // This assertion is only fired in debug builds.
#if !defined(NDEBUG) && defined(GTEST_HAS_DEATH_TEST)
    EXPECT_DEATH({
        auto argBuffer = MetalArgumentBuffer::Builder().build();
        MetalArgumentBuffer::destroy(&argBuffer);
    }, "failed assertion");
    EXPECT_DEATH({
        auto argBuffer = MetalArgumentBuffer::Builder().name("").build();
        MetalArgumentBuffer::destroy(&argBuffer);
    }, "failed assertion");
#endif
}

TEST(ArgBufferFixup, DuplicateIndices) {
    // Each index must be unique.
#if !defined(NDEBUG) && defined(GTEST_HAS_DEATH_TEST)
    EXPECT_DEATH({
                auto argBuffer = MetalArgumentBuffer::Builder()
                                         .name("myArgumentBuffer")
                                         .sampler(0, "samplerA")
                                         .sampler(0, "samplerB")
                        .build();
                MetalArgumentBuffer::destroy(&argBuffer);
    }, "failed assertion");
#endif
}

TEST(ArgBufferFixup, TextureAndSampler) {
    auto argBuffer =
            MetalArgumentBuffer::Builder()
                    .name("myArgumentBuffer")
                    .texture(0, "textureA", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, false)
                    .sampler(1, "samplerA")
                    .build();
    auto argBufferStr = argBuffer->getMsl();

    const std::string expected =
            "struct myArgumentBuffer {\n"
            "texture2d<float> textureA [[id(0)]];\n"
            "sampler samplerA [[id(1)]];\n"
            "}";

    EXPECT_EQ(argBuffer->getMsl(), expected);

    MetalArgumentBuffer::destroy(&argBuffer);
}

TEST(ArgBufferFixup, TextureAndSamplerMS) {
    auto argBuffer =
            MetalArgumentBuffer::Builder()
                    .name("myArgumentBuffer")
                    .texture(0, "textureA", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, true)
                    .sampler(1, "samplerA")
                    .build();
    auto argBufferStr = argBuffer->getMsl();

    const std::string expected =
            "struct myArgumentBuffer {\n"
            "texture2d_ms<float> textureA [[id(0)]];\n"
            "sampler samplerA [[id(1)]];\n"
            "}";

    EXPECT_EQ(argBuffer->getMsl(), expected);

    MetalArgumentBuffer::destroy(&argBuffer);
}

TEST(ArgBufferFixup, Sorted) {
    auto argBuffer =
            MetalArgumentBuffer::Builder()
                    .name("myArgumentBuffer")
                    .sampler(3, "samplerB")
                    .texture(0, "textureA", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, false)
                    .texture(2, "textureB", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, false)
                    .sampler(1, "samplerA")
                    .build();
    auto argBufferStr = argBuffer->getMsl();

    const std::string expected =
            "struct myArgumentBuffer {\n"
            "texture2d<float> textureA [[id(0)]];\n"
            "sampler samplerA [[id(1)]];\n"
            "texture2d<float> textureB [[id(2)]];\n"
            "sampler samplerB [[id(3)]];\n"
            "}";

    EXPECT_EQ(argBuffer->getMsl(), expected);

    MetalArgumentBuffer::destroy(&argBuffer);
}

TEST(ArgBufferFixup, TextureTypes) {
    auto argBuffer =
            MetalArgumentBuffer::Builder()
                    .name("myArgumentBuffer")
                    .texture(0, "textureA", SamplerType::SAMPLER_2D, SamplerFormat::INT, false)
                    .texture(1, "textureB", SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::UINT, false)
                    .texture(2, "textureC", SamplerType::SAMPLER_CUBEMAP, SamplerFormat::FLOAT, false)
                    .texture(3, "textureD", SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT, false)
                    .texture(4, "textureE", SamplerType::SAMPLER_3D, SamplerFormat::FLOAT, false)
                    .texture(5, "textureF", SamplerType::SAMPLER_CUBEMAP_ARRAY, SamplerFormat::SHADOW, false)
                    .build();
    auto argBufferStr = argBuffer->getMsl();

    const std::string expected =
            "struct myArgumentBuffer {\n"
            "texture2d<int> textureA [[id(0)]];\n"
            "texture2d_array<uint> textureB [[id(1)]];\n"
            "texturecube<float> textureC [[id(2)]];\n"
            "texture2d<float> textureD [[id(3)]];\n"
            "texture3d<float> textureE [[id(4)]];\n"
            "depthcube_array<float> textureF [[id(5)]];\n"
            "}";

    EXPECT_EQ(argBuffer->getMsl(), expected);

    MetalArgumentBuffer::destroy(&argBuffer);
}

TEST(ArgBufferFixup, InvalidType) {
    // All combinations of SamplerType and SamplerFormat are valid except for SAMPLER_3D / SHADOW.
#if !defined(NDEBUG) && defined(GTEST_HAS_DEATH_TEST)
    EXPECT_DEATH({
        auto argBuffer =
                MetalArgumentBuffer::Builder()
                        .name("myArgumentBuffer")
                        .texture(0, "textureA", SamplerType::SAMPLER_3D, SamplerFormat::SHADOW, false)
                        .build();
        MetalArgumentBuffer::destroy(&argBuffer);
    }, "failed assertion");
#endif
}

TEST(ArgBufferFixup, ReplaceInShader) {
    {
        std::string shader = "struct argBuffer {}"; // different name
        EXPECT_FALSE(MetalArgumentBuffer::replaceInShader(shader, "argBufferX", "xyz"));
    }
    {
        std::string shader = "argBuffer {}   struct";    // no 'struct' keyword before
        EXPECT_FALSE(MetalArgumentBuffer::replaceInShader(shader, "argBuffer", "xyz"));
    }
    {
        std::string shader = "struct argBuffer";    // no braces
        EXPECT_FALSE(MetalArgumentBuffer::replaceInShader(shader, "argBuffer", "xyz"));
    }
    {
        std::string shader = "## struct argBuffer {}  ###";
        EXPECT_TRUE(MetalArgumentBuffer::replaceInShader(shader, "argBuffer", "replacement"));
        EXPECT_EQ(shader, "## replacement  ###");
    }
    {
        std::string shader = "argBuffer {} struct argBuffer {}  argBuffer";
        EXPECT_TRUE(MetalArgumentBuffer::replaceInShader(shader, "argBuffer", "replacement"));
        EXPECT_EQ(shader, "argBuffer {} replacement  argBuffer");
    }
    {
        std::string shader =
            "struct FooBar {};\n"
            "targetArgBuffer {};\n"
            "struct targetArgBufferX {};\n"
            "void someFunctionThatTakesAnArgBuffer(targetArgBuffer& arg);\n"
            "struct targetArgBuffer{};\n";
        std::string expected =
                "struct FooBar {};\n"
                "targetArgBuffer {};\n"
                "struct targetArgBufferX {};\n"
                "void someFunctionThatTakesAnArgBuffer(targetArgBuffer& arg);\n"
                "replacement;\n";
        EXPECT_TRUE(MetalArgumentBuffer::replaceInShader(shader, "targetArgBuffer", "replacement"));
        EXPECT_EQ(shader, expected);
    }
}
