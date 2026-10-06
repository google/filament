/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "GLSLPostProcessor.h"

#include "sca/builtinResource.h"

#include <private/filament/EngineEnums.h>

#include <filamat/MaterialBuilder.h>
#include <filamat/Package.h>

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/JobSystem.h>

#include <GlslangToSpv.h>
#include <gtest/gtest.h>
#include <ShaderLang.h>
#include <tint/tint.h>

#include <memory>
#include <string>
#include <vector>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

using namespace filamat;
using namespace filament::backend;

namespace {

// The material samplers live in the per-material descriptor set.
constexpr descriptor_set_t MATERIAL_SET = +filament::DescriptorSetBindingPoints::PER_MATERIAL;

// Sampler declarations shared by the tests: `regular` is a SAMPLER_2D and `video` is a
// SAMPLER_EXTERNAL. As in Filament's generated code, the external sampler is a sampler2D.
constexpr char const* SAMPLER_DECLARATIONS = R"(
layout(set = 2, binding = 0) uniform sampler2D regular;
layout(set = 2, binding = 1) uniform sampler2D video;
)";

constexpr descriptor_binding_t VIDEO_BINDING = 1;

size_t countOccurrences(std::string const& text, std::string const& pattern) {
    size_t count = 0;
    for (size_t pos = text.find(pattern); pos != std::string::npos;
            pos = text.find(pattern, pos + pattern.size())) {
        count++;
    }
    return count;
}

class WgslExternalTextures : public testing::Test {
protected:
    void SetUp() override {
        mJobSystem = std::make_unique<utils::JobSystem>();
        mJobSystem->adopt();
        MaterialBuilder::init();
    }

    void TearDown() override {
        MaterialBuilder::shutdown();
        mJobSystem->emancipate();
    }

    // Compiles Vulkan-flavored GLSL to SPIR-V, without optimizations.
    static SpirvBlob compileToSpirv(char const* body, EShLanguage stage) {
        utils::CString source("#version 450\n");
        source += SAMPLER_DECLARATIONS;
        source += body;
        char const* const sourceCString = source.c_str();

        glslang::TShader shader(stage);
        shader.setStrings(&sourceCString, 1);
        shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        EShMessages const messages = EShMessages(EShMsgVulkanRules | EShMsgSpvRules);
        if (!shader.parse(&DefaultTBuiltInResource, 450, false, messages)) {
            ADD_FAILURE() << shader.getInfoLog();
            return {};
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages)) {
            ADD_FAILURE() << program.getInfoLog();
            return {};
        }

        SpirvBlob spirv;
        glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
        return spirv;
    }

    // Compiles GLSL to WGSL, declaring `video` as an external sampler. Returns false if the
    // conversion fails.
    static bool compileToWgsl(char const* body, EShLanguage stage, std::string* wgsl) {
        SpirvBlob spirv = compileToSpirv(body, stage);
        if (spirv.empty()) {
            return false;
        }
        return GLSLPostProcessor::spirvToWgsl(&spirv, wgsl, { { MATERIAL_SET, VIDEO_BINDING } });
    }

    static testing::AssertionResult isValidWgsl(std::string const& wgsl) {
        tint::Source::File const file("test.wgsl", wgsl);
        tint::wgsl::reader::Options options;
        options.allowed_features = tint::wgsl::AllowedFeatures::Everything();
        tint::Program const program = tint::wgsl::reader::Parse(&file, options);
        if (!program.IsValid()) {
            return testing::AssertionFailure() << program.Diagnostics().Str() << "\n" << wgsl;
        }
        return testing::AssertionSuccess();
    }

    std::unique_ptr<utils::JobSystem> mJobSystem;
};

TEST_F(WgslExternalTextures, Sample) {
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        void main() {
            color = texture(video, vec2(0.5));
        }
    )", EShLangFragment, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    EXPECT_NE(wgsl.find("var video_image : texture_external;"), std::string::npos) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "texture_external"), 1) << wgsl;
    EXPECT_NE(wgsl.find("textureSampleBaseClampToEdge(video_image, "), std::string::npos) << wgsl;
    EXPECT_EQ(wgsl.find("textureSample("), std::string::npos) << wgsl;
}

TEST_F(WgslExternalTextures, RegularSamplerIsUnchanged) {
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        void main() {
            color = texture(regular, vec2(0.5)) + texture(video, vec2(0.5));
        }
    )", EShLangFragment, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    EXPECT_EQ(countOccurrences(wgsl, "texture_external"), 1) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "texture_2d<f32>"), 1) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "textureSample("), 1) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "textureSampleBaseClampToEdge("), 1) << wgsl;
}

TEST_F(WgslExternalTextures, OutputIsUnchangedWithoutExternalSamplers) {
    char const* const body = R"(
        layout(location = 0) out vec4 color;
        void main() {
            color = texture(regular, vec2(0.5)) + texture(video, vec2(0.5));
        }
    )";
    SpirvBlob spirv0 = compileToSpirv(body, EShLangFragment);
    SpirvBlob spirv1 = spirv0;

    std::string expected;
    ASSERT_TRUE(GLSLPostProcessor::spirvToWgsl(&spirv0, &expected));

    // An external sampler that the shader doesn't use.
    std::string actual;
    ASSERT_TRUE(GLSLPostProcessor::spirvToWgsl(&spirv1, &actual, { { MATERIAL_SET, 7 } }));

    EXPECT_EQ(expected, actual);
}

TEST_F(WgslExternalTextures, BiasLevelAndGradientsAreDropped) {
    // Filament adds a LOD bias to the texture() calls of the material code, so biased samples
    // must be supported.
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        void main() {
            vec2 uv = vec2(0.5);
            color = texture(video, uv, 1.0)
                  + textureLod(video, uv, 2.0)
                  + textureGrad(video, uv, vec2(0.1), vec2(0.1));
        }
    )", EShLangFragment, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    EXPECT_EQ(countOccurrences(wgsl, "textureSampleBaseClampToEdge("), 3) << wgsl;
    EXPECT_EQ(wgsl.find("textureSampleBias("), std::string::npos) << wgsl;
    EXPECT_EQ(wgsl.find("textureSampleLevel("), std::string::npos) << wgsl;
    EXPECT_EQ(wgsl.find("textureSampleGrad("), std::string::npos) << wgsl;
}

TEST_F(WgslExternalTextures, VertexStage) {
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        void main() {
            gl_Position = texture(video, vec2(0.5));
        }
    )", EShLangVertex, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    EXPECT_NE(wgsl.find("texture_external"), std::string::npos) << wgsl;
    EXPECT_NE(wgsl.find("textureSampleBaseClampToEdge("), std::string::npos) << wgsl;
}

TEST_F(WgslExternalTextures, TexelFetchAndTextureSize) {
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        void main() {
            ivec2 size = textureSize(video, 0);
            color = texelFetch(video, size / 2, 0);
        }
    )", EShLangFragment, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    EXPECT_NE(wgsl.find("texture_external"), std::string::npos) << wgsl;
    EXPECT_NE(wgsl.find("textureLoad("), std::string::npos) << wgsl;
    EXPECT_NE(wgsl.find("textureDimensions("), std::string::npos) << wgsl;
}

TEST_F(WgslExternalTextures, HelperFunctionParameter) {
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        vec4 sampleIt(sampler2D s, vec2 uv) {
            return texture(s, uv);
        }
        void main() {
            color = sampleIt(video, vec2(0.5));
        }
    )", EShLangFragment, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    // The helper is retyped in place: there is no leftover texture_2d version of it. Note that
    // Tint doesn't keep glslang's mangled function names, so functions are counted by signature.
    EXPECT_EQ(countOccurrences(wgsl, "s_image : texture_external"), 1) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "s_image : texture_2d"), 0) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "textureSampleBaseClampToEdge(s_image, "), 1) << wgsl;
}

TEST_F(WgslExternalTextures, HelperFunctionCalledWithBothKinds) {
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        vec4 sampleIt(sampler2D s, vec2 uv) {
            return texture(s, uv);
        }
        void main() {
            color = sampleIt(regular, vec2(0.5)) + sampleIt(video, vec2(0.5));
        }
    )", EShLangFragment, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    // The helper is duplicated: one version for each kind of texture.
    EXPECT_EQ(countOccurrences(wgsl, "s_image : texture_2d<f32>"), 1) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "s_image : texture_external"), 1) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "textureSample(s_image, "), 1) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "textureSampleBaseClampToEdge(s_image, "), 1) << wgsl;
}

TEST_F(WgslExternalTextures, NestedHelperFunctions) {
    std::string wgsl;
    ASSERT_TRUE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        vec4 inner(sampler2D s, vec2 uv) {
            return texture(s, uv);
        }
        vec4 outer(sampler2D s, vec2 uv) {
            return inner(s, uv) * 0.5;
        }
        void main() {
            color = outer(video, vec2(0.5));
        }
    )", EShLangFragment, &wgsl));

    EXPECT_TRUE(isValidWgsl(wgsl));
    // Both helpers are retyped in place.
    EXPECT_EQ(countOccurrences(wgsl, "s_image : texture_external"), 2) << wgsl;
    EXPECT_EQ(countOccurrences(wgsl, "s_image : texture_2d"), 0) << wgsl;
}

TEST_F(WgslExternalTextures, GatherIsRejected) {
    std::string wgsl;
    EXPECT_FALSE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        void main() {
            color = textureGather(video, vec2(0.5));
        }
    )", EShLangFragment, &wgsl));
}

TEST_F(WgslExternalTextures, OffsetIsRejected) {
    std::string wgsl;
    EXPECT_FALSE(compileToWgsl(R"(
        layout(location = 0) out vec4 color;
        void main() {
            color = textureOffset(video, vec2(0.5), ivec2(1, 1));
        }
    )", EShLangFragment, &wgsl));
}

// End-to-end: an external sampler in a material compiled for WebGPU.
class WgslExternalTexturesMaterial : public WgslExternalTextures,
        public testing::WithParamInterface<MaterialBuilder::Optimization> {
};

TEST_P(WgslExternalTexturesMaterial, Builds) {
    MaterialBuilder builder;
    builder.name("ExternalSampler");
    builder.parameter("video", SamplerType::SAMPLER_EXTERNAL);
    builder.parameter("regular", SamplerType::SAMPLER_2D);
    builder.shading(filament::Shading::UNLIT);
    builder.require(filament::VertexAttribute::UV0);
    builder.targetApi(MaterialBuilder::TargetApi::WEBGPU);
    builder.optimization(GetParam());
    builder.material(R"(
        vec4 sampleVideo(sampler2D s, vec2 uv) {
            return texture(s, uv);
        }
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            vec2 uv = getUV0();
            material.baseColor = sampleVideo(materialParams_video, uv)
                    + texture(materialParams_regular, uv);
        }
    )");
    Package const package = builder.build(*mJobSystem);
    EXPECT_TRUE(package.isValid());
}

INSTANTIATE_TEST_SUITE_P(Optimizations, WgslExternalTexturesMaterial,
        testing::Values(MaterialBuilder::Optimization::NONE,
                MaterialBuilder::Optimization::PREPROCESSOR,
                MaterialBuilder::Optimization::PERFORMANCE));

// Builds an unlit feature level 1 material with one external sampler and `regularCount` 2D
// samplers. Filament uses 4 of the 16 fragment samplers for unlit materials, which leaves 12.
bool buildMaterialWithSamplers(utils::JobSystem& jobSystem, MaterialBuilder::TargetApi targetApi,
        size_t regularCount) {
    MaterialBuilder builder;
    builder.name("SamplerBudget");
    builder.parameter("video", SamplerType::SAMPLER_EXTERNAL);
    for (size_t i = 0; i < regularCount; i++) {
        char name[32];
        snprintf(name, sizeof(name), "regular%zu", i);
        builder.parameter(name, SamplerType::SAMPLER_2D);
    }
    builder.shading(filament::Shading::UNLIT);
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_1);
    builder.require(filament::VertexAttribute::UV0);
    builder.targetApi(targetApi);
    builder.material(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = texture(materialParams_video, getUV0());
        }
    )");
    return builder.build(jobSystem).isValid();
}

// WebGPU counts an external texture as 4 sampled textures: 4 + 8 = 12 fits, 4 + 9 doesn't.
TEST_F(WgslExternalTextures, ExternalSamplerUsesFourSlotsWithWebGpu) {
    EXPECT_TRUE(buildMaterialWithSamplers(*mJobSystem, MaterialBuilder::TargetApi::WEBGPU, 8));
    EXPECT_FALSE(buildMaterialWithSamplers(*mJobSystem, MaterialBuilder::TargetApi::WEBGPU, 9));
}

// Other backends count an external sampler as 2 samplers: 2 + 10 = 12 fits, 2 + 11 doesn't.
TEST_F(WgslExternalTextures, ExternalSamplerUsesTwoSlotsWithOpenGl) {
    EXPECT_TRUE(buildMaterialWithSamplers(*mJobSystem, MaterialBuilder::TargetApi::OPENGL, 10));
    EXPECT_FALSE(buildMaterialWithSamplers(*mJobSystem, MaterialBuilder::TargetApi::OPENGL, 11));
}

// The WebGPU cost applies as soon as WebGPU is one of the targets.
TEST_F(WgslExternalTextures, ExternalSamplerUsesFourSlotsWhenWebGpuIsOneOfTheTargets) {
    EXPECT_FALSE(buildMaterialWithSamplers(*mJobSystem,
            MaterialBuilder::TargetApi::OPENGL | MaterialBuilder::TargetApi::WEBGPU, 9));
}

} // anonymous namespace
