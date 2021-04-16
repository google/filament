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

#include "vulkan/VulkanHandles.h"

#include "DataReshaper.h"
#include "VulkanPlatform.h"

#include <utils/Panic.h>

#define FILAMENT_VULKAN_VERBOSE 0

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
        .texture = spec.texture,
        .layout = getTextureLayout(spec.texture->usage),
        .level = spec.level,
        .layer = spec.layer
    };
}

// Creates a special "default" render target (i.e. associated with the swap chain)
// Note that the attachment structs are unused in this case in favor of VulkanSurfaceContext.
VulkanRenderTarget::VulkanRenderTarget(VulkanContext& context) : HwRenderTarget(0, 0),
        mContext(context), mOffscreen(false), mSamples(1) {}

VulkanRenderTarget::VulkanRenderTarget(VulkanContext& context, uint32_t width, uint32_t height,
            uint8_t samples, VulkanAttachment color[MRT::TARGET_COUNT],
            VulkanAttachment depthStencil[2], VulkanStagePool& stagePool) :
            HwRenderTarget(width, height), mContext(context), mOffscreen(true), mSamples(samples) {

    // For each color attachment, create (or fetch from cache) a VkImageView that selects a specific
    // miplevel and array layer.
    for (int index = 0; index < MRT::TARGET_COUNT; index++) {
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
    for (int index = 0; index < MRT::TARGET_COUNT; index++) {
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
        .texture = msTexture,
        .level = depthSpec.level,
        .layer = depthSpec.layer,
    });
    mMsaaDepthAttachment.view = msTexture->getImageView(depthSpec.level, depthSpec.layer,
            VK_IMAGE_ASPECT_DEPTH_BIT);
}

VulkanRenderTarget::~VulkanRenderTarget() {
    for (int index = 0; index < MRT::TARGET_COUNT; index++) {
        if (mMsaaAttachments[index].texture != mColor[index].texture) {
            delete mMsaaAttachments[index].texture;
        }
    }
    if (mMsaaDepthAttachment.texture != mDepth.texture) {
        delete mMsaaDepthAttachment.texture;
    }
}

// Primary SwapChain constructor. (not headless)
VulkanSwapChain::VulkanSwapChain(VulkanContext& context, VkSurfaceKHR vksurface) {
    surfaceContext.suboptimal = false;
    surfaceContext.surface = vksurface;
    getPresentationQueue(context, surfaceContext);
    createSwapChain(context, surfaceContext);
}

// Headless SwapChain constructor. (does not create a VkSwapChainKHR)
VulkanSwapChain::VulkanSwapChain(VulkanContext& context, uint32_t width, uint32_t height) {
    surfaceContext.surface = VK_NULL_HANDLE;
    getHeadlessQueue(context, surfaceContext);

    surfaceContext.surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    surfaceContext.swapchain = VK_NULL_HANDLE;

    // Somewhat arbitrarily, headless rendering is double-buffered.
    surfaceContext.swapContexts.resize(2);

    // Allocate a command buffer for each swap context, just like a real swap chain.
    VkCommandBufferAllocateInfo allocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = context.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t) surfaceContext.swapContexts.size()
    };
    std::vector<VkCommandBuffer> cmdbufs(allocateInfo.commandBufferCount);
    vkAllocateCommandBuffers(context.device, &allocateInfo, cmdbufs.data());
    for (uint32_t i = 0; i < allocateInfo.commandBufferCount; ++i) {
        surfaceContext.swapContexts[i].commands.cmdbuffer = cmdbufs[i];
    }

    // Begin a new command buffer in order to transition image layouts via vkCmdPipelineBarrier.
    VkCommandBuffer cmdbuffer = acquireWorkCommandBuffer(context);

    for (size_t i = 0; i < surfaceContext.swapContexts.size(); ++i) {
        VkImage image;
        VkImageCreateInfo iCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = surfaceContext.surfaceFormat.format,
            .extent = {
                .width = width,
                .height = height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        };
        assert_invariant(iCreateInfo.extent.width > 0);
        assert_invariant(iCreateInfo.extent.height > 0);
        vkCreateImage(context.device, &iCreateInfo, VKALLOC, &image);

        VkMemoryRequirements memReqs = {};
        vkGetImageMemoryRequirements(context.device, image, &memReqs);
        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memReqs.size,
            .memoryTypeIndex = selectMemoryType(context, memReqs.memoryTypeBits,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };
        VkDeviceMemory imageMemory;
        vkAllocateMemory(context.device, &allocInfo, VKALLOC, &imageMemory);
        vkBindImageMemory(context.device, image, imageMemory, 0);

        surfaceContext.swapContexts[i].attachment = {
            .format = surfaceContext.surfaceFormat.format, .image = image,
            .view = {}, .memory = imageMemory, .texture = {}, .layout = VK_IMAGE_LAYOUT_GENERAL
        };
        VkImageViewCreateInfo ivCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surfaceContext.surfaceFormat.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            }
        };
        vkCreateImageView(context.device, &ivCreateInfo, VKALLOC,
                    &surfaceContext.swapContexts[i].attachment.view);

        VkImageMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        };
        vkCmdPipelineBarrier(cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    flushWorkCommandBuffer(context);

    surfaceContext.surfaceCapabilities.currentExtent.width = width;
    surfaceContext.surfaceCapabilities.currentExtent.height = height;

    surfaceContext.clientSize.width = width;
    surfaceContext.clientSize.height = height;

    surfaceContext.imageAvailable = VK_NULL_HANDLE;
    surfaceContext.renderingFinished = VK_NULL_HANDLE;

    createFinalDepthBuffer(context, surfaceContext, context.finalDepthFormat);
}

void VulkanRenderTarget::transformClientRectToPlatform(VkRect2D* bounds) const {
    // For the backbuffer, there are corner cases where the platform's surface resolution does not
    // match what Filament expects, so we need to make an appropriate transformation (e.g. create a
    // VkSurfaceKHR on a high DPI display, then move it to a low DPI display).
    if (!mOffscreen) {
        const VkExtent2D platformSize = mContext.currentSurface->surfaceCapabilities.currentExtent;
        const VkExtent2D clientSize = mContext.currentSurface->clientSize;

        // Because these types of coordinates are pixel-addressable, we purposefully use integer
        // math and rely on left-to-right evaluation.
        bounds->offset.x = bounds->offset.x * platformSize.width / clientSize.width;
        bounds->offset.y = bounds->offset.y * platformSize.height / clientSize.height;
        bounds->extent.width = bounds->extent.width * platformSize.width / clientSize.width;
        bounds->extent.height = bounds->extent.height * platformSize.height / clientSize.height;
    }
    const auto& extent = getExtent();
    flipVertically(bounds, extent.height);
    clampToFramebuffer(bounds, extent.width, extent.height);
}

void VulkanRenderTarget::transformClientRectToPlatform(VkViewport* bounds) const {
    // For the backbuffer, we must check if platform size and client size differ, then scale
    // appropriately. Note the +2 correction factor. This prevents the platform from lerping pixels
    // along the edge of the viewport with pixels that live outside the viewport. Luckily this
    // correction factor only applies in obscure conditions (e.g. after dragging a high-DPI window
    // to a low-DPI display).
    if (!mOffscreen) {
        const VkExtent2D platformSize = mContext.currentSurface->surfaceCapabilities.currentExtent;
        const VkExtent2D clientSize = mContext.currentSurface->clientSize;
        if (platformSize.width != clientSize.width) {
            const float xscale = float(platformSize.width + 2) / float(clientSize.width);
            bounds->x *= xscale;
            bounds->width *= xscale;
        }
        if (platformSize.height != clientSize.height) {
            const float yscale = float(platformSize.height + 2) / float(clientSize.height);
            bounds->y *= yscale;
            bounds->height *= yscale;
        }
    }
    flipVertically(bounds, getExtent().height);
}

VkExtent2D VulkanRenderTarget::getExtent() const {
    if (mOffscreen) {
        return {width, height};
    }
    return mContext.currentSurface->surfaceCapabilities.currentExtent;
}

VulkanAttachment VulkanRenderTarget::getColor(int target) const {
    return (mOffscreen || target > 0) ? mColor[target] : getSwapContext(mContext).attachment;
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
    for (int i = 0; i < MRT::TARGET_COUNT; i++) {
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

bool VulkanRenderTarget::invalidate() {
    if (!mOffscreen && getSwapContext(mContext).invalid) {
        getSwapContext(mContext).invalid = false;
        return true;
    }
    return false;
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

    auto copyToDevice = [this, numBytes, stage] (VulkanCommandBuffer& commands) {
        VkBufferCopy region { .size = numBytes };
        vkCmdCopyBuffer(commands.cmdbuffer, stage->buffer, mGpuBuffer, 1, &region);
        mDisposer.acquire(this, commands.resources);

        // Ensure that the copy finishes before the next draw call.
        VkBufferMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = mGpuBuffer,
            .size = VK_WHOLE_SIZE
        };

        vkCmdPipelineBarrier(commands.cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                nullptr, 1, &barrier, 0, nullptr);

        mStagePool.releaseStage(stage, commands);
    };

    // If inside beginFrame / endFrame, use the swap context, otherwise use the work cmdbuffer.
    if (mContext.currentCommands) {
        copyToDevice(*mContext.currentCommands);
    } else {
        acquireWorkCommandBuffer(mContext);
        copyToDevice(mContext.work);
        flushWorkCommandBuffer(mContext);
    }
}

VulkanUniformBuffer::~VulkanUniformBuffer() {
    vmaDestroyBuffer(mContext.allocator, mGpuBuffer, mGpuMemory);
}

VulkanTexture::VulkanTexture(VulkanContext& context, SamplerType target, uint8_t levels,
        TextureFormat tformat, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage tusage, VulkanStagePool& stagePool, VkComponentMapping swizzle) :
        HwTexture(target, levels, samples, w, h, depth, tformat, tusage),

        // Vulkan does not support 24-bit depth, use the official fallback format.
        mVkFormat(tformat == TextureFormat::DEPTH24 ? context.finalDepthFormat :
                backend::getVkFormat(tformat)),

        mSwizzle(swizzle), mContext(context), mStagePool(stagePool) {

    // Create an appropriately-sized device-only VkImage, but do not fill it yet.
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = target == SamplerType::SAMPLER_3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
        .format = mVkFormat,
        .extent = { w, h, depth },
        .mipLevels = levels,
        .arrayLayers = 1,
        .samples = (VkSampleCountFlagBits) samples,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = 0
    };
    if (target == SamplerType::SAMPLER_CUBEMAP) {
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    if (target == SamplerType::SAMPLER_2D_ARRAY) {
        imageInfo.arrayLayers = depth;
        imageInfo.extent.depth = 1;
        // NOTE: We do not use VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT here because:
        //
        //  (a) MoltenVK does not support it, and
        //  (b) it is necessary only when 3D textures need to support array-style access
        //
        // In other words, the "arrayness" of the texture is an aspect of the VkImageView,
        // not the VkImage.
    }

    // Filament expects blit() to work with any texture, so we almost always set these usage flags.
    // TODO: investigate performance implications of setting these flags.
    const VkImageUsageFlags blittable = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (any(usage & TextureUsage::SAMPLEABLE)) {

#if VK_ENABLE_VALIDATION
        // Validate that the format is actually sampleable.
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(context.physicalDevice, mVkFormat, &props);
        if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
            utils::slog.w << "Texture usage is SAMPLEABLE but format " << mVkFormat << " is not "
                    "sampleable with optimal tiling." << utils::io::endl;
        }
#endif

        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | blittable;
        if (any(usage & TextureUsage::SUBPASS_INPUT)) {
            imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
    }
    if (any(usage & TextureUsage::STENCIL_ATTACHMENT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (any(usage & TextureUsage::UPLOADABLE)) {
        imageInfo.usage |= blittable;
    }
    if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
        imageInfo.usage |= blittable;
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    VkResult error = vkCreateImage(context.device, &imageInfo, VKALLOC, &mTextureImage);
    if (error || FILAMENT_VULKAN_VERBOSE) {
        utils::slog.d << "vkCreateImage: "
            << "result = " << error << ", "
            << "handle = " << utils::io::hex << mTextureImage << utils::io::dec << ", "
            << "extent = " << w << "x" << h << "x"<< depth << ", "
            << "mipLevels = " << int(levels) << ", "
            << "usage = " << imageInfo.usage << ", "
            << "samples = " << imageInfo.samples << ", "
            << "format = " << mVkFormat << utils::io::endl;
    }
    ASSERT_POSTCONDITION(!error, "Unable to create image.");

    // Allocate memory for the VkImage and bind it.
    VkMemoryRequirements memReqs = {};
    vkGetImageMemoryRequirements(context.device, mTextureImage, &memReqs);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = selectMemoryType(context, memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    error = vkAllocateMemory(context.device, &allocInfo, nullptr, &mTextureImageMemory);
    ASSERT_POSTCONDITION(!error, "Unable to allocate image memory.");
    error = vkBindImageMemory(context.device, mTextureImage, mTextureImageMemory, 0);
    ASSERT_POSTCONDITION(!error, "Unable to bind image.");

    mAspect = any(usage & TextureUsage::DEPTH_ATTACHMENT) ? VK_IMAGE_ASPECT_DEPTH_BIT :
            VK_IMAGE_ASPECT_COLOR_BIT;

    // Spec out the "primary" VkImageView that shaders use to sample from the image.
    mPrimaryViewRange.aspectMask = mAspect;
    mPrimaryViewRange.baseMipLevel = 0;
    mPrimaryViewRange.levelCount = levels;
    mPrimaryViewRange.baseArrayLayer = 0;
    if (target == SamplerType::SAMPLER_CUBEMAP) {
        mViewType = VK_IMAGE_VIEW_TYPE_CUBE;
        mPrimaryViewRange.layerCount = 6;
    } else if (target == SamplerType::SAMPLER_2D_ARRAY) {
        mViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        mPrimaryViewRange.layerCount = depth;
    } else if (target == SamplerType::SAMPLER_3D) {
        mViewType = VK_IMAGE_VIEW_TYPE_3D;
        mPrimaryViewRange.layerCount = 1;
    } else {
        mViewType = VK_IMAGE_VIEW_TYPE_2D;
        mPrimaryViewRange.layerCount = 1;
    }

    // Go ahead and create the primary image view, no need to do it lazily.
    getImageView(mPrimaryViewRange);

    // Transition the layout.
    if (any(usage & (TextureUsage::COLOR_ATTACHMENT | TextureUsage::DEPTH_ATTACHMENT))) {
        auto transition = [=](VulkanCommandBuffer commands) {
            // If this is a SAMPLER_2D_ARRAY texture, then the depth argument stores the number of
            // texture layers.
            const uint32_t layers = target == SamplerType::SAMPLER_2D_ARRAY ? depth : 1;
            VulkanTexture::transitionImageLayout(commands.cmdbuffer, mTextureImage,
                    VK_IMAGE_LAYOUT_UNDEFINED, getTextureLayout(usage), 0, layers, levels, mAspect);
        };
        if (mContext.currentCommands) {
            transition(*mContext.currentCommands);
        } else {
            acquireWorkCommandBuffer(mContext);
            transition(mContext.work);
            flushWorkCommandBuffer(mContext);
        }
    }
}

VulkanTexture::~VulkanTexture() {
    vkDestroyImage(mContext.device, mTextureImage, VKALLOC);
    vkFreeMemory(mContext.device, mTextureImageMemory, VKALLOC);
    for (auto entry : mCachedImageViews) {
        vkDestroyImageView(mContext.device, entry.second, VKALLOC);
    }
}

void VulkanTexture::update2DImage(const PixelBufferDescriptor& data, uint32_t width,
        uint32_t height, int miplevel) {
    update3DImage(std::move(data), width, height, 1, miplevel);
}

void VulkanTexture::update3DImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
        uint32_t depth, int miplevel) {
    assert_invariant(width <= this->width && height <= this->height && depth <= this->depth);
    const uint32_t srcBytesPerTexel = getBytesPerPixel(format);
    const bool reshape = srcBytesPerTexel == 3 || srcBytesPerTexel == 6;
    const void* cpuData = data.buffer;
    const uint32_t numSrcBytes = data.size;
    const uint32_t numDstBytes = reshape ? (4 * numSrcBytes / 3) : numSrcBytes;

    // Create and populate the staging buffer.
    VulkanStage const* stage = mStagePool.acquireStage(numDstBytes);
    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    switch (srcBytesPerTexel) {
        case 3:
            // Morph the data from 3 bytes per texel to 4 bytes per texel and set alpha to 1.
            DataReshaper::reshape<uint8_t, 3, 4>(mapped, cpuData, numSrcBytes);
            break;
        case 6:
            // Morph the data from 6 bytes per texel to 8 bytes per texel. Note that this does not
            // set alpha to 1 for half-float formats, but in practice that's fine since alpha is
            // just a dummy channel in this situation.
            DataReshaper::reshape<uint16_t, 3, 4>(mapped, cpuData, numSrcBytes);
            break;
        default:
            memcpy(mapped, cpuData, numSrcBytes);
    }
    vmaUnmapMemory(mContext.allocator, stage->memory);
    vmaFlushAllocation(mContext.allocator, stage->memory, 0, numDstBytes);

    // Create a copy-to-device functor.
    auto copyToDevice = [this, stage, width, height, depth, miplevel] (VulkanCommandBuffer& commands) {
        transitionImageLayout(commands.cmdbuffer, mTextureImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, miplevel, 1, 1, mAspect);
        copyBufferToImage(commands.cmdbuffer, stage->buffer, mTextureImage, width, height, depth,
                nullptr, miplevel);
        transitionImageLayout(commands.cmdbuffer, mTextureImage,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                getTextureLayout(usage), miplevel, 1, 1, mAspect);

        mStagePool.releaseStage(stage, commands);
    };

    // If inside beginFrame / endFrame, use the swap context, otherwise use the work cmdbuffer.
    if (mContext.currentCommands) {
        copyToDevice(*mContext.currentCommands);
    } else {
        acquireWorkCommandBuffer(mContext);
        copyToDevice(mContext.work);
        flushWorkCommandBuffer(mContext);
    }
}

void VulkanTexture::updateCubeImage(const PixelBufferDescriptor& data,
        const FaceOffsets& faceOffsets, int miplevel) {
    assert_invariant(this->target == SamplerType::SAMPLER_CUBEMAP);
    const bool reshape = getBytesPerPixel(format) == 3;
    const void* cpuData = data.buffer;
    const uint32_t numSrcBytes = data.size;
    const uint32_t numDstBytes = reshape ? (4 * numSrcBytes / 3) : numSrcBytes;

    // Create and populate the staging buffer.
    VulkanStage const* stage = mStagePool.acquireStage(numDstBytes);
    void* mapped;
    vmaMapMemory(mContext.allocator, stage->memory, &mapped);
    if (reshape) {
        DataReshaper::reshape<uint8_t, 3, 4>(mapped, cpuData, numSrcBytes);
    } else {
        memcpy(mapped, cpuData, numSrcBytes);
    }
    vmaUnmapMemory(mContext.allocator, stage->memory);
    vmaFlushAllocation(mContext.allocator, stage->memory, 0, numDstBytes);

    // Create a copy-to-device functor.
    auto copyToDevice = [this, faceOffsets, stage, miplevel] (VulkanCommandBuffer& commands) {
        uint32_t width = std::max(1u, this->width >> miplevel);
        uint32_t height = std::max(1u, this->height >> miplevel);
        transitionImageLayout(commands.cmdbuffer, mTextureImage, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, miplevel, 6, 1, mAspect);
        copyBufferToImage(commands.cmdbuffer, stage->buffer, mTextureImage, width, height, 1,
                &faceOffsets, miplevel);
        transitionImageLayout(commands.cmdbuffer, mTextureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, getTextureLayout(usage), miplevel, 6,
                1, mAspect);

        mStagePool.releaseStage(stage, commands);
    };

    // If inside beginFrame / endFrame, use the swap context, otherwise use the work cmdbuffer.
    if (mContext.currentCommands) {
        copyToDevice(*mContext.currentCommands);
    } else {
        acquireWorkCommandBuffer(mContext);
        copyToDevice(mContext.work);
        flushWorkCommandBuffer(mContext);
    }
}

void VulkanTexture::setPrimaryRange(uint32_t minMiplevel, uint32_t maxMiplevel) {
    maxMiplevel = filament::math::min(int(maxMiplevel), int(this->levels - 1));
    mPrimaryViewRange.baseMipLevel = minMiplevel;
    mPrimaryViewRange.levelCount = maxMiplevel - minMiplevel + 1;
    getImageView(mPrimaryViewRange);
}

VkImageView VulkanTexture::getImageView(VkImageSubresourceRange range) {
    auto iter = mCachedImageViews.find(range);
    if (iter != mCachedImageViews.end()) {
        return iter->second;
    }
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = mTextureImage,
        .viewType = mViewType,
        .format = mVkFormat,
        .components = mSwizzle,
        .subresourceRange = range
    };
    VkImageView imageView;
    vkCreateImageView(mContext.device, &viewInfo, VKALLOC, &imageView);
    mCachedImageViews.emplace(range, imageView);
    return imageView;
}

// TODO: replace the last 4 args with VkImageSubresourceRange
// TODO: replace this function with a flexible thin wrapper over image barrier creation
void VulkanTexture::transitionImageLayout(VkCommandBuffer cmd, VkImage image,
        VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t miplevel,
        uint32_t layerCount, uint32_t levelCount, VkImageAspectFlags aspect) {
    if (oldLayout == newLayout) {
        return;
    }
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect;
    barrier.subresourceRange.baseMipLevel = miplevel;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_GENERAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;

        // We support PRESENT as a target layout to allow blitting from the swap chain.
        // See also makeSwapChainPresentable().
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;

        default:
           PANIC_POSTCONDITION("Unsupported layout transition.");
    }
    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
            &barrier);
}

void VulkanTexture::copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image,
        uint32_t width, uint32_t height, uint32_t depth, FaceOffsets const* faceOffsets, uint32_t miplevel) {
    VkExtent3D extent { width, height, depth };
    if (target == SamplerType::SAMPLER_CUBEMAP) {
        assert_invariant(faceOffsets);
        VkBufferImageCopy regions[6] = {{}};
        for (size_t face = 0; face < 6; face++) {
            auto& region = regions[face];
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;
            region.imageSubresource.mipLevel = miplevel;
            region.imageExtent = extent;
            region.bufferOffset = faceOffsets->offsets[face];
        }
        vkCmdCopyBufferToImage(cmd, buffer, image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6, regions);
        return;
    }
    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = miplevel;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = extent;
    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
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
