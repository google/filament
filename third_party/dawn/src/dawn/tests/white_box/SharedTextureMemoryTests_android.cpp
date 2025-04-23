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

#include <android/hardware_buffer.h>
#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/tests/white_box/SharedTextureMemoryTests.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

template <typename BackendBase>
class SharedTextureMemoryTestAndroidBackend : public BackendBase {
  public:
    std::string Name() const override { return "AHardwareBuffer"; }

    static std::string MakeLabel(const AHardwareBuffer_Desc& desc) {
        std::string label = std::to_string(desc.width) + "x" + std::to_string(desc.height);
        switch (desc.format) {
            case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
                label += " R8G8B8A8_UNORM";
                break;
            case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
                label += " R8G8B8X8_UNORM";
                break;
            case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
                label += " R16G16B16A16_FLOAT";
                break;
            case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
                label += " R10G10B10A2_UNORM";
                break;
            case AHARDWAREBUFFER_FORMAT_R8_UNORM:
                label += " R8_UNORM";
                break;
        }
        if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
            label += " GPU_FRAMEBUFFER";
        }
        if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
            label += " GPU_SAMPLED_IMAGE";
        }
        return label;
    }

    template <typename CreateFn>
    auto CreateSharedTextureMemoryHelper(uint32_t size,
                                         AHardwareBuffer_Format format,
                                         AHardwareBuffer_UsageFlags usage,
                                         CreateFn createFn) {
        wgpu::SharedTextureMemoryDescriptor desc;
        AHardwareBuffer_Desc aHardwareBufferDesc = {};
        aHardwareBufferDesc.width = size;
        aHardwareBufferDesc.height = size;
        aHardwareBufferDesc.layers = 1;
        aHardwareBufferDesc.format = format;
        aHardwareBufferDesc.usage = usage;

        AHardwareBuffer* aHardwareBuffer;
        if (AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer) < 0) {
            // This combination of format / usage is not supported.
            // Unfortunately, AHardwareBuffer_isSupported requires API version 29.
            return decltype(createFn(desc)){};
        }

        wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
        stmAHardwareBufferDesc.handle = aHardwareBuffer;

        std::string label = MakeLabel(aHardwareBufferDesc);
        desc.label = label.c_str();
        desc.nextInChain = &stmAHardwareBufferDesc;

        auto ret = createFn(desc);
        AHardwareBuffer_release(aHardwareBuffer);

        return ret;
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        wgpu::SharedTextureMemory ret = CreateSharedTextureMemoryHelper(
            16, AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
            static_cast<AHardwareBuffer_UsageFlags>(
                AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                AHARDWAREBUFFER_USAGE_CPU_READ_NEVER | AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER),
            [&](const wgpu::SharedTextureMemoryDescriptor& desc) {
                return device.ImportSharedTextureMemory(&desc);
            });
        EXPECT_NE(ret, nullptr);
        return ret;
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices,
        int layerCount) override {
        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;

        // AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM behaves inconsistantly between GLES (alpha is
        // always 1.0) and Vulkan (alpha is writeable) so it is omitted. There is no TextureFormat
        // to represent the difference so it's undetectable when testing.
        for (auto format : {
                 AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
                 AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT,
                 AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM,
                 AHARDWAREBUFFER_FORMAT_R8_UNORM,
             }) {
            for (auto usage : {
                     static_cast<AHardwareBuffer_UsageFlags>(
                         AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                         AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                         AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                         AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER),
                 }) {
                for (uint32_t size : {4, 64}) {
                    std::vector<wgpu::SharedTextureMemory> perDeviceMemories =
                        CreateSharedTextureMemoryHelper(
                            size, format, usage,
                            [&](const wgpu::SharedTextureMemoryDescriptor& desc) {
                                std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                                for (auto& device : devices) {
                                    auto stm = device.ImportSharedTextureMemory(&desc);
                                    if (!stm) {
                                        // The format/usage is not supported.
                                        // It won't be supported on any device, so break.
                                        return std::vector<wgpu::SharedTextureMemory>{};
                                    }
                                    perDeviceMemories.push_back(std::move(stm));
                                }
                                return perDeviceMemories;
                            });
                    if (!perDeviceMemories.empty()) {
                        memories.push_back(std::move(perDeviceMemories));
                    }
                }
            }
        }
        return memories;
    }
};

class SharedTextureMemoryTestAndroidVulkanBackend
    : public SharedTextureMemoryTestAndroidBackend<SharedTextureMemoryTestVulkanBackend> {
  public:
    static SharedTextureMemoryTestBackend* GetInstance() {
        static SharedTextureMemoryTestAndroidVulkanBackend b;
        return &b;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter&) const override {
        return {wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer,
                wgpu::FeatureName::SharedFenceSyncFD, wgpu::FeatureName::YCbCrVulkanSamplers};
    }
};

class SharedTextureMemoryTestAndroidSyncFDOpenGLESBackend
    : public SharedTextureMemoryTestAndroidBackend<SharedTextureMemoryTestBackend> {
  public:
    static SharedTextureMemoryTestBackend* GetInstance() {
        static SharedTextureMemoryTestAndroidSyncFDOpenGLESBackend b;
        return &b;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter&) const override {
        return {wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer,
                wgpu::FeatureName::SharedFenceSyncFD};
    }
};

class SharedTextureMemoryTestAndroidEGLSyncOpenGLESBackend
    : public SharedTextureMemoryTestAndroidBackend<SharedTextureMemoryTestBackend> {
  public:
    static SharedTextureMemoryTestBackend* GetInstance() {
        static SharedTextureMemoryTestAndroidEGLSyncOpenGLESBackend b;
        return &b;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter&) const override {
        return {wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer,
                wgpu::FeatureName::SharedFenceEGLSync};
    }
};

// Test clearing the texture memory on the device, then reading it on the CPU.
TEST_P(SharedTextureMemoryTests, GPUWriteThenCPURead) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::SharedFenceSyncFD}));

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Get actual desc for allocated buffer so we know the stride for cpu data.
    AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDesc);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()});
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
    passDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    passDescriptor.cColorAttachments[0].clearValue = {0.5001, 1.0, 0.2501, 1.0};

    encoder.BeginRenderPass(&passDescriptor).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout{};
    if (IsVulkan()) {
        beginLayout.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        beginLayout.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        beginDesc.nextInChain = &beginLayout;
    }
    memory.BeginAccess(texture, &beginDesc);

    device.GetQueue().Submit(1, &commandBuffer);

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    wgpu::SharedTextureMemoryVkImageLayoutEndState endLayout{};
    if (IsVulkan()) {
        endState.nextInChain = &endLayout;
    }
    memory.EndAccess(texture, &endState);

    wgpu::SharedFenceExportInfo exportInfo;
    endState.fences[0].ExportInfo(&exportInfo);

    // AHardwareBuffer_lock requires a fd to wait on. Otherwise we would need wait until the
    // submitted work is finished here.
    DAWN_TEST_UNSUPPORTED_IF(exportInfo.type != wgpu::SharedFenceType::SyncFD);

    wgpu::SharedFenceSyncFDExportInfo syncFdExportInfo;
    exportInfo.nextInChain = &syncFdExportInfo;
    endState.fences[0].ExportInfo(&exportInfo);

    void* ptr;
    EXPECT_EQ(AHardwareBuffer_lock(aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN,
                                   syncFdExportInfo.handle, nullptr, &ptr),
              0);

    auto* pixels = static_cast<utils::RGBA8*>(ptr);
    for (uint32_t r = 0; r < aHardwareBufferDesc.height; ++r) {
        for (uint32_t c = 0; c < aHardwareBufferDesc.width; ++c) {
            EXPECT_EQ(pixels[r * aHardwareBufferDesc.stride + c], utils::RGBA8(128, 255, 64, 255))
                << r << ", " << c;
        }
    }

    EXPECT_EQ(AHardwareBuffer_unlock(aHardwareBuffer, nullptr), 0);
    AHardwareBuffer_release(aHardwareBuffer);
}

// Test writing the memory on the CPU, then sampling on the device.
TEST_P(SharedTextureMemoryTests, CPUWriteThenGPURead) {
    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        // Include at least one GPU usage. Otherwise import to Vulkan can fail.
        .usage = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Get actual desc for allocated buffer so we know the stride for cpu data.
    AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDesc);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);
    wgpu::Texture texture = memory.CreateTexture();

    void* ptr;
    EXPECT_EQ(AHardwareBuffer_lock(aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1,
                                   nullptr, &ptr),
              0);

    std::array<utils::RGBA8, 16> expected = {{
        utils::RGBA8::kRed,
        utils::RGBA8::kGreen,
        utils::RGBA8::kBlue,
        utils::RGBA8::kYellow,
        utils::RGBA8::kGreen,
        utils::RGBA8::kBlue,
        utils::RGBA8::kYellow,
        utils::RGBA8::kRed,
        utils::RGBA8::kBlue,
        utils::RGBA8::kYellow,
        utils::RGBA8::kRed,
        utils::RGBA8::kGreen,
        utils::RGBA8::kYellow,
        utils::RGBA8::kRed,
        utils::RGBA8::kGreen,
        utils::RGBA8::kBlue,
    }};

    auto* pixels = static_cast<utils::RGBA8*>(ptr);
    for (uint32_t r = 0; r < aHardwareBufferDesc.height; ++r) {
        for (uint32_t c = 0; c < aHardwareBufferDesc.width; ++c) {
            pixels[r * aHardwareBufferDesc.stride + c] =
                expected[r * aHardwareBufferDesc.width + c];
        }
    }

    EXPECT_EQ(AHardwareBuffer_unlock(aHardwareBuffer, nullptr), 0);
    AHardwareBuffer_release(aHardwareBuffer);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;

    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout{};
    if (IsVulkan()) {
        beginDesc.nextInChain = &beginLayout;
    }

    memory.BeginAccess(texture, &beginDesc);
    EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0},
                      {aHardwareBufferDesc.width, aHardwareBufferDesc.height});
}

// Test validation of an incorrectly-configured SharedTextureMemoryAHardwareBufferProperties
// instance.
TEST_P(SharedTextureMemoryTests, InvalidSharedTextureMemoryAHardwareBufferProperties) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::SharedTextureMemoryProperties properties;
    wgpu::SharedTextureMemoryAHardwareBufferProperties ahbProperties = {};
    wgpu::YCbCrVkDescriptor yCbCrDesc;

    // Chaining anything onto the passed-in YCbCrVkDescriptor is invalid.
    yCbCrDesc.nextInChain = &stmAHardwareBufferDesc;
    ahbProperties.yCbCrInfo = yCbCrDesc;
    properties.nextInChain = &ahbProperties;

    ASSERT_DEVICE_ERROR(memory.GetProperties(&properties));
}

// Test querying YCbCr info from the Device.
TEST_P(SharedTextureMemoryTests, QueryYCbCrInfoFromDevice) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Query the YCbCr properties of the AHardwareBuffer.
    auto deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    native::vulkan::PNextChainBuilder bufferPropertiesChain(&bufferProperties);
    bufferPropertiesChain.Add(&bufferFormatProperties,
                              VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

    VkDevice vkDevice = deviceVk->GetVkDevice();
    EXPECT_EQ(deviceVk->fn.GetAndroidHardwareBufferPropertiesANDROID(vkDevice, aHardwareBuffer,
                                                                     &bufferProperties),
              VK_SUCCESS);

    // Query the YCbCr properties of this AHB via the Device.
    wgpu::AHardwareBufferProperties ahbProperties;
    device.GetAHardwareBufferProperties(aHardwareBuffer, &ahbProperties);
    auto yCbCrInfo = ahbProperties.yCbCrInfo;
    uint32_t formatFeatures = bufferFormatProperties.formatFeatures;

    // Verify that the YCbCr properties match.
    EXPECT_EQ(bufferFormatProperties.format, yCbCrInfo.vkFormat);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrModel, yCbCrInfo.vkYCbCrModel);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrRange, yCbCrInfo.vkYCbCrRange);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.r,
              yCbCrInfo.vkComponentSwizzleRed);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.g,
              yCbCrInfo.vkComponentSwizzleGreen);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.b,
              yCbCrInfo.vkComponentSwizzleBlue);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.a,
              yCbCrInfo.vkComponentSwizzleAlpha);
    EXPECT_EQ(bufferFormatProperties.suggestedXChromaOffset, yCbCrInfo.vkXChromaOffset);
    EXPECT_EQ(bufferFormatProperties.suggestedYChromaOffset, yCbCrInfo.vkYChromaOffset);

    wgpu::FilterMode expectedFilter =
        (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)
            ? wgpu::FilterMode::Linear
            : wgpu::FilterMode::Nearest;
    EXPECT_EQ(expectedFilter, yCbCrInfo.vkChromaFilter);
    EXPECT_EQ(
        formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
        yCbCrInfo.forceExplicitReconstruction);
    EXPECT_EQ(bufferFormatProperties.externalFormat, yCbCrInfo.externalFormat);
}

// Test querying YCbCr info from the SharedTextureMemory without external format.
TEST_P(SharedTextureMemoryTests, QueryYCbCrInfoWithoutExternalFormat) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Query the YCbCr properties of the AHardwareBuffer.
    auto deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    native::vulkan::PNextChainBuilder bufferPropertiesChain(&bufferProperties);
    bufferPropertiesChain.Add(&bufferFormatProperties,
                              VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

    VkDevice vkDevice = deviceVk->GetVkDevice();
    EXPECT_EQ(deviceVk->fn.GetAndroidHardwareBufferPropertiesANDROID(vkDevice, aHardwareBuffer,
                                                                     &bufferProperties),
              VK_SUCCESS);

    // Query the YCbCr properties of a SharedTextureMemory created from this
    // AHB.
    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;
    stmAHardwareBufferDesc.useExternalFormat = false;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::SharedTextureMemoryProperties properties;
    wgpu::SharedTextureMemoryAHardwareBufferProperties ahbProperties = {};
    properties.nextInChain = &ahbProperties;
    memory.GetProperties(&properties);
    auto yCbCrInfo = ahbProperties.yCbCrInfo;
    uint32_t formatFeatures = bufferFormatProperties.formatFeatures;

    // Verify that the YCbCr properties match.
    EXPECT_EQ(bufferFormatProperties.format, yCbCrInfo.vkFormat);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrModel, yCbCrInfo.vkYCbCrModel);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrRange, yCbCrInfo.vkYCbCrRange);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.r,
              yCbCrInfo.vkComponentSwizzleRed);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.g,
              yCbCrInfo.vkComponentSwizzleGreen);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.b,
              yCbCrInfo.vkComponentSwizzleBlue);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.a,
              yCbCrInfo.vkComponentSwizzleAlpha);
    EXPECT_EQ(bufferFormatProperties.suggestedXChromaOffset, yCbCrInfo.vkXChromaOffset);
    EXPECT_EQ(bufferFormatProperties.suggestedYChromaOffset, yCbCrInfo.vkYChromaOffset);

    wgpu::FilterMode expectedFilter =
        (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)
            ? wgpu::FilterMode::Linear
            : wgpu::FilterMode::Nearest;
    EXPECT_EQ(expectedFilter, yCbCrInfo.vkChromaFilter);
    EXPECT_EQ(
        formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
        yCbCrInfo.forceExplicitReconstruction);
    uint64_t expectedExternalFormat = 0u;
    EXPECT_EQ(expectedExternalFormat, yCbCrInfo.externalFormat);
}

// Test querying YCbCr info from the SharedTextureMemory with external format.
TEST_P(SharedTextureMemoryTests, QueryYCbCrInfoWithExternalFormat) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));

    AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    // Query the YCbCr properties of the AHardwareBuffer.
    auto deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
        .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
    };

    VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
    native::vulkan::PNextChainBuilder bufferPropertiesChain(&bufferProperties);
    bufferPropertiesChain.Add(&bufferFormatProperties,
                              VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

    VkDevice vkDevice = deviceVk->GetVkDevice();
    EXPECT_EQ(deviceVk->fn.GetAndroidHardwareBufferPropertiesANDROID(vkDevice, aHardwareBuffer,
                                                                     &bufferProperties),
              VK_SUCCESS);

    // Query the YCbCr properties of a SharedTextureMemory created from this
    // AHB.
    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;
    stmAHardwareBufferDesc.useExternalFormat = true;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::SharedTextureMemoryProperties properties;
    wgpu::SharedTextureMemoryAHardwareBufferProperties ahbProperties = {};
    properties.nextInChain = &ahbProperties;
    memory.GetProperties(&properties);
    auto yCbCrInfo = ahbProperties.yCbCrInfo;
    uint32_t formatFeatures = bufferFormatProperties.formatFeatures;

    // Verify that the YCbCr properties match.
    VkFormat expectedVkFormat = VK_FORMAT_UNDEFINED;
    EXPECT_EQ(expectedVkFormat, yCbCrInfo.vkFormat);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrModel, yCbCrInfo.vkYCbCrModel);
    EXPECT_EQ(bufferFormatProperties.suggestedYcbcrRange, yCbCrInfo.vkYCbCrRange);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.r,
              yCbCrInfo.vkComponentSwizzleRed);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.g,
              yCbCrInfo.vkComponentSwizzleGreen);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.b,
              yCbCrInfo.vkComponentSwizzleBlue);
    EXPECT_EQ(bufferFormatProperties.samplerYcbcrConversionComponents.a,
              yCbCrInfo.vkComponentSwizzleAlpha);
    EXPECT_EQ(bufferFormatProperties.suggestedXChromaOffset, yCbCrInfo.vkXChromaOffset);
    EXPECT_EQ(bufferFormatProperties.suggestedYChromaOffset, yCbCrInfo.vkYChromaOffset);

    wgpu::FilterMode expectedFilter =
        (formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)
            ? wgpu::FilterMode::Linear
            : wgpu::FilterMode::Nearest;
    EXPECT_EQ(expectedFilter, yCbCrInfo.vkChromaFilter);
    EXPECT_EQ(
        formatFeatures &
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
        yCbCrInfo.forceExplicitReconstruction);
    EXPECT_EQ(bufferFormatProperties.externalFormat, yCbCrInfo.externalFormat);
}

// Test BeginAccess on an uninitialized texture with external format fails.
TEST_P(SharedTextureMemoryTests, GPUReadForUninitializedTextureWithExternalFormatFails) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::YCbCrVulkanSamplers}));

    const AHardwareBuffer_Desc aHardwareBufferDesc = {
        .width = 4,
        .height = 4,
        .layers = 1,
        .format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
        .usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
    };
    AHardwareBuffer* aHardwareBuffer;
    EXPECT_EQ(AHardwareBuffer_allocate(&aHardwareBufferDesc, &aHardwareBuffer), 0);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor stmAHardwareBufferDesc;
    stmAHardwareBufferDesc.handle = aHardwareBuffer;
    stmAHardwareBufferDesc.useExternalFormat = true;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &stmAHardwareBufferDesc;

    const wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.size.depthOrArrayLayers = 1u;
    descriptor.sampleCount = 1u;
    descriptor.format = wgpu::TextureFormat::External;
    descriptor.mipLevelCount = 1u;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    auto texture = memory.CreateTexture(&descriptor);
    AHardwareBuffer_release(aHardwareBuffer);

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = false;
    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout{};
    beginDesc.nextInChain = &beginLayout;

    ASSERT_DEVICE_ERROR(memory.BeginAccess(texture, &beginDesc));
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(Vulkan,
                                 SharedTextureMemoryNoFeatureTests,
                                 {VulkanBackend()},
                                 {SharedTextureMemoryTestAndroidVulkanBackend::GetInstance()},
                                 {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(Vulkan,
                                 SharedTextureMemoryTests,
                                 {VulkanBackend()},
                                 {SharedTextureMemoryTestAndroidVulkanBackend::GetInstance()},
                                 {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    OpenGLES_SyncFD,
    SharedTextureMemoryNoFeatureTests,
    {OpenGLESBackend()},
    {SharedTextureMemoryTestAndroidSyncFDOpenGLESBackend::GetInstance()},
    {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    OpenGLES_SyncFD,
    SharedTextureMemoryTests,
    {OpenGLESBackend()},
    {SharedTextureMemoryTestAndroidSyncFDOpenGLESBackend::GetInstance()},
    {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    OpenGLES_EGLSync,
    SharedTextureMemoryNoFeatureTests,
    {OpenGLESBackend()},
    {SharedTextureMemoryTestAndroidEGLSyncOpenGLESBackend::GetInstance()},
    {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    OpenGLES_EGLSync,
    SharedTextureMemoryTests,
    {OpenGLESBackend()},
    {SharedTextureMemoryTestAndroidEGLSyncOpenGLESBackend::GetInstance()},
    {1});
}  // anonymous namespace
}  // namespace dawn
