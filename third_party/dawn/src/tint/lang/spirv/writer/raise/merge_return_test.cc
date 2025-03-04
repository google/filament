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

#include "src/tint/lang/spirv/writer/raise/merge_return.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_MergeReturnTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_MergeReturnTest, NoModify_SingleReturnInRootBlock) {
    auto* in = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({in});

    b.Append(func->Block(), [&] { b.Return(func, b.Add(ty.i32(), in, 1_i)); });

    auto* src = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %3:i32 = add %2, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, NoModify_SingleReturnInMergeBlock) {
    auto* in = b.FunctionParam(ty.i32());
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({in, cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 1_i)); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 2_i)); });

        b.Return(func, ifelse->Result(0));
    });
    auto* src = R"(
%foo = func(%2:i32, %3:bool):i32 {
  $B1: {
    %4:i32 = if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %5:i32 = add %2, 1i
        exit_if %5  # if_1
      }
      $B3: {  # false
        %6:i32 = add %2, 2i
        exit_if %6  # if_1
      }
    }
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, NoModify_SingleReturnInNestedMergeBlock) {
    auto* in = b.FunctionParam(ty.i32());
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({in, cond});

    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(in);
        b.Append(b.DefaultCase(swtch), [&] { b.ExitSwitch(swtch); });

        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.ExitLoop(l); });

        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 1_i)); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 2_i)); });

        b.Return(func, ifelse->Result(0));
    });

    auto* src = R"(
%foo = func(%2:i32, %3:bool):i32 {
  $B1: {
    switch %2 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    loop [b: $B3] {  # loop_1
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    %4:i32 = if %3 [t: $B4, f: $B5] {  # if_1
      $B4: {  # true
        %5:i32 = add %2, 1i
        exit_if %5  # if_1
      }
      $B5: {  # false
        %6:i32 = add %2, 2i
        exit_if %6  # if_1
      }
    }
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_OneSideReturns) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

        b.Return(func);
    });

    auto* src = R"(
%foo = func(%2:bool):void {
  $B1: {
    if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
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

    auto* expect = R"(
%foo = func(%2:bool):void {
  $B1: {
    if %2 [t: $B2, f: $B3] {  # if_1
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

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, NoModify_EntryPoint_IfElse_OneSideReturns) {
    auto* cond = b.FunctionParam(ty.u32());
    core::IOAttributes attr;
    attr.location = 0;
    cond->SetAttributes(attr);
    auto* func = b.ComputeFunction("entrypointfunction", 2_u, 3_u, 4_u);
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(b.Equal(ty.bool_(), cond, 0_u));
        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

        b.Return(func);
    });

    auto* src = R"(
%entrypointfunction = @compute @workgroup_size(2u, 3u, 4u) func(%2:u32 [@location(0)]):void {
  $B1: {
    %3:bool = eq %2, 0u
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
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

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

// This is the same as the above tests, but we create the return instructions in a different order
// to make sure that creation order doesn't matter.
TEST_F(SpirvWriter_MergeReturnTest, IfElse_OneSideReturns_ReturnsCreatedInDifferentOrder) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Return(func);

        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });
    });

    auto* src = R"(
%foo = func(%2:bool):void {
  $B1: {
    if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
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

    auto* expect = R"(
%foo = func(%2:bool):void {
  $B1: {
    if %2 [t: $B2, f: $B3] {  # if_1
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

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_OneSideReturns_WithValue) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func, 1_i); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

        b.Return(func, 2_i);
    });

    auto* src = R"(
%foo = func(%2:bool):i32 {
  $B1: {
    if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret 1i
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    ret 2i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        store %continue_execution, false
        store %return_value, 1i
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: $B4] {  # if_2
      $B4: {  # true
        store %return_value, 2i
        exit_if  # if_2
      }
    }
    %6:i32 = load %return_value
    ret %6
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_OneSideReturns_WithValue_MergeHasBasicBlockArguments) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.Return(func, 1_i); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, 2_i); });

        b.Return(func, ifelse->Result(0));
    });

    auto* src = R"(
%foo = func(%2:bool):i32 {
  $B1: {
    %3:i32 = if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret 1i
      }
      $B3: {  # false
        exit_if 2i  # if_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:i32 = if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        store %continue_execution, false
        store %return_value, 1i
        exit_if undef  # if_1
      }
      $B3: {  # false
        exit_if 2i  # if_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: $B4] {  # if_2
      $B4: {  # true
        store %return_value, %5
        exit_if  # if_2
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest,
       IfElse_OneSideReturns_WithValue_MergeHasUndefBasicBlockArguments) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.Return(func, 1_i); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, nullptr); });

        b.Return(func, ifelse->Result(0));
    });

    auto* src = R"(
%foo = func(%2:bool):i32 {
  $B1: {
    %3:i32 = if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret 1i
      }
      $B3: {  # false
        exit_if undef  # if_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:i32 = if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        store %continue_execution, false
        store %return_value, 1i
        exit_if undef  # if_1
      }
      $B3: {  # false
        exit_if undef  # if_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: $B4] {  # if_2
      $B4: {  # true
        store %return_value, %5
        exit_if  # if_2
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_BothSidesReturn) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.Return(func); });

        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%2:bool):void {
  $B1: {
    if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
      }
      $B3: {  # false
        ret
      }
    }
    unreachable
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):void {
  $B1: {
    if %2 [t: $B2, f: $B3] {  # if_1
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

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_BothSidesReturn_NestedInAnotherIfWithResults) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* outer = b.If(cond);
        outer->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.f32()));
        b.Append(outer->True(), [&] {
            auto* inner = b.If(cond);
            b.Append(inner->True(), [&] {  //
                b.Return(func);
            });
            b.Append(inner->False(), [&] {  //
                b.Return(func);
            });
            b.Unreachable();
        });
        b.Append(outer->False(), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%2:bool):void {
  $B1: {
    %3:i32, %4:f32 = if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        if %2 [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            ret
          }
          $B5: {  # false
            ret
          }
        }
        unreachable
      }
      $B3: {  # false
        ret
      }
    }
    unreachable
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):void {
  $B1: {
    %3:i32, %4:f32 = if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        if %2 [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            exit_if  # if_2
          }
          $B5: {  # false
            exit_if  # if_2
          }
        }
        exit_if undef, undef  # if_1
      }
      $B3: {  # false
        exit_if undef, undef  # if_1
      }
    }
    ret
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_BothSidesReturn_NestedInLoop) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* inner = b.If(cond);
            b.Append(inner->True(), [&] {  //
                b.Return(func);
            });
            b.Append(inner->False(), [&] {  //
                b.Return(func);
            });
            b.Unreachable();
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%2:bool):void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        if %2 [t: $B3, f: $B4] {  # if_1
          $B3: {  # true
            ret
          }
          $B4: {  # false
            ret
          }
        }
        unreachable
      }
    }
    unreachable
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        if %2 [t: $B3, f: $B4] {  # if_1
          $B3: {  # true
            exit_if  # if_1
          }
          $B4: {  # false
            exit_if  # if_1
          }
        }
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_ThenStatements) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

        b.Store(global, 42_i);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void {
  $B2: {
    if %3 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        ret
      }
      $B4: {  # false
        exit_if  # if_1
      }
    }
    store %1, 42i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void {
  $B2: {
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %3 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
      $B4: {  # false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: $B5] {  # if_2
      $B5: {  # true
        store %1, 42i
        exit_if  # if_2
      }
    }
    ret
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

// This is the same as the above tests, but we create the return instructions in a different order
// to make sure that creation order doesn't matter.
TEST_F(SpirvWriter_MergeReturnTest, IfElse_ThenStatements_ReturnsCreatedInDifferentOrder) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Store(global, 42_i);
        b.Return(func);

        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void {
  $B2: {
    if %3 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        ret
      }
      $B4: {  # false
        exit_if  # if_1
      }
    }
    store %1, 42i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void {
  $B2: {
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %3 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
      $B4: {  # false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: $B5] {  # if_2
      $B5: {  # true
        store %1, 42i
        exit_if  # if_2
      }
    }
    ret
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_Nested) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* func = b.Function("foo", ty.i32());
    auto* condA = b.FunctionParam("condA", ty.bool_());
    auto* condB = b.FunctionParam("condB", ty.bool_());
    auto* condC = b.FunctionParam("condC", ty.bool_());
    func->SetParams({condA, condB, condC});

    b.Append(func->Block(), [&] {
        auto* ifelse_outer = b.If(condA);
        b.Append(ifelse_outer->True(), [&] { b.Return(func, 3_i); });
        b.Append(ifelse_outer->False(), [&] {
            auto* ifelse_middle = b.If(condB);
            b.Append(ifelse_middle->True(), [&] {
                auto* ifelse_inner = b.If(condC);
                b.Append(ifelse_inner->True(), [&] { b.Return(func, 1_i); });
                b.Append(ifelse_inner->False(), [&] { b.ExitIf(ifelse_inner); });

                b.Store(global, 1_i);
                b.Return(func, 2_i);
            });
            b.Append(ifelse_middle->False(), [&] { b.ExitIf(ifelse_middle); });
            b.Store(global, 2_i);
            b.ExitIf(ifelse_outer);
        });
        b.Store(global, 3_i);
        b.Return(func, b.Add(ty.i32(), 5_i, 6_i));
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 {
  $B2: {
    if %condA [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        ret 3i
      }
      $B4: {  # false
        if %condB [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            if %condC [t: $B7, f: $B8] {  # if_3
              $B7: {  # true
                ret 1i
              }
              $B8: {  # false
                exit_if  # if_3
              }
            }
            store %1, 1i
            ret 2i
          }
          $B6: {  # false
            exit_if  # if_2
          }
        }
        store %1, 2i
        exit_if  # if_1
      }
    }
    store %1, 3i
    %6:i32 = add 5i, 6i
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 {
  $B2: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %condA [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        store %continue_execution, false
        store %return_value, 3i
        exit_if  # if_1
      }
      $B4: {  # false
        if %condB [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            if %condC [t: $B7, f: $B8] {  # if_3
              $B7: {  # true
                store %continue_execution, false
                store %return_value, 1i
                exit_if  # if_3
              }
              $B8: {  # false
                exit_if  # if_3
              }
            }
            %8:bool = load %continue_execution
            if %8 [t: $B9] {  # if_4
              $B9: {  # true
                store %1, 1i
                store %continue_execution, false
                store %return_value, 2i
                exit_if  # if_4
              }
            }
            exit_if  # if_2
          }
          $B6: {  # false
            exit_if  # if_2
          }
        }
        %9:bool = load %continue_execution
        if %9 [t: $B10] {  # if_5
          $B10: {  # true
            store %1, 2i
            exit_if  # if_5
          }
        }
        exit_if  # if_1
      }
    }
    %10:bool = load %continue_execution
    if %10 [t: $B11] {  # if_6
      $B11: {  # true
        store %1, 3i
        %11:i32 = add 5i, 6i
        store %return_value, %11
        exit_if  # if_6
      }
    }
    %12:i32 = load %return_value
    ret %12
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_Nested_TrivialMerge) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* func = b.Function("foo", ty.i32());
    auto* condA = b.FunctionParam("condA", ty.bool_());
    auto* condB = b.FunctionParam("condB", ty.bool_());
    auto* condC = b.FunctionParam("condC", ty.bool_());
    func->SetParams({condA, condB, condC});

    b.Append(func->Block(), [&] {
        auto* ifelse_outer = b.If(condA);
        b.Append(ifelse_outer->True(), [&] { b.Return(func, 3_i); });
        b.Append(ifelse_outer->False(), [&] {
            auto* ifelse_middle = b.If(condB);
            b.Append(ifelse_middle->True(), [&] {
                auto* ifelse_inner = b.If(condC);
                b.Append(ifelse_inner->True(), [&] { b.Return(func, 1_i); });
                b.Append(ifelse_inner->False(), [&] { b.ExitIf(ifelse_inner); });

                b.ExitIf(ifelse_middle);
            });
            b.Append(ifelse_middle->False(), [&] { b.ExitIf(ifelse_middle); });

            b.ExitIf(ifelse_outer);
        });
        b.Return(func, 3_i);
    });
    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 {
  $B2: {
    if %condA [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        ret 3i
      }
      $B4: {  # false
        if %condB [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            if %condC [t: $B7, f: $B8] {  # if_3
              $B7: {  # true
                ret 1i
              }
              $B8: {  # false
                exit_if  # if_3
              }
            }
            exit_if  # if_2
          }
          $B6: {  # false
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
    }
    ret 3i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 {
  $B2: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %condA [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        store %continue_execution, false
        store %return_value, 3i
        exit_if  # if_1
      }
      $B4: {  # false
        if %condB [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            if %condC [t: $B7, f: $B8] {  # if_3
              $B7: {  # true
                store %continue_execution, false
                store %return_value, 1i
                exit_if  # if_3
              }
              $B8: {  # false
                exit_if  # if_3
              }
            }
            exit_if  # if_2
          }
          $B6: {  # false
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
    }
    %8:bool = load %continue_execution
    if %8 [t: $B9] {  # if_4
      $B9: {  # true
        store %return_value, 3i
        exit_if  # if_4
      }
    }
    %9:i32 = load %return_value
    ret %9
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_Nested_WithBasicBlockArguments) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* func = b.Function("foo", ty.i32());
    auto* condA = b.FunctionParam("condA", ty.bool_());
    auto* condB = b.FunctionParam("condB", ty.bool_());
    auto* condC = b.FunctionParam("condC", ty.bool_());
    func->SetParams({condA, condB, condC});

    b.Append(func->Block(), [&] {
        auto* ifelse_outer = b.If(condA);
        ifelse_outer->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse_outer->True(), [&] { b.Return(func, 3_i); });
        b.Append(ifelse_outer->False(), [&] {
            auto* ifelse_middle = b.If(condB);
            ifelse_middle->SetResults(b.InstructionResult(ty.i32()));
            b.Append(ifelse_middle->True(), [&] {
                auto* ifelse_inner = b.If(condC);

                b.Append(ifelse_inner->True(), [&] { b.Return(func, 1_i); });
                b.Append(ifelse_inner->False(), [&] { b.ExitIf(ifelse_inner); });

                b.ExitIf(ifelse_middle, b.Add(ty.i32(), 42_i, 1_i));
            });
            b.Append(ifelse_middle->False(),
                     [&] { b.ExitIf(ifelse_middle, b.Add(ty.i32(), 43_i, 2_i)); });
            b.ExitIf(ifelse_outer, b.Add(ty.i32(), ifelse_middle->Result(0), 1_i));
        });

        b.Return(func, b.Add(ty.i32(), ifelse_outer->Result(0), 1_i));
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 {
  $B2: {
    %6:i32 = if %condA [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        ret 3i
      }
      $B4: {  # false
        %7:i32 = if %condB [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            if %condC [t: $B7, f: $B8] {  # if_3
              $B7: {  # true
                ret 1i
              }
              $B8: {  # false
                exit_if  # if_3
              }
            }
            %8:i32 = add 42i, 1i
            exit_if %8  # if_2
          }
          $B6: {  # false
            %9:i32 = add 43i, 2i
            exit_if %9  # if_2
          }
        }
        %10:i32 = add %7, 1i
        exit_if %10  # if_1
      }
    }
    %11:i32 = add %6, 1i
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 {
  $B2: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %8:i32 = if %condA [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        store %continue_execution, false
        store %return_value, 3i
        exit_if undef  # if_1
      }
      $B4: {  # false
        %9:i32 = if %condB [t: $B5, f: $B6] {  # if_2
          $B5: {  # true
            if %condC [t: $B7, f: $B8] {  # if_3
              $B7: {  # true
                store %continue_execution, false
                store %return_value, 1i
                exit_if  # if_3
              }
              $B8: {  # false
                exit_if  # if_3
              }
            }
            %10:bool = load %continue_execution
            %11:i32 = if %10 [t: $B9] {  # if_4
              $B9: {  # true
                %12:i32 = add 42i, 1i
                exit_if %12  # if_4
              }
              # implicit false block: exit_if undef
            }
            exit_if %11  # if_2
          }
          $B6: {  # false
            %13:i32 = add 43i, 2i
            exit_if %13  # if_2
          }
        }
        %14:bool = load %continue_execution
        %15:i32 = if %14 [t: $B10] {  # if_5
          $B10: {  # true
            %16:i32 = add %9, 1i
            exit_if %16  # if_5
          }
          # implicit false block: exit_if undef
        }
        exit_if %15  # if_1
      }
    }
    %17:bool = load %continue_execution
    if %17 [t: $B11] {  # if_6
      $B11: {  # true
        %18:i32 = add %8, 1i
        store %return_value, %18
        exit_if  # if_6
      }
    }
    %19:i32 = load %return_value
    ret %19
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_Consecutive) {
    auto* value = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value});

    b.Append(func->Block(), [&] {
        {
            auto* ifelse = b.If(b.Equal(ty.bool_(), value, 1_i));
            b.Append(ifelse->True(), [&] { b.Return(func, 101_i); });
        }
        {
            auto* ifelse = b.If(b.Equal(ty.bool_(), value, 2_i));
            b.Append(ifelse->True(), [&] { b.Return(func, 202_i); });
        }
        {
            auto* ifelse = b.If(b.Equal(ty.bool_(), value, 3_i));
            b.Append(ifelse->True(), [&] { b.Return(func, 303_i); });
        }
        b.Return(func, 404_i);
    });

    auto* src = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %3:bool = eq %2, 1i
    if %3 [t: $B2] {  # if_1
      $B2: {  # true
        ret 101i
      }
    }
    %4:bool = eq %2, 2i
    if %4 [t: $B3] {  # if_2
      $B3: {  # true
        ret 202i
      }
    }
    %5:bool = eq %2, 3i
    if %5 [t: $B4] {  # if_3
      $B4: {  # true
        ret 303i
      }
    }
    ret 404i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:bool = eq %2, 1i
    if %5 [t: $B2] {  # if_1
      $B2: {  # true
        store %continue_execution, false
        store %return_value, 101i
        exit_if  # if_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: $B3] {  # if_2
      $B3: {  # true
        %7:bool = eq %2, 2i
        if %7 [t: $B4] {  # if_3
          $B4: {  # true
            store %continue_execution, false
            store %return_value, 202i
            exit_if  # if_3
          }
        }
        %8:bool = load %continue_execution
        if %8 [t: $B5] {  # if_4
          $B5: {  # true
            %9:bool = eq %2, 3i
            if %9 [t: $B6] {  # if_5
              $B6: {  # true
                store %continue_execution, false
                store %return_value, 303i
                exit_if  # if_5
              }
            }
            %10:bool = load %continue_execution
            if %10 [t: $B7] {  # if_6
              $B7: {  # true
                store %return_value, 404i
                exit_if  # if_6
              }
            }
            exit_if  # if_4
          }
        }
        exit_if  # if_2
      }
    }
    %11:i32 = load %return_value
    ret %11
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_Consecutive_ThenUnreachable) {
    auto* value = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value});

    b.Append(func->Block(), [&] {
        {
            auto* if_ = b.If(b.Equal(ty.bool_(), value, 1_i));
            b.Append(if_->True(), [&] { b.Return(func, 101_i); });
        }
        {
            auto* ifelse = b.If(b.Equal(ty.bool_(), value, 2_i));
            b.Append(ifelse->True(), [&] { b.Return(func, 202_i); });
            b.Append(ifelse->False(), [&] { b.Return(func, 303_i); });
        }
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %3:bool = eq %2, 1i
    if %3 [t: $B2] {  # if_1
      $B2: {  # true
        ret 101i
      }
    }
    %4:bool = eq %2, 2i
    if %4 [t: $B3, f: $B4] {  # if_2
      $B3: {  # true
        ret 202i
      }
      $B4: {  # false
        ret 303i
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:bool = eq %2, 1i
    if %5 [t: $B2] {  # if_1
      $B2: {  # true
        store %continue_execution, false
        store %return_value, 101i
        exit_if  # if_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: $B3] {  # if_2
      $B3: {  # true
        %7:bool = eq %2, 2i
        if %7 [t: $B4, f: $B5] {  # if_3
          $B4: {  # true
            store %continue_execution, false
            store %return_value, 202i
            exit_if  # if_3
          }
          $B5: {  # false
            store %continue_execution, false
            store %return_value, 303i
            exit_if  # if_3
          }
        }
        exit_if  # if_2
      }
    }
    %8:i32 = load %return_value
    ret %8
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_NestedConsecutives) {
    auto* value = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value});

    b.Append(func->Block(), [&] {
        auto* outer = b.If(b.Equal(ty.bool_(), value, 1_i));
        b.Append(outer->True(), [&] {
            auto* middle_first = b.If(b.Equal(ty.bool_(), value, 2_i));
            b.Append(middle_first->True(), [&] {  //
                b.Return(func, 202_i);
            });

            auto* middle_second = b.If(b.Equal(ty.bool_(), value, 3_i));
            b.Append(middle_second->True(), [&] {
                auto* inner_first = b.If(b.Equal(ty.bool_(), value, 4_i));
                b.Append(inner_first->True(), [&] {  //
                    b.Return(func, 404_i);
                });

                auto* inner_second = b.If(b.Equal(ty.bool_(), value, 5_i));
                b.Append(inner_second->True(), [&] {  //
                    b.Return(func, 505_i);
                });

                b.ExitIf(middle_second);
            });

            b.ExitIf(outer);
        });

        b.Return(func, 606_i);
    });

    auto* src = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %3:bool = eq %2, 1i
    if %3 [t: $B2] {  # if_1
      $B2: {  # true
        %4:bool = eq %2, 2i
        if %4 [t: $B3] {  # if_2
          $B3: {  # true
            ret 202i
          }
        }
        %5:bool = eq %2, 3i
        if %5 [t: $B4] {  # if_3
          $B4: {  # true
            %6:bool = eq %2, 4i
            if %6 [t: $B5] {  # if_4
              $B5: {  # true
                ret 404i
              }
            }
            %7:bool = eq %2, 5i
            if %7 [t: $B6] {  # if_5
              $B6: {  # true
                ret 505i
              }
            }
            exit_if  # if_3
          }
        }
        exit_if  # if_1
      }
    }
    ret 606i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:bool = eq %2, 1i
    if %5 [t: $B2] {  # if_1
      $B2: {  # true
        %6:bool = eq %2, 2i
        if %6 [t: $B3] {  # if_2
          $B3: {  # true
            store %continue_execution, false
            store %return_value, 202i
            exit_if  # if_2
          }
        }
        %7:bool = load %continue_execution
        if %7 [t: $B4] {  # if_3
          $B4: {  # true
            %8:bool = eq %2, 3i
            if %8 [t: $B5] {  # if_4
              $B5: {  # true
                %9:bool = eq %2, 4i
                if %9 [t: $B6] {  # if_5
                  $B6: {  # true
                    store %continue_execution, false
                    store %return_value, 404i
                    exit_if  # if_5
                  }
                }
                %10:bool = load %continue_execution
                if %10 [t: $B7] {  # if_6
                  $B7: {  # true
                    %11:bool = eq %2, 5i
                    if %11 [t: $B8] {  # if_7
                      $B8: {  # true
                        store %continue_execution, false
                        store %return_value, 505i
                        exit_if  # if_7
                      }
                    }
                    exit_if  # if_6
                  }
                }
                exit_if  # if_4
              }
            }
            exit_if  # if_3
          }
        }
        exit_if  # if_1
      }
    }
    %12:bool = load %continue_execution
    if %12 [t: $B9] {  # if_8
      $B9: {  # true
        store %return_value, 606i
        exit_if  # if_8
      }
    }
    %13:i32 = load %return_value
    ret %13
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, IfElse_NestedConsecutives_WithResults) {
    auto* value = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({value});

    b.Append(func->Block(), [&] {
        auto* outer_result = b.InstructionResult(ty.i32());
        auto* outer = b.If(b.Equal(ty.bool_(), value, 1_i));
        outer->SetResults(Vector{outer_result});
        b.Append(outer->True(), [&] {
            auto* middle_first = b.If(b.Equal(ty.bool_(), value, 2_i));
            b.Append(middle_first->True(), [&] {  //
                b.Return(func, 202_i);
            });

            auto middle_result = b.InstructionResult(ty.i32());
            auto* middle_second = b.If(b.Equal(ty.bool_(), value, 3_i));
            middle_second->SetResults(Vector{middle_result});
            b.Append(middle_second->True(), [&] {
                auto* inner_first = b.If(b.Equal(ty.bool_(), value, 4_i));
                b.Append(inner_first->True(), [&] {  //
                    b.Return(func, 404_i);
                });

                auto inner_result = b.InstructionResult(ty.i32());
                auto* inner_second = b.If(b.Equal(ty.bool_(), value, 5_i));
                inner_second->SetResults(Vector{inner_result});
                b.Append(inner_second->True(), [&] {  //
                    b.ExitIf(inner_second, 505_i);
                });

                b.ExitIf(middle_second, inner_result);
            });

            b.ExitIf(outer, middle_result);
        });

        b.Return(func, outer_result);
    });

    auto* src = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %3:bool = eq %2, 1i
    %4:i32 = if %3 [t: $B2] {  # if_1
      $B2: {  # true
        %5:bool = eq %2, 2i
        if %5 [t: $B3] {  # if_2
          $B3: {  # true
            ret 202i
          }
        }
        %6:bool = eq %2, 3i
        %7:i32 = if %6 [t: $B4] {  # if_3
          $B4: {  # true
            %8:bool = eq %2, 4i
            if %8 [t: $B5] {  # if_4
              $B5: {  # true
                ret 404i
              }
            }
            %9:bool = eq %2, 5i
            %10:i32 = if %9 [t: $B6] {  # if_5
              $B6: {  # true
                exit_if 505i  # if_5
              }
              # implicit false block: exit_if undef
            }
            exit_if %10  # if_3
          }
          # implicit false block: exit_if undef
        }
        exit_if %7  # if_1
      }
      # implicit false block: exit_if undef
    }
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:bool = eq %2, 1i
    %6:i32 = if %5 [t: $B2] {  # if_1
      $B2: {  # true
        %7:bool = eq %2, 2i
        if %7 [t: $B3] {  # if_2
          $B3: {  # true
            store %continue_execution, false
            store %return_value, 202i
            exit_if  # if_2
          }
        }
        %8:bool = load %continue_execution
        %9:i32 = if %8 [t: $B4] {  # if_3
          $B4: {  # true
            %10:bool = eq %2, 3i
            %11:i32 = if %10 [t: $B5] {  # if_4
              $B5: {  # true
                %12:bool = eq %2, 4i
                if %12 [t: $B6] {  # if_5
                  $B6: {  # true
                    store %continue_execution, false
                    store %return_value, 404i
                    exit_if  # if_5
                  }
                }
                %13:bool = load %continue_execution
                %14:i32 = if %13 [t: $B7] {  # if_6
                  $B7: {  # true
                    %15:bool = eq %2, 5i
                    %16:i32 = if %15 [t: $B8] {  # if_7
                      $B8: {  # true
                        exit_if 505i  # if_7
                      }
                      # implicit false block: exit_if undef
                    }
                    exit_if %16  # if_6
                  }
                  # implicit false block: exit_if undef
                }
                exit_if %14  # if_4
              }
              # implicit false block: exit_if undef
            }
            exit_if %11  # if_3
          }
          # implicit false block: exit_if undef
        }
        exit_if %9  # if_1
      }
      # implicit false block: exit_if undef
    }
    %17:bool = load %continue_execution
    if %17 [t: $B9] {  # if_8
      $B9: {  # true
        store %return_value, %6
        exit_if  # if_8
      }
    }
    %18:i32 = load %return_value
    ret %18
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, Loop_UnconditionalReturnInBody) {
    auto* func = b.Function("foo", ty.i32());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Return(func, 42_i); });

        b.Unreachable();
    });
    auto* src = R"(
%foo = func():i32 {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        ret 42i
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        store %return_value, 42i
        exit_loop  # loop_1
      }
    }
    %3:i32 = load %return_value
    ret %3
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, Loop_ConditionalReturnInBody) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(cond);
            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(global, 1_i);
            b.BreakIf(loop, true);
        });

        b.Store(global, 3_i);
        b.Return(func, 43_i);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        if %3 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            ret 42i
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        continue  # -> $B4
      }
      $B4: {  # continuing
        store %1, 1i
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    store %1, 3i
    ret 43i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 {
  $B2: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        if %3 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        %6:bool = load %continue_execution
        if %6 [t: $B7] {  # if_2
          $B7: {  # true
            store %1, 2i
            continue  # -> $B4
          }
        }
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        store %1, 1i
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    %7:bool = load %continue_execution
    if %7 [t: $B8] {  # if_3
      $B8: {  # true
        store %1, 3i
        store %return_value, 43i
        exit_if  # if_3
      }
    }
    %8:i32 = load %return_value
    ret %8
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, Loop_ConditionalReturnInBody_UnreachableMerge) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(cond);

            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(global, 1_i);
            b.NextIteration(loop);
        });

        b.Unreachable();
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        if %3 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            ret 42i
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        continue  # -> $B4
      }
      $B4: {  # continuing
        store %1, 1i
        next_iteration  # -> $B3
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 {
  $B2: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        if %3 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        %6:bool = load %continue_execution
        if %6 [t: $B7] {  # if_2
          $B7: {  # true
            store %1, 2i
            continue  # -> $B4
          }
        }
        exit_loop  # loop_1
      }
      $B4: {  # continuing
        store %1, 1i
        next_iteration  # -> $B3
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, DISABLED_Loop_WithBasicBlockArgumentsOnMerge) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult(ty.i32()));
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(cond);
            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(global, 1_i);
            b.BreakIf(loop, true, /* next_iter */ b.Values(4_i), /* exit */ Empty);
        });

        b.Store(global, 3_i);
        b.Return(func, loop->Result(0));
    });
    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    %4:i32 = loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            ret 42i
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        continue %b4
      }
      %b4 = block {  # continuing
        store %1, 1i
        break_if true %b3 4i
      }
    }
    store %1, 3i
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %6:i32 = loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        %7:bool = load %continue_execution
        if %7 [t: %b7] {  # if_2
          %b7 = block {  # true
            store %1, 2i
            continue %b4
          }
        }
        exit_loop  # loop_1
      }
      %b4 = block {  # continuing
        store %1, 1i
        break_if true %b3 4i
      }
    }
    %8:bool = load %continue_execution
    if %8 [t: %b8] {  # if_3
      %b8 = block {  # true
        store %1, 3i
        store %return_value, %6
        exit_if  # if_3
      }
    }
    %9:i32 = load %return_value
    ret %9
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, Switch_UnconditionalReturnInCase) {
    auto* cond = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(cond);
        b.Append(b.Case(sw, {b.Constant(1_i)}), [&] { b.Return(func, 42_i); });
        b.Append(b.DefaultCase(sw), [&] { b.ExitSwitch(sw); });

        b.Return(func, 0_i);
    });

    auto* src = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    switch %2 [c: (1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        ret 42i
      }
      $B3: {  # case
        exit_switch  # switch_1
      }
    }
    ret 0i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    switch %2 [c: (1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        store %continue_execution, false
        store %return_value, 42i
        exit_switch  # switch_1
      }
      $B3: {  # case
        exit_switch  # switch_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: $B4] {  # if_1
      $B4: {  # true
        store %return_value, 0i
        exit_if  # if_1
      }
    }
    %6:i32 = load %return_value
    ret %6
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, Switch_ConditionalReturnInBody) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(global);

    auto* cond = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(cond);
        b.Append(b.Case(sw, {b.Constant(1_i)}), [&] {
            auto* ifcond = b.Equal(ty.bool_(), cond, 1_i);
            auto* ifelse = b.If(ifcond);
            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.ExitSwitch(sw);
        });

        b.Append(b.DefaultCase(sw), [&] { b.ExitSwitch(sw); });

        b.Return(func, 0_i);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:i32):i32 {
  $B2: {
    switch %3 [c: (1i, $B3), c: (default, $B4)] {  # switch_1
      $B3: {  # case
        %4:bool = eq %3, 1i
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            ret 42i
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        exit_switch  # switch_1
      }
      $B4: {  # case
        exit_switch  # switch_1
      }
    }
    ret 0i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:i32):i32 {
  $B2: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    switch %3 [c: (1i, $B3), c: (default, $B4)] {  # switch_1
      $B3: {  # case
        %6:bool = eq %3, 1i
        if %6 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        %7:bool = load %continue_execution
        if %7 [t: $B7] {  # if_2
          $B7: {  # true
            store %1, 2i
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
      $B4: {  # case
        exit_switch  # switch_1
      }
    }
    %8:bool = load %continue_execution
    if %8 [t: $B8] {  # if_3
      $B8: {  # true
        store %return_value, 0i
        exit_if  # if_3
      }
    }
    %9:i32 = load %return_value
    ret %9
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, Switch_WithBasicBlockArgumentsOnMerge) {
    auto* cond = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(cond);
        sw->SetResults(b.InstructionResult(ty.i32()));  // NOLINT: false detection of std::tuple
        b.Append(b.Case(sw, {b.Constant(1_i)}), [&] { b.Return(func, 42_i); });
        b.Append(b.Case(sw, {b.Constant(2_i)}), [&] { b.Return(func, 99_i); });
        b.Append(b.Case(sw, {b.Constant(3_i)}), [&] { b.ExitSwitch(sw, 1_i); });
        b.Append(b.DefaultCase(sw), [&] { b.ExitSwitch(sw, 0_i); });

        b.Return(func, sw->Result(0));
    });

    auto* src = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %3:i32 = switch %2 [c: (1i, $B2), c: (2i, $B3), c: (3i, $B4), c: (default, $B5)] {  # switch_1
      $B2: {  # case
        ret 42i
      }
      $B3: {  # case
        ret 99i
      }
      $B4: {  # case
        exit_switch 1i  # switch_1
      }
      $B5: {  # case
        exit_switch 0i  # switch_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:i32 = switch %2 [c: (1i, $B2), c: (2i, $B3), c: (3i, $B4), c: (default, $B5)] {  # switch_1
      $B2: {  # case
        store %continue_execution, false
        store %return_value, 42i
        exit_switch undef  # switch_1
      }
      $B3: {  # case
        store %continue_execution, false
        store %return_value, 99i
        exit_switch undef  # switch_1
      }
      $B4: {  # case
        exit_switch 1i  # switch_1
      }
      $B5: {  # case
        exit_switch 0i  # switch_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: $B6] {  # if_1
      $B6: {  # true
        store %return_value, %5
        exit_if  # if_1
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, LoopIfReturnThenContinue) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Append(b.If(true)->True(), [&] { b.Return(func); });
            b.Continue(loop);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        if true [t: $B3] {  # if_1
          $B3: {  # true
            ret
          }
        }
        continue  # -> $B4
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void {
  $B1: {
    %continue_execution:ptr<function, bool, read_write> = var, true
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        if true [t: $B3] {  # if_1
          $B3: {  # true
            store %continue_execution, false
            exit_if  # if_1
          }
        }
        %3:bool = load %continue_execution
        if %3 [t: $B4] {  # if_2
          $B4: {  # true
            continue  # -> $B5
          }
        }
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_MergeReturnTest, NestedIfsWithReturns) {
    auto* func = b.Function("foo", ty.i32());

    b.Append(func->Block(), [&] {
        b.Append(b.If(true)->True(), [&] {
            b.Append(b.If(true)->True(), [&] { b.Return(func, 1_i); });
            b.Return(func, 2_i);
        });
        b.Return(func, 3_i);
    });

    auto* src = R"(
%foo = func():i32 {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        if true [t: $B3] {  # if_2
          $B3: {  # true
            ret 1i
          }
        }
        ret 2i
      }
    }
    ret 3i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():i32 {
  $B1: {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if true [t: $B2] {  # if_1
      $B2: {  # true
        if true [t: $B3] {  # if_2
          $B3: {  # true
            store %continue_execution, false
            store %return_value, 1i
            exit_if  # if_2
          }
        }
        %4:bool = load %continue_execution
        if %4 [t: $B4] {  # if_3
          $B4: {  # true
            store %continue_execution, false
            store %return_value, 2i
            exit_if  # if_3
          }
        }
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: $B5] {  # if_4
      $B5: {  # true
        store %return_value, 3i
        exit_if  # if_4
      }
    }
    %6:i32 = load %return_value
    ret %6
  }
}
)";

    Run(MergeReturn);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
