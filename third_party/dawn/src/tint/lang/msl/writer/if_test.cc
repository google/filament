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

#include "src/tint/lang/msl/writer/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

TEST_F(MslWriterTest, If) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  if (true) {
  }
}
)");
}

TEST_F(MslWriterTest, IfWithElseIf) {
    auto* func = b.Function("foo", ty.void_());
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

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  if (true) {
  } else {
    if (false) {
    }
  }
}
)");
}

TEST_F(MslWriterTest, IfWithElse) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Append(if_->False(), [&] { b.Return(func); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  if (true) {
  } else {
    return;
  }
}
)");
}

TEST_F(MslWriterTest, IfBothBranchesReturn) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(func); });
        b.Append(if_->False(), [&] { b.Return(func); });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
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

TEST_F(MslWriterTest, IfBothBranchesReturn_NonVoidFunction) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(func, 0_u); });
        b.Append(if_->False(), [&] { b.Return(func, 1_u); });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
uint foo() {
  if (true) {
    return 0u;
  } else {
    return 1u;
  }
  /* unreachable */
  return 0u;
}
)");
}

TEST_F(MslWriterTest, IfWithSinglePhi) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int v = 0;
  if (true) {
    v = 10;
  } else {
    v = 20;
  }
}
)");
}

TEST_F(MslWriterTest, IfWithMultiPhi) {
    auto* func = b.Function("foo", ty.void_());
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

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
  int v = 0;
  bool v_1 = false;
  if (true) {
    v = 10;
    v_1 = true;
  } else {
    v = 20;
    v_1 = false;
  }
}
)");
}

TEST_F(MslWriterTest, IfWithMultiPhiReturn1) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
int foo() {
  int v = 0;
  bool v_1 = false;
  if (true) {
    v = 10;
    v_1 = true;
  } else {
    v = 20;
    v_1 = false;
  }
  return v;
}
)");
}

TEST_F(MslWriterTest, IfWithMultiPhiReturn2) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
bool foo() {
  int v = 0;
  bool v_1 = false;
  if (true) {
    v = 10;
    v_1 = true;
  } else {
    v = 20;
    v_1 = false;
  }
  return v_1;
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
