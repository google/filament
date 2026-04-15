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

#include "src/dawn/native/webgpu/CaptureContext.h"

#include <concepts>
#include <vector>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/BlockInfo.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/QuerySetWGPU.h"
#include "dawn/native/webgpu/QueueWGPU.h"
#include "dawn/native/webgpu/Serialization.h"
#include "dawn/native/webgpu/TextureWGPU.h"

namespace dawn::native::webgpu {

CaptureContext::ScopedContentWriter::ScopedContentWriter(CaptureContext& context)
    : mContext(context) {}

CaptureContext::ScopedContentWriter::~ScopedContentWriter() {
    uint64_t offset = mBytesWritten % 4;
    if (offset) {
        static char zero[3] = {0};
        uint64_t paddingNeeded = 4 - offset;
        mContext->WriteContentBytes(zero, paddingNeeded);
    }
}

void CaptureContext::ScopedContentWriter::WriteContentBytes(const void* data, size_t size) {
    mContext->WriteContentBytes(data, size);
    mBytesWritten += size;
}

MaybeError CaptureContext::CaptureCreation(schema::ObjectId id,
                                           const std::string& label,
                                           RecordableObject* object) {
    schema::RootCommandCreateResourceCmd cmd{{
        .data = {{
            .resource = {{
                .type = object->GetObjectType(),
                .id = id,
                .label = label,
            }},
        }},
    }};
    Serialize(*this, cmd);
    return object->CaptureCreationParameters(*this);
}

CaptureContext::CaptureContext(Device* device,
                               std::ostream& commandStream,
                               std::ostream& contentStream)
    : mDevice(device), mCommandStream(commandStream), mContentStream(contentStream) {}

CaptureContext::~CaptureContext() {
    if (mCopyBuffer) {
        mDevice->wgpu->bufferDestroy(mCopyBuffer);
    }
}

WGPUBuffer CaptureContext::GetCopyBuffer() {
    if (!mCopyBuffer) {
        WGPUBufferDescriptor desc = {};
        // Add a label to the copy buffer for better debugging experience.
        desc.label = ToOutputStringView("Capture Copy Buffer");
        desc.size = kCopyBufferSize;
        desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
        mCopyBuffer = mDevice->wgpu->deviceCreateBuffer(mDevice->GetInnerHandle(), &desc);
    }
    return mCopyBuffer;
}

void CaptureContext::WriteContentBytes(const void* data, size_t size) {
    mContentStream->write(reinterpret_cast<const char*>(data), size);
}

void CaptureContext::WriteCommandBytes(const void* data, size_t size) {
    mCommandStream->write(reinterpret_cast<const char*>(data), size);
    mCommandBytesWritten += size;
}

MaybeError CaptureContext::CaptureQueueWriteBuffer(Buffer* buffer,
                                                   uint64_t bufferOffset,
                                                   const void* data,
                                                   size_t size) {
    schema::ObjectId id;
    DAWN_TRY_ASSIGN(id, AddResourceAndGetId(buffer));
    schema::RootCommandWriteBufferCmd cmd{{
        .data = {{
            .bufferId = id,
            .bufferOffset = bufferOffset,
            .size = size,
        }},
    }};

    Serialize(*this, cmd);
    WriteContentBytes(data, size);
    return {};
}

MaybeError CaptureContext::CaptureQueueWriteTexture(const TexelCopyTextureInfo& destination,
                                                    const void* data,
                                                    size_t dataSize,
                                                    const TexelCopyBufferLayout& dataLayout,
                                                    const TexelExtent3D& writeSizePixel) {
    DAWN_TRY(AddResource(ToBackend(destination.texture)));
    schema::RootCommandWriteTextureCmd cmd{{
        .data = {{
            .destination = ToSchema(*this, destination),
            .layout = ToSchema(dataLayout),
            .size = ToSchema(writeSizePixel),
            .dataSize = dataSize,
        }},
    }};
    Serialize(*this, cmd);

    CaptureContext::ScopedContentWriter writer(*this);
    writer.WriteContentBytes(data, dataSize);
    return {};
}

wgpu::TextureAspect ToDawn(const Aspect aspect) {
    switch (aspect) {
        case Aspect::Depth:
            return wgpu::TextureAspect::DepthOnly;
        case Aspect::Stencil:
            return wgpu::TextureAspect::StencilOnly;
        case Aspect::Plane0:
            return wgpu::TextureAspect::Plane0Only;
        case Aspect::Plane1:
            return wgpu::TextureAspect::Plane1Only;
        case Aspect::Plane2:
            return wgpu::TextureAspect::Plane2Only;
        default:
            return wgpu::TextureAspect::All;
    }
}

schema::Origin3D ToSchema(const TexelOrigin3D& origin) {
    return {{
        .x = static_cast<uint32_t>(origin.x),
        .y = static_cast<uint32_t>(origin.y),
        .z = static_cast<uint32_t>(origin.z),
    }};
}

schema::Origin2D ToSchema(const Origin2D& origin) {
    return {{
        .x = origin.x,
        .y = origin.y,
    }};
}

schema::Extent3D ToSchema(const TexelExtent3D& extent) {
    return {{
        .width = static_cast<uint32_t>(extent.width),
        .height = static_cast<uint32_t>(extent.height),
        .depthOrArrayLayers = static_cast<uint32_t>(extent.depthOrArrayLayers),
    }};
}

schema::Extent2D ToSchema(const Extent2D& extent) {
    return {{
        .width = extent.width,
        .height = extent.height,
    }};
}

schema::Color ToSchema(const Color& color) {
    return {{
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    }};
}

schema::TexelCopyBufferLayout ToSchema(const BufferCopy& bufferCopy,
                                       const TypedTexelBlockInfo& blockInfo) {
    return {{
        .offset = bufferCopy.offset,
        .bytesPerRow = static_cast<uint32_t>(blockInfo.ToBytes(bufferCopy.blocksPerRow)),
        .rowsPerImage = static_cast<uint32_t>(bufferCopy.rowsPerImage),
    }};
}

schema::TexelCopyBufferInfo ToSchema(CaptureContext& captureContext,
                                     const BufferCopy& bufferCopy,
                                     const TypedTexelBlockInfo& blockInfo) {
    return {{
        .bufferId = captureContext.GetId(bufferCopy.buffer),
        .layout = ToSchema(bufferCopy, blockInfo),
    }};
}

schema::TexelCopyTextureInfo ToSchema(CaptureContext& captureContext,
                                      const TextureCopy& textureCopy) {
    return {{
        .textureId = captureContext.GetId(textureCopy.texture),
        .mipLevel = textureCopy.mipLevel,
        .origin = ToSchema(textureCopy.origin),
        .aspect = ToDawn(textureCopy.aspect),
    }};
}

schema::TexelCopyBufferLayout ToSchema(const TexelCopyBufferLayout& layout) {
    return {{
        .offset = layout.offset,
        .bytesPerRow = layout.bytesPerRow,
        .rowsPerImage = layout.rowsPerImage,
    }};
}

schema::TexelCopyTextureInfo ToSchema(CaptureContext& captureContext,
                                      const TexelCopyTextureInfo& info) {
    return {{
        .textureId = captureContext.GetId(info.texture),
        .mipLevel = info.mipLevel,
        .origin = ToSchema(info.origin),
        .aspect = info.aspect,
    }};
}

schema::TimestampWrites ToSchema(CaptureContext& captureContext,
                                 const TimestampWrites& timestampWrites) {
    return {{
        .querySetId = captureContext.GetId(timestampWrites.querySet),
        .beginningOfPassWriteIndex = timestampWrites.beginningOfPassWriteIndex,
        .endOfPassWriteIndex = timestampWrites.endOfPassWriteIndex,
    }};
}

schema::ProgrammableStage ToSchema(CaptureContext& captureContext, const ProgrammableStage& stage) {
    std::vector<schema::PipelineConstant> constants;
    for (const auto& [name, value] : stage.constants) {
        constants.push_back({{
            .name = name,
            .value = value,
        }});
    }
    return {{
        .moduleId = captureContext.GetId(stage.module.Get()),
        .entryPoint = stage.entryPoint,
        .constants = constants,
    }};
}

}  // namespace dawn::native::webgpu
