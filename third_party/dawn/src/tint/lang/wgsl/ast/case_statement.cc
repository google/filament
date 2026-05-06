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

#include "src/tint/lang/wgsl/ast/case_statement.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::CaseStatement);

namespace tint::ast {

CaseStatement::CaseStatement(NodeID nid,
                             const Source& src,
                             VectorRef<const CaseSelector*> s,
                             const BlockStatement* b)
    : Base(nid, src), selectors(std::move(s)), body(b) {
    TINT_ASSERT(body);
    TINT_ASSERT(!selectors.IsEmpty());
    for (auto* selector : selectors) {
        TINT_ASSERT(selector);
    }
}

CaseStatement::~CaseStatement() = default;

bool CaseStatement::ContainsDefault() const {
    for (const auto* sel : selectors) {
        if (sel->IsDefault()) {
            return true;
        }
    }
    return false;
}

}  // namespace tint::ast
