// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.h"
#include "dawn/common/Assert.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementation.h"

namespace dawn::native::vulkan::external_memory {

class ServiceImplementationAHardwareBuffer : public ServiceImplementation {
  public:
    explicit ServiceImplementationAHardwareBuffer(Device* device)
        : ServiceImplementation(device), mSupported(CheckSupport(device->GetDeviceInfo())) {}
    ~ServiceImplementationAHardwareBuffer() override = default;

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo) {
        return deviceInfo.HasExt(DeviceExt::ExternalMemoryAndroidHardwareBuffer);
    }

    bool SupportsImportMemory(VkFormat format,
                              VkImageType type,
                              VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkImageCreateFlags flags) override {
        // Early out before we try using extension functions
        if (!mSupported) {
            return false;
        }

        VkPhysicalDeviceImageFormatInfo2 formatInfo = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR,
            .pNext = nullptr,
            .format = format,
            .type = type,
            .tiling = tiling,
            .usage = usage,
            .flags = flags,
        };

        VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo = {
            .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
        };

        PNextChainBuilder formatInfoChain(&formatInfo);
        formatInfoChain.Add(&externalFormatInfo,
                            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR);

        VkImageFormatProperties2 formatProperties = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR,
            .pNext = nullptr,
        };

        VkExternalImageFormatProperties externalFormatProperties;
        PNextChainBuilder formatPropertiesChain(&formatProperties);
        formatPropertiesChain.Add(&externalFormatProperties,
                                  VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR);

        VkResult result = VkResult::WrapUnsafe(mDevice->fn.GetPhysicalDeviceImageFormatProperties2(
            ToBackend(mDevice->GetPhysicalDevice())->GetVkPhysicalDevice(), &formatInfo,
            &formatProperties));

        // If handle not supported, result == VK_ERROR_FORMAT_NOT_SUPPORTED
        if (result != VK_SUCCESS) {
            return false;
        }

        // TODO(http://crbug.com/dawn/206): Investigate dedicated only images
        VkFlags memoryFlags =
            externalFormatProperties.externalMemoryProperties.externalMemoryFeatures;
        return (memoryFlags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR) != 0;
    }

    bool SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                             VkFormat format,
                             VkImageUsageFlags usage,
                             bool* supportsDisjoint) override {
        *supportsDisjoint = false;
        return mSupported;
    }

    ResultOrError<MemoryImportParams> GetMemoryImportParams(
        const ExternalImageDescriptor* descriptor,
        VkImage image) override {
        DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::AHardwareBuffer,
                        "ExternalImageDescriptor is not an AHardwareBuffer descriptor.");

        const ExternalImageDescriptorAHardwareBuffer* aHardwareBufferDescriptor =
            static_cast<const ExternalImageDescriptorAHardwareBuffer*>(descriptor);

        VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {
            .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
            .pNext = nullptr,
        };

        PNextChainBuilder bufferPropertiesChain(&bufferProperties);

        VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties;
        bufferPropertiesChain.Add(
            &bufferFormatProperties,
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

        DAWN_TRY(CheckVkSuccess(
            mDevice->fn.GetAndroidHardwareBufferPropertiesANDROID(
                mDevice->GetVkDevice(), aHardwareBufferDescriptor->handle, &bufferProperties),
            "vkGetAndroidHardwareBufferPropertiesANDROID"));

        MemoryImportParams params;
        params.allocationSize = bufferProperties.allocationSize;
        params.memoryTypeIndex = bufferProperties.memoryTypeBits;
        params.dedicatedAllocation = RequiresDedicatedAllocation(aHardwareBufferDescriptor, image);
        return params;
    }

    uint32_t GetQueueFamilyIndex() override { return VK_QUEUE_FAMILY_FOREIGN_EXT; }

    ResultOrError<VkDeviceMemory> ImportMemory(ExternalMemoryHandle handle,
                                               const MemoryImportParams& importParams,
                                               VkImage image) override {
        DAWN_INVALID_IF(handle == nullptr, "Importing memory with an invalid handle.");

        VkMemoryRequirements requirements;
        mDevice->fn.GetImageMemoryRequirements(mDevice->GetVkDevice(), image, &requirements);
        DAWN_INVALID_IF(requirements.size > importParams.allocationSize,
                        "Requested allocation size (%u) is smaller than the image requires (%u).",
                        importParams.allocationSize, requirements.size);

        VkMemoryAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = importParams.allocationSize,
            .memoryTypeIndex = importParams.memoryTypeIndex,
        };

        PNextChainBuilder allocateInfoChain(&allocateInfo);

        VkImportAndroidHardwareBufferInfoANDROID importMemoryAHBInfo = {
            .buffer = handle,
        };
        allocateInfoChain.Add(&importMemoryAHBInfo,
                              VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID);

        VkMemoryDedicatedAllocateInfo dedicatedAllocateInfo;
        if (importParams.dedicatedAllocation) {
            dedicatedAllocateInfo.image = image;
            dedicatedAllocateInfo.buffer = VkBuffer{};
            allocateInfoChain.Add(&dedicatedAllocateInfo,
                                  VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO);
        }

        VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;
        DAWN_TRY(CheckVkSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &allocateInfo,
                                                           nullptr, &*allocatedMemory),
                                "vkAllocateMemory"));
        return allocatedMemory;
    }

    ResultOrError<VkImage> CreateImage(const ExternalImageDescriptor* descriptor,
                                       const VkImageCreateInfo& baseCreateInfo) override {
        VkImageCreateInfo createInfo = baseCreateInfo;
        createInfo.flags |= VK_IMAGE_CREATE_ALIAS_BIT_KHR;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        PNextChainBuilder createInfoChain(&createInfo);

        VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
        };
        createInfoChain.Add(&externalMemoryImageCreateInfo,
                            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);

        DAWN_ASSERT(IsSampleCountSupported(mDevice, createInfo));

        VkImage image;
        DAWN_TRY(CheckVkSuccess(
            mDevice->fn.CreateImage(mDevice->GetVkDevice(), &createInfo, nullptr, &*image),
            "CreateImage"));
        return image;
    }

    bool Supported() const override { return mSupported; }

  private:
    bool mSupported = false;
};

bool CheckAHardwareBufferSupport(const VulkanDeviceInfo& deviceInfo) {
    return ServiceImplementationAHardwareBuffer::CheckSupport(deviceInfo);
}

std::unique_ptr<ServiceImplementation> CreateAHardwareBufferService(Device* device) {
    return std::make_unique<ServiceImplementationAHardwareBuffer>(device);
}

}  // namespace dawn::native::vulkan::external_memory
