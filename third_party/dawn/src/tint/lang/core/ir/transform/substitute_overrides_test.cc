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
#include <utility>

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

using IR_SubstituteOverridesTest = TransformTest;

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
        auto* add = b.Add(ty.u32(), 2_u, 4_u);

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
        auto* add = b.Add(ty.u32(), 2_u, 4_u);

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

        auto* add = b.Add(ty.u32(), x, 4_u);

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
        auto* add = b.Add(ty.u32(), x, 4_u);
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

        auto* add = b.Add(ty.f32(), x, 4_f);

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

TEST_F(IR_SubstituteOverridesTest, OverrideWithComplexGenError) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.f32());
        x->SetOverrideId({2});

        auto* add = b.Add(ty.f32(), x, f32(std::numeric_limits<float>::max() - 1));
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

        auto* add = b.Add(ty.u32(), x, 4_u);

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

TEST_F(IR_SubstituteOverridesTest, FunctionExpression) {
    core::ir::Override* o = nullptr;
    core::ir::Override* x = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});

        auto* add = b.Add(ty.u32(), x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(ty.u32(), 10_u, x));
        b.Let("z", b.Multiply(ty.u32(), 5_u, o));
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

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("y", b.Call(ty.f32(), core::BuiltinFn::kDpdx, b.Multiply(ty.f32(), x, 4_f)));
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

        auto* add = b.Add(ty.u32(), x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(ty.u32(), 10_u, o));
        auto* k = b.Add(ty.u32(), 1_u, b.Multiply(ty.u32(), 2_u, x));
        b.Let("z", b.Multiply(ty.u32(), k, o));
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

        auto* add = b.Add(ty.u32(), x, 4_u);

        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(ty.u32(), 10_u, o));
        auto* k = b.Add(ty.u32(), 1_u, b.Multiply(ty.u32(), 2_u, o));
        b.Let("z", b.Multiply(ty.u32(), k, x));
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

        auto* add = b.Add(ty.f32(), x, 4_f);

        o = b.Override(Source{{1, 2}}, "a", ty.f32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(ty.f32(), 10_f, x));
        auto* k = b.Call(ty.f32(), core::BuiltinFn::kDpdx, x);
        b.Let("z", b.Multiply(ty.f32(), k, o));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
  %2:f32 = add %x, 4.0f
  %a:f32 = override %2 @id(1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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

        auto* add = b.Add(ty.f32(), x, 4_f);

        o = b.Override(Source{{1, 2}}, "a", ty.f32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("y", b.Divide(ty.f32(), 10_f, x));
        auto* k = b.Let("k", b.Call(ty.f32(), core::BuiltinFn::kDpdx, x));
        b.Let("z", b.Multiply(ty.f32(), k, o));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %x:f32 = override undef @id(2)
  %2:f32 = add %x, 4.0f
  %a:f32 = override %2 @id(1)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
        auto* ary = ty.Get<core::type::Array>(ty.i32(), cnt, 4_u, 4_u, 4_u, 4_u);
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
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u, 4_u, 4_u, 4_u);
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
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u, 4_u, 4_u, 4_u);
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
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u, 4_u, 4_u, 4_u);
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

        auto* inst = b.Multiply(ty.u32(), x, 2_u);
        auto* cnt = ty.Get<core::ir::type::ValueArrayCount>(inst->Result());
        auto* ary = ty.Get<core::type::Array>(ty.i32(), cnt, 4_u, 4_u, 4_u, 4_u);
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
        auto* ary = ty.Get<core::type::Array>(ty.i32(), cnt, 4_u, 4_u, 4_u, 4_u);
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
            auto* three = b.Divide(ty.f32(), one_f32, 0.0_f);
            auto* four = b.Equal(ty.bool_(), three, 0.0_f);
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
            auto* three = b.Divide(ty.f32(), one_f32, 0.0_f);
            auto* four = b.Equal(ty.bool_(), three, 0.0_f);
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
            auto* three = b.Divide(ty.f32(), one_f32, 1.0_f);
            auto* four = b.Equal(ty.bool_(), three, 1.0_f);
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
                auto* bad_eval = b.Divide(ty.f32(), 1.0_f, zero_f32);
                auto* bad_eval_equal = b.Equal(ty.bool_(), bad_eval, 1.0_f);
                b.ExitIf(constexpr_if_inner, bad_eval_equal);
            });
            b.Append(constexpr_if_inner->False(), [&] {
                auto* bad_eval = b.Divide(ty.f32(), 1.0_f, zero_f32);
                auto* bad_eval_equal = b.Equal(ty.bool_(), bad_eval, 1.0_f);
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
            auto* k4 = b.Add(ty.u32(), 10_u, 5_u);
            auto* k = b.Divide(ty.u32(), k4, x);
            auto* k2 = b.Equal(ty.bool_(), k, 10_u);
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
            auto* k4 = b.Divide(ty.u32(), 10_u, 0_u);
            auto* k = b.Add(ty.u32(), k4, k4);
            auto* k2 = b.Equal(ty.bool_(), k, 10_u);
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

        auto* e = b.Construct(ty.vec4<f16>(), o0, o1, o2, o3);
        // auto* e = b.Splat(ty.vec4<f16>(), 1.0_h);
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
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u, 4_u, 4_u, 4_u);
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
        auto* ary = ty.Get<core::type::Array>(ty.u32(), cnt, 4_u, 4_u, 4_u, 4_u);
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

}  // namespace
}  // namespace tint::core::ir::transform
