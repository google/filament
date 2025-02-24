// Copyright 2024 The Dawn & Tint Authors
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

#include "langsvr/lsp/primitives.h"
#include "src/tint/lang/wgsl/ls/server.h"

#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/variable.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

typename lsp::TextDocumentInlayHintRequest::ResultType  //
Server::Handle(const lsp::TextDocumentInlayHintRequest& r) {
    auto file = files_.Get(r.text_document.uri);
    if (!file) {
        return lsp::Null{};
    }

    std::vector<lsp::InlayHint> hints;
    auto& program = (*file)->program;
    for (auto* decl : program.AST().TypeDecls()) {
        if (auto* str = program.Sem().Get<sem::Struct>(decl)) {
            if (!str->UsedAs(core::AddressSpace::kStorage) &&
                !str->UsedAs(core::AddressSpace::kUniform)) {
                continue;
            }
            for (auto* member : str->Members()) {
                auto pos = (*file)->Conv(member->Declaration()->name->source.range.begin);
                auto add = [&](std::string text) {
                    lsp::InlayHint hint;
                    hint.position = pos;
                    hint.label = text;
                    hint.padding_left = true;
                    hint.padding_right = true;
                    hints.push_back(hint);
                };
                add("offset: " + std::to_string(member->Offset()));
                add("size: " + std::to_string(member->Size()));
            }
        }
    }

    for (auto* node : program.ASTNodes().Objects()) {
        if (auto* decl = node->As<ast::Variable>()) {
            if (!decl->type) {
                if (auto* variable = program.Sem().Get(decl); variable && variable->Type()) {
                    lsp::InlayHint hint;
                    hint.position = (*file)->Conv(decl->name->source.range.end);
                    hint.label = " : " + variable->Type()->UnwrapRef()->FriendlyName();
                    hints.push_back(hint);
                }
            }
        }
    }

    if (hints.empty()) {
        return lsp::Null{};
    }

    return std::move(hints);
}

}  // namespace tint::wgsl::ls
