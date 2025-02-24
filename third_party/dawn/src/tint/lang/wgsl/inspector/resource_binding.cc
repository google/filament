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

#include "src/tint/lang/wgsl/inspector/resource_binding.h"

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"

namespace tint::inspector {

ResourceBinding::TextureDimension TypeTextureDimensionToResourceBindingTextureDimension(
    const core::type::TextureDimension& type_dim) {
    switch (type_dim) {
        case core::type::TextureDimension::k1d:
            return ResourceBinding::TextureDimension::k1d;
        case core::type::TextureDimension::k2d:
            return ResourceBinding::TextureDimension::k2d;
        case core::type::TextureDimension::k2dArray:
            return ResourceBinding::TextureDimension::k2dArray;
        case core::type::TextureDimension::k3d:
            return ResourceBinding::TextureDimension::k3d;
        case core::type::TextureDimension::kCube:
            return ResourceBinding::TextureDimension::kCube;
        case core::type::TextureDimension::kCubeArray:
            return ResourceBinding::TextureDimension::kCubeArray;
        case core::type::TextureDimension::kNone:
            return ResourceBinding::TextureDimension::kNone;
    }
    return ResourceBinding::TextureDimension::kNone;
}

ResourceBinding::SampledKind BaseTypeToSampledKind(const core::type::Type* base_type) {
    if (!base_type) {
        return ResourceBinding::SampledKind::kUnknown;
    }

    if (auto* at = base_type->As<core::type::Array>()) {
        base_type = at->ElemType();
    } else if (auto* mt = base_type->As<core::type::Matrix>()) {
        base_type = mt->Type();
    } else if (auto* vt = base_type->As<core::type::Vector>()) {
        base_type = vt->Type();
    }

    if (base_type->Is<core::type::F32>()) {
        return ResourceBinding::SampledKind::kFloat;
    } else if (base_type->Is<core::type::U32>()) {
        return ResourceBinding::SampledKind::kUInt;
    } else if (base_type->Is<core::type::I32>()) {
        return ResourceBinding::SampledKind::kSInt;
    } else {
        return ResourceBinding::SampledKind::kUnknown;
    }
}

ResourceBinding::TexelFormat TypeTexelFormatToResourceBindingTexelFormat(
    const core::TexelFormat& image_format) {
    switch (image_format) {
        case core::TexelFormat::kBgra8Unorm:
            return ResourceBinding::TexelFormat::kBgra8Unorm;
        case core::TexelFormat::kR32Uint:
            return ResourceBinding::TexelFormat::kR32Uint;
        case core::TexelFormat::kR32Sint:
            return ResourceBinding::TexelFormat::kR32Sint;
        case core::TexelFormat::kR32Float:
            return ResourceBinding::TexelFormat::kR32Float;
        case core::TexelFormat::kRgba8Unorm:
            return ResourceBinding::TexelFormat::kRgba8Unorm;
        case core::TexelFormat::kRgba8Snorm:
            return ResourceBinding::TexelFormat::kRgba8Snorm;
        case core::TexelFormat::kRgba8Uint:
            return ResourceBinding::TexelFormat::kRgba8Uint;
        case core::TexelFormat::kRgba8Sint:
            return ResourceBinding::TexelFormat::kRgba8Sint;
        case core::TexelFormat::kRg32Uint:
            return ResourceBinding::TexelFormat::kRg32Uint;
        case core::TexelFormat::kRg32Sint:
            return ResourceBinding::TexelFormat::kRg32Sint;
        case core::TexelFormat::kRg32Float:
            return ResourceBinding::TexelFormat::kRg32Float;
        case core::TexelFormat::kRgba16Uint:
            return ResourceBinding::TexelFormat::kRgba16Uint;
        case core::TexelFormat::kRgba16Sint:
            return ResourceBinding::TexelFormat::kRgba16Sint;
        case core::TexelFormat::kRgba16Float:
            return ResourceBinding::TexelFormat::kRgba16Float;
        case core::TexelFormat::kRgba32Uint:
            return ResourceBinding::TexelFormat::kRgba32Uint;
        case core::TexelFormat::kRgba32Sint:
            return ResourceBinding::TexelFormat::kRgba32Sint;
        case core::TexelFormat::kRgba32Float:
            return ResourceBinding::TexelFormat::kRgba32Float;
        case core::TexelFormat::kR8Unorm:
            return ResourceBinding::TexelFormat::kR8Unorm;
        case core::TexelFormat::kUndefined:
            return ResourceBinding::TexelFormat::kNone;
    }
    return ResourceBinding::TexelFormat::kNone;
}

}  // namespace tint::inspector
