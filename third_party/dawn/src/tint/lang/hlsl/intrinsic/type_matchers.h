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

#ifndef SRC_TINT_LANG_HLSL_INTRINSIC_TYPE_MATCHERS_H_
#define SRC_TINT_LANG_HLSL_INTRINSIC_TYPE_MATCHERS_H_

#include "src/tint/lang/core/intrinsic/table.h"  // IWYU pragma: export
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/hlsl/type/byte_address_buffer.h"
#include "src/tint/lang/hlsl/type/int8_t4_packed.h"
#include "src/tint/lang/hlsl/type/rasterizer_ordered_texture_2d.h"
#include "src/tint/lang/hlsl/type/uint8_t4_packed.h"

namespace tint::hlsl::intrinsic {

inline bool MatchByteAddressBuffer(core::intrinsic::MatchState&,
                                   const core::type::Type* ty,
                                   core::intrinsic::Number& A) {
    if (auto* buf = ty->As<type::ByteAddressBuffer>()) {
        A = core::intrinsic::Number(static_cast<uint32_t>(buf->Access()));
        return true;
    }
    return false;
}

inline const type::ByteAddressBuffer* BuildByteAddressBuffer(core::intrinsic::MatchState& state,
                                                             const core::type::Type*,
                                                             core::intrinsic::Number& A) {
    return state.types.Get<type::ByteAddressBuffer>(static_cast<core::Access>(A.Value()));
}

inline bool MatchRasterizerOrderedTexture2D(core::intrinsic::MatchState&,
                                            const core::type::Type* ty,
                                            core::intrinsic::Number& F) {
    if (auto* buf = ty->As<type::RasterizerOrderedTexture2D>()) {
        F = core::intrinsic::Number(static_cast<uint32_t>(buf->TexelFormat()));
        return true;
    }
    return false;
}

inline const type::RasterizerOrderedTexture2D* BuildRasterizerOrderedTexture2D(
    core::intrinsic::MatchState& state,
    const core::type::Type*,
    core::intrinsic::Number& F) {
    auto format = static_cast<core::TexelFormat>(F.Value());
    auto* subtype = type::RasterizerOrderedTexture2D::SubtypeFor(format, state.types);
    return state.types.Get<type::RasterizerOrderedTexture2D>(format, subtype);
}

inline bool MatchInt8T4Packed(core::intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<core::intrinsic::Any, type::Int8T4Packed>();
}

inline const type::Int8T4Packed* BuildInt8T4Packed(core::intrinsic::MatchState& state,
                                                   const core::type::Type*) {
    return state.types.Get<type::Int8T4Packed>();
}

inline bool MatchUint8T4Packed(core::intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<core::intrinsic::Any, type::Uint8T4Packed>();
}

inline const type::Uint8T4Packed* BuildUint8T4Packed(core::intrinsic::MatchState& state,
                                                     const core::type::Type*) {
    return state.types.Get<type::Uint8T4Packed>();
}

}  // namespace tint::hlsl::intrinsic

#endif  // SRC_TINT_LANG_HLSL_INTRINSIC_TYPE_MATCHERS_H_
