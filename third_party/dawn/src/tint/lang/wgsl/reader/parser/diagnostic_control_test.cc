// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::wgsl::reader {
namespace {

using SeverityPair = std::pair<std::string, wgsl::DiagnosticSeverity>;
class DiagnosticControlParserTest : public WGSLParserTestWithParam<SeverityPair> {};

TEST_P(DiagnosticControlParserTest, DiagnosticControl_Name) {
    auto& params = GetParam();
    auto p = parser("(" + params.first + ", foo)");
    auto e = p->expect_diagnostic_control();
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e->severity, params.second);

    auto* r = e->rule_name;
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->category, nullptr);
    ast::CheckIdentifier(r->name, "foo");
}
TEST_P(DiagnosticControlParserTest, DiagnosticControl_CategoryAndName) {
    auto& params = GetParam();
    auto p = parser("(" + params.first + ", foo.bar)");
    auto e = p->expect_diagnostic_control();
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e->severity, params.second);

    auto* r = e->rule_name;
    ASSERT_NE(r, nullptr);
    ast::CheckIdentifier(r->category, "foo");
    ast::CheckIdentifier(r->name, "bar");
}
INSTANTIATE_TEST_SUITE_P(DiagnosticControlParserTest,
                         DiagnosticControlParserTest,
                         testing::Values(SeverityPair{"error", wgsl::DiagnosticSeverity::kError},
                                         SeverityPair{"warning",
                                                      wgsl::DiagnosticSeverity::kWarning},
                                         SeverityPair{"info", wgsl::DiagnosticSeverity::kInfo},
                                         SeverityPair{"off", wgsl::DiagnosticSeverity::kOff}));

TEST_F(WGSLParserTest, DiagnosticControl_Name_TrailingComma) {
    auto p = parser("(error, foo,)");
    auto e = p->expect_diagnostic_control();
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e->severity, wgsl::DiagnosticSeverity::kError);

    auto* r = e->rule_name;
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->category, nullptr);
    ast::CheckIdentifier(r->name, "foo");
}

TEST_F(WGSLParserTest, DiagnosticControl_CategoryAndName_TrailingComma) {
    auto p = parser("(error, foo.bar,)");
    auto e = p->expect_diagnostic_control();
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e->severity, wgsl::DiagnosticSeverity::kError);

    auto* r = e->rule_name;
    ASSERT_NE(r, nullptr);
    ast::CheckIdentifier(r->category, "foo");
    ast::CheckIdentifier(r->name, "bar");
}

TEST_F(WGSLParserTest, DiagnosticControl_MissingOpenParen) {
    auto p = parser("off, foo)");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:1: expected '(' for diagnostic control)");
}

TEST_F(WGSLParserTest, DiagnosticControl_MissingCloseParen) {
    auto p = parser("(off, foo");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:10: expected ')' for diagnostic control)");
}

TEST_F(WGSLParserTest, DiagnosticControl_MissingDiagnosticSeverity) {
    auto p = parser("(, foo");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:2: expected severity control
Possible values: 'error', 'info', 'off', 'warning')");
}

TEST_F(WGSLParserTest, DiagnosticControl_InvalidDiagnosticSeverity) {
    auto p = parser("(fatal, foo)");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:2: expected severity control
Possible values: 'error', 'info', 'off', 'warning')");
}

TEST_F(WGSLParserTest, DiagnosticControl_MissingComma) {
    auto p = parser("(off foo");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:6: expected ',' for diagnostic control)");
}

TEST_F(WGSLParserTest, DiagnosticControl_MissingRuleName) {
    auto p = parser("(off,)");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:6: expected diagnostic rule name)");
}

TEST_F(WGSLParserTest, DiagnosticControl_MissingRuleCategory) {
    auto p = parser("(off,for.foo)");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:6: expected diagnostic rule category)");
}

TEST_F(WGSLParserTest, DiagnosticControl_InvalidRuleName) {
    auto p = parser("(off, foo$bar)");
    auto e = p->expect_diagnostic_control();
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:10: invalid character found)");
}

}  // namespace
}  // namespace tint::wgsl::reader
