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

#include <filament/EngineEnums.h>
#include <filament/SamplerBindingMap.h>

namespace filament {
namespace driver {

struct VulkanProgram : public HwProgram {
    VulkanProgram(VulkanContext& context, const Program& builder) noexcept;
    ~VulkanProgram();
    VulkanContext& context;
    VulkanBinder::ProgramBundle bundle;
    SamplerBindingMap samplerBindings;
};

struct VulkanTexture;

// The render target bundles together a set of attachments, each of which can have one of the
// following ownership semantics:
//
// - The attachment's VkImage is owned by VulkanRenderTarget and is never sampled from; this is
//   somewhat similar to GL renderbuffer attachment (as opposed to a texture attachment).
// - The attachment's VkImage is shared and the owner is VulkanSwapChain.
// - The attachment's VkImage is shared and the owner is VulkanTexture.
//
// We use private inheritence to shield clients from the width / height fields in HwRenderTarget,
// which are not representative when this is the default render target.
struct VulkanRenderTarget : private HwRenderTarget {

    // Creates an offscreen render target.
    VulkanRenderTarget(VulkanContext& context, uint32_t w, uint32_t h) : HwRenderTarget(w, h),
            mContext(context), mOffscreen(true) {}

    // Creates a special "default" render target (i.e. associated with the swap chain)
    VulkanRenderTarget(VulkanContext& context) : HwRenderTarget(0, 0), mContext(context),
            mOffscreen(false) {}

    ~VulkanRenderTarget();
    bool isOffscreen() const { return mOffscreen; }
    void transformClientRectToPlatform(VkRect2D* bounds) const;
    void transformClientRectToPlatform(VkViewport* bounds) const;
    VkExtent2D getExtent() const;
    VulkanAttachment getColor() const;
    VulkanAttachment getDepth() const;
    void createColorImage(VkFormat format);
    void createDepthImage(VkFormat format);
    void setColorImage(VulkanAttachment c);
    void setDepthImage(VulkanAttachment d);
private:
    VulkanAttachment mColor = {};
    VulkanAttachment mDepth = {};
    VulkanContext& mContext;
    bool mOffscreen;
    bool mSharedColorImage = true;
    bool mSharedDepthImage = true;
};

struct VulkanSwapChain : public HwSwapChain {
    VulkanSurfaceContext surfaceContext;
};

struct VulkanVertexBuffer : public HwVertexBuffer {
    VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool, uint8_t bufferCount,
            uint8_t attributeCount, uint32_t elementCount,
            Driver::AttributeArray const& attributes);
    std::vector<std::unique_ptr<VulkanBuffer>> buffers;
};

struct VulkanIndexBuffer : public HwIndexBuffer {
    VulkanIndexBuffer(VulkanContext& context, VulkanStagePool& stagePool, uint8_t elementSize,
            uint32_t indexCount) : HwIndexBuffer(elementSize, indexCount),
            indexType(elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32),
            buffer(new VulkanBuffer(context, stagePool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            elementSize * indexCount)) {}
    const VkIndexType indexType;
    const std::unique_ptr<VulkanBuffer> buffer;
};

struct VulkanUniformBuffer : public HwUniformBuffer {
    VulkanUniformBuffer(VulkanContext& context, VulkanStagePool& stagePool, uint32_t numBytes);
    ~VulkanUniformBuffer();
    void loadFromCpu(const void* cpuData, uint32_t numBytes);
    VkBuffer getGpuBuffer() const { return mGpuBuffer; }
private:
    VulkanContext& mContext;
    VulkanStagePool& mStagePool;
    VkBuffer mGpuBuffer;
    VmaAllocation mGpuMemory;
};

struct VulkanSamplerBuffer : public HwSamplerBuffer {
    VulkanSamplerBuffer(VulkanContext& context, uint32_t count) : HwSamplerBuffer(count) {}
};

struct VulkanTexture : public HwTexture {
    VulkanTexture(VulkanContext& context, SamplerType target, uint8_t levels,
            TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
            TextureUsage usage, VulkanStagePool& stagePool);
    ~VulkanTexture();
    void load2DImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
            int miplevel);
    void loadCubeImage(const PixelBufferDescriptor& data, const FaceOffsets& faceOffsets,
            int miplevel);
    VkFormat format;
    VkImageView imageView = VK_NULL_HANDLE;
    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
private:
    void transitionImageLayout(VkCommandBuffer cmdbuffer, VkImage image,
            VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t miplevel);
    void copyBufferToImage(VkCommandBuffer cmdbuffer, VkBuffer buffer, VkImage image,
            uint32_t width, uint32_t height, FaceOffsets const* faceOffsets, uint32_t miplevel);
    VulkanContext& mContext;
    VulkanStagePool& mStagePool;
    uint32_t mByteCount;
};

struct VulkanRenderPrimitive : public HwRenderPrimitive {
    VulkanRenderPrimitive(VulkanContext& context) {}
    void setPrimitiveType(Driver::PrimitiveType pt);
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

} // namespace filament
} // namespace driver

#endif // TNT_FILAMENT_DRIVER_VULKANHANDLES_H
