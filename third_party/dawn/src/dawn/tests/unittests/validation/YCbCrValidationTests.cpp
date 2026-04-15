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

#include <vector>

// This must be included instead of vulkan.h so that we can wrap it with vulkan_platform.h.
#include "dawn/common/vulkan_platform.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

wgpu::ExternalTextureDescriptor CreateDefaultExternalTextureDescriptor() {
    static constexpr std::array<float, 12> kPlaceholderConstantArray = {};

    wgpu::ExternalTextureDescriptor desc;
    desc.yuvToRgbConversionMatrix = kPlaceholderConstantArray.data();
    desc.gamutConversionMatrix = kPlaceholderConstantArray.data();
    desc.srcTransferFunctionParameters = kPlaceholderConstantArray.data();
    desc.dstTransferFunctionParameters = kPlaceholderConstantArray.data();
    desc.cropSize = {1, 1};
    desc.apparentSize = {1, 1};
    return desc;
}

class YCbCrVulkanSamplersWithoutFeatureTest : public ValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MultiPlanarFormatExtendedUsages};
    }
};

// Tests that creating a sampler with a valid ycbcr vulkan descriptor raises an error
// if the required feature is not enabled.
TEST_F(YCbCrVulkanSamplersWithoutFeatureTest, YCbCrSamplerNotSupported) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    samplerDesc.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
}

// Tests that creating a texture with OpaqueYCbCrAndroid is not possible without the feature.
TEST_F(YCbCrVulkanSamplersWithoutFeatureTest, YCbCrTextureNotSupported) {
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    ASSERT_DEVICE_ERROR(device.CreateTexture(&tDesc));
}

class YCbCrVulkanSamplersTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MultiPlanarFormatP210,
                wgpu::FeatureName::MultiPlanarFormatExtendedUsages,
                wgpu::FeatureName::StaticSamplers, wgpu::FeatureName::YCbCrVulkanSamplers};
    }

    wgpu::TextureView CreateYCbCrView(const wgpu::YCbCrVkDescriptor* yCbCrDesc) {
        wgpu::TextureDescriptor tDesc = {
            .usage = wgpu::TextureUsage::TextureBinding,
            .size = {2, 2},
            .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
        };
        wgpu::Texture texture = device.CreateTexture(&tDesc);

        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.nextInChain = yCbCrDesc;
        return texture.CreateView(&viewDesc);
    }

    wgpu::Sampler CreateYCbCrSampler(const wgpu::YCbCrVkDescriptor* yCbCrDesc) {
        wgpu::SamplerDescriptor samplerDesc = {};
        samplerDesc.nextInChain = yCbCrDesc;
        return device.CreateSampler(&samplerDesc);
    }
};

// Test that creating a YCbCr sampler requires a VkFormat or an external format.
TEST_F(YCbCrVulkanSamplersTest, YCbCrSamplerRequiresVkOrExternalFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    samplerDesc.nextInChain = &yCbCrDesc;

    // Error case: no VkFormat nor ExternalFormat specified.
    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));

    // Success case: VkFormat specified
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    yCbCrDesc.externalFormat = 0;
    device.CreateSampler(&samplerDesc);

    // Success case: external format specified
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 5;
    device.CreateSampler(&samplerDesc);

    // Success case: bth VkFormat and external format specified
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    yCbCrDesc.externalFormat = 5;
    device.CreateSampler(&samplerDesc);
}

// Test that only OpaqueYCbCrAndroid textures can be used to create YCbCr views.
TEST_F(YCbCrVulkanSamplersTest, YCbCrTextureViewRequiresOpaqueYCbCrAndroid) {
    wgpu::TextureViewDescriptor viewDesc{};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    viewDesc.nextInChain = &yCbCrDesc;
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    yCbCrDesc.externalFormat = 0;

    // Success case: creating a YCbCr view on an OpaqueYCbCrAndroid texture.
    {
        wgpu::TextureDescriptor yCbCrDesc = {
            .usage = wgpu::TextureUsage::TextureBinding,
            .size = {2, 2},
            .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
        };
        wgpu::Texture ycbcr = device.CreateTexture(&yCbCrDesc);
        ycbcr.CreateView(&viewDesc);
    }

    // Error case: creating a YCbCr view on an RGBA texture.
    {
        wgpu::TextureDescriptor rgbaDesc = {
            .usage = wgpu::TextureUsage::TextureBinding,
            .size = {2, 2},
            .format = wgpu::TextureFormat::RGBA8Unorm,
        };
        wgpu::Texture rgba = device.CreateTexture(&rgbaDesc);
        ASSERT_DEVICE_ERROR(rgba.CreateView(&viewDesc));
    }

    // Error case: creating a YCbCr view on an YUV texture that's not OpaqueYCbCrAndroid.
    {
        wgpu::TextureDescriptor yuvDesc = {
            .usage = wgpu::TextureUsage::TextureBinding,
            .size = {2, 2},
            .format = wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm,
        };
        wgpu::Texture yuv = device.CreateTexture(&yuvDesc);
        ASSERT_DEVICE_ERROR(yuv.CreateView(&viewDesc));
    }
}

// Test that creating a YCbCr texture view requires a VkFormat or an external format.
TEST_F(YCbCrVulkanSamplersTest, YCbCrTextureViewRequiresVkOrExternalFormat) {
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    wgpu::TextureViewDescriptor viewDesc{};

    // Error case: creating a view without a YCbCrVkDescriptor is not allowed.
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    viewDesc.nextInChain = &yCbCrDesc;

    // Error case: no VkFormat nor ExternalFormat specified.
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    // Success case: VkFormat specified
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    yCbCrDesc.externalFormat = 0;
    texture.CreateView(&viewDesc);

    // Success case: external format specified
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 5;
    texture.CreateView(&viewDesc);

    // Success case: bth VkFormat and external format specified
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    yCbCrDesc.externalFormat = 5;
    texture.CreateView(&viewDesc);
}

// Tests that creating a bind group layout with a valid static sampler succeeds if the
// required feature is enabled.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupWithYCbCrSamplerSupported) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding0.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    binding1.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
    binding1.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding1.texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&layoutDesc);

    utils::MakeBindGroup(device, layout, {{1, CreateYCbCrView(&yCbCrDesc)}});
}

// Verify that creating a bind group layout fails with a static sampler binding that has no texture
// binding.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupLayoutWithYCbCrSamplerNoTextureBinding) {
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Verifies that creating a bind group layout fails with a static sampler binding that refers to an
// invalid binding index for texture binding.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupLayoutWithYCbCrSamplerInvalidTextureBinding) {
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Verifies that creating a bind group layout fails with a static sampler binding that refers to a
// binding that is not a texture binding.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupLayoutWithYCbCrSamplerInvalidTextureBindingType) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding0.nextInChain = &staticSamplerBinding;

    // This should be a texture binding but it's not.
    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    binding1.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&layoutDesc));
}

// Verify that creating a bind group layout fails when two static samplers have the same
// sampled texture binding.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupLayoutWithYCbCrSamplerDuplicateSampledTextures) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    auto& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding0.nextInChain = &staticSamplerBinding;

    auto& binding1 = entries.emplace_back();
    binding1.binding = 1;
    binding1.texture.sampleType = wgpu::TextureSampleType::Float;
    binding1.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding1.texture.multisampled = false;

    // This static sampler has the same `sampledTextureBinding` as binding 0.
    auto& binding2 = entries.emplace_back();
    binding2.binding = 2;
    binding2.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&layoutDesc));
}

// Regression test where BindGroup validation checked for sampledTextureBinding even when none was
// specified on the API side.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupLayoutWithYCbCrSamplerDuplicateSampledTextures2) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    // Static sampler that has a sampledTextureBinding.
    auto& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding0 = {};
    staticSamplerBinding0.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding0.sampledTextureBinding = 1;
    binding0.nextInChain = &staticSamplerBinding0;

    // The texture for that sampler.
    auto& binding1 = entries.emplace_back();
    binding1.binding = 1;
    binding1.texture.sampleType = wgpu::TextureSampleType::Float;

    // A second static sampler that shouldn't check for its sampledTextureBinding since it doesn't
    // have one.
    auto& binding2 = entries.emplace_back();
    binding2.binding = 2;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding2 = {};
    staticSamplerBinding2.sampler = device.CreateSampler();
    binding2.nextInChain = &staticSamplerBinding2;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();
    auto bgl = device.CreateBindGroupLayout(&layoutDesc);

    utils::MakeBindGroup(device, bgl, {{1, CreateYCbCrView(&yCbCrDesc)}});
}

// Verifies that creation of a correctly-specified bind group for a layout that
// has a sampler and a static sampler succeeds.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupWithSamplerAndStaticSamplerSupported) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 2;
    binding1.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutEntry& binding2 = entries.emplace_back();
    binding2.binding = 2;
    binding2.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
    binding2.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding2.texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    utils::MakeBindGroup(device, layout,
                         {{0, device.CreateSampler()}, {2, CreateYCbCrView(&yCbCrDesc)}});
}

// Verifies that creation of a bind group with the correct number of entries for a layout that has a
// sampler and a static sampler raises an error if the entry is specified at the index of the static
// sampler rather than that of the sampler.
TEST_F(YCbCrVulkanSamplersTest, BindGroupCreationForSamplerBindingTypeCausesError) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 2;
    binding1.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutEntry& binding2 = entries.emplace_back();
    binding2.binding = 2;
    binding2.texture.sampleType = wgpu::TextureSampleType::Float;
    binding2.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding2.texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = entries.size();
    desc.entries = entries.data();
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout,
                                             {{0, device.CreateSampler()},
                                              {1, device.CreateSampler()},
                                              {2, CreateYCbCrView(&yCbCrDesc)}}));
}

// Tests that creating a bind group fails when YCbCr texture isn't sampled by a static sampler.
TEST_F(YCbCrVulkanSamplersTest, CreateBindGroupWithoutYCbCrSampler) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    binding0.texture.sampleType = wgpu::TextureSampleType::Float;
    binding0.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding0.texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&layoutDesc);

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, CreateYCbCrView(&yCbCrDesc)}}));
}

// Tests that creating a bind group fails when a YCbCr static sampler samples a non-YCbCr texture.
TEST_F(YCbCrVulkanSamplersTest, CreatBindGroupYCbCrStaticSamplerWrongTexture) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(&yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding0.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    binding1.texture.sampleType = wgpu::TextureSampleType::Float;
    binding1.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding1.texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&layoutDesc);

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, texture.CreateView()}}));
}

// Tests that creating a bind group fails when a non-YCbCr static sampler samples a YCbCr texture.
TEST_F(YCbCrVulkanSamplersTest, CreatBindGroupYCbCrTextureWrongStaticSampler) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    wgpu::SamplerDescriptor samplerDesc;
    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding0.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    binding1.texture.sampleType = wgpu::TextureSampleType::Float;
    binding1.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding1.texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&layoutDesc);

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, CreateYCbCrView(&yCbCrDesc)}}));
}

// Test that using different YCbCr info creates different texture views. This is a test for a bug
// where the TextureViewCache would not look at the YCbCr info and would cache as the same view all
// textures views differing only on that info.
TEST_F(YCbCrVulkanSamplersTest, YCbCrMakesDifferentTextureView) {
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.nextInChain = &yCbCrDesc;

    yCbCrDesc.forceExplicitReconstruction = true;
    wgpu::TextureView view1 = texture.CreateView(&viewDesc);
    yCbCrDesc.forceExplicitReconstruction = false;
    wgpu::TextureView view2 = texture.CreateView(&viewDesc);

    ASSERT_NE(view1.Get(), view2.Get());
}

// That that OpaqueYCbCrAndroid views cannot be used to create ExternalTextures with only this
// feature.
TEST_F(YCbCrVulkanSamplersTest, YCbCrViewNotAllowedForExternalTexture) {
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    wgpu::TextureView ycbcrView = CreateYCbCrView(&yCbCrDesc);

    // Error case, the OpaqueYCbCrForExternalTexture extension is not enabled.
    wgpu::ExternalTextureDescriptor etDesc = CreateDefaultExternalTextureDescriptor();
    etDesc.plane0 = ycbcrView;
    ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&etDesc));
}

class OpaqueYCbCrForExternalTextureWithoutFeatureTest : public ValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MultiPlanarFormatExtendedUsages};
    }
};

// Tests that creating a texture with OpaqueYCbCrAndroid is not possible without the feature.
TEST_F(OpaqueYCbCrForExternalTextureWithoutFeatureTest, YCbCrTextureNotSupported) {
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    ASSERT_DEVICE_ERROR(device.CreateTexture(&tDesc));
}

class OpaqueYCbCrForExternalTextureTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MultiPlanarFormatP210,
                wgpu::FeatureName::MultiPlanarFormatExtendedUsages,
                wgpu::FeatureName::OpaqueYCbCrAndroidForExternalTexture};
    }
};

// Test that OpaqueYCbCrAndroid texture views can be created.
TEST_F(OpaqueYCbCrForExternalTextureTest, YCbCrViewSupported) {
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Success case: a texture view can be created without YCbCrVkDescriptor.
    texture.CreateView();

    // Error case: using a YCbCrVkDescriptor is not allowed with just this feature.
    wgpu::TextureViewDescriptor viewDesc = {};
    wgpu::YCbCrVkDescriptor ycbcr = {};
    ycbcr.externalFormat = 1;
    viewDesc.nextInChain = &ycbcr;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
}

// That that OpaqueYCbCrAndroid views cannot be used in BindGroups with just this feature.
TEST_F(OpaqueYCbCrForExternalTextureTest, YCbCrViewNotAllowedInBindGroup) {
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::UnfilterableFloat}});
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, texture.CreateView()}}));
}

// That that OpaqueYCbCrAndroid views can be used to create single-planar ExternalTextures.
TEST_F(OpaqueYCbCrForExternalTextureTest, YCbCrViewForExternalTexture) {
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    wgpu::TextureView ycbcrView = device.CreateTexture(&tDesc).CreateView();

    // Success case, the ExternalTexture uses a single "plane" that's the YCbCr view.
    wgpu::ExternalTextureDescriptor etDesc = CreateDefaultExternalTextureDescriptor();
    etDesc.plane0 = ycbcrView;
    device.CreateExternalTexture(&etDesc);

    // Error case, the ExternalTexture cannot be created with a YCbCrView and a second plane.
    tDesc.format = wgpu::TextureFormat::RG8Unorm;  // What could be a UV plane.
    etDesc.plane1 = device.CreateTexture(&tDesc).CreateView();
    ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&etDesc));
}

class BothYCbCrExtensionTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MultiPlanarFormatP210,
                wgpu::FeatureName::MultiPlanarFormatExtendedUsages,
                wgpu::FeatureName::StaticSamplers, wgpu::FeatureName::YCbCrVulkanSamplers,
                wgpu::FeatureName::OpaqueYCbCrAndroidForExternalTexture};
    }
};

// That that only views without a YCbCrVkDescriptor can be used to create ExternalTextures.
TEST_F(BothYCbCrExtensionTest, YCbCrViewForExternalTexture) {
    // Create view with / without a YCbCrVkDescriptor.
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    wgpu::TextureViewDescriptor viewDesc;
    wgpu::TextureView noYcbcrDescView = texture.CreateView(&viewDesc);

    wgpu::YCbCrVkDescriptor ycbcrDesc;
    ycbcrDesc.externalFormat = 1;
    viewDesc.nextInChain = &ycbcrDesc;
    wgpu::TextureView ycbcrDescView = texture.CreateView(&viewDesc);

    // Success case: the view doesn't have a YCbCrVkDescriptor.
    wgpu::ExternalTextureDescriptor etDesc = CreateDefaultExternalTextureDescriptor();
    etDesc.plane0 = noYcbcrDescView;
    device.CreateExternalTexture(&etDesc);

    // Error case: the view has a YCbCrVkDescriptor.
    etDesc.plane0 = ycbcrDescView;
    ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&etDesc));
}

// Tests that creating a bind group layout with a valid static sampler succeeds if the
// required feature is enabled.
TEST_F(BothYCbCrExtensionTest, CreateBindGroupWithYCbCrView) {
    wgpu::YCbCrVkDescriptor ycbcrDesc;
    ycbcrDesc.externalFormat = 1;

    // Create view with / without a YCbCrVkDescriptor.
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {2, 2},
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    wgpu::TextureViewDescriptor viewDesc;
    wgpu::TextureView noYcbcrDescView = texture.CreateView(&viewDesc);

    viewDesc.nextInChain = &ycbcrDesc;
    wgpu::TextureView ycbcrDescView = texture.CreateView(&viewDesc);

    // Set up the BGL with the static YCbCr sampler.
    wgpu::SamplerDescriptor sDesc = {};
    sDesc.nextInChain = &ycbcrDesc;
    wgpu::Sampler ycbcrSampler = device.CreateSampler(&sDesc);

    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = ycbcrSampler;
    staticSamplerBinding.sampledTextureBinding = 1;
    binding0.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    binding1.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
    binding1.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    binding1.texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor layoutDesc = {};
    layoutDesc.entryCount = entries.size();
    layoutDesc.entries = entries.data();
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&layoutDesc);

    // Success case: the view has a YCbCrVkDescriptor
    utils::MakeBindGroup(device, layout, {{1, ycbcrDescView}});

    // Error case: the view doesn't have a YCbCrVkDescriptor
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, noYcbcrDescView}}));
}

}  // anonymous namespace
}  // namespace dawn
