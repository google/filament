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

#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, AttributeDecl_Parses) {
    auto p = parser("@invariant");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);
    ASSERT_EQ(attrs.value.Length(), 1u);
    auto* invariant = attrs.value[0]->As<ast::Attribute>();
    EXPECT_TRUE(invariant->Is<ast::InvariantAttribute>());
}

TEST_F(WGSLParserTest, AttributeDecl_MissingParenLeft) {
    auto p = parser("@location 1)");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), "1:11: expected '(' for location attribute");
}

TEST_F(WGSLParserTest, AttributeDecl_MissingValue) {
    auto p = parser("@location()");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), "1:2: location expects 1 argument");
}

TEST_F(WGSLParserTest, AttributeDecl_MissingParenRight) {
    auto p = parser("@location(1");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), "1:12: expected ')' for location attribute");
}

TEST_F(WGSLParserTest, AttributeDecl_Invalidattribute) {
    auto p = parser("@invalid");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
}

}  // namespace
}  // namespace tint::wgsl::reader
