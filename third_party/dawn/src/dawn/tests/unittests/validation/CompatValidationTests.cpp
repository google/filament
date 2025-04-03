// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class CompatValidationTest : public ValidationTest {
  protected:
    bool UseCompatibilityMode() const override { return true; }
};

TEST_F(CompatValidationTest, CanNotCreateCubeArrayTextureView) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 6};
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture cubeTexture = device.CreateTexture(&descriptor);

    {
        wgpu::TextureViewDescriptor cubeViewDescriptor;
        cubeViewDescriptor.dimension = wgpu::TextureViewDimension::Cube;
        cubeViewDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;

        cubeTexture.CreateView(&cubeViewDescriptor);
    }

    {
        wgpu::TextureViewDescriptor cubeArrayViewDescriptor;
        cubeArrayViewDescriptor.dimension = wgpu::TextureViewDimension::CubeArray;
        cubeArrayViewDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;

        ASSERT_DEVICE_ERROR(cubeTexture.CreateView(&cubeArrayViewDescriptor));
    }

    cubeTexture.Destroy();
}

TEST_F(CompatValidationTest, CanNotSpecifyAlternateCompatibleViewFormatRGBA8Unorm) {
    constexpr wgpu::TextureFormat viewFormat = wgpu::TextureFormat::RGBA8UnormSrgb;

    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    descriptor.viewFormatCount = 1;
    descriptor.viewFormats = &viewFormat;
    wgpu::Texture texture;
    ASSERT_DEVICE_ERROR(texture = device.CreateTexture(&descriptor),
                        testing::HasSubstr("must match format"));
    texture.Destroy();
}

TEST_F(CompatValidationTest, CanNotSpecifyAlternateCompatibleViewFormatRGBA8UnormSrgb) {
    constexpr wgpu::TextureFormat viewFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8UnormSrgb;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    descriptor.viewFormatCount = 1;
    descriptor.viewFormats = &viewFormat;
    wgpu::Texture texture;
    ASSERT_DEVICE_ERROR(texture = device.CreateTexture(&descriptor),
                        testing::HasSubstr("must match format"));
    texture.Destroy();
}

TEST_F(CompatValidationTest, CanNotCreatePipelineWithDifferentPerTargetBlendStateOrWriteMask) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0);
        }

        struct FragmentOut {
            @location(0) fragColor0 : vec4f,
            @location(1) fragColor1 : vec4f,
            @location(2) fragColor2 : vec4f,
        }

        @fragment fn fs() -> FragmentOut {
            var output : FragmentOut;
            output.fragColor0 = vec4f(0);
            output.fragColor1 = vec4f(0);
            output.fragColor2 = vec4f(0);
            return output;
        }
    )");

    utils::ComboRenderPipelineDescriptor testDescriptor;
    testDescriptor.layout = {};
    testDescriptor.vertex.module = module;
    testDescriptor.cFragment.module = module;
    testDescriptor.cFragment.targetCount = 3;
    testDescriptor.cTargets[1].format = wgpu::TextureFormat::Undefined;

    for (int i = 0; i < 10; ++i) {
        wgpu::BlendState blend0;
        wgpu::BlendState blend2;

        // Blend state intentionally omitted for target 1
        testDescriptor.cTargets[0].blend = &blend0;
        testDescriptor.cTargets[2].blend = &blend2;

        bool expectError = true;
        switch (i) {
            case 0:  // default
                expectError = false;
                break;
            case 1:  // no blend
                testDescriptor.cTargets[0].blend = nullptr;
                break;
            case 2:  // no blend second target
                testDescriptor.cTargets[2].blend = nullptr;
                break;
            case 3:  // color.operation
                blend2.color.operation = wgpu::BlendOperation::Subtract;
                break;
            case 4:  // color.srcFactor
                blend2.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
                break;
            case 5:  // color.dstFactor
                blend2.color.dstFactor = wgpu::BlendFactor::DstAlpha;
                break;
            case 6:  // alpha.operation
                blend2.alpha.operation = wgpu::BlendOperation::Subtract;
                break;
            case 7:  // alpha.srcFactor
                blend2.alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
                break;
            case 8:  // alpha.dstFactor
                blend2.alpha.dstFactor = wgpu::BlendFactor::DstAlpha;
                break;
            case 9:  // writeMask
                testDescriptor.cTargets[2].writeMask = wgpu::ColorWriteMask::Green;
                break;
            default:
                DAWN_UNREACHABLE();
        }

        if (expectError) {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&testDescriptor));
        } else {
            device.CreateRenderPipeline(&testDescriptor);
        }
    }
}

TEST_F(CompatValidationTest, CanNotCreatePipelineWithNonZeroDepthBiasClamp) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0);
        }

        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(0);
        }
    )");

    utils::ComboRenderPipelineDescriptor testDescriptor;
    testDescriptor.layout = {};
    testDescriptor.vertex.module = module;
    testDescriptor.cFragment.module = module;
    testDescriptor.cFragment.targetCount = 1;
    testDescriptor.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::DepthStencilState* depthStencil =
        testDescriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
    depthStencil->depthBias = 0;
    depthStencil->depthBiasSlopeScale = 0;

    depthStencil->depthBiasClamp = 0;
    device.CreateRenderPipeline(&testDescriptor);

    depthStencil->depthBiasClamp = 1;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&testDescriptor));
}

TEST_F(CompatValidationTest, CanNotCreatePipelineWithTextureLoadOfDepthTexture) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<storage, read_write> dstBuf : array<vec4f>;
        @group(0) @binding(1) var tex1 : texture_2d<f32>;
        @group(0) @binding(2) var tex2 : texture_depth_2d;
        @group(0) @binding(3) var tex3 : texture_depth_2d_array;
        @group(0) @binding(4) var tex4 : texture_depth_multisampled_2d;

        @compute @workgroup_size(1) fn main1() {
            dstBuf[0] = textureLoad(tex1, vec2(0), 0);
        }

        @compute @workgroup_size(1) fn main2() {
            dstBuf[0] = vec4f(textureLoad(tex2, vec2(0), 0));
        }

        @compute @workgroup_size(1) fn main3() {
            dstBuf[0] = vec4f(textureLoad(tex3, vec2(0), 0, 0));
        }

        @compute @workgroup_size(1) fn main4() {
            dstBuf[4] = vec4f(textureLoad(tex4, vec2(0), 0));
        }
    )");

    const char* entryPoints[] = {"main1", "main2", "main3", "main4"};
    for (auto entryPoint : entryPoints) {
        wgpu::ComputePipelineDescriptor pDesc;
        pDesc.compute.module = module;
        pDesc.compute.entryPoint = entryPoint;
        if (entryPoint == entryPoints[0]) {
            device.CreateComputePipeline(&pDesc);
        } else {
            ASSERT_DEVICE_ERROR(
                device.CreateComputePipeline(&pDesc),
                testing::HasSubstr(
                    "textureLoad can not be used with depth textures in compatibility mode"));
        }
    }
}

TEST_F(CompatValidationTest, CanNotCreatePipelineWithDepthTextureUsedWithNonComparisonSampler) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(1) @binding(0) var s: sampler;
        @group(1) @binding(1) var sc: sampler_comparison;
        @group(0) @binding(0) var tex2d : texture_depth_2d;
        @group(0) @binding(1) var tex2dArray: texture_depth_2d_array;
        @group(0) @binding(2) var texCube : texture_depth_cube;
        @group(0) @binding(3) var texCubeArray : texture_depth_cube_array;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0);
        }

        // valid
        @fragment fn main0() -> @location(0) vec4f {
            return textureGatherCompare(tex2d, sc, vec2(0), 0) +
                   textureGatherCompare(tex2dArray, sc, vec2(0), 0, 0) +
                   textureGatherCompare(texCube, sc, vec3(0), 0) +
                   textureGatherCompare(texCubeArray, sc, vec3(0), 0, 0) +
                   vec4f(textureSampleCompare(tex2d, sc, vec2(0), 0)) +
                   vec4f(textureSampleCompare(tex2dArray, sc, vec2(0), 0, 0)) +
                   vec4f(textureSampleCompare(texCube, sc, vec3(0), 0)) +
                   vec4f(textureSampleCompare(texCubeArray, sc, vec3(0), 0, 0)) +
                   vec4f(textureSampleCompareLevel(tex2d, sc, vec2(0), 0)) +
                   vec4f(textureSampleCompareLevel(tex2dArray, sc, vec2(0), 0, 0)) +
                   vec4f(textureSampleCompareLevel(texCube, sc, vec3(0), 0)) +
                   vec4f(textureSampleCompareLevel(texCubeArray, sc, vec3(0), 0, 0)) ;
        }

        @fragment fn main1() -> @location(0) vec4f {
            return textureGather(tex2d, s, vec2(0));
        }

        @fragment fn main2() -> @location(0) vec4f {
            return textureGather(tex2dArray, s, vec2(0), 0);
        }

        @fragment fn main3() -> @location(0) vec4f {
            return textureGather(texCube, s, vec3(0));
        }

        @fragment fn main4() -> @location(0) vec4f {
            return textureGather(texCubeArray, s, vec3(0), 0);
        }

        @fragment fn main5() -> @location(0) vec4f {
            return vec4f(textureSample(tex2d, s, vec2(0)));
        }

        @fragment fn main6() -> @location(0) vec4f {
            return vec4f(textureSample(tex2dArray, s, vec2(0), 0));
        }

        @fragment fn main7() -> @location(0) vec4f {
            return vec4f(textureSample(texCube, s, vec3(0)));
        }

        @fragment fn main8() -> @location(0) vec4f {
            return vec4f(textureSample(texCubeArray, s, vec3(0), 0));
        }

        @fragment fn main9() -> @location(0) vec4f {
            return vec4f(textureSampleLevel(tex2d, s, vec2(0), 0));
        }

        @fragment fn main10() -> @location(0) vec4f {
            return vec4f(textureSampleLevel(tex2dArray, s, vec2(0), 0, 0));
        }

        @fragment fn main11() -> @location(0) vec4f {
            return vec4f(textureSampleLevel(texCube, s, vec3(0), 0));
        }

        @fragment fn main12() -> @location(0) vec4f {
            return vec4f(textureSampleLevel(texCubeArray, s, vec3(0), 0, 0));
        }
    )");

    const char* entryPoints[] = {"main0", "main1", "main2", "main3",  "main4",  "main5", "main6",
                                 "main7", "main8", "main9", "main10", "main11", "main12"};
    for (auto entryPoint : entryPoints) {
        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.vertex.module = module;
        pDesc.cFragment.module = module;
        pDesc.cFragment.entryPoint = entryPoint;
        pDesc.cFragment.targetCount = 1;
        pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        if (entryPoint == entryPoints[0]) {
            device.CreateRenderPipeline(&pDesc);
        } else {
            ASSERT_DEVICE_ERROR(
                device.CreateRenderPipeline(&pDesc),
                testing::HasSubstr("texture_depth_xx can not be used with non-comparison samplers "
                                   "in compatibility mode"));
        }
    }
}

TEST_F(CompatValidationTest, CanNotUseTooManyTextureSamplerCombos) {
    wgpu::Limits limits;
    device.GetLimits(&limits);
    uint32_t maxCombos =
        std::min(limits.maxSampledTexturesPerShaderStage, limits.maxSamplersPerShaderStage);

    struct Test {
        bool expectSuccess;
        uint32_t numCombos;
        uint32_t numNonSamplerUsages;
        uint32_t numExternalTextures;
        bool useSameExternalTexture;
        wgpu::ShaderStage stages;
    };
    // clang-format off
    Test comboTests[] = {
        //                     num                 use
        //                     non        num      same
        //                     sampler    external external
        // pass numCombos      uses       textures tex        stage
        {true , maxCombos    , 0        , 0      , false, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment},
        {true , 1            , maxCombos, 0      , false, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment},
        {false, 2            , maxCombos, 0      , false, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment},
        {true , maxCombos - 4, 0        , 1      , false, wgpu::ShaderStage::Vertex},
        {false, maxCombos - 3, 0        , 1      , false, wgpu::ShaderStage::Vertex},
        {true , maxCombos - 8, 0        , 2      , false, wgpu::ShaderStage::Vertex},
        {false, maxCombos - 7, 0        , 2      , false, wgpu::ShaderStage::Vertex},
        {true , maxCombos - 7, 0        , 2      , true,  wgpu::ShaderStage::Vertex},
        {false, maxCombos - 6, 0        , 2      , true,  wgpu::ShaderStage::Vertex},
        {true , maxCombos - 4, 0        , 1      , false, wgpu::ShaderStage::Fragment},
        {false, maxCombos - 3, 0        , 1      , false, wgpu::ShaderStage::Fragment},
        {true , maxCombos - 8, 0        , 2      , false, wgpu::ShaderStage::Fragment},
        {false, maxCombos - 7, 0        , 2      , false, wgpu::ShaderStage::Fragment},
        {true , maxCombos - 7, 0        , 2      , true,  wgpu::ShaderStage::Fragment},
        {false, maxCombos - 6, 0        , 2      , true,  wgpu::ShaderStage::Fragment},
        {false, maxCombos + 1, 0        , 0      , false, wgpu::ShaderStage::Vertex},
        {false, maxCombos + 1, 0        , 0      , false, wgpu::ShaderStage::Fragment},
    };
    // clang-format on
    for (const auto& test : comboTests) {
        uint32_t maxTexturesPerShaderStage =
            limits.maxSampledTexturesPerShaderStage - (test.numExternalTextures * 3);
        auto numCombos = test.numCombos;
        std::vector<std::string> textureDeclarations[2];
        std::vector<std::string> samplerDeclarations[2];
        std::vector<std::string> usages[2];
        for (uint32_t stage = 0; stage < 2; ++stage) {
            uint32_t count = 0;
            for (uint32_t t = 0; count < numCombos && t < maxTexturesPerShaderStage; ++t) {
                textureDeclarations[stage].push_back(
                    absl::StrFormat("@group(%u) @binding(%u) var t%u_%u: texture_2d<f32>;",
                                    stage * 2, t, stage, t));
                for (uint32_t s = 0; count < numCombos && t < limits.maxSamplersPerShaderStage;
                     ++s) {
                    if (t == 0) {
                        samplerDeclarations[stage].push_back(
                            absl::StrFormat("@group(%u) @binding(%u) var s%u_%u: sampler;",
                                            (stage * 2) + 1, s, stage, s));
                    }
                    usages[stage].push_back(
                        absl::StrFormat("c += textureSampleLevel(t%u_%u, s%u_%u, vec2f(0), 0);",
                                        stage, t, stage, s));
                    ++count;
                }
            }

            for (uint32_t t = 0; t < test.numNonSamplerUsages; ++t) {
                if (t >= textureDeclarations[stage].size()) {
                    textureDeclarations[stage].push_back(
                        absl::StrFormat("@group(%u) @binding(%u) var t%u_%u: texture_2d<f32>;",
                                        stage * 2, t, stage, t));
                }
                usages[stage].push_back(
                    absl::StrFormat("c += textureLoad(t%u_%u, vec2u(0), 0);", stage, t));
            }

            for (uint32_t t = 0; t < test.numExternalTextures; ++t) {
                if (t == 0 || !test.useSameExternalTexture) {
                    auto et = textureDeclarations[stage].size() + t;
                    textureDeclarations[stage].push_back(
                        absl::StrFormat("@group(%u) @binding(%u) var e%u_%u: texture_external;",
                                        stage * 2, et, stage, t));
                }
                usages[stage].push_back(
                    absl::StrFormat("c += textureSampleBaseClampToEdge(e%u_%u, s%u_%u, vec2f(0));",
                                    stage, test.useSameExternalTexture ? 0 : t, stage,
                                    test.useSameExternalTexture ? t : 0));
            }
        }

        auto wgsl =
            absl::StrFormat(R"(
%s
%s

%s
%s

fn usage0() -> vec4f {
  var c: vec4f;
  %s
  return c;
}

fn usage1() -> vec4f {
  var c: vec4f;
  %s
  return c;
}

@vertex fn vs() -> @builtin(position) vec4f {
  _ = %s;
  return vec4f(0);
}

@group(2) @binding(0) var tt: texture_2d<f32>;

@fragment fn fs() -> @location(0) vec4f {
  return %s;
}
        )",
                            absl::StrJoin(textureDeclarations[0], "\n"),
                            absl::StrJoin(samplerDeclarations[0], "\n"),
                            absl::StrJoin(textureDeclarations[1], "\n"),
                            absl::StrJoin(samplerDeclarations[1], "\n"),
                            absl::StrJoin(usages[0], "\n  "), absl::StrJoin(usages[1], "\n  "),
                            test.stages & wgpu::ShaderStage::Vertex ? "usage0()" : "vec4f(0)",
                            test.stages & wgpu::ShaderStage::Fragment ? "usage1()" : "vec4f(0)");

        wgpu::ShaderModule module = utils::CreateShaderModule(device, wgsl.c_str());
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = module;
        descriptor.cFragment.module = module;
        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        if (test.expectSuccess) {
            device.CreateRenderPipeline(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                                testing::HasSubstr("compat"));
        }
    }
}

TEST_F(CompatValidationTest, CanNotUseSampleMask) {
    wgpu::ShaderModule moduleSampleMaskOutput = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(1);
        }
        struct Output {
            @builtin(sample_mask) mask_out: u32,
            @location(0) color : vec4f,
        }
        @fragment fn fsWithoutSampleMaskUsage() -> @location(0) vec4f {
            return vec4f(1.0, 1.0, 1.0, 1.0);
        }
        @fragment fn fsWithSampleMaskUsage() -> Output {
            var o: Output;
            // We need to make sure this sample_mask isn't optimized out even its value equals "no op".
            o.mask_out = 0xFFFFFFFFu;
            o.color = vec4f(1.0, 1.0, 1.0, 1.0);
            return o;
        }
    )");

    // Check we can use a fragment shader that doesn't use sample_mask from
    // the same module as one that does.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = moduleSampleMaskOutput;
        descriptor.cFragment.module = moduleSampleMaskOutput;
        descriptor.cFragment.entryPoint = "fsWithoutSampleMaskUsage";
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = false;

        device.CreateRenderPipeline(&descriptor);
    }

    // Check we can not use a fragment shader that uses sample_mask.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = moduleSampleMaskOutput;
        descriptor.cFragment.module = moduleSampleMaskOutput;
        descriptor.cFragment.entryPoint = "fsWithSampleMaskUsage";
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = false;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("sample_mask"));
    }
}

TEST_F(CompatValidationTest, CanNotUseFragmentShaderWithSampleIndex) {
    wgpu::ShaderModule moduleSampleMaskOutput = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(1);
        }
        struct Output {
            @location(0) color : vec4f,
        }
        @fragment fn fsWithoutSampleIndexUsage() -> @location(0) vec4f {
            return vec4f(1.0, 1.0, 1.0, 1.0);
        }
        @fragment fn fsWithSampleIndexUsage(@builtin(sample_index) sNdx: u32) -> Output {
            var o: Output;
            _ = sNdx;
            o.color = vec4f(1.0, 1.0, 1.0, 1.0);
            return o;
        }
    )");

    // Check we can use a fragment shader that doesn't use sample_index from
    // the same module as one that does.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = moduleSampleMaskOutput;
        descriptor.vertex.entryPoint = "vs";
        descriptor.cFragment.module = moduleSampleMaskOutput;
        descriptor.cFragment.entryPoint = "fsWithoutSampleIndexUsage";
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = false;

        device.CreateRenderPipeline(&descriptor);
    }

    // Check we can not use a fragment shader that uses sample_index.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = moduleSampleMaskOutput;
        descriptor.vertex.entryPoint = "vs";
        descriptor.cFragment.module = moduleSampleMaskOutput;
        descriptor.cFragment.entryPoint = "fsWithSampleIndexUsage";
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = false;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("sample_index"));
    }
}

TEST_F(CompatValidationTest, CanNotUseShaderWithUnsupportedInterpolateTypeOrSampling) {
    static const char* interpolateParams[] = {
        "perspective",  // should pass
        "linear",      "perspective, sample", "flat", "flat, first",
    };
    for (auto interpolateParam : interpolateParams) {
        auto wgsl = absl::StrFormat(R"(
            struct Vertex {
                @builtin(position) pos: vec4f,
                @location(0) @interpolate(%s) color : vec4f,
            };
            @vertex fn vs() -> Vertex {
                var v: Vertex;
                v.pos = vec4f(1);
                v.color = vec4f(1);
                return v;
            }
            @fragment fn fsWithoutBadInterpolationUsage() -> @location(0) vec4f {
                return vec4f(1);
            }
            @fragment fn fsWithBadInterpolationUsage1(v: Vertex) -> @location(0) vec4f {
                return vec4f(1);
            }
            @fragment fn fsWithBadInterpolationUsage2(v: Vertex) -> @location(0) vec4f {
                return v.pos;
            }
            @fragment fn fsWithBadInterpolationUsage3(v: Vertex) -> @location(0) vec4f {
                return v.color;
            }
        )",
                                    interpolateParam);
        wgpu::ShaderModule moduleInterpolationLinear =
            utils::CreateShaderModule(device, wgsl.c_str());

        static const char* entryPoints[] = {
            "fsWithoutBadInterpolationUsage",
            "fsWithBadInterpolationUsage1",
            "fsWithBadInterpolationUsage2",
            "fsWithBadInterpolationUsage3",
        };
        for (auto entryPoint : entryPoints) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = moduleInterpolationLinear;
            descriptor.cFragment.module = moduleInterpolationLinear;
            descriptor.cFragment.entryPoint = entryPoint;

            bool shouldSucceed =
                entryPoint == entryPoints[0] || interpolateParam == interpolateParams[0];

            if (shouldSucceed) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                                    testing::HasSubstr("in compatibility mode"));
            }
        }
    }
}

TEST_F(CompatValidationTest, CanNotCreateRGxxxStorageTexture) {
    const wgpu::TextureFormat formats[] = {
        wgpu::TextureFormat::RGBA8Unorm,  // pass check
        wgpu::TextureFormat::RG32Sint,
        wgpu::TextureFormat::RG32Uint,
        wgpu::TextureFormat::RG32Float,
    };
    for (auto format : formats) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {1, 1, 1};
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = format;
        descriptor.usage = wgpu::TextureUsage::StorageBinding;
        wgpu::Texture texture;

        if (format == wgpu::TextureFormat::RGBA8Unorm) {
            texture = device.CreateTexture(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(texture = device.CreateTexture(&descriptor));
        }
        texture.Destroy();
    }
}

TEST_F(CompatValidationTest, CanNotUseStorageBufferInVertexStageWithDefaultLimit0) {
    const wgpu::ShaderStage stages[] = {
        wgpu::ShaderStage::Compute,
        wgpu::ShaderStage::Fragment,
        wgpu::ShaderStage::Vertex,
    };
    const wgpu::BufferBindingType buffer_types[] = {
        wgpu::BufferBindingType::Storage,
        wgpu::BufferBindingType::ReadOnlyStorage,
    };
    for (auto stage : stages) {
        for (auto buffer_type : buffer_types) {
            if (stage == wgpu::ShaderStage::Vertex &&
                buffer_type == wgpu::BufferBindingType::Storage) {
                continue;
            }
            wgpu::BindGroupLayoutEntry entries[1];
            entries[0].binding = 0;
            entries[0].visibility = stage;
            entries[0].buffer.type = buffer_type;

            wgpu::BindGroupLayoutDescriptor descriptor;
            descriptor.entryCount = 1;
            descriptor.entries = entries;

            if (stage != wgpu::ShaderStage::Vertex) {
                wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor),
                                    testing::HasSubstr("maxStorageBuffers"));
            }
        }
    }
}

TEST_F(CompatValidationTest, CanNotUseStorageTexturesInVertexStageWithDefaultLimit0) {
    const wgpu::ShaderStage stages[] = {
        wgpu::ShaderStage::Compute,
        wgpu::ShaderStage::Fragment,
        wgpu::ShaderStage::Vertex,
    };
    const wgpu::StorageTextureAccess access_types[] = {
        wgpu::StorageTextureAccess::ReadOnly,
        wgpu::StorageTextureAccess::ReadWrite,
        wgpu::StorageTextureAccess::WriteOnly,
    };
    for (auto stage : stages) {
        for (auto access : access_types) {
            if (stage == wgpu::ShaderStage::Vertex &&
                (access == wgpu::StorageTextureAccess::ReadWrite ||
                 access == wgpu::StorageTextureAccess::WriteOnly)) {
                continue;
            }
            wgpu::BindGroupLayoutEntry entries[1];
            entries[0].binding = 0;
            entries[0].visibility = stage;
            entries[0].storageTexture.format = wgpu::TextureFormat::R32Float;
            entries[0].storageTexture.access = access;

            wgpu::BindGroupLayoutDescriptor descriptor;
            descriptor.entryCount = 1;
            descriptor.entries = entries;

            if (stage != wgpu::ShaderStage::Vertex) {
                wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor),
                                    testing::HasSubstr("maxStorageTextures"));
            }
        }
    }
}

constexpr const char* kRenderTwoTexturesOneBindgroupWGSL = R"(
    @vertex
    fn vs(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
        var pos = array(
            vec4f(-1,  3, 0, 1),
            vec4f( 3, -1, 0, 1),
            vec4f(-1, -1, 0, 1));
        return pos[VertexIndex];
    }

    @group(0) @binding(0) var tex0 : texture_2d<f32>;
    @group(0) @binding(1) var tex1 : texture_2d<f32>;

    @fragment
    fn fs(@builtin(position) pos: vec4f) -> @location(0) vec4f {
        _ = tex0;
        _ = tex1;
        return vec4f(0);
    }
)";

constexpr const char* kRenderTwoTexturesTwoBindgroupsWGSL = R"(
    @vertex
    fn vs(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
        var pos = array(
            vec4f(-1,  3, 0, 1),
            vec4f( 3, -1, 0, 1),
            vec4f(-1, -1, 0, 1));
        return pos[VertexIndex];
    }

    @group(0) @binding(0) var tex0 : texture_2d<f32>;
    @group(1) @binding(0) var tex1 : texture_2d<f32>;

    @fragment
    fn fs(@builtin(position) pos: vec4f) -> @location(0) vec4f {
        _ = tex0;
        _ = tex1;
        return vec4f(0);
    }
)";

void TestMultipleTextureViewValidationInRenderPass(
    wgpu::Device device,
    const char* wgsl,
    std::function<void(wgpu::Device device,
                       wgpu::Texture texture,
                       wgpu::RenderPipeline pipeline,
                       std::function<void(wgpu::RenderPassEncoder pass)> drawFn)> fn) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {2, 1, 1};
    descriptor.mipLevelCount = 2;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&descriptor);

    constexpr uint32_t indices[] = {0, 1, 2};
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData(device, indices, sizeof indices, wgpu::BufferUsage::Index);

    // Create a pipeline that will sample from 2 2D textures and output to an attachment.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, wgsl);

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    fn(device, texture, pipeline, [](wgpu::RenderPassEncoder pass) { pass.Draw(3); });

    fn(device, texture, pipeline, [indexBuffer](wgpu::RenderPassEncoder pass) {
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(3);
    });

    indexBuffer.Destroy();
    texture.Destroy();
}

enum FlexibleTextureViewsFeature {
    Enabled,
    Disabled,
};

class CompatTextureViewValidationTests
    : public CompatValidationTest,
      public ::testing::WithParamInterface<FlexibleTextureViewsFeature> {
  public:
    static std::string PrintToStringParamName(
        const testing::TestParamInfo<FlexibleTextureViewsFeature>& info) {
        std::ostringstream ss;
        ss << "WithFlexibleTextureViews";
        if (info.param == FlexibleTextureViewsFeature::Enabled) {
            ss << "Enabled";
        } else {
            ss << "Disabled";
        }

        return ss.str();
    }

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (HasFlexibleTextureViews()) {
            return {wgpu::FeatureName::FlexibleTextureViews};
        }

        return {};
    }

    bool HasFlexibleTextureViews() const {
        return GetParam() == FlexibleTextureViewsFeature::Enabled;
    }
};

#define ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(statement, matcher) \
    do {                                                                     \
        if (HasFlexibleTextureViews()) {                                     \
            statement;                                                       \
        } else {                                                             \
            ASSERT_DEVICE_ERROR(statement, matcher);                         \
        }                                                                    \
    } while (0)

// Test we get a validation error if we have 2 different views of a texture
// in the same bind group. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewValidationTests, CanNotDrawDifferentMipsSameTextureSameBindGroup) {
    TestMultipleTextureViewValidationInRenderPass(
        device, kRenderTwoTexturesOneBindgroupWGSL,
        [this](wgpu::Device device, wgpu::Texture texture, wgpu::RenderPipeline pipeline,
               std::function<void(wgpu::RenderPassEncoder pass)> drawFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            wgpu::BindGroup bindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 4, 1);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            drawFn(pass);
            pass.End();

            ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(encoder.Finish(),
                                                             testing::HasSubstr("different views"));
        });
}

// Test we get a validation error if we have 2 different views of a texture spanning
// different bind groups. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewValidationTests, CanNotDrawDifferentMipsSameTextureDifferentBindGroups) {
    TestMultipleTextureViewValidationInRenderPass(
        device, kRenderTwoTexturesTwoBindgroupsWGSL,
        [this](wgpu::Device device, wgpu::Texture texture, wgpu::RenderPipeline pipeline,
               std::function<void(wgpu::RenderPassEncoder pass)> drawFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 4, 1);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            drawFn(pass);
            pass.End();

            ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(encoder.Finish(),
                                                             testing::HasSubstr("different views"));
        });
}

// Test that it's possible to set a bindgroup that uses a texture with multiple views
// which would be an error if you issued a draw command but, you then fix the issue by replacing
// the bindgroup with one that does not have multiple views. We're trying to test
// that the implementation does the validation at draw command time and not before.
TEST_P(CompatTextureViewValidationTests,
       CanBindDifferentMipsSameTextureSameBindGroupAndFixWithoutError) {
    TestMultipleTextureViewValidationInRenderPass(
        device, kRenderTwoTexturesOneBindgroupWGSL,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::RenderPipeline pipeline,
           std::function<void(wgpu::RenderPassEncoder pass)> drawFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            // Bindgroup with different views of same texture
            wgpu::BindGroup badBindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip1ViewDesc)}});

            // Bindgroup with same views of texture
            wgpu::BindGroup goodBindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip0ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 4, 1);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, badBindGroup);
            pass.SetBindGroup(0, goodBindGroup);
            drawFn(pass);
            pass.End();

            // No Error is expected
            encoder.Finish();
        });
}

// Test that having 2 texture views that have the same settings, in 2 different
// bindgroups, does not generate a validation error.
TEST_P(CompatTextureViewValidationTests, CanBindSameViewIn2BindGroups) {
    TestMultipleTextureViewValidationInRenderPass(
        device, kRenderTwoTexturesTwoBindgroupsWGSL,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::RenderPipeline pipeline,
           std::function<void(wgpu::RenderPassEncoder pass)> drawFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;

            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            // Bindgroup with same view of texture as bindGroup0
            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 4, 1);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            drawFn(pass);
            pass.End();

            // No Error is expected
            encoder.Finish();
        });
}

// Test that no validation error happens if we have multiple views of a texture
// but don't draw.
TEST_P(CompatTextureViewValidationTests, NoErrorIfMultipleDifferentViewsOfTextureAreNotUsed) {
    TestMultipleTextureViewValidationInRenderPass(
        device, kRenderTwoTexturesTwoBindgroupsWGSL,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::RenderPipeline pipeline,
           std::function<void(wgpu::RenderPassEncoder pass)> drawFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            // Bindgroup with different views of same texture
            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            // Bindgroup with same views of texture
            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 4, 1);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            pass.End();

            // No Error is expected because draw was never called
            encoder.Finish();
        });
}

constexpr const char* kComputeTwoTexturesOneBindgroupWGSL = R"(
    @group(0) @binding(0) var tex0 : texture_2d<f32>;
    @group(0) @binding(1) var tex1 : texture_2d<f32>;

    @compute @workgroup_size(1)
    fn cs() {
        _ = tex0;
        _ = tex1;
    }
)";

constexpr const char* kComputeTwoTexturesTwoBindgroupsWGSL = R"(
    @group(0) @binding(0) var tex0 : texture_2d<f32>;
    @group(1) @binding(0) var tex1 : texture_2d<f32>;

    @compute @workgroup_size(1)
    fn cs() {
        _ = tex0;
        _ = tex1;
    }
)";

void TestMultipleTextureViewValidationInComputePass(
    wgpu::Device device,
    const char* wgsl,
    wgpu::TextureUsage textureUsage,
    std::function<void(wgpu::Device device,
                       wgpu::Texture texture,
                       wgpu::ComputePipeline pipeline,
                       std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn)> fn) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {2, 1, 1};
    descriptor.mipLevelCount = 2;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = textureUsage;
    wgpu::Texture texture = device.CreateTexture(&descriptor);

    constexpr float indirectData[] = {1, 1, 1};
    wgpu::Buffer indirectBuffer = utils::CreateBufferFromData(
        device, indirectData, sizeof indirectData, wgpu::BufferUsage::Indirect);

    // Create a pipeline that will sample from 2 2D textures and output to an attachment.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, wgsl);

    wgpu::ComputePipelineDescriptor pDesc;
    pDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pDesc);

    fn(device, texture, pipeline,
       [](wgpu::ComputePassEncoder pass) { pass.DispatchWorkgroups(1); });

    fn(device, texture, pipeline, [indirectBuffer](wgpu::ComputePassEncoder pass) {
        pass.DispatchWorkgroupsIndirect(indirectBuffer, 0);
    });

    indirectBuffer.Destroy();
    texture.Destroy();
}

// Test we get a validation error if we have 2 different views of a texture
// in the same bind group. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewValidationTests, CanNotComputeWithDifferentMipsSameTextureSameBindGroup) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoTexturesOneBindgroupWGSL, wgpu::TextureUsage::TextureBinding,
        [this](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
               std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            wgpu::BindGroup bindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            dispatchFn(pass);
            pass.End();

            ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(encoder.Finish(),
                                                             testing::HasSubstr("different views"));
        });
}

// Test we get a validation error if we have 2 different views of a texture spanning
// different bind groups. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewValidationTests,
       CanNotComputeWithDifferentMipsSameTextureDifferentBindGroups) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoTexturesTwoBindgroupsWGSL, wgpu::TextureUsage::TextureBinding,
        [this](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
               std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            dispatchFn(pass);
            pass.End();

            ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(encoder.Finish(),
                                                             testing::HasSubstr("different views"));
        });
}

// Test that it's possible to set a bindgroup that uses a texture with multiple views
// which would be an error if you issued a draw command but, you then fix the issue by replacing
// the bindgroup with one that does not have multiple views. We're trying to test
// that the implementation does the validation at draw command time and not before.
TEST_P(CompatTextureViewValidationTests,
       CanBindDifferentMipsSameTextureSameBindGroupAndFixWithoutErrorInComputePass) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoTexturesOneBindgroupWGSL, wgpu::TextureUsage::TextureBinding,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
           std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            // Bindgroup with different views of same texture
            wgpu::BindGroup badBindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip1ViewDesc)}});

            // Bindgroup with same views of texture
            wgpu::BindGroup goodBindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip0ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, badBindGroup);
            pass.SetBindGroup(0, goodBindGroup);
            dispatchFn(pass);
            pass.End();

            // No Error is expected
            encoder.Finish();
        });
}

// Test that having 2 texture views that have the same settings, in 2 different
// bindgroups, does not generate a validation error.
TEST_P(CompatTextureViewValidationTests, CanBindSameViewIn2BindGroupsInComputePass) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoTexturesTwoBindgroupsWGSL, wgpu::TextureUsage::TextureBinding,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
           std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;

            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            // Bindgroup with same view of texture as bindGroup0
            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            dispatchFn(pass);
            pass.End();

            // No Error is expected
            encoder.Finish();
        });
}

// Test that no validation error happens if we have multiple views of a texture
// but don't draw.
TEST_P(CompatTextureViewValidationTests,
       NoErrorIfMultipleDifferentViewsOfTextureAreNotUsedInComputePass) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoTexturesTwoBindgroupsWGSL, wgpu::TextureUsage::TextureBinding,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
           std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            // Bindgroup with different views of same texture
            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            // Bindgroup with same views of texture
            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            pass.End();

            // No Error is expected because draw was never called
            encoder.Finish();
        });
}

constexpr const char* kComputeTwoStorageTexturesOneBindgroupWGSL = R"(
    @group(0) @binding(0) var tex0 : texture_storage_2d<rgba8unorm, write>;
    @group(0) @binding(1) var tex1 : texture_storage_2d<rgba8unorm, write>;

    @compute @workgroup_size(1)
    fn cs() {
        _ = tex0;
        _ = tex1;
    }
)";

constexpr const char* kComputeTwoStorageTexturesTwoBindgroupsWGSL = R"(
    @group(0) @binding(0) var tex0 : texture_storage_2d<rgba8unorm, write>;
    @group(1) @binding(0) var tex1 : texture_storage_2d<rgba8unorm, write>;

    @compute @workgroup_size(1)
    fn cs() {
        _ = tex0;
        _ = tex1;
    }
)";

// Test we get a validation error if we have 2 different views of a storage texture
// in the same bind group. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewValidationTests,
       CanNotComputeWithDifferentMipsSameStorageTextureSameBindGroup) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoStorageTexturesOneBindgroupWGSL, wgpu::TextureUsage::StorageBinding,
        [this](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
               std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            wgpu::BindGroup bindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            dispatchFn(pass);
            pass.End();

            ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(encoder.Finish(),
                                                             testing::HasSubstr("different views"));
        });
}

// Test we get a validation error if we have 2 different views of a texture spanning
// different bind groups. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewValidationTests,
       CanNotComputeWithDifferentMipsSameStorageTextureDifferentBindGroups) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoStorageTexturesTwoBindgroupsWGSL, wgpu::TextureUsage::StorageBinding,
        [this](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
               std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            dispatchFn(pass);
            pass.End();

            ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(encoder.Finish(),
                                                             testing::HasSubstr("different views"));
        });
}

// Test that it's possible to set a bindgroup that uses a texture with multiple views
// which would be an error if you issued a draw command but, you then fix the issue by replacing
// the bindgroup with one that does not have multiple views. We're trying to test
// that the implementation does the validation at draw command time and not before.
TEST_P(CompatTextureViewValidationTests,
       CanBindDifferentMipsSameStorageTextureSameBindGroupAndFixWithoutErrorInComputePass) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoStorageTexturesOneBindgroupWGSL, wgpu::TextureUsage::StorageBinding,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
           std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.baseMipLevel = 0;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            // Bindgroup with different views of same texture
            wgpu::BindGroup badBindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture.CreateView(&mip1ViewDesc)}});

            wgpu::TextureDescriptor descriptor;
            descriptor.size = {2, 1, 1};
            descriptor.mipLevelCount = 2;
            descriptor.dimension = wgpu::TextureDimension::e2D;
            descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
            descriptor.usage = wgpu::TextureUsage::StorageBinding;
            wgpu::Texture texture2 = device.CreateTexture(&descriptor);

            // Bindgroup with same views of texture
            wgpu::BindGroup goodBindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0),
                {{0, texture.CreateView(&mip0ViewDesc)}, {1, texture2.CreateView(&mip0ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, badBindGroup);
            pass.SetBindGroup(0, goodBindGroup);
            dispatchFn(pass);
            pass.End();

            // No Error is expected
            encoder.Finish();
        });
}

// Test that no validation error happens if we have multiple views of a texture
// but don't draw.
TEST_P(CompatTextureViewValidationTests,
       NoErrorIfMultipleDifferentViewsOfStorageTextureAreNotUsedInComputePass) {
    TestMultipleTextureViewValidationInComputePass(
        device, kComputeTwoStorageTexturesTwoBindgroupsWGSL, wgpu::TextureUsage::StorageBinding,
        [](wgpu::Device device, wgpu::Texture texture, wgpu::ComputePipeline pipeline,
           std::function<void(wgpu::ComputePassEncoder pass)> dispatchFn) {
            wgpu::TextureViewDescriptor mip0ViewDesc;
            mip0ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip0ViewDesc.mipLevelCount = 1;

            wgpu::TextureViewDescriptor mip1ViewDesc;
            mip1ViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            mip1ViewDesc.baseMipLevel = 1;
            mip1ViewDesc.mipLevelCount = 1;

            // Bindgroup with different views of same texture
            wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, texture.CreateView(&mip0ViewDesc)}});

            // Bindgroup with same views of texture
            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(1), {{0, texture.CreateView(&mip1ViewDesc)}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass({});
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup0);
            pass.SetBindGroup(1, bindGroup1);
            pass.End();

            // No Error is expected because draw was never called
            encoder.Finish();
        });
}

TEST_F(CompatValidationTest, CanNotCreateBGRA8UnormSRGBTexture) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::BGRA8UnormSrgb;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor),
                        testing::HasSubstr("not supported in compatibility mode"));
}

TEST_F(CompatValidationTest, CanNotCreateBGRA8UnormTextureWithBGRA8UnormSrgbView) {
    constexpr wgpu::TextureFormat viewFormat = wgpu::TextureFormat::BGRA8UnormSrgb;

    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::BGRA8Unorm;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    descriptor.viewFormatCount = 1;
    descriptor.viewFormats = &viewFormat;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor),
                        testing::HasSubstr("not supported in compatibility mode"));
}

TEST_F(CompatValidationTest, CanNotCopyMultisampleTextureToTexture) {
    wgpu::TextureDescriptor srcDescriptor;
    srcDescriptor.size = {4, 4, 1};
    srcDescriptor.dimension = wgpu::TextureDimension::e2D;
    srcDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    srcDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    srcDescriptor.sampleCount = 4;
    wgpu::Texture srcTexture = device.CreateTexture(&srcDescriptor);

    wgpu::TextureDescriptor dstDescriptor;
    dstDescriptor.size = {4, 4, 1};
    dstDescriptor.dimension = wgpu::TextureDimension::e2D;
    dstDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    dstDescriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    dstDescriptor.sampleCount = 4;
    wgpu::Texture dstTexture = device.CreateTexture(&dstDescriptor);

    wgpu::TexelCopyTextureInfo source = utils::CreateTexelCopyTextureInfo(srcTexture);
    wgpu::TexelCopyTextureInfo destination = utils::CreateTexelCopyTextureInfo(dstTexture);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToTexture(&source, &destination, &srcDescriptor.size);
    ASSERT_DEVICE_ERROR(encoder.Finish(),
                        testing::HasSubstr("cannot be copied in compatibility mode"));
}

// Regression test for crbug.com/339704108
// Error texture should not resolve mCompatibilityTextureBindingViewDimension,
// as dimension could be in bad form.
TEST_P(CompatTextureViewValidationTests,
       DoNotResolveDefaultTextureBindingViewDimensionOnErrorTexture) {
    // Set incompatible texture format and view format.
    // This validation happens before texture dimension validation and binding view dimension
    // resolving and shall return an error texture.
    constexpr wgpu::TextureFormat format = wgpu::TextureFormat::BGRA8Unorm;
    constexpr wgpu::TextureFormat viewFormat = wgpu::TextureFormat::RGBA8UnormSrgb;

    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.dimension = wgpu::TextureDimension::Undefined;
    descriptor.format = format;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    descriptor.viewFormatCount = 1;
    descriptor.viewFormats = &viewFormat;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

// Regression test for crbug.com/341167195
// Resolved default compatibility textureBindingViewDimension should be validated as it may come
// from the TextureBindingViewDimensionDescriptor
TEST_P(CompatTextureViewValidationTests, InvalidTextureBindingViewDimensionDescriptorDescriptor) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.dimension = wgpu::TextureDimension::Undefined;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
    descriptor.nextInChain = &textureBindingViewDimensionDesc;
    // Forcefully set an invalid view dimension.
    textureBindingViewDimensionDesc.textureBindingViewDimension =
        static_cast<wgpu::TextureViewDimension>(99);

    ASSERT_TEXTURE_VIEW_ERROR_IF_NO_FLEXIBLE_FEATURE(device.CreateTexture(&descriptor), testing::_);
}

class CompatTextureViewDimensionValidationTests : public CompatTextureViewValidationTests {
  protected:
    void TestBindingTextureViewDimensions(
        const uint32_t depth,
        const wgpu::TextureViewDimension textureBindingViewDimension,
        const wgpu::TextureViewDimension viewDimension,
        bool success) {
        wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                      viewDimension == wgpu::TextureViewDimension::Undefined
                          ? wgpu::TextureViewDimension::e2D
                          : viewDimension}});

        wgpu::Texture texture = CreateTextureWithViewDimension(depth, wgpu::TextureDimension::e2D,
                                                               textureBindingViewDimension);

        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.dimension = viewDimension;

        if (success) {
            utils::MakeBindGroup(device, layout, {{0, texture.CreateView(&viewDesc)}});
        } else {
            ASSERT_DEVICE_ERROR(
                utils::MakeBindGroup(device, layout, {{0, texture.CreateView(&viewDesc)}}),
                testing::HasSubstr("must match textureBindingViewDimension"));
        }

        texture.Destroy();
    }

    void TestCreateTextureWithViewDimensionImpl(
        const uint32_t depth,
        const wgpu::TextureDimension dimension,
        const wgpu::TextureViewDimension textureBindingViewDimension,
        bool success,
        const char* expectedSubstr) {
        if (success) {
            CreateTextureWithViewDimension(depth, dimension, textureBindingViewDimension);
        } else {
            ASSERT_DEVICE_ERROR(
                CreateTextureWithViewDimension(depth, dimension, textureBindingViewDimension);
                testing::HasSubstr(expectedSubstr));
        }
    }

    void TestCreateTextureIsCompatibleWithViewDimension(
        const uint32_t depth,
        const wgpu::TextureDimension dimension,
        const wgpu::TextureViewDimension textureBindingViewDimension,
        bool success) {
        TestCreateTextureWithViewDimensionImpl(depth, dimension, textureBindingViewDimension,
                                               success, "is not compatible with the dimension");
    }

    void TestCreateTextureLayersIsCompatibleWithViewDimension(
        const uint32_t depth,
        const wgpu::TextureDimension dimension,
        const wgpu::TextureViewDimension textureBindingViewDimension,
        bool success) {
        TestCreateTextureWithViewDimensionImpl(depth, dimension, textureBindingViewDimension,
                                               success,
                                               "is only compatible with depthOrArrayLayers ==");
    }

    wgpu::Texture CreateTextureWithViewDimension(
        const uint32_t depth,
        const wgpu::TextureDimension dimension,
        const wgpu::TextureViewDimension textureBindingViewDimension) {
        constexpr wgpu::TextureFormat viewFormat = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::TextureDescriptor textureDesc;
        textureDesc.size = {1, 1, depth};
        textureDesc.dimension = dimension;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.usage = wgpu::TextureUsage::TextureBinding;
        textureDesc.viewFormatCount = 1;
        textureDesc.viewFormats = &viewFormat;

        wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;

        if (textureBindingViewDimension != wgpu::TextureViewDimension::Undefined) {
            textureDesc.nextInChain = &textureBindingViewDimensionDesc;
            textureBindingViewDimensionDesc.textureBindingViewDimension =
                textureBindingViewDimension;
        }

        return device.CreateTexture(&textureDesc);
    }
};

// Note: CubeArray is not included because CubeArray is not allowed
// in compatibility mode.
const wgpu::TextureViewDimension kViewDimensions[] = {
    wgpu::TextureViewDimension::e1D,  wgpu::TextureViewDimension::e2D,
    wgpu::TextureViewDimension::e3D,  wgpu::TextureViewDimension::e2DArray,
    wgpu::TextureViewDimension::Cube,
};

// Test creating 1d textures with each view dimension. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests, E1D) {
    for (auto viewDimension : kViewDimensions) {
        TestCreateTextureIsCompatibleWithViewDimension(
            1, wgpu::TextureDimension::e1D, viewDimension,
            HasFlexibleTextureViews() || viewDimension == wgpu::TextureViewDimension::e1D);
    }
}

// Test creating 2d textures with each view dimension. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests, E2D) {
    for (auto viewDimension : kViewDimensions) {
        TestCreateTextureIsCompatibleWithViewDimension(
            viewDimension == wgpu::TextureViewDimension::e2D ? 1 : 6, wgpu::TextureDimension::e2D,
            viewDimension,
            HasFlexibleTextureViews() || (viewDimension != wgpu::TextureViewDimension::e1D &&
                                          viewDimension != wgpu::TextureViewDimension::e3D));
    }
}

// Test creating 1d textures with each view dimension. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests, E3D) {
    for (auto viewDimension : kViewDimensions) {
        TestCreateTextureIsCompatibleWithViewDimension(
            1, wgpu::TextureDimension::e3D, viewDimension,
            HasFlexibleTextureViews() || viewDimension == wgpu::TextureViewDimension::e3D);
    }
}

// Test creating a 2d texture with a 2d view and depthOrArrayLayers > 1 fails. Unless
// FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests, E2DViewMoreThan1Layer) {
    TestCreateTextureLayersIsCompatibleWithViewDimension(
        2, wgpu::TextureDimension::e2D, wgpu::TextureViewDimension::e2D, HasFlexibleTextureViews());
}

// Test creating a 2d texture with a cube view with depthOrArrayLayers != 6 fails. Unless
// FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests, CubeViewMoreWhereLayersIsNot6) {
    uint32_t layers[] = {1, 5, 6, 7, 12};
    for (auto numLayers : layers) {
        TestCreateTextureLayersIsCompatibleWithViewDimension(
            numLayers, wgpu::TextureDimension::e2D, wgpu::TextureViewDimension::Cube,
            HasFlexibleTextureViews() || numLayers == 6);
    }
}

TEST_P(CompatTextureViewDimensionValidationTests, OneLayerIs2DView) {
    TestBindingTextureViewDimensions(1, wgpu::TextureViewDimension::Undefined,
                                     wgpu::TextureViewDimension::e2D, true);
}

// Test 2 layer texture gets a 2d-array viewDimension
TEST_P(CompatTextureViewDimensionValidationTests, TwoLayersIs2DArrayView) {
    TestBindingTextureViewDimensions(2, wgpu::TextureViewDimension::Undefined,
                                     wgpu::TextureViewDimension::e2DArray, true);
}

// Test 6 layer texture gets a 2d-array viewDimension
TEST_P(CompatTextureViewDimensionValidationTests, SixLayersIs2DArrayView) {
    TestBindingTextureViewDimensions(6, wgpu::TextureViewDimension::Undefined,
                                     wgpu::TextureViewDimension::e2DArray, true);
}

// Test 2d texture can not be viewed as 2D array. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests, TwoDTextureViewDimensionCanNotBeViewedAs2DArray) {
    TestBindingTextureViewDimensions(1, wgpu::TextureViewDimension::e2D,
                                     wgpu::TextureViewDimension::e2DArray,
                                     HasFlexibleTextureViews());
}

// Test 2d-array texture can not be viewed as cube. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests,
       TwoDArrayTextureViewDimensionCanNotBeViewedAsCube) {
    TestBindingTextureViewDimensions(6, wgpu::TextureViewDimension::e2DArray,
                                     wgpu::TextureViewDimension::Cube, HasFlexibleTextureViews());
}

// Test cube texture can not be viewed as 2d-array. Unless FlexibleTextureViews is enabled.
TEST_P(CompatTextureViewDimensionValidationTests, CubeTextureViewDimensionCanNotBeViewedAs2DArray) {
    TestBindingTextureViewDimensions(6, wgpu::TextureViewDimension::Cube,
                                     wgpu::TextureViewDimension::e2DArray,
                                     HasFlexibleTextureViews());
}

// Test 2Darray != 2d
// Test cube !== 2d
// Test cube !== 2d-array

class CompatCompressedCopyT2BAndCopyT2TValidationTests : public CompatValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures;
        for (TextureInfo textureInfo : textureInfos) {
            if (adapter.HasFeature(textureInfo.feature)) {
                requiredFeatures.push_back(textureInfo.feature);
            }
        }
        return requiredFeatures;
    }

    struct TextureInfo {
        wgpu::FeatureName feature;
        wgpu::TextureFormat format;
    };
    static constexpr TextureInfo textureInfos[] = {
        {
            wgpu::FeatureName::TextureCompressionBC,
            wgpu::TextureFormat::BC2RGBAUnorm,
        },
        {
            wgpu::FeatureName::TextureCompressionETC2,
            wgpu::TextureFormat::ETC2RGB8Unorm,
        },
        {
            wgpu::FeatureName::TextureCompressionASTC,
            wgpu::TextureFormat::ASTC4x4Unorm,
        },
    };
};

TEST_F(CompatCompressedCopyT2BAndCopyT2TValidationTests, CanNotCopyCompressedTextureToBuffer) {
    for (TextureInfo textureInfo : textureInfos) {
        if (!device.HasFeature(textureInfo.feature)) {
            continue;
        }

        wgpu::TextureDescriptor descriptor;
        descriptor.size = {4, 4, 1};
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = textureInfo.format;
        descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = 256 * 4;
        bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

        wgpu::TexelCopyTextureInfo source = utils::CreateTexelCopyTextureInfo(texture);
        wgpu::TexelCopyBufferInfo destination = utils::CreateTexelCopyBufferInfo(buffer, 0, 256, 4);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&source, &destination, &descriptor.size);
        ASSERT_DEVICE_ERROR(encoder.Finish(), testing::HasSubstr("cannot be used"));
    }
}

TEST_F(CompatCompressedCopyT2BAndCopyT2TValidationTests, CanNotCopyCompressedTextureToTexture) {
    for (TextureInfo textureInfo : textureInfos) {
        if (!device.HasFeature(textureInfo.feature)) {
            continue;
        }

        wgpu::TextureDescriptor descriptor;
        descriptor.size = {4, 4, 1};
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = textureInfo.format;
        descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
        wgpu::Texture srcTexture = device.CreateTexture(&descriptor);

        descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
        wgpu::Texture dstTexture = device.CreateTexture(&descriptor);

        wgpu::TexelCopyTextureInfo source = utils::CreateTexelCopyTextureInfo(srcTexture);
        wgpu::TexelCopyTextureInfo destination = utils::CreateTexelCopyTextureInfo(dstTexture);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&source, &destination, &descriptor.size);
        ASSERT_DEVICE_ERROR(encoder.Finish(), testing::HasSubstr("cannot be used"));
    }
}

class CompatMaxVertexAttributesTest : public CompatValidationTest {
  protected:
    void TestMaxVertexAttributes(bool usesVertexIndex, bool usesInstanceIndex) {
        wgpu::Limits limits;
        device.GetLimits(&limits);

        uint32_t maxAttributes = limits.maxVertexAttributes;
        uint32_t numAttributesUsedByBuiltins =
            (usesVertexIndex ? 1 : 0) + (usesInstanceIndex ? 1 : 0);

        TestAttributes(maxAttributes - numAttributesUsedByBuiltins, usesVertexIndex,
                       usesInstanceIndex, true);
        if (usesVertexIndex || usesInstanceIndex) {
            TestAttributes(maxAttributes - numAttributesUsedByBuiltins + 1, usesVertexIndex,
                           usesInstanceIndex, false);
        }
    }

    void TestAttributes(uint32_t numAttributes,
                        bool usesVertexIndex,
                        bool usesInstanceIndex,
                        bool expectSuccess) {
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = {};
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 16;
        descriptor.cBuffers[0].attributeCount = numAttributes;

        for (uint32_t i = 0; i < numAttributes; ++i) {
            inputs.push_back(absl::StrFormat("@location(%u) v%u: vec4f", i, i));
            outputs.push_back(absl::StrFormat("v%u", i));
            descriptor.cAttributes[i].format = wgpu::VertexFormat::Float32x4;
            descriptor.cAttributes[i].shaderLocation = i;
        }

        if (usesVertexIndex) {
            inputs.push_back("@builtin(vertex_index) vNdx: u32");
            outputs.push_back("vec4f(f32(vNdx))");
        }

        if (usesInstanceIndex) {
            inputs.push_back("@builtin(instance_index) iNdx: u32");
            outputs.push_back("vec4f(f32(iNdx))");
        }

        auto wgsl = absl::StrFormat(R"(
            @fragment fn fs() -> @location(0) vec4f {
                return vec4f(1);
            }
            @vertex fn vs(%s) -> @builtin(position) vec4f {
                return %s;
            }
            )",
                                    absl::StrJoin(inputs, ", "), absl::StrJoin(outputs, " + "));

        wgpu::ShaderModule module = utils::CreateShaderModule(device, wgsl.c_str());
        descriptor.vertex.module = module;
        descriptor.cFragment.module = module;

        if (expectSuccess) {
            device.CreateRenderPipeline(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                                testing::HasSubstr("compat"));
        }
    }
};

TEST_F(CompatMaxVertexAttributesTest, CanUseMaxVertexAttributes) {
    TestMaxVertexAttributes(false, false);
}

TEST_F(CompatMaxVertexAttributesTest, VertexIndexTakesAnAttribute) {
    TestMaxVertexAttributes(true, false);
}

TEST_F(CompatMaxVertexAttributesTest, InstanceIndexTakesAnAttribute) {
    TestMaxVertexAttributes(false, true);
}

TEST_F(CompatMaxVertexAttributesTest, VertexAndInstanceIndexEachTakeAnAttribute) {
    TestMaxVertexAttributes(true, true);
}

INSTANTIATE_TEST_SUITE_P(,
                         CompatTextureViewValidationTests,
                         ::testing::Values(FlexibleTextureViewsFeature::Disabled,
                                           FlexibleTextureViewsFeature::Enabled),
                         CompatTextureViewValidationTests::PrintToStringParamName);

INSTANTIATE_TEST_SUITE_P(,
                         CompatTextureViewDimensionValidationTests,
                         ::testing::Values(FlexibleTextureViewsFeature::Disabled,
                                           FlexibleTextureViewsFeature::Enabled),
                         CompatTextureViewValidationTests::PrintToStringParamName);

class CompatLayoutLimitsTests : public CompatValidationTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        wgpu::Limits required = {};
        required.maxStorageBuffersInFragmentStage = supported.maxStorageBuffersInFragmentStage / 2;
        required.maxStorageBuffersInVertexStage = supported.maxStorageBuffersInVertexStage / 2;
        required.maxStorageTexturesInFragmentStage =
            supported.maxStorageTexturesInFragmentStage / 2;
        required.maxStorageTexturesInVertexStage = supported.maxStorageTexturesInVertexStage / 2;
        required.maxStorageBuffersPerShaderStage = supported.maxStorageBuffersPerShaderStage;
        required.maxStorageTexturesPerShaderStage = supported.maxStorageTexturesPerShaderStage;
        return required;
    }

    void DoBindGroupLayoutTest(uint32_t limitInStage,
                               uint32_t limitPerStage,
                               const wgpu::BindGroupLayoutEntry& entry,
                               const char* expectedErrorSubstring) {
        EXPECT_TRUE(limitInStage > 0);
        EXPECT_TRUE(limitInStage < limitPerStage);

        std::vector<wgpu::BindGroupLayoutEntry> entries(limitInStage + 1);
        for (size_t i = 0; i < entries.size(); ++i) {
            entries[i] = entry;
            entries[i].binding = i;
        }

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor),
                            testing::HasSubstr(expectedErrorSubstring));
    }

    void DoPipelineLayoutTest(uint32_t limitInStage,
                              uint32_t limitPerStage,
                              const wgpu::BindGroupLayoutEntry& entry,
                              const char* expectedErrorSubstring) {
        EXPECT_TRUE(limitInStage > 0);
        EXPECT_TRUE(limitInStage < limitPerStage);

        wgpu::BindGroupLayout bgls[2];

        std::vector<wgpu::BindGroupLayoutEntry> entries(limitInStage);
        for (size_t i = 0; i < entries.size(); ++i) {
            entries[i] = entry;
            entries[i].binding = i;
        }

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();
        bgls[0] = device.CreateBindGroupLayout(&descriptor);

        descriptor.entryCount = 1;
        descriptor.entries = entries.data();
        bgls[1] = device.CreateBindGroupLayout(&descriptor);

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {};
        pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
        pipelineLayoutDescriptor.bindGroupLayouts = bgls;

        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&pipelineLayoutDescriptor),
                            testing::HasSubstr(expectedErrorSubstring));
    }
};

// Test that in compat we get an error if we use more than maxStorageBuffersInFragmentStage
// when it's lower than maxStorageBuffersPerShaderStage in createBindGroupLayout
TEST_F(CompatLayoutLimitsTests, CanNotPassLimitOfStorageBuffersInFragmentStageBindGroupLayout) {
    const auto limits = GetSupportedLimits();
    wgpu::BindGroupLayoutEntry entry;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    DoBindGroupLayoutTest(limits.maxStorageBuffersInFragmentStage,
                          limits.maxStorageBuffersPerShaderStage, entry,
                          "maxStorageBuffersInFragmentStage");
}

// Test that in compat we get an error if we use more than maxStorageBuffersInVertexStage
// when it's lower than maxStorageBuffersPerShaderStage in createBindGroupLayout
TEST_F(CompatLayoutLimitsTests, CanNotPassLimitOfStorageBuffersInVertexStageBindGroupLayout) {
    const auto limits = GetSupportedLimits();
    wgpu::BindGroupLayoutEntry entry;
    entry.visibility = wgpu::ShaderStage::Vertex;
    entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    DoBindGroupLayoutTest(limits.maxStorageBuffersInVertexStage,
                          limits.maxStorageBuffersPerShaderStage, entry,
                          "maxStorageBuffersInVertexStage");
}

// Test that in compat we get an error if we use more than maxStorageTexturesInVertexStage
// when it's lower than maxStorageTexturesPerShaderStage in createBindGroupLayout
TEST_F(CompatLayoutLimitsTests, CanNotPassLimitOfStorageTexturesInVertexStageBindGroupLayout) {
    const auto limits = GetSupportedLimits();
    wgpu::BindGroupLayoutEntry entry;
    entry.visibility = wgpu::ShaderStage::Vertex;
    entry.storageTexture.format = wgpu::TextureFormat::R32Float;
    entry.storageTexture.access = wgpu::StorageTextureAccess::ReadOnly;
    DoBindGroupLayoutTest(limits.maxStorageTexturesInVertexStage,
                          limits.maxStorageTexturesPerShaderStage, entry,
                          "maxStorageTexturesInVertexStage");
}

// Test that in compat we get an error if we use more than maxStorageBuffersInFragmentStage
// when it's lower than maxStorageBuffersPerShaderStage in createPipelineLayout
TEST_F(CompatLayoutLimitsTests, CanNotPassLimitOfStorageBuffersInFragmentStagePipelineLayout) {
    const auto limits = GetSupportedLimits();
    wgpu::BindGroupLayoutEntry entry;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    DoPipelineLayoutTest(limits.maxStorageBuffersInFragmentStage,
                         limits.maxStorageBuffersPerShaderStage, entry,
                         "maxStorageBuffersInFragmentStage");
}

// Test that in compat we get an error if we use more than maxStorageBuffersInVertexStage
// when it's lower than maxStorageBuffersPerShaderStage in createPipelineLayout
TEST_F(CompatLayoutLimitsTests, CanNotPassLimitOfStorageBuffersInVertexStagePipelineLayout) {
    const auto limits = GetSupportedLimits();
    wgpu::BindGroupLayoutEntry entry;
    entry.visibility = wgpu::ShaderStage::Vertex;
    entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    DoPipelineLayoutTest(limits.maxStorageBuffersInVertexStage,
                         limits.maxStorageBuffersPerShaderStage, entry,
                         "maxStorageBuffersInVertexStage");
}

// Test that in compat we get an error if we use more than maxStorageTexturesInVertexStage
// when it's lower than maxStorageTexturesPerShaderStage in createPipelineLayout
TEST_F(CompatLayoutLimitsTests, CanNotPassLimitOfStorageTexturesInVertexStagePipelineLayout) {
    const auto limits = GetSupportedLimits();
    wgpu::BindGroupLayoutEntry entry;
    entry.visibility = wgpu::ShaderStage::Vertex;
    entry.storageTexture.format = wgpu::TextureFormat::R32Float;
    entry.storageTexture.access = wgpu::StorageTextureAccess::ReadOnly;
    DoPipelineLayoutTest(limits.maxStorageTexturesInVertexStage,
                         limits.maxStorageTexturesPerShaderStage, entry,
                         "maxStorageTexturesInVertexStage");
}

}  // anonymous namespace
}  // namespace dawn
