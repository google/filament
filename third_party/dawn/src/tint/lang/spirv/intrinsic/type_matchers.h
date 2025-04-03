// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_INTRINSIC_TYPE_MATCHERS_H_
#define SRC_TINT_LANG_SPIRV_INTRINSIC_TYPE_MATCHERS_H_

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/spirv/type/sampled_image.h"

namespace tint::spirv::intrinsic {

inline bool MatchStructWithRuntimeArray(core::intrinsic::MatchState&, const core::type::Type* ty) {
    if (auto* str = ty->As<core::type::Struct>()) {
        if (str->Members().IsEmpty()) {
            return false;
        }
        if (auto* ary = str->Members().Back()->Type()->As<core::type::Array>()) {
            if (ary->Count()->Is<core::type::RuntimeArrayCount>()) {
                return true;
            }
        }
    }
    return false;
}

inline const core::type::Type* BuildStructWithRuntimeArray(core::intrinsic::MatchState&,
                                                           const core::type::Type* ty) {
    return ty;
}

inline bool MatchSampledImage(core::intrinsic::MatchState&,
                              const core::type::Type* ty,
                              const core::type::Type*& T) {
    if (ty->Is<core::intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<spirv::type::SampledImage>()) {
        T = v->Image();
        return true;
    }
    return false;
}

inline const core::type::Type* BuildSampledImage(core::intrinsic::MatchState& state,
                                                 const core::type::Type*,
                                                 const core::type::Type* T) {
    return state.types.Get<type::SampledImage>(T);
}

inline bool MatchImage(core::intrinsic::MatchState&,
                       const core::type::Type* ty,
                       const core::type::Type*& T,
                       core::intrinsic::Number& D,
                       core::intrinsic::Number& E,
                       core::intrinsic::Number& R,
                       core::intrinsic::Number& M,
                       core::intrinsic::Number& S,
                       core::intrinsic::Number& F,
                       core::intrinsic::Number& A) {
    if (ty->Is<core::intrinsic::Any>()) {
        T = ty;
        D = core::intrinsic::Number::any;
        E = core::intrinsic::Number::any;
        R = core::intrinsic::Number::any;
        M = core::intrinsic::Number::any;
        S = core::intrinsic::Number::any;
        F = core::intrinsic::Number::any;
        A = core::intrinsic::Number::any;
        return true;
    }
    if (auto* v = ty->As<spirv::type::Image>()) {
        T = v->GetSampledType();
        D = core::intrinsic::Number(static_cast<uint32_t>(v->GetDim()));
        E = core::intrinsic::Number(static_cast<uint32_t>(v->GetDepth()));
        R = core::intrinsic::Number(static_cast<uint32_t>(v->GetArrayed()));
        M = core::intrinsic::Number(static_cast<uint32_t>(v->GetMultisampled()));
        S = core::intrinsic::Number(static_cast<uint32_t>(v->GetSampled()));
        F = core::intrinsic::Number(static_cast<uint32_t>(v->GetTexelFormat()));
        A = core::intrinsic::Number(static_cast<uint32_t>(v->GetAccess()));
        return true;
    }
    return false;
}

inline const core::type::Type* BuildImage(core::intrinsic::MatchState& state,
                                          const core::type::Type*,
                                          const core::type::Type* T,
                                          core::intrinsic::Number D,
                                          core::intrinsic::Number E,
                                          core::intrinsic::Number R,
                                          core::intrinsic::Number M,
                                          core::intrinsic::Number S,
                                          core::intrinsic::Number F,
                                          core::intrinsic::Number A) {
    auto dim = static_cast<spirv::type::Dim>(D.Value());
    auto depth = static_cast<spirv::type::Depth>(E.Value());
    auto arrayed = static_cast<spirv::type::Arrayed>(R.Value());
    auto ms = static_cast<spirv::type::Multisampled>(M.Value());
    auto sampled = static_cast<spirv::type::Sampled>(S.Value());
    auto fmt = static_cast<core::TexelFormat>(F.Value());
    auto access = static_cast<core::Access>(A.Value());

    return state.types.Get<type::Image>(T, dim, depth, arrayed, ms, sampled, fmt, access);
}

}  // namespace tint::spirv::intrinsic

#endif  // SRC_TINT_LANG_SPIRV_INTRINSIC_TYPE_MATCHERS_H_
