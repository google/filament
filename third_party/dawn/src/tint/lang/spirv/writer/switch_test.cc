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

TEST_F(SpirvWriterTest, Switch_Basic) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* def_case = b.DefaultCase(swtch);
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_MultipleCases) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{b.Constant(1_i)});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* case_b = b.Case(swtch, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* def_case = b.DefaultCase(swtch);
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %int_42 %5 1 %8 2 %9
          %5 = OpLabel
               OpBranch %10
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_MultipleSelectorsPerCase) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{b.Constant(1_i), b.Constant(3_i)});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* case_b = b.Case(swtch, Vector{b.Constant(2_i), b.Constant(4_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(swtch);
        });

        auto* def_case = b.Case(swtch, Vector{b.Constant(5_i), nullptr});
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %int_42 %5 1 %8 3 %8 2 %9 4 %9 5 %5
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_AllCasesReturn) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{b.Constant(1_i)});
        b.Append(case_a, [&] {  //
            b.Return(func);
        });

        auto* case_b = b.Case(swtch, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.Return(func);
        });

        auto* def_case = b.DefaultCase(swtch);
        b.Append(def_case, [&] {  //
            b.Return(func);
        });

        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %10 None
               OpSwitch %int_42 %5 1 %8 2 %9
          %5 = OpLabel
               OpBranch %10
          %8 = OpLabel
               OpBranch %10
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_ConditionalBreak) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(42_i);

        auto* case_a = b.Case(swtch, Vector{b.Constant(1_i)});
        b.Append(case_a, [&] {
            auto* cond_break = b.If(true);
            b.Append(cond_break->True(), [&] {  //
                b.ExitSwitch(swtch);
            });
            b.Append(cond_break->False(), [&] {  //
                b.ExitIf(cond_break);
            });

            b.Return(func);
        });

        auto* def_case = b.DefaultCase(swtch);
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch);
        });

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %9 None
               OpSwitch %int_42 %5 1 %8
          %5 = OpLabel
               OpBranch %9
          %8 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %true %11 %10
         %11 = OpLabel
               OpBranch %9
         %10 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_SingleValue) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(42_i);
        s->SetResult(b.InstructionResult(ty.i32()));
        auto* case_a = b.Case(s, Vector{b.Constant(1_i), nullptr});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(s, 10_i);
        });

        auto* case_b = b.Case(s, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i);
        });

        b.Return(func, s);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5 1 %5 2 %7
          %5 = OpLabel
               OpBranch %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
          %9 = OpPhi %int %int_10 %5 %int_20 %7
               OpReturnValue %9
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_SingleValue_CaseReturn) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(42_i);
        s->SetResult(b.InstructionResult(ty.i32()));
        auto* case_a = b.Case(s, Vector{b.Constant(1_i), nullptr});
        b.Append(case_a, [&] {  //
            b.Return(func, 10_i);
        });

        auto* case_b = b.Case(s, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i);
        });

        b.Return(func, s);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
%return_value = OpVariable %_ptr_Function_int Function %7
%continue_execution = OpVariable %_ptr_Function_bool Function
               OpStore %continue_execution %true
               OpSelectionMerge %15 None
               OpSwitch %int_42 %12 1 %12 2 %14
         %12 = OpLabel
               OpStore %continue_execution %false None
               OpStore %return_value %int_10 None
               OpBranch %15
         %14 = OpLabel
               OpBranch %15
         %15 = OpLabel
         %16 = OpPhi %int %17 %12 %int_20 %14
         %19 = OpLoad %bool %continue_execution None
               OpSelectionMerge %20 None
               OpBranchConditional %19 %21 %20
         %21 = OpLabel
               OpStore %return_value %16 None
               OpBranch %20
         %20 = OpLabel
         %22 = OpLoad %int %return_value None
               OpReturnValue %22
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_MultipleValue_0) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(42_i);
        s->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        auto* case_a = b.Case(s, Vector{b.Constant(1_i), nullptr});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(s, 10_i, true);
        });

        auto* case_b = b.Case(s, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i, false);
        });

        b.Return(func, s->Result(0));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5 1 %5 2 %7
          %5 = OpLabel
               OpBranch %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
          %9 = OpPhi %int %int_10 %5 %int_20 %7
         %13 = OpPhi %bool %true %5 %false %7
               OpReturnValue %9
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_MultipleValue_1) {
    auto* func = b.Function("foo", ty.bool_());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(b.Constant(42_i));
        s->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        auto* case_a = b.Case(s, Vector{b.Constant(1_i), nullptr});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(s, 10_i, true);
        });

        auto* case_b = b.Case(s, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i, false);
        });

        b.Return(func, s->Result(1));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %9 None
               OpSwitch %int_42 %5 1 %5 2 %8
          %5 = OpLabel
               OpBranch %9
          %8 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %10 = OpPhi %int %int_10 %5 %int_20 %8
         %13 = OpPhi %bool %true %5 %false %8
               OpReturnValue %13
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_NestedIf) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(42_i);
        s->SetResult(b.InstructionResult(ty.i32()));
        auto* case_a = b.Case(s, Vector{b.Constant(1_i), nullptr});
        b.Append(case_a, [&] {  //
            auto* inner = b.If(true);
            inner->SetResult(b.InstructionResult(ty.i32()));
            b.Append(inner->True(), [&] {  //
                b.ExitIf(inner, 10_i);
            });
            b.Append(inner->False(), [&] {  //
                b.ExitIf(inner, 20_i);
            });

            b.ExitSwitch(s, inner->Result());
        });

        auto* case_b = b.Case(s, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(s, 20_i);
        });

        b.Return(func, s);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5 1 %5 2 %7
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %13 %14
         %13 = OpLabel
               OpBranch %9
         %14 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %12 = OpPhi %int %int_10 %13 %int_20 %14
               OpBranch %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %10 = OpPhi %int %int_20 %7 %12 %9
               OpReturnValue %10
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Switch_Phi_NestedSwitch) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* outer = b.Switch(42_i);
        outer->SetResult(b.InstructionResult(ty.i32()));
        auto* case_a = b.Case(outer, Vector{b.Constant(1_i), nullptr});
        b.Append(case_a, [&] {  //
            auto* inner = b.Switch(42_i);
            auto* case_inner = b.Case(inner, Vector{b.Constant(2_i), nullptr});
            b.Append(case_inner, [&] {  //
                b.ExitSwitch(inner);
            });

            b.ExitSwitch(outer, 10_i);
        });

        auto* case_b = b.Case(outer, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(outer, 20_i);
        });

        b.Return(func, outer);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpSelectionMerge %8 None
               OpSwitch %int_42 %5 1 %5 2 %7
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpSwitch %int_42 %13 2 %13
         %13 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpBranch %8
          %7 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %10 = OpPhi %int %int_20 %7 %int_10 %9
               OpReturnValue %10
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
