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

#include "src/tint/lang/glsl/writer/raise/binary_polyfill.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_BinaryPolyfillTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_BinaryPolyfillTest, BitwiseBoolAnd) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.And(ty.bool_(), true, false));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:bool = and true, false
    %x:bool = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = convert false
    %4:u32 = and %2, %3
    %5:bool = convert %4
    %x:bool = let %5
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, BitwiseBoolAnd_Vec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* lhs = b.Splat(ty.vec2<bool>(), true);
        auto* rhs = b.Splat(ty.vec2<bool>(), false);
        b.Let("x", b.And(ty.vec2<bool>(), lhs, rhs));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<bool> = and vec2<bool>(true), vec2<bool>(false)
    %x:vec2<bool> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<u32> = convert vec2<bool>(true)
    %3:vec2<u32> = convert vec2<bool>(false)
    %4:vec2<u32> = and %2, %3
    %5:vec2<bool> = convert %4
    %x:vec2<bool> = let %5
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, BitwiseBoolOr) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Or(ty.bool_(), true, false));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:bool = or true, false
    %x:bool = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = convert false
    %4:u32 = or %2, %3
    %5:bool = convert %4
    %x:bool = let %5
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, BitwiseBoolOr_Vec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* lhs = b.Splat(ty.vec2<bool>(), true);
        auto* rhs = b.Splat(ty.vec2<bool>(), false);
        b.Let("x", b.Or(ty.vec2<bool>(), lhs, rhs));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<bool> = or vec2<bool>(true), vec2<bool>(false)
    %x:vec2<bool> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<u32> = convert vec2<bool>(true)
    %3:vec2<u32> = convert vec2<bool>(false)
    %4:vec2<u32> = or %2, %3
    %5:vec2<bool> = convert %4
    %x:vec2<bool> = let %5
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, RelationalEqualF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<f16>(), 1_h));
        auto* r = b.Let("right", b.Splat(ty.vec2<f16>(), 2_h));
        auto* bin = b.Equal(ty.vec2<bool>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f16> = let vec2<f16>(1.0h)
    %right:vec2<f16> = let vec2<f16>(2.0h)
    %4:vec2<bool> = eq %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f16> = let vec2<f16>(1.0h)
    %right:vec2<f16> = let vec2<f16>(2.0h)
    %4:vec2<bool> = glsl.equal %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, RelationalEqualBool) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<bool>(), true));
        auto* r = b.Let("right", b.Splat(ty.vec2<bool>(), false));
        auto* bin = b.Equal(ty.vec2<bool>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<bool> = let vec2<bool>(true)
    %right:vec2<bool> = let vec2<bool>(false)
    %4:vec2<bool> = eq %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<bool> = let vec2<bool>(true)
    %right:vec2<bool> = let vec2<bool>(false)
    %4:vec2<bool> = glsl.equal %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, RelationalNotEqualI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<i32>(), 1_i));
        auto* r = b.Let("right", b.Splat(ty.vec2<i32>(), 2_i));
        auto* bin = b.NotEqual(ty.vec2<bool>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<i32> = let vec2<i32>(1i)
    %right:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<bool> = neq %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<i32> = let vec2<i32>(1i)
    %right:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<bool> = glsl.notEqual %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, RelationalLessThanF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<f32>(), 1_f));
        auto* r = b.Let("right", b.Splat(ty.vec2<f32>(), 2_f));
        auto* bin = b.LessThan(ty.vec2<bool>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<bool> = lt %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<bool> = glsl.lessThan %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, RelationalLessThanEqualU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<u32>(), 1_u));
        auto* r = b.Let("right", b.Splat(ty.vec2<u32>(), 2_u));
        auto* bin = b.LessThanEqual(ty.vec2<bool>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<u32> = let vec2<u32>(1u)
    %right:vec2<u32> = let vec2<u32>(2u)
    %4:vec2<bool> = lte %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<u32> = let vec2<u32>(1u)
    %right:vec2<u32> = let vec2<u32>(2u)
    %4:vec2<bool> = glsl.lessThanEqual %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, RelationalGreaterThanF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<f32>(), 1_f));
        auto* r = b.Let("right", b.Splat(ty.vec2<f32>(), 2_f));
        auto* bin = b.GreaterThan(ty.vec2<bool>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<bool> = gt %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<bool> = glsl.greaterThan %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, RelationalGreaterThanEqualF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<f32>(), 1_f));
        auto* r = b.Let("right", b.Splat(ty.vec2<f32>(), 2_f));
        auto* bin = b.GreaterThanEqual(ty.vec2<bool>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<bool> = gte %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<bool> = glsl.greaterThanEqual %left, %right
    %val:vec2<bool> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, FloatModF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<f32>(), 1_f));
        auto* r = b.Let("right", b.Splat(ty.vec2<f32>(), 2_f));
        auto* bin = b.Modulo(ty.vec2<f32>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<f32> = mod %left, %right
    %val:vec2<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f32> = let vec2<f32>(1.0f)
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<f32> = call %tint_float_modulo, %left, %right
    %val:vec2<f32> = let %4
    ret
  }
}
%tint_float_modulo = func(%x:vec2<f32>, %y:vec2<f32>):vec2<f32> {
  $B2: {
    %9:vec2<f32> = div %x, %y
    %10:vec2<f32> = trunc %9
    %11:vec2<f32> = mul %y, %10
    %12:vec2<f32> = sub %x, %11
    ret %12
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, FloatModMixedF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", 1_f);
        auto* r = b.Let("right", b.Splat(ty.vec2<f32>(), 2_f));
        auto* bin = b.Modulo(ty.vec2<f32>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:f32 = let 1.0f
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<f32> = mod %left, %right
    %val:vec2<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:f32 = let 1.0f
    %right:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<f32> = construct %left
    %5:vec2<f32> = call %tint_float_modulo, %4, %right
    %val:vec2<f32> = let %5
    ret
  }
}
%tint_float_modulo = func(%x:vec2<f32>, %y:vec2<f32>):vec2<f32> {
  $B2: {
    %10:vec2<f32> = div %x, %y
    %11:vec2<f32> = trunc %10
    %12:vec2<f32> = mul %y, %11
    %13:vec2<f32> = sub %x, %12
    ret %13
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BinaryPolyfillTest, FloatModF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Splat(ty.vec2<f16>(), 1_h));
        auto* r = b.Let("right", b.Splat(ty.vec2<f16>(), 2_h));
        auto* bin = b.Modulo(ty.vec2<f16>(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f16> = let vec2<f16>(1.0h)
    %right:vec2<f16> = let vec2<f16>(2.0h)
    %4:vec2<f16> = mod %left, %right
    %val:vec2<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %left:vec2<f16> = let vec2<f16>(1.0h)
    %right:vec2<f16> = let vec2<f16>(2.0h)
    %4:vec2<f16> = call %tint_float_modulo, %left, %right
    %val:vec2<f16> = let %4
    ret
  }
}
%tint_float_modulo = func(%x:vec2<f16>, %y:vec2<f16>):vec2<f16> {
  $B2: {
    %9:vec2<f16> = div %x, %y
    %10:vec2<f16> = trunc %9
    %11:vec2<f16> = mul %y, %10
    %12:vec2<f16> = sub %x, %11
    ret %12
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
