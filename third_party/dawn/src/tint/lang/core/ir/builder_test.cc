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

#include "src/tint/lang/core/ir/builder.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_BuilderTest = IRTestHelper;

TEST_F(IR_BuilderTest, Append) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("a", 1_u);
        b.Let("b", 2_u);
        b.Let("c", 3_u);
    });
    b.Append(func->Block(), [&] {
        b.Let("d", 4_u);
        b.Let("e", 5_u);
        b.Let("f", 6_u);
    });
    EXPECT_EQ(str(), R"(
%foo = func():void {
  $B1: {
    %a:u32 = let 1u
    %b:u32 = let 2u
    %c:u32 = let 3u
    %d:u32 = let 4u
    %e:u32 = let 5u
    %f:u32 = let 6u
  }
}
)");
}

TEST_F(IR_BuilderTest, InsertBefore) {
    auto* func = b.Function("foo", ty.void_());
    Instruction* ip = nullptr;
    b.Append(func->Block(), [&] {
        b.Let("a", 1_u);
        ip = b.Let("b", 2_u);
        b.Let("c", 3_u);
    });
    b.InsertBefore(ip, [&] {
        b.Let("d", 4_u);
        b.Let("e", 5_u);
        b.Let("f", 6_u);
    });
    EXPECT_EQ(str(), R"(
%foo = func():void {
  $B1: {
    %a:u32 = let 1u
    %d:u32 = let 4u
    %e:u32 = let 5u
    %f:u32 = let 6u
    %b:u32 = let 2u
    %c:u32 = let 3u
  }
}
)");
}

TEST_F(IR_BuilderTest, InsertAfter) {
    auto* func = b.Function("foo", ty.void_());
    Instruction* ip = nullptr;
    b.Append(func->Block(), [&] {
        b.Let("a", 1_u);
        ip = b.Let("b", 2_u);
        b.Let("c", 3_u);
    });
    b.InsertAfter(ip, [&] {
        b.Let("d", 4_u);
        b.Let("e", 5_u);
        b.Let("f", 6_u);
    });
    EXPECT_EQ(str(), R"(
%foo = func():void {
  $B1: {
    %a:u32 = let 1u
    %b:u32 = let 2u
    %d:u32 = let 4u
    %e:u32 = let 5u
    %f:u32 = let 6u
    %c:u32 = let 3u
  }
}
)");
}

TEST_F(IR_BuilderTest, InsertInBlockAfter_InstructionResult) {
    auto* func = b.Function("foo", ty.void_());
    Instruction* ip = nullptr;
    b.Append(func->Block(), [&] {
        b.Let("a", 1_u);
        ip = b.Let("b", 2_u);
        b.Let("c", 3_u);
    });
    b.InsertInBlockAfter(ip->Result(0), [&] {
        b.Let("d", 4_u);
        b.Let("e", 5_u);
        b.Let("f", 6_u);
    });
    EXPECT_EQ(str(), R"(
%foo = func():void {
  $B1: {
    %a:u32 = let 1u
    %b:u32 = let 2u
    %d:u32 = let 4u
    %e:u32 = let 5u
    %f:u32 = let 6u
    %c:u32 = let 3u
  }
}
)");
}

TEST_F(IR_BuilderTest, InsertInBlockAfter_Param) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("param", ty.u32());
    func->SetParams({param});
    b.Append(func->Block(), [&] {
        b.Let("a", 1_u);
        b.Let("b", 2_u);
        b.Let("c", 3_u);
    });
    b.InsertInBlockAfter(param, [&] {
        b.Let("d", 4_u);
        b.Let("e", 5_u);
        b.Let("f", 6_u);
    });
    EXPECT_EQ(str(), R"(
%foo = func(%param:u32):void {
  $B1: {
    %d:u32 = let 4u
    %e:u32 = let 5u
    %f:u32 = let 6u
    %a:u32 = let 1u
    %b:u32 = let 2u
    %c:u32 = let 3u
  }
}
)");
}

TEST_F(IR_BuilderTest, InsertInBlockAfter_Param_EmptyFunction) {
    auto* func = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("param", ty.u32());
    func->SetParams({param});
    b.InsertInBlockAfter(param, [&] {
        b.Let("a", 1_u);
        b.Let("b", 2_u);
        b.Let("c", 3_u);
    });
    EXPECT_EQ(str(), R"(
%foo = func(%param:u32):void {
  $B1: {
    %a:u32 = let 1u
    %b:u32 = let 2u
    %c:u32 = let 3u
  }
}
)");
}

TEST_F(IR_BuilderTest, InsertInBlockAfter_BlockParam) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        auto* param = b.BlockParam("param", ty.u32());
        loop->Body()->SetParams({param});
        b.Append(loop->Body(), [&] {
            b.Let("a", 1_u);
            b.Let("b", 2_u);
            b.Let("c", 3_u);
        });
        b.InsertInBlockAfter(param, [&] {
            b.Let("d", 4_u);
            b.Let("e", 5_u);
            b.Let("f", 6_u);
        });
    });
    EXPECT_EQ(str(), R"(
%foo = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2 (%param:u32): {  # body
        %d:u32 = let 4u
        %e:u32 = let 5u
        %f:u32 = let 6u
        %a:u32 = let 1u
        %b:u32 = let 2u
        %c:u32 = let 3u
      }
    }
  }
}
)");
}

TEST_F(IR_BuilderTest, InsertInBlockAfter_BlockParam_EmptyBlock) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        auto* param = b.BlockParam("param", ty.u32());
        loop->Body()->SetParams({param});
        b.InsertInBlockAfter(param, [&] {
            b.Let("a", 1_u);
            b.Let("b", 2_u);
            b.Let("c", 3_u);
        });
    });
    EXPECT_EQ(str(), R"(
%foo = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2 (%param:u32): {  # body
        %a:u32 = let 1u
        %b:u32 = let 2u
        %c:u32 = let 3u
      }
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::core::ir
