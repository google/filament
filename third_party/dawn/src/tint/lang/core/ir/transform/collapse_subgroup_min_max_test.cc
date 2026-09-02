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

#include "src/tint/lang/core/ir/transform/collapse_subgroup_min_max.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_CollapseSubgroupMinMaxTest : public TransformTest {
  public:
    IR_CollapseSubgroupMinMaxTest() {}
};

TEST_F(IR_CollapseSubgroupMinMaxTest, SubgroupMin_SubgroupMin) {
    auto* u = b.FunctionParam("u", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({u});

    b.Append(func->Block(), [&] {
        auto* first = b.Call<i32>(core::BuiltinFn::kSubgroupMin, u);
        auto* second = b.Call<i32>(core::BuiltinFn::kSubgroupMin, first);
        b.Return(func, second);
    });

    auto* src = R"(
%foo = func(%u:i32):i32 {
  $B1: {
    %3:i32 = subgroupMin %u
    %4:i32 = subgroupMin %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%u:i32):i32 {
  $B1: {
    %3:i32 = subgroupMin %u
    ret %3
  }
}
)";

    Run(CollapseSubgroupMinMax);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_CollapseSubgroupMinMaxTest, SubgroupMin_SubgroupMax) {
    auto* u = b.FunctionParam("u", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({u});

    b.Append(func->Block(), [&] {
        auto* first = b.Call<i32>(core::BuiltinFn::kSubgroupMax, u);
        b.Call<i32>(core::BuiltinFn::kSubgroupMin, first);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%u:i32):void {
  $B1: {
    %3:i32 = subgroupMax %u
    %4:i32 = subgroupMin %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%u:i32):void {
  $B1: {
    %3:i32 = subgroupMax %u
    ret
  }
}
)";

    Run(CollapseSubgroupMinMax);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_CollapseSubgroupMinMaxTest, SubgroupMax_SubgroupMin_SubgroupMax) {
    auto* u = b.FunctionParam("u", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({u});

    b.Append(func->Block(), [&] {
        auto* first = b.Call<i32>(core::BuiltinFn::kSubgroupMax, u);
        auto* second = b.Call<i32>(core::BuiltinFn::kSubgroupMin, first);
        b.Call<i32>(core::BuiltinFn::kSubgroupMax, second);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%u:i32):void {
  $B1: {
    %3:i32 = subgroupMax %u
    %4:i32 = subgroupMin %3
    %5:i32 = subgroupMax %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%u:i32):void {
  $B1: {
    %3:i32 = subgroupMax %u
    ret
  }
}
)";

    Run(CollapseSubgroupMinMax);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_CollapseSubgroupMinMaxTest, SubgroupMin_Let) {
    auto* u = b.FunctionParam("u", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({u});

    b.Append(func->Block(), [&] {
        auto* first = b.Call<i32>(core::BuiltinFn::kSubgroupMin, u);
        auto* let = b.Let(first);
        b.Call<i32>(core::BuiltinFn::kSubgroupMin, let);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%u:i32):void {
  $B1: {
    %3:i32 = subgroupMin %u
    %4:i32 = let %3
    %5:i32 = subgroupMin %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%u:i32):void {
  $B1: {
    %3:i32 = subgroupMin %u
    %4:i32 = let %3
    ret
  }
}
)";

    Run(CollapseSubgroupMinMax);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
