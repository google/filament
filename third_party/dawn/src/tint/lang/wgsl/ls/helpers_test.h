
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

#ifndef SRC_TINT_LANG_WGSL_LS_HELPERS_TEST_H_
#define SRC_TINT_LANG_WGSL_LS_HELPERS_TEST_H_

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "langsvr/lsp/lsp.h"
#include "langsvr/result.h"
#include "src/tint/lang/wgsl/ls/server.h"

namespace tint::wgsl::ls {

/// A helper base class for language-server tests.
/// Provides a server and client session, and helpers for 'opening' WGSL documents.
/// @note use LsTest or LsTestWithParam<T> instead of using this class directly.
template <typename BASE>
class LsTestImpl : public BASE {
  public:
    /// Constructor
    /// Initializes the server and client sessions, forming a bi-directional message channel.
    /// Registers a langsvr::lsp::TextDocumentPublishDiagnosticsNotification handler for the client
    /// session, so that diagnostic notifications are added to #diagnostics_.
    LsTestImpl() {
        server_session_.SetSender(
            [&](std::string_view msg) { return client_session_.Receive(msg); });
        client_session_.SetSender(
            [&](std::string_view msg) { return server_session_.Receive(msg); });

        client_session_.Register(
            [&](const langsvr::lsp::TextDocumentPublishDiagnosticsNotification& n) {
                diagnostics_.Push(n);
                return langsvr::Success;
            });
    }

    /// Constructs a langsvr::lsp::TextDocumentDidOpenNotification from @p wgsl and a new, unique
    /// URI, and sends this to the server via the #client_session_. Once the server has finished
    /// processing the notification, the server will respond with a
    /// langsvr::lsp::TextDocumentPublishDiagnosticsNotification, which is handled by the
    /// #client_session_ and placed into #diagnostics_.
    std::string OpenDocument(std::string_view wgsl) {
        std::string uri = "document-" + std::to_string(next_document_id_++) + ".wgsl";
        langsvr::lsp::TextDocumentDidOpenNotification notification{};
        notification.text_document.text = wgsl;
        notification.text_document.uri = uri;
        auto res = client_session_.Send(notification);
        EXPECT_EQ(res, langsvr::Success);
        return uri;
    }

    langsvr::Session server_session_;
    langsvr::Session client_session_;
    Server server_{server_session_};
    int next_document_id_ = 0;
    Vector<langsvr::lsp::TextDocumentPublishDiagnosticsNotification, 4> diagnostics_;
};

using LsTest = LsTestImpl<testing::Test>;

template <typename T>
using LsTestWithParam = LsTestImpl<testing::TestWithParam<T>>;

/// Result structure of ParseMarkers
struct ParsedMarkers {
    /// All parsed ranges, marked up with '「' and '」'. For example: `「my_range」`
    std::vector<langsvr::lsp::Range> ranges;
    /// All parsed positions, marked up with '⧘'. For example: `posi⧘tion`
    std::vector<langsvr::lsp::Position> positions;
    /// The string with all markup removed.
    /// '「' and '」' are replaced with whitespace.
    /// '⧘' are omitted with no replacement characters.
    std::string clean;
};

/// ParseMarkers parses location and range markers from the string @p str.
ParsedMarkers ParseMarkers(std::string_view str);

}  // namespace tint::wgsl::ls

#endif  // SRC_TINT_LANG_WGSL_LS_HELPERS_TEST_H_
