// Copyright 2026 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/ExternalTextureWGPU.h"

#include <string>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/TextureWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"
namespace dawn::native::webgpu {

ExternalTexture::CreationParams::CreationParams(const ExternalTextureDescriptor* descriptor) {
    cropOrigin = descriptor->cropOrigin;
    cropSize = descriptor->cropSize;
    apparentSize = descriptor->apparentSize;
    doYuvToRgbConversionOnly = descriptor->doYuvToRgbConversionOnly;
    mirrored = descriptor->mirrored;
    rotation = descriptor->rotation;

    hasPlane1 = descriptor->plane1 != nullptr;

    std::copy_n(descriptor->yuvToRgbConversionMatrix, 12, yuvToRgbConversionMatrix.begin());
    std::copy_n(descriptor->srcTransferFunctionParameters, 7,
                srcTransferFunctionParameters.begin());
    std::copy_n(descriptor->dstTransferFunctionParameters, 7,
                dstTransferFunctionParameters.begin());
    std::copy_n(descriptor->gamutConversionMatrix, 9, gamutConversionMatrix.begin());
}

// static
ResultOrError<Ref<ExternalTextureBase>> ExternalTexture::Create(
    Device* device,
    const ExternalTextureDescriptor* descriptor) {
    Ref<ExternalTexture> externalTexture = AcquireRef(new ExternalTexture(device, descriptor));
    DAWN_TRY(externalTexture->Initialize(device, descriptor));
    return externalTexture;
}

ExternalTexture::ExternalTexture(Device* device, const ExternalTextureDescriptor* descriptor)
    : ExternalTextureBase(device, descriptor),
      RecordableObject(schema::ObjectType::ExternalTexture),
      ObjectWGPU(device->wgpu->externalTextureRelease),
      mCreationParams(descriptor) {
    std::string label = GetLabel();
    WGPUExternalTextureDescriptor desc = {
        .nextInChain = nullptr,
        .label = ToOutputStringView(label),
        .plane0 = ToBackend(descriptor->plane0)->GetInnerHandle(),
        .plane1 = descriptor->plane1 ? ToBackend(descriptor->plane1)->GetInnerHandle() : nullptr,
        .cropOrigin = ToWGPU(descriptor->cropOrigin),
        .cropSize = ToWGPU(descriptor->cropSize),
        .apparentSize = ToWGPU(descriptor->apparentSize),
        .doYuvToRgbConversionOnly = descriptor->doYuvToRgbConversionOnly,
        .yuvToRgbConversionMatrix = descriptor->yuvToRgbConversionMatrix,
        .srcTransferFunctionParameters = descriptor->srcTransferFunctionParameters,
        .dstTransferFunctionParameters = descriptor->dstTransferFunctionParameters,
        .gamutConversionMatrix = descriptor->gamutConversionMatrix,
        .mirrored = descriptor->mirrored,
        .rotation = ToAPI(descriptor->rotation),
    };

    mInnerHandle = device->wgpu->deviceCreateExternalTexture(device->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);
}

ExternalTexture::~ExternalTexture() = default;

void ExternalTexture::DestroyImpl(DestroyReason reason) {
    ExternalTextureBase::DestroyImpl(reason);
    auto& wgpu = ToBackend(GetDevice())->wgpu.get();
    wgpu.externalTextureDestroy(mInnerHandle);
}

void ExternalTexture::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError ExternalTexture::AddReferenced(CaptureContext& captureContext) {
    for (auto view : GetTextureViews()) {
        if (view.Get() != nullptr) {
            DAWN_TRY(captureContext.AddResource(ToBackend(view.Get())));
        }
    }
    return {};
}

MaybeError ExternalTexture::CaptureCreationParameters(CaptureContext& captureContext) {
    schema::ExternalTexture tex{{
        .plane0Id = captureContext.GetId(GetTextureViews()[0].Get()),
        // ExternalTextureBase assigns mExternalTexturePlaceholderView to plane1 if it is not
        // assigned explicitly in descriptor, so we need to avoid assigning the placeholder view.
        .plane1Id =
            mCreationParams.hasPlane1 ? captureContext.GetId(GetTextureViews()[1].Get()) : 0,
        .cropOrigin = ToSchema(mCreationParams.cropOrigin),
        .cropSize = ToSchema(mCreationParams.cropSize),
        .apparentSize = ToSchema(mCreationParams.apparentSize),
        .doYuvToRgbConversionOnly = mCreationParams.doYuvToRgbConversionOnly,
        .yuvToRgbConversionMatrix = mCreationParams.yuvToRgbConversionMatrix,
        .srcTransferFunctionParameters = mCreationParams.srcTransferFunctionParameters,
        .dstTransferFunctionParameters = mCreationParams.dstTransferFunctionParameters,
        .gamutConversionMatrix = mCreationParams.gamutConversionMatrix,
        .mirrored = mCreationParams.mirrored,
        .rotation = mCreationParams.rotation,
    }};

    Serialize(captureContext, tex);
    return {};
}

}  // namespace dawn::native::webgpu
