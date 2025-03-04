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

#include "src/tint/lang/wgsl/ast/const.h"
#include "src/tint/lang/wgsl/ls/server.h"

#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::wgsl::ls {

namespace lsp = langsvr::lsp;

typename lsp::TextDocumentDocumentSymbolRequest::ResultType  //
Server::Handle(const lsp::TextDocumentDocumentSymbolRequest& r) {
    typename lsp::TextDocumentDocumentSymbolRequest::SuccessType result = lsp::Null{};

    std::vector<lsp::DocumentSymbol> symbols;
    if (auto file = files_.Get(r.text_document.uri)) {
        for (auto* decl : (*file)->program.AST().Functions()) {
            lsp::DocumentSymbol sym;
            sym.range = (*file)->Conv(decl->source.range);
            sym.selection_range = (*file)->Conv(decl->name->source.range);
            sym.kind = lsp::SymbolKind::kFunction;
            sym.name = decl->name->symbol.NameView();
            symbols.push_back(sym);
        }
        for (auto* decl : (*file)->program.AST().GlobalVariables()) {
            lsp::DocumentSymbol sym;
            sym.range = (*file)->Conv(decl->source.range);
            sym.selection_range = (*file)->Conv(decl->name->source.range);
            sym.kind =
                decl->Is<ast::Const>() ? lsp::SymbolKind::kConstant : lsp::SymbolKind::kVariable;
            sym.name = decl->name->symbol.NameView();
            symbols.push_back(sym);
        }
        for (auto* decl : (*file)->program.AST().TypeDecls()) {
            Switch(
                decl,  //
                [&](const ast::Struct* str) {
                    lsp::DocumentSymbol sym;
                    sym.range = (*file)->Conv(str->source.range);
                    sym.selection_range = (*file)->Conv(decl->name->source.range);
                    sym.kind = lsp::SymbolKind::kStruct;
                    sym.name = decl->name->symbol.NameView();
                    symbols.push_back(sym);
                },
                [&](const ast::Alias* str) {
                    lsp::DocumentSymbol sym;
                    sym.range = (*file)->Conv(str->source.range);
                    sym.selection_range = (*file)->Conv(decl->name->source.range);
                    // TODO(crbug.com/tint/2127): Is there a better symbol kind?
                    sym.kind = lsp::SymbolKind::kObject;
                    sym.name = decl->name->symbol.NameView();
                    symbols.push_back(sym);
                });
        }
    }

    if (!symbols.empty()) {
        result = std::move(symbols);
    }

    return result;
}

}  // namespace tint::wgsl::ls
