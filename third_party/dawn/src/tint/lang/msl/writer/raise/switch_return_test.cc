// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/raise/switch_return.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_SwitchReturnTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_SwitchReturnTest, ReturnInsideSwitch) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(1_i);
        b.Append(b.Case(sw, {b.Constant(0_i)}), [&] { b.Return(func); });
        b.Append(b.DefaultCase(sw), [&] { b.ExitSwitch(sw); });
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        exit_switch  # switch_1
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
    switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        %2:u32 = msl.volatile_zero
        %3:bool = eq %2, 0u
        if %3 [t: $B4] {  # if_1
          $B4: {  # true
            ret
          }
        }
        exit_switch  # switch_1
      }
      $B3: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    Run(SwitchReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_SwitchReturnTest, ReturnValueInsideSwitch) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(1_i);
        b.Append(b.Case(sw, {b.Constant(0_i)}), [&] { b.Return(func, 42_i); });
        b.Append(b.DefaultCase(sw), [&] { b.ExitSwitch(sw); });
        b.Return(func, 0_i);
    });

    auto* src = R"(
%foo = func():i32 {
  $B1: {
    switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
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
%foo = func():i32 {
  $B1: {
    switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        %2:u32 = msl.volatile_zero
        %3:bool = eq %2, 0u
        if %3 [t: $B4] {  # if_1
          $B4: {  # true
            ret 42i
          }
        }
        exit_switch  # switch_1
      }
      $B3: {  # case
        exit_switch  # switch_1
      }
    }
    ret 0i
  }
}
)";

    Run(SwitchReturn);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_SwitchReturnTest, SwitchReturnValueAndReturnInsideSwitch) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* res_a = b.InstructionResult(ty.i32());
        auto* swtch = b.Switch(1_i);
        swtch->SetResults(Vector{res_a});

        auto* case_a = b.Case(swtch, Vector{b.Constant(0_i)});
        b.Append(case_a, [&] { b.Return(func, 42_i); });

        auto* def_case = b.DefaultCase(swtch);
        b.Append(def_case, [&] { b.ExitSwitch(swtch, 5_i); });

        b.Return(func, res_a);
    });

    auto* src = R"(
%foo = func():i32 {
  $B1: {
    %2:i32 = switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        ret 42i
      }
      $B3: {  # case
        exit_switch 5i  # switch_1
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
    %2:i32 = switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        %3:u32 = msl.volatile_zero
        %4:bool = eq %3, 0u
        if %4 [t: $B4] {  # if_1
          $B4: {  # true
            ret 42i
          }
        }
        exit_switch undef  # switch_1
      }
      $B3: {  # case
        exit_switch 5i  # switch_1
      }
    }
    ret %2
  }
}
)";

    Run(SwitchReturn);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
