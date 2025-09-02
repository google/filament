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

#ifndef SRC_TINT_LANG_WGSL_INSPECTOR_RESOURCE_BINDING_H_
#define SRC_TINT_LANG_WGSL_INSPECTOR_RESOURCE_BINDING_H_

#include <cstdint>
#include <optional>
#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::inspector {

/// Container for information about how a resource is bound
struct ResourceBinding {
    /// The dimensionality of a texture
    enum class TextureDimension : uint8_t {
        /// 1 dimensional texture
        k1d,
        /// 2 dimensional texture
        k2d,
        /// 2 dimensional array texture
        k2dArray,
        /// 3 dimensional texture
        k3d,
        /// cube texture
        kCube,
        /// cube array texture
        kCubeArray,
        /// Invalid texture
        kNone,
    };

    /// Component type of the texture's data. Same as the Sampled Type parameter
    /// in SPIR-V OpTypeImage.
    enum class SampledKind : uint8_t { kFloat, kUInt, kSInt, kUnknown };

    /// Enumerator of texel image formats
    enum class TexelFormat : uint8_t {
        kR8Snorm,
        kR8Uint,
        kR8Sint,
        kRg8Unorm,
        kRg8Snorm,
        kRg8Uint,
        kRg8Sint,
        kR16Unorm,
        kR16Snorm,
        kR16Uint,
        kR16Sint,
        kR16Float,
        kRg16Unorm,
        kRg16Snorm,
        kRg16Uint,
        kRg16Sint,
        kRg16Float,
        kBgra8Unorm,
        kRgba8Unorm,
        kRgba8Snorm,
        kRgba8Uint,
        kRgba8Sint,
        kRgba16Unorm,
        kRgba16Snorm,
        kRgba16Uint,
        kRgba16Sint,
        kRgba16Float,
        kR32Uint,
        kR32Sint,
        kR32Float,
        kRg32Uint,
        kRg32Sint,
        kRg32Float,
        kRgba32Uint,
        kRgba32Sint,
        kRgba32Float,
        kR8Unorm,
        kRgb10A2Uint,
        kRgb10A2Unorm,
        kRg11B10Ufloat,
        kNone,
    };

    /// kXXX maps to entries returned by GetXXXResourceBindings call.
    enum class ResourceType {
        kUniformBuffer,
        kStorageBuffer,
        kReadOnlyStorageBuffer,
        kSampler,
        kComparisonSampler,
        kSampledTexture,
        kMultisampledTexture,
        kWriteOnlyStorageTexture,
        kReadOnlyStorageTexture,
        kReadWriteStorageTexture,
        kDepthTexture,
        kDepthMultisampledTexture,
        kExternalTexture,
        kReadOnlyTexelBuffer,
        kReadWriteTexelBuffer,
        kInputAttachment,
    };

    /// Type of resource that is bound.
    ResourceType resource_type;
    /// Bind group the binding belongs
    uint32_t bind_group;
    /// Identifier to identify this binding within the bind group
    uint32_t binding;
    /// Input attachment index. Only available for input attachments.
    uint32_t input_attachment_index;
    /// Size for this binding, in bytes, if defined.
    uint64_t size;
    /// The array_size, if the binding is in a binding_array
    std::optional<uint32_t> array_size;
    /// Size for this binding without trailing structure padding, in bytes, if
    /// defined.
    uint64_t size_no_padding;
    /// Dimensionality of this binding, if defined.
    TextureDimension dim;
    /// Kind of data being sampled, if defined.
    SampledKind sampled_kind;
    /// Format of data, if defined.
    TexelFormat image_format;
    /// Variable name of the binding.
    std::string variable_name;
};

/// Convert from internal core::type::TextureDimension to public
/// ResourceBinding::TextureDimension
/// @param type_dim internal value to convert from
/// @returns the publicly visible equivalent
ResourceBinding::TextureDimension TypeTextureDimensionToResourceBindingTextureDimension(
    const core::type::TextureDimension& type_dim);

/// Infer ResourceBinding::SampledKind for a given core::type::Type
/// @param base_type internal type to infer from
/// @returns the publicly visible equivalent
ResourceBinding::SampledKind BaseTypeToSampledKind(const core::type::Type* base_type);

/// Convert from internal core::TexelFormat to public
/// ResourceBinding::TexelFormat
/// @param image_format internal value to convert from
/// @returns the publicly visible equivalent
ResourceBinding::TexelFormat TypeTexelFormatToResourceBindingTexelFormat(
    const core::TexelFormat& image_format);

}  // namespace tint::inspector

#endif  // SRC_TINT_LANG_WGSL_INSPECTOR_RESOURCE_BINDING_H_
