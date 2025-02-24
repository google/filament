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

#include "langsvr/lsp/primitives.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

typename lsp::TextDocumentPrepareRenameRequest::ResultType  //
Server::Handle(const lsp::TextDocumentPrepareRenameRequest& r) {
    typename lsp::TextDocumentPrepareRenameRequest::SuccessType result = lsp::Null{};

    auto file = files_.Get(r.text_document.uri);
    if (!file) {
        return lsp::Null{};
    }

    auto def = (*file)->Definition((*file)->Conv(r.position));
    if (!def) {
        return lsp::Null{};
    }

    lsp::PrepareRenamePlaceholder out;
    out.range = (*file)->Conv(def->reference);
    out.placeholder = def->text;
    return lsp::PrepareRenameResult{out};
}

typename lsp::TextDocumentRenameRequest::ResultType  //
Server::Handle(const lsp::TextDocumentRenameRequest& r) {
    auto file = files_.Get(r.text_document.uri);
    if (!file) {
        return lsp::Null{};
    }

    if (!(*file)->Definition((*file)->Conv(r.position))) {
        return lsp::Null{};
    }

    std::vector<lsp::TextEdit> changes;
    for (auto& ref :
         (*file)->References((*file)->Conv(r.position), /* include_declaration */ true)) {
        lsp::TextEdit edit;
        edit.range = (*file)->Conv(ref);
        edit.new_text = r.new_name;
        changes.emplace_back(std::move(edit));
    }

    if (changes.empty()) {
        return lsp::Null{};
    }
    std::unordered_map<lsp::DocumentUri, std::vector<lsp::TextEdit>> uri_changes;
    uri_changes[r.text_document.uri] = std::move(changes);

    lsp::WorkspaceEdit edit;
    edit.changes = std::move(uri_changes);
    return std::move(edit);
}

}  // namespace tint::wgsl::ls
