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

#include "src/tint/lang/wgsl/sem/block_statement.h"

#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/function.h"
#include "src/tint/lang/wgsl/sem/function.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::BlockStatement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::FunctionBlockStatement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::LoopBlockStatement);

namespace tint::sem {

BlockStatement::BlockStatement(const ast::BlockStatement* declaration,
                               const CompoundStatement* parent,
                               const sem::Function* function)
    : Base(declaration, parent, function) {}

BlockStatement::~BlockStatement() = default;

const ast::BlockStatement* BlockStatement::Declaration() const {
    return Base::Declaration()->As<ast::BlockStatement>();
}

FunctionBlockStatement::FunctionBlockStatement(const sem::Function* function)
    : Base(function->Declaration()->body, nullptr, function) {
    TINT_ASSERT(function);
}

FunctionBlockStatement::~FunctionBlockStatement() = default;

LoopBlockStatement::LoopBlockStatement(const ast::BlockStatement* declaration,
                                       const CompoundStatement* parent,
                                       const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(parent);
    TINT_ASSERT(function);
}
LoopBlockStatement::~LoopBlockStatement() = default;

void LoopBlockStatement::SetFirstContinue(const ast::ContinueStatement* first_continue,
                                          size_t num_decls) {
    first_continue_ = first_continue;
    num_decls_at_first_continue_ = num_decls;
}

}  // namespace tint::sem
