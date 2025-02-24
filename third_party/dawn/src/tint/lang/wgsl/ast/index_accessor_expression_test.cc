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

namespace tint::ast {
namespace {

using IndexAccessorExpressionTest = TestHelper;
using IndexAccessorExpressionDeathTest = IndexAccessorExpressionTest;

TEST_F(IndexAccessorExpressionTest, Create) {
    auto* obj = Expr("obj");
    auto* idx = Expr("idx");

    auto* exp = IndexAccessor(obj, idx);
    ASSERT_EQ(exp->object, obj);
    ASSERT_EQ(exp->index, idx);
}

TEST_F(IndexAccessorExpressionTest, CreateWithSource) {
    auto* obj = Expr("obj");
    auto* idx = Expr("idx");

    auto* exp = IndexAccessor(Source{{20, 2}}, obj, idx);
    auto src = exp->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(IndexAccessorExpressionTest, IsIndexAccessor) {
    auto* obj = Expr("obj");
    auto* idx = Expr("idx");

    auto* exp = IndexAccessor(obj, idx);
    EXPECT_TRUE(exp->Is<IndexAccessorExpression>());
}

TEST_F(IndexAccessorExpressionDeathTest, Assert_Null_Array) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.IndexAccessor(nullptr, b.Expr("idx"));
        },
        "internal compiler error");
}

TEST_F(IndexAccessorExpressionDeathTest, Assert_Null_Index) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.IndexAccessor(b.Expr("arr"), nullptr);
        },
        "internal compiler error");
}

TEST_F(IndexAccessorExpressionDeathTest, Assert_DifferentGenerationID_Array) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.IndexAccessor(b2.Expr("arr"), b1.Expr("idx"));
        },
        "internal compiler error");
}

TEST_F(IndexAccessorExpressionDeathTest, Assert_DifferentGenerationID_Index) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.IndexAccessor(b1.Expr("arr"), b2.Expr("idx"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
