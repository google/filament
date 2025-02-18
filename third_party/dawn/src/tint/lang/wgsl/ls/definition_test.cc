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
#include "src/tint/utils/text/unicode.h"

namespace tint::wgsl::ls {
namespace {

namespace lsp = langsvr::lsp;

using LsDefinitionTest = LsTestWithParam<std::string_view>;
TEST_P(LsDefinitionTest, Definition) {
    auto parsed = ParseMarkers(GetParam());
    ASSERT_EQ(parsed.positions.size(), 1u);
    ASSERT_LE(parsed.ranges.size(), 1u);

    lsp::TextDocumentDefinitionRequest req{};
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
    if (parsed.ranges.empty()) {
        ASSERT_TRUE(res.Is<lsp::Null>());
    } else {
        ASSERT_TRUE(res.Is<lsp::Definition>());
        auto definition = *res.Get<lsp::Definition>();
        ASSERT_TRUE(definition.Is<lsp::Location>());
        EXPECT_THAT(definition.Get<lsp::Location>()->uri, req.text_document.uri);
        EXPECT_THAT(definition.Get<lsp::Location>()->range, parsed.ranges[0]);
    }
}

// TODO(crbug.com/tint/2127): Type aliases.
INSTANTIATE_TEST_SUITE_P(,
                         LsDefinitionTest,
                         ::testing::ValuesIn(std::vector<std::string_view>{
                             R"(
const「CONST」= 42;
fn f() { _ = ⧘CONST; }
)",  // =========================================
                             R"(
var<private>「VAR」= 42;
fn f() { _ = V⧘AR; }
)",  // =========================================
                             R"(
override「OVERRIDE」= 42;
fn f() { _ = OVERRID⧘E; }
)",  // =========================================
                             R"(
struct「STRUCT」{ i : i32 }
fn f(s : ⧘STRUCT) {}
)",  // =========================================
                             R"(
struct S {「i」: i32 }
fn f(s : S) { _ = s.⧘i; }
)",  // =========================================
                             R"(
fn f(「p」: i32) { _ = ⧘p; }
)",  // =========================================
                             R"(
fn f() {
    const「i」= 42;
    _ = ⧘i;
}
)",  // =========================================
                             R"(
fn f() {
    let「i」= 42;
    _ = ⧘i;
}
)",  // =========================================
                             R"(
fn f() {
    var「i」= 42;
    _ = ⧘i;
}
)",  // =========================================
                             R"(
fn f() {
    var i = 42;
    {
        var「i」= 42;
        _ = ⧘i;
    }
}
)",  // =========================================
                             R"(
fn f() {
    var「i」= 42;
    {
        var i = 42;
    }
    _ = ⧘i;
}
)",  // =========================================
                             R"(
const i = 42;
fn f() {
    var「i」= 42;
    _ = ⧘i;
}
)",  // =========================================
                             R"(
const i = 42;
fn f(「i」: i32) {
    _ = ⧘i;
}
)",  // =========================================
                             R"(
fn「a」() {}
fn b() { ⧘a(); }
)",  // =========================================
                             R"(
fn b() { ⧘a(); }
fn「a」() {}
)",  // =========================================
                             R"(
fn f() {
    let「i」= 42;
    _ = (max(i⧘, 8) * 5);
}
)",  // =========================================
                             R"(
const C = m⧘ax(1, 2);
)",  // =========================================
                             R"(
const C : i⧘32 = 42;
)"}));

}  // namespace
}  // namespace tint::wgsl::ls
