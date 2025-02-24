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

#include "src/tint/lang/wgsl/ast/if_statement.h"

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/clone_context.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::IfStatement);

namespace tint::ast {

IfStatement::IfStatement(GenerationID pid,
                         NodeID nid,
                         const Source& src,
                         const Expression* cond,
                         const BlockStatement* b,
                         const Statement* else_stmt,
                         VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src),
      condition(cond),
      body(b),
      else_statement(else_stmt),
      attributes(std::move(attrs)) {
    TINT_ASSERT(condition);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(condition, generation_id);
    TINT_ASSERT(body);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(body, generation_id);
    if (else_statement) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(else_statement, generation_id);
        TINT_ASSERT((else_statement->IsAnyOf<IfStatement, BlockStatement>()));
    }
    for (auto* attr : attributes) {
        TINT_ASSERT(attr);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(attr, generation_id);
    }
}

IfStatement::~IfStatement() = default;

const IfStatement* IfStatement::Clone(CloneContext& ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx.Clone(source);
    auto* cond = ctx.Clone(condition);
    auto* b = ctx.Clone(body);
    auto* el = ctx.Clone(else_statement);
    auto attrs = ctx.Clone(attributes);
    return ctx.dst->create<IfStatement>(src, cond, b, el, std::move(attrs));
}

}  // namespace tint::ast
