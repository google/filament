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

#include "src/tint/lang/wgsl/sem/variable.h"

#include <utility>

#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/parameter.h"
#include "src/tint/lang/wgsl/ast/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Variable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::GlobalVariable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::LocalVariable);
TINT_INSTANTIATE_TYPEINFO(tint::sem::Parameter);
TINT_INSTANTIATE_TYPEINFO(tint::sem::VariableUser);

namespace tint::sem {
Variable::Variable(const ast::Variable* declaration) : declaration_(declaration) {}

Variable::~Variable() = default;

LocalVariable::LocalVariable(const ast::Variable* declaration, const sem::Statement* statement)
    : Base(declaration), statement_(statement) {}

LocalVariable::~LocalVariable() = default;

GlobalVariable::GlobalVariable(const ast::Variable* declaration) : Base(declaration) {}

GlobalVariable::~GlobalVariable() = default;

void GlobalVariable::AddTransitivelyReferencedOverride(const GlobalVariable* var) {
    if (transitively_referenced_overrides_.Add(var)) {
        for (auto* ref : var->TransitivelyReferencedOverrides()) {
            AddTransitivelyReferencedOverride(ref);
        }
    }
}

Parameter::Parameter(const ast::Parameter* declaration,
                     uint32_t index,
                     const core::type::Type* type /* = nullptr */,
                     core::ParameterUsage usage /* = core::ParameterUsage::kNone */)
    : Base(declaration), index_(index), usage_(usage) {
    SetType(type);
}

Parameter::~Parameter() = default;

VariableUser::VariableUser(const ast::IdentifierExpression* declaration,
                           core::EvaluationStage stage,
                           Statement* statement,
                           const core::constant::Value* constant,
                           sem::Variable* variable)
    : Base(declaration,
           variable->Type(),
           stage,
           statement,
           constant,
           /* has_side_effects */ false),
      variable_(variable) {
    auto* type = variable->Type();
    if (type->Is<core::type::Pointer>() && variable->Initializer()) {
        root_identifier_ = variable->Initializer()->RootIdentifier();
    } else {
        root_identifier_ = variable;
    }
}

VariableUser::~VariableUser() = default;

const ast::IdentifierExpression* VariableUser::Declaration() const {
    return static_cast<const ast::IdentifierExpression*>(Base::Declaration());
}

}  // namespace tint::sem
