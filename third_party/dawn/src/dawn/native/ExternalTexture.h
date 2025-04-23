// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_EXTERNALTEXTURE_H_
#define SRC_DAWN_NATIVE_EXTERNALTEXTURE_H_

#include <array>

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/Subresource.h"

namespace dawn::native {

class TextureViewBase;

struct ExternalTextureParams {
    uint32_t numPlanes;
    // TODO(crbug.com/dawn/1466): Only go as few steps as necessary.
    uint32_t doYuvToRgbConversionOnly;
    std::array<uint32_t, 2> padding;
    std::array<float, 12> yuvToRgbConversionMatrix;
    std::array<float, 8> gammaDecodingParams = {};
    std::array<float, 8> gammaEncodingParams = {};
    std::array<float, 12> gamutConversionMatrix = {};
    std::array<float, 6> sampleTransform = {};
    std::array<float, 6> loadTransform = {};
    std::array<float, 2> samplePlane0RectMin = {};
    std::array<float, 2> samplePlane0RectMax = {};
    std::array<float, 2> samplePlane1RectMin = {};
    std::array<float, 2> samplePlane1RectMax = {};
    // The shader-visible size of the texture for textureLoad and textureDimensions
    std::array<uint32_t, 2> apparentSize = {};
    // textureLoad() passes coords in plane0 related size.
    // Use this Factor to calculate plane1 load coord.
    std::array<float, 2> plane1CoordFactor = {};
};

MaybeError ValidateExternalTextureDescriptor(const DeviceBase* device,
                                             const ExternalTextureDescriptor* descriptor);

// Create a parameter buffer for a simple texture view intended for use as an external texture. The
// buffer contains the uniform parameters required by a shader to sample from an external texture.
ResultOrError<Ref<BufferBase>> MakeParamsBufferForSimpleView(DeviceBase* device,
                                                             Ref<TextureViewBase> textureView);

class ExternalTextureBase : public ApiObjectBase {
  public:
    static ResultOrError<Ref<ExternalTextureBase>> Create(
        DeviceBase* device,
        const ExternalTextureDescriptor* descriptor);

    BufferBase* GetParamsBuffer() const;
    const std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat>& GetTextureViews() const;
    ObjectType GetType() const override;

    MaybeError ValidateCanUseInSubmitNow() const;
    static Ref<ExternalTextureBase> MakeError(DeviceBase* device, StringView label = {});

    void APIExpire();
    void APIDestroy();
    void APIRefresh();

  protected:
    ExternalTextureBase(DeviceBase* device, const ExternalTextureDescriptor* descriptor);
    void DestroyImpl() override;

    MaybeError Initialize(DeviceBase* device, const ExternalTextureDescriptor* descriptor);

    ~ExternalTextureBase() override;

  private:
    enum class ExternalTextureState { Active, Expired, Destroyed };
    ExternalTextureBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    MaybeError ValidateRefresh();
    MaybeError ValidateExpire();

    Ref<TextureBase> mPlaceholderTexture;
    Ref<BufferBase> mParamsBuffer;
    std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat> mTextureViews;
    ExternalTextureState mState;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EXTERNALTEXTURE_H_
