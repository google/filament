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

#ifndef SRC_DAWN_NATIVE_WEBGPU_EXTERNALTTEXTUREWGPU_H_
#define SRC_DAWN_NATIVE_WEBGPU_EXTERNALTTEXTUREWGPU_H_

#include <array>

#include "dawn/native/ExternalTexture.h"
#include "dawn/native/webgpu/ObjectWGPU.h"
#include "dawn/native/webgpu/RecordableObject.h"

namespace dawn::native::webgpu {

class Device;

class ExternalTexture final : public ExternalTextureBase,
                              public RecordableObject,
                              public ObjectWGPU<WGPUExternalTexture> {
  public:
    struct CreationParams {
        Origin2D cropOrigin;
        Extent2D cropSize;
        Extent2D apparentSize;
        bool doYuvToRgbConversionOnly;
        std::array<float, 12> yuvToRgbConversionMatrix;
        std::array<float, 7> srcTransferFunctionParameters;
        std::array<float, 7> dstTransferFunctionParameters;
        std::array<float, 9> gamutConversionMatrix;
        bool mirrored;
        wgpu::ExternalTextureRotation rotation;

        bool hasPlane1;

        explicit CreationParams(const ExternalTextureDescriptor* descriptor);
    };

    static ResultOrError<Ref<ExternalTextureBase>> Create(
        Device* device,
        const ExternalTextureDescriptor* descriptor);

    MaybeError AddReferenced(CaptureContext& captureContext) override;
    MaybeError CaptureCreationParameters(CaptureContext& context) override;

  private:
    ExternalTexture(Device* device, const ExternalTextureDescriptor* descriptor);
    ~ExternalTexture() override;

    void DestroyImpl(DestroyReason reason) override;
    void SetLabelImpl() override;
    CreationParams mCreationParams;
};

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_EXTERNALTTEXTUREWGPU_H_
