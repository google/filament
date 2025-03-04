// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/diagnostic_rule_name.h"

#include <string>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/clone_context.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DiagnosticRuleName);

namespace tint::ast {

DiagnosticRuleName::DiagnosticRuleName(GenerationID pid,
                                       NodeID nid,
                                       const Source& src,
                                       const Identifier* n)
    : Base(pid, nid, src), name(n) {
    TINT_ASSERT(name != nullptr);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(name, generation_id);
    if (name) {
        // It is invalid for a diagnostic rule name to be templated
        TINT_ASSERT(!name->Is<TemplatedIdentifier>());
    }
}

DiagnosticRuleName::DiagnosticRuleName(GenerationID pid,
                                       NodeID nid,
                                       const Source& src,
                                       const Identifier* c,
                                       const Identifier* n)
    : Base(pid, nid, src), category(c), name(n) {
    TINT_ASSERT(name != nullptr);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(name, generation_id);
    if (name) {
        // It is invalid for a diagnostic rule name to be templated
        TINT_ASSERT(!name->Is<TemplatedIdentifier>());
    }
    if (category) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(category, generation_id);
        // It is invalid for a diagnostic rule category to be templated
        TINT_ASSERT(!category->Is<TemplatedIdentifier>());
    }
}

const DiagnosticRuleName* DiagnosticRuleName::Clone(CloneContext& ctx) const {
    auto src = ctx.Clone(source);
    auto n = ctx.Clone(name);
    if (auto c = ctx.Clone(category)) {
        return ctx.dst->create<DiagnosticRuleName>(src, c, n);
    }
    return ctx.dst->create<DiagnosticRuleName>(src, n);
}

std::string DiagnosticRuleName::String() const {
    if (category) {
        return category->symbol.Name() + "." + name->symbol.Name();
    } else {
        return name->symbol.Name();
    }
}

}  // namespace tint::ast
