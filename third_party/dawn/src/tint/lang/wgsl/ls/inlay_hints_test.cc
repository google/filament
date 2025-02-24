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

struct Hint {
    std::string_view label;
    bool pad_left = false;
    bool pad_right = false;
};

struct Case {
    std::string_view markup;
    std::vector<Hint> hints;
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.markup << "'";
}

using LsInlayHintsTest = LsTestWithParam<Case>;
TEST_P(LsInlayHintsTest, InlayHints) {
    auto parsed = ParseMarkers(GetParam().markup);
    ASSERT_EQ(parsed.ranges.size(), 0u);
    ASSERT_EQ(parsed.positions.size(), GetParam().hints.size());

    lsp::TextDocumentInlayHintRequest req{};
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
    if (parsed.positions.empty()) {
        ASSERT_TRUE(res.Is<lsp::Null>());
    } else {
        ASSERT_TRUE(res.Is<std::vector<langsvr::lsp::InlayHint>>());
        auto& got_hints = *res.Get<std::vector<lsp::InlayHint>>();
        std::vector<lsp::InlayHint> expected_hints;
        for (size_t i = 0; i < parsed.positions.size(); i++) {
            auto& hint = GetParam().hints[i];
            lsp::InlayHint expected{};
            expected.position = parsed.positions[i];
            expected.label = lsp::String{hint.label};
            if (hint.pad_left) {
                expected.padding_left = true;
            }
            if (hint.pad_right) {
                expected.padding_right = true;
            }
            expected_hints.push_back(std::move(expected));
        }
        EXPECT_EQ(got_hints, expected_hints);
    }
}

INSTANTIATE_TEST_SUITE_P(,
                         LsInlayHintsTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {
                                 R"(
const CONST⧘ = 42;
)",
                                 {
                                     {
                                         " : abstract-int",
                                     },
                                 },
                             },  // =========================================
                             {
                                 R"(
var<private> VAR⧘ = 42;
)",
                                 {
                                     {
                                         " : i32",
                                     },
                                 },
                             },  // =========================================
                             {
                                 R"(
override OVERRIDE⧘ = 42;
)",
                                 {
                                     {
                                         " : i32",
                                     },
                                 },
                             },  // =========================================
                             {
                                 R"(
struct STRUCT {
    ⧘⧘a : i32,
    @size(32) ⧘⧘b : i32,
    ⧘⧘c : i32,
}
@group(0) @binding(0) var<storage> v : STRUCT;
)",
                                 {
                                     {"offset: 0", /* pad_left */ true, /* pad_right */ true},
                                     {"size: 4", /* pad_left */ true, /* pad_right */ true},

                                     {"offset: 4", /* pad_left */ true, /* pad_right */ true},
                                     {"size: 32", /* pad_left */ true, /* pad_right */ true},

                                     {"offset: 36", /* pad_left */ true, /* pad_right */ true},
                                     {"size: 4", /* pad_left */ true, /* pad_right */ true},
                                 },
                             },  // =========================================
                             {
                                 R"(
struct STRUCT {
    i : i32
}
)",
                                 {},
                             },  // =========================================
                             {
                                 R"(
const CONST : i32 = 42;
)",
                                 {},
                             },  // =========================================
                             {
                                 R"(
var<private> VAR : f32 = 42;
)",
                                 {},
                             },  // =========================================
                             {
                                 R"(
override OVERRIDE : u32 = 42;
)",
                                 {},
                             },  // =========================================
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
