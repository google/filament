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

#include "VulkanHandles.h"

#include "VulkanConstants.h"
#include "VulkanPlatform.h"

#include <utils/Panic.h>

using namespace bluevk;

namespace filament {
namespace backend {

static void flipVertically(VkRect2D* rect, uint32_t framebufferHeight) {
    rect->offset.y = framebufferHeight - rect->offset.y - rect->extent.height;
}

static void flipVertically(VkViewport* rect, uint32_t framebufferHeight) {
    rect->y = framebufferHeight - rect->y - rect->height;
}

static void clampToFramebuffer(VkRect2D* rect, uint32_t fbWidth, uint32_t fbHeight) {
    int32_t x = std::max(rect->offset.x, 0);
    int32_t y = std::max(rect->offset.y, 0);
    int32_t right = std::min(rect->offset.x + (int32_t) rect->extent.width, (int32_t) fbWidth);
    int32_t top = std::min(rect->offset.y + (int32_t) rect->extent.height, (int32_t) fbHeight);
    rect->offset.x = std::min(x, (int32_t) fbWidth);
    rect->offset.y = std::min(y, (int32_t) fbHeight);
    rect->extent.width = std::max(right - x, 0);
    rect->extent.height = std::max(top - y, 0);
}

VulkanProgram::VulkanProgram(VulkanContext& context, const Program& builder) noexcept :
        HwProgram(builder.getName()), context(context) {
    auto const& blobs = builder.getShadersSource();
    VkShaderModule* modules[2] = { &bundle.vertex, &bundle.fragment };
    bool missing = false;
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const auto& blob = blobs[i];
        VkShaderModule* module = modules[i];
        if (blob.empty()) {
            missing = true;
            continue;
        }
        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = blob.size();
        moduleInfo.pCode = (uint32_t*) blob.data();
        VkResult result = vkCreateShaderModule(context.device, &moduleInfo, VKALLOC, module);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create shader module.");
    }

    // Output a warning because it's okay to encounter empty blobs, but it's not okay to use
    // this program handle in a draw call.
    if (missing) {
        utils::slog.w << "Missing SPIR-V shader: " << builder.getName().c_str() << utils::io::endl;
        return;
    }

    // Make a copy of the binding map
    samplerGroupInfo = builder.getSamplerGroupInfo();
#if FILAMENT_VULKAN_VERBOSE
    utils::slog.d << "Created VulkanProgram " << builder.getName().c_str()
                << ", variant = (" << utils::io::hex
                << (int) builder.getVariant() << utils::io::dec << "), "
                << "shaders = (" << bundle.vertex << ", " << bundle.fragment << ")"
                << utils::io::endl;
#endif
}

VulkanProgram::~VulkanProgram() {
    vkDestroyShaderModule(context.device, bundle.vertex, VKALLOC);
    vkDestroyShaderModule(context.device, bundle.fragment, VKALLOC);
}

static VulkanAttachment createAttachment(VulkanAttachment spec) {
    if (spec.texture == nullptr) {
        return spec;
    }
    return {
        .format = spec.texture->getVkFormat(),
        .image = spec.texture->getVkImage(),
        .view = {},
        .memory = {},
        .texture = spec.texture,
        .layout = getTextureLayout(spec.texture->usage),
        .level = spec.level,
        .layer = spec.layer
    };
}

// Creates a special "default" render target (i.e. associated with the swap chain)
// Note that the attachment structs are unused in this case in favor of VulkanSwapChain.
VulkanRenderTarget::VulkanRenderTarget(VulkanContext& context) : HwRenderTarget(0, 0),
        mContext(context), mOffscreen(false), mSamples(1) {}

VulkanRenderTarget::VulkanRenderTarget(VulkanContext& context, uint32_t width, uint32_t height,
            uint8_t samples, VulkanAttachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
            VulkanAttachment depthStencil[2], VulkanStagePool& stagePool) :
            HwRenderTarget(width, height), mContext(context), mOffscreen(true), mSamples(samples) {

    // For each color attachment, create (or fetch from cache) a VkImageView that selects a specific
    // miplevel and array layer.
    for (int index = 0; index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; index++) {
        const VulkanAttachment& spec = color[index];
        mColor[index] = createAttachment(spec);
        VulkanTexture* texture = spec.texture;
        if (texture == nullptr) {
            continue;
        }
        mColor[index].view = texture->getImageView(spec.level, spec.layer,
                VK_IMAGE_ASPECT_COLOR_BIT);
    }

    // For the depth attachment, create (or fetch from cache) a VkImageView that selects a specific
    // miplevel and array layer.
    const VulkanAttachment& depthSpec = depthStencil[0];
    mDepth = createAttachment(depthSpec);
    VulkanTexture* depthTexture = mDepth.texture;
    if (depthTexture) {
        mDepth.view = depthTexture->getImageView(mDepth.level, mDepth.layer,
                VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    if (samples == 1) {
        return;
    }

    // The sidecar textures need to have only 1 miplevel and 1 array slice.
    const int level = 1;
    const int depth = 1;

    // Create sidecar MSAA textures for color attachments.
    for (int index = 0; index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; index++) {
        const VulkanAttachment& spec = color[index];
        VulkanTexture* texture = spec.texture;
        if (texture && texture->samples == 1) {
            VulkanTexture* msTexture = new VulkanTexture(context, texture->target, level,
                    texture->format, samples, width, height, depth, texture->usage, stagePool);
            mMsaaAttachments[index] = createAttachment({ .texture = msTexture });
            mMsaaAttachments[index].view = msTexture->getImageView(0, 0, VK_IMAGE_ASPECT_COLOR_BIT);
        }
        if (texture && texture->samples > 1) {
            mMsaaAttachments[index] = mColor[index];
        }
    }

    if (depthTexture == nullptr) {
        return;
    }

    // There is no need for sidecar depth if the depth texture is already MSAA.
    if (depthTexture->samples > 1) {
        mMsaaDepthAttachment = mDepth;
        return;
    }

    // Create sidecar MSAA texture for the depth attachment.
    VulkanTexture* msTexture = new VulkanTexture(context, depthTexture->target, level,
            depthTexture->format, samples, width, height, depth, depthTexture->usage, stagePool);
    mMsaaDepthAttachment = createAttachment({
        .format = {},
        .image = {},
        .view = {},
        .memory = {},
        .texture = msTexture,
        .layout = {},
        .level = depthSpec.level,
        .layer = depthSpec.layer,
    });
    mMsaaDepthAttachment.view = msTexture->getImageView(depthSpec.level, depthSpec.layer,
            VK_IMAGE_ASPECT_DEPTH_BIT);
}

VulkanRenderTarget::~VulkanRenderTarget() {
    for (int index = 0; index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; index++) {
        if (mMsaaAttachments[index].texture != mColor[index].texture) {
            delete mMsaaAttachments[index].texture;
        }
    }
    if (mMsaaDepthAttachment.texture != mDepth.texture) {
        delete mMsaaDepthAttachment.texture;
    }
}

void VulkanRenderTarget::transformClientRectToPlatform(VkRect2D* bounds) const {
    const auto& extent = getExtent();
    flipVertically(bounds, extent.height);
    clampToFramebuffer(bounds, extent.width, extent.height);
}

void VulkanRenderTarget::transformClientRectToPlatform(VkViewport* bounds) const {
    flipVertically(bounds, getExtent().height);
}

VkExtent2D VulkanRenderTarget::getExtent() const {
    if (mOffscreen) {
        return {width, height};
    }
    return mContext.currentSurface->clientSize;
}

VulkanAttachment VulkanRenderTarget::getColor(int target) const {
    return (mOffscreen || target > 0) ? mColor[target] : getSwapChainAttachment(mContext);
}

VulkanAttachment VulkanRenderTarget::getMsaaColor(int target) const {
    return mMsaaAttachments[target];
}

VulkanAttachment VulkanRenderTarget::getDepth() const {
    return mOffscreen ? mDepth : mContext.currentSurface->depth;
}

VulkanAttachment VulkanRenderTarget::getMsaaDepth() const {
    return mMsaaDepthAttachment;
}

int VulkanRenderTarget::getColorTargetCount(const VulkanRenderPass& pass) const {
    if (!mOffscreen) {
        return 1;
    }
    int count = 0;
    for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (mColor[i].format == VK_FORMAT_UNDEFINED) {
            continue;
        }
        // NOTE: This must be consistent with VkRenderPass construction (see VulkanFboCache).
        if (!(pass.subpassMask & (1 << i)) || pass.currentSubpass == 1) {
            count++;
        }
    }
    return count;
}

VulkanVertexBuffer::VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        VulkanDisposer& disposer,  uint8_t bufferCount, uint8_t attributeCount,
        uint32_t elementCount, AttributeArray const& attribs) :
        HwVertexBuffer(bufferCount, attributeCount, elementCount, attribs),
        buffers(bufferCount) {}

VulkanUniformBuffer::VulkanUniformBuffer(VulkanContext& context, VulkanStagePool& stagePool,
        VulkanDisposer& disposer, uint32_t numBytes, backend::BufferUsage usage)
        : mContext(context), mStagePool(stagePool), mDisposer(disposer) {
    // Create the VkBuffer.
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = numBytes,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    VmaAllocationCreateInfo allocInfo {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY
    };
    vmaCreateBuffer(mContext.allocator, &bufferInfo, &allocInfo, &mGpuBuffer, &mGpuMemory, nullptr);
}

void VulkanUniformBuffer::loadFromCpu(const void* cpuData, uint32_t numBytes) {
    VulkanStage const* stage = mStagePool.acquireStage(numBytes);
    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    memcpy(mapped, cpuData, numBytes);
    vmaUnmapMemory(mContext.allocator, stage->memory);
    vmaFlushAllocation(mContext.allocator, stage->memory, 0, numBytes);

    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;

    VkBufferCopy region { .size = numBytes };
    vkCmdCopyBuffer(cmdbuffer, stage->buffer, mGpuBuffer, 1, &region);
    mDisposer.acquire(this);

    // First, ensure that the copy finishes before the next draw call.
    // Second, in case the user decides to upload another chunk (without ever using the first one)
    // we need to ensure that this upload completes first.

    // NOTE: ideally dstStageMask would include VERTEX_SHADER_BIT | FRAGMENT_SHADER_BIT, but this
    // seems to be insufficient on Mali devices. To work around this we are using a more
    // aggressive ALL_GRAPHICS_BIT barrier.

    VkBufferMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = mGpuBuffer,
        .size = VK_WHOLE_SIZE
    };

    vkCmdPipelineBarrier(cmdbuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
            0, 0, nullptr, 1, &barrier, 0, nullptr);
}

VulkanUniformBuffer::~VulkanUniformBuffer() {
    vmaDestroyBuffer(mContext.allocator, mGpuBuffer, mGpuMemory);
}

void VulkanRenderPrimitive::setPrimitiveType(backend::PrimitiveType pt) {
    this->type = pt;
    switch (pt) {
        case backend::PrimitiveType::NONE:
        case backend::PrimitiveType::POINTS:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case backend::PrimitiveType::LINES:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case backend::PrimitiveType::TRIANGLES:
            primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
    }
}

void VulkanRenderPrimitive::setBuffers(VulkanVertexBuffer* vertexBuffer,
        VulkanIndexBuffer* indexBuffer) {
    this->vertexBuffer = vertexBuffer;
    this->indexBuffer = indexBuffer;
}

VulkanTimerQuery::VulkanTimerQuery(VulkanContext& context) : mContext(context) {
    std::unique_lock<utils::Mutex> lock(context.timestamps.mutex);
    utils::bitset32& bitset = context.timestamps.used;
    const size_t maxTimers = bitset.size();
    assert_invariant(bitset.count() < maxTimers);
    for (size_t timerIndex = 0; timerIndex < maxTimers; ++timerIndex) {
        if (!bitset.test(timerIndex)) {
            bitset.set(timerIndex);
            startingQueryIndex = timerIndex * 2;
            stoppingQueryIndex = timerIndex * 2 + 1;
            return;
        }
    }
    utils::slog.e << "More than " << maxTimers << " timers are not supported." << utils::io::endl;
    startingQueryIndex = 0;
    stoppingQueryIndex = 1;
}

VulkanTimerQuery::~VulkanTimerQuery() {
    std::unique_lock<utils::Mutex> lock(mContext.timestamps.mutex);
    mContext.timestamps.used.unset(startingQueryIndex / 2);
}

} // namespace filament
} // namespace backend
