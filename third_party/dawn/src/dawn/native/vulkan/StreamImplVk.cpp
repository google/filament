// Copyright 2022 The Dawn & Tint Authors
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

#include <cstring>
#include <map>

#include "dawn/common/Assert.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/stream/Stream.h"
#include "dawn/native/vulkan/RenderPassCache.h"

#include <vulkan/utility/vk_struct_helper.hpp>  // NOLINT(build/include_order)

namespace dawn::native {

namespace {

namespace detail {

template <typename... VK_STRUCT_TYPES>
void ValidatePnextImpl(const VkBaseOutStructure* root) {
    const VkBaseOutStructure* next = reinterpret_cast<const VkBaseOutStructure*>(root->pNext);
    while (next != nullptr) {
        // Assert that the type of each pNext struct is exactly one of the specified
        // templates.
        DAWN_ASSERT(((vku::GetSType<VK_STRUCT_TYPES>() == next->sType ? 1 : 0) + ... + 0) == 1);
        next = reinterpret_cast<const VkBaseOutStructure*>(next->pNext);
    }
}

template <typename VK_STRUCT_TYPE>
void SerializePnextImpl(stream::Sink* sink, const VkBaseOutStructure* root) {
    const VkBaseOutStructure* next = reinterpret_cast<const VkBaseOutStructure*>(root->pNext);
    const VK_STRUCT_TYPE* found = nullptr;
    while (next != nullptr) {
        if (vku::GetSType<VK_STRUCT_TYPE>() == next->sType) {
            if (found == nullptr) {
                found = reinterpret_cast<const VK_STRUCT_TYPE*>(next);
            } else {
                // Fail an assert here since that means that the chain had more than one of
                // the same typed chained object.
                DAWN_ASSERT(false);
            }
        }
        next = reinterpret_cast<const VkBaseOutStructure*>(next->pNext);
    }
    if (found != nullptr) {
        StreamIn(sink, found);
    }
}

template <typename VK_STRUCT_TYPE,
          typename... VK_STRUCT_TYPES,
          typename = std::enable_if_t<(sizeof...(VK_STRUCT_TYPES) > 0)>>
void SerializePnextImpl(stream::Sink* sink, const VkBaseOutStructure* root) {
    SerializePnextImpl<VK_STRUCT_TYPE>(sink, root);
    SerializePnextImpl<VK_STRUCT_TYPES...>(sink, root);
}

template <typename VK_STRUCT_TYPE>
const VkBaseOutStructure* ToVkBaseOutStructure(const VK_STRUCT_TYPE* t) {
    // Checks to ensure proper type safety.
    static_assert(offsetof(VK_STRUCT_TYPE, sType) == offsetof(VkBaseOutStructure, sType) &&
                      offsetof(VK_STRUCT_TYPE, pNext) == offsetof(VkBaseOutStructure, pNext),
                  "Argument type is not a proper Vulkan structure type");
    return reinterpret_cast<const VkBaseOutStructure*>(t);
}

}  // namespace detail

template <typename... VK_STRUCT_TYPES,
          typename VK_STRUCT_TYPE,
          typename = std::enable_if_t<(sizeof...(VK_STRUCT_TYPES) > 0)>>
void SerializePnext(stream::Sink* sink, const VK_STRUCT_TYPE* t) {
    const VkBaseOutStructure* root = detail::ToVkBaseOutStructure(t);
    detail::ValidatePnextImpl<VK_STRUCT_TYPES...>(root);
    detail::SerializePnextImpl<VK_STRUCT_TYPES...>(sink, root);
}

// Empty template specialization so that we can put this in to ensure failures occur if new
// extensions are added without updating serialization.
template <typename VK_STRUCT_TYPE>
void SerializePnext(stream::Sink* sink, const VK_STRUCT_TYPE* t) {
    const VkBaseOutStructure* root = detail::ToVkBaseOutStructure(t);
    detail::ValidatePnextImpl<>(root);
}

}  // namespace

template <>
void stream::Stream<VkDescriptorSetLayoutBinding>::Write(stream::Sink* sink,
                                                         const VkDescriptorSetLayoutBinding& t) {
    StreamIn(sink, t.binding, t.descriptorType, t.descriptorCount, t.stageFlags);
}

template <>
void stream::Stream<VkDescriptorSetLayoutCreateInfo>::Write(
    stream::Sink* sink,
    const VkDescriptorSetLayoutCreateInfo& t) {
    StreamIn(sink, t.flags, Iterable(t.pBindings, t.bindingCount));
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkPushConstantRange>::Write(stream::Sink* sink, const VkPushConstantRange& t) {
    StreamIn(sink, t.stageFlags, t.offset, t.size);
}

template <>
void stream::Stream<VkPipelineLayoutCreateInfo>::Write(stream::Sink* sink,
                                                       const VkPipelineLayoutCreateInfo& t) {
    // The set layouts are not serialized here because they are pointers to backend objects.
    // They need to be cross-referenced with the frontend objects and serialized from there.
    StreamIn(sink, t.flags, Iterable(t.pPushConstantRanges, t.pushConstantRangeCount));
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>::Write(
    stream::Sink* sink,
    const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& t) {
    StreamIn(sink, t.requiredSubgroupSize);
}

template <>
void stream::Stream<VkPipelineRasterizationDepthClipStateCreateInfoEXT>::Write(
    stream::Sink* sink,
    const VkPipelineRasterizationDepthClipStateCreateInfoEXT& t) {
    StreamIn(sink, t.depthClipEnable, t.flags);
}

template <>
void stream::Stream<VkSpecializationMapEntry>::Write(stream::Sink* sink,
                                                     const VkSpecializationMapEntry& t) {
    StreamIn(sink, t.constantID, t.offset, t.size);
}

template <>
void stream::Stream<VkSpecializationInfo>::Write(stream::Sink* sink,
                                                 const VkSpecializationInfo& t) {
    StreamIn(sink, Iterable(t.pMapEntries, t.mapEntryCount),
             Iterable(static_cast<const uint8_t*>(t.pData), t.dataSize));
}

template <>
void stream::Stream<VkPipelineShaderStageCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineShaderStageCreateInfo& t) {
    // The shader module is not serialized here because it is a pointer to a backend object.
    StreamIn(sink, t.flags, t.stage, Iterable(t.pName, strlen(t.pName)), t.pSpecializationInfo);
    SerializePnext<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>(sink, &t);
}

template <>
void stream::Stream<VkComputePipelineCreateInfo>::Write(stream::Sink* sink,
                                                        const VkComputePipelineCreateInfo& t) {
    // The pipeline layout is not serialized here because it is a pointer to a backend object.
    // It needs to be cross-referenced with the frontend objects and serialized from there. The
    // base pipeline information is also currently not serialized since we do not use them in our
    // backend implementation. If we decide to use them later on, they also need to be
    // cross-referenced from the frontend.
    StreamIn(sink, t.flags, t.stage);
}

template <>
void stream::Stream<VkVertexInputBindingDescription>::Write(
    stream::Sink* sink,
    const VkVertexInputBindingDescription& t) {
    StreamIn(sink, t.binding, t.stride, t.inputRate);
}

template <>
void stream::Stream<VkVertexInputAttributeDescription>::Write(
    stream::Sink* sink,
    const VkVertexInputAttributeDescription& t) {
    StreamIn(sink, t.location, t.binding, t.format, t.offset);
}

template <>
void stream::Stream<VkPipelineVertexInputStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineVertexInputStateCreateInfo& t) {
    StreamIn(sink, t.flags, Iterable(t.pVertexBindingDescriptions, t.vertexBindingDescriptionCount),
             Iterable(t.pVertexAttributeDescriptions, t.vertexAttributeDescriptionCount));
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkPipelineInputAssemblyStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineInputAssemblyStateCreateInfo& t) {
    StreamIn(sink, t.flags, t.topology, t.primitiveRestartEnable);
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkPipelineTessellationStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineTessellationStateCreateInfo& t) {
    StreamIn(sink, t.flags, t.patchControlPoints);
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkViewport>::Write(stream::Sink* sink, const VkViewport& t) {
    StreamIn(sink, t.x, t.y, t.width, t.height, t.minDepth, t.maxDepth);
}

template <>
void stream::Stream<VkOffset2D>::Write(stream::Sink* sink, const VkOffset2D& t) {
    StreamIn(sink, t.x, t.y);
}

template <>
void stream::Stream<VkExtent2D>::Write(stream::Sink* sink, const VkExtent2D& t) {
    StreamIn(sink, t.width, t.height);
}

template <>
void stream::Stream<VkRect2D>::Write(stream::Sink* sink, const VkRect2D& t) {
    StreamIn(sink, t.offset, t.extent);
}

template <>
void stream::Stream<VkPipelineViewportStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineViewportStateCreateInfo& t) {
    StreamIn(sink, t.flags, Iterable(t.pViewports, t.viewportCount),
             Iterable(t.pScissors, t.scissorCount));
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkPipelineRasterizationStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineRasterizationStateCreateInfo& t) {
    StreamIn(sink, t.flags, t.depthClampEnable, t.rasterizerDiscardEnable, t.polygonMode,
             t.cullMode, t.frontFace, t.depthBiasEnable, t.depthBiasConstantFactor,
             t.depthBiasClamp, t.depthBiasSlopeFactor, t.lineWidth);
    SerializePnext<VkPipelineRasterizationDepthClipStateCreateInfoEXT>(sink, &t);
}

template <>
void stream::Stream<VkPipelineMultisampleStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineMultisampleStateCreateInfo& t) {
    StreamIn(sink, t.flags, t.rasterizationSamples, t.sampleShadingEnable, t.minSampleShading,
             t.pSampleMask, t.alphaToCoverageEnable, t.alphaToOneEnable);
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkStencilOpState>::Write(stream::Sink* sink, const VkStencilOpState& t) {
    StreamIn(sink, t.failOp, t.passOp, t.depthFailOp, t.compareOp, t.compareMask, t.writeMask,
             t.reference);
}

template <>
void stream::Stream<VkPipelineDepthStencilStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineDepthStencilStateCreateInfo& t) {
    StreamIn(sink, t.flags, t.depthTestEnable, t.depthWriteEnable, t.depthCompareOp,
             t.depthBoundsTestEnable, t.stencilTestEnable, t.front, t.back, t.minDepthBounds,
             t.maxDepthBounds);
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkPipelineColorBlendAttachmentState>::Write(
    stream::Sink* sink,
    const VkPipelineColorBlendAttachmentState& t) {
    StreamIn(sink, t.blendEnable, t.srcColorBlendFactor, t.dstColorBlendFactor, t.colorBlendOp,
             t.srcAlphaBlendFactor, t.dstAlphaBlendFactor, t.alphaBlendOp, t.colorWriteMask);
}

template <>
void stream::Stream<VkPipelineColorBlendStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineColorBlendStateCreateInfo& t) {
    StreamIn(sink, t.flags, t.logicOpEnable, t.logicOp, Iterable(t.pAttachments, t.attachmentCount),
             t.blendConstants);
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<VkPipelineDynamicStateCreateInfo>::Write(
    stream::Sink* sink,
    const VkPipelineDynamicStateCreateInfo& t) {
    StreamIn(sink, t.flags, Iterable(t.pDynamicStates, t.dynamicStateCount));
    SerializePnext(sink, &t);
}

template <>
void stream::Stream<vulkan::RenderPassCacheQuery>::Write(stream::Sink* sink,
                                                         const vulkan::RenderPassCacheQuery& t) {
    StreamIn(sink, t.colorMask.to_ulong(), t.resolveTargetMask.to_ulong(), t.sampleCount);

    // Manually iterate the color attachment indices and their corresponding format/load/store
    // ops because the data is sparse and may be uninitialized. Since we serialize the colorMask
    // member above, serializing sparse data should be fine here.
    for (auto i : t.colorMask) {
        StreamIn(sink, t.colorFormats[i], t.colorLoadOp[i], t.colorStoreOp[i]);
    }

    // Serialize the depth-stencil toggle bit, and the parameters if applicable.
    StreamIn(sink, t.hasDepthStencil);
    if (t.hasDepthStencil) {
        StreamIn(sink, t.depthStencilFormat, t.depthLoadOp, t.depthStoreOp, t.depthReadOnly,
                 t.stencilLoadOp, t.stencilStoreOp, t.stencilReadOnly);
    }
}

template <>
void stream::Stream<VkGraphicsPipelineCreateInfo>::Write(stream::Sink* sink,
                                                         const VkGraphicsPipelineCreateInfo& t) {
    // The pipeline layout and render pass are not serialized here because they are pointers to
    // backend objects. They need to be cross-referenced with the frontend objects and
    // serialized from there. The base pipeline information is also currently not serialized since
    // we do not use them in our backend implementation. If we decide to use them later on, they
    // also need to be cross-referenced from the frontend.
    StreamIn(sink, t.flags, Iterable(t.pStages, t.stageCount), t.pVertexInputState,
             t.pInputAssemblyState, t.pTessellationState, t.pViewportState, t.pRasterizationState,
             t.pMultisampleState, t.pDepthStencilState, t.pColorBlendState, t.pDynamicState,
             t.subpass);
    SerializePnext(sink, &t);
}

}  // namespace dawn::native
