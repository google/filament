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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, StructDecl_Parses) {
    auto p = parser(R"(
struct S {
  a : i32,
  b : f32,
})");
    auto s = p->struct_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(s.errored);
    EXPECT_TRUE(s.matched);
    ASSERT_NE(s.value, nullptr);
    ASSERT_EQ(s->name->symbol, p->builder().Symbols().Register("S"));
    ASSERT_EQ(s->members.Length(), 2u);
    EXPECT_EQ(s->members[0]->name->symbol, p->builder().Symbols().Register("a"));
    EXPECT_EQ(s->members[1]->name->symbol, p->builder().Symbols().Register("b"));

    EXPECT_EQ(s->source.range.begin.line, 2u);
    EXPECT_EQ(s->source.range.begin.column, 1u);
    EXPECT_EQ(s->source.range.end.line, 5u);
    EXPECT_EQ(s->source.range.end.column, 2u);

    EXPECT_EQ(s->name->source.range.begin.line, 2u);
    EXPECT_EQ(s->name->source.range.begin.column, 8u);
    EXPECT_EQ(s->name->source.range.end.line, 2u);
    EXPECT_EQ(s->name->source.range.end.column, 9u);
}

TEST_F(WGSLParserTest, StructDecl_Unicode_Parses) {
    const std::string struct_ident =  // "𝓼𝓽𝓻𝓾𝓬𝓽𝓾𝓻𝓮"
        "\xf0\x9d\x93\xbc\xf0\x9d\x93\xbd\xf0\x9d\x93\xbb\xf0\x9d\x93\xbe\xf0\x9d"
        "\x93\xac\xf0\x9d\x93\xbd\xf0\x9d\x93\xbe\xf0\x9d\x93\xbb\xf0\x9d\x93"
        "\xae";
    const std::string member_a_ident =  // "𝕞𝕖𝕞𝕓𝕖𝕣_𝕒"
        "\xf0\x9d\x95\x9e\xf0\x9d\x95\x96\xf0\x9d\x95\x9e\xf0\x9d\x95\x93\xf0\x9d"
        "\x95\x96\xf0\x9d\x95\xa3\x5f\xf0\x9d\x95\x92";
    const std::string member_b_ident =  // "𝔪𝔢𝔪𝔟𝔢𝔯_𝔟"
        "\xf0\x9d\x94\xaa\xf0\x9d\x94\xa2\xf0\x9d\x94\xaa\xf0\x9d\x94\x9f\xf0\x9d"
        "\x94\xa2\xf0\x9d\x94\xaf\x5f\xf0\x9d\x94\x9f";

    std::string src = R"(
struct $struct {
  $member_a : i32,
  $member_b : f32,
})";
    src = tint::ReplaceAll(src, "$struct", struct_ident);
    src = tint::ReplaceAll(src, "$member_a", member_a_ident);
    src = tint::ReplaceAll(src, "$member_b", member_b_ident);

    auto p = parser(src);

    auto s = p->struct_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(s.errored);
    EXPECT_TRUE(s.matched);
    ASSERT_NE(s.value, nullptr);
    ASSERT_EQ(s->name->symbol, p->builder().Symbols().Register(struct_ident));
    ASSERT_EQ(s->members.Length(), 2u);
    EXPECT_EQ(s->members[0]->name->symbol, p->builder().Symbols().Register(member_a_ident));
    EXPECT_EQ(s->members[1]->name->symbol, p->builder().Symbols().Register(member_b_ident));
}

TEST_F(WGSLParserTest, StructDecl_EmptyMembers) {
    auto p = parser("struct S {}");

    auto s = p->struct_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(s.errored);
    EXPECT_TRUE(s.matched);
    ASSERT_NE(s.value, nullptr);
    ASSERT_EQ(s->members.Length(), 0u);
}

TEST_F(WGSLParserTest, StructDecl_MissingIdent) {
    auto p = parser("struct {}");

    auto s = p->struct_decl();
    EXPECT_TRUE(s.errored);
    EXPECT_FALSE(s.matched);
    EXPECT_EQ(s.value, nullptr);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected identifier for struct declaration");
}

TEST_F(WGSLParserTest, StructDecl_MissingBracketLeft) {
    auto p = parser("struct S }");

    auto s = p->struct_decl();
    EXPECT_TRUE(s.errored);
    EXPECT_FALSE(s.matched);
    EXPECT_EQ(s.value, nullptr);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '{' for struct declaration");
}

}  // namespace
}  // namespace tint::wgsl::reader
