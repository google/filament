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

TEST_F(WGSLParserTest, TypeDecl_ParsesType) {
    auto p = parser("alias a = i32");

    auto t = p->type_alias_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(t.errored);
    EXPECT_TRUE(t.matched);
    ASSERT_NE(t.value, nullptr);
    ASSERT_TRUE(t->Is<ast::Alias>());
    auto* alias = t->As<ast::Alias>();
    ast::CheckIdentifier(alias->type, "i32");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 14u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Parses_Ident) {
    auto p = parser("alias a = B");

    auto t = p->type_alias_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(t.errored);
    EXPECT_TRUE(t.matched);
    ASSERT_NE(t.value, nullptr);
    ASSERT_TRUE(t.value->Is<ast::Alias>());
    auto* alias = t.value->As<ast::Alias>();
    ast::CheckIdentifier(alias->name, "a");
    ast::CheckIdentifier(alias->type, "B");
    EXPECT_EQ(alias->source.range, (Source::Range{{1u, 1u}, {1u, 12u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Unicode_Parses_Ident) {
    const std::string ident =  // "𝓶𝔂_𝓽𝔂𝓹𝓮"
        "\xf0\x9d\x93\xb6\xf0\x9d\x94\x82\x5f\xf0\x9d\x93\xbd\xf0\x9d\x94\x82\xf0"
        "\x9d\x93\xb9\xf0\x9d\x93\xae";

    auto p = parser("alias " + ident + " = i32");

    auto t = p->type_alias_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(t.errored);
    EXPECT_TRUE(t.matched);
    ASSERT_NE(t.value, nullptr);
    ASSERT_TRUE(t.value->Is<ast::Alias>());
    auto* alias = t.value->As<ast::Alias>();
    ast::CheckIdentifier(alias->name, ident);
    ast::CheckIdentifier(alias->type, "i32");
    EXPECT_EQ(alias->source.range, (Source::Range{{1u, 1u}, {1u, 38u}}));
}

TEST_F(WGSLParserTest, TypeDecl_MissingIdent) {
    auto p = parser("alias = i32");
    auto t = p->type_alias_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(t.value, nullptr);
    EXPECT_EQ(p->error(), R"(1:7: expected identifier for type alias)");
}

TEST_F(WGSLParserTest, TypeDecl_InvalidIdent) {
    auto p = parser("alias 123 = i32");
    auto t = p->type_alias_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(t.value, nullptr);
    EXPECT_EQ(p->error(), R"(1:7: expected identifier for type alias)");
}

TEST_F(WGSLParserTest, TypeDecl_MissingEqual) {
    auto p = parser("alias a i32");
    auto t = p->type_alias_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(t.value, nullptr);
    EXPECT_EQ(p->error(), R"(1:9: expected '=' for type alias)");
}

}  // namespace
}  // namespace tint::wgsl::reader
