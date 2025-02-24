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

#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, Loop) {
    auto* func = b.ComputeFunction("a");

    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Append(l->Continuing(), [&] { b.NextIteration(l); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      break;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, LoopContinueAndBreakIf) {
    auto* func = b.ComputeFunction("a");

    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.Continue(l); });
        b.Append(l->Continuing(), [&] { b.BreakIf(l, true); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        if (true) { break; }
      }
      continue;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, LoopBodyVarInContinue) {
    auto* func = b.ComputeFunction("a");

    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] {
            auto* v = b.Var("v", true);
            b.Continue(l);

            b.Append(l->Continuing(), [&] { b.BreakIf(l, v); });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      bool v = true;
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        if (v) { break; }
      }
      continue;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, LoopInitializer) {
    auto* func = b.ComputeFunction("a");

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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  {
    uint2 tint_loop_idx = (0u).xx;
    bool v = true;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        if (v) { break; }
      }
      continue;
    }
  }
}

)");
}

TEST_F(HlslWriterTest, Loop_UnconditionalReturn_NonVoid) {
    auto* func = b.Function("a", ty.i32());

    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.Return(func, 1_i); });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
int a() {
  {
    uint2 tint_loop_idx = (0u).xx;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      return int(1);
    }
  }
  /* unreachable */
  return int(0);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
