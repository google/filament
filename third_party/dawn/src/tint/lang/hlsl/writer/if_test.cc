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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, If) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  if (true) {
  }
}

)");
}

TEST_F(HlslWriterTest, IfWithElseIf) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Append(if_->False(), [&] {
            auto* false_ = b.If(false);
            b.Append(false_->True(), [&] { b.ExitIf(false_); });
            b.ExitIf(if_);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  if (true) {
  } else {
    if (false) {
    }
  }
}

)");
}

TEST_F(HlslWriterTest, IfWithElse) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Append(if_->False(), [&] { b.Return(func); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  if (true) {
  } else {
    return;
  }
}

)");
}

TEST_F(HlslWriterTest, IfBothBranchesReturn) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(func); });
        b.Append(if_->False(), [&] { b.Return(func); });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  if (true) {
    return;
  } else {
    return;
  }
  /* unreachable */
}

)");
}

TEST_F(HlslWriterTest, IfBothBranchesReturn_NonVoidFunction) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(func, 0_u); });
        b.Append(if_->False(), [&] { b.Return(func, 1_u); });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint foo() {
  if (true) {
    return 0u;
  } else {
    return 1u;
  }
  /* unreachable */
  return 0u;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, IfWithSinglePhi) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResult(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  int v = int(0);
  if (true) {
    v = int(10);
  } else {
    v = int(20);
  }
}

)");
}

TEST_F(HlslWriterTest, IfWithMultiPhi) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  int v = int(0);
  bool v_1 = false;
  if (true) {
    v = int(10);
    v_1 = true;
  } else {
    v = int(20);
    v_1 = false;
  }
}

)");
}

TEST_F(HlslWriterTest, IfWithMultiPhiReturn1) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(0));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
int foo() {
  int v = int(0);
  bool v_1 = false;
  if (true) {
    v = int(10);
    v_1 = true;
  } else {
    v = int(20);
    v_1 = false;
  }
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, IfWithMultiPhiReturn2) {
    auto* func = b.Function("foo", ty.bool_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(1));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
bool foo() {
  int v = int(0);
  bool v_1 = false;
  if (true) {
    v = int(10);
    v_1 = true;
  } else {
    v = int(20);
    v_1 = false;
  }
  return v_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
