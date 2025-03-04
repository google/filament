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
#ifndef SRC_DAWN_NATIVE_VULKAN_COMMANDRECORDINGCONTEXT_H_
#define SRC_DAWN_NATIVE_VULKAN_COMMANDRECORDINGCONTEXT_H_

#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/VulkanFunctions.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

class ImportedTextureBase;

// Wrapping class that currently associates a command buffer to it's corresponding pool.
// TODO(dawn:1601) Revisit this structure since it is where the 1:1 mapping is implied.
//                 Also consider reusing this in CommandRecordingContext below instead of
//                 flattening the pool and command buffer again.
struct CommandPoolAndBuffer {
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};

// Used to track operations that are handled after recording.
// Currently only tracks semaphores, but may be used to do barrier coalescing in the future.
struct CommandRecordingContext {
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    std::vector<VkSemaphore> waitSemaphores = {};
    std::vector<VkSemaphore> signalSemaphores = {};

    // The internal buffers used in the workaround of texture-to-texture copies with compressed
    // formats.
    std::vector<Ref<Buffer>> tempBuffers;

    // Textures can have special synchronization requirements that need to be handled during submit.
    // They are tracked here to avoid walking all the textures during submission. It is ok to keep a
    // raw_ptr as they are kept alive by the CommandBuffer. Special synchronization can be:
    //
    //  - Eager transition to a new usage after the submit.
    //  - Acquiring extra semaphores or fences.
    //  - Exporting extra semaphore or fences.
    //  - and more!
    absl::flat_hash_set<raw_ptr<ImportedTextureBase>> specialSyncTextures;

    // Mappable buffers which will be eagerly transitioned to usage MapRead or MapWrite after
    // VkSubmit.
    absl::flat_hash_set<Ref<Buffer>> mappableBuffersForEagerTransition;

    // For Device state tracking only.
    VkCommandPool commandPool = VK_NULL_HANDLE;
    bool needsSubmit = false;
    bool used = false;

    // In some cases command buffer will need to be split to accomodate driver bug workarounds.
    // See the VulkanSplitCommandBufferOnDepthStencilComputeSampleAfterRenderPass toggle as an
    // example. This tracks the list of all command buffers used for this recording context,
    // with commandBuffer always being the last element.
    std::vector<VkCommandBuffer> commandBufferList;
    std::vector<VkCommandPool> commandPoolList;

    // Need to track if a render pass has already been recorded for the
    // VulkanSplitCommandBufferOnComputePassAfterRenderPass workaround.
    bool hasRecordedRenderPass = false;

    void AddBufferBarrier(VkAccessFlags srcAccessMask,
                          VkAccessFlags dstAccessMask,
                          VkPipelineStageFlags srcStages,
                          VkPipelineStageFlags dstStages);
    void EmitBufferBarriers(Device* device);

  private:
    struct BufferBarrier {
        VkAccessFlags bufferSrcAccessMask = 0;
        VkAccessFlags bufferDstAccessMask = 0;
        VkPipelineStageFlags bufferSrcStages = 0;
        VkPipelineStageFlags bufferDstStages = 0;
    };

    BufferBarrier mVertexBufferBarrier;
    BufferBarrier mNonVertexBufferBarrier;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_COMMANDRECORDINGCONTEXT_H_
