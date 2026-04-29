// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_BINDINGINFO_H_
#define SRC_DAWN_NATIVE_BINDINGINFO_H_

#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Ref.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/Error.h"
#include "dawn/native/Format.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

// Not a real WebGPU limit, but used to optimize parts of Dawn which expect valid usage of the
// API. There should never be more bindings than the max per stage, for each stage.
inline constexpr uint32_t kMaxBindingsPerPipelineLayout =
    3 * (kMaxSampledTexturesPerShaderStage + kMaxSamplersPerShaderStage +
         kMaxStorageBuffersPerShaderStage + kMaxStorageTexturesPerShaderStage +
         kMaxUniformBuffersPerShaderStage);

inline constexpr BindingIndex kMaxBindingsPerPipelineLayoutTyped =
    BindingIndex(kMaxBindingsPerPipelineLayout);

// TODO(enga): Figure out a good number for this.
inline constexpr uint32_t kMaxOptimalBindingsPerGroup = 32;

enum class BindingInfoType {
    Buffer,
    Sampler,
    Texture,
    StorageTexture,
    TexelBuffer,
    ExternalTexture,
    StaticSampler,
    // Internal to vulkan only.
    InputAttachment,
};

// A mirror of wgpu::BufferBindingLayout for use inside dawn::native.
#define BUFFER_BINDING_INFO_MEMBER(X)                           \
    X(wgpu::BufferBindingType, type)                            \
    X(uint64_t, minBindingSize)                                 \
    /* hasDynamicOffset is always false in shader reflection */ \
    X(bool, hasDynamicOffset)
DAWN_SERIALIZABLE(struct, BufferBindingInfo, BUFFER_BINDING_INFO_MEMBER) {
    static BufferBindingInfo From(const BufferBindingLayout& layout);
};
#undef BUFFER_BINDING_INFO_MEMBER

// A mirror of wgpu::TextureBindingLayout for use inside dawn::native.
#define TEXTURE_BINDING_INFO_MEMBER(X)           \
    X(wgpu::TextureSampleType, sampleType)       \
    X(wgpu::TextureViewDimension, viewDimension) \
    X(bool, multisampled)
DAWN_SERIALIZABLE(struct, TextureBindingInfo, TEXTURE_BINDING_INFO_MEMBER) {
    static TextureBindingInfo From(const TextureBindingLayout& layout);
};
#undef TEXTURE_BINDING_INFO_MEMBER

// A mirror of wgpu::StorageTextureBindingLayout for use inside dawn::native.
#define STORAGE_TEXTURE_BINDING_INFO_MEMBER(X)   \
    X(wgpu::TextureFormat, format)               \
    X(wgpu::TextureViewDimension, viewDimension) \
    X(wgpu::StorageTextureAccess, access)
DAWN_SERIALIZABLE(struct, StorageTextureBindingInfo, STORAGE_TEXTURE_BINDING_INFO_MEMBER) {
    static StorageTextureBindingInfo From(const StorageTextureBindingLayout& layout);
};
#undef STORAGE_TEXTURE_BINDING_INFO_MEMBER

// A mirror of wgpu::TexelBufferBindingLayout for use inside dawn::native.
#define TEXEL_BUFFER_BINDING_INFO_MEMBER(X) \
    X(wgpu::TextureFormat, format)          \
    X(wgpu::TexelBufferAccess, access)
DAWN_SERIALIZABLE(struct, TexelBufferBindingInfo, TEXEL_BUFFER_BINDING_INFO_MEMBER) {
    static TexelBufferBindingInfo From(const TexelBufferBindingLayout& layout);
};
#undef TEXEL_BUFFER_BINDING_INFO_MEMBER

// A mirror of wgpu::SamplerBindingLayout for use inside dawn::native.
#define SAMPLER_BINDING_INFO_MEMBER(X) X(wgpu::SamplerBindingType, type)
DAWN_SERIALIZABLE(struct, SamplerBindingInfo, SAMPLER_BINDING_INFO_MEMBER) {
    static SamplerBindingInfo From(const SamplerBindingLayout& layout);
};
#undef SAMPLER_BINDING_INFO_MEMBER

// The binding layout for ExternalTexture contains the indices of the expanded entries for it.
#define EXTERNAL_TEXTURE_BINDING_INFO_MEMBER(X) \
    X(BindingIndex, metadata)                   \
    X(BindingIndex, plane0)                     \
    X(BindingIndex, plane1)                     \
    X(std::optional<BindingIndex>, staticSampler)
DAWN_SERIALIZABLE(struct, ExternalTextureBindingInfo, EXTERNAL_TEXTURE_BINDING_INFO_MEMBER){};
#undef EXTERNAL_TEXTURE_BINDING_INFO_MEMBER

// Internal to vulkan only.
#define INPUT_ATTACHMENT_BINDING_INFO_MEMBER(X) X(wgpu::TextureSampleType, sampleType)
DAWN_SERIALIZABLE(struct, InputAttachmentBindingInfo, INPUT_ATTACHMENT_BINDING_INFO_MEMBER){};
#undef INPUT_ATTACHMENT_BINDING_INFO_MEMBER

// Describes what a static sampler binding is used for. While it could just be a boolean for whether
// or not a single texture is required, having the enums lets us do more precise assertions.
enum class StaticSamplerUse : uint8_t {
    // A static sampler that can be combined with any (non-YCbCr) texture in the shader.
    Freestanding,
    // A YCbCr sampler that can only be used to sample the texture at `sampledTextureIndex`.
    SingleTextureYCbCr,
    // A static sampler created in the expansion of an ExternalTexture, so a YCbCr sampler may be
    // swapped in if needed during pipeline specialization in the Vulkan backend.
    InternalForExternalTexture,
};

// A mirror of wgpu::StaticSamplerBindingLayout for use inside dawn::native.
struct StaticSamplerBindingInfo {
    // Note that this doesn't initialize the BindingIndex member as it is computed after sorting the
    // entries in the BindGroupLayout.
    static StaticSamplerBindingInfo From(const StaticSamplerBindingLayout& layout);

    // Hold a ref as the sampler to keep it alive even if it is freed after BGL creation.
    Ref<SamplerBase> sampler;
    // Holds the BindingIndex of the single texture with which this sampler is statically paired, if
    // any.
    BindingIndex sampledTextureIndex = BindingIndex(0);
    // What this sampler will be used for.
    StaticSamplerUse use;

    bool operator==(const StaticSamplerBindingInfo& other) const = default;
};

struct BindingInfo {
    BindingNumber binding;
    wgpu::ShaderStage visibility;

    // The size of the array this binding is part of. Each BindingInfo represents a single entry.
    BindingIndex arraySize{1u};
    // The index of this entry in the array. Must be 0 if this entry is not in an array.
    BindingIndex indexInArray{0u};

    std::variant<BufferBindingInfo,
                 SamplerBindingInfo,
                 TextureBindingInfo,
                 TexelBufferBindingInfo,
                 StorageTextureBindingInfo,
                 ExternalTextureBindingInfo,
                 StaticSamplerBindingInfo,
                 InputAttachmentBindingInfo>
        bindingLayout;

    bool operator==(const BindingInfo& other) const = default;
};

BindingInfoType GetBindingInfoType(const BindingInfo& bindingInfo);
// Returns the type of binding contained in a valid BindGroupLayoutEntry.
BindingInfoType GetBindingInfoType(const BindGroupLayoutEntry* entry);

// Match tint::BindingPoint, can convert to/from tint::BindingPoint using ToTint and FromTint.
#define WGSL_BIND_POINT_MEMBER(X) \
    X(BindGroupIndex, group)      \
    X(BindingNumber, binding)
// clang-format off
DAWN_SERIALIZABLE(struct, WGSLBindPoint, WGSL_BIND_POINT_MEMBER){
    constexpr WGSLBindPoint() = default;
    constexpr WGSLBindPoint(BindGroupIndex groupIn, BindingNumber bindingIn)
        : Contents{groupIn, bindingIn} {}

    constexpr bool operator<(const WGSLBindPoint& rhs) const {
        if (group < rhs.group) {
            return true;
        }
        if (group > rhs.group) {
            return false;
        }
        return binding < rhs.binding;
    }

    template <typename H>
    friend H AbslHashValue(H h, const WGSLBindPoint& b) {
        return H::combine(std::move(h), b.group, b.binding);
    }
};
// clang-format on
#undef BINDING_SLOT_MEMBER

// Represents a single binding in a pipeline.
struct BindPoint {
    BindGroupIndex group;
    BindingIndex binding;

    template <typename H>
    friend H AbslHashValue(H h, const BindPoint& b) {
        return H::combine(std::move(h), b.group, b.binding);
    }
    constexpr bool operator==(const BindPoint& other) const = default;
};

// Represents a single pre-expansion binding in a pipeline.
struct APIBindPoint {
    BindGroupIndex group;
    APIBindingIndex binding;

    template <typename H>
    friend H AbslHashValue(H h, const APIBindPoint& b) {
        return H::combine(std::move(h), b.group, b.binding);
    }
    constexpr bool operator==(const APIBindPoint& other) const = default;
};

struct PerStageBindingCounts {
    uint32_t sampledTextureCount;
    uint32_t samplerCount;
    uint32_t storageBufferCount;
    uint32_t storageTextureCount;
    uint32_t texelBufferCount;
    uint32_t uniformBufferCount;
    uint32_t externalTextureCount;
    uint32_t staticSamplerCount;
};

struct BindingCounts {
    uint32_t totalCount;
    uint32_t bufferCount;
    uint32_t unverifiedBufferCount;  // Buffers with minimum buffer size unspecified
    uint32_t dynamicUniformBufferCount;
    uint32_t dynamicStorageBufferCount;
    uint32_t staticSamplerCount;
    PerStage<PerStageBindingCounts> perStage;
};

struct CombinedLimits;

void IncrementBindingCounts(BindingCounts* bindingCounts,
                            const UnpackedPtr<BindGroupLayoutEntry>& entry);
void AccumulateBindingCounts(BindingCounts* bindingCounts, const BindingCounts& rhs);
MaybeError ValidateBindingCounts(const CombinedLimits& limits,
                                 const BindingCounts& bindingCounts,
                                 const AdapterBase* adapter);

// For buffer size validation
using RequiredBufferSizes = PerBindGroup<std::vector<uint64_t>>;

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDINGINFO_H_
