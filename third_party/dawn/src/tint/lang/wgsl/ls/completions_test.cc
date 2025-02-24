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

#include <sstream>
#include <string_view>

#include "gmock/gmock.h"

#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/primitives.h"
#include "langsvr/lsp/printer.h"
#include "src/tint/lang/wgsl/ls/helpers_test.h"

namespace tint::wgsl::ls {
namespace {

namespace lsp = langsvr::lsp;

struct CompletionItem {
    std::string_view label;
    langsvr::Optional<lsp::CompletionItemKind> kind;

    bool operator==(const CompletionItem& other) const {
        return label == other.label && kind == other.kind;
    }
};

std::ostream& operator<<(std::ostream& stream, const CompletionItem& i) {
    return stream << "['" << i.label << "', " << i.kind << "]";
}

struct Case {
    std::string_view markup;
    std::vector<CompletionItem> completions;  // Subset of all the returns completions
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.markup << "'";
}

using LsCompletionsTest = LsTestWithParam<Case>;
TEST_P(LsCompletionsTest, Completions) {
    auto parsed = ParseMarkers(GetParam().markup);
    ASSERT_EQ(parsed.ranges.size(), 0u);
    ASSERT_EQ(parsed.positions.size(), 1u);

    lsp::TextDocumentCompletionRequest req{};
    req.text_document.uri = OpenDocument(parsed.clean);
    req.position = parsed.positions[0];

    for (auto& n : diagnostics_) {
        for (auto& d : n.diagnostics) {
            if (d.severity == lsp::DiagnosticSeverity::kError) {
                FAIL() << "Error: " << d.message << "\nWGSL:\n" << parsed.clean;
            }
        }
    }

    auto future = client_session_.Send(req);
    ASSERT_EQ(future, langsvr::Success);
    auto res = future->get();
    ASSERT_TRUE(res.Is<std::vector<langsvr::lsp::CompletionItem>>());
    auto& got_lsp_items = *res.Get<std::vector<lsp::CompletionItem>>();

    std::vector<CompletionItem> got_items;
    for (auto& item : got_lsp_items) {
        got_items.push_back(CompletionItem{item.label, item.kind});
    }
    EXPECT_THAT(GetParam().completions, testing::IsSubsetOf(got_items));
}

INSTANTIATE_TEST_SUITE_P(,
                         LsCompletionsTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {
                                 R"(const X⧘ = 42;)",
                                 {
                                     {"max", lsp::CompletionItemKind::kFunction},
                                 },
                             },
                             {
                                 R"(
const ABC = 42;
const XYZ = 1⧘;
)",
                                 {
                                     {"max", lsp::CompletionItemKind::kFunction},
                                     {"ABC", lsp::CompletionItemKind::kVariable},
                                 },
                             },
                             {
                                 R"(
fn ABC() { _ = 2⧘; }
fn DEF() -> i32 { return 1; }
)",
                                 {
                                     {"max", lsp::CompletionItemKind::kFunction},
                                     {"DEF", lsp::CompletionItemKind::kFunction},
                                 },
                             },
                             {
                                 R"(
fn A() {
    let XYZ = 1;
    _ = 2⧘;
}
)",
                                 {
                                     {"max", lsp::CompletionItemKind::kFunction},
                                     {"XYZ", lsp::CompletionItemKind::kVariable},
                                 },
                             },
                             {
                                 R"(
fn A(XYZ : i32) {
    _ = 2⧘;
}
)",
                                 {
                                     {"max", lsp::CompletionItemKind::kFunction},
                                     {"XYZ", lsp::CompletionItemKind::kVariable},
                                 },
                             },
                             {
                                 R"(
struct S { i : i32 }
fn f(s : i⧘32) {}
)",
                                 {
                                     {"max", lsp::CompletionItemKind::kFunction},
                                     {"S", lsp::CompletionItemKind::kStruct},
                                 },
                             },
                             {
                                 R"(
alias A = i32;
fn f(s : i⧘32) {}
)",
                                 {
                                     {"max", lsp::CompletionItemKind::kFunction},
                                     {"A", lsp::CompletionItemKind::kStruct},
                                 },
                             },
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
