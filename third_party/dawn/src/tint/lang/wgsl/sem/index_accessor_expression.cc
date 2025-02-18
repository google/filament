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

#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"

#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"

#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::sem::IndexAccessorExpression);

namespace tint::sem {

IndexAccessorExpression::IndexAccessorExpression(const ast::IndexAccessorExpression* declaration,
                                                 const core::type::Type* type,
                                                 core::EvaluationStage stage,
                                                 const ValueExpression* object,
                                                 const ValueExpression* index,
                                                 const Statement* statement,
                                                 const core::constant::Value* constant,
                                                 bool has_side_effects,
                                                 const Variable* root_ident /* = nullptr */)
    : Base(declaration, type, stage, object, statement, constant, has_side_effects, root_ident),
      index_(index) {}

IndexAccessorExpression::~IndexAccessorExpression() = default;

}  // namespace tint::sem
