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

#ifndef SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_SERVICEIMPLEMENTATION_H_
#define SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_SERVICEIMPLEMENTATION_H_

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/ExternalHandle.h"
#include "dawn/native/vulkan/external_memory/MemoryImportParams.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {
class Device;
struct VulkanDeviceInfo;
}  // namespace dawn::native::vulkan

namespace dawn::native::vulkan::external_memory {

class ServiceImplementation {
  public:
    explicit ServiceImplementation(Device* device);
    virtual ~ServiceImplementation();

    // True if the device reports it supports importing external memory.
    virtual bool SupportsImportMemory(VkFormat format,
                                      VkImageType type,
                                      VkImageTiling tiling,
                                      VkImageUsageFlags usage,
                                      VkImageCreateFlags flags) = 0;

    // True if the device reports it supports creating VkImages from external memory.
    virtual bool SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                                     VkFormat format,
                                     VkImageUsageFlags usage,
                                     bool* supportsDisjoint) = 0;

    // Returns the parameters required for importing memory
    virtual ResultOrError<MemoryImportParams> GetMemoryImportParams(
        const ExternalImageDescriptor* descriptor,
        VkImage image) = 0;

    // Returns the index of the queue memory from this services should be exported with.
    virtual uint32_t GetQueueFamilyIndex() = 0;

    // Given an external handle pointing to memory, import it into a VkDeviceMemory
    virtual ResultOrError<VkDeviceMemory> ImportMemory(ExternalMemoryHandle handle,
                                                       const MemoryImportParams& importParams,
                                                       VkImage image) = 0;

    // Create a VkImage for the given handle type
    virtual ResultOrError<VkImage> CreateImage(const ExternalImageDescriptor* descriptor,
                                               const VkImageCreateInfo& baseCreateInfo) = 0;

    // True if the device reports it supports this feature
    virtual bool Supported() const = 0;

  protected:
    bool RequiresDedicatedAllocation(const ExternalImageDescriptorVk* descriptor,
                                     VkImage image) const;

    raw_ptr<Device> mDevice = nullptr;
};

}  // namespace dawn::native::vulkan::external_memory

#endif  // SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_SERVICEIMPLEMENTATION_H_
