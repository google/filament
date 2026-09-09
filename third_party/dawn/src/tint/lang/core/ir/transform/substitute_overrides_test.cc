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

#include "src/tint/lang/core/ir/transform/substitute_overrides.h"

#include <limits>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_SubstituteOverridesTest : public TransformTest {
  protected:
    void SetUp() override {
        TransformTest::SetUp();
        mod.properties.Add(core::ir::Property::kAllow16BitFloats,
                           core::ir::Property::kAllowOverrides,
                           core::ir::Property::kAllowBufferTypes);
    }
};

TEST_F(IR_SubstituteOverridesTest, OverridePropertyRemoved) {
    SubstituteOverridesConfig cfg{};
    Run(SubstituteOverrides, cfg);
    EXPECT_FALSE(mod.properties.Contains(Property::kAllowOverrides));
}

TEST_F(IR_SubstituteOverridesTest, NoOverridesNoChange) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
%foo = func():void {
  $B1: {
    ret
  }
}
)";

    SubstituteOverridesConfig cfg{};
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, UnsetOverrideTriggersError) {
    b.Append(mod.root_block, [&] {
        auto* o = b.Override(Source{{1, 2}}, "a", ty.i32());
        o->SetOverrideId({1});
    });

    auto* src = R"(
$B1: {  # root
  %a:i32 = override undef @id(1)
}

)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(1:2 error: Initializer not provided for override, and override not overridden.)");
}

TEST_F(IR_SubstituteOverridesTest, OverrideNotInFile) {
    auto* f = b.ComputeFunction("main");
    b.Append(f->Block(), [&] { b.Return(f); });

    auto* src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";

    auto* expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{99}] = 55;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithDefault) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override(Source{{1, 2}}, "a", 2_u);
        o->SetOverrideId({1});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %a:u32 = override 2u @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
)";

    auto* expect = R"(
%foo = func():u32 {
  $B1: {
    ret 2u
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithDefaultWithOverride) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override(Source{{1, 2}}, "a", 2_u);
        o->SetOverrideId({1});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %a:u32 = override 2u @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
)";

    auto* expect = R"(
%foo = func():u32 {
  $B1: {
    ret 55u
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 55;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithoutDefaultWithOverride) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %a:u32 = override undef @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
)";

    auto* expect = R"(
%foo = func():u32 {
  $B1: {
    ret 55u
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 55;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithComplexInitNoOverrides) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* add = b.Add(2_u, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %1:u32 = add 2u, 4u
  %a:u32 = override %1 @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
)";

    auto* expect = R"(
%foo = func():u32 {
  $B1: {
    ret 6u
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithComplexInitComponentOverride) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* add = b.Add(2_u, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %1:u32 = add 2u, 4u
  %a:u32 = override %1 @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
)";

    auto* expect = R"(
%foo = func():u32 {
  $B1: {
    ret 55u
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 55;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithComplexIncludingOverride) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
)";

    auto* expect = R"(
%foo = func():u32 {
  $B1: {
    ret 9u
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithSubgroupShuffle) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
        auto* add = b.Add(x, 4_u);
        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* shuffle_func = b.Call(ty.u32(), core::BuiltinFn::kSubgroupShuffle, 1_u, o);
        b.Return(func, shuffle_func->Result());
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = func():u32 {
  $B2: {
    %5:u32 = subgroupShuffle 1u, %a
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 125.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: The sourceLaneIndex argument of subgroupShuffle must be less than 128)");
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithQuantizeF16) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.f32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_f);

        o = b.Override(Source{{1, 2}}, "a", ty.f32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* shuffle_func = b.Call(ty.f32(), core::BuiltinFn::kQuantizeToF16, o);
        b.Return(func, shuffle_func->Result());
    });

    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
  %2:f32 = add %x, 4.0f
  %a:f32 = override %2 @id(1)
}

%foo = func():f32 {
  $B2: {
    %5:f32 = quantizeToF16 %a
    ret %5
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = -65505.0 - 4.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: value -65505.0 cannot be represented as 'f16')");
}

TEST_F(IR_SubstituteOverridesTest, Override_ShiftLeftAmountTooLarge_ConstLHS) {
    core::ir::Override* rhs = nullptr;
    b.Append(mod.root_block, [&] {
        rhs = b.Override(Source{{1, 2}}, "rhs", ty.u32());
        rhs->SetOverrideId({1});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* shift = b.ShiftLeft(1_u, rhs);
        b.Return(func, shift->Result());
    });

    auto* src = R"(
$B1: {  # root
  %rhs:u32 = override undef @id(1)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = shl 1u, %rhs
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 125.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(IR_SubstituteOverridesTest, Override_ShiftLeftAmountTooLarge_RuntimeLHS) {
    core::ir::Override* rhs = nullptr;
    b.Append(mod.root_block, [&] {
        rhs = b.Override(Source{{1, 2}}, "rhs", ty.u32());
        rhs->SetOverrideId({1});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* lhs = b.Let("lhs", 1_u);
        auto* shift = b.ShiftLeft(lhs, rhs);
        b.Return(func, shift->Result());
    });

    auto* src = R"(
$B1: {  # root
  %rhs:u32 = override undef @id(1)
}

%foo = func():u32 {
  $B2: {
    %lhs:u32 = let 1u
    %4:u32 = shl %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 125.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(IR_SubstituteOverridesTest, Override_ShiftRightAmountTooLarge_ConstLHS) {
    core::ir::Override* rhs = nullptr;
    b.Append(mod.root_block, [&] {
        rhs = b.Override(Source{{1, 2}}, "rhs", ty.u32());
        rhs->SetOverrideId({1});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* shift = b.ShiftRight(1_u, rhs);
        b.Return(func, shift->Result());
    });

    auto* src = R"(
$B1: {  # root
  %rhs:u32 = override undef @id(1)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = shr 1u, %rhs
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 125.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: shift right value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(IR_SubstituteOverridesTest, Override_ShiftRightAmountTooLarge_RuntimeLHS) {
    core::ir::Override* rhs = nullptr;
    b.Append(mod.root_block, [&] {
        rhs = b.Override(Source{{1, 2}}, "rhs", ty.u32());
        rhs->SetOverrideId({1});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* lhs = b.Let("lhs", 1_u);
        auto* shift = b.ShiftRight(lhs, rhs);
        b.Return(func, shift->Result());
    });

    auto* src = R"(
$B1: {  # root
  %rhs:u32 = override undef @id(1)
}

%foo = func():u32 {
  $B2: {
    %lhs:u32 = let 1u
    %4:u32 = shr %lhs, %rhs
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 125.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: shift right value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(IR_SubstituteOverridesTest, OverrideWithComplexGenError) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.f32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, f32(std::numeric_limits<float>::max() - 1));
        b.ir.SetSource(add, Source{{1, 2}});

        o = b.Override("a", ty.f32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
  %2:f32 = add %x, 340282346638528859811704183484516925440.0f
  %a:f32 = override %2 @id(1)
}

%foo = func():f32 {
  $B2: {
    ret %a
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = static_cast<double>(std::numeric_limits<float>::max());
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(1:2 error: '340282346638528859811704183484516925440.0 + 340282346638528859811704183484516925440.0' cannot be represented as 'f32')");
}

TEST_F(IR_SubstituteOverridesTest, OverrideWorkgroupSize) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo", o, x, o);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = @compute @workgroup_size(%a, %x, %a) func():void {
  $B2: {
    ret
  }
}
)";

    auto* expect = R"(
%foo = @compute @workgroup_size(9u, 5u, 9u) func():void {
  $B1: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideWorkgroupSizeMustBeGreaterThanZero) {
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
    });

    auto* func = b.ComputeFunction("foo", x, 1_u, 1_u);
    b.Append(func->Block(), [&] { b.Return(func); });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 0.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: @workgroup_size values must be greater than 0)");
}

TEST_F(IR_SubstituteOverridesTest, OverrideWorkgroupSizeExceedsMax) {
    core::ir::Override* x = nullptr;
    core::ir::Override* y = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({1});
        y = b.Override("y", ty.u32());
        y->SetOverrideId({2});
    });

    auto* func = b.ComputeFunction("foo", x, y, 1_u);
    b.Append(func->Block(), [&] { b.Return(func); });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 65536.0;
    cfg.map[OverrideId{2}] = 65536.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: workgroup grid size cannot exceed 4294967295)");
}

TEST_F(IR_SubstituteOverridesTest, FunctionExpression) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(10_u, x));
        b.Let("z", b.Multiply(5_u, o));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:u32 = div 10u, %x
    %y:u32 = let %5
    %7:u32 = mul 5u, %a
    %z:u32 = let %7
    ret
  }
}
)";

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %y:u32 = let 2u
    %z:u32 = let 45u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, FunctionExpressionNonConstBuiltin) {
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.f32());
        x->SetOverrideId({2});
    });

    auto* func = b.FragmentFunction("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("y", b.Call(ty.f32(), core::BuiltinFn::kDpdx, b.Multiply(x, 4_f)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
}

%foo = @fragment func():void {
  $B2: {
    %3:f32 = mul %x, 4.0f
    %4:f32 = dpdx %3
    %y:f32 = let %4
    ret
  }
}
)";

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:f32 = dpdx 20.0f
    %y:f32 = let %2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, FunctionExpressionMultiOperand) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(10_u, o));
        auto* k = b.Add(1_u, b.Multiply(2_u, x));
        b.Let("z", b.Multiply(k, o));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:u32 = div 10u, %a
    %y:u32 = let %5
    %7:u32 = mul 2u, %x
    %8:u32 = add 1u, %7
    %9:u32 = mul %8, %a
    %z:u32 = let %9
    ret
  }
}
)";

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %y:u32 = let 1u
    %z:u32 = let 99u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, FunctionExpressionMultiOperandFlipOrder) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(10_u, o));
        auto* k = b.Add(1_u, b.Multiply(2_u, o));
        b.Let("z", b.Multiply(k, x));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:u32 = div 10u, %a
    %y:u32 = let %5
    %7:u32 = mul 2u, %a
    %8:u32 = add 1u, %7
    %9:u32 = mul %8, %x
    %z:u32 = let %9
    ret
  }
}
)";

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %y:u32 = let 1u
    %z:u32 = let 95u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, FunctionExpressionMultiOperandNonConstFn) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.f32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_f);

        o = b.Override(Source{{1, 2}}, "a", ty.f32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.FragmentFunction("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(10_f, x));
        auto* k = b.Call(ty.f32(), core::BuiltinFn::kDpdx, x);
        b.Let("z", b.Multiply(k, o));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
  %2:f32 = add %x, 4.0f
  %a:f32 = override %2 @id(1)
}

%foo = @fragment func():void {
  $B2: {
    %5:f32 = div 10.0f, %x
    %y:f32 = let %5
    %7:f32 = dpdx %x
    %8:f32 = mul %7, %a
    %z:f32 = let %8
    ret
  }
}
)";

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %y:f32 = let 2.0f
    %3:f32 = dpdx 5.0f
    %4:f32 = mul %3, 9.0f
    %z:f32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, FunctionExpressionMultiOperandLet) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.f32());
        x->SetOverrideId({2});

        auto* add = b.Add(x, 4_f);

        o = b.Override(Source{{1, 2}}, "a", ty.f32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.FragmentFunction("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(10_f, x));
        auto* k = b.Let("k", b.Call(ty.f32(), core::BuiltinFn::kDpdx, x));
        b.Let("z", b.Multiply(k, o));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
  %2:f32 = add %x, 4.0f
  %a:f32 = override %2 @id(1)
}

%foo = @fragment func():void {
  $B2: {
    %5:f32 = div 10.0f, %x
    %y:f32 = let %5
    %7:f32 = dpdx %x
    %k:f32 = let %7
    %9:f32 = mul %k, %a
    %z:f32 = let %9
    ret
  }
}
)";

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %y:f32 = let 2.0f
    %3:f32 = dpdx 5.0f
    %k:f32 = let %3
    %5:f32 = mul %k, 9.0f
    %z:f32 = let %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySize) {
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        auto* ary = ty.Get<core::type::Array>(ty.i32(), cnt, 4_u);
        b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %v:ptr<workgroup, array<i32, %x>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<i32, 5>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeOverrideOutOfBounds) {
    ir::Var* v = nullptr;
    ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
        o = b.Override("y", ty.u32());
        o->SetOverrideId({3});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u);
        v = b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, u32>(), v, o);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %y:u32 = override undef @id(3)
  %v:ptr<workgroup, array<u32, %x>, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %5:ptr<workgroup, u32, read_write> = access %v, %y
    %6:u32 = load %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    cfg.map[OverrideId{3}] = 7;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: index 7 out of bounds [0..4])");
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeLetOutOfBounds) {
    ir::Var* v = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u);
        v = b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* p = b.Let("p", v);
        auto* access = b.Access(ty.ptr<workgroup, u32>(), p, 7_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %v:ptr<workgroup, array<u32, %x>, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %p:ptr<workgroup, array<u32, %x>, read_write> = let %v
    %5:ptr<workgroup, u32, read_write> = access %p, 7u
    %6:u32 = load %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: index 7 out of bounds [0..4])");
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeOutOfBounds) {
    ir::Var* v = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u);
        v = b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, u32>(), v, 7_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %v:ptr<workgroup, array<u32, %x>, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = access %v, 7u
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: index 7 out of bounds [0..4])");
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeExpression) {
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* inst = b.Multiply(x, 2_u);
        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(inst->Result());
        auto* ary = ty.Get<core::type::Array>(ty.i32(), cnt, 4_u);
        b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = mul %x, 2u
  %v:ptr<workgroup, array<i32, %2>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<i32, 10>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeIntoLet) {
    core::ir::Var* v = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        auto* ary = ty.Get<core::type::Array>(ty.i32(), cnt, 4_u);
        v = b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* y = b.Let("y", v);
        b.Let("z", y);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %v:ptr<workgroup, array<i32, %x>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %y:ptr<workgroup, array<i32, %x>, read_write> = let %v
    %z:ptr<workgroup, array<i32, %x>, read_write> = let %y
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<i32, 5>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %y:ptr<workgroup, array<i32, 5>, read_write> = let %v
    %z:ptr<workgroup, array<i32, 5>, read_write> = let %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 5;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideCondConstExprSuccess) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* cond = b.Override("cond", ty.bool_());
        cond->SetOverrideId({0});
        auto* one_f32 = b.Override("one_f32", 1_f);
        one_f32->SetOverrideId({2});
        auto* constexpr_if = b.ConstExprIf(cond);
        constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(constexpr_if->True(), [&] {
            auto* three = b.Divide(one_f32, 0.0_f);
            auto* four = b.Equal(three, 0.0_f);
            b.ExitIf(constexpr_if, four);
        });
        b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });
        o = b.Override(Source{{1, 2}}, "foo", ty.bool_());
        o->SetOverrideId({1});
        o->SetInitializer(constexpr_if->Result());
    });

    auto* func = b.Function("foo2", ty.bool_());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %cond:bool = override undef @id(0)
  %one_f32:f32 = override 1.0f @id(2)
  %3:bool = constexpr_if %cond [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %4:f32 = div %one_f32, 0.0f
      %5:bool = eq %4, 0.0f
      exit_if %5  # constexpr_if_1
    }
    $B3: {  # false
      exit_if false  # constexpr_if_1
    }
  }
  %foo:bool = override %3 @id(1)
}

%foo2 = func():bool {
  $B4: {
    ret %foo
  }
}
)";

    auto* expect = R"(
%foo2 = func():bool {
  $B1: {
    ret false
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 0;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideCondConstExprFailure) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* cond = b.Override("cond", ty.bool_());
        cond->SetOverrideId({0});
        auto* one_f32 = b.Override("one_f32", 1_f);
        one_f32->SetOverrideId({2});
        auto* constexpr_if = b.ConstExprIf(cond);
        constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(constexpr_if->True(), [&] {
            auto* three = b.Divide(one_f32, 0.0_f);
            auto* four = b.Equal(three, 0.0_f);
            b.ExitIf(constexpr_if, four);
        });
        b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });
        o = b.Override(Source{{1, 2}}, "foo", ty.bool_());
        o->SetOverrideId({1});
        o->SetInitializer(constexpr_if->Result());
    });

    auto* func = b.Function("foo2", ty.bool_());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %cond:bool = override undef @id(0)
  %one_f32:f32 = override 1.0f @id(2)
  %3:bool = constexpr_if %cond [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %4:f32 = div %one_f32, 0.0f
      %5:bool = eq %4, 0.0f
      exit_if %5  # constexpr_if_1
    }
    $B3: {  # false
      exit_if false  # constexpr_if_1
    }
  }
  %foo:bool = override %3 @id(1)
}

%foo2 = func():bool {
  $B4: {
    ret %foo
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 1;

    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: '1.0 / 0.0' cannot be represented as 'f32')");
}

TEST_F(IR_SubstituteOverridesTest, OverrideCondComplexConstExprSuccess) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* cond = b.Override("cond", ty.bool_());
        cond->SetOverrideId({0});
        auto* one_f32 = b.Override("one_f32", 1_f);
        one_f32->SetOverrideId({2});

        auto* constexpr_if = b.ConstExprIf(cond);
        constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(constexpr_if->True(), [&] {
            auto* three = b.Divide(one_f32, 1.0_f);
            auto* four = b.Equal(three, 1.0_f);
            b.ExitIf(constexpr_if, four);
        });
        b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, true); });
        o = b.Override(Source{{1, 2}}, "foo", ty.bool_());
        o->SetOverrideId({1});
        o->SetInitializer(constexpr_if->Result());
    });

    auto* func = b.Function("foo2", ty.bool_());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %cond:bool = override undef @id(0)
  %one_f32:f32 = override 1.0f @id(2)
  %3:bool = constexpr_if %cond [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %4:f32 = div %one_f32, 1.0f
      %5:bool = eq %4, 1.0f
      exit_if %5  # constexpr_if_1
    }
    $B3: {  # false
      exit_if true  # constexpr_if_1
    }
  }
  %foo:bool = override %3 @id(1)
}

%foo2 = func():bool {
  $B4: {
    ret %foo
  }
}
)";

    auto* expect = R"(
%foo2 = func():bool {
  $B1: {
    ret true
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 1;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideCondComplexConstExprNestedSuccess) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* cond = b.Override("cond", ty.bool_());
        cond->SetOverrideId({0});
        auto* zero_f32 = b.Override("zero_f32", 0_f);
        zero_f32->SetOverrideId({2});

        auto* constexpr_if = b.ConstExprIf(cond);
        constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(constexpr_if->True(), [&] {
            // Both sides (t/f) of this ConstExprIf branch will cause division by zero if evaluated.
            // However it does not get evaluated if the outer branch constant evaluates to false.
            auto* constexpr_if_inner = b.ConstExprIf(cond);
            constexpr_if_inner->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(constexpr_if_inner->True(), [&] {
                auto* bad_eval = b.Divide(1.0_f, zero_f32);
                auto* bad_eval_equal = b.Equal(bad_eval, 1.0_f);
                b.ExitIf(constexpr_if_inner, bad_eval_equal);
            });
            b.Append(constexpr_if_inner->False(), [&] {
                auto* bad_eval = b.Divide(1.0_f, zero_f32);
                auto* bad_eval_equal = b.Equal(bad_eval, 1.0_f);
                b.ExitIf(constexpr_if_inner, bad_eval_equal);
            });
            b.ExitIf(constexpr_if, constexpr_if_inner);
        });
        b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });
        o = b.Override(Source{{1, 2}}, "foo", ty.bool_());
        o->SetOverrideId({1});
        o->SetInitializer(constexpr_if->Result());
    });

    auto* func = b.Function("foo2", ty.bool_());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });

    auto* src = R"(
$B1: {  # root
  %cond:bool = override undef @id(0)
  %zero_f32:f32 = override 0.0f @id(2)
  %3:bool = constexpr_if %cond [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %4:bool = constexpr_if %cond [t: $B4, f: $B5] {  # constexpr_if_2
        $B4: {  # true
          %5:f32 = div 1.0f, %zero_f32
          %6:bool = eq %5, 1.0f
          exit_if %6  # constexpr_if_2
        }
        $B5: {  # false
          %7:f32 = div 1.0f, %zero_f32
          %8:bool = eq %7, 1.0f
          exit_if %8  # constexpr_if_2
        }
      }
      exit_if %4  # constexpr_if_1
    }
    $B3: {  # false
      exit_if false  # constexpr_if_1
    }
  }
  %foo:bool = override %3 @id(1)
}

%foo2 = func():bool {
  $B6: {
    ret %foo
  }
}
)";

    auto* expect = R"(
%foo2 = func():bool {
  $B1: {
    ret false
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 0;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, ConstExprIfInsideKernel) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({1});

        o = b.Override("y", ty.bool_());
        o->SetOverrideId({2});
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* constexpr_if = b.ConstExprIf(o);
        constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(constexpr_if->True(), [&] {
            auto* k4 = b.Add(10_u, 5_u);
            auto* k = b.Divide(k4, x);
            auto* k2 = b.Equal(k, 10_u);
            b.ExitIf(constexpr_if, k2);
        });
        b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });

        b.Let("z", constexpr_if);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(1)
  %y:bool = override undef @id(2)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = constexpr_if %y [t: $B3, f: $B4] {  # constexpr_if_1
      $B3: {  # true
        %5:u32 = add 10u, 5u
        %6:u32 = div %5, %x
        %7:bool = eq %6, 10u
        exit_if %7  # constexpr_if_1
      }
      $B4: {  # false
        exit_if false  # constexpr_if_1
      }
    }
    %z:bool = let %4
    ret
  }
}
)";

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %z:bool = let false
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 0;
    cfg.map[OverrideId{2}] = 0;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, ConstExpIfDuplicateUsage) {
    core::ir::Override* y = nullptr;
    b.Append(mod.root_block, [&] {
        y = b.Override("y", ty.bool_());
        y->SetOverrideId({1});
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* constexpr_if = b.ConstExprIf(y);
        constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(constexpr_if->True(), [&] {
            auto* k4 = b.Divide(10_u, 0_u);
            auto* k = b.Add(k4, k4);
            auto* k2 = b.Equal(k, 10_u);
            b.ExitIf(constexpr_if, k2);
        });
        b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });
        b.Let("z", constexpr_if);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %y:bool = override undef @id(1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = constexpr_if %y [t: $B3, f: $B4] {  # constexpr_if_1
      $B3: {  # true
        %4:u32 = div 10u, 0u
        %5:u32 = add %4, %4
        %6:bool = eq %5, 10u
        exit_if %6  # constexpr_if_1
      }
      $B4: {  # false
        exit_if false  # constexpr_if_1
      }
    }
    %z:bool = let %3
    ret
  }
}
)";

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %z:bool = let false
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 0;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideArrayAccessAndFailure) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.u32());
        o->SetOverrideId({0});
    });

    auto* func = b.Function("foo2", ty.u32());
    b.Append(func->Block(), [&] {
        auto* arr =
            mod.constant_values.Composite(ty.array<u32, 4>(), Vector{
                                                                  mod.constant_values.Get(1_u),
                                                                  mod.constant_values.Get(2_u),
                                                                  mod.constant_values.Get(3_u),
                                                                  mod.constant_values.Get(4_u),
                                                              });
        auto* access = b.Access(ty.u32(), b.Constant(arr), o);
        auto* r = b.Let("q", access);
        b.Return(func, r);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(0)
}

%foo2 = func():u32 {
  $B2: {
    %3:u32 = access array<u32, 4>(1u, 2u, 3u, 4u), %x
    %q:u32 = let %3
    ret %q
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 10;

    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: index 10 out of bounds [0..3])");
}

TEST_F(IR_SubstituteOverridesTest, OverrideRuntimeSizedArrayFailure) {
    core::ir::Override* o = nullptr;
    core::ir::Var* arr = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.i32());
        o->SetOverrideId({0});
        arr = b.Var("arr", ty.ptr(storage, ty.array<u32>()));
        arr->SetBindingPoint(0, 0);
    });

    auto* func = b.Function("foo2", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, u32>(), arr, o);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(0)
  %arr:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo2 = func():u32 {
  $B2: {
    %4:ptr<storage, u32, read_write> = access %arr, %x
    %5:u32 = load %4
    ret %5
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = -10;

    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: index -10 out of bounds)");
}

TEST_F(IR_SubstituteOverridesTest, OverrideConstruct) {
    core::ir::Var* global = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o0 = b.Override("o0", ty.f16());
        o0->SetOverrideId({0});
        auto* o1 = b.Override("o1", ty.f16());
        o1->SetOverrideId({1});
        auto* o2 = b.Override("o2", ty.f16());
        o2->SetOverrideId({2});
        auto* o3 = b.Override("o3", ty.f16());
        o3->SetOverrideId({3});

        auto* e = b.Construct(ty.vec4h(), o0, o1, o2, o3);
        // auto* e = b.Splat(ty.vec4h(), 1.0_h);
        auto* call_func = b.Call(ty.vec4(ty.f16()), core::BuiltinFn::kCeil, e);
        global = b.Var<private_>("global", call_func->Result());
        // global = b.Var<private_>("global", e);//e->Result());
    });

    auto* func = b.Function("foo2", ty.vec4(ty.f16()));
    b.Append(func->Block(), [&] {
        auto* inst = b.Load(global);
        b.Return(func, inst->Result());
    });

    auto* src = R"(
$B1: {  # root
  %o0:f16 = override undef @id(0)
  %o1:f16 = override undef @id(1)
  %o2:f16 = override undef @id(2)
  %o3:f16 = override undef @id(3)
  %5:vec4<f16> = construct %o0, %o1, %o2, %o3
  %6:vec4<f16> = ceil %5
  %global:ptr<private, vec4<f16>, read_write> = var %6
}

%foo2 = func():vec4<f16> {
  $B2: {
    %9:vec4<f16> = load %global
    ret %9
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %global:ptr<private, vec4<f16>, read_write> = var vec4<f16>(2.0h, 2.0h, 3.0h, 4.0h)
}

%foo2 = func():vec4<f16> {
  $B2: {
    %3:vec4<f16> = load %global
    ret %3
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 1.3;
    cfg.map[OverrideId{1}] = 2;
    cfg.map[OverrideId{2}] = 3;
    cfg.map[OverrideId{3}] = 4;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideInvalidRepresentationU32) {
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
    });
    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
}

)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = -100.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: Pipeline overridable constant 2 with value (-100.0)  is not representable in type (u32))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideInvalidRepresentationI32) {
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.i32());
        x->SetOverrideId({2});
    });
    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(2)
}

)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 8'000'000'000.0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: Pipeline overridable constant 2 with value (8000000000.0)  is not representable in type (i32))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideInvalidRepresentationF32) {
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.f32());
        x->SetOverrideId({2});
    });
    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
}

)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 3.14e40;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: Pipeline overridable constant 2 with value (31399999999999998802000170346751583059968.0)  is not representable in type (f32))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideInvalidRepresentationF16) {
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.f16());
        x->SetOverrideId({2});
    });
    auto* src = R"(
$B1: {  # root
  %x:f16 = override undef @id(2)
}

)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 65505;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: Pipeline overridable constant 2 with value (65505.0)  is not representable in type (f16))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeZeroFailure) {
    ir::Var* v = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        mod.SetSource(cnt->value, Source{{5, 8}});
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u);
        v = b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
        mod.SetSource(v, Source{{3, 2}});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, u32>(), v, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %v:ptr<workgroup, array<u32, %x>, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = access %v, 0u
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = 0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(5:8 error: array count (0) must be greater than 0)");
}

TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeNegativeFailure) {
    ir::Var* v = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.i32());
        x->SetOverrideId({2});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        mod.SetSource(cnt->value, Source{{5, 8}});
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u);
        v = b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
        mod.SetSource(v, Source{{3, 2}});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, u32>(), v, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(2)
  %v:ptr<workgroup, array<u32, %x>, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = access %v, 0u
    %5:u32 = load %4
    ret %5
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{2}] = -1;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(5:8 error: array count (-1) must be greater than 0)");
}

// See https://crbug.com/483751167
TEST_F(IR_SubstituteOverridesTest, OverrideArraySizeOverflow) {
    ir::Var* v = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.i32());
        x->SetOverrideId({0});

        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(x->Result());
        mod.SetSource(cnt->value, Source{{5, 8}});
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u);
        v = b.Var("v", ty.ptr(core::AddressSpace::kWorkgroup, ary, core::Access::kReadWrite));
        mod.SetSource(v, Source{{3, 2}});
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, u32>(), v, 10000_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(0)
  %v:ptr<workgroup, array<u32, %x>, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = access %v, 10000u
    %5:u32 = load %4
    ret %5
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 1'073'741'825;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(5:8 error: array size (4294967300) is too large)");
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedArrayParam) {
    core::ir::Var* v = nullptr;
    core::ir::Value* add = nullptr;
    const core::ir::type::ValueArrayCount* c1 = nullptr;
    const core::type::Type* a1 = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({0});
        add = b.Add(o, 2_i)->Result();
        c1 = ty.Get<core::ir::type::ValueArrayCount>(add);
        a1 = ty.Get<core::type::Array>(ty.u32(), c1, 4u);
        v = b.Var("v", ty.ptr(workgroup, a1));
    });
    auto* param = b.FunctionParam("param", ty.ptr(workgroup, a1));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({param});
    b.Append(func->Block(), [&] { b.Return(func); });
    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.Call(ty.void_(), func, v);
        b.Return(ep);
    });
    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(0)
  %2:i32 = add %x, 2i
  %v:ptr<workgroup, array<u32, %2>, read_write> = var undef
}

%foo = func(%param:ptr<workgroup, array<u32, %2>, read_write>):void {
  $B2: {
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:void = call %foo, %v
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, array<u32, 64>, read_write> = var undef
}

%foo = func(%param:ptr<workgroup, array<u32, 64>, read_write>):void {
  $B2: {
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %5:void = call %foo, %v
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 62;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedBuffer) {
    core::ir::Var* v = nullptr;
    core::ir::Value* add = nullptr;
    const core::ir::type::ValueArrayCount* c1 = nullptr;
    const core::type::Type* b1 = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({0});
        add = b.Add(o, 2_i)->Result();
        c1 = ty.Get<core::ir::type::ValueArrayCount>(add);
        b1 = ty.Get<core::type::Buffer>(c1);
        v = b.Var("v", ty.ptr(workgroup, b1));
    });
    auto* param = b.FunctionParam("param", ty.ptr(workgroup, b1));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({param});
    b.Append(func->Block(), [&] { b.Return(func); });
    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.Call(ty.void_(), func, v);
        b.Return(ep);
    });
    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(0)
  %2:i32 = add %x, 2i
  %v:ptr<workgroup, buffer<%2>, read_write> = var undef
}

%foo = func(%param:ptr<workgroup, buffer<%2>, read_write>):void {
  $B2: {
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:void = call %foo, %v
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, buffer<64>, read_write> = var undef
}

%foo = func(%param:ptr<workgroup, buffer<64>, read_write>):void {
  $B2: {
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %5:void = call %foo, %v
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{0}] = 62;
    Run(SubstituteOverrides, cfg);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedBuffer_BufferView_FixedSize) {
    core::ir::Var* v = nullptr;
    core::type::Type* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("l", v);
        b.CallExplicit(ty.ptr(workgroup, ty.array(ty.u32(), 4u)), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{ty.array(ty.u32(), 4u)}, let, 0_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(1)
  %v:ptr<workgroup, buffer<%x>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %l:ptr<workgroup, buffer<%x>, read_write> = let %v
    %5:ptr<workgroup, array<u32, 4>, read_write> = bufferView<array<u32, 4>> %l, 0u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 12;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (12 bytes) when used with bufferView (16 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedBuffer_BufferView_FixedSize_ConstOffset) {
    core::ir::Var* v = nullptr;
    core::type::Type* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("l", v);
        b.CallExplicit(ty.ptr(workgroup, ty.array(ty.u32(), 4u)), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{ty.array(ty.u32(), 4u)}, let, 4_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(1)
  %v:ptr<workgroup, buffer<%x>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %l:ptr<workgroup, buffer<%x>, read_write> = let %v
    %5:ptr<workgroup, array<u32, 4>, read_write> = bufferView<array<u32, 4>> %l, 4u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 16;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (16 bytes) when used with bufferView (20 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedBuffer_BufferView_RuntimeArray) {
    core::ir::Var* v = nullptr;
    core::type::Type* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("l", v);
        b.CallExplicit(ty.ptr(workgroup, ty.runtime_array(ty.vec4u())), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{ty.runtime_array(ty.vec4u())}, let, 0_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(1)
  %v:ptr<workgroup, buffer<%x>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %l:ptr<workgroup, buffer<%x>, read_write> = let %v
    %5:ptr<workgroup, array<vec4<u32>>, read_write> = bufferView<array<vec4<u32>>> %l, 0u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 12;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (12 bytes) when used with bufferView (16 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedBuffer_BufferView_RuntimeArray_ConstOffset) {
    core::ir::Var* v = nullptr;
    core::type::Type* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("l", v);
        b.CallExplicit(ty.ptr(workgroup, ty.runtime_array(ty.vec4u())), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{ty.runtime_array(ty.vec4u())}, let, 4_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %x:i32 = override undef @id(1)
  %v:ptr<workgroup, buffer<%x>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %l:ptr<workgroup, buffer<%x>, read_write> = let %v
    %5:ptr<workgroup, array<vec4<u32>>, read_write> = bufferView<array<vec4<u32>>> %l, 4u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 16;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (16 bytes) when used with bufferView (20 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedBuffer_BufferView_RuntimeStruct) {
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.vec4u()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.u32())},
                                        });
    core::ir::Var* v = nullptr;
    core::type::Type* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("l", v);
        b.CallExplicit(ty.ptr(workgroup, S), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{S}, let, 0_u);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:array<u32> @offset(16)
}

$B1: {  # root
  %x:i32 = override undef @id(1)
  %v:ptr<workgroup, buffer<%x>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %l:ptr<workgroup, buffer<%x>, read_write> = let %v
    %5:ptr<workgroup, S, read_write> = bufferView<S> %l, 0u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 16;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (16 bytes) when used with bufferView (20 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, OverrideSizedBuffer_BufferView_RuntimeStruct_ConstOffset) {
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.vec4u()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.u32())},
                                        });
    core::ir::Var* v = nullptr;
    core::type::Type* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        auto* o = b.Override("x", ty.i32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("l", v);
        b.CallExplicit(ty.ptr(workgroup, S), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{S}, let, 4_u);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:array<u32> @offset(16)
}

$B1: {  # root
  %x:i32 = override undef @id(1)
  %v:ptr<workgroup, buffer<%x>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %l:ptr<workgroup, buffer<%x>, read_write> = let %v
    %5:ptr<workgroup, S, read_write> = bufferView<S> %l, 4u
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 20;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (20 bytes) when used with bufferView (24 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, BufferView_OverrideSizedOffset) {
    core::ir::Var* v = nullptr;
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(workgroup, ty.buffer(128)));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.CallExplicit(ty.ptr(workgroup, ty.u32()), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{ty.u32()}, v, o);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(1)
  %v:ptr<workgroup, buffer<128>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = bufferView<u32> %v, %x
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 3;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: bufferView offset (3 bytes) must be a multiple of result alignment (4 bytes))");
}

TEST_F(IR_SubstituteOverridesTest, BufferArrayView_BufferSize) {
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.vec4u()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.u32())},
                                        });
    core::ir::Var* v = nullptr;
    core::ir::Override* buf = nullptr;
    core::ir::Override* s = nullptr;
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        buf = b.Override("b", ty.u32());
        buf->SetOverrideId({1});
        o = b.Override("o", ty.u32());
        o->SetOverrideId({2});
        s = b.Override("s", ty.u32());
        s->SetOverrideId({3});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(buf->Result());
        auto* buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.CallExplicit(ty.ptr(workgroup, S), BuiltinFn::kBufferArrayView,
                       Vector<TemplateParameter, 1>{S}, v, o, s);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:array<u32> @offset(16)
}

$B1: {  # root
  %b:u32 = override undef @id(1)
  %o:u32 = override undef @id(2)
  %s:u32 = override undef @id(3)
  %v:ptr<workgroup, buffer<%b>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %6:ptr<workgroup, S, read_write> = bufferArrayView<S> %v, %o, %s
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 24;
    cfg.map[OverrideId{2}] = 16;
    cfg.map[OverrideId{3}] = 24;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (24 bytes) when used with bufferArrayView (36 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, BufferArrayView_Size) {
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.vec4u()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.u32())},
                                        });
    core::ir::Var* v = nullptr;
    core::ir::Override* buf = nullptr;
    core::ir::Override* s = nullptr;
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        buf = b.Override("b", ty.u32());
        buf->SetOverrideId({1});
        o = b.Override("o", ty.u32());
        o->SetOverrideId({2});
        s = b.Override("s", ty.u32());
        s->SetOverrideId({3});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(buf->Result());
        auto* buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.CallExplicit(ty.ptr(workgroup, S), BuiltinFn::kBufferArrayView,
                       Vector<TemplateParameter, 1>{S}, v, o, s);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:array<u32> @offset(16)
}

$B1: {  # root
  %b:u32 = override undef @id(1)
  %o:u32 = override undef @id(2)
  %s:u32 = override undef @id(3)
  %v:ptr<workgroup, buffer<%b>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %6:ptr<workgroup, S, read_write> = bufferArrayView<S> %v, %o, %s
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 20;
    cfg.map[OverrideId{2}] = 0;
    cfg.map[OverrideId{3}] = 16;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason,
              R"(error: bufferArrayView has invalid size (16 bytes, requires 20 bytes))");
}

TEST_F(IR_SubstituteOverridesTest, BufferArrayView_SizeMultiple) {
    auto* S =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.vec4u()},
                                            {mod.symbols.New("b"), ty.runtime_array(ty.u32())},
                                        });
    core::ir::Var* v = nullptr;
    core::ir::Override* buf = nullptr;
    core::ir::Override* s = nullptr;
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        buf = b.Override("b", ty.u32());
        buf->SetOverrideId({1});
        o = b.Override("o", ty.u32());
        o->SetOverrideId({2});
        s = b.Override("s", ty.u32());
        s->SetOverrideId({3});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(buf->Result());
        auto* buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.CallExplicit(ty.ptr(workgroup, S), BuiltinFn::kBufferArrayView,
                       Vector<TemplateParameter, 1>{S}, v, o, s);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(16) {
  a:vec4<u32> @offset(0)
  b:array<u32> @offset(16)
}

$B1: {  # root
  %b:u32 = override undef @id(1)
  %o:u32 = override undef @id(2)
  %s:u32 = override undef @id(3)
  %v:ptr<workgroup, buffer<%b>, read_write> = var undef
}

%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %6:ptr<workgroup, S, read_write> = bufferArrayView<S> %v, %o, %s
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 40;
    cfg.map[OverrideId{2}] = 0;
    cfg.map[OverrideId{3}] = 21;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: bufferArrayView size (21 bytes) minus type offset (16 bytes) must be a multiple of the type stride (4 bytes))");
}

TEST_F(IR_SubstituteOverridesTest, BufferView_ThroughCall_Unsized) {
    core::ir::Var* v = nullptr;
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.u32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        auto* buffer_ty = ty.Get<core::type::Buffer>(count);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* foo = b.Function("foo", ty.void_());
    auto* p = b.FunctionParam("p", ty.ptr(workgroup, ty.unsized_buffer()));
    foo->SetParams({p});
    b.Append(foo->Block(), [&] {
        b.CallExplicit(ty.ptr(workgroup, ty.vec4u()), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{ty.vec4u()}, p, 0_u);
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.Call(ty.void_(), foo, v);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(1)
  %v:ptr<workgroup, buffer<%x>, read_write> = var undef
}

%foo = func(%p:ptr<workgroup, buffer, read_write>):void {
  $B2: {
    %5:ptr<workgroup, vec4<u32>, read_write> = bufferView<vec4<u32>> %p, 0u
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:void = call %foo, %v
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 12;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (12 bytes) when used with bufferView (16 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, BufferView_ThroughCall_SmallerSize) {
    core::ir::Var* v = nullptr;
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.u32());
        o->SetOverrideId({1});
        auto* buffer_ty = ty.buffer(128);
        v = b.Var("v", ty.ptr(workgroup, buffer_ty));
    });

    auto* foo = b.Function("foo", ty.void_());
    auto* p = b.FunctionParam("p", ty.ptr(workgroup, ty.buffer(16)));
    foo->SetParams({p});
    b.Append(foo->Block(), [&] {
        b.CallExplicit(ty.ptr(workgroup, ty.vec4u()), BuiltinFn::kBufferView,
                       Vector<TemplateParameter, 1>{ty.vec4u()}, p, o);
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("ep", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        b.Call(ty.void_(), foo, v);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(1)
  %v:ptr<workgroup, buffer<128>, read_write> = var undef
}

%foo = func(%p:ptr<workgroup, buffer<16>, read_write>):void {
  $B2: {
    %5:ptr<workgroup, vec4<u32>, read_write> = bufferView<vec4<u32>> %p, %x
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:void = call %foo, %v
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 4;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(
        result.Failure().reason,
        R"(error: invalid buffer size (16 bytes) when used with bufferView (20 bytes required))");
}

TEST_F(IR_SubstituteOverridesTest, Buffer_WorkgroupPtr_ThreeBytes) {
    Override* o = nullptr;
    core::type::Buffer* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.u32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("p", ty.ptr(workgroup, buffer_ty));
    foo->SetParams({param});
    foo->Block()->Append(b.Return(foo));

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 3;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: buffer size must be evenly divisible by 2)");
}

TEST_F(IR_SubstituteOverridesTest, Buffer_WorkgroupPtr_TwoBytes_NoF16) {
    mod.properties.Remove(Property::kAllow16BitFloats);
    Override* o = nullptr;
    core::type::Buffer* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.u32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("p", ty.ptr(workgroup, buffer_ty));
    foo->SetParams({param});
    foo->Block()->Append(b.Return(foo));

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 2;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: buffer size must be evenly divisible by 4)");
}

TEST_F(IR_SubstituteOverridesTest, Buffer_WorkgroupPtr_TwoBytes_F16) {
    Override* o = nullptr;
    core::type::Buffer* buffer_ty = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("x", ty.u32());
        o->SetOverrideId({1});
        auto* count = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        buffer_ty = ty.Get<core::type::Buffer>(count);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("p", ty.ptr(workgroup, buffer_ty));
    foo->SetParams({param});
    foo->Block()->Append(b.Return(foo));

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 2;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_EQ(result, Success);
}

TEST_F(IR_SubstituteOverridesTest, SubgroupSize_NotPowerOf2) {
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
    });
    auto* foo = b.ComputeFunction("foo", o->Result(), 1_u, 1_u);
    foo->SetSubgroupSize(o->Result());
    foo->Block()->Append(b.Return(foo));

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 3;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: @subgroup_size value must be a power of two)");
}

TEST_F(IR_SubstituteOverridesTest, SubgroupSize_NotPowerOf2_Expr) {
    Override* o = nullptr;
    Value* add = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        add = b.Add(o, 1_u)->Result();
    });
    auto* foo = b.ComputeFunction("foo", add, 1_u, 1_u);
    foo->SetSubgroupSize(add);
    foo->Block()->Append(b.Return(foo));

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 2;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, R"(error: @subgroup_size value must be a power of two)");
}

template <typename T>
const core::type::Type* TypeBuilder(core::type::Manager& m) {
    return m.Get<T>();
}

using TypeBuilderFn = const core::type::Type* (*)(core::type::Manager&);

// Params:
// - component type
// - columns
// - rows
// - col_major
// - load/store
// - array type
using SubgroupMatrixSizesParam =
    std::tuple<TypeBuilderFn, uint32_t, uint32_t, bool, bool, bool, TypeBuilderFn>;

struct SubgroupMatrixSizes : public TransformTestWithParam<SubgroupMatrixSizesParam> {
    const core::type::SubgroupMatrix* MatrixType() {
        auto* type = std::get<0>(GetParam())(ty);
        const uint32_t cols = std::get<1>(GetParam());
        const uint32_t rows = std::get<2>(GetParam());
        return ty.subgroup_matrix_left(type, cols, rows);
    }
    const core::type::Type* ArrayElemType() { return std::get<6>(GetParam())(ty); }
    uint32_t ArrayStride() { return ty.runtime_array(ArrayElemType())->ImplicitStride(); }
    uint32_t MajorSize() {
        const uint32_t cols = std::get<1>(GetParam());
        const uint32_t rows = std::get<2>(GetParam());
        const bool col_major = std::get<3>(GetParam());
        return col_major ? cols : rows;
    }
    uint32_t MinorSize() {
        const uint32_t cols = std::get<1>(GetParam());
        const uint32_t rows = std::get<2>(GetParam());
        const bool col_major = std::get<3>(GetParam());
        return col_major ? rows : cols;
    }
    uint32_t MinStride() {
        auto* type = std::get<0>(GetParam())(ty);
        return MinorSize() * type->Size();
    }
    CoreBuiltinCall* MakeCall(Value* pointer, Value* object, Value* offset, Value* stride) {
        const bool col_major = std::get<3>(GetParam());
        const bool load = std::get<4>(GetParam());
        auto* mat_ty = MatrixType();
        if (load) {
            return b.CallExplicit(
                mat_ty, BuiltinFn::kSubgroupMatrixLoad,
                Vector<TemplateParameter, 2>{
                    mat_ty, col_major ? Majorness::kColMajor : Majorness::kRowMajor},
                pointer, offset, stride);
        } else {
            return b.CallExplicit(ty.void_(), BuiltinFn::kSubgroupMatrixStore,
                                  Vector<TemplateParameter, 1>{col_major ? Majorness::kColMajor
                                                                         : Majorness::kRowMajor},
                                  pointer, offset, object, stride);
        }
    }
};

TEST_P(SubgroupMatrixSizes, Stride_TooSmallForType) {
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.runtime_array(ArrayElemType())));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        MakeCall(v->Result(), value, b.Constant(u32(0)), o->Result());
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = (MinStride() / ArrayStride()) - 1;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("stride (" + std::to_string(MinStride() - ArrayStride()) +
                                   " bytes) must be greater or equal to " +
                                   std::to_string(MinStride()) + " bytes"));
}

TEST_P(SubgroupMatrixSizes, Stride_TooLarge) {
    auto* mat_ty = MatrixType();

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.runtime_array(ArrayElemType())));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        MakeCall(v->Result(), value, b.Constant(u32(0)), o->Result());
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 0xfffffffe;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason, testing::HasSubstr("has a stride exceeding 32 bits"));
}

TEST_P(SubgroupMatrixSizes, Offset_TooLarge) {
    auto* mat_ty = MatrixType();

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.runtime_array(ArrayElemType())));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        MakeCall(v->Result(), value, o->Result(), b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 0xfffffffe;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason, testing::HasSubstr("has an offset exceeding 32 bits"));
}

TEST_P(SubgroupMatrixSizes, Pointer_TooSmallForType_MinStride) {
    auto* type = std::get<0>(GetParam())(ty);
    const bool load = std::get<4>(GetParam());
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t min_array_size = MinStride() * (MajorSize() - 1) + MinorSize() * type->Size();

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.array(ArrayElemType(), min_array_size / ArrayStride())));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        MakeCall(v->Result(), value, o->Result(), b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 1;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(
        result.Failure().reason,
        testing::HasSubstr("invalid storage size (" + std::to_string(min_array_size) +
                           " bytes) when used with " +
                           (load ? "subgroupMatrixLoad" : "subgroupMatrixStore") + " (" +
                           std::to_string(min_array_size + ArrayStride()) + " bytes required)"));
}

TEST_P(SubgroupMatrixSizes, Storage_TooSmallForType) {
    auto* type = std::get<0>(GetParam())(ty);
    const bool load = std::get<4>(GetParam());
    auto* mat_ty = MatrixType();

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * 2 * (MajorSize() - 1) + MinorSize() * type->Size());

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.array(ArrayElemType(), array_size / ArrayStride())));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        MakeCall(v->Result(), value, b.Constant(u32(4)), o->Result());
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = (2 * MinStride()) / ArrayStride();
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    auto rounded = RoundUp(ArrayStride(), array_size + 4 * ArrayStride());
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("invalid storage size (" + std::to_string(array_size) +
                                   " bytes) when used with " +
                                   (load ? "subgroupMatrixLoad" : "subgroupMatrixStore") + " (" +
                                   std::to_string(rounded) + " bytes required)"));
}

TEST_P(SubgroupMatrixSizes, Storage_TooSmallForType_NonConstStride) {
    auto* type = std::get<0>(GetParam())(ty);
    const bool load = std::get<4>(GetParam());
    auto* mat_ty = MatrixType();

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.array(ArrayElemType(), array_size / ArrayStride())));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({value, stride});
    b.Append(foo->Block(), [&] {
        MakeCall(v->Result(), value, o->Result(), stride);
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 4;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    uint32_t rounded = RoundUp(ArrayStride(), array_size + 4 * ArrayStride());
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("invalid storage size (" + std::to_string(array_size) +
                                   " bytes) when used with " +
                                   (load ? "subgroupMatrixLoad" : "subgroupMatrixStore") + " (" +
                                   std::to_string(rounded) + " bytes required)"));
}

TEST_P(SubgroupMatrixSizes, Storage_TooSmall_BufferView) {
    auto* type = std::get<0>(GetParam())(ty);
    const bool load = std::get<4>(GetParam());
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size = MinStride() * (MajorSize() - 1) + MinorSize() * type->Size();

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(workgroup, ty.buffer(array_size)));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        auto* view = b.CallExplicit(
            ty.ptr(workgroup, ty.runtime_array(ArrayElemType())), BuiltinFn::kBufferView,
            Vector<TemplateParameter, 1>{ty.runtime_array(ArrayElemType())}, v, o->Result());
        MakeCall(view->Result(), value, b.Constant(u32(0)),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = ArrayStride();
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(
        result.Failure().reason,
        testing::HasSubstr("invalid storage size (" + std::to_string(array_size) +
                           " bytes) when used with " +
                           (load ? "subgroupMatrixLoad" : "subgroupMatrixStore") + " (" +
                           std::to_string(array_size + ArrayStride()) + " bytes required)"));
}

TEST_P(SubgroupMatrixSizes, Storage_TooSmall_BufferView_SizedParam) {
    auto* type = std::get<0>(GetParam())(ty);
    const bool load = std::get<4>(GetParam());
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size = MinStride() * (MajorSize() - 1) + MinorSize() * type->Size();

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.unsized_buffer()));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    auto* p = b.FunctionParam("p", ty.buffer(array_size));
    foo->SetParams({value, p});
    b.Append(foo->Block(), [&] {
        auto* view = b.CallExplicit(
            ty.ptr(workgroup, ty.runtime_array(ArrayElemType())), BuiltinFn::kBufferView,
            Vector<TemplateParameter, 1>{ty.runtime_array(ArrayElemType())}, p, o->Result());
        MakeCall(view->Result(), value, b.Constant(u32(0)),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });
    auto* bar = b.Function("bar", ty.void_());
    auto* value2 = b.FunctionParam("mat", mat_ty);
    bar->SetParams({value2});
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo, value2, v);
        b.Return(bar);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = ArrayStride();
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(
        result.Failure().reason,
        testing::HasSubstr("invalid storage size (" + std::to_string(array_size) +
                           " bytes) when used with " +
                           (load ? "subgroupMatrixLoad" : "subgroupMatrixStore") + " (" +
                           std::to_string(array_size + ArrayStride()) + " bytes required)"));
}

TEST_P(SubgroupMatrixSizes, Pointer_TooSmall_BufferView_Result) {
    auto* type = std::get<0>(GetParam())(ty);
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.unsized_buffer()));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    auto* p = b.FunctionParam("p", ty.buffer(2 * array_size));
    foo->SetParams({value, p});
    b.Append(foo->Block(), [&] {
        auto* arr_ty = ty.array(ArrayElemType(), array_size / ArrayStride() - 1);
        auto* view = b.CallExplicit(ty.ptr(workgroup, arr_ty), BuiltinFn::kBufferView,
                                    Vector<TemplateParameter, 1>{arr_ty}, p, o->Result());
        MakeCall(view->Result(), value, b.Constant(u32(0)),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });
    auto* bar = b.Function("bar", ty.void_());
    auto* value2 = b.FunctionParam("mat", mat_ty);
    bar->SetParams({value2});
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo, value2, v);
        b.Return(bar);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("requires more memory (" + std::to_string(array_size) +
                                   " bytes) than pointed to (" +
                                   std::to_string(array_size - ArrayStride()) + " bytes)"));
}

TEST_P(SubgroupMatrixSizes, Pointer_TooSmall_BufferArrayView_SizeParam) {
    auto* type = std::get<0>(GetParam())(ty);
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(workgroup, ty.buffer(2 * array_size)));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        auto* view = b.CallExplicit(
            ty.ptr(workgroup, ty.runtime_array(ArrayElemType())), BuiltinFn::kBufferArrayView,
            Vector<TemplateParameter, 1>{ty.runtime_array(ArrayElemType())}, v, 0_u, o->Result());
        MakeCall(view->Result(), value, b.Constant(u32(0)),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = array_size - ArrayStride();
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("requires more memory (" + std::to_string(array_size) +
                                   " bytes) than pointed to (" +
                                   std::to_string(array_size - ArrayStride()) + " bytes)"));
}

TEST_P(SubgroupMatrixSizes, Pointer_TooSmall_Access_Array) {
    auto* type = std::get<0>(GetParam())(ty);
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());
    auto* arr_ty = ty.array(ArrayElemType(), array_size / ArrayStride() - 1);

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.runtime_array(arr_ty)));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, arr_ty), v, o->Result());
        MakeCall(access->Result(), value, b.Constant(u32(0)),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("requires more memory (" + std::to_string(array_size) +
                                   " bytes) than pointed to (" +
                                   std::to_string(array_size - ArrayStride()) + " bytes)"));
}

TEST_P(SubgroupMatrixSizes, Pointer_TooSmall_Access_Array_Offset) {
    auto* type = std::get<0>(GetParam())(ty);
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());
    auto* arr_ty = ty.array(ArrayElemType(), array_size / ArrayStride());

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.array(arr_ty, 2)));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, arr_ty), v, 1_u);
        MakeCall(access->Result(), value, o->Result(),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 1;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(
        result.Failure().reason,
        testing::HasSubstr("requires more memory (" + std::to_string(array_size + ArrayStride()) +
                           " bytes) than pointed to (" + std::to_string(array_size) + " bytes)"));
}

TEST_P(SubgroupMatrixSizes, Pointer_TooSmall_StructMember) {
    auto* type = std::get<0>(GetParam())(ty);
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());
    auto* arr_ty = ty.array(ArrayElemType(), array_size / ArrayStride() - 1);

    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.vec4u()},
                                                  {mod.symbols.New("b"), arr_ty},
                                              });

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, S));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, arr_ty), v, 1_u);
        MakeCall(access->Result(), value, o->Result(),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 0;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("requires more memory (" + std::to_string(array_size) +
                                   " bytes) than pointed to (" +
                                   std::to_string(array_size - ArrayStride()) + " bytes)"));
}

TEST_P(SubgroupMatrixSizes, Storage_TooSmall_BufferView_Access_Array) {
    auto* type = std::get<0>(GetParam())(ty);
    const bool load = std::get<4>(GetParam());
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());
    auto* arr_ty = ty.array(ArrayElemType(), array_size / ArrayStride());

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.buffer(2 * array_size)));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        auto* view =
            b.CallExplicit(ty.ptr(storage, ty.runtime_array(arr_ty)), BuiltinFn::kBufferView,
                           Vector<TemplateParameter, 1>{ty.runtime_array(arr_ty)}, v, o->Result());
        auto* access = b.Access(ty.ptr(storage, arr_ty), view, 1_u);
        MakeCall(access->Result(), value, b.Constant(u32(0)),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = ArrayStride();
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(
        result.Failure().reason,
        testing::HasSubstr("invalid storage size (" + std::to_string(2 * array_size) +
                           " bytes) when used with " +
                           (load ? "subgroupMatrixLoad" : "subgroupMatrixStore") + " (" +
                           std::to_string(2 * array_size + ArrayStride()) + " bytes required)"));
}

TEST_P(SubgroupMatrixSizes, Storage_TooSmall_BufferView_Access_Struct) {
    auto* type = std::get<0>(GetParam())(ty);
    const bool load = std::get<4>(GetParam());
    auto* mat_ty = MatrixType();

    if (MinStride() < ArrayStride()) {
        return;
    }

    uint32_t array_size =
        RoundUp(ArrayStride(), MinStride() * (MajorSize() - 1) + MinorSize() * type->Size());

    auto* S = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.New("a"), ty.vec4u()},
                            {mod.symbols.New("b"), ty.runtime_array(ArrayElemType())},
                        });

    Var* v = nullptr;
    Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override("o", ty.u32());
        o->SetOverrideId({1});
        v = b.Var("v", ty.ptr(storage, ty.buffer(array_size + 16)));
        v->SetBindingPoint(0, 0);
    });
    auto* foo = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("mat", mat_ty);
    foo->SetParams({value});
    b.Append(foo->Block(), [&] {
        auto* view =
            b.CallExplicit(ty.ptr(storage, S), BuiltinFn::kBufferView,
                           Vector<TemplateParameter, 1>{ty.runtime_array(ArrayElemType())}, v, 0_u);
        auto* access = b.Access(ty.ptr(storage, ty.runtime_array(ArrayElemType())), view, 1_u);
        MakeCall(access->Result(), value, o->Result(),
                 b.Constant(u32(MinStride() / ArrayStride())));
        b.Return(foo);
    });

    SubstituteOverridesConfig cfg{};
    cfg.map[OverrideId{1}] = 1;
    auto result = RunWithFailure(SubstituteOverrides, cfg);
    ASSERT_NE(result, Success);
    EXPECT_THAT(
        result.Failure().reason,
        testing::HasSubstr("invalid storage size (" + std::to_string(array_size + 16) +
                           " bytes) when used with " +
                           (load ? "subgroupMatrixLoad" : "subgroupMatrixStore") + " (" +
                           std::to_string(array_size + ArrayStride() + 16) + " bytes required)"));
}

// Only worth testing one type of each size.
INSTANTIATE_TEST_SUITE_P(
    IR_SubstituteOverridesTest,
    SubgroupMatrixSizes,
    testing::Combine(testing::Values(TypeBuilder<f32>, TypeBuilder<f16>, TypeBuilder<i8>),
                     testing::Values(8, 16),
                     testing::Values(8, 16),
                     testing::Values(true, false),
                     testing::Values(true, false),
                     testing::Values(true, false),
                     testing::Values(TypeBuilder<f16>,
                                     TypeBuilder<u32>,
                                     TypeBuilder<vec2i>,
                                     TypeBuilder<vec3f>,
                                     TypeBuilder<vec4u>)));

}  // namespace
}  // namespace tint::core::ir::transform
