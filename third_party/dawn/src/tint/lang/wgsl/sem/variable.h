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

#ifndef SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_
#define SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_

#include <optional>
#include <utility>
#include <vector>

#include "src/tint/api/common/override_id.h"

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/wgsl/ast/parameter.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/unique_vector.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
class Parameter;
class Variable;
}  // namespace tint::ast
namespace tint::sem {
class CallTarget;
class VariableUser;
}  // namespace tint::sem

namespace tint::sem {

/// Variable is the base class for local variables, global variables and
/// parameters.
class Variable : public Castable<Variable, Node> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    explicit Variable(const ast::Variable* declaration);

    /// Destructor
    ~Variable() override;

    /// @returns the AST declaration node
    const ast::Variable* Declaration() const { return declaration_; }

    /// @param type the variable type
    void SetType(const core::type::Type* type) { type_ = type; }

    /// @returns the canonical type for the variable
    const core::type::Type* Type() const { return type_; }

    /// @param stage the evaluation stage for an expression of this variable type
    void SetStage(core::EvaluationStage stage) { stage_ = stage; }

    /// @returns the evaluation stage for an expression of this variable type
    core::EvaluationStage Stage() const { return stage_; }

    /// @param space the variable address space
    void SetAddressSpace(core::AddressSpace space) { address_space_ = space; }

    /// @returns the address space for the variable
    core::AddressSpace AddressSpace() const { return address_space_; }

    /// @param access the variable access control type
    void SetAccess(core::Access access) { access_ = access; }

    /// @returns the access control for the variable
    core::Access Access() const { return access_; }

    /// @param value the constant value for the variable. May be null
    void SetConstantValue(const core::constant::Value* value) { constant_value_ = value; }

    /// @return the constant value of this expression
    const core::constant::Value* ConstantValue() const { return constant_value_; }

    /// Sets the variable initializer expression.
    /// @param initializer the initializer expression to assign to this variable.
    void SetInitializer(const ValueExpression* initializer) { initializer_ = initializer; }

    /// @returns the variable initializer expression, or nullptr if the variable
    /// does not have one.
    const ValueExpression* Initializer() const { return initializer_; }

    /// @returns the expressions that use the variable
    VectorRef<const VariableUser*> Users() const { return users_; }

    /// @param user the user to add
    void AddUser(const VariableUser* user) { users_.Push(user); }

  private:
    const ast::Variable* const declaration_ = nullptr;
    const core::type::Type* type_ = nullptr;
    core::EvaluationStage stage_ = core::EvaluationStage::kRuntime;
    core::AddressSpace address_space_ = core::AddressSpace::kUndefined;
    core::Access access_ = core::Access::kUndefined;
    const core::constant::Value* constant_value_ = nullptr;
    const ValueExpression* initializer_ = nullptr;
    tint::Vector<const VariableUser*, 8> users_;
};

/// LocalVariable is a function-scope variable
class LocalVariable final : public Castable<LocalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param statement the statement that declared this local variable
    LocalVariable(const ast::Variable* declaration, const sem::Statement* statement);

    /// Destructor
    ~LocalVariable() override;

    /// @returns the statement that declares this local variable
    const sem::Statement* Statement() const { return statement_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const CastableBase* shadows) { shadows_ = shadows; }

    /// @returns the Type, Function or Variable that this local variable shadows
    const CastableBase* Shadows() const { return shadows_; }

  private:
    const sem::Statement* const statement_;
    const CastableBase* shadows_ = nullptr;
};

/// Attributes that can be applied to global variables
struct GlobalVariableAttributes {
    /// the pipeline constant ID associated with the variable
    std::optional<tint::OverrideId> override_id;
    /// the resource binding point for the variable, if set.
    std::optional<tint::BindingPoint> binding_point;
    /// The `input_attachment_index` attribute value for the variable, if set
    std::optional<uint32_t> input_attachment_index;
};

/// GlobalVariable is a module-scope variable
class GlobalVariable final : public Castable<GlobalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    explicit GlobalVariable(const ast::Variable* declaration);

    /// Destructor
    ~GlobalVariable() override;

    /// Records that this variable (transitively) references the given override variable.
    /// @param var the module-scope override variable
    void AddTransitivelyReferencedOverride(const GlobalVariable* var);

    /// @returns all transitively referenced override variables
    VectorRef<const GlobalVariable*> TransitivelyReferencedOverrides() const {
        return transitively_referenced_overrides_;
    }

    /// @return the mutable attributes for the variable
    GlobalVariableAttributes& Attributes() { return attributes_; }

    /// @return the immutable attributes for the variable
    const GlobalVariableAttributes& Attributes() const { return attributes_; }

  private:
    tint::OverrideId override_id_;
    UniqueVector<const GlobalVariable*, 4> transitively_referenced_overrides_;
    GlobalVariableAttributes attributes_;
};

/// Attributes that can be applied to parameters
struct ParameterAttributes {
    /// The `location` attribute value for the variable, if set
    std::optional<uint32_t> location;
    /// The `blend_src` attribute value for the variable, if set
    std::optional<uint32_t> blend_src;
    /// The `color` attribute value for the variable, if set
    std::optional<uint32_t> color;
};

/// Parameter is a function parameter
class Parameter final : public Castable<Parameter, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param index the index of the parameter in the function
    /// @param type the variable type
    /// @param usage the parameter usage
    Parameter(const ast::Parameter* declaration,
              uint32_t index,
              const core::type::Type* type = nullptr,
              core::ParameterUsage usage = core::ParameterUsage::kNone);

    /// Destructor
    ~Parameter() override;

    /// @returns the AST declaration node
    const ast::Parameter* Declaration() const {
        return static_cast<const ast::Parameter*>(Variable::Declaration());
    }

    /// @return the index of the parameter in the function
    uint32_t Index() const { return index_; }

    /// @returns the semantic usage for the parameter
    core::ParameterUsage Usage() const { return usage_; }

    /// @param owner the CallTarget owner of this parameter
    void SetOwner(const CallTarget* owner) { owner_ = owner; }

    /// @returns the CallTarget owner of this parameter
    const CallTarget* Owner() const { return owner_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const CastableBase* shadows) { shadows_ = shadows; }

    /// @returns the Type, Function or Variable that this local variable shadows
    const CastableBase* Shadows() const { return shadows_; }

    /// @return the mutable attributes for the parameter
    ParameterAttributes& Attributes() { return attributes_; }

    /// @return the immutable attributes for the parameter
    const ParameterAttributes& Attributes() const { return attributes_; }

  private:
    const uint32_t index_ = 0;
    core::ParameterUsage usage_ = core::ParameterUsage::kNone;
    CallTarget const* owner_ = nullptr;
    const CastableBase* shadows_ = nullptr;
    ParameterAttributes attributes_;
};

/// VariableUser holds the semantic information for an identifier expression
/// node that resolves to a variable.
class VariableUser final : public Castable<VariableUser, ValueExpression> {
  public:
    /// Constructor
    /// @param declaration the AST identifier node
    /// @param stage the evaluation stage for an expression of this variable type
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param variable the semantic variable
    VariableUser(const ast::IdentifierExpression* declaration,
                 core::EvaluationStage stage,
                 Statement* statement,
                 const core::constant::Value* constant,
                 sem::Variable* variable);
    ~VariableUser() override;

    /// @returns the AST node
    const ast::IdentifierExpression* Declaration() const;

    /// @returns the variable that this expression refers to
    const sem::Variable* Variable() const { return variable_; }

  private:
    const sem::Variable* const variable_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_
