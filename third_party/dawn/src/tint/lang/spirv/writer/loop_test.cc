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

TEST_F(SpirvWriterTest, Loop_BreakIf) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_BreakIf_WithRobustness) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;

    EXPECT_INST("%14 = OpConstantComposite %v2uint %uint_4294967295 %uint_4294967295");
    EXPECT_INST(R"(
          %4 = OpLabel
%tint_loop_idx = OpVariable %_ptr_Function_v2uint Function
               OpBranch %5
          %5 = OpLabel
               OpStore %tint_loop_idx %14
               OpBranch %8
          %8 = OpLabel
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
         %16 = OpLoad %v2uint %tint_loop_idx None
         %17 = OpIEqual %v2bool %16 %18
         %21 = OpAll %bool %17
               OpSelectionMerge %22 None
               OpBranchConditional %21 %23 %22
         %23 = OpLabel
               OpBranch %9
         %22 = OpLabel
               OpBranch %7
          %7 = OpLabel
         %24 = OpAccessChain %_ptr_Function_uint %tint_loop_idx %uint_0
         %27 = OpLoad %uint %24 None
%tint_low_inc = OpISub %uint %27 %uint_1
         %30 = OpAccessChain %_ptr_Function_uint %tint_loop_idx %uint_0
               OpStore %30 %tint_low_inc None
         %31 = OpIEqual %bool %tint_low_inc %uint_4294967295
 %tint_carry = OpSelect %uint %31 %uint_1 %uint_0
         %33 = OpAccessChain %_ptr_Function_uint %tint_loop_idx %uint_1
         %34 = OpLoad %uint %33 None
         %35 = OpISub %uint %34 %tint_carry
         %36 = OpAccessChain %_ptr_Function_uint %tint_loop_idx %uint_1
               OpStore %36 %35 None
               OpBranchConditional %true %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we still emit the continuing block with a back-edge, even when it is unreachable.
TEST_F(SpirvWriterTest, Loop_UnconditionalBreakInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.ExitLoop(loop);
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_ConditionalBreakInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* cond_break = b.If(true);
            b.Append(cond_break->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Append(cond_break->False(), [&] {  //
                b.ExitIf(cond_break);
            });
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %9
         %10 = OpLabel
               OpBranch %8
          %9 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_ConditionalContinueInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* cond_break = b.If(true);
            b.Append(cond_break->True(), [&] {  //
                b.Continue(loop);
            });
            b.Append(cond_break->False(), [&] {  //
                b.ExitIf(cond_break);
            });
            b.ExitLoop(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %9
         %10 = OpLabel
               OpBranch %6
          %9 = OpLabel
               OpBranch %8
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we still emit the continuing block with a back-edge, and the merge block, even when
// they are unreachable.
TEST_F(SpirvWriterTest, Loop_UnconditionalReturnInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_UseResultFromBodyInContinuing) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* result = b.Equal(ty.bool_(), 1_i, 2_i);
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, result);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
          %9 = OpIEqual %bool %int_1 %int_2
               OpBranch %6
          %6 = OpLabel
               OpBranchConditional %9 %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_NestedLoopInBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            auto* inner_loop = b.Loop();
            b.Append(inner_loop->Body(), [&] {
                b.ExitLoop(inner_loop);

                b.Append(inner_loop->Continuing(), [&] {  //
                    b.NextIteration(inner_loop);
                });
            });
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(),
                     [&] {  //
                         b.BreakIf(outer_loop, true);
                     });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %12 %10 None
               OpBranch %9
          %9 = OpLabel
               OpBranch %12
         %10 = OpLabel
               OpBranch %11
         %12 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_NestedLoopInContinuing) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(), [&] {
                auto* inner_loop = b.Loop();
                b.Append(inner_loop->Body(), [&] {
                    b.Continue(inner_loop);

                    b.Append(inner_loop->Continuing(), [&] {  //
                        b.BreakIf(inner_loop, true);
                    });
                });
                b.BreakIf(outer_loop, true);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %12 %10 None
               OpBranch %9
          %9 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpBranchConditional %true %12 %11
         %12 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we generate valid SPIR-V when there is an unreachable instruction in the body of a
// loop nested inside another loop's continuing block. SPIR-V requires that continue blocks are
// structurally post-dominated by back-edge blocks, and the presence of OpUnreachable (a function
// terminator) can trip up this validation. See crbug.com/354627692.
TEST_F(SpirvWriterTest, Loop_NestedLoopInContinuing_UnreachableInNestedBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(), [&] {
                auto* inner_loop = b.Loop();
                b.Append(inner_loop->Body(), [&] {
                    auto* ifelse = b.If(true);
                    b.Append(ifelse->True(), [&] {  //
                        b.ExitLoop(inner_loop);
                    });
                    b.Append(ifelse->False(), [&] {  //
                        b.ExitLoop(inner_loop);
                    });
                    b.Unreachable();

                    b.Append(inner_loop->Continuing(), [&] {  //
                        b.BreakIf(inner_loop, true);
                    });
                });
                b.BreakIf(outer_loop, true);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %12 %10 None
               OpBranch %9
          %9 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %true %14 %15
         %14 = OpLabel
               OpBranch %12
         %15 = OpLabel
               OpBranch %12
         %13 = OpLabel
               OpBranch %12
         %10 = OpLabel
               OpBranchConditional %true %12 %11
         %12 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_NestedLoopInContinuing_UnreachableInNestedBody_WithResults) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* outer_result = b.InstructionResult(ty.i32());
        auto* outer_loop = b.Loop();
        outer_loop->SetResult(outer_result);
        b.Append(outer_loop->Body(), [&] {
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(), [&] {
                auto* inner_result = b.InstructionResult(ty.i32());
                auto* inner_loop = b.Loop();
                inner_loop->SetResult(inner_result);
                b.Append(inner_loop->Body(), [&] {
                    auto* ifelse = b.If(true);
                    b.Append(ifelse->True(), [&] {  //
                        b.ExitLoop(inner_loop, 1_i);
                    });
                    b.Append(ifelse->False(), [&] {  //
                        b.ExitLoop(inner_loop, 2_i);
                    });
                    b.Unreachable();

                    b.Append(inner_loop->Continuing(), [&] {  //
                        b.BreakIf(inner_loop, true, Empty, 3_i);
                    });
                });
                b.BreakIf(outer_loop, true, Empty, inner_result);
            });
        });
        b.Return(func, outer_result);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranch %11
         %11 = OpLabel
               OpLoopMerge %12 %10 None
               OpBranch %9
          %9 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %true %14 %15
         %14 = OpLabel
               OpBranch %12
         %15 = OpLabel
               OpBranch %12
         %13 = OpLabel
               OpBranch %12
         %10 = OpLabel
               OpBranchConditional %true %12 %11
         %12 = OpLabel
         %18 = OpPhi %int %int_3 %10 %20 %13 %int_1 %14 %int_2 %15
               OpBranchConditional %true %8 %7
          %8 = OpLabel
         %23 = OpPhi %int %18 %12
               OpReturnValue %23
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %26
         %27 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_SingleValue) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i);
        });

        auto* loop_param = b.BlockParam(ty.i32());
        loop->Body()->SetParams({loop_param});

        b.Append(loop->Body(), [&] {
            auto* inc = b.Add(ty.i32(), loop_param, 1_i);
            b.Continue(loop, inc);
        });

        auto* cont_param = b.BlockParam(ty.i32());
        loop->Continuing()->SetParams({cont_param});
        b.Append(loop->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param, 5_i);
            b.BreakIf(loop, cmp, /* next_iter */ Vector{cont_param}, /* exit */ Empty);
        });

        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
         %14 = OpIAdd %int %11 %int_1
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %14 %6
         %15 = OpSGreaterThan %bool %13 %int_5
               OpBranchConditional %15 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_MultipleValue) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i, false);
        });

        auto* loop_param_a = b.BlockParam(ty.i32());
        auto* loop_param_b = b.BlockParam(ty.bool_());
        loop->Body()->SetParams({loop_param_a, loop_param_b});

        b.Append(loop->Body(), [&] {
            auto* inc = b.Add(ty.i32(), loop_param_a, 1_i);
            b.Continue(loop, inc, loop_param_b);
        });

        auto* cont_param_a = b.BlockParam(ty.i32());
        auto* cont_param_b = b.BlockParam(ty.bool_());
        loop->Continuing()->SetParams({cont_param_a, cont_param_b});
        b.Append(loop->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param_a, 5_i);
            auto* not_b = b.Not(ty.bool_(), cont_param_b);
            b.BreakIf(loop, cmp, b.Values(cont_param_a, not_b), Empty);
        });

        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
         %15 = OpPhi %bool %false %5 %17 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
         %18 = OpIAdd %int %11 %int_1
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %18 %6
         %19 = OpPhi %bool %15 %6
         %20 = OpSGreaterThan %bool %13 %int_5
         %17 = OpLogicalNot %bool %19
               OpBranchConditional %20 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_NestedIf) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i);
        });

        auto* loop_param = b.BlockParam(ty.i32());
        loop->Body()->SetParams({loop_param});
        b.Append(loop->Body(), [&] {
            auto* inner = b.If(true);
            inner->SetResult(b.InstructionResult(ty.i32()));
            b.Append(inner->True(), [&] {  //
                b.ExitIf(inner, 10_i);
            });
            b.Append(inner->False(), [&] {  //
                b.ExitIf(inner, 20_i);
            });
            b.Continue(loop, inner->Result());
        });

        auto* cont_param = b.BlockParam(ty.i32());
        loop->Continuing()->SetParams({cont_param});
        b.Append(loop->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param, 5_i);
            b.BreakIf(loop, cmp, /* next_iter */ Vector{cont_param}, /* exit */ Empty);
        });

        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
               OpSelectionMerge %14 None
               OpBranchConditional %true %15 %16
         %15 = OpLabel
               OpBranch %14
         %16 = OpLabel
               OpBranch %14
         %14 = OpLabel
         %19 = OpPhi %int %int_10 %15 %int_20 %16
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %19 %14
         %22 = OpSGreaterThan %bool %13 %int_5
               OpBranchConditional %22 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_NestedLoop) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* outer = b.Loop();
        b.Append(outer->Initializer(), [&] {  //
            b.NextIteration(outer, 1_i);
        });

        auto* outer_param = b.BlockParam(ty.i32());
        outer->Body()->SetParams({outer_param});
        b.Append(outer->Body(), [&] {
            auto* inner = b.Loop();
            b.Append(inner->Initializer(), [&] {  //
                b.NextIteration(inner);
            });
            b.Append(inner->Body(), [&] {  //
                b.Continue(inner);
            });
            b.Append(inner->Continuing(), [&] {  //
                b.BreakIf(inner, true);
            });

            b.Continue(outer, outer_param);
        });

        auto* cont_param = b.BlockParam(ty.i32());
        outer->Continuing()->SetParams({cont_param});
        b.Append(outer->Continuing(), [&] {
            auto* cmp = b.GreaterThan(ty.bool_(), cont_param, 5_i);
            b.BreakIf(outer, cmp, /* next_iter */ Vector{cont_param}, /* exit */ Empty);
        });

        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %8 = OpLabel
         %11 = OpPhi %int %int_1 %5 %13 %7
               OpLoopMerge %9 %7 None
               OpBranch %6
          %6 = OpLabel
               OpBranch %14
         %14 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpLoopMerge %18 %16 None
               OpBranch %15
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %true %18 %17
         %18 = OpLabel
               OpBranch %7
          %7 = OpLabel
         %13 = OpPhi %int %11 %18
         %21 = OpSGreaterThan %bool %13 %int_5
               OpBranchConditional %21 %9 %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_Phi_NestedIfWithResultAndImplicitFalse_InContinuing) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {  //
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            auto* if_ = b.If(true);
            auto* cond = b.InstructionResult(ty.bool_());
            if_->SetResult(cond);
            b.Append(if_->True(), [&] {  //
                b.ExitIf(if_, true);
            });
            b.BreakIf(loop, cond);
        });

        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST("%15 = OpUndef %bool");
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %11
         %10 = OpLabel
               OpBranch %9
         %11 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %14 = OpPhi %bool %true %10 %15 %11
               OpBranchConditional %14 %8 %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_ExitValue) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* result = b.InstructionResult(ty.i32());
        auto* loop = b.Loop();
        loop->SetResult(result);
        b.Append(loop->Body(), [&] {  //
            b.ExitLoop(loop, 42_i);
        });
        b.Return(func, result);
    });

    EXPECT_EQ(IR(), R"(
%foo = func():i32 {
  $B1: {
    %2:i32 = loop [b: $B2] {  # loop_1
      $B2: {  # body
        exit_loop 42i  # loop_1
      }
    }
    ret %2
  }
}
)");

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpBranch %8
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
          %9 = OpPhi %int %int_42 %5
               OpReturnValue %9
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Loop_ExitValue_BreakIf) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* result = b.InstructionResult(ty.i32());
        auto* loop = b.Loop();
        loop->SetResult(result);
        b.Append(loop->Body(), [&] {  //
            auto* if_ = b.If(false);
            b.Append(if_->True(), [&] {  //
                b.ExitLoop(loop, 1_i);
            });
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true, Empty, 42_i);
            });
        });
        b.Return(func, result);
    });

    EXPECT_EQ(IR(), R"(
%foo = func():i32 {
  $B1: {
    %2:i32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if false [t: $B4] {  # if_1
          $B4: {  # true
            exit_loop 1i  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true exit_loop: [ 42i ]  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret %2
  }
}
)");

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %false %10 %9
         %10 = OpLabel
               OpBranch %8
          %9 = OpLabel
               OpBranch %6
          %6 = OpLabel
               OpBranchConditional %true %8 %7
          %8 = OpLabel
         %14 = OpPhi %int %int_42 %6 %int_1 %10
               OpReturnValue %14
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
