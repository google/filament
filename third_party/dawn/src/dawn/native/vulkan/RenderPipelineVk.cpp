// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/RenderPipelineVk.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PipelineCacheVk.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/RenderPassCache.h"
#include "dawn/native/vulkan/ShaderModuleVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native::vulkan {

namespace {

VkVertexInputRate VulkanInputRate(wgpu::VertexStepMode stepMode) {
    switch (stepMode) {
        case wgpu::VertexStepMode::Vertex:
            return VK_VERTEX_INPUT_RATE_VERTEX;
        case wgpu::VertexStepMode::Instance:
            return VK_VERTEX_INPUT_RATE_INSTANCE;
        case wgpu::VertexStepMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkFormat VulkanVertexFormat(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8:
            return VK_FORMAT_R8_UINT;
        case wgpu::VertexFormat::Uint8x2:
            return VK_FORMAT_R8G8_UINT;
        case wgpu::VertexFormat::Uint8x4:
            return VK_FORMAT_R8G8B8A8_UINT;
        case wgpu::VertexFormat::Sint8:
            return VK_FORMAT_R8_SINT;
        case wgpu::VertexFormat::Sint8x2:
            return VK_FORMAT_R8G8_SINT;
        case wgpu::VertexFormat::Sint8x4:
            return VK_FORMAT_R8G8B8A8_SINT;
        case wgpu::VertexFormat::Unorm8:
            return VK_FORMAT_R8_UNORM;
        case wgpu::VertexFormat::Unorm8x2:
            return VK_FORMAT_R8G8_UNORM;
        case wgpu::VertexFormat::Unorm8x4:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case wgpu::VertexFormat::Snorm8:
            return VK_FORMAT_R8_SNORM;
        case wgpu::VertexFormat::Snorm8x2:
            return VK_FORMAT_R8G8_SNORM;
        case wgpu::VertexFormat::Snorm8x4:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case wgpu::VertexFormat::Uint16:
            return VK_FORMAT_R16_UINT;
        case wgpu::VertexFormat::Uint16x2:
            return VK_FORMAT_R16G16_UINT;
        case wgpu::VertexFormat::Uint16x4:
            return VK_FORMAT_R16G16B16A16_UINT;
        case wgpu::VertexFormat::Sint16:
            return VK_FORMAT_R16_SINT;
        case wgpu::VertexFormat::Sint16x2:
            return VK_FORMAT_R16G16_SINT;
        case wgpu::VertexFormat::Sint16x4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case wgpu::VertexFormat::Unorm16:
            return VK_FORMAT_R16_UNORM;
        case wgpu::VertexFormat::Unorm16x2:
            return VK_FORMAT_R16G16_UNORM;
        case wgpu::VertexFormat::Unorm16x4:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case wgpu::VertexFormat::Snorm16:
            return VK_FORMAT_R16_SNORM;
        case wgpu::VertexFormat::Snorm16x2:
            return VK_FORMAT_R16G16_SNORM;
        case wgpu::VertexFormat::Snorm16x4:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case wgpu::VertexFormat::Float16:
            return VK_FORMAT_R16_SFLOAT;
        case wgpu::VertexFormat::Float16x2:
            return VK_FORMAT_R16G16_SFLOAT;
        case wgpu::VertexFormat::Float16x4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case wgpu::VertexFormat::Float32:
            return VK_FORMAT_R32_SFLOAT;
        case wgpu::VertexFormat::Float32x2:
            return VK_FORMAT_R32G32_SFLOAT;
        case wgpu::VertexFormat::Float32x3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case wgpu::VertexFormat::Float32x4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case wgpu::VertexFormat::Uint32:
            return VK_FORMAT_R32_UINT;
        case wgpu::VertexFormat::Uint32x2:
            return VK_FORMAT_R32G32_UINT;
        case wgpu::VertexFormat::Uint32x3:
            return VK_FORMAT_R32G32B32_UINT;
        case wgpu::VertexFormat::Uint32x4:
            return VK_FORMAT_R32G32B32A32_UINT;
        case wgpu::VertexFormat::Sint32:
            return VK_FORMAT_R32_SINT;
        case wgpu::VertexFormat::Sint32x2:
            return VK_FORMAT_R32G32_SINT;
        case wgpu::VertexFormat::Sint32x3:
            return VK_FORMAT_R32G32B32_SINT;
        case wgpu::VertexFormat::Sint32x4:
            return VK_FORMAT_R32G32B32A32_SINT;
        case wgpu::VertexFormat::Unorm10_10_10_2:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case wgpu::VertexFormat::Unorm8x4BGRA:
            return VK_FORMAT_B8G8R8A8_UNORM;
        default:
            DAWN_UNREACHABLE();
    }
}

VkPrimitiveTopology VulkanPrimitiveTopology(wgpu::PrimitiveTopology topology) {
    switch (topology) {
        case wgpu::PrimitiveTopology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case wgpu::PrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case wgpu::PrimitiveTopology::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case wgpu::PrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case wgpu::PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

bool ShouldEnablePrimitiveRestart(wgpu::PrimitiveTopology topology) {
    // Primitive restart is always enabled in WebGPU but Vulkan validation rules ask that
    // primitive restart be only enabled on primitive topologies that support restarting.
    switch (topology) {
        case wgpu::PrimitiveTopology::PointList:
        case wgpu::PrimitiveTopology::LineList:
        case wgpu::PrimitiveTopology::TriangleList:
            return false;
        case wgpu::PrimitiveTopology::LineStrip:
        case wgpu::PrimitiveTopology::TriangleStrip:
            return true;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkFrontFace VulkanFrontFace(wgpu::FrontFace face) {
    switch (face) {
        case wgpu::FrontFace::CCW:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case wgpu::FrontFace::CW:
            return VK_FRONT_FACE_CLOCKWISE;
        case wgpu::FrontFace::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkCullModeFlagBits VulkanCullMode(wgpu::CullMode mode) {
    switch (mode) {
        case wgpu::CullMode::None:
            return VK_CULL_MODE_NONE;
        case wgpu::CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case wgpu::CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case wgpu::CullMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkBlendFactor VulkanBlendFactor(wgpu::BlendFactor factor) {
    switch (factor) {
        case wgpu::BlendFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case wgpu::BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case wgpu::BlendFactor::Src:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case wgpu::BlendFactor::OneMinusSrc:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case wgpu::BlendFactor::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return VK_BLEND_FACTOR_DST_COLOR;
        case wgpu::BlendFactor::OneMinusDst:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case wgpu::BlendFactor::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case wgpu::BlendFactor::OneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case wgpu::BlendFactor::SrcAlphaSaturated:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case wgpu::BlendFactor::Constant:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case wgpu::BlendFactor::OneMinusConstant:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case wgpu::BlendFactor::Src1:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case wgpu::BlendFactor::OneMinusSrc1:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case wgpu::BlendFactor::Src1Alpha:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc1Alpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        case wgpu::BlendFactor::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkBlendOp VulkanBlendOperation(wgpu::BlendOperation operation) {
    switch (operation) {
        case wgpu::BlendOperation::Add:
            return VK_BLEND_OP_ADD;
        case wgpu::BlendOperation::Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case wgpu::BlendOperation::ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case wgpu::BlendOperation::Min:
            return VK_BLEND_OP_MIN;
        case wgpu::BlendOperation::Max:
            return VK_BLEND_OP_MAX;
        case wgpu::BlendOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

VkColorComponentFlags VulkanColorWriteMask(wgpu::ColorWriteMask mask,
                                           bool isDeclaredInFragmentShader) {
    // Vulkan and Dawn color write masks match, static assert it and return the mask
    static_assert(static_cast<VkColorComponentFlagBits>(wgpu::ColorWriteMask::Red) ==
                  VK_COLOR_COMPONENT_R_BIT);
    static_assert(static_cast<VkColorComponentFlagBits>(wgpu::ColorWriteMask::Green) ==
                  VK_COLOR_COMPONENT_G_BIT);
    static_assert(static_cast<VkColorComponentFlagBits>(wgpu::ColorWriteMask::Blue) ==
                  VK_COLOR_COMPONENT_B_BIT);
    static_assert(static_cast<VkColorComponentFlagBits>(wgpu::ColorWriteMask::Alpha) ==
                  VK_COLOR_COMPONENT_A_BIT);

    // According to Vulkan SPEC (Chapter 14.3): "The input values to blending or color
    // attachment writes are undefined for components which do not correspond to a fragment
    // shader outputs", we set the color write mask to 0 to prevent such undefined values
    // being written into the color attachments.
    return isDeclaredInFragmentShader ? static_cast<VkColorComponentFlags>(mask)
                                      : static_cast<VkColorComponentFlags>(0);
}

VkPipelineColorBlendAttachmentState ComputeColorDesc(const ColorTargetState* state,
                                                     bool isDeclaredInFragmentShader) {
    VkPipelineColorBlendAttachmentState attachment;
    attachment.blendEnable = state->blend != nullptr ? VK_TRUE : VK_FALSE;
    if (attachment.blendEnable) {
        attachment.srcColorBlendFactor = VulkanBlendFactor(state->blend->color.srcFactor);
        attachment.dstColorBlendFactor = VulkanBlendFactor(state->blend->color.dstFactor);
        attachment.colorBlendOp = VulkanBlendOperation(state->blend->color.operation);
        attachment.srcAlphaBlendFactor = VulkanBlendFactor(state->blend->alpha.srcFactor);
        attachment.dstAlphaBlendFactor = VulkanBlendFactor(state->blend->alpha.dstFactor);
        attachment.alphaBlendOp = VulkanBlendOperation(state->blend->alpha.operation);
    } else {
        // Swiftshader's Vulkan implementation appears to expect these values to be valid
        // even when blending is not enabled.
        attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachment.colorBlendOp = VK_BLEND_OP_ADD;
        attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    attachment.colorWriteMask = VulkanColorWriteMask(state->writeMask, isDeclaredInFragmentShader);
    return attachment;
}

VkStencilOp VulkanStencilOp(wgpu::StencilOperation op) {
    switch (op) {
        case wgpu::StencilOperation::Keep:
            return VK_STENCIL_OP_KEEP;
        case wgpu::StencilOperation::Zero:
            return VK_STENCIL_OP_ZERO;
        case wgpu::StencilOperation::Replace:
            return VK_STENCIL_OP_REPLACE;
        case wgpu::StencilOperation::IncrementClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case wgpu::StencilOperation::DecrementClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case wgpu::StencilOperation::Invert:
            return VK_STENCIL_OP_INVERT;
        case wgpu::StencilOperation::IncrementWrap:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case wgpu::StencilOperation::DecrementWrap:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case wgpu::StencilOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

}  // anonymous namespace

// static
Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

MaybeError RenderPipeline::InitializeImpl() {
    Device* device = ToBackend(GetDevice());
    PipelineLayout* layout = ToBackend(GetLayout());

    // The cache key is only used for storing VkPipelineCache objects in BlobStore. That's not
    // done with the monolithic pipeline cache so it's unnecessary work and memory usage.
    bool buildCacheKey =
        !device->GetTogglesState().IsEnabled(Toggle::VulkanMonolithicPipelineCache);
    if (buildCacheKey) {
        // Vulkan devices need cache UUID field to be serialized into pipeline cache keys.
        StreamIn(&mCacheKey, device->GetDeviceInfo().properties.pipelineCacheUUID);
    }

    // Gather list of internal immediate constants used by this pipeline
    if (UsesFragDepth() && !HasUnclippedDepth()) {
        mImmediateMask |= GetImmediateConstantBlockBits(
            offsetof(RenderImmediateConstants, clampFragDepth), sizeof(ClampFragDepthArgs));
    }

    // There are at most 2 shader stages in render pipeline, i.e. vertex and fragment
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    uint32_t stageCount = 0;

    auto AddShaderStage = [&](SingleShaderStage stage, VkShaderStageFlagBits vkStage,
                              bool emitPointSize) -> MaybeError {
        const ProgrammableStage& programmableStage = GetStage(stage);
        ShaderModule::ModuleAndSpirv moduleAndSpirv;
        DAWN_TRY_ASSIGN(moduleAndSpirv, ToBackend(programmableStage.module)
                                            ->GetHandleAndSpirv(stage, programmableStage, layout,
                                                                emitPointSize, GetImmediateMask()));
        mHasInputAttachment = mHasInputAttachment || moduleAndSpirv.hasInputAttachment;
        if (buildCacheKey) {
            // Record cache key for each shader since it will become inaccessible later on.
            StreamIn(&mCacheKey, moduleAndSpirv.spirv);
        }

        VkPipelineShaderStageCreateInfo* shaderStage = &shaderStages[stageCount];
        shaderStage->module = moduleAndSpirv.module;
        shaderStage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage->pNext = nullptr;
        shaderStage->flags = 0;
        shaderStage->pSpecializationInfo = nullptr;
        shaderStage->stage = vkStage;
        // string_view returned by GetIsolatedEntryPointName() points to a null-terminated string.
        shaderStage->pName = device->GetIsolatedEntryPointName().data();

        stageCount++;
        return {};
    };

    // Add the vertex stage that's always present.
    DAWN_TRY(AddShaderStage(SingleShaderStage::Vertex, VK_SHADER_STAGE_VERTEX_BIT,
                            GetPrimitiveTopology() == wgpu::PrimitiveTopology::PointList));

    // Add the fragment stage if present.
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        DAWN_TRY(AddShaderStage(SingleShaderStage::Fragment, VK_SHADER_STAGE_FRAGMENT_BIT,
                                /*emitPointSize*/ false));
    }

    PipelineVertexInputStateCreateInfoTemporaryAllocations tempAllocations;
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo =
        ComputeVertexInputDesc(&tempAllocations);

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VulkanPrimitiveTopology(GetPrimitiveTopology());
    inputAssembly.primitiveRestartEnable =
        ShouldEnablePrimitiveRestart(GetPrimitiveTopology()) ? VK_TRUE : VK_FALSE;

    // A placeholder viewport/scissor info. The validation layers force use to provide at least
    // one scissor and one viewport here, even if we choose to make them dynamic.
    VkViewport viewportDesc;
    viewportDesc.x = 0.0f;
    viewportDesc.y = 0.0f;
    viewportDesc.width = 1.0f;
    viewportDesc.height = 1.0f;
    viewportDesc.minDepth = 0.0f;
    viewportDesc.maxDepth = 1.0f;
    VkRect2D scissorRect;
    scissorRect.offset.x = 0;
    scissorRect.offset.y = 0;
    scissorRect.extent.width = 1;
    scissorRect.extent.height = 1;
    VkPipelineViewportStateCreateInfo viewport;
    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.pNext = nullptr;
    viewport.flags = 0;
    viewport.viewportCount = 1;
    viewport.pViewports = &viewportDesc;
    viewport.scissorCount = 1;
    viewport.pScissors = &scissorRect;

    VkPipelineRasterizationStateCreateInfo rasterization;
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.pNext = nullptr;
    rasterization.flags = 0;
    rasterization.depthClampEnable = HasUnclippedDepth() ? VK_TRUE : VK_FALSE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VulkanCullMode(GetCullMode());
    rasterization.frontFace = VulkanFrontFace(GetFrontFace());
    rasterization.depthBiasEnable = IsDepthBiasEnabled() ? VK_TRUE : VK_FALSE;
    rasterization.depthBiasConstantFactor = GetDepthBias();
    rasterization.depthBiasClamp = GetDepthBiasClamp();
    rasterization.depthBiasSlopeFactor = GetDepthBiasSlopeScale();
    rasterization.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample;
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.pNext = nullptr;
    multisample.flags = 0;
    multisample.rasterizationSamples = VulkanSampleCount(GetSampleCount());
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 0.0f;
    // VkPipelineMultisampleStateCreateInfo.pSampleMask is an array of length
    // ceil(rasterizationSamples / 32) and since we're passing a single uint32_t
    // we have to assert that this length is indeed 1.
    DAWN_ASSERT(multisample.rasterizationSamples <= 32);
    VkSampleMask sampleMask = GetSampleMask();
    multisample.pSampleMask = &sampleMask;
    multisample.alphaToCoverageEnable = IsAlphaToCoverageEnabled() ? VK_TRUE : VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = ComputeDepthStencilDesc();

    VkPipelineColorBlendStateCreateInfo colorBlend;
    // colorBlend may hold pointers to elements in colorBlendAttachments, so it must have a
    // definition scope as same as colorBlend
    PerColorAttachment<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        // Initialize the "blend state info" that will be chained in the "create info" from the
        // data pre-computed in the ColorState
        for (auto& blend : colorBlendAttachments) {
            blend.blendEnable = VK_FALSE;
            blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            blend.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            blend.colorBlendOp = VK_BLEND_OP_ADD;
            blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            blend.alphaBlendOp = VK_BLEND_OP_ADD;
            blend.colorWriteMask = 0;
        }

        const auto& fragmentOutputMask =
            GetStage(SingleShaderStage::Fragment).metadata->fragmentOutputMask;
        auto highestColorAttachmentIndexPlusOne =
            GetHighestBitIndexPlusOne(GetColorAttachmentsMask());
        for (auto i : GetColorAttachmentsMask()) {
            const ColorTargetState* target = GetColorTargetState(i);
            colorBlendAttachments[i] = ComputeColorDesc(target, fragmentOutputMask[i]);
        }

        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.pNext = nullptr;
        colorBlend.flags = 0;
        // LogicOp isn't supported so we disable it.
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.logicOp = VK_LOGIC_OP_CLEAR;
        colorBlend.attachmentCount = static_cast<uint8_t>(highestColorAttachmentIndexPlusOne);
        colorBlend.pAttachments = colorBlendAttachments.data();
        // The blend constant is always dynamic so we fill in a placeholder value
        colorBlend.blendConstants[0] = 0.0f;
        colorBlend.blendConstants[1] = 0.0f;
        colorBlend.blendConstants[2] = 0.0f;
        colorBlend.blendConstants[3] = 0.0f;
    }

    // Tag all state as dynamic but stencil masks and depth bias.
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,     VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,   VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    };
    VkPipelineDynamicStateCreateInfo dynamic;
    dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic.pNext = nullptr;
    dynamic.flags = 0;
    dynamic.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
    dynamic.pDynamicStates = dynamicStates;

    // Get a VkRenderPass that matches the attachment formats for this pipeline.
    // VkRenderPass compatibility rules let us provide placeholder data for a bunch of arguments.
    // Load and store ops are all equivalent, though we still specify ExpandResolveTexture as that
    // controls the use of input attachments. Single subpass VkRenderPasses are compatible
    // irrespective of resolve attachments being used, but for ExpandResolveTexture that uses two
    // subpasses we need to specify which attachments will be resolved.
    RenderPassCache::RenderPassInfo renderPassInfo;
    {
        RenderPassCacheQuery query;
        ColorAttachmentMask resolveMask =
            GetAttachmentState()->GetExpandResolveInfo().resolveTargetsMask;
        ColorAttachmentMask expandResolveMask =
            GetAttachmentState()->GetExpandResolveInfo().attachmentsToExpandResolve;

        for (auto i : GetColorAttachmentsMask()) {
            wgpu::LoadOp colorLoadOp = wgpu::LoadOp::Load;
            bool hasResolveTarget = resolveMask.test(i);

            if (expandResolveMask.test(i)) {
                // ExpandResolveTexture will use 2 subpasses in a render pass so we have to create
                // an appropriate query.
                colorLoadOp = wgpu::LoadOp::ExpandResolveTexture;
            }
            query.SetColor(i, GetColorAttachmentFormat(i), colorLoadOp, wgpu::StoreOp::Store,
                           hasResolveTarget);
        }

        if (HasDepthStencilAttachment()) {
            query.SetDepthStencil(GetDepthStencilFormat(), wgpu::LoadOp::Load, wgpu::StoreOp::Store,
                                  false, wgpu::LoadOp::Load, wgpu::StoreOp::Store, false);
        }

        query.SetSampleCount(GetSampleCount());

        if (buildCacheKey) {
            StreamIn(&mCacheKey, query);
        }
        DAWN_TRY_ASSIGN(renderPassInfo, device->GetRenderPassCache()->GetRenderPass(query));
    }
    DAWN_TRY(PipelineVk::InitializeBase(layout, mImmediateMask));

    // The create info chains in a bunch of things created on the stack here or inside state
    // objects.
    VkGraphicsPipelineCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stageCount = stageCount;
    createInfo.pStages = shaderStages.data();
    createInfo.pVertexInputState = &vertexInputCreateInfo;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pTessellationState = nullptr;
    createInfo.pViewportState = &viewport;
    createInfo.pRasterizationState = &rasterization;
    createInfo.pMultisampleState = &multisample;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.pColorBlendState =
        (GetStageMask() & wgpu::ShaderStage::Fragment) ? &colorBlend : nullptr;
    createInfo.pDynamicState = &dynamic;
    createInfo.layout = GetVkLayout();
    createInfo.renderPass = renderPassInfo.renderPass;
    createInfo.basePipelineHandle = VkPipeline{};
    createInfo.basePipelineIndex = -1;

    // - If the pipeline uses input attachments in shader, currently this is only used by
    //   ExpandResolveTexture subpass, hence we need to set the subpass to 0.
    // - Otherwise, the pipeline will operate on the main subpass.
    // - TODO(42240662): Add explicit way to specify subpass instead of implicitly deducing based on
    // mHasInputAttachment.
    //   That also means mHasInputAttachment would be removed in future.
    createInfo.subpass = mHasInputAttachment ? 0 : renderPassInfo.mainSubpass;

    if (buildCacheKey) {
        // Record cache key information now since createInfo is not stored.
        StreamIn(&mCacheKey, createInfo, layout->GetCacheKey());
    }

    // Try to see if we have anything in the blob cache.
    platform::metrics::DawnHistogramTimer cacheTimer(GetDevice()->GetPlatform());
    Ref<PipelineCache> cache = ToBackend(GetDevice()->GetOrCreatePipelineCache(GetCacheKey()));
    DAWN_TRY(
        CheckVkSuccess(device->fn.CreateGraphicsPipelines(device->GetVkDevice(), cache->GetHandle(),
                                                          1, &createInfo, nullptr, &*mHandle),
                       "CreateGraphicsPipelines"));
    cacheTimer.RecordMicroseconds(cache->CacheHit() ? "Vulkan.CreateGraphicsPipelines.CacheHit"
                                                    : "Vulkan.CreateGraphicsPipelines.CacheMiss");

    DAWN_TRY(cache->DidCompilePipeline());

    SetLabelImpl();

    for (uint32_t i = 0; i < stageCount; ++i) {
        device->fn.DestroyShaderModule(device->GetVkDevice(), shaderStages[i].module, nullptr);
    }

    return {};
}

void RenderPipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_RenderPipeline", GetLabel());
}

VkPipelineVertexInputStateCreateInfo RenderPipeline::ComputeVertexInputDesc(
    PipelineVertexInputStateCreateInfoTemporaryAllocations* tempAllocations) {
    // Fill in the "binding info" that will be chained in the create info
    uint32_t bindingCount = 0;
    for (VertexBufferSlot slot : GetVertexBuffersUsed()) {
        const VertexBufferInfo& bindingInfo = GetVertexBuffer(slot);

        VkVertexInputBindingDescription* bindingDesc = &tempAllocations->bindings[bindingCount];
        bindingDesc->binding = static_cast<uint8_t>(slot);
        bindingDesc->stride = bindingInfo.arrayStride;
        bindingDesc->inputRate = VulkanInputRate(bindingInfo.stepMode);

        bindingCount++;
    }

    // Fill in the "attribute info" that will be chained in the create info
    uint32_t attributeCount = 0;
    for (VertexAttributeLocation loc : GetAttributeLocationsUsed()) {
        const VertexAttributeInfo& attributeInfo = GetAttribute(loc);

        VkVertexInputAttributeDescription* attributeDesc =
            &tempAllocations->attributes[attributeCount];
        attributeDesc->location = static_cast<uint8_t>(loc);
        attributeDesc->binding = static_cast<uint8_t>(attributeInfo.vertexBufferSlot);
        attributeDesc->format = VulkanVertexFormat(attributeInfo.format);
        attributeDesc->offset = attributeInfo.offset;

        attributeCount++;
    }

    // Build the create info
    VkPipelineVertexInputStateCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.vertexBindingDescriptionCount = bindingCount;
    createInfo.pVertexBindingDescriptions = tempAllocations->bindings.data();
    createInfo.vertexAttributeDescriptionCount = attributeCount;
    createInfo.pVertexAttributeDescriptions = tempAllocations->attributes.data();
    return createInfo;
}

VkPipelineDepthStencilStateCreateInfo RenderPipeline::ComputeDepthStencilDesc() {
    const DepthStencilState* descriptor = GetDepthStencilState();

    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.pNext = nullptr;
    depthStencilState.flags = 0;

    // Depth writes only occur if depth is enabled
    depthStencilState.depthTestEnable =
        (descriptor->depthCompare == wgpu::CompareFunction::Always &&
         descriptor->depthWriteEnabled != wgpu::OptionalBool::True)
            ? VK_FALSE
            : VK_TRUE;
    depthStencilState.depthWriteEnable =
        descriptor->depthWriteEnabled == wgpu::OptionalBool::True ? VK_TRUE : VK_FALSE;
    depthStencilState.depthCompareOp = ToVulkanCompareOp(descriptor->depthCompare);
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;

    depthStencilState.stencilTestEnable = UsesStencil() ? VK_TRUE : VK_FALSE;

    depthStencilState.front.failOp = VulkanStencilOp(descriptor->stencilFront.failOp);
    depthStencilState.front.passOp = VulkanStencilOp(descriptor->stencilFront.passOp);
    depthStencilState.front.depthFailOp = VulkanStencilOp(descriptor->stencilFront.depthFailOp);
    depthStencilState.front.compareOp = ToVulkanCompareOp(descriptor->stencilFront.compare);

    depthStencilState.back.failOp = VulkanStencilOp(descriptor->stencilBack.failOp);
    depthStencilState.back.passOp = VulkanStencilOp(descriptor->stencilBack.passOp);
    depthStencilState.back.depthFailOp = VulkanStencilOp(descriptor->stencilBack.depthFailOp);
    depthStencilState.back.compareOp = ToVulkanCompareOp(descriptor->stencilBack.compare);

    // Dawn doesn't have separate front and back stencil masks.
    depthStencilState.front.compareMask = descriptor->stencilReadMask;
    depthStencilState.back.compareMask = descriptor->stencilReadMask;
    depthStencilState.front.writeMask = descriptor->stencilWriteMask;
    depthStencilState.back.writeMask = descriptor->stencilWriteMask;

    // The stencil reference is always dynamic
    depthStencilState.front.reference = 0;
    depthStencilState.back.reference = 0;

    return depthStencilState;
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::DestroyImpl() {
    RenderPipelineBase::DestroyImpl();
    PipelineVk::DestroyImpl();
    if (mHandle != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

VkPipeline RenderPipeline::GetHandle() const {
    return mHandle;
}

}  // namespace dawn::native::vulkan
