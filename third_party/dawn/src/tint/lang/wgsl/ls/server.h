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

#ifndef SRC_TINT_LANG_WGSL_LS_SERVER_H_
#define SRC_TINT_LANG_WGSL_LS_SERVER_H_

#include <memory>
#include <string>
#include <utility>

#include "langsvr/lsp/lsp.h"
#include "langsvr/session.h"

#include "src/tint/lang/wgsl/ls/file.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::wgsl::ls {

/// The language server state object.
class Server {
  public:
    /// Constructor
    /// @param session the LSP session.
    explicit Server(langsvr::Session& session);

    /// Destructor
    ~Server();

    /// @returns true if the server has been requested to shut down.
    bool ShuttingDown() const { return shutting_down_; }

  private:
    ////////////////////////////////////////////////////////////////////////////
    // Requests
    ////////////////////////////////////////////////////////////////////////////

    /// Handler for langsvr::lsp::TextDocumentCompletionRequest
    typename langsvr::lsp::TextDocumentCompletionRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentCompletionRequest&);

    /// Handler for langsvr::lsp::TextDocumentDefinitionRequest
    typename langsvr::lsp::TextDocumentDefinitionRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentDefinitionRequest&);

    /// Handler for langsvr::lsp::TextDocumentDocumentSymbolRequest
    typename langsvr::lsp::TextDocumentDocumentSymbolRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentDocumentSymbolRequest& r);

    /// Handler for langsvr::lsp::TextDocumentHoverRequest
    typename langsvr::lsp::TextDocumentHoverRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentHoverRequest&);

    /// Handler for langsvr::lsp::TextDocumentInlayHintRequest
    typename langsvr::lsp::TextDocumentInlayHintRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentInlayHintRequest&);

    /// Handler for langsvr::lsp::TextDocumentPrepareRenameRequest
    typename langsvr::lsp::TextDocumentPrepareRenameRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentPrepareRenameRequest&);

    /// Handler for langsvr::lsp::TextDocumentReferencesRequest
    typename langsvr::lsp::TextDocumentReferencesRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentReferencesRequest&);

    /// Handler for langsvr::lsp::TextDocumentRenameRequest
    typename langsvr::lsp::TextDocumentRenameRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentRenameRequest&);

    ////////////////////////////////////////////////////////////////////////////
    // Notifications
    ////////////////////////////////////////////////////////////////////////////

    /// Handler for langsvr::lsp::CancelRequestNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::CancelRequestNotification&);

    /// Handler for langsvr::lsp::InitializedNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::InitializedNotification&);

    /// Handler for langsvr::lsp::SetTraceNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::SetTraceNotification&);

    /// Handler for langsvr::lsp::TextDocumentSignatureHelpRequest
    typename langsvr::lsp::TextDocumentSignatureHelpRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentSignatureHelpRequest&);

    /// Handler for langsvr::lsp::TextDocumentDidOpenNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::TextDocumentDidOpenNotification&);

    /// Handler for langsvr::lsp::TextDocumentDidCloseNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::TextDocumentDidCloseNotification&);

    /// Handler for langsvr::lsp::TextDocumentDidChangeNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::TextDocumentDidChangeNotification&);

    /// Handler for langsvr::lsp::TextDocumentSemanticTokensFullRequest
    typename langsvr::lsp::TextDocumentSemanticTokensFullRequest::ResultType  //
    Handle(const langsvr::lsp::TextDocumentSemanticTokensFullRequest&);

    /// Handler for langsvr::lsp::WorkspaceDidChangeConfigurationNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::WorkspaceDidChangeConfigurationNotification&);

    /// Handler for langsvr::lsp::WorkspaceDidChangeWatchedFilesNotification
    langsvr::Result<langsvr::SuccessType>  //
    Handle(const langsvr::lsp::WorkspaceDidChangeWatchedFilesNotification&);

    /// Publishes the tint::Program diagnostics to the server via a
    /// TextDocumentPublishDiagnosticsNotification.
    langsvr::Result<langsvr::SuccessType>  //
    PublishDiagnostics(File& file);

    /// Logger is a string-stream like utility for logging to the client.
    /// Append message content with '<<'. The message is sent when the logger is destructed.
    struct Logger {
        ~Logger();

        /// @brief Appends @p value to the log message
        /// @return this logger
        template <typename T>
        Logger& operator<<(T&& value) {
            msg << std::forward<T>(value);
            return *this;
        }

        langsvr::Session& session;
        langsvr::lsp::MessageType type;
        StringStream msg{};
    };

    /// Log constructs a new Logger to send a log message to the client.
    Logger Log() { return Logger{session_, langsvr::lsp::MessageType::kLog}; }

    /// Error constructs a new Logger to send a log message to the client.
    Logger Error() { return Logger{session_, langsvr::lsp::MessageType::kError}; }

    /// The LSP session.
    langsvr::Session& session_;
    /// Map of URI to File.
    Hashmap<std::string, std::shared_ptr<File>, 8> files_;
    /// True if the server has been asked to shutdown.
    bool shutting_down_ = false;
};

}  // namespace tint::wgsl::ls

#endif  // SRC_TINT_LANG_WGSL_LS_SERVER_H_
