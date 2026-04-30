// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_

#include "absl/container/flat_hash_set.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ResourceMemoryAllocation.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;
struct VulkanFunctions;

// Holds the parameters for a buffer related pipeline memory barrier. Must be
// submitted via CommandRecordingContext.
struct BufferBarrier {
    bool IsEmpty() const;
    void Merge(const BufferBarrier& other);

    VkAccessFlags srcAccessMask = 0;
    VkAccessFlags dstAccessMask = 0;
    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;
};

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device,
                                             const UnpackedPtr<BufferDescriptor>& descriptor);

    VkBuffer GetHandle() const;

    // Transitions the buffer to be used as `usage`, recording any necessary barrier in
    // `commands`.
    // TODO(crbug.com/dawn/851): coalesce barriers and do them early when possible.
    void TransitionUsageNow(CommandRecordingContext* recordingContext,
                            wgpu::BufferUsage usage,
                            wgpu::ShaderStage shaderStage = wgpu::ShaderStage::None);

    // Tracks that buffer had `usage` from `shaderStage`. Returns a barrier to be inserted if
    // necessary based on previous usage. For map usage this returns a GPU->HOST barrier if
    // necessary but doesn't track the map usage. As a result this will never produce HOST->GPU
    // barriers.
    BufferBarrier TrackUsageAndGetResourceBarrier(wgpu::BufferUsage usage,
                                                  wgpu::ShaderStage shaderStage);

    // All the Ensure methods return true if the buffer was initialized to zero.
    bool EnsureDataInitialized(CommandRecordingContext* recordingContext);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                            uint64_t offset,
                                            uint64_t size);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                            const CopyTextureToBufferCmd* copy);

    // Dawn API
    void SetLabelImpl() override;

    static void TransitionMappableBuffersEagerly(Device* device,
                                                 CommandRecordingContext* recordingContext,
                                                 const absl::flat_hash_set<Ref<Buffer>>& buffers);

  private:
    ~Buffer() override;
    using BufferBase::BufferBase;

    MaybeError Initialize(bool mappedAtCreation);
    MaybeError InitializeHostMapped(const BufferHostMappedPointer* hostMappedDesc);
    void InitializeToZero(CommandRecordingContext* recordingContext);
    void ClearBuffer(CommandRecordingContext* recordingContext,
                     uint32_t clearValue,
                     uint64_t offset = 0,
                     uint64_t size = 0);

    // Maps buffer memory to perform some operation, eg. initialization or upload. The memory will
    // be mapped/invalidated if necessary, `op` function will run and then memory will
    // unmapped/flushed if necessary. The op function receives a span of exactly the requested
    // size.
    template <typename F>
    MaybeError MapMemoryAndPerformOperation(uint64_t requestedOffset, size_t requestedSize, F&& op);

    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    MaybeError FinalizeMapImpl(BufferState newState) override;
    void UnmapImpl(BufferState oldState, BufferState newState) override;
    void DestroyImpl(DestroyReason reason) override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointerImpl() override;
    MaybeError UploadData(uint64_t bufferOffset, const void* data, size_t size) override;

    VkBuffer mHandle = VK_NULL_HANDLE;
    ResourceMemoryAllocation mMemoryAllocation;

    // VkDeviceMemory that is used strictly for this buffer.
    VkDeviceMemory mDedicatedDeviceMemory = VK_NULL_HANDLE;

    wgpu::Callback mHostMappedDisposeCallback = nullptr;
    raw_ptr<void, DisableDanglingPtrDetection> mHostMappedDisposeUserdata = nullptr;

    // Track which usage was the last to write to the buffer.
    wgpu::BufferUsage mLastWriteUsage = wgpu::BufferUsage::None;
    wgpu::ShaderStage mLastWriteShaderStage = wgpu::ShaderStage::None;

    // Track which usages have read the buffer since the last write.
    wgpu::BufferUsage mReadUsage = wgpu::BufferUsage::None;
    wgpu::ShaderStage mReadShaderStages = wgpu::ShaderStage::None;

    bool mHostVisible : 1 = false;
    bool mHostCoherent : 1 = false;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_
