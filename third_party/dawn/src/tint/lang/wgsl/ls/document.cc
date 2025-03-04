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

#include "src/tint/lang/wgsl/reader/reader.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

namespace {

/// @returns the byte offsets of the start of all the lines in @p str.
std::vector<size_t> LineOffsets(std::string_view str) {
    std::vector<size_t> offsets;
    offsets.push_back(0);
    for (size_t i = 0, n = str.length(); i < n; i++) {
        if (str[i] == '\n') {
            offsets.push_back(i + 1);
        }
    }
    return offsets;
}

}  // namespace

langsvr::Result<langsvr::SuccessType> Server::Handle(
    const lsp::TextDocumentDidOpenNotification& n) {
    wgsl::reader::Options options;
    options.allowed_features = wgsl::AllowedFeatures::Everything();
    auto source = std::make_unique<Source::File>(n.text_document.uri, n.text_document.text);
    auto program = wgsl::reader::Parse(source.get(), options);
    auto file =
        std::make_shared<File>(std::move(source), n.text_document.version, std::move(program));
    files_.Add(n.text_document.uri, file);
    return PublishDiagnostics(*file);
}

langsvr::Result<langsvr::SuccessType> Server::Handle(
    const lsp::TextDocumentDidCloseNotification& n) {
    files_.Remove(n.text_document.uri);
    return langsvr::Success;
}

langsvr::Result<langsvr::SuccessType> Server::Handle(
    const lsp::TextDocumentDidChangeNotification& n) {
    auto file = files_.Get(n.text_document.uri);
    if (!file) {
        return langsvr::Failure{"document not found"};
    }

    auto utf8 = (*file)->source->content.data;
    for (auto& change : n.content_changes) {
        if (auto* edit = change.Get<lsp::TextDocumentContentChangePartial>()) {
            auto range = (*file)->Conv(edit->range);
            std::vector<size_t> line_offsets = LineOffsets(utf8);
            size_t utf8_start = line_offsets[range.begin.line - 1] + range.begin.column - 1;
            size_t utf8_end = line_offsets[range.end.line - 1] + range.end.column - 1;
            utf8 = utf8.substr(0, utf8_start) + edit->text + utf8.substr(utf8_end);
        }
    }
    wgsl::reader::Options options;
    options.allowed_features = wgsl::AllowedFeatures::Everything();
    auto source = std::make_unique<Source::File>(n.text_document.uri, utf8);
    auto program = wgsl::reader::Parse(source.get(), options);
    *file = std::make_shared<File>(std::move(source), n.text_document.version, std::move(program));
    return PublishDiagnostics(**file);
}

}  // namespace tint::wgsl::ls
