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
#include "VulkanMemory.h"
#include "caching/VulkanDescriptorSetManager.h"

#include <utils/Log.h>
#include <utils/Panic.h>

#include "VulkanConstants.h"
#include "VulkanHandles.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"

using namespace bluevk;

namespace filament::backend {

VulkanPipelineCache::VulkanPipelineCache(VkDevice device, VmaAllocator allocator)
    : mDevice(device),
      mAllocator(allocator) {
}

VulkanPipelineCache::~VulkanPipelineCache() {
    // This does nothing because VulkanDriver::terminate() calls terminate() in order to
    // be explicit about teardown order of various components.
}

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
    // Check if the required pipeline is already bound.
    if (cacheEntry->handle == commands->pipeline()) {
        return;
    }

    // If an error occurred, allow higher levels to handle it gracefully.
    assert_invariant(cacheEntry != nullptr && "Failed to create/find pipeline");

    mBoundPipeline = mPipelineRequirements;
    vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cacheEntry->handle);
    commands->setPipeline(cacheEntry->handle);
}

VulkanPipelineCache::PipelineCacheEntry* VulkanPipelineCache::createPipeline() noexcept {
    assert_invariant(mPipelineRequirements.shaders[0] && "Vertex shader is not bound.");
    assert_invariant(mPipelineRequirements.layout && "No pipeline layout specified");

    VkPipelineShaderStageCreateInfo shaderStages[SHADER_MODULE_COUNT];
    shaderStages[0] = VkPipelineShaderStageCreateInfo{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pName = "main";

    shaderStages[1] = VkPipelineShaderStageCreateInfo{};
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pName = "main";

    VkPipelineColorBlendAttachmentState colorBlendAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState = VkPipelineColorBlendStateCreateInfo{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = mPipelineRequirements.rasterState.colorTargetCount;    
    colorBlendState.pAttachments = colorBlendAttachments;

    // If we reach this point, we need to create and stash a brand new pipeline object.
    shaderStages[0].module = mPipelineRequirements.shaders[0];
    shaderStages[1].module = mPipelineRequirements.shaders[1];

    // Expand our size-optimized structs into the proper Vk structs.
    uint32_t numVertexAttribs = 0;
    uint32_t numVertexBuffers = 0;
    VkVertexInputAttributeDescription vertexAttributes[VERTEX_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription vertexBuffers[VERTEX_ATTRIBUTE_COUNT];
    for (uint32_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
        if (mPipelineRequirements.vertexAttributes[i].format > 0) {
            vertexAttributes[numVertexAttribs] = mPipelineRequirements.vertexAttributes[i];
            numVertexAttribs++;
        }
        if (mPipelineRequirements.vertexBuffers[i].stride > 0) {
            vertexBuffers[numVertexBuffers] = mPipelineRequirements.vertexBuffers[i];
            numVertexBuffers++;
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = numVertexBuffers;
    vertexInputState.pVertexBindingDescriptions = vertexBuffers;
    vertexInputState.vertexAttributeDescriptionCount = numVertexAttribs;
    vertexInputState.pVertexAttributeDescriptions = vertexAttributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = (VkPrimitiveTopology) mPipelineRequirements.topology;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkDynamicState dynamicStateEnables[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 2;

    const bool hasFragmentShader = shaderStages[1].module != VK_NULL_HANDLE;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = mPipelineRequirements.layout;
    pipelineCreateInfo.renderPass = mPipelineRequirements.renderPass;
    pipelineCreateInfo.subpass = mPipelineRequirements.subpassIndex;
    pipelineCreateInfo.stageCount = hasFragmentShader ? SHADER_MODULE_COUNT : 1;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;

    VkPipelineRasterizationStateCreateInfo vkRaster = {};
    vkRaster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineCreateInfo.pRasterizationState = &vkRaster;

    VkPipelineMultisampleStateCreateInfo vkMs = {};
    vkMs.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineCreateInfo.pMultisampleState = &vkMs;

    VkPipelineDepthStencilStateCreateInfo vkDs = {};
    vkDs.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkDs.front = vkDs.back = {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .compareMask = 0u,
            .writeMask = 0u,
            .reference = 0u,
    };
    pipelineCreateInfo.pDepthStencilState = &vkDs;

    const auto& raster = mPipelineRequirements.rasterState;

    vkRaster.polygonMode = VK_POLYGON_MODE_FILL;
    vkRaster.cullMode = raster.cullMode;
    vkRaster.frontFace = raster.frontFace;
    vkRaster.depthClampEnable = raster.depthClamp;
    vkRaster.depthBiasEnable = raster.depthBiasEnable;
    vkRaster.depthBiasConstantFactor = raster.depthBiasConstantFactor;
    vkRaster.depthBiasClamp = 0.0f;
    vkRaster.depthBiasSlopeFactor = raster.depthBiasSlopeFactor;
    vkRaster.lineWidth = 1.0f;

    vkMs.rasterizationSamples = (VkSampleCountFlagBits) raster.rasterizationSamples;
    vkMs.sampleShadingEnable = VK_FALSE;
    vkMs.minSampleShading = 0.0f;
    vkMs.alphaToCoverageEnable = raster.alphaToCoverageEnable;
    vkMs.alphaToOneEnable = VK_FALSE;

    vkDs.depthTestEnable = VK_TRUE;
    vkDs.depthWriteEnable = raster.depthWriteEnable;
    vkDs.depthCompareOp = getCompareOp(raster.depthCompareOp);
    vkDs.depthBoundsTestEnable = VK_FALSE;
    vkDs.stencilTestEnable = VK_FALSE;
    vkDs.minDepthBounds = 0.0f;
    vkDs.maxDepthBounds = 0.0f;

    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Filament assumes consistent blend state across all color attachments.
    for (uint8_t i = 0; i < colorBlendState.attachmentCount; ++i) {
        auto& target = colorBlendAttachments[i];
        target.blendEnable = mPipelineRequirements.rasterState.blendEnable;
        target.srcColorBlendFactor = mPipelineRequirements.rasterState.srcColorBlendFactor;
        target.dstColorBlendFactor = mPipelineRequirements.rasterState.dstColorBlendFactor;
        target.colorBlendOp = (VkBlendOp) mPipelineRequirements.rasterState.colorBlendOp;
        target.srcAlphaBlendFactor = mPipelineRequirements.rasterState.srcAlphaBlendFactor;
        target.dstAlphaBlendFactor = mPipelineRequirements.rasterState.dstAlphaBlendFactor;
        target.alphaBlendOp = (VkBlendOp) mPipelineRequirements.rasterState.alphaBlendOp;
        target.colorWriteMask = mPipelineRequirements.rasterState.colorWriteMask;
    }

    // There are no color attachments if there is no bound fragment shader.  (e.g. shadow map gen)
    // TODO: This should be handled in a higher layer.
    if (!hasFragmentShader) {
        colorBlendState.attachmentCount = 0;
    }

    PipelineCacheEntry cacheEntry = {};

    #if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
        FVK_LOGD << "vkCreateGraphicsPipelines with shaders = ("
                 << shaderStages[0].module << ", " << shaderStages[1].module << ")"
                 << utils::io::endl;
    #endif
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

void VulkanPipelineCache::bindRasterState(const RasterState& rasterState) noexcept {
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

#pragma clang diagnostic pop
