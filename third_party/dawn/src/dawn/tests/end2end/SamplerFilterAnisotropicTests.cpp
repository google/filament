// Copyright 2020 The Dawn & Tint Authors
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
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static unsigned int kRTSize = 16;

// MipLevel colors, ordering from base level to high level
// each mipmap of the texture is having a different color
// so we can check if the sampler anisotropic filtering is fetching
// from the correct miplevel
const std::array<utils::RGBA8, 3> colors = {utils::RGBA8::kRed, utils::RGBA8::kGreen,
                                            utils::RGBA8::kBlue};

class SamplerFilterAnisotropicTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            struct Uniforms {
                matrix : mat4x4<f32>
            }

            struct VertexIn {
                @location(0) position : vec4f,
                @location(1) uv : vec2f,
            }

            @group(0) @binding(2) var<uniform> uniforms : Uniforms;

            struct VertexOut {
                @location(0) uv : vec2f,
                @builtin(position) position : vec4f,
            }

            @vertex
            fn main(input : VertexIn) -> VertexOut {
                var output : VertexOut;
                output.uv = input.uv;
                output.position = uniforms.matrix * input.position;
                return output;
            }
        )");
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d<f32>;

            struct FragmentIn {
                @location(0) uv: vec2f,
                @builtin(position) fragCoord : vec4f,
            }

            @fragment
            fn main(input : FragmentIn) -> @location(0) vec4f {
                return textureSample(texture0, sampler0, input.uv);
            })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cBuffers[0].attributeCount = 2;
        pipelineDescriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        pipelineDescriptor.cAttributes[1].shaderLocation = 1;
        pipelineDescriptor.cAttributes[1].offset = 4 * sizeof(float);
        pipelineDescriptor.cAttributes[1].format = wgpu::VertexFormat::Float32x2;
        pipelineDescriptor.vertex.bufferCount = 1;
        pipelineDescriptor.cBuffers[0].arrayStride = 6 * sizeof(float);
        pipelineDescriptor.cTargets[0].format = mRenderPass.colorFormat;

        mPipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        mBindGroupLayout = mPipeline.GetBindGroupLayout(0);

        InitTexture();
    }

    void InitTexture() {
        const uint32_t mipLevelCount = colors.size();

        const uint32_t textureWidthLevel0 = 1 << (mipLevelCount - 1);
        const uint32_t textureHeightLevel0 = 1 << (mipLevelCount - 1);

        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = textureWidthLevel0;
        descriptor.size.height = textureHeightLevel0;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        const uint32_t rowPixels = kTextureBytesPerRowAlignment / sizeof(utils::RGBA8);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Populate each mip level with a different color
        for (uint32_t level = 0; level < mipLevelCount; ++level) {
            const uint32_t texWidth = textureWidthLevel0 >> level;
            const uint32_t texHeight = textureHeightLevel0 >> level;

            const utils::RGBA8 color = colors[level];

            std::vector<utils::RGBA8> data(rowPixels * texHeight, color);
            wgpu::Buffer stagingBuffer =
                utils::CreateBufferFromData(device, data.data(), data.size() * sizeof(utils::RGBA8),
                                            wgpu::BufferUsage::CopySrc);
            wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
                utils::CreateTexelCopyBufferInfo(stagingBuffer, 0, kTextureBytesPerRowAlignment);
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(texture, level, {0, 0, 0});
            wgpu::Extent3D copySize = {texWidth, texHeight, 1};
            encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);
        }
        wgpu::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        mTextureView = texture.CreateView();
    }

    // void TestFilterAnisotropic(const FilterAnisotropicTestCase& testCase) {
    void TestFilterAnisotropic(const uint16_t maxAnisotropy) {
        wgpu::Sampler sampler;
        {
            wgpu::SamplerDescriptor descriptor = {};
            descriptor.minFilter = wgpu::FilterMode::Linear;
            descriptor.magFilter = wgpu::FilterMode::Linear;
            descriptor.mipmapFilter = wgpu::MipmapFilterMode::Linear;
            descriptor.maxAnisotropy = maxAnisotropy;
            sampler = device.CreateSampler(&descriptor);
        }

        // The transform matrix gives us a slanted plane
        // Tweaking happens at: https://jsfiddle.net/t8k7c95o/5/
        // You can get an idea of what the test looks like at the url rendered by webgl
        std::array<float, 16> transform = {-1.7320507764816284,
                                           1.8322050568049563e-16,
                                           -6.176817699518044e-17,
                                           -6.170640314703498e-17,
                                           -2.1211504944260596e-16,
                                           -1.496108889579773,
                                           0.5043753981590271,
                                           0.5038710236549377,
                                           0,
                                           -43.63650894165039,
                                           -43.232173919677734,
                                           -43.18894577026367,
                                           0,
                                           21.693578720092773,
                                           21.789791107177734,
                                           21.86800193786621};
        wgpu::Buffer transformBuffer = utils::CreateBufferFromData(
            device, transform.data(), sizeof(transform), wgpu::BufferUsage::Uniform);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, mBindGroupLayout,
            {{0, sampler}, {1, mTextureView}, {2, transformBuffer, 0, sizeof(transform)}});

        // The plane is scaled on z axis in the transform matrix
        // so uv here is also scaled
        // vertex attribute layout:
        // position : vec4, uv : vec2
        const float vertexData[] = {
            -0.5, 0.5, -0.5, 1, 0, 0,  0.5, 0.5, -0.5, 1, 1, 0, -0.5, 0.5, 0.5, 1, 0, 50,
            -0.5, 0.5, 0.5,  1, 0, 50, 0.5, 0.5, -0.5, 1, 1, 0, 0.5,  0.5, 0.5, 1, 1, 50,
        };
        wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(
            device, vertexData, sizeof(vertexData), wgpu::BufferUsage::Vertex);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&mRenderPass.renderPassInfo);
            pass.SetPipeline(mPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // https://jsfiddle.net/t8k7c95o/5/
        // (x, y) -> (8, [0,15)) full readpixels result on Win10 Nvidia D3D12 GPU
        // maxAnisotropy: 1
        //  0 - 00 00 00
        //  1 - 00 00 ff
        //  2 - 00 00 ff
        //  3 - 00 00 ff
        //  4 - 00 f9 06
        //  5 - 00 f9 06
        //  6 - f2 0d 00
        //  7 - f2 0d 00
        //  8 - ff 00 00
        //  9 - ff 00 00
        // 10 - ff 00 00
        // 11 - ff 00 00
        // 12 - ff 00 00
        // 13 - ff 00 00
        // 14 - ff 00 00
        // 15 - ff 00 00

        // maxAnisotropy: 2
        //  0 - 00 00 00
        //  1 - 00 00 ff
        //  2 - 00 7e 81
        //  3 - 00 7e 81
        //  4 - ff 00 00
        //  5 - ff 00 00
        //  6 - ff 00 00
        //  7 - ff 00 00
        //  8 - ff 00 00
        //  9 - ff 00 00
        // 10 - ff 00 00
        // 11 - ff 00 00
        // 12 - ff 00 00
        // 13 - ff 00 00
        // 14 - ff 00 00
        // 15 - ff 00 00

        // maxAnisotropy: 16
        //  0 - 00 00 00
        //  1 - 00 00 ff
        //  2 - dd 22 00
        //  3 - dd 22 00
        //  4 - ff 00 00
        //  5 - ff 00 00
        //  6 - ff 00 00
        //  7 - ff 00 00
        //  8 - ff 00 00
        //  9 - ff 00 00
        // 10 - ff 00 00
        // 11 - ff 00 00
        // 12 - ff 00 00
        // 13 - ff 00 00
        // 14 - ff 00 00
        // 15 - ff 00 00

        if (maxAnisotropy >= 16) {
            EXPECT_PIXEL_RGBA8_BETWEEN(colors[0], colors[1], mRenderPass.color, 8, 2);
            EXPECT_PIXEL_RGBA8_EQ(colors[0], mRenderPass.color, 8, 6);
        } else if (maxAnisotropy == 2) {
            EXPECT_PIXEL_RGBA8_BETWEEN(colors[1], colors[2], mRenderPass.color, 8, 2);
            EXPECT_PIXEL_RGBA8_EQ(colors[0], mRenderPass.color, 8, 6);
        } else if (maxAnisotropy <= 1) {
            EXPECT_PIXEL_RGBA8_EQ(colors[2], mRenderPass.color, 8, 2);
            EXPECT_PIXEL_RGBA8_BETWEEN(colors[0], colors[1], mRenderPass.color, 8, 6);
        }
    }

    utils::BasicRenderPass mRenderPass;
    wgpu::BindGroupLayout mBindGroupLayout;
    wgpu::RenderPipeline mPipeline;
    wgpu::TextureView mTextureView;
};

TEST_P(SamplerFilterAnisotropicTest, SlantedPlaneMipmap) {
    // TODO(crbug.com/388318201): investigate
    DAWN_SUPPRESS_TEST_IF(IsCompatibilityMode() &&
                          HasToggleEnabled("gl_force_es_31_and_no_extensions"));
    const uint16_t maxAnisotropyLists[] = {1, 2, 16, 128};
    for (uint16_t t : maxAnisotropyLists) {
        TestFilterAnisotropic(t);
    }
}

DAWN_INSTANTIATE_TEST(SamplerFilterAnisotropicTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
