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

#include "src/tint/lang/spirv/writer/raise/case_switch_to_if_else.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_CaseSwitchToIfElseTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, BasicSwitch) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(-1_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(2_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (-1i, $B2), c: (2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %3:bool = eq %param0, -1i
        if %3 [t: $B3] {  # if_1
          $B3: {  # true
            ret
          }
        }
        %4:bool = eq %param0, 2i
        if %4 [t: $B4] {  # if_2
          $B4: {  # true
            ret
          }
        }
        if true [t: $B5] {  # if_3
          $B5: {  # true
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

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, ReorderedBasicSwitch) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(2_i)}), [&] {  //
            b.Return(func);
        });

        b.Append(b.Case(s, {b.Constant(-1_i)}), [&] {  //
            b.Return(func);
        });

        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (default, $B2), c: (2i, $B3), c: (-1i, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %3:bool = eq %param0, 2i
        if %3 [t: $B3] {  # if_1
          $B3: {  # true
            ret
          }
        }
        %4:bool = eq %param0, -1i
        if %4 [t: $B4] {  # if_2
          $B4: {  # true
            ret
          }
        }
        if true [t: $B5] {  # if_3
          $B5: {  # true
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

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, SwitchOnlyDefault) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret
      }
    }
    unreachable
  }
}
)";

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, SwitchWithReturn) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(-1_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(2_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (-1i, $B2), c: (2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %3:bool = eq %param0, -1i
        if %3 [t: $B3] {  # if_1
          $B3: {  # true
            ret
          }
        }
        %4:bool = eq %param0, 2i
        if %4 [t: $B4] {  # if_2
          $B4: {  # true
            ret
          }
        }
        if true [t: $B5] {  # if_3
          $B5: {  # true
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

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, SwitchMultiSelectorCase) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(-1_i), b.Constant(2_i)}), [&] { b.Return(func); });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (-1i 2i, $B2), c: (default, $B3)] {  # switch_1
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
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %3:bool = eq %param0, -1i
        %4:bool = eq %param0, 2i
        %5:bool = or %4, %3
        if %5 [t: $B3] {  # if_1
          $B3: {  # true
            ret
          }
        }
        if true [t: $B4] {  # if_2
          $B4: {  # true
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

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, NestedSwitch) {
    auto* cond1 = b.FunctionParam("cond1", ty.i32());
    auto* cond2 = b.FunctionParam("cond2", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond1, cond2});
    b.Append(func->Block(), [&] {
        auto* s1 = b.Switch(cond1);
        b.Append(b.Case(s1, {b.Constant(1_i), b.Constant(-1_i)}), [&] {
            auto* s2 = b.Switch(cond2);
            b.Append(b.Case(s2, {b.Constant(-2_i), b.Constant(2_i)}), [&] {  //
                b.Return(func);
            });
            b.Append(b.DefaultCase(s2), [&] {  //
                b.Return(func);
            });
            b.Unreachable();
        });
        b.Append(b.DefaultCase(s1), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%cond1:i32, %cond2:i32):void {
  $B1: {
    switch %cond1 [c: (-1i 1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        switch %cond2 [c: (-2i 2i, $B4), c: (default, $B5)] {  # switch_2
          $B4: {  # case
            ret
          }
          $B5: {  # case
            ret
          }
        }
        unreachable
      }
      $B3: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%cond1:i32, %cond2:i32):void {
  $B1: {
    switch %cond1 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %4:bool = eq %cond1, 1i
        %5:bool = eq %cond1, -1i
        %6:bool = or %5, %4
        if %6 [t: $B3] {  # if_1
          $B3: {  # true
            switch %cond2 [c: (default, $B4)] {  # switch_2
              $B4: {  # case
                %7:bool = eq %cond2, -2i
                %8:bool = eq %cond2, 2i
                %9:bool = or %8, %7
                if %9 [t: $B5] {  # if_2
                  $B5: {  # true
                    ret
                  }
                }
                if true [t: $B6] {  # if_3
                  $B6: {  # true
                    ret
                  }
                }
                unreachable
              }
            }
            unreachable
          }
        }
        if true [t: $B7] {  # if_4
          $B7: {  # true
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

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, MixedIfAndSwitch) {
    auto* cond1 = b.FunctionParam("cond1", ty.i32());
    auto* cond2 = b.FunctionParam("cond2", ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond1, cond2});

    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond1);
        b.Append(b.Case(s, {b.Constant(1_i), b.Constant(-1_i)}), [&] {
            auto* ifelse = b.If(cond2);
            b.Append(ifelse->True(), [&] {  //
                b.Return(func);
            });
            b.Append(ifelse->False(), [&] {  //
                b.Return(func);
            });
            b.Unreachable();
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%cond1:i32, %cond2:bool):void {
  $B1: {
    switch %cond1 [c: (-1i 1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        if %cond2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            ret
          }
          $B5: {  # false
            ret
          }
        }
        unreachable
      }
      $B3: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%cond1:i32, %cond2:bool):void {
  $B1: {
    switch %cond1 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %4:bool = eq %cond1, 1i
        %5:bool = eq %cond1, -1i
        %6:bool = or %5, %4
        if %6 [t: $B3] {  # if_1
          $B3: {  # true
            if %cond2 [t: $B4, f: $B5] {  # if_2
              $B4: {  # true
                ret
              }
              $B5: {  # false
                ret
              }
            }
            unreachable
          }
        }
        if true [t: $B6] {  # if_3
          $B6: {  # true
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

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, BasicSwitch_VarAssignment) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* var = b.Var("v", b.Zero(ty.i32()));
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(1_i)}), [&] {
            b.Store(var, 10_i);
            b.ExitSwitch(s);
        });
        b.Append(b.Case(s, {b.Constant(-2_i)}), [&] {
            b.Store(var, 20_i);
            b.ExitSwitch(s);
        });
        b.Append(b.DefaultCase(s), [&] {
            b.Store(var, 30_i);
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %param0 [c: (1i, $B2), c: (-2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        store %v, 10i
        exit_switch  # switch_1
      }
      $B3: {  # case
        store %v, 20i
        exit_switch  # switch_1
      }
      $B4: {  # case
        store %v, 30i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%param0:i32):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %4:bool = eq %param0, 1i
        if %4 [t: $B3] {  # if_1
          $B3: {  # true
            store %v, 10i
            exit_switch  # switch_1
          }
        }
        %5:bool = eq %param0, -2i
        if %5 [t: $B4] {  # if_2
          $B4: {  # true
            store %v, 20i
            exit_switch  # switch_1
          }
        }
        if true [t: $B5] {  # if_3
          $B5: {  # true
            store %v, 30i
            exit_switch  # switch_1
          }
        }
        unreachable
      }
    }
    ret
  }
}
)";

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, SwitchMultiSelectorCase_VarAssignment) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* var = b.Var("v", b.Zero(ty.i32()));
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(1_i), b.Constant(-2_i)}), [&] {
            b.Store(var, 10_i);
            b.ExitSwitch(s);
        });
        b.Append(b.DefaultCase(s), [&] {
            b.Store(var, 20_i);
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %param0 [c: (-2i 1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        store %v, 10i
        exit_switch  # switch_1
      }
      $B3: {  # case
        store %v, 20i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%param0:i32):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %4:bool = eq %param0, 1i
        %5:bool = eq %param0, -2i
        %6:bool = or %5, %4
        if %6 [t: $B3] {  # if_1
          $B3: {  # true
            store %v, 10i
            exit_switch  # switch_1
          }
        }
        if true [t: $B4] {  # if_2
          $B4: {  # true
            store %v, 20i
            exit_switch  # switch_1
          }
        }
        unreachable
      }
    }
    ret
  }
}
)";

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, NestedSwitch_VarAssignment) {
    auto* cond1 = b.FunctionParam("cond1", ty.i32());
    auto* cond2 = b.FunctionParam("cond2", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond1, cond2});
    b.Append(func->Block(), [&] {
        auto* var = b.Var("v", b.Zero(ty.i32()));
        auto* s1 = b.Switch(cond1);
        b.Append(b.Case(s1, {b.Constant(1_i), b.Constant(-1_i)}), [&] {
            auto* s2 = b.Switch(cond2);
            b.Append(b.Case(s2, {b.Constant(2_i), b.Constant(-2_i)}), [&] {
                b.Store(var, 10_i);
                b.ExitSwitch(s2);
            });
            b.Append(b.DefaultCase(s2), [&] {
                b.Store(var, 20_i);
                b.ExitSwitch(s2);
            });
            b.ExitSwitch(s1);
        });
        b.Append(b.DefaultCase(s1), [&] {
            b.Store(var, 30_i);
            b.ExitSwitch(s1);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%cond1:i32, %cond2:i32):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond1 [c: (-1i 1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        switch %cond2 [c: (-2i 2i, $B4), c: (default, $B5)] {  # switch_2
          $B4: {  # case
            store %v, 10i
            exit_switch  # switch_2
          }
          $B5: {  # case
            store %v, 20i
            exit_switch  # switch_2
          }
        }
        exit_switch  # switch_1
      }
      $B3: {  # case
        store %v, 30i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%cond1:i32, %cond2:i32):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond1 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %5:bool = eq %cond1, 1i
        %6:bool = eq %cond1, -1i
        %7:bool = or %6, %5
        if %7 [t: $B3] {  # if_1
          $B3: {  # true
            switch %cond2 [c: (default, $B4)] {  # switch_2
              $B4: {  # case
                %8:bool = eq %cond2, 2i
                %9:bool = eq %cond2, -2i
                %10:bool = or %9, %8
                if %10 [t: $B5] {  # if_2
                  $B5: {  # true
                    store %v, 10i
                    exit_switch  # switch_2
                  }
                }
                if true [t: $B6] {  # if_3
                  $B6: {  # true
                    store %v, 20i
                    exit_switch  # switch_2
                  }
                }
                unreachable
              }
            }
            exit_switch  # switch_1
          }
        }
        if true [t: $B7] {  # if_4
          $B7: {  # true
            store %v, 30i
            exit_switch  # switch_1
          }
        }
        unreachable
      }
    }
    ret
  }
}
)";

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, MixedIfAndSwitch_VarAssignment) {
    auto* cond1 = b.FunctionParam("cond1", ty.i32());
    auto* cond2 = b.FunctionParam("cond2", ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond1, cond2});

    b.Append(func->Block(), [&] {
        auto* var = b.Var("v", b.Zero(ty.i32()));
        auto* s = b.Switch(cond1);
        b.Append(b.Case(s, {b.Constant(1_i), b.Constant(-1_i)}), [&] {
            auto* ifelse = b.If(cond2);
            b.Append(ifelse->True(), [&] {
                b.Store(var, 10_i);
                b.ExitIf(ifelse);
            });
            b.Append(ifelse->False(), [&] {
                b.Store(var, 20_i);
                b.ExitIf(ifelse);
            });
            b.ExitSwitch(s);
        });
        b.Append(b.DefaultCase(s), [&] {
            b.Store(var, 30_i);
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%cond1:i32, %cond2:bool):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond1 [c: (-1i 1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        if %cond2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            store %v, 10i
            exit_if  # if_1
          }
          $B5: {  # false
            store %v, 20i
            exit_if  # if_1
          }
        }
        exit_switch  # switch_1
      }
      $B3: {  # case
        store %v, 30i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%cond1:i32, %cond2:bool):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond1 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %5:bool = eq %cond1, 1i
        %6:bool = eq %cond1, -1i
        %7:bool = or %6, %5
        if %7 [t: $B3] {  # if_1
          $B3: {  # true
            if %cond2 [t: $B4, f: $B5] {  # if_2
              $B4: {  # true
                store %v, 10i
                exit_if  # if_2
              }
              $B5: {  # false
                store %v, 20i
                exit_if  # if_2
              }
            }
            exit_switch  # switch_1
          }
        }
        if true [t: $B6] {  # if_3
          $B6: {  # true
            store %v, 30i
            exit_switch  # switch_1
          }
        }
        unreachable
      }
    }
    ret
  }
}
)";

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, RangeSmall) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(1_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(2_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (1i, $B2), c: (2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, RangeSmallSigned) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(-1_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(-2_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (-1i, $B2), c: (-2i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, RangeSmallSigned2) {
    auto* cond = b.FunctionParam("param0", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(-2147483640_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(2147483640_i)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:i32):void {
  $B1: {
    switch %param0 [c: (-2147483640i, $B2), c: (2147483640i, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, RangeSmallUnsigned) {
    auto* cond = b.FunctionParam("param0", ty.u32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(2'000'000'000_u)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(2'000'000'001_u)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:u32):void {
  $B1: {
    switch %param0 [c: (2000000000u, $B2), c: (2000000001u, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, RangeSmallUnsignedLarge) {
    auto* cond = b.FunctionParam("param0", ty.u32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(0_u)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.Case(s, {b.Constant(4000'000'000_u)}), [&] {  //
            b.Return(func);
        });
        b.Append(b.DefaultCase(s), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%param0:u32):void {
  $B1: {
    switch %param0 [c: (0u, $B2), c: (4000000000u, $B3), c: (default, $B4)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        ret
      }
      $B4: {  # case
        ret
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    Run(CaseSwitchToIfElse);
    auto* expect = R"(
%foo = func(%param0:u32):void {
  $B1: {
    switch %param0 [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %3:bool = eq %param0, 0u
        if %3 [t: $B3] {  # if_1
          $B3: {  # true
            ret
          }
        }
        %4:bool = eq %param0, 4000000000u
        if %4 [t: $B4] {  # if_2
          $B4: {  # true
            ret
          }
        }
        if true [t: $B5] {  # if_3
          $B5: {  # true
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
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, BreakInsideIfInCase) {
    auto* cond = b.FunctionParam("cond", ty.i32());
    auto* pred = b.FunctionParam("pred", ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond, pred});

    b.Append(func->Block(), [&] {
        auto* var = b.Var("v", b.Zero(ty.i32()));
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(1_i), b.Constant(-1_i)}), [&] {
            b.Store(var, 10_i);
            auto* ifelse = b.If(pred);
            b.Append(ifelse->True(), [&] {
                b.Store(var, 11_i);
                b.ExitSwitch(s);  // break
            });
            b.Store(var, 12_i);
            b.ExitSwitch(s);
        });
        b.Append(b.DefaultCase(s), [&] {
            b.Store(var, 20_i);
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%cond:i32, %pred:bool):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond [c: (-1i 1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        store %v, 10i
        if %pred [t: $B4] {  # if_1
          $B4: {  # true
            store %v, 11i
            exit_switch  # switch_1
          }
        }
        store %v, 12i
        exit_switch  # switch_1
      }
      $B3: {  # case
        store %v, 20i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%cond:i32, %pred:bool):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %5:bool = eq %cond, 1i
        %6:bool = eq %cond, -1i
        %7:bool = or %6, %5
        if %7 [t: $B3] {  # if_1
          $B3: {  # true
            store %v, 10i
            if %pred [t: $B4] {  # if_2
              $B4: {  # true
                store %v, 11i
                exit_switch  # switch_1
              }
            }
            store %v, 12i
            exit_switch  # switch_1
          }
        }
        if true [t: $B5] {  # if_3
          $B5: {  # true
            store %v, 20i
            exit_switch  # switch_1
          }
        }
        unreachable
      }
    }
    ret
  }
}
)";

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_CaseSwitchToIfElseTest, MultipleBreaksInCase) {
    auto* cond = b.FunctionParam("cond", ty.i32());
    auto* pred1 = b.FunctionParam("pred1", ty.bool_());
    auto* pred2 = b.FunctionParam("pred2", ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond, pred1, pred2});

    b.Append(func->Block(), [&] {
        auto* var = b.Var("v", b.Zero(ty.i32()));
        auto* s = b.Switch(cond);
        b.Append(b.Case(s, {b.Constant(1_i), b.Constant(-1_i)}), [&] {
            b.Store(var, 10_i);
            auto* if1 = b.If(pred1);
            b.Append(if1->True(), [&] {
                b.Store(var, 11_i);
                b.ExitSwitch(s);  // break
            });

            auto* if2 = b.If(pred2);
            b.Append(if2->True(), [&] {
                b.Store(var, 12_i);
                b.ExitSwitch(s);  // break
            });
            b.Store(var, 13_i);
            b.ExitSwitch(s);
        });
        b.Append(b.DefaultCase(s), [&] {
            b.Store(var, 20_i);
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%cond:i32, %pred1:bool, %pred2:bool):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond [c: (-1i 1i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        store %v, 10i
        if %pred1 [t: $B4] {  # if_1
          $B4: {  # true
            store %v, 11i
            exit_switch  # switch_1
          }
        }
        if %pred2 [t: $B5] {  # if_2
          $B5: {  # true
            store %v, 12i
            exit_switch  # switch_1
          }
        }
        store %v, 13i
        exit_switch  # switch_1
      }
      $B3: {  # case
        store %v, 20i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%cond:i32, %pred1:bool, %pred2:bool):void {
  $B1: {
    %v:ptr<function, i32, read_write> = var 0i
    switch %cond [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %6:bool = eq %cond, 1i
        %7:bool = eq %cond, -1i
        %8:bool = or %7, %6
        if %8 [t: $B3] {  # if_1
          $B3: {  # true
            store %v, 10i
            if %pred1 [t: $B4] {  # if_2
              $B4: {  # true
                store %v, 11i
                exit_switch  # switch_1
              }
            }
            if %pred2 [t: $B5] {  # if_3
              $B5: {  # true
                store %v, 12i
                exit_switch  # switch_1
              }
            }
            store %v, 13i
            exit_switch  # switch_1
          }
        }
        if true [t: $B6] {  # if_4
          $B6: {  # true
            store %v, 20i
            exit_switch  # switch_1
          }
        }
        unreachable
      }
    }
    ret
  }
}
)";

    Run(CaseSwitchToIfElse);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
