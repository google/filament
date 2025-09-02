// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

#include "src/tint/lang/core/ir/unary.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

/// A parameterized test case.
struct UnaryTestCase {
    /// The element type to test.
    TestElementType type;
    /// The unary operation.
    core::UnaryOp op;
    /// The expected SPIR-V instruction.
    std::string spirv_inst;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};

using Arithmetic = SpirvWriterTestWithParam<UnaryTestCase>;
TEST_P(Arithmetic, Scalar) {
    auto params = GetParam();

    auto* arg = b.FunctionParam("arg", MakeScalarType(params.type));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Unary(params.op, MakeScalarType(params.type), arg);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %" + params.spirv_type_name + " %arg");
}
TEST_P(Arithmetic, Vector) {
    auto params = GetParam();

    auto* arg = b.FunctionParam("arg", MakeVectorType(params.type));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Unary(params.op, MakeVectorType(params.type), arg);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2" + params.spirv_type_name + " %arg");
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest_Unary,
    Arithmetic,
    testing::Values(UnaryTestCase{kI32, core::UnaryOp::kComplement, "OpNot", "int"},
                    UnaryTestCase{kU32, core::UnaryOp::kComplement, "OpNot", "uint"},
                    UnaryTestCase{kF32, core::UnaryOp::kNegation, "OpFNegate", "float"},
                    UnaryTestCase{kF16, core::UnaryOp::kNegation, "OpFNegate", "half"}));

TEST_F(SpirvWriterTest, Unary_Negate_i32) {
    auto* arg = b.FunctionParam("arg", MakeVectorType(kI32));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Unary(core::UnaryOp::kNegation, MakeVectorType(kI32), arg);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
          %6 = OpTypeFunction %void %v2int
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
     %uint_1 = OpConstant %uint 1
         %13 = OpConstantComposite %v2uint %uint_1 %uint_1
         %17 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %void None %6
        %arg = OpFunctionParameter %v2int
          %7 = OpLabel
         %10 = OpBitcast %v2uint %arg
         %11 = OpNot %v2uint %10
         %12 = OpIAdd %v2uint %11 %13
         %15 = OpBitcast %v2int %12
               OpReturn
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %17
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Unary_Negate_Vector_i32) {
    auto* arg = b.FunctionParam("arg", MakeScalarType(kI32));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        auto* result = b.Unary(core::UnaryOp::kNegation, MakeScalarType(kI32), arg);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %5
        %arg = OpFunctionParameter %int
          %6 = OpLabel
          %8 = OpBitcast %uint %arg
          %9 = OpNot %uint %8
         %10 = OpIAdd %uint %9 %uint_1
         %12 = OpBitcast %int %10
               OpReturn
               OpFunctionEnd

    )");
}

}  // namespace
}  // namespace tint::spirv::writer
