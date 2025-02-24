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

using CallExpressionTest = TestHelper;
using CallExpressionDeathTest = CallExpressionTest;

TEST_F(CallExpressionTest, CreationIdentifier) {
    auto* func = Expr("func");
    tint::Vector params{
        Expr("param1"),
        Expr("param2"),
    };

    auto* stmt = Call(func, params);
    EXPECT_EQ(stmt->target, func);

    const auto& vec = stmt->args;
    ASSERT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec[0], params[0]);
    EXPECT_EQ(vec[1], params[1]);
}

TEST_F(CallExpressionTest, CreationIdentifier_WithSource) {
    auto* func = Expr("func");
    auto* stmt = Call(Source{{20, 2}}, func);
    EXPECT_EQ(stmt->target, func);

    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, CreationType) {
    auto* type = Expr(ty.f32());
    tint::Vector params{
        Expr("param1"),
        Expr("param2"),
    };

    auto* stmt = Call(type, params);
    EXPECT_EQ(stmt->target, type);

    const auto& vec = stmt->args;
    ASSERT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec[0], params[0]);
    EXPECT_EQ(vec[1], params[1]);
}

TEST_F(CallExpressionTest, CreationType_WithSource) {
    auto* type = Expr(ty.f32());
    auto* stmt = Call(Source{{20, 2}}, type);
    EXPECT_EQ(stmt->target, type);

    auto src = stmt->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(CallExpressionTest, IsCall) {
    auto* func = Expr("func");
    auto* stmt = Call(func);
    EXPECT_TRUE(stmt->Is<CallExpression>());
}

TEST_F(CallExpressionDeathTest, Assert_Null_Identifier) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Call(static_cast<Identifier*>(nullptr));
        },
        "internal compiler error");
}

TEST_F(CallExpressionDeathTest, Assert_Null_Param) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Call(b.Ident("func"), tint::Vector{
                                        b.Expr("param1"),
                                        nullptr,
                                        b.Expr("param2"),
                                    });
        },
        "internal compiler error");
}

TEST_F(CallExpressionDeathTest, Assert_DifferentGenerationID_Identifier) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Call(b2.Ident("func"));
        },
        "internal compiler error");
}

TEST_F(CallExpressionDeathTest, Assert_DifferentGenerationID_Type) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Call(b2.ty.f32());
        },
        "internal compiler error");
}

TEST_F(CallExpressionDeathTest, Assert_DifferentGenerationID_Param) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Call(b1.Ident("func"), b2.Expr("param1"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
