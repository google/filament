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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

struct BinaryData {
    const char* result;
    core::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    StringStream str;
    str << data.op;
    out << str.str();
    return out;
}
using WgslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(WgslBinaryTest, Emit) {
    auto params = GetParam();

    auto op_ty = [&] {
        if (params.op == core::BinaryOp::kLogicalAnd || params.op == core::BinaryOp::kLogicalOr) {
            return ty.bool_();
        } else {
            return ty.u32();
        }
    };

    GlobalVar("left", op_ty(), core::AddressSpace::kPrivate);
    GlobalVar("right", op_ty(), core::AddressSpace::kPrivate);
    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    WgslASTPrinterTest,
    WgslBinaryTest,
    testing::Values(BinaryData{"(left & right)", core::BinaryOp::kAnd},
                    BinaryData{"(left | right)", core::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", core::BinaryOp::kXor},
                    BinaryData{"(left && right)", core::BinaryOp::kLogicalAnd},
                    BinaryData{"(left || right)", core::BinaryOp::kLogicalOr},
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
                    BinaryData{"(left / right)", core::BinaryOp::kDivide},
                    BinaryData{"(left % right)", core::BinaryOp::kModulo}));

}  // namespace
}  // namespace tint::wgsl::writer
