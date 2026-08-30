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

#include "VulkanConstants.h"
#include "VulkanHandles.h"

#include "vulkan/utils/Conversion.h"

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#if defined(__clang__)
// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif

using namespace bluevk;

namespace filament::backend {

namespace {

using utils::JobSystem;

enum DynamicStateBits : uint16_t {
    DIRTY_NONE                 = 0,
    DIRTY_DEPTH_BIAS           = 1 << 0,
    // VK_EXT_extended_dynamic_state
    DIRTY_CULL_MODE            = 1 << 1,
    DIRTY_DEPTH_COMPARE_OP     = 1 << 2,
    DIRTY_DEPTH_TEST_ENABLE    = 1 << 3,
    DIRTY_DEPTH_WRITE_ENABLE   = 1 << 4,
    DIRTY_FRONT_FACE           = 1 << 5,
    DIRTY_PRIMITIVE_TOPOLOGY   = 1 << 6,
    DIRTY_STENCIL_STATE        = 1 << 7,
    DIRTY_STENCIL_TEST_ENABLE  = 1 << 8,
    // VK_EXT_extended_dynamic_state2
    DIRTY_DEPTH_BIAS_ENABLE    = 1 << 9,
    // VK_EXT_vertex_input_dynamic_state
    DIRTY_VERTEX_INPUT         = 1 << 10,
};

constexpr bool isDepthTestEnabled(VulkanPipelineCache::RasterState const& raster) noexcept {
    return raster.depthCompareOp != SamplerCompareFunc::A || raster.depthWriteEnable;
}

constexpr bool isStencilTestEnabled(StencilState const& stencil) noexcept {
    return stencil.front.stencilFunc != StencilState::StencilFunction::A ||
           stencil.back.stencilFunc != StencilState::StencilFunction::A ||
           stencil.front.stencilOpDepthFail != StencilOperation::KEEP ||
           stencil.back.stencilOpDepthFail != StencilOperation::KEEP ||
           stencil.front.stencilOpStencilFail != StencilOperation::KEEP ||
           stencil.back.stencilOpStencilFail != StencilOperation::KEEP ||
           stencil.front.stencilOpDepthStencilPass != StencilOperation::KEEP ||
           stencil.back.stencilOpDepthStencilPass != StencilOperation::KEEP ||
           stencil.stencilWrite;
}

#if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
void printPipelineFeedbackInfo(VkPipelineCreationFeedbackCreateInfo const& feedbackInfo) {
    VkPipelineCreationFeedback const& pipelineInfo = *feedbackInfo.pPipelineCreationFeedback;
    if (!(pipelineInfo.flags & VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT)) {
        return;
    }

    bool const isCacheHit =
            (pipelineInfo.flags & VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT);
    FVK_LOGD << "Pipeline build stats - Cache hit: " << isCacheHit
             << ", Time: " << pipelineInfo.duration / 1000000.0 << "ms";

    for (uint32_t i = 0; i < feedbackInfo.pipelineStageCreationFeedbackCount; ++i) {
        VkPipelineCreationFeedback const& stageInfo = feedbackInfo.pPipelineStageCreationFeedbacks[i];
        if (!(stageInfo.flags & VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT)) {
            continue;
        }

        bool const isVertexShader = (i == 0);
        bool const isCacheHit = (stageInfo.flags &
                                 VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT);
        FVK_LOGD << (isVertexShader ? "Vertex" : "Fragment")
                 << " shader build stats - Cache hit: " << isCacheHit
                 << ", Time: " << stageInfo.duration / 1000000.0 << "ms";
    }
}
#endif

} // namespace

VulkanPipelineCache::VulkanPipelineCache(DriverBase& driver, VkDevice device, VulkanContext const& context)
        : mDevice(device),
          mCallbackManager(driver),
          mHasVertexInputDynamicState(context.isVertexInputDynamicStateSupported() && context.isPipelineDynamicStateEnabled()),
          mHasDynamicState(context.isExtendedDynamicStateSupported() && context.isPipelineDynamicStateEnabled()),
          mHasDynamicState2(context.isExtendedDynamicState2Supported() && context.isPipelineDynamicStateEnabled()),
          mContext(context) {
    VkPipelineCacheCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
    };
    bluevk::vkCreatePipelineCache(mDevice, &createInfo, VKALLOC, &mPipelineCache);

    if (mContext.shouldUsePipelineCachePrewarming()) {
        mCompilerThreadPool.init(
            /*threadCount=*/1,
            []() {
                JobSystem::setThreadName("CompilerThreadPool");
                // This thread should be lower priority than the main thread.
                JobSystem::setThreadPriority(JobSystem::Priority::DISPLAY);
            }, []() {
                // No cleanup required.
            });
    }

    FVK_LOGD << "Vertex input dynamic state supported: " << mHasVertexInputDynamicState;
    FVK_LOGD << "Extended dynamic state supported: " << mHasDynamicState;
    FVK_LOGD << "Extended dynamic state 2 supported: " << mHasDynamicState2;
}

void VulkanPipelineCache::bindLayout(VkPipelineLayout layout) noexcept {
    mStaticPipelineKey.layout = layout;
}

VulkanPipelineCache::PipelineCacheEntry* VulkanPipelineCache::getOrCreatePipeline() noexcept {
    // If a cached object exists, re-use it, otherwise create a new one.
    if (PipelineMap::iterator pipelineIter = mPipelines.find(mStaticPipelineKey);
            pipelineIter != mPipelines.end()) {
        auto& pipeline = pipelineIter.value();
        pipeline.lastUsed = mCurrentTime;
        FILAMENT_TRACING_EVENT(FILAMENT_TRACING_CATEGORY_FILAMENT,
                "Pipeline(Hit)", "program", mBoundProgram.c_str_safe());
        return &pipeline;
    }
    FILAMENT_TRACING_EVENT(FILAMENT_TRACING_CATEGORY_FILAMENT,
            "Pipeline(Miss)", "program", mBoundProgram.c_str_safe());
    PipelineCacheEntry cacheEntry {
        .handle = createPipeline(mStaticPipelineKey),
        .lastUsed = mCurrentTime,
    };
    assert_invariant(cacheEntry.handle != VK_NULL_HANDLE && "Pipeline handle is VK_NULL_HANDLE");
    return &mPipelines.emplace(mStaticPipelineKey, cacheEntry).first.value();
}

void VulkanPipelineCache::bindPipeline(VulkanCommandBuffer* commands) {
    VkCommandBuffer const cmdbuffer = commands->buffer();

    // The frontend's PipelineState may trigger a backend bindPipeline call due to dynamic
    // state changes (e.g. vertex buffer info, polygon offset, or raster state). However, when
    // dynamic extensions are enabled, mStaticPipelineKey remains unchanged across these calls.
    // By checking if mBoundPipeline matches mStaticPipelineKey first, we avoid expensive
    // MurmurHash calculations and hash map lookups on consecutive draws sharing the same static pipeline.
    static PipelineEqual equal;
    if (UTILS_UNLIKELY(!equal(mBoundPipeline, mStaticPipelineKey))) {
        PipelineCacheEntry* cacheEntry = getOrCreatePipeline();
        assert_invariant(cacheEntry != nullptr && "Failed to create/find pipeline");
        mBoundPipeline = mStaticPipelineKey;
        vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cacheEntry->handle);
    }

    uint16_t const dirtyMask = computeDirtyDynamicState();
    if (dirtyMask == DIRTY_NONE) {
        return;
    }

    bindDynamicState(cmdbuffer, dirtyMask);
}

void VulkanPipelineCache::asyncPrewarmCache(
        resource_ptr<VulkanProgram> vprogram,
        VkPipelineLayout layout,
        StereoscopicType stereoscopicType,
        uint8_t stereoscopicViewCount,
        CompilerPriorityQueue priority) {
    PipelineKey key {
        .shaders = {
            vprogram->getVertexShader(),
            vprogram->getFragmentShader(),
        },
        // We're using dynamic rendering, so this should be empty.
        .renderPass = VK_NULL_HANDLE,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .subpassIndex = 0,
        // We're using vertex input dynamic state, so these should be empty.
        .vertexAttributes = {},
        .vertexBuffers = {},
        // Create a reasonable default raster state; we're assuming this is not cached.
        .rasterState = {
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .blendEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE,
            .alphaToCoverageEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .colorWriteMask = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .depthClamp = VK_FALSE,
            .colorTargetCount = 1,
            .colorBlendOp = BlendEquation::SUBTRACT,
            .alphaBlendOp = BlendEquation::SUBTRACT,
            .depthCompareOp = SamplerCompareFunc::L,
            .depthBiasConstantFactor = 0.f,
            .depthBiasSlopeFactor = 0.f,
        },
        .stencilState = {},
        .layout = layout,
    };
    PipelineDynamicOptions dynamicOptions {
        .useDynamicVertexInputState = true,
        .useDynamicRenderPasses = true,
        .stereoscopicType = stereoscopicType,
        .stereoscopicViewCount = stereoscopicViewCount,
    };

    CallbackManager::Handle cmh = mCallbackManager.get();
    auto token = std::make_shared<ProgramToken>();
    // Note: we keep a ref to vprogram in this lambda so that the shader modules don't get
    // destroyed before we call createPipeline(). We can catch this in some cases, to avoid
    // compiling too many unnecessary already-destroyed-materials.
    mCompilerThreadPool.queue(
        priority, token, [this, vprogram, key, dynamicOptions, cmh, priority]() mutable {
            UTILS_UNUSED_WITHOUT_TRACING const CompilerPriorityQueue priorityQueue = priority;
            FILAMENT_TRACING_EVENT(FILAMENT_TRACING_CATEGORY_FILAMENT,
                    "asyncPrewarmCache(Job)",
                    "program", vprogram->programString.c_str_safe(),
                    "priority", static_cast<uint32_t>(priorityQueue));
            if (vprogram->isParallelCompilationCanceled()) {
                FVK_LOGD << "Skipping prewarm for a program that has been destroyed already.";
                return;
            }

            VkPipeline pipeline = createPipeline(key, dynamicOptions);
            mCallbackManager.put(cmh);
            // We don't actually need this pipeline, we just wanted to force the
            // driver to cache the pipeline's information.
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(mDevice, pipeline, VKALLOC);
            } else {
                FVK_LOGW << "Failed to create a pipeline during prewarming, draw-time pipeline "
                            "creation may fail.";
            }
        });
}

VkPipeline VulkanPipelineCache::createPipeline(
        const PipelineKey& key, const PipelineDynamicOptions& dynamicOptions) noexcept {
    assert_invariant(key.shaders[0] && "Vertex shader is not bound.");
    assert_invariant(key.layout && "No pipeline layout specified");

    VkPipelineShaderStageCreateInfo shaderStages[SHADER_MODULE_COUNT];
    shaderStages[0] = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = key.shaders[0],
        .pName = "main",
    };
    shaderStages[1] = shaderStages[0];
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = key.shaders[1];

    bool const hasFragmentShader = shaderStages[1].module != VK_NULL_HANDLE;

    VkPipelineColorBlendAttachmentState colorBlendAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
    VkPipelineColorBlendStateCreateInfo colorBlendState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = key.rasterState.colorTargetCount,
        .pAttachments = colorBlendAttachments,
    };

    const bool hasVertexInputState = !dynamicOptions.useDynamicVertexInputState && !mHasVertexInputDynamicState;

    VkPipelineVertexInputStateCreateInfo vertexInputState;
    VkVertexInputAttributeDescription vertexAttributes[VERTEX_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription vertexBuffers[VERTEX_ATTRIBUTE_COUNT];
    if (hasVertexInputState) {
        uint32_t numVertexAttribs = 0;
        uint32_t numVertexBuffers = 0;

        for (uint32_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
            if (key.vertexAttributes[i].format > 0) {
                vertexAttributes[numVertexAttribs++] = key.vertexAttributes[i];
            }
            if (key.vertexBuffers[i].stride > 0) {
                vertexBuffers[numVertexBuffers++] = key.vertexBuffers[i];
            }
        }
        vertexInputState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = numVertexBuffers,
            .pVertexBindingDescriptions = vertexBuffers,
            .vertexAttributeDescriptionCount = numVertexAttribs,
            .pVertexAttributeDescriptions = vertexAttributes,
        };
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        // For dynamic state avoid VUID-VkGraphicsPipelineCreateInfo-topology-08773
        .topology = mHasDynamicState ?
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : (VkPrimitiveTopology) key.topology,
    };
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    constexpr size_t maxDynamicStates = 16;
    size_t numDynamicStates = 2;
    VkDynamicState enabledDynamicStates[maxDynamicStates] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    if (!hasVertexInputState) {
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;
    }

    if (mHasDynamicState) {
        // VK_DYNAMIC_STATE_DEPTH_BIAS is always available but for now only use it with
        // VK_EXT_extended_dynamic_state.
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_DEPTH_BIAS;

        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_CULL_MODE_EXT;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_FRONT_FACE_EXT;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_STENCIL_OP_EXT;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT;

        // These dynamic states are always available but only used with
        // VK_EXT_extended_dynamic_state because StencilState is tracked as a whole.
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    }

    if (mHasDynamicState2) {
        enabledDynamicStates[numDynamicStates++] = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT;
    }

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(numDynamicStates),
        .pDynamicStates = enabledDynamicStates,
    };

    auto const& raster = key.rasterState;
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
    bool const enableDepthTest = isDepthTestEnabled(raster);
    bool const enableStencilTest = isStencilTestEnabled(key.stencilState);
    auto const& stencil = key.stencilState;
    VkPipelineDepthStencilStateCreateInfo vkDs = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = enableDepthTest ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = raster.depthWriteEnable,
        .depthCompareOp = fvkutils::getCompareOp(raster.depthCompareOp),
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = enableStencilTest ? VK_TRUE : VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 0.0f,
    };
    vkDs.front = {
        .failOp = fvkutils::getStencilOp(stencil.front.stencilOpStencilFail),
        .passOp = fvkutils::getStencilOp(stencil.front.stencilOpDepthStencilPass),
        .depthFailOp = fvkutils::getStencilOp(stencil.front.stencilOpDepthFail),
        .compareOp = fvkutils::getCompareOp(stencil.front.stencilFunc),
        .compareMask = stencil.front.readMask,
        .writeMask = (uint32_t) (stencil.stencilWrite ? stencil.front.writeMask : 0u),
        .reference = (uint32_t) stencil.front.ref,
    };
    vkDs.back = {
        .failOp = fvkutils::getStencilOp(stencil.back.stencilOpStencilFail),
        .passOp = fvkutils::getStencilOp(stencil.back.stencilOpDepthStencilPass),
        .depthFailOp = fvkutils::getStencilOp(stencil.back.stencilOpDepthFail),
        .compareOp = fvkutils::getCompareOp(stencil.back.stencilFunc),
        .compareMask = stencil.back.readMask,
        .writeMask = (uint32_t) (stencil.stencilWrite ? stencil.back.writeMask : 0u),
        .reference = (uint32_t) stencil.back.ref,
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = hasFragmentShader ? SHADER_MODULE_COUNT : 1,
        .pStages = shaderStages,
        .pVertexInputState = hasVertexInputState ? &vertexInputState : nullptr,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &vkRaster,
        .pMultisampleState = &vkMs,
        .pDepthStencilState = &vkDs,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,
        .layout = key.layout,
        .renderPass = key.renderPass,
        .subpass = key.subpassIndex,
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

    VkPipelineRenderingCreateInfoKHR renderingInfo {};
    VkFormat pipelineRenderingColorFormats[] = {VK_FORMAT_UNDEFINED};
    if (dynamicOptions.useDynamicRenderPasses) {
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        renderingInfo.pNext = pipelineCreateInfo.pNext;
        // Fill values in with empty values where we do not already have values.
        renderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
        // If multiview, create a bitmask with bits representing each enabled view set to 1;
        // otherwise, set the view mask to 0.
        renderingInfo.viewMask = dynamicOptions.stereoscopicType == StereoscopicType::MULTIVIEW ?
            (1 << dynamicOptions.stereoscopicViewCount) - 1 : 0;

        if (hasFragmentShader) {
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachmentFormats = pipelineRenderingColorFormats;
        }

        pipelineCreateInfo.pNext = &renderingInfo;
    }

#if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
    FVK_LOGD << "vkCreateGraphicsPipelines with shaders = (" << shaderStages[0].module << ", "
             << shaderStages[1].module << ")";

    VkPipelineCreationFeedback stageFeedbacks[SHADER_MODULE_COUNT] = {};
    VkPipelineCreationFeedback pipelineFeedback = {};
    VkPipelineCreationFeedbackCreateInfo feedbackInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT,
        .pNext = nullptr,
        .pPipelineCreationFeedback = &pipelineFeedback,
        .pipelineStageCreationFeedbackCount = hasFragmentShader ? SHADER_MODULE_COUNT : 1,
        .pPipelineStageCreationFeedbacks = stageFeedbacks,
    };

    if (mContext.pipelineCreationFeedbackSupported()) {
        feedbackInfo.pNext = pipelineCreateInfo.pNext;
        pipelineCreateInfo.pNext = &feedbackInfo;
    }
#endif
    VkPipeline pipeline;
    VkResult error = vkCreateGraphicsPipelines(mDevice, mPipelineCache, 1, &pipelineCreateInfo,
            VKALLOC, &pipeline);

#if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
    FVK_LOGD << "vkCreateGraphicsPipelines with shaders = (" << shaderStages[0].module << ", "
             << shaderStages[1].module << ")";

    if (mContext.pipelineCreationFeedbackSupported()) {
        printPipelineFeedbackInfo(feedbackInfo);
    }
#endif

    assert_invariant(error == VK_SUCCESS);
    if (error != VK_SUCCESS) {
        FVK_LOGE << "vkCreateGraphicsPipelines error " << error;
        return VK_NULL_HANDLE;
    }
    return pipeline;
}

void VulkanPipelineCache::bindProgram(fvkmemory::resource_ptr<VulkanProgram> program) noexcept {
    mStaticPipelineKey.shaders[0] = program->getVertexShader();
    mStaticPipelineKey.shaders[1] = program->getFragmentShader();
    mBoundProgram = program->programString;

    // If this is a debug build, validate the current shader.
#if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
    if (mStaticPipelineKey.shaders[0] == VK_NULL_HANDLE ||
            mStaticPipelineKey.shaders[1] == VK_NULL_HANDLE) {
        FVK_LOGE << "Binding missing shader: " << program->name.c_str();
    }
#endif
}

void VulkanPipelineCache::bindRasterState(RasterState const& rasterState) noexcept {
    RasterState& dsTarget = mHasDynamicState ?
            mDynamicPipelineKey.rasterState : mStaticPipelineKey.rasterState;
    dsTarget.cullMode = rasterState.cullMode;
    dsTarget.frontFace = rasterState.frontFace;
    dsTarget.depthWriteEnable = rasterState.depthWriteEnable;
    dsTarget.depthCompareOp = rasterState.depthCompareOp;
    dsTarget.depthBiasConstantFactor = rasterState.depthBiasConstantFactor;
    dsTarget.depthBiasSlopeFactor = rasterState.depthBiasSlopeFactor;

    RasterState& ds2Target = mHasDynamicState2 ?
            mDynamicPipelineKey.rasterState : mStaticPipelineKey.rasterState;
    ds2Target.depthBiasEnable = rasterState.depthBiasEnable;

    mStaticPipelineKey.rasterState.blendEnable = rasterState.blendEnable;
    mStaticPipelineKey.rasterState.alphaToCoverageEnable = rasterState.alphaToCoverageEnable;
    mStaticPipelineKey.rasterState.srcColorBlendFactor = rasterState.srcColorBlendFactor;
    mStaticPipelineKey.rasterState.dstColorBlendFactor = rasterState.dstColorBlendFactor;
    mStaticPipelineKey.rasterState.srcAlphaBlendFactor = rasterState.srcAlphaBlendFactor;
    mStaticPipelineKey.rasterState.dstAlphaBlendFactor = rasterState.dstAlphaBlendFactor;
    mStaticPipelineKey.rasterState.colorWriteMask = rasterState.colorWriteMask;
    mStaticPipelineKey.rasterState.rasterizationSamples = rasterState.rasterizationSamples;
    mStaticPipelineKey.rasterState.depthClamp = rasterState.depthClamp;
    mStaticPipelineKey.rasterState.colorTargetCount = rasterState.colorTargetCount;
    mStaticPipelineKey.rasterState.colorBlendOp = rasterState.colorBlendOp;
    mStaticPipelineKey.rasterState.alphaBlendOp = rasterState.alphaBlendOp;
}

void VulkanPipelineCache::bindStencilState(StencilState const& stencilState) noexcept {
    if (mHasDynamicState) {
        mDynamicPipelineKey.stencilState = stencilState;
    } else {
        mStaticPipelineKey.stencilState = stencilState;
    }
}

void VulkanPipelineCache::bindRenderPass(
        fvkmemory::resource_ptr<VulkanRenderPass> renderPass,
        int subpassIndex) noexcept {
    mStaticPipelineKey.renderPass = renderPass->getVkRenderPass();
    mStaticPipelineKey.subpassIndex = subpassIndex;
}

void VulkanPipelineCache::bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept {
    if (mHasDynamicState) {
        mDynamicPipelineKey.topology = topology;
    } else {
        mStaticPipelineKey.topology = topology;
    }
}

void VulkanPipelineCache::bindVertexArray(VkVertexInputAttributeDescription const* attribDesc,
        VkVertexInputBindingDescription const* bufferDesc, uint8_t count) {
    PipelineKey& targetKey = mHasVertexInputDynamicState ?
            mDynamicPipelineKey : mStaticPipelineKey;
    for (size_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
        if (i < count) {
            targetKey.vertexAttributes[i] = attribDesc[i];
            targetKey.vertexBuffers[i] = bufferDesc[i];
        } else {
            targetKey.vertexAttributes[i] = VertexInputAttributeDescription{};
            targetKey.vertexBuffers[i] = VertexInputBindingDescription{};
        }
    }

    if (mHasVertexInputDynamicState) {
        // TODO: Hash only once in VulkanVertexBufferInfo to avoid recomputing every bind call.
        mDynamicVertexHash = utils::hash::murmur3(
            reinterpret_cast<const uint32_t*>(targetKey.vertexAttributes),
            (sizeof(targetKey.vertexAttributes) + sizeof(targetKey.vertexBuffers)) / 4,
            0);
    }
}

void VulkanPipelineCache::addCachePrewarmCallback(CallbackHandler* handler,
                                                  const CallbackHandler::Callback callback,
                                                  void* user) {
    if (callback) {
        mCallbackManager.setCallback(handler, callback, user);
    }
}

void VulkanPipelineCache::resetBoundDynamicState() noexcept {
    mForceDirtyDynamicState = true;
}

void VulkanPipelineCache::resetBoundPipeline() {
    mBoundPipeline = {};
    mBoundDynamicState = {};
    mForceDirtyDynamicState = true;
    mBoundVertexHash = 0;
}

void VulkanPipelineCache::terminate() noexcept {
    for (auto& iter : mPipelines) {
        vkDestroyPipeline(mDevice, iter.second.handle, VKALLOC);
    }
    mPipelines.clear();
    resetBoundPipeline();

    mCallbackManager.terminate();
    mCompilerThreadPool.terminate();

    vkDestroyPipelineCache(mDevice, mPipelineCache, VKALLOC);
}

void VulkanPipelineCache::gc() noexcept {
    // The timestamp associated with a given cache entry represents "time" as a count of flush
    // events since the cache was constructed. If any cache entry was most recently used over
    // FVK_MAX_PIPELINE_AGE flush events in the past, then we can be sure that it is no longer
    // being used by the GPU, and is therefore safe to destroy or reclaim.
    ++mCurrentTime;

    // The Vulkan spec says: "When a command buffer begins recording, all state in that command
    // buffer is undefined." Therefore, we need to clear all bindings at this time.
    resetBoundPipeline();

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

uint16_t VulkanPipelineCache::computeDirtyDynamicState() noexcept {
    if (UTILS_UNLIKELY(mForceDirtyDynamicState)) {
       mForceDirtyDynamicState = false;
        uint16_t mask = DIRTY_NONE;
        if (mHasVertexInputDynamicState) {
            mask |= DIRTY_VERTEX_INPUT;
        }
        if (mHasDynamicState) {
            mask |= DIRTY_DEPTH_BIAS | DIRTY_CULL_MODE | DIRTY_FRONT_FACE | DIRTY_PRIMITIVE_TOPOLOGY |
                    DIRTY_DEPTH_TEST_ENABLE | DIRTY_DEPTH_WRITE_ENABLE | DIRTY_DEPTH_COMPARE_OP |
                    DIRTY_STENCIL_TEST_ENABLE | DIRTY_STENCIL_STATE;
        }
        if (mHasDynamicState2) {
            mask |= DIRTY_DEPTH_BIAS_ENABLE;
        }
        return mask;
    }

    uint16_t dirty = DIRTY_NONE;

    if (mHasVertexInputDynamicState) {
        if (mDynamicVertexHash != mBoundVertexHash) {
            dirty |= DIRTY_VERTEX_INPUT;
        }
    }

    if (mHasDynamicState) {
        RasterState const& dyn = mDynamicPipelineKey.rasterState;
        RasterState const& bound = mBoundDynamicState.rasterState;
        if (dyn.depthBiasConstantFactor != bound.depthBiasConstantFactor ||
                dyn.depthBiasSlopeFactor != bound.depthBiasSlopeFactor) {
            dirty |= DIRTY_DEPTH_BIAS;
        }

        if (dyn.cullMode != bound.cullMode) dirty |= DIRTY_CULL_MODE;
        if (dyn.frontFace != bound.frontFace) dirty |= DIRTY_FRONT_FACE;
        if (mDynamicPipelineKey.topology != mBoundDynamicState.topology) dirty |= DIRTY_PRIMITIVE_TOPOLOGY;

        bool const enableDepth = isDepthTestEnabled(dyn);
        bool const prevEnableDepth = isDepthTestEnabled(bound);
        if (enableDepth != prevEnableDepth) dirty |= DIRTY_DEPTH_TEST_ENABLE;
        if (dyn.depthWriteEnable != bound.depthWriteEnable) dirty |= DIRTY_DEPTH_WRITE_ENABLE;
        if (dyn.depthCompareOp != bound.depthCompareOp) dirty |= DIRTY_DEPTH_COMPARE_OP;

        bool const enableStencil = isStencilTestEnabled(mDynamicPipelineKey.stencilState);
        bool const prevEnableStencil = isStencilTestEnabled(mBoundDynamicState.stencilState);
        if (enableStencil != prevEnableStencil) dirty |= DIRTY_STENCIL_TEST_ENABLE;
        if (memcmp(&mDynamicPipelineKey.stencilState, &mBoundDynamicState.stencilState,
                   sizeof(mDynamicPipelineKey.stencilState)) != 0) {
            dirty |= DIRTY_STENCIL_STATE;
        }
    }

    if (mHasDynamicState2) {
        if (mDynamicPipelineKey.rasterState.depthBiasEnable != mBoundDynamicState.rasterState.depthBiasEnable) {
            dirty |= DIRTY_DEPTH_BIAS_ENABLE;
        }
    }

    return dirty;
}

void VulkanPipelineCache::bindDynamicState(VkCommandBuffer cmdbuffer, uint16_t dirtyMask) {
    if (dirtyMask & DIRTY_VERTEX_INPUT) {
        uint32_t numVertexAttribs = 0;
        uint32_t numVertexBuffers = 0;
        VkVertexInputAttributeDescription2EXT vertexAttributes[VERTEX_ATTRIBUTE_COUNT];
        VkVertexInputBindingDescription2EXT vertexBuffers[VERTEX_ATTRIBUTE_COUNT];
        for (uint32_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
            if (mDynamicPipelineKey.vertexAttributes[i].format > 0) {
                vertexAttributes[numVertexAttribs++] = VkVertexInputAttributeDescription2EXT{
                    .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                    .location = mDynamicPipelineKey.vertexAttributes[i].location,
                    .binding = mDynamicPipelineKey.vertexAttributes[i].binding,
                    .format = (VkFormat) mDynamicPipelineKey.vertexAttributes[i].format,
                    .offset = mDynamicPipelineKey.vertexAttributes[i].offset,
                };
            }
            if (mDynamicPipelineKey.vertexBuffers[i].stride > 0) {
                vertexBuffers[numVertexBuffers++] = VkVertexInputBindingDescription2EXT{
                    .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                    .binding = mDynamicPipelineKey.vertexBuffers[i].binding,
                    .stride = mDynamicPipelineKey.vertexBuffers[i].stride,
                    .inputRate = (VkVertexInputRate) mDynamicPipelineKey.vertexBuffers[i].inputRate,
                    .divisor = 1,
                };
            }
        }
        mBoundVertexHash = mDynamicVertexHash;
        vkCmdSetVertexInputEXT(cmdbuffer, numVertexBuffers, vertexBuffers, numVertexAttribs, vertexAttributes);
    }

    if (dirtyMask & DIRTY_DEPTH_BIAS) {
        mBoundDynamicState.rasterState.depthBiasConstantFactor =
                mDynamicPipelineKey.rasterState.depthBiasConstantFactor;
        mBoundDynamicState.rasterState.depthBiasSlopeFactor =
                mDynamicPipelineKey.rasterState.depthBiasSlopeFactor;
        vkCmdSetDepthBias(cmdbuffer, mDynamicPipelineKey.rasterState.depthBiasConstantFactor,
                /*depthBiasClamp=*/0.0, mDynamicPipelineKey.rasterState.depthBiasSlopeFactor);
    }

    if (dirtyMask & DIRTY_CULL_MODE) {
        mBoundDynamicState.rasterState.cullMode = mDynamicPipelineKey.rasterState.cullMode;
        vkCmdSetCullModeEXT(cmdbuffer, mDynamicPipelineKey.rasterState.cullMode);
    }

    if (dirtyMask & DIRTY_FRONT_FACE) {
        mBoundDynamicState.rasterState.frontFace = mDynamicPipelineKey.rasterState.frontFace;
        vkCmdSetFrontFaceEXT(cmdbuffer, mDynamicPipelineKey.rasterState.frontFace);
    }

    if (dirtyMask & DIRTY_PRIMITIVE_TOPOLOGY) {
        mBoundDynamicState.topology = mDynamicPipelineKey.topology;
        vkCmdSetPrimitiveTopologyEXT(cmdbuffer, (VkPrimitiveTopology) mDynamicPipelineKey.topology);
    }

    if (dirtyMask & DIRTY_DEPTH_TEST_ENABLE) {
        bool const enableDepthTest = isDepthTestEnabled(mDynamicPipelineKey.rasterState);
        vkCmdSetDepthTestEnableEXT(cmdbuffer, enableDepthTest ? VK_TRUE : VK_FALSE);
    }

    if (dirtyMask & DIRTY_DEPTH_WRITE_ENABLE) {
        mBoundDynamicState.rasterState.depthWriteEnable = mDynamicPipelineKey.rasterState.depthWriteEnable;
        vkCmdSetDepthWriteEnableEXT(cmdbuffer, mDynamicPipelineKey.rasterState.depthWriteEnable);
    }

    if (dirtyMask & DIRTY_DEPTH_COMPARE_OP) {
        mBoundDynamicState.rasterState.depthCompareOp = mDynamicPipelineKey.rasterState.depthCompareOp;
        vkCmdSetDepthCompareOpEXT(cmdbuffer, fvkutils::getCompareOp(mDynamicPipelineKey.rasterState.depthCompareOp));
    }

    if (dirtyMask & DIRTY_STENCIL_TEST_ENABLE) {
        bool const enableStencilTest = isStencilTestEnabled(mDynamicPipelineKey.stencilState);
        vkCmdSetStencilTestEnableEXT(cmdbuffer, enableStencilTest ? VK_TRUE : VK_FALSE);
    }

    if (dirtyMask & DIRTY_STENCIL_STATE) {
        mBoundDynamicState.stencilState = mDynamicPipelineKey.stencilState;
        StencilState const& stencil = mDynamicPipelineKey.stencilState;
        vkCmdSetStencilOpEXT(cmdbuffer, VK_STENCIL_FACE_FRONT_BIT,
                fvkutils::getStencilOp(stencil.front.stencilOpStencilFail),
                fvkutils::getStencilOp(stencil.front.stencilOpDepthStencilPass),
                fvkutils::getStencilOp(stencil.front.stencilOpDepthFail),
                fvkutils::getCompareOp(stencil.front.stencilFunc));
        vkCmdSetStencilCompareMask(cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, stencil.front.readMask);
        vkCmdSetStencilWriteMask(cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, stencil.front.writeMask);
        vkCmdSetStencilReference(cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, stencil.front.ref);

        vkCmdSetStencilOpEXT(cmdbuffer, VK_STENCIL_FACE_BACK_BIT,
                fvkutils::getStencilOp(stencil.back.stencilOpStencilFail),
                fvkutils::getStencilOp(stencil.back.stencilOpDepthStencilPass),
                fvkutils::getStencilOp(stencil.back.stencilOpDepthFail),
                fvkutils::getCompareOp(stencil.back.stencilFunc));
        vkCmdSetStencilCompareMask(cmdbuffer, VK_STENCIL_FACE_BACK_BIT, stencil.back.readMask);
        vkCmdSetStencilWriteMask(cmdbuffer, VK_STENCIL_FACE_BACK_BIT, stencil.back.writeMask);
        vkCmdSetStencilReference(cmdbuffer, VK_STENCIL_FACE_BACK_BIT, stencil.back.ref);
    }

    if (dirtyMask & DIRTY_DEPTH_BIAS_ENABLE) {
        mBoundDynamicState.rasterState.depthBiasEnable = mDynamicPipelineKey.rasterState.depthBiasEnable;
        vkCmdSetDepthBiasEnableEXT(cmdbuffer, mDynamicPipelineKey.rasterState.depthBiasEnable);
    }
}

} // namespace filament::backend

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
