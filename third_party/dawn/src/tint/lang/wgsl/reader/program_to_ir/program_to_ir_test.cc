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

#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/ir_program_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::wgsl::reader {
namespace {

/// Looks for the instruction with the given type T.
/// If no instruction is found, then nullptr is returned.
/// If multiple instructions are found with the type T, then an error is raised and the first is
/// returned.
template <typename T>
T* FindSingleInstruction(core::ir::Module& mod) {
    T* found = nullptr;
    size_t count = 0;
    for (auto* node : mod.Instructions()) {
        if (auto* as = node->As<T>()) {
            count++;
            if (!found) {
                found = as;
            }
        }
    }
    if (count > 1) {
        ADD_FAILURE() << "FindSingleInstruction() found " << count << " nodes of type "
                      << tint::TypeInfo::Of<T>().name;
    }
    return found;
}

using namespace tint::core::number_suffixes;  // NOLINT

using IR_FromProgramTest = helpers::IRProgramTest;

TEST_F(IR_FromProgramTest, Func) {
    Func("f", tint::Empty, ty.void_(), tint::Empty);

    auto m = Build();
    ASSERT_EQ(m, Success);

    ASSERT_EQ(1u, m->functions.Length());

    core::ir::Function* f = m->functions[0];
    ASSERT_NE(f->Block(), nullptr);
    EXPECT_EQ(m->functions[0]->Stage(), core::ir::Function::PipelineStage::kUndefined);

    EXPECT_EQ(Dis(m.Get()), R"(%f = func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam) {
    Func("f", Vector{Param("a", ty.u32())}, ty.u32(), Vector{Return("a")});

    auto m = Build();
    ASSERT_EQ(m, Success);

    ASSERT_EQ(1u, m->functions.Length());

    core::ir::Function* f = m->functions[0];
    ASSERT_NE(f->Block(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), core::ir::Function::PipelineStage::kUndefined);

    EXPECT_EQ(Dis(m.Get()), R"(%f = func(%a:u32):u32 {
  $B1: {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithMultipleParam) {
    Func("f", Vector{Param("a", ty.u32()), Param("b", ty.i32()), Param("c", ty.bool_())},
         ty.void_(), tint::Empty);

    auto m = Build();
    ASSERT_EQ(m, Success);

    ASSERT_EQ(1u, m->functions.Length());

    core::ir::Function* f = m->functions[0];
    ASSERT_NE(f->Block(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), core::ir::Function::PipelineStage::kUndefined);

    EXPECT_EQ(Dis(m.Get()), R"(%f = func(%a:u32, %b:i32, %c:bool):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, EntryPoint) {
    Func("f", tint::Empty, ty.void_(), tint::Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(m->functions[0]->Stage(), core::ir::Function::PipelineStage::kFragment);
}

TEST_F(IR_FromProgramTest, IfStatement) {
    auto* ast_if = If(true, Block(), Else(Block()));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
)");
}

TEST_F(IR_FromProgramTest, IfStatement_TrueReturns) {
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, IfStatement_FalseReturns) {
    auto* ast_if = If(true, Block(), Else(Block(Return())));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, IfStatement_BothReturn) {
    auto* ast_if = If(true, Block(Return()), Else(Block(Return())));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
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
)");
}

TEST_F(IR_FromProgramTest, IfStatement_JumpChainToMerge) {
    auto* ast_loop = Loop(Block(Break()));
    auto* ast_if = If(true, Block(ast_loop));
    WrapInFunction(ast_if);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        loop [b: $B3] {  # loop_1
          $B3: {  # body
            exit_loop  # loop_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithBreak) {
    auto* ast_loop = Loop(Block(Break()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(0u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithContinue) {
    auto* ast_if = If(true, Block(Break()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithContinuing_BreakIf) {
    auto* ast_break_if = BreakIf(true);
    auto* ast_loop = Loop(Block(), Block(ast_break_if));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_Continuing_Body_Scope) {
    auto* a = Decl(Let("a", Expr(true)));
    auto* ast_break_if = BreakIf("a");
    auto* ast_loop = Loop(Block(a), Block(ast_break_if));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %a:bool = let true
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if %a  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithReturn) {
    auto* ast_if = If(true, Block(Return()));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4] {  # if_1
          $B4: {  # true
            ret
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    unreachable
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithOnlyReturn) {
    auto* ast_loop = Loop(Block(Return(), Continue()));
    WrapInFunction(ast_loop, If(true, Block(Return())));

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(0u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        ret
      }
    }
    unreachable
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithOnlyReturn_ContinuingBreakIf) {
    // Note, even though there is code in the loop merge (specifically, the
    // `ast_if` below), it doesn't get emitted as there is no way to reach the
    // loop merge due to the loop itself doing a `return`. This is why the
    // loop merge gets marked as Dead and the `ast_if` doesn't appear.
    //
    // Similar, the continuing block goes away as there is no way to get there, so it's treated
    // as dead code and dropped.
    auto* ast_break_if = BreakIf(true);
    auto* ast_loop = Loop(Block(Return()), Block(ast_break_if));
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(Block(ast_loop, ast_if));

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(0u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        ret
      }
    }
    if true [t: $B3] {  # if_1
      $B3: {  # true
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_WithIf_BothBranchesBreak) {
    auto* ast_if = If(true, Block(Break()), Else(Block(Break())));
    auto* ast_loop = Loop(Block(ast_if, Continue()));
    WrapInFunction(ast_loop);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(0u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        if true [t: $B3, f: $B4] {  # if_1
          $B3: {  # true
            exit_loop  # loop_1
          }
          $B4: {  # false
            exit_loop  # loop_1
          }
        }
        unreachable
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Loop_Nested) {
    auto* ast_if_a = If(true, Block(Break()));
    auto* ast_if_b = If(true, Block(Continue()));
    auto* ast_if_c = BreakIf(true);
    auto* ast_if_d = If(true, Block(Break()));

    auto* ast_loop_d = Loop(Block(), Block(ast_if_c));
    auto* ast_loop_c = Loop(Block(Break()));

    auto* ast_loop_b = Loop(Block(ast_if_a, ast_if_b), Block(ast_loop_c, ast_loop_d));
    auto* ast_loop_a = Loop(Block(ast_loop_b, ast_if_d));

    WrapInFunction(ast_loop_a);

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        loop [b: $B4, c: $B5] {  # loop_2
          $B4: {  # body
            if true [t: $B6] {  # if_1
              $B6: {  # true
                exit_loop  # loop_2
              }
            }
            if true [t: $B7] {  # if_2
              $B7: {  # true
                continue  # -> $B5
              }
            }
            continue  # -> $B5
          }
          $B5: {  # continuing
            loop [b: $B8] {  # loop_3
              $B8: {  # body
                exit_loop  # loop_3
              }
            }
            loop [b: $B9, c: $B10] {  # loop_4
              $B9: {  # body
                continue  # -> $B10
              }
              $B10: {  # continuing
                break_if true  # -> [t: exit_loop loop_4, f: $B9]
              }
            }
            next_iteration  # -> $B4
          }
        }
        if true [t: $B11] {  # if_3
          $B11: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, While) {
    auto* ast_while = While(false, Block());
    WrapInFunction(ast_while);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, While_Return) {
    auto* ast_while = While(true, Block(Return()));
    WrapInFunction(ast_while);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        ret
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, For) {
    auto* ast_for = For(Decl(Var("i", ty.i32())), LessThan("i", 10_a), Increment("i"), Block());
    WrapInFunction(ast_for);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(2u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(1u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %i:ptr<function, i32, read_write> = var undef
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:i32 = load %i
        %4:bool = lt %3, 10i
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
        %5:i32 = load %i
        %6:i32 = add %5, 1i
        store %i, %6
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, For_Init_NoCondOrContinuing) {
    auto* ast_for = For(Decl(Var("i", ty.i32())), nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(1u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %i:ptr<function, i32, read_write> = var undef
        next_iteration  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, For_NoInitCondOrContinuing) {
    auto* ast_for = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(ast_for);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* loop = FindSingleInstruction<core::ir::Loop>(m);

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(0u, loop->Body()->InboundSiblingBranches().Length());
    EXPECT_EQ(0u, loop->Continuing()->InboundSiblingBranches().Length());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch) {
    auto* ast_switch =
        Switch(1_i, Vector{Case(Vector{CaseSelector(0_i)}, Block()),
                           Case(Vector{CaseSelector(1_i)}, Block()), DefaultCase(Block())});

    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<core::ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto& cases = swtch->Cases();
    ASSERT_EQ(3u, cases.Length());

    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<core::constant::Scalar<i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    ASSERT_TRUE(cases[1].selectors[0].val->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(1_i,
              cases[1].selectors[0].val->Value()->As<core::constant::Scalar<i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[2].selectors.Length());
    EXPECT_TRUE(cases[2].selectors[0].IsDefault());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (0i, $B2), c: (1i, $B3), c: (default, $B4)] {  # switch_1
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
)");
}

TEST_F(IR_FromProgramTest, Switch_MultiSelector) {
    auto* ast_switch = Switch(
        1_i,
        Vector{Case(Vector{CaseSelector(0_i), CaseSelector(1_i), DefaultCaseSelector()}, Block())});

    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<core::ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto& cases = swtch->Cases();
    ASSERT_EQ(1u, cases.Length());
    ASSERT_EQ(3u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<core::constant::Scalar<i32>>()->ValueOf());

    ASSERT_TRUE(cases[0].selectors[1].val->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(1_i,
              cases[0].selectors[1].val->Value()->As<core::constant::Scalar<i32>>()->ValueOf());

    EXPECT_TRUE(cases[0].selectors[2].IsDefault());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (0i 1i default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_OnlyDefault) {
    auto* ast_switch = Switch(1_i, Vector{DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<core::ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto& cases = swtch->Cases();
    ASSERT_EQ(1u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    EXPECT_TRUE(cases[0].selectors[0].IsDefault());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_WithBreak) {
    auto* ast_switch = Switch(
        1_i, Vector{Case(Vector{CaseSelector(0_i)}, Block(Break(), If(true, Block(Return())))),
                    DefaultCase(Block())});
    WrapInFunction(ast_switch);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* swtch = FindSingleInstruction<core::ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto& cases = swtch->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<core::constant::Scalar<i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    // This is 1 because the if is dead-code eliminated and the return doesn't happen.

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Switch_AllReturn) {
    auto* ast_switch = Switch(1_i, Vector{Case(Vector{CaseSelector(0_i)}, Block(Return())),
                                          DefaultCase(Block(Return()))});
    auto* ast_if = If(true, Block(Return()));
    WrapInFunction(ast_switch, ast_if);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    auto* swtch = FindSingleInstruction<core::ir::Switch>(m);

    ASSERT_EQ(1u, m.functions.Length());

    auto& cases = swtch->Cases();
    ASSERT_EQ(2u, cases.Length());
    ASSERT_EQ(1u, cases[0].selectors.Length());
    ASSERT_TRUE(cases[0].selectors[0].val->Value()->Is<core::constant::Scalar<i32>>());
    EXPECT_EQ(0_i,
              cases[0].selectors[0].val->Value()->As<core::constant::Scalar<i32>>()->ValueOf());

    ASSERT_EQ(1u, cases[1].selectors.Length());
    EXPECT_TRUE(cases[1].selectors[0].IsDefault());

    EXPECT_EQ(Dis(m),
              R"(%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
    }
    unreachable
  }
}
)");
}

TEST_F(IR_FromProgramTest, Emit_Phony) {
    Func("b", tint::Empty, ty.i32(), Return(1_i));
    WrapInFunction(Ignore(Call("b")));

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()),
              R"(%b = func():i32 {
  $B1: {
    ret 1i
  }
}
%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = call %b
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Invariant) {
    Func("f",
         Vector{Param("a", ty.vec4<f32>(),
                      Vector{Invariant(), Builtin(core::BuiltinValue::kPosition)})},
         ty.vec4<f32>(), Vector{Return("a")}, Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(1_i)});
    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(
        Dis(m.Get()),
        R"(%f = @fragment func(%a:vec4<f32> [@invariant, @position]):vec4<f32> [@location(1)] {
  $B1: {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Location) {
    Func("f", Vector{Param("a", ty.f32(), Vector{Location(2_i)})}, ty.f32(), Vector{Return("a")},
         Vector{Stage(ast::PipelineStage::kFragment)}, Vector{Location(1_i)});

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()),
              R"(%f = @fragment func(%a:f32 [@location(2)]):f32 [@location(1)] {
  $B1: {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Color) {
    Enable(wgsl::Extension::kChromiumExperimentalFramebufferFetch);
    Func("f", Vector{Param("a", ty.f32(), Vector{Color(2_i)})}, ty.f32(), Vector{Return("a")},
         Vector{Stage(ast::PipelineStage::kFragment)}, Vector{Location(1_i)});

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()),
              R"(%f = @fragment func(%a:f32 [@color(2)]):f32 [@location(1)] {
  $B1: {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Location_WithInterpolation_LinearCentroid) {
    Func("f",
         Vector{Param("a", ty.f32(),
                      Vector{Location(2_i), Interpolate(core::InterpolationType::kLinear,
                                                        core::InterpolationSampling::kCentroid)})},
         ty.f32(), Vector{Return("a")}, Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(1_i)});

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(
        Dis(m.Get()),
        R"(%f = @fragment func(%a:f32 [@location(2), @interpolate(linear, centroid)]):f32 [@location(1)] {
  $B1: {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Func_WithParam_WithAttribute_Location_WithInterpolation_Flat) {
    Func("f",
         Vector{Param("a", ty.f32(),
                      Vector{Location(2_i), Interpolate(core::InterpolationType::kFlat)})},
         ty.f32(), Vector{Return("a")}, Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(1_i)});

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()),
              R"(%f = @fragment func(%a:f32 [@location(2), @interpolate(flat)]):f32 [@location(1)] {
  $B1: {
    ret %a
  }
}
)");
}

TEST_F(IR_FromProgramTest, Requires) {
    Require(wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures);
    Func("f", tint::Empty, ty.void_(), tint::Empty);

    auto m = Build();
    ASSERT_EQ(m, Success);

    ASSERT_EQ(1u, m->functions.Length());

    core::ir::Function* f = m->functions[0];
    ASSERT_NE(f->Block(), nullptr);

    EXPECT_EQ(m->functions[0]->Stage(), core::ir::Function::PipelineStage::kUndefined);

    EXPECT_EQ(Dis(m.Get()), R"(%f = func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, BugChromium324466107) {
    Func("f", Empty, ty.void_(),
         Vector{
             // Abstract type on the RHS - cannot be emitted.
             Assign(Phony(), Call(core::BuiltinFn::kFrexp, Call(ty.vec2<Infer>(), 2.0_a))),
         });

    auto m = Build();
    ASSERT_EQ(m, Success);

    EXPECT_EQ(Dis(m.Get()),
              R"(%f = func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, OverrideNoInitializer) {
    Override(Source{{1, 2}}, "a", ty.i32());

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* override = FindSingleInstruction<core::ir::Override>(m);

    ASSERT_NE(override, nullptr);
    ASSERT_EQ(override->Initializer(), nullptr);

    Source::Location loc{1u, 2u};
    EXPECT_EQ(m.SourceOf(override).range.begin, loc);

    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %a:i32 = override undef @id(0)
}

)");
}

TEST_F(IR_FromProgramTest, OverrideWithConstantInitializer) {
    Override("a", Expr(1_f));

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* override = FindSingleInstruction<core::ir::Override>(m);

    ASSERT_NE(override, nullptr);
    ASSERT_NE(override->Initializer(), nullptr);

    auto* init = override->Initializer()->As<core::ir::Constant>();
    ASSERT_NE(init, nullptr);
    EXPECT_FLOAT_EQ(1.0f, init->Value()->ValueAs<float>());

    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %a:f32 = override 1.0f @id(0)
}

)");
}

TEST_F(IR_FromProgramTest, OverrideWithAddInitializer) {
    Override("a", Add(1_u, 2_u));

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    auto* override = FindSingleInstruction<core::ir::Override>(m);

    ASSERT_NE(override, nullptr);
    ASSERT_NE(override->Initializer(), nullptr);

    auto* init = override->Initializer()->As<core::ir::Constant>();
    ASSERT_NE(init, nullptr);
    EXPECT_EQ(3u, init->Value()->ValueAs<uint32_t>());

    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %a:u32 = override 3u @id(0)
}

)");
}

TEST_F(IR_FromProgramTest, OverrideWithShortCircuitExpression) {
    auto* o0 = Override(Source{{1, 2}}, "a", ty.u32());
    auto* o1 = Override(Source{{2, 3}}, "b", ty.bool_());
    Override("c", LogicalAnd(o1, Equal(Div(1_u, o0), 0_u)));

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %a:u32 = override undef @id(0)
  %b:bool = override undef @id(1)
  %3:bool = constexpr_if %b [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %4:u32 = div 1u, %a
      %5:bool = eq %4, 0u
      exit_if %5  # constexpr_if_1
    }
    $B3: {  # false
      exit_if false  # constexpr_if_1
    }
  }
  %c:bool = override %3 @id(2)
}

)");
}

TEST_F(IR_FromProgramTest, OverrideShortCircuitStatementInFunction) {
    auto* o0 = Override(Source{{1, 2}}, "a", ty.u32());
    auto* o1 = Override(Source{{2, 3}}, "b", ty.bool_());
    auto* logical = LogicalAnd(o1, Equal(Div(1_u, o0), 0_u));
    WrapInFunction(logical);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %a:u32 = override undef @id(0)
  %b:bool = override undef @id(1)
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = constexpr_if %b [t: $B3, f: $B4] {  # constexpr_if_1
      $B3: {  # true
        %5:u32 = div 1u, %a
        %6:bool = eq %5, 0u
        exit_if %6  # constexpr_if_1
      }
      $B4: {  # false
        exit_if false  # constexpr_if_1
      }
    }
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, NonOverrideShortCircuitStatementInFunction) {
    auto* o0 = Override(Source{{1, 2}}, "a", ty.u32());
    auto* o1 = Decl(Let("x", Expr(true)));
    auto* logical = LogicalAnd(o1->variable, Equal(Div(1_u, o0), 0_u));
    WrapInFunction(o1, logical);

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();

    ASSERT_EQ(1u, m.functions.Length());

    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %a:u32 = override undef @id(0)
}

%test_function = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %x:bool = let true
    %4:bool = if %x [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %5:u32 = div 1u, %a
        %6:bool = eq %5, 0u
        exit_if %6  # if_1
      }
      $B4: {  # false
        exit_if false  # if_1
      }
    }
    %tint_symbol:bool = let %4
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, OverrideWithOverrideAddInitializer) {
    auto* z = Override("z", ty.u32());
    Override("a", Add(z, 2_u));

    auto res = Build();
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %z:u32 = override undef @id(0)
  %2:u32 = add %z, 2u
  %a:u32 = override %2 @id(1)
}

)");
}

TEST_F(IR_FromProgramTest, OverrideWithLetAddressOf) {
    auto* src = R"(
override x = 1;
var<workgroup> arr : array<u32, x>;

fn a() {
  let y = &arr;
}
)";
    auto res = Build(src);
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %x:i32 = override 1i @id(0)
  %arr:ptr<workgroup, array<u32, %x>, read_write> = var undef
}

%a = func():void {
  $B2: {
    %y:ptr<workgroup, array<u32, %x>, read_write> = let %arr
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, OverrideWithPhony) {
    auto* src = R"(
override cond : bool;
override zero_i32 = 0i;
override one_f32 = 1.0f;
override thirty_one = 31u;
override foo = cond && (one_f32 / 0) == 0;

@compute @workgroup_size(1)
fn main() {
  _ = cond;
_ = foo;
}
)";
    auto res = Build(src);
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %cond:bool = override undef @id(0)
  %zero_i32:i32 = override 0i @id(1)
  %one_f32:f32 = override 1.0f @id(2)
  %thirty_one:u32 = override 31u @id(3)
  %5:bool = constexpr_if %cond [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %6:f32 = div %one_f32, 0.0f
      %7:bool = eq %6, 0.0f
      exit_if %7  # constexpr_if_1
    }
    $B3: {  # false
      exit_if false  # constexpr_if_1
    }
  }
  %foo:bool = override %5 @id(4)
}

%main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B4: {
    %10:bool = let %cond
    %11:bool = let %foo
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, OverrideInExpressionInArraySize) {
    auto* src = R"(
override x : u32;
var<workgroup> arr : array<u32, x*2>;

@compute @workgroup_size(64)
fn main() {
  _ = arr[0];
}
)";

    auto res = Build(src);
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %x:u32 = override undef @id(0)
  %2:u32 = mul %x, 2u
  %arr:ptr<workgroup, array<u32, %2>, read_write> = var undef
}

%main = @compute @workgroup_size(64i, 1i, 1i) func():void {
  $B2: {
    %5:ptr<workgroup, u32, read_write> = access %arr, 0i
    %6:u32 = load %5
    ret
  }
}
)");
}

TEST_F(IR_FromProgramTest, OverrideInExpressionInWorkgroupSizeAttribute) {
    auto* src = R"(
override x : u32;

@compute @workgroup_size(x * 2)
fn main() { }
)";

    auto res = Build(src);
    ASSERT_EQ(res, Success);

    auto m = res.Move();
    EXPECT_EQ(Dis(m), R"($B1: {  # root
  %x:u32 = override undef @id(0)
  %2:u32 = mul %x, 2u
}

%main = @compute @workgroup_size(%2, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
