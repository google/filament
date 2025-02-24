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

TEST_F(WGSLParserTest, VariableIdentDecl_Parses) {
    auto p = parser("my_var : f32");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(decl.errored);
    ast::CheckIdentifier(decl->name, "my_var");
    ASSERT_NE(decl->type, nullptr);
    ast::CheckIdentifier(decl->type, "f32");

    EXPECT_EQ(decl->name->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
    EXPECT_EQ(decl->type->source.range, (Source::Range{{1u, 10u}, {1u, 13u}}));
}

TEST_F(WGSLParserTest, VariableIdentDecl_Parses_AllowInferredType) {
    auto p = parser("my_var : f32");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(decl.errored);
    ast::CheckIdentifier(decl->name, "my_var");
    ASSERT_NE(decl->type, nullptr);
    ast::CheckIdentifier(decl->type, "f32");

    EXPECT_EQ(decl->name->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
    EXPECT_EQ(decl->type->source.range, (Source::Range{{1u, 10u}, {1u, 13u}}));
}

TEST_F(WGSLParserTest, VariableIdentDecl_Inferred_Parse_Failure) {
    auto p = parser("my_var = 1.0");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:8: expected ':' for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_Inferred_Parses_AllowInferredType) {
    auto p = parser("my_var = 1.0");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(decl.errored);
    ast::CheckIdentifier(decl->name, "my_var");
    ASSERT_EQ(decl->type, nullptr);

    EXPECT_EQ(decl->name->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingIdent) {
    auto p = parser(": f32");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingIdent_AllowInferredType) {
    auto p = parser(": f32");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingType) {
    auto p = parser("my_var :");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:9: invalid type for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingType_AllowInferredType) {
    auto p = parser("my_var :");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:9: invalid type for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_InvalidIdent) {
    auto p = parser("123 : f32");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_InvalidIdent_AllowInferredType) {
    auto p = parser("123 : f32");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

}  // namespace
}  // namespace tint::wgsl::reader
