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
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::hlsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using HlslASTPrinterTest_Binary = TestHelper;

struct BinaryData {
    const char* result;
    core::BinaryOp op;

    enum Types { All = 0b11, Integer = 0b10, Float = 0b01 };
    Types valid_for = Types::All;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using HlslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(HlslBinaryTest, Emit_f32) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Float) == 0) {
        return;
    }

    // Skip ops that are illegal for this type
    if (params.op == core::BinaryOp::kAnd || params.op == core::BinaryOp::kOr ||
        params.op == core::BinaryOp::kXor || params.op == core::BinaryOp::kShiftLeft ||
        params.op == core::BinaryOp::kShiftRight) {
        return;
    }

    GlobalVar("left", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("right", ty.f32(), core::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(HlslBinaryTest, Emit_f16) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Float) == 0) {
        return;
    }

    // Skip ops that are illegal for this type
    if (params.op == core::BinaryOp::kAnd || params.op == core::BinaryOp::kOr ||
        params.op == core::BinaryOp::kXor || params.op == core::BinaryOp::kShiftLeft ||
        params.op == core::BinaryOp::kShiftRight) {
        return;
    }

    Enable(wgsl::Extension::kF16);

    GlobalVar("left", ty.f16(), core::AddressSpace::kPrivate);
    GlobalVar("right", ty.f16(), core::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(HlslBinaryTest, Emit_u32) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Integer) == 0) {
        return;
    }

    GlobalVar("left", ty.u32(), core::AddressSpace::kPrivate);
    GlobalVar("right", ty.u32(), core::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(HlslBinaryTest, Emit_i32) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Integer) == 0) {
        return;
    }

    // Skip ops that are illegal for this type
    if (params.op == core::BinaryOp::kShiftLeft || params.op == core::BinaryOp::kShiftRight) {
        return;
    }

    GlobalVar("left", ty.i32(), core::AddressSpace::kPrivate);
    GlobalVar("right", ty.i32(), core::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    HlslASTPrinterTest,
    HlslBinaryTest,
    testing::Values(BinaryData{"(left & right)", core::BinaryOp::kAnd},
                    BinaryData{"(left | right)", core::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", core::BinaryOp::kXor},
                    BinaryData{"(left == right)", core::BinaryOp::kEqual},
                    BinaryData{"(left != right)", core::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", core::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", core::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", core::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", core::BinaryOp::kGreaterThanEqual},
                    BinaryData{"(left << right)", core::BinaryOp::kShiftLeft},
                    BinaryData{"(left >> right)", core::BinaryOp::kShiftRight},
                    BinaryData{"(left + right)", core::BinaryOp::kAdd},
                    BinaryData{"(left - right)", core::BinaryOp::kSubtract},
                    BinaryData{"(left * right)", core::BinaryOp::kMultiply},
                    // NOTE: Integer divide covered by DivOrModBy* tests below
                    BinaryData{"(left / right)", core::BinaryOp::kDivide, BinaryData::Types::Float},
                    // NOTE: Integer modulo covered by DivOrModBy* tests below
                    BinaryData{"(left % right)", core::BinaryOp::kModulo,
                               BinaryData::Types::Float}));

TEST_F(HlslASTPrinterTest_Binary, Multiply_VectorScalar_f32) {
    auto* lhs = Call<vec3<f32>>(1_f, 1_f, 1_f);
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(1.0f).xxx");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_VectorScalar_f16) {
    Enable(wgsl::Extension::kF16);

    auto* lhs = Call<vec3<f16>>(1_h, 1_h, 1_h);
    auto* rhs = Expr(1_h);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(float16_t(1.0h)).xxx");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_ScalarVector_f32) {
    auto* lhs = Expr(1_f);
    auto* rhs = Call<vec3<f32>>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(1.0f).xxx");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_ScalarVector_f16) {
    Enable(wgsl::Extension::kF16);

    auto* lhs = Expr(1_h);
    auto* rhs = Call<vec3<f16>>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(float16_t(1.0h)).xxx");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_MatrixScalar_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(mat * 1.0f)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_MatrixScalar_f16) {
    Enable(wgsl::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), core::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Expr(1_h);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(mat * float16_t(1.0h))");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_ScalarMatrix_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);
    auto* lhs = Expr(1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(1.0f * mat)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_ScalarMatrix_f16) {
    Enable(wgsl::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), core::AddressSpace::kPrivate);
    auto* lhs = Expr(1_h);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(float16_t(1.0h) * mat)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_MatrixVector_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Call<vec3<f32>>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul((1.0f).xxx, mat)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_MatrixVector_f16) {
    Enable(wgsl::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), core::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Call<vec3<f16>>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul((float16_t(1.0h)).xxx, mat)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_VectorMatrix_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);
    auto* lhs = Call<vec3<f32>>(1_f, 1_f, 1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(mat, (1.0f).xxx)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_VectorMatrix_f16) {
    Enable(wgsl::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), core::AddressSpace::kPrivate);
    auto* lhs = Call<vec3<f16>>(1_h, 1_h, 1_h);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(mat, (float16_t(1.0h)).xxx)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_MatrixMatrix_f32) {
    GlobalVar("lhs", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.mat3x3<f32>(), core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(rhs, lhs)");
}

TEST_F(HlslASTPrinterTest_Binary, Multiply_MatrixMatrix_f16) {
    Enable(wgsl::Extension::kF16);

    GlobalVar("lhs", ty.mat3x3<f16>(), core::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.mat3x3<f16>(), core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kMultiply, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(rhs, lhs)");
}

TEST_F(HlslASTPrinterTest_Binary, Logical_And) {
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kLogicalAnd, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
)");
}

TEST_F(HlslASTPrinterTest_Binary, Logical_Multi) {
    // (a && b) || (c || d)
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(
        core::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(core::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        create<ast::BinaryExpression>(core::BinaryOp::kLogicalOr, Expr("c"), Expr("d")));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp_1 = a;
if (tint_tmp_1) {
  tint_tmp_1 = b;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  bool tint_tmp_2 = c;
  if (!tint_tmp_2) {
    tint_tmp_2 = d;
  }
  tint_tmp = (tint_tmp_2);
}
)");
}

TEST_F(HlslASTPrinterTest_Binary, Logical_Or) {
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(core::BinaryOp::kLogicalOr, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp = a;
if (!tint_tmp) {
  tint_tmp = b;
}
)");
}

TEST_F(HlslASTPrinterTest_Binary, If_WithLogical) {
    // if (a && b) {
    //   return 1i;
    // } else if (b || c) {
    //   return 2i;
    // } else {
    //   return 3i;
    // }

    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate);

    auto* expr =
        If(create<ast::BinaryExpression>(core::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
           Block(Return(1_i)),
           Else(If(create<ast::BinaryExpression>(core::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Block(Return(2_i)), Else(Block(Return(3_i))))));
    Func("func", tint::Empty, ty.i32(), Vector{WrapInStatement(expr)});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
if ((tint_tmp)) {
  return 1;
} else {
  bool tint_tmp_1 = b;
  if (!tint_tmp_1) {
    tint_tmp_1 = c;
  }
  if ((tint_tmp_1)) {
    return 2;
  } else {
    return 3;
  }
}
)");
}

TEST_F(HlslASTPrinterTest_Binary, Return_WithLogical) {
    // return (a && b) || c;

    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate);

    auto* expr = Return(create<ast::BinaryExpression>(
        core::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(core::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        Expr("c")));
    Func("func", tint::Empty, ty.bool_(), Vector{WrapInStatement(expr)});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp_1 = a;
if (tint_tmp_1) {
  tint_tmp_1 = b;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  tint_tmp = c;
}
return (tint_tmp);
)");
}

TEST_F(HlslASTPrinterTest_Binary, Assign_WithLogical) {
    // a = (b || c) && d;

    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), core::AddressSpace::kPrivate);

    auto* expr =
        Assign(Expr("a"),
               create<ast::BinaryExpression>(
                   core::BinaryOp::kLogicalAnd,
                   create<ast::BinaryExpression>(core::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Expr("d")));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp_1 = b;
if (!tint_tmp_1) {
  tint_tmp_1 = c;
}
bool tint_tmp = (tint_tmp_1);
if (tint_tmp) {
  tint_tmp = d;
}
a = (tint_tmp);
)");
}

TEST_F(HlslASTPrinterTest_Binary, Decl_WithLogical) {
    // var a : bool = (b && c) || d;

    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), core::AddressSpace::kPrivate);

    auto* var =
        Var("a", ty.bool_(), core::AddressSpace::kUndefined,
            create<ast::BinaryExpression>(
                core::BinaryOp::kLogicalOr,
                create<ast::BinaryExpression>(core::BinaryOp::kLogicalAnd, Expr("b"), Expr("c")),
                Expr("d")));

    auto* decl = Decl(var);
    WrapInFunction(decl);

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(decl)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp_1 = b;
if (tint_tmp_1) {
  tint_tmp_1 = c;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  tint_tmp = d;
}
bool a = (tint_tmp);
)");
}

TEST_F(HlslASTPrinterTest_Binary, Call_WithLogical) {
    // foo(a && b, c || d, (a || c) && (b || d))

    Func("foo",
         Vector{
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
         },
         ty.void_(), tint::Empty, tint::Empty);
    GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), core::AddressSpace::kPrivate);

    Vector params{
        create<ast::BinaryExpression>(core::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        create<ast::BinaryExpression>(core::BinaryOp::kLogicalOr, Expr("c"), Expr("d")),
        create<ast::BinaryExpression>(
            core::BinaryOp::kLogicalAnd,
            create<ast::BinaryExpression>(core::BinaryOp::kLogicalOr, Expr("a"), Expr("c")),
            create<ast::BinaryExpression>(core::BinaryOp::kLogicalOr, Expr("b"), Expr("d"))),
    };

    auto* expr = CallStmt(Call("foo", params));
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
bool tint_tmp_1 = c;
if (!tint_tmp_1) {
  tint_tmp_1 = d;
}
bool tint_tmp_3 = a;
if (!tint_tmp_3) {
  tint_tmp_3 = c;
}
bool tint_tmp_2 = (tint_tmp_3);
if (tint_tmp_2) {
  bool tint_tmp_4 = b;
  if (!tint_tmp_4) {
    tint_tmp_4 = d;
  }
  tint_tmp_2 = (tint_tmp_4);
}
foo((tint_tmp), (tint_tmp_1), (tint_tmp_2));
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
