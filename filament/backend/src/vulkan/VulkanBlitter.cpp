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
#include "VulkanHandles.h"

#include <utils/Panic.h>

#include "generated/vkshaders/vkshaders.h"

#define FILAMENT_VULKAN_CHECK_BLIT_FORMAT 0

using namespace bluevk;

namespace filament {
namespace backend {

struct BlitterUniforms {
    int sampleCount;
    float inverseSampleCount;
};

// Helper function for populating barrier fields based on the desired image layout.
// This logic is specific to blitting, please keep this private to VulkanBlitter.
static VulkanLayoutTransition transitionHelper(VulkanLayoutTransition transition) {
    switch (transition.newLayout) {
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_GENERAL:
            transition.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            transition.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            transition.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        default:
            transition.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            transition.dstAccessMask = 0;
            transition.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            transition.dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
    }
    return transition;
}

void VulkanBlitter::blitColor(BlitArgs args) {
    const VulkanAttachment src = args.srcTarget->getColor(args.targetIndex);
    const VulkanAttachment dst = args.dstTarget->getColor(0);
    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;

#if FILAMENT_VULKAN_CHECK_BLIT_FORMAT
    const VkPhysicalDevice gpu = mContext.physicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
            "Source format is not blittable")) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
            "Destination format is not blittable")) {
        return;
    }
#endif

    blitFast(aspect, args.filter, args.srcTarget->getExtent(), src, dst, args.srcRectPair,
            args.dstRectPair);
}

void VulkanBlitter::blitDepth(BlitArgs args) {
    const VulkanAttachment src = args.srcTarget->getDepth();
    const VulkanAttachment dst = args.dstTarget->getDepth();
    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

#if FILAMENT_VULKAN_CHECK_BLIT_FORMAT
    const VkPhysicalDevice gpu = mContext.physicalDevice;
    VkFormatProperties info;
    vkGetPhysicalDeviceFormatProperties(gpu, src.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
            "Depth format is not blittable")) {
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu, dst.format, &info);
    if (!ASSERT_POSTCONDITION_NON_FATAL(info.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
            "Depth format is not blittable")) {
        return;
    }
#endif

    if (src.texture && src.texture->samples > 1 && dst.texture && dst.texture->samples == 1) {
        blitSlowDepth(aspect, args.filter, args.srcTarget->getExtent(), src, dst, args.srcRectPair,
            args.dstRectPair);
        return;
    }

    blitFast(aspect, args.filter, args.srcTarget->getExtent(), src, dst, args.srcRectPair,
            args.dstRectPair);
}

void VulkanBlitter::blitFast(VkImageAspectFlags aspect, VkFilter filter,
    const VkExtent2D srcExtent, VulkanAttachment src, VulkanAttachment dst,
    const VkOffset3D srcRect[2], const VkOffset3D dstRect[2]) {
    const VkImageBlit blitRegions[1] = {{
        .srcSubresource = { aspect, src.level, src.layer, 1 },
        .srcOffsets = { srcRect[0], srcRect[1] },
        .dstSubresource = { aspect, dst.level, dst.layer, 1 },
        .dstOffsets = { dstRect[0], dstRect[1] }
    }};

    const VkImageResolve resolveRegions[1] = {{
        .srcSubresource = { aspect, src.level, src.layer, 1 },
        .srcOffset = srcRect[0],
        .dstSubresource = { aspect, dst.level, dst.layer, 1 },
        .dstOffset = dstRect[0],
        .extent = { srcExtent.width, srcExtent.height, 1 }
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

    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;

    transitionImageLayout(cmdbuffer, {
        src.image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        srcRange,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT
    });

    transitionImageLayout(cmdbuffer, {
        dst.image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        dstRange,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
    });

    if (src.texture && src.texture->samples > 1 && dst.texture && dst.texture->samples == 1) {
        assert_invariant(aspect != VK_IMAGE_ASPECT_DEPTH_BIT && "Resolve with depth is not yet supported.");
        vkCmdResolveImage(cmdbuffer, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, resolveRegions);
    } else {
        vkCmdBlitImage(cmdbuffer, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, blitRegions, filter);
    }

    if (src.texture) {
        transitionImageLayout(cmdbuffer, transitionHelper({
            .image = src.image,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = mContext.getTextureLayout(src.texture->usage),
            .subresources = srcRange
        }));
    } else if (!mContext.currentSurface->headlessQueue) {
        transitionImageLayout(cmdbuffer, transitionHelper({
            .image = src.image,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .subresources = srcRange
        }));
    }

    // Determine the desired texture layout for the destination while ensuring that the default
    // render target is supported, which has no associated texture.
    const VkImageLayout desiredLayout = dst.texture ?
            mContext.getTextureLayout(dst.texture->usage) :
            mContext.currentSurface->getColor().layout;

    transitionImageLayout(cmdbuffer, transitionHelper({
        .image = dst.image,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = desiredLayout,
        .subresources = dstRange,
    }));
}

void VulkanBlitter::shutdown() noexcept {
    if (mContext.device) {
        delete mDepthResolveProgram;
        mDepthResolveProgram = nullptr;

        delete mTriangleBuffer;
        mTriangleBuffer = nullptr;

        delete mParamsBuffer;
        mParamsBuffer = nullptr;

        // delete mTriangleVertexBuffer;
        // mTriangleVertexBuffer = nullptr;

        // delete mRenderPrimitive;
        // mRenderPrimitive = nullptr;
    }
}

// If we created these shader modules in the constructor, the device might not be ready yet.
// It is easier to do lazy initialization, which can also improve load time.
void VulkanBlitter::lazyInit() noexcept {
    if (mDepthResolveProgram != nullptr) {
        return;
    }
    assert_invariant(mContext.device);

    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    VkResult result;

    VkShaderModule vertexShader;
    moduleInfo.codeSize = VKSHADERS_BLITDEPTHVS_SIZE;
    moduleInfo.pCode = (uint32_t*) VKSHADERS_BLITDEPTHVS_DATA;
    result = vkCreateShaderModule(mContext.device, &moduleInfo, VKALLOC, &vertexShader);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create vertex shader for blit.");

    VkShaderModule fragmentShader;
    moduleInfo.codeSize = VKSHADERS_BLITDEPTHFS_SIZE;
    moduleInfo.pCode = (uint32_t*) VKSHADERS_BLITDEPTHFS_DATA;
    result = vkCreateShaderModule(mContext.device, &moduleInfo, VKALLOC, &fragmentShader);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Unable to create fragment shader for blit.");

    mDepthResolveProgram = new VulkanProgram(mContext, vertexShader, fragmentShader);

    static const float kTriangleVertices[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        -1.0f, +1.0f,
        +1.0f, +1.0f,
    };

    mTriangleBuffer = new VulkanBuffer(mContext, mStagePool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            sizeof(kTriangleVertices));

    mTriangleBuffer->loadFromCpu(kTriangleVertices, 0, sizeof(kTriangleVertices));

    mParamsBuffer = new VulkanUniformBuffer(mContext, mStagePool,
            sizeof(BlitterUniforms), backend::BufferUsage::STATIC);

    mTriangleBuffer->loadFromCpu(kTriangleVertices, 0, sizeof(kTriangleVertices));

    // mDisposer.acquire(mTriangleBuffer);

    // AttributeArray attributes = {};
    // attributes[0].buffer = 0;
    // attributes[0].type = ElementType::FLOAT2;
    // attributes[0].stride = sizeof(float) * 2;
    // mTriangleVertexBuffer = new VulkanVertexBuffer(mContext, mStagePool, 1, 1, 4, attributes);

    // mRenderPrimitive = new VulkanRenderPrimitive();
    // mRenderPrimitive->setPrimitiveType(PrimitiveType::TRIANGLES);
    // mRenderPrimitive->vertexBuffer = mTriangleVertexBuffer;
}

// At a high level, the procedure for resolving depth looks like this:
// 1. Load uniforms
// 2. Begin render pass
// 3. Draw a big triangle
// 4. End render pass.
void VulkanBlitter::blitSlowDepth(VkImageAspectFlags aspect, VkFilter filter,
        const VkExtent2D srcExtent, VulkanAttachment src, VulkanAttachment dst,
        const VkOffset3D srcRect[2], const VkOffset3D dstRect[2]) {
    lazyInit();

    const BlitterUniforms uniforms = {
        .sampleCount = src.texture->samples,
        .inverseSampleCount = 1.0f / float(src.texture->samples),
    };
    mParamsBuffer->loadFromCpu(&uniforms, sizeof(uniforms));

    // BEGIN RENDER PASS

    const VkImageLayout layout = mContext.getTextureLayout(TextureUsage::DEPTH_ATTACHMENT);

    VulkanFboCache::RenderPassKey rpkey = {
        .depthLayout = layout,
        .depthFormat = dst.format,
        .clear = {},
        .discardStart = TargetBufferFlags::DEPTH,
        .samples = 1,
    };

    const VkRenderPass renderPass = mFramebufferCache.getRenderPass(rpkey);
    mPipelineCache.bindRenderPass(renderPass, 0);

    VulkanFboCache::FboKey fbkey {
        .renderPass = renderPass,
        .width = (uint16_t) (dst.texture->width >> dst.level),
        .height = (uint16_t) (dst.texture->height >> dst.level),
        .layers = 1,
        .samples = rpkey.samples,
        .color = {},
        .resolve = {},
        .depth = dst.view,
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

//    rt->transformClientRectToPlatform(&renderPassInfo.renderArea);

    // Even though we don't clear anything, we have to provide a clear value.
    VkClearValue clearValues[1] = {};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = clearValues;

    const VkCommandBuffer cmdbuffer = mContext.commands->get().cmdbuffer;
    vkCmdBeginRenderPass(cmdbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = mContext.viewport = {
        .x = (float) dstRect[0].x,
        .y = (float) dstRect[0].y,
        .width = (float) renderPassInfo.renderArea.extent.width,
        .height = (float) renderPassInfo.renderArea.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    // mCurrentRenderTarget->transformClientRectToPlatform(&viewport);
    vkCmdSetViewport(cmdbuffer, 0, 1, &viewport);

    // DRAW

    mPipelineCache.bindProgramBundle(mDepthResolveProgram->bundle);
    mPipelineCache.bindPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

    mContext.rasterState.depthStencil = {
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = true,
        .depthCompareOp = VK_COMPARE_OP_ALWAYS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };
    mContext.rasterState.multisampling = {
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .alphaToCoverageEnable = false,
    };
    mContext.rasterState.blending = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = (VkColorComponentFlags) 0,
    };
    auto& vkraster = mContext.rasterState.rasterization;
    vkraster.cullMode = VK_CULL_MODE_NONE;
    vkraster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkraster.depthBiasEnable = VK_FALSE;
    mContext.rasterState.colorTargetCount = 0;
    mPipelineCache.bindRasterState(mContext.rasterState);

    VulkanPipelineCache::VertexArray varray = {};
    VkBuffer buffers[1] = {};
    VkDeviceSize offsets[1] = {};
    buffers[0] = mTriangleBuffer->getGpuBuffer();
    varray.attributes[0] = { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT };
    varray.buffers[0] = { .binding = 0, .stride = sizeof(float) * 2 };
    mPipelineCache.bindVertexArray(varray);

    // NEAREST and CLAMP_TO_EDGE.
    SamplerParams samplerParams = {};
    VkSampler vksampler = mSamplerCache.getSampler(samplerParams);

    VkDescriptorImageInfo samplers[VulkanPipelineCache::SAMPLER_BINDING_COUNT];
    for (auto& sampler : samplers) {
        sampler = {
            .sampler = vksampler,
            .imageView = mContext.emptyTexture->getPrimaryImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL
        };
    }

    samplers[0] = {
        .sampler = vksampler,
        .imageView = src.view,
        .imageLayout = layout
    };

    mPipelineCache.bindSamplers(samplers);

    auto previousUbo = mPipelineCache.getUniformBufferBinding(0);
    mPipelineCache.bindUniformBuffer(0, mParamsBuffer->getGpuBuffer());

    if (!mPipelineCache.bindDescriptors(cmdbuffer)) {
        assert_invariant(false);
    }

    const VkRect2D scissor = {
        .offset = renderPassInfo.renderArea.offset,
        .extent = renderPassInfo.renderArea.extent,
    };

    // rt->transformClientRectToPlatform(&scissor);
    mPipelineCache.bindScissor(cmdbuffer, scissor);

    // Bind a new pipeline if the pipeline state changed.
    mPipelineCache.bindPipeline(cmdbuffer);

    // Next bind the vertex buffers and index buffer. One potential performance improvement is to
    // avoid rebinding these if they are already bound, but since we do not (yet) support subranges
    // it would be rare for a client to make consecutive draw calls with the same render primitive.
    vkCmdBindVertexBuffers(cmdbuffer, 0, 1, buffers, offsets);

    vkCmdDraw(cmdbuffer, 4, 1, 0, 0);

    // END RENDER PASS

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

} // namespace filament
} // namespace backend
