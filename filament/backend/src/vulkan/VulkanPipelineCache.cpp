/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "VulkanPipelineCache.h"

#include <utils/Log.h>
#include <utils/Panic.h>

#include "VulkanConstants.h"
#include "VulkanHandles.h"
#include "vulkan/utils/Conversion.h"

#if defined(__clang__)
// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPipelineCache::VulkanPipelineCache(VkDevice device)
        : mDevice(device) {}

void VulkanPipelineCache::bindLayout(VkPipelineLayout layout) noexcept {
    mPipelineRequirements.layout = layout;
}

VulkanPipelineCache::PipelineCacheEntry* VulkanPipelineCache::getOrCreatePipeline() noexcept {
    // If a cached object exists, re-use it, otherwise create a new one.
    if (PipelineMap::iterator pipelineIter = mPipelines.find(mPipelineRequirements);
            pipelineIter != mPipelines.end()) {
        auto& pipeline = pipelineIter.value();
        pipeline.lastUsed = mCurrentTime;
        return &pipeline;
    }
    auto ret = createPipeline();
    ret->lastUsed = mCurrentTime;
    return ret;
}

void VulkanPipelineCache::bindPipeline(VulkanCommandBuffer* commands) {
    VkCommandBuffer const cmdbuffer = commands->buffer();
    PipelineCacheEntry* cacheEntry = getOrCreatePipeline();

    // If an error occurred, allow higher levels to handle it gracefully.
    assert_invariant(cacheEntry != nullptr && "Failed to create/find pipeline");

    mBoundPipeline = mPipelineRequirements;
    vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cacheEntry->handle);
}

VulkanPipelineCache::PipelineCacheEntry* VulkanPipelineCache::createPipeline() noexcept {
    assert_invariant(mPipelineRequirements.shaders[0] && "Vertex shader is not bound.");
    assert_invariant(mPipelineRequirements.layout && "No pipeline layout specified");

    VkPipelineShaderStageCreateInfo shaderStages[SHADER_MODULE_COUNT];
    shaderStages[0] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = mPipelineRequirements.shaders[0],
        .pName = "main",
    };
    shaderStages[1] = shaderStages[0];
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = mPipelineRequirements.shaders[1];

    bool const hasFragmentShader = shaderStages[1].module != VK_NULL_HANDLE;

    VkPipelineColorBlendAttachmentState colorBlendAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
    VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = mPipelineRequirements.rasterState.colorTargetCount,
        .pAttachments = colorBlendAttachments,
    };

    // Expand our size-optimized structs into the proper Vk structs.
    uint32_t numVertexAttribs = 0;
    uint32_t numVertexBuffers = 0;

    VkVertexInputAttributeDescription vertexAttributes[VERTEX_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription vertexBuffers[VERTEX_ATTRIBUTE_COUNT];
    for (uint32_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
        if (mPipelineRequirements.vertexAttributes[i].format > 0) {
            vertexAttributes[numVertexAttribs++] = mPipelineRequirements.vertexAttributes[i];
        }
        if (mPipelineRequirements.vertexBuffers[i].stride > 0) {
            vertexBuffers[numVertexBuffers++] = mPipelineRequirements.vertexBuffers[i];
        }
    }
    VkPipelineVertexInputStateCreateInfo vertexInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = numVertexBuffers,
        .pVertexBindingDescriptions = vertexBuffers,
        .vertexAttributeDescriptionCount = numVertexAttribs,
        .pVertexAttributeDescriptions = vertexAttributes,
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = (VkPrimitiveTopology) mPipelineRequirements.topology,
    };
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkDynamicState dynamicStateEnables[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStateEnables,
    };
    auto const& raster = mPipelineRequirements.rasterState;
    VkPipelineRasterizationStateCreateInfo vkRaster = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = raster.depthClamp,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = raster.cullMode,
        .frontFace = raster.frontFace,
        .depthBiasEnable = raster.depthBiasEnable,
        .depthBiasConstantFactor = raster.depthBiasConstantFactor,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = raster.depthBiasSlopeFactor,
        .lineWidth = 1.0f,
    };
    VkPipelineMultisampleStateCreateInfo vkMs = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = (VkSampleCountFlagBits) raster.rasterizationSamples,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .alphaToCoverageEnable = raster.alphaToCoverageEnable,
        .alphaToOneEnable = VK_FALSE,
    };
    VkPipelineDepthStencilStateCreateInfo vkDs = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = raster.depthWriteEnable,
        .depthCompareOp = fvkutils::getCompareOp(raster.depthCompareOp),
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 0.0f,
    };
    vkDs.front = vkDs.back = {
        .failOp = VK_STENCIL_OP_KEEP,
        .passOp = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .compareMask = 0u,
        .writeMask = 0u,
        .reference = 0u,
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = hasFragmentShader ? SHADER_MODULE_COUNT : 1,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &vkRaster,
        .pMultisampleState = &vkMs,
        .pDepthStencilState = &vkDs,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = mPipelineRequirements.layout,
        .renderPass = mPipelineRequirements.renderPass,
        .subpass = mPipelineRequirements.subpassIndex,
    };

    // There are no color attachments if there is no bound fragment shader.  (e.g. shadow map gen)
    // TODO: This should be handled in a higher layer.
    if (!hasFragmentShader) {
        colorBlendState.attachmentCount = 0;
    } else {
        // Filament assumes consistent blend state across all color attachments.
        colorBlendAttachments[0] = {
            .blendEnable = raster.blendEnable,
            .srcColorBlendFactor = raster.srcColorBlendFactor,
            .dstColorBlendFactor = raster.dstColorBlendFactor,
            .colorBlendOp = (VkBlendOp) raster.colorBlendOp,
            .srcAlphaBlendFactor = raster.srcAlphaBlendFactor,
            .dstAlphaBlendFactor = raster.dstAlphaBlendFactor,
            .alphaBlendOp = (VkBlendOp) raster.alphaBlendOp,
            .colorWriteMask = raster.colorWriteMask,
        };
        for (uint8_t i = 1; i < colorBlendState.attachmentCount; ++i) {
            colorBlendAttachments[i] = colorBlendAttachments[0];
        }
    }

    #if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
        FVK_LOGD << "vkCreateGraphicsPipelines with shaders = ("
                 << shaderStages[0].module << ", " << shaderStages[1].module << ")"
                 << utils::io::endl;
    #endif
    PipelineCacheEntry cacheEntry = {
        .lastUsed = mCurrentTime,
    };
    VkResult error = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
            VKALLOC, &cacheEntry.handle);
    assert_invariant(error == VK_SUCCESS);
    if (error != VK_SUCCESS) {
        FVK_LOGE << "vkCreateGraphicsPipelines error " << error << utils::io::endl;
        return nullptr;
    }
    return &mPipelines.emplace(mPipelineRequirements, cacheEntry).first.value();
}

void VulkanPipelineCache::bindProgram(fvkmemory::resource_ptr<VulkanProgram> program) noexcept {
    mPipelineRequirements.shaders[0] = program->getVertexShader();
    mPipelineRequirements.shaders[1] = program->getFragmentShader();

    // If this is a debug build, validate the current shader.
#if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
    if (mPipelineRequirements.shaders[0] == VK_NULL_HANDLE ||
            mPipelineRequirements.shaders[1] == VK_NULL_HANDLE) {
        FVK_LOGE << "Binding missing shader: " << program->name.c_str() << utils::io::endl;
    }
#endif
}

void VulkanPipelineCache::bindRasterState(RasterState const& rasterState) noexcept {
    mPipelineRequirements.rasterState = rasterState;
}

void VulkanPipelineCache::bindRenderPass(VkRenderPass renderPass, int subpassIndex) noexcept {
    mPipelineRequirements.renderPass = renderPass;
    mPipelineRequirements.subpassIndex = subpassIndex;
}

void VulkanPipelineCache::bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept {
    assert_invariant(uint32_t(topology) <= 0xffffu);
    mPipelineRequirements.topology = topology;
}

void VulkanPipelineCache::bindVertexArray(VkVertexInputAttributeDescription const* attribDesc,
        VkVertexInputBindingDescription const* bufferDesc, uint8_t count) {
    for (size_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
        if (i < count) {
            mPipelineRequirements.vertexAttributes[i] = attribDesc[i];
            mPipelineRequirements.vertexBuffers[i] = bufferDesc[i];
        } else {
            mPipelineRequirements.vertexAttributes[i] = {};
            mPipelineRequirements.vertexBuffers[i] = {};
        }
    }
}

void VulkanPipelineCache::terminate() noexcept {
    for (auto& iter : mPipelines) {
        vkDestroyPipeline(mDevice, iter.second.handle, VKALLOC);
    }
    mPipelines.clear();
    mBoundPipeline = {};
}

void VulkanPipelineCache::gc() noexcept {
    // The timestamp associated with a given cache entry represents "time" as a count of flush
    // events since the cache was constructed. If any cache entry was most recently used over
    // FVK_MAX_PIPELINE_AGE flush events in the past, then we can be sure that it is no longer
    // being used by the GPU, and is therefore safe to destroy or reclaim.
    ++mCurrentTime;

    // The Vulkan spec says: "When a command buffer begins recording, all state in that command
    // buffer is undefined." Therefore, we need to clear all bindings at this time.
    mBoundPipeline = {};

    // NOTE: Due to robin_map restrictions, we cannot use auto or range-based loops.

    // Evict any pipelines that have not been used in a while.
    // Any pipeline older than FVK_MAX_COMMAND_BUFFERS can be safely destroyed.
   using ConstPipeIterator = decltype(mPipelines)::const_iterator;
   for (ConstPipeIterator iter = mPipelines.begin(); iter != mPipelines.end();) {
       const PipelineCacheEntry& cacheEntry = iter.value();
       if (cacheEntry.lastUsed + FVK_MAX_PIPELINE_AGE < mCurrentTime) {
           vkDestroyPipeline(mDevice, iter->second.handle, VKALLOC);
           iter = mPipelines.erase(iter);
       } else {
           ++iter;
       }
   }
}

bool VulkanPipelineCache::PipelineEqual::operator()(const PipelineKey& k1,
        const PipelineKey& k2) const {
    return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(k1));
}

} // namespace filament::backend

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
