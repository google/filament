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
    const std::vector<lsp::DocumentSymbol> symbols;
};

std::ostream& operator<<(std::ostream& stream, const Case& c) {
    return stream << "wgsl: '" << c.wgsl << "'";
}

struct Symbol : lsp::DocumentSymbol {
    explicit Symbol(std::string_view n) { name = n; }

    Symbol& Kind(lsp::SymbolKind k) {
        kind = k;
        return *this;
    }

    Symbol& Range(lsp::Uinteger start_line,
                  lsp::Uinteger start_column,
                  lsp::Uinteger end_line,
                  lsp::Uinteger end_column) {
        range = lsp::Range{{start_line, start_column}, {end_line, end_column}};
        return *this;
    }

    Symbol& SelectionRange(lsp::Uinteger start_line,
                           lsp::Uinteger start_column,
                           lsp::Uinteger end_line,
                           lsp::Uinteger end_column) {
        selection_range = lsp::Range{{start_line, start_column}, {end_line, end_column}};
        return *this;
    }
};

using LsSymbolsTest = LsTestWithParam<Case>;
TEST_P(LsSymbolsTest, Symbols) {
    lsp::TextDocumentDocumentSymbolRequest req{};
    req.text_document.uri = OpenDocument(GetParam().wgsl);
    auto future = client_session_.Send(req);
    ASSERT_EQ(future, langsvr::Success);
    auto res = future->get();
    if (GetParam().symbols.empty()) {
        ASSERT_TRUE(res.Is<lsp::Null>());
    } else {
        ASSERT_TRUE(res.Is<std::vector<lsp::DocumentSymbol>>());
        EXPECT_THAT(*res.Get<std::vector<lsp::DocumentSymbol>>(),
                    testing::ContainerEq(GetParam().symbols));
    }
}

INSTANTIATE_TEST_SUITE_P(,
                         LsSymbolsTest,
                         ::testing::ValuesIn(std::vector<Case>{
                             {
                                 "",
                                 {},
                             },
                             {
                                 ""
                                 /* 0 */ "const C = 1;\n"
                                 /* 1 */ "/* blah */ var V : i32 = 2i;\n"
                                 /* 2 */ "override O = 3f;\n",
                                 {
                                     Symbol{"C"}
                                         .Kind(lsp::SymbolKind::kConstant)
                                         .Range(0, 0, 0, 11)
                                         .SelectionRange(0, 6, 0, 7),
                                     Symbol{"V"}
                                         .Kind(lsp::SymbolKind::kVariable)
                                         .Range(1, 11, 1, 27)
                                         .SelectionRange(1, 15, 1, 16),
                                     Symbol{"O"}
                                         .Kind(lsp::SymbolKind::kVariable)
                                         .Range(2, 0, 2, 15)
                                         .SelectionRange(2, 9, 2, 10),
                                 },
                             },
                             {
                                 ""
                                 /* 0 */ "fn fa() {}\n"
                                 /* 1 */ "/* blah */ fn fb() -> i32 {\n"
                                 /* 2 */ "  return 1;\n"
                                 /* 3 */ "} // blah",
                                 {
                                     Symbol{"fa"}
                                         .Kind(lsp::SymbolKind::kFunction)
                                         .Range(0, 0, 0, 10)
                                         .SelectionRange(0, 3, 0, 5),
                                     Symbol{"fb"}
                                         .Kind(lsp::SymbolKind::kFunction)
                                         .Range(1, 11, 3, 1)
                                         .SelectionRange(1, 14, 1, 16),
                                 },
                             },
                             {
                                 ""
                                 /* 0 */ "struct s1 { i : i32 }\n"
                                 /* 1 */ "alias A = i32;\n"
                                 /* 2 */ "/* blah */ struct s2 {\n"
                                 /* 3 */ "  a : i32,\n"
                                 /* 4 */ "} // blah",
                                 {
                                     Symbol{"s1"}
                                         .Kind(lsp::SymbolKind::kStruct)
                                         .Range(0, 0, 0, 21)
                                         .SelectionRange(0, 7, 0, 9),
                                     Symbol{"A"}
                                         .Kind(lsp::SymbolKind::kObject)
                                         .Range(1, 0, 1, 13)
                                         .SelectionRange(1, 6, 1, 7),
                                     Symbol{"s2"}
                                         .Kind(lsp::SymbolKind::kStruct)
                                         .Range(2, 11, 4, 1)
                                         .SelectionRange(2, 18, 2, 20),
                                 },
                             },
                         }));

}  // namespace
}  // namespace tint::wgsl::ls
