// Copyright 2021 The Dawn & Tint Authors
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

#include <unistd.h>

#include <memory>
#include <utility>
#include <vector>

#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/tests/white_box/VulkanImageWrappingTests_OpaqueFD.h"
#include "gtest/gtest.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

class ExternalSemaphoreOpaqueFD : public VulkanImageWrappingTestBackend::ExternalSemaphore {
  public:
    explicit ExternalSemaphoreOpaqueFD(int handle) : mHandle(handle) {}
    ~ExternalSemaphoreOpaqueFD() override {
        if (mHandle != -1) {
            close(mHandle);
        }
    }
    int AcquireHandle() {
        int handle = mHandle;
        mHandle = -1;
        return handle;
    }

  private:
    int mHandle = -1;
};

class ExternalTextureOpaqueFD : public VulkanImageWrappingTestBackend::ExternalTexture {
  public:
    ExternalTextureOpaqueFD(native::vulkan::Device* device,
                            int fd,
                            VkDeviceMemory allocation,
                            VkImage handle,
                            VkDeviceSize allocationSize,
                            uint32_t memoryTypeIndex)
        : mDevice(device),
          mFd(fd),
          mAllocation(allocation),
          mHandle(handle),
          allocationSize(allocationSize),
          memoryTypeIndex(memoryTypeIndex) {}

    ~ExternalTextureOpaqueFD() override {
        if (mFd != -1) {
            close(mFd);
        }
        if (mAllocation != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mAllocation);
        }
        if (mHandle != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        }
    }

    int Dup() const { return dup(mFd); }

  private:
    raw_ptr<native::vulkan::Device> mDevice;
    int mFd = -1;
    VkDeviceMemory mAllocation = VK_NULL_HANDLE;
    VkImage mHandle = VK_NULL_HANDLE;

  public:
    const VkDeviceSize allocationSize;
    const uint32_t memoryTypeIndex;
};

class VulkanImageWrappingTestBackendOpaqueFD : public VulkanImageWrappingTestBackend {
  public:
    explicit VulkanImageWrappingTestBackendOpaqueFD(const wgpu::Device& device) : mDevice(device) {
        mDeviceVk = native::vulkan::ToBackend(native::FromAPI(device.Get()));
    }

    bool SupportsTestParams(const TestParams& params) const override {
        return mDeviceVk->GetDeviceInfo().HasExt(DeviceExt::ExternalMemoryFD) &&
               (!params.useDedicatedAllocation ||
                mDeviceVk->GetDeviceInfo().HasExt(DeviceExt::DedicatedAllocation));
    }

    std::unique_ptr<ExternalTexture> CreateTexture(uint32_t width,
                                                   uint32_t height,
                                                   wgpu::TextureFormat format,
                                                   wgpu::TextureUsage usage) override {
        EXPECT_EQ(format, wgpu::TextureFormat::RGBA8Unorm);
        VkFormat vulkanFormat = VK_FORMAT_R8G8B8A8_UNORM;

        VkImage handle;
        ::VkResult result = CreateImage(mDeviceVk, width, height, vulkanFormat, &handle);
        EXPECT_EQ(result, VK_SUCCESS) << "Failed to create external image";

        VkDeviceMemory allocation;
        VkDeviceSize allocationSize;
        uint32_t memoryTypeIndex;
        ::VkResult resultBool =
            AllocateMemory(mDeviceVk, handle, &allocation, &allocationSize, &memoryTypeIndex);
        EXPECT_EQ(resultBool, VK_SUCCESS) << "Failed to allocate external memory";

        result = BindMemory(mDeviceVk, handle, allocation);
        EXPECT_EQ(result, VK_SUCCESS) << "Failed to bind image memory";

        int fd = GetMemoryFd(mDeviceVk, allocation);

        return std::make_unique<ExternalTextureOpaqueFD>(mDeviceVk, fd, allocation, handle,
                                                         allocationSize, memoryTypeIndex);
    }

    wgpu::Texture WrapImage(const wgpu::Device& device,
                            const ExternalTexture* texture,
                            const ExternalImageDescriptorVkForTesting& descriptor,
                            std::vector<std::unique_ptr<ExternalSemaphore>> semaphores) override {
        const ExternalTextureOpaqueFD* textureOpaqueFD =
            static_cast<const ExternalTextureOpaqueFD*>(texture);
        std::vector<int> waitFDs;
        for (auto& semaphore : semaphores) {
            waitFDs.push_back(
                static_cast<ExternalSemaphoreOpaqueFD*>(semaphore.get())->AcquireHandle());
        }

        ExternalImageDescriptorOpaqueFD descriptorOpaqueFD;
        *static_cast<ExternalImageDescriptorVk*>(&descriptorOpaqueFD) = descriptor;
        descriptorOpaqueFD.memoryFD = textureOpaqueFD->Dup();
        descriptorOpaqueFD.allocationSize = textureOpaqueFD->allocationSize;
        descriptorOpaqueFD.memoryTypeIndex = textureOpaqueFD->memoryTypeIndex;
        descriptorOpaqueFD.waitFDs = std::move(waitFDs);

        if (GetParam().detectDedicatedAllocation) {
            descriptorOpaqueFD.dedicatedAllocation = NeedsDedicatedAllocation::Detect;
        } else if (GetParam().useDedicatedAllocation) {
            descriptorOpaqueFD.dedicatedAllocation = NeedsDedicatedAllocation::Yes;
        } else {
            descriptorOpaqueFD.dedicatedAllocation = NeedsDedicatedAllocation::No;
        }

        return wgpu::Texture::Acquire(
            native::vulkan::WrapVulkanImage(device.Get(), &descriptorOpaqueFD));
    }

    bool ExportImage(const wgpu::Texture& texture,
                     ExternalImageExportInfoVkForTesting* exportInfo) override {
        ExternalImageExportInfoOpaqueFD infoOpaqueFD;
        bool success = ExportVulkanImage(texture.Get(), VK_IMAGE_LAYOUT_UNDEFINED, &infoOpaqueFD);

        *static_cast<ExternalImageExportInfoVk*>(exportInfo) = infoOpaqueFD;
        for (int fd : infoOpaqueFD.semaphoreHandles) {
            EXPECT_NE(fd, -1);
            exportInfo->semaphores.push_back(std::make_unique<ExternalSemaphoreOpaqueFD>(fd));
        }

        return success;
    }

  private:
    // Creates a VkImage with external memory
    ::VkResult CreateImage(native::vulkan::Device* deviceVk,
                           uint32_t width,
                           uint32_t height,
                           VkFormat format,
                           VkImage* image) {
        auto usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkImageCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_IMAGE_CREATE_ALIAS_BIT_KHR;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.format = format;
        createInfo.extent = {width, height, 1};
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkExternalMemoryImageCreateInfoKHR externalInfo;
        externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        PNextChainBuilder createChain(&createInfo);
        createChain.Add(&externalInfo, VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR);

        return deviceVk->fn.CreateImage(deviceVk->GetVkDevice(), &createInfo, nullptr, &**image);
    }

    // Allocates memory for an image
    ::VkResult AllocateMemory(native::vulkan::Device* deviceVk,
                              VkImage handle,
                              VkDeviceMemory* allocation,
                              VkDeviceSize* allocationSize,
                              uint32_t* memoryTypeIndex) {
        // Create the image memory and associate it with the container
        VkMemoryRequirements requirements;
        deviceVk->fn.GetImageMemoryRequirements(deviceVk->GetVkDevice(), handle, &requirements);

        int bestType = deviceVk->GetResourceMemoryAllocator()->FindBestTypeIndex(
            requirements, MemoryKind::DeviceLocal);

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = static_cast<uint32_t>(bestType);

        // Import memory from file descriptor
        VkExportMemoryAllocateInfoKHR externalInfo;
        externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        PNextChainBuilder allocateChain(&allocateInfo);
        allocateChain.Add(&externalInfo, VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR);

        // Use a dedicated memory allocation if testing that path.
        VkMemoryDedicatedAllocateInfo dedicatedInfo;
        if (GetParam().useDedicatedAllocation) {
            dedicatedInfo.image = handle;
            dedicatedInfo.buffer = VkBuffer{};
            allocateChain.Add(&dedicatedInfo, VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO);
        }

        *allocationSize = allocateInfo.allocationSize;
        *memoryTypeIndex = allocateInfo.memoryTypeIndex;

        return deviceVk->fn.AllocateMemory(deviceVk->GetVkDevice(), &allocateInfo, nullptr,
                                           &**allocation);
    }

    // Binds memory to an image
    ::VkResult BindMemory(native::vulkan::Device* deviceVk, VkImage handle, VkDeviceMemory memory) {
        return deviceVk->fn.BindImageMemory(deviceVk->GetVkDevice(), handle, memory, 0);
    }

    // Extracts a file descriptor representing memory on a device
    int GetMemoryFd(native::vulkan::Device* deviceVk, VkDeviceMemory memory) {
        VkMemoryGetFdInfoKHR getFdInfo;
        getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
        getFdInfo.pNext = nullptr;
        getFdInfo.memory = memory;
        getFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

        int memoryFd = -1;
        deviceVk->fn.GetMemoryFdKHR(deviceVk->GetVkDevice(), &getFdInfo, &memoryFd);

        EXPECT_GE(memoryFd, 0) << "Failed to get file descriptor for external memory";
        return memoryFd;
    }

    // Prepares and exports memory for an image on a given device
    void CreateBindExportImage(native::vulkan::Device* deviceVk,
                               uint32_t width,
                               uint32_t height,
                               VkFormat format,
                               VkImage* handle,
                               VkDeviceMemory* allocation,
                               VkDeviceSize* allocationSize,
                               uint32_t* memoryTypeIndex,
                               int* memoryFd) {}

    wgpu::Device mDevice;
    raw_ptr<native::vulkan::Device> mDeviceVk;
};

std::unique_ptr<VulkanImageWrappingTestBackend> CreateOpaqueFDBackend(const wgpu::Device& device) {
    return std::make_unique<VulkanImageWrappingTestBackendOpaqueFD>(device);
}

}  // namespace dawn::native::vulkan
