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

struct Case {
    bool include_declaration;
    std::string_view markup;
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.markup << "'";
}

using LsReferencesTest = LsTestWithParam<Case>;
TEST_P(LsReferencesTest, References) {
    auto parsed = ParseMarkers(GetParam().markup);
    ASSERT_EQ(parsed.positions.size(), 1u);

    lsp::TextDocumentReferencesRequest req{};
    req.text_document.uri = OpenDocument(parsed.clean);
    req.context.include_declaration = GetParam().include_declaration;
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
    if (parsed.ranges.empty()) {
        ASSERT_TRUE(res.Is<lsp::Null>());
    } else {
        ASSERT_TRUE(res.Is<std::vector<lsp::Location>>());
        std::vector<lsp::Range> got_ranges;
        for (auto& location : *res.Get<std::vector<lsp::Location>>()) {
            EXPECT_EQ(location.uri, req.text_document.uri);
            got_ranges.push_back(location.range);
        }
        EXPECT_THAT(got_ranges, testing::UnorderedElementsAreArray(parsed.ranges));
    }
}

// TODO(crbug.com/tint/2127): Type aliases.
INSTANTIATE_TEST_SUITE_P(IncludeDeclaration,
                         LsReferencesTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {/* include_declaration */ true, R"(
const「CONST」= 42;
fn f() { _ =「⧘CONST」; }
const C =「CONST」;
)"},  // =========================================
                             {/* include_declaration */ true, R"(
var<private>「VAR」= 42;
fn f() { _ =「V⧘AR」; }
fn g() { _ = 「VAR」; }
)"},  // =========================================
                             {/* include_declaration */ true, R"(
override「OVERRIDE」= 42;
fn f() { _ =「OVERRID⧘E」+「OVERRIDE」; }
)"},  // =========================================
                             {/* include_declaration */ true, R"(
struct「STRUCT」{ i : i32 }
fn f(s :「⧘STRUCT」) { var v : 「STRUCT」; }
)"},  // =========================================
                             {/* include_declaration */ true, R"(
struct S {「i」: i32 }
fn f(s : S) { _ = s.「⧘i」; }
fn g(s : S) { _ = s.「i」; }
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn f(「p」: i32) { _ =「⧘p」* 「p」; }
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn f() {
    const「i」= 42;
    _ =「⧘i」*「i」;
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn f() {
    let「i」= 42;
    _ =「⧘i」+「i」;
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn f() {
    var「i」= 42;
    「i」=「⧘i」;
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn f() {
    var i = 42;
    {
        var「i」= 42;
        「i」=「⧘i」;
    }
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn f() {
    var「i」= 42;
    {
        var i = 42;
    }
    「i」=「⧘i」;
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
const i = 42;
fn f() {
    var「i」= 42;
    「i」=「⧘i」;
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
const i = 42;
fn f(「i」: i32) {
    _ =「⧘i」*「i」;
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn「a」() {}
fn b() { 「⧘a」(); }
fn c() { 「a」(); }
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn b() { 「a⧘」(); }
fn「a」() {}
fn c() { 「a」(); }
)"},  // =========================================
                             {/* include_declaration */ true, R"(
fn f() {
    let「i」= 42;
    _ = (max(「i⧘」, 「i」) * 「i」);
}
)"},  // =========================================
                             {/* include_declaration */ true, R"(
const C = m⧘ax(1, 2);
)"},  // =========================================
                             {/* include_declaration */ true, R"(
const C : i⧘32 = 42;
)"},
                         }));

INSTANTIATE_TEST_SUITE_P(ExcludeDeclaration,
                         LsReferencesTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {/* include_declaration */ false, R"(
const CONST = 42;
fn f() { _ =「⧘CONST」; }
const C =「CONST」;
)"},  // =========================================
                             {/* include_declaration */ false, R"(
var<private> VAR = 42;
fn f() { _ =「V⧘AR」; }
fn g() { _ = 「VAR」; }
)"},  // =========================================
                             {/* include_declaration */ false, R"(
override OVERRIDE = 42;
fn f() { _ =「OVERRID⧘E」+「OVERRIDE」; }
)"},  // =========================================
                             {/* include_declaration */ false, R"(
struct STRUCT { i : i32 }
fn f(s :「⧘STRUCT」) { var v : 「STRUCT」; }
)"},  // =========================================
                             {/* include_declaration */ false, R"(
struct S { i : i32 }
fn f(s : S) { _ = s.「⧘i」; }
fn g(s : S) { _ = s.「i」; }
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn f( p : i32) { _ =「⧘p」* 「p」; }
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn f() {
    const i = 42;
    _ =「⧘i」*「i」;
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn f() {
    let i = 42;
    _ =「⧘i」+「i」;
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn f() {
    var i = 42;
    「i」=「⧘i」;
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn f() {
    var i = 42;
    {
        var i = 42;
        「i」=「⧘i」;
    }
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn f() {
    var i = 42;
    {
        var i = 42;
    }
    「i」=「⧘i」;
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
const i = 42;
fn f() {
    var i = 42;
    「i」=「⧘i」;
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
const i = 42;
fn f( i : i32) {
    _ =「⧘i」*「i」;
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn a() {}
fn b() { 「⧘a」(); }
fn c() { 「a」(); }
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn b() { 「a⧘」(); }
fn a() {}
fn c() { 「a」(); }
)"},  // =========================================
                             {/* include_declaration */ false, R"(
fn f() {
    let i = 42;
    _ = (max(「i⧘」, 「i」) * 「i」);
}
)"},  // =========================================
                             {/* include_declaration */ false, R"(
const C = m⧘ax(1, 2);
)"},  // =========================================
                             {/* include_declaration */ false, R"(
const C : i⧘32 = 42;
)"},
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
