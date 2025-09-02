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

#include "src/tint/lang/core/ir/transform/dead_code_elimination.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_DeadCodeEliminationTest = TransformTest;

TEST_F(IR_DeadCodeEliminationTest, NoModify) {
    capabilities = Capability::kAllowUnannotatedModuleIOVariables;

    auto* buffer = b.Var("buffer", ty.ptr(core::AddressSpace::kOut, ty.i32()));
    mod.root_block->Append(buffer);

    auto* used = b.Function("used", ty.f32());
    b.Append(used->Block(), [&] { b.Return(used, 0.5_f); });

    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {  //
        b.Store(buffer, 42_i);
        b.Return(ep, b.Call(ty.f32(), used));
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<__out, i32, read_write> = var undef
}

%used = func():f32 {
  $B2: {
    ret 0.5f
  }
}
%ep = @fragment func():f32 [@location(0)] {
  $B3: {
    store %buffer, 42i
    %4:f32 = call %used
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DeadCodeElimination);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DeadCodeEliminationTest, RemoveFunction) {
    capabilities = Capabilities{
        Capability::kAllowUnannotatedModuleIOVariables,
        Capability::kAllowPhonyInstructions,
    };

    auto* buffer = b.Var("buffer", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    mod.root_block->Append(buffer);

    auto* unused = b.Function("unused", ty.f32());
    b.Append(unused->Block(), [&] { b.Return(unused, 0.5_f); });

    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {  //
        b.Phony(b.Load(buffer));
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<__in, i32, read> = var undef
}

%unused = func():f32 {
  $B2: {
    ret 0.5f
  }
}
%ep = @fragment func():f32 [@location(0)] {
  $B3: {
    %4:i32 = load %buffer
    undef = phony %4
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<__in, i32, read> = var undef
}

%ep = @fragment func():f32 [@location(0)] {
  $B2: {
    %3:i32 = load %buffer
    undef = phony %3
    ret 0.5f
  }
}
)";

    Run(DeadCodeElimination);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DeadCodeEliminationTest, RemoveFunction_Recursive) {
    auto* buffer = b.Var("buffer", ty.ptr(core::AddressSpace::kPrivate, ty.i32()));
    mod.root_block->Append(buffer);

    auto* unused2 = b.Function("unused2", ty.f32());
    b.Append(unused2->Block(), [&] { b.Return(unused2, 0.5_f); });

    auto* unused = b.Function("unused", ty.f32());
    b.Append(unused->Block(), [&] { b.Return(unused, b.Call(ty.f32(), unused2)); });

    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {  //
        b.Store(buffer, 42_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<private, i32, read_write> = var undef
}

%unused2 = func():f32 {
  $B2: {
    ret 0.5f
  }
}
%unused = func():f32 {
  $B3: {
    %4:f32 = call %unused2
    ret %4
  }
}
%ep = @fragment func():f32 [@location(0)] {
  $B4: {
    store %buffer, 42i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<private, i32, read_write> = var undef
}

%ep = @fragment func():f32 [@location(0)] {
  $B2: {
    store %buffer, 42i
    ret 0.5f
  }
}
)";

    Run(DeadCodeElimination);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DeadCodeEliminationTest, RemoveVarIn_Nested) {
    auto* buffer = b.Var("buffer", ty.ptr(core::AddressSpace::kIn, ty.i32()));
    mod.root_block->Append(buffer);

    auto* unused2 = b.Function("unused2", ty.f32());
    b.Append(unused2->Block(), [&] {
        b.Phony(b.Load(buffer));
        b.Return(unused2, 0.5_f);
    });

    auto* unused = b.Function("unused", ty.f32());
    b.Append(unused->Block(), [&] { b.Return(unused, b.Call(ty.f32(), unused2)); });

    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<__in, i32, read> = var undef
}

%unused2 = func():f32 {
  $B2: {
    %3:i32 = load %buffer
    undef = phony %3
    ret 0.5f
  }
}
%unused = func():f32 {
  $B3: {
    %5:f32 = call %unused2
    ret %5
  }
}
%ep = @fragment func():f32 [@location(0)] {
  $B4: {
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%ep = @fragment func():f32 [@location(0)] {
  $B1: {
    ret 0.5f
  }
}
)";

    Run(DeadCodeElimination);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DeadCodeEliminationTest, RemoveVarOut_Nested) {
    auto* buffer =
        b.Var("buffer", ty.ptr(core::AddressSpace::kOut, ty.i32(), core::Access::kWrite));
    mod.root_block->Append(buffer);

    auto* unused2 = b.Function("unused2", ty.f32());
    b.Append(unused2->Block(), [&] {
        b.Store(buffer, 42_i);
        b.Return(unused2, 0.5_f);
    });

    auto* unused = b.Function("unused", ty.f32());
    b.Append(unused->Block(), [&] { b.Return(unused, b.Call(ty.f32(), unused2)); });

    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<__out, i32, write> = var undef
}

%unused2 = func():f32 {
  $B2: {
    store %buffer, 42i
    ret 0.5f
  }
}
%unused = func():f32 {
  $B3: {
    %4:f32 = call %unused2
    ret %4
  }
}
%ep = @fragment func():f32 [@location(0)] {
  $B4: {
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%ep = @fragment func():f32 [@location(0)] {
  $B1: {
    ret 0.5f
  }
}
)";

    Run(DeadCodeElimination);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DeadCodeEliminationTest, RemoveVarPrivate_Nested) {
    auto* buffer = b.Var("buffer", ty.ptr(core::AddressSpace::kPrivate, ty.i32()));
    mod.root_block->Append(buffer);

    auto* unused2 = b.Function("unused2", ty.f32());
    b.Append(unused2->Block(), [&] {
        b.Store(buffer, 42_i);
        b.Return(unused2, 0.5_f);
    });

    auto* unused = b.Function("unused", ty.f32());
    b.Append(unused->Block(), [&] { b.Return(unused, b.Call(ty.f32(), unused2)); });

    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0_u);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<private, i32, read_write> = var undef
}

%unused2 = func():f32 {
  $B2: {
    store %buffer, 42i
    ret 0.5f
  }
}
%unused = func():f32 {
  $B3: {
    %4:f32 = call %unused2
    ret %4
  }
}
%ep = @fragment func():f32 [@location(0)] {
  $B4: {
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%ep = @fragment func():f32 [@location(0)] {
  $B1: {
    ret 0.5f
  }
}
)";

    Run(DeadCodeElimination);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
