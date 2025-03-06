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
#include <array>
#include <cmath>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static unsigned int kRTSize = 1;

class DualSourceBlendTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::DualSourceBlending));

        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
        pipelineLayout = utils::MakePipelineLayout(device, {bindGroupLayout});

        vsModule = utils::CreateShaderModule(device, R"(
                @vertex
                fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                    var pos = array(
                        vec2f(-1.0, -1.0),
                        vec2f(3.0, -1.0),
                        vec2f(-1.0, 3.0));
                    return vec4f(pos[VertexIndex], 0.0, 1.0);
                }
            )");

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        renderPass.renderPassInfo.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::DualSourceBlending})) {
            requiredFeatures.push_back(wgpu::FeatureName::DualSourceBlending);
        }
        return requiredFeatures;
    }

    struct TestParams {
        wgpu::BlendFactor srcBlendFactor;
        wgpu::BlendFactor dstBlendFactor;
        utils::RGBA8 baseColor;
        utils::RGBA8 testColorIndex0;
        utils::RGBA8 testColorIndex1;
    };

    std::array<float, 4> RGBA8ToVec4F32(utils::RGBA8 rgba) {
        return {rgba.r / 255.f, rgba.g / 255.f, rgba.b / 255.f, rgba.a / 255.f};
    }

    std::array<float, 4> ApplyBlendOperation(wgpu::BlendFactor blendFactor,
                                             const std::array<float, 4>& currentBlendColorF32,
                                             const std::array<float, 4>& dstF32,
                                             const std::array<float, 4>& src0F32,
                                             const std::array<float, 4>& src1F32) {
        std::array<float, 4> idealBlendOutputF32;
        // Currently in this test blendComponents are same for both color and alpha so we can
        // compute them together.
        switch (blendFactor) {
            case wgpu::BlendFactor::Zero:
                idealBlendOutputF32 = {};
                break;
            case wgpu::BlendFactor::One:
                idealBlendOutputF32 = currentBlendColorF32;
                break;
            case wgpu::BlendFactor::Src:
                for (uint32_t i = 0; i < idealBlendOutputF32.size(); ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * src0F32[i];
                }
                break;
            case wgpu::BlendFactor::Src1:
                for (uint32_t i = 0; i < idealBlendOutputF32.size(); ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * src1F32[i];
                }
                break;
            case wgpu::BlendFactor::SrcAlpha:
                for (uint32_t i = 0; i < idealBlendOutputF32.size(); ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * src0F32[3];
                }
                break;
            case wgpu::BlendFactor::Src1Alpha:
                for (uint32_t i = 0; i < 4; ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * src1F32[3];
                }
                break;
            case wgpu::BlendFactor::OneMinusSrc:
                for (uint32_t i = 0; i < 4; ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * (1.f - src0F32[i]);
                }
                break;
            case wgpu::BlendFactor::OneMinusSrc1:
                for (uint32_t i = 0; i < 4; ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * (1.f - src1F32[i]);
                }
                break;
            case wgpu::BlendFactor::OneMinusSrcAlpha:
                for (uint32_t i = 0; i < 4; ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * (1.f - src0F32[3]);
                }
                break;
            case wgpu::BlendFactor::OneMinusSrc1Alpha:
                for (uint32_t i = 0; i < 4; ++i) {
                    idealBlendOutputF32[i] = currentBlendColorF32[i] * (1.f - src1F32[3]);
                }
                break;
            default:
                DAWN_UNREACHABLE();
        }
        return idealBlendOutputF32;
    }

    std::array<utils::RGBA8, 2> GetRGBA8ExpectationRange(const TestParams& params) {
        std::array dstF32 = RGBA8ToVec4F32(params.baseColor);
        std::array src0F32 = RGBA8ToVec4F32(params.testColorIndex0);
        std::array src1F32 = RGBA8ToVec4F32(params.testColorIndex1);

        std::array idealBlendSrcOperationOutputF32 =
            ApplyBlendOperation(params.srcBlendFactor, src0F32, dstF32, src0F32, src1F32);
        std::array idealBlendDstOperationOutputF32 =
            ApplyBlendOperation(params.dstBlendFactor, dstF32, dstF32, src0F32, src1F32);

        std::array<utils::RGBA8, 2> rgba8ExpectationRange;
        // In this test the blend operation is always `wgpu::BlendOperation::Add`.
        for (uint32_t i = 0; i < 4; ++i) {
            float idealBlendOperationUnorm8Unquantized =
                (idealBlendSrcOperationOutputF32[i] + idealBlendDstOperationOutputF32[i]) * 255.f +
                0.5f;

            // The float-to-unorm conversion is permitted tolerance of 0.6f ULP (on the integer
            // side). This means that after converting from float to integer scale, any value within
            // 0.6f ULP of a representable target format value is permitted to map to that value.
            // See the chapter "Integer Conversion / FLOAT->UNORM" in D3D SPEC for more details:
            // https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm
            switch (i) {
                case 0:
                    rgba8ExpectationRange[0].r =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized - 0.6f);
                    rgba8ExpectationRange[1].r =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized + 0.6f);
                    break;
                case 1:
                    rgba8ExpectationRange[0].g =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized - 0.6f);
                    rgba8ExpectationRange[1].g =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized + 0.6f);
                    break;
                case 2:
                    rgba8ExpectationRange[0].b =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized - 0.6f);
                    rgba8ExpectationRange[1].b =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized + 0.6f);
                    break;
                case 3:
                    rgba8ExpectationRange[0].a =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized - 0.6f);
                    rgba8ExpectationRange[1].a =
                        static_cast<uint8_t>(idealBlendOperationUnorm8Unquantized + 0.6f);
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
        }

        return rgba8ExpectationRange;
    }

    void RunTest(TestParams params) {
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                enable dual_source_blending;

                struct TestData {
                    color : vec4f,
                    blend : vec4f
                }

                @group(0) @binding(0) var<uniform> testData : TestData;

                struct FragOut {
                  @location(0) @blend_src(0) color : vec4<f32>,
                  @location(0) @blend_src(1) blend : vec4<f32>,
                }

                @fragment fn main() -> FragOut {
                  var output : FragOut;
                  output.color = testData.color;
                  output.blend = testData.blend;
                  return output;
                }
            )");

        wgpu::BlendComponent blendComponent;
        blendComponent.operation = wgpu::BlendOperation::Add;
        blendComponent.srcFactor = params.srcBlendFactor;
        blendComponent.dstFactor = params.dstBlendFactor;

        wgpu::BlendState blend;
        blend.color = blendComponent;
        blend.alpha = blendComponent;

        wgpu::ColorTargetState colorTargetState;
        colorTargetState.blend = &blend;

        utils::ComboRenderPipelineDescriptor baseDescriptor;
        baseDescriptor.layout = pipelineLayout;
        baseDescriptor.vertex.module = vsModule;
        baseDescriptor.cFragment.module = fsModule;
        baseDescriptor.cTargets[0].format = renderPass.colorFormat;

        basePipeline = device.CreateRenderPipeline(&baseDescriptor);

        utils::ComboRenderPipelineDescriptor testDescriptor;
        testDescriptor.layout = pipelineLayout;
        testDescriptor.vertex.module = vsModule;
        testDescriptor.cFragment.module = fsModule;
        testDescriptor.cTargets[0] = colorTargetState;
        testDescriptor.cTargets[0].format = renderPass.colorFormat;

        testPipeline = device.CreateRenderPipeline(&testDescriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            // First use the base pipeline to draw a triangle with no blending
            pass.SetPipeline(basePipeline);
            wgpu::BindGroup baseColors = MakeBindGroupForColors(
                std::array<utils::RGBA8, 2>({{params.baseColor, params.baseColor}}));
            pass.SetBindGroup(0, baseColors);
            pass.Draw(3);

            // Then use the test pipeline to draw the test triangle with blending
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(
                0, MakeBindGroupForColors({params.testColorIndex0, params.testColorIndex1}));
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::array expectationRange = GetRGBA8ExpectationRange(params);
        EXPECT_PIXEL_RGBA8_BETWEEN(expectationRange[0], expectationRange[1], renderPass.color,
                                   kRTSize / 2, kRTSize / 2);
    }

    // Create a bind group to set the colors as a uniform buffer
    wgpu::BindGroup MakeBindGroupForColors(std::array<utils::RGBA8, 2> colors) {
        std::array<float, 16> data;
        for (unsigned int i = 0; i < 2; ++i) {
            data[4 * i + 0] = static_cast<float>(colors[i].r) / 255.f;
            data[4 * i + 1] = static_cast<float>(colors[i].g) / 255.f;
            data[4 * i + 2] = static_cast<float>(colors[i].b) / 255.f;
            data[4 * i + 3] = static_cast<float>(colors[i].a) / 255.f;
        }

        wgpu::Buffer buffer =
            utils::CreateBufferFromData(device, &data, sizeof(data), wgpu::BufferUsage::Uniform);
        return utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0),
                                    {{0, buffer, 0, sizeof(data)}});
    }

    wgpu::PipelineLayout pipelineLayout;
    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline basePipeline;
    wgpu::RenderPipeline testPipeline;
    wgpu::ShaderModule vsModule;
};

// Test that Src and Src1 BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorSrc1) {
    // Test source blend factor with source index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::Src;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 128);
    RunTest(params);

    // Test source blend factor with source index 1
    params.srcBlendFactor = wgpu::BlendFactor::Src1;
    RunTest(params);

    // Test destination blend factor with source index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::Src;
    RunTest(params);

    // Test destination blend factor with source index 1
    params.dstBlendFactor = wgpu::BlendFactor::Src1;
    RunTest(params);
}

// Test that SrcAlpha and SrcAlpha1 BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorSrc1Alpha) {
    // Test source blend factor with source alpha index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::SrcAlpha;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 128);
    RunTest(params);

    // Test source blend factor with source alpha index 1
    params.srcBlendFactor = wgpu::BlendFactor::Src1Alpha;
    RunTest(params);

    // Test destination blend factor with source alpha index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::SrcAlpha;
    RunTest(params);

    // Test destination blend factor with source alpha index 1
    params.dstBlendFactor = wgpu::BlendFactor::Src1Alpha;
    RunTest(params);
}

// Test that OneMinusSrc and OneMinusSrc1 BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorOneMinusSrc1) {
    // Test source blend factor with one minus source index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrc;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 128);
    RunTest(params);

    // Test source blend factor with one minus source index 1
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrc1;
    RunTest(params);

    // Test destination blend factor with one minus source index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrc;
    RunTest(params);

    // Test destination blend factor with one minus source index 1
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrc1;
    RunTest(params);
}

// Test that OneMinusSrcAlpha and OneMinusSrc1Alpha BlendFactors work with dual source blending.
TEST_P(DualSourceBlendTests, BlendFactorOneMinusSrc1Alpha) {
    // Test source blend factor with one minus source alpha index 0
    TestParams params;
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    params.dstBlendFactor = wgpu::BlendFactor::Zero;
    params.baseColor = utils::RGBA8(100, 150, 200, 250);
    params.testColorIndex0 = utils::RGBA8(100, 150, 200, 96);
    params.testColorIndex1 = utils::RGBA8(32, 64, 96, 160);
    RunTest(params);

    // Test source blend factor with one minus source alpha index 1
    params.srcBlendFactor = wgpu::BlendFactor::OneMinusSrc1Alpha;
    RunTest(params);

    // Test destination blend factor with one minus source alpha index 0
    params.srcBlendFactor = wgpu::BlendFactor::Zero;
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    RunTest(params);

    // Test destination blend factor with one minus source alpha index 1
    params.dstBlendFactor = wgpu::BlendFactor::OneMinusSrc1Alpha;
    RunTest(params);
}

DAWN_INSTANTIATE_TEST(DualSourceBlendTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
