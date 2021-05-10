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
#include "VulkanTexture.h"
#include "VulkanUtility.h"

namespace filament {
namespace backend {

struct VulkanProgram : public HwProgram {
    VulkanProgram(VulkanContext& context, const Program& builder) noexcept;
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
    const uint8_t mSamples;
    VulkanAttachment mMsaaAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VulkanAttachment mMsaaDepthAttachment = {};
};

struct VulkanSwapChain : public HwSwapChain {
    VulkanSwapChain(VulkanContext& context, VkSurfaceKHR vksurface);
    VulkanSwapChain(VulkanContext& context, uint32_t width, uint32_t height);
    VulkanSurfaceContext surfaceContext;
};

struct VulkanVertexBuffer : public HwVertexBuffer {
    VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool, VulkanDisposer& disposer,
            uint8_t bufferCount, uint8_t attributeCount, uint32_t elementCount,
            AttributeArray const& attributes);
    std::vector<VulkanBuffer*> buffers;
};

struct VulkanIndexBuffer : public HwIndexBuffer {
    VulkanIndexBuffer(VulkanContext& context, VulkanStagePool& stagePool, VulkanDisposer& disposer,
            uint8_t elementSize, uint32_t indexCount) : HwIndexBuffer(elementSize, indexCount),
            indexType(elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32),
            buffer(new VulkanBuffer(context, stagePool, disposer, this,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, elementSize * indexCount)) {}
    const VkIndexType indexType;
    const std::unique_ptr<VulkanBuffer> buffer;
};

struct VulkanBufferObject : public HwBufferObject {
    VulkanBufferObject(VulkanContext& context, VulkanStagePool& stagePool, VulkanDisposer& disposer,
            uint32_t byteCount) : HwBufferObject(byteCount),
            buffer(new VulkanBuffer(context, stagePool, disposer, this,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, byteCount)) {}
    const std::unique_ptr<VulkanBuffer> buffer;
};

struct VulkanUniformBuffer : public HwUniformBuffer {
    VulkanUniformBuffer(VulkanContext& context, VulkanStagePool& stagePool,
            VulkanDisposer& disposer, uint32_t numBytes, backend::BufferUsage usage);
    ~VulkanUniformBuffer();
    void loadFromCpu(const void* cpuData, uint32_t numBytes);
    VkBuffer getGpuBuffer() const { return mGpuBuffer; }

private:
    VulkanContext& mContext;
    VulkanStagePool& mStagePool;
    VulkanDisposer& mDisposer;
    VkBuffer mGpuBuffer;
    VmaAllocation mGpuMemory;
};

struct VulkanSamplerGroup : public HwSamplerGroup {
    VulkanSamplerGroup(VulkanContext& context, uint32_t count) : HwSamplerGroup(count) {}
};

struct VulkanRenderPrimitive : public HwRenderPrimitive {
    explicit VulkanRenderPrimitive(VulkanContext& context) {}
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

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANHANDLES_H
