// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_RESOLVER_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_RESOLVER_H_

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/constant/eval.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/subgroup_matrix_kind.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/wgsl/common/allowed_features.h"
#include "src/tint/lang/wgsl/intrinsic/dialect.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/dependency_graph.h"
#include "src/tint/lang/wgsl/resolver/sem_helper.h"
#include "src/tint/lang/wgsl/resolver/validator.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/utils/containers/bitset.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/text/styled_text.h"

// Forward declarations
namespace tint::ast {
class IndexAccessorExpression;
class BinaryExpression;
class BitcastExpression;
class CallExpression;
class CallStatement;
class CaseStatement;
class ForLoopStatement;
class Function;
class IdentifierExpression;
class LoopStatement;
class MemberAccessorExpression;
class ReturnStatement;
class SwitchStatement;
class UnaryOpExpression;
class Variable;
class WhileStatement;
}  // namespace tint::ast
namespace tint::sem {
class Array;
class BlockStatement;
class BuiltinFn;
class CaseStatement;
class ForLoopStatement;
class IfStatement;
class LoopStatement;
class Statement;
class StructMember;
class SwitchStatement;
class ValueConstructor;
class ValueConversion;
class WhileStatement;
}  // namespace tint::sem
namespace tint::core::type {
class Atomic;
}  // namespace tint::core::type

namespace tint::resolver {

/// Resolves types for all items in the given tint program
class Resolver {
  public:
    /// Constructor
    /// @param builder the program builder
    /// @param allowed_features the extensions and features that are allowed to be used
    Resolver(ProgramBuilder* builder, const wgsl::AllowedFeatures& allowed_features);

    /// Destructor
    ~Resolver();

    /// @returns error messages from the resolver
    std::string error() const { return diagnostics_.Str(); }

    /// @returns the list of diagnostics raised by the generator.
    const diag::List& Diagnostics() const { return diagnostics_; }

    /// @returns true if the resolver was successful
    bool Resolve();

    /// @param type the given type
    /// @returns true if the given type is a plain type
    bool IsPlain(const core::type::Type* type) const { return validator_.IsPlain(type); }

    /// @param type the given type
    /// @returns true if the given type is a fixed-footprint type
    bool IsFixedFootprint(const core::type::Type* type) const {
        return validator_.IsFixedFootprint(type);
    }

    /// @param type the given type
    /// @returns true if the given type is storable
    bool IsStorable(const core::type::Type* type) const { return validator_.IsStorable(type); }

    /// @param type the given type
    /// @returns true if the given type is host-shareable
    bool IsHostShareable(const core::type::Type* type) const {
        return validator_.IsHostShareable(type);
    }

    /// @returns the validator for testing
    const Validator* GetValidatorForTesting() const { return &validator_; }

  private:
    /// Resolves the program, without creating final the semantic nodes.
    /// @returns true on success, false on error
    bool ResolveInternal();

    /// Creates the nodes and adds them to the sem::Info mappings of the
    /// ProgramBuilder.
    void CreateSemanticNodes() const;

    /// @returns the call of Expression() cast to a sem::ValueExpression. If the sem::Expression is
    /// not a sem::ValueExpression, then an error diagnostic is raised and nullptr is returned.
    sem::ValueExpression* ValueExpression(const ast::Expression* expr);

    /// @returns the call of Expression() cast to a sem::TypeExpression. If the sem::Expression is
    /// not a sem::TypeExpression, then an error diagnostic is raised and nullptr is returned.
    sem::TypeExpression* TypeExpression(const ast::Expression* expr);

    /// @returns the call of Expression() cast to a sem::FunctionExpression. If the sem::Expression
    /// is not a sem::FunctionExpression, then an error diagnostic is raised and nullptr is
    /// returned.
    sem::FunctionExpression* FunctionExpression(const ast::Expression* expr);

    /// @returns the resolved type from an expression, or nullptr on error
    core::type::Type* Type(const ast::Expression* ast);

    /// @returns a new abstract-float
    core::type::AbstractFloat* AF();

    /// @returns a new f32
    core::type::F32* F32();

    /// @returns a new i32
    core::type::I32* I32();

    /// @returns a new u32
    core::type::U32* U32();

    /// @returns a new f16, if the f16 extension is enabled, otherwise nullptr
    core::type::F16* F16(const ast::Identifier* ident);

    /// @returns a vector with the element type @p el of width @p n resolved from the identifier @p
    /// ident.
    core::type::Vector* Vec(const ast::Identifier* ident, core::type::Type* el, uint32_t n);

    /// @returns a vector of width @p n resolved from the templated identifier @p ident, or an
    /// IncompleteType if the identifier is not templated.
    core::type::Type* VecT(const ast::Identifier* ident, core::BuiltinType builtin, uint32_t n);

    /// @returns a matrix with the element type @p el of dimensions @p num_columns x @p num_rows
    /// resolved from the identifier @p ident.
    core::type::Matrix* Mat(const ast::Identifier* ident,
                            core::type::Type* el,
                            uint32_t num_columns,
                            uint32_t num_rows);

    /// @returns a matrix of dimensions @p num_columns x @p num_rows resolved from the templated
    /// identifier @p ident, or an IncompleteType if the identifier is not templated.
    core::type::Type* MatT(const ast::Identifier* ident,
                           core::BuiltinType builtin,
                           uint32_t num_columns,
                           uint32_t num_rows);

    /// @returns an array resolved from the templated identifier @p ident, or an IncompleteType if
    /// the identifier is not templated.
    core::type::Type* Array(const ast::Identifier* ident);

    /// @returns a binding_array resolved from the templated identifier @p ident.
    core::type::BindingArray* BindingArray(const ast::Identifier* ident);

    /// @returns an atomic resolved from the templated identifier @p ident.
    core::type::Atomic* Atomic(const ast::Identifier* ident);

    /// @returns a pointer resolved from the templated identifier @p ident.
    core::type::Pointer* Ptr(const ast::Identifier* ident);

    /// @returns a sampled texture resolved from the templated identifier @p ident with the
    /// dimensions @p dim.
    core::type::SampledTexture* SampledTexture(const ast::Identifier* ident,
                                               core::type::TextureDimension dim);

    /// @returns a multisampled texture resolved from the templated identifier @p ident with the
    /// dimensions @p dim.
    core::type::MultisampledTexture* MultisampledTexture(const ast::Identifier* ident,
                                                         core::type::TextureDimension dim);

    /// @returns a storage texture resolved from the templated identifier @p ident with the
    /// dimensions @p dim.
    core::type::StorageTexture* StorageTexture(const ast::Identifier* ident,
                                               core::type::TextureDimension dim);

    /// @returns an input attachment resolved from the templated identifier @p ident
    core::type::InputAttachment* InputAttachment(const ast::Identifier* ident);

    /// @returns a subgroup matrix resolved from the templated identifier @p ident
    core::type::SubgroupMatrix* SubgroupMatrix(const ast::Identifier* ident,
                                               core::SubgroupMatrixKind kind);

    /// @returns @p ident cast to an ast::TemplatedIdentifier, if the identifier is templated and
    /// the number of templated arguments are between @p min_args and @p max_args.
    const ast::TemplatedIdentifier* TemplatedIdentifier(const ast::Identifier* ident,
                                                        size_t min_args,
                                                        size_t max_args = /* use min */ 0);

    /// @returns true if the number of templated arguments are between @p min_args and  @p max_args
    /// otherwise raises an error and returns false.
    bool CheckTemplatedIdentifierArgs(const ast::TemplatedIdentifier* ident,
                                      size_t min_args,
                                      size_t max_args = /* use min */ 0);

    /// @returns the call of Expression() cast to a
    /// sem::BuiltinEnumExpression<core::AddressSpace>. If the sem::Expression is not a
    /// sem::BuiltinEnumExpression<core::AddressSpace>, then an error diagnostic is raised and
    /// nullptr is returned.
    sem::BuiltinEnumExpression<core::AddressSpace>* AddressSpaceExpression(
        const ast::Expression* expr);

    /// @returns the call of Expression() cast to a
    /// sem::BuiltinEnumExpression<core::type::TexelFormat>. If the sem::Expression is not a
    /// sem::BuiltinEnumExpression<core::type::TexelFormat>, then an error diagnostic is raised and
    /// nullptr is returned.
    sem::BuiltinEnumExpression<core::TexelFormat>* TexelFormatExpression(
        const ast::Expression* expr);

    /// @returns the call of Expression() cast to a sem::BuiltinEnumExpression<core::Access>*.
    /// If the sem::Expression is not a sem::BuiltinEnumExpression<core::Access>*, then an error
    /// diagnostic is raised and nullptr is returned.
    sem::BuiltinEnumExpression<core::Access>* AccessExpression(const ast::Expression* expr);

    /// Expression traverses the graph of expressions starting at `expr`, building a post-ordered
    /// list (leaf-first) of all the expression nodes. Each of the expressions are then resolved by
    /// dispatching to the appropriate expression handlers below.
    /// @returns the resolved semantic node for the expression `expr`, or nullptr on failure.
    sem::Expression* Expression(const ast::Expression* expr);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Expression resolving methods
    //
    // Returns the semantic node pointer on success, nullptr on failure.
    //
    // These methods are invoked by Expression(), in postorder (child-first). These methods should
    // not attempt to resolve their children. This design avoids recursion, which is a common cause
    // of stack-overflows.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    sem::ValueExpression* IndexAccessor(const ast::IndexAccessorExpression*);
    sem::ValueExpression* Binary(const ast::BinaryExpression*);
    sem::Call* Call(const ast::CallExpression*);
    sem::Function* Function(const ast::Function*);
    sem::Call* FunctionCall(const ast::CallExpression*,
                            sem::Function* target,
                            VectorRef<const sem::ValueExpression*> args,
                            sem::Behaviors arg_behaviors);
    sem::Expression* Identifier(const ast::IdentifierExpression*);
    template <size_t N>
    sem::Call* BuiltinCall(const ast::CallExpression*,
                           wgsl::BuiltinFn,
                           Vector<const sem::ValueExpression*, N>& args);
    sem::ValueExpression* Literal(const ast::LiteralExpression*);
    sem::ValueExpression* MemberAccessor(const ast::MemberAccessorExpression*);
    sem::ValueExpression* UnaryOp(const ast::UnaryOpExpression*);

    /// Register a memory store to an expression, to track accesses to root identifiers in order to
    /// perform alias analysis.
    void RegisterStore(const sem::ValueExpression* expr);

    /// Register a memory load of an expression, to track accesses to root identifiers in order to
    /// perform alias analysis.
    void RegisterLoad(const sem::ValueExpression* expr);

    /// Perform pointer alias analysis for `call`.
    /// @returns true is the call arguments are free from aliasing issues, false otherwise.
    bool AliasAnalysis(const sem::Call* call);

    /// If `expr` is of a reference type, then Load will create and return a sem::Load node wrapping
    /// `expr`. If `expr` is not of a reference type, then Load will just return `expr`.
    const sem::ValueExpression* Load(const sem::ValueExpression* expr);

    /// If `expr` is not of an abstract-numeric type, then Materialize() will just return `expr`.
    /// * Materialize will create and return a sem::Materialize node wrapping `expr`.
    /// * The AST -> Sem binding will be updated to point to the new sem::Materialize node.
    /// * The sem::Materialize node will have a new concrete type, which will be `target_type` if
    ///   not nullptr, otherwise:
    ///     * a type with the element type of `i32` (e.g. `i32`, `vec2<i32>`) if `expr` has a
    ///       element type of abstract-integer...
    ///     * ... or a type with the element type of `f32` (e.g. `f32`, vec3<f32>`, `mat2x3<f32>`)
    ///       if `expr` has a element type of abstract-float.
    /// * The sem::Materialize constant value will be the value of `expr` value-converted to the
    ///   materialized type.
    /// If `expr` is not of an abstract-numeric type, then Materialize() will just return `expr`.
    /// If `expr` is nullptr, then Materialize() will also return nullptr.
    const sem::ValueExpression* Materialize(const sem::ValueExpression* expr,
                                            const core::type::Type* target_type = nullptr);

    /// For each argument in `args`:
    /// * Calls Materialize() passing the argument and the corresponding parameter type.
    /// * Calls Load() passing the argument, iff the corresponding parameter type is not a
    ///   reference type.
    /// @returns true on success, false on failure.
    template <size_t N>
    bool MaybeMaterializeAndLoadArguments(Vector<const sem::ValueExpression*, N>& args,
                                          const sem::CallTarget* target);

    /// @returns true if an argument of an abstract numeric type, passed to a parameter of type
    /// `parameter_ty` should be materialized.
    bool ShouldMaterializeArgument(const core::type::Type* parameter_ty) const;

    /// Converts `c` to `target_ty`
    /// @returns true on success, false on failure.
    bool Convert(const core::constant::Value*& c,
                 const core::type::Type* target_ty,
                 const Source& source);

    /// Transforms `args` to a vector of constants, and converts each constant to the call target's
    /// parameter type.
    /// @returns the vector of constants, `tint::Failure` on failure.
    template <size_t N>
    tint::Result<Vector<const core::constant::Value*, N>> ConvertArguments(
        const Vector<const sem::ValueExpression*, N>& args,
        const sem::CallTarget* target);

    /// Converts the value of `args`[`i`] to the type of the `i`th of the `target`.
    /// Assumes `i` is a valid argument index.  Returns nullptr if the value is not
    /// a constant, or can't be converted.
    /// @returns the possibly converted argument
    template <size_t N>
    const core::constant::Value* ConvertConstArgument(
        const Vector<const sem::ValueExpression*, N>& args,
        const sem::CallTarget* target,
        unsigned i);

    /// @param ty the type that may hold abstract numeric types
    /// @param target_ty the target type for the expression (variable type, parameter type, etc).
    ///        May be nullptr.
    /// @param source the source of the expression requiring materialization
    /// @returns the concrete (materialized) type for the given type, or nullptr if the type is
    ///          already concrete.
    const core::type::Type* ConcreteType(const core::type::Type* ty,
                                         const core::type::Type* target_ty,
                                         const Source& source);

    // Statement resolving methods
    // Each return true on success, false on failure.
    sem::Statement* AssignmentStatement(const ast::AssignmentStatement*);
    sem::BlockStatement* BlockStatement(const ast::BlockStatement*);
    sem::Statement* BreakStatement(const ast::BreakStatement*);
    sem::Statement* BreakIfStatement(const ast::BreakIfStatement*);
    sem::Statement* CallStatement(const ast::CallStatement*);
    sem::CaseStatement* CaseStatement(const ast::CaseStatement*, const core::type::Type*);
    sem::Statement* CompoundAssignmentStatement(const ast::CompoundAssignmentStatement*);
    sem::Statement* ContinueStatement(const ast::ContinueStatement*);
    sem::Statement* ConstAssert(const ast::ConstAssert*);
    sem::Statement* DiscardStatement(const ast::DiscardStatement*);
    sem::ForLoopStatement* ForLoopStatement(const ast::ForLoopStatement*);
    sem::WhileStatement* WhileStatement(const ast::WhileStatement*);
    sem::GlobalVariable* GlobalVariable(const ast::Variable*);
    sem::Statement* Parameter(const ast::Variable*);
    sem::IfStatement* IfStatement(const ast::IfStatement*);
    sem::Statement* IncrementDecrementStatement(const ast::IncrementDecrementStatement*);
    sem::LoopStatement* LoopStatement(const ast::LoopStatement*);
    sem::Statement* ReturnStatement(const ast::ReturnStatement*);
    sem::Statement* Statement(const ast::Statement*);
    sem::SwitchStatement* SwitchStatement(const ast::SwitchStatement* s);
    sem::Statement* VariableDeclStatement(const ast::VariableDeclStatement*);
    bool Statements(VectorRef<const ast::Statement*>);

    /// Resolves the WorkgroupSize for the given function, assigning it to
    /// current_function_
    bool WorkgroupSize(const ast::Function*);

    /// Resolves the `@location` attribute @p attr
    /// @returns the location value on success.
    tint::Result<uint32_t> LocationAttribute(const ast::LocationAttribute* attr);

    /// Resolves the `@color` attribute @p attr
    /// @returns the color value on success.
    tint::Result<uint32_t> ColorAttribute(const ast::ColorAttribute* attr);

    /// Resolves the `@blend_src` attribute @p attr
    /// @returns the blend_src value on success.
    tint::Result<uint32_t> BlendSrcAttribute(const ast::BlendSrcAttribute* attr);

    /// Resolves the `@binding` attribute @p attr
    /// @returns the binding value on success.
    tint::Result<uint32_t> BindingAttribute(const ast::BindingAttribute* attr);

    /// Resolves the `@group` attribute @p attr
    /// @returns the group value on success.
    tint::Result<uint32_t> GroupAttribute(const ast::GroupAttribute* attr);

    /// Resolves the `@input_attachment_index` attribute @p attr
    /// @returns the index value on success.
    tint::Result<uint32_t> InputAttachmentIndexAttribute(
        const ast::InputAttachmentIndexAttribute* attr);

    /// Resolves the `@workgroup_size` attribute @p attr
    /// @returns the workgroup size on success.
    tint::Result<sem::WorkgroupSize> WorkgroupAttribute(const ast::WorkgroupAttribute* attr);

    /// Resolves the `@diagnostic` attribute @p attr
    /// @returns true on success, false on failure
    bool DiagnosticAttribute(const ast::DiagnosticAttribute* attr);

    /// Resolves the stage attribute @p attr
    /// @returns true on success, false on failure
    bool StageAttribute(const ast::StageAttribute* attr);

    /// Resolves the `@must_use` attribute @p attr
    /// @returns true on success, false on failure
    bool MustUseAttribute(const ast::MustUseAttribute* attr);

    /// Resolves the `@invariant` attribute @p attr
    /// @returns true on success, false on failure
    bool InvariantAttribute(const ast::InvariantAttribute*);

    /// Resolves the `@stride` attribute @p attr
    /// @returns true on success, false on failure
    bool StrideAttribute(const ast::StrideAttribute*);

    /// Resolves the internal attribute @p attr
    /// @returns true on success, false on failure
    bool InternalAttribute(const ast::InternalAttribute* attr);

    /// @param control the diagnostic control
    /// @returns true on success, false on failure
    bool DiagnosticControl(const ast::DiagnosticControl& control);

    /// @param enable the enable declaration
    /// @returns true on success, false on failure
    bool Enable(const ast::Enable* enable);

    /// @param req the requires declaration
    /// @returns true on success, false on failure
    bool Requires(const ast::Requires* req);

    /// @param named_type the named type to resolve
    /// @returns the resolved semantic type
    core::type::Type* TypeDecl(const ast::TypeDecl* named_type);

    /// Resolves and validates the expression used as the count parameter of an array.
    /// @param count_expr the expression used as the second template parameter to an array<>.
    /// @returns the number of elements in the array.
    const core::type::ArrayCount* ArrayCount(const ast::Expression* count_expr);

    /// Resolves and validates the attributes on an array.
    /// @param attributes the attributes on the array type.
    /// @param el_ty the element type of the array.
    /// @param explicit_stride assigned the specified stride of the array in bytes.
    /// @returns true on success, false on failure
    bool ArrayAttributes(VectorRef<const ast::Attribute*> attributes,
                         const core::type::Type* el_ty,
                         uint32_t& explicit_stride);

    /// Builds and returns the semantic information for an array.
    /// @returns the semantic Array information, or nullptr if an error is raised.
    /// @param array_source the source of the array
    /// @param el_source the source of the array element, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @param count_source the source of the array count, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @param el_ty the Array element type
    /// @param el_count the number of elements in the array.
    /// @param explicit_stride the explicit byte stride of the array. Zero means implicit stride.
    sem::Array* Array(const Source& array_source,
                      const Source& el_source,
                      const Source& count_source,
                      const core::type::Type* el_ty,
                      const core::type::ArrayCount* el_count,
                      uint32_t explicit_stride);

    /// Builds and returns the semantic information for the alias `alias`.
    /// This method does not mark the ast::Alias node, nor attach the generated
    /// semantic information to the AST node.
    /// @returns the aliased type, or nullptr if an error is raised.
    core::type::Type* Alias(const ast::Alias* alias);

    /// Builds and returns the semantic information for the structure `str`.
    /// This method does not mark the ast::Struct node, nor attach the generated
    /// semantic information to the AST node.
    /// @returns the semantic Struct information, or nullptr if an error is
    /// raised.
    sem::Struct* Structure(const ast::Struct* str);

    /// @returns the semantic info for the variable `v`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param var the variable
    /// @param is_global true if this is module scope, otherwise function scope
    sem::Variable* Variable(const ast::Variable* var, bool is_global);

    /// @returns the semantic info for the `ast::Let` `v`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param var the variable
    sem::Variable* Let(const ast::Let* var);

    /// @returns the semantic info for the module-scope `ast::Override` `v`. If an error is raised,
    /// nullptr is returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param override the variable
    sem::Variable* Override(const ast::Override* override);

    /// @returns the semantic info for an `ast::Const` `v`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param const_ the variable
    /// @param is_global true if this is module scope, otherwise function scope
    sem::Variable* Const(const ast::Const* const_, bool is_global);

    /// @returns the semantic info for the `ast::Var` `var`. If an error is raised, nullptr is
    /// returned.
    /// @note this method does not resolve the attributes as these are context-dependent (global,
    /// local)
    /// @param var the variable
    /// @param is_global true if this is module scope, otherwise function scope
    sem::Variable* Var(const ast::Var* var, bool is_global);

    /// @returns the semantic info for the function parameter `param`. If an error is raised,
    /// nullptr is returned.
    /// @note the caller is expected to validate the parameter
    /// @param param the AST parameter
    /// @param func the AST function that owns the parameter
    /// @param index the index of the parameter
    sem::Parameter* Parameter(const ast::Parameter* param,
                              const ast::Function* func,
                              uint32_t index);

    /// Records the address space usage for the given type, and any transient
    /// dependencies of the type. Validates that the type can be used for the
    /// given address space, erroring if it cannot.
    /// @param sc the address space to apply to the type and transitent types
    /// @param ty the type to apply the address space on
    /// @param usage the Source of the root variable declaration that uses the
    /// given type and address space. Used for generating sensible error
    /// messages.
    /// @returns true on success, false on error
    bool ApplyAddressSpaceUsageToType(core::AddressSpace sc,
                                      core::type::Type* ty,
                                      const Source& usage);

    /// @param address_space the address space
    /// @returns the default access control for the given address space
    core::Access DefaultAccessForAddressSpace(core::AddressSpace address_space);

    /// Allocate constant IDs for pipeline-overridable constants.
    /// @returns true on success, false on error
    bool AllocateOverridableConstantIds();

    /// Set the shadowing information on variable declarations.
    /// @note this method must only be called after all semantic nodes are built.
    void SetShadows();

    /// StatementScope() does the following:
    /// * Creates the AST -> SEM mapping.
    /// * Assigns `sem` to #current_statement_
    /// * Assigns `sem` to #current_compound_statement_ if `sem` derives from
    ///   sem::CompoundStatement.
    /// * Then calls `callback`.
    /// * Before returning #current_statement_ and #current_compound_statement_ are restored to
    /// their original values.
    /// @returns `sem` if `callback` returns true, otherwise `nullptr`.
    template <typename SEM, typename F>
    SEM* StatementScope(const ast::Statement* ast, SEM* sem, F&& callback);

    /// Mark records that the given AST node has been visited, and asserts that
    /// the given node has not already been seen. Diamonds in the AST are
    /// illegal.
    /// @param node the AST node.
    /// @returns true on success, false on error
    bool Mark(const ast::Node* node);

    /// Applies the diagnostic severities from the current scope to a semantic node.
    /// @param node the semantic node to apply the diagnostic severities to
    template <typename NODE>
    void ApplyDiagnosticSeverities(NODE* node);

    /// Checks @p ident is not an ast::TemplatedIdentifier.
    /// If @p ident is a ast::TemplatedIdentifier, then an error diagnostic is raised.
    /// @returns true if @p ident is not a ast::TemplatedIdentifier.
    bool CheckNotTemplated(const char* use, const ast::Identifier* ident);

    /// Raises an error that the attribute is not valid for the given use.
    /// @param attr the invalue attribute
    /// @param use the thing that the attribute was applied to
    void ErrorInvalidAttribute(const ast::Attribute* attr, StyledText use);

    /// @returns a new error message added to the program's diagnostics
    diag::Diagnostic& AddError(const Source& source) const;

    /// @returns a new warning message added to the program's diagnostics
    diag::Diagnostic& AddWarning(const Source& source) const;

    /// @returns a new note message added to the program's diagnostics
    diag::Diagnostic& AddNote(const Source& source) const;

    /// @returns the core::type::Type for the builtin type @p builtin_ty with the identifier @p
    /// ident
    /// @note: Will raise an ICE if @p symbol is not a builtin type.
    core::type::Type* BuiltinType(core::BuiltinType builtin_ty, const ast::Identifier* ident);

    /// @returns the nesting depth of @ty as defined in
    /// https://gpuweb.github.io/gpuweb/wgsl/#composite-types
    size_t NestDepth(const core::type::Type* ty) const;

    // ArrayConstructorSig represents a unique array constructor signature.
    // It is a tuple of the array type, number of arguments provided and earliest evaluation stage.
    using ArrayConstructorSig =
        tint::UnorderedKeyWrapper<std::tuple<const sem::Array*, size_t, core::EvaluationStage>>;

    // StructConstructorSig represents a unique structure constructor signature.
    // It is a tuple of the structure type, number of arguments provided and earliest evaluation
    // stage.
    using StructConstructorSig = tint::UnorderedKeyWrapper<
        std::tuple<const core::type::Struct*, size_t, core::EvaluationStage>>;

    // SubgroupMatrixConstructorSig represents a unique subgroup matrix constructor signature.
    // It is a tuple of the subgroup matrix type and the number of arguments provided.
    using SubgroupMatrixConstructorSig =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::SubgroupMatrix*, size_t>>;

    /// ExprEvalStageConstraint describes a constraint on when expressions can be evaluated.
    struct ExprEvalStageConstraint {
        /// The latest stage that the expression can be evaluated
        core::EvaluationStage stage = core::EvaluationStage::kRuntime;
        /// The 'thing' that is imposing the contraint. e.g. "var declaration"
        /// If nullptr, then there is no constraint
        const char* constraint = nullptr;
    };

    /// AliasAnalysisInfo captures the memory accesses performed by a given function for the purpose
    /// of determining if any two arguments alias at any callsite.
    struct AliasAnalysisInfo {
        /// The set of module-scope variables that are written to, and where that write occurs.
        Hashmap<const sem::Variable*, const sem::ValueExpression*, 4> module_scope_writes;
        /// The set of module-scope variables that are read from, and where that read occurs.
        Hashmap<const sem::Variable*, const sem::ValueExpression*, 4> module_scope_reads;
        /// The set of function parameters that are written to.
        Hashset<const sem::Variable*, 4> parameter_writes;
        /// The set of function parameters that are read from.
        Hashset<const sem::Variable*, 4> parameter_reads;
    };

    ProgramBuilder& b;
    diag::List& diagnostics_;
    core::constant::Eval const_eval_;
    core::intrinsic::Table<wgsl::intrinsic::Dialect> intrinsic_table_;
    DependencyGraph dependencies_;
    SemHelper sem_;
    Validator validator_;
    wgsl::AllowedFeatures allowed_features_;
    wgsl::Extensions enabled_extensions_;
    Vector<sem::Function*, 8> entry_points_;
    Hashmap<const core::type::Type*, const Source*, 8> atomic_composite_info_;
    Hashset<const core::type::Type*, 8> subgroup_matrix_uses_;
    tint::Bitset<0> marked_;
    ExprEvalStageConstraint expr_eval_stage_constraint_;
    std::unordered_map<const sem::Function*, AliasAnalysisInfo> alias_analysis_infos_;
    Hashmap<OverrideId, const sem::Variable*, 8> override_ids_;
    Hashmap<ArrayConstructorSig, sem::CallTarget*, 8> array_ctors_;
    Hashmap<StructConstructorSig, sem::CallTarget*, 8> struct_ctors_;
    Hashmap<SubgroupMatrixConstructorSig, sem::CallTarget*, 8> subgroup_matrix_ctors_;
    sem::Function* current_function_ = nullptr;
    sem::Statement* current_statement_ = nullptr;
    sem::CompoundStatement* current_compound_statement_ = nullptr;
    Vector<std::function<void(const sem::GlobalVariable*)>, 4> on_transitively_reference_global_;
    uint32_t current_scoping_depth_ = 0;
    Hashset<TypeAndAddressSpace, 8> valid_type_storage_layouts_;
    Hashmap<const ast::Expression*, const ast::BinaryExpression*, 8> logical_binary_lhs_to_parent_;
    Hashset<const ast::Expression*, 8> not_evaluated_;
    Hashmap<const core::type::Type*, size_t, 8> nest_depth_;
    Hashmap<std::pair<core::intrinsic::Overload, wgsl::BuiltinFn>, sem::BuiltinFn*, 64> builtins_;
    Hashmap<core::intrinsic::Overload, sem::ValueConstructor*, 16> constructors_;
    Hashmap<core::intrinsic::Overload, sem::ValueConversion*, 16> converters_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_RESOLVER_H_
