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

#include "dawn/native/metal/TextureMTL.h"

#include <CoreVideo/CVPixelBuffer.h>

#include "absl/strings/str_format.h"
#include "dawn/common/Constants.h"
#include "dawn/common/IOSurfaceUtils.h"
#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Queue.h"
#include "dawn/native/metal/BufferMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/QueueMTL.h"
#include "dawn/native/metal/SharedFenceMTL.h"
#include "dawn/native/metal/SharedTextureMemoryMTL.h"
#include "dawn/native/metal/UtilsMetal.h"

namespace dawn::native::metal {

namespace {

MTLTextureUsage MetalTextureUsage(const Format& format, wgpu::TextureUsage usage) {
    MTLTextureUsage result = MTLTextureUsageUnknown;  // This is 0

    if (usage & (wgpu::TextureUsage::StorageBinding)) {
        result |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    }

    if (usage & (wgpu::TextureUsage::TextureBinding)) {
        result |= MTLTextureUsageShaderRead;

        // For sampling stencil aspect of combined depth/stencil.
        // See TextureView::Initialize.
        // Depth views for depth/stencil textures in Metal simply use the original
        // texture's format, but stencil views require format reinterpretation.
        if (IsSubset(Aspect::Depth | Aspect::Stencil, format.aspects)) {
            result |= MTLTextureUsagePixelFormatView;
        }
    }

    if (usage & wgpu::TextureUsage::RenderAttachment) {
        result |= MTLTextureUsageRenderTarget;
    }

    if (usage & wgpu::TextureUsage::StorageAttachment) {
        // TODO(dawn:1704): Support PLS on non-tiler Metal devices.
        result |= MTLTextureUsageRenderTarget;
    }

    return result;
}

MTLTextureType MetalTextureViewType(wgpu::TextureViewDimension dimension,
                                    unsigned int sampleCount) {
    switch (dimension) {
        case wgpu::TextureViewDimension::e1D:
            return MTLTextureType1D;
        case wgpu::TextureViewDimension::e2D:
            return (sampleCount > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
        case wgpu::TextureViewDimension::e2DArray:
            return MTLTextureType2DArray;
        case wgpu::TextureViewDimension::Cube:
            return MTLTextureTypeCube;
        case wgpu::TextureViewDimension::CubeArray:
            return MTLTextureTypeCubeArray;
        case wgpu::TextureViewDimension::e3D:
            return MTLTextureType3D;

        case wgpu::TextureViewDimension::Undefined:
            DAWN_UNREACHABLE();
    }
}

MTLTextureSwizzle MetalTextureSwizzle(wgpu::ComponentSwizzle swizzle) {
    switch (swizzle) {
        case wgpu::ComponentSwizzle::Zero:
            return MTLTextureSwizzleZero;
        case wgpu::ComponentSwizzle::One:
            return MTLTextureSwizzleOne;
        case wgpu::ComponentSwizzle::R:
            return MTLTextureSwizzleRed;
        case wgpu::ComponentSwizzle::G:
            return MTLTextureSwizzleGreen;
        case wgpu::ComponentSwizzle::B:
            return MTLTextureSwizzleBlue;
        case wgpu::ComponentSwizzle::A:
            return MTLTextureSwizzleAlpha;

        case wgpu::ComponentSwizzle::Undefined:
            DAWN_UNREACHABLE();
    }
}

MTLTextureSwizzleChannels ToMetalTextureSwizzleChannels(wgpu::TextureComponentSwizzle swizzle) {
    MTLTextureSwizzleChannels mtlSwizzle;
    mtlSwizzle.red = MetalTextureSwizzle(swizzle.r);
    mtlSwizzle.green = MetalTextureSwizzle(swizzle.g);
    mtlSwizzle.blue = MetalTextureSwizzle(swizzle.b);
    mtlSwizzle.alpha = MetalTextureSwizzle(swizzle.a);
    return mtlSwizzle;
}

bool RequiresCreatingNewTextureView(const Texture* texture,
                                    bool viewMtlFormatDiffers,
                                    wgpu::TextureUsage internalViewUsage,
                                    const UnpackedPtr<TextureViewDescriptor>& textureViewDescriptor,
                                    MTLTextureType textureViewType,
                                    const std::optional<MTLTextureSwizzleChannels>& swizzle) {
    // A view can have render attachment usages, shader binding usages, or both, so those are the
    // cases handled below. (Technically they can have copy-only usages, but such views can't be
    // used for anything, so we don't bother to optimize to avoid creating views in that case.)

    // Different format (color format reinterpretation or aspect subsetting) always requires a view.
    if (viewMtlFormatDiffers) {
        return true;
    }

    // A view is only needed for any other parameters if used from a shader. (For attachments,
    // subresource indices are passed elsewhere in the backend, not as part of the Metal view.)
    bool hasShaderUsages = internalViewUsage & (wgpu::TextureUsage::StorageBinding |
                                                wgpu::TextureUsage::TextureBinding);
    if (hasShaderUsages) {
        if (texture->GetNumMipLevels() != textureViewDescriptor->mipLevelCount) {
            return true;
        }
        if (texture->GetArrayLayers() != textureViewDescriptor->arrayLayerCount) {
            return true;
        }
        if (texture->GetMTLTextureType() != textureViewType) {
            return true;
        }
        if (swizzle.has_value()) {
            return true;
        }
    }

    return false;
}

// Metal only allows format reinterpretation to happen on swizzle pattern or conversion
// between linear space and sRGB without setting MTLTextureUsagePixelFormatView flag. For
// example, creating bgra8Unorm texture view on rgba8Unorm texture or creating
// rgba8Unorm_srgb texture view on rgab8Unorm texture.
bool AllowFormatReinterpretationWithoutFlag(MTLPixelFormat origin,
                                            MTLPixelFormat reinterpretation) {
#define SRGB_PAIR(a, b)               \
    case a:                           \
        return reinterpretation == b; \
    case b:                           \
        return reinterpretation == a

    switch (origin) {
        case MTLPixelFormatRGBA8Unorm:
            return reinterpretation == MTLPixelFormatBGRA8Unorm ||
                   reinterpretation == MTLPixelFormatRGBA8Unorm_sRGB;
        case MTLPixelFormatBGRA8Unorm:
            return reinterpretation == MTLPixelFormatRGBA8Unorm ||
                   reinterpretation == MTLPixelFormatBGRA8Unorm_sRGB;
        case MTLPixelFormatRGBA8Unorm_sRGB:
            return reinterpretation == MTLPixelFormatBGRA8Unorm_sRGB ||
                   reinterpretation == MTLPixelFormatRGBA8Unorm;
        case MTLPixelFormatBGRA8Unorm_sRGB:
            return reinterpretation == MTLPixelFormatRGBA8Unorm_sRGB ||
                   reinterpretation == MTLPixelFormatBGRA8Unorm;

#if DAWN_PLATFORM_IS(MACOS)
            SRGB_PAIR(MTLPixelFormatBC1_RGBA, MTLPixelFormatBC1_RGBA_sRGB);
            SRGB_PAIR(MTLPixelFormatBC2_RGBA, MTLPixelFormatBC2_RGBA_sRGB);
            SRGB_PAIR(MTLPixelFormatBC3_RGBA, MTLPixelFormatBC3_RGBA_sRGB);
            SRGB_PAIR(MTLPixelFormatBC7_RGBAUnorm, MTLPixelFormatBC7_RGBAUnorm_sRGB);
#endif
        default:
            switch (origin) {
                SRGB_PAIR(MTLPixelFormatEAC_RGBA8, MTLPixelFormatEAC_RGBA8_sRGB);

                SRGB_PAIR(MTLPixelFormatETC2_RGB8, MTLPixelFormatETC2_RGB8_sRGB);
                SRGB_PAIR(MTLPixelFormatETC2_RGB8A1, MTLPixelFormatETC2_RGB8A1_sRGB);

                SRGB_PAIR(MTLPixelFormatASTC_4x4_LDR, MTLPixelFormatASTC_4x4_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_5x4_LDR, MTLPixelFormatASTC_5x4_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_5x5_LDR, MTLPixelFormatASTC_5x5_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_6x5_LDR, MTLPixelFormatASTC_6x5_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_6x6_LDR, MTLPixelFormatASTC_6x6_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_8x5_LDR, MTLPixelFormatASTC_8x5_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_8x6_LDR, MTLPixelFormatASTC_8x6_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_8x8_LDR, MTLPixelFormatASTC_8x8_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_10x5_LDR, MTLPixelFormatASTC_10x5_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_10x6_LDR, MTLPixelFormatASTC_10x6_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_10x8_LDR, MTLPixelFormatASTC_10x8_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_10x10_LDR, MTLPixelFormatASTC_10x10_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_12x10_LDR, MTLPixelFormatASTC_12x10_sRGB);
                SRGB_PAIR(MTLPixelFormatASTC_12x12_LDR, MTLPixelFormatASTC_12x12_sRGB);

                default:
                    break;
            }

            return false;
    }
#undef SRGB_PAIR
}

}  // namespace

NSRef<MTLTextureDescriptor> Texture::CreateMetalTextureDescriptor() const {
    NSRef<MTLTextureDescriptor> mtlDescRef = AcquireNSRef([MTLTextureDescriptor new]);
    MTLTextureDescriptor* mtlDesc = mtlDescRef.Get();

    DAWN_ASSERT(!GetFormat().IsMultiPlanar());
    mtlDesc.width = GetBaseSize().width;
    mtlDesc.sampleCount = GetSampleCount();
    // Metal only allows format reinterpretation to happen on swizzle pattern or conversion
    // between linear space and sRGB. For example, creating bgra8Unorm texture view on
    // rgba8Unorm texture or creating rgba8Unorm_srgb texture view on rgab8Unorm texture.
    mtlDesc.usage = MetalTextureUsage(GetFormat(), GetInternalUsage());
    mtlDesc.pixelFormat = MetalPixelFormat(GetDevice(), GetFormat().format);
    if (GetDevice()->IsToggleEnabled(Toggle::MetalUseCombinedDepthStencilFormatForStencil8) &&
        GetFormat().format == wgpu::TextureFormat::Stencil8) {
        // If we used a combined depth stencil format instead of stencil8, we need
        // MTLTextureUsagePixelFormatView to reinterpret as stencil8.
        mtlDesc.usage |= MTLTextureUsagePixelFormatView;
    }
    mtlDesc.mipmapLevelCount = GetNumMipLevels();

    // Create the texture in private storage mode unless the client has
    // specified that this texture is for a transient attachment, in which case
    // the texture should be created in memoryless storage mode.
    mtlDesc.storageMode = MTLStorageModePrivate;
    if (GetInternalUsage() & wgpu::TextureUsage::TransientAttachment &&
        [ToBackend(GetDevice())->GetMTLDevice() supportsFamily:MTLGPUFamilyApple2]) {
        mtlDesc.storageMode = MTLStorageModeMemoryless;
    }

    // Choose the correct MTLTextureType and paper over differences in how the array layer count
    // is specified.
    switch (GetDimension()) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();

        case wgpu::TextureDimension::e1D:
            mtlDesc.arrayLength = 1;
            mtlDesc.depth = 1;
            DAWN_ASSERT(mtlDesc.sampleCount == 1);
            mtlDesc.textureType = MTLTextureType1D;
            break;

        case wgpu::TextureDimension::e2D:
            mtlDesc.height = GetBaseSize().height;
            mtlDesc.arrayLength = GetArrayLayers();
            mtlDesc.depth = 1;
            if (mtlDesc.arrayLength > 1) {
                DAWN_ASSERT(mtlDesc.sampleCount == 1);
                mtlDesc.textureType = MTLTextureType2DArray;
            } else if (mtlDesc.sampleCount > 1) {
                mtlDesc.textureType = MTLTextureType2DMultisample;
            } else {
                mtlDesc.textureType = MTLTextureType2D;
            }
            break;
        case wgpu::TextureDimension::e3D:
            mtlDesc.height = GetBaseSize().height;
            mtlDesc.depth = GetBaseSize().depthOrArrayLayers;
            mtlDesc.arrayLength = 1;
            DAWN_ASSERT(mtlDesc.sampleCount == 1);
            mtlDesc.textureType = MTLTextureType3D;
            break;
    }

    return mtlDescRef;
}

// static
ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                            const UnpackedPtr<TextureDescriptor>& descriptor) {
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor));
    DAWN_TRY(texture->InitializeAsInternalTexture(descriptor));

    if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        DAWN_TRY(texture->ClearTexture(ToBackend(device->GetQueue())->GetPendingCommandContext(),
                                       texture->GetAllSubresources(),
                                       TextureBase::ClearValue::NonZero));
    } else if (texture->ShouldKeepInitialized()) {
        DAWN_TRY(texture->ClearTexture(ToBackend(device->GetQueue())->GetPendingCommandContext(),
                                       texture->GetAllSubresources(),
                                       TextureBase::ClearValue::Zero));
    }

    return texture;
}

// static
ResultOrError<Ref<Texture>> Texture::CreateFromSharedTextureMemory(
    SharedTextureMemory* memory,
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    // Note: Initialized state is set on memory.BeginAccess, so we leave
    // subresources as uninitialized at this point.

    Device* device = ToBackend(memory->GetDevice());
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor));
    DAWN_TRY(texture->InitializeFromSharedTextureMemory(memory, descriptor));
    texture->mSharedResourceMemoryContents = memory->GetContents();
    return texture;
}

// static
Ref<Texture> Texture::CreateWrapping(Device* device,
                                     const UnpackedPtr<TextureDescriptor>& descriptor,
                                     NSPRef<id<MTLTexture>> wrapped) {
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor));
    texture->InitializeAsWrapping(descriptor, std::move(wrapped));
    return texture;
}

MaybeError Texture::InitializeAsInternalTexture(const UnpackedPtr<TextureDescriptor>& descriptor) {
    Device* device = ToBackend(GetDevice());

    if (!GetFormat().IsMultiPlanar()) {
        NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
        mMtlTextureType = [*mtlDesc textureType];
        mMtlUsage = [*mtlDesc usage];
        mMtlFormat = [*mtlDesc pixelFormat];
        mMtlPlaneTextures.resize(1);
        mMtlPlaneTextures[0] =
            AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:mtlDesc.Get()]);

        if (mMtlPlaneTextures[0] == nil) {
            return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate texture.");
        }
    } else {
        // Metal doesn't allow creating multiplanar texture directly. So we create an IOSurface
        // internally and wrap it.
        DAWN_ASSERT(descriptor->dimension == wgpu::TextureDimension::e2D &&
                    descriptor->mipLevelCount == 1 && descriptor->size.depthOrArrayLayers == 1);

        mIOSurface = CreateMultiPlanarIOSurface(descriptor->format, descriptor->size.width,
                                                descriptor->size.height);

        DAWN_INVALID_IF(mIOSurface == nullptr,
                        "Failed to create IOSurface for multiplanar texture.");
        DAWN_INVALID_IF(
            GetInternalUsage() & wgpu::TextureUsage::TransientAttachment,
            "Usage flags (%s) include %s, which is not compatible with creation from IOSurface.",
            GetInternalUsage(), wgpu::TextureUsage::TransientAttachment);

        mMtlTextureType = MTLTextureType2D;
        mMtlUsage = MetalTextureUsage(GetFormat(), GetInternalUsage());
        // Multiplanar format doesn't have equivalent MTLPixelFormat so just set it to invalid.
        mMtlFormat = MTLPixelFormatInvalid;
        const size_t numPlanes = IOSurfaceGetPlaneCount(GetIOSurface());
        mMtlPlaneTextures.resize(numPlanes);
        for (size_t plane = 0; plane < numPlanes; ++plane) {
            mMtlPlaneTextures[plane] = AcquireNSPRef(
                CreateTextureMtlForPlane(mMtlUsage, GetFormat(), plane, device, GetIOSurface()));
            if (mMtlPlaneTextures[plane] == nil) {
                return DAWN_INTERNAL_ERROR("Failed to create MTLTexture plane view for IOSurface.");
            }
        }
    }

    SetLabelImpl();

    return {};
}

void Texture::InitializeAsWrapping(const UnpackedPtr<TextureDescriptor>& descriptor,
                                   NSPRef<id<MTLTexture>> wrapped) {
    NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
    mMtlTextureType = [*mtlDesc textureType];
    mMtlUsage = [*mtlDesc usage];
    mMtlFormat = [*mtlDesc pixelFormat];
    mMtlPlaneTextures.resize(1);
    mMtlPlaneTextures[0] = std::move(wrapped);
    SetLabelImpl();
}

MaybeError Texture::InitializeFromSharedTextureMemory(
    SharedTextureMemory* memory,
    const UnpackedPtr<TextureDescriptor>& textureDescriptor) {
    DAWN_INVALID_IF(
        GetInternalUsage() & wgpu::TextureUsage::TransientAttachment,
        "Usage flags (%s) include %s, which is not compatible with creation from IOSurface.",
        GetInternalUsage(), wgpu::TextureUsage::TransientAttachment);

    mIOSurface = memory->GetIOSurface();
    mMtlTextureType = MTLTextureType2D;
    mMtlUsage = memory->GetMtlTextureUsage();
    mMtlFormat = memory->GetMtlPixelFormat();
    mMtlPlaneTextures = memory->GetMtlPlaneTextures();

    SetLabelImpl();

    return {};
}

void Texture::SynchronizeTextureBeforeUse(CommandRecordingContext* commandContext) {
    SharedTextureMemoryBase::PendingFenceList fences;
    SharedResourceMemoryContents* contents = GetSharedResourceMemoryContents();
    if (contents != nullptr) {
        contents->AcquirePendingFences(&fences);
    }
    for (const auto& fence : fences) {
        commandContext->WaitForSharedEvent(ToBackend(fence.object)->GetMTLSharedEvent(),
                                           fence.signaledValue);
    }

    mLastSharedTextureMemoryUsageSerial = GetDevice()->GetQueue()->GetPendingCommandSerial();
}

Texture::Texture(DeviceBase* dev, const UnpackedPtr<TextureDescriptor>& desc)
    : TextureBase(dev, desc) {}

Texture::~Texture() {}

void Texture::DestroyImpl(DestroyReason reason) {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    TextureBase::DestroyImpl(reason);
    mMtlPlaneTextures.clear();
    mIOSurface = nullptr;
}

void Texture::SetLabelImpl() {
    if (!GetFormat().IsMultiPlanar()) {
        DAWN_ASSERT(mMtlPlaneTextures.size() == 1);
        SetDebugName(GetDevice(), mMtlPlaneTextures[0].Get(), "Dawn_Texture", GetLabel());
    } else {
        for (size_t i = 0; i < mMtlPlaneTextures.size(); ++i) {
            SetDebugName(GetDevice(), mMtlPlaneTextures[i].Get(),
                         absl::StrFormat("Dawn_Plane_Texture[%zu]", i).c_str(), GetLabel());
        }
    }
}

MTLTextureType Texture::GetMTLTextureType() const {
    return mMtlTextureType;
}

MTLTextureUsage Texture::GetMTLTextureUsage() const {
    return mMtlUsage;
}

id<MTLTexture> Texture::GetMTLTexture(Aspect aspect) const {
    id<MTLTexture> tex;
    switch (aspect) {
        case Aspect::Plane0:
            DAWN_ASSERT(mMtlPlaneTextures.size() > 1);
            tex = mMtlPlaneTextures[0].Get();
            break;
        case Aspect::Plane1:
            DAWN_ASSERT(mMtlPlaneTextures.size() > 1);
            tex = mMtlPlaneTextures[1].Get();
            break;
        case Aspect::Plane2:
            DAWN_ASSERT(mMtlPlaneTextures.size() > 2);
            tex = mMtlPlaneTextures[2].Get();
            break;
        default:
            DAWN_ASSERT(mMtlPlaneTextures.size() == 1);
            tex = mMtlPlaneTextures[0].Get();
            break;
    }
    DAWN_ASSERT(tex != nullptr);
    return tex;
}

IOSurfaceRef Texture::GetIOSurface() {
    return mIOSurface.Get();
}

NSPRef<id<MTLTexture>> Texture::CreateFormatView(wgpu::TextureFormat format) {
    DAWN_ASSERT(!GetFormat().IsMultiPlanar());
    DAWN_ASSERT(mMtlFormat != MTLPixelFormatInvalid);

    if (GetFormat().format == format) {
        return mMtlPlaneTextures[0];
    }

    DAWN_ASSERT(
        AllowFormatReinterpretationWithoutFlag(mMtlFormat, MetalPixelFormat(GetDevice(), format)));
    return AcquireNSPRef([mMtlPlaneTextures[0].Get()
        newTextureViewWithPixelFormat:MetalPixelFormat(GetDevice(), format)]);
}

bool Texture::ShouldKeepInitialized() const {
    return GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) &&
           GetDevice()->IsToggleEnabled(
               Toggle::MetalKeepMultisubresourceDepthStencilTexturesInitialized) &&
           GetFormat().HasDepthOrStencil() && (GetArrayLayers() > 1 || GetNumMipLevels() > 1);
}

MaybeError Texture::ClearTexture(CommandRecordingContext* commandContext,
                                 const SubresourceRange& range,
                                 TextureBase::ClearValue clearValue) {
    Device* device = ToBackend(GetDevice());

    const uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
    const double dClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.0 : 1.0;

    if ((mMtlUsage & MTLTextureUsageRenderTarget) != 0) {
        DAWN_ASSERT(GetFormat().IsRenderable());

        // End the blit encoder if it is open.
        commandContext->EndBlit();

        if (GetFormat().HasDepthOrStencil()) {
            // Create a render pass to clear each subresource.
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                for (uint32_t arrayLayer = range.baseArrayLayer;
                     arrayLayer < range.baseArrayLayer + range.layerCount; arrayLayer++) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                            level, arrayLayer, range.aspects))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    // Note that this creates a descriptor that's autoreleased so we don't use
                    // AcquireNSRef
                    NSRef<MTLRenderPassDescriptor> descriptorRef =
                        [MTLRenderPassDescriptor renderPassDescriptor];
                    MTLRenderPassDescriptor* descriptor = descriptorRef.Get();

                    // At least one aspect needs clearing. Iterate the aspects individually to
                    // determine which to clear.
                    for (Aspect aspect : IterateEnumMask(range.aspects)) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, arrayLayer, aspect))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        DAWN_ASSERT(GetDimension() == wgpu::TextureDimension::e2D);
                        switch (aspect) {
                            case Aspect::Depth:
                                descriptor.depthAttachment.texture = GetMTLTexture(aspect);
                                descriptor.depthAttachment.level = level;
                                descriptor.depthAttachment.slice = arrayLayer;
                                descriptor.depthAttachment.loadAction = MTLLoadActionClear;
                                descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                                descriptor.depthAttachment.clearDepth = dClearColor;
                                break;
                            case Aspect::Stencil:
                                descriptor.stencilAttachment.texture = GetMTLTexture(aspect);
                                descriptor.stencilAttachment.level = level;
                                descriptor.stencilAttachment.slice = arrayLayer;
                                descriptor.stencilAttachment.loadAction = MTLLoadActionClear;
                                descriptor.stencilAttachment.storeAction = MTLStoreActionStore;
                                descriptor.stencilAttachment.clearStencil =
                                    static_cast<uint32_t>(clearColor);
                                break;
                            default:
                                DAWN_UNREACHABLE();
                        }
                    }

                    DAWN_TRY(EncodeEmptyMetalRenderPass(
                        device, commandContext, descriptor,
                        GetMipLevelSingleSubresourceVirtualSize(level, range.aspects)));
                }
            }
        } else if (GetFormat().IsMultiPlanar()) {
            DAWN_ASSERT(range.levelCount == 1);
            DAWN_ASSERT(range.layerCount == 1);
            DAWN_ASSERT(GetDimension() == wgpu::TextureDimension::e2D);
            DAWN_ASSERT(GetBaseSize().depthOrArrayLayers == 1);

            // At least one aspect needs clearing. Iterate the aspects individually to
            // determine which to clear.
            for (Aspect aspect : IterateEnumMask(range.aspects)) {
                if (clearValue == TextureBase::ClearValue::Zero &&
                    IsSubresourceContentInitialized(
                        SubresourceRange::SingleMipAndLayer(0, 0, aspect))) {
                    // Skip lazy clears if already initialized.
                    continue;
                }

                NSRef<MTLRenderPassDescriptor> descriptorRef =
                    [MTLRenderPassDescriptor renderPassDescriptor];
                MTLRenderPassDescriptor* descriptor = descriptorRef.Get();

                Extent3D aspectSize = GetMipLevelSingleSubresourcePhysicalSize(0, aspect);

                descriptor.colorAttachments[0].texture = GetMTLTexture(aspect);
                descriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
                descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
                descriptor.colorAttachments[0].clearColor =
                    MTLClearColorMake(dClearColor, dClearColor, dClearColor, dClearColor);

                DAWN_TRY(
                    EncodeEmptyMetalRenderPass(device, commandContext, descriptor, aspectSize));
            }

        } else {
            DAWN_ASSERT(GetFormat().IsColor());
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                // Create multiple render passes with each subresource as a color attachment to
                // clear them all. Only do this for array layers to ensure all attachments have
                // the same size.
                NSRef<MTLRenderPassDescriptor> descriptor;
                uint32_t attachment = 0;

                uint32_t depth = GetMipLevelSingleSubresourceVirtualSize(level, Aspect::Color)
                                     .depthOrArrayLayers;

                for (uint32_t arrayLayer = range.baseArrayLayer;
                     arrayLayer < range.baseArrayLayer + range.layerCount; arrayLayer++) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                            level, arrayLayer, Aspect::Color))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }

                    for (uint32_t z = 0; z < depth; ++z) {
                        if (descriptor == nullptr) {
                            // Note that this creates a descriptor that's autoreleased so we
                            // don't use AcquireNSRef
                            descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
                        }

                        [*descriptor colorAttachments][attachment].texture =
                            GetMTLTexture(Aspect::Color);
                        [*descriptor colorAttachments][attachment].loadAction = MTLLoadActionClear;
                        [*descriptor colorAttachments][attachment].storeAction =
                            MTLStoreActionStore;
                        [*descriptor colorAttachments][attachment].clearColor =
                            MTLClearColorMake(dClearColor, dClearColor, dClearColor, dClearColor);
                        [*descriptor colorAttachments][attachment].level = level;
                        [*descriptor colorAttachments][attachment].slice = arrayLayer;
                        [*descriptor colorAttachments][attachment].depthPlane = z;

                        attachment++;

                        if (attachment == kMaxColorAttachments) {
                            attachment = 0;
                            DAWN_TRY(EncodeEmptyMetalRenderPass(
                                device, commandContext, descriptor.Get(),
                                GetMipLevelSingleSubresourceVirtualSize(level, Aspect::Color)));
                            descriptor = nullptr;
                        }
                    }
                }

                if (descriptor != nullptr) {
                    DAWN_TRY(EncodeEmptyMetalRenderPass(
                        device, commandContext, descriptor.Get(),
                        GetMipLevelSingleSubresourceVirtualSize(level, Aspect::Color)));
                }
            }
        }
    } else {
        DAWN_ASSERT(!IsMultisampledTexture());

        // Encode a buffer to texture copy to clear each subresource.
        for (Aspect aspect : IterateEnumMask(range.aspects)) {
            // Compute the buffer size big enough to fill the largest mip.
            const TypedTexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(aspect).block;

            // Computations for the bytes per row / image height are done using the physical size
            // so that enough data is reserved for compressed textures.
            BlockExtent3D largestMipSize = blockInfo.ToBlock(
                GetMipLevelSingleSubresourcePhysicalSize(range.baseMipLevel, aspect));

            BlockCount uploadBlocks =
                largestMipSize.width * largestMipSize.height * largestMipSize.depthOrArrayLayers;
            uint64_t largestMipBytesPerRow = blockInfo.ToBytes(largestMipSize.width);
            uint64_t largestMipBytesPerImage =
                blockInfo.ToBytes(largestMipSize.width * largestMipSize.height);
            uint64_t uploadSize = blockInfo.ToBytes(uploadBlocks);

            DAWN_TRY(device->GetDynamicUploader()->WithUploadReservation(
                uploadSize, blockInfo.byteSize, [&](UploadReservation reservation) -> MaybeError {
                    memset(reservation.mappedPointer, clearColor, uploadSize);

                    id<MTLBuffer> buffer = ToBackend(reservation.buffer)->GetMTLBuffer();
                    for (uint32_t level = range.baseMipLevel;
                         level < range.baseMipLevel + range.levelCount; ++level) {
                        Extent3D virtualSize =
                            GetMipLevelSingleSubresourceVirtualSize(level, aspect);

                        for (uint32_t arrayLayer = range.baseArrayLayer;
                             arrayLayer < range.baseArrayLayer + range.layerCount; ++arrayLayer) {
                            if (clearValue == TextureBase::ClearValue::Zero &&
                                IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                                    level, arrayLayer, aspect))) {
                                // Skip lazy clears if already initialized.
                                continue;
                            }

                            MTLBlitOption blitOption = ComputeMTLBlitOption(aspect);
                            [commandContext->EnsureBlit()
                                     copyFromBuffer:buffer
                                       sourceOffset:reservation.offsetInBuffer
                                  sourceBytesPerRow:largestMipBytesPerRow
                                sourceBytesPerImage:largestMipBytesPerImage
                                         sourceSize:MTLSizeMake(virtualSize.width,
                                                                virtualSize.height,
                                                                virtualSize.depthOrArrayLayers)
                                          toTexture:GetMTLTexture(aspect)
                                   destinationSlice:arrayLayer
                                   destinationLevel:level
                                  destinationOrigin:MTLOriginMake(0, 0, 0)
                                            options:blitOption];
                        }
                    }

                    return {};
                }));
        }
    }
    return {};
}

MTLBlitOption Texture::ComputeMTLBlitOption(Aspect aspect) const {
    DAWN_ASSERT(HasOneBit(aspect));
    DAWN_ASSERT(GetFormat().aspects & aspect);

    if (mMtlFormat == MTLPixelFormatDepth32Float_Stencil8) {
        // We only provide a blit option if the format has both depth and stencil.
        // It is invalid to provide a blit option otherwise.
        switch (aspect) {
            case Aspect::Depth:
                return MTLBlitOptionDepthFromDepthStencil;
            case Aspect::Stencil:
                return MTLBlitOptionStencilFromDepthStencil;
            default:
                DAWN_UNREACHABLE();
        }
    }
    return MTLBlitOptionNone;
}

MaybeError Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                        const SubresourceRange& range) {
    if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        return {};
    }
    if (!IsSubresourceContentInitialized(range)) {
        // If subresource has not been initialized, clear it to black as it could
        // contain dirty bits from recycled memory
        DAWN_TRY(ClearTexture(commandContext, range, TextureBase::ClearValue::Zero));
        SetIsSubresourceContentInitialized(true, range);
        GetDevice()->IncrementLazyClearCountForTesting();
    }
    return {};
}

// static
ResultOrError<Ref<TextureView>> TextureView::Create(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    Ref<TextureView> view = AcquireRef(new TextureView(texture, descriptor));
    DAWN_TRY(view->Initialize(descriptor));
    return view;
}

MaybeError TextureView::Initialize(const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    DeviceBase* device = GetDevice();
    Texture* texture = ToBackend(GetTexture());

    // Texture could be destroyed by the time we make a view.
    if (GetTexture()->IsDestroyed()) {
        return {};
    }

    Aspect aspect = SelectFormatAspects(texture->GetFormat(), descriptor->aspect);
    id<MTLTexture> mtlTexture = texture->GetMTLTexture(aspect);

    if (texture->GetFormat().IsMultiPlanar()) {
        // For multiplanar texture, plane view is already created in
        // InitializeFromInternalMultiPlanarTexture(). The view is only nullptr if aspect is
        // invalid.
        DAWN_ASSERT(mtlTexture != nullptr);
        mMtlTextureView = mtlTexture;
    } else {
        MTLTextureType textureViewType =
            MetalTextureViewType(descriptor->dimension, texture->GetSampleCount());
        std::optional<MTLTextureSwizzleChannels> mtlSwizzle = ComputeMetalSwizzle();

        MTLPixelFormat viewFormat = MetalPixelFormat(device, descriptor->format);
        MTLPixelFormat textureFormat = MetalPixelFormat(device, texture->GetFormat().format);
        if (aspect == Aspect::Stencil && textureFormat != MTLPixelFormatStencil8) {
            // Note we never use MTLPixelFormatDepth24Unorm_Stencil8 (doesn't exist on Apple GPUs).
            DAWN_ASSERT(textureFormat == MTLPixelFormatDepth32Float_Stencil8);
            viewFormat = MTLPixelFormatX32_Stencil8;
        } else if (texture->GetFormat().HasDepth() && texture->GetFormat().HasStencil()) {
            // Depth-only views for depth/stencil textures in Metal simply use the original
            // texture's format.
            viewFormat = textureFormat;
        }

        if (!RequiresCreatingNewTextureView(texture, viewFormat != textureFormat,
                                            GetInternalUsage(), descriptor, textureViewType,
                                            mtlSwizzle)) {
            mMtlTextureView = mtlTexture;
        } else {
            auto mipLevelRange = NSMakeRange(descriptor->baseMipLevel, descriptor->mipLevelCount);
            auto arrayLayerRange =
                NSMakeRange(descriptor->baseArrayLayer, descriptor->arrayLayerCount);

            if (mtlSwizzle) {
                mMtlTextureView =
                    AcquireNSPRef([mtlTexture newTextureViewWithPixelFormat:viewFormat
                                                                textureType:textureViewType
                                                                     levels:mipLevelRange
                                                                     slices:arrayLayerRange
                                                                    swizzle:mtlSwizzle.value()]);
            } else {
                mMtlTextureView =
                    AcquireNSPRef([mtlTexture newTextureViewWithPixelFormat:viewFormat
                                                                textureType:textureViewType
                                                                     levels:mipLevelRange
                                                                     slices:arrayLayerRange]);
            }

            if (mMtlTextureView == nil) {
                return DAWN_INTERNAL_ERROR("Failed to create MTLTexture view.");
            }
        }
    }

    SetLabelImpl();
    return {};
}

void TextureView::DestroyImpl(DestroyReason reason) {
    mMtlTextureView = nil;
}

void TextureView::SetLabelImpl() {
    SetDebugName(GetDevice(), mMtlTextureView.Get(), "Dawn_TextureView", GetLabel());
}

id<MTLTexture> TextureView::GetMTLTexture() const {
    DAWN_ASSERT(mMtlTextureView != nullptr);
    return mMtlTextureView.Get();
}

TextureView::AttachmentInfo TextureView::GetAttachmentInfo() const {
    DAWN_ASSERT(GetInternalUsage() &
                (wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageAttachment));
    // Use our own view if the formats do not match.
    // If the formats do not match, format reinterpretation will be required.
    // Note: Depth/stencil formats don't support reinterpretation.
    // Also, we compute |useOwnView| here instead of relying on whether or not
    // a view was created in Initialize, because rendering to a depth/stencil
    // texture on Metal only works when using the original texture, not a view.
    bool useOwnView = GetFormat().format != GetTexture()->GetFormat().format &&
                      !GetTexture()->GetFormat().HasDepthOrStencil();
    if (useOwnView) {
        DAWN_ASSERT(mMtlTextureView.Get());
        return {mMtlTextureView, 0, 0};
    }
    AttachmentInfo info;
    info.texture = ToBackend(GetTexture())->GetMTLTexture(GetTexture()->GetFormat().aspects);
    info.baseMipLevel = GetBaseMipLevel();
    info.baseArrayLayer = GetBaseArrayLayer();
    return info;
}

std::optional<MTLTextureSwizzleChannels> TextureView::ComputeMetalSwizzle() {
    if (!SupportTextureComponentSwizzle(ToBackend(GetDevice())->GetMTLDevice())) {
        // If the device doesn't support texture component swizzling,
        // we should have caught this during validation.
        DAWN_ASSERT(GetSwizzle() == kRGBASwizzle);
        return {};
    }

    // A backend swizzle is always used for depth-stencil if supported.
    // If not, we returned {} above so G/B/A will be undefined.
    if (GetTexture()->GetFormat().HasDepthOrStencil()) {
        return ToMetalTextureSwizzleChannels(ComposeSwizzle(kR001Swizzle, GetSwizzle()));
    }

    // Note: For formats with <4 channels, this could be refined to consider that some channels are
    // nonexistent. We don't bother to optimize that, because such swizzles are not actually useful.
    if (GetSwizzle() != kRGBASwizzle) {
        return ToMetalTextureSwizzleChannels(GetSwizzle());
    }

    return {};
}

}  // namespace dawn::native::metal
