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

#include "gmock/gmock.h"
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_IsEmpty) {
    auto p = parser(std::vector<uint32_t>{});

    auto result = p->ConvertMemberDecoration(1, 1, nullptr, {});
    EXPECT_TRUE(result.list.IsEmpty());
    EXPECT_THAT(p->error(), Eq("malformed SPIR-V decoration: it's empty"));
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_OffsetWithoutOperand) {
    auto p = parser(std::vector<uint32_t>{});

    auto result = p->ConvertMemberDecoration(12, 13, nullptr, {uint32_t(spv::Decoration::Offset)});
    EXPECT_TRUE(result.list.IsEmpty());
    EXPECT_THAT(p->error(), Eq("malformed Offset decoration: expected 1 literal "
                               "operand, has 0: member 13 of SPIR-V type 12"));
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_OffsetWithTooManyOperands) {
    auto p = parser(std::vector<uint32_t>{});

    auto result =
        p->ConvertMemberDecoration(12, 13, nullptr, {uint32_t(spv::Decoration::Offset), 3, 4});
    EXPECT_TRUE(result.list.IsEmpty());
    EXPECT_THAT(p->error(), Eq("malformed Offset decoration: expected 1 literal "
                               "operand, has 2: member 13 of SPIR-V type 12"));
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_Offset) {
    auto p = parser(std::vector<uint32_t>{});

    auto result = p->ConvertMemberDecoration(1, 1, nullptr, {uint32_t(spv::Decoration::Offset), 8});
    ASSERT_FALSE(result.list.IsEmpty());
    EXPECT_TRUE(result.list[0]->Is<ast::StructMemberOffsetAttribute>());
    auto* offset_deco = result.list[0]->As<ast::StructMemberOffsetAttribute>();
    ASSERT_NE(offset_deco, nullptr);
    ASSERT_TRUE(offset_deco->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(offset_deco->expr->As<ast::IntLiteralExpression>()->value, 8u);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_Matrix2x2_Stride_Natural) {
    auto p = parser(std::vector<uint32_t>{});

    ast_parser::F32 f32;
    ast_parser::Matrix matrix(&f32, 2, 2);
    auto result =
        p->ConvertMemberDecoration(1, 1, &matrix, {uint32_t(spv::Decoration::MatrixStride), 8});
    ASSERT_FALSE(result.list.IsEmpty());
    EXPECT_TRUE(result.list[0]->Is<ast::StrideAttribute>());
    auto* stride_deco = result.list[0]->As<ast::StrideAttribute>();
    ASSERT_NE(stride_deco, nullptr);
    EXPECT_EQ(stride_deco->stride, 8u);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_Matrix2x2_Stride_Custom) {
    auto p = parser(std::vector<uint32_t>{});

    ast_parser::F32 f32;
    ast_parser::Matrix matrix(&f32, 2, 2);
    auto result =
        p->ConvertMemberDecoration(1, 1, &matrix, {uint32_t(spv::Decoration::MatrixStride), 16});
    ASSERT_FALSE(result.list.IsEmpty());
    EXPECT_TRUE(result.list[0]->Is<ast::StrideAttribute>());
    auto* stride_deco = result.list[0]->As<ast::StrideAttribute>();
    ASSERT_NE(stride_deco, nullptr);
    EXPECT_EQ(stride_deco->stride, 16u);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_Matrix2x4_Stride_Natural) {
    auto p = parser(std::vector<uint32_t>{});

    ast_parser::F32 f32;
    ast_parser::Matrix matrix(&f32, 2, 4);
    auto result =
        p->ConvertMemberDecoration(1, 1, &matrix, {uint32_t(spv::Decoration::MatrixStride), 16});
    ASSERT_FALSE(result.list.IsEmpty());
    EXPECT_TRUE(result.list[0]->Is<ast::StrideAttribute>());
    auto* stride_deco = result.list[0]->As<ast::StrideAttribute>();
    ASSERT_NE(stride_deco, nullptr);
    EXPECT_EQ(stride_deco->stride, 16u);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_Matrix2x4_Stride_Custom) {
    auto p = parser(std::vector<uint32_t>{});

    ast_parser::F32 f32;
    ast_parser::Matrix matrix(&f32, 2, 4);
    auto result =
        p->ConvertMemberDecoration(1, 1, &matrix, {uint32_t(spv::Decoration::MatrixStride), 64});
    ASSERT_FALSE(result.list.IsEmpty());
    EXPECT_TRUE(result.list[0]->Is<ast::StrideAttribute>());
    auto* stride_deco = result.list[0]->As<ast::StrideAttribute>();
    ASSERT_NE(stride_deco, nullptr);
    EXPECT_EQ(stride_deco->stride, 64u);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_Matrix2x3_Stride_Custom) {
    auto p = parser(std::vector<uint32_t>{});

    ast_parser::F32 f32;
    ast_parser::Matrix matrix(&f32, 2, 3);
    auto result =
        p->ConvertMemberDecoration(1, 1, &matrix, {uint32_t(spv::Decoration::MatrixStride), 32});
    ASSERT_FALSE(result.list.IsEmpty());
    EXPECT_TRUE(result.list[0]->Is<ast::StrideAttribute>());
    auto* stride_deco = result.list[0]->As<ast::StrideAttribute>();
    ASSERT_NE(stride_deco, nullptr);
    EXPECT_EQ(stride_deco->stride, 32u);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_RelaxedPrecision) {
    // WGSL does not support relaxed precision. Drop it.
    // It's functionally correct to use full precision f32 instead of
    // relaxed precision f32.
    auto p = parser(std::vector<uint32_t>{});

    auto result =
        p->ConvertMemberDecoration(1, 1, nullptr, {uint32_t(spv::Decoration::RelaxedPrecision)});
    EXPECT_TRUE(result.list.IsEmpty());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertMemberDecoration_UnhandledDecoration) {
    auto p = parser(std::vector<uint32_t>{});

    auto result = p->ConvertMemberDecoration(12, 13, nullptr, {12345678});
    EXPECT_TRUE(result.list.IsEmpty());
    EXPECT_THAT(p->error(), Eq("unhandled member decoration: 12345678 on member "
                               "13 of SPIR-V type 12"));
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
