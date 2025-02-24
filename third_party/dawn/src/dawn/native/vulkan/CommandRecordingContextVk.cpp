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

#include "src/dawn/native/vulkan/CommandRecordingContextVk.h"

#include "dawn/native/vulkan/DeviceVk.h"
#include "vulkan/vulkan_core.h"

namespace dawn::native::vulkan {
namespace {

// Separate barriers with vertex stages in destination stages from all other barriers.
// This avoids creating unnecessary fragment->vertex dependencies when merging barriers.
// Eg. merging a compute->vertex barrier and a fragment->fragment barrier would create
// a compute|fragment->vertex|fragment barrier.
constexpr VkPipelineStageFlags vertexStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
                                              VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                              VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

}  // namespace

void CommandRecordingContext::AddBufferBarrier(VkAccessFlags srcAccessMask,
                                               VkAccessFlags dstAccessMask,
                                               VkPipelineStageFlags srcStages,
                                               VkPipelineStageFlags dstStages) {
    BufferBarrier* barrier = nullptr;
    if (dstStages & vertexStages) {
        barrier = &mVertexBufferBarrier;
    } else {
        barrier = &mNonVertexBufferBarrier;
    }

    barrier->bufferSrcAccessMask |= srcAccessMask;
    barrier->bufferDstAccessMask |= dstAccessMask;
    barrier->bufferSrcStages |= srcStages;
    barrier->bufferDstStages |= dstStages;
}

void CommandRecordingContext::EmitBufferBarriers(Device* device) {
    std::array<VkMemoryBarrier, 2> barriers;
    size_t idx = 0;

    VkPipelineStageFlags srcStages = 0;
    VkPipelineStageFlags dstStages = 0;

    auto CreateBarrierIfNeeded = [&](const BufferBarrier& barrier) {
        if (barrier.bufferSrcStages == 0 || barrier.bufferDstStages == 0) {
            return;
        }

        barriers[idx].sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barriers[idx].pNext = nullptr;
        barriers[idx].srcAccessMask = barrier.bufferSrcAccessMask;
        barriers[idx].dstAccessMask = barrier.bufferDstAccessMask;

        srcStages |= barrier.bufferSrcStages;
        dstStages |= barrier.bufferDstStages;

        idx++;
    };
    CreateBarrierIfNeeded(mVertexBufferBarrier);
    CreateBarrierIfNeeded(mNonVertexBufferBarrier);

    if (idx > 0) {
        device->fn.CmdPipelineBarrier(commandBuffer, srcStages, dstStages, 0, idx, barriers.data(),
                                      0, nullptr, 0, nullptr);
    }

    mVertexBufferBarrier = {};
    mNonVertexBufferBarrier = {};
}

}  // namespace dawn::native::vulkan
