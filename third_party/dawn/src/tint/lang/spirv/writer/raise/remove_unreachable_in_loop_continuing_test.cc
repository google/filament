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

#include "src/tint/lang/spirv/writer/raise/remove_unreachable_in_loop_continuing.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_RemoveUnreachableInLoopContinuingTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_RemoveUnreachableInLoopContinuingTest, NoModify_TopLevel) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {  //
            b.Return(func);
        });
        b.Append(ifelse->False(), [&] {  //
            b.Return(func);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func():void {
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
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(RemoveUnreachableInLoopContinuing);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_RemoveUnreachableInLoopContinuingTest, NoModify_InLoopBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
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
            b.ExitLoop(outer_loop);
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        loop [b: $B3, c: $B4] {  # loop_2
          $B3: {  # body
            if true [t: $B5, f: $B6] {  # if_1
              $B5: {  # true
                exit_loop  # loop_2
              }
              $B6: {  # false
                exit_loop  # loop_2
              }
            }
            unreachable
          }
          $B4: {  # continuing
            break_if true  # -> [t: exit_loop loop_2, f: $B3]
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

    auto* expect = src;

    Run(RemoveUnreachableInLoopContinuing);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_RemoveUnreachableInLoopContinuingTest, InContinuing_NestedInIfBlock) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(), [&] {
                auto* ifelse = b.If(true);
                b.Append(ifelse->True(), [&] {  //
                    b.ExitIf(ifelse);
                });
                b.Append(ifelse->False(), [&] {  //
                    b.Unreachable();
                });
                b.BreakIf(outer_loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            unreachable
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveUnreachableInLoopContinuing);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_RemoveUnreachableInLoopContinuingTest, InContinuing_NestedInSwitchCase) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(), [&] {
                auto* swtch = b.Switch(1_i);
                auto* case_1 = b.Case(swtch, {b.Constant(1_i)});
                auto* def = b.DefaultCase(swtch);
                b.Append(case_1, [&] {  //
                    b.ExitSwitch(swtch);
                });
                b.Append(def, [&] {  //
                    b.Unreachable();
                });
                b.BreakIf(outer_loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        switch 1i [c: (1i, $B4), c: (default, $B5)] {  # switch_1
          $B4: {  # case
            exit_switch  # switch_1
          }
          $B5: {  # case
            unreachable
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        switch 1i [c: (1i, $B4), c: (default, $B5)] {  # switch_1
          $B4: {  # case
            exit_switch  # switch_1
          }
          $B5: {  # case
            exit_switch  # switch_1
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveUnreachableInLoopContinuing);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_RemoveUnreachableInLoopContinuingTest, InContinuing_NestedInLoopBody) {
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

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        loop [b: $B4, c: $B5] {  # loop_2
          $B4: {  # body
            if true [t: $B6, f: $B7] {  # if_1
              $B6: {  # true
                exit_loop  # loop_2
              }
              $B7: {  # false
                exit_loop  # loop_2
              }
            }
            unreachable
          }
          $B5: {  # continuing
            break_if true  # -> [t: exit_loop loop_2, f: $B4]
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        loop [b: $B4, c: $B5] {  # loop_2
          $B4: {  # body
            if true [t: $B6, f: $B7] {  # if_1
              $B6: {  # true
                exit_loop  # loop_2
              }
              $B7: {  # false
                exit_loop  # loop_2
              }
            }
            exit_loop  # loop_2
          }
          $B5: {  # continuing
            break_if true  # -> [t: exit_loop loop_2, f: $B4]
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveUnreachableInLoopContinuing);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_RemoveUnreachableInLoopContinuingTest,
       InContinuing_NestedInLoopBody_WithResults) {
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

    auto* src = R"(
%foo = func():i32 {
  $B1: {
    %2:i32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        %3:i32 = loop [b: $B4, c: $B5] {  # loop_2
          $B4: {  # body
            if true [t: $B6, f: $B7] {  # if_1
              $B6: {  # true
                exit_loop 1i  # loop_2
              }
              $B7: {  # false
                exit_loop 2i  # loop_2
              }
            }
            unreachable
          }
          $B5: {  # continuing
            break_if true exit_loop: [ 3i ]  # -> [t: exit_loop loop_2, f: $B4]
          }
        }
        break_if true exit_loop: [ %3 ]  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():i32 {
  $B1: {
    %2:i32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        %3:i32 = loop [b: $B4, c: $B5] {  # loop_2
          $B4: {  # body
            if true [t: $B6, f: $B7] {  # if_1
              $B6: {  # true
                exit_loop 1i  # loop_2
              }
              $B7: {  # false
                exit_loop 2i  # loop_2
              }
            }
            exit_loop undef  # loop_2
          }
          $B5: {  # continuing
            break_if true exit_loop: [ 3i ]  # -> [t: exit_loop loop_2, f: $B4]
          }
        }
        break_if true exit_loop: [ %3 ]  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret %2
  }
}
)";

    Run(RemoveUnreachableInLoopContinuing);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_RemoveUnreachableInLoopContinuingTest, InContinuing_DeeplyNestedInLoopBody) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* outer_loop = b.Loop();
        b.Append(outer_loop->Body(), [&] {
            b.Continue(outer_loop);

            b.Append(outer_loop->Continuing(), [&] {
                auto* outer_if = b.If(true);
                b.Append(outer_if->True(), [&] {
                    auto* inner_loop = b.Loop();
                    b.Append(inner_loop->Body(), [&] {
                        auto* inner_if = b.If(true);
                        b.Append(inner_if->True(), [&] {
                            auto* ifelse = b.If(true);
                            b.Append(ifelse->True(), [&] {  //
                                b.ExitLoop(inner_loop);
                            });
                            b.Append(ifelse->False(), [&] {  //
                                b.ExitLoop(inner_loop);
                            });
                            b.Unreachable();
                        });
                        b.Unreachable();

                        b.Append(inner_loop->Continuing(), [&] {  //
                            b.BreakIf(inner_loop, true);
                        });
                    });
                    b.ExitIf(outer_if);
                });
                b.BreakIf(outer_loop, true);
            });
        });
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        if true [t: $B4] {  # if_1
          $B4: {  # true
            loop [b: $B5, c: $B6] {  # loop_2
              $B5: {  # body
                if true [t: $B7] {  # if_2
                  $B7: {  # true
                    if true [t: $B8, f: $B9] {  # if_3
                      $B8: {  # true
                        exit_loop  # loop_2
                      }
                      $B9: {  # false
                        exit_loop  # loop_2
                      }
                    }
                    unreachable
                  }
                }
                unreachable
              }
              $B6: {  # continuing
                break_if true  # -> [t: exit_loop loop_2, f: $B5]
              }
            }
            exit_if  # if_1
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
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
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        if true [t: $B4] {  # if_1
          $B4: {  # true
            loop [b: $B5, c: $B6] {  # loop_2
              $B5: {  # body
                if true [t: $B7] {  # if_2
                  $B7: {  # true
                    if true [t: $B8, f: $B9] {  # if_3
                      $B8: {  # true
                        exit_loop  # loop_2
                      }
                      $B9: {  # false
                        exit_loop  # loop_2
                      }
                    }
                    exit_if  # if_2
                  }
                }
                exit_loop  # loop_2
              }
              $B6: {  # continuing
                break_if true  # -> [t: exit_loop loop_2, f: $B5]
              }
            }
            exit_if  # if_1
          }
        }
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)";

    Run(RemoveUnreachableInLoopContinuing);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
