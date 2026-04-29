// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/ToWGPU.h"

#include "dawn/common/Math.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/BlockInfo.h"
#include "dawn/native/Commands.h"
#include "dawn/native/dawn_platform_autogen.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/Forward.h"
#include "dawn/native/webgpu/QuerySetWGPU.h"
#include "dawn/native/webgpu/TextureWGPU.h"

namespace dawn::native::webgpu {

WGPUExtent3D ToWGPU(const TexelExtent3D& extent) {
    return {
        .width = static_cast<uint32_t>(extent.width),
        .height = static_cast<uint32_t>(extent.height),
        .depthOrArrayLayers = static_cast<uint32_t>(extent.depthOrArrayLayers),
    };
}

WGPUOrigin3D ToWGPU(const TexelOrigin3D& origin) {
    return {
        .x = static_cast<uint32_t>(origin.x),
        .y = static_cast<uint32_t>(origin.y),
        .z = static_cast<uint32_t>(origin.z),
    };
}

WGPUOrigin2D ToWGPU(const Origin2D& origin) {
    return {
        .x = static_cast<uint32_t>(origin.x),
        .y = static_cast<uint32_t>(origin.y),
    };
}

WGPUExtent2D ToWGPU(const Extent2D& extent) {
    return {
        .width = static_cast<uint32_t>(extent.width),
        .height = static_cast<uint32_t>(extent.height),
    };
}

WGPUColor ToWGPU(const dawn::native::Color& color) {
    return {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
}

WGPUTexelCopyBufferLayout ToWGPU(const TexelCopyBufferLayout& copy) {
    return {
        .offset = copy.offset,
        .bytesPerRow = copy.bytesPerRow,
        .rowsPerImage = copy.rowsPerImage,
    };
}

WGPUTexelCopyBufferInfo ToWGPU(const BufferCopy& copy, const TypedTexelBlockInfo& blockInfo) {
    return {
        .layout =
            {
                .offset = copy.offset,
                .bytesPerRow = static_cast<uint32_t>(blockInfo.ToBytes(copy.blocksPerRow)),
                .rowsPerImage = static_cast<uint32_t>(copy.rowsPerImage),
            },
        .buffer = ToBackend(copy.buffer)->GetInnerHandle(),
    };
}

WGPUTexelCopyBufferInfo ToWGPU(const TexelCopyBufferLayout& copy, const BufferBase* buffer) {
    return {
        .layout = ToWGPU(copy),
        .buffer = ToBackend(buffer)->GetInnerHandle(),
    };
}

WGPUTextureAspect ToWGPU(const Aspect aspect) {
    switch (aspect) {
        case Aspect::Depth:
            return WGPUTextureAspect_DepthOnly;
        case Aspect::Stencil:
            return WGPUTextureAspect_StencilOnly;
        case Aspect::Plane0:
            return WGPUTextureAspect_Plane0Only;
        case Aspect::Plane1:
            return WGPUTextureAspect_Plane1Only;
        case Aspect::Plane2:
            return WGPUTextureAspect_Plane2Only;
        default:
            return WGPUTextureAspect_All;
    }
}

WGPULoadOp ToWGPU(const wgpu::LoadOp op) {
    switch (op) {
        case wgpu::LoadOp::Load:
            return WGPULoadOp_Load;
        case wgpu::LoadOp::Clear:
            return WGPULoadOp_Clear;
        case wgpu::LoadOp::ExpandResolveTexture:
            return WGPULoadOp_ExpandResolveTexture;
        default:
            return WGPULoadOp_Undefined;
    }
}

WGPUStoreOp ToWGPU(const wgpu::StoreOp op) {
    switch (op) {
        case wgpu::StoreOp::Store:
            return WGPUStoreOp_Store;
        case wgpu::StoreOp::Discard:
            return WGPUStoreOp_Discard;
        default:
            return WGPUStoreOp_Undefined;
    }
}

WGPUIndexFormat ToWGPU(const wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Uint16:
            return WGPUIndexFormat_Uint16;
        case wgpu::IndexFormat::Uint32:
            return WGPUIndexFormat_Uint32;
        default:
            return WGPUIndexFormat_Undefined;
    }
}

WGPUPassTimestampWrites ToWGPU(const TimestampWrites& writes) {
    return {
        .nextInChain = nullptr,
        .querySet = ToBackend(writes.querySet)->GetInnerHandle(),
        .beginningOfPassWriteIndex = writes.beginningOfPassWriteIndex,
        .endOfPassWriteIndex = writes.endOfPassWriteIndex,
    };
}

WGPUTexelCopyTextureInfo ToWGPU(const TextureCopy& copy) {
    return {
        .texture = ToBackend(copy.texture)->GetInnerHandle(),
        .mipLevel = copy.mipLevel,
        .origin = ToWGPU(copy.origin),
        .aspect = ToWGPU(copy.aspect),
    };
}

WGPURenderPassColorAttachment ToWGPU(const RenderPassColorAttachmentInfo& info) {
    return {
        .nextInChain = nullptr,
        .view = ToBackend(info.view)->GetInnerHandle(),
        // depthSlice is set to 0 for undefined in native::CommandEncoder.
        .depthSlice =
            info.depthSlice == 0 && info.view->GetDimension() != wgpu::TextureViewDimension::e3D
                ? WGPU_DEPTH_SLICE_UNDEFINED
                : info.depthSlice,
        .resolveTarget = info.resolveTarget == nullptr
                             ? nullptr
                             : ToBackend(info.resolveTarget)->GetInnerHandle(),
        .loadOp = ToWGPU(info.loadOp),
        .storeOp = ToWGPU(info.storeOp),
        .clearValue = ToWGPU(info.clearColor),
    };
}

WGPURenderPassDepthStencilAttachment ToWGPU(const RenderPassDepthStencilAttachmentInfo& info) {
    // Depth and stencil op in info is set to default value is not explicitly assigned.
    // Avoid setting them if there's no such aspect or the aspect is read only.
    bool setDepthOp = !info.depthReadOnly && IsSubset(Aspect::Depth, info.view->GetAspects());
    bool setStencilOp = !info.stencilReadOnly && IsSubset(Aspect::Stencil, info.view->GetAspects());
    return {
        .nextInChain = nullptr,
        .view = info.view == nullptr ? nullptr : ToBackend(info.view)->GetInnerHandle(),
        .depthLoadOp = setDepthOp ? ToWGPU(info.depthLoadOp) : WGPULoadOp_Undefined,
        .depthStoreOp = setDepthOp ? ToWGPU(info.depthStoreOp) : WGPUStoreOp_Undefined,
        .depthClearValue = info.clearDepth,
        .depthReadOnly = info.depthReadOnly,
        .stencilLoadOp = setStencilOp ? ToWGPU(info.stencilLoadOp) : WGPULoadOp_Undefined,
        .stencilStoreOp = setStencilOp ? ToWGPU(info.stencilStoreOp) : WGPUStoreOp_Undefined,
        .stencilClearValue = info.clearStencil,
        .stencilReadOnly = info.stencilReadOnly,
    };
}

WGPUStencilFaceState ToWGPU(const StencilFaceState* desc) {
    return {
        .compare = ToAPI(desc->compare),
        .failOp = ToAPI(desc->failOp),
        .depthFailOp = ToAPI(desc->depthFailOp),
        .passOp = ToAPI(desc->passOp),
    };
}

WGPUDepthStencilState ToWGPU(const DepthStencilState* desc) {
    return {
        .nextInChain = nullptr,
        .format = ToAPI(desc->format),
        .depthWriteEnabled = desc->depthWriteEnabled,
        .depthCompare = ToAPI(desc->depthCompare),
        .stencilFront = ToWGPU(&desc->stencilFront),
        .stencilBack = ToWGPU(&desc->stencilBack),
        .stencilReadMask = desc->stencilReadMask,
        .stencilWriteMask = desc->stencilWriteMask,
        .depthBias = desc->depthBias,
        .depthBiasSlopeScale = desc->depthBiasSlopeScale,
        .depthBiasClamp = desc->depthBiasClamp,
    };
}

WGPUPrimitiveState ToWGPU(const PrimitiveState* desc) {
    return {
        .nextInChain = nullptr,
        .topology = ToAPI(desc->topology),
        .stripIndexFormat = ToAPI(desc->stripIndexFormat),
        .frontFace = ToAPI(desc->frontFace),
        .cullMode = ToAPI(desc->cullMode),
        .unclippedDepth = desc->unclippedDepth,
    };
}

WGPUMultisampleState ToWGPU(const MultisampleState* desc) {
    return {
        .nextInChain = nullptr,
        .count = desc->count,
        .mask = desc->mask,
        .alphaToCoverageEnabled = desc->alphaToCoverageEnabled,
    };
}

WGPUBlendState ToWGPU(const BlendState* desc) {
    return {
        .color =
            {
                .operation = ToAPI(desc->color.operation),
                .srcFactor = ToAPI(desc->color.srcFactor),
                .dstFactor = ToAPI(desc->color.dstFactor),
            },
        .alpha =
            {
                .operation = ToAPI(desc->alpha.operation),
                .srcFactor = ToAPI(desc->alpha.srcFactor),
                .dstFactor = ToAPI(desc->alpha.dstFactor),
            },
    };
}

void PopulateWGPUConstants(std::vector<WGPUConstantEntry>* constants,
                           std::vector<std::string>* keys,
                           const PipelineConstantEntries& entries) {
    keys->reserve(entries.size());
    constants->reserve(entries.size());
    for (const auto& [key, value] : entries) {
        keys->push_back(key);
        constants->push_back(WGPUConstantEntry{
            .nextInChain = nullptr,
            .key = ToOutputStringView(keys->back()),
            .value = value,
        });
    }
}

}  // namespace dawn::native::webgpu
