// Copyright 2018 The Dawn & Tint Authors
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

#include <array>
#include <cmath>
#include <vector>

#include "dawn/tests/DawnTest.h"

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static unsigned int kRTSize = 64;

const char* kBasicFS = R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d<f32>;

            @fragment
            fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
                return textureSample(texture0, sampler0, FragCoord.xy / vec2(2.0, 2.0));
            })";

const char* kPassThroughUserFunctionsFS = R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d<f32>;

            fn foo(t : texture_2d<f32>, s : sampler, FragCoord : vec4f) -> vec4f {
                return textureSample(t, s, FragCoord.xy / vec2(2.0, 2.0));
            }

            @fragment
            fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
                return foo(texture0, sampler0, FragCoord);
            })";

struct AddressModeTestCase {
    wgpu::AddressMode mMode;
    uint8_t mExpected2;
    uint8_t mExpected3;
};
AddressModeTestCase addressModes[] = {
    {
        wgpu::AddressMode::Repeat,
        0,
        255,
    },
    {
        wgpu::AddressMode::MirrorRepeat,
        255,
        0,
    },
    {
        wgpu::AddressMode::ClampToEdge,
        255,
        255,
    },
};

class SamplerTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = 2;
        descriptor.size.height = 2;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        // Create a 2x2 checkerboard texture, with black in the top left and bottom right corners.
        const uint32_t rowPixels = kTextureBytesPerRowAlignment / sizeof(utils::RGBA8);
        std::array<utils::RGBA8, rowPixels * 2> pixels;
        pixels[0] = pixels[rowPixels + 1] = utils::RGBA8::kBlack;
        pixels[1] = pixels[rowPixels] = utils::RGBA8::kWhite;

        wgpu::Buffer stagingBuffer =
            utils::CreateBufferFromData(device, pixels.data(), pixels.size() * sizeof(utils::RGBA8),
                                        wgpu::BufferUsage::CopySrc);
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(stagingBuffer, 0, 256);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::Extent3D copySize = {2, 2, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);

        wgpu::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        mTextureView = texture.CreateView();
    }

    // Initializes the pipeline used by tests. Uses `bgl` to set the pipeline
    // layout if it is non-null; otherwise the pipeline is constructed with the
    // default layout.
    void InitShaders(const char* frag_shader, wgpu::BindGroupLayout bgl = nullptr) {
        auto vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-2.0, -2.0),
                    vec2f(-2.0,  2.0),
                    vec2f( 2.0, -2.0),
                    vec2f(-2.0,  2.0),
                    vec2f( 2.0, -2.0),
                    vec2f( 2.0,  2.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            }
        )");
        auto fsModule = utils::CreateShaderModule(device, frag_shader);

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        if (bgl) {
            wgpu::PipelineLayout pl = utils::MakePipelineLayout(device, {bgl});
            pipelineDescriptor.layout = pl;
        }

        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].format = mRenderPass.colorFormat;

        mPipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    }

    // Creates a sampler with the given address modes.
    wgpu::Sampler CreateSampler(AddressModeTestCase u,
                                AddressModeTestCase v,
                                AddressModeTestCase w) {
        wgpu::Sampler sampler;
        wgpu::SamplerDescriptor descriptor = {};
        descriptor.minFilter = wgpu::FilterMode::Nearest;
        descriptor.magFilter = wgpu::FilterMode::Nearest;
        descriptor.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
        descriptor.addressModeU = u.mMode;
        descriptor.addressModeV = v.mMode;
        descriptor.addressModeW = w.mMode;
        return device.CreateSampler(&descriptor);
    }

    // Creates a bind group that has a sampler with the given address modes and
    // `mTextureView` as the texture to be sampled.
    wgpu::BindGroup CreateBindGroup(AddressModeTestCase u,
                                    AddressModeTestCase v,
                                    AddressModeTestCase w) {
        wgpu::Sampler sampler = CreateSampler(u, v, w);
        return utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0),
                                    {{0, sampler}, {1, mTextureView}});
    }

    // Tests drawing with the given address modes and bind group (if non-null).
    // The pipeline must already have been configured. If the bind group is
    // null, creates a bind group that has a sampler with the given address
    // modes. If non-null, the bind group must be compatible with the configured
    // pipeline.
    void TestAddressModes(AddressModeTestCase u,
                          AddressModeTestCase v,
                          AddressModeTestCase w,
                          wgpu::BindGroup bindGroup = nullptr) {
        if (!bindGroup) {
            bindGroup = CreateBindGroup(u, v, w);
        }

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&mRenderPass.renderPassInfo);
            pass.SetPipeline(mPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        utils::RGBA8 expectedU2(u.mExpected2, u.mExpected2, u.mExpected2, 255);
        utils::RGBA8 expectedU3(u.mExpected3, u.mExpected3, u.mExpected3, 255);
        utils::RGBA8 expectedV2(v.mExpected2, v.mExpected2, v.mExpected2, 255);
        utils::RGBA8 expectedV3(v.mExpected3, v.mExpected3, v.mExpected3, 255);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlack, mRenderPass.color, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, mRenderPass.color, 0, 1);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, mRenderPass.color, 1, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlack, mRenderPass.color, 1, 1);
        EXPECT_PIXEL_RGBA8_EQ(expectedU2, mRenderPass.color, 2, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedU3, mRenderPass.color, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedV2, mRenderPass.color, 0, 2);
        EXPECT_PIXEL_RGBA8_EQ(expectedV3, mRenderPass.color, 0, 3);
    }

    utils::BasicRenderPass mRenderPass;
    wgpu::RenderPipeline mPipeline;
    wgpu::TextureView mTextureView;
};

// Test drawing a rect with a checkerboard texture with different address modes.
TEST_P(SamplerTest, AddressMode) {
    InitShaders(kBasicFS);
    for (auto u : addressModes) {
        for (auto v : addressModes) {
            for (auto w : addressModes) {
                TestAddressModes(u, v, w);
            }
        }
    }
}

// Test that passing texture and sampler objects through user-defined functions works correctly.
TEST_P(SamplerTest, PassThroughUserFunctionParameters) {
    InitShaders(kPassThroughUserFunctionsFS);
    for (auto u : addressModes) {
        for (auto v : addressModes) {
            for (auto w : addressModes) {
                TestAddressModes(u, v, w);
            }
        }
    }
}

DAWN_INSTANTIATE_TEST(SamplerTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class StaticSamplerTest : public SamplerTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::StaticSamplers})) {
            requiredFeatures.push_back(wgpu::FeatureName::StaticSamplers);
        }
        return requiredFeatures;
    }

    void SetUp() override {
        SamplerTest::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::StaticSamplers}));
    }

    // Creates a bind group layout with a static sampler with the given address
    // modes as well as the texture to be sampled.
    wgpu::BindGroupLayout CreateBindGroupLayoutWithStaticSampler(AddressModeTestCase u,
                                                                 AddressModeTestCase v,
                                                                 AddressModeTestCase w) {
        wgpu::Sampler sampler = CreateSampler(u, v, w);
        std::vector<wgpu::BindGroupLayoutEntry> entries;

        wgpu::BindGroupLayoutEntry binding = {};
        binding.binding = 0;
        binding.visibility = wgpu::ShaderStage::Fragment;
        wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
        staticSamplerBinding.sampler = sampler;
        binding.nextInChain = &staticSamplerBinding;
        entries.push_back(binding);

        wgpu::BindGroupLayoutEntry binding1 = {};
        binding1.binding = 1;
        binding1.visibility = wgpu::ShaderStage::Fragment;
        binding1.texture.sampleType = wgpu::TextureSampleType::Float;
        entries.push_back(binding1);

        wgpu::BindGroupLayoutDescriptor desc = {};
        desc.entryCount = 2;
        desc.entries = entries.data();

        return device.CreateBindGroupLayout(&desc);
    }

    // Creates a bind group from the given layout (which must have a static
    // sampler at binding 0) that contains the texture to be sampled.
    wgpu::BindGroup CreateBindGroupWithStaticSampler(wgpu::BindGroupLayout bgl) {
        return utils::MakeBindGroup(device, bgl, {{1, mTextureView}});
    }
};

// Test drawing a rect with a checkerboard texture using a static sampler with different address
// modes.
TEST_P(StaticSamplerTest, AddressMode) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    for (auto u : addressModes) {
        for (auto v : addressModes) {
            for (auto w : addressModes) {
                // Create the bind group layout with a static sampler for the
                // given address modes, configure the pipeline with that layout,
                // and test drawing with a bind group created from that layout.
                auto bgl = CreateBindGroupLayoutWithStaticSampler(u, v, w);
                InitShaders(kBasicFS, bgl);
                TestAddressModes(u, v, w, CreateBindGroupWithStaticSampler(bgl));
            }
        }
    }
}

// Test that passing texture and static sampler objects through user-defined functions works
// correctly.
TEST_P(StaticSamplerTest, PassThroughUserFunctionParameters) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    for (auto u : addressModes) {
        for (auto v : addressModes) {
            for (auto w : addressModes) {
                // Create the bind group layout with a static sampler for the
                // given address modes, configure the pipeline with that layout,
                // and test drawing with a bind group created from that layout.
                auto bgl = CreateBindGroupLayoutWithStaticSampler(u, v, w);
                InitShaders(kPassThroughUserFunctionsFS, bgl);
                TestAddressModes(u, v, w, CreateBindGroupWithStaticSampler(bgl));
            }
        }
    }
}

DAWN_INSTANTIATE_TEST(StaticSamplerTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
