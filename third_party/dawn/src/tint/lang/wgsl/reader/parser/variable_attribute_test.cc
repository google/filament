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

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, Attribute_Id) {
    auto p = parser("id(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(WGSLParserTest, Attribute_Id_Expression) {
    auto p = parser("id(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::BinaryExpression>());
    auto* expr = loc->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(core::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(WGSLParserTest, Attribute_Id_TrailingComma) {
    auto p = parser("id(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(WGSLParserTest, Attribute_Id_MissingLeftParen) {
    auto p = parser("id 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected '(' for id attribute");
}

TEST_F(WGSLParserTest, Attribute_Id_MissingRightParen) {
    auto p = parser("id(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: expected ')' for id attribute");
}

TEST_F(WGSLParserTest, Attribute_Id_MissingValue) {
    auto p = parser("id()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: id expects 1 argument");
}

TEST_F(WGSLParserTest, Attribute_Id_MissingInvalid) {
    auto p = parser("id(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected expression for id");
}

TEST_F(WGSLParserTest, Attribute_Location) {
    auto p = parser("location(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(WGSLParserTest, Attribute_Location_Expression) {
    auto p = parser("location(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::BinaryExpression>());
    auto* expr = loc->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(core::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(WGSLParserTest, Attribute_Location_TrailingComma) {
    auto p = parser("location(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(WGSLParserTest, Attribute_Location_MissingLeftParen) {
    auto p = parser("location 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '(' for location attribute");
}

TEST_F(WGSLParserTest, Attribute_Location_MissingRightParen) {
    auto p = parser("location(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: expected ')' for location attribute");
}

TEST_F(WGSLParserTest, Attribute_Location_MissingValue) {
    auto p = parser("location()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: location expects 1 argument");
}

TEST_F(WGSLParserTest, Attribute_Location_MissingInvalid) {
    auto p = parser("location(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected expression for location");
}

class BuiltinTest : public WGSLParserTestWithParam<core::BuiltinValue> {};

TEST_P(BuiltinTest, Attribute_Builtin) {
    auto param = GetParam();
    auto p = parser("builtin(" + tint::ToString(param) + ")");

    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::BuiltinAttribute>());

    auto* builtin = var_attr->As<ast::BuiltinAttribute>();
    EXPECT_EQ(builtin->builtin, param);
}
TEST_P(BuiltinTest, Attribute_Builtin_TrailingComma) {
    auto param = GetParam();
    auto p = parser("builtin(" + tint::ToString(param) + ",)");

    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::BuiltinAttribute>());

    auto* builtin = var_attr->As<ast::BuiltinAttribute>();
    EXPECT_EQ(builtin->builtin, param);
}
INSTANTIATE_TEST_SUITE_P(WGSLParserTest,
                         BuiltinTest,
                         testing::Values(core::BuiltinValue::kPosition,
                                         core::BuiltinValue::kVertexIndex,
                                         core::BuiltinValue::kInstanceIndex,
                                         core::BuiltinValue::kFrontFacing,
                                         core::BuiltinValue::kFragDepth,
                                         core::BuiltinValue::kLocalInvocationId,
                                         core::BuiltinValue::kLocalInvocationIndex,
                                         core::BuiltinValue::kGlobalInvocationId,
                                         core::BuiltinValue::kWorkgroupId,
                                         core::BuiltinValue::kNumWorkgroups,
                                         core::BuiltinValue::kSampleIndex,
                                         core::BuiltinValue::kSampleMask));

TEST_F(WGSLParserTest, Attribute_Builtin_MissingLeftParen) {
    auto p = parser("builtin position)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for builtin attribute");
}

TEST_F(WGSLParserTest, Attribute_Builtin_MissingRightParen) {
    auto p = parser("builtin(position");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for builtin attribute");
}

TEST_F(WGSLParserTest, Attribute_Builtin_MissingValue) {
    auto p = parser("builtin()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:9: expected builtin value name
Possible values: 'clip_distances', 'frag_depth', 'front_facing', 'global_invocation_id', 'instance_index', 'local_invocation_id', 'local_invocation_index', 'num_workgroups', 'position', 'sample_index', 'sample_mask', 'subgroup_invocation_id', 'subgroup_size', 'vertex_index', 'workgroup_id')");
}

TEST_F(WGSLParserTest, Attribute_Builtin_MisspelledValue) {
    auto p = parser("builtin(positon)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:9: expected builtin value name
Did you mean 'position'?
Possible values: 'clip_distances', 'frag_depth', 'front_facing', 'global_invocation_id', 'instance_index', 'local_invocation_id', 'local_invocation_index', 'num_workgroups', 'position', 'sample_index', 'sample_mask', 'subgroup_invocation_id', 'subgroup_size', 'vertex_index', 'workgroup_id')");
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Flat) {
    auto p = parser("interpolate(flat)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kFlat);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kUndefined);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Flat_First) {
    auto p = parser("interpolate(flat, first)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kFlat);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kFirst);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Either) {
    auto p = parser("interpolate(flat, either)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kFlat);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kEither);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Single_TrailingComma) {
    auto p = parser("interpolate(flat,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kFlat);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kUndefined);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Single_DoubleTrailingComma) {
    auto p = parser("interpolate(flat,,)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:18: expected interpolation sampling name
Possible values: 'center', 'centroid', 'either', 'first', 'sample')");
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Perspective_Center) {
    auto p = parser("interpolate(perspective, center)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kPerspective);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kCenter);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Double_TrailingComma) {
    auto p = parser("interpolate(perspective, center,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kPerspective);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kCenter);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Perspective_Centroid) {
    auto p = parser("interpolate(perspective, centroid)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kPerspective);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kCentroid);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_Linear_Sample) {
    auto p = parser("interpolate(linear, sample)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->interpolation.type, core::InterpolationType::kLinear);
    EXPECT_EQ(interp->interpolation.sampling, core::InterpolationSampling::kSample);
}

TEST_F(WGSLParserTest, Attribute_Interpolate_MissingLeftParen) {
    auto p = parser("interpolate flat)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: expected '(' for interpolate attribute");
}

TEST_F(WGSLParserTest, Attribute_Interpolate_MissingRightParen) {
    auto p = parser("interpolate(flat");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for interpolate attribute");
}

TEST_F(WGSLParserTest, Attribute_Interpolate_MissingFirstValue) {
    auto p = parser("interpolate()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:13: expected interpolation type name
Possible values: 'flat', 'linear', 'perspective')");
}

TEST_F(WGSLParserTest, Attribute_Interpolate_MisspelledType) {
    auto p = parser("interpolate(liner)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:13: expected interpolation type name
Did you mean 'linear'?
Possible values: 'flat', 'linear', 'perspective')");
}

TEST_F(WGSLParserTest, Attribute_Interpolate_MisspelledSampling) {
    auto p = parser("interpolate(linear, centre)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:21: expected interpolation sampling name
Did you mean 'center'?
Possible values: 'center', 'centroid', 'either', 'first', 'sample')");
}

TEST_F(WGSLParserTest, Attribute_Binding) {
    auto p = parser("binding(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::IntLiteralExpression>());
    auto* expr = binding->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Binding_Expression) {
    auto p = parser("binding(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::BinaryExpression>());
    auto* expr = binding->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(core::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(WGSLParserTest, Attribute_Binding_TrailingComma) {
    auto p = parser("binding(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::IntLiteralExpression>());
    auto* expr = binding->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Binding_MissingLeftParen) {
    auto p = parser("binding 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for binding attribute");
}

TEST_F(WGSLParserTest, Attribute_Binding_MissingRightParen) {
    auto p = parser("binding(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected ')' for binding attribute");
}

TEST_F(WGSLParserTest, Attribute_Binding_MissingValue) {
    auto p = parser("binding()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: binding expects 1 argument");
}

TEST_F(WGSLParserTest, Attribute_Binding_MissingInvalid) {
    auto p = parser("binding(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected expression for binding");
}

TEST_F(WGSLParserTest, Attribute_group) {
    auto p = parser("group(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::IntLiteralExpression>());
    auto* expr = group->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_group_expression) {
    auto p = parser("group(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::BinaryExpression>());
    auto* expr = group->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(core::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(WGSLParserTest, Attribute_group_TrailingComma) {
    auto p = parser("group(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::IntLiteralExpression>());
    auto* expr = group->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_Group_MissingLeftParen) {
    auto p = parser("group 2)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected '(' for group attribute");
}

TEST_F(WGSLParserTest, Attribute_Group_MissingRightParen) {
    auto p = parser("group(2");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')' for group attribute");
}

TEST_F(WGSLParserTest, Attribute_Group_MissingValue) {
    auto p = parser("group()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: group expects 1 argument");
}

TEST_F(WGSLParserTest, Attribute_Group_MissingInvalid) {
    auto p = parser("group(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected expression for group");
}

TEST_F(WGSLParserTest, Attribute_InputAttachmentIndex) {
    auto p = parser("input_attachment_index(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::InputAttachmentIndexAttribute>());

    auto* group = var_attr->As<ast::InputAttachmentIndexAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::IntLiteralExpression>());
    auto* expr = group->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(WGSLParserTest, Attribute_InputAttachmentIndex_expression) {
    auto p = parser("input_attachment_index(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::InputAttachmentIndexAttribute>());

    auto* group = var_attr->As<ast::InputAttachmentIndexAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::BinaryExpression>());
    auto* expr = group->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(core::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

}  // namespace
}  // namespace tint::wgsl::reader
