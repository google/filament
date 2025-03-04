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

#include "langsvr/lsp/lsp.h"
#include "langsvr/session.h"

#include "src/tint/lang/wgsl/ls/sem_token.h"
#include "src/tint/lang/wgsl/ls/utils.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

Server::Server(langsvr::Session& session) : session_(session) {
    session.Register([&](const lsp::InitializeRequest&) {
        lsp::InitializeResult result;
        result.capabilities.completion_provider = [] {
            lsp::CompletionOptions opts;
            opts.completion_item = lsp::ServerCompletionItemOptions{};
            return opts;
        }();
        result.capabilities.definition_provider = true;
        result.capabilities.document_symbol_provider = [] {
            lsp::DocumentSymbolOptions opts;
            return opts;
        }();
        result.capabilities.hover_provider = true;
        result.capabilities.inlay_hint_provider = true;
        result.capabilities.references_provider = [] {
            lsp::ReferenceOptions opts;
            return opts;
        }();
        result.capabilities.text_document_sync = [] {
            lsp::TextDocumentSyncOptions opts;
            opts.open_close = true;
            opts.change = lsp::TextDocumentSyncKind::kIncremental;
            return opts;
        }();
        result.capabilities.rename_provider = [] {
            lsp::RenameOptions opts;
            opts.prepare_provider = true;
            return opts;
        }();
        result.capabilities.semantic_tokens_provider = [] {
            lsp::SemanticTokensOptions opts;
            opts.full = true;
            for (auto name : SemToken::kNames) {
                opts.legend.token_types.push_back(name);
            }
            return opts;
        }();
        result.capabilities.signature_help_provider = [] {
            lsp::SignatureHelpOptions opts;
            return opts;
        }();
        return result;
    });

    session.Register([&](const lsp::ShutdownRequest&) {
        shutting_down_ = true;
        return lsp::Null{};
    });

    // Notification handlers
    session.Register([&](const lsp::CancelRequestNotification& n) { return Handle(n); });
    session.Register([&](const lsp::InitializedNotification& n) { return Handle(n); });
    session.Register([&](const lsp::SetTraceNotification& n) { return Handle(n); });
    session.Register([&](const lsp::TextDocumentDidChangeNotification& n) { return Handle(n); });
    session.Register([&](const lsp::TextDocumentDidCloseNotification& n) { return Handle(n); });
    session.Register([&](const lsp::TextDocumentDidOpenNotification& n) { return Handle(n); });
    session.Register(
        [&](const lsp::WorkspaceDidChangeConfigurationNotification& n) { return Handle(n); });

    // Request handlers
    session.Register([&](const lsp::TextDocumentCompletionRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentDefinitionRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentDocumentSymbolRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentHoverRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentInlayHintRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentPrepareRenameRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentReferencesRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentRenameRequest& r) { return Handle(r); });
    session.Register(
        [&](const lsp::TextDocumentSemanticTokensFullRequest& r) { return Handle(r); });
    session.Register([&](const lsp::TextDocumentSignatureHelpRequest& r) { return Handle(r); });
    session.Register(
        [&](const lsp::WorkspaceDidChangeWatchedFilesNotification& n) { return Handle(n); });
}

Server::~Server() = default;

Server::Logger::~Logger() {
    lsp::WindowLogMessageNotification n;
    n.message = msg.str();
    n.type = type;
    (void)session.Send(n);
}

}  // namespace tint::wgsl::ls
