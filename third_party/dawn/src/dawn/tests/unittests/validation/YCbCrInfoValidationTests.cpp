// Copyright 2024 The Dawn & Tint Authors
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

#include <vulkan/vulkan.h>
#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kWidth = 32u;
constexpr uint32_t kHeight = 32u;
constexpr uint32_t kDefaultMipLevels = 1u;
constexpr uint32_t kDefaultLayerCount = 1u;
constexpr uint32_t kDefaultSampleCount = 1u;
constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::External;

wgpu::Texture Create2DTexture(wgpu::Device& device,
                              wgpu::TextureFormat format = kDefaultTextureFormat) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kWidth;
    descriptor.size.height = kHeight;
    descriptor.size.depthOrArrayLayers = kDefaultLayerCount;
    descriptor.sampleCount = kDefaultSampleCount;
    descriptor.format = format;
    descriptor.mipLevelCount = kDefaultMipLevels;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    return device.CreateTexture(&descriptor);
}

wgpu::TextureViewDescriptor CreateDefaultViewDescriptor(wgpu::TextureViewDimension dimension) {
    wgpu::TextureViewDescriptor descriptor;
    descriptor.format = kDefaultTextureFormat;
    descriptor.dimension = dimension;
    descriptor.baseMipLevel = 0;
    if (dimension != wgpu::TextureViewDimension::e1D) {
        descriptor.mipLevelCount = kDefaultMipLevels;
    }
    descriptor.baseArrayLayer = 0;
    descriptor.arrayLayerCount = kDefaultLayerCount;
    return descriptor;
}

class YCbCrInfoWithoutFeatureValidationTest : public ValidationTest {
    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }
};

// Tests that creating a sampler with a valid ycbcr vulkan descriptor raises an error
// if the required feature is not enabled.
TEST_F(YCbCrInfoWithoutFeatureValidationTest, YCbCrSamplerNotSupported) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    samplerDesc.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
}

// Tests that creating a texture with External format raises an error if the required feature is not
// enabled.
TEST_F(YCbCrInfoWithoutFeatureValidationTest, ExternalTextureNotSupported) {
    ASSERT_DEVICE_ERROR(Create2DTexture(device));
}

// Tests that creating a texture view with a valid ycbcr vulkan descriptor raises an error
// if the required feature is not enabled.
TEST_F(YCbCrInfoWithoutFeatureValidationTest, YCbCrTextureViewNotSupported) {
    wgpu::Texture texture = Create2DTexture(device, wgpu::TextureFormat::RGBA8Unorm);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    descriptor.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
}

class YCbCrInfoWithFeatureValidationTest : public YCbCrInfoWithoutFeatureValidationTest {
    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(!DAWN_PLATFORM_IS(ANDROID));
        DAWN_SKIP_TEST_IF(UsesWire());
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::StaticSamplers, wgpu::FeatureName::YCbCrVulkanSamplers};
    }
};

// Tests that creating a sampler with a valid ycbcr vulkan descriptor succeeds if the
// required feature is enabled.
TEST_F(YCbCrInfoWithFeatureValidationTest, YCbCrSamplerSupported) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    samplerDesc.nextInChain = &yCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Tests that creating a texture view with a valid ycbcr vulkan descriptor succeeds if the
// required feature is enabled.
TEST_F(YCbCrInfoWithFeatureValidationTest, YCbCrTextureViewSupported) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    descriptor.nextInChain = &yCbCrDesc;

    texture.CreateView(&descriptor);
}


}  // anonymous namespace
}  // namespace dawn
