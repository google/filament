/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "VulkanBlitter.h"
#include "VulkanContext.h"
#include "VulkanFboCache.h"
#include "VulkanHandles.h"
#include "VulkanSamplerCache.h"
#include "VulkanTexture.h"

#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <smolv.h>

#include "generated/vkshaders/vkshaders.h"

using namespace bluevk;
using namespace utils;

namespace filament::backend {

using ImgUtil = VulkanImageUtility;

namespace {

inline void blitFast(const VkCommandBuffer cmdbuffer, VkImageAspectFlags aspect, VkFilter filter,
        const VkExtent2D srcExtent, VulkanAttachment src, VulkanAttachment dst,
        const VkOffset3D srcRect[2], const VkOffset3D dstRect[2]) {
    const VkImageBlit blitRegions[1] = {{
            .srcSubresource = {aspect, src.level, src.layer, 1},
            .srcOffsets = {srcRect[0], srcRect[1]},
            .dstSubresource = {aspect, dst.level, dst.layer, 1},
            .dstOffsets = {dstRect[0], dstRect[1]},
        }};

    const VkImageResolve resolveRegions[1] = {{
            .srcSubresource = {aspect, src.level, src.layer, 1},
            .srcOffset = srcRect[0],
            .dstSubresource = {aspect, dst.level, dst.layer, 1},
            .dstOffset = dstRect[0],
            .extent = {srcExtent.width, srcExtent.height, 1},
        }};

    const VkImageSubresourceRange srcRange = {
            .aspectMask = aspect,
            .baseMipLevel = src.level,
            .levelCount = 1,
            .baseArrayLayer = src.layer,
            .layerCount = 1,
    };

    const VkImageSubresourceRange dstRange = {
            .aspectMask = aspect,
            .baseMipLevel = dst.level,
            .levelCount = 1,
            .baseArrayLayer = dst.layer,
            .layerCount = 1,
    };

    if constexpr (FVK_ENABLED(FVK_DEBUG_BLITTER)) {
        utils::slog.d << "Fast blit from=" << src.texture->getVkImage() << ",level=" << (int) src.level
                      << " layout=" << src.getLayout()
                      << " to=" << dst.texture->getVkImage() << ",level=" << (int) dst.level
                      << " layout=" << dst.getLayout() << utils::io::endl;
    }

    VulkanLayout oldSrcLayout = src.getLayout();
    VulkanLayout oldDstLayout = dst.getLayout();

    src.texture->transitionLayout(cmdbuffer, srcRange, VulkanLayout::TRANSFER_SRC);
    dst.texture->transitionLayout(cmdbuffer, dstRange, VulkanLayout::TRANSFER_DST);

    if (src.texture->samples > 1 && dst.texture->samples == 1) {
        assert_invariant(
                aspect != VK_IMAGE_ASPECT_DEPTH_BIT && "Resolve with depth is not yet supported.");
        vkCmdResolveImage(cmdbuffer,
                src.getImage(), ImgUtil::getVkLayout(VulkanLayout::TRANSFER_SRC),
                dst.getImage(), ImgUtil::getVkLayout(VulkanLayout::TRANSFER_DST),
                1, resolveRegions);
    } else {
        vkCmdBlitImage(cmdbuffer,
                src.getImage(), ImgUtil::getVkLayout(VulkanLayout::TRANSFER_SRC),
                dst.getImage(), ImgUtil::getVkLayout(VulkanLayout::TRANSFER_DST),
                1, blitRegions, filter);
    }

    if (oldSrcLayout == VulkanLayout::UNDEFINED) {
        oldSrcLayout = ImgUtil::getDefaultLayout(src.texture->usage);
    }
    if (oldDstLayout == VulkanLayout::UNDEFINED) {
        oldDstLayout = ImgUtil::getDefaultLayout(dst.texture->usage);
    }
    src.texture->transitionLayout(cmdbuffer, srcRange, oldSrcLayout);
    dst.texture->transitionLayout(cmdbuffer, dstRange, oldDstLayout);
}

struct BlitterUniforms {
    int sampleCount;
    float inverseSampleCount;
};

}// anonymous namespace

VulkanBlitter::VulkanBlitter(VulkanStagePool& stagePool, VulkanPipelineCache& pipelineCache,
        VulkanFboCache& fboCache, VulkanSamplerCache& samplerCache) noexcept
    : mStagePool(stagePool), mPipelineCache(pipelineCache), mFramebufferCache(fboCache),
      mSamplerCache(samplerCache) {}

void VulkanBlitter::initialize(VkPhysicalDevice physicalDevice, VkDevice device,
        VmaAllocator allocator, VulkanCommands* commands, VulkanTexture* emptyTexture) noexcept {
    mPhysicalDevice = physicalDevice;
    mDevice = device;
    mAllocator = allocator;
    mCommands = commands;
    mEmptyTexture = emptyTexture;
}

void VulkanBlitter::blitColor(BlitArgs args) {
    const VulkanAttachment src = args.srcTarget->getColor(args.targetIndex);
    const VulkanAttachment dst = args.dstTarget->getColor(0);
    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;

#if FVK_ENABLED(FVK_DEBUG_BLIT_FORMAT)
    VkPhysicalDevice const gpu = mPhysicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
                "Source format is not blittable %d", src.getFormat())) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
                "Destination format is not blittable %d", dst.getFormat())) {
        return;
    }
#endif
    VulkanCommandBuffer& commands = mCommands->get();
    VkCommandBuffer const cmdbuffer = commands.buffer();
    commands.acquire(src.texture);
    commands.acquire(dst.texture);

    blitFast(cmdbuffer, aspect, args.filter, args.srcTarget->getExtent(), src, dst,
            args.srcRectPair, args.dstRectPair);
}

void VulkanBlitter::blitDepth(BlitArgs args) {
    const VulkanAttachment src = args.srcTarget->getDepth();
    const VulkanAttachment dst = args.dstTarget->getDepth();
    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

#if FVK_ENABLED(FVK_DEBUG_BLIT_FORMAT)
    VkPhysicalDevice const gpu = mPhysicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
                "Depth src format is not blittable %d", src.getFormat())) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.getFormat(), &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
                "Depth dst format is not blittable %d", dst.getFormat())) {
        return;
    }
#endif

    assert_invariant(src.texture && dst.texture);

    if (src.texture->samples > 1 && dst.texture->samples == 1) {
        blitSlowDepth(args.filter, args.srcTarget->getExtent(), src, dst, args.srcRectPair,
                args.dstRectPair);
        return;
    }

    VulkanCommandBuffer& commands = mCommands->get();
    VkCommandBuffer const cmdbuffer = commands.buffer();
    commands.acquire(src.texture);
    commands.acquire(dst.texture);
    blitFast(cmdbuffer, aspect, args.filter, args.srcTarget->getExtent(), src, dst, args.srcRectPair,
            args.dstRectPair);
}

void VulkanBlitter::terminate() noexcept {
    if (mDepthResolveProgram) {
        delete mDepthResolveProgram;
        mDepthResolveProgram = nullptr;
    }

    if (mTriangleBuffer) {
        delete mTriangleBuffer;
        mTriangleBuffer = nullptr;
    }

    if (mParamsBuffer) {
        delete mParamsBuffer;
        mParamsBuffer = nullptr;
    }
}

// If we created these shader modules in the constructor, the device might not be ready yet.
// It is easier to do lazy initialization, which can also improve load time.
void VulkanBlitter::lazyInit() noexcept {
    if (mDepthResolveProgram != nullptr) {
        return;
    }
    assert_invariant(mDevice);

    auto decode = [device = mDevice](const uint8_t* compressed, int compressedSize) {
        const size_t spirvSize = smolv::GetDecodedBufferSize(compressed, compressedSize);
        FixedCapacityVector<uint8_t> spirv(spirvSize);
        smolv::Decode(compressed, compressedSize, spirv.data(), spirvSize);

        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = spirvSize;
        moduleInfo.pCode = (uint32_t*) spirv.data();

        VkShaderModule result = VK_NULL_HANDLE;
        vkCreateShaderModule(device, &moduleInfo, VKALLOC, &result);
        assert_invariant(result);

        return result;
    };

    VkShaderModule vertexShader = decode(VKSHADERS_BLITDEPTHVS_DATA, VKSHADERS_BLITDEPTHVS_SIZE);
    VkShaderModule fragmentShader = decode(VKSHADERS_BLITDEPTHFS_DATA, VKSHADERS_BLITDEPTHFS_SIZE);

    // Allocate one anonymous sampler at slot 0.
    VulkanProgram::CustomSamplerInfoList samplers = {
        {0, 0, ShaderStageFlags::FRAGMENT},
    };
    mDepthResolveProgram = new VulkanProgram(mDevice, vertexShader, fragmentShader, samplers);

#if FVK_ENABLED(FVK_DEBUG_BLITTER)
    utils::slog.d << "Created Shader Module for VulkanBlitter "
                  << "shaders = (" << vertexShader << ", " << fragmentShader << ")"
                  << utils::io::endl;
#endif

    static const float kTriangleVertices[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        -1.0f, +1.0f,
        +1.0f, +1.0f,
    };

    VulkanCommandBuffer& commands = mCommands->get();
    VkCommandBuffer const cmdbuffer = commands.buffer();

    mTriangleBuffer = new VulkanBuffer(mAllocator, mStagePool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            sizeof(kTriangleVertices));

    mTriangleBuffer->loadFromCpu(cmdbuffer, kTriangleVertices, 0, sizeof(kTriangleVertices));

    mParamsBuffer = new VulkanBuffer(mAllocator, mStagePool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            sizeof(BlitterUniforms));
}

// At a high level, the procedure for resolving depth looks like this:
// 1. Load uniforms
// 2. Begin render pass
// 3. Draw a big triangle
// 4. End render pass.
void VulkanBlitter::blitSlowDepth(VkFilter filter, const VkExtent2D srcExtent, VulkanAttachment src,
        VulkanAttachment dst, const VkOffset3D srcRect[2], const VkOffset3D dstRect[2]) {
    lazyInit();

    VulkanCommandBuffer* commands = &mCommands->get();
    VkCommandBuffer const cmdbuffer = commands->buffer();
    commands->acquire(src.texture);
    commands->acquire(dst.texture);

    BlitterUniforms const uniforms = {
            .sampleCount = src.texture->samples,
            .inverseSampleCount = 1.0f / float(src.texture->samples),
    };
    mParamsBuffer->loadFromCpu(cmdbuffer, &uniforms, 0, sizeof(uniforms));

    VkImageAspectFlags const aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

    // We will transition the src into sampler layout and also keep it in sampler layout for
    // consistency.
    VulkanLayout const samplerLayout = VulkanLayout::DEPTH_SAMPLER;

    // BEGIN RENDER PASS
    // -----------------

    const VulkanFboCache::RenderPassKey rpkey = {
        .initialColorLayoutMask = 0,
        .initialDepthLayout = VulkanLayout::UNDEFINED,
        .renderPassDepthLayout = samplerLayout,
        .finalDepthLayout = samplerLayout,
        .depthFormat = dst.getFormat(),
        .clear = {},
        .discardStart = TargetBufferFlags::DEPTH,
        .samples = 1,
    };

    const VkRenderPass renderPass = mFramebufferCache.getRenderPass(rpkey);
    mPipelineCache.bindRenderPass(renderPass, 0);

    const VulkanFboCache::FboKey fbkey {
        .renderPass = renderPass,
        .width = (uint16_t) (dst.texture->width >> dst.level),
        .height = (uint16_t) (dst.texture->height >> dst.level),
        .layers = 1,
        .samples = rpkey.samples,
        .color = {},
        .resolve = {},
        .depth = dst.getImageView(aspect),
    };
    const VkFramebuffer vkfb = mFramebufferCache.getFramebuffer(fbkey);

    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = vkfb,
    };
    renderPassInfo.renderArea.offset.x = dstRect[0].x;
    renderPassInfo.renderArea.offset.y = dstRect[0].y;
    renderPassInfo.renderArea.extent.width = dstRect[1].x - dstRect[0].x;
    renderPassInfo.renderArea.extent.height = dstRect[1].y - dstRect[0].y;

    // We need to transition the source into a sampler since it'll be sampled in the shader.
    const VkImageSubresourceRange srcRange = {
            .aspectMask = aspect,
            .baseMipLevel = src.level,
            .levelCount = 1,
            .baseArrayLayer = src.layer,
            .layerCount = 1,
    };
    src.texture->transitionLayout(cmdbuffer, srcRange, samplerLayout);

    vkCmdBeginRenderPass(cmdbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = (float) dstRect[0].x,
        .y = (float) dstRect[0].y,
        .width = (float) renderPassInfo.renderArea.extent.width,
        .height = (float) renderPassInfo.renderArea.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    vkCmdSetViewport(cmdbuffer, 0, 1, &viewport);

    // DRAW THE TRIANGLE
    // -----------------

    mPipelineCache.bindProgram(mDepthResolveProgram);
    mPipelineCache.bindPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

    auto vkraster = mPipelineCache.getCurrentRasterState();
    vkraster.depthWriteEnable = true;
    vkraster.depthCompareOp = SamplerCompareFunc::A;
    vkraster.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    vkraster.alphaToCoverageEnable = false,
    vkraster.blendEnable = VK_FALSE,
    vkraster.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    vkraster.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    vkraster.colorBlendOp = BlendEquation::ADD,
    vkraster.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    vkraster.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    vkraster.alphaBlendOp = BlendEquation::ADD,
    vkraster.colorWriteMask = (VkColorComponentFlags) 0,
    vkraster.cullMode = VK_CULL_MODE_NONE;
    vkraster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkraster.depthBiasEnable = VK_FALSE;
    vkraster.colorTargetCount = 0;
    mPipelineCache.bindRasterState(vkraster);

    VkBuffer buffers[1] = {};
    VkDeviceSize offsets[1] = {};
    buffers[0] = mTriangleBuffer->getGpuBuffer();
    VkVertexInputAttributeDescription attribDesc = {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
    };
    VkVertexInputBindingDescription bufferDesc = {
            .binding = 0,
            .stride = sizeof(float) * 2,
    };
    mPipelineCache.bindVertexArray(&attribDesc, &bufferDesc, 1);

    // Select nearest filtering and clamp_to_edge.
    VkSampler vksampler = mSamplerCache.getSampler({});

    VkDescriptorImageInfo samplers[VulkanPipelineCache::SAMPLER_BINDING_COUNT];
    VulkanTexture* textures[VulkanPipelineCache::SAMPLER_BINDING_COUNT] = {nullptr};
    for (auto& sampler : samplers) {
        sampler = {
            .sampler = vksampler,
            .imageView = mEmptyTexture->getPrimaryImageView(),
            .imageLayout = ImgUtil::getVkLayout(VulkanLayout::READ_WRITE),
        };
    }

    samplers[0] = {
        .sampler = vksampler,
        .imageView = src.getImageView(VK_IMAGE_ASPECT_DEPTH_BIT),
        .imageLayout = ImgUtil::getVkLayout(samplerLayout),
    };
    textures[0] = src.texture;

    mPipelineCache.bindSamplers(samplers, textures,
            VulkanPipelineCache::getUsageFlags(0, ShaderStageFlags::FRAGMENT));

    auto previousUbo = mPipelineCache.getUniformBufferBinding(0);
    mPipelineCache.bindUniformBuffer(0, mParamsBuffer->getGpuBuffer());

    if (!mPipelineCache.bindDescriptors(cmdbuffer)) {
        assert_invariant(false);
    }

    const VkRect2D scissor = {
        .offset = renderPassInfo.renderArea.offset,
        .extent = renderPassInfo.renderArea.extent,
    };

    mPipelineCache.bindScissor(cmdbuffer, scissor);

    if (!mPipelineCache.bindPipeline(commands)) {
        assert_invariant(false);
    }

    vkCmdBindVertexBuffers(cmdbuffer, 0, 1, buffers, offsets);
    vkCmdDraw(cmdbuffer, 4, 1, 0, 0);

    // END RENDER PASS
    // ---------------

    vkCmdEndRenderPass(cmdbuffer);

    VkMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    };
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    vkCmdPipelineBarrier(cmdbuffer, srcStageMask,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | // <== For Mali
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 1, &barrier, 0, nullptr, 0, nullptr);

    mPipelineCache.bindUniformBuffer(0, previousUbo.buffer, previousUbo.offset, previousUbo.size);
}

} // namespace filament::backend
