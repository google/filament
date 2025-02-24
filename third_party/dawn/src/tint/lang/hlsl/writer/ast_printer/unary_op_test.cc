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

#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::hlsl::writer {
namespace {

using HlslUnaryOpTest = TestHelper;

TEST_F(HlslUnaryOpTest, AddressOf) {
    GlobalVar("expr", ty.f32(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(core::UnaryOp::kAddressOf, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "expr");
}

TEST_F(HlslUnaryOpTest, Complement) {
    GlobalVar("expr", ty.u32(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(core::UnaryOp::kComplement, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "~(expr)");
}

TEST_F(HlslUnaryOpTest, Indirection) {
    GlobalVar("G", ty.f32(), core::AddressSpace::kPrivate);
    auto* p = Let("expr", create<ast::UnaryOpExpression>(core::UnaryOp::kAddressOf, Expr("G")));
    auto* op = create<ast::UnaryOpExpression>(core::UnaryOp::kIndirection, Expr("expr"));
    WrapInFunction(p, op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "expr");
}

TEST_F(HlslUnaryOpTest, Not) {
    GlobalVar("expr", ty.bool_(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(core::UnaryOp::kNot, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "!(expr)");
}

TEST_F(HlslUnaryOpTest, Negation) {
    GlobalVar("expr", ty.i32(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(core::UnaryOp::kNegation, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "-(expr)");
}
}  // namespace
}  // namespace tint::hlsl::writer
