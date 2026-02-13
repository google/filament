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

#ifndef SRC_TINT_LANG_CORE_TYPE_MANAGER_H_
#define SRC_TINT_LANG_CORE_TYPE_MANAGER_H_

#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/string.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/texel_buffer.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/unique_node.h"
#include "src/tint/utils/containers/unique_allocator.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations
namespace tint::core::type {
class AbstractFloat;
class AbstractInt;
class Array;
class BindingArray;
class Bool;
class F16;
class F32;
class Function;
class I8;
class I32;
class Invalid;
class Matrix;
class Pointer;
class Reference;
class SampledTexture;
class StorageTexture;
class U8;
class U32;
class U64;
class Vector;
class Void;
}  // namespace tint::core::type

namespace tint::core::type {

/// @param space the address space of the memory view
/// @returns the default access control for a memory view with the given address space.
static constexpr inline core::Access DefaultAccessFor(core::AddressSpace space) {
    switch (space) {
        case core::AddressSpace::kIn:
        case core::AddressSpace::kImmediate:
        case core::AddressSpace::kUniform:
        case core::AddressSpace::kHandle:
            return core::Access::kRead;

        case core::AddressSpace::kUndefined:
        case core::AddressSpace::kOut:
        case core::AddressSpace::kFunction:
        case core::AddressSpace::kPixelLocal:
        case core::AddressSpace::kPrivate:
        case core::AddressSpace::kStorage:
        case core::AddressSpace::kWorkgroup:
            break;
    }

    return core::Access::kReadWrite;
}

/// The type manager holds all the pointers to the known types.
class Manager final {
  public:
    /// Iterator is the type returned by begin() and end()
    using TypeIterator = BlockAllocator<Type>::ConstIterator;

    /// Constructor
    Manager();

    /// Move constructor
    Manager(Manager&&);

    /// Move assignment operator
    /// @param rhs the Manager to move
    /// @return this Manager
    Manager& operator=(Manager&& rhs);

    /// Destructor
    ~Manager();

    /// Wrap returns a new Manager created with the types of `inner`.
    /// The Manager returned by Wrap is intended to temporarily extend the types
    /// of an existing immutable Manager.
    /// @warning As the copied types are owned by `inner`, `inner` must not be destructed or
    /// assigned while using the returned Manager.
    /// TODO(crbug.com/tint/460) - Evaluate whether there are safer alternatives to this
    /// function.
    /// @param inner the immutable Manager to extend
    /// @return the Manager that wraps `inner`
    static Manager Wrap(const Manager& inner) {
        Manager out;
        out.types_.Wrap(inner.types_);
        out.unique_nodes_.Wrap(inner.unique_nodes_);
        return out;
    }

    /// Constructs or returns an existing type, unique node or node
    /// @param args the arguments used to construct the type, unique node or node.
    /// @tparam T a class deriving from core::type::Node, or a C-like type that's automatically
    /// translated to the equivalent type node type. For example `Get<i32>()` is equivalent to
    /// `Get<core::type::I32>()`
    /// @return a pointer to an instance of `T` with the provided arguments.
    /// If `T` derives from UniqueNode and an existing instance of `T` has been constructed, then
    /// the same pointer is returned.
    template <typename T, typename... ARGS>
    auto* Get(ARGS&&... args) {
        if constexpr (std::is_same_v<T, tint::core::AInt>) {
            return Get<core::type::AbstractInt>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::AFloat>) {
            return Get<core::type::AbstractFloat>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::i8>) {
            return Get<core::type::I8>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::i32>) {
            return Get<core::type::I32>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::u8>) {
            return Get<core::type::U8>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::u32>) {
            return Get<core::type::U32>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::u64>) {
            return Get<core::type::U64>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::f32>) {
            return Get<core::type::F32>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, tint::core::f16>) {
            return Get<core::type::F16>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, bool>) {
            return Get<core::type::Bool>(std::forward<ARGS>(args)...);
        } else if constexpr (std::is_same_v<T, void>) {
            return Get<core::type::Void>(std::forward<ARGS>(args)...);
        } else if constexpr (core::fluent_types::IsVector<T>) {
            return vec<typename T::type, T::width>(std::forward<ARGS>(args)...);
        } else if constexpr (core::fluent_types::IsMatrix<T>) {
            return mat<T::columns, T::rows, typename T::type>(std::forward<ARGS>(args)...);
        } else if constexpr (core::fluent_types::IsPointer<T>) {
            return ptr<T::address, typename T::type, T::access>(std::forward<ARGS>(args)...);
        } else if constexpr (core::fluent_types::IsArray<T>) {
            return array<typename T::type, T::length>(std::forward<ARGS>(args)...);
        } else if constexpr (core::fluent_types::IsAtomic<T>) {
            return atomic<typename T::type>(std::forward<ARGS>(args)...);
        } else if constexpr (tint::traits::IsTypeOrDerived<T, Type>) {
            return types_.Get<T>(std::forward<ARGS>(args)...);
        } else if constexpr (tint::traits::IsTypeOrDerived<T, UniqueNode>) {
            return unique_nodes_.Get<T>(std::forward<ARGS>(args)...);
        } else {
            return nodes_.Create<T>(std::forward<ARGS>(args)...);
        }
    }

    /// @param args the arguments used to create the temporary used for the search.
    /// @return a pointer to an instance of `T` with the provided arguments, or nullptr if the item
    ///         was not found.
    template <typename TYPE,
              typename _ = std::enable_if<tint::traits::IsTypeOrDerived<TYPE, Type>>,
              typename... ARGS>
    auto* Find(ARGS&&... args) const {
        return types_.Find<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @returns an invalid type
    const core::type::Invalid* invalid();

    /// @returns an function type
    const core::type::Function* function();

    /// @returns a void type
    const core::type::Void* void_();

    /// @returns a bool type
    const core::type::Bool* bool_();

    /// @returns an i8 type
    const core::type::I8* i8();

    /// @returns an i32 type
    const core::type::I32* i32();

    /// @returns a u8 type
    const core::type::U8* u8();

    /// @returns a u32 type
    const core::type::U32* u32();

    /// @returns a u64 type
    const core::type::U64* u64();

    /// @returns an f32 type
    const core::type::F32* f32();

    /// @returns an f16 type
    const core::type::F16* f16();

    /// @returns a abstract-float type
    const core::type::AbstractFloat* AFloat();

    /// @returns a abstract-int type
    const core::type::AbstractInt* AInt();

    /// @param inner the inner type
    /// @returns an atomic type with the element type @p inner
    const core::type::Atomic* atomic(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns the atomic type
    template <typename T>
    const core::type::Atomic* atomic() {
        return atomic(Get<T>());
    }

    /// @param inner the inner type
    /// @param size the vector size
    /// @returns the vector type
    const core::type::Vector* packed_vec(const core::type::Type* inner, uint32_t size);

    /// @param inner the inner type
    /// @param size the vector size
    /// @returns the vector type
    const core::type::Vector* vec(const core::type::Type* inner, uint32_t size);

    /// @param inner the inner type
    /// @returns a vec2 type with the element type @p inner
    const core::type::Vector* vec2(const core::type::Type* inner);

    /// @param inner the inner type
    /// @returns a vec3 type with the element type @p inner
    const core::type::Vector* vec3(const core::type::Type* inner);

    /// @param inner the inner type
    /// @returns a vec4 type with the element type @p inner
    const core::type::Vector* vec4(const core::type::Type* inner);

    /// @param dim the dimensionality of the texture
    /// @param type the data type of the sampled texture
    /// @returns a sampled texture type with the provided params
    const core::type::SampledTexture* sampled_texture(TextureDimension dim,
                                                      const core::type::Type* type);

    /// @param dim the dimensionality of the texture
    /// @param type the data type of the sampled texture
    /// @returns a multisampled texture type with the provided params
    const core::type::MultisampledTexture* multisampled_texture(TextureDimension dim,
                                                                const core::type::Type* type);

    /// @param dim the dimensionality of the texture
    /// @param format the texel format of the texture
    /// @param access the access control type of the texture
    /// @returns a storage texture type with the provided params
    const core::type::StorageTexture* storage_texture(TextureDimension dim,
                                                      core::TexelFormat format,
                                                      core::Access access);

    /// @param format the texel format of the texel buffer
    /// @param access the access control type of the texel buffer
    /// @returns a texel buffer type with the provided params
    const core::type::TexelBuffer* texel_buffer(core::TexelFormat format, core::Access access);

    /// @param dim the dimensionality of the texture
    /// @returns a depth texture type with the provided params
    const core::type::DepthTexture* depth_texture(TextureDimension dim);

    /// @param dim the dimensionality of the texture
    /// @returns a depth multisampled texture type with the provided params
    const core::type::DepthMultisampledTexture* depth_multisampled_texture(TextureDimension dim);

    /// Return a type with element type `el_ty` that has the same number of vector components as
    /// `match`. If `match` is scalar just return `el_ty`.
    /// @param el_ty the type to extend
    /// @param match the type to match the component count of
    /// @returns a type with the same number of vector components as `match`
    const core::type::Type* MatchWidth(const core::type::Type* el_ty,
                                       const core::type::Type* match);

    // Return a type with element type `el_ty` that has the `size` number of vector components.
    // If `size` is 1, just return `el_ty`.
    /// @param el_ty the type to extend
    /// @param size the component size to match
    /// @returns a type with the `size` number of components
    const core::type::Type* MatchWidth(const core::type::Type* el_ty, size_t size);

    /// @tparam T the element type
    /// @tparam N the vector width
    /// @returns the vector type
    template <typename T, size_t N>
    const core::type::Vector* vec() {
        TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);
        static_assert(N >= 2 && N <= 4);
        switch (N) {
            case 2:
                return vec2<T>();
            case 3:
                return vec3<T>();
            case 4:
                return vec4<T>();
        }
        return nullptr;  // unreachable
        TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
    }

    /// @tparam T the element type
    /// @returns a vec2 with the element type `T`
    template <typename T>
    const core::type::Vector* vec2() {
        return vec2(Get<T>());
    }

    /// @tparam T the element type
    /// @returns a vec2 with the element type `T`
    template <typename T>
    const core::type::Vector* vec3() {
        return vec3(Get<T>());
    }

    /// @tparam T the element type
    /// @returns a vec2 with the element type `T`
    template <typename T>
    const core::type::Vector* vec4() {
        return vec4(Get<T>());
    }

    /// @param inner the inner type
    /// @param cols the number of columns
    /// @param rows the number of rows
    /// @returns the matrix type
    const core::type::Matrix* mat(const core::type::Type* inner, uint32_t cols, uint32_t rows);

    /// @param column_type the column vector type
    /// @param cols the number of columns
    /// @returns the matrix type
    const core::type::Matrix* mat(const core::type::Vector* column_type, uint32_t cols);

    /// @param inner the inner type
    /// @returns a mat2x2 with the element @p inner
    const core::type::Matrix* mat2x2(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat2x2 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat2x2() {
        return mat2x2(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat2x3 with the element @p inner
    const core::type::Matrix* mat2x3(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat2x3 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat2x3() {
        return mat2x3(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat2x4 with the element @p inner
    const core::type::Matrix* mat2x4(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat2x4 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat2x4() {
        return mat2x4(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat3x2 with the element @p inner
    const core::type::Matrix* mat3x2(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat3x2 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat3x2() {
        return mat3x2(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat3x3 with the element @p inner
    const core::type::Matrix* mat3x3(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat3x3 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat3x3() {
        return mat3x3(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat3x4 with the element @p inner
    const core::type::Matrix* mat3x4(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat3x4 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat3x4() {
        return mat3x4(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat4x2 with the element @p inner
    const core::type::Matrix* mat4x2(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat4x2 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat4x2() {
        return mat4x2(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat4x3 with the element @p inner
    const core::type::Matrix* mat4x3(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat4x3 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat4x3() {
        return mat4x3(Get<T>());
    }

    /// @param inner the inner type
    /// @returns a mat4x4 with the element @p inner
    const core::type::Matrix* mat4x4(const core::type::Type* inner);

    /// @tparam T the element type
    /// @returns a mat4x4 with the element type `T`
    template <typename T>
    const core::type::Matrix* mat4x4() {
        return mat4x4(Get<T>());
    }

    /// @param columns the number of columns of the matrix
    /// @param rows the number of rows of the matrix
    /// @tparam T the element type
    /// @returns a matrix with the given number of columns and rows
    template <typename T>
    const core::type::Matrix* mat(uint32_t columns, uint32_t rows) {
        return mat(Get<T>(), columns, rows);
    }

    /// @tparam C the number of columns in the matrix
    /// @tparam R the number of rows in the matrix
    /// @tparam T the element type
    /// @returns a matrix with the given number of columns and rows
    template <uint32_t C, uint32_t R, typename T>
    const core::type::Matrix* mat() {
        return mat(Get<T>(), C, R);
    }

    /// @param kind the subgroup matrix kind
    /// @param inner the inner type
    /// @param cols the number of columns
    /// @param rows the number of rows
    /// @returns the subgroup_matrix type
    const core::type::SubgroupMatrix* subgroup_matrix(SubgroupMatrixKind kind,
                                                      const core::type::Type* inner,
                                                      uint32_t cols,
                                                      uint32_t rows);

    /// @param inner the inner type
    /// @param cols the number of columns
    /// @param rows the number of rows
    /// @returns the subgroup_matrix type
    const core::type::SubgroupMatrix* subgroup_matrix_left(const core::type::Type* inner,
                                                           uint32_t cols,
                                                           uint32_t rows) {
        return subgroup_matrix(SubgroupMatrixKind::kLeft, inner, cols, rows);
    }

    /// @param inner the inner type
    /// @param cols the number of columns
    /// @param rows the number of rows
    /// @returns the subgroup_matrix type
    const core::type::SubgroupMatrix* subgroup_matrix_right(const core::type::Type* inner,
                                                            uint32_t cols,
                                                            uint32_t rows) {
        return subgroup_matrix(SubgroupMatrixKind::kRight, inner, cols, rows);
    }

    /// @param inner the inner type
    /// @param cols the number of columns
    /// @param rows the number of rows
    /// @returns the subgroup_matrix type
    const core::type::SubgroupMatrix* subgroup_matrix_result(const core::type::Type* inner,
                                                             uint32_t cols,
                                                             uint32_t rows) {
        return subgroup_matrix(SubgroupMatrixKind::kResult, inner, cols, rows);
    }

    /// @tparam K the kind of the matrix
    /// @tparam T the element type
    /// @tparam C the number of columns in the matrix
    /// @tparam R the number of rows in the matrix
    /// @returns a matrix with the given number of columns and rows
    template <SubgroupMatrixKind K, typename T, uint32_t C, uint32_t R>
    const core::type::SubgroupMatrix* subgroup_matrix() {
        return subgroup_matrix(K, Get<T>(), C, R);
    }

    /// @param elem_ty the array element type
    /// @param count the array element count
    /// @param stride the optional array element stride
    /// @returns the array type
    const core::type::Array* array(const core::type::Type* elem_ty,
                                   uint32_t count,
                                   uint32_t stride = 0);

    /// @param elem_ty the array element type
    /// @param stride the optional array element stride
    /// @returns the runtime array type
    const core::type::Array* runtime_array(const core::type::Type* elem_ty, uint32_t stride = 0);

    /// @returns an array type with the element type `T` and size `N`.
    /// @tparam T the element type
    /// @tparam N the array length. If zero, then constructs a runtime-sized array.
    /// @param stride the optional array element stride
    template <typename T, size_t N = 0>
    const core::type::Array* array(uint32_t stride = 0) {
        if constexpr (N == 0) {
            return runtime_array(Get<T>(), stride);
        } else {
            return array(Get<T>(), N, stride);
        }
    }

    /// @param elem_ty the array element type
    /// @param count the array element count
    /// @returns the array type
    const core::type::BindingArray* binding_array(const core::type::Type* elem_ty, uint32_t count);

    /// @param address_space the address space
    /// @param subtype the pointer subtype
    /// @param access the access settings
    /// @returns the pointer type
    const core::type::Pointer* ptr(core::AddressSpace address_space,
                                   const core::type::Type* subtype,
                                   core::Access access = core::Access::kUndefined);

    /// @tparam SPACE the address space
    /// @tparam T the storage type
    /// @tparam ACCESS the access mode
    /// @returns the pointer type with the templated address space, storage type and access.
    template <core::AddressSpace SPACE, typename T, core::Access ACCESS = DefaultAccessFor(SPACE)>
    const core::type::Pointer* ptr() {
        return ptr(SPACE, Get<T>(), ACCESS);
    }

    /// @param subtype the pointer subtype
    /// @tparam SPACE the address space
    /// @tparam ACCESS the access mode
    /// @returns the pointer type with the templated address space, storage type and access.
    template <core::AddressSpace SPACE, core::Access ACCESS = DefaultAccessFor(SPACE)>
    const core::type::Pointer* ptr(const core::type::Type* subtype) {
        return ptr(SPACE, subtype, ACCESS);
    }

    /// @param address_space the address space
    /// @param subtype the reference subtype
    /// @param access the access settings
    /// @returns the reference type
    const core::type::Reference* ref(core::AddressSpace address_space,
                                     const core::type::Type* subtype,
                                     core::Access access = core::Access::kReadWrite);

    /// @tparam SPACE the address space
    /// @tparam T the storage type
    /// @tparam ACCESS the access mode
    /// @returns the reference type with the templated address space, storage type and access.
    template <core::AddressSpace SPACE, typename T, core::Access ACCESS = core::Access::kReadWrite>
    const core::type::Reference* ref() {
        return ref(SPACE, Get<T>(), ACCESS);
    }

    /// @param subtype the reference subtype
    /// @tparam SPACE the address space
    /// @tparam ACCESS the access mode
    /// @returns the reference type with the templated address space, storage type and access.
    template <core::AddressSpace SPACE, core::Access ACCESS = core::Access::kReadWrite>
    const core::type::Reference* ref(const core::type::Type* subtype) {
        return ref(SPACE, subtype, ACCESS);
    }

    /// @returns the sampler type
    const core::type::Sampler* sampler() {
        return Get<core::type::Sampler>(core::type::SamplerKind::kSampler);
    }

    /// @returns the comparison sampler type
    const core::type::Sampler* comparison_sampler() {
        return Get<core::type::Sampler>(core::type::SamplerKind::kComparisonSampler);
    }

    /// @returns an input attachment type
    const core::type::InputAttachment* input_attachment(const core::type::Type* inner) {
        return Get<core::type::InputAttachment>(inner);
    }

    /// @returns a string type
    const core::type::String* String() { return Get<core::type::String>(); }

    /// A structure member descriptor.
    struct StructMemberDesc {
        /// The name of the struct member.
        Symbol name;
        /// The type of the struct member.
        const core::type::Type* type = nullptr;
        /// The optional struct member attributes.
        core::IOAttributes attributes = {};
    };

    /// Create a new structure declaration.
    /// @param name the name of the structure
    /// @param members the list of structure members
    /// @note a structure must not already exist with the same name
    /// @returns the structure type
    core::type::Struct* Struct(Symbol name, VectorRef<const StructMember*> members);

    /// Create a new structure declaration.
    /// @param name the name of the structure
    /// @param members the list of structure member descriptors
    /// @note a structure must not already exist with the same name
    /// @returns the structure type
    core::type::Struct* Struct(Symbol name, VectorRef<StructMemberDesc> members) {
        return Struct(name, /* is_wgsl_internal */ false,
                      tint::Vector<StructMemberDesc, 4>(members));
    }

    /// Create a new structure declaration.
    /// @param name the name of the structure
    /// @param members the list of structure member descriptors
    /// @note a structure must not already exist with the same name
    /// @returns the structure type
    core::type::Struct* Struct(Symbol name, std::initializer_list<StructMemberDesc> members) {
        return Struct(name, tint::Vector<StructMemberDesc, 4>(members));
    }

    /// Create a new WGSL internal structure declaration.
    /// @param name the name of the structure
    /// @param members the list of structure member descriptors
    /// @note an internal structure must not already exist with the same name
    /// @returns the structure type
    core::type::Struct* WgslInternalStruct(Symbol name,
                                           std::initializer_list<StructMemberDesc> members) {
        return Struct(name, /* is_wgsl_internal */ true,
                      tint::Vector<StructMemberDesc, 4>(members));
    }

    /// @returns the external texture type
    core::type::ExternalTexture* external_texture() { return Get<core::type::ExternalTexture>(); }

    /// @returns an iterator to the beginning of the types
    TypeIterator begin() const { return types_.begin(); }
    /// @returns an iterator to the end of the types
    TypeIterator end() const { return types_.end(); }

  private:
    /// Unique types owned by the manager
    UniqueAllocator<Type> types_;
    /// Unique nodes (excluding types) owned by the manager
    UniqueAllocator<UniqueNode> unique_nodes_;
    /// Non-unique nodes owned by the manager
    BlockAllocator<Node> nodes_;

    /// Create a new structure declaration.
    /// @param name the name of the structure
    /// @param is_wgsl_internal `true` if the structure is internally defined in WGSL
    /// @param members the list of structure member descriptors
    /// @note a structure must not already exist with the same name
    /// @returns the structure type
    core::type::Struct* Struct(Symbol name,
                               bool is_wgsl_internal,
                               VectorRef<StructMemberDesc> members);
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_MANAGER_H_
