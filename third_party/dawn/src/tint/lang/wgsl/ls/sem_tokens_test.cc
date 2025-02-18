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
#include <cstddef>
#include <sstream>
#include <string_view>

#include "gmock/gmock.h"

#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/primitives.h"
#include "langsvr/lsp/printer.h"
#include "src/tint/lang/wgsl/ls/helpers_test.h"
#include "src/tint/lang/wgsl/ls/sem_token.h"

namespace tint::wgsl::ls {
namespace {

namespace lsp = langsvr::lsp;

struct Case {
    std::string_view markup;
    std::vector<SemToken::Kind> tokens;
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.markup << "'";
}

struct RangeAndToken {
    lsp::Range range;
    SemToken::Kind token;

    bool operator==(const RangeAndToken& other) const {
        return range == other.range && token == other.token;
    }
};

std::ostream& operator<<(std::ostream& stream, const RangeAndToken& rt) {
    return stream << "\n" << SemToken::kNames[rt.token] << ": " << rt.range;
}

using LsSemTokensTest = LsTestWithParam<Case>;
TEST_P(LsSemTokensTest, SemTokens) {
    auto parsed = ParseMarkers(GetParam().markup);
    ASSERT_EQ(parsed.positions.size(), 0u);
    ASSERT_EQ(parsed.ranges.size(), GetParam().tokens.size());

    lsp::TextDocumentSemanticTokensFullRequest req{};
    req.text_document.uri = OpenDocument(parsed.clean);

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
        ASSERT_TRUE(res.Is<lsp::SemanticTokens>());
        std::vector<RangeAndToken> expect;
        for (size_t i = 0; i < parsed.ranges.size(); i++) {
            expect.push_back(RangeAndToken{parsed.ranges[i], GetParam().tokens[i]});
        }
        // https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocument_semanticTokens
        auto& data = res.Get<lsp::SemanticTokens>()->data;
        ASSERT_EQ(data.size() % 5, 0u);
        lsp::Position pos{};
        std::vector<RangeAndToken> got;
        for (size_t i = 0; i < data.size(); i += 5) {
            const auto delta_line = data[i + 0];
            const auto delta_start = data[i + 1];
            const auto length = data[i + 2];
            const auto token_type = data[i + 3];
            const auto modifiers = data[i + 4];

            pos.line += delta_line;
            pos.character = (delta_line == 0) ? (pos.character + delta_start) : delta_start;
            lsp::Range range;
            range.start = pos;
            range.end = lsp::Position{pos.line, pos.character + length};
            auto token = static_cast<SemToken::Kind>(token_type);
            ASSERT_EQ(modifiers, 0u);
            got.push_back(RangeAndToken{range, token});
        }
        EXPECT_EQ(got, expect);
    }
}

// TODO(crbug.com/tint/2127): Type aliases.
INSTANTIATE_TEST_SUITE_P(,
                         LsSemTokensTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {
                                 R"(
const「CONST」= 42;
fn 「f」() { _=「CONST」; }
const 「C」=「CONST」;
)",
                                 {
                                     /* 'CONST' */ SemToken::kVariable,
                                     /* 'f'     */ SemToken::kFunction,
                                     /* 'CONST' */ SemToken::kVariable,
                                     /* 'C'     */ SemToken::kVariable,
                                     /* 'CONST' */ SemToken::kVariable,
                                 },
                             },  // =========================================
                             {
                                 R"(
var<「private」>「VAR」= 42;
fn「f」() { _ =「VAR」; }
fn「g」() { _ = 「VAR」; }
)",
                                 {
                                     /* 'private' */ SemToken::kEnumMember,
                                     /* 'VAR'     */ SemToken::kVariable,
                                     /* 'f'       */ SemToken::kFunction,
                                     /* 'VAR'     */ SemToken::kVariable,
                                     /* 'g'       */ SemToken::kFunction,
                                     /* 'VAR'     */ SemToken::kVariable,
                                 },
                             },  // =========================================
                             {
                                 R"(
override「OVERRIDE」= 42;
fn「f」() { _ =「OVERRIDE」+「OVERRIDE」; }
)",
                                 {
                                     /* 'OVERRIDE' */ SemToken::kVariable,
                                     /* 'f'        */ SemToken::kFunction,
                                     /* 'OVERRIDE' */ SemToken::kVariable,
                                     /* 'OVERRIDE' */ SemToken::kVariable,
                                 },
                             },  // =========================================
                             {
                                 R"(
struct「STRUCT」{「i」:「i32」}
fn「f」(「s」:「STRUCT」) { var「v」:「STRUCT」; }
)",
                                 {
                                     /* 'STRUCT' */ SemToken::kType,
                                     /* 'i'      */ SemToken::kMember,
                                     /* 'i32'    */ SemToken::kType,
                                     /* 'f'      */ SemToken::kFunction,
                                     /* 's'      */ SemToken::kVariable,
                                     /* 'STRUCT' */ SemToken::kType,
                                     /* 'v'      */ SemToken::kVariable,
                                     /* 'STRUCT' */ SemToken::kType,
                                 },
                             },  // =========================================
                             {
                                 R"(
struct「S」{「i」: 「i32」 }
fn「f」(「s」 : 「S」) { _ = 「s」.「i」; }
fn「g」(「s」 : 「S」) { _ = 「s」.「i」; }
)",
                                 {
                                     /* 'S'      */ SemToken::kType,
                                     /* 'i'      */ SemToken::kMember,
                                     /* 'i32'    */ SemToken::kType,
                                     /* 'f'      */ SemToken::kFunction,
                                     /* 's'      */ SemToken::kVariable,
                                     /* 'S'      */ SemToken::kType,
                                     /* 's'      */ SemToken::kVariable,
                                     /* 'i'      */ SemToken::kMember,
                                     /* 'g'      */ SemToken::kFunction,
                                     /* 's'      */ SemToken::kVariable,
                                     /* 'S'      */ SemToken::kType,
                                     /* 's'      */ SemToken::kVariable,
                                     /* 'i'      */ SemToken::kMember,
                                 },
                             },  // =========================================
                             {
                                 R"(
fn「f」(「p」:「i32」) { _ =「p」*「p」; }
)",
                                 {
                                     /* 'f'    */ SemToken::kFunction,
                                     /* 'p'    */ SemToken::kVariable,
                                     /* 'i32'  */ SemToken::kType,
                                     /* 'p'    */ SemToken::kVariable,
                                     /* 'p'    */ SemToken::kVariable,
                                 },
                             },  // =========================================
                             {
                                 R"(
fn「f」() {
    const「i」= 42;
    _ =「i」*「i」;
}
)",
                                 {
                                     /* 'f' */ SemToken::kFunction,
                                     /* 'i' */ SemToken::kVariable,
                                     /* 'i' */ SemToken::kVariable,
                                     /* 'i' */ SemToken::kVariable,
                                 },
                             },  // =========================================
                             {
                                 R"(
fn「f」() {
    let「i」= 42;
    _ =「i」+「i」;
}
)",
                                 {
                                     /* 'f' */ SemToken::kFunction,
                                     /* 'i' */ SemToken::kVariable,
                                     /* 'i' */ SemToken::kVariable,
                                     /* 'i' */ SemToken::kVariable,
                                 },
                             },  // =========================================
                             {
                                 R"(
fn「f」() {
    var「i」= 42;
    「i」=「i」;
}
)",
                                 {
                                     /* 'f' */ SemToken::kFunction,
                                     /* 'i' */ SemToken::kVariable,
                                     /* 'i' */ SemToken::kVariable,
                                     /* 'i' */ SemToken::kVariable,
                                 },
                             },  // =========================================
                             {
                                 R"(
fn「f」() {
    let「i」= 42;
    _ = (「max」(「i」, 「i」) * 「i」);
}
)",
                                 {
                                     /* 'f'   */ SemToken::kFunction,
                                     /* 'i'   */ SemToken::kVariable,
                                     /* 'max' */ SemToken::kFunction,
                                     /* 'i'   */ SemToken::kVariable,
                                     /* 'i'   */ SemToken::kVariable,
                                     /* 'i'   */ SemToken::kVariable,
                                 },
                             },
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
