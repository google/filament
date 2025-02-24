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
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, Attribute_Workgroup) {
    auto p = parser("workgroup_size(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[1], nullptr);
    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_Expression) {
    auto p = parser("workgroup_size(4 + 2)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::BinaryExpression>());
    auto* expr = values[0]->As<ast::BinaryExpression>();
    EXPECT_EQ(expr->op, core::BinaryOp::kAdd);

    EXPECT_EQ(expr->lhs->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(expr->lhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(expr->rhs->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(expr->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[1], nullptr);
    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_1Param_TrailingComma) {
    auto p = parser("workgroup_size(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[1], nullptr);
    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_1Param_TrailingComma_Double) {
    auto p = parser("workgroup_size(4,,)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:18: expected expression for workgroup_size");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_2Param) {
    auto p = parser("workgroup_size(4, 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr) << p->error();
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_2Param_Expression) {
    auto p = parser("workgroup_size(4, 5 - 2)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr) << p->error();
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::BinaryExpression>());
    auto* expr = values[1]->As<ast::BinaryExpression>();
    EXPECT_EQ(expr->op, core::BinaryOp::kSubtract);

    EXPECT_EQ(expr->lhs->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(expr->lhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(expr->rhs->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(expr->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_2Param_TrailingComma) {
    auto p = parser("workgroup_size(4, 5,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr) << p->error();
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(values[2], nullptr);
}

TEST_F(WGSLParserTest, Attribute_Workgroup21Param_TrailingComma_Double) {
    auto p = parser("workgroup_size(4,5,,)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:20: expected expression for workgroup_size");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_3Param) {
    auto p = parser("workgroup_size(4, 5, 6)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->value, 6);
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_3Param_Expression) {
    auto p = parser("workgroup_size(4, 5, 6 << 1)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[2]->Is<ast::BinaryExpression>());
    auto* expr = values[2]->As<ast::BinaryExpression>();
    EXPECT_EQ(expr->op, core::BinaryOp::kShiftLeft);

    EXPECT_EQ(expr->lhs->As<ast::IntLiteralExpression>()->value, 6);
    EXPECT_EQ(expr->lhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(expr->rhs->As<ast::IntLiteralExpression>()->value, 1);
    EXPECT_EQ(expr->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_3Param_TrailingComma) {
    auto p = parser("workgroup_size(4, 5, 6,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 5);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->value, 6);
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_WithIdent) {
    auto p = parser("workgroup_size(4, height)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::WorkgroupAttribute>());

    auto values = func_attr->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_NE(values[1], nullptr);
    ast::CheckIdentifier(values[1], "height");

    ASSERT_EQ(values[2], nullptr);
}

TEST_F(WGSLParserTest, Attribute_Workgroup_TooManyValues) {
    auto p = parser("workgroup_size(1, 2, 3, 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: workgroup_size expects at most 3 arguments, got 4");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_MissingLeftParam) {
    auto p = parser("workgroup_size 4, 5, 6)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected '(' for workgroup_size attribute");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_MissingRightParam) {
    auto p = parser("workgroup_size(4, 5, 6");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:23: expected ')' for workgroup_size attribute");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_MissingValues) {
    auto p = parser("workgroup_size()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: workgroup_size expects at least 1 argument");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_Missing_X_Value) {
    auto p = parser("workgroup_size(, 2, 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:16: expected expression for workgroup_size");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_Missing_Y_Comma) {
    auto p = parser("workgroup_size(1 2, 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:18: expected ')' for workgroup_size attribute");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_Missing_Y_Value) {
    auto p = parser("workgroup_size(1, , 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:19: expected expression for workgroup_size");
}

TEST_F(WGSLParserTest, Attribute_Workgroup_Missing_Z_Comma) {
    auto p = parser("workgroup_size(1, 2 3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:21: expected ')' for workgroup_size attribute");
}

TEST_F(WGSLParserTest, Attribute_Compute) {
    auto p = parser("compute");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::StageAttribute>());
    EXPECT_EQ(func_attr->As<ast::StageAttribute>()->stage, ast::PipelineStage::kCompute);
}

TEST_F(WGSLParserTest, Attribute_Vertex) {
    auto p = parser("vertex");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::StageAttribute>());
    EXPECT_EQ(func_attr->As<ast::StageAttribute>()->stage, ast::PipelineStage::kVertex);
}

TEST_F(WGSLParserTest, Attribute_Fragment) {
    auto p = parser("fragment");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    ASSERT_TRUE(func_attr->Is<ast::StageAttribute>());
    EXPECT_EQ(func_attr->As<ast::StageAttribute>()->stage, ast::PipelineStage::kFragment);
}

TEST_F(WGSLParserTest, Attribute_MustUse) {
    auto p = parser("must_use");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    auto* func_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(func_attr, nullptr);
    EXPECT_TRUE(func_attr->Is<ast::MustUseAttribute>());
}

}  // namespace
}  // namespace tint::wgsl::reader
