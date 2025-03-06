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

#include "dawn/native/RenderBundleEncoder.h"

#include <string>
#include <utility>

#include "dawn/native/Adapter.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/Format.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

MaybeError ValidateColorAttachmentFormat(const DeviceBase* device,
                                         wgpu::TextureFormat textureFormat) {
    DAWN_TRY(ValidateTextureFormat(textureFormat));
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(textureFormat));
    DAWN_INVALID_IF(!format->IsColor() || !format->isRenderable,
                    "Texture format %s is not color renderable.", textureFormat);
    return {};
}

MaybeError ValidateDepthStencilAttachmentFormat(const DeviceBase* device,
                                                wgpu::TextureFormat textureFormat,
                                                bool depthReadOnly,
                                                bool stencilReadOnly) {
    DAWN_TRY(ValidateTextureFormat(textureFormat));
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(textureFormat));
    DAWN_INVALID_IF(!format->HasDepthOrStencil() || !format->isRenderable,
                    "Texture format %s is not depth/stencil renderable.", textureFormat);
    return {};
}

MaybeError ValidateRenderBundleEncoderDescriptor(DeviceBase* device,
                                                 const RenderBundleEncoderDescriptor* descriptor) {
    DAWN_INVALID_IF(!IsValidSampleCount(descriptor->sampleCount),
                    "Sample count (%u) is not supported.", descriptor->sampleCount);

    uint32_t maxColorAttachments = device->GetLimits().v1.maxColorAttachments;
    DAWN_INVALID_IF(descriptor->colorFormatCount > maxColorAttachments,
                    "Color formats count (%u) exceeds maximum number of color attachments (%u).%s",
                    descriptor->colorFormatCount, maxColorAttachments,
                    DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter(), maxColorAttachments,
                                                descriptor->colorFormatCount));

    bool allColorFormatsUndefined = true;
    ColorAttachmentFormats colorAttachmentFormats;
    for (uint32_t i = 0; i < descriptor->colorFormatCount; ++i) {
        wgpu::TextureFormat format = descriptor->colorFormats[i];
        if (format != wgpu::TextureFormat::Undefined) {
            DAWN_TRY_CONTEXT(ValidateColorAttachmentFormat(device, format),
                             "validating colorFormats[%u]", i);
            colorAttachmentFormats.push_back(&device->GetValidInternalFormat(format));
            allColorFormatsUndefined = false;
        }
    }
    DAWN_TRY_CONTEXT(ValidateColorAttachmentBytesPerSample(device, colorAttachmentFormats),
                     "validating color attachment bytes per sample.");

    if (descriptor->depthStencilFormat != wgpu::TextureFormat::Undefined) {
        DAWN_TRY_CONTEXT(ValidateDepthStencilAttachmentFormat(
                             device, descriptor->depthStencilFormat, descriptor->depthReadOnly,
                             descriptor->stencilReadOnly),
                         "validating depthStencilFormat");
    } else {
        DAWN_INVALID_IF(
            allColorFormatsUndefined,
            "No color or depthStencil attachments specified. At least one is required.");
    }

    return {};
}

RenderBundleEncoder::RenderBundleEncoder(DeviceBase* device,
                                         const RenderBundleEncoderDescriptor* descriptor)
    : RenderEncoderBase(device,
                        descriptor->label,
                        &mBundleEncodingContext,
                        device->GetOrCreateAttachmentState(descriptor),
                        descriptor->depthReadOnly,
                        descriptor->stencilReadOnly),
      mBundleEncodingContext(device, this) {
    GetObjectTrackingList()->Track(this);
}

RenderBundleEncoder::RenderBundleEncoder(DeviceBase* device, ErrorTag errorTag, StringView label)
    : RenderEncoderBase(device, &mBundleEncodingContext, errorTag, label),
      mBundleEncodingContext(device, errorTag) {}

RenderBundleEncoder::~RenderBundleEncoder() {
    mEncodingContext = nullptr;
}

void RenderBundleEncoder::DestroyImpl() {
    mIndirectDrawMetadata.ClearIndexedIndirectBufferValidationInfo();
    mCommandBufferState.End();
    RenderEncoderBase::DestroyImpl();
    mBundleEncodingContext.Destroy();
}

// static
Ref<RenderBundleEncoder> RenderBundleEncoder::Create(
    DeviceBase* device,
    const RenderBundleEncoderDescriptor* descriptor) {
    return AcquireRef(new RenderBundleEncoder(device, descriptor));
}

// static
Ref<RenderBundleEncoder> RenderBundleEncoder::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new RenderBundleEncoder(device, ObjectBase::kError, label));
}

ObjectType RenderBundleEncoder::GetType() const {
    return ObjectType::RenderBundleEncoder;
}

CommandIterator RenderBundleEncoder::AcquireCommands() {
    return mBundleEncodingContext.AcquireCommands();
}

RenderBundleBase* RenderBundleEncoder::APIFinish(const RenderBundleDescriptor* descriptor) {
    Ref<RenderBundleBase> result;

    if (GetDevice()->ConsumedError(Finish(descriptor), &result, "calling %s.Finish(%s).", this,
                                   descriptor)) {
        result = RenderBundleBase::MakeError(GetDevice(), descriptor ? descriptor->label : nullptr);
        result->SetEncoderLabel(this->GetLabel());
    }

    return ReturnToAPI(std::move(result));
}

ResultOrError<Ref<RenderBundleBase>> RenderBundleEncoder::Finish(
    const RenderBundleDescriptor* descriptor) {
    mCommandBufferState.End();
    // Even if mBundleEncodingContext.Finish() validation fails, calling it will mutate the
    // internal state of the encoding context. Subsequent calls to encode commands will generate
    // errors.
    DAWN_TRY(mBundleEncodingContext.Finish());
    DAWN_TRY(GetDevice()->ValidateIsAlive());

    RenderPassResourceUsage usages = mUsageTracker.AcquireResourceUsage();
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidateFinish(usages));
    }

    return AcquireRef(new RenderBundleBase(this, descriptor, AcquireAttachmentState(),
                                           IsDepthReadOnly(), IsStencilReadOnly(),
                                           std::move(usages), std::move(mIndirectDrawMetadata)));
}

MaybeError RenderBundleEncoder::ValidateFinish(const RenderPassResourceUsage& usages) const {
    TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "RenderBundleEncoder::ValidateFinish");
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(ValidateProgrammableEncoderEnd());
    DAWN_TRY(ValidateSyncScopeResourceUsage(usages));
    return {};
}

}  // namespace dawn::native
