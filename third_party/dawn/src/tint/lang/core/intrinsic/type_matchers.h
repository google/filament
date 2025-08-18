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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_TYPE_MATCHERS_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_TYPE_MATCHERS_H_

#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/abstract_numeric.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/string.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"

//! @cond Doxygen_Suppress

namespace tint::core::intrinsic {

inline bool MatchBool(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::Bool>();
}

inline const type::Bool* BuildBool(intrinsic::MatchState& state, const type::Type*) {
    return state.types.bool_();
}

inline const type::F16* BuildF16(intrinsic::MatchState& state, const type::Type*) {
    return state.types.f16();
}

inline bool MatchF16(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::F16, type::AbstractNumeric>();
}

inline const type::F32* BuildF32(intrinsic::MatchState& state, const type::Type*) {
    return state.types.f32();
}

inline bool MatchF32(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::F32, type::AbstractNumeric>();
}

inline const type::I32* BuildI32(intrinsic::MatchState& state, const type::Type*) {
    return state.types.i32();
}

inline bool MatchI32(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::I32, type::AbstractInt>();
}

inline const type::I8* BuildI8(intrinsic::MatchState& state, const type::Type*) {
    return state.types.i8();
}

inline bool MatchI8(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::I8, type::AbstractInt>();
}

inline const type::U32* BuildU32(intrinsic::MatchState& state, const type::Type*) {
    return state.types.u32();
}

inline bool MatchU32(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::U32, type::AbstractInt>();
}

inline const type::U64* BuildU64(intrinsic::MatchState& state, const type::Type*) {
    return state.types.u64();
}

inline bool MatchU64(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::U64, type::AbstractInt>();
}

inline const type::U8* BuildU8(intrinsic::MatchState& state, const type::Type*) {
    return state.types.u8();
}

inline bool MatchU8(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::U8, type::AbstractInt>();
}

inline bool MatchVec(intrinsic::MatchState&,
                     const type::Type* ty,
                     intrinsic::Number& N,
                     const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        N = intrinsic::Number::any;
        T = ty;
        return true;
    }

    if (auto* v = ty->As<type::Vector>()) {
        N = v->Width();
        T = v->Type();
        return true;
    }
    return false;
}

template <uint32_t N>
inline bool MatchVec(intrinsic::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* v = ty->As<type::Vector>()) {
        if (v->Width() == N) {
            T = v->Type();
            return true;
        }
    }
    return false;
}

inline const type::Vector* BuildVec(intrinsic::MatchState& state,
                                    const type::Type*,
                                    intrinsic::Number N,
                                    const type::Type* el) {
    return state.types.vec(el, N.Value());
}

template <uint32_t N>
inline const type::Vector* BuildVec(intrinsic::MatchState& state,
                                    const type::Type*,
                                    const type::Type* el) {
    return state.types.vec(el, N);
}

constexpr auto MatchVec2 = MatchVec<2>;
constexpr auto MatchVec3 = MatchVec<3>;
constexpr auto MatchVec4 = MatchVec<4>;

constexpr auto BuildVec2 = BuildVec<2>;
constexpr auto BuildVec3 = BuildVec<3>;
constexpr auto BuildVec4 = BuildVec<4>;

inline bool MatchPackedVec3(intrinsic::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* v = ty->As<type::Vector>()) {
        if (v->Packed()) {
            T = v->Type();
            return true;
        }
    }
    return false;
}

inline const type::Vector* BuildPackedVec3(intrinsic::MatchState& state,
                                           const type::Type*,
                                           const type::Type* el) {
    return state.types.packed_vec(el, 3u);
}

inline bool MatchMat(intrinsic::MatchState&,
                     const type::Type* ty,
                     intrinsic::Number& M,
                     intrinsic::Number& N,
                     const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        M = intrinsic::Number::any;
        N = intrinsic::Number::any;
        T = ty;
        return true;
    }
    if (auto* m = ty->As<type::Matrix>()) {
        M = m->Columns();
        N = m->ColumnType()->Width();
        T = m->Type();
        return true;
    }
    return false;
}

template <uint32_t C, uint32_t R>
inline bool MatchMat(intrinsic::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* m = ty->As<type::Matrix>()) {
        if (m->Columns() == C && m->Rows() == R) {
            T = m->Type();
            return true;
        }
    }
    return false;
}

inline const type::Matrix* BuildMat(intrinsic::MatchState& state,
                                    const type::Type*,
                                    intrinsic::Number C,
                                    intrinsic::Number R,
                                    const type::Type* T) {
    auto* column_type = state.types.vec(T, R.Value());
    return state.types.mat(column_type, C.Value());
}

template <uint32_t C, uint32_t R>
inline const type::Matrix* BuildMat(intrinsic::MatchState& state,
                                    const type::Type*,
                                    const type::Type* T) {
    auto* column_type = state.types.vec(T, R);
    return state.types.mat(column_type, C);
}

constexpr auto BuildMat2X2 = BuildMat<2, 2>;
constexpr auto BuildMat2X3 = BuildMat<2, 3>;
constexpr auto BuildMat2X4 = BuildMat<2, 4>;
constexpr auto BuildMat3X2 = BuildMat<3, 2>;
constexpr auto BuildMat3X3 = BuildMat<3, 3>;
constexpr auto BuildMat3X4 = BuildMat<3, 4>;
constexpr auto BuildMat4X2 = BuildMat<4, 2>;
constexpr auto BuildMat4X3 = BuildMat<4, 3>;
constexpr auto BuildMat4X4 = BuildMat<4, 4>;

constexpr auto MatchMat2X2 = MatchMat<2, 2>;
constexpr auto MatchMat2X3 = MatchMat<2, 3>;
constexpr auto MatchMat2X4 = MatchMat<2, 4>;
constexpr auto MatchMat3X2 = MatchMat<3, 2>;
constexpr auto MatchMat3X3 = MatchMat<3, 3>;
constexpr auto MatchMat3X4 = MatchMat<3, 4>;
constexpr auto MatchMat4X2 = MatchMat<4, 2>;
constexpr auto MatchMat4X3 = MatchMat<4, 3>;
constexpr auto MatchMat4X4 = MatchMat<4, 4>;

inline bool MatchSubgroupMatrix(intrinsic::MatchState&,
                                const type::Type* ty,
                                intrinsic::Number& S,
                                const type::Type*& T,
                                intrinsic::Number& A,
                                intrinsic::Number& B) {
    if (ty->Is<intrinsic::Any>()) {
        A = intrinsic::Number::any;
        B = intrinsic::Number::any;
        S = intrinsic::Number::any;
        T = ty;
        return true;
    }
    if (auto* sm = ty->As<type::SubgroupMatrix>()) {
        A = sm->Columns();
        B = sm->Rows();
        S = intrinsic::Number(static_cast<uint32_t>(sm->Kind()));
        T = sm->Type();
        return true;
    }
    return false;
}

inline const type::SubgroupMatrix* BuildSubgroupMatrix(intrinsic::MatchState& state,
                                                       const type::Type*,
                                                       intrinsic::Number S,
                                                       const type::Type* T,
                                                       intrinsic::Number A,
                                                       intrinsic::Number B) {
    return state.types.subgroup_matrix(static_cast<core::SubgroupMatrixKind>(S.Value()), T,
                                       A.Value(), B.Value());
}

inline bool MatchArray(intrinsic::MatchState&,
                       const type::Type* ty,
                       const type::Type*& T,
                       intrinsic::Number& C) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        C = intrinsic::Number::any;
        return true;
    }

    if (auto* a = ty->As<type::Array>()) {
        if (auto count = a->Count()->As<type::ConstantArrayCount>()) {
            T = a->ElemType();
            C = intrinsic::Number(count->value);
            return true;
        }
    }
    return false;
}

inline const type::Array* BuildArray(intrinsic::MatchState& state,
                                     const type::Type*,
                                     const type::Type* el,
                                     intrinsic::Number C) {
    return state.types.array(el, C.Value());
}

inline bool MatchRuntimeArray(intrinsic::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<type::Array>()) {
        if (a->Count()->Is<type::RuntimeArrayCount>()) {
            T = a->ElemType();
            return true;
        }
    }
    return false;
}

inline const type::Array* BuildRuntimeArray(intrinsic::MatchState& state,
                                            const type::Type*,
                                            const type::Type* el) {
    return state.types.runtime_array(el);
}

inline const type::BindingArray* BuildBindingArray(intrinsic::MatchState& state,
                                                   const type::Type*,
                                                   const type::Type* el,
                                                   intrinsic::Number N) {
    return state.types.binding_array(el, N.Value());
}

inline bool MatchBindingArray(intrinsic::MatchState&,
                              const type::Type* ty,
                              const type::Type*& T,
                              intrinsic::Number& N) {
    if (ty->Is<intrinsic::Any>()) {
        N = intrinsic::Number::any;
        T = ty;
        return true;
    }

    if (auto* a = ty->As<type::BindingArray>()) {
        if (auto count = a->Count()->As<type::ConstantArrayCount>()) {
            N = intrinsic::Number(count->value);
            T = a->ElemType();
            return true;
        }
    }
    return false;
}

inline bool MatchPtr(intrinsic::MatchState&,
                     const type::Type* ty,
                     intrinsic::Number& S,
                     const type::Type*& T,
                     intrinsic::Number& A) {
    if (ty->Is<intrinsic::Any>()) {
        S = intrinsic::Number::any;
        T = ty;
        A = intrinsic::Number::any;
        return true;
    }

    if (auto* p = ty->As<type::Pointer>()) {
        S = intrinsic::Number(static_cast<uint32_t>(p->AddressSpace()));
        T = p->StoreType();
        A = intrinsic::Number(static_cast<uint32_t>(p->Access()));
        return true;
    }
    return false;
}

inline const type::Pointer* BuildPtr(intrinsic::MatchState& state,
                                     const type::Type*,
                                     intrinsic::Number S,
                                     const type::Type* T,
                                     intrinsic::Number& A) {
    return state.types.ptr(static_cast<core::AddressSpace>(S.Value()), T,
                           static_cast<core::Access>(A.Value()));
}

inline bool MatchRef(intrinsic::MatchState&,
                     const type::Type* ty,
                     intrinsic::Number& S,
                     const type::Type*& T,
                     intrinsic::Number& A) {
    if (ty->Is<intrinsic::Any>()) {
        S = intrinsic::Number::any;
        T = ty;
        A = intrinsic::Number::any;
        return true;
    }

    if (auto* p = ty->As<type::Reference>()) {
        S = intrinsic::Number(static_cast<uint32_t>(p->AddressSpace()));
        T = p->StoreType();
        A = intrinsic::Number(static_cast<uint32_t>(p->Access()));
        return true;
    }
    return false;
}

inline const type::Reference* BuildRef(intrinsic::MatchState& state,
                                       const type::Type*,
                                       intrinsic::Number S,
                                       const type::Type* T,
                                       intrinsic::Number& A) {
    return state.types.ref(static_cast<core::AddressSpace>(S.Value()), T,
                           static_cast<core::Access>(A.Value()));
}

inline bool MatchAtomic(intrinsic::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<type::Atomic>()) {
        T = a->Type();
        return true;
    }
    return false;
}

inline const type::Atomic* BuildAtomic(intrinsic::MatchState& state,
                                       const type::Type*,
                                       const type::Type* T) {
    return state.types.atomic(T);
}

inline bool MatchSampler(intrinsic::MatchState&, const type::Type* ty) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is([](const type::Sampler* s) { return s->Kind() == type::SamplerKind::kSampler; });
}

inline const type::Sampler* BuildSampler(intrinsic::MatchState& state, const type::Type*) {
    return state.types.sampler();
}

inline bool MatchSamplerComparison(intrinsic::MatchState&, const type::Type* ty) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is(
        [](const type::Sampler* s) { return s->Kind() == type::SamplerKind::kComparisonSampler; });
}

inline const type::Sampler* BuildSamplerComparison(intrinsic::MatchState& state,
                                                   const type::Type*) {
    return state.types.comparison_sampler();
}

inline bool MatchTexture(intrinsic::MatchState&,
                         const type::Type* ty,
                         type::TextureDimension dim,
                         const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<type::SampledTexture>()) {
        if (v->Dim() == dim) {
            T = v->Type();
            return true;
        }
    }
    return false;
}

#define JOIN(a, b) a##b

#define DECLARE_SAMPLED_TEXTURE(suffix, dim)                                                    \
    inline bool JOIN(MatchTexture, suffix)(intrinsic::MatchState & state, const type::Type* ty, \
                                           const type::Type*& T) {                              \
        return MatchTexture(state, ty, dim, T);                                                 \
    }                                                                                           \
    inline const type::SampledTexture* JOIN(BuildTexture, suffix)(                              \
        intrinsic::MatchState & state, const type::Type*, const type::Type* T) {                \
        return state.types.sampled_texture(dim, T);                                             \
    }

DECLARE_SAMPLED_TEXTURE(1D, type::TextureDimension::k1d)
DECLARE_SAMPLED_TEXTURE(2D, type::TextureDimension::k2d)
DECLARE_SAMPLED_TEXTURE(2DArray, type::TextureDimension::k2dArray)
DECLARE_SAMPLED_TEXTURE(3D, type::TextureDimension::k3d)
DECLARE_SAMPLED_TEXTURE(Cube, type::TextureDimension::kCube)
DECLARE_SAMPLED_TEXTURE(CubeArray, type::TextureDimension::kCubeArray)
#undef DECLARE_SAMPLED_TEXTURE

inline bool MatchTextureMultisampled(intrinsic::MatchState&,
                                     const type::Type* ty,
                                     type::TextureDimension dim,
                                     const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<type::MultisampledTexture>()) {
        if (v->Dim() == dim) {
            T = v->Type();
            return true;
        }
    }
    return false;
}

#define DECLARE_MULTISAMPLED_TEXTURE(suffix, dim)                                    \
    inline bool JOIN(MatchTextureMultisampled, suffix)(                              \
        intrinsic::MatchState & state, const type::Type* ty, const type::Type*& T) { \
        return MatchTextureMultisampled(state, ty, dim, T);                          \
    }                                                                                \
    inline const type::MultisampledTexture* JOIN(BuildTextureMultisampled, suffix)(  \
        intrinsic::MatchState & state, const type::Type*, const type::Type* T) {     \
        return state.types.multisampled_texture(dim, T);                             \
    }

DECLARE_MULTISAMPLED_TEXTURE(2D, type::TextureDimension::k2d)
#undef DECLARE_MULTISAMPLED_TEXTURE

inline bool MatchTextureDepth(intrinsic::MatchState&,
                              const type::Type* ty,
                              type::TextureDimension dim) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is([&](const type::DepthTexture* t) { return t->Dim() == dim; });
}

#define DECLARE_DEPTH_TEXTURE(suffix, dim)                                     \
    inline bool JOIN(MatchTextureDepth, suffix)(intrinsic::MatchState & state, \
                                                const type::Type* ty) {        \
        return MatchTextureDepth(state, ty, dim);                              \
    }                                                                          \
    inline const type::DepthTexture* JOIN(BuildTextureDepth, suffix)(          \
        intrinsic::MatchState & state, const type::Type*) {                    \
        return state.types.depth_texture(dim);                                 \
    }

DECLARE_DEPTH_TEXTURE(2D, type::TextureDimension::k2d)
DECLARE_DEPTH_TEXTURE(2DArray, type::TextureDimension::k2dArray)
DECLARE_DEPTH_TEXTURE(Cube, type::TextureDimension::kCube)
DECLARE_DEPTH_TEXTURE(CubeArray, type::TextureDimension::kCubeArray)
#undef DECLARE_DEPTH_TEXTURE

inline bool MatchTextureDepthMultisampled2D(intrinsic::MatchState&, const type::Type* ty) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is([&](const type::DepthMultisampledTexture* t) {
        return t->Dim() == type::TextureDimension::k2d;
    });
}

inline const type::DepthMultisampledTexture* BuildTextureDepthMultisampled2D(
    intrinsic::MatchState& state,
    const type::Type*) {
    return state.types.depth_multisampled_texture(type::TextureDimension::k2d);
}

inline bool MatchTextureStorage(intrinsic::MatchState&,
                                const type::Type* ty,
                                type::TextureDimension dim,
                                intrinsic::Number& F,
                                intrinsic::Number& A) {
    if (ty->Is<intrinsic::Any>()) {
        F = intrinsic::Number::any;
        A = intrinsic::Number::any;
        return true;
    }
    if (auto* v = ty->As<type::StorageTexture>()) {
        if (v->Dim() == dim) {
            F = intrinsic::Number(static_cast<uint32_t>(v->TexelFormat()));
            A = intrinsic::Number(static_cast<uint32_t>(v->Access()));
            return true;
        }
    }
    return false;
}

#define DECLARE_STORAGE_TEXTURE(suffix, dim)                                                  \
    inline bool JOIN(MatchTextureStorage, suffix)(intrinsic::MatchState & state,              \
                                                  const type::Type* ty, intrinsic::Number& F, \
                                                  intrinsic::Number& A) {                     \
        return MatchTextureStorage(state, ty, dim, F, A);                                     \
    }                                                                                         \
    inline const type::StorageTexture* JOIN(BuildTextureStorage, suffix)(                     \
        intrinsic::MatchState & state, const type::Type*, intrinsic::Number F,                \
        intrinsic::Number A) {                                                                \
        auto format = static_cast<TexelFormat>(F.Value());                                    \
        auto access = static_cast<Access>(A.Value());                                         \
        return state.types.storage_texture(dim, format, access);                              \
    }

DECLARE_STORAGE_TEXTURE(1D, type::TextureDimension::k1d)
DECLARE_STORAGE_TEXTURE(2D, type::TextureDimension::k2d)
DECLARE_STORAGE_TEXTURE(2DArray, type::TextureDimension::k2dArray)
DECLARE_STORAGE_TEXTURE(3D, type::TextureDimension::k3d)
#undef DECLARE_STORAGE_TEXTURE

inline bool MatchTextureExternal(intrinsic::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, type::ExternalTexture>();
}

inline const type::ExternalTexture* BuildTextureExternal(intrinsic::MatchState& state,
                                                         const type::Type*) {
    return state.types.external_texture();
}

inline bool MatchTexelBuffer(intrinsic::MatchState&,
                             const type::Type* ty,
                             intrinsic::Number& F,
                             intrinsic::Number& A) {
    if (ty->Is<intrinsic::Any>()) {
        F = intrinsic::Number::any;
        A = intrinsic::Number::any;
        return true;
    }
    if (auto* v = ty->As<type::TexelBuffer>()) {
        F = intrinsic::Number(static_cast<uint32_t>(v->TexelFormat()));
        A = intrinsic::Number(static_cast<uint32_t>(v->Access()));
        return true;
    }
    return false;
}

inline const type::TexelBuffer* BuildTexelBuffer(intrinsic::MatchState& state,
                                                 const type::Type*,
                                                 intrinsic::Number F,
                                                 intrinsic::Number A) {
    auto format = static_cast<TexelFormat>(F.Value());
    auto access = static_cast<Access>(A.Value());
    return state.types.texel_buffer(format, access);
}

inline bool MatchInputAttachment(intrinsic::MatchState&,
                                 const type::Type* ty,
                                 const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<type::InputAttachment>()) {
        T = v->Type();
        return true;
    }
    return false;
}

inline const type::InputAttachment* BuildInputAttachment(intrinsic::MatchState& state,
                                                         const type::Type*,
                                                         const type::Type* T) {
    return state.types.input_attachment(T);
}

// Builtin types starting with a _ prefix cannot be declared in WGSL, so they
// can only be used as return types. Because of this, they must only match Any,
// which is used as the return type matcher.
inline bool MatchModfResult(intrinsic::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    T = ty;
    return true;
}
inline bool MatchModfResultVec(intrinsic::MatchState&,
                               const type::Type* ty,
                               intrinsic::Number& N,
                               const type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    N = intrinsic::Number::any;
    T = ty;
    return true;
}
inline bool MatchFrexpResult(intrinsic::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    T = ty;
    return true;
}
inline bool MatchFrexpResultVec(intrinsic::MatchState&,
                                const type::Type* ty,
                                intrinsic::Number& N,
                                const type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    N = intrinsic::Number::any;
    T = ty;
    return true;
}

inline bool MatchAtomicCompareExchangeResult(intrinsic::MatchState&,
                                             const type::Type* ty,
                                             const type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    return false;
}

inline const type::Struct* BuildModfResult(intrinsic::MatchState& state,
                                           const type::Type*,
                                           const type::Type* el) {
    return type::CreateModfResult(state.types, state.symbols, el);
}

inline const type::Struct* BuildModfResultVec(intrinsic::MatchState& state,
                                              const type::Type*,
                                              intrinsic::Number& n,
                                              const type::Type* el) {
    auto* vec = state.types.vec(el, n.Value());
    return type::CreateModfResult(state.types, state.symbols, vec);
}

inline const type::Struct* BuildFrexpResult(intrinsic::MatchState& state,
                                            const type::Type*,
                                            const type::Type* el) {
    return type::CreateFrexpResult(state.types, state.symbols, el);
}

inline const type::Struct* BuildFrexpResultVec(intrinsic::MatchState& state,
                                               const type::Type*,
                                               intrinsic::Number& n,
                                               const type::Type* el) {
    auto* vec = state.types.vec(el, n.Value());
    return type::CreateFrexpResult(state.types, state.symbols, vec);
}

inline const type::Struct* BuildAtomicCompareExchangeResult(intrinsic::MatchState& state,
                                                            const type::Type*,
                                                            const type::Type* ty) {
    return type::CreateAtomicCompareExchangeResult(state.types, state.symbols, ty);
}

inline bool MatchString(core::intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->Is<type::String>();
}

inline const core::type::Type* BuildString(core::intrinsic::MatchState& state,
                                           const core::type::Type*) {
    return state.types.String();
}

}  // namespace tint::core::intrinsic

//! @endcond

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_TYPE_MATCHERS_H_
