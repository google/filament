// Copyright 2017 The Dawn & Tint Authors
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

constexpr static unsigned int kRTSize = 64;

class ColorStateTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

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
    }

    struct TriangleSpec {
        utils::RGBA8 color;
        std::array<float, 4> blendFactor = {};
    };

    // Set up basePipeline and testPipeline. testPipeline has the given blend state on the first
    // attachment. basePipeline has no blending
    void SetupSingleSourcePipelines(wgpu::ColorTargetState colorTargetState) {
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                struct MyBlock {
                    color : vec4f
                }

                @group(0) @binding(0) var<uniform> myUbo : MyBlock;

                @fragment fn main() -> @location(0) vec4f {
                    return myUbo.color;
                }
            )");

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
    }

    // Create a bind group to set the colors as a uniform buffer
    template <size_t N>
    wgpu::BindGroup MakeBindGroupForColors(std::array<utils::RGBA8, N> colors) {
        std::array<float, 4 * N> data;
        for (unsigned int i = 0; i < N; ++i) {
            data[4 * i + 0] = static_cast<float>(colors[i].r) / 255.f;
            data[4 * i + 1] = static_cast<float>(colors[i].g) / 255.f;
            data[4 * i + 2] = static_cast<float>(colors[i].b) / 255.f;
            data[4 * i + 3] = static_cast<float>(colors[i].a) / 255.f;
        }

        uint32_t bufferSize = static_cast<uint32_t>(4 * N * sizeof(float));

        wgpu::Buffer buffer =
            utils::CreateBufferFromData(device, &data, bufferSize, wgpu::BufferUsage::Uniform);
        return utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0),
                                    {{0, buffer, 0, bufferSize}});
    }

    // Test that after drawing a triangle with the base color, and then the given triangle spec, the
    // color is as expected
    void DoSingleSourceTest(utils::RGBA8 base,
                            const TriangleSpec& triangle,
                            const utils::RGBA8& expected) {
        wgpu::Color blendConstant{triangle.blendFactor[0], triangle.blendFactor[1],
                                  triangle.blendFactor[2], triangle.blendFactor[3]};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            // First use the base pipeline to draw a triangle with no blending
            pass.SetPipeline(basePipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>({{base}})));
            pass.Draw(3);

            // Then use the test pipeline to draw the test triangle with blending
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(
                0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>({{triangle.color}})));
            pass.SetBlendConstant(&blendConstant);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expected, renderPass.color, kRTSize / 2, kRTSize / 2);
    }

    // Given a vector of tests where each element is <testColor, expectedColor>, check that all
    // expectations are true for the given blend operation
    void CheckBlendOperation(utils::RGBA8 base,
                             wgpu::BlendOperation operation,
                             std::vector<std::pair<utils::RGBA8, utils::RGBA8>> tests) {
        wgpu::BlendComponent blendComponent;
        blendComponent.operation = operation;
        blendComponent.srcFactor = wgpu::BlendFactor::One;
        blendComponent.dstFactor = wgpu::BlendFactor::One;

        wgpu::BlendState blend;
        blend.color = blendComponent;
        blend.alpha = blendComponent;

        wgpu::ColorTargetState descriptor;
        descriptor.blend = &blend;
        descriptor.writeMask = wgpu::ColorWriteMask::All;

        SetupSingleSourcePipelines(descriptor);

        for (const auto& [triangleColor, expectedColor] : tests) {
            DoSingleSourceTest(base, {triangleColor}, expectedColor);
        }
    }

    // Given a vector of tests where each element is <testSpec, expectedColor>, check that all
    // expectations are true for the given blend factors
    void CheckBlendFactor(utils::RGBA8 base,
                          wgpu::BlendFactor colorSrcFactor,
                          wgpu::BlendFactor colorDstFactor,
                          wgpu::BlendFactor alphaSrcFactor,
                          wgpu::BlendFactor alphaDstFactor,
                          std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests) {
        wgpu::BlendComponent colorBlend;
        colorBlend.operation = wgpu::BlendOperation::Add;
        colorBlend.srcFactor = colorSrcFactor;
        colorBlend.dstFactor = colorDstFactor;

        wgpu::BlendComponent alphaBlend;
        alphaBlend.operation = wgpu::BlendOperation::Add;
        alphaBlend.srcFactor = alphaSrcFactor;
        alphaBlend.dstFactor = alphaDstFactor;

        wgpu::BlendState blend;
        blend.color = colorBlend;
        blend.alpha = alphaBlend;

        wgpu::ColorTargetState descriptor;
        descriptor.blend = &blend;
        descriptor.writeMask = wgpu::ColorWriteMask::All;

        SetupSingleSourcePipelines(descriptor);

        for (const auto& [triangles, expectedColor] : tests) {
            DoSingleSourceTest(base, triangles, expectedColor);
        }
    }

    void CheckSrcBlendFactor(utils::RGBA8 base,
                             wgpu::BlendFactor colorFactor,
                             wgpu::BlendFactor alphaFactor,
                             std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests) {
        CheckBlendFactor(base, colorFactor, wgpu::BlendFactor::One, alphaFactor,
                         wgpu::BlendFactor::One, tests);
    }

    void CheckDstBlendFactor(utils::RGBA8 base,
                             wgpu::BlendFactor colorFactor,
                             wgpu::BlendFactor alphaFactor,
                             std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests) {
        CheckBlendFactor(base, wgpu::BlendFactor::One, colorFactor, wgpu::BlendFactor::One,
                         alphaFactor, tests);
    }

    wgpu::PipelineLayout pipelineLayout;
    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline basePipeline;
    wgpu::RenderPipeline testPipeline;
    wgpu::ShaderModule vsModule;
};

namespace {
// Add two colors and clamp
constexpr utils::RGBA8 operator+(const utils::RGBA8& col1, const utils::RGBA8& col2) {
    int r = static_cast<int>(col1.r) + static_cast<int>(col2.r);
    int g = static_cast<int>(col1.g) + static_cast<int>(col2.g);
    int b = static_cast<int>(col1.b) + static_cast<int>(col2.b);
    int a = static_cast<int>(col1.a) + static_cast<int>(col2.a);
    r = (r > 255 ? 255 : (r < 0 ? 0 : r));
    g = (g > 255 ? 255 : (g < 0 ? 0 : g));
    b = (b > 255 ? 255 : (b < 0 ? 0 : b));
    a = (a > 255 ? 255 : (a < 0 ? 0 : a));

    return utils::RGBA8(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b),
                        static_cast<uint8_t>(a));
}

// Subtract two colors and clamp
constexpr utils::RGBA8 operator-(const utils::RGBA8& col1, const utils::RGBA8& col2) {
    int r = static_cast<int>(col1.r) - static_cast<int>(col2.r);
    int g = static_cast<int>(col1.g) - static_cast<int>(col2.g);
    int b = static_cast<int>(col1.b) - static_cast<int>(col2.b);
    int a = static_cast<int>(col1.a) - static_cast<int>(col2.a);
    r = (r > 255 ? 255 : (r < 0 ? 0 : r));
    g = (g > 255 ? 255 : (g < 0 ? 0 : g));
    b = (b > 255 ? 255 : (b < 0 ? 0 : b));
    a = (a > 255 ? 255 : (a < 0 ? 0 : a));

    return utils::RGBA8(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b),
                        static_cast<uint8_t>(a));
}

// Get the component-wise minimum of two colors
utils::RGBA8 min(const utils::RGBA8& col1, const utils::RGBA8& col2) {
    return utils::RGBA8(std::min(col1.r, col2.r), std::min(col1.g, col2.g),
                        std::min(col1.b, col2.b), std::min(col1.a, col2.a));
}

// Get the component-wise maximum of two colors
utils::RGBA8 max(const utils::RGBA8& col1, const utils::RGBA8& col2) {
    return utils::RGBA8(std::max(col1.r, col2.r), std::max(col1.g, col2.g),
                        std::max(col1.b, col2.b), std::max(col1.a, col2.a));
}

// Blend two RGBA8 color values parameterized by the provided factors in the range [0.f, 1.f]
utils::RGBA8 mix(const utils::RGBA8& col1, const utils::RGBA8& col2, std::array<float, 4> fac) {
    float r = static_cast<float>(col1.r) * (1.f - fac[0]) + static_cast<float>(col2.r) * fac[0];
    float g = static_cast<float>(col1.g) * (1.f - fac[1]) + static_cast<float>(col2.g) * fac[1];
    float b = static_cast<float>(col1.b) * (1.f - fac[2]) + static_cast<float>(col2.b) * fac[2];
    float a = static_cast<float>(col1.a) * (1.f - fac[3]) + static_cast<float>(col2.a) * fac[3];

    return utils::RGBA8({static_cast<uint8_t>(std::round(r)), static_cast<uint8_t>(std::round(g)),
                         static_cast<uint8_t>(std::round(b)), static_cast<uint8_t>(std::round(a))});
}

// Blend two RGBA8 color values parameterized by the provided RGBA8 factor
utils::RGBA8 mix(const utils::RGBA8& col1, const utils::RGBA8& col2, const utils::RGBA8& fac) {
    std::array<float, 4> f = {{
        static_cast<float>(fac.r) / 255.f,
        static_cast<float>(fac.g) / 255.f,
        static_cast<float>(fac.b) / 255.f,
        static_cast<float>(fac.a) / 255.f,
    }};
    return mix(col1, col2, f);
}

constexpr std::array<utils::RGBA8, 8> kColors = {{
    // check operations over multiple channels
    utils::RGBA8(64, 0, 0, 0),
    utils::RGBA8(0, 64, 0, 0),
    utils::RGBA8(64, 0, 32, 0),
    utils::RGBA8(0, 64, 32, 0),
    utils::RGBA8(128, 0, 128, 128),
    utils::RGBA8(0, 128, 128, 128),

    // check cases that may cause overflow
    utils::RGBA8(0, 0, 0, 0),
    utils::RGBA8(255, 255, 255, 255),
}};
}  // namespace

// Test compilation and usage of the fixture
TEST_P(ColorStateTest, Basic) {
    wgpu::BlendComponent blendComponent;
    // Spot-test for defaulting of these three fields.
    blendComponent.operation = wgpu::BlendOperation::Undefined;  // add
    blendComponent.srcFactor = wgpu::BlendFactor::Undefined;     // one
    blendComponent.dstFactor = wgpu::BlendFactor::Undefined;     // zero

    wgpu::BlendState blend;
    blend.color = blendComponent;
    blend.alpha = blendComponent;

    wgpu::ColorTargetState descriptor;
    descriptor.blend = &blend;
    descriptor.writeMask = wgpu::ColorWriteMask::All;

    SetupSingleSourcePipelines(descriptor);

    DoSingleSourceTest(utils::RGBA8(0, 0, 0, 0), {utils::RGBA8(255, 0, 0, 0)},
                       utils::RGBA8(255, 0, 0, 0));
}

// The following tests check test that the blend operation works
TEST_P(ColorStateTest, BlendOperationAdd) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<utils::RGBA8, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) { return std::make_pair(color, base + color); });
    CheckBlendOperation(base, wgpu::BlendOperation::Add, tests);
}

TEST_P(ColorStateTest, BlendOperationSubtract) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<utils::RGBA8, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) { return std::make_pair(color, color - base); });
    CheckBlendOperation(base, wgpu::BlendOperation::Subtract, tests);
}

TEST_P(ColorStateTest, BlendOperationReverseSubtract) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<utils::RGBA8, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) { return std::make_pair(color, base - color); });
    CheckBlendOperation(base, wgpu::BlendOperation::ReverseSubtract, tests);
}

TEST_P(ColorStateTest, BlendOperationMin) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<utils::RGBA8, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests),
        [&](const utils::RGBA8& color) { return std::make_pair(color, min(base, color)); });
    CheckBlendOperation(base, wgpu::BlendOperation::Min, tests);
}

TEST_P(ColorStateTest, BlendOperationMax) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<utils::RGBA8, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests),
        [&](const utils::RGBA8& color) { return std::make_pair(color, max(base, color)); });
    CheckBlendOperation(base, wgpu::BlendOperation::Max, tests);
}

// The following tests check that the Source blend factor works
TEST_P(ColorStateTest, SrcBlendFactorZero) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests),
        [&](const utils::RGBA8& color) { return std::make_pair(TriangleSpec({{color}}), base); });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::Zero, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorOne) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       return std::make_pair(TriangleSpec({{color}}), base + color);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::One, wgpu::BlendFactor::One, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorSrc) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = color;
                       fac.a = 0;
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::Src, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorOneMinusSrc) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = utils::RGBA8(255, 255, 255, 255) - color;
                       fac.a = 0;
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::OneMinusSrc, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorSrcAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac(color.a, color.a, color.a, color.a);
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::SrcAlpha, wgpu::BlendFactor::SrcAlpha, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorOneMinusSrcAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const utils::RGBA8& color) {
            utils::RGBA8 fac =
                utils::RGBA8(255, 255, 255, 255) - utils::RGBA8(color.a, color.a, color.a, color.a);
            utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
            return std::make_pair(TriangleSpec({{color}}), expected);
        });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::OneMinusSrcAlpha,
                        wgpu::BlendFactor::OneMinusSrcAlpha, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorDst) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = base;
                       fac.a = 0;
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::Dst, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorOneMinusDst) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = utils::RGBA8(255, 255, 255, 255) - base;
                       fac.a = 0;
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::OneMinusDst, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorDstAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac(base.a, base.a, base.a, base.a);
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::DstAlpha, wgpu::BlendFactor::DstAlpha, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorOneMinusDstAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const utils::RGBA8& color) {
            utils::RGBA8 fac =
                utils::RGBA8(255, 255, 255, 255) - utils::RGBA8(base.a, base.a, base.a, base.a);
            utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
            return std::make_pair(TriangleSpec({{color}}), expected);
        });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::OneMinusDstAlpha,
                        wgpu::BlendFactor::OneMinusDstAlpha, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorSrcAlphaSaturated) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       uint8_t f = std::min(color.a, static_cast<uint8_t>(255 - base.a));
                       utils::RGBA8 fac(f, f, f, 255);
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::SrcAlphaSaturated,
                        wgpu::BlendFactor::SrcAlphaSaturated, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorConstant) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       auto triangleSpec = TriangleSpec({{color}, {{0.2f, 0.4f, 0.6f, 0.8f}}});
                       utils::RGBA8 expected =
                           base + mix(utils::RGBA8(0, 0, 0, 0), color, triangleSpec.blendFactor);
                       return std::make_pair(triangleSpec, expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::Constant, wgpu::BlendFactor::Constant, tests);
}

TEST_P(ColorStateTest, SrcBlendFactorOneMinusConstant) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       auto triangleSpec = TriangleSpec({{color}, {{0.2f, 0.4f, 0.6f, 0.8f}}});
                       std::array<float, 4> f = {{0.8f, 0.6f, 0.4f, 0.2f}};
                       utils::RGBA8 expected = base + mix(utils::RGBA8(0, 0, 0, 0), color, f);
                       return std::make_pair(triangleSpec, expected);
                   });
    CheckSrcBlendFactor(base, wgpu::BlendFactor::OneMinusConstant,
                        wgpu::BlendFactor::OneMinusConstant, tests);
}

// The following tests check that the Destination blend factor works
TEST_P(ColorStateTest, DstBlendFactorZero) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests),
        [&](const utils::RGBA8& color) { return std::make_pair(TriangleSpec({{color}}), color); });
    CheckDstBlendFactor(base, wgpu::BlendFactor::Zero, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, DstBlendFactorOne) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       return std::make_pair(TriangleSpec({{color}}), base + color);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::One, wgpu::BlendFactor::One, tests);
}

TEST_P(ColorStateTest, DstBlendFactorSrc) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = color;
                       fac.a = 0;
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::Src, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, DstBlendFactorOneMinusSrc) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = utils::RGBA8(255, 255, 255, 255) - color;
                       fac.a = 0;
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::OneMinusSrc, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, DstBlendFactorSrcAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac(color.a, color.a, color.a, color.a);
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::SrcAlpha, wgpu::BlendFactor::SrcAlpha, tests);
}

TEST_P(ColorStateTest, DstBlendFactorOneMinusSrcAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const utils::RGBA8& color) {
            utils::RGBA8 fac =
                utils::RGBA8(255, 255, 255, 255) - utils::RGBA8(color.a, color.a, color.a, color.a);
            utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
            return std::make_pair(TriangleSpec({{color}}), expected);
        });
    CheckDstBlendFactor(base, wgpu::BlendFactor::OneMinusSrcAlpha,
                        wgpu::BlendFactor::OneMinusSrcAlpha, tests);
}

TEST_P(ColorStateTest, DstBlendFactorDst) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = base;
                       fac.a = 0;
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::Dst, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, DstBlendFactorOneMinusDst) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac = utils::RGBA8(255, 255, 255, 255) - base;
                       fac.a = 0;
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::OneMinusDst, wgpu::BlendFactor::Zero, tests);
}

TEST_P(ColorStateTest, DstBlendFactorDstAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac(base.a, base.a, base.a, base.a);
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::DstAlpha, wgpu::BlendFactor::DstAlpha, tests);
}

TEST_P(ColorStateTest, DstBlendFactorOneMinusDstAlpha) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(
        kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const utils::RGBA8& color) {
            utils::RGBA8 fac =
                utils::RGBA8(255, 255, 255, 255) - utils::RGBA8(base.a, base.a, base.a, base.a);
            utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
            return std::make_pair(TriangleSpec({{color}}), expected);
        });
    CheckDstBlendFactor(base, wgpu::BlendFactor::OneMinusDstAlpha,
                        wgpu::BlendFactor::OneMinusDstAlpha, tests);
}

TEST_P(ColorStateTest, DstBlendFactorSrcAlphaSaturated) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       uint8_t f = std::min(color.a, static_cast<uint8_t>(255 - base.a));
                       utils::RGBA8 fac(f, f, f, 255);
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::SrcAlphaSaturated,
                        wgpu::BlendFactor::SrcAlphaSaturated, tests);
}

TEST_P(ColorStateTest, DstBlendFactorConstant) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       auto triangleSpec = TriangleSpec({{color}, {{0.2f, 0.4f, 0.6f, 0.8f}}});
                       utils::RGBA8 expected =
                           color + mix(utils::RGBA8(0, 0, 0, 0), base, triangleSpec.blendFactor);
                       return std::make_pair(triangleSpec, expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::Constant, wgpu::BlendFactor::Constant, tests);
}

TEST_P(ColorStateTest, DstBlendFactorOneMinusConstant) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       auto triangleSpec = TriangleSpec({{color}, {{0.2f, 0.4f, 0.6f, 0.8f}}});
                       std::array<float, 4> f = {{0.8f, 0.6f, 0.4f, 0.2f}};
                       utils::RGBA8 expected = color + mix(utils::RGBA8(0, 0, 0, 0), base, f);
                       return std::make_pair(triangleSpec, expected);
                   });
    CheckDstBlendFactor(base, wgpu::BlendFactor::OneMinusConstant,
                        wgpu::BlendFactor::OneMinusConstant, tests);
}

// Check that the color write mask works
TEST_P(ColorStateTest, ColorWriteMask) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    wgpu::BlendComponent blendComponent;
    blendComponent.operation = wgpu::BlendOperation::Add;
    blendComponent.srcFactor = wgpu::BlendFactor::One;
    blendComponent.dstFactor = wgpu::BlendFactor::One;

    wgpu::BlendState blend;
    blend.color = blendComponent;
    blend.alpha = blendComponent;

    wgpu::ColorTargetState descriptor;
    descriptor.blend = &blend;
    {
        // Test single channel color write
        descriptor.writeMask = wgpu::ColorWriteMask::Red;
        SetupSingleSourcePipelines(descriptor);

        utils::RGBA8 base(32, 64, 128, 192);
        for (auto& color : kColors) {
            utils::RGBA8 expected = base + utils::RGBA8(color.r, 0, 0, 0);
            DoSingleSourceTest(base, {color}, expected);
        }
    }

    {
        // Test multi channel color write
        descriptor.writeMask = wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Alpha;
        SetupSingleSourcePipelines(descriptor);

        utils::RGBA8 base(32, 64, 128, 192);
        for (auto& color : kColors) {
            utils::RGBA8 expected = base + utils::RGBA8(0, color.g, 0, color.a);
            DoSingleSourceTest(base, {color}, expected);
        }
    }

    {
        // Test no channel color write
        descriptor.writeMask = wgpu::ColorWriteMask::None;
        SetupSingleSourcePipelines(descriptor);

        utils::RGBA8 base(32, 64, 128, 192);
        for (auto& color : kColors) {
            DoSingleSourceTest(base, {color}, base);
        }
    }
}

// Check that the color write mask works when blending is disabled
TEST_P(ColorStateTest, ColorWriteMaskBlendingDisabled) {
    {
        wgpu::BlendComponent blendComponent;
        blendComponent.operation = wgpu::BlendOperation::Add;
        blendComponent.srcFactor = wgpu::BlendFactor::One;
        blendComponent.dstFactor = wgpu::BlendFactor::Zero;

        wgpu::BlendState blend;
        blend.color = blendComponent;
        blend.alpha = blendComponent;

        wgpu::ColorTargetState descriptor;
        descriptor.blend = &blend;
        descriptor.writeMask = wgpu::ColorWriteMask::Red;
        SetupSingleSourcePipelines(descriptor);

        utils::RGBA8 base(32, 64, 128, 192);
        utils::RGBA8 expected(32, 0, 0, 0);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>({{base}})));
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        EXPECT_PIXEL_RGBA8_EQ(expected, renderPass.color, kRTSize / 2, kRTSize / 2);
    }
}

// Test that independent color states on render targets works
TEST_P(ColorStateTest, IndependentColorState) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    // Compatibility mode doesn't support per-draw-buffer blending
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_indexed_draw_buffers"));

    std::array<wgpu::Texture, 4> renderTargets;
    std::array<wgpu::TextureView, 4> renderTargetViews;

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kRTSize;
    descriptor.size.height = kRTSize;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

    for (uint32_t i = 0; i < 4; ++i) {
        renderTargets[i] = device.CreateTexture(&descriptor);
        renderTargetViews[i] = renderTargets[i].CreateView();
    }

    utils::ComboRenderPassDescriptor renderPass(
        {renderTargetViews[0], renderTargetViews[1], renderTargetViews[2], renderTargetViews[3]});

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct MyBlock {
            color0 : vec4f,
            color1 : vec4f,
            color2 : vec4f,
            color3 : vec4f,
        }

        @group(0) @binding(0) var<uniform> myUbo : MyBlock;

        struct FragmentOut {
            @location(0) fragColor0 : vec4f,
            @location(1) fragColor1 : vec4f,
            @location(2) fragColor2 : vec4f,
            @location(3) fragColor3 : vec4f,
        }

        @fragment fn main() -> FragmentOut {
            var output : FragmentOut;
            output.fragColor0 = myUbo.color0;
            output.fragColor1 = myUbo.color1;
            output.fragColor2 = myUbo.color2;
            output.fragColor3 = myUbo.color3;
            return output;
        }
    )");

    utils::ComboRenderPipelineDescriptor baseDescriptor;
    baseDescriptor.layout = pipelineLayout;
    baseDescriptor.vertex.module = vsModule;
    baseDescriptor.cFragment.module = fsModule;
    baseDescriptor.cFragment.targetCount = 4;

    basePipeline = device.CreateRenderPipeline(&baseDescriptor);

    utils::ComboRenderPipelineDescriptor testDescriptor;
    testDescriptor.layout = pipelineLayout;
    testDescriptor.vertex.module = vsModule;
    testDescriptor.cFragment.module = fsModule;
    testDescriptor.cFragment.targetCount = 4;

    // set color states
    wgpu::BlendComponent blendComponent0;
    blendComponent0.operation = wgpu::BlendOperation::Add;
    blendComponent0.srcFactor = wgpu::BlendFactor::One;
    blendComponent0.dstFactor = wgpu::BlendFactor::One;

    wgpu::BlendState blend0;
    blend0.color = blendComponent0;
    blend0.alpha = blendComponent0;

    wgpu::BlendComponent blendComponent1;
    blendComponent1.operation = wgpu::BlendOperation::Subtract;
    blendComponent1.srcFactor = wgpu::BlendFactor::One;
    blendComponent1.dstFactor = wgpu::BlendFactor::One;

    wgpu::BlendState blend1;
    blend1.color = blendComponent1;
    blend1.alpha = blendComponent1;

    // Blend state intentionally omitted for target 2

    wgpu::BlendComponent blendComponent3;
    blendComponent3.operation = wgpu::BlendOperation::Min;
    blendComponent3.srcFactor = wgpu::BlendFactor::One;
    blendComponent3.dstFactor = wgpu::BlendFactor::One;

    wgpu::BlendState blend3;
    blend3.color = blendComponent3;
    blend3.alpha = blendComponent3;

    testDescriptor.cTargets[0].blend = &blend0;
    testDescriptor.cTargets[1].blend = &blend1;
    testDescriptor.cTargets[3].blend = &blend3;

    testPipeline = device.CreateRenderPipeline(&testDescriptor);

    for (unsigned int c = 0; c < kColors.size(); ++c) {
        utils::RGBA8 base = kColors[((c + 31) * 29) % kColors.size()];
        utils::RGBA8 color0 = kColors[((c + 19) * 13) % kColors.size()];
        utils::RGBA8 color1 = kColors[((c + 11) * 43) % kColors.size()];
        utils::RGBA8 color2 = kColors[((c + 7) * 3) % kColors.size()];
        utils::RGBA8 color3 = kColors[((c + 13) * 71) % kColors.size()];

        utils::RGBA8 expected0 = color0 + base;
        utils::RGBA8 expected1 = color1 - base;
        utils::RGBA8 expected2 = color2;
        utils::RGBA8 expected3 = min(color3, base);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(basePipeline);
            pass.SetBindGroup(
                0, MakeBindGroupForColors(std::array<utils::RGBA8, 4>({{base, base, base, base}})));
            pass.Draw(3);

            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 4>(
                                     {{color0, color1, color2, color3}})));
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expected0, renderTargets[0], kRTSize / 2, kRTSize / 2)
            << "Attachment slot 0 should have been " << color0 << " + " << base << " = "
            << expected0;
        EXPECT_PIXEL_RGBA8_EQ(expected1, renderTargets[1], kRTSize / 2, kRTSize / 2)
            << "Attachment slot 1 should have been " << color1 << " - " << base << " = "
            << expected1;
        EXPECT_PIXEL_RGBA8_EQ(expected2, renderTargets[2], kRTSize / 2, kRTSize / 2)
            << "Attachment slot 2 should have been " << color2 << " = " << expected2
            << "(no blending)";
        EXPECT_PIXEL_RGBA8_EQ(expected3, renderTargets[3], kRTSize / 2, kRTSize / 2)
            << "Attachment slot 3 should have been min(" << color3 << ", " << base
            << ") = " << expected3;
    }
}

// Test that the default blend color is correctly set at the beginning of every subpass
TEST_P(ColorStateTest, DefaultBlendColor) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct MyBlock {
            color : vec4f
        }

        @group(0) @binding(0) var<uniform> myUbo : MyBlock;

        @fragment fn main() -> @location(0) vec4f {
            return myUbo.color;
        }
    )");

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
    testDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::BlendComponent blendComponent;
    blendComponent.operation = wgpu::BlendOperation::Add;
    blendComponent.srcFactor = wgpu::BlendFactor::Constant;
    blendComponent.dstFactor = wgpu::BlendFactor::One;

    wgpu::BlendState blend;
    blend.color = blendComponent;
    blend.alpha = blendComponent;

    testDescriptor.cTargets[0].blend = &blend;

    testPipeline = device.CreateRenderPipeline(&testDescriptor);
    constexpr wgpu::Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};

    // Check that the initial blend color is (0,0,0,0)
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(basePipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(
                                     std::array<utils::RGBA8, 1>({{utils::RGBA8(0, 0, 0, 0)}})));
            pass.Draw(3);
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>(
                                     {{utils::RGBA8(255, 255, 255, 255)}})));
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 0, 0, 0), renderPass.color, kRTSize / 2, kRTSize / 2);
    }

    // Check that setting the blend color works
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(basePipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(
                                     std::array<utils::RGBA8, 1>({{utils::RGBA8(0, 0, 0, 0)}})));
            pass.Draw(3);
            pass.SetPipeline(testPipeline);
            pass.SetBlendConstant(&kWhite);
            pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>(
                                     {{utils::RGBA8(255, 255, 255, 255)}})));
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(255, 255, 255, 255), renderPass.color, kRTSize / 2,
                              kRTSize / 2);
    }

    // Check that the blend color is not inherited between render passes
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(basePipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(
                                     std::array<utils::RGBA8, 1>({{utils::RGBA8(0, 0, 0, 0)}})));
            pass.Draw(3);
            pass.SetPipeline(testPipeline);
            pass.SetBlendConstant(&kWhite);
            pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>(
                                     {{utils::RGBA8(255, 255, 255, 255)}})));
            pass.Draw(3);
            pass.End();
        }
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(basePipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(
                                     std::array<utils::RGBA8, 1>({{utils::RGBA8(0, 0, 0, 0)}})));
            pass.Draw(3);
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>(
                                     {{utils::RGBA8(255, 255, 255, 255)}})));
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 0, 0, 0), renderPass.color, kRTSize / 2, kRTSize / 2);
    }
}

// This tests a problem in the OpenGL backend where a previous color write mask
// persisted and prevented a render pass loadOp from fully clearing the output
// attachment.
TEST_P(ColorStateTest, ColorWriteMaskDoesNotAffectRenderPassLoadOpClear) {
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct MyBlock {
            color : vec4f
        }

        @group(0) @binding(0) var<uniform> myUbo : MyBlock;

        @fragment fn main() -> @location(0) vec4f {
            return myUbo.color;
        }
    )");

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
    testDescriptor.cTargets[0].format = renderPass.colorFormat;
    testDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::Red;

    testPipeline = device.CreateRenderPipeline(&testDescriptor);

    utils::RGBA8 base(32, 64, 128, 192);
    utils::RGBA8 expected(0, 0, 0, 0);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        // Clear the render attachment to |base|
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(basePipeline);
        pass.SetBindGroup(0, MakeBindGroupForColors(std::array<utils::RGBA8, 1>({{base}})));
        pass.Draw(3);

        // Set a pipeline that will dirty the color write mask
        pass.SetPipeline(testPipeline);
        pass.End();
    }
    {
        // This renderpass' loadOp should clear all channels of the render attachment
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(expected, renderPass.color, kRTSize / 2, kRTSize / 2);
}

TEST_P(ColorStateTest, SparseAttachmentsDifferentColorMask) {
    // Compatibility mode doesn't support per-draw-buffer color mask
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_indexed_draw_buffers"));

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct Outputs {
            @location(1) o1 : vec4f,
            @location(3) o3 : vec4f,
        }

        @fragment fn main() -> Outputs {
            return Outputs(vec4f(1.0), vec4f(0.0, 1.0, 1.0, 1.0));
        }
    )");

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = vsModule;
    pipelineDesc.cFragment.module = fsModule;
    pipelineDesc.cFragment.targetCount = 4;
    pipelineDesc.cTargets[0].format = wgpu::TextureFormat::Undefined;
    pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
    pipelineDesc.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;
    pipelineDesc.cTargets[2].format = wgpu::TextureFormat::Undefined;
    pipelineDesc.cTargets[2].writeMask = wgpu::ColorWriteMask::None;
    pipelineDesc.cTargets[3].format = wgpu::TextureFormat::RGBA8Unorm;
    pipelineDesc.cTargets[3].writeMask = wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Alpha;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::TextureDescriptor texDesc;
    texDesc.dimension = wgpu::TextureDimension::e2D;
    texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    texDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    texDesc.size = {1, 1};
    wgpu::Texture attachment1 = device.CreateTexture(&texDesc);
    wgpu::Texture attachment3 = device.CreateTexture(&texDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassColorAttachment colorAttachments[4]{};
        colorAttachments[0].view = nullptr;
        colorAttachments[1].view = attachment1.CreateView();
        colorAttachments[1].loadOp = wgpu::LoadOp::Load;
        colorAttachments[1].storeOp = wgpu::StoreOp::Store;
        colorAttachments[2].view = nullptr;
        colorAttachments[3].view = attachment3.CreateView();
        colorAttachments[3].loadOp = wgpu::LoadOp::Load;
        colorAttachments[3].storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor rpDesc;
        rpDesc.colorAttachmentCount = 4;
        rpDesc.colorAttachments = colorAttachments;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, attachment1, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, attachment3, 0, 0);
}

// This is a regression test against an Intel driver issue about using DstAlpha as
// SrcBlendFactor for both color and alpha blend factors.
TEST_P(ColorStateTest, SrcBlendFactorDstAlphaDstBlendFactorZero) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    utils::RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, utils::RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests),
                   [&](const utils::RGBA8& color) {
                       utils::RGBA8 fac(base.a, base.a, base.a, base.a);
                       utils::RGBA8 expected = mix(utils::RGBA8(0, 0, 0, 0), color, fac);
                       return std::make_pair(TriangleSpec({{color}}), expected);
                   });
    CheckBlendFactor(base, wgpu::BlendFactor::DstAlpha, wgpu::BlendFactor::Zero,
                     wgpu::BlendFactor::DstAlpha, wgpu::BlendFactor::Zero, tests);
}

DAWN_INSTANTIATE_TEST(ColorStateTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
