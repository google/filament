/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #ifndef TNT_FILAMENT_DRIVER_VULKANHANDLES_H
 #define TNT_FILAMENT_DRIVER_VULKANHANDLES_H

#include "VulkanDriver.h"
#include "VulkanPipelineCache.h"
#include "VulkanBuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"

namespace filament {
namespace backend {

struct VulkanProgram : public HwProgram {
    VulkanProgram(VulkanContext& context, const Program& builder) noexcept;
    VulkanProgram(VulkanContext& context, VkShaderModule vs, VkShaderModule fs) noexcept;
    ~VulkanProgram();
    VulkanContext& context;
    VulkanPipelineCache::ProgramBundle bundle;
    Program::SamplerGroupInfo samplerGroupInfo;
};

// The render target bundles together a set of attachments, each of which can have one of the
// following ownership semantics:
//
// - The attachment's VkImage is shared and the owner is VulkanSwapChain (mOffscreen = false).
// - The attachment's VkImage is shared and the owner is VulkanTexture   (mOffscreen = true).
//
// We use private inheritance to shield clients from the width / height fields in HwRenderTarget,
// which are not representative when this is the default render target.
struct VulkanRenderTarget : private HwRenderTarget {
    // Creates an offscreen render target.
    VulkanRenderTarget(VulkanContext& context, uint32_t width, uint32_t height, uint8_t samples,
            VulkanAttachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT], VulkanAttachment depthStencil[2],
            VulkanStagePool& stagePool);

    // Creates a special "default" render target (i.e. associated with the swap chain)
    explicit VulkanRenderTarget(VulkanContext& context);

    ~VulkanRenderTarget();

    void transformClientRectToPlatform(VkRect2D* bounds) const;
    void transformClientRectToPlatform(VkViewport* bounds) const;
    VkExtent2D getExtent() const;
    VulkanAttachment getColor(int target) const;
    VulkanAttachment getMsaaColor(int target) const;
    VulkanAttachment getDepth() const;
    VulkanAttachment getMsaaDepth() const;
    int getColorTargetCount(const VulkanRenderPass& pass) const;
    uint8_t getSamples() const { return mSamples; }
    bool hasDepth() const { return mDepth.format != VK_FORMAT_UNDEFINED; }
    bool isSwapChain() const { return !mOffscreen; }

private:
    VulkanAttachment mColor[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VulkanAttachment mDepth = {};
    VulkanContext& mContext;
    const bool mOffscreen;
    uint8_t mSamples;
    VulkanAttachment mMsaaAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VulkanAttachment mMsaaDepthAttachment = {};
};

struct VulkanVertexBuffer : public HwVertexBuffer {
    VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
            uint8_t bufferCount, uint8_t attributeCount, uint32_t elementCount,
            AttributeArray const& attributes);
    utils::FixedCapacityVector<VulkanBuffer const*> buffers;
};

struct VulkanIndexBuffer : public HwIndexBuffer {
    VulkanIndexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
            uint8_t elementSize, uint32_t indexCount) : HwIndexBuffer(elementSize, indexCount),
            buffer(context, stagePool,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, elementSize * indexCount),
            indexType(elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32) {}
    void terminate(VulkanContext& context) { buffer.terminate(context); }
    VulkanBuffer buffer;
    const VkIndexType indexType;
};

struct VulkanBufferObject : public HwBufferObject {
    VulkanBufferObject(VulkanContext& context, VulkanStagePool& stagePool,
            uint32_t byteCount, BufferObjectBinding bindingType, BufferUsage usage);
    void terminate(VulkanContext& context) { buffer.terminate(context); }
    VulkanBuffer buffer;
    const BufferObjectBinding bindingType;
};

struct VulkanUniformBuffer : public HwUniformBuffer {
    VulkanUniformBuffer(VulkanContext& context, VulkanStagePool& stagePool,
            uint32_t numBytes, backend::BufferUsage usage);
    ~VulkanUniformBuffer();
    void loadFromCpu(const void* cpuData, uint32_t numBytes);
    VkBuffer getGpuBuffer() const { return mGpuBuffer; }

private:
    VulkanContext& mContext;
    VulkanStagePool& mStagePool;
    VkBuffer mGpuBuffer;
    VmaAllocation mGpuMemory;
};

struct VulkanSamplerGroup : public HwSamplerGroup {
    VulkanSamplerGroup(uint32_t count) : HwSamplerGroup(count) {}
};

struct VulkanRenderPrimitive : public HwRenderPrimitive {
    void setPrimitiveType(backend::PrimitiveType pt);
    void setBuffers(VulkanVertexBuffer* vertexBuffer, VulkanIndexBuffer* indexBuffer);
    VulkanVertexBuffer* vertexBuffer = nullptr;
    VulkanIndexBuffer* indexBuffer = nullptr;
    VkPrimitiveTopology primitiveTopology;
};

struct VulkanFence : public HwFence {
    VulkanFence(const VulkanCommandBuffer& commands) : fence(commands.fence) {}
    std::shared_ptr<VulkanCmdFence> fence;
};

struct VulkanSync : public HwSync {
    VulkanSync(const VulkanCommandBuffer& commands) : fence(commands.fence) {}
    std::shared_ptr<VulkanCmdFence> fence;
};

struct VulkanTimerQuery : public HwTimerQuery {
    VulkanTimerQuery(VulkanContext& context);
    ~VulkanTimerQuery();
    uint32_t startingQueryIndex;
    uint32_t stoppingQueryIndex;
    VulkanContext& mContext;
    std::atomic<VulkanCommandBuffer const*> cmdbuffer;
};

inline constexpr VkBufferUsageFlagBits getBufferObjectUsage(
        BufferObjectBinding bindingType) noexcept {
    switch(bindingType) {
        case BufferObjectBinding::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferObjectBinding::UNIFORM:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
}

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANHANDLES_H
