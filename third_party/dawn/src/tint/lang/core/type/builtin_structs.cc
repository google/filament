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

#include "src/tint/lang/core/type/builtin_structs.h"

#include <algorithm>
#include <string>
#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/symbol/symbol_table.h"
#include "src/tint/utils/text/string.h"

namespace tint::core::type {

/// An array of `modf()` return type names for an argument of `vecN<f32>`.
constexpr std::array kModfVecF32Names{
    core::BuiltinType::kModfResultVec2F32,  // return type of modf(vec2<f32>)
    core::BuiltinType::kModfResultVec3F32,  // return type of modf(vec3<f32>)
    core::BuiltinType::kModfResultVec4F32,  // return type of modf(vec4<f32>)
};

/// An array of `modf()` return type names for an argument of `vecN<f16>`.
constexpr std::array kModfVecF16Names{
    core::BuiltinType::kModfResultVec2F16,  // return type of modf(vec2<f16>)
    core::BuiltinType::kModfResultVec3F16,  // return type of modf(vec3<f16>)
    core::BuiltinType::kModfResultVec4F16,  // return type of modf(vec4<f16>)
};

/// An array of `modf()` return type names for an argument of `vecN<abstract-float>`.
constexpr std::array kModfVecAbstractNames{
    core::BuiltinType::kModfResultVec2Abstract,  // return type of modf(vec2<abstract-float>)
    core::BuiltinType::kModfResultVec3Abstract,  // return type of modf(vec3<abstract-float>)
    core::BuiltinType::kModfResultVec4Abstract,  // return type of modf(vec4<abstract-float>)
};

Struct* CreateModfResult(Manager& types, SymbolTable& symbols, const Type* ty) {
    auto build = [&](core::BuiltinType name, const Type* t) {
        auto symbol = symbols.Register(tint::ToString(name));
        if (auto* existing = types.Find<type::Struct>(symbol, /* is_wgsl_internal */ true)) {
            return existing;
        }
        return types.WgslInternalStruct(
            symbol, {{symbols.Register("fract"), t}, {symbols.Register("whole"), t}});
    };
    return Switch(
        ty,  //
        [&](const F32*) { return build(core::BuiltinType::kModfResultF32, ty); },
        [&](const F16*) { return build(core::BuiltinType::kModfResultF16, ty); },
        [&](const AbstractFloat*) {
            auto* abstract = build(core::BuiltinType::kModfResultAbstract, ty);
            abstract->SetConcreteTypes(tint::Vector{
                build(core::BuiltinType::kModfResultF32, types.f32()),
                build(core::BuiltinType::kModfResultF16, types.f16()),
            });
            return abstract;
        },
        [&](const Vector* vec) {
            auto width = vec->Width();
            return Switch(
                vec->Type(),  //
                [&](const F32*) { return build(kModfVecF32Names[width - 2], vec); },
                [&](const F16*) { return build(kModfVecF16Names[width - 2], vec); },
                [&](const AbstractFloat*) {
                    auto* abstract = build(kModfVecAbstractNames[width - 2], vec);
                    abstract->SetConcreteTypes(tint::Vector{
                        build(kModfVecF32Names[width - 2], types.vec(types.f32(), width)),
                        build(kModfVecF16Names[width - 2], types.vec(types.f16(), width)),
                    });
                    return abstract;
                },  //
                TINT_ICE_ON_NO_MATCH);
        },  //
        TINT_ICE_ON_NO_MATCH);
}

/// An array of `frexp()` return type names for an argument of `vecN<f32>`.
constexpr std::array kFrexpVecF32Names{
    core::BuiltinType::kFrexpResultVec2F32,  // return type of frexp(vec2<f32>)
    core::BuiltinType::kFrexpResultVec3F32,  // return type of frexp(vec3<f32>)
    core::BuiltinType::kFrexpResultVec4F32,  // return type of frexp(vec4<f32>)
};

/// An array of `frexp()` return type names for an argument of `vecN<f16>`.
constexpr std::array kFrexpVecF16Names{
    core::BuiltinType::kFrexpResultVec2F16,  // return type of frexp(vec2<f16>)
    core::BuiltinType::kFrexpResultVec3F16,  // return type of frexp(vec3<f16>)
    core::BuiltinType::kFrexpResultVec4F16,  // return type of frexp(vec4<f16>)
};

/// An array of `frexp()` return type names for an argument of `vecN<abstract-float>`.
constexpr std::array kFrexpVecAbstractNames{
    core::BuiltinType::kFrexpResultVec2Abstract,  // return type of frexp(vec2<abstract-float>)
    core::BuiltinType::kFrexpResultVec3Abstract,  // return type of frexp(vec3<abstract-float>)
    core::BuiltinType::kFrexpResultVec4Abstract,  // return type of frexp(vec4<abstract-float>)
};

Struct* CreateFrexpResult(Manager& types, SymbolTable& symbols, const Type* ty) {
    auto build = [&](core::BuiltinType name, const Type* fract_ty, const Type* exp_ty) {
        auto symbol = symbols.Register(tint::ToString(name));
        if (auto* existing = types.Find<type::Struct>(symbol, /* is_wgsl_internal */ true)) {
            return existing;
        }
        return types.WgslInternalStruct(
            symbol, {{symbols.Register("fract"), fract_ty}, {symbols.Register("exp"), exp_ty}});
    };
    return Switch(
        ty,  //
        [&](const F32*) { return build(core::BuiltinType::kFrexpResultF32, ty, types.i32()); },
        [&](const F16*) { return build(core::BuiltinType::kFrexpResultF16, ty, types.i32()); },
        [&](const AbstractFloat*) {
            auto* abstract = build(core::BuiltinType::kFrexpResultAbstract, ty, types.AInt());
            abstract->SetConcreteTypes(tint::Vector{
                build(core::BuiltinType::kFrexpResultF32, types.f32(), types.i32()),
                build(core::BuiltinType::kFrexpResultF16, types.f16(), types.i32()),
            });
            return abstract;
        },
        [&](const Vector* vec) {
            auto width = vec->Width();
            return Switch(
                vec->Type(),  //
                [&](const F32*) {
                    return build(kFrexpVecF32Names[width - 2], ty, types.vec(types.i32(), width));
                },
                [&](const F16*) {
                    return build(kFrexpVecF16Names[width - 2], ty, types.vec(types.i32(), width));
                },
                [&](const AbstractFloat*) {
                    auto* vec_f32 = types.vec(types.f32(), width);
                    auto* vec_f16 = types.vec(types.f16(), width);
                    auto* vec_i32 = types.vec(types.i32(), width);
                    auto* vec_ai = types.vec(types.AInt(), width);
                    auto* abstract = build(kFrexpVecAbstractNames[width - 2], ty, vec_ai);
                    abstract->SetConcreteTypes(tint::Vector{
                        build(kFrexpVecF32Names[width - 2], vec_f32, vec_i32),
                        build(kFrexpVecF16Names[width - 2], vec_f16, vec_i32),
                    });
                    return abstract;
                },  //
                TINT_ICE_ON_NO_MATCH);
        },  //
        TINT_ICE_ON_NO_MATCH);
}

Struct* CreateAtomicCompareExchangeResult(Manager& types, SymbolTable& symbols, const Type* ty) {
    auto build = [&](core::BuiltinType name) {
        auto symbol = symbols.Register(tint::ToString(name));
        if (auto* existing = types.Find<type::Struct>(symbol, /* is_wgsl_internal */ true)) {
            return existing;
        }
        return types.WgslInternalStruct(symbol, {
                                                    {symbols.Register("old_value"), ty},
                                                    {symbols.Register("exchanged"), types.bool_()},
                                                });
    };
    return Switch(
        ty,  //
        [&](const I32*) { return build(core::BuiltinType::kAtomicCompareExchangeResultI32); },
        [&](const U32*) { return build(core::BuiltinType::kAtomicCompareExchangeResultU32); },
        TINT_ICE_ON_NO_MATCH);
}

}  // namespace tint::core::type
