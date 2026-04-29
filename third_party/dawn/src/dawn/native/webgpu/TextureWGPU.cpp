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

#include "dawn/native/webgpu/TextureWGPU.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/BlockInfo.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/QueueWGPU.h"
#include "dawn/native/webgpu/SharedFenceWGPU.h"
#include "dawn/native/webgpu/SharedTextureMemoryWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                            const UnpackedPtr<TextureDescriptor>& descriptor) {
    return AcquireRef(new Texture(device, descriptor));
}

// static
ResultOrError<Ref<Texture>> Texture::CreateFromSharedTextureMemory(
    const SharedTextureMemory* memory,
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    Device* device = ToBackend(memory->GetDevice());
    return AcquireRef(new Texture(device, descriptor, memory));
}

struct ComboTextureDescriptor {
    WGPUTextureDescriptor desc = {};
    std::vector<WGPUTextureFormat> viewFormats;
    std::string label;

    explicit ComboTextureDescriptor(const Texture* texture) {
        Device* device = ToBackend(texture->GetDevice());

        wgpu::TextureUsage actualUsage = texture->GetInternalUsage();
        // Resolve internal usages to regular ones
        if (actualUsage & kReadOnlyStorageTexture) {
            actualUsage &= ~kReadOnlyStorageTexture;
        }
        if (actualUsage & kWriteOnlyStorageTexture) {
            actualUsage &= ~kWriteOnlyStorageTexture;
        }
        if (actualUsage & kReadOnlyRenderAttachment) {
            actualUsage &= ~kReadOnlyRenderAttachment;
        }
        if (actualUsage & kResolveAttachmentLoadingUsage) {
            actualUsage &= ~kResolveAttachmentLoadingUsage;
        }
        if (!(actualUsage & wgpu::TextureUsage::TransientAttachment)) {
            actualUsage |= wgpu::TextureUsage::CopySrc;
        }

        viewFormats.reserve(texture->GetViewFormats().size());
        for (FormatIndex i : texture->GetViewFormats()) {
            viewFormats.push_back(ToAPI(device->GetValidInternalFormat(i).format));
        }

        label = texture->GetLabel();
        desc = {
            .nextInChain = nullptr,
            .label = ToOutputStringView(label),
            .usage = ToAPI(actualUsage),
            .dimension = ToAPI(texture->GetDimension()),
            .size = ToWGPU(texture->GetBaseSize()),
            .format = ToAPI(texture->GetFormat().format),
            .mipLevelCount = texture->GetNumMipLevels(),
            .sampleCount = texture->GetSampleCount(),
            .viewFormatCount = static_cast<uint32_t>(viewFormats.size()),
            .viewFormats = viewFormats.data(),
        };
    }
};

Texture::Texture(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor)
    : TextureBase(device, descriptor),
      RecordableObject(schema::ObjectType::Texture),
      ObjectWGPU(device->wgpu->textureRelease) {
    ComboTextureDescriptor comboDesc(this);

    mInnerHandle = device->wgpu->deviceCreateTexture(device->GetInnerHandle(), &comboDesc.desc);

    DAWN_ASSERT(mInnerHandle);
}

Texture::Texture(Device* device,
                 const UnpackedPtr<TextureDescriptor>& descriptor,
                 const SharedTextureMemory* memory)
    : TextureBase(device, descriptor),
      RecordableObject(schema::ObjectType::Texture),
      ObjectWGPU(device->wgpu->textureRelease) {
    ComboTextureDescriptor comboDesc(this);

    mInnerHandle =
        device->wgpu->sharedTextureMemoryCreateTexture(memory->GetInnerHandle(), &comboDesc.desc);
    mSharedResourceMemoryContents = memory->GetContents();

    DAWN_ASSERT(mInnerHandle);
}

void Texture::DestroyImpl(DestroyReason reason) {
    TextureBase::DestroyImpl(reason);
    auto& wgpu = ToBackend(GetDevice())->wgpu.get();
    wgpu.textureDestroy(mInnerHandle);
}

void Texture::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

ExecutionSerial Texture::OnEndAccess() {
    mPendingBeginAccess = false;
    return TextureBase::OnEndAccess();
}

void Texture::SynchronizeTextureBeforeUse() {
    if (SharedResourceMemoryContents* contents = GetSharedResourceMemoryContents()) {
        SharedTextureMemoryBase::PendingFenceList fences;
        contents->AcquirePendingFences(&fences);

        if (mPendingBeginAccess) {
            auto srm = contents->GetSharedResourceMemory().Promote();
            DAWN_ASSERT(srm != nullptr);
            auto* stm = static_cast<SharedTextureMemory*>(srm.Get());

            WGPUSharedTextureMemoryBeginAccessDescriptor innerDesc =
                WGPU_SHARED_TEXTURE_MEMORY_BEGIN_ACCESS_DESCRIPTOR_INIT;
            innerDesc.concurrentRead = mPendingConcurrentRead;
            innerDesc.initialized = mPendingInitialized;

            std::vector<WGPUSharedFence> innerFences;
            std::vector<uint64_t> signaledValues;
            innerFences.reserve(fences.size());
            signaledValues.reserve(fences.size());
            for (const auto& fence : fences) {
                innerFences.push_back(ToBackend(fence.object)->GetInnerHandle());
                signaledValues.push_back(fence.signaledValue);
            }
            innerDesc.fenceCount = innerFences.size();
            innerDesc.fences = innerFences.data();
            innerDesc.signaledValues = signaledValues.data();

            const DawnProcTable& wgpu = ToBackend(GetDevice())->wgpu.get();
            WGPUStatus status = wgpu.sharedTextureMemoryBeginAccess(stm->GetInnerHandle(),
                                                                    GetInnerHandle(), &innerDesc);
            DAWN_ASSERT(status == WGPUStatus_Success);

            mPendingBeginAccess = false;
        }
    }

    // Update the last serial to sync frontend state.
    mLastSharedTextureMemoryUsageSerial = GetDevice()->GetQueue()->GetPendingCommandSerial();
}

void Texture::SetPendingBeginAccess(bool concurrentRead, bool initialized) {
    mPendingBeginAccess = true;
    mPendingConcurrentRead = concurrentRead;
    mPendingInitialized = initialized;
}

// TextureView

// static
ResultOrError<Ref<TextureView>> TextureView::Create(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    Device* device = ToBackend(texture->GetDevice());
    auto* desc = ToAPI(*descriptor);

    WGPUTextureView innerView =
        device->wgpu->textureCreateView(ToBackend(texture)->GetInnerHandle(), desc);
    DAWN_ASSERT(innerView);

    return AcquireRef(new TextureView(texture, descriptor, innerView));
}

TextureView::TextureView(TextureBase* texture,
                         const UnpackedPtr<TextureViewDescriptor>& descriptor,
                         WGPUTextureView innerView)
    : TextureViewBase(texture, descriptor),
      RecordableObject(schema::ObjectType::TextureView),
      ObjectWGPU(ToBackend(texture->GetDevice())->wgpu->textureViewRelease) {
    mInnerHandle = innerView;
}

MaybeError Texture::AddReferenced(CaptureContext& captureContext) {
    // Textures do not reference other objects.
    return {};
}

MaybeError Texture::CaptureCreationParameters(CaptureContext& captureContext) {
    Device* device = ToBackend(GetDevice());
    std::vector<wgpu::TextureFormat> viewFormats;
    for (FormatIndex i : GetViewFormats()) {
        const Format& viewFormat = device->GetValidInternalFormat(i);
        viewFormats.emplace_back(viewFormat.format);
    }
    schema::Texture tex{{
        .usage = GetUsage(),
        .dimension = GetDimension(),
        .size = ToSchema(GetBaseSize()),
        .format = GetFormat().format,
        .mipLevelCount = GetNumMipLevels(),
        .sampleCount = GetSampleCount(),
        .viewFormats = viewFormats,
    }};
    Serialize(captureContext, tex);
    return {};
}

namespace {

bool IsDepthAspectDepth24Plus(const Format& format, Aspect aspect) {
    return aspect == Aspect::Depth && (format.format == wgpu::TextureFormat::Depth24Plus ||
                                       format.format == wgpu::TextureFormat::Depth24PlusStencil8);
}

MaybeError MapBufferAndWriteTextureData(CaptureContext::ScopedContentWriter& writer,
                                        Device* device,
                                        WGPUBuffer copyBuffer,
                                        BlockCount blockRows,
                                        uint32_t alignedBytesPerRow,
                                        uint32_t mappableBytesPerRow,
                                        uint32_t usedBytesPerRow) {
    struct MapAsyncResult {
        WGPUMapAsyncStatus status;
        std::string message;
    } mapAsyncResult = {};

    auto& wgpu = device->wgpu;

    // Map the buffer to read back the content.
    WGPUBufferMapCallbackInfo innerCallbackInfo = {};
    innerCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
    innerCallbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView message,
                                    void* result_param, void* userdata_param) {
        MapAsyncResult* result = reinterpret_cast<MapAsyncResult*>(result_param);
        result->status = status;
        result->message = ToString(message);
    };
    innerCallbackInfo.userdata1 = &mapAsyncResult;

    // Read this back synchronously.
    WGPUFutureWaitInfo waitInfo = {};
    uint64_t offset = 0;
    waitInfo.future = wgpu->bufferMapAsync(copyBuffer, WGPUMapMode_Read, offset,
                                           CaptureContext::kCopyBufferSize, innerCallbackInfo);
    wgpu->instanceWaitAny(device->GetInnerInstance(), 1, &waitInfo, UINT64_MAX);

    DAWN_ASSERT(mapAsyncResult.status == WGPUMapAsyncStatus_Success);

    if (mapAsyncResult.status != WGPUMapAsyncStatus_Success) {
        return DAWN_INTERNAL_ERROR(mapAsyncResult.message);
    }

    // We only write out the beginning of each row, the rest is padding.
    for (BlockCount blockRow{0}; blockRow < blockRows; ++blockRow) {
        const void* data = wgpu->bufferGetConstMappedRange(
            copyBuffer, uint32_t(blockRow) * alignedBytesPerRow, mappableBytesPerRow);
        writer.WriteContentBytes(data, usedBytesPerRow);
    }
    wgpu->bufferUnmap(copyBuffer);

    return {};
}

MaybeError CopyTextureRegionToBuffer(Device* device,
                                     const WGPUTexelCopyTextureInfo& srcTexture,
                                     const WGPUTexelCopyBufferInfo& dstBuffer,
                                     const WGPUExtent3D& copySize) {
    WGPUDevice innerDevice = device->GetInnerHandle();
    WGPUQueue queue = ToBackend(device->GetQueue())->GetInnerHandle();
    auto& wgpu = device->wgpu.get();

    WGPUCommandEncoder encoder = wgpu.deviceCreateCommandEncoder(innerDevice, nullptr);
    wgpu.commandEncoderCopyTextureToBuffer(encoder, &srcTexture, &dstBuffer, &copySize);
    WGPUCommandBuffer commandBuffer = wgpu.commandEncoderFinish(encoder, nullptr);
    wgpu.queueSubmit(queue, 1, &commandBuffer);
    wgpu.commandBufferRelease(commandBuffer);
    wgpu.commandEncoderRelease(encoder);

    return {};
}

}  // namespace

// TODO(451559917): Make this a helper for copying a texture to memory N bytes at a time.
// so that other parts of dawn can use it.
MaybeError Texture::CaptureContentIfNeeded(CaptureContext& captureContext,
                                           schema::ObjectId id,
                                           bool newResource) {
    // If it's all zeros or it's transient we don't need to capture it.
    if (!IsInitialized() || !newResource ||
        (GetUsage() & wgpu::TextureUsage::TransientAttachment)) {
        return {};
    }
    WGPUBuffer copyBuffer = captureContext.GetCopyBuffer();
    Device* device = ToBackend(GetDevice());

    // TODO(473870505): multi-planar textures.
    // Also, this can't handle compressed textures on compat as they are not readable (no copyT2B)
    auto format = GetFormat();
    for (dawn::native::Aspect aspect : IterateEnumMask(format.aspects)) {
        const TypedTexelBlockInfo& blockInfo = format.GetAspectInfo(aspect).block;
        // Check if (aspect == depth && depth24plus) ASSERT(byteSize == 4)
        // This is because there really is no size for depth24plus but it's conveniently
        // set to 4 in Format.cpp. The code below relies on this as it's going to use
        // r32float as a substitute for depth24plus and r32float has a size of 4.
        DAWN_ASSERT(!IsDepthAspectDepth24Plus(format, aspect) || blockInfo.byteSize == 4);

        // For each mip level copy the texture to a buffer, map it, and write the buffer data for
        // that level.
        for (uint32_t mipLevel = 0; mipLevel < GetNumMipLevels(); ++mipLevel) {
            auto size = TexelExtent3D(GetMipLevelSubresourcePhysicalSize(mipLevel, aspect));
            auto blockSize = blockInfo.ToBlock(size);
            uint32_t usedBytesPerRow = uint32_t(blockInfo.ToBytes(blockSize.width));
            uint32_t mappableBytesPerRow = RoundUp(usedBytesPerRow, 4);

            schema::RootCommandInitTextureCmd cmd{{
                .data = {{
                    .destination = {{
                        .textureId = id,
                        .mipLevel = mipLevel,
                        .origin = {{.x = 0, .y = 0, .z = 0}},
                        .aspect = ToDawn(aspect),
                    }},
                    .layout = {{
                        .offset = 0,
                        .bytesPerRow = usedBytesPerRow,
                        .rowsPerImage = uint32_t(blockSize.height),
                    }},
                    .size = {{
                        .width = uint32_t(size.width),
                        .height = uint32_t(size.height),
                        .depthOrArrayLayers = uint32_t(size.depthOrArrayLayers),
                    }},
                    .dataSize = blockInfo.ToBytes(blockSize.width * blockSize.height *
                                                  blockSize.depthOrArrayLayers),
                }},
            }};
            Serialize(captureContext, cmd);

            CaptureContext::ScopedContentWriter writer(captureContext);

            uint32_t alignedBytesPerRow = Align(usedBytesPerRow, 256);
            BlockCount maxBlockRowsPerRead{CaptureContext::kCopyBufferSize / alignedBytesPerRow};
            DAWN_ASSERT(maxBlockRowsPerRead > BlockCount{0});

            for (BlockCount z{0}; z < blockSize.depthOrArrayLayers; ++z) {
                for (BlockCount y{0}; y < blockSize.height; y += maxBlockRowsPerRead) {
                    BlockCount blockRows = std::min(maxBlockRowsPerRead, blockSize.height - y);

                    // Copy Data from Texture to Buffer. Then map and write buffer.
                    WGPUTexelCopyTextureInfo srcTexture{
                        .texture = GetInnerHandle(),
                        .mipLevel = mipLevel,
                        .origin =
                            {
                                .x = 0,
                                .y = uint32_t(blockInfo.ToTexelHeight(y)),
                                .z = uint32_t(blockInfo.ToTexelHeight(z)),
                            },
                        .aspect = ToWGPU(aspect),
                    };
                    WGPUTexelCopyBufferInfo dstBuffer{
                        .layout =
                            {
                                .offset = 0,
                                .bytesPerRow = alignedBytesPerRow,
                                .rowsPerImage = uint32_t(blockRows),
                            },
                        .buffer = copyBuffer,
                    };
                    WGPUExtent3D copySize{
                        .width = uint32_t(blockInfo.ToTexelWidth(blockSize.width)),
                        .height = uint32_t(blockInfo.ToTexelHeight(blockRows)),
                        .depthOrArrayLayers = 1,
                    };

                    DAWN_TRY(CopyTextureRegionToBuffer(device, srcTexture, dstBuffer, copySize));
                    DAWN_TRY(MapBufferAndWriteTextureData(writer, device, copyBuffer, blockRows,
                                                          alignedBytesPerRow, mappableBytesPerRow,
                                                          usedBytesPerRow));
                }
            }
        }
    }
    return {};
}

void TextureView::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError TextureView::AddReferenced(CaptureContext& captureContext) {
    return captureContext.AddResource(ToBackend(GetTexture()));
}

MaybeError TextureView::CaptureCreationParameters(CaptureContext& captureContext) {
    schema::TextureView tex{{
        .textureId = captureContext.GetId(GetTexture()),
        .format = GetFormat().format,
        .dimension = GetDimension(),
        .baseMipLevel = GetBaseMipLevel(),
        .mipLevelCount = GetLevelCount(),
        .baseArrayLayer = GetBaseArrayLayer(),
        .arrayLayerCount = GetLayerCount(),
        .aspect = ToDawn(GetAspects()),
        .usage = GetUsage(),
        .swizzle = {{
            .r = GetSwizzle().r,
            .g = GetSwizzle().g,
            .b = GetSwizzle().b,
            .a = GetSwizzle().a,
        }},
    }};
    Serialize(captureContext, tex);
    return {};
}

}  // namespace dawn::native::webgpu
