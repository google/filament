// Copyright 2021 The Dawn & Tint Authors
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

namespace tint::wgsl::reader {
namespace {

using ParserImplReservedKeywordTest = WGSLParserTestWithParam<std::string>;
TEST_P(ParserImplReservedKeywordTest, Function) {
    auto name = GetParam();
    auto p = parser("fn " + name + "() {}");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, ModuleConst) {
    auto name = GetParam();
    auto p = parser("const " + name + " : i32 = 1;");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, ModuleVar) {
    auto name = GetParam();
    auto p = parser("var " + name + " : i32 = 1;");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, FunctionLet) {
    auto name = GetParam();
    auto p = parser("fn f() { let " + name + " : i32 = 1; }");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, FunctionVar) {
    auto name = GetParam();
    auto p = parser("fn f() { var " + name + " : i32 = 1; }");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, FunctionParam) {
    auto name = GetParam();
    auto p = parser("fn f(" + name + " : i32) {}");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, Struct) {
    auto name = GetParam();
    auto p = parser("struct " + name + " {};");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:8: ')" + name + R"(' is a reserved keyword
1:)" + std::to_string(9 + name.length()) +
                              R"(: statement found outside of function body)");
}
TEST_P(ParserImplReservedKeywordTest, StructMember) {
    auto name = GetParam();
    auto p = parser("struct S { " + name + " : i32, };");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:12: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, Alias) {
    auto name = GetParam();
    auto p = parser("alias " + name + " = i32;");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: '" + name + "' is a reserved keyword");
}
INSTANTIATE_TEST_SUITE_P(ParserImplReservedKeywordTest,
                         ParserImplReservedKeywordTest,
                         testing::Values("NULL",
                                         "Self",
                                         "abstract",
                                         "active",
                                         "alignas",
                                         "alignof",
                                         "as",
                                         "asm",
                                         "asm_fragment",
                                         "async",
                                         "attribute",
                                         "auto",
                                         "await",
                                         "become",
                                         "binding_array",
                                         "cast",
                                         "catch",
                                         "class",
                                         "co_await",
                                         "co_return",
                                         "co_yield",
                                         "coherent",
                                         "column_major",
                                         "common",
                                         "compile",
                                         "compile_fragment",
                                         "concept",
                                         "const_cast",
                                         "consteval",
                                         "constexpr",
                                         "constinit",
                                         "crate",
                                         "debugger",
                                         "decltype",
                                         "delete",
                                         "demote",
                                         "demote_to_helper",
                                         "do",
                                         "dynamic_cast",
                                         "enum",
                                         "explicit",
                                         "export",
                                         "extends",
                                         "extern",
                                         "external",
                                         "filter",
                                         "final",
                                         "finally",
                                         "friend",
                                         "from",
                                         "fxgroup",
                                         "get",
                                         "goto",
                                         "groupshared",
                                         "highp",
                                         "impl",
                                         "implements",
                                         "import",
                                         "inline",
                                         "instanceof",
                                         "interface",
                                         "layout",
                                         "lowp",
                                         "macro",
                                         "macro_rules",
                                         "match",
                                         "mediump",
                                         "meta",
                                         "mod",
                                         "module",
                                         "move",
                                         "mut",
                                         "mutable",
                                         "namespace",
                                         "new",
                                         "nil",
                                         "noexcept",
                                         "noinline",
                                         "nointerpolation",
                                         "non_coherent",
                                         "noncoherent",
                                         "noperspective",
                                         "null",
                                         "nullptr",
                                         "of",
                                         "operator",
                                         "package",
                                         "packoffset",
                                         "partition",
                                         "pass",
                                         "patch",
                                         "pixelfragment",
                                         "precise",
                                         "precision",
                                         "premerge",
                                         "priv",
                                         "protected",
                                         "pub",
                                         "public",
                                         "readonly",
                                         "ref",
                                         "regardless",
                                         "register",
                                         "reinterpret_cast",
                                         "resource",
                                         "restrict",
                                         "self",
                                         "set",
                                         "shared",
                                         "sizeof",
                                         "smooth",
                                         "snorm",
                                         "static",
                                         "static_assert",
                                         "static_cast",
                                         "std",
                                         "subroutine",
                                         "super",
                                         "target",
                                         "template",
                                         "this",
                                         "thread_local",
                                         "throw",
                                         "trait",
                                         "try",
                                         "typedef",
                                         "typeid",
                                         "typename",
                                         "typeof",
                                         "union",
                                         "unless",
                                         "unorm",
                                         "unsafe",
                                         "unsized",
                                         "use",
                                         "using",
                                         "varying",
                                         "virtual",
                                         "volatile",
                                         "wgsl",
                                         "where",
                                         "with",
                                         "writeonly",
                                         "yield"

                                         ));

}  // namespace
}  // namespace tint::wgsl::reader
