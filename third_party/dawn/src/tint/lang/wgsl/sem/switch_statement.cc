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

#include "src/tint/lang/wgsl/sem/switch_statement.h"

#include "src/tint/lang/wgsl/ast/case_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::CaseStatement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::CaseSelector);
TINT_INSTANTIATE_TYPEINFO(tint::sem::SwitchStatement);

namespace tint::sem {

SwitchStatement::SwitchStatement(const ast::SwitchStatement* declaration,
                                 const CompoundStatement* parent,
                                 const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(parent);
    TINT_ASSERT(function);
}

SwitchStatement::~SwitchStatement() = default;

const ast::SwitchStatement* SwitchStatement::Declaration() const {
    return static_cast<const ast::SwitchStatement*>(Base::Declaration());
}

CaseStatement::CaseStatement(const ast::CaseStatement* declaration,
                             const CompoundStatement* parent,
                             const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(parent);
    TINT_ASSERT(function);
}
CaseStatement::~CaseStatement() = default;

const ast::CaseStatement* CaseStatement::Declaration() const {
    return static_cast<const ast::CaseStatement*>(Base::Declaration());
}

CaseSelector::CaseSelector(const ast::CaseSelector* decl, const core::constant::Value* val)
    : Base(), decl_(decl), val_(val) {}

CaseSelector::~CaseSelector() = default;

const ast::CaseSelector* CaseSelector::Declaration() const {
    return decl_;
}

}  // namespace tint::sem
