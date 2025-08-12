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

#include "src/tint/lang/wgsl/ls/server.h"

#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/type_decl.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/utils/rtti/switch.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

typename lsp::TextDocumentCompletionRequest::ResultType  //
Server::Handle(const lsp::TextDocumentCompletionRequest& r) {
    auto file = files_.Get(r.text_document.uri);
    if (!file) {
        return lsp::Null{};
    }

    // TODO(bclayton): This is very much a first-pass effort.
    // To handle completions properly, the resolver will need to parse ASTs that are incomplete, and
    // provide semantic information even in the case of resolver error.

    std::vector<lsp::CompletionItem> out;
    Hashset<std::string, 32> seen;

    auto loc = (*file)->Conv(r.position);
    if (auto* stmt = (*file)->NodeAt<sem::Statement>(loc)) {
        for (auto* s = stmt; s; s = s->Parent()) {
            Switch(s,  //
                   [&](const sem::BlockStatement* block) {
                       for (auto it : block->Decls()) {
                           if (seen.Add(it.key->Name())) {
                               lsp::CompletionItem item;
                               item.label = it.key->Name();
                               item.kind = lsp::CompletionItemKind::kVariable;
                               out.push_back(item);
                           }
                       }
                   });
        }
        if (auto* fn = stmt->Function()) {
            for (auto* param : fn->Parameters()) {
                auto name = param->Declaration()->name->symbol.Name();
                if (seen.Add(name)) {
                    lsp::CompletionItem item;
                    item.label = name;
                    item.kind = lsp::CompletionItemKind::kVariable;
                    out.push_back(item);
                }
            }
        }
    }

    for (auto decl : (*file)->program.AST().TypeDecls()) {
        auto name = decl->name->symbol.Name();
        if (seen.Add(name)) {
            lsp::CompletionItem item;
            item.label = name;
            item.kind = lsp::CompletionItemKind::kStruct;
            out.push_back(item);
        }
    }

    for (auto fn : (*file)->program.AST().Functions()) {
        auto name = fn->name->symbol.Name();
        if (seen.Add(name)) {
            lsp::CompletionItem item;
            item.label = name;
            item.kind = lsp::CompletionItemKind::kFunction;
            out.push_back(item);
        }
    }

    for (auto v : (*file)->program.AST().GlobalVariables()) {
        auto name = v->name->symbol.Name();
        if (seen.Add(name)) {
            lsp::CompletionItem item;
            item.label = name;
            item.kind = lsp::CompletionItemKind::kVariable;
            out.push_back(item);
        }
    }

    for (auto& builtin : wgsl::kBuiltinFnStrings) {
        if (seen.Add(builtin)) {
            lsp::CompletionItem item;
            item.label = builtin;
            item.kind = lsp::CompletionItemKind::kFunction;
            out.push_back(item);
        }
    }

    return out;
}

}  // namespace tint::wgsl::ls
