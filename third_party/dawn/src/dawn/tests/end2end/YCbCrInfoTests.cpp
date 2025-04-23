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

// This must be above VulkanBackend.h otherwise vulkan.h will be included before we can wrap it with
// vulkan_platform.h.
#include "dawn/common/vulkan_platform.h"

#include "dawn/native/VulkanBackend.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include <android/hardware_buffer.h>
#endif  // DAWN_PLATFORM_IS(ANDROID)

namespace dawn {
namespace {

constexpr uint32_t kDefaultMipLevels = 1u;
constexpr uint32_t kDefaultLayerCount = 1u;
constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::External;

wgpu::Texture Create2DTexture(wgpu::Device& device) {
#if DAWN_PLATFORM_IS(ANDROID)
    constexpr uint32_t kWidth = 32u;
    constexpr uint32_t kHeight = 32u;
    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = kWidth,
        .height = kHeight,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Get actual desc for allocated buffer so we know the stride for cpu data.
    AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDesc);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;
    stmAHardwareBufferDesc.useExternalFormat = true;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kWidth;
    descriptor.size.height = kHeight;
    descriptor.size.depthOrArrayLayers = kDefaultLayerCount;
    descriptor.sampleCount = 1u;
    descriptor.format = kDefaultTextureFormat;
    descriptor.mipLevelCount = kDefaultMipLevels;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    auto texture = memory.CreateTexture(&descriptor);
    AHardwareBuffer_release(aHardwareBuffer);
    return texture;
#else
    return {};
#endif
}

wgpu::TextureViewDescriptor CreateDefaultViewDescriptor(
    wgpu::TextureViewDimension dimension,
    wgpu::TextureFormat format = kDefaultTextureFormat) {
    wgpu::TextureViewDescriptor descriptor;
    descriptor.format = format;
    descriptor.dimension = dimension;
    descriptor.baseMipLevel = 0;
    if (dimension != wgpu::TextureViewDimension::e1D) {
        descriptor.mipLevelCount = kDefaultMipLevels;
    }
    descriptor.baseArrayLayer = 0;
    descriptor.arrayLayerCount = kDefaultLayerCount;
    return descriptor;
}

wgpu::TextureView Create2DTextureView(wgpu::Texture texture,
                                      const wgpu::YCbCrVkDescriptor* yCbCrDesc) {
    wgpu::TextureViewDescriptor textureDesc =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    textureDesc.arrayLayerCount = 1;
    textureDesc.nextInChain = yCbCrDesc;

    return texture.CreateView(&textureDesc);
}

wgpu::Sampler CreateYCbCrSampler(wgpu::Device device, const wgpu::YCbCrVkDescriptor* yCbCrDesc) {
    wgpu::SamplerDescriptor samplerDesc = {};
    samplerDesc.nextInChain = yCbCrDesc;
    return device.CreateSampler(&samplerDesc);
}

class YCbCrInfoTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip tests if platform is not Android.
        DAWN_TEST_UNSUPPORTED_IF(!DAWN_PLATFORM_IS(ANDROID));
        // Skip all tests if ycbcr sampler feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::StaticSamplers}) &&
            SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}) &&
            SupportsFeatures({wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer})) {
            requiredFeatures.push_back(wgpu::FeatureName::YCbCrVulkanSamplers);
            requiredFeatures.push_back(wgpu::FeatureName::StaticSamplers);
            requiredFeatures.push_back(wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer);
        }
        return requiredFeatures;
    }
};

// Test that it is possible to create the sampler with ycbcr vulkan descriptor.
TEST_P(YCbCrInfoTest, YCbCrSamplerValidWhenFeatureEnabled) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    samplerDesc.nextInChain = &yCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is possible to create the sampler with ycbcr vulkan descriptor with only vulkan
// format set.
TEST_P(YCbCrInfoTest, YCbCrSamplerValidWithOnlyVkFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    // format is set as VK_FORMAT.
    yCbCrDesc.externalFormat = 0;
    samplerDesc.nextInChain = &yCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is possible to create the sampler with ycbcr vulkan descriptor with only external
// format set.
TEST_P(YCbCrInfoTest, YCbCrSamplerValidWithOnlyExternalFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    // format is set as externalFormat.
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 5;
    samplerDesc.nextInChain = &yCbCrDesc;

    device.CreateSampler(&samplerDesc);
}

// Test that it is NOT possible to create the sampler with ycbcr vulkan descriptor and no format
// set.
TEST_P(YCbCrInfoTest, YCbCrSamplerInvalidWithNoFormat) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 0;
    samplerDesc.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
}

// Test that it is invalid to create texture view with formats other than External.
TEST_P(YCbCrInfoTest, YCbCrTextureViewInvalidWithoutWgpuFormatExternal) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    // Pass RGBA8Unorm instead of External format.
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    descriptor.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
}

// Test that it is possible to create texture view with ycbcr vulkan descriptor.
TEST_P(YCbCrInfoTest, YCbCrTextureViewValidWithWgpuFormatExternal) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    descriptor.nextInChain = &yCbCrDesc;

    texture.CreateView(&descriptor);
}

// Test that it is possible to create texture view with ycbcr vulkan descriptor with only vulkan
// format set.
TEST_P(YCbCrInfoTest, YCbCrTextureViewValidWithOnlyVkFormat) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    // format is set as VK_FORMAT.
    yCbCrDesc.externalFormat = 0;
    descriptor.nextInChain = &yCbCrDesc;

    texture.CreateView(&descriptor);
}

// Test that it is possible to create texture view with ycbcr vulkan descriptor with only external
// format set.
TEST_P(YCbCrInfoTest, YCbCrTextureViewValidWithOnlyExternalFormat) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    // format is set as externalFormat.
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 5;
    descriptor.nextInChain = &yCbCrDesc;

    texture.CreateView(&descriptor);
}

// Test that it is NOT possible to create texture view with ycbcr vulkan descriptor and no format
// set.
TEST_P(YCbCrInfoTest, YCbCrTextureViewInvalidWithNoFormat) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_UNDEFINED;
    yCbCrDesc.externalFormat = 0;
    descriptor.nextInChain = &yCbCrDesc;

    ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
}

// Test that it is NOT possible to create texture view from texture created with
// TextureFormat::External but NO ycbcr vulkan descriptor passed.
TEST_P(YCbCrInfoTest, YCbCrTextureViewInvalidWithNoYCbCrDescriptor) {
    wgpu::Texture texture = Create2DTexture(device);

    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    descriptor.arrayLayerCount = 1;

    ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
}

// Tests that creating a bind group layout with a valid static sampler succeeds if the
// required feature is enabled.
TEST_P(YCbCrInfoTest, CreateBindGroupWithYCbCrSamplerSupported) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(device, &yCbCrDesc);
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

    wgpu::Texture texture = Create2DTexture(device);

    wgpu::YCbCrVkDescriptor yCbCrDescTex = {};
    yCbCrDescTex.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    wgpu::TextureView textureView = Create2DTextureView(texture, &yCbCrDescTex);

    utils::MakeBindGroup(device, layout, {{1, textureView}});
}

// Verify that creating a bind group layout fails with a static sampler binding that has no texture
// binding.
TEST_P(YCbCrInfoTest, CreateBindGroupLayoutWithYCbCrSamplerNoTextureBinding) {
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(device, &yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Verifies that creating a bind group layout fails with a static sampler binding that refers to an
// invalid binding index for texture binding.
TEST_P(YCbCrInfoTest, CreateBindGroupLayoutWithYCbCrSamplerInvalidTextureBinding) {
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(device, &yCbCrDesc);
    staticSamplerBinding.sampledTextureBinding = 1;
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Verifies that creating a bind group layout fails with a static sampler binding that refers to a
// binding that is not a texture binding.
TEST_P(YCbCrInfoTest, CreateBindGroupLayoutWithYCbCrSamplerInvalidTextureBindingType) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(device, &yCbCrDesc);
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
TEST_P(YCbCrInfoTest, CreateBindGroupLayoutWithYCbCrSamplerDuplicateSampledTextures) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    auto& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(device, &yCbCrDesc);
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

// Verifies that creation of a correctly-specified bind group for a layout that
// has a sampler and a static sampler succeeds.
TEST_P(YCbCrInfoTest, CreateBindGroupWithSamplerAndStaticSamplerSupported) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};

    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    samplerDesc.nextInChain = &yCbCrDesc;

    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
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

    wgpu::SamplerDescriptor dynamicSamplerDesc;
    dynamicSamplerDesc.minFilter = wgpu::FilterMode::Linear;

    wgpu::Texture texture = Create2DTexture(device);
    wgpu::TextureView textureView = Create2DTextureView(texture, &yCbCrDesc);

    utils::MakeBindGroup(device, layout,
                         {{0, device.CreateSampler(&dynamicSamplerDesc)}, {2, textureView}});
}

// Verifies that creation of a bind group with the correct number of entries for a layout that has a
// sampler and a static sampler raises an error if the entry is specified at the index of the static
// sampler rather than that of the sampler.
TEST_P(YCbCrInfoTest, BindGroupCreationForSamplerBindingTypeCausesError) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutEntry& binding1 = entries.emplace_back();
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};

    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    samplerDesc.nextInChain = &yCbCrDesc;

    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
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

    wgpu::SamplerDescriptor samplerDesc0;
    samplerDesc0.minFilter = wgpu::FilterMode::Linear;

    wgpu::Texture texture = Create2DTexture(device);
    wgpu::TextureView textureView = Create2DTextureView(texture, &yCbCrDesc);

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(
        device, layout, {{1, device.CreateSampler(&samplerDesc0)}, {2, textureView}}));
}

// Tests that creating a bind group fails when YCbCr texture isn't sampled by a static sampler.
TEST_P(YCbCrInfoTest, CreateBindGroupWithoutYCbCrSampler) {
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

    wgpu::Texture texture = Create2DTexture(device);
    wgpu::TextureView textureView = Create2DTextureView(texture, &yCbCrDesc);

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, textureView}}));
}

// Tests that creating a bind group fails when a YCbCr static sampler samples a non-YCbCr texture.
TEST_P(YCbCrInfoTest, CreatBindGroupYCbCrStaticSamplerWrongTexture) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    wgpu::BindGroupLayoutEntry& binding0 = entries.emplace_back();
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = CreateYCbCrSampler(device, &yCbCrDesc);
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

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 32;
    descriptor.size.height = 32;
    descriptor.size.depthOrArrayLayers = kDefaultLayerCount;
    descriptor.sampleCount = 1u;
    descriptor.format = wgpu::TextureFormat::RGBA8Snorm;
    descriptor.mipLevelCount = kDefaultMipLevels;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&descriptor);

    wgpu::TextureViewDescriptor textureDesc = CreateDefaultViewDescriptor(
        wgpu::TextureViewDimension::e2D, wgpu::TextureFormat::RGBA8Snorm);
    textureDesc.arrayLayerCount = 1;
    wgpu::TextureView textureView = texture.CreateView(&textureDesc);

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, textureView}}));
}

// Tests that creating a bind group fails when a non-YCbCr static sampler samples a YCbCr texture.
TEST_P(YCbCrInfoTest, CreatBindGroupYCbCrTextureWrongStaticSampler) {
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

    wgpu::Texture texture = Create2DTexture(device);
    wgpu::TextureView textureView = Create2DTextureView(texture, &yCbCrDesc);

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, textureView}}));
}

DAWN_INSTANTIATE_TEST(YCbCrInfoTest, VulkanBackend());

// TODO(crbug.com/dawn/2476): Add validation that mipLevel, arrayLayers are always 1 along with 2D
// view dimension (see
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageCreateInfo.html) with
// YCbCr and tests for it.

}  // anonymous namespace
}  // namespace dawn
