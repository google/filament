// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/CommandEncoder.h"

#include <string_view>
#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/Math.h"
#include "dawn/common/NonMovable.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/ApplyClearColorValueWithDrawHelper.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BlitBufferToDepthStencil.h"
#include "dawn/native/BlitDepthToDepth.h"
#include "dawn/native/BlitTextureToBuffer.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandBufferStateTracker.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/Device.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/QueryHelper.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPassWorkaroundsHelper.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

namespace {

// Record the subresource range of a attachment used in render pass for checking overlaps.
struct RecordedAttachment {
    const TextureBase* texture;
    uint32_t mipLevel;
    // For 3d color attachment, it's the attachment's depthSlice.
    uint32_t depthOrArrayLayer;

    bool operator==(const RecordedAttachment& other) const {
        return ((other.texture == texture) && (other.mipLevel == mipLevel) &&
                (other.depthOrArrayLayer == depthOrArrayLayer));
    }
};

enum class AttachmentType : uint8_t {
    ColorAttachment,
    ResolveTarget,
    DepthStencilAttachment,
    StorageAttachment
};

std::string_view GetAttachmentTypeStr(AttachmentType type) {
    switch (type) {
        case AttachmentType::ColorAttachment:
            return "color attachment";
        case AttachmentType::ResolveTarget:
            return "resolve target";
        case AttachmentType::DepthStencilAttachment:
            return "depth stencil attachment";
        case AttachmentType::StorageAttachment:
            return "storage attachment";
    }
    DAWN_UNREACHABLE();
}

// The width, height, sample count and subresource range need to be validated between all
// attachments in the render pass. This class records all the states and validate them for each
// attachment.
class RenderPassValidationState final : public NonMovable {
  public:
    explicit RenderPassValidationState(bool unsafeApi) : mUnsafeApi(unsafeApi) {}
    ~RenderPassValidationState() = default;

    // Record the attachment in the render pass if it passes all validations:
    // - the attachment has same with, height and sample count with other attachments
    // - no overlaps with other attachments
    // TODO(dawn:1020): Improve the error messages to include the index information of the
    // attachment in the render pass descriptor.
    MaybeError AddAttachment(const TextureViewBase* attachment,
                             AttachmentType attachmentType,
                             uint32_t depthSlice = wgpu::kDepthSliceUndefined) {
        if (attachment == nullptr) {
            return {};
        }

        DAWN_ASSERT(attachment->GetLevelCount() == 1);
        DAWN_ASSERT(attachment->GetLayerCount() == 1);

        const std::string_view attachmentTypeStr = GetAttachmentTypeStr(attachmentType);

        std::string_view implicitPrefixStr;
        // Not need to validate the implicit sample count for the depth stencil attachment.
        if (mImplicitSampleCount > 1 && attachmentType != AttachmentType::DepthStencilAttachment) {
            DAWN_INVALID_IF(attachment->GetTexture()->GetSampleCount() != 1,
                            "The %s %s sample count (%u) is not 1 when it has implicit "
                            "sample count (%u).",
                            attachmentTypeStr, attachment,
                            attachment->GetTexture()->GetSampleCount(), mImplicitSampleCount);

            implicitPrefixStr = "implicit ";
        }

        Extent3D renderSize = attachment->GetSingleSubresourceVirtualSize();
        Extent3D attachmentValidationSize = renderSize;
        if (attachment->GetTexture()->GetFormat().IsMultiPlanar()) {
            // For multi-planar texture, D3D requires depth stencil buffer size must be equal to the
            // size of the plane 0 for the color attachment texture (`attachmentValidationSize`).
            // Vulkan, Metal and GL requires buffer size equal or bigger than render size. To make
            // all dawn backends work, dawn requires depth attachment's size equal to the
            // `attachmentValidationSize`.
            // Vulkan:
            // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFramebufferCreateInfo.html#VUID-VkFramebufferCreateInfo-flags-04533
            // OpenGLES3.0 (https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf
            // section 4.4.4.2) allows attachments have unequal size.
            attachmentValidationSize =
                attachment->GetTexture()->GetMipLevelSingleSubresourceVirtualSize(
                    attachment->GetBaseMipLevel(), Aspect::Plane0);
        }
        if (HasAttachment()) {
            switch (attachmentType) {
                case AttachmentType::ColorAttachment:
                case AttachmentType::StorageAttachment: {
                    DAWN_INVALID_IF(
                        renderSize.width != mRenderWidth || renderSize.height != mRenderHeight,
                        "The %s %s size (width: %u, height: %u) does not match the size of the "
                        "other attachments (width: %u, height: %u).",
                        attachmentTypeStr, attachment, renderSize.width, renderSize.height,
                        mRenderWidth, mRenderHeight);
                    break;
                }
                case AttachmentType::ResolveTarget: {
                    // TODO(chromium:324422644): support using multi-planar texture as resolve
                    // target.
                    DAWN_INVALID_IF(attachment->GetTexture()->GetFormat().IsMultiPlanar(),
                                    "The resolve target %s used as resolve target is from a "
                                    "multi-planar texture. It is not supported by dawn yet.",
                                    attachment);
                    DAWN_INVALID_IF(
                        renderSize.width != mRenderWidth || renderSize.height != mRenderHeight,
                        "The resolve target %s size (width: %u, height: %u) does not match the "
                        "size of the other attachments (width: %u, height: %u).",
                        attachment, renderSize.width, renderSize.height, mRenderWidth,
                        mRenderHeight);
                    break;
                }
                case AttachmentType::DepthStencilAttachment: {
                    // TODO(chromium:324422644): re-enable this validation code.
                    // This validation code will block skia to chromium autoroll, so disable it
                    // temporarily.
                    const bool disableValidation =
                        mUnsafeApi && mAttachmentValidationWidth != mRenderWidth;
                    DAWN_INVALID_IF(
                        !disableValidation &&
                            (attachmentValidationSize.width != mAttachmentValidationWidth ||
                             attachmentValidationSize.height != mAttachmentValidationHeight),
                        "The depth stencil attachment %s size (width: %u, height: %u) does not "
                        "match the size of the other attachments' base plane (width: %u, height: "
                        "%u).",
                        attachment, attachmentValidationSize.width, attachmentValidationSize.height,
                        mAttachmentValidationWidth, mAttachmentValidationHeight);
                    break;
                }
            }

            // Skip the sampleCount validation for resolve target
            DAWN_INVALID_IF(attachmentType != AttachmentType::ResolveTarget &&
                                attachment->GetTexture()->GetSampleCount() != mSampleCount,
                            "The %s %s %ssample count (%u) does not match the sample count of the "
                            "other attachments (%u).",
                            attachmentTypeStr, attachment, implicitPrefixStr,
                            attachment->GetTexture()->GetSampleCount(), mSampleCount);
        } else {
            mRenderWidth = renderSize.width;
            mRenderHeight = renderSize.height;
            mAttachmentValidationWidth = attachmentValidationSize.width;
            mAttachmentValidationHeight = attachmentValidationSize.height;
            mSampleCount = mImplicitSampleCount > 1 ? mImplicitSampleCount
                                                    : attachment->GetTexture()->GetSampleCount();
            DAWN_ASSERT(mRenderWidth != 0);
            DAWN_ASSERT(mRenderHeight != 0);
            DAWN_ASSERT(mAttachmentValidationWidth != 0);
            DAWN_ASSERT(mAttachmentValidationHeight != 0);
            DAWN_ASSERT(mSampleCount != 0);
        }

        RecordedAttachment record;
        record.texture = attachment->GetTexture();
        record.mipLevel = attachment->GetBaseMipLevel();
        if (attachment->GetDimension() == wgpu::TextureViewDimension::e3D) {
            DAWN_ASSERT(attachment->GetBaseArrayLayer() == 0);
            record.depthOrArrayLayer = depthSlice;
        } else {
            DAWN_ASSERT(depthSlice == wgpu::kDepthSliceUndefined);
            record.depthOrArrayLayer = attachment->GetBaseArrayLayer();
        }

        for (size_t i = 0; i < mRecords.size(); i++) {
            DAWN_INVALID_IF(
                mRecords[i] == record,
                "The %s %s has read-write or write-write conflict with another attachment.",
                attachmentTypeStr, attachment);
        }

        mRecords.push_back(record);

        return {};
    }

    bool HasAttachment() const { return !mRecords.empty(); }

    bool IsValidState() const {
        return ((mRenderWidth > 0) && (mRenderHeight > 0) && (mSampleCount > 0) &&
                (mImplicitSampleCount == 0 || mImplicitSampleCount == mSampleCount));
    }

    uint32_t GetRenderWidth() const { return mRenderWidth; }

    uint32_t GetRenderHeight() const { return mRenderHeight; }

    uint32_t GetSampleCount() const { return mSampleCount; }

    uint32_t GetImplicitSampleCount() const { return mImplicitSampleCount; }

    void SetImplicitSampleCount(uint32_t implicitSampleCount) {
        mImplicitSampleCount = implicitSampleCount;
    }

    bool WillExpandResolveTexture() const { return mWillExpandResolveTexture; }
    void SetWillExpandResolveTexture(bool enabled) { mWillExpandResolveTexture = enabled; }

  private:
    const bool mUnsafeApi;

    // The attachment's width, height and sample count.
    uint32_t mRenderWidth = 0;
    uint32_t mRenderHeight = 0;
    uint32_t mSampleCount = 0;
    // The implicit multisample count used by MSAA render to single sampled.
    uint32_t mImplicitSampleCount = 0;

    uint32_t mAttachmentValidationWidth = 0;
    uint32_t mAttachmentValidationHeight = 0;

    // The records of the attachments that were validated in render pass.
    absl::InlinedVector<RecordedAttachment, kMaxColorAttachments> mRecords;

    bool mWillExpandResolveTexture = false;
};

MaybeError ValidateB2BCopyAlignment(uint64_t dataSize, uint64_t srcOffset, uint64_t dstOffset) {
    // Copy size must be a multiple of 4 bytes on macOS.
    DAWN_INVALID_IF(dataSize % 4 != 0, "Copy size (%u) is not a multiple of 4.", dataSize);

    // SourceOffset and destinationOffset must be multiples of 4 bytes on macOS.
    DAWN_INVALID_IF(srcOffset % 4 != 0 || dstOffset % 4 != 0,
                    "Source offset (%u) or destination offset (%u) is not a multiple of 4 bytes,",
                    srcOffset, dstOffset);

    return {};
}

MaybeError ValidateTextureSampleCountInBufferCopyCommands(const TextureBase* texture) {
    DAWN_INVALID_IF(texture->GetSampleCount() > 1,
                    "%s sample count (%u) is not 1 when copying to or from a buffer.", texture,
                    texture->GetSampleCount());

    return {};
}

MaybeError ValidateLinearTextureCopyOffset(const TexelCopyBufferLayout& layout,
                                           const TexelBlockInfo& blockInfo,
                                           const bool hasDepthOrStencil) {
    if (hasDepthOrStencil) {
        // For depth-stencil texture, buffer offset must be a multiple of 4.
        DAWN_INVALID_IF(layout.offset % 4 != 0,
                        "Offset (%u) is not a multiple of 4 for depth/stencil texture.",
                        layout.offset);
    } else {
        DAWN_INVALID_IF(layout.offset % blockInfo.byteSize != 0,
                        "Offset (%u) is not a multiple of the texel block byte size (%u).",
                        layout.offset, blockInfo.byteSize);
    }
    return {};
}

MaybeError ValidateTextureFormatForTextureToBufferCopyInCompatibilityMode(
    const TextureBase* texture) {
    DAWN_INVALID_IF(texture->GetFormat().isCompressed,
                    "%s with format %s cannot be used as the source in a texture to buffer copy in "
                    "compatibility mode.",
                    texture, texture->GetFormat().format);
    return {};
}

MaybeError ValidateSourceTextureFormatForTextureToTextureCopyInCompatibilityMode(
    const TextureBase* texture) {
    DAWN_INVALID_IF(
        texture->GetFormat().isCompressed,
        "%s with format %s cannot be used as the source in a texture to texture copy in "
        "compatibility mode.",
        texture, texture->GetFormat().format);
    return {};
}

MaybeError ValidateTextureDepthStencilToBufferCopyRestrictions(const TexelCopyTextureInfo& src) {
    Aspect aspectUsed;
    DAWN_TRY_ASSIGN(aspectUsed, SingleAspectUsedByTexelCopyTextureInfo(src));
    if (aspectUsed == Aspect::Depth) {
        switch (src.texture->GetFormat().format) {
            case wgpu::TextureFormat::Depth24Plus:
            case wgpu::TextureFormat::Depth24PlusStencil8:
                return DAWN_VALIDATION_ERROR(
                    "The depth aspect of %s format %s cannot be selected in a texture to "
                    "buffer copy.",
                    src.texture, src.texture->GetFormat().format);
            case wgpu::TextureFormat::Depth32Float:
            case wgpu::TextureFormat::Depth16Unorm:
            case wgpu::TextureFormat::Depth32FloatStencil8:
                break;

            default:
                DAWN_UNREACHABLE();
        }
    }

    return {};
}

MaybeError ValidateAttachmentArrayLayersAndLevelCount(const TextureViewBase* attachment) {
    // Currently we do not support layered rendering.
    DAWN_INVALID_IF(attachment->GetLayerCount() > 1,
                    "The layer count (%u) of %s used as attachment is greater than 1.",
                    attachment->GetLayerCount(), attachment);

    DAWN_INVALID_IF(attachment->GetLevelCount() > 1,
                    "The mip level count (%u) of %s used as attachment is greater than 1.",
                    attachment->GetLevelCount(), attachment);

    return {};
}

MaybeError ValidateResolveTarget(const DeviceBase* device,
                                 const RenderPassColorAttachment& colorAttachment,
                                 UsageValidationMode usageValidationMode) {
    if (colorAttachment.resolveTarget == nullptr) {
        return {};
    }

    const TextureViewBase* resolveTarget = colorAttachment.resolveTarget;
    const TextureViewBase* attachment = colorAttachment.view;
    DAWN_TRY(device->ValidateObject(colorAttachment.resolveTarget));
    DAWN_TRY(ValidateCanUseAs(colorAttachment.resolveTarget, wgpu::TextureUsage::RenderAttachment,
                              usageValidationMode));

    DAWN_INVALID_IF(!attachment->GetTexture()->IsMultisampledTexture(),
                    "Cannot set %s as a resolve target when the color attachment %s has a sample "
                    "count of 1.",
                    resolveTarget, attachment);

    DAWN_INVALID_IF(resolveTarget->GetTexture()->IsMultisampledTexture(),
                    "Cannot use %s as resolve target. Sample count (%u) is greater than 1.",
                    resolveTarget, resolveTarget->GetTexture()->GetSampleCount());

    DAWN_INVALID_IF(resolveTarget->GetDimension() != wgpu::TextureViewDimension::e2D &&
                        resolveTarget->GetDimension() != wgpu::TextureViewDimension::e2DArray,
                    "The dimension (%s) of resolve target %s is not 2D or 2DArray.",
                    resolveTarget->GetDimension(), resolveTarget);

    DAWN_INVALID_IF(resolveTarget->GetLayerCount() > 1,
                    "The resolve target %s array layer count (%u) is not 1.", resolveTarget,
                    resolveTarget->GetLayerCount());

    DAWN_INVALID_IF(resolveTarget->GetLevelCount() > 1,
                    "The resolve target %s mip level count (%u) is not 1.", resolveTarget,
                    resolveTarget->GetLevelCount());

    wgpu::TextureFormat resolveTargetFormat = resolveTarget->GetFormat().format;
    DAWN_INVALID_IF(
        resolveTargetFormat != attachment->GetFormat().format,
        "The resolve target %s format (%s) does not match the color attachment %s format "
        "(%s).",
        resolveTarget, resolveTargetFormat, attachment, attachment->GetFormat().format);
    DAWN_INVALID_IF(
        !resolveTarget->GetFormat().supportsResolveTarget,
        "The resolve target %s format (%s) does not support being used as resolve target.",
        resolveTarget, resolveTargetFormat);

    return {};
}

MaybeError ValidateColorAttachmentDepthSlice(const TextureViewBase* attachment,
                                             uint32_t depthSlice) {
    if (attachment->GetDimension() != wgpu::TextureViewDimension::e3D) {
        DAWN_INVALID_IF(depthSlice != wgpu::kDepthSliceUndefined,
                        "depthSlice (%u) is defined for a non-3D attachment (%s).", depthSlice,
                        attachment);
        return {};
    }

    DAWN_INVALID_IF(depthSlice == wgpu::kDepthSliceUndefined,
                    "depthSlice (%u) for a 3D attachment (%s) is undefined.", depthSlice,
                    attachment);

    const Extent3D& attachmentSize = attachment->GetSingleSubresourceVirtualSize();
    DAWN_INVALID_IF(depthSlice >= attachmentSize.depthOrArrayLayers,
                    "depthSlice (%u) of the attachment (%s) is >= the "
                    "depthOrArrayLayers (%u) of the attachment's subresource at mip level (%u).",
                    depthSlice, attachment, attachmentSize.depthOrArrayLayers,
                    attachment->GetBaseMipLevel());

    return {};
}

MaybeError ValidateColorAttachmentRenderToSingleSampled(
    const DeviceBase* device,
    const RenderPassColorAttachment& colorAttachment,
    const DawnRenderPassColorAttachmentRenderToSingleSampled* msaaRenderToSingleSampledDesc) {
    DAWN_ASSERT(msaaRenderToSingleSampledDesc != nullptr);

    DAWN_INVALID_IF(
        !device->HasFeature(Feature::MSAARenderToSingleSampled),
        "The color attachment %s has implicit sample count while the %s feature is not enabled.",
        colorAttachment.view, ToAPI(Feature::MSAARenderToSingleSampled));

    DAWN_INVALID_IF(!IsValidSampleCount(msaaRenderToSingleSampledDesc->implicitSampleCount) ||
                        msaaRenderToSingleSampledDesc->implicitSampleCount <= 1,
                    "The color attachment %s's implicit sample count (%u) is not supported.",
                    colorAttachment.view, msaaRenderToSingleSampledDesc->implicitSampleCount);

    DAWN_INVALID_IF(!colorAttachment.view->GetTexture()->IsImplicitMSAARenderTextureViewSupported(),
                    "Color attachment %s was not created with %s usage, which is required for "
                    "having implicit sample count (%u).",
                    colorAttachment.view, wgpu::TextureUsage::TextureBinding,
                    msaaRenderToSingleSampledDesc->implicitSampleCount);

    DAWN_INVALID_IF(!colorAttachment.view->GetFormat().supportsResolveTarget,
                    "The color attachment %s format (%s) does not support being used with "
                    "implicit sample count (%u). The format does not support resolve.",
                    colorAttachment.view, colorAttachment.view->GetFormat().format,
                    msaaRenderToSingleSampledDesc->implicitSampleCount);

    DAWN_INVALID_IF(colorAttachment.resolveTarget != nullptr,
                    "Cannot set %s as a resolve target. No resolve target should be specified "
                    "for the color attachment %s with implicit sample count (%u).",
                    colorAttachment.resolveTarget, colorAttachment.view,
                    msaaRenderToSingleSampledDesc->implicitSampleCount);

    return {};
}

MaybeError ValidateExpandResolveTextureLoadOp(const DeviceBase* device,
                                              const RenderPassColorAttachment& colorAttachment,
                                              RenderPassValidationState* validationState) {
    DAWN_INVALID_IF(!device->HasFeature(Feature::DawnLoadResolveTexture),
                    "%s is used while the %s is not enabled.", wgpu::LoadOp::ExpandResolveTexture,
                    ToAPI(Feature::DawnLoadResolveTexture));

    uint32_t textureSampleCount = colorAttachment.view->GetTexture()->GetSampleCount();

    DAWN_INVALID_IF(!IsValidSampleCount(textureSampleCount) || textureSampleCount <= 1,
                    "The color attachment %s's sample count (%u) is not supported by %s.",
                    colorAttachment.view, textureSampleCount, wgpu::LoadOp::ExpandResolveTexture);

    // These should already be validated before entering this function.
    DAWN_ASSERT(colorAttachment.resolveTarget != nullptr &&
                !colorAttachment.resolveTarget->IsError());
    DAWN_ASSERT(colorAttachment.view->GetFormat().supportsResolveTarget);

    DAWN_INVALID_IF(
        (colorAttachment.resolveTarget->GetUsage() & wgpu::TextureUsage::TextureBinding) == 0,
        "Resolve target %s was not created with %s usage, which is required for "
        "%s.",
        colorAttachment.resolveTarget, wgpu::TextureUsage::TextureBinding,
        wgpu::LoadOp::ExpandResolveTexture);

    // TODO(42240662): multiplanar textures are not supported as resolve target.
    // The RenderPassValidationState currently rejects such usage.
    DAWN_ASSERT(!colorAttachment.resolveTarget->GetTexture()->GetFormat().IsMultiPlanar());

    validationState->SetWillExpandResolveTexture(true);

    return {};
}

MaybeError ValidateRenderPassColorAttachment(DeviceBase* device,
                                             const RenderPassColorAttachment& colorAttachment,
                                             UsageValidationMode usageValidationMode,
                                             RenderPassValidationState* validationState) {
    TextureViewBase* attachment = colorAttachment.view;
    if (attachment == nullptr) {
        return {};
    }

    DAWN_TRY(device->ValidateObject(attachment));
    DAWN_TRY(
        ValidateCanUseAs(attachment, wgpu::TextureUsage::RenderAttachment, usageValidationMode));

    UnpackedPtr<RenderPassColorAttachment> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(&colorAttachment));

    const auto* msaaRenderToSingleSampledDesc =
        unpacked.Get<DawnRenderPassColorAttachmentRenderToSingleSampled>();
    if (msaaRenderToSingleSampledDesc) {
        DAWN_TRY(ValidateColorAttachmentRenderToSingleSampled(device, colorAttachment,
                                                              msaaRenderToSingleSampledDesc));
        validationState->SetImplicitSampleCount(msaaRenderToSingleSampledDesc->implicitSampleCount);
        // Note: we don't need to check whether the implicit sample count of different attachments
        // are the same. That already is done by indirectly comparing the sample count in
        // ValidateOrSetColorAttachmentSampleCount.
    }

    // Plane0, Plane1, and Plane2 aspects for multiplanar texture views should be allowed as color
    // attachments.
    Aspect kRenderableAspects = Aspect::Color | Aspect::Plane0 | Aspect::Plane1 | Aspect::Plane2;
    DAWN_INVALID_IF(
        !(attachment->GetAspects() & kRenderableAspects) || !attachment->GetFormat().isRenderable,
        "The color attachment %s format (%s) is not color renderable.", attachment,
        attachment->GetFormat().format);

    DAWN_TRY(ValidateLoadOp(colorAttachment.loadOp));
    DAWN_TRY(ValidateStoreOp(colorAttachment.storeOp));
    DAWN_INVALID_IF(colorAttachment.loadOp == wgpu::LoadOp::Undefined, "loadOp must be set.");
    DAWN_INVALID_IF(colorAttachment.storeOp == wgpu::StoreOp::Undefined, "storeOp must be set.");
    if (attachment->GetUsage() & wgpu::TextureUsage::TransientAttachment) {
        DAWN_INVALID_IF(colorAttachment.loadOp != wgpu::LoadOp::Clear &&
                            colorAttachment.loadOp != wgpu::LoadOp::ExpandResolveTexture,
                        "The color attachment %s has the load op set to %s while its usage (%s) "
                        "has the transient attachment bit set.",
                        attachment, colorAttachment.loadOp, attachment->GetUsage());
        DAWN_INVALID_IF(colorAttachment.storeOp != wgpu::StoreOp::Discard,
                        "The color attachment %s has the store op set to %s while its usage (%s) "
                        "has the transient attachment bit set.",
                        attachment, wgpu::StoreOp::Store, attachment->GetUsage());
    }

    const dawn::native::Color& clearValue = colorAttachment.clearValue;
    if (colorAttachment.loadOp == wgpu::LoadOp::Clear) {
        DAWN_INVALID_IF(std::isnan(clearValue.r) || std::isnan(clearValue.g) ||
                            std::isnan(clearValue.b) || std::isnan(clearValue.a),
                        "Color clear value (%s) contains a NaN.", &clearValue);
    } else if (colorAttachment.loadOp == wgpu::LoadOp::ExpandResolveTexture) {
        DAWN_INVALID_IF(colorAttachment.resolveTarget == nullptr,
                        "%s is used without resolve target.", wgpu::LoadOp::ExpandResolveTexture);
    }

    DAWN_TRY(ValidateColorAttachmentDepthSlice(attachment, colorAttachment.depthSlice));
    DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment));

    DAWN_TRY(validationState->AddAttachment(attachment, AttachmentType::ColorAttachment,
                                            colorAttachment.depthSlice));

    if (validationState->GetImplicitSampleCount() <= 1) {
        // This step is skipped if implicitSampleCount > 1, because in that case, there shoudn't be
        // any explicit resolveTarget specified.
        DAWN_TRY(ValidateResolveTarget(device, colorAttachment, usageValidationMode));

        if (colorAttachment.loadOp == wgpu::LoadOp::ExpandResolveTexture) {
            DAWN_TRY(ValidateExpandResolveTextureLoadOp(device, colorAttachment, validationState));
        }
        // Add resolve target after adding color attachment to make sure there is already a color
        // attachment for the comparation of with and height.
        DAWN_TRY(validationState->AddAttachment(colorAttachment.resolveTarget,
                                                AttachmentType::ResolveTarget));
    }

    return {};
}

MaybeError ValidateRenderPassDepthStencilAttachment(
    DeviceBase* device,
    const RenderPassDepthStencilAttachment* depthStencilAttachment,
    UsageValidationMode usageValidationMode,
    RenderPassValidationState* validationState) {
    DAWN_ASSERT(depthStencilAttachment != nullptr);
    UnpackedPtr<RenderPassDepthStencilAttachment> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(depthStencilAttachment));

    TextureViewBase* attachment = unpacked->view;
    DAWN_TRY(device->ValidateObject(attachment));
    DAWN_TRY(
        ValidateCanUseAs(attachment, wgpu::TextureUsage::RenderAttachment, usageValidationMode));

    // DS attachments must encompass all aspects of the texture, so we first check that this is
    // true, which means that in the rest of the function we can assume that the view's format is
    // the same as the texture's format.
    const Format& format = attachment->GetTexture()->GetFormat();
    DAWN_INVALID_IF(
        attachment->GetAspects() != format.aspects,
        "The depth stencil attachment %s must encompass all aspects of it's texture's format (%s).",
        attachment, format.format);
    DAWN_ASSERT(attachment->GetFormat().format == format.format);

    DAWN_INVALID_IF(!format.HasDepthOrStencil(),
                    "The depth stencil attachment %s format (%s) is not a depth stencil format.",
                    attachment, format.format);

    DAWN_INVALID_IF(!format.isRenderable,
                    "The depth stencil attachment %s format (%s) is not renderable.", attachment,
                    format.format);

    // Read only, or depth doesn't exist.
    if (unpacked->depthReadOnly || !IsSubset(Aspect::Depth, attachment->GetAspects())) {
        DAWN_INVALID_IF(unpacked->depthLoadOp != wgpu::LoadOp::Undefined ||
                            unpacked->depthStoreOp != wgpu::StoreOp::Undefined,
                        "Both depthLoadOp (%s) and depthStoreOp (%s) must not be set if the "
                        "attachment (%s) has no depth aspect or depthReadOnly (%u) is true.",
                        unpacked->depthLoadOp, unpacked->depthStoreOp, attachment,
                        unpacked->depthReadOnly);
    } else {
        DAWN_TRY(ValidateLoadOp(unpacked->depthLoadOp));
        DAWN_TRY(ValidateStoreOp(unpacked->depthStoreOp));
        DAWN_INVALID_IF(unpacked->depthLoadOp == wgpu::LoadOp::Undefined ||
                            unpacked->depthStoreOp == wgpu::StoreOp::Undefined,
                        "Both depthLoadOp (%s) and depthStoreOp (%s) must be set if the attachment "
                        "(%s) has a depth aspect or depthReadOnly (%u) is false.",
                        unpacked->depthLoadOp, unpacked->depthStoreOp, attachment,
                        unpacked->depthReadOnly);
    }

    DAWN_INVALID_IF(unpacked->depthLoadOp == wgpu::LoadOp::ExpandResolveTexture ||
                        unpacked->stencilLoadOp == wgpu::LoadOp::ExpandResolveTexture,
                    "%s is not supported on depth/stencil attachment",
                    wgpu::LoadOp::ExpandResolveTexture);

    // Read only, or stencil doesn't exist.
    if (unpacked->stencilReadOnly || !IsSubset(Aspect::Stencil, attachment->GetAspects())) {
        DAWN_INVALID_IF(unpacked->stencilLoadOp != wgpu::LoadOp::Undefined ||
                            unpacked->stencilStoreOp != wgpu::StoreOp::Undefined,
                        "Both stencilLoadOp (%s) and stencilStoreOp (%s) must not be set if the "
                        "attachment (%s) has no stencil aspect or stencilReadOnly (%u) is true.",
                        unpacked->stencilLoadOp, unpacked->stencilStoreOp, attachment,
                        unpacked->stencilReadOnly);
    } else {
        DAWN_TRY(ValidateLoadOp(unpacked->stencilLoadOp));
        DAWN_TRY(ValidateStoreOp(unpacked->stencilStoreOp));
        DAWN_INVALID_IF(unpacked->stencilLoadOp == wgpu::LoadOp::Undefined ||
                            unpacked->stencilStoreOp == wgpu::StoreOp::Undefined,
                        "Both stencilLoadOp (%s) and stencilStoreOp (%s) must be set if the "
                        "attachment (%s) has a stencil aspect or stencilReadOnly (%u) is false.",
                        unpacked->stencilLoadOp, unpacked->stencilStoreOp, attachment,
                        unpacked->stencilReadOnly);
    }

    if (unpacked->depthLoadOp == wgpu::LoadOp::Clear &&
        IsSubset(Aspect::Depth, attachment->GetAspects())) {
        DAWN_INVALID_IF(
            std::isnan(unpacked->depthClearValue),
            "depthClearValue (%f) must be set and must not be a NaN value if the attachment "
            "(%s) has a depth aspect and depthLoadOp is clear.",
            unpacked->depthClearValue, attachment);
        DAWN_INVALID_IF(unpacked->depthClearValue < 0.0f || unpacked->depthClearValue > 1.0f,
                        "depthClearValue (%f) must be between 0.0 and 1.0 if the attachment (%s) "
                        "has a depth aspect and depthLoadOp is clear.",
                        unpacked->depthClearValue, attachment);
    }

    DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment));

    DAWN_TRY(validationState->AddAttachment(attachment, AttachmentType::DepthStencilAttachment));

    return {};
}

MaybeError ValidateRenderPassPLS(DeviceBase* device,
                                 const RenderPassPixelLocalStorage* pls,
                                 UsageValidationMode usageValidationMode,
                                 RenderPassValidationState* validationState) {
    absl::InlinedVector<StorageAttachmentInfoForValidation, 4> attachments;

    for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
        const RenderPassStorageAttachment& attachment = pls->storageAttachments[i];

        // Validate the attachment can be used as a storage attachment.
        DAWN_TRY(device->ValidateObject(attachment.storage));
        DAWN_TRY(ValidateCanUseAs(attachment.storage, wgpu::TextureUsage::StorageAttachment,
                                  usageValidationMode));
        DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment.storage));

        // Validate the load/storeOp and the clearValue.
        DAWN_TRY(ValidateLoadOp(attachment.loadOp));
        DAWN_TRY(ValidateStoreOp(attachment.storeOp));
        DAWN_INVALID_IF(attachment.loadOp == wgpu::LoadOp::Undefined,
                        "storageAttachments[%i].loadOp must be set.", i);
        DAWN_INVALID_IF(attachment.storeOp == wgpu::StoreOp::Undefined,
                        "storageAttachments[%i].storeOp must be set.", i);

        const dawn::native::Color& clearValue = attachment.clearValue;
        if (attachment.loadOp == wgpu::LoadOp::Clear) {
            DAWN_INVALID_IF(std::isnan(clearValue.r) || std::isnan(clearValue.g) ||
                                std::isnan(clearValue.b) || std::isnan(clearValue.a),
                            "storageAttachments[%i].clearValue (%s) contains a NaN.", i,
                            &clearValue);
        }

        DAWN_TRY(
            validationState->AddAttachment(attachment.storage, AttachmentType::StorageAttachment));

        attachments.push_back({attachment.offset, attachment.storage->GetFormat().format});
    }

    return ValidatePLSInfo(device, pls->totalPixelLocalStorageSize,
                           {attachments.data(), attachments.size()});
}

ResultOrError<UnpackedPtr<RenderPassDescriptor>> ValidateRenderPassDescriptor(
    DeviceBase* device,
    const RenderPassDescriptor* rawDescriptor,
    UsageValidationMode usageValidationMode,
    RenderPassValidationState* validationState) {
    UnpackedPtr<RenderPassDescriptor> descriptor;
    DAWN_TRY_ASSIGN_CONTEXT(descriptor, ValidateAndUnpack(rawDescriptor),
                            "validating chained structs.");

    uint32_t maxColorAttachments = device->GetLimits().v1.maxColorAttachments;
    DAWN_INVALID_IF(
        descriptor->colorAttachmentCount > maxColorAttachments,
        "Color attachment count (%u) exceeds the maximum number of color attachments (%u).%s",
        descriptor->colorAttachmentCount, maxColorAttachments,
        DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter(), maxColorAttachments,
                                    descriptor->colorAttachmentCount));

    auto colorAttachments = ityp::SpanFromUntyped<ColorAttachmentIndex>(
        descriptor->colorAttachments, descriptor->colorAttachmentCount);
    ColorAttachmentFormats colorAttachmentFormats;
    for (auto [i, attachment] : Enumerate(colorAttachments)) {
        DAWN_TRY_CONTEXT(ValidateRenderPassColorAttachment(device, attachment, usageValidationMode,
                                                           validationState),
                         "validating colorAttachments[%u].", i);
        if (attachment.view) {
            colorAttachmentFormats.push_back(&attachment.view->GetFormat());
        }
    }
    DAWN_TRY_CONTEXT(ValidateColorAttachmentBytesPerSample(device, colorAttachmentFormats),
                     "validating color attachment bytes per sample.");

    if (descriptor->depthStencilAttachment != nullptr) {
        DAWN_TRY_CONTEXT(
            ValidateRenderPassDepthStencilAttachment(device, descriptor->depthStencilAttachment,
                                                     usageValidationMode, validationState),
            "validating depthStencilAttachment.");
    }

    if (descriptor->occlusionQuerySet != nullptr) {
        DAWN_TRY(device->ValidateObject(descriptor->occlusionQuerySet));

        DAWN_INVALID_IF(descriptor->occlusionQuerySet->GetQueryType() != wgpu::QueryType::Occlusion,
                        "The occlusionQuerySet %s type (%s) is not %s.",
                        descriptor->occlusionQuerySet,
                        descriptor->occlusionQuerySet->GetQueryType(), wgpu::QueryType::Occlusion);
    }

    if (descriptor->timestampWrites != nullptr) {
        DAWN_TRY_CONTEXT(ValidatePassTimestampWrites(device, descriptor->timestampWrites),
                         "validating timestampWrites.");
    }

    // Validation for any pixel local storage.
    auto pls = descriptor.Get<RenderPassPixelLocalStorage>();
    if (pls != nullptr) {
        DAWN_TRY(ValidateRenderPassPLS(device, pls, usageValidationMode, validationState));
    }

    DAWN_INVALID_IF(!validationState->HasAttachment(), "Render pass has no attachments.");

    if (validationState->GetImplicitSampleCount() > 1) {
        // TODO(dawn:1710): support multiple attachments.
        DAWN_INVALID_IF(
            descriptor->colorAttachmentCount != 1,
            "colorAttachmentCount (%u) is not supported when the render pass has implicit sample "
            "count (%u). (Currently) colorAttachmentCount = 1 is supported.",
            descriptor->colorAttachmentCount, validationState->GetImplicitSampleCount());
        // TODO(dawn:1704): Consider supporting MSAARenderToSingleSampled + PLS
        DAWN_INVALID_IF(
            pls != nullptr,
            "For now pixel local storage is invalid to use with MSAARenderToSingleSampled.");
    }

    if (validationState->WillExpandResolveTexture()) {
        // TODO(dawn:1704): Consider supporting ExpandResolveTexture + PLS
        DAWN_INVALID_IF(pls != nullptr, "For now pixel local storage is invalid to use with %s.",
                        wgpu::LoadOp::ExpandResolveTexture);
    }

    if (const auto* rect = descriptor.Get<RenderPassDescriptorExpandResolveRect>()) {
        DAWN_INVALID_IF(!device->HasFeature(Feature::DawnPartialLoadResolveTexture),
                        "RenderPassDescriptorExpandResolveRect can't be used without %s.",
                        ToAPI(Feature::DawnPartialLoadResolveTexture));
        DAWN_INVALID_IF(
            !validationState->WillExpandResolveTexture(),
            "ExpandResolveRect is invalid to use without wgpu::LoadOp::ExpandResolveTexture.");

        DAWN_INVALID_IF(
            static_cast<uint64_t>(rect->x) + static_cast<uint64_t>(rect->width) >
                validationState->GetRenderWidth(),
            "The x (%u) and width (%u) of ExpandResolveRect is out of the render area width(% u).",
            rect->x, rect->width, validationState->GetRenderWidth());
        DAWN_INVALID_IF(static_cast<uint64_t>(rect->y) + static_cast<uint64_t>(rect->height) >
                            validationState->GetRenderHeight(),
                        "The y (%u) and height (%u) of ExpandResolveRect is out of the render area "
                        "height(% u).",
                        rect->y, rect->height, validationState->GetRenderHeight());
    }

    return descriptor;
}

MaybeError ValidateComputePassDescriptor(const DeviceBase* device,
                                         const ComputePassDescriptor* descriptor) {
    if (descriptor == nullptr) {
        return {};
    }

    if (descriptor->timestampWrites != nullptr) {
        DAWN_TRY_CONTEXT(ValidatePassTimestampWrites(device, descriptor->timestampWrites),
                         "validating timestampWrites.");
    }

    return {};
}

MaybeError ValidateQuerySetResolve(const QuerySetBase* querySet,
                                   uint32_t firstQuery,
                                   uint32_t queryCount,
                                   const BufferBase* destination,
                                   uint64_t destinationOffset) {
    DAWN_INVALID_IF(firstQuery >= querySet->GetQueryCount(),
                    "First query (%u) exceeds the number of queries (%u) in %s.", firstQuery,
                    querySet->GetQueryCount(), querySet);

    DAWN_INVALID_IF(
        queryCount > querySet->GetQueryCount() - firstQuery,
        "The query range (firstQuery: %u, queryCount: %u) exceeds the number of queries "
        "(%u) in %s.",
        firstQuery, queryCount, querySet->GetQueryCount(), querySet);

    DAWN_INVALID_IF(destinationOffset % kQueryResolveAlignment != 0,
                    "The destination buffer %s offset (%u) is not a multiple of %u.", destination,
                    destinationOffset, kQueryResolveAlignment);

    uint64_t bufferSize = destination->GetSize();
    // The destination buffer must have enough storage, from destination offset, to contain
    // the result of resolved queries
    bool fitsInBuffer =
        destinationOffset <= bufferSize &&
        (static_cast<uint64_t>(queryCount) * sizeof(uint64_t) <= (bufferSize - destinationOffset));
    DAWN_INVALID_IF(
        !fitsInBuffer,
        "The resolved %s data size (%u) would not fit in %s with size %u at the offset %u.",
        querySet, static_cast<uint64_t>(queryCount) * sizeof(uint64_t), destination, bufferSize,
        destinationOffset);

    return {};
}

MaybeError EncodeTimestampsToNanosecondsConversion(CommandEncoder* encoder,
                                                   QuerySetBase* querySet,
                                                   uint32_t firstQuery,
                                                   uint32_t queryCount,
                                                   BufferBase* destination,
                                                   uint64_t destinationOffset) {
    DeviceBase* device = encoder->GetDevice();

    // The availability got from query set is a reference to vector<bool>, need to covert
    // bool to uint32_t due to a user input in pipeline must not contain a bool type in
    // WGSL.
    std::vector<uint32_t> availability{querySet->GetQueryAvailability().begin(),
                                       querySet->GetQueryAvailability().end()};

    // Timestamp availability storage buffer
    BufferDescriptor availabilityDesc = {};
    availabilityDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    availabilityDesc.size = querySet->GetQueryCount() * sizeof(uint32_t);
    Ref<BufferBase> availabilityBuffer;
    DAWN_TRY_ASSIGN(availabilityBuffer, device->CreateBuffer(&availabilityDesc));

    DAWN_TRY(device->GetQueue()->WriteBuffer(availabilityBuffer.Get(), 0, availability.data(),
                                             availability.size() * sizeof(uint32_t)));

    const uint32_t quantization_mask = (device->IsToggleEnabled(Toggle::TimestampQuantization))
                                           ? kTimestampQuantizationMask
                                           : 0xFFFFFFFF;

    // Timestamp params uniform buffer
    TimestampParams params(firstQuery, queryCount, static_cast<uint32_t>(destinationOffset),
                           quantization_mask, device->GetTimestampPeriodInNS());

    BufferDescriptor parmsDesc = {};
    parmsDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    parmsDesc.size = sizeof(params);
    Ref<BufferBase> paramsBuffer;
    DAWN_TRY_ASSIGN(paramsBuffer, device->CreateBuffer(&parmsDesc));

    DAWN_TRY(device->GetQueue()->WriteBuffer(paramsBuffer.Get(), 0, &params, sizeof(params)));

    // In the internal shader to convert timestamps to nanoseconds, we can ensure no uninitialized
    // data will be read and the full buffer range will be filled with valid data.
    if (!destination->IsInitialized() &&
        destination->IsFullBufferRange(firstQuery, sizeof(uint64_t) * queryCount)) {
        destination->SetInitialized(true);
    }

    return EncodeConvertTimestampsToNanoseconds(encoder, destination, availabilityBuffer.Get(),
                                                paramsBuffer.Get());
}

bool ShouldUseTextureToBufferBlit(const DeviceBase* device,
                                  const Format& format,
                                  const Aspect& aspect) {
    // Snorm
    if (format.IsSnorm() && device->IsToggleEnabled(Toggle::UseBlitForSnormTextureToBufferCopy)) {
        return true;
    }
    // BGRA8Unorm
    if (format.format == wgpu::TextureFormat::BGRA8Unorm &&
        device->IsToggleEnabled(Toggle::UseBlitForBGRA8UnormTextureToBufferCopy)) {
        return true;
    }
    // RGB9E5Ufloat
    if (format.format == wgpu::TextureFormat::RGB9E5Ufloat &&
        device->IsToggleEnabled(Toggle::UseBlitForRGB9E5UfloatTextureCopy)) {
        return true;
    }
    // RG11B10Ufloat
    if (format.format == wgpu::TextureFormat::RG11B10Ufloat &&
        device->IsToggleEnabled(Toggle::UseBlitForRG11B10UfloatTextureCopy)) {
        return true;
    }
    // float16
    if ((format.format == wgpu::TextureFormat::R16Float ||
         format.format == wgpu::TextureFormat::RG16Float ||
         format.format == wgpu::TextureFormat::RGBA16Float) &&
        device->IsToggleEnabled(Toggle::UseBlitForFloat16TextureCopy)) {
        return true;
    }
    // float32
    if ((format.format == wgpu::TextureFormat::R32Float ||
         format.format == wgpu::TextureFormat::RG32Float ||
         format.format == wgpu::TextureFormat::RGBA32Float) &&
        device->IsToggleEnabled(Toggle::UseBlitForFloat32TextureCopy)) {
        return true;
    }
    // Depth
    if (aspect == Aspect::Depth &&
        ((format.format == wgpu::TextureFormat::Depth16Unorm &&
          device->IsToggleEnabled(Toggle::UseBlitForDepth16UnormTextureToBufferCopy)) ||
         (format.format == wgpu::TextureFormat::Depth32Float &&
          device->IsToggleEnabled(Toggle::UseBlitForDepth32FloatTextureToBufferCopy)))) {
        return true;
    }
    // Stencil
    if (aspect == Aspect::Stencil &&
        device->IsToggleEnabled(Toggle::UseBlitForStencilTextureToBufferCopy)) {
        return true;
    }

    if (device->IsToggleEnabled(Toggle::UseBlitForT2B) &&
        IsFormatSupportedByTextureToBufferBlit(format.format)) {
        return true;
    }

    return false;
}

bool ShouldUseT2B2TForT2T(const DeviceBase* device,
                          const Format& srcFormat,
                          const Format& dstFormat) {
    // RGB9E5Ufloat
    if (srcFormat.baseFormat == wgpu::TextureFormat::RGB9E5Ufloat &&
        device->IsToggleEnabled(Toggle::UseBlitForRGB9E5UfloatTextureCopy)) {
        return true;
    }
    // sRGB <-> non-sRGB
    if (srcFormat.format != dstFormat.format && srcFormat.baseFormat == dstFormat.baseFormat &&
        device->IsToggleEnabled(Toggle::UseT2B2TForSRGBTextureCopy)) {
        return true;
    }
    // Snorm
    if (srcFormat.IsSnorm() &&
        device->IsToggleEnabled(Toggle::UseBlitForSnormTextureToBufferCopy)) {
        return true;
    }
    return false;
}

}  // namespace

Color ClampClearColorValueToLegalRange(const Color& originalColor, const Format& format) {
    const AspectInfo& aspectInfo = format.GetAspectInfo(Aspect::Color);
    double minValue = 0;
    double maxValue = 0;
    switch (aspectInfo.baseType) {
        case TextureComponentType::Float: {
            return originalColor;
        }
        case TextureComponentType::Sint: {
            const uint32_t bitsPerComponent =
                (aspectInfo.block.byteSize * 8 / format.componentCount);
            maxValue =
                static_cast<double>((static_cast<uint64_t>(1) << (bitsPerComponent - 1)) - 1);
            minValue = -static_cast<double>(static_cast<uint64_t>(1) << (bitsPerComponent - 1));
            break;
        }
        case TextureComponentType::Uint: {
            const uint32_t bitsPerComponent =
                (aspectInfo.block.byteSize * 8 / format.componentCount);
            maxValue = static_cast<double>((static_cast<uint64_t>(1) << bitsPerComponent) - 1);
            break;
        }
    }

    return {std::clamp(originalColor.r, minValue, maxValue),
            std::clamp(originalColor.g, minValue, maxValue),
            std::clamp(originalColor.b, minValue, maxValue),
            std::clamp(originalColor.a, minValue, maxValue)};
}

ResultOrError<UnpackedPtr<CommandEncoderDescriptor>> ValidateCommandEncoderDescriptor(
    const DeviceBase* device,
    const CommandEncoderDescriptor* descriptor) {
    UnpackedPtr<CommandEncoderDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    const auto* internalUsageDesc = unpacked.Get<DawnEncoderInternalUsageDescriptor>();
    DAWN_INVALID_IF(internalUsageDesc != nullptr &&
                        !device->APIHasFeature(wgpu::FeatureName::DawnInternalUsages),
                    "%s is not available.", wgpu::FeatureName::DawnInternalUsages);
    return unpacked;
}

// static
Ref<CommandEncoder> CommandEncoder::Create(
    DeviceBase* device,
    const UnpackedPtr<CommandEncoderDescriptor>& descriptor) {
    return AcquireRef(new CommandEncoder(device, descriptor));
}

// static
Ref<CommandEncoder> CommandEncoder::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new CommandEncoder(device, ObjectBase::kError, label));
}

CommandEncoder::CommandEncoder(DeviceBase* device,
                               const UnpackedPtr<CommandEncoderDescriptor>& descriptor)
    : ApiObjectBase(device, descriptor->label), mEncodingContext(device, this) {
    GetObjectTrackingList()->Track(this);

    auto* internalUsageDesc = descriptor.Get<DawnEncoderInternalUsageDescriptor>();
    if (internalUsageDesc != nullptr && internalUsageDesc->useInternalUsages) {
        mUsageValidationMode = UsageValidationMode::Internal;
    } else {
        mUsageValidationMode = UsageValidationMode::Default;
    }
}

CommandEncoder::CommandEncoder(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label),
      mEncodingContext(device, tag),
      mUsageValidationMode(UsageValidationMode::Default) {}

ObjectType CommandEncoder::GetType() const {
    return ObjectType::CommandEncoder;
}

void CommandEncoder::DestroyImpl() {
    mEncodingContext.Destroy();
}

CommandBufferResourceUsage CommandEncoder::AcquireResourceUsages() {
    return CommandBufferResourceUsage{
        mEncodingContext.AcquireRenderPassUsages(), mEncodingContext.AcquireComputePassUsages(),
        std::move(mTopLevelBuffers), std::move(mTopLevelTextures), std::move(mUsedQuerySets)};
}

CommandIterator CommandEncoder::AcquireCommands() {
    return mEncodingContext.AcquireCommands();
}

void CommandEncoder::TrackUsedQuerySet(QuerySetBase* querySet) {
    mUsedQuerySets.insert(querySet);
}

void CommandEncoder::TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex) {
    DAWN_ASSERT(querySet != nullptr);

    if (GetDevice()->IsValidationEnabled()) {
        TrackUsedQuerySet(querySet);
    }

    // Set the query at queryIndex to available for resolving in query set.
    querySet->SetQueryAvailability(queryIndex, true);
}

std::vector<IndirectDrawMetadata> CommandEncoder::AcquireIndirectDrawMetadata() {
    return mEncodingContext.AcquireIndirectDrawMetadata();
}

// Implementation of the API's command recording methods

ComputePassEncoder* CommandEncoder::APIBeginComputePass(const ComputePassDescriptor* descriptor) {
    // This function will create new object, need to lock the Device.
    auto deviceLock(GetDevice()->GetScopedLock());

    return ReturnToAPI(BeginComputePass(descriptor));
}

Ref<ComputePassEncoder> CommandEncoder::BeginComputePass(const ComputePassDescriptor* descriptor) {
    DeviceBase* device = GetDevice();
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    bool success = mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY(ValidateComputePassDescriptor(device, descriptor));

            BeginComputePassCmd* cmd =
                allocator->Allocate<BeginComputePassCmd>(Command::BeginComputePass);

            if (descriptor == nullptr) {
                return {};
            }

            if (!descriptor->label.IsUndefined()) {
                cmd->label = std::string(descriptor->label);
            }

            if (descriptor->timestampWrites != nullptr) {
                QuerySetBase* querySet = descriptor->timestampWrites->querySet;
                uint32_t beginningOfPassWriteIndex =
                    descriptor->timestampWrites->beginningOfPassWriteIndex;
                uint32_t endOfPassWriteIndex = descriptor->timestampWrites->endOfPassWriteIndex;

                cmd->timestampWrites.querySet = querySet;
                cmd->timestampWrites.beginningOfPassWriteIndex = beginningOfPassWriteIndex;
                cmd->timestampWrites.endOfPassWriteIndex = endOfPassWriteIndex;
                if (beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, beginningOfPassWriteIndex);
                }
                if (endOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, endOfPassWriteIndex);
                }
            }
            return {};
        },
        "encoding %s.BeginComputePass(%s).", this, descriptor);

    if (success) {
        const ComputePassDescriptor defaultDescriptor = {};
        if (descriptor == nullptr) {
            descriptor = &defaultDescriptor;
        }

        Ref<ComputePassEncoder> passEncoder =
            ComputePassEncoder::Create(device, descriptor, this, &mEncodingContext);
        mEncodingContext.EnterPass(passEncoder.Get());
        return passEncoder;
    }

    return ComputePassEncoder::MakeError(device, this, &mEncodingContext,
                                         descriptor ? descriptor->label : nullptr);
}

RenderPassEncoder* CommandEncoder::APIBeginRenderPass(const RenderPassDescriptor* descriptor) {
    // This function will create new object, need to lock the Device.
    auto deviceLock(GetDevice()->GetScopedLock());

    return ReturnToAPI(BeginRenderPass(descriptor));
}

Ref<RenderPassEncoder> CommandEncoder::BeginRenderPass(const RenderPassDescriptor* rawDescriptor) {
    DeviceBase* device = GetDevice();
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    RenderPassResourceUsageTracker usageTracker;

    bool depthReadOnly = false;
    bool stencilReadOnly = false;
    Ref<AttachmentState> attachmentState;

    RenderPassValidationState validationState(
        GetDevice()->IsToggleEnabled(Toggle::AllowUnsafeAPIs));

    // Lazy make error function to be called if we error and need to return an error encoder.
    auto MakeError = [&]() {
        return RenderPassEncoder::MakeError(device, this, &mEncodingContext,
                                            rawDescriptor ? rawDescriptor->label : nullptr);
    };

    UnpackedPtr<RenderPassDescriptor> descriptor;
    ClearWithDrawHelper clearWithDrawHelper;
    RenderPassWorkaroundsHelper renderpassWorkaroundsHelper;

    RenderPassEncoder::EndCallback passEndCallback = nullptr;

    bool success = mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY_ASSIGN(descriptor,
                            ValidateRenderPassDescriptor(device, rawDescriptor,
                                                         mUsageValidationMode, &validationState));

            DAWN_ASSERT(validationState.IsValidState());

            DAWN_TRY(clearWithDrawHelper.Initialize(this, descriptor));
            DAWN_TRY(renderpassWorkaroundsHelper.Initialize(this, descriptor));

            mEncodingContext.WillBeginRenderPass();
            BeginRenderPassCmd* cmd =
                allocator->Allocate<BeginRenderPassCmd>(Command::BeginRenderPass);

            if (!descriptor->label.IsUndefined()) {
                cmd->label = std::string(descriptor->label);
            }

            cmd->attachmentState = device->GetOrCreateAttachmentState(descriptor);
            attachmentState = cmd->attachmentState;

            auto descColorAttachments = ityp::SpanFromUntyped<ColorAttachmentIndex>(
                descriptor->colorAttachments, descriptor->colorAttachmentCount);
            for (auto i : IterateBitSet(cmd->attachmentState->GetColorAttachmentsMask())) {
                auto& descColorAttachment = descColorAttachments[i];
                auto& cmdColorAttachment = cmd->colorAttachments[i];

                TextureViewBase* colorTarget;
                TextureViewBase* resolveTarget;

                colorTarget = descColorAttachment.view;
                resolveTarget = descColorAttachment.resolveTarget;

                cmdColorAttachment.view = colorTarget;
                // Explicitly set depthSlice to 0 if it's undefined. The
                // wgpu::kDepthSliceUndefined is defined to differentiate between `undefined`
                // and 0 for depthSlice, but we use it as 0 for 2d attachments in backends.
                cmdColorAttachment.depthSlice =
                    descColorAttachment.depthSlice == wgpu::kDepthSliceUndefined
                        ? 0
                        : descColorAttachment.depthSlice;
                cmdColorAttachment.loadOp = descColorAttachment.loadOp;
                cmdColorAttachment.storeOp = descColorAttachment.storeOp;

                cmdColorAttachment.resolveTarget = resolveTarget;
                cmdColorAttachment.clearColor = ClampClearColorValueToLegalRange(
                    descColorAttachment.clearValue, colorTarget->GetFormat());

                usageTracker.TextureViewUsedAs(colorTarget, wgpu::TextureUsage::RenderAttachment);

                if (resolveTarget != nullptr) {
                    usageTracker.TextureViewUsedAs(resolveTarget,
                                                   wgpu::TextureUsage::RenderAttachment);
                }
            }

            if (cmd->attachmentState->HasDepthStencilAttachment()) {
                TextureViewBase* view = descriptor->depthStencilAttachment->view;
                TextureBase* attachment = view->GetTexture();
                cmd->depthStencilAttachment.view = view;
                // Range that will be modified per aspect to track the usage.
                SubresourceRange usageRange = view->GetSubresourceRange();

                switch (descriptor->depthStencilAttachment->depthLoadOp) {
                    case wgpu::LoadOp::Clear:
                        cmd->depthStencilAttachment.clearDepth =
                            descriptor->depthStencilAttachment->depthClearValue;
                        break;
                    case wgpu::LoadOp::Load:
                    case wgpu::LoadOp::Undefined:
                        // Set depthClearValue to 0 if it is the load op is not clear.
                        // The default value NaN may be invalid in the backend.
                        cmd->depthStencilAttachment.clearDepth = 0.f;
                        break;
                    case wgpu::LoadOp::ExpandResolveTexture:
                        DAWN_UNREACHABLE();
                        break;
                }

                // GPURenderPassDepthStencilAttachment.stencilClearValue will be converted to
                // the type of the stencil aspect of view by taking the same number of LSBs as
                // the number of bits in the stencil aspect of one texel block of view.
                DAWN_ASSERT(!(view->GetFormat().aspects & Aspect::Stencil) ||
                            view->GetFormat().GetAspectInfo(Aspect::Stencil).block.byteSize == 1u);
                cmd->depthStencilAttachment.clearStencil =
                    descriptor->depthStencilAttachment->stencilClearValue & 0xFF;

                // Depth aspect:
                //  - Copy parameters for the aspect, reyifing the values when it is not present or
                //  readonly.
                //  - Export depthReadOnly to the outside of the depth-stencil attachment handling.
                //  - Track the usage of this aspect.
                depthReadOnly = descriptor->depthStencilAttachment->depthReadOnly;

                cmd->depthStencilAttachment.depthReadOnly = false;
                cmd->depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Load;
                cmd->depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
                if (attachment->GetFormat().HasDepth()) {
                    cmd->depthStencilAttachment.depthReadOnly = depthReadOnly;
                    if (!depthReadOnly) {
                        cmd->depthStencilAttachment.depthLoadOp =
                            descriptor->depthStencilAttachment->depthLoadOp;
                        cmd->depthStencilAttachment.depthStoreOp =
                            descriptor->depthStencilAttachment->depthStoreOp;
                    }

                    usageRange.aspects = Aspect::Depth;
                    usageTracker.TextureRangeUsedAs(attachment, usageRange,
                                                    depthReadOnly
                                                        ? kReadOnlyRenderAttachment
                                                        : wgpu::TextureUsage::RenderAttachment);
                }

                // Stencil aspect:
                //  - Copy parameters for the aspect, reyifing the values when it is not present or
                //  readonly.
                //  - Export stencilReadOnly to the outside of the depth-stencil attachment
                //  handling.
                //  - Track the usage of this aspect.
                stencilReadOnly = descriptor->depthStencilAttachment->stencilReadOnly;

                cmd->depthStencilAttachment.stencilReadOnly = false;
                cmd->depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Load;
                cmd->depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
                if (attachment->GetFormat().HasStencil()) {
                    cmd->depthStencilAttachment.stencilReadOnly = stencilReadOnly;
                    if (!stencilReadOnly) {
                        cmd->depthStencilAttachment.stencilLoadOp =
                            descriptor->depthStencilAttachment->stencilLoadOp;
                        cmd->depthStencilAttachment.stencilStoreOp =
                            descriptor->depthStencilAttachment->stencilStoreOp;
                    }

                    usageRange.aspects = Aspect::Stencil;
                    usageTracker.TextureRangeUsedAs(attachment, usageRange,
                                                    stencilReadOnly
                                                        ? kReadOnlyRenderAttachment
                                                        : wgpu::TextureUsage::RenderAttachment);
                }
            }

            cmd->width = validationState.GetRenderWidth();
            cmd->height = validationState.GetRenderHeight();

            cmd->occlusionQuerySet = descriptor->occlusionQuerySet;

            if (descriptor->timestampWrites != nullptr) {
                QuerySetBase* querySet = descriptor->timestampWrites->querySet;
                uint32_t beginningOfPassWriteIndex =
                    descriptor->timestampWrites->beginningOfPassWriteIndex;
                uint32_t endOfPassWriteIndex = descriptor->timestampWrites->endOfPassWriteIndex;

                cmd->timestampWrites.querySet = querySet;
                cmd->timestampWrites.beginningOfPassWriteIndex = beginningOfPassWriteIndex;
                cmd->timestampWrites.endOfPassWriteIndex = endOfPassWriteIndex;
                if (beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, beginningOfPassWriteIndex);
                    // Track the query availability with true on render pass again for rewrite
                    // validation and query reset on Vulkan
                    usageTracker.TrackQueryAvailability(querySet, beginningOfPassWriteIndex);
                }
                if (endOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, endOfPassWriteIndex);
                    // Track the query availability with true on render pass again for rewrite
                    // validation and query reset on Vulkan
                    usageTracker.TrackQueryAvailability(querySet, endOfPassWriteIndex);
                }
            }

            if (auto* pls = descriptor.Get<RenderPassPixelLocalStorage>()) {
                for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
                    const RenderPassStorageAttachment& apiAttachment = pls->storageAttachments[i];
                    RenderPassStorageAttachmentInfo* attachmentInfo =
                        &cmd->storageAttachments[apiAttachment.offset / kPLSSlotByteSize];

                    attachmentInfo->storage = apiAttachment.storage;
                    attachmentInfo->loadOp = apiAttachment.loadOp;
                    attachmentInfo->storeOp = apiAttachment.storeOp;
                    attachmentInfo->clearColor = ClampClearColorValueToLegalRange(
                        apiAttachment.clearValue, apiAttachment.storage->GetFormat());

                    usageTracker.TextureViewUsedAs(apiAttachment.storage,
                                                   wgpu::TextureUsage::StorageAttachment);
                }
            }

            DAWN_TRY(renderpassWorkaroundsHelper.ApplyOnPostEncoding(
                this, descriptor, &usageTracker, cmd, &passEndCallback));

            return {};
        },
        "encoding %s.BeginRenderPass(%s).", this, descriptor);

    if (success) {
        Ref<RenderPassEncoder> passEncoder = RenderPassEncoder::Create(
            device, descriptor, this, &mEncodingContext, std::move(usageTracker),
            std::move(attachmentState), validationState.GetRenderWidth(),
            validationState.GetRenderHeight(), depthReadOnly, stencilReadOnly, passEndCallback);

        mEncodingContext.EnterPass(passEncoder.Get());

        auto error = [&]() -> MaybeError {
            // clearWithDrawHelper.Apply() applies clear with draw if clear_color_with_draw or
            // apply_clear_big_integer_color_value_with_draw toggle is enabled, and the render pass
            // attachments need to be cleared.
            // TODO(341129591): move inside RenderPassWorkaroundsHelper.
            DAWN_TRY(clearWithDrawHelper.Apply(passEncoder.Get()));

            DAWN_TRY(
                renderpassWorkaroundsHelper.ApplyOnRenderPassStart(passEncoder.Get(), descriptor));

            return {};
        }();

        if (device->ConsumedError(std::move(error))) {
            return MakeError();
        }

        return passEncoder;
    }

    return MakeError();
}

void CommandEncoder::APICopyBufferToBuffer(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                DAWN_INVALID_IF(source == destination,
                                "Source and destination are the same buffer (%s).", source);

                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(source, sourceOffset, size),
                                 "validating source %s copy size.", source);
                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(destination, destinationOffset, size),
                                 "validating destination %s copy size.", destination);
                DAWN_TRY(ValidateB2BCopyAlignment(size, sourceOffset, destinationOffset));

                DAWN_TRY_CONTEXT(ValidateCanUseAs(source, wgpu::BufferUsage::CopySrc),
                                 "validating source %s usage.", source);
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination, wgpu::BufferUsage::CopyDst),
                                 "validating destination %s usage.", destination);
            }

            mTopLevelBuffers.insert(source);
            mTopLevelBuffers.insert(destination);

            CopyBufferToBufferCmd* copy =
                allocator->Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
            copy->source = source;
            copy->sourceOffset = sourceOffset;
            copy->destination = destination;
            copy->destinationOffset = destinationOffset;
            copy->size = size;

            return {};
        },
        "encoding %s.CopyBufferToBuffer(%s, %u, %s, %u, %u).", this, source, sourceOffset,
        destination, destinationOffset, size);
}

// The internal version of APICopyBufferToBuffer which validates against mAllocatedSize instead of
// mSize of buffers.
void CommandEncoder::InternalCopyBufferToBufferWithAllocatedSize(BufferBase* source,
                                                                 uint64_t sourceOffset,
                                                                 BufferBase* destination,
                                                                 uint64_t destinationOffset,
                                                                 uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                DAWN_INVALID_IF(source == destination,
                                "Source and destination are the same buffer (%s).", source);

                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(source, sourceOffset, size,
                                                              BufferSizeType::AllocatedSize),
                                 "validating source %s copy size against allocated size.", source);
                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(destination, destinationOffset, size,
                                                              BufferSizeType::AllocatedSize),
                                 "validating destination %s copy size against allocated size.",
                                 destination);
                DAWN_TRY(ValidateB2BCopyAlignment(size, sourceOffset, destinationOffset));

                DAWN_TRY_CONTEXT(ValidateCanUseAsInternal(
                                     source, wgpu::BufferUsage::CopySrc | kInternalCopySrcBuffer),
                                 "validating source %s usage.", source);
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination, wgpu::BufferUsage::CopyDst),
                                 "validating destination %s usage.", destination);
            }

            mTopLevelBuffers.insert(source);
            mTopLevelBuffers.insert(destination);

            CopyBufferToBufferCmd* copy =
                allocator->Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
            copy->source = source;
            copy->sourceOffset = sourceOffset;
            copy->destination = destination;
            copy->destinationOffset = destinationOffset;
            copy->size = size;

            return {};
        },
        "encoding internal %s.CopyBufferToBuffer(%s, %u, %s, %u, %u).", this, source, sourceOffset,
        destination, destinationOffset, size);
}

void CommandEncoder::APICopyBufferToTexture(const TexelCopyBufferInfo* source,
                                            const TexelCopyTextureInfo* destinationOrig,
                                            const Extent3D* copySize) {
    TexelCopyTextureInfo destination = destinationOrig->WithTrivialFrontendDefaults();

    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateTexelCopyBufferInfo(GetDevice(), *source));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(source->buffer, wgpu::BufferUsage::CopySrc),
                                 "validating source %s usage.", source->buffer);

                DAWN_TRY(ValidateTexelCopyTextureInfo(GetDevice(), destination, *copySize));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination.texture, wgpu::TextureUsage::CopyDst,
                                                  mUsageValidationMode),
                                 "validating destination %s usage.", destination.texture);
                DAWN_TRY(ValidateTextureSampleCountInBufferCopyCommands(destination.texture));

                DAWN_TRY(ValidateLinearToDepthStencilCopyRestrictions(destination));
                // We validate texture copy range before validating linear texture data,
                // because in the latter we divide copyExtent.width by blockWidth and
                // copyExtent.height by blockHeight while the divisibility conditions are
                // checked in validating texture copy range.
                DAWN_TRY(ValidateTextureCopyRange(GetDevice(), destination, *copySize));
            }
            const TexelBlockInfo& blockInfo =
                destination.texture->GetFormat().GetAspectInfo(destination.aspect).block;
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateLinearTextureCopyOffset(
                    source->layout, blockInfo,
                    destination.texture->GetFormat().HasDepthOrStencil()));
                DAWN_TRY(ValidateLinearTextureData(source->layout, source->buffer->GetSize(),
                                                   blockInfo, *copySize));
            }

            mTopLevelBuffers.insert(source->buffer);
            mTopLevelTextures.insert(destination.texture);

            TexelCopyBufferLayout srcLayout = source->layout;
            ApplyDefaultTexelCopyBufferLayoutOptions(&srcLayout, blockInfo, *copySize);

            TextureCopy dst;
            dst.texture = destination.texture;
            dst.origin = destination.origin;
            dst.mipLevel = destination.mipLevel;
            dst.aspect = ConvertAspect(destination.texture->GetFormat(), destination.aspect);

            if (dst.aspect == Aspect::Depth &&
                GetDevice()->IsToggleEnabled(Toggle::UseBlitForBufferToDepthTextureCopy)) {
                // The below function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY_CONTEXT(
                    BlitBufferToDepth(GetDevice(), this, source->buffer, srcLayout, dst, *copySize),
                    "copying from %s to depth aspect of %s using blit workaround.", source->buffer,
                    dst.texture.Get());
                return {};
            } else if (dst.aspect == Aspect::Stencil &&
                       GetDevice()->IsToggleEnabled(Toggle::UseBlitForBufferToStencilTextureCopy)) {
                // The below function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY_CONTEXT(BlitBufferToStencil(GetDevice(), this, source->buffer, srcLayout,
                                                     dst, *copySize),
                                 "copying from %s to stencil aspect of %s using blit workaround.",
                                 source->buffer, dst.texture.Get());
                return {};
            }

            CopyBufferToTextureCmd* copy =
                allocator->Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
            copy->source.buffer = source->buffer;
            copy->source.offset = srcLayout.offset;
            copy->source.bytesPerRow = srcLayout.bytesPerRow;
            copy->source.rowsPerImage = srcLayout.rowsPerImage;
            copy->destination = dst;
            copy->copySize = *copySize;

            return {};
        },
        "encoding %s.CopyBufferToTexture(%s, %s, %s).", this, source->buffer, destination.texture,
        copySize);
}

void CommandEncoder::APICopyTextureToBuffer(const TexelCopyTextureInfo* sourceOrig,
                                            const TexelCopyBufferInfo* destination,
                                            const Extent3D* copySize) {
    TexelCopyTextureInfo source = sourceOrig->WithTrivialFrontendDefaults();

    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateTexelCopyTextureInfo(GetDevice(), source, *copySize));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(source.texture, wgpu::TextureUsage::CopySrc,
                                                  mUsageValidationMode),
                                 "validating source %s usage.", source.texture);
                DAWN_TRY(ValidateTextureSampleCountInBufferCopyCommands(source.texture));
                DAWN_TRY(ValidateTextureDepthStencilToBufferCopyRestrictions(source));

                DAWN_TRY(ValidateTexelCopyBufferInfo(GetDevice(), *destination));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination->buffer, wgpu::BufferUsage::CopyDst),
                                 "validating destination %s usage.", destination->buffer);

                // We validate texture copy range before validating linear texture data,
                // because in the latter we divide copyExtent.width by blockWidth and
                // copyExtent.height by blockHeight while the divisibility conditions are
                // checked in validating texture copy range.
                DAWN_TRY(ValidateTextureCopyRange(GetDevice(), source, *copySize));

                if (GetDevice()->IsCompatibilityMode()) {
                    DAWN_TRY(ValidateTextureFormatForTextureToBufferCopyInCompatibilityMode(
                        source.texture));
                }
            }
            const TexelBlockInfo& blockInfo =
                source.texture->GetFormat().GetAspectInfo(source.aspect).block;
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateLinearTextureCopyOffset(
                    destination->layout, blockInfo,
                    source.texture->GetFormat().HasDepthOrStencil()));
                DAWN_TRY(ValidateLinearTextureData(
                    destination->layout, destination->buffer->GetSize(), blockInfo, *copySize));
            }

            mTopLevelTextures.insert(source.texture);
            mTopLevelBuffers.insert(destination->buffer);

            TexelCopyBufferLayout dstLayout = destination->layout;
            ApplyDefaultTexelCopyBufferLayoutOptions(&dstLayout, blockInfo, *copySize);

            if (copySize->width == 0 || copySize->height == 0 ||
                copySize->depthOrArrayLayers == 0) {
                // Noop copy but is valid, simply skip encoding any command.
                return {};
            }

            auto format = source.texture->GetFormat();
            auto aspect = ConvertAspect(format, source.aspect);

            // Workaround to use compute pass to emulate texture to buffer copy
            if (ShouldUseTextureToBufferBlit(GetDevice(), format, aspect)) {
                // This function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                TextureCopy src;
                src.texture = source.texture;
                src.origin = source.origin;
                src.mipLevel = source.mipLevel;
                src.aspect = aspect;

                BufferCopy dst;
                dst.buffer = destination->buffer;
                dst.bytesPerRow = destination->layout.bytesPerRow;
                dst.rowsPerImage = destination->layout.rowsPerImage;
                dst.offset = destination->layout.offset;
                DAWN_TRY_CONTEXT(BlitTextureToBuffer(GetDevice(), this, src, dst, *copySize),
                                 "copying texture %s to %s using blit workaround.",
                                 src.texture.Get(), destination->buffer);

                return {};
            }

            CopyTextureToBufferCmd* t2b =
                allocator->Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
            t2b->source.texture = source.texture;
            t2b->source.origin = source.origin;
            t2b->source.mipLevel = source.mipLevel;
            t2b->source.aspect = ConvertAspect(source.texture->GetFormat(), source.aspect);
            t2b->destination.buffer = destination->buffer;
            t2b->destination.offset = dstLayout.offset;
            t2b->destination.bytesPerRow = dstLayout.bytesPerRow;
            t2b->destination.rowsPerImage = dstLayout.rowsPerImage;
            t2b->copySize = *copySize;

            return {};
        },
        "encoding %s.CopyTextureToBuffer(%s, %s, %s).", this, source.texture, destination->buffer,
        copySize);
}

void CommandEncoder::APICopyTextureToTexture(const TexelCopyTextureInfo* sourceOrig,
                                             const TexelCopyTextureInfo* destinationOrig,
                                             const Extent3D* copySize) {
    TexelCopyTextureInfo source = sourceOrig->WithTrivialFrontendDefaults();
    TexelCopyTextureInfo destination = destinationOrig->WithTrivialFrontendDefaults();

    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source.texture));
                DAWN_TRY(GetDevice()->ValidateObject(destination.texture));

                DAWN_INVALID_IF(source.texture->GetFormat().IsMultiPlanar() ||
                                    destination.texture->GetFormat().IsMultiPlanar(),
                                "Copying between a multiplanar texture and another texture is "
                                "currently not allowed.");
                DAWN_TRY_CONTEXT(ValidateTexelCopyTextureInfo(GetDevice(), source, *copySize),
                                 "validating source %s.", source.texture);
                DAWN_TRY_CONTEXT(ValidateTexelCopyTextureInfo(GetDevice(), destination, *copySize),
                                 "validating destination %s.", destination.texture);

                DAWN_TRY_CONTEXT(ValidateTextureCopyRange(GetDevice(), source, *copySize),
                                 "validating source %s copy range.", source.texture);
                DAWN_TRY_CONTEXT(ValidateTextureCopyRange(GetDevice(), destination, *copySize),
                                 "validating source %s copy range.", destination.texture);

                DAWN_TRY(ValidateTextureToTextureCopyRestrictions(GetDevice(), source, destination,
                                                                  *copySize));

                DAWN_TRY(ValidateCanUseAs(source.texture, wgpu::TextureUsage::CopySrc,
                                          mUsageValidationMode));
                DAWN_TRY(ValidateCanUseAs(destination.texture, wgpu::TextureUsage::CopyDst,
                                          mUsageValidationMode));

                if (GetDevice()->IsCompatibilityMode()) {
                    DAWN_TRY(ValidateSourceTextureFormatForTextureToTextureCopyInCompatibilityMode(
                        source.texture));
                }
            }

            mTopLevelTextures.insert(source.texture);
            mTopLevelTextures.insert(destination.texture);

            Aspect aspect = ConvertAspect(source.texture->GetFormat(), source.aspect);
            DAWN_ASSERT(aspect ==
                        ConvertAspect(destination.texture->GetFormat(), destination.aspect));

            TextureCopy src;
            src.texture = source.texture;
            src.origin = source.origin;
            src.mipLevel = source.mipLevel;
            src.aspect = aspect;

            TextureCopy dst;
            dst.texture = destination.texture;
            dst.origin = destination.origin;
            dst.mipLevel = destination.mipLevel;
            dst.aspect = aspect;

            // Emulate a T2T copy with a T2B copy and a B2T copy.
            if (ShouldUseT2B2TForT2T(GetDevice(), src.texture->GetFormat(),
                                     dst.texture->GetFormat())) {
                // Calculate needed buffer size to hold copied texel data.
                const TexelBlockInfo& blockInfo =
                    source.texture->GetFormat().GetAspectInfo(aspect).block;
                const uint64_t bytesPerRow =
                    Align(4 * copySize->width, kTextureBytesPerRowAlignment);
                const uint64_t rowsPerImage = copySize->height;
                uint64_t requiredBytes;
                DAWN_TRY_ASSIGN(
                    requiredBytes,
                    ComputeRequiredBytesInCopy(blockInfo, *copySize, bytesPerRow, rowsPerImage));

                // Create an intermediate dst buffer.
                BufferDescriptor descriptor = {};
                descriptor.size = Align(requiredBytes, 4);
                descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
                Ref<BufferBase> intermediateBuffer;
                DAWN_TRY_ASSIGN(intermediateBuffer, GetDevice()->CreateBuffer(&descriptor));

                TexelCopyBufferInfo buf;
                buf.buffer = intermediateBuffer.Get();
                buf.layout.bytesPerRow = bytesPerRow;
                buf.layout.rowsPerImage = rowsPerImage;

                APICopyTextureToBuffer(&source, &buf, copySize);
                APICopyBufferToTexture(&buf, &destination, copySize);

                return {};
            }

            const bool blitDepth =
                (aspect & Aspect::Depth) &&
                GetDevice()->IsToggleEnabled(
                    Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource) &&
                (dst.mipLevel > 0 || dst.origin.z > 0 || copySize->depthOrArrayLayers > 1);

            // If we're not using a blit, or there are aspects other than depth,
            // issue the copy. This is because if there's also stencil, we still need the copy
            // command to copy the stencil portion.
            if (!blitDepth || aspect != Aspect::Depth) {
                CopyTextureToTextureCmd* copy =
                    allocator->Allocate<CopyTextureToTextureCmd>(Command::CopyTextureToTexture);
                copy->source = src;
                copy->destination = dst;
                copy->copySize = *copySize;
            }

            // Use a blit to copy the depth aspect.
            if (blitDepth) {
                // This function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY_CONTEXT(BlitDepthToDepth(GetDevice(), this, src, dst, *copySize),
                                 "copying depth aspect from %s to %s using blit workaround.",
                                 source.texture, destination.texture);
            }

            return {};
        },
        "encoding %s.CopyTextureToTexture(%s, %s, %s).", this, source.texture, destination.texture,
        copySize);
}

void CommandEncoder::APIClearBuffer(BufferBase* buffer, uint64_t offset, uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(buffer));

                uint64_t bufferSize = buffer->GetSize();
                DAWN_INVALID_IF(offset > bufferSize,
                                "Buffer offset (%u) is larger than the size (%u) of %s.", offset,
                                bufferSize, buffer);

                uint64_t remainingSize = bufferSize - offset;
                if (size == wgpu::kWholeSize) {
                    size = remainingSize;
                } else {
                    DAWN_INVALID_IF(size > remainingSize,
                                    "Buffer range (offset: %u, size: %u) doesn't fit in "
                                    "the size (%u) of %s.",
                                    offset, size, bufferSize, buffer);
                }

                DAWN_TRY_CONTEXT(ValidateCanUseAs(buffer, wgpu::BufferUsage::CopyDst),
                                 "validating buffer %s usage.", buffer);

                // Size must be a multiple of 4 bytes on macOS.
                DAWN_INVALID_IF(size % 4 != 0, "Fill size (%u) is not a multiple of 4 bytes.",
                                size);

                // Offset must be multiples of 4 bytes on macOS.
                DAWN_INVALID_IF(offset % 4 != 0, "Offset (%u) is not a multiple of 4 bytes,",
                                offset);

            } else {
                if (size == wgpu::kWholeSize) {
                    DAWN_ASSERT(buffer->GetSize() >= offset);
                    size = buffer->GetSize() - offset;
                }
            }

            mTopLevelBuffers.insert(buffer);

            ClearBufferCmd* cmd = allocator->Allocate<ClearBufferCmd>(Command::ClearBuffer);
            cmd->buffer = buffer;
            cmd->offset = offset;
            cmd->size = size;

            return {};
        },
        "encoding %s.ClearBuffer(%s, %u, %u).", this, buffer, offset, size);
}

void CommandEncoder::APIInjectValidationError(StringView messageIn) {
    std::string_view message = utils::NormalizeMessageString(messageIn);
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator*) -> MaybeError {
            return DAWN_MAKE_ERROR(InternalErrorType::Validation, std::string(message));
        },
        "injecting validation error: %s.", message);
}

void CommandEncoder::APIInsertDebugMarker(StringView markerIn) {
    std::string_view marker = utils::NormalizeMessageString(markerIn);
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            InsertDebugMarkerCmd* cmd =
                allocator->Allocate<InsertDebugMarkerCmd>(Command::InsertDebugMarker);
            AddNullTerminatedString(allocator, marker, &cmd->length);
            return {};
        },
        "encoding %s.InsertDebugMarker(%s).", this, marker);
}

void CommandEncoder::APIPopDebugGroup() {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_INVALID_IF(mDebugGroupStackSize == 0,
                                "PopDebugGroup called when no debug groups are currently pushed.");
            }
            allocator->Allocate<PopDebugGroupCmd>(Command::PopDebugGroup);
            mDebugGroupStackSize--;
            mEncodingContext.PopDebugGroupLabel();

            return {};
        },
        "encoding %s.PopDebugGroup().", this);
}

void CommandEncoder::APIPushDebugGroup(StringView groupLabelIn) {
    std::string_view groupLabel = utils::NormalizeMessageString(groupLabelIn);
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            PushDebugGroupCmd* cmd =
                allocator->Allocate<PushDebugGroupCmd>(Command::PushDebugGroup);
            const char* label = AddNullTerminatedString(allocator, groupLabel, &cmd->length);

            mDebugGroupStackSize++;
            mEncodingContext.PushDebugGroupLabel(std::string_view(label, cmd->length));

            return {};
        },
        "encoding %s.PushDebugGroup(%s).", this, groupLabel);
}

void CommandEncoder::APIResolveQuerySet(QuerySetBase* querySet,
                                        uint32_t firstQuery,
                                        uint32_t queryCount,
                                        BufferBase* destination,
                                        uint64_t destinationOffset) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(querySet));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                DAWN_TRY(ValidateQuerySetResolve(querySet, firstQuery, queryCount, destination,
                                                 destinationOffset));

                DAWN_TRY(ValidateCanUseAs(destination, wgpu::BufferUsage::QueryResolve));

                TrackUsedQuerySet(querySet);
            }

            mTopLevelBuffers.insert(destination);

            ResolveQuerySetCmd* cmd =
                allocator->Allocate<ResolveQuerySetCmd>(Command::ResolveQuerySet);
            cmd->querySet = querySet;
            cmd->firstQuery = firstQuery;
            cmd->queryCount = queryCount;
            cmd->destination = destination;
            cmd->destinationOffset = destinationOffset;

            // Encode internal compute pipeline for timestamp query
            if (querySet->GetQueryType() == wgpu::QueryType::Timestamp &&
                !GetDevice()->IsToggleEnabled(Toggle::DisableTimestampQueryConversion)) {
                // The below function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY(EncodeTimestampsToNanosecondsConversion(
                    this, querySet, firstQuery, queryCount, destination, destinationOffset));
            }

            return {};
        },
        "encoding %s.ResolveQuerySet(%s, %u, %u, %s, %u).", this, querySet, firstQuery, queryCount,
        destination, destinationOffset);
}

void CommandEncoder::APIWriteBuffer(BufferBase* buffer,
                                    uint64_t bufferOffset,
                                    const uint8_t* data,
                                    uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateWriteBuffer(GetDevice(), buffer, bufferOffset, size));
            }

            WriteBufferCmd* cmd = allocator->Allocate<WriteBufferCmd>(Command::WriteBuffer);
            cmd->buffer = buffer;
            cmd->offset = bufferOffset;
            cmd->size = size;

            uint8_t* inlinedData = allocator->AllocateData<uint8_t>(size);
            memcpy(inlinedData, data, size);

            mTopLevelBuffers.insert(buffer);

            return {};
        },
        "encoding %s.WriteBuffer(%s, %u, ..., %u).", this, buffer, bufferOffset, size);
}

void CommandEncoder::APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_INVALID_IF(!GetDevice()->IsToggleEnabled(Toggle::AllowUnsafeAPIs),
                            "writeTimestamp requires enabling toggle allow_unsafe_apis.");

            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateTimestampQuery(GetDevice(), querySet, queryIndex));
            }

            TrackQueryAvailability(querySet, queryIndex);

            WriteTimestampCmd* cmd =
                allocator->Allocate<WriteTimestampCmd>(Command::WriteTimestamp);
            cmd->querySet = querySet;
            cmd->queryIndex = queryIndex;

            return {};
        },
        "encoding %s.WriteTimestamp(%s, %u).", this, querySet, queryIndex);
}

CommandBufferBase* CommandEncoder::APIFinish(const CommandBufferDescriptor* descriptor) {
    // This function will create new object, need to lock the Device.
    auto deviceLock(GetDevice()->GetScopedLock());

    Ref<CommandBufferBase> commandBuffer;
    if (GetDevice()->ConsumedError(Finish(descriptor), &commandBuffer, "finishing %s.", this)) {
        Ref<CommandBufferBase> errorCommandBuffer =
            CommandBufferBase::MakeError(GetDevice(), descriptor ? descriptor->label : nullptr);
        errorCommandBuffer->SetEncoderLabel(this->GetLabel());
        return ReturnToAPI(std::move(errorCommandBuffer));
    }

    DAWN_ASSERT(!IsError());
    return ReturnToAPI(std::move(commandBuffer));
}

ResultOrError<Ref<CommandBufferBase>> CommandEncoder::Finish(
    const CommandBufferDescriptor* descriptor) {
    DeviceBase* device = GetDevice();

    TRACE_EVENT0(device->GetPlatform(), Recording, "CommandEncoder::Finish");

    // Even if mEncodingContext.Finish() validation fails, calling it will mutate the internal
    // state of the encoding context. The internal state is set to finished, and subsequent
    // calls to encode commands will generate errors.
    DAWN_TRY(mEncodingContext.Finish());
    DAWN_TRY(device->ValidateIsAlive());

    if (device->IsValidationEnabled()) {
        DAWN_TRY(ValidateFinish());
    }

    const CommandBufferDescriptor defaultDescriptor = {};
    if (descriptor == nullptr) {
        descriptor = &defaultDescriptor;
    }

    return device->CreateCommandBuffer(this, descriptor);
}

// Implementation of the command buffer validation that can be precomputed before submit
MaybeError CommandEncoder::ValidateFinish() const {
    TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "CommandEncoder::ValidateFinish");
    DAWN_TRY(GetDevice()->ValidateObject(this));

    for (const RenderPassResourceUsage& passUsage : mEncodingContext.GetRenderPassUsages()) {
        DAWN_TRY_CONTEXT(ValidateSyncScopeResourceUsage(passUsage),
                         "validating render pass usage.");
    }

    for (const ComputePassResourceUsage& passUsage : mEncodingContext.GetComputePassUsages()) {
        for (const SyncScopeResourceUsage& scope : passUsage.dispatchUsages) {
            DAWN_TRY_CONTEXT(ValidateSyncScopeResourceUsage(scope),
                             "validating compute pass usage.");
        }
    }

    DAWN_INVALID_IF(
        mDebugGroupStackSize != 0,
        "PushDebugGroup called %u time(s) without a corresponding PopDebugGroup prior to "
        "calling Finish.",
        mDebugGroupStackSize);

    return {};
}

CommandEncoder::InternalUsageScope CommandEncoder::MakeInternalUsageScope() {
    return InternalUsageScope(this);
}

CommandEncoder::InternalUsageScope::InternalUsageScope(CommandEncoder* encoder)
    : mEncoder(encoder), mUsageValidationMode(mEncoder->mUsageValidationMode) {
    mEncoder->mUsageValidationMode = UsageValidationMode::Internal;
}

CommandEncoder::InternalUsageScope::~InternalUsageScope() {
    mEncoder->mUsageValidationMode = mUsageValidationMode;
}

}  // namespace dawn::native
