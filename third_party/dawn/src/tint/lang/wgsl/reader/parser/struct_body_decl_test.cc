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

TEST_F(WGSLParserTest, StructBodyDecl_Parses) {
    auto p = parser("{a : i32}");

    auto& builder = p->builder();

    auto m = p->expect_struct_body_decl();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_EQ(m.value.Length(), 1u);

    const auto* mem = m.value[0];
    EXPECT_EQ(mem->name->symbol, builder.Symbols().Get("a"));
    ast::CheckIdentifier(mem->type, "i32");
    EXPECT_EQ(mem->attributes.Length(), 0u);
}

TEST_F(WGSLParserTest, StructBodyDecl_Parses_TrailingComma) {
    auto p = parser("{a : i32,}");

    auto& builder = p->builder();

    auto m = p->expect_struct_body_decl();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_EQ(m.value.Length(), 1u);

    const auto* mem = m.value[0];
    EXPECT_EQ(mem->name->symbol, builder.Symbols().Get("a"));
    ast::CheckIdentifier(mem->type, "i32");
    EXPECT_EQ(mem->attributes.Length(), 0u);
}

TEST_F(WGSLParserTest, StructBodyDecl_ParsesEmpty) {
    auto p = parser("{}");
    auto m = p->expect_struct_body_decl();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_EQ(m.value.Length(), 0u);
}

TEST_F(WGSLParserTest, StructBodyDecl_InvalidAlign) {
    auto p = parser(R"(
{
  @align(if) a : i32,
})");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "3:10: expected expression for align");
}

TEST_F(WGSLParserTest, StructBodyDecl_InvalidSize) {
    auto p = parser(R"(
{
  @size(if) a : i32,
})");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "3:9: expected expression for size");
}

TEST_F(WGSLParserTest, StructBodyDecl_MissingClosingBracket) {
    auto p = parser("{a : i32,");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "1:10: expected '}' for struct declaration");
}

TEST_F(WGSLParserTest, StructBodyDecl_InvalidToken) {
    auto p = parser(R"(
{
  a : i32,
  1.23
} )");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "4:3: expected '}' for struct declaration");
}

}  // namespace
}  // namespace tint::wgsl::reader
