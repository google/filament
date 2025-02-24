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

#include <string>
#include <string_view>

#include "gmock/gmock.h"

#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/primitives.h"
#include "langsvr/lsp/printer.h"
#include "src/tint/lang/wgsl/ls/helpers_test.h"

namespace tint::wgsl::ls {
namespace {

namespace lsp = langsvr::lsp;

struct Case {
    const std::string_view wgsl;
    const std::vector<lsp::Diagnostic> diagnostics;
};

struct Diagnostic : lsp::Diagnostic {
    explicit Diagnostic(std::string_view msg) { message = msg; }

    Diagnostic& Range(lsp::Uinteger start_line,
                      lsp::Uinteger start_column,
                      lsp::Uinteger end_line,
                      lsp::Uinteger end_column) {
        range = lsp::Range{{start_line, start_column}, {end_line, end_column}};
        return *this;
    }
    Diagnostic& Severity(lsp::DiagnosticSeverity s) {
        severity = s;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.wgsl << "'";
}

using LsDiagnosticsTest = LsTestWithParam<Case>;
TEST_P(LsDiagnosticsTest, Diagnostics) {
    lsp::TextDocumentDocumentSymbolRequest req{};
    req.text_document.uri = OpenDocument(GetParam().wgsl);
    ASSERT_EQ(diagnostics_.Length(), 1u);
    auto& notification = diagnostics_[0];
    EXPECT_EQ(notification.uri, req.text_document.uri);
    EXPECT_THAT(notification.diagnostics, testing::ContainerEq(GetParam().diagnostics));
}

INSTANTIATE_TEST_SUITE_P(,
                         LsDiagnosticsTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {
                                 "",
                                 {},
                             },
                             {
                                 "unknown_symbol",
                                 {
                                     Diagnostic{"unexpected token"}
                                         .Range(0, 0, 0, 14)
                                         .Severity(lsp::DiagnosticSeverity::kError),
                                 },
                             },
                             {
                                 "fn f() { return; return; }",
                                 {
                                     Diagnostic{"code is unreachable"}
                                         .Range(0, 17, 0, 23)
                                         .Severity(lsp::DiagnosticSeverity::kWarning),
                                 },
                             },
                             {
                                 "const A = B; const B = A;",
                                 {
                                     Diagnostic{"cyclic dependency found: 'A' -> 'B' -> 'A'"}
                                         .Range(0, 0, 0, 11)
                                         .Severity(lsp::DiagnosticSeverity::kError),

                                     Diagnostic{"const 'A' references const 'B' here"}
                                         .Range(0, 10, 0, 11)
                                         .Severity(lsp::DiagnosticSeverity::kInformation),

                                     Diagnostic{"const 'B' references const 'A' here"}
                                         .Range(0, 23, 0, 24)
                                         .Severity(lsp::DiagnosticSeverity::kInformation),
                                 },
                             },
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
