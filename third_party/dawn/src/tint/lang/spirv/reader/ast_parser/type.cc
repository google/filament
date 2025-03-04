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

#include "src/tint/lang/spirv/reader/ast_parser/type.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/unique_allocator.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Type);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Void);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Bool);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::U32);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::F32);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::F16);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::I32);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Pointer);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Reference);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Vector);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Matrix);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Array);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Sampler);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Texture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::DepthTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::DepthMultisampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::MultisampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::SampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::StorageTexture);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Named);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Alias);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::ast_parser::Struct);

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::reader::ast_parser {
namespace {

struct PointerHasher {
    HashCode operator()(const Pointer& t) const { return Hash(t.address_space, t.type, t.access); }
};

struct ReferenceHasher {
    HashCode operator()(const Reference& t) const {
        return Hash(t.address_space, t.type, t.access);
    }
};

struct VectorHasher {
    HashCode operator()(const Vector& t) const { return Hash(t.type, t.size); }
};

struct MatrixHasher {
    HashCode operator()(const Matrix& t) const { return Hash(t.type, t.columns, t.rows); }
};

struct ArrayHasher {
    HashCode operator()(const Array& t) const { return Hash(t.type, t.size, t.stride); }
};

struct AliasHasher {
    HashCode operator()(const Alias& t) const { return Hash(t.name); }
};

struct StructHasher {
    HashCode operator()(const Struct& t) const { return Hash(t.name); }
};

struct SamplerHasher {
    HashCode operator()(const Sampler& s) const { return Hash(s.kind); }
};

struct DepthTextureHasher {
    HashCode operator()(const DepthTexture& t) const { return Hash(t.dims); }
};

struct DepthMultisampledTextureHasher {
    HashCode operator()(const DepthMultisampledTexture& t) const { return Hash(t.dims); }
};

struct MultisampledTextureHasher {
    HashCode operator()(const MultisampledTexture& t) const { return Hash(t.dims, t.type); }
};

struct SampledTextureHasher {
    HashCode operator()(const SampledTexture& t) const { return Hash(t.dims, t.type); }
};

struct StorageTextureHasher {
    HashCode operator()(const StorageTexture& t) const { return Hash(t.dims, t.format, t.access); }
};
}  // namespace

// Equality operators
//! @cond Doxygen_Suppress
static bool operator==(const Pointer& a, const Pointer& b) {
    return a.type == b.type && a.address_space == b.address_space && a.access == b.access;
}
static bool operator==(const Reference& a, const Reference& b) {
    return a.type == b.type && a.address_space == b.address_space && a.access == b.access;
}
static bool operator==(const Vector& a, const Vector& b) {
    return a.type == b.type && a.size == b.size;
}
static bool operator==(const Matrix& a, const Matrix& b) {
    return a.type == b.type && a.columns == b.columns && a.rows == b.rows;
}
static bool operator==(const Array& a, const Array& b) {
    return a.type == b.type && a.size == b.size && a.stride == b.stride;
}
static bool operator==(const Named& a, const Named& b) {
    return a.name == b.name;
}
static bool operator==(const Sampler& a, const Sampler& b) {
    return a.kind == b.kind;
}
static bool operator==(const DepthTexture& a, const DepthTexture& b) {
    return a.dims == b.dims;
}
static bool operator==(const DepthMultisampledTexture& a, const DepthMultisampledTexture& b) {
    return a.dims == b.dims;
}
static bool operator==(const MultisampledTexture& a, const MultisampledTexture& b) {
    return a.dims == b.dims && a.type == b.type;
}
static bool operator==(const SampledTexture& a, const SampledTexture& b) {
    return a.dims == b.dims && a.type == b.type;
}
static bool operator==(const StorageTexture& a, const StorageTexture& b) {
    return a.dims == b.dims && a.format == b.format;
}
//! @endcond

ast::Type Void::Build(ProgramBuilder& b) const {
    return b.ty.void_();
}

ast::Type Bool::Build(ProgramBuilder& b) const {
    return b.ty.bool_();
}

ast::Type U32::Build(ProgramBuilder& b) const {
    return b.ty.u32();
}

ast::Type F32::Build(ProgramBuilder& b) const {
    return b.ty.f32();
}

ast::Type F16::Build(ProgramBuilder& b) const {
    return b.ty.f16();
}

ast::Type I32::Build(ProgramBuilder& b) const {
    return b.ty.i32();
}

Type::Type() = default;
Type::Type(const Type&) = default;
Type::~Type() = default;

Texture::~Texture() = default;

Pointer::Pointer(core::AddressSpace s, const Type* t, core::Access a)
    : address_space(s), type(t), access(a) {}
Pointer::Pointer(const Pointer&) = default;

ast::Type Pointer::Build(ProgramBuilder& b) const {
    auto store_type = type->Build(b);
    if (!store_type) {
        // TODO(crbug.com/tint/1838): We should not be constructing pointers with 'void' store
        // types.
        return b.ty("invalid_spirv_ptr_type");
    }
    return b.ty.ptr(address_space, type->Build(b), access);
}

Reference::Reference(core::AddressSpace s, const Type* t, core::Access a)
    : address_space(s), type(t), access(a) {}
Reference::Reference(const Reference&) = default;

ast::Type Reference::Build(ProgramBuilder& b) const {
    return type->Build(b);
}

Vector::Vector(const Type* t, uint32_t s) : type(t), size(s) {}
Vector::Vector(const Vector&) = default;

ast::Type Vector::Build(ProgramBuilder& b) const {
    auto prefix = "vec" + std::to_string(size);
    return Switch(
        type,  //
        [&](const I32*) { return b.ty(prefix + "i"); },
        [&](const U32*) { return b.ty(prefix + "u"); },
        [&](const F32*) { return b.ty(prefix + "f"); },
        [&](const F16*) { return b.ty(prefix + "h"); },
        [&](Default) { return b.ty.vec(type->Build(b), size); });
}

Matrix::Matrix(const Type* t, uint32_t c, uint32_t r) : type(t), columns(c), rows(r) {}
Matrix::Matrix(const Matrix&) = default;

ast::Type Matrix::Build(ProgramBuilder& b) const {
    if (type->IsAnyOf<F32, F16>()) {
        std::ostringstream ss;
        ss << "mat" << columns << "x" << rows;
        if (type->Is<F32>()) {
            ss << "f";
        } else {
            ss << "h";
        }
        return b.ty(ss.str());
    }
    return b.ty.mat(type->Build(b), columns, rows);
}

Array::Array(const Type* t, uint32_t sz, uint32_t st) : type(t), size(sz), stride(st) {}
Array::Array(const Array&) = default;

ast::Type Array::Build(ProgramBuilder& b) const {
    if (size > 0) {
        if (stride > 0) {
            return b.ty.array(type->Build(b), u32(size), tint::Vector{b.Stride(stride)});
        } else {
            return b.ty.array(type->Build(b), u32(size));
        }
    } else {
        if (stride > 0) {
            return b.ty.array(type->Build(b), tint::Vector{b.Stride(stride)});
        } else {
            return b.ty.array(type->Build(b));
        }
    }
}

Sampler::Sampler(core::type::SamplerKind k) : kind(k) {}
Sampler::Sampler(const Sampler&) = default;

ast::Type Sampler::Build(ProgramBuilder& b) const {
    return b.ty.sampler(kind);
}

Texture::Texture(core::type::TextureDimension d) : dims(d) {}
Texture::Texture(const Texture&) = default;

DepthTexture::DepthTexture(core::type::TextureDimension d) : Base(d) {}
DepthTexture::DepthTexture(const DepthTexture&) = default;

ast::Type DepthTexture::Build(ProgramBuilder& b) const {
    return b.ty.depth_texture(dims);
}

DepthMultisampledTexture::DepthMultisampledTexture(core::type::TextureDimension d) : Base(d) {}
DepthMultisampledTexture::DepthMultisampledTexture(const DepthMultisampledTexture&) = default;

ast::Type DepthMultisampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.depth_multisampled_texture(dims);
}

MultisampledTexture::MultisampledTexture(core::type::TextureDimension d, const Type* t)
    : Base(d), type(t) {}
MultisampledTexture::MultisampledTexture(const MultisampledTexture&) = default;

ast::Type MultisampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.multisampled_texture(dims, type->Build(b));
}

SampledTexture::SampledTexture(core::type::TextureDimension d, const Type* t) : Base(d), type(t) {}
SampledTexture::SampledTexture(const SampledTexture&) = default;

ast::Type SampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.sampled_texture(dims, type->Build(b));
}

StorageTexture::StorageTexture(core::type::TextureDimension d, core::TexelFormat f, core::Access a)
    : Base(d), format(f), access(a) {}
StorageTexture::StorageTexture(const StorageTexture&) = default;

ast::Type StorageTexture::Build(ProgramBuilder& b) const {
    return b.ty.storage_texture(dims, format, access);
}

Named::Named(Symbol n) : name(n) {}
Named::Named(const Named&) = default;
Named::~Named() = default;

Alias::Alias(Symbol n, const Type* ty) : Base(n), type(ty) {}
Alias::Alias(const Alias&) = default;

ast::Type Alias::Build(ProgramBuilder& b) const {
    return b.ty(name);
}

Struct::Struct(Symbol n, TypeList m) : Base(n), members(std::move(m)) {}
Struct::Struct(const Struct&) = default;
Struct::~Struct() = default;

ast::Type Struct::Build(ProgramBuilder& b) const {
    return b.ty(name);
}

/// The PIMPL state of the Types object.
struct TypeManager::State {
    /// The allocator of primitive types
    BlockAllocator<Type> allocator_;
    /// The lazily-created Void type
    ast_parser::Void const* void_ = nullptr;
    /// The lazily-created Bool type
    ast_parser::Bool const* bool_ = nullptr;
    /// The lazily-created U32 type
    ast_parser::U32 const* u32_ = nullptr;
    /// The lazily-created F32 type
    ast_parser::F32 const* f32_ = nullptr;
    /// The lazily-created F16 type
    ast_parser::F16 const* f16_ = nullptr;
    /// The lazily-created I32 type
    ast_parser::I32 const* i32_ = nullptr;
    /// Unique Pointer instances
    UniqueAllocator<ast_parser::Pointer, PointerHasher> pointers_;
    /// Unique Reference instances
    UniqueAllocator<ast_parser::Reference, ReferenceHasher> references_;
    /// Unique Vector instances
    UniqueAllocator<ast_parser::Vector, VectorHasher> vectors_;
    /// Unique Matrix instances
    UniqueAllocator<ast_parser::Matrix, MatrixHasher> matrices_;
    /// Unique Array instances
    UniqueAllocator<ast_parser::Array, ArrayHasher> arrays_;
    /// Unique Alias instances
    UniqueAllocator<ast_parser::Alias, AliasHasher> aliases_;
    /// Unique Struct instances
    UniqueAllocator<ast_parser::Struct, StructHasher> structs_;
    /// Unique Sampler instances
    UniqueAllocator<ast_parser::Sampler, SamplerHasher> samplers_;
    /// Unique DepthTexture instances
    UniqueAllocator<ast_parser::DepthTexture, DepthTextureHasher> depth_textures_;
    /// Unique DepthMultisampledTexture instances
    UniqueAllocator<ast_parser::DepthMultisampledTexture, DepthMultisampledTextureHasher>
        depth_multisampled_textures_;
    /// Unique MultisampledTexture instances
    UniqueAllocator<ast_parser::MultisampledTexture, MultisampledTextureHasher>
        multisampled_textures_;
    /// Unique SampledTexture instances
    UniqueAllocator<ast_parser::SampledTexture, SampledTextureHasher> sampled_textures_;
    /// Unique StorageTexture instances
    UniqueAllocator<ast_parser::StorageTexture, StorageTextureHasher> storage_textures_;
};

const Type* Type::UnwrapPtr() const {
    const Type* type = this;
    while (auto* ptr = type->As<Pointer>()) {
        type = ptr->type;
    }
    return type;
}

const Type* Type::UnwrapRef() const {
    const Type* type = this;
    while (auto* ptr = type->As<Reference>()) {
        type = ptr->type;
    }
    return type;
}

const Type* Type::UnwrapAlias() const {
    const Type* type = this;
    while (auto* alias = type->As<Alias>()) {
        type = alias->type;
    }
    return type;
}

const Type* Type::UnwrapAll() const {
    auto* type = this;
    while (true) {
        if (auto* alias = type->As<Alias>()) {
            type = alias->type;
        } else if (auto* ptr = type->As<Pointer>()) {
            type = ptr->type;
        } else {
            break;
        }
    }
    return type;
}

bool Type::IsFloatScalar() const {
    return IsAnyOf<F32, F16>();
}

bool Type::IsFloatScalarOrVector() const {
    return IsFloatScalar() || IsFloatVector();
}

bool Type::IsFloatVector() const {
    return Is([](const Vector* v) { return v->type->IsFloatScalar(); });
}

bool Type::IsIntegerScalar() const {
    return IsAnyOf<U32, I32>();
}

bool Type::IsIntegerScalarOrVector() const {
    return IsUnsignedScalarOrVector() || IsSignedScalarOrVector();
}

bool Type::IsScalar() const {
    return IsAnyOf<F32, F16, U32, I32, Bool>();
}

bool Type::IsSignedIntegerVector() const {
    return Is([](const Vector* v) { return v->type->Is<I32>(); });
}

bool Type::IsSignedScalarOrVector() const {
    return Is<I32>() || IsSignedIntegerVector();
}

bool Type::IsUnsignedIntegerVector() const {
    return Is([](const Vector* v) { return v->type->Is<U32>(); });
}

bool Type::IsUnsignedScalarOrVector() const {
    return Is<U32>() || IsUnsignedIntegerVector();
}

TypeManager::TypeManager() {
    state = std::make_unique<State>();
}

TypeManager::~TypeManager() = default;

const ast_parser::Void* TypeManager::Void() {
    if (!state->void_) {
        state->void_ = state->allocator_.Create<ast_parser::Void>();
    }
    return state->void_;
}

const ast_parser::Bool* TypeManager::Bool() {
    if (!state->bool_) {
        state->bool_ = state->allocator_.Create<ast_parser::Bool>();
    }
    return state->bool_;
}

const ast_parser::U32* TypeManager::U32() {
    if (!state->u32_) {
        state->u32_ = state->allocator_.Create<ast_parser::U32>();
    }
    return state->u32_;
}

const ast_parser::F32* TypeManager::F32() {
    if (!state->f32_) {
        state->f32_ = state->allocator_.Create<ast_parser::F32>();
    }
    return state->f32_;
}

const ast_parser::F16* TypeManager::F16() {
    if (!state->f16_) {
        state->f16_ = state->allocator_.Create<ast_parser::F16>();
    }
    return state->f16_;
}

const ast_parser::I32* TypeManager::I32() {
    if (!state->i32_) {
        state->i32_ = state->allocator_.Create<ast_parser::I32>();
    }
    return state->i32_;
}

const Type* TypeManager::AsUnsigned(const Type* ty) {
    return Switch(
        ty,                                             //
        [&](const ast_parser::I32*) { return U32(); },  //
        [&](const ast_parser::U32*) { return ty; },     //
        [&](const ast_parser::Vector* vec) {
            return Switch(
                vec->type,                                                         //
                [&](const ast_parser::I32*) { return Vector(U32(), vec->size); },  //
                [&](const ast_parser::U32*) { return ty; }                         //
            );
        });
}

const ast_parser::Pointer* TypeManager::Pointer(core::AddressSpace address_space,
                                                const Type* el,
                                                core::Access access) {
    return state->pointers_.Get(address_space, el, access);
}

const ast_parser::Reference* TypeManager::Reference(core::AddressSpace address_space,
                                                    const Type* el,
                                                    core::Access access) {
    return state->references_.Get(address_space, el, access);
}

const ast_parser::Vector* TypeManager::Vector(const Type* el, uint32_t size) {
    return state->vectors_.Get(el, size);
}

const ast_parser::Matrix* TypeManager::Matrix(const Type* el, uint32_t columns, uint32_t rows) {
    return state->matrices_.Get(el, columns, rows);
}

const ast_parser::Array* TypeManager::Array(const Type* el, uint32_t size, uint32_t stride) {
    return state->arrays_.Get(el, size, stride);
}

const ast_parser::Alias* TypeManager::Alias(Symbol name, const Type* ty) {
    return state->aliases_.Get(name, ty);
}

const ast_parser::Struct* TypeManager::Struct(Symbol name, TypeList members) {
    return state->structs_.Get(name, std::move(members));
}

const ast_parser::Sampler* TypeManager::Sampler(core::type::SamplerKind kind) {
    return state->samplers_.Get(kind);
}

const ast_parser::DepthTexture* TypeManager::DepthTexture(core::type::TextureDimension dims) {
    return state->depth_textures_.Get(dims);
}

const ast_parser::DepthMultisampledTexture* TypeManager::DepthMultisampledTexture(
    core::type::TextureDimension dims) {
    return state->depth_multisampled_textures_.Get(dims);
}

const ast_parser::MultisampledTexture* TypeManager::MultisampledTexture(
    core::type::TextureDimension dims,
    const Type* ty) {
    return state->multisampled_textures_.Get(dims, ty);
}

const ast_parser::SampledTexture* TypeManager::SampledTexture(core::type::TextureDimension dims,
                                                              const Type* ty) {
    return state->sampled_textures_.Get(dims, ty);
}

const ast_parser::StorageTexture* TypeManager::StorageTexture(core::type::TextureDimension dims,
                                                              core::TexelFormat fmt,
                                                              core::Access access) {
    return state->storage_textures_.Get(dims, fmt, access);
}

// Debug String() methods for Type classes. Only enabled in debug builds.
#ifndef NDEBUG
std::string Void::String() const {
    return "void";
}

std::string Bool::String() const {
    return "bool";
}

std::string U32::String() const {
    return "u32";
}

std::string F32::String() const {
    return "f32";
}

std::string F16::String() const {
    return "f16";
}

std::string I32::String() const {
    return "i32";
}

std::string Pointer::String() const {
    StringStream ss;
    ss << "ptr<" << tint::ToString(address_space) << ", " << type->String() + ">";
    return ss.str();
}

std::string Reference::String() const {
    StringStream ss;
    ss << "ref<" + tint::ToString(address_space) << ", " << type->String() << ">";
    return ss.str();
}

std::string Vector::String() const {
    StringStream ss;
    ss << "vec" << size << "<" << type->String() << ">";
    return ss.str();
}

std::string Matrix::String() const {
    StringStream ss;
    ss << "mat" << columns << "x" << rows << "<" << type->String() << ">";
    return ss.str();
}

std::string Array::String() const {
    StringStream ss;
    ss << "array<" << type->String() << ", " << size << ", " << stride << ">";
    return ss.str();
}

std::string Sampler::String() const {
    switch (kind) {
        case core::type::SamplerKind::kSampler:
            return "sampler";
        case core::type::SamplerKind::kComparisonSampler:
            return "sampler_comparison";
    }
    return "<unknown sampler>";
}

std::string DepthTexture::String() const {
    StringStream ss;
    ss << "depth_" << dims;
    return ss.str();
}

std::string DepthMultisampledTexture::String() const {
    StringStream ss;
    ss << "depth_multisampled_" << dims;
    return ss.str();
}

std::string MultisampledTexture::String() const {
    StringStream ss;
    ss << "texture_multisampled_" << dims << "<" << type << ">";
    return ss.str();
}

std::string SampledTexture::String() const {
    StringStream ss;
    ss << "texture_" << dims << "<" << type << ">";
    return ss.str();
}

std::string StorageTexture::String() const {
    StringStream ss;
    ss << "texture_storage_" << dims << "<" << format << ", " << access << ">";
    return ss.str();
}

std::string Named::String() const {
    return name.to_str();
}
#endif  // NDEBUG

}  // namespace tint::spirv::reader::ast_parser
