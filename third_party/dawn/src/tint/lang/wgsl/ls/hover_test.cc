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
    std::string_view markup;
    std::vector<lsp::MarkedString> expected;
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.markup << "'";
}

using LsHoverTest = LsTestWithParam<Case>;
TEST_P(LsHoverTest, Hover) {
    auto parsed = ParseMarkers(GetParam().markup);
    ASSERT_EQ(parsed.positions.size(), 1u);

    lsp::TextDocumentHoverRequest req{};
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
    if (GetParam().expected.empty()) {
        ASSERT_TRUE(res.Is<lsp::Null>());
        ASSERT_TRUE(parsed.ranges.empty());
    } else {
        ASSERT_TRUE(res.Is<lsp::Hover>());
        auto& hover = *res.Get<lsp::Hover>();
        ASSERT_EQ(parsed.ranges.size(), 1u);
        ASSERT_TRUE(hover.range);
        EXPECT_EQ(*hover.range, parsed.ranges[0]);
        ASSERT_TRUE(hover.contents.Is<std::vector<lsp::MarkedString>>());
        auto& marked_strings = *hover.contents.Get<std::vector<lsp::MarkedString>>();
        EXPECT_THAT(marked_strings, testing::ContainerEq(GetParam().expected));
    }
}

// TODO(crbug.com/tint/2127): Type aliases.
INSTANTIATE_TEST_SUITE_P(
    ,
    LsHoverTest,
    ::testing::ValuesIn(std::vector<Case>{
        {
            R"(
const CONST = 42;
fn f() { _ =「⧘CONST」; }
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "const CONST : abstract-int = 42"},
            },
        },
        // =========================================
        {
            R"(
var<private> VAR = 42;
fn f() { _ = 「V⧘AR」; }
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "var VAR : i32 = 42i"},
            },
        },  // =========================================
        {
            R"(
override OVERRIDE = 42;
fn f() { _ = 「OVERRID⧘E」 ; }
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "override OVERRIDE : i32 = 42i"},
            },
        },  // =========================================
        {
            R"(
struct STRUCT { i : i32 }
fn f(s : 「⧘STRUCT」 ) {}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "struct STRUCT"},
            },
        },  // =========================================
        {
            R"(
struct S { i : i32 }
fn f(s : S) { _ = s.「⧘i」 ; }
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "i : i32"},
            },
        },  // =========================================
        {
            R"(
fn f(p : i32) { _ =「⧘p」 ; }
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "p : i32"},
            },
        },  // =========================================
        {
            R"(
fn f() {
    const i = 42;
    _ =「i⧘」;
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "const i : abstract-int = 42"},
            },
        },  // =========================================
        {
            R"(
fn f() {
    let i = 42;
    _ = 「⧘i」;
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "let i : i32 = 42i"},
            },
        },  // =========================================
        {
            R"(
fn f() {
    var i = 10 + 32;
     i =「i⧘」;
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "var i : i32 = 42i"},
            },
        },  // =========================================
        {
            R"(
fn f() {
    var i = 10;
    {
        var i = 42;
         i =「i⧘」;
    }
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "var i : i32 = 42i"},
            },
        },  // =========================================
        {
            R"(
fn f() {
    var i = 10.0;
    {
        var i = 42;
    }
    i =「i⧘」;
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "var i : f32 = 10.0f"},
            },
        },  // =========================================
        {
            R"(
const i = 42;
fn f() {
    var i = 42u;
     i =「⧘i」;
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "var i : u32 = 42u"},
            },
        },  // =========================================
        {
            R"(
const i = 42;
fn f(i : i32) {
    _ =「i⧘」;
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "i : i32"},
            },
        },  // =========================================
        {
            R"(
fn a() {}
fn b() { 「⧘a」(); }
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "fn a()"},
            },
        },  // =========================================
        {
            R"(
fn b() { 「⧘a」(); }
fn a() -> i32 { return 1; }
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "fn a() -> i32"},
            },
        },  // =========================================
        {
            R"(
fn f() {
    _ = max(3f,「⧘5」);
}
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "5.0f"},
            },
        },  // =========================================
        {
            R"(
const C =「m⧘ax」(1, 2);
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl",
                                              "max(abstract-int, abstract-int) -> abstract-int"},
            },
        },  // =========================================
        {
            R"(
const C : 「i⧘32」 = 42;
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "i32"},
            },
        },  // =========================================
        {
            R"(
const C = 「1 ⧘+ 2」;
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "3"},
            },
        },  // =========================================
        {
            R"(
const C = 「(1 + 2) ⧘* 3」;
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "9"},
            },
        },  // =========================================
        {
            R"(
const C = (「1 +⧘ 2」) * 3;
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "3"},
            },
        },  // =========================================
        {
            R"(
const C : f32 = 「(1 + 2) *⧘ 3」;
)",
            {
                lsp::MarkedStringWithLanguage{"wgsl", "9.0f"},
            },
        },
    }));

}  // namespace
}  // namespace tint::wgsl::ls
