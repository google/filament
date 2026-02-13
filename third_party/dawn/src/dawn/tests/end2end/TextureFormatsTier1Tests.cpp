// Copyright 2025 The Dawn & Tint Authors
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

#include <cmath>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class TextureFormatsTier1Test : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TextureFormatsTier1})) {
            requiredFeatures.push_back(wgpu::FeatureName::TextureFormatsTier1);
        }
        return requiredFeatures;
    }

    const char* GetFullScreenQuadVS() {
        return R"(
                @vertex
                fn vs_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                    var positions = array<vec2<f32>, 3>(
                        vec2<f32>(-1.0, -1.0),
                        vec2<f32>( 3.0, -1.0),
                        vec2<f32>( -1.0, 3.0));
                    return vec4<f32>(positions[VertexIndex], 0.0, 1.0);
                }
            )";
    }

    std::string GenerateFragmentShader(const std::vector<float>& srcColorFloats) {
        std::ostringstream fsCodeStream;
        fsCodeStream << R"(
    @fragment
    fn fs_main() -> @location(0) vec4<f32> {
        return vec4<f32>()";

        for (size_t i = 0; i < 4; ++i) {
            if (i < srcColorFloats.size()) {
                fsCodeStream << srcColorFloats[i];
            } else {
                fsCodeStream << (i == 3 ? "1.0" : "0.0");
            }
            if (i < 3) {
                fsCodeStream << ", ";
            }
        }

        fsCodeStream << ");\n"
                     << "}\n";

        return fsCodeStream.str();
    }

    // Function to run a basic render pass drawing a full-screen quad.
    // Returns the finished command buffer for submission.
    wgpu::CommandBuffer RunDrawPass(wgpu::TextureView renderTargetView,
                                    wgpu::TextureFormat renderTargetFormat,
                                    const std::vector<float>& drawColorFloats,
                                    wgpu::MultisampleState multisampleState = {}) {
        std::string fsCode = GenerateFragmentShader(drawColorFloats);
        wgpu::ShaderModule shaderModule =
            utils::CreateShaderModule(device, (GetFullScreenQuadVS() + fsCode).c_str());

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.cFragment.module = shaderModule;
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.cFragment.targetCount = 1;
        pipelineDesc.cTargets[0].format = renderTargetFormat;
        pipelineDesc.multisample = multisampleState;  // Apply provided multisample state
        pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass;
        renderPass.cColorAttachments[0].view = renderTargetView;
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        renderPass.colorAttachmentCount = 1;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
        return encoder.Finish();
    }

    // Function to run a render pass with blending.
    wgpu::CommandBuffer RunBlendPass(wgpu::TextureView renderTargetView,
                                     wgpu::TextureFormat renderTargetFormat,
                                     const std::vector<float>& srcColorFloats,
                                     const std::vector<float>& clearColorFloats) {
        std::string fsCode = GenerateFragmentShader(srcColorFloats);
        wgpu::ShaderModule shaderModule =
            utils::CreateShaderModule(device, (GetFullScreenQuadVS() + fsCode).c_str());

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.cFragment.module = shaderModule;
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.cFragment.targetCount = 1;
        pipelineDesc.cTargets[0].format = renderTargetFormat;
        pipelineDesc.cTargets[0].blend = &pipelineDesc.cBlends[0];

        // Standard alpha blending: SrcAlpha * Src + (1 - SrcAlpha) * Dst
        pipelineDesc.cBlends[0].color.srcFactor = wgpu::BlendFactor::SrcAlpha;
        pipelineDesc.cBlends[0].color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
        pipelineDesc.cBlends[0].color.operation = wgpu::BlendOperation::Add;
        pipelineDesc.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::One;
        pipelineDesc.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::Zero;
        pipelineDesc.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
        pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::All;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass;
        renderPass.cColorAttachments[0].view = renderTargetView;
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        renderPass.cColorAttachments[0].clearValue = {clearColorFloats[0], clearColorFloats[1],
                                                      clearColorFloats[2], clearColorFloats[3]};
        renderPass.colorAttachmentCount = 1;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
        return encoder.Finish();
    }

    // Helper function for calculating expectations and checking pixels.
    void CalculateAndCheckPixels(wgpu::Texture texture,
                                 wgpu::TextureFormat format,
                                 const std::vector<float>& originData) {
        uint32_t componentCount = utils::GetTextureComponentCount(format);

        // Define vectors for expected bounds for each possible format type.
        std::vector<int8_t> expectedLowerBoundSnorm8;
        std::vector<int8_t> expectedUpperBoundSnorm8;
        std::vector<int16_t> expectedLowerBoundSnorm16;
        std::vector<int16_t> expectedUpperBoundSnorm16;
        std::vector<uint16_t> expectedLowerBoundUnorm16;
        std::vector<uint16_t> expectedUpperBoundUnorm16;

        for (uint32_t i = 0; i < componentCount; ++i) {
            float floatComponent = originData[i];
            // FLOAT -> SNORM/UNORM is permitted tolerance of 0.6f ULP. See
            // https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#3.2.3.4%20FLOAT%20-%3E%20SNORM
            float tolerance = 0.6f;

            switch (format) {
                case wgpu::TextureFormat::R8Snorm:
                case wgpu::TextureFormat::RG8Snorm:
                case wgpu::TextureFormat::RGBA8Snorm: {
                    float scaledComponent = floatComponent * 127.0f;
                    int8_t lowerExpectation =
                        utils::ConvertFloatToSnorm8(scaledComponent - tolerance);
                    int8_t upperExpectation =
                        utils::ConvertFloatToSnorm8(scaledComponent + tolerance);
                    expectedLowerBoundSnorm8.push_back(lowerExpectation);
                    expectedUpperBoundSnorm8.push_back(upperExpectation);
                    break;
                }
                case wgpu::TextureFormat::R16Snorm:
                case wgpu::TextureFormat::RG16Snorm:
                case wgpu::TextureFormat::RGBA16Snorm: {
                    float scaledComponent = floatComponent * 32767.0f;
                    int16_t lowerExpectation =
                        utils::ConvertFloatToSnorm16(scaledComponent - tolerance);
                    int16_t upperExpectation =
                        utils::ConvertFloatToSnorm16(scaledComponent + tolerance);
                    expectedLowerBoundSnorm16.push_back(lowerExpectation);
                    expectedUpperBoundSnorm16.push_back(upperExpectation);
                    break;
                }
                case wgpu::TextureFormat::R16Unorm:
                case wgpu::TextureFormat::RG16Unorm:
                case wgpu::TextureFormat::RGBA16Unorm: {
                    float clampedComponent = floatComponent * 65535.0f;
                    uint16_t lowerExpectation =
                        utils::ConvertFloatToUnorm16(clampedComponent - tolerance);
                    uint16_t upperExpectation =
                        utils::ConvertFloatToUnorm16(clampedComponent + tolerance);
                    expectedLowerBoundUnorm16.push_back(lowerExpectation);
                    expectedUpperBoundUnorm16.push_back(upperExpectation);
                    break;
                }
                default:
                    DAWN_UNREACHABLE();
            }
        }

        // Perform the texture pixel validation based on the determined format type.
        // It asserts that the pixel at {0,0} with size {1,1} falls within the calculated bounds.
        switch (format) {
            case wgpu::TextureFormat::R8Snorm:
            case wgpu::TextureFormat::RG8Snorm:
            case wgpu::TextureFormat::RGBA8Snorm:
                EXPECT_TEXTURE_NORM_BETWEEN(expectedLowerBoundSnorm8, expectedUpperBoundSnorm8,
                                            texture, {0, 0}, {1, 1}, format);
                break;
            case wgpu::TextureFormat::R16Snorm:
            case wgpu::TextureFormat::RG16Snorm:
            case wgpu::TextureFormat::RGBA16Snorm:
                EXPECT_TEXTURE_NORM_BETWEEN(expectedLowerBoundSnorm16, expectedUpperBoundSnorm16,
                                            texture, {0, 0}, {1, 1}, format);
                break;
            case wgpu::TextureFormat::R16Unorm:
            case wgpu::TextureFormat::RG16Unorm:
            case wgpu::TextureFormat::RGBA16Unorm:
                EXPECT_TEXTURE_NORM_BETWEEN(expectedLowerBoundUnorm16, expectedUpperBoundUnorm16,
                                            texture, {0, 0}, {1, 1}, format);
                break;
            default:
                DAWN_UNREACHABLE();
        }
    }
};

class RenderAttachmentFormatsTest : public TextureFormatsTier1Test {
  protected:
    void RunRenderTest(wgpu::TextureFormat format, const std::vector<float>& originData) {
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

        wgpu::Extent3D textureSize = {16, 16, 1};
        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.size = textureSize;
        textureDesc.format = format;

        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        wgpu::CommandBuffer drawCommands = RunDrawPass(texture.CreateView(), format, originData);
        queue.Submit(1, &drawCommands);
        CalculateAndCheckPixels(texture, format, originData);
    }
};

// Test that r8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, R8SnormRenderAttachment) {
    std::vector<float> originData = {-0.5f};  // R
    RunRenderTest(wgpu::TextureFormat::R8Snorm, originData);
}

// Test that rg8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, RG8SnormRenderAttachment) {
    std::vector<float> originData = {-0.5f, 0.25f};  // RG
    RunRenderTest(wgpu::TextureFormat::RG8Snorm, originData);
}

// Test that rgba8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, RGBA8SnormRenderAttachment) {
    std::vector<float> originData = {-0.5f, 0.25f, -1.0f, 1.0f};  // RGBA
    RunRenderTest(wgpu::TextureFormat::RGBA8Snorm, originData);
}

// Test that r16unorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, R16UnormRenderAttachment) {
    std::vector<float> originData = {0.375f};
    RunRenderTest(wgpu::TextureFormat::R16Unorm, originData);
}

// Test that r16snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, R16SnormRenderAttachment) {
    std::vector<float> originData = {-0.375f};
    RunRenderTest(wgpu::TextureFormat::R16Snorm, originData);
}

// Test that rg16unorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, RG16UnormRenderAttachment) {
    std::vector<float> originData = {0.375f, 0.75f};
    RunRenderTest(wgpu::TextureFormat::RG16Unorm, originData);
}

// Test that rg16snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, RG16SnormRenderAttachment) {
    std::vector<float> originData = {-0.375f, 0.625f};
    RunRenderTest(wgpu::TextureFormat::RG16Snorm, originData);
}

// Test that rgba16unorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, RGBA16UnormRenderAttachment) {
    std::vector<float> originData = {0.375f, 0.75f, 0.125f, 0.875f};
    RunRenderTest(wgpu::TextureFormat::RGBA16Unorm, originData);
}

// Test that rgba16snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentFormatsTest, RGBA16SnormRenderAttachment) {
    std::vector<float> originData = {-0.375f, 0.625f, -0.875f, 0.5f};
    RunRenderTest(wgpu::TextureFormat::RGBA16Snorm, originData);
}

DAWN_INSTANTIATE_TEST(RenderAttachmentFormatsTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend());

class BlendableFormatsTest : public TextureFormatsTier1Test {
  protected:
    void RunBlendTest(wgpu::TextureFormat format,
                      const std::vector<float>& srcColorFloats,
                      const std::vector<float>& clearColorFloats) {
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

        wgpu::Extent3D textureSize = {16, 16, 1};
        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.size = textureSize;
        textureDesc.format = format;
        wgpu::Texture renderTarget = device.CreateTexture(&textureDesc);
        wgpu::TextureView renderTargetView = renderTarget.CreateView();

        wgpu::CommandBuffer blendCommands =
            RunBlendPass(renderTargetView, format, srcColorFloats, clearColorFloats);
        queue.Submit(1, &blendCommands);

        std::vector<float> expectedBlendedFloats(4);
        float srcR = (srcColorFloats.size() > 0) ? srcColorFloats[0] : 0.0f;
        float srcG = (srcColorFloats.size() > 1) ? srcColorFloats[1] : 0.0f;
        float srcB = (srcColorFloats.size() > 2) ? srcColorFloats[2] : 0.0f;
        float srcA = (srcColorFloats.size() > 3) ? srcColorFloats[3] : 1.0f;

        float dstR = clearColorFloats[0];
        float dstG = clearColorFloats[1];
        float dstB = clearColorFloats[2];

        expectedBlendedFloats[0] = srcR * srcA + dstR * (1.0f - srcA);
        expectedBlendedFloats[1] = srcG * srcA + dstG * (1.0f - srcA);
        expectedBlendedFloats[2] = srcB * srcA + dstB * (1.0f - srcA);
        expectedBlendedFloats[3] = srcA;
        CalculateAndCheckPixels(renderTarget, format, expectedBlendedFloats);
    }
};

// Test that r8snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, R8SnormBlendable) {
    std::vector<float> srcColor = {1.0f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::R8Snorm, srcColor, clearColor);
}

// Test that rg8snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, RG8SnormBlendable) {
    std::vector<float> srcColor = {1.0f, -0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RG8Snorm, srcColor, clearColor);
}

// Test that rgba8snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, RGBA8SnormBlendable) {
    std::vector<float> srcColor = {1.0f, -0.5f, 0.0f, 0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RGBA8Snorm, srcColor, clearColor);
}

// Test that r16unorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, R16UnormBlendable) {
    std::vector<float> srcColor = {1.0f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::R16Unorm, srcColor, clearColor);
}

// Test that r16snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, R16SnormBlendable) {
    std::vector<float> srcColor = {1.0f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::R16Snorm, srcColor, clearColor);
}

// Test that rg16unorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, RG16UnormBlendable) {
    std::vector<float> srcColor = {1.0f, 0.0f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RG16Unorm, srcColor, clearColor);
}

// Test that rg16snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, RG16SnormBlendable) {
    std::vector<float> srcColor = {1.0f, -0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RG16Snorm, srcColor, clearColor);
}

// Test that rgba16unorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, RGBA16UnormBlendable) {
    std::vector<float> srcColor = {1.0f, 0.0f, 0.0f, 0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RGBA16Unorm, srcColor, clearColor);
}

// Test that rgba16snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableFormatsTest, RGBA16SnormBlendable) {
    std::vector<float> srcColor = {1.0f, -0.5f, 0.0f, 0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RGBA16Snorm, srcColor, clearColor);
}

DAWN_INSTANTIATE_TEST(BlendableFormatsTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend());

class MultisampleResolveFormatsTest : public TextureFormatsTier1Test {
  protected:
    static constexpr uint32_t kSize = 16;
    static constexpr uint32_t kMultisampleCount = 4;
    static constexpr uint32_t kSingleSampleCount = 1;

    void RunMultisampleResolveFormatsTest(wgpu::TextureFormat format,
                                          const std::vector<float>& expectedDrawColorFloats) {
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

        std::string fsCode = GenerateFragmentShader(expectedDrawColorFloats);
        wgpu::ShaderModule shaderModule =
            utils::CreateShaderModule(device, (GetFullScreenQuadVS() + fsCode).c_str());
        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.cFragment.module = shaderModule;
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.cFragment.targetCount = 1;
        pipelineDesc.cTargets[0].format = format;
        pipelineDesc.multisample.count = kMultisampleCount;
        pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::TextureDescriptor multisampleColorDesc;
        multisampleColorDesc.usage = wgpu::TextureUsage::RenderAttachment;
        multisampleColorDesc.dimension = wgpu::TextureDimension::e2D;
        multisampleColorDesc.size = {kSize, kSize, 1};
        multisampleColorDesc.format = format;
        multisampleColorDesc.sampleCount = kMultisampleCount;

        wgpu::Texture multisampleColorTexture = device.CreateTexture(&multisampleColorDesc);
        wgpu::TextureView multisampleColorView = multisampleColorTexture.CreateView();

        wgpu::TextureDescriptor resolveTargetDesc;
        resolveTargetDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        resolveTargetDesc.dimension = wgpu::TextureDimension::e2D;
        resolveTargetDesc.size = {kSize, kSize, 1};
        resolveTargetDesc.format = format;
        resolveTargetDesc.sampleCount = kSingleSampleCount;

        wgpu::Texture resolveTargetTexture = device.CreateTexture(&resolveTargetDesc);
        wgpu::TextureView resolveTargetView = resolveTargetTexture.CreateView();

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass;
        renderPass.cColorAttachments[0].view = multisampleColorView;
        renderPass.cColorAttachments[0].resolveTarget = resolveTargetView;
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        renderPass.colorAttachmentCount = 1;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        CalculateAndCheckPixels(resolveTargetTexture, format, expectedDrawColorFloats);
    }
};

// Test that r8snorm format has multisample and resolve capability
// if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleResolveFormatsTest, R8SnormMultisampleResolve) {
    std::vector<float> expectedDrawColor = {1.0f};
    RunMultisampleResolveFormatsTest(wgpu::TextureFormat::R8Snorm, expectedDrawColor);
}

// Test that rg8snorm format has multisample and resolve capability
// if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleResolveFormatsTest, RG8SnormMultisampleResolve) {
    std::vector<float> expectedDrawColor = {1.0f, -0.5f};
    RunMultisampleResolveFormatsTest(wgpu::TextureFormat::RG8Snorm, expectedDrawColor);
}

// Test that rgba8snorm format has multisample and resolve capability
// if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleResolveFormatsTest, RGBA8SnormMultisampleResolve) {
    std::vector<float> expectedDrawColor = {1.0f, -0.5f, -1.0f, 0.5f};
    RunMultisampleResolveFormatsTest(wgpu::TextureFormat::RGBA8Snorm, expectedDrawColor);
}

DAWN_INSTANTIATE_TEST(MultisampleResolveFormatsTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend());

class MultisampleFormatsTest : public TextureFormatsTier1Test {
  protected:
    static constexpr uint32_t kSize = 16;
    static constexpr uint32_t kMultisampleCount = 4;

    // Fragment shader for the second pass: reading multisample texture and writing verification
    // color. This shader outputs red if all samples match the expected color, else green.
    std::string GetMultisampleValidationFS(wgpu::TextureFormat format) {
        std::ostringstream fsCodeStream;
        std::string componentType = "f32";
        std::string textureType = "texture_multisampled_2d<f32>";

        fsCodeStream << R"(
                @group(0) @binding(0) var myMultisampleTexture: )"
                     << textureType << R"(;
                @group(0) @binding(1) var<uniform> expectedColorUniform: vec4<f32>;

                @fragment
                fn fs_main(@builtin(position) fragCoord: vec4<f32>) -> @location(0) vec4<f32> {
                    let texelCoord = vec2<i32>(fragCoord.xy);
                    let numSamples = textureNumSamples(myMultisampleTexture);

                    var allSamplesMatch = true;
                    // Epsilon for floating point comparison of normalized values
                    // A small epsilon like 1/65535 or 1/32767 is appropriate for 16-bit formats.
                    // We will use a slightly larger one to account for potential precision differences.
                    let epsilon = 0.005;

                    for (var i: u32 = 0; i < numSamples; i = i + 1) {
                        let sampleColor = textureLoad(myMultisampleTexture, texelCoord, i);
                        if (abs(sampleColor.r - expectedColorUniform.r) > epsilon ||
                            abs(sampleColor.g - expectedColorUniform.g) > epsilon ||
                            abs(sampleColor.b - expectedColorUniform.b) > epsilon ||
                            abs(sampleColor.a - expectedColorUniform.a) > epsilon) {
                            allSamplesMatch = false;
                            break;
                        }
                    }

                    if (allSamplesMatch) {
                        return vec4<f32>(1.0, 0.0, 0.0, 1.0); // Output Red if all samples match
                    } else {
                        return vec4<f32>(0.0, 1.0, 0.0, 1.0); // Output Green if mismatch
                    }
                }
            )";
        return fsCodeStream.str();
    }

    // This function tests multisampled texture functionality in WebGPU. It renders
    // a color to a multisampled texture, then samples it and validates the color
    // to ensure correct multisampling and format handling.
    void RunMultisampleFormatsTest(wgpu::TextureFormat format,
                                   std::vector<float> expectedDrawColorFloats) {
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

        // Create a multisampled texture that will be drawn to.
        wgpu::TextureDescriptor multisampleColorDesc;
        multisampleColorDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
        multisampleColorDesc.dimension = wgpu::TextureDimension::e2D;
        multisampleColorDesc.size = {kSize, kSize, 1};
        multisampleColorDesc.format = format;
        multisampleColorDesc.sampleCount = kMultisampleCount;

        wgpu::Texture multisampleColorTexture = device.CreateTexture(&multisampleColorDesc);
        wgpu::TextureView multisampleColorView = multisampleColorTexture.CreateView({});

        // This pass draws a full-screen quad with the expected color to the multisampled texture.
        wgpu::CommandBuffer drawCommands =
            RunDrawPass(multisampleColorView, format, expectedDrawColorFloats,
                        {/* nextInChain */ nullptr, kMultisampleCount});
        queue.Submit(1, &drawCommands);

        // Create a single-sampled texture to render the validation result to.
        // This will be checked by EXPECT_PIXEL_RGBA8_EQ.
        wgpu::TextureDescriptor validationTextureDesc;
        validationTextureDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        validationTextureDesc.dimension = wgpu::TextureDimension::e2D;
        validationTextureDesc.size = {kSize, kSize, 1};
        validationTextureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        validationTextureDesc.sampleCount = 1;

        wgpu::Texture validationTexture = device.CreateTexture(&validationTextureDesc);
        wgpu::TextureView validationTextureView = validationTexture.CreateView({});

        std::string validationFsCode = GetMultisampleValidationFS(format);
        wgpu::ShaderModule validationShaderModule =
            utils::CreateShaderModule(device, (GetFullScreenQuadVS() + validationFsCode).c_str());

        // Pad the expectedDrawColorFloats to ensure it's vec4 for the uniform buffer.
        while (expectedDrawColorFloats.size() < 3) {
            expectedDrawColorFloats.push_back(0.0f);
        }
        if (expectedDrawColorFloats.size() < 4) {
            expectedDrawColorFloats.push_back(1.0f);
        }

        // Create uniform buffer to pass expected color to the validation shader.
        wgpu::BufferDescriptor uniformBufferDesc;
        uniformBufferDesc.size = 16;
        uniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer expectedColorBuffer = device.CreateBuffer(&uniformBufferDesc);
        queue.WriteBuffer(expectedColorBuffer, 0, expectedDrawColorFloats.data(), 16);

        wgpu::BindGroupLayout validationBindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::UnfilterableFloat,
                      wgpu::TextureViewDimension::e2D, true},
                     {1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

        wgpu::BindGroup validationBindGroup =
            utils::MakeBindGroup(device, validationBindGroupLayout,
                                 {{0, multisampleColorView}, {1, expectedColorBuffer}});

        // Pipeline for the second pass, which samples the multisampled texture.
        utils::ComboRenderPipelineDescriptor validationPipelineDesc;
        validationPipelineDesc.vertex.module = validationShaderModule;
        validationPipelineDesc.cFragment.module = validationShaderModule;
        validationPipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        validationPipelineDesc.cFragment.targetCount = 1;
        validationPipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        validationPipelineDesc.multisample.count = 1;
        validationPipelineDesc.layout =
            utils::MakePipelineLayout(device, {validationBindGroupLayout});
        validationPipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::All;

        wgpu::RenderPipeline validationPipeline =
            device.CreateRenderPipeline(&validationPipelineDesc);

        // This pass draws to the validationTexture, sampling the content from
        // multisampleColorTexture.
        wgpu::CommandEncoder encoder2 = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass2;
        renderPass2.cColorAttachments[0].view = validationTextureView;
        renderPass2.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass2.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        renderPass2.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        renderPass2.colorAttachmentCount = 1;

        wgpu::RenderPassEncoder pass2 = encoder2.BeginRenderPass(&renderPass2);
        pass2.SetPipeline(validationPipeline);
        pass2.SetBindGroup(0, validationBindGroup);
        pass2.Draw(3);
        pass2.End();

        wgpu::CommandBuffer commands2 = encoder2.Finish();
        queue.Submit(1, &commands2);

        // We expect red, confirming the multisampled texture was correctly drawn to and sampled.
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, validationTexture, 0, 0);
    }
};

// Test that r16unorm format has multisample capability if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleFormatsTest, R16UnormMultisample) {
    RunMultisampleFormatsTest(wgpu::TextureFormat::R16Unorm, {0.5f});
}

// Test that r16snorm format has multisample capability if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleFormatsTest, R16SnormMultisample) {
    RunMultisampleFormatsTest(wgpu::TextureFormat::R16Snorm, {0.25f});
}

// Test that rg16unorm format has multisample capability if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleFormatsTest, RG16UnormMultisample) {
    RunMultisampleFormatsTest(wgpu::TextureFormat::RG16Unorm, {0.25f, 0.75f});
}

// Test that rg16snorm format has multisample capability if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleFormatsTest, RG16SnormMultisample) {
    RunMultisampleFormatsTest(wgpu::TextureFormat::RG16Snorm, {-0.5f, 0.5f});
}

// Test that rgba16unorm format has multisample capability if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleFormatsTest, RGBA16UnormMultisample) {
    RunMultisampleFormatsTest(wgpu::TextureFormat::RGBA16Unorm, {0.1f, 0.4f, 0.7f, 1.0f});
}

// Test that rgba16snorm format has multisample capability if 'texture-formats-tier1' is enabled.
TEST_P(MultisampleFormatsTest, RGBA16SnormMultisample) {
    RunMultisampleFormatsTest(wgpu::TextureFormat::RGBA16Snorm, {-0.8f, -0.2f, 0.3f, 0.9f});
}

DAWN_INSTANTIATE_TEST(MultisampleFormatsTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend());

}  // anonymous namespace
}  // namespace dawn
