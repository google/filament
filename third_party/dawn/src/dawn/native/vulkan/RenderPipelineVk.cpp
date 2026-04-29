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

uint16_t PackStencilOpState(VkStencilOpState state, VkBool32 enabled) {
    // Both VkStencilOp and VkCompareOp have values ranging from 0-7, so they fit in 3 bits.
    // So we can encode the full dynamic stencil state in 12 bits.
    DAWN_ASSERT(static_cast<uint32_t>(state.failOp) < 8);
    DAWN_ASSERT(static_cast<uint32_t>(state.passOp) < 8);
    DAWN_ASSERT(static_cast<uint32_t>(state.depthFailOp) < 8);
    DAWN_ASSERT(static_cast<uint32_t>(state.compareOp) < 8);
    uint16_t packed = state.failOp | state.passOp << 3 | state.depthFailOp << 6 |
                      state.compareOp << 9 |
                      (enabled ? 0x8000 : 0);  // Set high bit if stencil is enabled.
    return packed;
}

VkStencilOpState UnpackStencilOpState(uint16_t packed) {
    VkStencilOpState state = {
        .failOp = static_cast<VkStencilOp>(packed & 0x0007),
        .passOp = static_cast<VkStencilOp>((packed >> 3) & 0x0007),
        .depthFailOp = static_cast<VkStencilOp>((packed >> 6) & 0x0007),
        .compareOp = static_cast<VkCompareOp>((packed >> 9) & 0x0007),
        .compareMask = 0,
        .writeMask = 0,
        .reference = 0,
    };
    return state;
}

VkPrimitiveTopology GetTopologyClass(VkPrimitiveTopology topology) {
    switch (topology) {
        case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:
            DAWN_UNREACHABLE();
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
}

}  // anonymous namespace

// static
Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

MaybeError RenderPipeline::InitializeImpl() {
    // Gather list of internal immediate constants used by this pipeline
    if ((NeedsPixelCenterPolyfill() || UsesFragDepth()) && !HasUnclippedDepth()) {
        mImmediateMask |= GetImmediateConstantBlockBits(
            offsetof(RenderImmediateConstants, clampFragDepth), sizeof(ClampFragDepthArgs));
    }

    if (GetDevice()->NeedsStaticSamplerForExternalTexture() && GetLayout()->HasExternalTextures()) {
        DAWN_ASSERT(!GetLayout()->HasAPIStaticSamplers());
        mRequiresSpecialization = true;
    }

    // The cache key is only used for storing VkPipelineCache objects in BlobStore. That's not
    // done with the monolithic pipeline cache so it's unnecessary work and memory usage.
    bool buildCacheKey =
        !GetDevice()->GetTogglesState().IsEnabled(Toggle::VulkanMonolithicPipelineCache);

    Specialization specialization = {
        .layout = {.pushConstantBytes = ToPushConstantBytes(mImmediateMask)},
    };

    SpecializationResult r;
    DAWN_TRY_ASSIGN(r, InitializeSpecialization(specialization, buildCacheKey));
    mHandles = {.pipeline = r.pipeline->Get(), .layout = r.layout->Get()};

    mSpecializations.emplace(std::move(specialization), std::move(r));

    return {};
}

ResultOrError<PipelineHandles> RenderPipeline::GetOrCreateSpecializedHandle(
    Specialization&& specializationIn) {
    Specialization specialization = specializationIn;
    specialization.layout.pushConstantBytes = ToPushConstantBytes(mImmediateMask);

    if (auto it = mSpecializations.find(specialization); it != mSpecializations.end()) {
        return PipelineHandles{.pipeline = it->second.pipeline->Get(),
                               .layout = it->second.layout->Get()};
    }

    // Do no make a new cache key, so that the VkPipelineCache from InitializeImpl is used for all
    // specializations.
    SpecializationResult r;
    DAWN_TRY_ASSIGN(r, InitializeSpecialization(specialization, /*buildCacheKey=*/false));

    auto handles = PipelineHandles{.pipeline = r.pipeline->Get(), .layout = r.layout->Get()};

    mSpecializations.emplace(std::move(specialization), std::move(r));
    return handles;
}

ResultOrError<RenderPipeline::SpecializationResult> RenderPipeline::InitializeSpecialization(
    const Specialization& specialization,
    bool buildCacheKey) {
    Device* device = ToBackend(GetDevice());
    PipelineLayout* layout = ToBackend(GetLayout());

    if (buildCacheKey) {
        // Vulkan devices need cache UUID field to be serialized into pipeline cache keys.
        StreamIn(&mCacheKey, device->GetDeviceInfo().properties.pipelineCacheUUID);
    }

    SpecializationResult result;
    DAWN_TRY_ASSIGN(result.layout,
                    layout->GetOrCreateVkLayoutObject(std::move(specialization.layout)));

    // There are at most 2 shader stages in render pipeline, i.e. vertex and fragment
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    uint32_t stageCount = 0;

    auto AddShaderStage = [&](const ShaderModule::CompileParameters& compileParams) -> MaybeError {
        ShaderModule::ModuleAndSpirv moduleAndSpirv;
        DAWN_TRY_ASSIGN(moduleAndSpirv,
                        ToBackend(compileParams.stage->module)->GetHandleAndSpirv(compileParams));
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
        shaderStage->stage = VulkanShaderStage(compileParams.stage->metadata->stage);
        // string_view returned by GetIsolatedEntryPointName() points to a null-terminated string.
        shaderStage->pName = device->GetIsolatedEntryPointName().data();

        stageCount++;
        return {};
    };

    // Add the vertex stage that's always present.
    DAWN_TRY(AddShaderStage({
        .stage = &GetStage(SingleShaderStage::Vertex),
        .layout = layout,
        .immediateMask = GetImmediateMask(),
        .ycbcrExternalTextures = &specialization.ycbcrExternalTextures,
        .emitPointSize = GetPrimitiveTopology() == wgpu::PrimitiveTopology::PointList,
        .polyfillPixelCenter = NeedsPixelCenterPolyfill(),
    }));

    // Add the fragment stage if present.
    if (GetStageMask() & wgpu::ShaderStage::Fragment) {
        DAWN_TRY(AddShaderStage({
            .stage = &GetStage(SingleShaderStage::Fragment),
            .layout = layout,
            .immediateMask = GetImmediateMask(),
            .ycbcrExternalTextures = &specialization.ycbcrExternalTextures,
            .polyfillPixelCenter = NeedsPixelCenterPolyfill(),
            .needsMultisampledFramebufferFetch = UseSampleRateShading() && UsesFramebufferFetch(),
        }));
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

    // A placeholder viewport/scissor info. The validation layers force us to provide at least
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
    const uint32_t kMaxDynamicStates = 14;
    absl::InlinedVector<VkDynamicState, kMaxDynamicStates> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,     VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,   VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    };

    // If ExtendedDynamicState is available tag several other states as dynamic.
    // The states will be ignored by CreateGraphicsPipelines, but they are reset to a constant value
    // here (0 where possible) to ensure the Vulkan cache key is compatible between pipelines.
    if (device->IsToggleEnabled(Toggle::VulkanUseExtendedDynamicState)) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT);
        mDynamicState.primitiveTopology = inputAssembly.topology;
        // TODO(463893795): Look into using dynamicPrimitiveTopologyUnrestricted from
        // VK_EXT_extended_dynamic_state3 to remove the requirement that this needs to be set to the
        // same topology class (point/line/triangle) as the dynamic state it will be used with.
        inputAssembly.topology = GetTopologyClass(inputAssembly.topology);

        dynamicStates.push_back(VK_DYNAMIC_STATE_CULL_MODE_EXT);
        mDynamicState.cullMode = rasterization.cullMode;
        rasterization.cullMode = VK_CULL_MODE_NONE;

        dynamicStates.push_back(VK_DYNAMIC_STATE_FRONT_FACE_EXT);
        mDynamicState.frontFace = rasterization.frontFace;
        rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT);
        mDynamicState.depthTestEnable = depthStencilState.depthTestEnable;
        depthStencilState.depthTestEnable = VK_FALSE;

        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT);
        mDynamicState.depthWriteEnable = depthStencilState.depthWriteEnable;
        depthStencilState.depthWriteEnable = VK_FALSE;

        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT);
        mDynamicState.depthCompareOp = depthStencilState.depthCompareOp;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_NEVER;

        dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT);
        mDynamicState.stencilTestEnable = depthStencilState.stencilTestEnable;
        depthStencilState.stencilTestEnable = VK_FALSE;

        dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_OP_EXT);
        mDynamicState.packedFrontStencil =
            PackStencilOpState(depthStencilState.front, depthStencilState.stencilTestEnable);
        depthStencilState.front.failOp = VK_STENCIL_OP_KEEP;
        depthStencilState.front.passOp = VK_STENCIL_OP_KEEP;
        depthStencilState.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depthStencilState.front.compareOp = VK_COMPARE_OP_NEVER;

        mDynamicState.packedBackStencil =
            PackStencilOpState(depthStencilState.back, depthStencilState.stencilTestEnable);
        depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
        depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
        depthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
        depthStencilState.back.compareOp = VK_COMPARE_OP_NEVER;
    }

    VkPipelineDynamicStateCreateInfo dynamic;
    dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic.pNext = nullptr;
    dynamic.flags = 0;
    dynamic.dynamicStateCount = dynamicStates.size();
    dynamic.pDynamicStates = dynamicStates.data();

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
    createInfo.layout = result.layout->Get();
    createInfo.basePipelineHandle = VkPipeline{};
    createInfo.basePipelineIndex = -1;
    PNextChainBuilder createInfoChain(&createInfo);

    RenderPassCache::RenderPassInfo renderPassInfo;
    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo;
    PerColorAttachment<VkFormat> colorAttachmentFormats;

    if (device->GetRenderPassType() == VulkanRenderPassType::DynamicRendering) {
        // Dynamic rendering doesn't need a VkRenderPass object, just a description of the
        // attachments formats that will be used with this pipeline.
        VkRenderPass nullRenderPass = VK_NULL_HANDLE;
        createInfo.renderPass = nullRenderPass;

        createInfoChain.Add(&pipelineRenderingCreateInfo,
                            VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR);

        pipelineRenderingCreateInfo.viewMask = 0;
        pipelineRenderingCreateInfo.colorAttachmentCount = 0;
        pipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        // Initialize all potential color attachment formats to undefined, which allows the
        // attachments to be sparse.
        colorAttachmentFormats.fill(VK_FORMAT_UNDEFINED);

        // Set the formats of the color attachments in use.
        ColorAttachmentMask attachmentMask = GetColorAttachmentsMask();
        for (auto i : attachmentMask) {
            colorAttachmentFormats[i] = VulkanImageFormat(device, GetColorAttachmentFormat(i));
        }

        pipelineRenderingCreateInfo.colorAttachmentCount =
            static_cast<uint32_t>(GetHighestBitIndexPlusOne(attachmentMask));
        pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachmentFormats.data();

        // Set the formats of the depth/stencil attachment.
        if (HasDepthStencilAttachment()) {
            wgpu::TextureFormat dsFormat = GetDepthStencilFormat();
            const Format& internalFormat = device->GetValidInternalFormat(dsFormat);
            VkFormat vkDsFormat = VulkanImageFormat(device, dsFormat);

            if (internalFormat.HasDepth()) {
                pipelineRenderingCreateInfo.depthAttachmentFormat = vkDsFormat;
            }
            if (internalFormat.HasStencil()) {
                pipelineRenderingCreateInfo.stencilAttachmentFormat = vkDsFormat;
            }
        }
    } else {
        // Get a VkRenderPass that matches the attachment formats for this pipeline.
        // VkRenderPass compatibility rules let us provide placeholder data for a bunch of
        // arguments. Load and store ops are all equivalent, though we still specify
        // ExpandResolveTexture as that controls the use of input attachments. Single subpass
        // VkRenderPasses are compatible irrespective of resolve attachments being used, but for
        // ExpandResolveTexture that uses two subpasses we need to specify which attachments will
        // be resolved.
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
            // This bool should match the value used in the final RenderPass, but we don't have the
            // appropriate MSRTSS values exposed here. Issue can be safely ignored, though.
            // See comments on skipped message VUID-vkCmdDraw-renderPass-02684 in BackendVk.cpp.
            bool renderToSingleSample = false;
            query.SetColor(i, GetColorAttachmentFormat(i), colorLoadOp, wgpu::StoreOp::Store,
                           hasResolveTarget, renderToSingleSample);
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

        createInfo.renderPass = renderPassInfo.renderPass;
    }

    // - If the pipeline uses input attachments in shader, currently this is only used by
    //   ExpandResolveTexture subpass, hence we need to set the subpass to 0.
    // - Otherwise, the pipeline will operate on the main subpass.
    // - TODO(42240662): Add explicit way to specify subpass instead of implicitly deducing based on
    // mHasInputAttachment.
    //   That also means mHasInputAttachment would be removed in future.
    createInfo.subpass = mHasInputAttachment ? 0 : renderPassInfo.mainSubpass;

    // If possible, specify where exactly robustness should be added in the pipeline. This is
    // necessary because when bindless is enabled and robustBufferAccessUpdateAfterBind is false, we
    // must only set robust buffer access on vertexInputs.
    VkPipelineRobustnessCreateInfo robustnessCreateInfo;
    if (device->GetDeviceInfo().HasExt(DeviceExt::PipelineRobustness)) {
        createInfoChain.Add(&robustnessCreateInfo,
                            VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO);

        // Without any specific toggle only vertex inputs can be made robust enough for WebGPU.
        robustnessCreateInfo.vertexInputs =
            VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS;
        robustnessCreateInfo.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED;
        robustnessCreateInfo.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED;
        robustnessCreateInfo.images = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED;

        if (device->IsToggleEnabled(Toggle::VulkanUseBufferRobustAccess2)) {
            // Uniform buffers are checked with WebGPU validation and don't need robustness.
            robustnessCreateInfo.storageBuffers =
                VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2;
        }
        if (device->IsToggleEnabled(Toggle::VulkanUseImageRobustAccess2)) {
            robustnessCreateInfo.images =
                VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2;
        }
    }

    // Record cache key information now since createInfo is not stored. Only store for the noop
    // specialization created in InitializeImpl so that future specializations use the same pipeline
    // cache, and may reuse the VkPipeline when they happen to be the same on the driver side.
    if (buildCacheKey) {
        StreamIn(&mCacheKey, createInfo, layout->GetCacheKey());
    }

    // Try to see if we have anything in the blob cache.
    platform::metrics::DawnHistogramTimer cacheTimer(GetDevice()->GetPlatform());
    Ref<PipelineCache> cache = ToBackend(GetDevice()->GetOrCreatePipelineCache(GetCacheKey()));
    VkPipeline pipeline;
    DAWN_TRY(
        CheckVkSuccess(device->fn.CreateGraphicsPipelines(device->GetVkDevice(), cache->GetHandle(),
                                                          1, &createInfo, nullptr, &*pipeline),
                       "CreateGraphicsPipelines"));
    result.pipeline = AcquireRef(new RefCountedVkHandle<VkPipeline>(device, pipeline));
    cacheTimer.RecordMicroseconds(cache->CacheHit() ? "Vulkan.CreateGraphicsPipelines.CacheHit"
                                                    : "Vulkan.CreateGraphicsPipelines.CacheMiss");

    DAWN_TRY(cache->DidCompilePipeline());

    SetLabelImpl();

    for (uint32_t i = 0; i < stageCount; ++i) {
        device->fn.DestroyShaderModule(device->GetVkDevice(), shaderStages[i].module, nullptr);
    }

    return result;
}

#define SetDynamicState(function, state)                       \
    if (!prevState || prevState->state != mDynamicState.state) \
    device->fn.function(commands, mDynamicState.state)

void RenderPipeline::ApplyDynamicState(VkCommandBuffer& commands,
                                       const RenderPipeline* prevPipeline) const {
    Device* device = ToBackend(GetDevice());

    // If Dynamic state is enabled, apply state changes between this pipeline and the previous one.
    if (device->IsToggleEnabled(Toggle::VulkanUseExtendedDynamicState)) {
        const RenderPipeline::DynamicState* prevState = nullptr;
        if (prevPipeline) {
            prevState = &prevPipeline->mDynamicState;
        }

        SetDynamicState(CmdSetPrimitiveTopologyEXT, primitiveTopology);
        SetDynamicState(CmdSetCullModeEXT, cullMode);
        SetDynamicState(CmdSetFrontFaceEXT, frontFace);
        SetDynamicState(CmdSetDepthTestEnableEXT, depthTestEnable);
        SetDynamicState(CmdSetDepthWriteEnableEXT, depthWriteEnable);
        SetDynamicState(CmdSetDepthCompareOpEXT, depthCompareOp);
        SetDynamicState(CmdSetStencilTestEnableEXT, stencilTestEnable);

        if (mDynamicState.stencilTestEnable) {
            if (!prevState || prevState->packedFrontStencil != mDynamicState.packedFrontStencil) {
                VkStencilOpState stencilOp = UnpackStencilOpState(mDynamicState.packedFrontStencil);
                device->fn.CmdSetStencilOpEXT(commands, VK_STENCIL_FACE_FRONT_BIT, stencilOp.failOp,
                                              stencilOp.passOp, stencilOp.depthFailOp,
                                              stencilOp.compareOp);
            }

            if (!prevState || prevState->packedBackStencil != mDynamicState.packedBackStencil) {
                VkStencilOpState stencilOp = UnpackStencilOpState(mDynamicState.packedBackStencil);
                device->fn.CmdSetStencilOpEXT(commands, VK_STENCIL_FACE_BACK_BIT, stencilOp.failOp,
                                              stencilOp.passOp, stencilOp.depthFailOp,
                                              stencilOp.compareOp);
            }
        }
    }
}

#undef SetDynamicState

void RenderPipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandles.pipeline, "Dawn_RenderPipeline", GetLabel());
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

void RenderPipeline::DestroyImpl(DestroyReason reason) {
    RenderPipelineBase::DestroyImpl(reason);

    mSpecializations.clear();

    // Handles were owned by refs in mSpecializations that were just deleted.
    mHandles = {};
}

bool RenderPipeline::NeedsPixelCenterPolyfill() const {
    return UseSampleRateShading() && UsesFragPosition();
}

bool RenderPipeline::RequiresSpecialization() const {
    return mRequiresSpecialization;
}

VkPipeline RenderPipeline::GetHandle() const {
    DAWN_ASSERT(mHandles.pipeline != VK_NULL_HANDLE);
    return mHandles.pipeline;
}

VkPipelineLayout RenderPipeline::GetVkLayout() const {
    DAWN_ASSERT(mHandles.layout != nullptr);
    return mHandles.layout;
}

}  // namespace dawn::native::vulkan
