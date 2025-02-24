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

#include "src/tint/lang/msl/writer/raise/simd_ballot.h"

#include "gtest/gtest.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_SimdBallotTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_SimdBallotTest, SimdBallot_WithUserDeclaredSubgroupSize) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto* subgroup_size = b.FunctionParam("user_subgroup_size", ty.u32());
    core::IOAttributes attr;
    attr.location = 0;
    subgroup_size->SetAttributes(attr);
    func->SetParams({subgroup_size});
    b.Append(func->Block(), [&] {  //
        b.Call<vec4<u32>>(core::BuiltinFn::kSubgroupBallot, true);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func(%user_subgroup_size:u32 [@location(0)]):void {
  $B1: {
    %3:vec4<u32> = subgroupBallot true
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_size_mask:ptr<private, vec2<u32>, read_write> = var
}

%foo = @fragment func(%user_subgroup_size:u32 [@location(0)], %tint_subgroup_size:u32 [@subgroup_size]):void {
  $B2: {
    %5:bool = gt %tint_subgroup_size, 32u
    %6:u32 = sub 32u, %tint_subgroup_size
    %7:u32 = shr 4294967295u, %6
    %8:u32 = select %7, 4294967295u, %5
    %9:u32 = sub 64u, %tint_subgroup_size
    %10:u32 = shr 4294967295u, %9
    %11:u32 = select 0u, %10, %5
    store_vector_element %tint_subgroup_size_mask, 0u, %8
    store_vector_element %tint_subgroup_size_mask, 1u, %11
    %12:vec4<u32> = call %tint_subgroup_ballot, true
    ret
  }
}
%tint_subgroup_ballot = func(%pred:bool):vec4<u32> {
  $B3: {
    %15:vec2<u32> = msl.simd_ballot %pred
    %16:vec2<u32> = load %tint_subgroup_size_mask
    %17:vec2<u32> = and %15, %16
    %18:vec4<u32> = construct %17, 0u, 0u
    ret %18
  }
}
)";

    Run(SimdBallot);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_SimdBallotTest, SimdBallot_WithoutUserDeclaredSubgroupSize) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Call<vec4<u32>>(core::BuiltinFn::kSubgroupBallot, true);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<u32> = subgroupBallot true
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_size_mask:ptr<private, vec2<u32>, read_write> = var
}

%foo = @fragment func(%tint_subgroup_size:u32 [@subgroup_size]):void {
  $B2: {
    %4:bool = gt %tint_subgroup_size, 32u
    %5:u32 = sub 32u, %tint_subgroup_size
    %6:u32 = shr 4294967295u, %5
    %7:u32 = select %6, 4294967295u, %4
    %8:u32 = sub 64u, %tint_subgroup_size
    %9:u32 = shr 4294967295u, %8
    %10:u32 = select 0u, %9, %4
    store_vector_element %tint_subgroup_size_mask, 0u, %7
    store_vector_element %tint_subgroup_size_mask, 1u, %10
    %11:vec4<u32> = call %tint_subgroup_ballot, true
    ret
  }
}
%tint_subgroup_ballot = func(%pred:bool):vec4<u32> {
  $B3: {
    %14:vec2<u32> = msl.simd_ballot %pred
    %15:vec2<u32> = load %tint_subgroup_size_mask
    %16:vec2<u32> = and %14, %15
    %17:vec4<u32> = construct %16, 0u, 0u
    ret %17
  }
}
)";

    Run(SimdBallot);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_SimdBallotTest, SimdBallot_InHelperFunction) {
    auto* foo = b.Function("foo", ty.vec4<u32>());
    auto* pred = b.FunctionParam("pred", ty.bool_());
    foo->SetParams({pred});
    b.Append(foo->Block(), [&] {  //
        auto* result = b.Call<vec4<u32>>(core::BuiltinFn::kSubgroupBallot, pred);
        b.Return(foo, result);
    });

    auto* ep1 = b.Function("ep1", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto* subgroup_size = b.FunctionParam("user_subgroup_size", ty.u32());
    core::IOAttributes attr;
    attr.location = 0;
    subgroup_size->SetAttributes(attr);
    ep1->SetParams({subgroup_size});
    b.Append(ep1->Block(), [&] {  //
        b.Call<vec4<u32>>(foo, true);
        b.Return(ep1);
    });

    auto* ep2 = b.Function("ep2", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep2->Block(), [&] {  //
        b.Call<vec4<u32>>(foo, false);
        b.Return(ep2);
    });

    auto* src = R"(
%foo = func(%pred:bool):vec4<u32> {
  $B1: {
    %3:vec4<u32> = subgroupBallot %pred
    ret %3
  }
}
%ep1 = @fragment func(%user_subgroup_size:u32 [@location(0)]):void {
  $B2: {
    %6:vec4<u32> = call %foo, true
    ret
  }
}
%ep2 = @fragment func():void {
  $B3: {
    %8:vec4<u32> = call %foo, false
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_size_mask:ptr<private, vec2<u32>, read_write> = var
}

%foo = func(%pred:bool):vec4<u32> {
  $B2: {
    %4:vec4<u32> = call %tint_subgroup_ballot, %pred
    ret %4
  }
}
%ep1 = @fragment func(%user_subgroup_size:u32 [@location(0)], %tint_subgroup_size:u32 [@subgroup_size]):void {
  $B3: {
    %9:bool = gt %tint_subgroup_size, 32u
    %10:u32 = sub 32u, %tint_subgroup_size
    %11:u32 = shr 4294967295u, %10
    %12:u32 = select %11, 4294967295u, %9
    %13:u32 = sub 64u, %tint_subgroup_size
    %14:u32 = shr 4294967295u, %13
    %15:u32 = select 0u, %14, %9
    store_vector_element %tint_subgroup_size_mask, 0u, %12
    store_vector_element %tint_subgroup_size_mask, 1u, %15
    %16:vec4<u32> = call %foo, true
    ret
  }
}
%ep2 = @fragment func(%tint_subgroup_size_1:u32 [@subgroup_size]):void {  # %tint_subgroup_size_1: 'tint_subgroup_size'
  $B4: {
    %19:bool = gt %tint_subgroup_size_1, 32u
    %20:u32 = sub 32u, %tint_subgroup_size_1
    %21:u32 = shr 4294967295u, %20
    %22:u32 = select %21, 4294967295u, %19
    %23:u32 = sub 64u, %tint_subgroup_size_1
    %24:u32 = shr 4294967295u, %23
    %25:u32 = select 0u, %24, %19
    store_vector_element %tint_subgroup_size_mask, 0u, %22
    store_vector_element %tint_subgroup_size_mask, 1u, %25
    %26:vec4<u32> = call %foo, false
    ret
  }
}
%tint_subgroup_ballot = func(%pred_1:bool):vec4<u32> {  # %pred_1: 'pred'
  $B5: {
    %28:vec2<u32> = msl.simd_ballot %pred_1
    %29:vec2<u32> = load %tint_subgroup_size_mask
    %30:vec2<u32> = and %28, %29
    %31:vec4<u32> = construct %30, 0u, 0u
    ret %31
  }
}
)";

    Run(SimdBallot);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_SimdBallotTest, SimdBallot_MultipleCalls) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Call<vec4<u32>>(core::BuiltinFn::kSubgroupBallot, true);
        b.Call<vec4<u32>>(core::BuiltinFn::kSubgroupBallot, false);
        b.Call<vec4<u32>>(core::BuiltinFn::kSubgroupBallot, true);
        b.Call<vec4<u32>>(core::BuiltinFn::kSubgroupBallot, false);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<u32> = subgroupBallot true
    %3:vec4<u32> = subgroupBallot false
    %4:vec4<u32> = subgroupBallot true
    %5:vec4<u32> = subgroupBallot false
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_size_mask:ptr<private, vec2<u32>, read_write> = var
}

%foo = @fragment func(%tint_subgroup_size:u32 [@subgroup_size]):void {
  $B2: {
    %4:bool = gt %tint_subgroup_size, 32u
    %5:u32 = sub 32u, %tint_subgroup_size
    %6:u32 = shr 4294967295u, %5
    %7:u32 = select %6, 4294967295u, %4
    %8:u32 = sub 64u, %tint_subgroup_size
    %9:u32 = shr 4294967295u, %8
    %10:u32 = select 0u, %9, %4
    store_vector_element %tint_subgroup_size_mask, 0u, %7
    store_vector_element %tint_subgroup_size_mask, 1u, %10
    %11:vec4<u32> = call %tint_subgroup_ballot, true
    %13:vec4<u32> = call %tint_subgroup_ballot, false
    %14:vec4<u32> = call %tint_subgroup_ballot, true
    %15:vec4<u32> = call %tint_subgroup_ballot, false
    ret
  }
}
%tint_subgroup_ballot = func(%pred:bool):vec4<u32> {
  $B3: {
    %17:vec2<u32> = msl.simd_ballot %pred
    %18:vec2<u32> = load %tint_subgroup_size_mask
    %19:vec2<u32> = and %17, %18
    %20:vec4<u32> = construct %19, 0u, 0u
    ret %20
  }
}
)";

    Run(SimdBallot);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
