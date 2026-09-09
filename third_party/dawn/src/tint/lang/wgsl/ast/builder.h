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

#ifndef SRC_TINT_LANG_WGSL_AST_BUILDER_H_
#define SRC_TINT_LANG_WGSL_AST_BUILDER_H_

#include <utility>

#include "src/tint/api/common/override_id.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/interpolation.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/sampler_kind.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/binding_attribute.h"
#include "src/tint/lang/wgsl/ast/blend_src_attribute.h"
#include "src/tint/lang/wgsl/ast/bool_literal_expression.h"
#include "src/tint/lang/wgsl/ast/break_if_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/case_statement.h"
#include "src/tint/lang/wgsl/ast/color_attribute.h"
#include "src/tint/lang/wgsl/ast/compound_assignment_statement.h"
#include "src/tint/lang/wgsl/ast/const.h"
#include "src/tint/lang/wgsl/ast/const_assert.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/diagnostic_attribute.h"
#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/ast/diagnostic_directive.h"
#include "src/tint/lang/wgsl/ast/diagnostic_rule_name.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/enable.h"
#include "src/tint/lang/wgsl/ast/float_literal_expression.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"
#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/input_attachment_index_attribute.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/ast/let.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/must_use_attribute.h"
#include "src/tint/lang/wgsl/ast/override.h"
#include "src/tint/lang/wgsl/ast/parameter.h"
#include "src/tint/lang/wgsl/ast/phony_expression.h"
#include "src/tint/lang/wgsl/ast/requires.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/lang/wgsl/ast/struct_member_align_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_size_attribute.h"
#include "src/tint/lang/wgsl/ast/subgroup_size_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/type.h"
#include "src/tint/lang/wgsl/ast/type_traits.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/utils/memory/block_allocator.h"
#include "src/tint/utils/symbol/symbol_table.h"
#include "src/tint/utils/text/string.h"

#ifdef CURRENTLY_IN_TINT_PUBLIC_HEADER
#error "internal tint header being #included from tint.h"
#endif

namespace tint::ast {

/// Builder is a mutable builder for AST nodes.
/// To construct a Program, populate the builder and then `std::move` it to a
/// Program.
class Builder {
    /// VarOptions is a helper for accepting an arbitrary number of order independent options for
    /// constructing a Var.
    struct VarOptions {
        template <typename... ARGS>
        explicit VarOptions(Builder& b, ARGS&&... args) {
            (Set(b, std::forward<ARGS>(args)), ...);
        }
        ~VarOptions();

        Type type;
        const Expression* address_space = nullptr;
        const Expression* access = nullptr;
        const Expression* initializer = nullptr;
        Vector<const Attribute*, 4> attributes;

      private:
        void Set(Builder&, Type t) { type = t; }
        void Set(Builder& b, core::AddressSpace addr_space) {
            if (addr_space != core::AddressSpace::kUndefined) {
                address_space = b.Expr(addr_space);
            }
        }
        void Set(Builder& b, core::Access ac) {
            if (ac != core::Access::kUndefined) {
                access = b.Expr(ac);
            }
        }
        void Set(Builder&, const Expression* c) { initializer = c; }
        void Set(Builder&, VectorRef<const Attribute*> l) { attributes = std::move(l); }
        void Set(Builder&, const Attribute* a) { attributes.Push(a); }
    };

  public:
    /// ASTNodeAllocator is an alias to BlockAllocator<Node>
    using ASTNodeAllocator = BlockAllocator<Node>;

    /// Constructor
    Builder();

    /// Move constructor
    /// @param rhs the builder to move
    Builder(Builder&& rhs);

    /// Destructor
    virtual ~Builder();

    /// Move assignment operator
    /// @param rhs the builder to move
    /// @return this builder
    Builder& operator=(Builder&& rhs);

    /// @returns a reference to the program's AST nodes storage
    ASTNodeAllocator& ASTNodes() {
        AssertNotMoved();
        return ast_nodes_;
    }

    /// @returns a reference to the program's AST nodes storage
    const ASTNodeAllocator& ASTNodes() const {
        AssertNotMoved();
        return ast_nodes_;
    }

    /// @returns a reference to the program's AST root Module
    Module& AST() {
        AssertNotMoved();
        return *ast_;
    }

    /// @returns a reference to the program's AST root Module
    const Module& AST() const {
        AssertNotMoved();
        return *ast_;
    }

    /// @returns a reference to the program's SymbolTable
    SymbolTable& Symbols() {
        AssertNotMoved();
        return symbols_;
    }

    /// @returns a reference to the program's SymbolTable
    const SymbolTable& Symbols() const {
        AssertNotMoved();
        return symbols_;
    }

    /// @returns a reference to the program's diagnostics
    diag::List& Diagnostics() {
        AssertNotMoved();
        return diagnostics_;
    }

    /// @returns a reference to the program's diagnostics
    const diag::List& Diagnostics() const {
        AssertNotMoved();
        return diagnostics_;
    }

    /// @returns true if the program has no error diagnostics and is not missing
    /// information
    bool IsValid() const;

    /// @returns the last allocated (numerically highest) AST node identifier.
    NodeID LastAllocatedNodeID() const { return last_ast_node_id_; }

    /// @returns the next sequentially unique node identifier.
    NodeID AllocateNodeID() {
        auto out = NodeID{last_ast_node_id_.value + 1};
        last_ast_node_id_ = out;
        return out;
    }

    /// Creates a new Node owned by the Builder. When the
    /// Builder is destructed, the Node will also be destructed.
    /// @param source the Source of the node
    /// @param args the arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
        requires(traits::IsTypeOrDerived<T, Node>)
    T* create(const Source& source, ARGS&&... args) {
        AssertNotMoved();
        return ast_nodes_.Create<T>(AllocateNodeID(), source, std::forward<ARGS>(args)...);
    }

    /// Creates a new Node owned by the Builder, injecting the current
    /// Source as set by the last call to SetSource() as the only argument to the
    /// constructor.
    /// When the Builder is destructed, the Node will also be
    /// destructed.
    /// @returns the node pointer
    template <typename T>
        requires(traits::IsTypeOrDerived<T, Node>)
    T* create() {
        AssertNotMoved();
        return ast_nodes_.Create<T>(AllocateNodeID(), source_);
    }

    /// Creates a new Node owned by the Builder, injecting the current
    /// Source as set by the last call to SetSource() as the first argument to the
    /// constructor.
    /// When the Builder is destructed, the Node will also be
    /// destructed.
    /// @param arg0 the first arguments to pass to the constructor
    /// @param args the remaining arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename ARG0, typename... ARGS>
        requires(traits::IsTypeOrDerived<T, Node> && !traits::IsTypeOrDerived<ARG0, Source>)
    T* create(ARG0&& arg0, ARGS&&... args) {
        AssertNotMoved();
        return ast_nodes_.Create<T>(AllocateNodeID(), source_, std::forward<ARG0>(arg0),
                                    std::forward<ARGS>(args)...);
    }

    /// Marks this builder as moved, preventing any further use of the builder.
    void MarkAsMoved();

    //////////////////////////////////////////////////////////////////////////////
    // TypesBuilder
    //////////////////////////////////////////////////////////////////////////////

    /// TypesBuilder holds basic `tint` types and methods for constructing
    /// complex types.
    class TypesBuilder {
      public:
        /// Constructor
        /// @param builder the program builder
        explicit TypesBuilder(Builder* builder);

        /// @return the C type `T`.
        template <typename T>
        Type Of() const {
            return CToAST<T>::get(this);
        }

        /// @param type the type
        /// @return an Type of the type declaration.
        Type Of(const TypeDecl* type) const;

        /// @param sym the name of the type
        /// @returns a type with the given name
        Type AsType(Symbol sym) const;

        /// @param source the source
        /// @param sym the name of the type
        /// @returns a type with the given name
        Type AsType(const Source& source, Symbol sym) const;

        /// @param name the name of the type
        /// @returns a type with the given name
        Type AsType(std::string_view name) const;

        /// @param source the source
        /// @param name the name of the type
        /// @returns a type with the given name
        Type AsType(const Source& source, std::string_view name) const;

        /// @param name the name of the type
        /// @returns a type with the given name
        template <typename... ARGS>
        Type AsType(std::string_view name, ARGS&&... args) const {
            return AsType(builder->source_, name, std::forward<ARGS>(args)...);
        }

        /// @param source the source
        /// @param name the name of the type
        /// @returns a type with the given name
        template <typename... ARGS>
        Type AsType(const Source& source, std::string_view name, ARGS&&... args) const {
            return {builder->Expr(builder->Ident(source, name, std::forward<ARGS>(args)...))};
        }

        /// @param ident the name of the type
        /// @returns a type with the given name
        Type AsType(const IdentifierExpression* ident) const;

        /// @returns a a nullptr expression wrapped in a Type
        Type void_() const;

        /// @returns a 'bool' type
        Type bool_() const;

        /// @param source the Source of the node
        /// @returns a 'bool' type
        Type bool_(const Source& source) const;

        /// @returns a 'f16' type
        Type f16() const;

        /// @param source the Source of the node
        /// @returns a 'f16' type
        Type f16(const Source& source) const;

        /// @returns a 'f32' type
        Type f32() const;

        /// @param source the Source of the node
        /// @returns a 'f32' type
        Type f32(const Source& source) const;

        /// @returns a 'i32' type
        Type i32() const;

        /// @param source the Source of the node
        /// @returns a 'i32' type
        Type i32(const Source& source) const;

        /// @returns a 'u32' type
        Type u32() const;

        /// @param source the Source of the node
        /// @returns a 'u32' type
        Type u32(const Source& source) const;

        /// @returns a 'i8' type
        Type i8() const;

        /// @param source the Source of the node
        /// @returns a 'i8' type
        Type i8(const Source& source) const;

        /// @returns a 'u8' type
        Type u8() const;

        /// @param source the Source of the node
        /// @returns a 'u8' type
        Type u8(const Source& source) const;

        /// @param type vector subtype
        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        Type vec(Type type, uint32_t n) const;

        /// @param source the Source of the node
        /// @param type vector subtype
        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        Type vec(const Source& source, Type type, uint32_t n) const;

        /// @param type vector subtype
        /// @return a 2-element vector of @p type
        Type vec2(Type type) const;

        /// @param source the vector source
        /// @param type vector subtype
        /// @return a 2-element vector of @p type
        Type vec2(const Source& source, Type type) const;

        /// @param type vector subtype
        /// @return a 3-element vector of @p type
        Type vec3(Type type) const;

        /// @param source the vector source
        /// @param type vector subtype
        /// @return a 3-element vector of @p type
        Type vec3(const Source& source, Type type) const;

        /// @param type vector subtype
        /// @return a 4-element vector of @p type
        Type vec4(Type type) const;

        /// @param source the vector source
        /// @param type vector subtype
        /// @return a 4-element vector of @p type
        Type vec4(const Source& source, Type type) const;

        /// @param source the Source of the node
        /// @return a 2-element vector of the type `T`
        template <typename T>
        Type vec2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType(source, "vec2");
            } else {
                return AsType(source, "vec2", Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @return a 3-element vector of the type `T`
        template <typename T>
        Type vec3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType(source, "vec3");
            } else {
                return AsType(source, "vec3", Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @return a 4-element vector of the type `T`
        template <typename T>
        Type vec4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType(source, "vec4");
            } else {
                return AsType(source, "vec4", Of<T>());
            }
        }

        /// @return a 2-element vector of the type `T`
        template <typename T>
        Type vec2() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("vec2");
            } else {
                return vec2(Of<T>());
            }
        }

        /// @return a 3-element vector of the type `T`
        template <typename T>
        Type vec3() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("vec3");
            } else {
                return vec3(Of<T>());
            }
        }

        /// @return a 4-element vector of the type `T`
        template <typename T>
        Type vec4() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("vec4");
            } else {
                return vec4(Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        template <typename T>
        Type vec(const Source& source, uint32_t n) const {
            switch (n) {
                case 2:
                    return vec2<T>(source);
                case 3:
                    return vec3<T>(source);
                case 4:
                    return vec4<T>(source);
            }
            TINT_ICE() << "invalid vector width " << n;
        }

        /// @return a @p N element vector of @p type
        template <typename T, uint32_t N>
        Type vec() const {
            return vec<T>(N);
        }

        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        template <typename T>
        Type vec(uint32_t n) const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("vec" + std::to_string(n));
            } else {
                return vec(Of<T>(), n);
            }
        }

        /// @param type matrix subtype
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        Type mat(Type type, uint32_t columns, uint32_t rows) const;

        /// @param source the Source of the node
        /// @param type matrix subtype
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        Type mat(const Source& source, Type type, uint32_t columns, uint32_t rows) const;

        /// @param type matrix subtype
        /// @return a 2x3 matrix of @p type.
        Type mat2x2(Type type) const;

        /// @param type matrix subtype
        /// @return a 2x3 matrix of @p type.
        Type mat2x3(Type type) const;

        /// @param type matrix subtype
        /// @return a 2x4 matrix of @p type.
        Type mat2x4(Type type) const;

        /// @param type matrix subtype
        /// @return a 3x2 matrix of @p type.
        Type mat3x2(Type type) const;

        /// @param type matrix subtype
        /// @return a 3x3 matrix of @p type.
        Type mat3x3(Type type) const;

        /// @param type matrix subtype
        /// @return a 3x4 matrix of @p type.
        Type mat3x4(Type type) const;

        /// @param type matrix subtype
        /// @return a 4x2 matrix of @p type.
        Type mat4x2(Type type) const;

        /// @param type matrix subtype
        /// @return a 4x3 matrix of @p type.
        Type mat4x3(Type type) const;

        /// @param type matrix subtype
        /// @return a 4x4 matrix of @p type.
        Type mat4x4(Type type) const;

        /// @param source the source of the type
        /// @return a 2x2 matrix of the type `T`
        template <typename T>
        Type mat2x2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat2x2(source);
            } else {
                return mat2x2(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 2x2 matrix
        Type mat2x2(const Source& source) const;

        /// @param source the source of the type
        /// @return a 2x2 matrix
        Type mat2x2(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 2x3 matrix of the type `T`
        template <typename T>
        Type mat2x3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat2x3(source);
            } else {
                return mat2x3(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 2x3 matrix
        Type mat2x3(const Source& source) const;

        /// @param source the source of the type
        /// @return a 2x3 matrix
        Type mat2x3(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 2x4 matrix of the type `T`
        template <typename T>
        Type mat2x4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat2x4(source);
            } else {
                return mat2x4(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 2x4 matrix
        Type mat2x4(const Source& source) const;

        /// @param source the source of the type
        /// @return a 2x4 matrix
        Type mat2x4(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 3x2 matrix of the type `T`
        template <typename T>
        Type mat3x2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat3x2(source);
            } else {
                return mat3x2(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 3x2 matrix
        Type mat3x2(const Source& source) const;

        /// @param source the source of the type
        /// @return a 3x2 matrix
        Type mat3x2(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 3x3 matrix of the type `T`
        template <typename T>
        Type mat3x3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat3x3(source);
            } else {
                return mat3x3(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 3x3 matrix
        Type mat3x3(const Source& source) const;

        /// @param source the source of the type
        /// @return a 3x3 matrix
        Type mat3x3(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 3x4 matrix of the type `T`
        template <typename T>
        Type mat3x4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat3x4(source);
            } else {
                return mat3x4(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 3x4 matrix
        Type mat3x4(const Source& source) const;

        /// @param source the source of the type
        /// @return a 3x4 matrix
        Type mat3x4(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 4x2 matrix of the type `T`
        template <typename T>
        Type mat4x2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat4x2(source);
            } else {
                return mat4x2(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 4x2 matrix
        Type mat4x2(const Source& source) const;

        /// @param source the source of the type
        /// @return a 4x2 matrix
        Type mat4x2(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 4x3 matrix of the type `T`
        template <typename T>
        Type mat4x3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat4x3(source);
            } else {
                return mat4x3(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 4x3 matrix
        Type mat4x3(const Source& source) const;

        /// @param source the source of the type
        /// @return a 4x3 matrix
        Type mat4x3(const Source& source, Type type) const;

        /// @param source the source of the type
        /// @return a 4x4 matrix of the type `T`
        template <typename T>
        Type mat4x4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return mat4x4(source);
            } else {
                return mat4x4(source, Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 4x4 matrix
        Type mat4x4(const Source& source) const;

        /// @param source the source of the type
        /// @return a 4x4 matrix
        Type mat4x4(const Source& source, Type type) const;

        /// @return a 2x2 matrix of the type `T`
        template <typename T>
        Type mat2x2() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat2x2");
            } else {
                return mat2x2(Of<T>());
            }
        }

        /// @return a 2x3 matrix of the type `T`
        template <typename T>
        Type mat2x3() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat2x3");
            } else {
                return mat2x3(Of<T>());
            }
        }

        /// @return a 2x4 matrix of the type `T`
        template <typename T>
        Type mat2x4() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat2x4");
            } else {
                return mat2x4(Of<T>());
            }
        }

        /// @return a 3x2 matrix of the type `T`
        template <typename T>
        Type mat3x2() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat3x2");
            } else {
                return mat3x2(Of<T>());
            }
        }

        /// @return a 3x3 matrix of the type `T`
        template <typename T>
        Type mat3x3() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat3x3");
            } else {
                return mat3x3(Of<T>());
            }
        }

        /// @return a 3x4 matrix of the type `T`
        template <typename T>
        Type mat3x4() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat3x4");
            } else {
                return mat3x4(Of<T>());
            }
        }

        /// @return a 4x2 matrix of the type `T`
        template <typename T>
        Type mat4x2() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat4x2");
            } else {
                return mat4x2(Of<T>());
            }
        }

        /// @return a 4x3 matrix of the type `T`
        template <typename T>
        Type mat4x3() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat4x3");
            } else {
                return mat4x3(Of<T>());
            }
        }

        /// @return a 4x4 matrix of the type `T`
        template <typename T>
        Type mat4x4() const {
            if constexpr (IsInferOrAbstract<T>) {
                return AsType("mat4x4");
            } else {
                return mat4x4(Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        template <typename T>
        Type mat(const Source& source, uint32_t columns, uint32_t rows) const {
            switch ((columns - 2) * 3 + (rows - 2)) {
                case 0:
                    return mat2x2<T>(source);
                case 1:
                    return mat2x3<T>(source);
                case 2:
                    return mat2x4<T>(source);
                case 3:
                    return mat3x2<T>(source);
                case 4:
                    return mat3x3<T>(source);
                case 5:
                    return mat3x4<T>(source);
                case 6:
                    return mat4x2<T>(source);
                case 7:
                    return mat4x3<T>(source);
                case 8:
                    return mat4x4<T>(source);
                default:
                    TINT_ICE() << "invalid matrix dimensions " << columns << "x" << rows;
            }
        }

        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        template <typename T>
        Type mat(uint32_t columns, uint32_t rows) const {
            switch ((columns - 2) * 3 + (rows - 2)) {
                case 0:
                    return mat2x2<T>();
                case 1:
                    return mat2x3<T>();
                case 2:
                    return mat2x4<T>();
                case 3:
                    return mat3x2<T>();
                case 4:
                    return mat3x3<T>();
                case 5:
                    return mat3x4<T>();
                case 6:
                    return mat4x2<T>();
                case 7:
                    return mat4x3<T>();
                case 8:
                    return mat4x4<T>();
                default:
                    TINT_ICE() << "invalid matrix dimensions " << columns << "x" << rows;
            }
        }

        /// @return a matrix of @p type
        template <typename T, uint32_t COLUMNS, uint32_t ROWS>
        Type mat() const {
            return mat<T>(COLUMNS, ROWS);
        }

        /// @return an array of abstract type
        Type array() const;

        /// @param source the source
        /// @return an array of abstract type
        Type array(const Source& source) const;

        /// @param subtype the array element type
        /// @return an array of type `T`
        Type array(Type subtype) const;

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @return an array of type `T`
        Type array(const Source& source, Type subtype) const;

        /// @param subtype the array element type
        /// @param n the array size.
        /// @return an array of size `n` of type `T`
        Type array(Type subtype, uint32_t n) const;

        /// @param subtype the array element type
        /// @param expr the array size. nullptr means runtime array
        /// @return an array of size `n` of type `T`
        Type array(Type subtype, const Const* expr) const;

        /// @param subtype the array element type
        /// @param expr the array size. nullptr means runtime array
        /// @return an array of size `n` of type `T`
        Type array(Type subtype, const Expression* expr) const;

        /// @param subtype the array element type
        /// @param expr the array size. nullptr means runtime array
        /// @return an array of size `n` of type `T`
        Type array(Type subtype, const Override* expr) const;

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param n the array size.
        /// @return an array of size `n` of type `T`
        Type array(const Source& source, Type subtype, uint32_t n) const;

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param expr the array size. nullptr means runtime array
        /// @return an array of size `n` of type `T`
        Type array(const Source& source, Type subtype, const Const* expr) const;

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param expr the array size. nullptr means runtime array
        /// @return an array of size `n` of type `T`
        Type array(const Source& source, Type subtype, const Expression* expr) const;

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param expr the array size. nullptr means runtime array
        /// @return an array of size `n` of type `T`
        Type array(const Source& source, Type subtype, const Override* expr) const;

        /// @param source the Source of the node
        /// @return a inferred-size or runtime-sized array of type `T`
        template <typename T, int N = 0, typename = DisableIfInferOrAbstract<T>>
        Type array(const Source& source) const {
            if constexpr (N == 0) {
                Expression* expr = nullptr;
                return array(source, Of<T>(), expr);
            } else {
                return array(source, Of<T>(), uint32_t{N});
            }
        }

        /// @return an array of size `N` of type `T`
        template <typename T, int N = 0>
        Type array() const {
            if constexpr (std::is_same_v<T, core::fluent_types::Infer>) {
                static_assert(N == 0, "arrays with a count cannot be inferred");
                return array();
            } else {
                return array(Of<T>(), uint32_t{N});
            }
        }

        /// Creates an alias type
        /// @param name the alias name
        /// @param type the alias type
        /// @returns the alias pointer
        const ast::Alias* alias(std::string_view name, Type type) const;

        /// Creates an alias type
        /// @param name the alias name
        /// @param type the alias type
        /// @returns the alias pointer
        const ast::Alias* alias(Symbol name, Type type) const;

        /// Creates an alias type
        /// @param source the Source of the node
        /// @param name the alias name
        /// @param type the alias type
        /// @returns the alias pointer
        const ast::Alias* alias(const Source& source, const Identifier* name, Type type) const;

        /// @param address_space the address space of the pointer
        /// @param type the type of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given core::AddressSpace
        Type ptr(core::AddressSpace address_space,
                 Type type,
                 core::Access access = core::Access::kUndefined) const;

        /// @param source the Source of the node
        /// @param address_space the address space of the pointer
        /// @param type the type of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given core::AddressSpace
        Type ptr(const Source& source,
                 core::AddressSpace address_space,
                 Type type,
                 core::Access access = core::Access::kUndefined) const;

        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to type `T` with the given core::AddressSpace.
        template <typename T>
        Type ptr(core::AddressSpace address_space,
                 core::Access access = core::Access::kUndefined) const {
            return ptr(address_space, Of<T>(), access);
        }

        /// @param source the Source of the node
        /// @return the pointer to type `T` with the core::AddressSpace `ADDRESS` and access
        /// control `ACCESS`.
        template <core::AddressSpace ADDRESS,
                  typename T,
                  core::Access ACCESS = core::Access::kUndefined>
        Type ptr(const Source& source) const {
            return ptr<T>(source, ADDRESS, ACCESS);
        }

        /// @param type the type of the pointer
        /// @return the pointer to the given type with the core::AddressSpace `ADDRESS` and
        /// access control `ACCESS`.
        template <core::AddressSpace ADDRESS, core::Access ACCESS = core::Access::kUndefined>
        Type ptr(Type type) const {
            return ptr(ADDRESS, type, ACCESS);
        }

        /// @param source the Source of the node
        /// @param type the type of the pointer
        /// @return the pointer to the given type with the core::AddressSpace `ADDRESS` and
        /// access control `ACCESS`.
        template <core::AddressSpace ADDRESS, core::Access ACCESS = core::Access::kUndefined>
        Type ptr(const Source& source, Type type) const {
            return ptr(source, ADDRESS, type, ACCESS);
        }

        /// @return the pointer to type `T` with the core::AddressSpace `ADDRESS` and access
        /// control `ACCESS`.
        template <core::AddressSpace ADDRESS,
                  typename T,
                  core::Access ACCESS = core::Access::kUndefined>
        Type ptr() const {
            return ptr<T>(ADDRESS, ACCESS);
        }

        /// @param source the Source of the node
        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to type `T` the core::AddressSpace `ADDRESS` and access control
        /// `ACCESS`.
        template <typename T>
        Type ptr(const Source& source,
                 core::AddressSpace address_space,
                 core::Access access = core::Access::kUndefined) const {
            if (access != core::Access::kUndefined) {
                return ptr(source, address_space, Of<T>(), access);
            } else {
                return ptr(source, address_space, Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @param type the type of the atomic
        /// @return the atomic to `type`
        Type atomic(const Source& source, Type type) const;

        /// @param type the type of the atomic
        /// @return the atomic to `type`
        Type atomic(Type type) const;

        /// @return the atomic to type `T`
        template <typename T>
        Type atomic() const {
            return atomic(Of<T>());
        }

        /// @param kind the kind of sampler
        /// @returns the sampler
        Type sampler(core::type::SamplerKind kind) const;

        /// @param source the Source of the node
        /// @param kind the kind of sampler
        /// @returns the sampler
        Type sampler(const Source& source, core::type::SamplerKind kind) const;

        /// @param dims the dimensionality of the texture
        /// @returns the depth texture
        Type depth_texture(core::type::TextureDimension dims) const;

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @returns the depth texture
        Type depth_texture(const Source& source, core::type::TextureDimension dims) const;

        /// @param dims the dimensionality of the texture
        /// @returns the multisampled depth texture
        Type depth_multisampled_texture(core::type::TextureDimension dims) const;

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @returns the multisampled depth texture
        Type depth_multisampled_texture(const Source& source,
                                        core::type::TextureDimension dims) const;

        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the sampled texture
        Type sampled_texture(core::type::TextureDimension dims, Type subtype) const;

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the sampled texture
        Type sampled_texture(const Source& source,
                             core::type::TextureDimension dims,
                             Type subtype) const;

        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the multisampled texture
        Type multisampled_texture(core::type::TextureDimension dims, Type subtype) const;

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the multisampled texture
        Type multisampled_texture(const Source& source,
                                  core::type::TextureDimension dims,
                                  Type subtype) const;

        /// @param dims the dimensionality of the texture
        /// @param format the texel format of the texture
        /// @param access the access control of the texture
        /// @returns the storage texture
        Type storage_texture(core::type::TextureDimension dims,
                             core::TexelFormat format,
                             core::Access access) const;

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param format the texel format of the texture
        /// @param access the access control of the texture
        /// @returns the storage texture
        Type storage_texture(const Source& source,
                             core::type::TextureDimension dims,
                             core::TexelFormat format,
                             core::Access access) const;

        /// @param format the texel format
        /// @param access the access control
        /// @returns the texel buffer
        Type texel_buffer(core::TexelFormat format, core::Access access) const;

        /// @param source the source
        /// @param format the texel format
        /// @param access the access control
        /// @returns the texel buffer
        Type texel_buffer(const Source& source,
                          core::TexelFormat format,
                          core::Access access) const;

        /// @param subtype the texture subtype.
        /// @returns the input attachment
        Type input_attachment(Type subtype) const;

        /// @param source the Source of the node
        /// @returns the external texture
        Type external_texture(const Source& source) const;

        /// @returns the external texture
        Type external_texture() const;

        /// @param el the subgroup matrix element type
        /// @param cols the column count
        /// @param rows the row count
        /// @returns the subgroup matrix
        template <typename C, typename R>
            requires(core::IsNumber<C> && core::IsNumeric<R>)
        Type subgroup_matrix_result(Type el, C cols, R rows) const {
            return AsType("subgroup_matrix_result", el, cols, rows);
        }

        /// @param source the source
        /// @param el the subgroup matrix element type
        /// @param cols the column count
        /// @param rows the row count
        /// @returns the subgroup matrix
        template <typename C, typename R>
            requires(core::IsNumber<C> && core::IsNumeric<R>)
        Type subgroup_matrix_result(const Source& source, Type el, C cols, R rows) const {
            return AsType(source, "subgroup_matrix_result", el, cols, rows);
        }

        /// @param el the subgroup matrix element type
        /// @param cols the column count
        /// @param rows the row count
        /// @returns the subgroup matrix
        Type subgroup_matrix_result(Type el, uint32_t cols, uint32_t rows) const;

        /// @param source the source
        /// @param el the subgroup matrix element type
        /// @param cols the column count
        /// @param rows the row count
        /// @returns the subgroup matrix
        Type subgroup_matrix_result(const Source& source,
                                    Type el,
                                    uint32_t cols,
                                    uint32_t rows) const;

        /// @param el the subgroup matrix element type
        /// @param cols the column count
        /// @param rows the row count
        /// @returns the subgroup matrix
        Type subgroup_matrix_right(Type el, uint32_t cols, uint32_t rows) const;

        /// @param el the subgroup matrix element type
        /// @param cols the column count
        /// @param rows the row count
        /// @returns the subgroup matrix
        Type subgroup_matrix_left(Type el, uint32_t cols, uint32_t rows) const;

        /// @param kind the subgroup matrix kind
        /// @param el the subgroup matrix element type
        /// @param cols the column count
        /// @param rows the row count
        /// @returns the subgroup matrix
        Type subgroup_matrix(core::SubgroupMatrixKind kind,
                             Type el,
                             uint32_t cols,
                             uint32_t rows) const;

        /// @param size the buffer size (0 is unsized)
        /// @returns the buffer
        template <typename NUM>
            requires(core::IsNumber<NUM>)
        Type buffer(NUM size) const {
            return AsType("buffer", size);
        }

        /// @param size the buffer size (0 is unsized)
        /// @returns the buffer
        Type buffer(uint32_t size = 0) const;

        /// @param el the binding_array element type
        /// @param size the number of binding array elements
        /// @returns the binding array
        template <typename COUNT, typename = DisableIfVectorLike<COUNT>>
        Type binding_array(Type el, COUNT&& size) const {
            return AsType("binding_array", el, std::forward<COUNT>(size));
        }

      private:
        /// CToAST<T> is specialized for various `T` types and each specialization
        /// contains a single static `get()` method for obtaining the corresponding
        /// AST type for the C type `T`.
        /// `get()` has the signature:
        ///    `static Type get(Types* t)`
        template <typename T>
        struct CToAST {};

        /// The Builder
        Builder* const builder;
    };

    //////////////////////////////////////////////////////////////////////////////
    // AST helper methods
    //////////////////////////////////////////////////////////////////////////////

    /// @return a new unnamed symbol
    Symbol Sym() { return Symbols().New(); }

    /// Passthrough
    /// @param sym the symbol
    /// @return `sym`
    Symbol Sym(Symbol sym) { return sym; }

    /// @param name the symbol string
    /// @return a Symbol with the given name
    Symbol Sym(std::string_view name) { return Symbols().Register(name); }

    /// @param enumerator the enumerator
    /// @return a Symbol with the given enum value
    template <typename ENUM>
        requires(std::is_enum_v<std::decay_t<ENUM>>)
    Symbol Sym(ENUM&& enumerator) {
        return Sym(tint::ToString(enumerator));
    }

    /// @return nullptr
    const Identifier* Ident(std::nullptr_t) { return nullptr; }

    /// @param identifier the identifier symbol
    /// @return an Identifier with the given symbol
    template <typename IDENTIFIER>
    const Identifier* Ident(IDENTIFIER&& identifier) {
        if constexpr (traits::IsTypeOrDerived<traits::PtrElTy<IDENTIFIER>, Identifier>) {
            return identifier;  // Passthrough
        } else {
            return Ident(source_, std::forward<IDENTIFIER>(identifier));
        }
    }

    /// @param source the source information
    /// @param identifier the identifier symbol
    /// @return an Identifier with the given symbol
    template <typename IDENTIFIER>
    const Identifier* Ident(const Source& source, IDENTIFIER&& identifier) {
        return create<Identifier>(source, Sym(std::forward<IDENTIFIER>(identifier)));
    }

    /// @param identifier the identifier symbol
    /// @param args the templated identifier arguments
    /// @return an Identifier with the given symbol and template arguments
    template <typename IDENTIFIER, typename... ARGS>
        requires(!IsSource<std::decay_t<IDENTIFIER>>)
    const Identifier* Ident(IDENTIFIER&& identifier, ARGS&&... args) {
        return Ident(source_, std::forward<IDENTIFIER>(identifier), std::forward<ARGS>(args)...);
    }

    /// @param source the source information
    /// @param identifier the identifier symbol
    /// @param args the templated identifier arguments
    /// @return an Identifier with the given symbol and template arguments
    template <typename IDENTIFIER, typename... ARGS>
    const Identifier* Ident(const Source& source, IDENTIFIER&& identifier, ARGS&&... args) {
        auto arg_exprs = ExprList(std::forward<ARGS>(args)...);
        if (arg_exprs.IsEmpty()) {
            return create<Identifier>(source, Sym(std::forward<IDENTIFIER>(identifier)));
        }
        return create<TemplatedIdentifier>(source, Sym(std::forward<IDENTIFIER>(identifier)),
                                           std::move(arg_exprs));
    }

    /// @param expr the expression
    /// @return expr (passthrough)
    template <typename T>
        requires(traits::IsTypeOrDerived<T, Expression>)
    const T* Expr(const T* expr) {
        return expr;
    }

    /// @param type an Type
    /// @return type.expr
    const IdentifierExpression* Expr(Type type) { return type.expr; }

    /// @param ident the identifier
    /// @return an IdentifierExpression with the given identifier
    const IdentifierExpression* Expr(const Identifier* ident) {
        return ident ? create<IdentifierExpression>(ident->source, ident) : nullptr;
    }

    /// Passthrough for nullptr
    /// @return nullptr
    const IdentifierExpression* Expr(std::nullptr_t) { return nullptr; }

    /// @param name the identifier name
    /// @return an IdentifierExpression with the given name
    template <typename NAME>
        requires(IsIdentifierLike<std::decay_t<NAME>>)
    const IdentifierExpression* Expr(NAME&& name) {
        auto* ident = Ident(source_, name);
        return create<IdentifierExpression>(ident->source, ident);
    }

    /// @param source the source information
    /// @param name the identifier name
    /// @return an IdentifierExpression with the given name
    template <typename NAME>
        requires(IsIdentifierLike<std::decay_t<NAME>>)
    const IdentifierExpression* Expr(const Source& source, NAME&& name) {
        return create<IdentifierExpression>(source, Ident(source, name));
    }

    /// @param variable the AST variable
    /// @return an IdentifierExpression with the variable's symbol
    const IdentifierExpression* Expr(const Variable* variable) {
        auto* ident = Ident(variable->source, variable->name->symbol);
        return create<IdentifierExpression>(ident->source, ident);
    }

    /// @param source the source information
    /// @param variable the AST variable
    /// @return an IdentifierExpression with the variable's symbol
    const IdentifierExpression* Expr(const Source& source, const Variable* variable) {
        return create<IdentifierExpression>(source, Ident(source, variable->name->symbol));
    }

    /// @param source the source information
    /// @param value the boolean value
    /// @return a Scalar constructor for the given value
    template <typename BOOL>
        requires(std::is_same_v<BOOL, bool>)
    const BoolLiteralExpression* Expr(const Source& source, BOOL value) {
        return create<BoolLiteralExpression>(source, value);
    }

    /// @param source the source information
    /// @param value the float value
    /// @return a 'f'-suffixed FloatLiteralExpression for the f32 value
    const FloatLiteralExpression* Expr(const Source& source, core::f32 value) {
        return create<FloatLiteralExpression>(source, static_cast<double>(value.value),
                                              FloatLiteralExpression::Suffix::kF);
    }

    /// @param source the source information
    /// @param value the float value
    /// @return a 'h'-suffixed FloatLiteralExpression for the f16 value
    const FloatLiteralExpression* Expr(const Source& source, core::f16 value) {
        return create<FloatLiteralExpression>(source, static_cast<double>(value.value),
                                              FloatLiteralExpression::Suffix::kH);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return an unsuffixed IntLiteralExpression for the AInt value
    const IntLiteralExpression* Expr(const Source& source, core::AInt value) {
        return create<IntLiteralExpression>(source, value, IntLiteralExpression::Suffix::kNone);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return an unsuffixed FloatLiteralExpression for the AFloat value
    const FloatLiteralExpression* Expr(const Source& source, core::AFloat value) {
        return create<FloatLiteralExpression>(source, value.value,
                                              FloatLiteralExpression::Suffix::kNone);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return a signed 'i'-suffixed IntLiteralExpression for the i32 value
    const IntLiteralExpression* Expr(const Source& source, core::i32 value) {
        return create<IntLiteralExpression>(source, value, IntLiteralExpression::Suffix::kI);
    }

    /// @param source the source information
    /// @param value the unsigned int value
    /// @return an unsigned 'u'-suffixed IntLiteralExpression for the u32 value
    const IntLiteralExpression* Expr(const Source& source, core::u32 value) {
        return create<IntLiteralExpression>(source, value, IntLiteralExpression::Suffix::kU);
    }

    /// @param value the scalar value
    /// @return literal expression of the appropriate type
    template <typename SCALAR, typename = EnableIfScalar<SCALAR>>
    const auto* Expr(SCALAR&& value) {
        return Expr(source_, std::forward<SCALAR>(value));
    }

    /// Converts `arg` to an `Expression` using `Expr()`, then appends it to
    /// `list`.
    /// @param list the list to append too
    /// @param arg the arg to create
    template <size_t N, typename ARG>
    void Append(Vector<const Expression*, N>& list, ARG&& arg) {
        list.Push(Expr(std::forward<ARG>(arg)));
    }

    /// Converts `arg0` and `args` to `Expression`s using `Expr()`,
    /// then appends them to `list`.
    /// @param list the list to append too
    /// @param arg0 the first argument
    /// @param args the rest of the arguments
    template <size_t N, typename ARG0, typename... ARGS>
    void Append(Vector<const Expression*, N>& list, ARG0&& arg0, ARGS&&... args) {
        Append(list, std::forward<ARG0>(arg0));
        Append(list, std::forward<ARGS>(args)...);
    }

    /// @return EmptyType
    EmptyType ExprList() { return Empty; }

    /// @param args the list of expressions
    /// @return the list of expressions converted to `Expression`s using
    /// `Expr()`,
    template <typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    auto ExprList(ARGS&&... args) {
        return Vector<const Expression*, sizeof...(ARGS)>{Expr(args)...};
    }

    /// @param list the list of expressions
    /// @return `list`
    template <typename T, size_t N>
    Vector<T, N> ExprList(Vector<T, N>&& list) {
        return std::move(list);
    }

    /// @param list the list of expressions
    /// @return `list`
    VectorRef<const Expression*> ExprList(VectorRef<const Expression*> list) { return list; }

    /// @param expr the expression for the bitcast
    /// @return a bitcast call of type `ty`, with the values of `expr` converted to
    /// `Expression`s using `Expr()`
    template <typename T, typename EXPR>
    const CallExpression* Bitcast(EXPR&& expr) {
        return Bitcast(ty.Of<T>(), std::forward<EXPR>(expr));
    }

    /// @param type the type to cast to
    /// @param expr the expression for the bitcast
    /// @return a bitcast call of @p type constructed with the values `expr`.
    template <typename EXPR>
    const CallExpression* Bitcast(Type type, EXPR&& expr) {
        return Bitcast(source_, type, Expr(std::forward<EXPR>(expr)));
    }

    /// @param source the source information
    /// @param type the type to cast to
    /// @param expr the expression for the bitcast
    /// @return a bitcast call of @p type constructed with the values `expr`.
    template <typename EXPR>
    const CallExpression* Bitcast(const Source& source, Type type, EXPR&& expr) {
        return Call(source, Ident(wgsl::BuiltinFn::kBitcast, type), Expr(std::forward<EXPR>(expr)));
    }

    /// @param type the vector type
    /// @param size the vector size
    /// @param args the arguments for the vector constructor
    /// @return an `CallExpression` of a `size`-element vector of
    /// type `type`, constructed with the values @p args.
    template <typename... ARGS>
    const CallExpression* vec(Type type, uint32_t size, ARGS&&... args) {
        return vec(source_, type, size, std::forward<ARGS>(args)...);
    }

    /// @param source the source of the call
    /// @param type the vector type
    /// @param size the vector size
    /// @param args the arguments for the vector constructor
    /// @return an `CallExpression` of a `size`-element vector of
    /// type `type`, constructed with the values @p args.
    template <typename... ARGS>
    const CallExpression* vec(const Source& source, Type type, uint32_t size, ARGS&&... args) {
        return Call(source, ty.vec(type, size), std::forward<ARGS>(args)...);
    }

    /// Adds the extension to the list of enable directives at the top of the module.
    /// @param extension the extension to enable
    /// @return an `Enable` enabling the given extension.
    const ast::Enable* Enable(wgsl::Extension extension) {
        auto* ext = create<Extension>(extension);
        auto* enable = create<ast::Enable>(Vector{ext});
        AST().AddEnable(enable);
        return enable;
    }

    /// Adds the extension to the list of enable directives at the top of the module.
    /// @param source the enable source
    /// @param extension the extension to enable
    /// @return an `Enable` enabling the given extension.
    const ast::Enable* Enable(const Source& source, wgsl::Extension extension) {
        auto* ext = create<Extension>(source, extension);
        auto* enable = create<ast::Enable>(source, Vector{ext});
        AST().AddEnable(enable);
        return enable;
    }

    /// Adds the language feature to the list of requires directives at the top of the module.
    /// @param feature the feature to require
    /// @return a `Requires` requiring the given language feature.
    const Requires* Require(wgsl::LanguageFeature feature) {
        auto* req = create<Requires>(Requires::LanguageFeatures({feature}));
        AST().AddRequires(req);
        return req;
    }

    /// Adds the language feature to the list of requires directives at the top of the module.
    /// @param source the requires source
    /// @param feature the feature to require
    /// @return a `Requires` requiring the given language feature.
    const Requires* Require(const Source& source, wgsl::LanguageFeature feature) {
        auto* req = create<Requires>(source, Requires::LanguageFeatures({feature}));
        AST().AddRequires(req);
        return req;
    }

    /// @param name the variable name
    /// @param options the extra options passed to the Var initializer
    /// Can be any of the following, in any order:
    ///   * Type              - specifies the variable's type
    ///   * core::AddressSpace  - specifies the variable's address space
    ///   * core::Access        - specifies the variable's access control
    ///   * Expression*       - specifies the variable's initializer expression
    ///   * Attribute*        - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a `Var` with the given name, type and additional
    /// options
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const Var* Var(NAME&& name, OPTIONS&&... options) {
        return Var(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the Var initializer
    /// Can be any of the following, in any order:
    ///   * Type              - specifies the variable's type
    ///   * core::AddressSpace  - specifies the variable's address space
    ///   * core::Access        - specifies the variable's access control
    ///   * Expression*       - specifies the variable's initializer expression
    ///   * Attribute*        - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a `Var` with the given name, address_space and type
    template <typename NAME, typename... OPTIONS>
    const ast::Var* Var(const Source& source, NAME&& name, OPTIONS&&... options) {
        VarOptions opts(*this, std::forward<OPTIONS>(options)...);
        return create<ast::Var>(source, Ident(std::forward<NAME>(name)), opts.type,
                                opts.address_space, opts.access, opts.initializer,
                                std::move(opts.attributes));
    }

    /// @param name the name
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* Const(std::string_view name, const ast::Expression* expr) {
        return Const(source_, name, expr);
    }

    /// @param source the source
    /// @param name the name
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* Const(const Source& source,
                            std::string_view name,
                            const ast::Expression* expr) {
        return Const(source, name, ast::Type{}, expr);
    }

    /// @param name the name
    /// @param type the type
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* Const(std::string_view name, Type type, const ast::Expression* expr) {
        return Const(source_, name, type, expr);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* Const(const Source& source,
                            std::string_view name,
                            Type type,
                            const ast::Expression* expr) {
        return Const(source, Ident(name), type, expr);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* Const(const Source& source,
                            const ast::Identifier* name,
                            Type type,
                            const ast::Expression* expr) {
        return create<ast::Const>(source, name, type, expr);
    }

    /// @param name the variable name
    /// @param type the variable's type
    /// @param init initializer expression (required)
    /// @returns an `Let` with the given name, type and initializer
    const ast::Let* Let(std::string_view name, Type type, const Expression* init) {
        return Let(source_, name, type, init);
    }

    /// @param name the variable name
    /// @param init initializer expression (required)
    /// @returns an `Let` with the given name, type and initializer
    const ast::Let* Let(std::string_view name, const Expression* init) {
        return Let(source_, name, ast::Type{}, init);
    }

    /// @param source the source location
    /// @param name the variable name
    /// @param type the variable's type
    /// @param init initializer expression (required)
    /// @returns an `Let` with the given name, type and initializer
    const ast::Let* Let(const Source& source, Symbol name, Type type, const Expression* init) {
        return Let(source, Ident(name), type, init);
    }

    /// @param name the variable name
    /// @param type the variable's type
    /// @param init initializer expression (required)
    /// @returns an `Let` with the given name, type and initializer
    const ast::Let* Let(Symbol name, Type type, const Expression* init) {
        return Let(source_, name, type, init);
    }

    /// @param source the source location
    /// @param name the variable name
    /// @param type the variable's type
    /// @param init initializer expression (required)
    /// @returns an `Let` with the given name, type and initializer
    const ast::Let* Let(const Source& source,
                        const ast::Identifier* name,
                        Type type,
                        const Expression* init) {
        return create<ast::Let>(source, name, type, init);
    }

    /// @param source the source location
    /// @param name the variable name
    /// @param type the variable's type
    /// @param init initializer expression (required)
    /// @returns an `Let` with the given name, type and initializer
    const ast::Let* Let(const Source& source, std::string_view name, const Expression* init) {
        return Let(source, name, ast::Type{}, init);
    }

    /// @param source the source location
    /// @param name the variable name
    /// @param type the variable's type
    /// @param init initializer expression (required)
    /// @returns an `Let` with the given name, type and initializer
    const ast::Let* Let(const Source& source,
                        std::string_view name,
                        Type type,
                        const Expression* init) {
        return create<ast::Let>(source, Ident(name), type, init);
    }

    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an `Parameter` with the given name and type
    template <typename NAME>
    const Parameter* Param(NAME&& name, Type type, VectorRef<const Attribute*> attributes = Empty) {
        return Param(source_, std::forward<NAME>(name), type, std::move(attributes));
    }

    /// @param source the parameter source
    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an `Parameter` with the given name and type
    template <typename NAME>
    const Parameter* Param(const Source& source,
                           NAME&& name,
                           Type type,
                           VectorRef<const Attribute*> attributes = Empty) {
        return create<Parameter>(source, Ident(std::forward<NAME>(name)), type,
                                 std::move(attributes));
    }

    /// @param name the variable name
    /// @param options the extra options passed to the Var initializer
    /// Can be any of the following, in any order:
    ///   * Type           - specifies the variable's type
    ///   * core::AddressSpace  - specifies the variable address space
    ///   * core::Access        - specifies the variable's access control
    ///   * Expression*    - specifies the variable's initializer expression
    ///   * Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a new `Var`, which is automatically registered as a global variable with the
    /// Module.
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Var* GlobalVar(NAME&& name, OPTIONS&&... options) {
        return GlobalVar(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the Var initializer
    /// Can be any of the following, in any order:
    ///   * Type           - specifies the variable's type
    ///   * core::AddressSpace  - specifies the variable address space
    ///   * core::Access        - specifies the variable's access control
    ///   * Expression*    - specifies the variable's initializer expression
    ///   * Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a new `Var`, which is automatically registered as a global variable with the
    /// Module.
    template <typename NAME, typename... OPTIONS>
    const ast::Var* GlobalVar(const Source& source, NAME&& name, OPTIONS&&... options) {
        auto* variable = Var(source, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param name the name
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* GlobalConst(std::string_view name, const ast::Expression* expr) {
        return GlobalConst(source_, name, ast::Type{}, expr);
    }

    /// @param name the name
    /// @param type the type
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* GlobalConst(std::string_view name, Type type, const ast::Expression* expr) {
        return GlobalConst(source_, name, type, expr);
    }

    /// @param source the source
    /// @param name the name
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* GlobalConst(const Source& source,
                                  std::string_view name,
                                  const ast::Expression* expr) {
        return GlobalConst(source, Ident(name), ast::Type{}, expr);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* GlobalConst(const Source& source,
                                  std::string_view name,
                                  Type type,
                                  const ast::Expression* expr) {
        return GlobalConst(source, Ident(name), type, expr);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* GlobalConst(const ast::Identifier* name,
                                  Type type,
                                  const ast::Expression* expr) {
        return GlobalConst(source_, name, type, expr);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param expr the initializer expression
    /// @returns an `Const` with the given name, and type
    const ast::Const* GlobalConst(const Source& source,
                                  const ast::Identifier* name,
                                  Type type,
                                  const ast::Expression* expr) {
        auto* variable = Const(source, name, type, expr);
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param name the name
    /// @param init the initializer
    /// @returns an `Override` with the given name
    const ast::Override* Override(std::string_view name, const Expression* init) {
        return Override(source_, name, init);
    }

    /// @param source the source
    /// @param name the name
    /// @param init the initializer
    /// @returns an `Override` with the given name
    const ast::Override* Override(const Source& source,
                                  std::string_view name,
                                  const Expression* init) {
        return Override(source, name, ast::Type{}, init);
    }

    /// @param name the name
    /// @param type the type
    /// @param init the initializer
    /// @param attrs the attributes
    /// @returns an `Override` with the given name
    const ast::Override* Override(std::string_view name,
                                  Type type,
                                  VectorRef<const ast::Attribute*> attrs) {
        return Override(source_, name, type, attrs);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param init the initializer
    /// @param attrs the attributes
    /// @returns an `Override` with the given name
    const ast::Override* Override(const Source& source,
                                  std::string_view name,
                                  Type type,
                                  VectorRef<const ast::Attribute*> attrs = {}) {
        return Override(source, name, type, nullptr, attrs);
    }

    /// @param name the name
    /// @param type the type
    /// @param init the initializer
    /// @param attrs the attributes
    /// @returns an `Override` with the given name
    const ast::Override* Override(Symbol name,
                                  Type type,
                                  const ast::Expression* init = nullptr,
                                  VectorRef<const ast::Attribute*> attrs = {}) {
        return Override(source_, Ident(name), type, init, attrs);
    }

    /// @param name the name
    /// @param type the type
    /// @param init the initializer
    /// @param attrs the attributes
    /// @returns an `Override` with the given name
    const ast::Override* Override(std::string_view name,
                                  Type type,
                                  const Expression* init = nullptr,
                                  VectorRef<const ast::Attribute*> attrs = {}) {
        return Override(source_, name, type, init, attrs);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param init the initializer
    /// @param attrs the attributes
    /// @returns an `Override` with the given name
    const ast::Override* Override(const Source& source,
                                  std::string_view name,
                                  Type type,
                                  const Expression* init,
                                  VectorRef<const ast::Attribute*> attrs = {}) {
        return Override(source, Ident(name), type, init, attrs);
    }

    /// @param source the source
    /// @param name the name
    /// @param type the type
    /// @param init the initializer
    /// @param attrs the attributes
    /// @returns an `Override` with the given name
    const ast::Override* Override(const Source& source,
                                  const ast::Identifier* name,
                                  Type type,
                                  const Expression* init,
                                  VectorRef<const ast::Attribute*> attrs) {
        auto* variable = create<ast::Override>(source, name, type, init, attrs);
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param source the source information
    /// @param condition the assertion condition
    /// @returns a new `ConstAssert`, which is automatically registered as a global statement
    /// with the Module.
    template <typename EXPR>
    const ast::ConstAssert* GlobalConstAssert(const Source& source, EXPR&& condition) {
        auto* sa = ConstAssert(source, std::forward<EXPR>(condition));
        AST().AddConstAssert(sa);
        return sa;
    }

    /// @param condition the assertion condition
    /// @returns a new `ConstAssert`, which is automatically registered as a global statement
    /// with the Module.
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ast::ConstAssert* GlobalConstAssert(EXPR&& condition) {
        auto* sa = ConstAssert(std::forward<EXPR>(condition));
        AST().AddConstAssert(sa);
        return sa;
    }

    /// @param source the source information
    /// @param condition the assertion condition
    /// @returns a new `ConstAssert` with the given assertion condition
    template <typename EXPR>
    const ast::ConstAssert* ConstAssert(const Source& source, EXPR&& condition) {
        return create<ast::ConstAssert>(source, Expr(std::forward<EXPR>(condition)));
    }

    /// @param condition the assertion condition
    /// @returns a new `ConstAssert` with the given assertion condition
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ast::ConstAssert* ConstAssert(EXPR&& condition) {
        return create<ast::ConstAssert>(Expr(std::forward<EXPR>(condition)));
    }

    /// @param source the source information
    /// @param expr the expression to take the address of
    /// @return an UnaryOpExpression that takes the address of `expr`
    template <typename EXPR>
    const UnaryOpExpression* AddressOf(const Source& source, EXPR&& expr) {
        return create<UnaryOpExpression>(source, core::UnaryOp::kAddressOf,
                                         Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to take the address of
    /// @return an UnaryOpExpression that takes the address of `expr`
    template <typename EXPR>
    const UnaryOpExpression* AddressOf(EXPR&& expr) {
        return create<UnaryOpExpression>(core::UnaryOp::kAddressOf, Expr(std::forward<EXPR>(expr)));
    }

    /// @param source the source information
    /// @param expr the expression to perform an indirection on
    /// @return an UnaryOpExpression that dereferences the pointer `expr`
    template <typename EXPR>
    const UnaryOpExpression* Deref(const Source& source, EXPR&& expr) {
        return create<UnaryOpExpression>(source, core::UnaryOp::kIndirection,
                                         Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to perform an indirection on
    /// @return an UnaryOpExpression that dereferences the pointer `expr`
    template <typename EXPR>
    const UnaryOpExpression* Deref(EXPR&& expr) {
        return create<UnaryOpExpression>(core::UnaryOp::kIndirection,
                                         Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to perform a unary not on
    /// @return an UnaryOpExpression that is the unary not of the input
    /// expression
    template <typename EXPR>
    const UnaryOpExpression* Not(EXPR&& expr) {
        return create<UnaryOpExpression>(core::UnaryOp::kNot, Expr(std::forward<EXPR>(expr)));
    }

    /// @param source the source information
    /// @param expr the expression to perform a unary not on
    /// @return an UnaryOpExpression that is the unary not of the input
    /// expression
    template <typename EXPR>
    const UnaryOpExpression* Not(const Source& source, EXPR&& expr) {
        return create<UnaryOpExpression>(source, core::UnaryOp::kNot,
                                         Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to perform a unary complement on
    /// @return an UnaryOpExpression that is the unary complement of the
    /// input expression
    template <typename EXPR>
    const UnaryOpExpression* Complement(EXPR&& expr) {
        return create<UnaryOpExpression>(core::UnaryOp::kComplement,
                                         Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to perform a unary negation on
    /// @return an UnaryOpExpression that is the unary negation of the
    /// input expression
    template <typename EXPR>
    const UnaryOpExpression* Negation(EXPR&& expr) {
        return create<UnaryOpExpression>(core::UnaryOp::kNegation, Expr(std::forward<EXPR>(expr)));
    }

    /// @param args the arguments for the constructor
    /// @returns an CallExpression to the type `T`, with the arguments of @p args converted to
    /// `Expression`s using Expr().
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const CallExpression* Call(ARGS&&... args) {
        return Call(source_, ty.Of<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the source of the call
    /// @param args the arguments for the constructor
    /// @returns an CallExpression to the type `T` with the arguments of @p args converted to
    /// `Expression`s using Expr().
    template <typename T, typename... ARGS>
    const CallExpression* Call(const Source& source, ARGS&&... args) {
        return Call(source, ty.Of<T>(), std::forward<ARGS>(args)...);
    }

    /// @param target the call target
    /// @param args the function call arguments
    /// @returns an CallExpression to the target @p target, with the arguments of @p args
    /// converted to `Expression`s using Expr().
    template <typename TARGET,
              typename... ARGS,
              typename = DisableIfSource<TARGET>,
              typename = DisableIfScalar<TARGET>>
    const CallExpression* Call(TARGET&& target, ARGS&&... args) {
        return Call(source_, Expr(std::forward<TARGET>(target)), std::forward<ARGS>(args)...);
    }

    /// @param source the source of the call
    /// @param target the call target
    /// @param args the function call arguments
    /// @returns an CallExpression to the target @p target, with the arguments of @p args
    /// converted to `Expression`s using Expr().
    template <typename TARGET, typename... ARGS, typename = DisableIfScalar<TARGET>>
    const CallExpression* Call(const Source& source, TARGET&& target, ARGS&&... args) {
        return create<CallExpression>(source, Expr(std::forward<TARGET>(target)),
                                      ExprList(std::forward<ARGS>(args)...));
    }

    /// @param source the source information
    /// @param call the call expression to wrap in a call statement
    /// @returns a `CallStatement` for the given call expression
    const CallStatement* CallStmt(const Source& source, const CallExpression* call) {
        return create<CallStatement>(source, call);
    }

    /// @param call the call expression to wrap in a call statement
    /// @returns a `CallStatement` for the given call expression
    const CallStatement* CallStmt(const CallExpression* call) {
        return create<CallStatement>(call);
    }

    /// @param source the source information
    /// @returns a `PhonyExpression`
    const PhonyExpression* Phony(const Source& source) { return create<PhonyExpression>(source); }

    /// @returns a `PhonyExpression`
    const PhonyExpression* Phony() { return create<PhonyExpression>(); }

    /// @param expr the expression to ignore
    /// @returns a `AssignmentStatement` that assigns 'expr' to the phony
    /// (underscore) variable.
    template <typename EXPR>
    const AssignmentStatement* Ignore(EXPR&& expr) {
        return create<AssignmentStatement>(Phony(), Expr(expr));
    }

    /// @param lhs the left hand argument to the addition operation
    /// @param rhs the right hand argument to the addition operation
    /// @returns a `BinaryExpression` summing the arguments `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Add(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kAdd, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the addition operation
    /// @param rhs the right hand argument to the addition operation
    /// @returns a `BinaryExpression` summing the arguments `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Add(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kAdd, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the and operation
    /// @param rhs the right hand argument to the and operation
    /// @returns a `BinaryExpression` bitwise anding `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* And(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kAnd, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the or operation
    /// @param rhs the right hand argument to the or operation
    /// @returns a `BinaryExpression` bitwise or-ing `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Or(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kOr, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the subtraction operation
    /// @param rhs the right hand argument to the subtraction operation
    /// @returns a `BinaryExpression` subtracting `rhs` from `lhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Sub(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kSubtract, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the multiplication operation
    /// @param rhs the right hand argument to the multiplication operation
    /// @returns a `BinaryExpression` multiplying `rhs` from `lhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Mul(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kMultiply, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the multiplication operation
    /// @param rhs the right hand argument to the multiplication operation
    /// @returns a `BinaryExpression` multiplying `rhs` from `lhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Mul(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kMultiply,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the division operation
    /// @param rhs the right hand argument to the division operation
    /// @returns a `BinaryExpression` dividing `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Div(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kDivide, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the division operation
    /// @param rhs the right hand argument to the division operation
    /// @returns a `BinaryExpression` dividing `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Div(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kDivide,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the modulo operation
    /// @param rhs the right hand argument to the modulo operation
    /// @returns a `BinaryExpression` applying modulo of `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Mod(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kModulo, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the bit shift right operation
    /// @param rhs the right hand argument to the bit shift right operation
    /// @returns a `BinaryExpression` bit shifting right `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Shr(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kShiftRight, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the bit shift right operation
    /// @param rhs the right hand argument to the bit shift right operation
    /// @returns a `BinaryExpression` bit shifting right `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Shr(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kShiftRight,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the bit shift left operation
    /// @param rhs the right hand argument to the bit shift left operation
    /// @returns a `BinaryExpression` bit shifting left `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Shl(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kShiftLeft, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the bit shift left operation
    /// @param rhs the right hand argument to the bit shift left operation
    /// @returns a `BinaryExpression` bit shifting left `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Shl(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kShiftLeft,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the xor operation
    /// @param rhs the right hand argument to the xor operation
    /// @returns a `BinaryExpression` bitwise xor-ing `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Xor(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kXor, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the logical and operation
    /// @param rhs the right hand argument to the logical and operation
    /// @returns a `BinaryExpression` of `lhs` && `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* LogicalAnd(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kLogicalAnd, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the logical and operation
    /// @param rhs the right hand argument to the logical and operation
    /// @returns a `BinaryExpression` of `lhs` && `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* LogicalAnd(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kLogicalAnd,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the logical or operation
    /// @param rhs the right hand argument to the logical or operation
    /// @returns a `BinaryExpression` of `lhs` || `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* LogicalOr(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kLogicalOr, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the logical or operation
    /// @param rhs the right hand argument to the logical or operation
    /// @returns a `BinaryExpression` of `lhs` || `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* LogicalOr(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kLogicalOr,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the greater than operation
    /// @param rhs the right hand argument to the greater than operation
    /// @returns a `BinaryExpression` of `lhs` > `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* GreaterThan(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kGreaterThan, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the greater than or equal operation
    /// @param rhs the right hand argument to the greater than or equal operation
    /// @returns a `BinaryExpression` of `lhs` >= `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* GreaterThanEqual(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kGreaterThanEqual,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the less than operation
    /// @param rhs the right hand argument to the less than operation
    /// @returns a `BinaryExpression` of `lhs` < `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* LessThan(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kLessThan, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the less than or equal operation
    /// @param rhs the right hand argument to the less than or equal operation
    /// @returns a `BinaryExpression` of `lhs` <= `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* LessThanEqual(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kLessThanEqual,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the equal expression
    /// @param rhs the right hand argument to the equal expression
    /// @returns a `BinaryExpression` comparing `lhs` equal to `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Equal(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kEqual, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the equal expression
    /// @param rhs the right hand argument to the equal expression
    /// @returns a `BinaryExpression` comparing `lhs` equal to `rhs`
    template <typename LHS, typename RHS>
    const BinaryExpression* Equal(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(source, core::BinaryOp::kEqual,
                                        Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the not-equal expression
    /// @param rhs the right hand argument to the not-equal expression
    /// @returns a `BinaryExpression` comparing `lhs` equal to `rhs` for
    ///          disequality
    template <typename LHS, typename RHS>
    const BinaryExpression* NotEqual(LHS&& lhs, RHS&& rhs) {
        return create<BinaryExpression>(core::BinaryOp::kNotEqual, Expr(std::forward<LHS>(lhs)),
                                        Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param object the object for the index accessor expression
    /// @param index the index argument for the index accessor expression
    /// @returns a `IndexAccessorExpression` that indexes @p object with @p index
    template <typename OBJECT, typename INDEX>
    const IndexAccessorExpression* IndexAccessor(const Source& source,
                                                 OBJECT&& object,
                                                 INDEX&& index) {
        return create<IndexAccessorExpression>(source, Expr(std::forward<OBJECT>(object)),
                                               Expr(std::forward<INDEX>(index)));
    }

    /// @param object the object for the index accessor expression
    /// @param index the index argument for the index accessor expression
    /// @returns a `IndexAccessorExpression` that indexes @p object with @p index
    template <typename OBJECT, typename INDEX>
    const IndexAccessorExpression* IndexAccessor(OBJECT&& object, INDEX&& index) {
        return create<IndexAccessorExpression>(Expr(std::forward<OBJECT>(object)),
                                               Expr(std::forward<INDEX>(index)));
    }

    /// @param source the source information
    /// @param object the object for the member accessor expression
    /// @param member the member argument for the member accessor expression
    /// @returns a `MemberAccessorExpression` that indexes @p object with @p member
    template <typename OBJECT, typename MEMBER>
    const MemberAccessorExpression* MemberAccessor(const Source& source,
                                                   OBJECT&& object,
                                                   MEMBER&& member) {
        static_assert(!traits::IsType<traits::PtrElTy<MEMBER>, TemplatedIdentifier>,
                      "it is currently invalid for a structure to hold a templated member");
        return create<MemberAccessorExpression>(source, Expr(std::forward<OBJECT>(object)),
                                                Ident(std::forward<MEMBER>(member)));
    }

    /// @param object the object for the member accessor expression
    /// @param member the member argument for the member accessor expression
    /// @returns a `MemberAccessorExpression` that indexes @p object with @p member
    template <typename OBJECT, typename MEMBER>
    const MemberAccessorExpression* MemberAccessor(OBJECT&& object, MEMBER&& member) {
        return MemberAccessor(source_, std::forward<OBJECT>(object), std::forward<MEMBER>(member));
    }

    /// Creates a StructMemberSizeAttribute
    /// @param source the source information
    /// @param val the size value
    /// @returns the size attribute pointer
    template <typename EXPR>
    const StructMemberSizeAttribute* MemberSize(const Source& source, EXPR&& val) {
        return create<StructMemberSizeAttribute>(source, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a StructMemberSizeAttribute
    /// @param val the size value
    /// @returns the size attribute pointer
    template <typename EXPR>
    const StructMemberSizeAttribute* MemberSize(EXPR&& val) {
        return create<StructMemberSizeAttribute>(source_, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a StructMemberAlignAttribute
    /// @param source the source information
    /// @param val the align value expression
    /// @returns the align attribute pointer
    template <typename EXPR>
    const StructMemberAlignAttribute* MemberAlign(const Source& source, EXPR&& val) {
        return create<StructMemberAlignAttribute>(source, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a StructMemberAlignAttribute
    /// @param val the align value expression
    /// @returns the align attribute pointer
    template <typename EXPR>
    const StructMemberAlignAttribute* MemberAlign(EXPR&& val) {
        return create<StructMemberAlignAttribute>(source_, Expr(std::forward<EXPR>(val)));
    }

    /// Creates the GroupAttribute
    /// @param value group attribute index expresion
    /// @returns the group attribute pointer
    template <typename EXPR>
    const GroupAttribute* Group(EXPR&& value) {
        return create<GroupAttribute>(Expr(std::forward<EXPR>(value)));
    }

    /// Creates the GroupAttribute
    /// @param source the source
    /// @param value group attribute index expression
    /// @returns the group attribute pointer
    template <typename EXPR>
    const GroupAttribute* Group(const Source& source, EXPR&& value) {
        return create<GroupAttribute>(source, Expr(std::forward<EXPR>(value)));
    }

    /// Creates the BindingAttribute
    /// @param value the binding index expression
    /// @returns the binding deocration pointer
    template <typename EXPR>
    const BindingAttribute* Binding(EXPR&& value) {
        return create<BindingAttribute>(Expr(std::forward<EXPR>(value)));
    }

    /// Creates the BindingAttribute
    /// @param source the source
    /// @param value the binding index expression
    /// @returns the binding deocration pointer
    template <typename EXPR>
    const BindingAttribute* Binding(const Source& source, EXPR&& value) {
        return create<BindingAttribute>(source, Expr(std::forward<EXPR>(value)));
    }

    /// Creates an Function and registers it with the Module.
    /// @param name the function name
    /// @param params the function parameters
    /// @param type the function return type
    /// @param body the function body. Can be an BlockStatement*, as Statement* which will
    /// be automatically placed into a block, or nullptr for a stub function.
    /// @param attributes the optional function attributes
    /// @param return_type_attributes the optional function return type attributes
    /// @returns the function pointer
    template <typename NAME, typename BODY = VectorRef<const Statement*>>
    const Function* Func(NAME&& name,
                         VectorRef<const Parameter*> params,
                         Type type,
                         BODY&& body,
                         VectorRef<const Attribute*> attributes = Empty,
                         VectorRef<const Attribute*> return_type_attributes = Empty) {
        return Func(source_, std::forward<NAME>(name), std::move(params), type,
                    std::forward<BODY>(body), std::move(attributes),
                    std::move(return_type_attributes));
    }

    /// Creates an Function and registers it with the Module.
    /// @param source the source information
    /// @param name the function name
    /// @param params the function parameters
    /// @param type the function return type
    /// @param body the function body. Can be an BlockStatement*, as Statement* which will
    /// be automatically placed into a block, or nullptr for a stub function.
    /// @param attributes the optional function attributes
    /// @param return_type_attributes the optional function return type attributes
    /// @returns the function pointer
    template <typename NAME, typename BODY = VectorRef<const Statement*>>
    const Function* Func(const Source& source,
                         NAME&& name,
                         VectorRef<const Parameter*> params,
                         Type type,
                         BODY&& body,
                         VectorRef<const Attribute*> attributes = Empty,
                         VectorRef<const Attribute*> return_type_attributes = Empty) {
        const BlockStatement* block = nullptr;
        using BODY_T = traits::PtrElTy<BODY>;
        if constexpr (traits::IsTypeOrDerived<BODY_T, BlockStatement> ||
                      std::is_same_v<BODY_T, std::nullptr_t>) {
            block = body;
        } else {
            block = Block(std::forward<BODY>(body));
        }
        auto* func =
            create<Function>(source, Ident(std::forward<NAME>(name)), std::move(params), type,
                             block, std::move(attributes), std::move(return_type_attributes));
        AST().AddFunction(func);
        return func;
    }

    /// Creates an BreakStatement
    /// @param source the source information
    /// @returns the break statement pointer
    const BreakStatement* Break(const Source& source) { return create<BreakStatement>(source); }

    /// Creates an BreakStatement
    /// @returns the break statement pointer
    const BreakStatement* Break() { return create<BreakStatement>(); }

    /// Creates a BreakIfStatement with input condition
    /// @param source the source information for the if statement
    /// @param condition the if statement condition expression
    /// @returns the break-if statement pointer
    template <typename CONDITION>
    const BreakIfStatement* BreakIf(const Source& source, CONDITION&& condition) {
        return create<BreakIfStatement>(source, Expr(std::forward<CONDITION>(condition)));
    }

    /// Creates a BreakIfStatement with input condition
    /// @param condition the if statement condition expression
    /// @returns the break-if statement pointer
    template <typename CONDITION>
    const BreakIfStatement* BreakIf(CONDITION&& condition) {
        return create<BreakIfStatement>(Expr(std::forward<CONDITION>(condition)));
    }

    /// Creates an ContinueStatement
    /// @param source the source information
    /// @returns the continue statement pointer
    const ContinueStatement* Continue(const Source& source) {
        return create<ContinueStatement>(source);
    }

    /// Creates an ContinueStatement
    /// @returns the continue statement pointer
    const ContinueStatement* Continue() { return create<ContinueStatement>(); }

    /// Creates an ReturnStatement with no return value
    /// @param source the source information
    /// @returns the return statement pointer
    const ReturnStatement* Return(const Source& source) { return create<ReturnStatement>(source); }

    /// Creates an ReturnStatement with no return value
    /// @returns the return statement pointer
    const ReturnStatement* Return() { return create<ReturnStatement>(); }

    /// Creates an ReturnStatement with the given return value
    /// @param source the source information
    /// @param val the return value
    /// @returns the return statement pointer
    template <typename EXPR>
    const ReturnStatement* Return(const Source& source, EXPR&& val) {
        return create<ReturnStatement>(source, Expr(std::forward<EXPR>(val)));
    }

    /// Creates an ReturnStatement with the given return value
    /// @param val the return value
    /// @returns the return statement pointer
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ReturnStatement* Return(EXPR&& val) {
        return create<ReturnStatement>(Expr(std::forward<EXPR>(val)));
    }

    /// Creates an DiscardStatement
    /// @param source the source information
    /// @returns the discard statement pointer
    const DiscardStatement* Discard(const Source& source) {
        return create<DiscardStatement>(source);
    }

    /// Creates an DiscardStatement
    /// @returns the discard statement pointer
    const DiscardStatement* Discard() { return create<DiscardStatement>(); }

    /// Creates a Alias registering it with the AST().TypeDecls().
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    const ast::Alias* Alias(std::string_view name, Type type) { return Alias(source_, name, type); }

    /// Creates a Alias registering it with the AST().TypeDecls().
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    const ast::Alias* Alias(Symbol name, Type type) { return Alias(source_, name, type); }

    /// Creates a Alias registering it with the AST().TypeDecls().
    /// @param source the source information
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    const ast::Alias* Alias(const Source& source, Symbol name, Type type) {
        auto out = ty.alias(source, Ident(name), type);
        AST().AddTypeDecl(out);
        return out;
    }

    /// Creates a Alias registering it with the AST().TypeDecls().
    /// @param source the source information
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    const ast::Alias* Alias(const Source& source, std::string_view name, Type type) {
        auto out = ty.alias(source, Ident(name), type);
        AST().AddTypeDecl(out);
        return out;
    }

    /// Creates a Struct registering it with the AST().TypeDecls().
    /// @param name the struct name
    /// @param members the struct members
    /// @param attributes the optional struct attributes
    /// @returns the struct type
    template <typename NAME>
    const Struct* Structure(NAME&& name,
                            VectorRef<const StructMember*> members,
                            VectorRef<const Attribute*> attributes = Empty) {
        return Structure(source_, std::forward<NAME>(name), std::move(members),
                         std::move(attributes));
    }

    /// Creates a Struct registering it with the AST().TypeDecls().
    /// @param source the source information
    /// @param name the struct name
    /// @param members the struct members
    /// @param attributes the optional struct attributes
    /// @returns the struct type
    template <typename NAME>
    const Struct* Structure(const Source& source,
                            NAME&& name,
                            VectorRef<const StructMember*> members,
                            VectorRef<const Attribute*> attributes = Empty) {
        auto* type = create<Struct>(source, Ident(std::forward<NAME>(name)), std::move(members),
                                    std::move(attributes));
        AST().AddTypeDecl(type);
        return type;
    }

    /// Creates a StructMember
    /// @param name the struct member name
    /// @param type the struct member type
    /// @param attributes the optional struct member attributes
    /// @returns the struct member pointer
    template <typename NAME, typename = DisableIfSource<NAME>>
    const StructMember* Member(NAME&& name,
                               Type type,
                               VectorRef<const Attribute*> attributes = Empty) {
        return Member(source_, std::forward<NAME>(name), type, std::move(attributes));
    }

    /// Creates a StructMember
    /// @param source the struct member source
    /// @param name the struct member name
    /// @param type the struct member type
    /// @param attributes the optional struct member attributes
    /// @returns the struct member pointer
    template <typename NAME>
    const StructMember* Member(const Source& source,
                               NAME&& name,
                               Type type,
                               VectorRef<const Attribute*> attributes = Empty) {
        return create<StructMember>(source, Ident(std::forward<NAME>(name)), type,
                                    std::move(attributes));
    }

    /// Creates a BlockStatement with input statements and attributes
    /// @param statements the statements of the block
    /// @param attributes the optional attributes of the block
    /// @returns the block statement pointer
    const BlockStatement* Block(VectorRef<const Statement*> statements,
                                VectorRef<const Attribute*> attributes = Empty) {
        return Block(source_, std::move(statements), std::move(attributes));
    }

    /// Creates a BlockStatement with input statements and attributes
    /// @param source the source information for the block
    /// @param statements the statements of the block
    /// @param attributes the optional attributes of the block
    /// @returns the block statement pointer
    const BlockStatement* Block(const Source& source,
                                VectorRef<const Statement*> statements,
                                VectorRef<const Attribute*> attributes = Empty) {
        return create<BlockStatement>(source, std::move(statements), std::move(attributes));
    }

    /// Creates a BlockStatement with a parameter list of input statements
    /// @param statements the optional statements of the block
    /// @returns the block statement pointer
    template <typename... STATEMENTS,
              typename = DisableIfSource<STATEMENTS...>,
              typename = DisableIfVectorLike<STATEMENTS...>>
    const BlockStatement* Block(STATEMENTS&&... statements) {
        return Block(source_, std::forward<STATEMENTS>(statements)...);
    }

    /// Creates a BlockStatement with a parameter list of input statements
    /// @param source the source information for the block
    /// @param statements the optional statements of the block
    /// @returns the block statement pointer
    template <typename... STATEMENTS, typename = DisableIfVectorLike<STATEMENTS...>>
    const BlockStatement* Block(const Source& source, STATEMENTS&&... statements) {
        return create<BlockStatement>(source,
                                      Vector<const Statement*, sizeof...(statements)>{
                                          std::forward<STATEMENTS>(statements)...,
                                      },
                                      Empty);
    }

    /// A wrapper type for the Else statement used to create If statements.
    struct ElseStmt {
        /// Default constructor - no else statement.
        ElseStmt() : stmt(nullptr) {}
        /// Constructor
        /// @param s The else statement
        explicit ElseStmt(const Statement* s) : stmt(s) {}
        /// The else statement, or nullptr.
        const Statement* stmt;
    };

    /// Creates a IfStatement with input condition, body, and optional
    /// else statement
    /// @param source the source information for the if statement
    /// @param condition the if statement condition expression
    /// @param body the if statement body
    /// @param else_stmt optional else statement
    /// @param attributes optional attributes
    /// @returns the if statement pointer
    template <typename CONDITION>
    const IfStatement* If(const Source& source,
                          CONDITION&& condition,
                          const BlockStatement* body,
                          const ElseStmt else_stmt = ElseStmt(),
                          VectorRef<const Attribute*> attributes = Empty) {
        return create<IfStatement>(source, Expr(std::forward<CONDITION>(condition)), body,
                                   else_stmt.stmt, std::move(attributes));
    }

    /// Creates a IfStatement with input condition, body, and optional
    /// else statement
    /// @param condition the if statement condition expression
    /// @param body the if statement body
    /// @param else_stmt optional else statement
    /// @param attributes optional attributes
    /// @returns the if statement pointer
    template <typename CONDITION>
    const IfStatement* If(CONDITION&& condition,
                          const BlockStatement* body,
                          const ElseStmt else_stmt = ElseStmt(),
                          VectorRef<const Attribute*> attributes = Empty) {
        return create<IfStatement>(Expr(std::forward<CONDITION>(condition)), body, else_stmt.stmt,
                                   std::move(attributes));
    }

    /// Creates an Else object.
    /// @param stmt else statement
    /// @returns the Else object
    ElseStmt Else(const Statement* stmt) { return ElseStmt(stmt); }

    /// Creates a AssignmentStatement with input lhs and rhs expressions
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @returns the assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const AssignmentStatement* Assign(const Source& source,
                                      LhsExpressionInit&& lhs,
                                      RhsExpressionInit&& rhs) {
        return create<AssignmentStatement>(source, Expr(std::forward<LhsExpressionInit>(lhs)),
                                           Expr(std::forward<RhsExpressionInit>(rhs)));
    }

    /// Creates a AssignmentStatement with input lhs and rhs expressions
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @returns the assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const AssignmentStatement* Assign(LhsExpressionInit&& lhs, RhsExpressionInit&& rhs) {
        return create<AssignmentStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                           Expr(std::forward<RhsExpressionInit>(rhs)));
    }

    /// Creates a CompoundAssignmentStatement with input lhs and rhs
    /// expressions, and a binary operator.
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @param op the binary operator
    /// @returns the compound assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const CompoundAssignmentStatement* CompoundAssign(const Source& source,
                                                      LhsExpressionInit&& lhs,
                                                      RhsExpressionInit&& rhs,
                                                      core::BinaryOp op) {
        return create<CompoundAssignmentStatement>(source,
                                                   Expr(std::forward<LhsExpressionInit>(lhs)),
                                                   Expr(std::forward<RhsExpressionInit>(rhs)), op);
    }

    /// Creates a CompoundAssignmentStatement with input lhs and rhs
    /// expressions, and a binary operator.
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @param op the binary operator
    /// @returns the compound assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const CompoundAssignmentStatement* CompoundAssign(LhsExpressionInit&& lhs,
                                                      RhsExpressionInit&& rhs,
                                                      core::BinaryOp op) {
        return create<CompoundAssignmentStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                                   Expr(std::forward<RhsExpressionInit>(rhs)), op);
    }

    /// Creates an IncrementDecrementStatement with input lhs.
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const IncrementDecrementStatement* Increment(const Source& source, LhsExpressionInit&& lhs) {
        return create<IncrementDecrementStatement>(
            source, Expr(std::forward<LhsExpressionInit>(lhs)), true);
    }

    /// Creates a IncrementDecrementStatement with input lhs.
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const IncrementDecrementStatement* Increment(LhsExpressionInit&& lhs) {
        return create<IncrementDecrementStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                                   true);
    }

    /// Creates an IncrementDecrementStatement with input lhs.
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const IncrementDecrementStatement* Decrement(const Source& source, LhsExpressionInit&& lhs) {
        return create<IncrementDecrementStatement>(
            source, Expr(std::forward<LhsExpressionInit>(lhs)), false);
    }

    /// Creates a IncrementDecrementStatement with input lhs.
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const IncrementDecrementStatement* Decrement(LhsExpressionInit&& lhs) {
        return create<IncrementDecrementStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                                   false);
    }

    /// Creates a LoopStatement with input body and optional continuing
    /// @param source the source information
    /// @param body the loop body
    /// @param continuing the optional continuing block
    /// @param attributes optional attributes
    /// @returns the loop statement pointer
    const LoopStatement* Loop(const Source& source,
                              const BlockStatement* body,
                              const BlockStatement* continuing = nullptr,
                              VectorRef<const Attribute*> attributes = Empty) {
        return create<LoopStatement>(source, body, continuing, std::move(attributes));
    }

    /// Creates a LoopStatement with input body and optional continuing
    /// @param body the loop body
    /// @param continuing the optional continuing block
    /// @param attributes optional attributes
    /// @returns the loop statement pointer
    const LoopStatement* Loop(const BlockStatement* body,
                              const BlockStatement* continuing = nullptr,
                              VectorRef<const Attribute*> attributes = Empty) {
        return create<LoopStatement>(body, continuing, std::move(attributes));
    }

    /// Creates a ForLoopStatement with input body and optional initializer, condition,
    /// continuing, and attributes.
    /// @param source the source information
    /// @param init the optional loop initializer
    /// @param cond the optional loop condition
    /// @param cont the optional loop continuing
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the for loop statement pointer
    template <typename COND>
    const ForLoopStatement* For(const Source& source,
                                const Statement* init,
                                COND&& cond,
                                const Statement* cont,
                                const BlockStatement* body,
                                VectorRef<const Attribute*> attributes = Empty) {
        return create<ForLoopStatement>(source, init, Expr(std::forward<COND>(cond)), cont, body,
                                        std::move(attributes));
    }

    /// Creates a ForLoopStatement with input body and optional initializer, condition,
    /// continuing, and attributes.
    /// @param init the optional loop initializer
    /// @param cond the optional loop condition
    /// @param cont the optional loop continuing
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the for loop statement pointer
    template <typename COND>
    const ForLoopStatement* For(const Statement* init,
                                COND&& cond,
                                const Statement* cont,
                                const BlockStatement* body,
                                VectorRef<const Attribute*> attributes = Empty) {
        return create<ForLoopStatement>(init, Expr(std::forward<COND>(cond)), cont, body,
                                        std::move(attributes));
    }

    /// Creates a WhileStatement with input body, condition, and optional attributes.
    /// @param source the source information
    /// @param cond the loop condition
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the while statement pointer
    template <typename COND>
    const WhileStatement* While(const Source& source,
                                COND&& cond,
                                const BlockStatement* body,
                                VectorRef<const Attribute*> attributes = Empty) {
        return create<WhileStatement>(source, Expr(std::forward<COND>(cond)), body,
                                      std::move(attributes));
    }

    /// Creates a WhileStatement with input body, condition, and optional attributes.
    /// @param cond the condition
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the while loop statement pointer
    template <typename COND>
    const WhileStatement* While(COND&& cond,
                                const BlockStatement* body,
                                VectorRef<const Attribute*> attributes = Empty) {
        return create<WhileStatement>(Expr(std::forward<COND>(cond)), body, std::move(attributes));
    }

    /// Creates a VariableDeclStatement for the input variable
    /// @param source the source information
    /// @param var the variable to wrap in a decl statement
    /// @returns the variable decl statement pointer
    const VariableDeclStatement* Decl(const Source& source, const Variable* var) {
        return create<VariableDeclStatement>(source, var);
    }

    /// Creates a VariableDeclStatement for the input variable
    /// @param var the variable to wrap in a decl statement
    /// @returns the variable decl statement pointer
    const VariableDeclStatement* Decl(const Variable* var) {
        return create<VariableDeclStatement>(var);
    }

    /// Creates a SwitchStatement with input expression and cases
    /// @param source the source information
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @returns the switch statement pointer
    template <typename ExpressionInit, typename... Cases, typename = DisableIfVectorLike<Cases...>>
    const SwitchStatement* Switch(const Source& source,
                                  ExpressionInit&& condition,
                                  Cases&&... cases) {
        return create<SwitchStatement>(
            source, Expr(std::forward<ExpressionInit>(condition)),
            Vector<const CaseStatement*, sizeof...(cases)>{std::forward<Cases>(cases)...}, Empty,
            Empty);
    }

    /// Creates a SwitchStatement with input expression and cases
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @returns the switch statement pointer
    template <typename ExpressionInit,
              typename... Cases,
              typename = DisableIfSource<ExpressionInit>,
              typename = DisableIfVectorLike<Cases...>>
    const SwitchStatement* Switch(ExpressionInit&& condition, Cases&&... cases) {
        return create<SwitchStatement>(
            Expr(std::forward<ExpressionInit>(condition)),
            Vector<const CaseStatement*, sizeof...(cases)>{std::forward<Cases>(cases)...}, Empty,
            Empty);
    }

    /// Creates a SwitchStatement with input expression, cases, and optional attributes
    /// @param source the source information
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @param stmt_attributes optional statement attributes
    /// @param body_attributes optional body attributes
    /// @returns the switch statement pointer
    template <typename ExpressionInit>
    const SwitchStatement* Switch(const Source& source,
                                  ExpressionInit&& condition,
                                  VectorRef<const CaseStatement*> cases,
                                  VectorRef<const Attribute*> stmt_attributes = Empty,
                                  VectorRef<const Attribute*> body_attributes = Empty) {
        return create<SwitchStatement>(source, Expr(std::forward<ExpressionInit>(condition)), cases,
                                       std::move(stmt_attributes), std::move(body_attributes));
    }

    /// Creates a SwitchStatement with input expression, cases, and optional attributes
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @param stmt_attributes optional statement attributes
    /// @param body_attributes optional body attributes
    /// @returns the switch statement pointer
    template <typename ExpressionInit, typename = DisableIfSource<ExpressionInit>>
    const SwitchStatement* Switch(ExpressionInit&& condition,
                                  VectorRef<const CaseStatement*> cases,
                                  VectorRef<const Attribute*> stmt_attributes = Empty,
                                  VectorRef<const Attribute*> body_attributes = Empty) {
        return create<SwitchStatement>(Expr(std::forward<ExpressionInit>(condition)), cases,
                                       std::move(stmt_attributes), std::move(body_attributes));
    }

    /// Creates a CaseStatement with input list of selectors, and body
    /// @param selectors list of selectors
    /// @param body the case body
    /// @returns the case statement pointer
    const CaseStatement* Case(VectorRef<const CaseSelector*> selectors,
                              const BlockStatement* body = nullptr) {
        return Case(source_, std::move(selectors), body);
    }

    /// Creates a CaseStatement with input list of selectors, and body
    /// @param source the source information
    /// @param selectors list of selectors
    /// @param body the case body
    /// @returns the case statement pointer
    const CaseStatement* Case(const Source& source,
                              VectorRef<const CaseSelector*> selectors,
                              const BlockStatement* body = nullptr) {
        return create<CaseStatement>(source, std::move(selectors), body ? body : Block());
    }

    /// Convenient overload that takes a single selector
    /// @param selector a single case selector
    /// @param body the case body
    /// @returns the case statement pointer
    const CaseStatement* Case(const CaseSelector* selector, const BlockStatement* body = nullptr) {
        return Case(Vector{selector}, body ? body : Block());
    }

    /// Convenience function that creates a 'default' CaseStatement
    /// @param body the case body
    /// @returns the case statement pointer
    const CaseStatement* DefaultCase(const BlockStatement* body = nullptr) {
        return DefaultCase(source_, body);
    }

    /// Convenience function that creates a 'default' CaseStatement
    /// @param source the source information
    /// @param body the case body
    /// @returns the case statement pointer
    const CaseStatement* DefaultCase(const Source& source, const BlockStatement* body = nullptr) {
        return Case(source, Vector{DefaultCaseSelector(source)}, body);
    }

    /// Convenience function that creates a case selector
    /// @param source the source information
    /// @param expr the selector expression
    /// @returns the selector pointer
    template <typename EXPR>
    const ast::CaseSelector* CaseSelector(const Source& source, EXPR&& expr) {
        return create<ast::CaseSelector>(source, Expr(std::forward<EXPR>(expr)));
    }

    /// Convenience function that creates a case selector
    /// @param expr the selector expression
    /// @returns the selector pointer
    template <typename EXPR>
    const ast::CaseSelector* CaseSelector(EXPR&& expr) {
        return create<ast::CaseSelector>(source_, Expr(std::forward<EXPR>(expr)));
    }

    /// Convenience function that creates a default case selector
    /// @param source the source information
    /// @returns the selector pointer
    const ast::CaseSelector* DefaultCaseSelector(const Source& source) {
        return create<ast::CaseSelector>(source, nullptr);
    }

    /// Convenience function that creates a default case selector
    /// @returns the selector pointer
    const ast::CaseSelector* DefaultCaseSelector() { return create<ast::CaseSelector>(nullptr); }

    /// Creates an BuiltinAttribute
    /// @param builtin the builtin value
    /// @returns the builtin attribute pointer
    const BuiltinAttribute* Builtin(core::BuiltinValue builtin) {
        return Builtin(source_, builtin);
    }

    /// Creates an BuiltinAttribute
    /// @param source the source information
    /// @param builtin the builtin value
    /// @returns the builtin attribute pointer
    const BuiltinAttribute* Builtin(const Source& source, core::BuiltinValue builtin) {
        return Builtin(source, builtin, core::BuiltinDepthMode::kUndefined);
    }

    /// Creates an BuiltinAttribute
    /// @param source the source information
    /// @param builtin the builtin value
    /// @param depth_mode the depth mode
    /// @returns the builtin attribute pointer
    const BuiltinAttribute* Builtin(const Source& source,
                                    core::BuiltinValue builtin,
                                    core::BuiltinDepthMode depth_mode) {
        return create<BuiltinAttribute>(source, builtin, depth_mode);
    }

    /// Creates an InterpolateAttribute
    /// @param type the interpolation type
    /// @returns the interpolate attribute pointer
    const InterpolateAttribute* Interpolate(core::InterpolationType type) {
        return Interpolate(source_, type);
    }

    /// Creates an InterpolateAttribute
    /// @param source the source information
    /// @param type the interpolation type
    /// @returns the interpolate attribute pointer
    const InterpolateAttribute* Interpolate(const Source& source, core::InterpolationType type) {
        return Interpolate(source, type, core::InterpolationSampling::kUndefined);
    }

    /// Creates an InterpolateAttribute
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the interpolate attribute pointer
    const InterpolateAttribute* Interpolate(core::InterpolationType type,
                                            core::InterpolationSampling sampling) {
        return Interpolate(source_, type, sampling);
    }

    /// Creates an InterpolateAttribute
    /// @param source the source information
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the interpolate attribute pointer
    const InterpolateAttribute* Interpolate(const Source& source,
                                            core::InterpolationType type,
                                            core::InterpolationSampling sampling) {
        core::Interpolation interpolation{type, sampling};
        return create<InterpolateAttribute>(source, interpolation);
    }

    /// Creates an InterpolateAttribute using flat interpolation
    /// @param source the source information
    /// @returns the interpolate attribute pointer
    const InterpolateAttribute* Flat(const Source& source) {
        return Interpolate(source, core::InterpolationType::kFlat);
    }

    /// Creates an InterpolateAttribute using flat interpolation
    /// @returns the interpolate attribute pointer
    const InterpolateAttribute* Flat() { return Interpolate(core::InterpolationType::kFlat); }

    /// Creates an InvariantAttribute
    /// @param source the source information
    /// @returns the invariant attribute pointer
    const InvariantAttribute* Invariant(const Source& source) {
        return create<InvariantAttribute>(source);
    }

    /// Creates an InvariantAttribute
    /// @returns the invariant attribute pointer
    const InvariantAttribute* Invariant() { return create<InvariantAttribute>(source_); }

    /// Creates an MustUseAttribute
    /// @param source the source information
    /// @returns the invariant attribute pointer
    const MustUseAttribute* MustUse(const Source& source) {
        return create<MustUseAttribute>(source);
    }

    /// Creates an MustUseAttribute
    /// @returns the invariant attribute pointer
    const MustUseAttribute* MustUse() { return create<MustUseAttribute>(source_); }

    /// Creates an LocationAttribute
    /// @param source the source information
    /// @param location the location value expression
    /// @returns the location attribute pointer
    template <typename EXPR>
    const LocationAttribute* Location(const Source& source, EXPR&& location) {
        return create<LocationAttribute>(source, Expr(std::forward<EXPR>(location)));
    }

    /// Creates an ColorAttribute
    /// @param index the index value expression
    /// @returns the index attribute pointer
    template <typename EXPR>
    const ColorAttribute* Color(EXPR&& index) {
        return create<ColorAttribute>(source_, Expr(std::forward<EXPR>(index)));
    }

    /// Creates an ColorAttribute
    /// @param source the source information
    /// @param index the index value expression
    /// @returns the index attribute pointer
    template <typename EXPR>
    const ColorAttribute* Color(const Source& source, EXPR&& index) {
        return create<ColorAttribute>(source, Expr(std::forward<EXPR>(index)));
    }

    /// Creates an LocationAttribute
    /// @param location the location value expression
    /// @returns the location attribute pointer
    template <typename EXPR>
    const LocationAttribute* Location(EXPR&& location) {
        return create<LocationAttribute>(source_, Expr(std::forward<EXPR>(location)));
    }

    /// Creates an BlendSrcAttribute
    /// @param source the source information
    /// @param blend_src the blend_src value expression
    /// @returns the blend_src attribute pointer
    template <typename EXPR>
    const BlendSrcAttribute* BlendSrc(const Source& source, EXPR&& blend_src) {
        return create<BlendSrcAttribute>(source, Expr(std::forward<EXPR>(blend_src)));
    }

    /// Creates an BlendSrcAttribute
    /// @param blend_src the blend_src value expression
    /// @returns the blend_src attribute pointer
    template <typename EXPR>
    const BlendSrcAttribute* BlendSrc(EXPR&& blend_src) {
        return create<BlendSrcAttribute>(source_, Expr(std::forward<EXPR>(blend_src)));
    }

    /// Creates an IdAttribute
    /// @param source the source information
    /// @param id the id value
    /// @returns the override attribute pointer
    const IdAttribute* Id(const Source& source, OverrideId id) {
        return create<IdAttribute>(source, Expr(core::AInt(id.value)));
    }

    /// Creates an IdAttribute with an override identifier
    /// @param id the optional id value
    /// @returns the override attribute pointer
    const IdAttribute* Id(OverrideId id) { return create<IdAttribute>(Expr(core::AInt(id.value))); }

    /// Creates an IdAttribute
    /// @param source the source information
    /// @param id the id value expression
    /// @returns the override attribute pointer
    template <typename EXPR>
    const IdAttribute* Id(const Source& source, EXPR&& id) {
        return create<IdAttribute>(source, Expr(std::forward<EXPR>(id)));
    }

    /// Creates an IdAttribute with an override identifier
    /// @param id the optional id value expression
    /// @returns the override attribute pointer
    template <typename EXPR>
    const IdAttribute* Id(EXPR&& id) {
        return create<IdAttribute>(Expr(std::forward<EXPR>(id)));
    }

    /// Creates an InputAttachmentIndexAttribute
    /// @param index the index value expression
    /// @returns the index attribute pointer
    template <typename EXPR>
    const InputAttachmentIndexAttribute* InputAttachmentIndex(EXPR&& index) {
        return create<InputAttachmentIndexAttribute>(source_, Expr(std::forward<EXPR>(index)));
    }

    /// Creates an InputAttachmentIndexAttribute
    /// @param source the source information
    /// @param index the index value expression
    /// @returns the index attribute pointer
    template <typename EXPR>
    const InputAttachmentIndexAttribute* InputAttachmentIndex(const Source& source, EXPR&& index) {
        return create<InputAttachmentIndexAttribute>(source, Expr(std::forward<EXPR>(index)));
    }

    /// Creates an StageAttribute
    /// @param source the source information
    /// @param stage the pipeline stage
    /// @returns the stage attribute pointer
    const StageAttribute* Stage(const Source& source, PipelineStage stage) {
        return create<StageAttribute>(source, stage);
    }

    /// Creates an StageAttribute
    /// @param stage the pipeline stage
    /// @returns the stage attribute pointer
    const StageAttribute* Stage(PipelineStage stage) {
        return create<StageAttribute>(source_, stage);
    }

    /// Creates an WorkgroupAttribute
    /// @param x the x dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X>
    const WorkgroupAttribute* WorkgroupSize(EXPR_X&& x) {
        return WorkgroupSize(std::forward<EXPR_X>(x), nullptr, nullptr);
    }

    /// Creates an WorkgroupAttribute
    /// @param source the source information
    /// @param x the x dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X>
    const WorkgroupAttribute* WorkgroupSize(const Source& source, EXPR_X&& x) {
        return WorkgroupSize(source, std::forward<EXPR_X>(x), nullptr, nullptr);
    }

    /// Creates an WorkgroupAttribute
    /// @param source the source information
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y>
    const WorkgroupAttribute* WorkgroupSize(const Source& source, EXPR_X&& x, EXPR_Y&& y) {
        return WorkgroupSize(source, std::forward<EXPR_X>(x), std::forward<EXPR_Y>(y), nullptr);
    }

    /// Creates an WorkgroupAttribute
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y, typename = DisableIfSource<EXPR_X>>
    const WorkgroupAttribute* WorkgroupSize(EXPR_X&& x, EXPR_Y&& y) {
        return WorkgroupSize(std::forward<EXPR_X>(x), std::forward<EXPR_Y>(y), nullptr);
    }

    /// Creates an WorkgroupAttribute
    /// @param source the source information
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @param z the z dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y, typename EXPR_Z>
    const WorkgroupAttribute* WorkgroupSize(const Source& source,
                                            EXPR_X&& x,
                                            EXPR_Y&& y,
                                            EXPR_Z&& z) {
        return create<WorkgroupAttribute>(source, Expr(std::forward<EXPR_X>(x)),
                                          Expr(std::forward<EXPR_Y>(y)),
                                          Expr(std::forward<EXPR_Z>(z)));
    }

    /// Creates an WorkgroupAttribute
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @param z the z dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y, typename EXPR_Z, typename = DisableIfSource<EXPR_X>>
    const WorkgroupAttribute* WorkgroupSize(EXPR_X&& x, EXPR_Y&& y, EXPR_Z&& z) {
        return create<WorkgroupAttribute>(source_, Expr(std::forward<EXPR_X>(x)),
                                          Expr(std::forward<EXPR_Y>(y)),
                                          Expr(std::forward<EXPR_Z>(z)));
    }

    /// Creates an SubgroupSizeAttribute
    /// @param source the source information
    /// @param subgroup_size the subgroup size value expression
    /// @returns the subgroup size attribute pointer
    template <typename EXPR>
    const SubgroupSizeAttribute* SubgroupSize(const Source& source, EXPR&& subgroup_size) {
        return create<SubgroupSizeAttribute>(source, std::forward<EXPR>(subgroup_size));
    }

    /// Creates an SubgroupSizeAttribute
    /// @param subgroup_size the subgroup size value expression
    /// @returns the subgroup size attribute pointer
    template <typename EXPR>
    const SubgroupSizeAttribute* SubgroupSize(EXPR&& subgroup_size) {
        return SubgroupSize(source_, Expr(std::forward<EXPR>(subgroup_size)));
    }

    /// Passthrough overload
    /// @param name the diagnostic rule name
    /// @returns @p name
    const ast::DiagnosticRuleName* DiagnosticRuleName(const ast::DiagnosticRuleName* name) {
        return name;
    }

    /// Creates an DiagnosticRuleName
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    const ast::DiagnosticRuleName* DiagnosticRuleName(const ast::Identifier* name) {
        TINT_ASSERT(!name->Is<TemplatedIdentifier>())
            << "it is invalid for a diagnostic rule name to be templated";
        auto* name_ident = Ident(name);
        return create<ast::DiagnosticRuleName>(name_ident->source, name_ident);
    }

    /// Creates an DiagnosticRuleName
    /// @param category the diagnostic rule category
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    const ast::DiagnosticRuleName* DiagnosticRuleName(const Identifier* category,
                                                      const Identifier* name) {
        TINT_ASSERT(!category->Is<TemplatedIdentifier>())
            << "it is invalid for a diagnostic rule category to be templated";
        TINT_ASSERT(!name->Is<TemplatedIdentifier>())
            << "it is invalid for a diagnostic rule name to be templated";

        auto* category_ident = Ident(category);
        auto* name_ident = Ident(name);
        Source source = category_ident->source;
        source.range.end = name_ident->source.range.end;
        return create<ast::DiagnosticRuleName>(source, category_ident, name_ident);
    }

    /// Creates an DiagnosticRuleName
    /// @param source the source information
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    const ast::DiagnosticRuleName* DiagnosticRuleName(const Source& source, std::string_view name) {
        auto* name_ident = Ident(name);
        return create<ast::DiagnosticRuleName>(source, name_ident);
    }

    /// Creates an ast::DiagnosticRuleName
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    const ast::DiagnosticRuleName* DiagnosticRuleName(std::string_view name) {
        return DiagnosticRuleName(source_, name);
    }

    /// Creates an ast::DiagnosticRuleName
    /// @param source the source information
    /// @param category the diagnostic rule category
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    const ast::DiagnosticRuleName* DiagnosticRuleName(const Source& source,
                                                      std::string_view category,
                                                      std::string_view name) {
        auto* category_ident = Ident(category);
        auto* name_ident = Ident(name);
        return create<ast::DiagnosticRuleName>(source, category_ident, name_ident);
    }

    /// Creates an ast::DiagnosticRuleName
    /// @param category the diagnostic rule category
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    const ast::DiagnosticRuleName* DiagnosticRuleName(std::string_view category,
                                                      std::string_view name) {
        return DiagnosticRuleName(source_, category, name);
    }

    /// Creates an ast::DiagnosticAttribute
    /// @param source the source information
    /// @param severity the diagnostic severity control
    /// @param rule the diagnostic rule
    /// @returns the diagnostic attribute pointer
    const ast::DiagnosticAttribute* DiagnosticAttribute(const Source& source,
                                                        wgsl::DiagnosticSeverity severity,
                                                        const ast::DiagnosticRuleName* rule) {
        return create<ast::DiagnosticAttribute>(source, ast::DiagnosticControl(severity, rule));
    }

    /// Creates an ast::DiagnosticAttribute
    /// @param severity the diagnostic severity control
    /// @param rule the diagnostic rule
    /// @returns the diagnostic attribute pointer
    const ast::DiagnosticAttribute* DiagnosticAttribute(wgsl::DiagnosticSeverity severity,
                                                        const ast::DiagnosticRuleName* rule) {
        return DiagnosticAttribute(source_, severity, rule);
    }

    /// Add a diagnostic directive to the module.
    /// @param source the source information
    /// @param severity the diagnostic severity control
    /// @param rule the diagnostic rule
    /// @returns the diagnostic directive pointer
    const ast::DiagnosticDirective* DiagnosticDirective(const Source& source,
                                                        wgsl::DiagnosticSeverity severity,
                                                        const ast::DiagnosticRuleName* rule) {
        auto* directive =
            create<ast::DiagnosticDirective>(source, ast::DiagnosticControl(severity, rule));
        AST().AddDiagnosticDirective(directive);
        return directive;
    }

    /// Add a diagnostic directive to the module.
    /// @param severity the diagnostic severity control
    /// @param rule the diagnostic rule
    /// @returns the diagnostic directive pointer
    const ast::DiagnosticDirective* DiagnosticDirective(wgsl::DiagnosticSeverity severity,
                                                        const ast::DiagnosticRuleName* rule) {
        return DiagnosticDirective(source_, severity, rule);
    }

    /// Sets the current builder source to `src`
    /// @param src the Source used for future create() calls
    void SetSource(const Source& src) {
        AssertNotMoved();
        source_ = src;
    }

    /// Sets the current builder source to `loc`
    /// @param loc the Source used for future create() calls
    void SetSource(const Source::Location& loc) {
        AssertNotMoved();
        source_ = Source(loc);
    }

    /// Wraps the Expression in a statement. This is used by tests that
    /// construct a partial AST and require the Resolver to reach these
    /// nodes.
    /// @param expr the Expression to be wrapped by an Statement
    /// @return the Statement that wraps the Expression
    const Statement* WrapInStatement(const Expression* expr);
    /// Wraps the Variable in a VariableDeclStatement. This is used by
    /// tests that construct a partial AST and require the Resolver to reach
    /// these nodes.
    /// @param v the Variable to be wrapped by an VariableDeclStatement
    /// @return the VariableDeclStatement that wraps the Variable
    const VariableDeclStatement* WrapInStatement(const Variable* v);
    /// Returns the statement argument. Used as a passthrough-overload by
    /// WrapInFunction().
    /// @param stmt the Statement
    /// @return `stmt`
    const Statement* WrapInStatement(const Statement* stmt);
    /// Wraps the list of arguments in a simple function so that each is reachable
    /// by the Resolver.
    /// @param args a mix of Expression, Statement, Variables.
    /// @returns the function
    template <typename... ARGS>
        requires(CanWrapInStatement<ARGS>::value && ...)
    const Function* WrapInFunction(ARGS&&... args) {
        Vector stmts{
            WrapInStatement(std::forward<ARGS>(args))...,
        };
        return WrapInFunction(std::move(stmts));
    }
    /// @param stmts a list of Statement that will be wrapped by a function,
    /// so that each statement is reachable by the Resolver.
    /// @returns the function
    const Function* WrapInFunction(VectorRef<const Statement*> stmts);

    /// The builder types
    TypesBuilder const ty{this};

  protected:
    /// Asserts that the builder has not been moved.
    virtual void AssertNotMoved() const;

    /// The last Node identifier
    NodeID last_ast_node_id_ = NodeID{static_cast<decltype(NodeID::value)>(0) - 1};

    /// Allocator for AST nodes
    ASTNodeAllocator ast_nodes_;

    /// The AST node module
    Module* ast_ = nullptr;

    /// The symbol table
    SymbolTable symbols_{};

    /// The diagnostic list
    diag::List diagnostics_;

    /// The source to use when creating AST nodes without providing a Source as
    /// the first argument.
    Source source_;

    /// Set by MarkAsMoved(). Once set, no methods may be called on this builder.
    bool moved_ = false;
};

//! @cond Doxygen_Suppress
// Various template specializations for Builder::TypesBuilder::CToAST.
template <>
struct Builder::TypesBuilder::CToAST<core::AInt> {
    static Type get(const Builder::TypesBuilder*) { return Type{}; }
};
template <>
struct Builder::TypesBuilder::CToAST<core::AFloat> {
    static Type get(const Builder::TypesBuilder*) { return Type{}; }
};
template <>
struct Builder::TypesBuilder::CToAST<core::i32> {
    static Type get(const Builder::TypesBuilder* t) { return t->i32(); }
};
template <>
struct Builder::TypesBuilder::CToAST<core::u32> {
    static Type get(const Builder::TypesBuilder* t) { return t->u32(); }
};
template <>
struct Builder::TypesBuilder::CToAST<core::f32> {
    static Type get(const Builder::TypesBuilder* t) { return t->f32(); }
};
template <>
struct Builder::TypesBuilder::CToAST<core::f16> {
    static Type get(const Builder::TypesBuilder* t) { return t->f16(); }
};
template <>
struct Builder::TypesBuilder::CToAST<bool> {
    static Type get(const Builder::TypesBuilder* t) { return t->bool_(); }
};
template <>
struct Builder::TypesBuilder::CToAST<core::i8> {
    static Type get(const Builder::TypesBuilder* t) { return t->i8(); }
};
template <>
struct Builder::TypesBuilder::CToAST<core::u8> {
    static Type get(const Builder::TypesBuilder* t) { return t->u8(); }
};
template <typename T, uint32_t N>
struct Builder::TypesBuilder::CToAST<core::fluent_types::array<T, N>> {
    static Type get(const Builder::TypesBuilder* t) { return t->array<T, N>(); }
};
template <typename T>
struct Builder::TypesBuilder::CToAST<core::fluent_types::atomic<T>> {
    static Type get(const Builder::TypesBuilder* t) { return t->atomic<T>(); }
};
template <uint32_t C, uint32_t R, typename T>
struct Builder::TypesBuilder::CToAST<core::fluent_types::mat<C, R, T>> {
    static Type get(const Builder::TypesBuilder* t) { return t->mat<T>(C, R); }
};
template <uint32_t N, typename T>
struct Builder::TypesBuilder::CToAST<core::fluent_types::vec<N, T>> {
    static Type get(const Builder::TypesBuilder* t) { return t->vec<T, N>(); }
};
template <core::AddressSpace ADDRESS, typename T, core::Access ACCESS>
struct Builder::TypesBuilder::CToAST<core::fluent_types::ptr<ADDRESS, T, ACCESS>> {
    static Type get(const Builder::TypesBuilder* t) { return t->ptr<ADDRESS, T, ACCESS>(); }
};
//! @endcond

// Primary template for metafunction that evaluates to true iff T can be wrapped in a statement.
template <typename T, typename /*  = void */>
struct CanWrapInStatement : std::false_type {};

// Specialization of CanWrapInStatement
template <typename T>
struct CanWrapInStatement<
    T,
    std::void_t<decltype(std::declval<Builder>().WrapInStatement(std::declval<T>()))>>
    : std::true_type {};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_BUILDER_H_
