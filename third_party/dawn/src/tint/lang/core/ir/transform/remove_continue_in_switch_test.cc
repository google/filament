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

#include "src/tint/lang/core/ir/transform/remove_continue_in_switch.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_RemoveContinueInSwitchTest = TransformTest;

TEST_F(IR_RemoveContinueInSwitchTest, NoModify_ContinueNotInSwitch) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* swtch = b.Switch(42_i);
            auto* def_case = b.DefaultCase(swtch);
            b.Append(def_case, [&] {  //
                b.ExitSwitch(swtch);
            });
            b.Continue(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        switch 42i [c: (default, $B3)] {  # switch_1
          $B3: {  # case
            exit_switch  # switch_1
          }
        }
        continue  # -> $B4
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(RemoveContinueInSwitch);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveContinueInSwitchTest, ContinueInSwitchCase_WithoutBreakIf) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* swtch = b.Switch(42_i);
            auto* def_case = b.DefaultCase(swtch);
            b.Append(def_case, [&] {  //
                b.Continue(loop);
            });
            b.Continue(loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        switch 42i [c: (default, $B3)] {  # switch_1
          $B3: {  # case
            continue  # -> $B4
          }
        }
        continue  # -> $B4
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
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        %tint_continue:ptr<function, bool, read_write> = var undef
        switch 42i [c: (default, $B3)] {  # switch_1
          $B3: {  # case
            store %tint_continue, true
            exit_switch  # switch_1
          }
        }
        %3:bool = load %tint_continue
        if %3 [t: $B4] {  # if_1
          $B4: {  # true
            continue  # -> $B5
          }
        }
        continue  # -> $B5
      }
    }
    ret
  }
}
)";

    Run(RemoveContinueInSwitch);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveContinueInSwitchTest, ContinueInSwitchCase_WithBreakIf) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* swtch = b.Switch(42_i);
            auto* def_case = b.DefaultCase(swtch);
            b.Append(def_case, [&] {  //
                b.Continue(loop);
            });
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        switch 42i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %tint_continue:ptr<function, bool, read_write> = var undef
        switch 42i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            store %tint_continue, true
            exit_switch  # switch_1
          }
        }
        %3:bool = load %tint_continue
        if %3 [t: $B5] {  # if_1
          $B5: {  # true
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveContinueInSwitch);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveContinueInSwitchTest, ContinueInMultipleCases) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* swtch = b.Switch(42_i);
            auto* case_a = b.Case(swtch, Vector{b.Constant(1_i)});
            b.Append(case_a, [&] {  //
                b.Continue(loop);
            });
            auto* case_b = b.Case(swtch, Vector{b.Constant(2_i)});
            b.Append(case_b, [&] {  //
                b.Continue(loop);
            });
            auto* def_case = b.DefaultCase(swtch);
            b.Append(def_case, [&] {  //
                b.Continue(loop);
            });
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        switch 42i [c: (1i, $B4), c: (2i, $B5), c: (default, $B6)] {  # switch_1
          $B4: {  # case
            continue  # -> $B3
          }
          $B5: {  # case
            continue  # -> $B3
          }
          $B6: {  # case
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %tint_continue:ptr<function, bool, read_write> = var undef
        switch 42i [c: (1i, $B4), c: (2i, $B5), c: (default, $B6)] {  # switch_1
          $B4: {  # case
            store %tint_continue, true
            exit_switch  # switch_1
          }
          $B5: {  # case
            store %tint_continue, true
            exit_switch  # switch_1
          }
          $B6: {  # case
            store %tint_continue, true
            exit_switch  # switch_1
          }
        }
        %3:bool = load %tint_continue
        if %3 [t: $B7] {  # if_1
          $B7: {  # true
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveContinueInSwitch);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveContinueInSwitchTest, ContinueInMultipleSwitches) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* swtch_a = b.Switch(42_i);
            auto* def_case_a = b.DefaultCase(swtch_a);
            b.Append(def_case_a, [&] {  //
                b.Continue(loop);
            });
            auto* swtch_b = b.Switch(43_i);
            auto* def_case_b = b.DefaultCase(swtch_b);
            b.Append(def_case_b, [&] {  //
                b.Continue(loop);
            });
            auto* swtch_c = b.Switch(44_i);
            auto* def_case_c = b.DefaultCase(swtch_c);
            b.Append(def_case_c, [&] {  //
                b.Continue(loop);
            });
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        switch 42i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            continue  # -> $B3
          }
        }
        switch 43i [c: (default, $B5)] {  # switch_2
          $B5: {  # case
            continue  # -> $B3
          }
        }
        switch 44i [c: (default, $B6)] {  # switch_3
          $B6: {  # case
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %tint_continue:ptr<function, bool, read_write> = var undef
        switch 42i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            store %tint_continue, true
            exit_switch  # switch_1
          }
        }
        %3:bool = load %tint_continue
        if %3 [t: $B5] {  # if_1
          $B5: {  # true
            continue  # -> $B3
          }
        }
        %tint_continue_1:ptr<function, bool, read_write> = var undef  # %tint_continue_1: 'tint_continue'
        switch 43i [c: (default, $B6)] {  # switch_2
          $B6: {  # case
            store %tint_continue_1, true
            exit_switch  # switch_2
          }
        }
        %5:bool = load %tint_continue_1
        if %5 [t: $B7] {  # if_2
          $B7: {  # true
            continue  # -> $B3
          }
        }
        %tint_continue_2:ptr<function, bool, read_write> = var undef  # %tint_continue_2: 'tint_continue'
        switch 44i [c: (default, $B8)] {  # switch_3
          $B8: {  # case
            store %tint_continue_2, true
            exit_switch  # switch_3
          }
        }
        %7:bool = load %tint_continue_2
        if %7 [t: $B9] {  # if_3
          $B9: {  # true
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveContinueInSwitch);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveContinueInSwitchTest, ContinueInSwitchCaseNestedInsideIf) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* outer_if = b.If(true);
            b.Append(outer_if->True(), [&] {
                auto* swtch = b.Switch(42_i);
                auto* def_case = b.DefaultCase(swtch);
                b.Append(def_case, [&] {  //
                    auto* inner_if = b.If(true);
                    b.Append(inner_if->True(), [&] {  //
                        b.Continue(loop);
                    });
                    b.Unreachable();
                });
                b.Unreachable();
            });
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4] {  # if_1
          $B4: {  # true
            switch 42i [c: (default, $B5)] {  # switch_1
              $B5: {  # case
                if true [t: $B6] {  # if_2
                  $B6: {  # true
                    continue  # -> $B3
                  }
                }
                unreachable
              }
            }
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4] {  # if_1
          $B4: {  # true
            %tint_continue:ptr<function, bool, read_write> = var undef
            switch 42i [c: (default, $B5)] {  # switch_1
              $B5: {  # case
                if true [t: $B6] {  # if_2
                  $B6: {  # true
                    store %tint_continue, true
                    exit_switch  # switch_1
                  }
                }
                unreachable
              }
            }
            %3:bool = load %tint_continue
            if %3 [t: $B7] {  # if_3
              $B7: {  # true
                continue  # -> $B3
              }
            }
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveContinueInSwitch);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_RemoveContinueInSwitchTest, ContinueInSwitchInsideAnotherSwitch) {
    auto* func = b.Function("func", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* outer_switch = b.Switch(42_i);
            auto* outer_def_case = b.DefaultCase(outer_switch);
            b.Append(outer_def_case, [&] {  //
                auto* inner_switch = b.Switch(42_i);
                auto* inner_def_case = b.DefaultCase(inner_switch);
                b.Append(inner_def_case, [&] {  //
                    b.Continue(loop);
                });
                b.ExitSwitch(outer_switch);
            });
            b.Continue(loop);
            b.Append(loop->Continuing(), [&] {  //
                b.BreakIf(loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        switch 42i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            switch 42i [c: (default, $B5)] {  # switch_2
              $B5: {  # case
                continue  # -> $B3
              }
            }
            exit_switch  # switch_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %tint_continue:ptr<function, bool, read_write> = var undef
        switch 42i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            %tint_continue_1:ptr<function, bool, read_write> = var undef  # %tint_continue_1: 'tint_continue'
            switch 42i [c: (default, $B5)] {  # switch_2
              $B5: {  # case
                store %tint_continue_1, true
                exit_switch  # switch_2
              }
            }
            %4:bool = load %tint_continue_1
            if %4 [t: $B6] {  # if_1
              $B6: {  # true
                store %tint_continue, true
                exit_switch  # switch_1
              }
            }
            exit_switch  # switch_1
          }
        }
        %5:bool = load %tint_continue
        if %5 [t: $B7] {  # if_2
          $B7: {  # true
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveContinueInSwitch);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
