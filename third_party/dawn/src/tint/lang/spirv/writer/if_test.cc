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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

TEST_F(SpirvWriterTest, If_TrueEmpty_FalseEmpty) {
    // Spirv 1.6 requires that 'OpBranchConditional' have different labels for the true/false target
    // blocks.
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {  //
            b.ExitIf(i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %5
          %6 = OpLabel
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_FalseEmpty) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {
            b.Add(ty.i32(), 1_i, 1_i);
            b.ExitIf(i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %5
          %6 = OpLabel
         %10 = OpBitcast %uint %int_1
         %13 = OpBitcast %uint %int_1
         %14 = OpIAdd %uint %10 %13
         %15 = OpBitcast %int %14
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_TrueEmpty) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {  //
            b.ExitIf(i);
        });
        b.Append(i->False(), [&] {
            b.Add(ty.i32(), 1_i, 1_i);
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %5 %6
          %6 = OpLabel
         %10 = OpBitcast %uint %int_1
         %13 = OpBitcast %uint %int_1
         %14 = OpIAdd %uint %10 %13
         %15 = OpBitcast %int %14
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_BothBranchesReturn) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {  //
            b.Return(func);
        });
        b.Append(i->False(), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %5
          %6 = OpLabel
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_SingleValue) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResult(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i);
        });
        b.Return(func, i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %10 = OpPhi %int %int_10 %6 %int_20 %7
               OpReturnValue %10
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_SingleValue_TrueReturn) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResult(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.Return(func, 42_i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i);
        });
        b.Return(func, i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%16 = OpUndef %int");
    EXPECT_INST(R"(
               OpSelectionMerge %12 None
               OpBranchConditional %true %13 %14
         %13 = OpLabel
               OpStore %continue_execution %false None
               OpStore %return_value %int_42 None
               OpBranch %12
         %14 = OpLabel
               OpBranch %12
         %12 = OpLabel
         %15 = OpPhi %int %16 %13 %int_20 %14
         %18 = OpLoad %bool %continue_execution None
               OpSelectionMerge %19 None
               OpBranchConditional %18 %20 %19
         %20 = OpLabel
               OpStore %return_value %15 None
               OpBranch %19
         %19 = OpLabel
         %21 = OpLoad %int %return_value None
               OpReturnValue %21
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_SingleValue_FalseReturn) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResult(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Append(i->False(), [&] {  //
            b.Return(func, 42_i);
        });
        b.Return(func, i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%17 = OpUndef %int");
    EXPECT_INST(R"(
               OpSelectionMerge %12 None
               OpBranchConditional %true %13 %14
         %13 = OpLabel
               OpBranch %12
         %14 = OpLabel
               OpStore %continue_execution %false None
               OpStore %return_value %int_42 None
               OpBranch %12
         %12 = OpLabel
         %15 = OpPhi %int %int_10 %13 %17 %14
         %18 = OpLoad %bool %continue_execution None
               OpSelectionMerge %19 None
               OpBranchConditional %18 %20 %19
         %20 = OpLabel
               OpStore %return_value %15 None
               OpBranch %19
         %19 = OpLabel
         %21 = OpLoad %int %return_value None
               OpReturnValue %21
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_SingleValue_ImplicitFalse) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResult(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Return(func, i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%12 = OpUndef %int");
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %10 = OpPhi %int %int_10 %6 %12 %7
               OpReturnValue %10
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_MultipleValue_0) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(0));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %10 = OpPhi %int %int_10 %6 %int_20 %7
         %13 = OpPhi %bool %true %6 %false %7
               OpReturnValue %10
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_MultipleValue_1) {
    auto* func = b.Function("foo", ty.bool_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(1));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %10 = OpPhi %int %int_10 %6 %int_20 %7
         %13 = OpPhi %bool %true %6 %false %7
               OpReturnValue %13
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, If_Phi_Nested) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* outer = b.If(true);
        outer->SetResult(b.InstructionResult(ty.i32()));
        b.Append(outer->True(), [&] {  //
            auto* inner = b.If(true);
            inner->SetResult(b.InstructionResult(ty.i32()));
            b.Append(inner->True(), [&] {  //
                b.ExitIf(inner, 10_i);
            });
            b.Append(inner->False(), [&] {  //
                b.ExitIf(inner, 20_i);
            });
            b.ExitIf(outer, inner->Result());
        });
        b.Append(outer->False(), [&] {  //
            b.ExitIf(outer, 30_i);
        });
        b.Return(func, outer);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %7
          %6 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %true %14 %15
         %14 = OpLabel
               OpBranch %10
         %15 = OpLabel
               OpBranch %10
         %10 = OpLabel
         %13 = OpPhi %int %int_10 %14 %int_20 %15
               OpBranch %5
          %7 = OpLabel
               OpBranch %5
          %5 = OpLabel
         %11 = OpPhi %int %int_30 %7 %13 %10
               OpReturnValue %11
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
