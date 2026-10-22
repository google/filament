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
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, FunctionHeader) {
    auto p = parser("fn main(a : i32, b: f32)");
    auto f = p->function_header();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    ASSERT_EQ(f->params.Length(), 2u);
    EXPECT_EQ(f->params[0]->name->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(f->params[1]->name->symbol, p->builder().Symbols().Get("b"));
    EXPECT_EQ(f->return_type, nullptr);
}

TEST_F(WGSLParserTest, FunctionHeader_TrailingComma) {
    auto p = parser("fn main(a :i32,)");
    auto f = p->function_header();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    ASSERT_EQ(f->params.Length(), 1u);
    EXPECT_EQ(f->params[0]->name->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(f->return_type, nullptr);
}

TEST_F(WGSLParserTest, FunctionHeader_AttributeReturnType) {
    auto p = parser("fn main() -> @location(1) f32");
    auto f = p->function_header();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    EXPECT_EQ(f->params.Length(), 0u);
    ast::CheckIdentifier(f->return_type, "f32");
    ASSERT_EQ(f->return_type_attributes.Length(), 1u);

    auto* loc = f->return_type_attributes[0]->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc != nullptr);
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 1u);
}

TEST_F(WGSLParserTest, FunctionHeader_InvariantReturnType) {
    auto p = parser("fn main() -> @invariant f32");
    auto f = p->function_header();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    EXPECT_EQ(f->params.Length(), 0u);
    ast::CheckIdentifier(f->return_type, "f32");
    ASSERT_EQ(f->return_type_attributes.Length(), 1u);
    EXPECT_TRUE(f->return_type_attributes[0]->Is<ast::InvariantAttribute>());
}

TEST_F(WGSLParserTest, FunctionHeader_MissingIdent) {
    auto p = parser("fn ()");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected identifier for function declaration");
}

TEST_F(WGSLParserTest, FunctionHeader_InvalidIdent) {
    auto p = parser("fn 133main() -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected identifier for function declaration");
}

TEST_F(WGSLParserTest, FunctionHeader_MissingParenLeft) {
    auto p = parser("fn main) -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected '(' for function declaration");
}

TEST_F(WGSLParserTest, FunctionHeader_InvalidParamList) {
    auto p = parser("fn main(a :i32, ,) -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for function declaration");
}

TEST_F(WGSLParserTest, FunctionHeader_MissingParenRight) {
    auto p = parser("fn main( -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected ')' for function declaration");
}

TEST_F(WGSLParserTest, FunctionHeader_MissingReturnType) {
    auto p = parser("fn main() ->");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: unable to determine function return type");
}

}  // namespace
}  // namespace tint::wgsl::reader
