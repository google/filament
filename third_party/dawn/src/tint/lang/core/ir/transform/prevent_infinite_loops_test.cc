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

#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_PreventInfiniteLoopsTest = TransformTest;

TEST_F(IR_PreventInfiniteLoopsTest, NoModify_SimpleFiniteLoop) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* idx = b.Var<function, u32>("idx");
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* ifelse = b.If(b.LessThan<bool>(b.Load(idx), 10_u));
                b.Append(ifelse->True(), [&] {  //
                    b.ExitIf(ifelse);
                });
                b.Append(ifelse->False(), [&] {  //
                    b.ExitLoop(loop);
                });
                b.Continue(loop);

                b.Append(loop->Continuing(), [&] {  //
                    b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
                    b.NextIteration(loop);
                });
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var
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

    auto* expect = src;

    Run(PreventInfiniteLoops);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreventInfiniteLoopsTest, InfiniteLoop) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* idx = b.Var<function, u32>("idx");
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* ifelse = b.If(b.LessThan<bool>(b.Load(idx), 10_u));
                b.Append(ifelse->True(), [&] {  //
                    b.ExitIf(ifelse);
                });
                b.Append(ifelse->False(), [&] {  //
                    b.ExitLoop(loop);
                });
                b.Store(idx, 0_u);
                b.Continue(loop);

                b.Append(loop->Continuing(), [&] {  //
                    b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
                    b.NextIteration(loop);
                });
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var
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
        store %idx, 0u
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

    auto* expect = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %tint_loop_idx:ptr<function, vec2<u32>, read_write> = var
        %idx:ptr<function, u32, read_write> = var
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:vec2<u32> = load %tint_loop_idx
        %5:vec2<bool> = eq %4, vec2<u32>(4294967295u)
        %6:bool = all %5
        if %6 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        %7:u32 = load %idx
        %8:bool = lt %7, 10u
        if %8 [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            exit_if  # if_2
          }
          $B7: {  # false
            exit_loop  # loop_1
          }
        }
        store %idx, 0u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %9:u32 = load_vector_element %tint_loop_idx, 0u
        %tint_low_inc:u32 = add %9, 1u
        store_vector_element %tint_loop_idx, 0u, %tint_low_inc
        %11:bool = eq %tint_low_inc, 0u
        %tint_carry:u32 = convert %11
        %13:u32 = load_vector_element %tint_loop_idx, 1u
        %14:u32 = add %13, %tint_carry
        store_vector_element %tint_loop_idx, 1u, %14
        %15:u32 = load %idx
        %16:u32 = add %15, 1u
        store %idx, %16
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";

    Run(PreventInfiniteLoops);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreventInfiniteLoopsTest, EmptyInitializer) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(false);
            b.Append(ifelse->True(), [&] {  //
                b.ExitLoop(loop);
            });
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if false [t: $B4] {  # if_1
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
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %tint_loop_idx:ptr<function, vec2<u32>, read_write> = var
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:vec2<u32> = load %tint_loop_idx
        %4:vec2<bool> = eq %3, vec2<u32>(4294967295u)
        %5:bool = all %4
        if %5 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        if false [t: $B6] {  # if_2
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %6:u32 = load_vector_element %tint_loop_idx, 0u
        %tint_low_inc:u32 = add %6, 1u
        store_vector_element %tint_loop_idx, 0u, %tint_low_inc
        %8:bool = eq %tint_low_inc, 0u
        %tint_carry:u32 = convert %8
        %10:u32 = load_vector_element %tint_loop_idx, 1u
        %11:u32 = add %10, %tint_carry
        store_vector_element %tint_loop_idx, 1u, %11
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";

    Run(PreventInfiniteLoops);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreventInfiniteLoopsTest, EmptyContinuing) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* idx = b.Var<function, u32>("idx");
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* ifelse = b.If(b.LessThan<bool>(b.Load(idx), 10_u));
                b.Append(ifelse->True(), [&] {  //
                    b.ExitIf(ifelse);
                });
                b.Append(ifelse->False(), [&] {  //
                    b.ExitLoop(loop);
                });
                b.Continue(loop);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B6
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %tint_loop_idx:ptr<function, vec2<u32>, read_write> = var
        %idx:ptr<function, u32, read_write> = var
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %4:vec2<u32> = load %tint_loop_idx
        %5:vec2<bool> = eq %4, vec2<u32>(4294967295u)
        %6:bool = all %5
        if %6 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        %7:u32 = load %idx
        %8:bool = lt %7, 10u
        if %8 [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            exit_if  # if_2
          }
          $B7: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %9:u32 = load_vector_element %tint_loop_idx, 0u
        %tint_low_inc:u32 = add %9, 1u
        store_vector_element %tint_loop_idx, 0u, %tint_low_inc
        %11:bool = eq %tint_low_inc, 0u
        %tint_carry:u32 = convert %11
        %13:u32 = load_vector_element %tint_loop_idx, 1u
        %14:u32 = add %13, %tint_carry
        store_vector_element %tint_loop_idx, 1u, %14
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";

    Run(PreventInfiniteLoops);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreventInfiniteLoopsTest, MultipleNestedLoops) {
    // We will create two pairs of nested loops:
    // loop outer_1 {   <- finite
    //   loop inner_1 { <- infinite
    //   }
    // }
    // loop outer_2 {   <- infinite
    //   loop inner_2 { <- finite
    //   }
    // }
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop_outer_1 = b.Loop();
        b.Append(loop_outer_1->Initializer(), [&] {  //
            auto* idx_outer_1 = b.Var("idx", 0_u);
            b.NextIteration(loop_outer_1);

            b.Append(loop_outer_1->Body(), [&] {
                auto* ifelse_outer = b.If(b.LessThan<bool>(b.Load(idx_outer_1), 10_u));
                b.Append(ifelse_outer->True(), [&] {  //
                    b.ExitLoop(loop_outer_1);
                });

                auto* loop_inner_1 = b.Loop();
                b.Append(loop_inner_1->Initializer(), [&] {  //
                    auto* idx_inner_1 = b.Var("idx", 0_u);
                    b.NextIteration(loop_inner_1);

                    b.Append(loop_inner_1->Body(), [&] {
                        auto* ifelse_inner = b.If(b.LessThan<bool>(b.Load(idx_inner_1), 10_u));
                        b.Append(ifelse_inner->True(), [&] {  //
                            b.ExitLoop(loop_inner_1);
                        });
                        b.Continue(loop_inner_1);
                    });
                    b.Append(loop_inner_1->Continuing(), [&] {  //
                        b.NextIteration(loop_inner_1);
                    });
                });

                b.Continue(loop_outer_1);
            });
            b.Append(loop_outer_1->Continuing(), [&] {  //
                b.Store(idx_outer_1, b.Add<u32>(b.Load(idx_outer_1), 1_u));
                b.NextIteration(loop_outer_1);
            });
        });

        auto* loop_outer_2 = b.Loop();
        b.Append(loop_outer_2->Initializer(), [&] {  //
            auto* idx_outer_2 = b.Var("idx", 0_u);
            b.NextIteration(loop_outer_2);

            b.Append(loop_outer_2->Body(), [&] {
                auto* ifelse_outer = b.If(b.LessThan<bool>(b.Load(idx_outer_2), 10_u));
                b.Append(ifelse_outer->True(), [&] {  //
                    b.ExitLoop(loop_outer_2);
                });

                auto* loop_inner_2 = b.Loop();
                b.Append(loop_inner_2->Initializer(), [&] {  //
                    auto* idx_inner_2 = b.Var("idx", 0_u);
                    b.NextIteration(loop_inner_2);

                    b.Append(loop_inner_2->Body(), [&] {
                        auto* ifelse_inner = b.If(b.LessThan<bool>(b.Load(idx_inner_2), 10_u));
                        b.Append(ifelse_inner->True(), [&] {  //
                            b.ExitLoop(loop_inner_2);
                        });
                        b.Continue(loop_inner_2);
                    });
                    b.Append(loop_inner_2->Continuing(), [&] {  //
                        b.Store(idx_inner_2, b.Add<u32>(b.Load(idx_inner_2), 1_u));
                        b.NextIteration(loop_inner_2);
                    });
                });

                b.Continue(loop_outer_2);
            });
            b.Append(loop_outer_2->Continuing(), [&] {  //
                b.NextIteration(loop_outer_2);
            });
        });

        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var, 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        loop [i: $B6, b: $B7, c: $B8] {  # loop_2
          $B6: {  # initializer
            %idx_1:ptr<function, u32, read_write> = var, 0u  # %idx_1: 'idx'
            next_iteration  # -> $B7
          }
          $B7: {  # body
            %6:u32 = load %idx_1
            %7:bool = lt %6, 10u
            if %7 [t: $B9] {  # if_2
              $B9: {  # true
                exit_loop  # loop_2
              }
            }
            continue  # -> $B8
          }
          $B8: {  # continuing
            next_iteration  # -> $B7
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
    loop [i: $B10, b: $B11, c: $B12] {  # loop_3
      $B10: {  # initializer
        %idx_2:ptr<function, u32, read_write> = var, 0u  # %idx_2: 'idx'
        next_iteration  # -> $B11
      }
      $B11: {  # body
        %11:u32 = load %idx_2
        %12:bool = lt %11, 10u
        if %12 [t: $B13] {  # if_3
          $B13: {  # true
            exit_loop  # loop_3
          }
        }
        loop [i: $B14, b: $B15, c: $B16] {  # loop_4
          $B14: {  # initializer
            %idx_3:ptr<function, u32, read_write> = var, 0u  # %idx_3: 'idx'
            next_iteration  # -> $B15
          }
          $B15: {  # body
            %14:u32 = load %idx_3
            %15:bool = lt %14, 10u
            if %15 [t: $B17] {  # if_4
              $B17: {  # true
                exit_loop  # loop_4
              }
            }
            continue  # -> $B16
          }
          $B16: {  # continuing
            %16:u32 = load %idx_3
            %17:u32 = add %16, 1u
            store %idx_3, %17
            next_iteration  # -> $B15
          }
        }
        continue  # -> $B12
      }
      $B12: {  # continuing
        next_iteration  # -> $B11
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var, 0u
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %3:u32 = load %idx
        %4:bool = lt %3, 10u
        if %4 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        loop [i: $B6, b: $B7, c: $B8] {  # loop_2
          $B6: {  # initializer
            %tint_loop_idx:ptr<function, vec2<u32>, read_write> = var
            %idx_1:ptr<function, u32, read_write> = var, 0u  # %idx_1: 'idx'
            next_iteration  # -> $B7
          }
          $B7: {  # body
            %7:vec2<u32> = load %tint_loop_idx
            %8:vec2<bool> = eq %7, vec2<u32>(4294967295u)
            %9:bool = all %8
            if %9 [t: $B9] {  # if_2
              $B9: {  # true
                exit_loop  # loop_2
              }
            }
            %10:u32 = load %idx_1
            %11:bool = lt %10, 10u
            if %11 [t: $B10] {  # if_3
              $B10: {  # true
                exit_loop  # loop_2
              }
            }
            continue  # -> $B8
          }
          $B8: {  # continuing
            %12:u32 = load_vector_element %tint_loop_idx, 0u
            %tint_low_inc:u32 = add %12, 1u
            store_vector_element %tint_loop_idx, 0u, %tint_low_inc
            %14:bool = eq %tint_low_inc, 0u
            %tint_carry:u32 = convert %14
            %16:u32 = load_vector_element %tint_loop_idx, 1u
            %17:u32 = add %16, %tint_carry
            store_vector_element %tint_loop_idx, 1u, %17
            next_iteration  # -> $B7
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %18:u32 = load %idx
        %19:u32 = add %18, 1u
        store %idx, %19
        next_iteration  # -> $B3
      }
    }
    loop [i: $B11, b: $B12, c: $B13] {  # loop_3
      $B11: {  # initializer
        %tint_loop_idx_1:ptr<function, vec2<u32>, read_write> = var  # %tint_loop_idx_1: 'tint_loop_idx'
        %idx_2:ptr<function, u32, read_write> = var, 0u  # %idx_2: 'idx'
        next_iteration  # -> $B12
      }
      $B12: {  # body
        %22:vec2<u32> = load %tint_loop_idx_1
        %23:vec2<bool> = eq %22, vec2<u32>(4294967295u)
        %24:bool = all %23
        if %24 [t: $B14] {  # if_4
          $B14: {  # true
            exit_loop  # loop_3
          }
        }
        %25:u32 = load %idx_2
        %26:bool = lt %25, 10u
        if %26 [t: $B15] {  # if_5
          $B15: {  # true
            exit_loop  # loop_3
          }
        }
        loop [i: $B16, b: $B17, c: $B18] {  # loop_4
          $B16: {  # initializer
            %idx_3:ptr<function, u32, read_write> = var, 0u  # %idx_3: 'idx'
            next_iteration  # -> $B17
          }
          $B17: {  # body
            %28:u32 = load %idx_3
            %29:bool = lt %28, 10u
            if %29 [t: $B19] {  # if_6
              $B19: {  # true
                exit_loop  # loop_4
              }
            }
            continue  # -> $B18
          }
          $B18: {  # continuing
            %30:u32 = load %idx_3
            %31:u32 = add %30, 1u
            store %idx_3, %31
            next_iteration  # -> $B17
          }
        }
        continue  # -> $B13
      }
      $B13: {  # continuing
        %32:u32 = load_vector_element %tint_loop_idx_1, 0u
        %tint_low_inc_1:u32 = add %32, 1u
        store_vector_element %tint_loop_idx_1, 0u, %tint_low_inc_1
        %34:bool = eq %tint_low_inc_1, 0u
        %tint_carry_1:u32 = convert %34
        %36:u32 = load_vector_element %tint_loop_idx_1, 1u
        %37:u32 = add %36, %tint_carry_1
        store_vector_element %tint_loop_idx_1, 1u, %37
        next_iteration  # -> $B12
      }
    }
    ret
  }
}
)";

    Run(PreventInfiniteLoops);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PreventInfiniteLoopsTest, LoopResults) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        loop->SetResults(Vector{
            b.InstructionResult<u32>(),
            b.InstructionResult<i32>(),
            b.InstructionResult<f32>(),
            b.InstructionResult<bool>(),
        });
        b.Append(loop->Initializer(), [&] {
            auto* idx = b.Var<function, u32>("idx");
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* ifelse = b.If(b.LessThan<bool>(b.Load(idx), 10_u));
                b.Append(ifelse->True(), [&] {  //
                    b.ExitIf(ifelse);
                });
                b.Append(ifelse->False(), [&] {  //
                    b.ExitLoop(loop, 1_u, 2_i, 3_f, true);
                });
                b.Store(idx, 0_u);
                b.Continue(loop);

                b.Append(loop->Continuing(), [&] {  //
                    b.Store(idx, b.Add<u32>(b.Load(idx), 1_u));
                    b.NextIteration(loop);
                });
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    %2:u32, %3:i32, %4:f32, %5:bool = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %idx:ptr<function, u32, read_write> = var
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %7:u32 = load %idx
        %8:bool = lt %7, 10u
        if %8 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop 1u, 2i, 3.0f, true  # loop_1
          }
        }
        store %idx, 0u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %9:u32 = load %idx
        %10:u32 = add %9, 1u
        store %idx, %10
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%func = func():void {
  $B1: {
    %2:u32, %3:i32, %4:f32, %5:bool = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        %tint_loop_idx:ptr<function, vec2<u32>, read_write> = var
        %idx:ptr<function, u32, read_write> = var
        next_iteration  # -> $B3
      }
      $B3: {  # body
        %8:vec2<u32> = load %tint_loop_idx
        %9:vec2<bool> = eq %8, vec2<u32>(4294967295u)
        %10:bool = all %9
        if %10 [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop undef, undef, undef, undef  # loop_1
          }
        }
        %11:u32 = load %idx
        %12:bool = lt %11, 10u
        if %12 [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            exit_if  # if_2
          }
          $B7: {  # false
            exit_loop 1u, 2i, 3.0f, true  # loop_1
          }
        }
        store %idx, 0u
        continue  # -> $B4
      }
      $B4: {  # continuing
        %13:u32 = load_vector_element %tint_loop_idx, 0u
        %tint_low_inc:u32 = add %13, 1u
        store_vector_element %tint_loop_idx, 0u, %tint_low_inc
        %15:bool = eq %tint_low_inc, 0u
        %tint_carry:u32 = convert %15
        %17:u32 = load_vector_element %tint_loop_idx, 1u
        %18:u32 = add %17, %tint_carry
        store_vector_element %tint_loop_idx, 1u, %18
        %19:u32 = load %idx
        %20:u32 = add %19, 1u
        store %idx, %20
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)";

    Run(PreventInfiniteLoops);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
