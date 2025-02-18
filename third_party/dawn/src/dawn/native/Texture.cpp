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

#include "dawn/native/Texture.h"

#include <algorithm>
#include <utility>

#include "absl/strings/str_format.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BlitTextureToBuffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/SharedTextureMemory.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {

MaybeError ValidateTextureViewFormatCompatibility(const DeviceBase* device,
                                                  const Format& format,
                                                  wgpu::TextureFormat viewFormatEnum) {
    const Format* viewFormat;
    DAWN_TRY_ASSIGN(viewFormat, device->GetInternalFormat(viewFormatEnum));

    DAWN_INVALID_IF(!format.ViewCompatibleWith(*viewFormat),
                    "The texture view format (%s) is not texture view format compatible "
                    "with the texture format (%s).",
                    viewFormatEnum, format.format);

    DAWN_INVALID_IF(device->IsCompatibilityMode() && viewFormat->format != format.format,
                    "viewFormat (%s) must match format (%s) in compatibility mode.",
                    viewFormat->format, format.format);
    return {};
}

MaybeError ValidateCanViewTextureAs(const DeviceBase* device,
                                    const TextureBase* texture,
                                    const Format& viewFormat,
                                    wgpu::TextureAspect aspect) {
    const Format& format = texture->GetFormat();

    if (aspect != wgpu::TextureAspect::All) {
        wgpu::TextureFormat aspectFormat = format.GetAspectInfo(aspect).format;
        if (viewFormat.format == aspectFormat) {
            return {};
        } else {
            return DAWN_VALIDATION_ERROR(
                "The view format (%s) is not compatible with %s of %s (%s).", viewFormat.format,
                aspect, format.format, aspectFormat);
        }
    }

    if (format.format == viewFormat.format) {
        return {};
    }

    const FormatSet& compatibleViewFormats = texture->GetViewFormats();
    if (compatibleViewFormats[viewFormat]) {
        // Validation of this list is done on texture creation, so we don't need to
        // handle the case where a format is in the list, but not compatible.
        return {};
    }

    // |viewFormat| is not in the list. Check compatibility to generate an error message
    // depending on whether it could be compatible, but needs to be explicitly listed,
    // or it could never be compatible.
    if (!format.ViewCompatibleWith(viewFormat)) {
        // The view format isn't compatible with the format at all. Return an error
        // that indicates this, in addition to reporting that it's missing from the
        // list.
        return DAWN_VALIDATION_ERROR(
            "The texture view format (%s) is not compatible with the "
            "texture format (%s)."
            "The formats must be compatible, and the view format "
            "must be passed in the list of view formats on texture creation.",
            viewFormat.format, format.format);
    }

    // The view format is compatible, but not in the list.
    return DAWN_VALIDATION_ERROR(
        "%s was not created with the texture view format (%s) "
        "in the list of compatible view formats.",
        texture, viewFormat.format);
}

bool IsTextureViewDimensionCompatibleWithTextureDimension(
    wgpu::TextureViewDimension textureViewDimension,
    wgpu::TextureDimension textureDimension) {
    switch (textureViewDimension) {
        case wgpu::TextureViewDimension::e2D:
        case wgpu::TextureViewDimension::e2DArray:
        case wgpu::TextureViewDimension::Cube:
        case wgpu::TextureViewDimension::CubeArray:
            return textureDimension == wgpu::TextureDimension::e2D;

        case wgpu::TextureViewDimension::e3D:
            return textureDimension == wgpu::TextureDimension::e3D;

        case wgpu::TextureViewDimension::e1D:
            return textureDimension == wgpu::TextureDimension::e1D;

        case wgpu::TextureViewDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateDepthOrArrayLayersIsCompatibleWithTextureBindingViewDimension(
    wgpu::TextureViewDimension textureBindingViewDimension,
    uint32_t depthOrArrayLayers) {
    switch (textureBindingViewDimension) {
        case wgpu::TextureViewDimension::e2D:
            if (depthOrArrayLayers != 1) {
                return DAWN_VALIDATION_ERROR(
                    "A resolved TextureViewDimension of e2D is only "
                    "compatible with depthOrArrayLayers equals 1.");
            }
            break;
        case wgpu::TextureViewDimension::Cube:
            if (depthOrArrayLayers != 6) {
                return DAWN_VALIDATION_ERROR(
                    "A resolved TextureViewDimension of Cube is only "
                    "compatible with depthOrArrayLayers equals 6.");
            }
            break;
        default:
            break;
    }
    return {};
}

bool IsArrayLayerValidForTextureViewDimension(wgpu::TextureViewDimension textureViewDimension,
                                              uint32_t textureViewArrayLayer) {
    switch (textureViewDimension) {
        case wgpu::TextureViewDimension::e2D:
        case wgpu::TextureViewDimension::e3D:
            return textureViewArrayLayer == 1u;
        case wgpu::TextureViewDimension::e2DArray:
            return true;
        case wgpu::TextureViewDimension::Cube:
            return textureViewArrayLayer == 6u;
        case wgpu::TextureViewDimension::CubeArray:
            return textureViewArrayLayer % 6 == 0;
        case wgpu::TextureViewDimension::e1D:
            return textureViewArrayLayer == 1u;

        case wgpu::TextureViewDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateSampleCount(const TextureDescriptor* descriptor,
                               wgpu::TextureUsage usage,
                               const Format* format) {
    DAWN_INVALID_IF(!IsValidSampleCount(descriptor->sampleCount),
                    "The sample count (%u) of the texture is not supported.",
                    descriptor->sampleCount);

    if (descriptor->sampleCount > 1) {
        DAWN_INVALID_IF(descriptor->mipLevelCount > 1,
                        "The mip level count (%u) of a multisampled texture is not 1.",
                        descriptor->mipLevelCount);

        // Multisampled 1D and 3D textures are not supported in D3D12/Metal/Vulkan.
        // Multisampled 2D array texture is not supported because on Metal it requires the
        // version of macOS be greater than 10.14.
        DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                        "The dimension (%s) of a multisampled texture is not 2D.",
                        descriptor->dimension);

        DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers > 1,
                        "The depthOrArrayLayers (%u) of a multisampled texture is not 1.",
                        descriptor->size.depthOrArrayLayers);

        DAWN_INVALID_IF(!format->supportsMultisample,
                        "The texture format (%s) does not support multisampling.", format->format);

        // Compressed formats are not renderable. They cannot support multisample.
        DAWN_ASSERT(!format->isCompressed);

        DAWN_INVALID_IF(usage & wgpu::TextureUsage::StorageBinding,
                        "The sample count (%u) of a storage texture is not 1.",
                        descriptor->sampleCount);
        DAWN_INVALID_IF(usage & wgpu::TextureUsage::StorageAttachment,
                        "The sample count (%u) of a storage attachment texture is not 1.",
                        descriptor->sampleCount);

        DAWN_INVALID_IF((usage & wgpu::TextureUsage::RenderAttachment) == 0,
                        "The usage (%s) of a multisampled texture doesn't include (%s).",
                        descriptor->usage, wgpu::TextureUsage::RenderAttachment);
    }

    return {};
}

MaybeError ValidateTextureViewDimensionCompatibility(
    const DeviceBase* device,
    const TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    DAWN_INVALID_IF(!IsArrayLayerValidForTextureViewDimension(descriptor->dimension,
                                                              descriptor->arrayLayerCount),
                    "The dimension (%s) of the texture view is not compatible with the layer count "
                    "(%u) of %s.",
                    descriptor->dimension, descriptor->arrayLayerCount, texture);

    DAWN_INVALID_IF(
        !IsTextureViewDimensionCompatibleWithTextureDimension(descriptor->dimension,
                                                              texture->GetDimension()),
        "The dimension (%s) of the texture view is not compatible with the dimension (%s) "
        "of %s.",
        descriptor->dimension, texture->GetDimension(), texture);

    DAWN_INVALID_IF(
        texture->GetSampleCount() > 1 && descriptor->dimension != wgpu::TextureViewDimension::e2D,
        "The dimension (%s) of the multisampled texture view is not %s.", descriptor->dimension,
        wgpu::TextureViewDimension::e2D);

    switch (descriptor->dimension) {
        case wgpu::TextureViewDimension::Cube:
        case wgpu::TextureViewDimension::CubeArray:
            DAWN_INVALID_IF(
                texture->GetSize(descriptor->aspect).width !=
                    texture->GetSize(descriptor->aspect).height,
                "A %s texture view is not compatible with %s because the texture's width "
                "(%u) and height (%u) are not equal.",
                descriptor->dimension, texture, texture->GetSize(descriptor->aspect).width,
                texture->GetSize(descriptor->aspect).height);
            DAWN_INVALID_IF(descriptor->dimension == wgpu::TextureViewDimension::CubeArray &&
                                device->IsCompatibilityMode(),
                            "A %s texture view for %s is not supported in compatibility mode",
                            descriptor->dimension, texture);
            break;
        case wgpu::TextureViewDimension::e1D:
        case wgpu::TextureViewDimension::e2D:
        case wgpu::TextureViewDimension::e2DArray:
        case wgpu::TextureViewDimension::e3D:
            break;

        case wgpu::TextureViewDimension::Undefined:
            DAWN_UNREACHABLE();
    }

    return {};
}

MaybeError ValidateTextureSize(const DeviceBase* device,
                               const TextureDescriptor* descriptor,
                               const Format* format) {
    DAWN_ASSERT(descriptor->size.width != 0 && descriptor->size.height != 0 &&
                descriptor->size.depthOrArrayLayers != 0);
    const CombinedLimits& limits = device->GetLimits();
    Extent3D maxExtent;
    switch (descriptor->dimension) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();
        case wgpu::TextureDimension::e1D:
            maxExtent = {limits.v1.maxTextureDimension1D, 1, 1};
            break;
        case wgpu::TextureDimension::e2D:
            maxExtent = {limits.v1.maxTextureDimension2D, limits.v1.maxTextureDimension2D,
                         limits.v1.maxTextureArrayLayers};
            break;
        case wgpu::TextureDimension::e3D:
            maxExtent = {limits.v1.maxTextureDimension3D, limits.v1.maxTextureDimension3D,
                         limits.v1.maxTextureDimension3D};
            break;
    }

    if (DAWN_UNLIKELY(descriptor->size.width > maxExtent.width ||
                      descriptor->size.height > maxExtent.height ||
                      descriptor->size.depthOrArrayLayers > maxExtent.depthOrArrayLayers)) {
        SupportedLimits adapterLimits;
        wgpu::Status status = device->GetAdapter()->APIGetLimits(&adapterLimits);
        DAWN_ASSERT(status == wgpu::Status::Success);

        Extent3D maxExtentAdapter;
        StringView limitName;
        uint32_t limitValue;
        switch (descriptor->dimension) {
            case wgpu::TextureDimension::Undefined:
                DAWN_UNREACHABLE();
            case wgpu::TextureDimension::e1D:
                maxExtentAdapter = {adapterLimits.limits.maxTextureDimension1D, 1, 1};
                limitName = "maxTextureDimension1D";
                limitValue = adapterLimits.limits.maxTextureDimension1D;
                break;
            case wgpu::TextureDimension::e2D:
                maxExtentAdapter = {adapterLimits.limits.maxTextureDimension2D,
                                    adapterLimits.limits.maxTextureDimension2D,
                                    adapterLimits.limits.maxTextureArrayLayers};
                if (descriptor->size.width > maxExtent.width ||
                    descriptor->size.height > maxExtent.height) {
                    limitName = "maxTextureDimension2D";
                    limitValue = adapterLimits.limits.maxTextureDimension2D;
                } else {
                    limitName = "maxTextureArrayLayers";
                    limitValue = adapterLimits.limits.maxTextureArrayLayers;
                }
                break;
            case wgpu::TextureDimension::e3D:
                maxExtentAdapter = {adapterLimits.limits.maxTextureDimension3D,
                                    adapterLimits.limits.maxTextureDimension3D,
                                    adapterLimits.limits.maxTextureDimension3D};
                limitName = "maxTextureDimension3D";
                limitValue = adapterLimits.limits.maxTextureDimension3D;
                break;
        }

        std::string increaseLimitAdvice =
            (descriptor->size.width <= maxExtentAdapter.width &&
             descriptor->size.height <= maxExtentAdapter.height &&
             descriptor->size.depthOrArrayLayers <= maxExtentAdapter.depthOrArrayLayers)
                ? MakeIncreaseLimitMessage(limitName, limitValue)
                : "";
        return DAWN_VALIDATION_ERROR("Texture size (%s) exceeded maximum texture size (%s).%s",
                                     &descriptor->size, &maxExtent, increaseLimitAdvice);
    }

    switch (descriptor->dimension) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();
        case wgpu::TextureDimension::e1D:
            DAWN_INVALID_IF(descriptor->mipLevelCount != 1,
                            "Texture mip level count (%u) is more than 1 when its dimension is %s.",
                            descriptor->mipLevelCount, wgpu::TextureDimension::e1D);
            break;
        case wgpu::TextureDimension::e2D: {
            uint32_t maxMippedDimension = std::max(descriptor->size.width, descriptor->size.height);
            DAWN_INVALID_IF(
                Log2(maxMippedDimension) + 1 < descriptor->mipLevelCount,
                "Texture mip level count (%u) exceeds the maximum (%u) for its size (%s).",
                descriptor->mipLevelCount, Log2(maxMippedDimension) + 1, &descriptor->size);
            break;
        }
        case wgpu::TextureDimension::e3D: {
            uint32_t maxMippedDimension =
                std::max(descriptor->size.width,
                         std::max(descriptor->size.height, descriptor->size.depthOrArrayLayers));
            DAWN_INVALID_IF(
                Log2(maxMippedDimension) + 1 < descriptor->mipLevelCount,
                "Texture mip level count (%u) exceeds the maximum (%u) for its size (%s).",
                descriptor->mipLevelCount, Log2(maxMippedDimension) + 1, &descriptor->size);
            break;
        }
    }

    if (format->isCompressed) {
        const TexelBlockInfo& blockInfo = format->GetAspectInfo(wgpu::TextureAspect::All).block;
        DAWN_INVALID_IF(
            descriptor->size.width % blockInfo.width != 0 ||
                descriptor->size.height % blockInfo.height != 0,
            "The size (%s) of the texture is not a multiple of the block width (%u) and "
            "height (%u) of the texture format (%s).",
            &descriptor->size, blockInfo.width, blockInfo.height, format->format);
    }

    return {};
}

MaybeError ValidateTextureUsage(const DeviceBase* device,
                                wgpu::TextureDimension textureDimension,
                                wgpu::TextureUsage usage,
                                const Format* format,
                                std::optional<wgpu::TextureUsage> allowedSharedTextureMemoryUsage) {
    DAWN_TRY(dawn::native::ValidateTextureUsage(usage));

    DAWN_INVALID_IF(usage == wgpu::TextureUsage::None, "The texture usage must not be 0.");

    constexpr wgpu::TextureUsage kValidCompressedUsages = wgpu::TextureUsage::TextureBinding |
                                                          wgpu::TextureUsage::CopySrc |
                                                          wgpu::TextureUsage::CopyDst;
    DAWN_INVALID_IF(
        format->isCompressed && !IsSubset(usage, kValidCompressedUsages),
        "The texture usage (%s) is incompatible with the compressed texture format (%s).", usage,
        format->format);

    DAWN_INVALID_IF(
        !format->isRenderable && (usage & wgpu::TextureUsage::RenderAttachment),
        "The texture usage (%s) includes %s, which is incompatible with the non-renderable "
        "format (%s).",
        usage, wgpu::TextureUsage::RenderAttachment, format->format);

    DAWN_INVALID_IF(textureDimension == wgpu::TextureDimension::e1D &&
                        (usage & wgpu::TextureUsage::RenderAttachment),
                    "The texture usage (%s) includes %s, which is incompatible with the texture "
                    "dimension (%s).",
                    usage, wgpu::TextureUsage::RenderAttachment, textureDimension);

    DAWN_INVALID_IF(
        !format->supportsStorageUsage && (usage & wgpu::TextureUsage::StorageBinding),
        "The texture usage (%s) includes %s, which is incompatible with the format (%s).", usage,
        wgpu::TextureUsage::StorageBinding, format->format);

    DAWN_INVALID_IF(
        !format->supportsStorageAttachment && (usage & wgpu::TextureUsage::StorageAttachment),
        "The texture usage (%s) includes %s, which is incompatible with the format (%s).", usage,
        wgpu::TextureUsage::StorageAttachment, format->format);

    const auto kTransientAttachment = wgpu::TextureUsage::TransientAttachment;
    if (usage & kTransientAttachment) {
        DAWN_INVALID_IF(
            !device->HasFeature(Feature::TransientAttachments),
            "The texture usage (%s) includes %s, which requires the %s feature to be set", usage,
            kTransientAttachment, ToAPI(Feature::TransientAttachments));

        DAWN_INVALID_IF(
            usage == kTransientAttachment,
            "The texture usage is only %s (which always requires another attachment usage).",
            kTransientAttachment);
        const auto kAttachmentUsages = kTransientAttachment | wgpu::TextureUsage::RenderAttachment |
                                       wgpu::TextureUsage::StorageAttachment;
        DAWN_INVALID_IF(!IsSubset(usage, kAttachmentUsages),
                        "The texture usage (%s) includes both %s and non-attachment usages (%s).",
                        usage, kTransientAttachment, usage & ~kAttachmentUsages);
    }

    if (!allowedSharedTextureMemoryUsage) {
        // Legacy path
        // TODO(crbug.com/dawn/1795): Remove after migrating all old usages.
        // Only allows simple readonly texture usages.
        wgpu::TextureUsage validMultiPlanarUsages =
            wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
        if (device->HasFeature(Feature::MultiPlanarFormatExtendedUsages)) {
            validMultiPlanarUsages |= wgpu::TextureUsage::CopyDst;
        }
        if (device->HasFeature(Feature::MultiPlanarRenderTargets)) {
            validMultiPlanarUsages |= wgpu::TextureUsage::RenderAttachment;
        }
        DAWN_INVALID_IF(format->IsMultiPlanar() && !IsSubset(usage, validMultiPlanarUsages),
                        "The texture usage (%s) is incompatible with the multi-planar format (%s).",
                        usage, format->format);
    } else {
        DAWN_INVALID_IF(
            !IsSubset(usage, *allowedSharedTextureMemoryUsage),
            "The texture usage (%s) is not a subset of the shared texture memory usage (%s).",
            usage, *allowedSharedTextureMemoryUsage);
    }
    return {};
}

wgpu::TextureUsage GetTextureViewUsage(wgpu::TextureUsage sourceTextureUsage,
                                       wgpu::TextureUsage requestedViewUsage) {
    // If a view's requested usage is None, inherit usage from the source texture.
    return (requestedViewUsage != wgpu::TextureUsage::None) ? requestedViewUsage
                                                            : sourceTextureUsage;
}

wgpu::TextureUsage RemoveInvalidViewUsages(wgpu::TextureUsage viewUsage, const Format* viewFormat) {
    wgpu::TextureUsage adjustedUsage = viewUsage;
    if (viewFormat->format == wgpu::TextureFormat::RGBA8UnormSrgb ||
        viewFormat->format == wgpu::TextureFormat::BGRA8UnormSrgb) {
        adjustedUsage = viewUsage & ~wgpu::TextureUsage::StorageBinding;
    }

    return adjustedUsage;
}

MaybeError ValidateTextureViewUsage(const DeviceBase* device,
                                    const TextureBase* texture,
                                    wgpu::TextureUsage usage,
                                    const Format* format) {
    wgpu::TextureUsage inheritedUsage = GetTextureViewUsage(texture->GetUsage(), usage);

    DAWN_INVALID_IF(!IsSubset(inheritedUsage, texture->GetUsage()),
                    "The texture view usage (%s) is not a subset of the texture usage (%s).",
                    inheritedUsage, texture->GetUsage());

    // Validate the view usage only when it is explicitly requested for now because it is not yet
    // possible to request view usage all the way from the WebGPU API.
    if (usage != wgpu::TextureUsage::None) {
        DAWN_TRY(ValidateTextureUsage(device, texture->GetDimension(), inheritedUsage, format, {}));
    }

    return {};
}

// We need to add an internal RenderAttachment usage to some textures that has CopyDst usage as we
// apply a workaround that writes to them with a render pipeline.
bool CopyDstNeedsInternalRenderAttachmentUsage(const DeviceBase* device, const Format& format) {
    // Depth
    if (format.HasDepth() &&
        (device->IsToggleEnabled(Toggle::UseBlitForBufferToDepthTextureCopy) ||
         device->IsToggleEnabled(
             Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource))) {
        return true;
    }
    // Stencil
    if (format.HasStencil() &&
        device->IsToggleEnabled(Toggle::UseBlitForBufferToStencilTextureCopy)) {
        return true;
    }
    return false;
}

// We need to add an internal TextureBinding usage to some textures that has CopySrc usage as we
// apply a workaround that binds them to a compute pipeline for their copy operation.
bool CopySrcNeedsInternalTextureBindingUsage(const DeviceBase* device, const Format& format) {
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
    // RG11B10ufloat
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
    if (format.HasDepth() &&
        (device->IsToggleEnabled(Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource) ||
         (format.format == wgpu::TextureFormat::Depth16Unorm &&
          device->IsToggleEnabled(Toggle::UseBlitForDepth16UnormTextureToBufferCopy)) ||
         (format.format == wgpu::TextureFormat::Depth32Float &&
          device->IsToggleEnabled(Toggle::UseBlitForDepth32FloatTextureToBufferCopy)))) {
        return true;
    }
    // Stencil
    if (format.HasStencil() &&
        device->IsToggleEnabled(Toggle::UseBlitForStencilTextureToBufferCopy)) {
        return true;
    }

    if (device->IsToggleEnabled(Toggle::UseBlitForT2B) &&
        IsFormatSupportedByTextureToBufferBlit(format.format)) {
        return true;
    }

    return false;
}

wgpu::TextureViewDimension ResolveDefaultCompatiblityTextureBindingViewDimension(
    const DeviceBase* device,
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    // In non-compatibility mode this value is not used so return undefined so that it is not
    // used by mistake.
    if (device->HasFlexibleTextureViews()) {
        return wgpu::TextureViewDimension::Undefined;
    }

    auto textureBindingViewDimension = wgpu::TextureViewDimension::Undefined;
    if (auto* subDesc = descriptor.Get<TextureBindingViewDimensionDescriptor>()) {
        textureBindingViewDimension = subDesc->textureBindingViewDimension;
    }
    if (textureBindingViewDimension != wgpu::TextureViewDimension::Undefined) {
        return textureBindingViewDimension;
    }

    switch (descriptor->dimension) {
        case wgpu::TextureDimension::e1D:
            return wgpu::TextureViewDimension::e1D;
        case wgpu::TextureDimension::e2D:
            return descriptor->size.depthOrArrayLayers == 1 ? wgpu::TextureViewDimension::e2D
                                                            : wgpu::TextureViewDimension::e2DArray;
        case wgpu::TextureDimension::e3D:
            return wgpu::TextureViewDimension::e3D;
        case wgpu::TextureDimension::Undefined:
        default:
            DAWN_UNREACHABLE();
    }
}

wgpu::TextureUsage AddInternalUsages(const DeviceBase* device,
                                     wgpu::TextureUsage usage,
                                     const Format& format,
                                     uint32_t sampleCount,
                                     uint32_t mipLevelCount,
                                     uint32_t arrayLayerCount) {
    wgpu::TextureUsage internalUsage = usage;

    // dawn:1569: If a texture with multiple array layers or mip levels is specified as a
    // texture attachment when this toggle is active, it needs to be given CopySrc | CopyDst usage
    // internally.
    bool applyAlwaysResolveIntoZeroLevelAndLayerToggle =
        device->IsToggleEnabled(Toggle::AlwaysResolveIntoZeroLevelAndLayer) &&
        (arrayLayerCount > 1 || mipLevelCount > 1) &&
        (internalUsage & wgpu::TextureUsage::RenderAttachment);
    if (applyAlwaysResolveIntoZeroLevelAndLayerToggle) {
        internalUsage |= wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    }

    if (internalUsage & wgpu::TextureUsage::CopyDst) {
        if (CopyDstNeedsInternalRenderAttachmentUsage(device, format)) {
            internalUsage |= wgpu::TextureUsage::RenderAttachment;
        }
    }
    if (internalUsage & wgpu::TextureUsage::CopySrc) {
        if (CopySrcNeedsInternalTextureBindingUsage(device, format)) {
            internalUsage |= wgpu::TextureUsage::TextureBinding;
        }
    }
    if (internalUsage & wgpu::TextureUsage::StorageBinding) {
        internalUsage |= kReadOnlyStorageTexture | kWriteOnlyStorageTexture;
    }

    bool supportsMSAAPartialResolve = device->HasFeature(Feature::DawnPartialLoadResolveTexture) &&
                                      sampleCount > 1 &&
                                      (usage & wgpu::TextureUsage::RenderAttachment);
    if (supportsMSAAPartialResolve) {
        internalUsage |= wgpu::TextureUsage::TextureBinding;
    }

    return internalUsage;
}

}  // anonymous namespace

MaybeError ValidateTextureDescriptor(
    const DeviceBase* device,
    const UnpackedPtr<TextureDescriptor>& descriptor,
    AllowMultiPlanarTextureFormat allowMultiPlanar,
    std::optional<wgpu::TextureUsage> allowedSharedTextureMemoryUsage) {
    wgpu::TextureUsage usage = descriptor->usage;
    if (auto* internalUsageDesc = descriptor.Get<DawnTextureInternalUsageDescriptor>()) {
        DAWN_INVALID_IF(
            !device->HasFeature(Feature::DawnInternalUsages),
            "The internalUsageDesc is not empty while the dawn-internal-usages feature is not "
            "enabled");
        usage |= internalUsageDesc->internalUsage;
    }

    const Format* format;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));

    if (format->IsMultiPlanar()) {
        switch (allowMultiPlanar) {
            case AllowMultiPlanarTextureFormat::Yes:
                DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D ||
                                    descriptor->mipLevelCount != 1,
                                "Multiplanar texture must be non-mipmapped & 2D in order to be "
                                "created directly.");
                break;
            case AllowMultiPlanarTextureFormat::SingleLayerOnly:
                DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D ||
                                    descriptor->mipLevelCount != 1 ||
                                    descriptor->size.depthOrArrayLayers != 1,
                                "Multiplanar texture must be non-mipmapped & 2D in order to be "
                                "created directly.");
                break;
            case AllowMultiPlanarTextureFormat::No:
                return DAWN_VALIDATION_ERROR(
                    "Creation of multiplanar texture format %s is not allowed.",
                    descriptor->format);
        }
    }

    for (uint32_t i = 0; i < descriptor->viewFormatCount; ++i) {
        DAWN_TRY_CONTEXT(
            ValidateTextureViewFormatCompatibility(device, *format, descriptor->viewFormats[i]),
            "validating viewFormats[%u]", i);
    }

    DAWN_TRY(ValidateTextureUsage(device, descriptor->dimension, usage, format,
                                  std::move(allowedSharedTextureMemoryUsage)));
    DAWN_TRY(ValidateTextureDimension(descriptor->dimension));
    if (!device->HasFlexibleTextureViews()) {
        const auto textureBindingViewDimension =
            ResolveDefaultCompatiblityTextureBindingViewDimension(device, descriptor);
        DAWN_TRY_CONTEXT(ValidateTextureViewDimension(textureBindingViewDimension),
                         "validating resolved compatibility textureBindingViewDimension");

        DAWN_INVALID_IF(
            !IsTextureViewDimensionCompatibleWithTextureDimension(textureBindingViewDimension,
                                                                  descriptor->dimension),
            "The textureBindingViewDimension (%s) is not compatible with the dimension (%s)",
            textureBindingViewDimension, descriptor->dimension);

        DAWN_TRY(ValidateDepthOrArrayLayersIsCompatibleWithTextureBindingViewDimension(
            textureBindingViewDimension, descriptor->size.depthOrArrayLayers));
    }
    DAWN_TRY(ValidateSampleCount(*descriptor, usage, format));

    DAWN_INVALID_IF(descriptor->size.width == 0 || descriptor->size.height == 0 ||
                        descriptor->size.depthOrArrayLayers == 0 || descriptor->mipLevelCount == 0,
                    "The texture size (%s) or mipLevelCount (%u) is empty.", &descriptor->size,
                    descriptor->mipLevelCount);

    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D && format->isCompressed,
                    "The dimension (%s) of a texture with a compressed format (%s) is not 2D.",
                    descriptor->dimension, format->format);

    // Depth/stencil formats are valid for 2D textures only. Metal has this limit. And D3D12
    // doesn't support depth/stencil formats on 3D textures.
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D &&
                        (format->aspects & (Aspect::Depth | Aspect::Stencil)),
                    "The dimension (%s) of a texture with a depth/stencil format (%s) is not 2D.",
                    descriptor->dimension, format->format);

    DAWN_TRY(ValidateTextureSize(device, *descriptor, format));
    return {};
}

MaybeError ValidateTextureViewDescriptor(const DeviceBase* device,
                                         const TextureBase* texture,
                                         const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    // Parent texture should have been already validated.
    DAWN_ASSERT(texture);
    DAWN_ASSERT(!texture->IsError());

    DAWN_TRY(ValidateTextureViewDimension(descriptor->dimension));
    DAWN_TRY(ValidateTextureFormat(descriptor->format));
    DAWN_TRY(ValidateTextureAspect(descriptor->aspect));

    const Format& format = texture->GetFormat();
    const Format* viewFormat;
    DAWN_TRY_ASSIGN(viewFormat, device->GetInternalFormat(descriptor->format));

    DAWN_TRY(ValidateTextureViewUsage(device, texture, descriptor->usage, viewFormat));

    const auto aspect = SelectFormatAspects(format, descriptor->aspect);
    DAWN_INVALID_IF(aspect == Aspect::None,
                    "Texture format (%s) does not have the texture view's selected aspect (%s).",
                    format.format, descriptor->aspect);

    DAWN_INVALID_IF(descriptor->arrayLayerCount == 0 || descriptor->mipLevelCount == 0,
                    "The texture view's arrayLayerCount (%u) or mipLevelCount (%u) is zero.",
                    descriptor->arrayLayerCount, descriptor->mipLevelCount);

    DAWN_INVALID_IF(
        uint64_t(descriptor->baseArrayLayer) + uint64_t(descriptor->arrayLayerCount) >
            uint64_t(texture->GetArrayLayers()),
        "Texture view array layer range (baseArrayLayer: %u, arrayLayerCount: %u) exceeds the "
        "texture's array layer count (%u).",
        descriptor->baseArrayLayer, descriptor->arrayLayerCount, texture->GetArrayLayers());

    DAWN_INVALID_IF(
        uint64_t(descriptor->baseMipLevel) + uint64_t(descriptor->mipLevelCount) >
            uint64_t(texture->GetNumMipLevels()),
        "Texture view mip level range (baseMipLevel: %u, mipLevelCount: %u) exceeds the "
        "texture's mip level count (%u).",
        descriptor->baseMipLevel, descriptor->mipLevelCount, texture->GetNumMipLevels());

    if (descriptor.Get<YCbCrVkDescriptor>()) {
        DAWN_INVALID_IF(!device->HasFeature(Feature::YCbCrVulkanSamplers), "%s is not enabled.",
                        wgpu::FeatureName::YCbCrVulkanSamplers);
        DAWN_INVALID_IF(format.format != wgpu::TextureFormat::External,
                        "Texture format (%s) is not (%s).", format.format,
                        wgpu::TextureFormat::External);
    } else if (format.format == wgpu::TextureFormat::External) {
        return DAWN_VALIDATION_ERROR("Invalid TextureViewDescriptor with Texture format (%s).",
                                     wgpu::TextureFormat::External);
    }

    DAWN_TRY(ValidateCanViewTextureAs(device, texture, *viewFormat, descriptor->aspect));
    DAWN_TRY(ValidateTextureViewDimensionCompatibility(device, texture, descriptor));

    return {};
}

ResultOrError<TextureViewDescriptor> GetTextureViewDescriptorWithDefaults(
    const TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    DAWN_ASSERT(texture);

    TextureViewDescriptor desc = {};
    if (descriptor) {
        desc = descriptor->WithTrivialFrontendDefaults();
    }

    // The default value for the view dimension depends on the texture's dimension with a
    // special case for 2DArray being chosen if texture is 2D but has more than one array layer.
    if (desc.dimension == wgpu::TextureViewDimension::Undefined) {
        switch (texture->GetDimension()) {
            case wgpu::TextureDimension::Undefined:
                DAWN_UNREACHABLE();

            case wgpu::TextureDimension::e1D:
                desc.dimension = wgpu::TextureViewDimension::e1D;
                break;

            case wgpu::TextureDimension::e2D:
                if (texture->GetArrayLayers() == 1) {
                    desc.dimension = wgpu::TextureViewDimension::e2D;
                } else {
                    desc.dimension = wgpu::TextureViewDimension::e2DArray;
                }
                break;

            case wgpu::TextureDimension::e3D:
                desc.dimension = wgpu::TextureViewDimension::e3D;
                break;
        }
    }

    if (desc.format == wgpu::TextureFormat::Undefined) {
        const Format& format = texture->GetFormat();

        // Check the aspect since |SelectFormatAspects| assumes a valid aspect.
        // Creation would have failed validation later since the aspect is invalid.
        DAWN_TRY(ValidateTextureAspect(desc.aspect));

        Aspect aspects = SelectFormatAspects(format, desc.aspect);
        if (HasOneBit(aspects)) {
            desc.format = format.GetAspectInfo(aspects).format;
        } else {
            desc.format = format.format;
        }
    }

    if (desc.arrayLayerCount == wgpu::kArrayLayerCountUndefined) {
        switch (desc.dimension) {
            case wgpu::TextureViewDimension::e1D:
            case wgpu::TextureViewDimension::e2D:
            case wgpu::TextureViewDimension::e3D:
                desc.arrayLayerCount = 1;
                break;
            case wgpu::TextureViewDimension::Cube:
                desc.arrayLayerCount = 6;
                break;
            case wgpu::TextureViewDimension::e2DArray:
            case wgpu::TextureViewDimension::CubeArray:
                desc.arrayLayerCount = texture->GetArrayLayers() - desc.baseArrayLayer;
                break;
            default:
                // We don't put DAWN_UNREACHABLE() here because we validate enums only after this
                // function sets default values. Otherwise, the DAWN_UNREACHABLE() will be hit.
                break;
        }
    }

    if (desc.mipLevelCount == wgpu::kMipLevelCountUndefined) {
        desc.mipLevelCount = texture->GetNumMipLevels() - desc.baseMipLevel;
    }
    return desc;
}

// WebGPU only supports sample counts of 1 and 4. We could expand to more based on
// platform support, but it would probably be a feature.
bool IsValidSampleCount(uint32_t sampleCount) {
    switch (sampleCount) {
        case 1:
        case 4:
            return true;

        default:
            return false;
    }
}

// TextureBase

TextureBase::TextureState::TextureState() : hasAccess(true), destroyed(false) {}

TextureBase::TextureBase(DeviceBase* device, const UnpackedPtr<TextureDescriptor>& descriptor)
    : SharedResource(device, descriptor->label),
      mDimension(descriptor->dimension),
      mCompatibilityTextureBindingViewDimension(
          ResolveDefaultCompatiblityTextureBindingViewDimension(device, descriptor)),
      mFormat(device->GetValidInternalFormat(descriptor->format)),
      mBaseSize(descriptor->size),
      mMipLevelCount(descriptor->mipLevelCount),
      mSampleCount(descriptor->sampleCount),
      mUsage(descriptor->usage),
      mInternalUsage(mUsage),
      mFormatEnumForReflection(descriptor->format) {
    uint32_t subresourceCount =
        mMipLevelCount * GetArrayLayers() * GetAspectCount(mFormat->aspects);
    mIsSubresourceContentInitializedAtIndex = std::vector<bool>(subresourceCount, false);

    for (uint32_t i = 0; i < descriptor->viewFormatCount; ++i) {
        if (descriptor->viewFormats[i] == descriptor->format) {
            // Skip our own format, so the backends don't allocate the texture for
            // reinterpretation if it's not needed.
            continue;
        }
        mViewFormats[device->GetValidInternalFormat(descriptor->viewFormats[i])] = true;
    }

    if (auto* internalUsageDesc = descriptor.Get<DawnTextureInternalUsageDescriptor>()) {
        mInternalUsage |= internalUsageDesc->internalUsage;
    }
    GetObjectTrackingList()->Track(this);

    mInternalUsage = AddInternalUsages(device, mInternalUsage, *mFormat, GetSampleCount(),
                                       GetNumMipLevels(), GetArrayLayers());
}

TextureBase::~TextureBase() = default;

static constexpr Format kUnusedFormat;

TextureBase::TextureBase(DeviceBase* device,
                         const TextureDescriptor* descriptor,
                         ObjectBase::ErrorTag tag)
    : SharedResource(device, tag, descriptor->label),
      mDimension(descriptor->dimension),
      mFormat(kUnusedFormat),
      mBaseSize(descriptor->size),
      mMipLevelCount(descriptor->mipLevelCount),
      mSampleCount(descriptor->sampleCount),
      mUsage(descriptor->usage),
      mFormatEnumForReflection(descriptor->format) {}

void TextureBase::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - Losing the last reference to a swap chain will also call APIDestroy on its
    //   current texture. This is protected by acquiring the global device lock on
    //   the last release. That lock can be removed when APIDestroy is made thread-safe.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    mState.destroyed = true;

    // Destroy all of the views associated with the texture as well.
    mTextureViews.Destroy();
}

// static
Ref<TextureBase> TextureBase::MakeError(DeviceBase* device, const TextureDescriptor* descriptor) {
    return AcquireRef(new TextureBase(device, descriptor, ObjectBase::kError));
}

ObjectType TextureBase::GetType() const {
    return ObjectType::Texture;
}

void TextureBase::FormatLabel(absl::FormatSink* s) const {
    s->Append(ObjectTypeAsString(GetType()));

    const std::string& label = GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    } else if (!IsError()) {
        s->Append(absl::StrFormat(" (unlabeled %s, %s)", GetSizeLabel(), mFormat->format));
    }
}

std::string TextureBase::GetSizeLabel() const {
    if (mDimension == wgpu::TextureDimension::e1D) {
        return absl::StrFormat("%d px", mBaseSize.width);
    } else if (mDimension == wgpu::TextureDimension::e3D) {
        return absl::StrFormat("%dx%dx%d px", mBaseSize.width, mBaseSize.height,
                               mBaseSize.depthOrArrayLayers);
    }

    if (mBaseSize.depthOrArrayLayers > 1) {
        return absl::StrFormat("%dx%d px, %d layer", mBaseSize.width, mBaseSize.height,
                               mBaseSize.depthOrArrayLayers);
    }
    return absl::StrFormat("%dx%d px", mBaseSize.width, mBaseSize.height);
}

wgpu::TextureDimension TextureBase::GetDimension() const {
    DAWN_ASSERT(!IsError());
    return mDimension;
}

wgpu::TextureViewDimension TextureBase::GetCompatibilityTextureBindingViewDimension() const {
    DAWN_ASSERT(!IsError());
    return mCompatibilityTextureBindingViewDimension;
}

const Format& TextureBase::GetFormat() const {
    DAWN_ASSERT(!IsError());
    return *mFormat;
}
const FormatSet& TextureBase::GetViewFormats() const {
    DAWN_ASSERT(!IsError());
    return mViewFormats;
}

const Extent3D& TextureBase::GetBaseSize() const {
    DAWN_ASSERT(!IsError());
    return mBaseSize;
}

Extent3D TextureBase::GetSize(Aspect aspect) const {
    DAWN_ASSERT(!IsError());
    switch (aspect) {
        case Aspect::Color:
        case Aspect::Depth:
        case Aspect::Stencil:
        case Aspect::CombinedDepthStencil:
            return mBaseSize;
        case Aspect::Plane0:
        case Aspect::Plane2:
            DAWN_ASSERT(GetFormat().IsMultiPlanar());
            return mBaseSize;
        case Aspect::Plane1: {
            DAWN_ASSERT(GetFormat().IsMultiPlanar());
            auto planeSize = mBaseSize;
            switch (GetFormat().subSampling) {
                case TextureSubsampling::e420:
                    if (planeSize.width > 1) {
                        planeSize.width >>= 1;
                    }
                    if (planeSize.height > 1) {
                        planeSize.height >>= 1;
                    }
                    break;
                case TextureSubsampling::e422:
                    if (planeSize.width > 1) {
                        planeSize.width >>= 1;
                    }
                    break;
                case TextureSubsampling::e444:
                    break;
                case TextureSubsampling::Undefined:
                    DAWN_UNREACHABLE();
            }
            return planeSize;
        }
        case Aspect::None:
            break;
    }

    if (aspect == (Aspect::Depth | Aspect::Stencil)) {
        return mBaseSize;
    }

    DAWN_UNREACHABLE();
}
Extent3D TextureBase::GetSize(wgpu::TextureAspect textureAspect) const {
    const auto aspect = SelectFormatAspects(GetFormat(), textureAspect);
    return GetSize(aspect);
}
uint32_t TextureBase::GetWidth(Aspect aspect) const {
    DAWN_ASSERT(!IsError());
    return GetSize(aspect).width;
}
uint32_t TextureBase::GetHeight(Aspect aspect) const {
    DAWN_ASSERT(!IsError());
    return GetSize(aspect).height;
}
uint32_t TextureBase::GetDepth(Aspect aspect) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mDimension == wgpu::TextureDimension::e3D);
    return GetSize(aspect).depthOrArrayLayers;
}
uint32_t TextureBase::GetArrayLayers() const {
    DAWN_ASSERT(!IsError());
    if (mDimension == wgpu::TextureDimension::e3D) {
        return 1;
    }
    return mBaseSize.depthOrArrayLayers;
}
uint32_t TextureBase::GetNumMipLevels() const {
    DAWN_ASSERT(!IsError());
    return mMipLevelCount;
}
SubresourceRange TextureBase::GetAllSubresources() const {
    DAWN_ASSERT(!IsError());
    return {mFormat->aspects, {0, GetArrayLayers()}, {0, mMipLevelCount}};
}
uint32_t TextureBase::GetSampleCount() const {
    DAWN_ASSERT(!IsError());
    return mSampleCount;
}
uint32_t TextureBase::GetSubresourceCount() const {
    DAWN_ASSERT(!IsError());
    return static_cast<uint32_t>(mIsSubresourceContentInitializedAtIndex.size());
}
wgpu::TextureUsage TextureBase::GetUsage() const {
    DAWN_ASSERT(!IsError());
    return mUsage;
}
wgpu::TextureUsage TextureBase::GetInternalUsage() const {
    DAWN_ASSERT(!IsError());
    return mInternalUsage;
}
void TextureBase::AddInternalUsage(wgpu::TextureUsage usage) {
    DAWN_ASSERT(!IsError());
    mInternalUsage |= usage;
}

bool TextureBase::IsDestroyed() const {
    DAWN_ASSERT(!IsError());
    return mState.destroyed;
}

bool TextureBase::IsInitialized() const {
    return IsSubresourceContentInitialized(GetAllSubresources());
}

void TextureBase::SetInitialized(bool initialized) {
    SetIsSubresourceContentInitialized(initialized, GetAllSubresources());
}

ExecutionSerial TextureBase::OnEndAccess() {
    mState.hasAccess = false;
    ExecutionSerial lastUsageSerial = mLastSharedTextureMemoryUsageSerial;
    mLastSharedTextureMemoryUsageSerial = kBeginningOfGPUTime;
    return lastUsageSerial;
}

void TextureBase::OnBeginAccess() {
    mState.hasAccess = true;
}

bool TextureBase::HasAccess() const {
    DAWN_ASSERT(!IsError());
    return mState.hasAccess;
}

uint32_t TextureBase::GetSubresourceIndex(uint32_t mipLevel,
                                          uint32_t arraySlice,
                                          Aspect aspect) const {
    DAWN_ASSERT(HasOneBit(aspect));
    return mipLevel + GetNumMipLevels() * (arraySlice + GetArrayLayers() * GetAspectIndex(aspect));
}

bool TextureBase::IsSubresourceContentInitialized(const SubresourceRange& range) const {
    DAWN_ASSERT(!IsError());
    for (Aspect aspect : IterateEnumMask(range.aspects)) {
        for (uint32_t arrayLayer = range.baseArrayLayer;
             arrayLayer < range.baseArrayLayer + range.layerCount; ++arrayLayer) {
            for (uint32_t mipLevel = range.baseMipLevel;
                 mipLevel < range.baseMipLevel + range.levelCount; ++mipLevel) {
                uint32_t subresourceIndex = GetSubresourceIndex(mipLevel, arrayLayer, aspect);
                DAWN_ASSERT(subresourceIndex < mIsSubresourceContentInitializedAtIndex.size());
                if (!mIsSubresourceContentInitializedAtIndex[subresourceIndex]) {
                    return false;
                }
            }
        }
    }
    return true;
}

void TextureBase::SetIsSubresourceContentInitialized(bool isInitialized,
                                                     const SubresourceRange& range) {
    DAWN_ASSERT(!IsError());
    for (Aspect aspect : IterateEnumMask(range.aspects)) {
        for (uint32_t arrayLayer = range.baseArrayLayer;
             arrayLayer < range.baseArrayLayer + range.layerCount; ++arrayLayer) {
            for (uint32_t mipLevel = range.baseMipLevel;
                 mipLevel < range.baseMipLevel + range.levelCount; ++mipLevel) {
                uint32_t subresourceIndex = GetSubresourceIndex(mipLevel, arrayLayer, aspect);
                DAWN_ASSERT(subresourceIndex < mIsSubresourceContentInitializedAtIndex.size());
                mIsSubresourceContentInitializedAtIndex[subresourceIndex] = isInitialized;
            }
        }
    }
}

MaybeError TextureBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    if (DAWN_UNLIKELY(mState.destroyed || !mState.hasAccess)) {
        DAWN_INVALID_IF(mState.destroyed, "Destroyed texture %s used in a submit.", this);
        if (DAWN_UNLIKELY(!mState.hasAccess)) {
            if (mSharedResourceMemoryContents != nullptr) {
                Ref<SharedTextureMemoryBase> memory =
                    mSharedResourceMemoryContents->GetSharedResourceMemory()
                        .Promote()
                        .Cast<Ref<SharedTextureMemoryBase>>();
                if (memory != nullptr) {
                    return DAWN_VALIDATION_ERROR("%s used in a submit without current access to %s",
                                                 this, memory.Get());
                }
                return DAWN_VALIDATION_ERROR(
                    "%s used in a submit without current access. It's SharedTextureMemory was "
                    "already destroyed.",
                    this);
            }
            return DAWN_VALIDATION_ERROR("%s used in a submit without current access.", this);
        }
    }

    return {};
}

bool TextureBase::IsMultisampledTexture() const {
    DAWN_ASSERT(!IsError());
    return mSampleCount > 1;
}

bool TextureBase::IsReadOnly() const {
    return IsSubset(mUsage, kReadOnlyTextureUsages);
}

bool TextureBase::CoversFullSubresource(uint32_t mipLevel,
                                        Aspect aspect,
                                        const Extent3D& size) const {
    Extent3D levelSize = GetMipLevelSingleSubresourcePhysicalSize(mipLevel, aspect);
    switch (GetDimension()) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();
        case wgpu::TextureDimension::e1D:
            return size.width == levelSize.width;
        case wgpu::TextureDimension::e2D:
            return size.width == levelSize.width && size.height == levelSize.height;
        case wgpu::TextureDimension::e3D:
            return size == levelSize;
    }
    DAWN_UNREACHABLE();
}

Extent3D TextureBase::GetMipLevelSingleSubresourceVirtualSize(uint32_t level, Aspect aspect) const {
    Extent3D aspectSize = GetSize(aspect);
    Extent3D extent = {std::max(aspectSize.width >> level, 1u), 1u, 1u};
    if (mDimension == wgpu::TextureDimension::e1D) {
        return extent;
    }

    extent.height = std::max(aspectSize.height >> level, 1u);
    if (mDimension == wgpu::TextureDimension::e2D) {
        return extent;
    }

    extent.depthOrArrayLayers = std::max(aspectSize.depthOrArrayLayers >> level, 1u);
    return extent;
}

Extent3D TextureBase::GetMipLevelSingleSubresourcePhysicalSize(uint32_t level,
                                                               Aspect aspect) const {
    Extent3D extent = GetMipLevelSingleSubresourceVirtualSize(level, aspect);

    // Compressed Textures will have paddings if their width or height is not a multiple of
    // 4 at non-zero mipmap levels.
    if (mFormat->isCompressed && level != 0) {
        // If |level| is non-zero, then each dimension of |extent| is at most half of
        // the max texture dimension. Computations here which add the block width/height
        // to the extent cannot overflow.
        const TexelBlockInfo& blockInfo = mFormat->GetAspectInfo(wgpu::TextureAspect::All).block;
        extent.width = (extent.width + blockInfo.width - 1) / blockInfo.width * blockInfo.width;
        extent.height =
            (extent.height + blockInfo.height - 1) / blockInfo.height * blockInfo.height;
    }

    return extent;
}

Extent3D TextureBase::ClampToMipLevelVirtualSize(uint32_t level,
                                                 Aspect aspect,
                                                 const Origin3D& origin,
                                                 const Extent3D& extent) const {
    const Extent3D virtualSizeAtLevel = GetMipLevelSingleSubresourceVirtualSize(level, aspect);
    DAWN_ASSERT(origin.x <= virtualSizeAtLevel.width);
    DAWN_ASSERT(origin.y <= virtualSizeAtLevel.height);
    uint32_t clampedCopyExtentWidth = (extent.width > virtualSizeAtLevel.width - origin.x)
                                          ? (virtualSizeAtLevel.width - origin.x)
                                          : extent.width;
    uint32_t clampedCopyExtentHeight = (extent.height > virtualSizeAtLevel.height - origin.y)
                                           ? (virtualSizeAtLevel.height - origin.y)
                                           : extent.height;
    return {clampedCopyExtentWidth, clampedCopyExtentHeight, extent.depthOrArrayLayers};
}

Extent3D TextureBase::GetMipLevelSubresourceVirtualSize(uint32_t level, Aspect aspect) const {
    Extent3D extent = GetMipLevelSingleSubresourceVirtualSize(level, aspect);
    if (mDimension == wgpu::TextureDimension::e2D) {
        extent.depthOrArrayLayers = mBaseSize.depthOrArrayLayers;
    }
    return extent;
}

ResultOrError<Ref<TextureViewBase>> TextureBase::CreateView(
    const TextureViewDescriptor* descriptor) {
    return GetDevice()->CreateTextureView(this, descriptor);
}

Ref<TextureViewBase> TextureBase::CreateErrorView(const TextureViewDescriptor* descriptor) {
    return TextureViewBase::MakeError(GetDevice(), descriptor ? descriptor->label : nullptr);
}

ApiObjectList* TextureBase::GetViewTrackingList() {
    return &mTextureViews;
}

TextureViewBase* TextureBase::APICreateView(const TextureViewDescriptor* descriptor) {
    DeviceBase* device = GetDevice();

    Ref<TextureViewBase> result;
    if (device->ConsumedError(CreateView(descriptor), &result, "calling %s.CreateView(%s).", this,
                              descriptor)) {
        result = CreateErrorView(descriptor);
    }
    return ReturnToAPI(std::move(result));
}

TextureViewBase* TextureBase::APICreateErrorView(const TextureViewDescriptor* descriptor) {
    return ReturnToAPI(CreateErrorView(descriptor));
}

bool TextureBase::IsImplicitMSAARenderTextureViewSupported() const {
    return (GetUsage() & wgpu::TextureUsage::TextureBinding) != 0;
}

void TextureBase::SetSharedResourceMemoryContentsForTesting(
    Ref<SharedResourceMemoryContents> contents) {
    mSharedResourceMemoryContents = std::move(contents);
}

void TextureBase::DumpMemoryStatistics(dawn::native::MemoryDump* dump, const char* prefix) const {
    std::string name = absl::StrFormat("%s/texture_%p", prefix, static_cast<const void*>(this));
    dump->AddScalar(name.c_str(), MemoryDump::kNameSize, MemoryDump::kUnitsBytes,
                    ComputeEstimatedByteSize());
    dump->AddString(name.c_str(), "label", GetLabel());
    dump->AddString(name.c_str(), "dimensions", GetSizeLabel());
    dump->AddString(name.c_str(), "format", absl::StrFormat("%s", GetFormat().format));
    dump->AddString(name.c_str(), "sample_count", absl::StrFormat("%u", GetSampleCount()));
    dump->AddString(name.c_str(), "usage", absl::StrFormat("%s", GetUsage()));
    dump->AddString(name.c_str(), "internal_usage", absl::StrFormat("%s", GetInternalUsage()));
}

uint64_t TextureBase::ComputeEstimatedByteSize() const {
    DAWN_ASSERT(IsAlive() && !IsError());
    // Do not emit a non-zero size for textures that wrap external shared texture memory, or
    // textures used as transient (memoryless) attachments.
    if (GetSharedResourceMemoryContents() != nullptr ||
        (GetInternalUsage() & wgpu::TextureUsage::TransientAttachment) != 0) {
        return 0;
    }
    uint64_t byteSize = 0;
    for (Aspect aspect : IterateEnumMask(SelectFormatAspects(*mFormat, wgpu::TextureAspect::All))) {
        const AspectInfo& info = mFormat->GetAspectInfo(aspect);
        for (uint32_t i = 0; i < mMipLevelCount; i++) {
            Extent3D mipSize = GetMipLevelSingleSubresourcePhysicalSize(i, aspect);
            byteSize += (mipSize.width / info.block.width) * (mipSize.height / info.block.height) *
                        info.block.byteSize * mSampleCount;
        }
    }
    if (mDimension == wgpu::TextureDimension::e2D) {
        byteSize *= mBaseSize.depthOrArrayLayers;
    }
    return byteSize;
}

void TextureBase::APIDestroy() {
    Destroy();
}

uint32_t TextureBase::APIGetWidth() const {
    return mBaseSize.width;
}

uint32_t TextureBase::APIGetHeight() const {
    return mBaseSize.height;
}
uint32_t TextureBase::APIGetDepthOrArrayLayers() const {
    return mBaseSize.depthOrArrayLayers;
}

uint32_t TextureBase::APIGetMipLevelCount() const {
    return mMipLevelCount;
}

uint32_t TextureBase::APIGetSampleCount() const {
    return mSampleCount;
}

wgpu::TextureDimension TextureBase::APIGetDimension() const {
    return mDimension;
}

wgpu::TextureFormat TextureBase::APIGetFormat() const {
    return mFormatEnumForReflection;
}

wgpu::TextureUsage TextureBase::APIGetUsage() const {
    return mUsage;
}

// TextureViewBase

TextureViewBase::TextureViewBase(TextureBase* texture,
                                 const UnpackedPtr<TextureViewDescriptor>& descriptor)
    : ApiObjectBase(texture->GetDevice(), descriptor->label),
      mTexture(texture),
      mFormat(GetDevice()->GetValidInternalFormat(descriptor->format)),
      mDimension(descriptor->dimension),
      mRange({ConvertViewAspect(*mFormat, descriptor->aspect),
              {descriptor->baseArrayLayer, descriptor->arrayLayerCount},
              {descriptor->baseMipLevel, descriptor->mipLevelCount}}),
      mUsage(RemoveInvalidViewUsages(GetTextureViewUsage(texture->GetUsage(), descriptor->usage),
                                     &mFormat.get())),
      mInternalUsage(
          AddInternalUsages(GetDevice(),
                            GetTextureViewUsage(texture->GetInternalUsage(), descriptor->usage),
                            *mFormat,
                            texture->GetSampleCount(),
                            texture->GetNumMipLevels(),
                            texture->GetArrayLayers())) {
    GetObjectTrackingList()->Track(this);

    // Emit a warning if invalid usages were removed for this view.
    // TODO(363903526): Remove this warning after deprecation period.
    wgpu::TextureUsage inheritedUsage = GetTextureViewUsage(texture->GetUsage(), descriptor->usage);
    if (mUsage != inheritedUsage) {
        DAWN_ASSERT(descriptor->usage == wgpu::TextureUsage::None);
        std::string warning = absl::StrFormat(
            "%s with format (%s) and inherited usage (%s) is deprecated. Please request explicit "
            "usages on texture views when the view format is not compatible with all inherited "
            "texture usages.",
            this, mFormat->format, inheritedUsage);
        GetDevice()->EmitWarningOnce(warning.c_str());
    }
}

TextureViewBase::TextureViewBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label), mFormat(kUnusedFormat) {}

TextureViewBase::~TextureViewBase() = default;

void TextureViewBase::DestroyImpl() {}

// static
Ref<TextureViewBase> TextureViewBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new TextureViewBase(device, ObjectBase::kError, label));
}

ObjectType TextureViewBase::GetType() const {
    return ObjectType::TextureView;
}

void TextureViewBase::FormatLabel(absl::FormatSink* s) const {
    s->Append(ObjectTypeAsString(GetType()));

    const std::string& label = GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }
    if (IsError()) {
        return;
    }

    if (label.empty()) {
        s->Append(" of ");
        GetTexture()->FormatLabel(s);
    }
}

const TextureBase* TextureViewBase::GetTexture() const {
    DAWN_ASSERT(!IsError());
    return mTexture.Get();
}

TextureBase* TextureViewBase::GetTexture() {
    DAWN_ASSERT(!IsError());
    return mTexture.Get();
}

Aspect TextureViewBase::GetAspects() const {
    DAWN_ASSERT(!IsError());
    return mRange.aspects;
}

const Format& TextureViewBase::GetFormat() const {
    DAWN_ASSERT(!IsError());
    return *mFormat;
}

wgpu::TextureViewDimension TextureViewBase::GetDimension() const {
    DAWN_ASSERT(!IsError());
    return mDimension;
}

uint32_t TextureViewBase::GetBaseMipLevel() const {
    DAWN_ASSERT(!IsError());
    return mRange.baseMipLevel;
}

uint32_t TextureViewBase::GetLevelCount() const {
    DAWN_ASSERT(!IsError());
    return mRange.levelCount;
}

uint32_t TextureViewBase::GetBaseArrayLayer() const {
    DAWN_ASSERT(!IsError());
    return mRange.baseArrayLayer;
}

uint32_t TextureViewBase::GetLayerCount() const {
    DAWN_ASSERT(!IsError());
    return mRange.layerCount;
}

const SubresourceRange& TextureViewBase::GetSubresourceRange() const {
    DAWN_ASSERT(!IsError());
    return mRange;
}

Extent3D TextureViewBase::GetSingleSubresourceVirtualSize() const {
    DAWN_ASSERT(!IsError());
    return GetTexture()->GetMipLevelSingleSubresourceVirtualSize(GetBaseMipLevel(), GetAspects());
}

wgpu::TextureUsage TextureViewBase::GetUsage() const {
    DAWN_ASSERT(!IsError());
    return mUsage;
}

wgpu::TextureUsage TextureViewBase::GetInternalUsage() const {
    DAWN_ASSERT(!IsError());
    return mInternalUsage;
}

bool TextureViewBase::IsYCbCr() const {
    return false;
}

YCbCrVkDescriptor TextureViewBase::GetYCbCrVkDescriptor() const {
    DAWN_UNREACHABLE();
    return {};
}

ApiObjectList* TextureViewBase::GetObjectTrackingList() {
    if (mTexture != nullptr) {
        return mTexture->GetViewTrackingList();
    }
    // Return the base device list for error objects so that
    // the list is never null. Error texture views are never tracked,
    // so liveness checks will always return false.
    DAWN_ASSERT(IsError());
    return ApiObjectBase::GetObjectTrackingList();
}

}  // namespace dawn::native
