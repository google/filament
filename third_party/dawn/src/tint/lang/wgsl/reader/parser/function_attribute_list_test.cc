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

#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, FunctionAttributeList_Parses) {
    auto p = parser("@workgroup_size(2) @compute");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);
    ASSERT_EQ(attrs.value.Length(), 2u);

    auto* attr_0 = attrs.value[0]->As<ast::Attribute>();
    auto* attr_1 = attrs.value[1]->As<ast::Attribute>();
    ASSERT_NE(attr_0, nullptr);
    ASSERT_NE(attr_1, nullptr);

    ASSERT_TRUE(attr_0->Is<ast::WorkgroupAttribute>());
    const ast::Expression* x = attr_0->As<ast::WorkgroupAttribute>()->x;
    ASSERT_NE(x, nullptr);
    auto* x_literal = x->As<ast::LiteralExpression>();
    ASSERT_NE(x_literal, nullptr);
    ASSERT_TRUE(x_literal->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(x_literal->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(x_literal->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
    ASSERT_TRUE(attr_1->Is<ast::StageAttribute>());
    EXPECT_EQ(attr_1->As<ast::StageAttribute>()->stage, ast::PipelineStage::kCompute);
}

TEST_F(WGSLParserTest, FunctionAttributeList_Invalid) {
    auto p = parser("@invalid");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), R"(1:2: expected attribute
Did you mean 'invariant'?
Possible values: 'align', 'binding', 'blend_src', 'builtin', 'color', 'compute', 'diagnostic', 'fragment', 'group', 'id', 'input_attachment_index', 'interpolate', 'invariant', 'location', 'must_use', 'size', 'vertex', 'workgroup_size')");
}

}  // namespace
}  // namespace tint::wgsl::reader
