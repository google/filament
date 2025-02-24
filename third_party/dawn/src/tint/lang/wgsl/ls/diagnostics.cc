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

#include "langsvr/session.h"
#include "src/tint/lang/wgsl/ls/utils.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

langsvr::Result<langsvr::SuccessType> Server::PublishDiagnostics(File& file) {
    lsp::TextDocumentPublishDiagnosticsNotification out;
    out.uri = file.source->path;
    for (auto& diag : file.program.Diagnostics()) {
        lsp::Diagnostic d;
        d.message = diag.message.Plain();
        d.range = file.Conv(diag.source.range);
        switch (diag.severity) {
            case diag::Severity::Note:
                d.severity = lsp::DiagnosticSeverity::kInformation;
                break;
            case diag::Severity::Warning:
                d.severity = lsp::DiagnosticSeverity::kWarning;
                break;
            case diag::Severity::Error:
                d.severity = lsp::DiagnosticSeverity::kError;
                break;
        }
        out.diagnostics.push_back(std::move(d));
    }

    return session_.Send(out);
}

}  // namespace tint::wgsl::ls
