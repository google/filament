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

#include "src/tint/lang/wgsl/ast/templated_identifier.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/clone_context.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::TemplatedIdentifier);

namespace tint::ast {

TemplatedIdentifier::TemplatedIdentifier(GenerationID pid,
                                         NodeID nid,
                                         const Source& src,
                                         const Symbol& sym,
                                         VectorRef<const Expression*> args,
                                         VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, sym), arguments(std::move(args)), attributes(std::move(attrs)) {
    TINT_ASSERT(!arguments.IsEmpty());  // Should have been an Identifier if this fires.
    for (auto* arg : arguments) {
        TINT_ASSERT_GENERATION_IDS_EQUAL(arg, generation_id);
    }
    for (auto* attr : attributes) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(attr, generation_id);
    }
}

TemplatedIdentifier::~TemplatedIdentifier() = default;

const TemplatedIdentifier* TemplatedIdentifier::Clone(CloneContext& ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx.Clone(source);
    auto sym = ctx.Clone(symbol);
    auto args = ctx.Clone(arguments);
    auto attrs = ctx.Clone(attributes);
    return ctx.dst->create<TemplatedIdentifier>(src, sym, std::move(args), std::move(attrs));
}

}  // namespace tint::ast
