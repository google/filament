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

#include "src/tint/lang/wgsl/ast/clone_context.h"

#include <string>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/utils/containers/map.h"

namespace tint::ast {

CloneContext::CloneContext(ast::Builder* to, GenerationID from) : dst(to), src_id(from) {}

CloneContext::~CloneContext() = default;

Symbol CloneContext::Clone(Symbol s) {
    return cloned_symbols_.GetOrAdd(s, [&]() -> Symbol {
        if (symbol_transform_) {
            return symbol_transform_(s);
        }
        return dst->Symbols().New(s.Name());
    });
}

ast::FunctionList CloneContext::Clone(const ast::FunctionList& v) {
    ast::FunctionList out;
    out.Reserve(v.Length());
    for (const ast::Function* el : v) {
        out.Add(Clone(el));
    }
    return out;
}

ast::Type CloneContext::Clone(const ast::Type& ty) {
    return {Clone(ty.expr)};
}

const ast::Node* CloneContext::CloneNode(const ast::Node* node) {
    // If the input is nullptr, there's nothing to clone - just return nullptr.
    if (node == nullptr) {
        return nullptr;
    }

    // Was Replace() called for this node?
    if (auto fn = replacements_.Get(node)) {
        return (*fn)();
    }

    // Attempt to clone using the registered replacer functions.
    auto& typeinfo = node->TypeInfo();
    for (auto& transform : transforms_) {
        if (typeinfo.Is(transform.typeinfo)) {
            if (auto* transformed = transform.function(node)) {
                return transformed;
            }
            break;
        }
    }

    // No transform for this type, or the transform returned nullptr.
    // Clone with T::Clone().
    return node->Clone(*this);
}

void CloneContext::CheckedCastFailure(const ast::Node* got, const TypeInfo& expected) {
    TINT_ICE() << "Cloned object was not of the expected type\n"
               << "got:      " << got->TypeInfo().name << "\n"
               << "expected: " << expected.name;
}

diag::List& CloneContext::Diagnostics() const {
    return dst->Diagnostics();
}

CloneContext::CloneableTransform::CloneableTransform() = default;
CloneContext::CloneableTransform::CloneableTransform(const CloneableTransform&) = default;
CloneContext::CloneableTransform::~CloneableTransform() = default;

}  // namespace tint::ast
