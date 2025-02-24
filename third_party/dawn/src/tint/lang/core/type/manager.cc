// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/manager.h"

#include <algorithm>

#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/function.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/invalid.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/macros/compiler.h"

namespace tint::core::type {

Manager::Manager() = default;

Manager::Manager(Manager&&) = default;

Manager& Manager::operator=(Manager&& rhs) = default;

Manager::~Manager() = default;

const core::type::Invalid* Manager::invalid() {
    return Get<core::type::Invalid>();
}

const core::type::Function* Manager::function() {
    return Get<core::type::Function>();
}

const core::type::Void* Manager::void_() {
    return Get<core::type::Void>();
}

const core::type::Bool* Manager::bool_() {
    return Get<core::type::Bool>();
}

const core::type::I8* Manager::i8() {
    return Get<core::type::I8>();
}

const core::type::I32* Manager::i32() {
    return Get<core::type::I32>();
}

const core::type::U8* Manager::u8() {
    return Get<core::type::U8>();
}

const core::type::U32* Manager::u32() {
    return Get<core::type::U32>();
}

const core::type::U64* Manager::u64() {
    return Get<core::type::U64>();
}

const core::type::F32* Manager::f32() {
    return Get<core::type::F32>();
}

const core::type::F16* Manager::f16() {
    return Get<core::type::F16>();
}

const core::type::AbstractFloat* Manager::AFloat() {
    return Get<core::type::AbstractFloat>();
}

const core::type::AbstractInt* Manager::AInt() {
    return Get<core::type::AbstractInt>();
}

const core::type::Atomic* Manager::atomic(const core::type::Type* inner) {
    return Get<core::type::Atomic>(inner);
}

const core::type::Vector* Manager::packed_vec(const core::type::Type* inner, uint32_t size) {
    return Get<core::type::Vector>(inner, size, true);
}

const core::type::Type* Manager::MatchWidth(const core::type::Type* el_ty,
                                            const core::type::Type* match) {
    if (auto* m = match->As<core::type::Vector>()) {
        return vec(el_ty, m->Width());
    }
    return el_ty;
}

const core::type::Type* Manager::MatchWidth(const core::type::Type* el_ty, size_t size) {
    if (size > 1) {
        return vec(el_ty, static_cast<uint32_t>(size));
    }
    return el_ty;
}

const core::type::Vector* Manager::vec(const core::type::Type* inner, uint32_t size) {
    return Get<core::type::Vector>(inner, size);
}

const core::type::Vector* Manager::vec2(const core::type::Type* inner) {
    return vec(inner, 2);
}

const core::type::Vector* Manager::vec3(const core::type::Type* inner) {
    return vec(inner, 3);
}

const core::type::Vector* Manager::vec4(const core::type::Type* inner) {
    return vec(inner, 4);
}

const core::type::StorageTexture* Manager::storage_texture(TextureDimension dim,
                                                           core::TexelFormat format,
                                                           core::Access access) {
    const auto* subtype = StorageTexture::SubtypeFor(format, *this);
    return Get<core::type::StorageTexture>(dim, format, access, subtype);
}

const core::type::Matrix* Manager::mat(const core::type::Type* inner,
                                       uint32_t cols,
                                       uint32_t rows) {
    return Get<core::type::Matrix>(vec(inner, rows), cols);
}

const core::type::Matrix* Manager::mat(const core::type::Vector* column_type, uint32_t cols) {
    return Get<core::type::Matrix>(column_type, cols);
}

const core::type::Matrix* Manager::mat2x2(const core::type::Type* inner) {
    return mat(inner, 2, 2);
}

const core::type::Matrix* Manager::mat2x3(const core::type::Type* inner) {
    return mat(inner, 2, 3);
}

const core::type::Matrix* Manager::mat2x4(const core::type::Type* inner) {
    return mat(inner, 2, 4);
}

const core::type::Matrix* Manager::mat3x2(const core::type::Type* inner) {
    return mat(inner, 3, 2);
}

const core::type::Matrix* Manager::mat3x3(const core::type::Type* inner) {
    return mat(inner, 3, 3);
}

const core::type::Matrix* Manager::mat3x4(const core::type::Type* inner) {
    return mat(inner, 3, 4);
}

const core::type::Matrix* Manager::mat4x2(const core::type::Type* inner) {
    return mat(inner, 4, 2);
}

const core::type::Matrix* Manager::mat4x3(const core::type::Type* inner) {
    return mat(inner, 4, 3);
}

const core::type::Matrix* Manager::mat4x4(const core::type::Type* inner) {
    return mat(inner, 4, 4);
}

const core::type::SubgroupMatrix* Manager::subgroup_matrix(SubgroupMatrixKind kind,
                                                           const core::type::Type* inner,
                                                           uint32_t cols,
                                                           uint32_t rows) {
    return Get<core::type::SubgroupMatrix>(kind, inner, cols, rows);
}

const core::type::Array* Manager::array(const core::type::Type* elem_ty,
                                        uint32_t count,
                                        uint32_t stride /* = 0*/) {
    uint32_t implicit_stride = tint::RoundUp(elem_ty->Align(), elem_ty->Size());
    if (stride == 0) {
        stride = implicit_stride;
    }
    TINT_ASSERT(stride >= implicit_stride);

    return Get<core::type::Array>(/* element type */ elem_ty,
                                  /* element count */ Get<ConstantArrayCount>(count),
                                  /* array alignment */ elem_ty->Align(),
                                  /* array size */ count * stride,
                                  /* element stride */ stride,
                                  /* implicit stride */ implicit_stride);
}

const core::type::Array* Manager::runtime_array(const core::type::Type* elem_ty,
                                                uint32_t stride /* = 0 */) {
    uint32_t implicit_stride = tint::RoundUp(elem_ty->Align(), elem_ty->Size());
    if (stride == 0) {
        stride = implicit_stride;
    }
    TINT_ASSERT(stride >= implicit_stride);

    return Get<core::type::Array>(
        /* element type */ elem_ty,
        /* element count */ Get<RuntimeArrayCount>(),
        /* array alignment */ elem_ty->Align(),
        /* array size */ stride,
        /* element stride */ stride,
        /* implicit stride */ implicit_stride);
}

const core::type::BindingArray* Manager::binding_array(const core::type::Type* elem_ty,
                                                       uint32_t count) {
    return Get<core::type::BindingArray>(elem_ty, count);
}

const core::type::Pointer* Manager::ptr(core::AddressSpace address_space,
                                        const core::type::Type* subtype,
                                        core::Access access /* = core::Access::kUndefined */) {
    return Get<core::type::Pointer>(
        address_space, subtype,
        access == core::Access::kUndefined ? DefaultAccessFor(address_space) : access);
}

const core::type::Reference* Manager::ref(core::AddressSpace address_space,
                                          const core::type::Type* subtype,
                                          core::Access access /* = core::Access::kReadWrite */) {
    return Get<core::type::Reference>(address_space, subtype, access);
}

core::type::Struct* Manager::Struct(Symbol name, VectorRef<const StructMember*> members) {
    if (auto* existing = Find<type::Struct>(name); DAWN_UNLIKELY(existing)) {
        TINT_ICE() << "attempting to construct two structs named " << name.NameView();
    }

    uint32_t max_align = 0u;
    for (const auto& m : members) {
        max_align = std::max(max_align, m->Align());
    }
    uint32_t size = members.Back()->Offset() + members.Back()->Size();
    return Get<core::type::Struct>(name, std::move(members), max_align,
                                   tint::RoundUp(max_align, size), size);
}

core::type::Struct* Manager::Struct(Symbol name, VectorRef<StructMemberDesc> md) {
    if (auto* existing = Find<type::Struct>(name); DAWN_UNLIKELY(existing)) {
        TINT_ICE() << "attempting to construct two structs named " << name.NameView();
    }

    tint::Vector<const StructMember*, 4> members;
    uint32_t current_size = 0u;
    uint32_t max_align = 0u;
    for (const auto& m : md) {
        uint32_t index = static_cast<uint32_t>(members.Length());
        uint32_t align = std::max<uint32_t>(m.type->Align(), 1u);
        uint32_t offset = tint::RoundUp(align, current_size);
        members.Push(Get<core::type::StructMember>(m.name, m.type, index, offset, align,
                                                   m.type->Size(), std::move(m.attributes)));
        current_size = offset + m.type->Size();
        max_align = std::max(max_align, align);
    }
    return Get<core::type::Struct>(name, std::move(members), max_align,
                                   tint::RoundUp(max_align, current_size), current_size);
}

}  // namespace tint::core::type
