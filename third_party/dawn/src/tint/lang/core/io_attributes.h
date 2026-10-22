// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IO_ATTRIBUTES_H_
#define SRC_TINT_LANG_CORE_IO_ATTRIBUTES_H_

#include <cstdint>
#include <optional>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/interpolation.h"

namespace tint::core {

/// Attributes that can be applied to an object that will be used for shader IO.
struct IOAttributes {
    /// The value of a `@location` attribute.
    std::optional<uint32_t> location = std::nullopt;
    /// The value of a `@blend_src` attribute.
    std::optional<uint32_t> blend_src = std::nullopt;
    /// The value of a `@color` attribute.
    std::optional<uint32_t> color = std::nullopt;
    /// The value of a `@builtin` attribute.
    std::optional<core::BuiltinValue> builtin = std::nullopt;
    /// The depth mode of a `@builtin` attribute.
    std::optional<core::BuiltinDepthMode> depth_mode = std::nullopt;
    /// The values of a `@interpolate` attribute.
    std::optional<core::Interpolation> interpolation = std::nullopt;
    /// The value of an `@input_attachment_index` attribute
    std::optional<uint32_t> input_attachment_index = std::nullopt;
    /// The value of the `@binding` and `@group` attributes
    std::optional<BindingPoint> binding_point = std::nullopt;
    /// True if the object is annotated with `@invariant`.
    bool invariant = false;
};

/// Used for referencing/tagging a specific IOAttribute.
/// IOAttributes above is intentionally not a key-value map (e.g. HashSet) using this enum, since it
/// has heterogeneous value types and would also cease to be a POD.
enum class IOAttributeKind : uint8_t {
    kLocation,
    kBlendSrc,
    kColor,
    kBuiltin,
    kDepthMode,
    kInterpolation,
    kInputAttachmentIndex,
    kBindingPoint,
    kInvariant,
};

/// @returns a human-readable string representation of @p kind
inline std::string_view ToString(const IOAttributeKind kind) {
    switch (kind) {
        case IOAttributeKind::kLocation:
            return "location";
        case IOAttributeKind::kBlendSrc:
            return "blend src";
        case IOAttributeKind::kColor:
            return "color";
        case IOAttributeKind::kBuiltin:
            return "builtin";
        case IOAttributeKind::kDepthMode:
            return "depth mode";
        case IOAttributeKind::kInterpolation:
            return "interpolation";
        case IOAttributeKind::kInputAttachmentIndex:
            return "input attachment index";
        case IOAttributeKind::kBindingPoint:
            return "binding point";
        case IOAttributeKind::kInvariant:
            return "invariant";
    }
    TINT_ICE() << "Unknown kind passed to ToString(IOAttributeKind)";
}

}  // namespace tint::core

#endif  // SRC_TINT_LANG_CORE_IO_ATTRIBUTES_H_
