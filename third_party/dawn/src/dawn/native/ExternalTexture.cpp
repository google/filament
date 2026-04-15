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

#include "dawn/native/ExternalTexture.h"

#include <algorithm>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Queue.h"
#include "dawn/native/Texture.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

MaybeError ValidateExternalTexturePlane(const TextureViewBase* textureView) {
    DAWN_INVALID_IF(
        (textureView->GetUsage() & wgpu::TextureUsage::TextureBinding) == 0,
        "The external texture plane (%s) usage (%s) doesn't include the required usage (%s)",
        textureView, textureView->GetUsage(), wgpu::TextureUsage::TextureBinding);

    DAWN_INVALID_IF(textureView->GetDimension() != wgpu::TextureViewDimension::e2D,
                    "The external texture plane (%s) dimension (%s) is not 2D.", textureView,
                    textureView->GetDimension());

    DAWN_INVALID_IF(textureView->GetLevelCount() > 1,
                    "The external texture plane (%s) mip level count (%u) is not 1.", textureView,
                    textureView->GetLevelCount());

    DAWN_INVALID_IF(textureView->GetTexture()->GetSampleCount() != 1,
                    "The external texture plane (%s) sample count (%u) is not one.", textureView,
                    textureView->GetTexture()->GetSampleCount());

    return {};
}

MaybeError ValidateExternalTextureDescriptor(const DeviceBase* device,
                                             const ExternalTextureDescriptor* descriptor) {
    DAWN_ASSERT(descriptor);
    DAWN_ASSERT(descriptor->plane0);

    DAWN_TRY(device->ValidateObject(descriptor->plane0));

    DAWN_INVALID_IF(!descriptor->gamutConversionMatrix,
                    "The gamut conversion matrix must be non-null.");

    DAWN_INVALID_IF(!descriptor->srcTransferFunctionParameters,
                    "The source transfer function parameters must be non-null.");

    DAWN_INVALID_IF(!descriptor->dstTransferFunctionParameters,
                    "The destination transfer function parameters must be non-null.");

    DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane0));

    auto CheckPlaneFormat = [](const DeviceBase* device, const Format& format,
                               uint32_t requiredComponentCount) -> MaybeError {
        DAWN_INVALID_IF(format.aspects != Aspect::Color, "The format (%s) is not a color format.",
                        format.format);

        // The Unorm16FormatsForExternalTexture feature allows using R/RG16Unorm to create
        // ExternalTextures even if they are not filterable float. This is a hack to allow YUV HDR
        // SharedTextureMemory to be used without enabling R/RG16Unorm for anything else.
        bool skipUnorm16 = device->HasFeature(Feature::Unorm16FormatsForExternalTexture) &&
                           (format.format == wgpu::TextureFormat::R16Unorm ||
                            format.format == wgpu::TextureFormat::RG16Unorm);

        if (!skipUnorm16) {
            DAWN_INVALID_IF(!IsSubset(SampleTypeBit::Float,
                                      format.GetAspectInfo(Aspect::Color).supportedSampleTypes),
                            "The format (%s) is not filterable float.", format.format);
        }

        DAWN_INVALID_IF(format.componentCount != requiredComponentCount,
                        "The format (%s) component count (%u) is not %u.", format.format,
                        format.componentCount, requiredComponentCount);
        return {};
    };

    if (descriptor->plane1) {
        DAWN_INVALID_IF(
            !descriptor->yuvToRgbConversionMatrix,
            "When more than one plane is set, the YUV-to-RGB conversion matrix must be non-null.");

        DAWN_TRY(device->ValidateObject(descriptor->plane1));
        DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane1));

        // Y + UV case.
        DAWN_TRY_CONTEXT(CheckPlaneFormat(device, descriptor->plane0->GetFormat(), 1),
                         "validating the format of plane 0 (%s)", descriptor->plane0);
        DAWN_TRY_CONTEXT(CheckPlaneFormat(device, descriptor->plane1->GetFormat(), 2),
                         "validating the format of plane 1 (%s)", descriptor->plane1);

    } else if (descriptor->plane0->GetFormat().format == wgpu::TextureFormat::OpaqueYCbCrAndroid) {
        // Special case for OpaqueYCbCrAndroid
        DAWN_INVALID_IF(!device->HasFeature(Feature::OpaqueYCbCrAndroidForExternalTexture),
                        "%s isn't enabled while plane0 (%s) has format %s.",
                        wgpu::FeatureName::OpaqueYCbCrAndroidForExternalTexture, descriptor->plane0,
                        wgpu::TextureFormat::OpaqueYCbCrAndroid);
        DAWN_INVALID_IF(descriptor->plane0->HasYCbCrDescriptor(),
                        "%s was created with a YCbCrVkDescriptor.", descriptor->plane0);

    } else {
        // RGBA case.
        DAWN_TRY_CONTEXT(CheckPlaneFormat(device, descriptor->plane0->GetFormat(), 4),
                         "validating the format of plane 0 (%s)", descriptor->plane0);
    }

    DAWN_INVALID_IF(descriptor->cropSize.width == 0 || descriptor->cropSize.height == 0,
                    "cropSize %s has 0 on width or height.", descriptor->cropSize);

    const Extent3D textureSize = descriptor->plane0->GetSingleSubresourceVirtualSize();
    DAWN_INVALID_IF(descriptor->cropSize.width > textureSize.width ||
                        descriptor->cropSize.height > textureSize.height,
                    "cropSize %s exceeds the texture size, defined by Plane0 size (%u, %u).",
                    descriptor->cropSize, textureSize.width, textureSize.height);
    DAWN_INVALID_IF(descriptor->cropOrigin.x > textureSize.width - descriptor->cropSize.width ||
                        descriptor->cropOrigin.y > textureSize.height - descriptor->cropSize.height,
                    "cropRect[Origin: %s, Size: %s] exceeds plane0's size (%u, %u).",
                    descriptor->cropOrigin, descriptor->cropSize, textureSize.width,
                    textureSize.height);

    DAWN_INVALID_IF(descriptor->apparentSize.width == 0 || descriptor->apparentSize.height == 0,
                    "apparentSize (%u, %u) is empty.", descriptor->apparentSize.width,
                    descriptor->apparentSize.height);
    DAWN_INVALID_IF(descriptor->apparentSize.width > device->GetLimits().v1.maxTextureDimension2D,
                    "apparentSize.width (%u) is larger than maxTextureDimension2D (%u)",
                    descriptor->apparentSize.width, device->GetLimits().v1.maxTextureDimension2D);
    DAWN_INVALID_IF(descriptor->apparentSize.height > device->GetLimits().v1.maxTextureDimension2D,
                    "apparentSize.height (%u) is larger than maxTextureDimension2D (%u)",
                    descriptor->apparentSize.height, device->GetLimits().v1.maxTextureDimension2D);

    return {};
}

namespace {
ExternalTextureParams ComputeExternalTextureParams(const ExternalTextureDescriptor* descriptor) {
    using math::Mat3x2f;
    using math::Mat3x3f;
    using math::Vec2f;
    using math::Vec2u;

    ExternalTextureParams params;
    params.numPlanes = descriptor->plane1 == nullptr ? 1 : 2;

    params.doYuvToRgbConversionOnly = descriptor->doYuvToRgbConversionOnly ? 1 : 0;

    // YUV-to-RGB conversion is performed by multiplying the source YUV values with a 3x4 matrix on
    // the right. This is to use 36 bytes of uniform buffer instead of 48 for a 4x3 matrix.

    // TODO(https://crbug.com/496604550): Make the conversion matrix required, users can pass the
    // identity if needed.
    math::Mat3x4f yMat = {
        {1, 0, 0, 0},  //
        {0, 1, 0, 0},  //
        {0, 0, 1, 0},  //
    };
    if (descriptor->yuvToRgbConversionMatrix) {
        const float* yMatIn = descriptor->yuvToRgbConversionMatrix;
        yMat = {
            {yMatIn[0], yMatIn[1], yMatIn[2], yMatIn[3]},  //
            {yMatIn[4], yMatIn[5], yMatIn[6], yMatIn[7]},  //
            {yMatIn[8], yMatIn[9], yMatIn[10], yMatIn[11]},
        };
    }

    // Vulkan's YCbCr sampling returns components in Cb, Y, Cr, 1 order while yMat expect Y, Cb,
    // Cr, 1. Reorder them by appending a "swizzling" matrix first.
    if (descriptor->plane0->GetTexture()->GetFormat().format ==
        wgpu::TextureFormat::OpaqueYCbCrAndroid) {
        constexpr math::Mat4x4f kUndoVulkanSwizzle = {
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {1, 0, 0, 0},
            {0, 0, 0, 1},
        };
        yMat = math::Mul(kUndoVulkanSwizzle, yMat);
    }

    params.yuvToRgbConversionMatrix = yMat;

    // Gamut correction is performed by multiplying a 3x3 matrix passed from Chromium. The
    // matrix was computed by multiplying the appropriate source and destination gamut
    // matrices sourced from ui/gfx/color_space.cc.
    const float* gMat = descriptor->gamutConversionMatrix;
    params.gamutConversionMatrix = {{gMat[0], gMat[1], gMat[2]},  //
                                    {gMat[3], gMat[4], gMat[5]},  //
                                    {gMat[6], gMat[7], gMat[8]}};

    // Gamma decode/encode is performed by the logic:
    //    if (abs(v) < params.D) {
    //        return sign(v) * (params.C * abs(v) + params.F);
    //    }
    //    return pow(A * x + B, G) + E
    //
    // Constants are passed from Chromium and originally sourced from ui/gfx/color_space.cc
    auto ToTransferFunctionParams = [](const float* params) -> TransferFunctionParams {
        TransferFunctionMode mode = TransferFunctionMode::Gamma;

        // TODO(https://crbug.com/496604550): Passing the mode as a negative value for G is a hack.
        // Have a dedicated mode enum at the Dawn API level instead.
        if (params[0] < 0) {
            mode = static_cast<TransferFunctionMode>(static_cast<uint32_t>(-params[0]));
        }

        return {
            .mode = mode,
            .a = params[1],
            .b = params[2],
            .c = params[3],
            .d = params[4],
            .e = params[5],
            .f = params[6],
            // This is the first param for historical reasons.
            // TODO(https://crbug.com/496604550): Make the parameters to Dawn a struct with members
            // in alphabetical order.
            .g = params[0],
        };
    };
    params.srcTransferFunction =
        ToTransferFunctionParams(descriptor->srcTransferFunctionParameters);
    params.dstTransferFunction =
        ToTransferFunctionParams(descriptor->dstTransferFunctionParameters);

    // Compute the various transforms and bounds used for sampling and loading operations. They make
    // them appear as if operating on a `apparentSize` texture but instead they are all happening in
    // a transformed and cropped rectangle of the planes. Perform all 2D operations in homogeneous
    // coordinates (in 3D with Z fixed to 1) so that translations can be expressed with matrices.

    // Extract all the relevant sizes as float to avoid extra casts in later computations.
    Extent3D plane0Extent = descriptor->plane0->GetSingleSubresourceVirtualSize();
    Extent3D plane1Extent = {1, 1, 1};
    if (params.numPlanes == 2) {
        plane1Extent = descriptor->plane1->GetSingleSubresourceVirtualSize();
    }
    auto plane0Size = Vec2f(plane0Extent.width, plane0Extent.height);
    auto plane1Size = Vec2f(plane1Extent.width, plane1Extent.height);
    auto cropOrigin = Vec2f(descriptor->cropOrigin.x, descriptor->cropOrigin.y);
    auto cropSize = Vec2f(descriptor->cropSize.width, descriptor->cropSize.height);

    // Offset the coordinates so the center texel is at the origin, so we can apply rotations and
    // y-flips. After translation, coordinates range from [-0.5 .. +0.5] in both U and V.
    Mat3x3f sampleTransform = Mat3x3f::Translation({-0.5, -0.5});

    // The video frame metadata both rotation and mirroring information. The rotation happens before
    // the mirroring when processing the video frame, so do the inverse order when converting UV
    // coordinates.
    if (descriptor->mirrored) {
        sampleTransform = Mul(Mat3x3f::ScaleHomogeneous({-1, 1}), sampleTransform);
    }

    // Apply rotations as needed for the sampling coordinate. This may also rotate the
    // shader-apparent size of the texture.
    Vec2u loadBounds = {descriptor->apparentSize.width - 1, descriptor->apparentSize.height - 1};
    switch (descriptor->rotation) {
        case wgpu::ExternalTextureRotation::Rotate0Degrees:
            break;
        case wgpu::ExternalTextureRotation::Rotate90Degrees:
            std::swap(loadBounds[0], loadBounds[1]);
            sampleTransform = Mul(Mat3x3f({0, -1, 0},  // x -> -y'
                                          {+1, 0, 0},  // y -> x'
                                          {0, 0, 1}),
                                  sampleTransform);
            break;
        case wgpu::ExternalTextureRotation::Rotate180Degrees:
            sampleTransform = Mul(Mat3x3f({-1, 0, 0},  // x -> -x'
                                          {0, -1, 0},  // y -> -y'
                                          {0, 0, 1}),
                                  sampleTransform);
            break;
        case wgpu::ExternalTextureRotation::Rotate270Degrees:
            std::swap(loadBounds[0], loadBounds[1]);
            sampleTransform = Mul(Mat3x3f({0, 1, 0},   // x -> y
                                          {-1, 0, 0},  // y -> -x'
                                          {0, 0, 1}),
                                  sampleTransform);
            break;
    }

    // Offset the coordinates so the bottom-left texel is at origin.
    // After translation, coordinates range from [0 .. 1] in both U and V.
    sampleTransform = Mul(Mat3x3f::Translation({0.5, 0.5}), sampleTransform);

    // Finally, scale and translate based on the crop rect.
    Vec2f rectScale = cropSize / plane0Size;
    Vec2f rectOffset = cropOrigin / plane0Size;

    sampleTransform = Mul(Mat3x3f::ScaleHomogeneous(rectScale), sampleTransform);
    sampleTransform = Mul(Mat3x3f::Translation(rectOffset), sampleTransform);
    params.sampleTransform = Mat3x2f::CropOrExpandFrom(sampleTransform);

    // Compute the load transformation matrix by using toTexels * sampleTransform * toNormalized
    // Note that coords starts from 0 so the max value is size - 1. Note that we use at least 1 for
    // the loadBounds to avoid a division by 0 (since it is not possible to map [0, 0] to [0, 1]).
    // The load coordinate will be set to 0 because of clamping in the shader so it will stay 0
    // after normalization.
    {
        Mat3x3f toTexels = Mat3x3f::ScaleHomogeneous(plane0Size - Vec2f(1, 1));
        Mat3x3f toNormalized =
            Mat3x3f::ScaleHomogeneous(Vec2f(1, 1) / Max(Vec2f(loadBounds), Vec2f(1, 1)));
        Mat3x3f loadTransform = Mul(toTexels, Mul(sampleTransform, toNormalized));

        params.loadTransform = Mat3x2f::CropOrExpandFrom(loadTransform);
    }

    // Compute the clamping for each plane individually: to avoid bleeding of OOB texels due to
    // interpolation we need to offset by a half texel in, which depends on the size of the plane.
    {
        Vec2f plane0HalfTexel = Vec2f(0.5f, 0.5f) / plane0Size;
        Vec2f plane1HalfTexel = Vec2f(0.5f, 0.5f) / plane1Size;

        params.samplePlane0RectMin = rectOffset + plane0HalfTexel;
        params.samplePlane1RectMin = rectOffset + plane1HalfTexel;
        params.samplePlane0RectMax = rectOffset + rectScale - plane0HalfTexel;
        params.samplePlane1RectMax = rectOffset + rectScale - plane1HalfTexel;
    }

    params.plane1CoordFactor = plane1Size / plane0Size;
    params.apparentSize = loadBounds;

    return params;
}
}  // anonymous namespace

ResultOrError<Ref<BufferBase>> MakeParamsBufferForSimpleView(DeviceBase* device,
                                                             Ref<TextureViewBase> textureView) {
    const Extent3D textureSize = textureView->GetSingleSubresourceVirtualSize();
    std::array<float, 12> placeholderConstantArray;

    // Make a fake ExternalTextureDescriptor for the view that reuses the code computing uniform
    // parameters passed to the shader.
    ExternalTextureDescriptor desc = {};
    desc.plane0 = textureView.Get();
    desc.cropOrigin = {0, 0};
    desc.cropSize = {textureSize.width, textureSize.height};
    desc.apparentSize = {textureSize.width, textureSize.height};
    desc.doYuvToRgbConversionOnly = true;
    desc.srcTransferFunctionParameters = placeholderConstantArray.data();
    desc.dstTransferFunctionParameters = placeholderConstantArray.data();
    desc.gamutConversionMatrix = placeholderConstantArray.data();

    ExternalTextureParams params = ComputeExternalTextureParams(&desc);
    return utils::CreateBufferFromData(device, "Dawn_Simple_Texture_View_Params_Buffer",
                                       wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
                                       {params});
}

// static
ResultOrError<Ref<ExternalTextureBase>> ExternalTextureBase::Create(
    DeviceBase* device,
    const ExternalTextureDescriptor* descriptor) {
    Ref<ExternalTextureBase> externalTexture =
        AcquireRef(new ExternalTextureBase(device, descriptor));
    DAWN_TRY(externalTexture->Initialize(device, descriptor));
    return std::move(externalTexture);
}

ExternalTextureBase::ExternalTextureBase(DeviceBase* device,
                                         const ExternalTextureDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label), mState(ExternalTextureState::Active) {
    GetObjectTrackingList()->Track(this);
}

// Error external texture cannot be used in bind group.
ExternalTextureBase::ExternalTextureBase(DeviceBase* device,
                                         ObjectBase::ErrorTag tag,
                                         StringView label)
    : ApiObjectBase(device, tag, label), mState(ExternalTextureState::Destroyed) {}

ExternalTextureBase::~ExternalTextureBase() = default;

MaybeError ExternalTextureBase::Initialize(DeviceBase* device,
                                           const ExternalTextureDescriptor* descriptor) {
    // Store any passed in TextureViews associated with individual planes.
    mTextureViews[0] = descriptor->plane0;

    if (descriptor->plane1) {
        mViewCount = 2;
        mTextureViews[1] = descriptor->plane1;
    } else {
        mViewCount = 1;
        DAWN_TRY_ASSIGN(mTextureViews[1],
                        device->GetOrCreatePlaceholderTextureViewForExternalTexture());
    }

    // We must create a buffer to store parameters needed by a shader that operates on this
    // external texture.
    ExternalTextureParams params = ComputeExternalTextureParams(descriptor);
    DAWN_TRY_ASSIGN(mParamsBuffer,
                    utils::CreateBufferFromData(
                        device, "Dawn_External_Texture_Params_Buffer",
                        wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, {params}));

    return {};
}

const std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat>& ExternalTextureBase::GetTextureViews()
    const {
    return mTextureViews;
}

MaybeError ExternalTextureBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_INVALID_IF(mState != ExternalTextureState::Active,
                    "External texture %s used in a submit is not active.", this);

    for (uint32_t i = 0; i < kMaxPlanesPerFormat; ++i) {
        if (mTextureViews[i] != nullptr) {
            DAWN_TRY_CONTEXT(mTextureViews[i]->GetTexture()->ValidateCanUseInSubmitNow(),
                             "Validate plane %u of %s can be used in a submit.", i, this);
        }
    }
    return {};
}

MaybeError ExternalTextureBase::ValidateRefresh() {
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_INVALID_IF(mState == ExternalTextureState::Destroyed, "%s is destroyed.", this);
    return {};
}

MaybeError ExternalTextureBase::ValidateExpire() {
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_INVALID_IF(mState != ExternalTextureState::Active, "%s is not active.", this);
    return {};
}

void ExternalTextureBase::APIRefresh() {
    if (GetDevice()->ConsumedError(ValidateRefresh(), "calling %s.Refresh()", this)) {
        return;
    }
    mState = ExternalTextureState::Active;
}

void ExternalTextureBase::APIExpire() {
    if (GetDevice()->ConsumedError(ValidateExpire(), "calling %s.Expire()", this)) {
        return;
    }
    mState = ExternalTextureState::Expired;
}

void ExternalTextureBase::APIDestroy() {
    Destroy();
}

void ExternalTextureBase::DestroyImpl(DestroyReason reason) {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    mState = ExternalTextureState::Destroyed;
}

// static
Ref<ExternalTextureBase> ExternalTextureBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new ExternalTextureBase(device, ObjectBase::kError, label));
}

BufferBase* ExternalTextureBase::GetParamsBuffer() const {
    DAWN_ASSERT(!IsError());
    return mParamsBuffer.Get();
}

bool ExternalTextureBase::HasSingleView() const {
    DAWN_ASSERT(!IsError());
    return mViewCount == 1;
}

ObjectType ExternalTextureBase::GetType() const {
    return ObjectType::ExternalTexture;
}

}  // namespace dawn::native
