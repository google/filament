// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_MEMORYSERVICE_H_
#define SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_MEMORYSERVICE_H_

#include <memory>

#include "dawn/common/ityp_array.h"
#include "dawn/native/Error.h"
#include "dawn/native/vulkan/ExternalHandle.h"
#include "dawn/native/vulkan/external_memory/MemoryImportParams.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementation.h"

namespace dawn::native::vulkan {
class Device;
struct VulkanDeviceInfo;
}  // namespace dawn::native::vulkan

namespace dawn::native::vulkan::external_memory {

class Service {
  public:
    explicit Service(Device* device);
    ~Service();

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo);

    // True if the device reports it supports importing external memory.
    bool SupportsImportMemory(ExternalImageType externalImageType,
                              VkFormat format,
                              VkImageType type,
                              VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkImageCreateFlags flags);

    // True if the device reports it supports creating VkImages from external memory.
    bool SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                             VkFormat format,
                             VkImageUsageFlags usage,
                             bool* supportsDisjoint);

    // Returns the parameters required for importing memory
    ResultOrError<MemoryImportParams> GetMemoryImportParams(
        const ExternalImageDescriptor* descriptor,
        VkImage image);

    // Returns the index of the queue memory from this services should be exported with.
    uint32_t GetQueueFamilyIndex(ExternalImageType externalImageType);

    // Given an external handle pointing to memory, import it into a VkDeviceMemory
    ResultOrError<VkDeviceMemory> ImportMemory(ExternalImageType externalImageType,
                                               ExternalMemoryHandle handle,
                                               const MemoryImportParams& importParams,
                                               VkImage image);

    // Create a VkImage for the given handle type
    ResultOrError<VkImage> CreateImage(const ExternalImageDescriptor* descriptor,
                                       const VkImageCreateInfo& baseCreateInfo);

  private:
    ServiceImplementation* GetServiceImplementation(ExternalImageType externalImageType);

    // ExternalImageType has 7 types. Vulkan backend uses 3 of them.
    ityp::array<ExternalImageType,
                std::unique_ptr<ServiceImplementation>,
                ExternalImageType::Last + 1>
        mServiceImpls = {};
};

}  // namespace dawn::native::vulkan::external_memory

#endif  // SRC_DAWN_NATIVE_VULKAN_EXTERNAL_MEMORY_MEMORYSERVICE_H_
