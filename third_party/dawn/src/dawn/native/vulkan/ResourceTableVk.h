// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_RESOURCETABLEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_RESOURCETABLEVK_H_

#include <memory>
#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/ResourceTable.h"

namespace dawn::native::vulkan {

class Device;
struct CommandRecordingContext;

class ResourceTable final : public ResourceTableBase {
  public:
    static ResultOrError<Ref<ResourceTable>> Create(Device* device,
                                                    const ResourceTableDescriptor* descriptor);

    // All the resource tables share the same VkDescriptorSetLayout so it is cached on the device.
    static ResultOrError<VkDescriptorSetLayout> MakeDescriptorSetLayout(Device* device);

    // Apply updates to resources or to the metadata buffers that are pending.
    MaybeError ApplyPendingUpdates(CommandRecordingContext* recordingContext);

    VkDescriptorSet GetHandle() const;

  protected:
    void DestroyImpl(DestroyReason reason) override;
    void SetLabelImpl() override;

  private:
    ~ResourceTable() override;

    using ResourceTableBase::ResourceTableBase;
    MaybeError Initialize();

    MaybeError UpdateMetadataBuffer(CommandRecordingContext* recordingContext,
                                    const std::vector<MetadataUpdate>& updates);
    MaybeError UpdateResourceBindings(const std::vector<ResourceUpdate>& updates);

    VkDescriptorPool mPool = VK_NULL_HANDLE;
    VkDescriptorSet mSet = VK_NULL_HANDLE;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RESOURCETABLEVK_H_
