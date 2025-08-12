// Copyright 2017 The Dawn & Tint Authors
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

#include <utility>

#include "dawn/native/CommandBuffer.h"

#include "dawn/native/Buffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Format.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

CommandBufferBase::CommandBufferBase(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor)
    : ApiObjectBase(encoder->GetDevice(), descriptor->label),
      mCommands(encoder->AcquireCommands()),
      mResourceUsages(encoder->AcquireResourceUsages()),
      mIndirectDrawMetadata(encoder->AcquireIndirectDrawMetadata()),
      mEncoderLabel(encoder->GetLabel()) {
    GetObjectTrackingList()->Track(this);
}

CommandBufferBase::CommandBufferBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label) {}

// static
Ref<CommandBufferBase> CommandBufferBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new CommandBufferBase(device, ObjectBase::kError, label));
}

ObjectType CommandBufferBase::GetType() const {
    return ObjectType::CommandBuffer;
}

void CommandBufferBase::FormatLabel(absl::FormatSink* s) const {
    s->Append(ObjectTypeAsString(GetType()));

    const std::string& label = GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }

    if (!mEncoderLabel.empty()) {
        s->Append(absl::StrFormat(" from %s \"%s\"", ObjectTypeAsString(ObjectType::CommandEncoder),
                                  mEncoderLabel));
    }
}

const std::string& CommandBufferBase::GetEncoderLabel() const {
    return mEncoderLabel;
}

void CommandBufferBase::SetEncoderLabel(std::string encoderLabel) {
    mEncoderLabel = encoderLabel;
}

MaybeError CommandBufferBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());

    DAWN_INVALID_IF(!IsAlive(), "%s cannot be submitted more than once.", this);
    return {};
}

void CommandBufferBase::DestroyImpl() {
    // These metadatas hold raw_ptr to the commands, so they need to be cleared first.
    mIndirectDrawMetadata.clear();
    FreeCommands(&mCommands);
    mResourceUsages = {};
}

const CommandBufferResourceUsage& CommandBufferBase::GetResourceUsages() const {
    return mResourceUsages;
}

const std::vector<IndirectDrawMetadata>& CommandBufferBase::GetIndirectDrawMetadata() {
    return mIndirectDrawMetadata;
}

CommandIterator* CommandBufferBase::GetCommandIteratorForTesting() {
    return &mCommands;
}

bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                   const Extent3D& copySize,
                                   const uint32_t mipLevel,
                                   Aspect aspect) {
    DAWN_ASSERT(HasOneBit(aspect) || aspect == (Aspect::Depth | Aspect::Stencil));

    Extent3D extent = texture->GetMipLevelSingleSubresourcePhysicalSize(mipLevel, aspect);

    switch (texture->GetDimension()) {
        case wgpu::TextureDimension::e1D:
            return extent.width == copySize.width;
        case wgpu::TextureDimension::e2D:
            return extent.width == copySize.width && extent.height == copySize.height;
        case wgpu::TextureDimension::e3D:
            return extent.width == copySize.width && extent.height == copySize.height &&
                   extent.depthOrArrayLayers == copySize.depthOrArrayLayers;
        case wgpu::TextureDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                   const Extent3D& copySize,
                                   const uint32_t mipLevel,
                                   wgpu::TextureAspect textureAspect) {
    auto aspect = SelectFormatAspects(texture->GetFormat(), textureAspect);
    return IsCompleteSubresourceCopiedTo(texture, copySize, mipLevel, aspect);
}

SubresourceRange GetSubresourcesAffectedByCopy(const TextureCopy& copy, const Extent3D& copySize) {
    switch (copy.texture->GetDimension()) {
        case wgpu::TextureDimension::e1D:
            DAWN_ASSERT(copy.origin.z == 0 && copySize.depthOrArrayLayers == 1);
            DAWN_ASSERT(copy.mipLevel == 0);
            return {copy.aspect, {0, 1}, {0, 1}};
        case wgpu::TextureDimension::e2D:
            return {copy.aspect, {copy.origin.z, copySize.depthOrArrayLayers}, {copy.mipLevel, 1}};
        case wgpu::TextureDimension::e3D:
            return {copy.aspect, {0, 1}, {copy.mipLevel, 1}};
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();
    }
    DAWN_UNREACHABLE();
}

void LazyClearRenderPassAttachments(BeginRenderPassCmd* renderPass) {
    for (auto i : renderPass->attachmentState->GetColorAttachmentsMask()) {
        auto& attachmentInfo = renderPass->colorAttachments[i];
        TextureViewBase* view = attachmentInfo.view.Get();
        bool hasResolveTarget = attachmentInfo.resolveTarget != nullptr;

        DAWN_ASSERT(view->GetLayerCount() == 1);
        DAWN_ASSERT(view->GetLevelCount() == 1);
        SubresourceRange range = view->GetSubresourceRange();

        // If the loadOp is Load, but the subresource is not initialized, use Clear instead.
        if (attachmentInfo.loadOp == wgpu::LoadOp::Load &&
            !view->GetTexture()->IsSubresourceContentInitialized(range)) {
            attachmentInfo.loadOp = wgpu::LoadOp::Clear;
            attachmentInfo.clearColor = {0.f, 0.f, 0.f, 0.f};
        }

        if (hasResolveTarget) {
            // We need to set the resolve target to initialized so that it does not get
            // cleared later in the pipeline. The texture will be resolved from the
            // source color attachment, which will be correctly initialized.
            TextureViewBase* resolveView = attachmentInfo.resolveTarget.Get();
            DAWN_ASSERT(resolveView->GetLayerCount() == 1);
            DAWN_ASSERT(resolveView->GetLevelCount() == 1);
            resolveView->GetTexture()->SetIsSubresourceContentInitialized(
                true, resolveView->GetSubresourceRange());
        }

        switch (attachmentInfo.storeOp) {
            case wgpu::StoreOp::Store:
                view->GetTexture()->SetIsSubresourceContentInitialized(true, range);
                break;

            case wgpu::StoreOp::Discard:
                view->GetTexture()->SetIsSubresourceContentInitialized(false, range);
                break;

            case wgpu::StoreOp::Undefined:
                DAWN_UNREACHABLE();
                break;
        }
    }

    if (renderPass->attachmentState->HasDepthStencilAttachment()) {
        auto& attachmentInfo = renderPass->depthStencilAttachment;
        TextureViewBase* view = attachmentInfo.view.Get();
        DAWN_ASSERT(view->GetLayerCount() == 1);
        DAWN_ASSERT(view->GetLevelCount() == 1);
        SubresourceRange range = view->GetSubresourceRange();

        SubresourceRange depthRange = range;
        depthRange.aspects = range.aspects & Aspect::Depth;

        SubresourceRange stencilRange = range;
        stencilRange.aspects = range.aspects & Aspect::Stencil;

        // If the depth stencil texture has not been initialized, we want to use loadop
        // clear to init the contents to 0's
        if (!view->GetTexture()->IsSubresourceContentInitialized(depthRange) &&
            attachmentInfo.depthLoadOp == wgpu::LoadOp::Load) {
            attachmentInfo.clearDepth = 0.0f;
            attachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        }

        if (!view->GetTexture()->IsSubresourceContentInitialized(stencilRange) &&
            attachmentInfo.stencilLoadOp == wgpu::LoadOp::Load) {
            attachmentInfo.clearStencil = 0u;
            attachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
        }

        view->GetTexture()->SetIsSubresourceContentInitialized(
            attachmentInfo.depthStoreOp == wgpu::StoreOp::Store, depthRange);

        view->GetTexture()->SetIsSubresourceContentInitialized(
            attachmentInfo.stencilStoreOp == wgpu::StoreOp::Store, stencilRange);
    }

    if (renderPass->attachmentState->HasPixelLocalStorage()) {
        for (auto& attachmentInfo : renderPass->storageAttachments) {
            TextureViewBase* view = attachmentInfo.storage.Get();

            if (view == nullptr) {
                continue;
            }

            DAWN_ASSERT(view->GetLayerCount() == 1);
            DAWN_ASSERT(view->GetLevelCount() == 1);
            const SubresourceRange& range = view->GetSubresourceRange();

            // If the loadOp is Load, but the subresource is not initialized, use Clear instead.
            if (attachmentInfo.loadOp == wgpu::LoadOp::Load &&
                !view->GetTexture()->IsSubresourceContentInitialized(range)) {
                attachmentInfo.loadOp = wgpu::LoadOp::Clear;
                attachmentInfo.clearColor = {0.f, 0.f, 0.f, 0.f};
            }

            switch (attachmentInfo.storeOp) {
                case wgpu::StoreOp::Store:
                    view->GetTexture()->SetIsSubresourceContentInitialized(true, range);
                    break;

                case wgpu::StoreOp::Discard:
                    view->GetTexture()->SetIsSubresourceContentInitialized(false, range);
                    break;

                case wgpu::StoreOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }
        }
    }
}

bool IsFullBufferOverwrittenInTextureToBufferCopy(const CopyTextureToBufferCmd* copy) {
    DAWN_ASSERT(copy != nullptr);
    return IsFullBufferOverwrittenInTextureToBufferCopy(copy->source, copy->destination,
                                                        copy->copySize);
}

bool IsFullBufferOverwrittenInTextureToBufferCopy(const TextureCopy& source,
                                                  const BufferCopy& destination,
                                                  const Extent3D& copySize) {
    if (destination.offset > 0) {
        // The copy doesn't touch the start of the buffer.
        return false;
    }

    const TextureBase* texture = source.texture.Get();
    const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(source.aspect).block;
    const uint64_t widthInBlocks = copySize.width / blockInfo.width;
    const uint64_t heightInBlocks = copySize.height / blockInfo.height;
    const bool multiSlice = copySize.depthOrArrayLayers > 1;
    const bool multiRow = multiSlice || heightInBlocks > 1;

    if (multiSlice && destination.rowsPerImage > heightInBlocks) {
        // There are gaps between slices that aren't overwritten
        return false;
    }

    const uint64_t copyTextureDataSizePerRow = widthInBlocks * blockInfo.byteSize;
    if (multiRow && destination.bytesPerRow > copyTextureDataSizePerRow) {
        // There are gaps between rows that aren't overwritten
        return false;
    }

    // After the above checks, we're sure the copy has no gaps.
    // Now, compute the total number of bytes written.
    const uint64_t writtenBytes =
        ComputeRequiredBytesInCopy(blockInfo, copySize, destination.bytesPerRow,
                                   destination.rowsPerImage)
            .AcquireSuccess();
    if (!destination.buffer->IsFullBufferRange(destination.offset, writtenBytes)) {
        // The written bytes don't cover the whole buffer.
        return false;
    }

    return true;
}

std::array<float, 4> ConvertToFloatColor(dawn::native::Color color) {
    const std::array<float, 4> outputValue = {
        static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b),
        static_cast<float>(color.a)};
    return outputValue;
}
std::array<int32_t, 4> ConvertToSignedIntegerColor(dawn::native::Color color) {
    const std::array<int32_t, 4> outputValue = {
        static_cast<int32_t>(color.r), static_cast<int32_t>(color.g), static_cast<int32_t>(color.b),
        static_cast<int32_t>(color.a)};
    return outputValue;
}

std::array<uint32_t, 4> ConvertToUnsignedIntegerColor(dawn::native::Color color) {
    const std::array<uint32_t, 4> outputValue = {
        static_cast<uint32_t>(color.r), static_cast<uint32_t>(color.g),
        static_cast<uint32_t>(color.b), static_cast<uint32_t>(color.a)};
    return outputValue;
}

}  // namespace dawn::native
