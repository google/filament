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

#include <gtest/gtest.h>
#include <algorithm>
#include <sstream>
#include <string_view>

#include "gmock/gmock.h"

#include "langsvr/lsp/comparators.h"
#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/primitives.h"
#include "langsvr/lsp/printer.h"
#include "src/tint/lang/wgsl/ls/helpers_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::ls {
namespace {

namespace lsp = langsvr::lsp;

struct Case {
    std::string_view markup;
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.markup << "'";
}

using LsRenameTest = LsTestWithParam<Case>;
TEST_P(LsRenameTest, Rename) {
    auto parsed = ParseMarkers(GetParam().markup);
    ASSERT_EQ(parsed.positions.size(), 1u);

    auto uri = OpenDocument(parsed.clean);

    for (auto& n : diagnostics_) {
        for (auto& d : n.diagnostics) {
            if (d.severity == lsp::DiagnosticSeverity::kError) {
                FAIL() << "Error: " << d.message << "\nWGSL:\n" << parsed.clean;
            }
        }
    }

    // lsp::TextDocumentPrepareRenameRequest
    {
        lsp::TextDocumentPrepareRenameRequest req{};
        req.text_document.uri = uri;
        req.position = parsed.positions[0];

        auto future = client_session_.Send(req);
        ASSERT_EQ(future, langsvr::Success);
        auto res = future->get();
        if (parsed.ranges.empty()) {
            ASSERT_TRUE(res.Is<lsp::Null>());
        } else {
            // Find the range that holds the position marker.
            auto range =
                std::find_if(parsed.ranges.begin(), parsed.ranges.end(),
                             [&](lsp::Range r) { return lsp::ContainsInclusive(r, req.position); });
            ASSERT_NE(range, parsed.ranges.end());
            ASSERT_EQ(range->start.line, range->end.line);
            auto lines = Split(parsed.clean, "\n");
            ASSERT_LT(range->start.line, lines.Length());
            auto line = lines[range->start.line];

            ASSERT_TRUE(res.Is<lsp::PrepareRenameResult>());
            auto& rename_result = *res.Get<lsp::PrepareRenameResult>();
            ASSERT_TRUE(rename_result.Is<lsp::PrepareRenamePlaceholder>());
            auto& placeholder = *rename_result.Get<lsp::PrepareRenamePlaceholder>();

            EXPECT_EQ(*range, placeholder.range);
            auto expected_placeholder =
                line.substr(range->start.character, range->end.character - range->start.character);
            EXPECT_EQ(expected_placeholder, placeholder.placeholder);
        }
    }

    // lsp::TextDocumentRenameRequest
    {
        lsp::TextDocumentRenameRequest req{};
        req.text_document.uri = uri;
        req.position = parsed.positions[0];
        req.new_name = "RENAMED";

        auto future = client_session_.Send(req);
        ASSERT_EQ(future, langsvr::Success);
        auto res = future->get();
        if (parsed.ranges.empty()) {
            ASSERT_TRUE(res.Is<lsp::Null>());
        } else {
            ASSERT_TRUE(res.Is<lsp::WorkspaceEdit>());
            auto& edit = *res.Get<lsp::WorkspaceEdit>();
            ASSERT_TRUE(edit.changes);
            // There should only be one document in the list of changes, which must be the document
            // we're renaming.
            ASSERT_EQ(edit.changes->size(), 1u);
            ASSERT_NE(edit.changes->find(uri), edit.changes->end());

            auto& uri_changes = edit.changes->at(uri);
            std::vector<lsp::Range> got_ranges;
            for (auto& text_edit : uri_changes) {
                EXPECT_EQ(text_edit.new_text, "RENAMED");
                got_ranges.push_back(text_edit.range);
            }
            EXPECT_THAT(got_ranges, testing::UnorderedElementsAreArray(parsed.ranges));
        }
    }
}

// TODO(crbug.com/tint/2127): Type aliases.
INSTANTIATE_TEST_SUITE_P(,
                         LsRenameTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {R"(
const「CONST」= 42;
fn f() { _ =「⧘CONST」; }
const C =「CONST」;
)"},  // =========================================
                             {R"(
var<private>「VAR」= 42;
fn f() { _ =「V⧘AR」; }
fn g() { _ = 「VAR」; }
)"},  // =========================================
                             {R"(
override「OVERRIDE」= 42;
fn f() { _ =「OVERRID⧘E」+「OVERRIDE」; }
)"},  // =========================================
                             {R"(
struct「STRUCT」{ i : i32 }
fn f(s :「⧘STRUCT」) { var v : 「STRUCT」; }
)"},  // =========================================
                             {R"(
struct S {「i」: i32 }
fn f(s : S) { _ = s.「⧘i」; }
fn g(s : S) { _ = s.「i」; }
)"},  // =========================================
                             {R"(
fn f(「p」: i32) { _ =「⧘p」* 「p」; }
)"},  // =========================================
                             {R"(
fn f() {
    const「i」= 42;
    _ =「⧘i」*「i」;
}
)"},  // =========================================
                             {R"(
fn f() {
    let「i」= 42;
    _ =「⧘i」+「i」;
}
)"},  // =========================================
                             {R"(
fn f() {
    var「i」= 42;
    「i」=「⧘i」;
}
)"},  // =========================================
                             {R"(
fn f() {
    var i = 42;
    {
        var「i」= 42;
        「i」=「⧘i」;
    }
}
)"},  // =========================================
                             {R"(
fn f() {
    var「i」= 42;
    {
        var i = 42;
    }
    「i」=「⧘i」;
}
)"},  // =========================================
                             {R"(
const i = 42;
fn f() {
    var「i」= 42;
    「i」=「⧘i」;
}
)"},  // =========================================
                             {R"(
const i = 42;
fn f(「i」: i32) {
    _ =「⧘i」*「i」;
}
)"},  // =========================================
                             {R"(
fn「a」() {}
fn b() { 「⧘a」(); }
fn c() { 「a」(); }
)"},  // =========================================
                             {R"(
fn b() { 「a⧘」(); }
fn「a」() {}
fn c() { 「a」(); }
)"},  // =========================================
                             {R"(
fn f() {
    let「i」= 42;
    _ = (max(「i⧘」, 「i」) * 「i」);
}
)"},  // =========================================
                             {R"(
const C = m⧘ax(1, 2);
)"},  // =========================================
                             {R"(
const C : i⧘32 = 42;
)"},
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
