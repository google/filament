// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/analysis/for_loop_analysis.h"

#include <cstddef>
#include <utility>

#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

namespace tint::core::ir::analysis {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ForLoopAnalysisTest : public IRTestHelper {};

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    Store* store = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            condition = b.LessThan(b.Load(idx), 10_u);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            store = b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(store, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, ConditionUniformArray) {
    Var* U = nullptr;
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var<uniform, array<vec4<u32>, 8>>("U");
                 U->SetBindingPoint(0, 0);
             });

    Var* idx = nullptr;
    Loop* loop = nullptr;
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    Store* store = nullptr;
    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* access = b.Access(ty.ptr<uniform, vec4<u32>>(), U, 3_i);
            auto* load = b.LoadVectorElement(access, 0_u);
            condition = b.LessThan(load, 10_u);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            store = b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(fn_b);
    });

    auto* src = R"(
$B1: {  # root
  %U:ptr<uniform, array<vec4<u32>, 8>, read> = var undef @binding_point(0, 0)
}

%b = func():void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B4
      }
      $B4: {  # body
        %4:ptr<uniform, vec4<u32>, read> = access %U, 3i
        %5:u32 = load_vector_element %4, 0u
        %6:bool = lt %5, 10u
        if %6 [t: $B6, f: $B7] {  # if_1
          $B6: {  # true
            exit_if  # if_1
          }
          $B7: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B5
      }
      $B5: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
        store %idx, %8
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);

    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(store, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, ConditionUniformStructure) {
    Var* U = nullptr;
    auto* str_ty =
        ty.Struct(mod.symbols.New("UniformStruct"), {
                                                        {mod.symbols.New("a"), ty.vec4u()},
                                                        {mod.symbols.New("b"), ty.vec4u()},
                                                        {mod.symbols.New("c"), ty.vec4u()},
                                                        {mod.symbols.New("d"), ty.vec4u()},
                                                    });
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var("U", uniform, str_ty);
                 U->SetBindingPoint(0, 0);
             });

    Var* idx = nullptr;
    Loop* loop = nullptr;
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    Store* store = nullptr;
    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* access = b.Access(ty.ptr<uniform, vec4<u32>>(), U, 0_i);
            auto* load = b.LoadVectorElement(access, 0_u);
            condition = b.LessThan(load, 10_u);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            store = b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(fn_b);
    });

    auto* src = R"(
UniformStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:vec4<u32> @offset(16)
  c:vec4<u32> @offset(32)
  d:vec4<u32> @offset(48)
}

$B1: {  # root
  %U:ptr<uniform, UniformStruct, read> = var undef @binding_point(0, 0)
}

%b = func():void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B4
      }
      $B4: {  # body
        %4:ptr<uniform, vec4<u32>, read> = access %U, 0i
        %5:u32 = load_vector_element %4, 0u
        %6:bool = lt %5, 10u
        if %6 [t: $B6, f: $B7] {  # if_1
          $B6: {  # true
            exit_if  # if_1
          }
          $B7: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B5
      }
      $B5: {  # continuing
        %7:u32 = load %idx
        %8:u32 = add %7, 1u
        store %idx, %8
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());

    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(store, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, ConditionUniformStructure_WithInfiniteLoopPrevention) {
    Var* U = nullptr;
    auto* str_ty =
        ty.Struct(mod.symbols.New("UniformStruct"), {
                                                        {mod.symbols.New("a"), ty.vec4u()},
                                                        {mod.symbols.New("b"), ty.vec4u()},
                                                        {mod.symbols.New("c"), ty.vec4u()},
                                                        {mod.symbols.New("d"), ty.vec4u()},
                                                    });
    b.Append(b.ir.root_block,
             [&] {  //
                 U = b.Var("U", uniform, str_ty);
                 U->SetBindingPoint(0, 0);
             });

    Var* idx = nullptr;
    Loop* loop = nullptr;
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* access = b.Access(ty.ptr<uniform, vec4<u32>>(), U, 0_i);
            auto* load = b.LoadVectorElement(access, 0_u);
            condition = b.LessThan(load, 10_u);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(fn_b);
    });

    // PreventInfiniteLoops can add in if with exits.
    auto res = transform::PreventInfiniteLoops(mod);
    EXPECT_TRUE(res == Success);

    auto* src = R"(
UniformStruct = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:vec4<u32> @offset(16)
  c:vec4<u32> @offset(32)
  d:vec4<u32> @offset(48)
}

$B1: {  # root
  %U:ptr<uniform, UniformStruct, read> = var undef @binding_point(0, 0)
}

%b = func():void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        %tint_loop_idx:ptr<function, vec2<u32>, read_write> = var vec2<u32>(4294967295u)
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B4
      }
      $B4: {  # body
        %5:vec2<u32> = load %tint_loop_idx
        %6:vec2<bool> = eq %5, vec2<u32>(0u)
        %7:bool = all %6
        if %7 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %8:ptr<uniform, vec4<u32>, read> = access %U, 0i
        %9:u32 = load_vector_element %8, 0u
        %10:bool = lt %9, 10u
        if %10 [t: $B7, f: $B8] {  # if_2
          $B7: {  # true
            exit_if  # if_2
          }
          $B8: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B5
      }
      $B5: {  # continuing
        %11:u32 = load_vector_element %tint_loop_idx, 0u
        %tint_low_inc:u32 = sub %11, 1u
        store_vector_element %tint_loop_idx, 0u, %tint_low_inc
        %13:bool = eq %tint_low_inc, 4294967295u
        %tint_carry:u32 = convert %13
        %15:u32 = load_vector_element %tint_loop_idx, 1u
        %16:u32 = sub %15, %tint_carry
        store_vector_element %tint_loop_idx, 1u, %16
        %17:u32 = load %idx
        %18:u32 = add %17, 1u
        store %idx, %18
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(nullptr, analysis.GetIfCondition());
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_FailLet) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* as_let = b.Let(10_u);
            condition = b.LessThan(b.Load(idx), as_let);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = let 10u
        %4:u32 = load %idx
        %5:bool = lt %4, %3
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = add %6, 1u
        store %idx, %7
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(nullptr, analysis.GetIfCondition());
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_FailStore) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            auto* as_let = b.Let(10_u);
            condition = b.LessThan(b.Load(idx), as_let);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:u32 = add %3, 1u
        store %idx, %4
        %5:u32 = let 10u
        %6:u32 = load %idx
        %7:bool = lt %6, %5
        if %7 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %8:u32 = load %idx
        %9:u32 = add %8, 1u
        store %idx, %9
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(nullptr, analysis.GetIfCondition());
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_FailNonCanonicalIfFlipped) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* as_let = b.Let(10_u);
            condition = b.LessThan(b.Load(idx), as_let);
            ifelse = b.If(condition);
            b.Append(ifelse->False(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = let 10u
        %4:u32 = load %idx
        %5:bool = lt %4, %3
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = add %6, 1u
        store %idx, %7
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(nullptr, analysis.GetIfCondition());
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_FailNonCanonicalIfOnlyExit) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* as_let = b.Let(10_u);
            condition = b.LessThan(b.Load(idx), as_let);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });

            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = let 10u
        %4:u32 = load %idx
        %5:bool = lt %4, %3
        if %5 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = add %6, 1u
        store %idx, %7
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(nullptr, analysis.GetIfCondition());
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_EmptyContinuingBlock) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop, 10_u);
        });
        auto* body_param = b.BlockParam<u32>();
        loop->Body()->AddParam(body_param);
        b.Append(loop->Body(), [&] {
            condition = b.LessThan(b.Load(idx), body_param);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            b.ExitLoop(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration 10u  # -> $B3
      }
      $B3 (%3:u32): {  # body
        %4:u32 = load %idx
        %5:bool = lt %4, %3
        if %5 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_ContinuingNextIterationHasOperands) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    Store* store = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop, 10_u);
        });
        auto* body_param = b.BlockParam<u32>();
        loop->Body()->AddParam(body_param);
        b.Append(loop->Body(), [&] {
            condition = b.LessThan(b.Load(idx), body_param);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            store = b.Store(idx, b.Add(b.Load(idx), 1_u));
            b.NextIteration(loop, 5_u);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration 10u  # -> $B3
      }
      $B3 (%3:u32): {  # body
        %4:u32 = load %idx
        %5:bool = lt %4, %3
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = add %6, 1u
        store %idx, %7
        next_iteration 5u  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_ContinuingHasNoStore) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    Store* store = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            condition = b.LessThan(b.Load(idx), 10_u);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            store = b.Store(idx, b.Add(b.Load(idx), 1_u));
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %5:u32 = load %idx
        %6:u32 = add %5, 1u
        store %idx, %6
        continue  # -> $B4
      }
      $B4: {  # continuing
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_ContinuingInstructionDoesNotSinkIntoStore) {
    Var* idx = nullptr;
    Var* bound = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            bound = b.Var("idx2", 10_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* idx_load = b.Load(idx);
            auto* bound_load = b.Load(bound);
            condition = b.LessThan(idx_load, bound_load);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            auto* load = b.Load(idx);
            b.Store(bound, 5_u);
            b.Store(idx, b.Add(load, 1_u));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        %idx2:ptr<function, u32, read_write> = var 10u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:u32 = load %idx
        %5:u32 = load %idx2
        %6:bool = lt %4, %5
        if %6 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %7:u32 = load %idx
        store %idx2, 5u
        %8:u32 = add %7, 1u
        store %idx, %8
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_ContinuingInstructionUsesBodyResult) {
    Var* idx = nullptr;
    Var* inc = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* idx_load = b.Load(idx);
            condition = b.LessThan(idx_load, 5_u);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            inc = b.Var("inc", 1_u);
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            auto* load = b.Load(idx);
            b.Store(idx, b.Add(load, b.Load(inc)));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 5u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %inc:ptr<function, u32, read_write> = var 1u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load %idx
        %7:u32 = load %inc
        %8:u32 = add %6, %7
        store %idx, %8
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

TEST_F(IR_ForLoopAnalysisTest, SimpleLoopCondition_ContinuingHasVarDeclaration) {
    Var* idx = nullptr;
    Loop* loop = nullptr;
    auto* func = b.Function("func", ty.void_());
    If* ifelse = nullptr;
    Continue* cont_statement = nullptr;
    CoreBinary* condition = nullptr;
    b.Append(func->Block(), [&] {
        loop = b.Loop();
        b.Append(loop->Initializer(), [&] {  //
            idx = b.Var("idx", 0_u);
            b.NextIteration(loop);
        });
        b.Append(loop->Body(), [&] {
            auto* idx_load = b.Load(idx);
            condition = b.LessThan(idx_load, 5_u);
            ifelse = b.If(condition);
            b.Append(ifelse->True(), [&] {  //
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {  //
                b.ExitLoop(loop);
            });
            cont_statement = b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {  //
            auto inc = b.Var("inc", 1_u);
            auto* load = b.Load(idx);
            b.Store(idx, b.Add(load, b.Load(inc)));
            b.NextIteration(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 5u
        if %4 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %inc:ptr<function, u32, read_write> = var 1u
        %6:u32 = load %idx
        %7:u32 = load %inc
        %8:u32 = add %6, %7
        store %idx, %8
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    EXPECT_EQ(Validate(mod), Success);

    ForLoopAnalysis analysis(*loop);
    EXPECT_EQ(ifelse->Condition(), analysis.GetIfCondition());
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(ifelse));
    EXPECT_TRUE(analysis.IsBodyRemovedInstruction(condition));
    EXPECT_FALSE(analysis.IsBodyRemovedInstruction(cont_statement));
    EXPECT_EQ(nullptr, analysis.GetContinuingUpdateStore());
}

}  // namespace
}  // namespace tint::core::ir::analysis
