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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_VALIDATOR_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_VALIDATOR_H_

#include <cstdint>
#include <set>
#include <string>
#include <utility>

#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/wgsl/allowed_features.h"
#include "src/tint/lang/wgsl/ast/input_attachment_index_attribute.h"
#include "src/tint/lang/wgsl/ast/pipeline_stage.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/sem_helper.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/scope_stack.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/diagnostic/source.h"
#include "src/tint/utils/math/hash.h"
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
class BreakIfStatement;
class BuiltinFn;
class Call;
class CaseStatement;
class ForLoopStatement;
class IfStatement;
class LoopStatement;
class Materialize;
class Statement;
class SwitchStatement;
class WhileStatement;
}  // namespace tint::sem
namespace tint::core::type {
class Atomic;
}  // namespace tint::core::type

namespace tint::resolver {

/// TypeAndAddressSpace is a pair of type and address space
struct TypeAndAddressSpace {
    /// The type
    const core::type::Type* type;
    /// The address space
    core::AddressSpace address_space;

    /// Equality operator
    /// @param other the other TypeAndAddressSpace to compare this TypeAndAddressSpace to
    /// @returns true if the type and address space of this TypeAndAddressSpace is equal to @p other
    bool operator==(const TypeAndAddressSpace& other) const {
        return type == other.type && address_space == other.address_space;
    }

    /// @returns the hash value of this object
    tint::HashCode HashCode() const { return Hash(type, address_space); }
};

/// DiagnosticFilterStack is a scoped stack of diagnostic filters.
using DiagnosticFilterStack = ScopeStack<wgsl::DiagnosticRule, wgsl::DiagnosticSeverity>;

/// Enumerator of duplication behavior for diagnostics.
enum class DiagnosticDuplicates : uint8_t {
    // Diagnostic duplicates are allowed.
    kAllowed,
    // Diagnostic duplicates are not allowed.
    kDenied,
};

/// Validation logic for various ast nodes. The validations in general should
/// be shallow and depend on the resolver to call on children. The validations
/// also assume that sem changes have already been made. The validation checks
/// should not alter the AST or SEM trees.
class Validator {
  public:
    /// Constructor
    /// @param builder the program builder
    /// @param helper the SEM helper to validate with
    /// @param enabled_extensions all the extensions declared in current module
    /// @param allowed_features the allowed extensions and features
    /// @param atomic_composite_info atomic composite info of the module
    /// @param valid_type_storage_layouts a set of validated type layouts by address space
    Validator(ProgramBuilder* builder,
              SemHelper& helper,
              const wgsl::Extensions& enabled_extensions,
              const wgsl::AllowedFeatures& allowed_features,
              const Hashmap<const core::type::Type*, const Source*, 8>& atomic_composite_info,
              Hashset<TypeAndAddressSpace, 8>& valid_type_storage_layouts);
    ~Validator();

    /// @returns an error diagnostic
    /// @param source the error source
    diag::Diagnostic& AddError(const Source& source) const;

    /// @returns an warning diagnostic
    /// @param source the warning source
    diag::Diagnostic& AddWarning(const Source& source) const;

    /// @returns an note diagnostic
    /// @param source the note source
    diag::Diagnostic& AddNote(const Source& source) const;

    /// Adds a diagnostic with current severity for the given rule.
    /// @param rule the diagnostic trigger rule
    /// @param source the diagnostic source
    /// @returns the diagnostic, if the diagnostic level isn't disabled
    diag::Diagnostic* MaybeAddDiagnostic(wgsl::DiagnosticRule rule, const Source& source) const;

    /// @returns the diagnostic filter stack
    DiagnosticFilterStack& DiagnosticFilters() { return diagnostic_filters_; }

    /// @param type the given type
    /// @returns true if the given type is a plain type
    bool IsPlain(const core::type::Type* type) const;

    /// @param type the given type
    /// @returns true if the given type is storable
    bool IsStorable(const core::type::Type* type) const;

    /// Validates the enabled extensions
    /// @param enables the extension enables
    /// @returns true on success, false otherwise.
    bool Enables(VectorRef<const ast::Enable*> enables) const;

    /// Validates pipeline stages
    /// @param entry_points the entry points to the module
    /// @returns true on success, false otherwise.
    bool PipelineStages(VectorRef<sem::Function*> entry_points) const;

    /// Validates usages of module-scope vars.
    /// @note Must only be called after all functions have been resolved.
    /// @param entry_points the entry points to the module
    /// @returns true on success, false otherwise.
    bool ModuleScopeVarUsages(VectorRef<sem::Function*> entry_points) const;

    /// Validates aliases
    /// @param alias the alias to validate
    /// @returns true on success, false otherwise.
    bool Alias(const ast::Alias* alias) const;

    /// Validates the array
    /// @param arr the array to validate
    /// @param el_source the source of the array element, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @returns true on success, false otherwise.
    bool Array(const sem::Array* arr, const Source& el_source) const;

    /// Validates an array stride attribute
    /// @param attr the stride attribute to validate
    /// @param el_size the element size
    /// @param el_align the element alignment
    /// @returns true on success, false otherwise
    bool ArrayStrideAttribute(const ast::StrideAttribute* attr,
                              uint32_t el_size,
                              uint32_t el_align) const;

    /// Validates an atomic type
    /// @param a the atomic ast node
    /// @param s the atomic sem node
    /// @returns true on success, false otherwise.
    bool Atomic(const ast::TemplatedIdentifier* a, const core::type::Atomic* s) const;

    /// Validates a pointer type
    /// @param a the pointer ast node
    /// @param s the pointer sem node
    /// @returns true on success, false otherwise.
    bool Pointer(const ast::TemplatedIdentifier* a, const core::type::Pointer* s) const;

    /// Validates an assignment
    /// @param a the assignment statement
    /// @param rhs_ty the type of the right hand side
    /// @returns true on success, false otherwise.
    bool Assignment(const ast::Statement* a, const core::type::Type* rhs_ty) const;

    /// Validates a binary expression
    /// @param node the ast binary expression or compound assignment node
    /// @param op the binary operator
    /// @param lhs the left hand side sem node
    /// @param rhs the right hand side sem node
    /// @returns true on success, false otherwise.
    bool BinaryExpression(const ast::Node* node,
                          const core::BinaryOp op,
                          const tint::sem::ValueExpression* lhs,
                          const tint::sem::ValueExpression* rhs) const;

    /// Validates a break statement
    /// @param stmt the break statement to validate
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise.
    bool BreakStatement(const sem::Statement* stmt, sem::Statement* current_statement) const;

    /// Validates a builtin attribute
    /// @param attr the attribute to validate
    /// @param storage_type the attribute storage type
    /// @param stage the current pipeline stage
    /// @param is_input true if this is an input attribute
    /// @returns true on success, false otherwise.
    bool BuiltinAttribute(const ast::BuiltinAttribute* attr,
                          const core::type::Type* storage_type,
                          ast::PipelineStage stage,
                          const bool is_input) const;

    /// Validates a continue statement
    /// @param stmt the continue statement to validate
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool ContinueStatement(const sem::Statement* stmt, sem::Statement* current_statement) const;

    /// Validates a call
    /// @param call the call
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool Call(const sem::Call* call, sem::Statement* current_statement) const;

    /// Validates an entry point
    /// @param func the entry point function to validate
    /// @param stage the pipeline stage for the entry point
    /// @returns true on success, false otherwise
    bool EntryPoint(const sem::Function* func, ast::PipelineStage stage) const;

    /// Validates that the expression must not be evaluated any later than @p latest_stage
    /// @param expr the expression to check
    /// @param latest_stage the latest evaluation stage that the expression can be evaluated
    /// @param constraint the 'thing' that is imposing the contraint. e.g. "var declaration"
    /// @returns true if @p expr is evaluated in or before @p latest_stage, false otherwise
    bool EvaluationStage(const sem::ValueExpression* expr,
                         core::EvaluationStage latest_stage,
                         std::string_view constraint) const;

    /// Validates a for loop
    /// @param stmt the for loop statement to validate
    /// @returns true on success, false otherwise
    bool ForLoopStatement(const sem::ForLoopStatement* stmt) const;

    /// Validates a while loop
    /// @param stmt the while statement to validate
    /// @returns true on success, false otherwise
    bool WhileStatement(const sem::WhileStatement* stmt) const;

    /// Validates a function
    /// @param func the function to validate
    /// @param stage the current pipeline stage
    /// @returns true on success, false otherwise.
    bool Function(const sem::Function* func, ast::PipelineStage stage) const;

    /// Validates a function call
    /// @param call the function call to validate
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool FunctionCall(const sem::Call* call, sem::Statement* current_statement) const;

    /// Validates a global variable
    /// @param var the global variable to validate
    /// @param override_id the set of override ids in the module
    /// @returns true on success, false otherwise
    bool GlobalVariable(const sem::GlobalVariable* var,
                        const Hashmap<OverrideId, const sem::Variable*, 8>& override_id) const;

    /// Validates a break-if statement
    /// @param stmt the statement to validate
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool BreakIfStatement(const sem::BreakIfStatement* stmt,
                          sem::Statement* current_statement) const;

    /// Validates an if statement
    /// @param stmt the statement to validate
    /// @returns true on success, false otherwise
    bool IfStatement(const sem::IfStatement* stmt) const;

    /// Validates an increment or decrement statement
    /// @param stmt the statement to validate
    /// @returns true on success, false otherwise
    bool IncrementDecrementStatement(const ast::IncrementDecrementStatement* stmt) const;

    /// Validates an interpolate attribute
    /// @param attr the attribute to validate
    /// @param storage_type the storage type of the attached variable
    /// @param stage the current pipeline stage
    /// @returns true on success, false otherwise
    bool InterpolateAttribute(const ast::InterpolateAttribute* attr,
                              const core::type::Type* storage_type,
                              const ast::PipelineStage stage) const;

    /// Validates an invariant attribute
    /// @param attr the attribute to validate
    /// @param stage the current pipeline stage
    /// @returns true on success, false otherwise
    bool InvariantAttribute(const ast::InvariantAttribute* attr,
                            const ast::PipelineStage stage) const;

    /// Validates a builtin call
    /// @param call the builtin call to validate
    /// @returns true on success, false otherwise.
    bool BuiltinCall(const sem::Call* call) const;

    /// Validates a local variable
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool LocalVariable(const sem::Variable* v) const;

    /// Validates a location attribute
    /// @param attr the attribute to validate
    /// @param type the variable type
    /// @param stage the current pipeline stage
    /// @param source the source of declaration using the attribute
    /// @returns true on success, false otherwise.
    bool LocationAttribute(const ast::LocationAttribute* attr,
                           const core::type::Type* type,
                           const ast::PipelineStage stage,
                           const Source& source) const;

    /// Validates a color attribute
    /// @param attr the color attribute to validate
    /// @param type the variable type
    /// @param stage the current pipeline stage
    /// @param source the source of declaration using the attribute
    /// @param is_input true if is an input variable, false if output variable, std::nullopt is
    /// unknown.
    /// @returns true on success, false otherwise.
    bool ColorAttribute(const ast::ColorAttribute* attr,
                        const core::type::Type* type,
                        ast::PipelineStage stage,
                        const Source& source,
                        const std::optional<bool> is_input = std::nullopt) const;

    /// Validates a blend_src attribute
    /// @param blend_src_attr the blend_src attribute to validate
    /// @param stage the current pipeline stage
    /// @param is_input true if is an input variable, false if output variable, std::nullopt is
    /// unknown.
    /// @returns true on success, false otherwise.
    bool BlendSrcAttribute(const ast::BlendSrcAttribute* blend_src_attr,
                           ast::PipelineStage stage,
                           const std::optional<bool> is_input = std::nullopt) const;

    /// Validates a loop statement
    /// @param stmt the loop statement
    /// @returns true on success, false otherwise.
    bool LoopStatement(const sem::LoopStatement* stmt) const;

    /// Validates a materialize of an abstract numeric value from the type `from` to the type `to`.
    /// @param to the target type
    /// @param from the abstract numeric type
    /// @param source the source of the materialization
    /// @returns true on success, false otherwise
    bool Materialize(const core::type::Type* to,
                     const core::type::Type* from,
                     const Source& source) const;

    /// Validates a matrix
    /// @param el_ty the matrix element type to validate
    /// @param source the source of the matrix
    /// @returns true on success, false otherwise
    bool Matrix(const core::type::Type* el_ty, const Source& source) const;

    /// Validates a function parameter
    /// @param var the variable to validate
    /// @returns true on success, false otherwise
    bool Parameter(const sem::Variable* var) const;

    /// Validates a return
    /// @param ret the return statement to validate
    /// @param func_type the return type of the curreunt function
    /// @param ret_type the return type
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool Return(const ast::ReturnStatement* ret,
                const core::type::Type* func_type,
                const core::type::Type* ret_type,
                sem::Statement* current_statement) const;

    /// Validates a list of statements
    /// @param stmts the statements to validate
    /// @returns true on success, false otherwise
    bool Statements(VectorRef<const ast::Statement*> stmts) const;

    /// Validates a storage texture
    /// @param t the texture to validate
    /// @param source the source of the texture
    /// @returns true on success, false otherwise
    bool StorageTexture(const core::type::StorageTexture* t, const Source& source) const;

    /// Validates a texel buffer
    /// @param t the texel buffer to validate
    /// @param source the source of the texel buffer
    /// @returns true on success, false otherwise
    bool TexelBuffer(const core::type::TexelBuffer* t, const Source& source) const;

    /// Validates a sampled texture
    /// @param t the texture to validate
    /// @param source the source of the texture
    /// @returns true on success, false otherwise
    bool SampledTexture(const core::type::SampledTexture* t, const Source& source) const;

    /// Validates a multisampled texture
    /// @param t the texture to validate
    /// @param source the source of the texture
    /// @returns true on success, false otherwise
    bool MultisampledTexture(const core::type::MultisampledTexture* t, const Source& source) const;

    /// Validates a input attachment
    /// @param t the input attachment to validate
    /// @param source the source of the input attachment
    /// @returns true on success, false otherwise
    bool InputAttachment(const core::type::InputAttachment* t, const Source& source) const;

    /// Validates a input attachment index attribute
    /// @param attr the input attachment index attribute to validate
    /// @param type the variable type
    /// @param source the source of declaration using the attribute
    /// @returns true on success, false otherwise.
    bool InputAttachmentIndexAttribute(const ast::InputAttachmentIndexAttribute* attr,
                                       const core::type::Type* type,
                                       const Source& source) const;

    /// Validates a binding array type
    /// @param t the binding array to validate
    /// @param source the source of the binding array type
    /// @returns true on success, false otherwise
    bool BindingArray(const core::type::BindingArray* t, const Source& source) const;

    /// Validates a subgroup matrix type
    /// @param t the subgroup matrix type to validate
    /// @param source the source of the subgroup matrix type
    /// @returns true on success, false otherwise
    bool SubgroupMatrix(const core::type::SubgroupMatrix* t, const Source& source) const;

    /// Validates a structure
    /// @param str the structure to validate
    /// @param stage the current pipeline stage
    /// @returns true on success, false otherwise.
    bool Structure(const sem::Struct* str, ast::PipelineStage stage) const;

    /// Validates a structure initializer
    /// @param ctor the call expression to validate
    /// @param struct_type the type of the structure
    /// @returns true on success, false otherwise
    bool StructureInitializer(const ast::CallExpression* ctor,
                              const core::type::Struct* struct_type) const;

    /// Validates a switch statement
    /// @param s the switch to validate
    /// @returns true on success, false otherwise
    bool SwitchStatement(const ast::SwitchStatement* s);

    /// Validates a 'var' variable declaration
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool Var(const sem::Variable* v) const;

    /// Validates a 'let' variable declaration
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool Let(const sem::Variable* v) const;

    /// Validates a 'override' variable declaration
    /// @param v the variable to validate
    /// @param override_id the set of override ids in the module
    /// @returns true on success, false otherwise.
    bool Override(const sem::GlobalVariable* v,
                  const Hashmap<OverrideId, const sem::Variable*, 8>& override_id) const;

    /// Validates a 'const' variable declaration
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool Const(const sem::Variable* v) const;

    /// Validates a variable initializer
    /// @param v the variable to validate
    /// @param storage_type the type of the storage
    /// @param initializer the RHS initializer expression
    /// @returns true on succes, false otherwise
    bool VariableInitializer(const ast::Variable* v,
                             const core::type::Type* storage_type,
                             const sem::ValueExpression* initializer) const;

    /// Validates a vector
    /// @param el_ty the vector element type to validate
    /// @param source the source of the vector
    /// @returns true on success, false otherwise
    bool Vector(const core::type::Type* el_ty, const Source& source) const;

    /// Validates an array constructor
    /// @param ctor the call expression to validate
    /// @param arr_type the type of the array
    /// @returns true on success, false otherwise
    bool ArrayConstructor(const ast::CallExpression* ctor, const sem::Array* arr_type) const;

    /// Validates a subgroup matrix constructor
    /// @param ctor the call expression to validate
    /// @param subgroup_matrix_type the type of the subgroup matrix
    /// @returns true on success, false otherwise
    bool SubgroupMatrixConstructor(const ast::CallExpression* ctor,
                                   const core::type::SubgroupMatrix* subgroup_matrix_type) const;

    /// Validates a subgroupShuffle builtin functions including Up,Down, and Xor.
    /// @param fn the builtin call type
    /// @param call the builtin call to validate
    /// @returns true on success, false otherwise
    bool SubgroupShuffleFunction(wgsl::BuiltinFn fn, const sem::Call* call) const;

    /// Validates a texture builtin function
    /// @param call the builtin call to validate
    /// @returns true on success, false otherwise
    bool TextureBuiltinFn(const sem::Call* call) const;

    /// Validates a subgroupBroadcast builtin function
    /// @param call the builtin call to validate
    /// @returns true on success, false otherwise
    bool SubgroupBroadcast(const sem::Call* call) const;

    /// Validates a quadBroadcast builtin function
    /// @param call the builtin call to validate
    /// @returns true on success, false otherwise
    bool QuadBroadcast(const sem::Call* call) const;

    /// Validates an optional builtin function and its required extensions and language features.
    /// @param call the builtin call to validate
    /// @returns true on success, false otherwise
    bool RequiredFeaturesForBuiltinFn(const sem::Call* call) const;

    /// Validates that 'f16' extension is enabled for f16 usage at @p source
    /// @param source the source of the f16 usage
    /// @returns true on success, false otherwise
    bool CheckF16Enabled(const Source& source) const;

    /// Validates that 'chromium_experimental_subgroup_matrix' extension is enabled for i8 usage at
    /// @p source
    /// @param source the source of the i8 usage
    /// @returns true on success, false otherwise
    bool CheckI8Enabled(const Source& source) const;

    /// Validates that 'chromium_experimental_subgroup_matrix' extension is enabled for u8 usage at
    /// @p source
    /// @param source the source of the u8 usage
    /// @returns true on success, false otherwise
    bool CheckU8Enabled(const Source& source) const;

    /// Validates there are no duplicate attributes
    /// @param attributes the list of attributes to validate
    /// @returns true on success, false otherwise.
    bool NoDuplicateAttributes(VectorRef<const ast::Attribute*> attributes) const;

    /// Validates a set of diagnostic controls.
    /// @param controls the diagnostic controls to validate
    /// @param use the place where the controls are being used ("directive" or "attribute")
    /// @param allow_duplicates if same name same severity diagnostics are allowed
    /// @returns true on success, false otherwise.
    bool DiagnosticControls(VectorRef<const ast::DiagnosticControl*> controls,
                            const char* use,
                            DiagnosticDuplicates allow_duplicates) const;

    /// Validates a address space layout
    /// @param type the type to validate
    /// @param sc the address space
    /// @param source the source of the type
    /// @returns true on success, false otherwise
    bool AddressSpaceLayout(const core::type::Type* type,
                            core::AddressSpace sc,
                            Source source) const;

    /// @returns true if the attribute list contains a
    /// ast::DisableValidationAttribute with the validation mode equal to
    /// `validation`
    /// @param attributes the attribute list to check
    /// @param validation the validation mode to check
    bool IsValidationDisabled(VectorRef<const ast::Attribute*> attributes,
                              ast::DisabledValidation validation) const;

    /// @returns true if the attribute list does not contains a
    /// ast::DisableValidationAttribute with the validation mode equal to
    /// `validation`
    /// @param attributes the attribute list to check
    /// @param validation the validation mode to check
    bool IsValidationEnabled(VectorRef<const ast::Attribute*> attributes,
                             ast::DisabledValidation validation) const;

  private:
    /// @param ty the type to check
    /// @returns true if @p ty is an array with an `override` expression element count, otherwise
    ///          false.
    bool IsArrayWithOverrideCount(const core::type::Type* ty) const;

    /// Raises an error about an array type using an `override` expression element count, outside
    /// the single allowed use of a `var<workgroup>`.
    /// @param source the source for the error
    void RaiseArrayWithOverrideCountError(const Source& source) const;

    /// Searches the current statement and up through parents of the current
    /// statement looking for a loop or for-loop continuing statement.
    /// @returns the closest continuing statement to the current statement that
    /// (transitively) owns the current statement.
    /// @param stop_at_loop if true then the function will return nullptr if a
    /// loop or for-loop was found before the continuing.
    /// @param stop_at_switch if true then the function will return nullptr if a switch was found
    /// before continuing.
    /// @param current_statement the current statement being resolved
    const ast::Statement* ClosestContinuing(bool stop_at_loop,
                                            bool stop_at_switch,
                                            sem::Statement* current_statement) const;

    /// Returns a human-readable string representation of the vector type name
    /// with the given parameters.
    /// @param size the vector dimension
    /// @param element_type scalar vector sub-element type
    /// @return pretty string representation
    std::string VectorPretty(uint32_t size, const core::type::Type* element_type) const;

    /// Raises an error if combination of @p store_ty, @p access and @p address_space are not valid
    /// for a `var` or `ptr` declaration.
    /// @param store_ty the store type of the var or pointer
    /// @param access the var or pointer access
    /// @param address_space the var or pointer address space
    /// @param source the source for the error
    /// @returns true on success, false if an error was raised.
    bool CheckTypeAccessAddressSpace(const core::type::Type* store_ty,
                                     core::Access access,
                                     core::AddressSpace address_space,
                                     const Source& source) const;

    /// Raises an error if the entry_point @p entry_point uses two or more module-scope 'var's with
    /// the address space @p space.
    /// @param entry_point the entry point
    /// @param space the address space
    /// @returns true if no duplicate uses were found or false if an error was raised.
    bool CheckNoMultipleModuleScopeVarsOfAddressSpace(sem::Function* entry_point,
                                                      core::AddressSpace space) const;

    SymbolTable& symbols_;
    diag::List& diagnostics_;
    SemHelper& sem_;
    DiagnosticFilterStack diagnostic_filters_;
    const wgsl::Extensions& enabled_extensions_;
    const wgsl::AllowedFeatures& allowed_features_;
    const Hashmap<const core::type::Type*, const Source*, 8>& atomic_composite_info_;
    Hashset<TypeAndAddressSpace, 8>& valid_type_storage_layouts_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_VALIDATOR_H_
