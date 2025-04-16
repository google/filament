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

#include "src/tint/lang/msl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

Options NoRobustness() {
    Options o;
    o.disable_robustness = true;
    return o;
}

TEST_F(MslWriterTest, Loop) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Append(l->Continuing(), [&] { b.NextIteration(l); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    uint2 tint_loop_idx = uint2(4294967295u);
    while(true) {
      if (all((tint_loop_idx == uint2(0u)))) {
        break;
      }
      break;
    }
  }
}
)");
}

TEST_F(MslWriterTest, Loop_WithoutRobustness) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Append(l->Continuing(), [&] { b.NextIteration(l); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate(NoRobustness())) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    while(true) {
      break;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopContinueAndBreakIf) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.Continue(l); });
        b.Append(l->Continuing(), [&] { b.BreakIf(l, true); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    uint2 tint_loop_idx = uint2(4294967295u);
    while(true) {
      if (all((tint_loop_idx == uint2(0u)))) {
        break;
      }
      {
        uint const tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint const tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        if (true) { break; }
      }
      continue;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopContinueAndBreakIf_WithoutRobustness) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.Continue(l); });
        b.Append(l->Continuing(), [&] { b.BreakIf(l, true); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate(NoRobustness())) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    while(true) {
      {
        if (true) { break; }
      }
      continue;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopBodyVarInContinue) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] {
            auto* v = b.Var("v", true);
            b.Continue(l);

            b.Append(l->Continuing(), [&] { b.BreakIf(l, v); });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    uint2 tint_loop_idx = uint2(4294967295u);
    while(true) {
      if (all((tint_loop_idx == uint2(0u)))) {
        break;
      }
      bool v = true;
      {
        uint const tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint const tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        if (v) { break; }
      }
      continue;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopBodyVarInContinue_WithoutRobustness) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] {
            auto* v = b.Var("v", true);
            b.Continue(l);

            b.Append(l->Continuing(), [&] { b.BreakIf(l, v); });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate(NoRobustness())) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    while(true) {
      bool v = true;
      {
        if (v) { break; }
      }
      continue;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopInitializer) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] {
            auto* v = b.Var("v", true);
            b.NextIteration(l);

            b.Append(l->Body(), [&] { b.Continue(l); });
            b.Append(l->Continuing(), [&] { b.BreakIf(l, v); });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    uint2 tint_loop_idx = uint2(4294967295u);
    bool v = true;
    while(true) {
      if (all((tint_loop_idx == uint2(0u)))) {
        break;
      }
      {
        uint const tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint const tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        if (v) { break; }
      }
      continue;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopInitializer_WithoutRobustness) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] {
            auto* v = b.Var("v", true);
            b.NextIteration(l);

            b.Append(l->Body(), [&] { b.Continue(l); });
            b.Append(l->Continuing(), [&] { b.BreakIf(l, v); });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate(NoRobustness())) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    bool v = true;
    while(true) {
      {
        if (v) { break; }
      }
      continue;
    }
  }
}
)");
}

// Test that we elide the mechanism used to avoid infinite loop UB when we detect a loop as finite.
TEST_F(MslWriterTest, LoopInitializer_WithRobustness_DetectedAsFinite) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] {
            auto* v = b.Var("v", 0_u);
            b.NextIteration(l);

            b.Append(l->Body(), [&] {
                auto* ifelse = b.If(b.LessThan<bool>(b.Load(v), 10_u));
                b.Append(ifelse->True(), [&] {  //
                    b.ExitIf(ifelse);
                });
                b.Append(ifelse->False(), [&] {  //
                    b.ExitLoop(l);
                });
                b.Continue(l);
            });
            b.Append(l->Continuing(), [&] {  //
                b.Store(v, b.Add<u32>(b.Load(v), 1_u));
                b.NextIteration(l);
            });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate(NoRobustness())) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

void a() {
  {
    uint v = 0u;
    while(true) {
      if ((v < 10u)) {
      } else {
        break;
      }
      {
        v = (v + 1u);
      }
      continue;
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
