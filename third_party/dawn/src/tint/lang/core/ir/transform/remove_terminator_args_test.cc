// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_RemoveTerminatorArgsTest = TransformTest;

TEST_F(IR_RemoveTerminatorArgsTest, NoModify_If) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            b.ExitIf(ifelse);
        });
        b.Append(ifelse->False(), [&] {  //
            b.ExitIf(ifelse);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, NoModify_Switch) {
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

    auto* src = R"(
%foo = func():void {
  $B1: {
    switch 42i [c: (1i, $B2), c: (2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        exit_switch  # switch_1
      }
      $B4: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, NoModify_Loop) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {  //
            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.BreakIf(loop, true);
        });

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration  # -> $B3
      }
      $B3: {  # body
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, IfResults) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* res_a = b.InstructionResult(ty.i32());
        auto* res_b = b.InstructionResult(ty.u32());

        auto* ifelse = b.If(true);
        ifelse->SetResults(Vector{res_a, res_b});
        b.Append(ifelse->True(), [&] {  //
            b.ExitIf(ifelse, 1_i, 42_u);
        });
        b.Append(ifelse->False(), [&] {  //
            b.ExitIf(ifelse, 42_i, 1_u);
        });

        // Use the results to make sure the uses get updated.
        b.Add<i32>(res_a, 1_i);
        b.Multiply<u32>(res_b, 2_u);

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:i32, %3:u32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if 1i, 42u  # if_1
      }
      $B3: {  # false
        exit_if 42i, 1u  # if_1
      }
    }
    %4:i32 = add %2, 1i
    %5:u32 = mul %3, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:ptr<function, u32, read_write> = var
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        store %2, 1i
        store %3, 42u
        exit_if  # if_1
      }
      $B3: {  # false
        store %2, 42i
        store %3, 1u
        exit_if  # if_1
      }
    }
    %4:u32 = load %3
    %5:i32 = load %2
    %6:i32 = add %5, 1i
    %7:u32 = mul %4, 2u
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, SwitchResults) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* res_a = b.InstructionResult(ty.i32());
        auto* res_b = b.InstructionResult(ty.u32());

        auto* swtch = b.Switch(42_i);
        swtch->SetResults(Vector{res_a, res_b});

        auto* case_a = b.Case(swtch, Vector{b.Constant(1_i)});
        b.Append(case_a, [&] {  //
            b.ExitSwitch(swtch, 1_i, 2_u);
        });

        auto* case_b = b.Case(swtch, Vector{b.Constant(2_i)});
        b.Append(case_b, [&] {  //
            b.ExitSwitch(swtch, 3_i, 4_u);
        });

        auto* def_case = b.DefaultCase(swtch);
        b.Append(def_case, [&] {  //
            b.ExitSwitch(swtch, 5_i, 6_u);
        });

        // Use the results to make sure the uses get updated.
        b.Add<i32>(res_a, 1_i);
        b.Multiply<u32>(res_b, 2_u);

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:i32, %3:u32 = switch 42i [c: (1i, $B2), c: (2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        exit_switch 1i, 2u  # switch_1
      }
      $B3: {  # case
        exit_switch 3i, 4u  # switch_1
      }
      $B4: {  # case
        exit_switch 5i, 6u  # switch_1
      }
    }
    %4:i32 = add %2, 1i
    %5:u32 = mul %3, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:ptr<function, u32, read_write> = var
    switch 42i [c: (1i, $B2), c: (2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        store %2, 1i
        store %3, 2u
        exit_switch  # switch_1
      }
      $B3: {  # case
        store %2, 3i
        store %3, 4u
        exit_switch  # switch_1
      }
      $B4: {  # case
        store %2, 5i
        store %3, 6u
        exit_switch  # switch_1
      }
    }
    %4:u32 = load %3
    %5:i32 = load %2
    %6:i32 = add %5, 1i
    %7:u32 = mul %4, 2u
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, Loop_Results) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* res_a = b.InstructionResult(ty.i32());
        auto* res_b = b.InstructionResult(ty.u32());

        auto* loop = b.Loop();
        loop->SetResults(Vector{res_a, res_b});

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {  //
            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] {  //
                b.ExitLoop(loop, 1_i, 2_u);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.BreakIf(loop, true, Empty, Vector{b.Constant(3_i), b.Constant(4_u)});
        });

        // Use the results to make sure the uses get updated.
        b.Add<i32>(res_a, 1_i);
        b.Multiply<u32>(res_b, 2_u);

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:i32, %3:u32 = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration  # -> $B3
      }
      $B3: {  # body
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop 1i, 2u  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        break_if true exit_loop: [ 3i, 4u ]  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    %4:i32 = add %2, 1i
    %5:u32 = mul %3, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:ptr<function, u32, read_write> = var
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration  # -> $B3
      }
      $B3: {  # body
        if true [t: $B5] {  # if_1
          $B5: {  # true
            store %2, 1i
            store %3, 2u
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        store %2, 3i
        store %3, 4u
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    %4:u32 = load %3
    %5:i32 = load %2
    %6:i32 = add %5, 1i
    %7:u32 = mul %4, 2u
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, Loop_BodyParams) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* param_a = b.BlockParam(ty.i32());
        auto* param_b = b.BlockParam(ty.u32());

        auto* loop = b.Loop();
        loop->Body()->SetParams(Vector{param_a, param_b});

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i, 2_u);
        });
        b.Append(loop->Body(), [&] {  //
            // Use the parameters to make sure the uses get updated.
            b.Add<i32>(param_a, 1_i);
            b.Multiply<u32>(param_b, 2_u);

            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.BreakIf(loop, true, Vector{b.Constant(3_i), b.Constant(4_u)}, Empty);
        });

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration 1i, 2u  # -> $B3
      }
      $B3 (%2:i32, %3:u32): {  # body
        %4:i32 = add %2, 1i
        %5:u32 = mul %3, 2u
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        break_if true next_iteration: [ 3i, 4u ]  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %2:ptr<function, i32, read_write> = var
        store %2, 1i
        %3:ptr<function, u32, read_write> = var
        store %3, 2u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %3
        %5:i32 = load %2
        %6:i32 = add %5, 1i
        %7:u32 = mul %4, 2u
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        store %2, 3i
        store %3, 4u
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, Loop_ContinuingParams) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* param_a = b.BlockParam(ty.i32());
        auto* param_b = b.BlockParam(ty.u32());

        auto* loop = b.Loop();
        loop->Continuing()->SetParams(Vector{param_a, param_b});

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {  //
            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop, 1_i, 2_u);
        });
        b.Append(loop->Continuing(), [&] {
            // Use the parameters to make sure the uses get updated.
            b.Add<i32>(param_a, 1_i);
            b.Multiply<u32>(param_b, 2_u);

            b.BreakIf(loop, true);
        });

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration  # -> $B3
      }
      $B3: {  # body
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue 1i, 2u  # -> $B4
      }
      $B4 (%2:i32, %3:u32): {  # continuing
        %4:i32 = add %2, 1i
        %5:u32 = mul %3, 2u
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %2:ptr<function, i32, read_write> = var
        %3:ptr<function, u32, read_write> = var
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        store %2, 1i
        store %3, 2u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %4:u32 = load %3
        %5:i32 = load %2
        %6:i32 = add %5, 1i
        %7:u32 = mul %4, 2u
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, Loop_BreakIfWithTwoArgLists) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* res_a = b.InstructionResult(ty.i32());
        auto* res_b = b.InstructionResult(ty.u32());
        auto* param_a = b.BlockParam(ty.f32());
        auto* param_b = b.BlockParam(ty.i32());

        auto* loop = b.Loop();
        loop->SetResults(Vector{res_a, res_b});
        loop->Body()->SetParams(Vector{param_a, param_b});

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_f, 2_i);
        });
        b.Append(loop->Body(), [&] {
            // Use the parameters to make sure the uses get updated.
            b.Subtract<f32>(param_a, 1_f);
            b.Divide<i32>(param_b, 2_i);

            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] {  //
                b.ExitLoop(loop, 3_i, 4_u);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.BreakIf(loop, true, Vector{b.Constant(5_f), b.Constant(6_i)},
                      Vector{b.Constant(7_i), b.Constant(8_u)});
        });

        // Use the results to make sure the uses get updated.
        b.Add<i32>(res_a, 1_i);
        b.Multiply<u32>(res_b, 2_u);

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:i32, %3:u32 = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration 1.0f, 2i  # -> $B3
      }
      $B3 (%4:f32, %5:i32): {  # body
        %6:f32 = sub %4, 1.0f
        %7:i32 = div %5, 2i
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop 3i, 4u  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        break_if true next_iteration: [ 5.0f, 6i ] exit_loop: [ 7i, 8u ]  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    %8:i32 = add %2, 1i
    %9:u32 = mul %3, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:ptr<function, u32, read_write> = var
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %4:ptr<function, f32, read_write> = var
        store %4, 1.0f
        %5:ptr<function, i32, read_write> = var
        store %5, 2i
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %6:i32 = load %5
        %7:f32 = load %4
        %8:f32 = sub %7, 1.0f
        %9:i32 = div %6, 2i
        if true [t: $B5] {  # if_1
          $B5: {  # true
            store %2, 3i
            store %3, 4u
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        store %2, 7i
        store %3, 8u
        store %4, 5.0f
        store %5, 6i
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    %10:u32 = load %3
    %11:i32 = load %2
    %12:i32 = add %11, 1i
    %13:u32 = mul %10, 2u
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, UndefResults) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* res_a = b.InstructionResult(ty.i32());
        auto* res_b = b.InstructionResult(ty.u32());

        auto* ifelse = b.If(true);
        ifelse->SetResults(Vector{res_a, res_b});
        b.Append(ifelse->True(), [&] {  //
            b.ExitIf(ifelse, 1_i, nullptr);
        });
        b.Append(ifelse->False(), [&] {  //
            b.ExitIf(ifelse, nullptr, 2_u);
        });

        // Use the results to make sure the uses get updated.
        b.Add<i32>(res_a, 1_i);
        b.Multiply<u32>(res_b, 2_u);

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    %2:i32, %3:u32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if 1i, undef  # if_1
      }
      $B3: {  # false
        exit_if undef, 2u  # if_1
      }
    }
    %4:i32 = add %2, 1i
    %5:u32 = mul %3, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:ptr<function, u32, read_write> = var
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        store %2, 1i
        exit_if  # if_1
      }
      $B3: {  # false
        store %3, 2u
        exit_if  # if_1
      }
    }
    %4:u32 = load %3
    %5:i32 = load %2
    %6:i32 = add %5, 1i
    %7:u32 = mul %4, 2u
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveTerminatorArgsTest, UndefBlockParams) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* param_a = b.BlockParam(ty.i32());
        auto* param_b = b.BlockParam(ty.u32());

        auto* loop = b.Loop();
        loop->Body()->SetParams(Vector{param_a, param_b});

        b.Append(loop->Initializer(), [&] {  //
            b.NextIteration(loop, 1_i, nullptr);
        });
        b.Append(loop->Body(), [&] {  //
            // Use the parameters to make sure the uses get updated.
            b.Add<i32>(param_a, 1_i);
            b.Multiply<u32>(param_b, 2_u);

            auto* if_ = b.If(true);
            b.Append(if_->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.BreakIf(loop, true, Vector{b.Constant(3_i), nullptr}, Empty);
        });

        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration 1i, undef  # -> $B3
      }
      $B3 (%2:i32, %3:u32): {  # body
        %4:i32 = add %2, 1i
        %5:u32 = mul %3, 2u
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        break_if true next_iteration: [ 3i, undef ]  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %2:ptr<function, i32, read_write> = var
        store %2, 1i
        %3:ptr<function, u32, read_write> = var
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %3
        %5:i32 = load %2
        %6:i32 = add %5, 1i
        %7:u32 = mul %4, 2u
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        store %2, 3i
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)";

    Run(RemoveTerminatorArgs);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
