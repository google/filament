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

#include "src/tint/lang/wgsl/sem/value_expression.h"

#include <utility>

#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/materialize.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::ValueExpression);

namespace tint::sem {

ValueExpression::ValueExpression(const ast::Expression* declaration,
                                 const core::type::Type* type,
                                 core::EvaluationStage stage,
                                 const Statement* statement,
                                 const core::constant::Value* constant,
                                 const Variable* root_ident /* = nullptr */)
    : Base(declaration, statement),
      root_identifier_(root_ident),
      type_(type),
      stage_(stage),
      constant_(std::move(constant)) {
    TINT_ASSERT(type_);
    TINT_ASSERT((constant != nullptr) == (stage == core::EvaluationStage::kConstant));
    if (constant != nullptr) {
        TINT_ASSERT(type_ == constant->Type());
    }
}

ValueExpression::~ValueExpression() = default;

const ValueExpression* ValueExpression::UnwrapMaterialize() const {
    if (auto* m = As<Materialize>()) {
        return m->Expr();
    }
    return this;
}

const ValueExpression* ValueExpression::UnwrapLoad() const {
    if (auto* l = As<Load>()) {
        return l->Source();
    }
    return this;
}

const ValueExpression* ValueExpression::Unwrap() const {
    return Switch(
        this,  // note: An expression can only be wrapped by a Load or Materialize, not both.
        [&](const Load* load) { return load->Source(); },
        [&](const Materialize* materialize) { return materialize->Expr(); },
        [&](Default) { return this; });
}

}  // namespace tint::sem
