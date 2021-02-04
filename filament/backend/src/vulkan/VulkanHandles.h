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
#include "VulkanBinder.h"
#include "VulkanBuffer.h"
#include "VulkanUtility.h"

namespace filament {
namespace backend {

struct VulkanProgram : public HwProgram {
    VulkanProgram(VulkanContext& context, const Program& builder) noexcept;
    ~VulkanProgram();
    VulkanContext& context;
    VulkanBinder::ProgramBundle bundle;
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
            VulkanAttachment color[MRT::TARGET_COUNT], VulkanAttachment depthStencil[2],
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
    bool invalidate();
    uint8_t getSamples() const { return mSamples; }
    bool hasDepth() const { return mDepth.format != VK_FORMAT_UNDEFINED; }

private:
    VulkanAttachment mColor[MRT::TARGET_COUNT] = {};
    VulkanAttachment mDepth = {};
    VulkanContext& mContext;
    const bool mOffscreen;
    const uint8_t mSamples;
    VulkanAttachment mMsaaAttachments[MRT::TARGET_COUNT] = {};
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
    std::vector<std::unique_ptr<VulkanBuffer>> buffers;
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

struct VulkanTexture : public HwTexture {
    VulkanTexture(VulkanContext& context, SamplerType target, uint8_t levels,
            TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
            TextureUsage usage, VulkanStagePool& stagePool);
    ~VulkanTexture();
    void update2DImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            int miplevel);
    void update3DImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            uint32_t depth, int miplevel);
    void updateCubeImage(const PixelBufferDescriptor& data, const FaceOffsets& faceOffsets,
            int miplevel);

    // Returns the primary image view, which is used for shader sampling.
    VkImageView getPrimaryImageView() const { return mCachedImageViews.at(mPrimaryViewRange); }

    // Sets the min/max range of miplevels in the primary image view.
    void setPrimaryRange(uint32_t minMiplevel, uint32_t maxMiplevel);

    // Gets or creates a cached VkImageView for a range of miplevels and array layers.
    VkImageView getImageView(VkImageSubresourceRange range);

    // Convenient "single subresource" overload for the above method.
    VkImageView getImageView(int singleLevel, int singleLayer, VkImageAspectFlags aspect) {
        return getImageView({
            .aspectMask = aspect,
            .baseMipLevel = uint32_t(singleLevel),
            .levelCount = uint32_t(1),
            .baseArrayLayer = uint32_t(singleLayer),
            .layerCount = uint32_t(1),
        });
    }

    VkFormat getVkFormat() const { return mVkFormat; }
    VkImage getVkImage() const { return mTextureImage; }

    // Issues a barrier that transforms the layout of the image, e.g. from a CPU-writeable
    // layout to a GPU-readable layout.
    static void transitionImageLayout(VkCommandBuffer cmdbuffer, VkImage image,
            VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t miplevel,
            uint32_t layers, uint32_t levels, VkImageAspectFlags aspect);

private:
    // Issues a copy from a VkBuffer to a specified miplevel in a VkImage. The given width and
    // height define a subregion within the miplevel.
    void copyBufferToImage(VkCommandBuffer cmdbuffer, VkBuffer buffer, VkImage image,
            uint32_t width, uint32_t height, uint32_t depth,
            FaceOffsets const* faceOffsets, uint32_t miplevel);

    VkFormat mVkFormat;
    VkImageViewType mViewType;
    VkImage mTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory mTextureImageMemory = VK_NULL_HANDLE;
    VkImageSubresourceRange mPrimaryViewRange;
    std::map<VkImageSubresourceRange, VkImageView> mCachedImageViews;
    VkImageAspectFlags mAspect;
    VulkanContext& mContext;
    VulkanStagePool& mStagePool;
};

struct VulkanRenderPrimitive : public HwRenderPrimitive {
    explicit VulkanRenderPrimitive(VulkanContext& context) {}
    void setPrimitiveType(backend::PrimitiveType pt);
    void setBuffers(VulkanVertexBuffer* vertexBuffer, VulkanIndexBuffer* indexBuffer,
            uint32_t enabledAttributes);
    VulkanVertexBuffer* vertexBuffer = nullptr;
    VulkanIndexBuffer* indexBuffer = nullptr;
    VkPrimitiveTopology primitiveTopology;
    // The "varray" field describes the structure of the vertex and gets passed to VulkanBinder,
    // which in turn passes it to vkCreateGraphicsPipelines. The "buffers" and "offsets" vectors are
    // passed to vkCmdBindVertexBuffers at draw call time.
    VulkanBinder::VertexArray varray;
    std::vector<VkBuffer> buffers;
    std::vector<VkDeviceSize> offsets;
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
    std::atomic<VulkanCommandBuffer*> cmdbuffer;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANHANDLES_H
