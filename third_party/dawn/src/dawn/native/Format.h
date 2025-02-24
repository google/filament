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

#ifndef SRC_DAWN_NATIVE_FORMAT_H_
#define SRC_DAWN_NATIVE_FORMAT_H_

#include <array>
#include <variant>

#include "dawn/native/dawn_platform.h"

#include "dawn/common/TypedInteger.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/EnumClassBitmasks.h"
#include "dawn/native/Error.h"
#include "dawn/native/Subresource.h"

#include "absl/strings/str_format.h"

// About multi-planar formats.
//
// Dawn supports additional multi-planar formats when the multiplanar-formats extension is enabled.
// When enabled, Dawn treats planar data as sub-resources (ie. 1 sub-resource == 1 view == 1 plane).
// A multi-planar format name encodes the channel mapping and order of planes. For example,
// R8BG8Biplanar420Unorm is YUV 4:2:0 where Plane 0 = R8, and Plane 1 = BG8.
//
// Requirements:
// * Plane aspects cannot be combined with color, depth, or stencil aspects.
// * Only compatible multi-planar formats of planes can be used with multi-planar texture
// formats.
// * Can't access multiple planes without creating per plane views (no color conversion).
// * Multi-planar format cannot be written or read without a per plane view.
//
// TODO(dawn:551): Consider moving this comment.

namespace dawn::native {

enum class Aspect : uint8_t;
class DeviceBase;

// This mirrors wgpu::TextureSampleType as a bitmask instead.
// NOTE: SampleTypeBit::External does not have an equivalent TextureSampleType. All future
// additions to SampleTypeBit that have an equivalent TextureSampleType should use
// SampleTypeBit::External's value and update SampleTypeBit::External to a higher value.
// TODO(crbug.com/dawn/2476): Validate SampleTypeBit::External is compatible with Sampler.
enum class SampleTypeBit : uint8_t {
    None = 0x0,
    Float = 0x1,
    UnfilterableFloat = 0x2,
    Depth = 0x4,
    Sint = 0x8,
    Uint = 0x10,
    External = 0x20,
};

// Converts a wgpu::TextureSampleType to its bitmask representation.
SampleTypeBit SampleTypeToSampleTypeBit(wgpu::TextureSampleType sampleType);

struct TexelBlockInfo {
    uint32_t byteSize;
    uint32_t width;
    uint32_t height;
};

enum class TextureComponentType {
    Float,
    Sint,
    Uint,
};

enum class TextureSubsampling {
    Undefined,
    e420,
    e422,
    e444,
};

struct RequiresFeature {
    wgpu::FeatureName feature;
};

struct CompatibilityMode {};

using UnsupportedReason =
    std::variant</* is supported */ std::monostate, RequiresFeature, CompatibilityMode>;

struct AspectInfo {
    TexelBlockInfo block;
    TextureComponentType baseType{};
    SampleTypeBit supportedSampleTypes{};
    wgpu::TextureFormat format = wgpu::TextureFormat::Undefined;
};

// The number of formats Dawn knows about. Asserts in BuildFormatTable ensure that this is the
// exact number of known format.
static constexpr uint32_t kWebGPUFormatCount = 95;
static constexpr uint32_t kDawnFormatCount = 14;
static constexpr uint32_t kKnownFormatCount = kWebGPUFormatCount + kDawnFormatCount;

using FormatIndex = TypedInteger<struct FormatIndexT, uint32_t>;

struct Format;
using FormatTable = ityp::array<FormatIndex, Format, kKnownFormatCount>;

// A wgpu::TextureFormat along with all the information about it necessary for validation.
struct Format {
    wgpu::TextureFormat format = wgpu::TextureFormat::Undefined;

    static const UnsupportedReason supported;

    // TODO(crbug.com/dawn/1332): These members could be stored in a Format capability matrix.
    bool isRenderable = false;
    bool isCompressed = false;
    bool isBlendable = false;
    // A format can be known but not supported because it is part of a disabled extension.
    UnsupportedReason unsupportedReason;
    bool supportsStorageUsage = false;
    bool supportsReadWriteStorageUsage = false;
    bool supportsMultisample = false;
    bool supportsResolveTarget = false;
    bool supportsStorageAttachment = false;
    Aspect aspects{};
    // Only used for renderable color formats:
    uint8_t componentCount = 0;                  // number of color channels
    uint8_t renderTargetPixelByteCost = 0;       // byte cost of pixel in render targets
    uint8_t renderTargetComponentAlignment = 0;  // byte alignment for components in render targets

    bool IsSupported() const;
    bool IsColor() const;
    bool HasDepth() const;
    bool HasStencil() const;
    bool HasDepthOrStencil() const;
    bool HasAlphaChannel() const;
    bool IsSnorm() const;

    // IsMultiPlanar() returns true if the format allows selecting a plane index. This is only
    // allowed by multi-planar formats (ex. NV12).
    bool IsMultiPlanar() const;

    const AspectInfo& GetAspectInfo(wgpu::TextureAspect aspect) const;
    const AspectInfo& GetAspectInfo(Aspect aspect) const;

    // The index of the format in the list of all known formats: a unique number for each format
    // in [0, kKnownFormatCount)
    FormatIndex GetIndex() const;

    // baseFormat represents the memory layout of the format.
    // If two formats has the same baseFormat, they could copy to and be viewed as the other
    // format. Currently two formats have the same baseFormat if they differ only in sRGB-ness.
    wgpu::TextureFormat baseFormat = wgpu::TextureFormat::Undefined;
    // Additional view format a base format is compatible with. Only populated for true base
    // formats. Only stores a single view format because Dawn currently only supports sRGB format
    // reinterpretation.
    wgpu::TextureFormat baseViewFormat = wgpu::TextureFormat::Undefined;
    // Chroma subsampling used by multi-planar formats (e.g. 4:2:0 is 1/2 horizontal and 1/2
    // vertical resolution for UV plane, 4:2:2 is 1/2 horizontal and 1/1 vertical resolution).
    TextureSubsampling subSampling = TextureSubsampling::Undefined;

    // Returns true if the formats are copy compatible.
    // Currently means they differ only in sRGB-ness.
    bool CopyCompatibleWith(const Format& otherFormat) const;

    // Returns true if the formats are texture view format compatible.
    // Currently means they differ only in sRGB-ness.
    bool ViewCompatibleWith(const Format& otherFormat) const;

  private:
    // Used to store the aspectInfo for one or more planes. For single plane "color" formats,
    // only the first aspect info or aspectInfo[0] is valid. For depth-stencil, the first aspect
    // info is depth and the second aspect info is stencil. For multi-planar formats,
    // aspectInfo[i] is the ith plane.
    std::array<AspectInfo, kMaxPlanesPerFormat> aspectInfo{};

    friend FormatTable BuildFormatTable(const DeviceBase* device);
};

class FormatSet : public ityp::bitset<FormatIndex, kKnownFormatCount> {
    using Base = ityp::bitset<FormatIndex, kKnownFormatCount>;

  public:
    using Base::Base;
    using Base::operator[];

    bool operator[](const Format& format) const;
    typename Base::reference operator[](const Format& format);
};

// Implementation details of the format table in the device.

// Returns the index of a format in the FormatTable.
FormatIndex ComputeFormatIndex(wgpu::TextureFormat format);
// Builds the format table with the extensions enabled on the device.
FormatTable BuildFormatTable(const DeviceBase* device);

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    const UnsupportedReason& value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s);

}  // namespace dawn::native

template <>
struct wgpu::IsWGPUBitmask<dawn::native::SampleTypeBit> {
    static constexpr bool enable = true;
};

#endif  // SRC_DAWN_NATIVE_FORMAT_H_
