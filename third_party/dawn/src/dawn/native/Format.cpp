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

#include "dawn/native/Format.h"

#include <bitset>
#include <utility>

#include "dawn/common/MatchVariant.h"
#include "dawn/common/TypedInteger.h"
#include "dawn/native/Device.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Features.h"
#include "dawn/native/Texture.h"

namespace dawn {
enum class Cap : uint16_t {
    None = 0x0,
    Multisample = 0x1,
    Renderable = 0x2,
    Resolve = 0x4,
    StorageROrW = 0x8,  // Read Or Write, but not ReadWrite in the shader.
    StorageRW = 0x10,   // Implies StorageROrW
    PLS = 0x20,
    Blendable = 0x40,
};
}  // namespace dawn

template <>
struct wgpu::IsWGPUBitmask<dawn::Cap> {
    static constexpr bool enable = true;
};

namespace dawn::native {

// Format

SampleTypeBit SampleTypeToSampleTypeBit(wgpu::TextureSampleType sampleType) {
    switch (sampleType) {
        case wgpu::TextureSampleType::Float:
        case wgpu::TextureSampleType::UnfilterableFloat:
        case wgpu::TextureSampleType::Sint:
        case wgpu::TextureSampleType::Uint:
        case wgpu::TextureSampleType::Depth:
        case wgpu::TextureSampleType::BindingNotUsed:
        case wgpu::TextureSampleType::Undefined:
            // When the compiler complains that you need to add a case statement here, please
            // also add a corresponding static assert below!
            break;
    }

    static_assert(static_cast<uint32_t>(wgpu::TextureSampleType::BindingNotUsed) == 0);
    if (sampleType == wgpu::TextureSampleType::BindingNotUsed) {
        return SampleTypeBit::None;
    }

    static_assert(static_cast<uint32_t>(wgpu::TextureSampleType::Undefined) == 1);
    if (sampleType == wgpu::TextureSampleType::Undefined) {
        DAWN_UNREACHABLE();
        return SampleTypeBit::None;
    }

    // Check that SampleTypeBit bits are in the same position / order as the respective
    // wgpu::TextureSampleType value.
    static_assert(SampleTypeBit::Float ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Float) - 2)));
    static_assert(
        SampleTypeBit::UnfilterableFloat ==
        static_cast<SampleTypeBit>(
            1 << (static_cast<uint32_t>(wgpu::TextureSampleType::UnfilterableFloat) - 2)));
    static_assert(SampleTypeBit::Uint ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Uint) - 2)));
    static_assert(SampleTypeBit::Sint ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Sint) - 2)));
    static_assert(SampleTypeBit::Depth ==
                  static_cast<SampleTypeBit>(
                      1 << (static_cast<uint32_t>(wgpu::TextureSampleType::Depth) - 2)));
    return static_cast<SampleTypeBit>(1 << (static_cast<uint32_t>(sampleType) - 2));
}

const UnsupportedReason Format::supported;

bool Format::IsSupported() const {
    return std::holds_alternative<std::monostate>(unsupportedReason);
}

bool Format::IsColor() const {
    return aspects == Aspect::Color;
}

bool Format::HasDepth() const {
    return aspects & Aspect::Depth;
}

bool Format::HasStencil() const {
    return aspects & Aspect::Stencil;
}

bool Format::HasDepthOrStencil() const {
    return aspects & (Aspect::Depth | Aspect::Stencil);
}

bool Format::HasAlphaChannel() const {
    // This is true for current formats. May need revisit if new formats and extensions are added.
    return componentCount == 4 && IsColor();
}

bool Format::IsSnorm() const {
    return format == wgpu::TextureFormat::RGBA8Snorm || format == wgpu::TextureFormat::RG8Snorm ||
           format == wgpu::TextureFormat::R8Snorm;
}

bool Format::IsMultiPlanar() const {
    return aspects & (Aspect::Plane0 | Aspect::Plane1 | Aspect::Plane2);
}

bool Format::CopyCompatibleWith(const Format& otherFormat) const {
    // TODO(crbug.com/dawn/1332): Add a Format compatibility matrix.
    return baseFormat == otherFormat.baseFormat;
}

bool Format::ViewCompatibleWith(const Format& otherFormat) const {
    // TODO(crbug.com/dawn/1332): Add a Format compatibility matrix.
    return baseFormat == otherFormat.baseFormat;
}

const AspectInfo& Format::GetAspectInfo(wgpu::TextureAspect aspect) const {
    return GetAspectInfo(SelectFormatAspects(*this, aspect));
}

const AspectInfo& Format::GetAspectInfo(Aspect aspect) const {
    DAWN_ASSERT(HasOneBit(aspect));
    DAWN_ASSERT(aspects & aspect);
    const size_t aspectIndex = GetAspectIndex(aspect);
    DAWN_ASSERT(aspectIndex < GetAspectCount(aspects));
    return aspectInfo[aspectIndex];
}

FormatIndex Format::GetIndex() const {
    return ComputeFormatIndex(format);
}

// FormatSet implementation

bool FormatSet::operator[](const Format& format) const {
    return Base::operator[](format.GetIndex());
}

typename std::bitset<kKnownFormatCount>::reference FormatSet::operator[](const Format& format) {
    return Base::operator[](format.GetIndex());
}

// Implementation details of the format table of the DeviceBase

// For the enum for formats are packed but this might change when we have a broader feature
// mechanism for webgpu.h. Formats start at 1 because 0 is the undefined format.
// Dawn internal formats start with a prefix. We strip the prefix and then pack
// them after the last WebGPU format.
FormatIndex ComputeFormatIndex(wgpu::TextureFormat format) {
    uint32_t formatValue = static_cast<uint32_t>(format);
    switch (formatValue & kEnumPrefixMask) {
        case 0:
            // This takes advantage of overflows to make the index of TextureFormat::Undefined
            // outside of the range of the FormatTable.
            static_assert(static_cast<uint32_t>(wgpu::TextureFormat::Undefined) - 1 >
                          kKnownFormatCount);
            return static_cast<FormatIndex>(formatValue - 1);
        case kDawnEnumPrefix: {
            uint32_t dawnIndex = formatValue & ~kEnumPrefixMask;
            if (dawnIndex < kDawnFormatCount) {
                return static_cast<FormatIndex>(dawnIndex + kWebGPUFormatCount);
            }
            break;
        }
        default:
            break;
    }
    // Invalid format. Return an index outside the format table.
    return FormatIndex(~0);
}

FormatTable BuildFormatTable(const DeviceBase* device) {
    FormatTable table;
    FormatSet formatsSet;

    static constexpr SampleTypeBit kAnyFloat =
        SampleTypeBit::Float | SampleTypeBit::UnfilterableFloat;

    auto AddFormat = [&table, &formatsSet](Format format) {
        FormatIndex index = ComputeFormatIndex(format.format);
        DAWN_ASSERT(index < table.size());

        // This checks that each format is set at most once, the first part of checking that all
        // formats are set exactly once.
        DAWN_ASSERT(!formatsSet[index]);

        // Vulkan describes bytesPerRow in units of texels. If there's any format for which this
        // DAWN_ASSERT isn't true, then additional validation on bytesPerRow must be added.
        const bool hasMultipleAspects = !HasOneBit(format.aspects);
        DAWN_ASSERT(hasMultipleAspects ||
                    (kTextureBytesPerRowAlignment % format.aspectInfo[0].block.byteSize) == 0);

        table[index] = format;
        formatsSet.set(index);
    };

    using Width = TypedInteger<struct WidthT, uint32_t>;
    using Height = TypedInteger<struct HeightT, uint32_t>;
    using ByteSize = TypedInteger<struct ByteSizeT, uint32_t>;
    using RenderTargetPixelByteCost = TypedInteger<struct RenderTargetPixelByteCostT, uint32_t>;
    using ComponentCount = TypedInteger<struct ComponentCountT, uint32_t>;
    using RenderTargetComponentAlignment =
        TypedInteger<struct RenderTargetComponentAlignmentT, uint32_t>;

    auto AddConditionalColorFormat =
        [&AddFormat](
            wgpu::TextureFormat format, UnsupportedReason unsupportedReason, Cap capabilities,
            ByteSize byteSize, SampleTypeBit sampleTypes, ComponentCount componentCount,
            RenderTargetPixelByteCost renderTargetPixelByteCost = RenderTargetPixelByteCost(0),
            RenderTargetComponentAlignment renderTargetComponentAlignment =
                RenderTargetComponentAlignment(0),
            wgpu::TextureFormat baseFormat = wgpu::TextureFormat::Undefined) {
            Format internalFormat;
            internalFormat.format = format;
            bool renderable = capabilities & Cap::Renderable;
            internalFormat.isRenderable = renderable;
            internalFormat.isBlendable = capabilities & Cap::Blendable;
            internalFormat.isCompressed = false;
            internalFormat.unsupportedReason = unsupportedReason;
            internalFormat.supportsStorageUsage =
                capabilities & (Cap::StorageROrW | Cap::StorageRW);
            internalFormat.supportsReadWriteStorageUsage = capabilities & Cap::StorageRW;

            bool supportsMultisample = capabilities & Cap::Multisample;
            if (supportsMultisample) {
                DAWN_ASSERT(renderable);
            }
            internalFormat.supportsMultisample = supportsMultisample;
            internalFormat.supportsResolveTarget = capabilities & Cap::Resolve;
            internalFormat.supportsStorageAttachment = capabilities & Cap::PLS;
            internalFormat.aspects = Aspect::Color;
            internalFormat.componentCount = static_cast<uint32_t>(componentCount);
            if (renderable) {
                // If the color format is renderable, it must have a pixel byte size and component
                // alignment specified.
                DAWN_ASSERT(renderTargetPixelByteCost != RenderTargetPixelByteCost(0) &&
                            renderTargetComponentAlignment != RenderTargetComponentAlignment(0));
                internalFormat.renderTargetPixelByteCost =
                    static_cast<uint32_t>(renderTargetPixelByteCost);
                internalFormat.renderTargetComponentAlignment =
                    static_cast<uint32_t>(renderTargetComponentAlignment);
            }

            // Default baseFormat of each color formats should be themselves.
            if (baseFormat == wgpu::TextureFormat::Undefined) {
                internalFormat.baseFormat = format;
            } else {
                internalFormat.baseFormat = baseFormat;
            }

            AspectInfo* firstAspect = internalFormat.aspectInfo.data();
            firstAspect->block.byteSize = static_cast<uint32_t>(byteSize);
            firstAspect->block.width = 1;
            firstAspect->block.height = 1;
            if (HasOneBit(sampleTypes)) {
                switch (sampleTypes) {
                    case SampleTypeBit::Float:
                    case SampleTypeBit::UnfilterableFloat:
                    case SampleTypeBit::External:
                        firstAspect->baseType = TextureComponentType::Float;
                        break;
                    case SampleTypeBit::Sint:
                        firstAspect->baseType = TextureComponentType::Sint;
                        break;
                    case SampleTypeBit::Uint:
                        firstAspect->baseType = TextureComponentType::Uint;
                        break;
                    default:
                        DAWN_UNREACHABLE();
                }
            } else {
                DAWN_ASSERT(sampleTypes & SampleTypeBit::Float);
                firstAspect->baseType = TextureComponentType::Float;
            }
            firstAspect->supportedSampleTypes = sampleTypes;
            firstAspect->format = format;
            AddFormat(internalFormat);
        };

    auto AddColorFormat = [&AddConditionalColorFormat](
                              wgpu::TextureFormat format, Cap capabilites, ByteSize byteSize,
                              SampleTypeBit sampleTypes, ComponentCount componentCount,
                              RenderTargetPixelByteCost renderTargetPixelByteCost =
                                  RenderTargetPixelByteCost(0),
                              RenderTargetComponentAlignment renderTargetComponentAlignment =
                                  RenderTargetComponentAlignment(0),
                              wgpu::TextureFormat baseFormat = wgpu::TextureFormat::Undefined) {
        AddConditionalColorFormat(format, std::monostate{}, capabilites, byteSize, sampleTypes,
                                  componentCount, renderTargetPixelByteCost,
                                  renderTargetComponentAlignment, baseFormat);
    };

    auto AddDepthFormat = [&AddFormat](wgpu::TextureFormat format, uint32_t byteSize,
                                       UnsupportedReason unsupportedReason) {
        Format internalFormat;
        internalFormat.format = format;
        internalFormat.baseFormat = format;
        internalFormat.isRenderable = true;
        internalFormat.isBlendable = false;
        internalFormat.isCompressed = false;
        internalFormat.unsupportedReason = unsupportedReason;
        internalFormat.supportsStorageUsage = false;
        internalFormat.supportsMultisample = true;
        internalFormat.supportsResolveTarget = false;
        internalFormat.aspects = Aspect::Depth;
        internalFormat.componentCount = 1;

        AspectInfo* firstAspect = internalFormat.aspectInfo.data();
        firstAspect->block.byteSize = byteSize;
        firstAspect->block.width = 1;
        firstAspect->block.height = 1;
        firstAspect->baseType = TextureComponentType::Float;
        firstAspect->supportedSampleTypes = SampleTypeBit::Depth | SampleTypeBit::UnfilterableFloat;
        firstAspect->format = format;
        AddFormat(internalFormat);
    };

    auto AddStencilFormat = [&AddFormat](wgpu::TextureFormat format,
                                         UnsupportedReason unsupportedReason) {
        Format internalFormat;
        internalFormat.format = format;
        internalFormat.baseFormat = format;
        internalFormat.isRenderable = true;
        internalFormat.isBlendable = false;
        internalFormat.isCompressed = false;
        internalFormat.unsupportedReason = unsupportedReason;
        internalFormat.supportsStorageUsage = false;
        internalFormat.supportsMultisample = true;
        internalFormat.supportsResolveTarget = false;
        internalFormat.aspects = Aspect::Stencil;
        internalFormat.componentCount = 1;

        // Duplicate the data for the stencil aspect in both the first and second aspect info.
        //  - aspectInfo[0] is used by AddMultiAspectFormat to copy the info for the whole
        //    stencil8 aspect of depth-stencil8 formats.
        //  - aspectInfo[1] is the actual info used in the rest of Dawn since
        //    GetAspectIndex(Aspect::Stencil) is 1.
        DAWN_ASSERT(GetAspectIndex(Aspect::Stencil) == 1);

        internalFormat.aspectInfo[0].block.byteSize = 1;
        internalFormat.aspectInfo[0].block.width = 1;
        internalFormat.aspectInfo[0].block.height = 1;
        internalFormat.aspectInfo[0].baseType = TextureComponentType::Uint;
        internalFormat.aspectInfo[0].supportedSampleTypes = SampleTypeBit::Uint;
        internalFormat.aspectInfo[0].format = format;

        internalFormat.aspectInfo[1] = internalFormat.aspectInfo[0];

        AddFormat(internalFormat);
    };

    auto AddCompressedFormat =
        [&AddFormat](wgpu::TextureFormat format, ByteSize byteSize, Width width, Height height,
                     UnsupportedReason unsupportedReason, ComponentCount componentCount,
                     wgpu::TextureFormat baseFormat = wgpu::TextureFormat::Undefined) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.isRenderable = false;
            internalFormat.isBlendable = false;
            internalFormat.isCompressed = true;
            internalFormat.unsupportedReason = unsupportedReason;
            internalFormat.supportsStorageUsage = false;
            internalFormat.supportsMultisample = false;
            internalFormat.supportsResolveTarget = false;
            internalFormat.aspects = Aspect::Color;
            internalFormat.componentCount = static_cast<uint32_t>(componentCount);

            // Default baseFormat of each compressed formats should be themselves.
            if (baseFormat == wgpu::TextureFormat::Undefined) {
                internalFormat.baseFormat = format;
            } else {
                internalFormat.baseFormat = baseFormat;
            }

            AspectInfo* firstAspect = internalFormat.aspectInfo.data();
            firstAspect->block.byteSize = static_cast<uint32_t>(byteSize);
            firstAspect->block.width = static_cast<uint32_t>(width);
            firstAspect->block.height = static_cast<uint32_t>(height);
            firstAspect->baseType = TextureComponentType::Float;
            firstAspect->supportedSampleTypes = kAnyFloat;
            firstAspect->format = format;
            AddFormat(internalFormat);
        };

    auto AddMultiAspectFormat =
        [&AddFormat, &table](wgpu::TextureFormat format, TextureSubsampling subSampling,
                             Aspect aspects, Cap capabilites, UnsupportedReason unsupportedReason,
                             ComponentCount componentCount, wgpu::TextureFormat firstFormat,
                             wgpu::TextureFormat secondFormat,
                             wgpu::TextureFormat thirdFormat = wgpu::TextureFormat::Undefined) {
            Format internalFormat;
            internalFormat.format = format;
            internalFormat.baseFormat = format;
            internalFormat.subSampling = subSampling;
            internalFormat.isRenderable = capabilites & Cap::Renderable;
            internalFormat.isBlendable = false;
            internalFormat.isCompressed = false;
            internalFormat.unsupportedReason = unsupportedReason;
            internalFormat.supportsStorageUsage = false;
            internalFormat.supportsMultisample = capabilites & Cap::Multisample;
            internalFormat.supportsResolveTarget = false;
            internalFormat.aspects = aspects;
            internalFormat.componentCount = static_cast<uint32_t>(componentCount);

            // Multi aspect formats just copy information about single-aspect formats. This
            // means that the single-plane formats must have been added before multi-aspect
            // ones. (it is ASSERTed below).
            const FormatIndex firstFormatIndex = ComputeFormatIndex(firstFormat);
            const FormatIndex secondFormatIndex = ComputeFormatIndex(secondFormat);

            DAWN_ASSERT(table[firstFormatIndex].aspectInfo[0].format !=
                        wgpu::TextureFormat::Undefined);
            DAWN_ASSERT(table[secondFormatIndex].aspectInfo[0].format !=
                        wgpu::TextureFormat::Undefined);

            internalFormat.aspectInfo[0] = table[firstFormatIndex].aspectInfo[0];
            internalFormat.aspectInfo[1] = table[secondFormatIndex].aspectInfo[0];
            if (thirdFormat != wgpu::TextureFormat::Undefined) {
                const FormatIndex thirdFormatIndex = ComputeFormatIndex(thirdFormat);
                internalFormat.aspectInfo[2] = table[thirdFormatIndex].aspectInfo[0];
            }

            AddFormat(internalFormat);
        };

    // Integer texture mutlisampling is only supported in core
    auto intMultisampleCaps = device->IsCompatibilityMode() ? Cap::None : Cap::Multisample;

    // clang-format off
    // 1 byte color formats
    auto r8unormSupportsStorage = device->HasFeature(Feature::R8UnormStorage) ? Cap::StorageROrW : Cap::None;
    AddColorFormat(wgpu::TextureFormat::R8Unorm, Cap::Renderable | Cap::Multisample | Cap::Resolve | r8unormSupportsStorage | Cap::Blendable, ByteSize(1), kAnyFloat, ComponentCount(1), RenderTargetPixelByteCost(1), RenderTargetComponentAlignment(1));
    AddColorFormat(wgpu::TextureFormat::R8Snorm, Cap::None, ByteSize(1), kAnyFloat, ComponentCount(1));
    AddColorFormat(wgpu::TextureFormat::R8Uint, Cap::Renderable | intMultisampleCaps, ByteSize(1), SampleTypeBit::Uint, ComponentCount(1), RenderTargetPixelByteCost(1), RenderTargetComponentAlignment(1));
    AddColorFormat(wgpu::TextureFormat::R8Sint, Cap::Renderable | intMultisampleCaps, ByteSize(1), SampleTypeBit::Sint, ComponentCount(1), RenderTargetPixelByteCost(1), RenderTargetComponentAlignment(1));

    // 2 bytes color formats
    AddColorFormat(wgpu::TextureFormat::R16Uint, Cap::Renderable | intMultisampleCaps, ByteSize(2), SampleTypeBit::Uint, ComponentCount(1), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::R16Sint, Cap::Renderable | intMultisampleCaps, ByteSize(2), SampleTypeBit::Sint, ComponentCount(1), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::R16Float, Cap::Renderable | Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(2), kAnyFloat, ComponentCount(1), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::RG8Unorm, Cap::Renderable | Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(2), kAnyFloat, ComponentCount(2), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(1));
    AddColorFormat(wgpu::TextureFormat::RG8Snorm, Cap::None, ByteSize(2), kAnyFloat, ComponentCount(2));
    AddColorFormat(wgpu::TextureFormat::RG8Uint, Cap::Renderable | intMultisampleCaps, ByteSize(2), SampleTypeBit::Uint, ComponentCount(2), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(1));
    AddColorFormat(wgpu::TextureFormat::RG8Sint, Cap::Renderable | intMultisampleCaps, ByteSize(2), SampleTypeBit::Sint, ComponentCount(2), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(1));

    // 4 bytes color formats
    SampleTypeBit sampleTypeFor32BitFloatFormats = device->HasFeature(Feature::Float32Filterable) ? kAnyFloat : SampleTypeBit::UnfilterableFloat;
    auto supportsPLS = device->HasFeature(Feature::PixelLocalStorageCoherent) || device->HasFeature(Feature::PixelLocalStorageNonCoherent) ? Cap::PLS : Cap::None;
    auto float32BlendableCaps = device->HasFeature(Feature::Float32Blendable) ? Cap::Blendable : Cap::None;
    // (github.com/gpuweb/gpuweb/issues/5049): r32float compat multisampled support is optional
    auto r32FloatMultisampleCaps = device->IsCompatibilityMode() ? Cap::None : Cap::Multisample;

    AddColorFormat(wgpu::TextureFormat::R32Uint, Cap::Renderable | Cap::StorageROrW | Cap::StorageRW | supportsPLS, ByteSize(4), SampleTypeBit::Uint, ComponentCount(1), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::R32Sint, Cap::Renderable | Cap::StorageROrW | Cap::StorageRW | supportsPLS, ByteSize(4), SampleTypeBit::Sint, ComponentCount(1), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::R32Float,  Cap::Renderable | r32FloatMultisampleCaps | Cap::StorageROrW | Cap::StorageRW | supportsPLS | float32BlendableCaps, ByteSize(4), sampleTypeFor32BitFloatFormats, ComponentCount(1), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RG16Uint, Cap::Renderable | intMultisampleCaps, ByteSize(4), SampleTypeBit::Uint, ComponentCount(2), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::RG16Sint, Cap::Renderable | intMultisampleCaps, ByteSize(4), SampleTypeBit::Sint, ComponentCount(2), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::RG16Float, Cap::Renderable | Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(4), kAnyFloat, ComponentCount(2), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::RGBA8Unorm, Cap::Renderable | Cap::StorageROrW | Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(4), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(1));
    AddColorFormat(wgpu::TextureFormat::RGBA8UnormSrgb, Cap::Renderable | Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(4), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(1), wgpu::TextureFormat::RGBA8Unorm);
    AddColorFormat(wgpu::TextureFormat::RGBA8Snorm, Cap::StorageROrW, ByteSize(4), kAnyFloat, ComponentCount(4));
    AddColorFormat(wgpu::TextureFormat::RGBA8Uint, Cap::Renderable | Cap::StorageROrW | intMultisampleCaps, ByteSize(4), SampleTypeBit::Uint, ComponentCount(4), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(1));
    AddColorFormat(wgpu::TextureFormat::RGBA8Sint, Cap::Renderable | Cap::StorageROrW | intMultisampleCaps, ByteSize(4), SampleTypeBit::Sint, ComponentCount(4), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(1));

    const UnsupportedReason externalUnsupportedReason = device->HasFeature(Feature::YCbCrVulkanSamplers) ?  Format::supported : RequiresFeature{wgpu::FeatureName::YCbCrVulkanSamplers};
    AddConditionalColorFormat(wgpu::TextureFormat::External, externalUnsupportedReason, Cap::None, ByteSize(1), SampleTypeBit::External, ComponentCount(0));

    auto BGRA8UnormSupportsStorageUsage = device->HasFeature(Feature::BGRA8UnormStorage) ? Cap::StorageROrW : Cap::None;
    AddColorFormat(wgpu::TextureFormat::BGRA8Unorm, Cap::Renderable | BGRA8UnormSupportsStorageUsage | Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(4), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(1));
    AddConditionalColorFormat(wgpu::TextureFormat::BGRA8UnormSrgb, device->IsCompatibilityMode() ? UnsupportedReason(CompatibilityMode{}) : Format::supported, Cap::Renderable |  Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(4), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(1), wgpu::TextureFormat::BGRA8Unorm);
    AddColorFormat(wgpu::TextureFormat::RGB10A2Uint, Cap::Renderable | intMultisampleCaps, ByteSize(4), SampleTypeBit::Uint, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RGB10A2Unorm, Cap::Renderable | Cap::Multisample | Cap::Resolve | Cap::Blendable, ByteSize(4), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(4));

    auto isRG11B10UfloatCapabilities = device->HasFeature(Feature::RG11B10UfloatRenderable) ? Cap::Renderable | Cap::Multisample | Cap::Resolve | Cap::Blendable : Cap::None;
    AddColorFormat(wgpu::TextureFormat::RG11B10Ufloat, isRG11B10UfloatCapabilities, ByteSize(4), kAnyFloat, ComponentCount(3), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RGB9E5Ufloat, Cap::None, ByteSize(4), kAnyFloat, ComponentCount(3));

    // 8 bytes color formats
    auto rg32StorageCaps = device->IsCompatibilityMode() ? Cap::None : Cap::StorageROrW;
    // (github.com/gpuweb/gpuweb/issues/5049): rgba16float compat multisampled support is optional
    auto rgba16FloatMultisampleCaps = device->IsCompatibilityMode() ? Cap::None : Cap::Multisample;

    AddColorFormat(wgpu::TextureFormat::RG32Uint, Cap::Renderable | rg32StorageCaps, ByteSize(8), SampleTypeBit::Uint, ComponentCount(2), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RG32Sint, Cap::Renderable | rg32StorageCaps, ByteSize(8), SampleTypeBit::Sint, ComponentCount(2), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RG32Float, Cap::Renderable | rg32StorageCaps | float32BlendableCaps, ByteSize(8), sampleTypeFor32BitFloatFormats, ComponentCount(2), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RGBA16Uint, Cap::Renderable | Cap::StorageROrW | intMultisampleCaps, ByteSize(8), SampleTypeBit::Uint, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::RGBA16Sint, Cap::Renderable | Cap::StorageROrW | intMultisampleCaps, ByteSize(8), SampleTypeBit::Sint, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(2));
    AddColorFormat(wgpu::TextureFormat::RGBA16Float, Cap::Renderable | Cap::StorageROrW | rgba16FloatMultisampleCaps | Cap::Resolve | Cap::Blendable, ByteSize(8), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(2));

    // 16 bytes color formats
    AddColorFormat(wgpu::TextureFormat::RGBA32Uint, Cap::Renderable | Cap::StorageROrW, ByteSize(16), SampleTypeBit::Uint, ComponentCount(4), RenderTargetPixelByteCost(16), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RGBA32Sint, Cap::Renderable | Cap::StorageROrW, ByteSize(16), SampleTypeBit::Sint, ComponentCount(4), RenderTargetPixelByteCost(16), RenderTargetComponentAlignment(4));
    AddColorFormat(wgpu::TextureFormat::RGBA32Float, Cap::Renderable | Cap::StorageROrW | float32BlendableCaps, ByteSize(16), sampleTypeFor32BitFloatFormats, ComponentCount(4), RenderTargetPixelByteCost(16), RenderTargetComponentAlignment(4));

    bool norm16TextureFormats = device->HasFeature(Feature::Norm16TextureFormats);
    // Unorm16 color formats
    auto unorm16Supported = (norm16TextureFormats || device->HasFeature(Feature::Unorm16TextureFormats)) ? Format::supported : RequiresFeature{wgpu::FeatureName::Unorm16TextureFormats};
    AddConditionalColorFormat(wgpu::TextureFormat::R16Unorm, unorm16Supported, Cap::Renderable | Cap::Multisample | Cap::Resolve, ByteSize(2), kAnyFloat, ComponentCount(1), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(2));
    AddConditionalColorFormat(wgpu::TextureFormat::RG16Unorm, unorm16Supported, Cap::Renderable | Cap::Multisample | Cap::Resolve, ByteSize(4), kAnyFloat, ComponentCount(2), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(2));
    AddConditionalColorFormat(wgpu::TextureFormat::RGBA16Unorm, unorm16Supported, Cap::Renderable | Cap::Multisample | Cap::Resolve, ByteSize(8), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(2));

    // Snorm16 color formats
    auto snorm16Supported = (norm16TextureFormats || device->HasFeature(Feature::Snorm16TextureFormats)) ? Format::supported : RequiresFeature{wgpu::FeatureName::Snorm16TextureFormats};
    AddConditionalColorFormat(wgpu::TextureFormat::R16Snorm, snorm16Supported, Cap::Renderable | Cap::Multisample | Cap::Resolve, ByteSize(2), kAnyFloat, ComponentCount(1), RenderTargetPixelByteCost(2), RenderTargetComponentAlignment(2));
    AddConditionalColorFormat(wgpu::TextureFormat::RG16Snorm, snorm16Supported, Cap::Renderable | Cap::Multisample | Cap::Resolve, ByteSize(4), kAnyFloat, ComponentCount(2), RenderTargetPixelByteCost(4), RenderTargetComponentAlignment(2));
    AddConditionalColorFormat(wgpu::TextureFormat::RGBA16Snorm, snorm16Supported, Cap::Renderable | Cap::Multisample | Cap::Resolve, ByteSize(8), kAnyFloat, ComponentCount(4), RenderTargetPixelByteCost(8), RenderTargetComponentAlignment(2));

    // Depth-stencil formats
    AddStencilFormat(wgpu::TextureFormat::Stencil8, Format::supported);
    AddDepthFormat(wgpu::TextureFormat::Depth16Unorm, 2, Format::supported);
    // TODO(crbug.com/dawn/843): This is 4 because we read this to perform zero initialization,
    // and textures are always use depth32float. We should improve this to be more robust. Perhaps,
    // using 0 here to mean "unsized" and adding a backend-specific query for the block size.
    AddDepthFormat(wgpu::TextureFormat::Depth24Plus, 4, Format::supported);
    AddMultiAspectFormat(wgpu::TextureFormat::Depth24PlusStencil8, TextureSubsampling::Undefined,
                          Aspect::Depth | Aspect::Stencil, Cap::Renderable | Cap::Multisample, Format::supported, ComponentCount(2), wgpu::TextureFormat::Depth24Plus, wgpu::TextureFormat::Stencil8);
    AddDepthFormat(wgpu::TextureFormat::Depth32Float, 4, Format::supported);
    UnsupportedReason d32s8UnsupportedReason = device->HasFeature(Feature::Depth32FloatStencil8) ? Format::supported : RequiresFeature{wgpu::FeatureName::Depth32FloatStencil8};
    AddMultiAspectFormat(wgpu::TextureFormat::Depth32FloatStencil8, TextureSubsampling::Undefined,
                          Aspect::Depth | Aspect::Stencil, Cap::Renderable | Cap::Multisample, d32s8UnsupportedReason, ComponentCount(2), wgpu::TextureFormat::Depth32Float, wgpu::TextureFormat::Stencil8);

    // BC compressed formats
    UnsupportedReason bcFormatUnsupportedReason = device->HasFeature(Feature::TextureCompressionBC) ? Format::supported : RequiresFeature{wgpu::FeatureName::TextureCompressionBC};
    AddCompressedFormat(wgpu::TextureFormat::BC1RGBAUnorm, ByteSize(8), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::BC1RGBAUnormSrgb, ByteSize(8), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::BC1RGBAUnorm);
    AddCompressedFormat(wgpu::TextureFormat::BC4RSnorm, ByteSize(8), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(1));
    AddCompressedFormat(wgpu::TextureFormat::BC4RUnorm, ByteSize(8), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(1));
    AddCompressedFormat(wgpu::TextureFormat::BC2RGBAUnorm, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::BC2RGBAUnormSrgb, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::BC2RGBAUnorm);
    AddCompressedFormat(wgpu::TextureFormat::BC3RGBAUnorm, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::BC3RGBAUnormSrgb, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::BC3RGBAUnorm);
    AddCompressedFormat(wgpu::TextureFormat::BC5RGSnorm, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(2));
    AddCompressedFormat(wgpu::TextureFormat::BC5RGUnorm, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(2));
    AddCompressedFormat(wgpu::TextureFormat::BC6HRGBFloat, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(3));
    AddCompressedFormat(wgpu::TextureFormat::BC6HRGBUfloat, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(3));
    AddCompressedFormat(wgpu::TextureFormat::BC7RGBAUnorm, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::BC7RGBAUnormSrgb, ByteSize(16), Width(4), Height(4), bcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::BC7RGBAUnorm);

    // ETC2/EAC compressed formats
    UnsupportedReason etc2FormatUnsupportedReason = device->HasFeature(Feature::TextureCompressionETC2) ?  Format::supported : RequiresFeature{wgpu::FeatureName::TextureCompressionETC2};
    AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8Unorm, ByteSize(8), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(3));
    AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8UnormSrgb, ByteSize(8), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(3), wgpu::TextureFormat::ETC2RGB8Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8A1Unorm, ByteSize(8), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ETC2RGB8A1UnormSrgb, ByteSize(8), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ETC2RGB8A1Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ETC2RGBA8Unorm, ByteSize(16), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ETC2RGBA8UnormSrgb, ByteSize(16), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ETC2RGBA8Unorm);
    AddCompressedFormat(wgpu::TextureFormat::EACR11Unorm, ByteSize(8), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(1));
    AddCompressedFormat(wgpu::TextureFormat::EACR11Snorm, ByteSize(8), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(1));
    AddCompressedFormat(wgpu::TextureFormat::EACRG11Unorm, ByteSize(16), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(2));
    AddCompressedFormat(wgpu::TextureFormat::EACRG11Snorm, ByteSize(16), Width(4), Height(4), etc2FormatUnsupportedReason, ComponentCount(2));

    // ASTC compressed formats
    UnsupportedReason astcFormatUnsupportedReason = device->HasFeature(Feature::TextureCompressionASTC) ?  Format::supported : RequiresFeature{wgpu::FeatureName::TextureCompressionASTC};
    AddCompressedFormat(wgpu::TextureFormat::ASTC4x4Unorm, ByteSize(16), Width(4), Height(4), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC4x4UnormSrgb, ByteSize(16), Width(4), Height(4), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC4x4Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC5x4Unorm, ByteSize(16), Width(5), Height(4), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC5x4UnormSrgb, ByteSize(16), Width(5), Height(4), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC5x4Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC5x5Unorm, ByteSize(16), Width(5), Height(5), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC5x5UnormSrgb, ByteSize(16), Width(5), Height(5), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC5x5Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC6x5Unorm, ByteSize(16), Width(6), Height(5), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC6x5UnormSrgb, ByteSize(16), Width(6), Height(5), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC6x5Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC6x6Unorm, ByteSize(16), Width(6), Height(6), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC6x6UnormSrgb, ByteSize(16), Width(6), Height(6), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC6x6Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC8x5Unorm, ByteSize(16), Width(8), Height(5), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC8x5UnormSrgb, ByteSize(16), Width(8), Height(5), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC8x5Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC8x6Unorm, ByteSize(16), Width(8), Height(6), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC8x6UnormSrgb, ByteSize(16), Width(8), Height(6), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC8x6Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC8x8Unorm, ByteSize(16), Width(8), Height(8), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC8x8UnormSrgb, ByteSize(16), Width(8), Height(8), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC8x8Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x5Unorm, ByteSize(16), Width(10), Height(5), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x5UnormSrgb, ByteSize(16), Width(10), Height(5), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC10x5Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x6Unorm, ByteSize(16), Width(10), Height(6), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x6UnormSrgb, ByteSize(16), Width(10), Height(6), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC10x6Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x8Unorm, ByteSize(16), Width(10), Height(8), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x8UnormSrgb, ByteSize(16), Width(10), Height(8), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC10x8Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x10Unorm, ByteSize(16), Width(10), Height(10), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC10x10UnormSrgb, ByteSize(16), Width(10), Height(10), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC10x10Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC12x10Unorm, ByteSize(16), Width(12), Height(10), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC12x10UnormSrgb, ByteSize(16), Width(12), Height(10), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC12x10Unorm);
    AddCompressedFormat(wgpu::TextureFormat::ASTC12x12Unorm, ByteSize(16), Width(12), Height(12), astcFormatUnsupportedReason, ComponentCount(4));
    AddCompressedFormat(wgpu::TextureFormat::ASTC12x12UnormSrgb, ByteSize(16), Width(12), Height(12), astcFormatUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::ASTC12x12Unorm);

    // multi-planar formats
    auto multiPlanarCapabilities = device->HasFeature(Feature::MultiPlanarRenderTargets) ? Cap::Renderable : Cap::None;
    const UnsupportedReason multiPlanarFormatNv12UnsupportedReason = device->HasFeature(Feature::DawnMultiPlanarFormats) ?  Format::supported : RequiresFeature{wgpu::FeatureName::DawnMultiPlanarFormats};
    AddMultiAspectFormat(wgpu::TextureFormat::R8BG8Biplanar420Unorm, TextureSubsampling::e420, Aspect::Plane0 | Aspect::Plane1,
        multiPlanarCapabilities, multiPlanarFormatNv12UnsupportedReason, ComponentCount(3), wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm);
    const UnsupportedReason multiPlanarFormatNv16UnsupportedReason = device->HasFeature(Feature::MultiPlanarFormatNv16) ?  Format::supported : RequiresFeature{wgpu::FeatureName::MultiPlanarFormatNv16};
    AddMultiAspectFormat(wgpu::TextureFormat::R8BG8Biplanar422Unorm, TextureSubsampling::e422, Aspect::Plane0 | Aspect::Plane1,
        multiPlanarCapabilities, multiPlanarFormatNv16UnsupportedReason, ComponentCount(3), wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm);
    const UnsupportedReason multiPlanarFormatNv24UnsupportedReason = device->HasFeature(Feature::MultiPlanarFormatNv24) ?  Format::supported : RequiresFeature{wgpu::FeatureName::MultiPlanarFormatNv24};
    AddMultiAspectFormat(wgpu::TextureFormat::R8BG8Biplanar444Unorm, TextureSubsampling::e444, Aspect::Plane0 | Aspect::Plane1,
        multiPlanarCapabilities, multiPlanarFormatNv24UnsupportedReason, ComponentCount(3), wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm);
    const UnsupportedReason multiPlanarFormatP010UnsupportedReason = device->HasFeature(Feature::MultiPlanarFormatP010) ?  Format::supported : RequiresFeature{wgpu::FeatureName::MultiPlanarFormatP010};
    AddMultiAspectFormat(wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm, TextureSubsampling::e420, Aspect::Plane0 | Aspect::Plane1,
        multiPlanarCapabilities, multiPlanarFormatP010UnsupportedReason, ComponentCount(3), wgpu::TextureFormat::R16Unorm, wgpu::TextureFormat::RG16Unorm);
    const UnsupportedReason multiPlanarFormatP210UnsupportedReason = device->HasFeature(Feature::MultiPlanarFormatP210) ?  Format::supported : RequiresFeature{wgpu::FeatureName::MultiPlanarFormatP210};
    AddMultiAspectFormat(wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm, TextureSubsampling::e422, Aspect::Plane0 | Aspect::Plane1,
        multiPlanarCapabilities, multiPlanarFormatP210UnsupportedReason, ComponentCount(3), wgpu::TextureFormat::R16Unorm, wgpu::TextureFormat::RG16Unorm);
    const UnsupportedReason multiPlanarFormatP410UnsupportedReason = device->HasFeature(Feature::MultiPlanarFormatP410) ?  Format::supported : RequiresFeature{wgpu::FeatureName::MultiPlanarFormatP410};
    AddMultiAspectFormat(wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm, TextureSubsampling::e444, Aspect::Plane0 | Aspect::Plane1,
        multiPlanarCapabilities, multiPlanarFormatP410UnsupportedReason, ComponentCount(3), wgpu::TextureFormat::R16Unorm, wgpu::TextureFormat::RG16Unorm);
    const UnsupportedReason multiPlanarFormatNv12aUnsupportedReason = device->HasFeature(Feature::MultiPlanarFormatNv12a) ?  Format::supported : RequiresFeature{wgpu::FeatureName::MultiPlanarFormatNv12a};
    AddMultiAspectFormat(wgpu::TextureFormat::R8BG8A8Triplanar420Unorm, TextureSubsampling::e420, Aspect::Plane0 | Aspect::Plane1 | Aspect::Plane2,
        multiPlanarCapabilities, multiPlanarFormatNv12aUnsupportedReason, ComponentCount(4), wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::R8Unorm);
    // clang-format on

    // This checks that each format is set at least once, the second part of checking that all
    // formats are checked exactly once. If this assertion is failing and texture formats have
    // been added or removed recently, check that kKnownFormatCount has been updated.
    DAWN_ASSERT(formatsSet.all());

    for (const Format& f : table) {
        if (f.format != f.baseFormat) {
            auto& baseViewFormat = table[ComputeFormatIndex(f.baseFormat)].baseViewFormat;
            // Currently, Dawn only supports sRGB reinterpretation, so there should only be one
            // view format.
            DAWN_ASSERT(baseViewFormat == wgpu::TextureFormat::Undefined);
            baseViewFormat = f.format;
        }
    }
    return table;
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const UnsupportedReason& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    MatchVariant(
        value, [](const std::monostate&) { DAWN_UNREACHABLE(); },
        [s](const RequiresFeature& requiresFeature) {
            s->Append(absl::StrFormat("requires feature %s", requiresFeature.feature));
        },
        [s](const CompatibilityMode&) { s->Append("not supported in compatibility mode"); });
    return {true};
}

}  // namespace dawn::native
