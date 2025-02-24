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

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/tests/white_box/SharedTextureMemoryTests.h"

namespace dawn::native::vulkan {
namespace {

template <typename CreateFn, typename... AdditionalChains>
auto CreateSharedTextureMemoryHelperImpl(native::vulkan::Device* deviceVk,
                                         uint32_t size,
                                         VkFormat format,
                                         VkImageUsageFlags usage,
                                         VkImageCreateFlagBits createFlags,
                                         bool dedicatedAllocation,
                                         CreateFn createFn,
                                         AdditionalChains*... additionalChains) {
    VkExternalMemoryImageCreateInfo externalInfo{};
    externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = &externalInfo;
    createInfo.flags = createFlags;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = format;
    createInfo.extent = {size, size, 1};
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    PNextChainBuilder createInfoChain(&createInfo);
    (createInfoChain.Add(additionalChains), ...);

    VkImage vkImage;
    EXPECT_EQ(deviceVk->fn.CreateImage(deviceVk->GetVkDevice(), &createInfo, nullptr, &*vkImage),
              VK_SUCCESS);

    // Create the image memory and associate it with the container
    VkMemoryRequirements requirements;
    deviceVk->fn.GetImageMemoryRequirements(deviceVk->GetVkDevice(), vkImage, &requirements);

    int bestType = deviceVk->GetResourceMemoryAllocator()->FindBestTypeIndex(
        requirements, native::vulkan::MemoryKind::Opaque);
    EXPECT_GE(bestType, 0);

    VkMemoryDedicatedAllocateInfo dedicatedInfo;
    dedicatedInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedInfo.pNext = nullptr;
    dedicatedInfo.image = vkImage;
    dedicatedInfo.buffer = VkBuffer{};

    VkExportMemoryAllocateInfoKHR externalAllocateInfo;
    externalAllocateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
    externalAllocateInfo.pNext = dedicatedAllocation ? &dedicatedInfo : nullptr;
    externalAllocateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = &externalAllocateInfo;
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = static_cast<uint32_t>(bestType);

    VkDeviceMemory vkDeviceMemory;
    EXPECT_EQ(deviceVk->fn.AllocateMemory(deviceVk->GetVkDevice(), &allocateInfo, nullptr,
                                          &*vkDeviceMemory),
              VK_SUCCESS);

    EXPECT_EQ(deviceVk->fn.BindImageMemory(deviceVk->GetVkDevice(), vkImage, vkDeviceMemory, 0),
              VK_SUCCESS);

    VkMemoryGetFdInfoKHR getFdInfo;
    getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    getFdInfo.pNext = nullptr;
    getFdInfo.memory = vkDeviceMemory;
    getFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    int memoryFD = -1;
    deviceVk->fn.GetMemoryFdKHR(deviceVk->GetVkDevice(), &getFdInfo, &memoryFD);
    EXPECT_GE(memoryFD, 0) << "Failed to get file descriptor for external memory";

    wgpu::SharedTextureMemoryOpaqueFDDescriptor opaqueFDDesc;
    opaqueFDDesc.vkImageCreateInfo = &createInfo;
    opaqueFDDesc.memoryFD = memoryFD;
    opaqueFDDesc.memoryTypeIndex = allocateInfo.memoryTypeIndex;
    opaqueFDDesc.allocationSize = allocateInfo.allocationSize;
    opaqueFDDesc.dedicatedAllocation = dedicatedAllocation;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &opaqueFDDesc;

    std::string label;
    label += "size: " + std::to_string(size);
    label += " format: " + std::to_string(format);
    label += " usage: " + std::to_string(usage);
    label += " createFlags: " + std::to_string(createFlags);
    label += " dedicatedAllocation: " + std::to_string(dedicatedAllocation);

    auto ret = createFn(&desc);

    close(memoryFD);
    deviceVk->GetFencedDeleter()->DeleteWhenUnused(vkDeviceMemory);
    deviceVk->GetFencedDeleter()->DeleteWhenUnused(vkImage);

    return ret;
}

template <typename CreateFn, typename... AdditionalChains>
auto CreateSharedTextureMemoryHelperImpl(native::vulkan::Device* deviceVk,
                                         uint32_t size,
                                         VkFormat format,
                                         VkImageUsageFlags usage,
                                         CreateFn createFn,
                                         AdditionalChains*... additionalChains) {
    return CreateSharedTextureMemoryHelperImpl(deviceVk, size, format, usage,
                                               VkImageCreateFlagBits(0), false, createFn,
                                               additionalChains...);
}

bool CheckFormatSupport(native::vulkan::Device* deviceVk,
                        VkFormat format,
                        VkImageUsageFlags usage,
                        VkImageCreateFlagBits createFlags = VkImageCreateFlagBits(0)) {
    VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo;
    externalFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR;
    externalFormatInfo.pNext = nullptr;
    externalFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkPhysicalDeviceImageFormatInfo2 formatInfo;
    formatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR;
    formatInfo.pNext = &externalFormatInfo;
    formatInfo.format = format;
    formatInfo.type = VK_IMAGE_TYPE_2D;
    formatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    formatInfo.usage = usage;
    formatInfo.flags = createFlags;

    VkExternalImageFormatProperties externalFormatProperties;
    externalFormatProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR;
    externalFormatProperties.pNext = nullptr;

    VkImageFormatProperties2 formatProperties;
    formatProperties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR;
    formatProperties.pNext = &externalFormatProperties;

    VkExternalMemoryImageCreateInfo externalInfo{};
    externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    return VkResult::WrapUnsafe(deviceVk->fn.GetPhysicalDeviceImageFormatProperties2(
               ToBackend(deviceVk->GetPhysicalDevice())->GetVkPhysicalDevice(), &formatInfo,
               &formatProperties)) == VK_SUCCESS;
}

template <wgpu::FeatureName FenceFeature, bool DedicatedAllocation>
class Backend : public SharedTextureMemoryTestVulkanBackend {
  public:
    static SharedTextureMemoryTestBackend* GetInstance() {
        static Backend b;
        return &b;
    }

    std::string Name() const override {
        std::string name = "OpaqueFD";
        if (DedicatedAllocation) {
            name += ", DedicatedAlloc";
        }
        switch (FenceFeature) {
            case wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD:
                name += ", OpaqueFDFence";
                break;
            case wgpu::FeatureName::SharedFenceSyncFD:
                name += ", SyncFDFence";
                break;
            default:
                DAWN_UNREACHABLE();
        }
        return name;
    }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter&) const override {
        return {wgpu::FeatureName::SharedTextureMemoryOpaqueFD,
                wgpu::FeatureName::DawnMultiPlanarFormats, FenceFeature};
    }

    bool SupportsConcurrentRead() const override {
        return FenceFeature != wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD;
    }

    template <typename CreateFn>
    auto CreateSharedTextureMemoryHelper(native::vulkan::Device* deviceVk,
                                         uint32_t size,
                                         VkFormat format,
                                         VkImageUsageFlags usage,
                                         CreateFn createFn) {
        VkImageCreateFlagBits flags{};
        if (format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM) {
            // Needed for view format reinterpretation.
            flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        }
        return CreateSharedTextureMemoryHelperImpl(deviceVk, size, format, usage, flags,
                                                   DedicatedAllocation, createFn);
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        return CreateSharedTextureMemoryHelper(
            native::vulkan::ToBackend(native::FromAPI(device.Get())), 16, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_STORAGE_BIT,
            [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                return device.ImportSharedTextureMemory(desc);
            });
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices,
        int layerCount) override {
        DAWN_ASSERT(!devices.empty());

        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;
        for (VkFormat format : {
                 VK_FORMAT_R8_UNORM,
                 VK_FORMAT_R8G8_UNORM,
                 VK_FORMAT_R8G8B8A8_UNORM,
                 VK_FORMAT_B8G8R8A8_UNORM,
                 VK_FORMAT_A2B10G10R10_UNORM_PACK32,
             }) {
            for (VkImageUsageFlags usage : {
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                         VK_IMAGE_USAGE_STORAGE_BIT,
                 }) {
                auto* deviceVk = native::vulkan::ToBackend(native::FromAPI(devices[0].Get()));
                if (!CheckFormatSupport(deviceVk, format, usage)) {
                    // Skip this format if it is not supported.
                    continue;
                }

                for (uint32_t size : {4, 64}) {
                    CreateSharedTextureMemoryHelper(
                        deviceVk, size, format, usage,
                        [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                            std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                            for (auto& device : devices) {
                                perDeviceMemories.push_back(device.ImportSharedTextureMemory(desc));
                            }
                            memories.push_back(std::move(perDeviceMemories));
                            return true;
                        });
                }
            }
        }
        return memories;
    }
};

class SharedTextureMemoryOpaqueFDValidationTest : public SharedTextureMemoryTests {};
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedTextureMemoryOpaqueFDValidationTest);

// Test that the Vulkan image must be created with VK_IMAGE_USAGE_TRANSFER_DST_BIT.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, RequiresCopyDst) {
    native::vulkan::Device* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    // Test that including TRANSFER_DST is not an error.
    CreateSharedTextureMemoryHelperImpl(
        deviceVk, 4, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
            device.ImportSharedTextureMemory(desc);
            return true;
        });

    // Test that excluding TRANSFER_DST is an error.
    CreateSharedTextureMemoryHelperImpl(
        deviceVk, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
            ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                    testing::HasSubstr("TRANSFER_DST"));
            return true;
        });
}

// Test requirements for the Vulkan image if it is BGRA8Unorm.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, BGRA8UnormStorageRequirements) {
    native::vulkan::Device* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
    DAWN_TEST_UNSUPPORTED_IF(!CheckFormatSupport(deviceVk, VK_FORMAT_B8G8R8A8_UNORM,
                                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                     VK_IMAGE_USAGE_STORAGE_BIT,
                                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT));

    // Test that including MUTABLE_FORMAT_BIT is valid.
    CreateSharedTextureMemoryHelperImpl(deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                            VK_IMAGE_USAGE_STORAGE_BIT,
                                        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                                        [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                            device.ImportSharedTextureMemory(desc);
                                            return true;
                                        });

    // Test that excluding MUTABLE_FORMAT_BIT is invalid.
    CreateSharedTextureMemoryHelperImpl(
        deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT,
        VkImageCreateFlagBits(0), false, [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
            ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                    testing::HasSubstr("MUTABLE_FORMAT_BIT"));
            return true;
        });

    // Test that excluding MUTABLE_FORMAT_BIT if STORAGE_BIT is not present is valid.
    CreateSharedTextureMemoryHelperImpl(
        deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkImageCreateFlagBits(0),
        false, [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
            device.ImportSharedTextureMemory(desc);
            return true;
        });

    // Test that including MUTABLE_FORMAT_BIT if STORAGE_BIT is not present is valid.
    CreateSharedTextureMemoryHelperImpl(
        deviceVk, 4, VK_FORMAT_B8G8R8A8_UNORM,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
        [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
            device.ImportSharedTextureMemory(desc);
            return true;
        });
}

// Test requirements for the Vulkan image if it is may need view format reinterpretation.
TEST_P(SharedTextureMemoryOpaqueFDValidationTest, ViewFormatRequirements) {
    native::vulkan::Device* deviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));

    std::array<VkFormat, 2> vkFormats = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB};
    for (VkFormat vkFormat : vkFormats) {
        // Test that including MUTABLE_FORMAT_BIT is valid.
        CreateSharedTextureMemoryHelperImpl(deviceVk, 4, vkFormat,
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                            VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                                            [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                                                device.ImportSharedTextureMemory(desc);
                                                return true;
                                            });

        // Test that excluding MUTABLE_FORMAT_BIT is invalid.
        CreateSharedTextureMemoryHelperImpl(
            deviceVk, 4, vkFormat,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkImageCreateFlagBits(0), false, [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                        testing::HasSubstr("MUTABLE_FORMAT_BIT"));
                return true;
            });

        // Test that including MUTABLE_FORMAT_BIT if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is not
        // present is valid.
        CreateSharedTextureMemoryHelperImpl(
            deviceVk, 4, vkFormat,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
            [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                device.ImportSharedTextureMemory(desc);
                return true;
            });

        // Test that excluding MUTABLE_FORMAT_BIT if VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is not
        // present is valid.
        CreateSharedTextureMemoryHelperImpl(
            deviceVk, 4, vkFormat,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VkImageCreateFlagBits(0), false, [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                device.ImportSharedTextureMemory(desc);
                return true;
            });

        // Test that if the image format list is provided, all of the vkFormats must be listed.
        VkImageFormatListCreateInfo imageFormatListInfo;
        imageFormatListInfo.pNext = nullptr;
        imageFormatListInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
        {
            // Passing all of them is valid.
            imageFormatListInfo.pViewFormats = vkFormats.data();
            imageFormatListInfo.viewFormatCount = vkFormats.size();

            CreateSharedTextureMemoryHelperImpl(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    device.ImportSharedTextureMemory(desc);
                    return true;
                },
                &imageFormatListInfo);
        }
        {
            // Passing the first is invalid.
            imageFormatListInfo.pViewFormats = vkFormats.data();
            imageFormatListInfo.viewFormatCount = 1;

            CreateSharedTextureMemoryHelperImpl(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                            testing::HasSubstr("VkImageFormatCreateInfo did not"));
                    return true;
                },
                &imageFormatListInfo);
        }
        {
            // Passing the second is invalid.
            imageFormatListInfo.pViewFormats = vkFormats.data() + 1;
            imageFormatListInfo.viewFormatCount = 1;

            CreateSharedTextureMemoryHelperImpl(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                            testing::HasSubstr("VkImageFormatCreateInfo did not"));
                    return true;
                },
                &imageFormatListInfo);
        }
        {
            // Passing none is invalid.
            imageFormatListInfo.pViewFormats = nullptr;
            imageFormatListInfo.viewFormatCount = 0;

            CreateSharedTextureMemoryHelperImpl(
                deviceVk, 4, vkFormat,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, false,
                [&](const wgpu::SharedTextureMemoryDescriptor* desc) {
                    ASSERT_DEVICE_ERROR_MSG(device.ImportSharedTextureMemory(desc),
                                            testing::HasSubstr("VkImageFormatCreateInfo did not"));
                    return true;
                },
                &imageFormatListInfo);
        }
    }
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryNoFeatureTests,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, false>::GetInstance()},
    {1});

// Only test DedicatedAllocation == false because validation never actually creates an allocation.
// Passing true wouldn't give extra coverage.
DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryOpaqueFDValidationTest,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, false>::GetInstance()},
    {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(
    Vulkan,
    SharedTextureMemoryTests,
    {VulkanBackend()},
    {Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, false>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceVkSemaphoreOpaqueFD, true>::GetInstance(),
     Backend<wgpu::FeatureName::SharedFenceSyncFD, true>::GetInstance()},
    {1});

}  // anonymous namespace
}  // namespace dawn::native::vulkan
