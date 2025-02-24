// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using IdentifierExpressionTest = TestHelper;
using IdentifierExpressionDeathTest = IdentifierExpressionTest;

TEST_F(IdentifierExpressionTest, Creation) {
    auto* i = Expr("ident");
    EXPECT_EQ(i->identifier->symbol, Symbol(1, ID(), "ident"));
}

TEST_F(IdentifierExpressionTest, CreationTemplated) {
    auto* i = Expr(Ident("ident", true));
    EXPECT_EQ(i->identifier->symbol, Symbol(1, ID(), "ident"));
    auto* tmpl_ident = i->identifier->As<TemplatedIdentifier>();
    ASSERT_NE(tmpl_ident, nullptr);
    EXPECT_EQ(tmpl_ident->arguments.Length(), 1_u);
    EXPECT_TRUE(tmpl_ident->arguments[0]->Is<BoolLiteralExpression>());
}

TEST_F(IdentifierExpressionTest, Creation_WithSource) {
    auto* i = Expr(Source{{20, 2}}, "ident");
    EXPECT_EQ(i->identifier->symbol, Symbol(1, ID(), "ident"));

    EXPECT_EQ(i->source.range, (Source::Range{{20, 2}}));
    EXPECT_EQ(i->identifier->source.range, (Source::Range{{20, 2}}));
}

TEST_F(IdentifierExpressionDeathTest, Assert_InvalidSymbol) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Expr("");
        },
        "internal compiler error");
}

TEST_F(IdentifierExpressionDeathTest, Assert_DifferentGenerationID_Symbol) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Expr(b2.Sym("b2"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
