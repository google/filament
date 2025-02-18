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

#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/sampled_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_BuiltinPolyfillTest : public TransformTest {
  protected:
    /// Helper to build a function that calls a builtin with the given result and argument types.
    /// @param builtin the builtin to call
    /// @param result_ty the result type of the builtin call
    /// @param arg_types the arguments types for the builtin call
    void Build(core::BuiltinFn builtin,
               const core::type::Type* result_ty,
               VectorRef<const core::type::Type*> arg_types) {
        Vector<FunctionParam*, 4> args;
        for (auto* arg_ty : arg_types) {
            args.Push(b.FunctionParam("arg", arg_ty));
        }
        auto* func = b.Function("foo", result_ty);
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            auto* result = b.Call(result_ty, builtin, args);
            b.Return(func, result);
            mod.SetName(result, "result");
        });
    }
};

TEST_F(IR_BuiltinPolyfillTest, Saturate_NoPolyfill) {
    Build(core::BuiltinFn::kSaturate, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = saturate %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_F32) {
    Build(core::BuiltinFn::kSaturate, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = clamp %arg, 0.0f, 1.0f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_F16) {
    Build(core::BuiltinFn::kSaturate, ty.f16(), Vector{ty.f16()});
    auto* src = R"(
%foo = func(%arg:f16):f16 {
  $B1: {
    %result:f16 = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f16):f16 {
  $B1: {
    %result:f16 = clamp %arg, 0.0h, 1.0h
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_Vec2F32) {
    Build(core::BuiltinFn::kSaturate, ty.vec2<f32>(), Vector{ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> {
  $B1: {
    %result:vec2<f32> = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> {
  $B1: {
    %result:vec2<f32> = clamp %arg, vec2<f32>(0.0f), vec2<f32>(1.0f)
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_Vec4F16) {
    Build(core::BuiltinFn::kSaturate, ty.vec4<f16>(), Vector{ty.vec4<f16>()});
    auto* src = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> {
  $B1: {
    %result:vec4<f16> = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> {
  $B1: {
    %result:vec4<f16> = clamp %arg, vec4<f16>(0.0h), vec4<f16>(1.0h)
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Smoothstep_F32) {
    Build(core::BuiltinFn::kSmoothstep, ty.f32(), Vector{ty.f32(), ty.f32(), ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32, %arg_1:f32, %arg_2:f32):f32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:f32 = smoothstep %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f32, %arg_1:f32, %arg_2:f32):f32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:f32 = sub %arg_2, %arg
    %6:f32 = sub %arg_1, %arg
    %7:f32 = div %5, %6
    %8:f32 = clamp %7, 0.0f, 1.0f
    %9:f32 = mul 2.0f, %8
    %10:f32 = sub 3.0f, %9
    %11:f32 = mul %8, %10
    %result:f32 = mul %8, %11
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Smoothstep_F16) {
    Build(core::BuiltinFn::kSmoothstep, ty.f16(), Vector{ty.f16(), ty.f16(), ty.f16()});
    auto* src = R"(
%foo = func(%arg:f16, %arg_1:f16, %arg_2:f16):f16 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:f16 = smoothstep %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f16, %arg_1:f16, %arg_2:f16):f16 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:f16 = sub %arg_2, %arg
    %6:f16 = sub %arg_1, %arg
    %7:f16 = div %5, %6
    %8:f16 = clamp %7, 0.0h, 1.0h
    %9:f16 = mul 2.0h, %8
    %10:f16 = sub 3.0h, %9
    %11:f16 = mul %8, %10
    %result:f16 = mul %8, %11
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Smoothstep_Vec2F32) {
    Build(core::BuiltinFn::kSmoothstep, ty.vec2<f32>(),
          Vector{ty.vec2<f32>(), ty.vec2<f32>(), ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:vec2<f32>, %arg_1:vec2<f32>, %arg_2:vec2<f32>):vec2<f32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec2<f32> = smoothstep %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<f32>, %arg_1:vec2<f32>, %arg_2:vec2<f32>):vec2<f32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:vec2<f32> = sub %arg_2, %arg
    %6:vec2<f32> = sub %arg_1, %arg
    %7:vec2<f32> = div %5, %6
    %8:vec2<f32> = clamp %7, vec2<f32>(0.0f), vec2<f32>(1.0f)
    %9:vec2<f32> = mul vec2<f32>(2.0f), %8
    %10:vec2<f32> = sub vec2<f32>(3.0f), %9
    %11:vec2<f32> = mul %8, %10
    %result:vec2<f32> = mul %8, %11
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Smoothstep_Vec4F16) {
    Build(core::BuiltinFn::kSmoothstep, ty.vec4<f16>(),
          Vector{ty.vec4<f16>(), ty.vec4<f16>(), ty.vec4<f16>()});
    auto* src = R"(
%foo = func(%arg:vec4<f16>, %arg_1:vec4<f16>, %arg_2:vec4<f16>):vec4<f16> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec4<f16> = smoothstep %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f16>, %arg_1:vec4<f16>, %arg_2:vec4<f16>):vec4<f16> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:vec4<f16> = sub %arg_2, %arg
    %6:vec4<f16> = sub %arg_1, %arg
    %7:vec4<f16> = div %5, %6
    %8:vec4<f16> = clamp %7, vec4<f16>(0.0h), vec4<f16>(1.0h)
    %9:vec4<f16> = mul vec4<f16>(2.0h), %8
    %10:vec4<f16> = sub vec4<f16>(3.0h), %9
    %11:vec4<f16> = mul %8, %10
    %result:vec4<f16> = mul %8, %11
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_NoPolyfill) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_U32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %3:bool = lte %arg, 65535u
    %4:u32 = select 0u, 16u, %3
    %5:u32 = shl %arg, %4
    %6:bool = lte %5, 16777215u
    %7:u32 = select 0u, 8u, %6
    %8:u32 = shl %5, %7
    %9:bool = lte %8, 268435455u
    %10:u32 = select 0u, 4u, %9
    %11:u32 = shl %8, %10
    %12:bool = lte %11, 1073741823u
    %13:u32 = select 0u, 2u, %12
    %14:u32 = shl %11, %13
    %15:bool = lte %14, 2147483647u
    %16:u32 = select 0u, 1u, %15
    %17:bool = eq %14, 0u
    %18:u32 = select 0u, 1u, %17
    %19:u32 = or %16, %18
    %20:u32 = or %13, %19
    %21:u32 = or %10, %20
    %22:u32 = or %7, %21
    %23:u32 = or %4, %22
    %result:u32 = add %23, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_I32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %result:i32 = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %3:u32 = bitcast %arg
    %4:bool = lte %3, 65535u
    %5:u32 = select 0u, 16u, %4
    %6:u32 = shl %3, %5
    %7:bool = lte %6, 16777215u
    %8:u32 = select 0u, 8u, %7
    %9:u32 = shl %6, %8
    %10:bool = lte %9, 268435455u
    %11:u32 = select 0u, 4u, %10
    %12:u32 = shl %9, %11
    %13:bool = lte %12, 1073741823u
    %14:u32 = select 0u, 2u, %13
    %15:u32 = shl %12, %14
    %16:bool = lte %15, 2147483647u
    %17:u32 = select 0u, 1u, %16
    %18:bool = eq %15, 0u
    %19:u32 = select 0u, 1u, %18
    %20:u32 = or %17, %19
    %21:u32 = or %14, %20
    %22:u32 = or %11, %21
    %23:u32 = or %8, %22
    %24:u32 = or %5, %23
    %25:u32 = add %24, %19
    %result:i32 = bitcast %25
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_Vec2U32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %3:vec2<bool> = lte %arg, vec2<u32>(65535u)
    %4:vec2<u32> = select vec2<u32>(0u), vec2<u32>(16u), %3
    %5:vec2<u32> = shl %arg, %4
    %6:vec2<bool> = lte %5, vec2<u32>(16777215u)
    %7:vec2<u32> = select vec2<u32>(0u), vec2<u32>(8u), %6
    %8:vec2<u32> = shl %5, %7
    %9:vec2<bool> = lte %8, vec2<u32>(268435455u)
    %10:vec2<u32> = select vec2<u32>(0u), vec2<u32>(4u), %9
    %11:vec2<u32> = shl %8, %10
    %12:vec2<bool> = lte %11, vec2<u32>(1073741823u)
    %13:vec2<u32> = select vec2<u32>(0u), vec2<u32>(2u), %12
    %14:vec2<u32> = shl %11, %13
    %15:vec2<bool> = lte %14, vec2<u32>(2147483647u)
    %16:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %15
    %17:vec2<bool> = eq %14, vec2<u32>(0u)
    %18:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %17
    %19:vec2<u32> = or %16, %18
    %20:vec2<u32> = or %13, %19
    %21:vec2<u32> = or %10, %20
    %22:vec2<u32> = or %7, %21
    %23:vec2<u32> = or %4, %22
    %result:vec2<u32> = add %23, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_Vec4I32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %result:vec4<i32> = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<u32> = bitcast %arg
    %4:vec4<bool> = lte %3, vec4<u32>(65535u)
    %5:vec4<u32> = select vec4<u32>(0u), vec4<u32>(16u), %4
    %6:vec4<u32> = shl %3, %5
    %7:vec4<bool> = lte %6, vec4<u32>(16777215u)
    %8:vec4<u32> = select vec4<u32>(0u), vec4<u32>(8u), %7
    %9:vec4<u32> = shl %6, %8
    %10:vec4<bool> = lte %9, vec4<u32>(268435455u)
    %11:vec4<u32> = select vec4<u32>(0u), vec4<u32>(4u), %10
    %12:vec4<u32> = shl %9, %11
    %13:vec4<bool> = lte %12, vec4<u32>(1073741823u)
    %14:vec4<u32> = select vec4<u32>(0u), vec4<u32>(2u), %13
    %15:vec4<u32> = shl %12, %14
    %16:vec4<bool> = lte %15, vec4<u32>(2147483647u)
    %17:vec4<u32> = select vec4<u32>(0u), vec4<u32>(1u), %16
    %18:vec4<bool> = eq %15, vec4<u32>(0u)
    %19:vec4<u32> = select vec4<u32>(0u), vec4<u32>(1u), %18
    %20:vec4<u32> = or %17, %19
    %21:vec4<u32> = or %14, %20
    %22:vec4<u32> = or %11, %21
    %23:vec4<u32> = or %8, %22
    %24:vec4<u32> = or %5, %23
    %25:vec4<u32> = add %24, %19
    %result:vec4<i32> = bitcast %25
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_NoPolyfill) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_U32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %3:u32 = and %arg, 65535u
    %4:bool = eq %3, 0u
    %5:u32 = select 0u, 16u, %4
    %6:u32 = shr %arg, %5
    %7:u32 = and %6, 255u
    %8:bool = eq %7, 0u
    %9:u32 = select 0u, 8u, %8
    %10:u32 = shr %6, %9
    %11:u32 = and %10, 15u
    %12:bool = eq %11, 0u
    %13:u32 = select 0u, 4u, %12
    %14:u32 = shr %10, %13
    %15:u32 = and %14, 3u
    %16:bool = eq %15, 0u
    %17:u32 = select 0u, 2u, %16
    %18:u32 = shr %14, %17
    %19:u32 = and %18, 1u
    %20:bool = eq %19, 0u
    %21:u32 = select 0u, 1u, %20
    %22:bool = eq %18, 0u
    %23:u32 = select 0u, 1u, %22
    %24:u32 = or %17, %21
    %25:u32 = or %13, %24
    %26:u32 = or %9, %25
    %27:u32 = or %5, %26
    %result:u32 = add %27, %23
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_I32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %result:i32 = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %3:u32 = bitcast %arg
    %4:u32 = and %3, 65535u
    %5:bool = eq %4, 0u
    %6:u32 = select 0u, 16u, %5
    %7:u32 = shr %3, %6
    %8:u32 = and %7, 255u
    %9:bool = eq %8, 0u
    %10:u32 = select 0u, 8u, %9
    %11:u32 = shr %7, %10
    %12:u32 = and %11, 15u
    %13:bool = eq %12, 0u
    %14:u32 = select 0u, 4u, %13
    %15:u32 = shr %11, %14
    %16:u32 = and %15, 3u
    %17:bool = eq %16, 0u
    %18:u32 = select 0u, 2u, %17
    %19:u32 = shr %15, %18
    %20:u32 = and %19, 1u
    %21:bool = eq %20, 0u
    %22:u32 = select 0u, 1u, %21
    %23:bool = eq %19, 0u
    %24:u32 = select 0u, 1u, %23
    %25:u32 = or %18, %22
    %26:u32 = or %14, %25
    %27:u32 = or %10, %26
    %28:u32 = or %6, %27
    %29:u32 = add %28, %24
    %result:i32 = bitcast %29
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_Vec2U32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = and %arg, vec2<u32>(65535u)
    %4:vec2<bool> = eq %3, vec2<u32>(0u)
    %5:vec2<u32> = select vec2<u32>(0u), vec2<u32>(16u), %4
    %6:vec2<u32> = shr %arg, %5
    %7:vec2<u32> = and %6, vec2<u32>(255u)
    %8:vec2<bool> = eq %7, vec2<u32>(0u)
    %9:vec2<u32> = select vec2<u32>(0u), vec2<u32>(8u), %8
    %10:vec2<u32> = shr %6, %9
    %11:vec2<u32> = and %10, vec2<u32>(15u)
    %12:vec2<bool> = eq %11, vec2<u32>(0u)
    %13:vec2<u32> = select vec2<u32>(0u), vec2<u32>(4u), %12
    %14:vec2<u32> = shr %10, %13
    %15:vec2<u32> = and %14, vec2<u32>(3u)
    %16:vec2<bool> = eq %15, vec2<u32>(0u)
    %17:vec2<u32> = select vec2<u32>(0u), vec2<u32>(2u), %16
    %18:vec2<u32> = shr %14, %17
    %19:vec2<u32> = and %18, vec2<u32>(1u)
    %20:vec2<bool> = eq %19, vec2<u32>(0u)
    %21:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %20
    %22:vec2<bool> = eq %18, vec2<u32>(0u)
    %23:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %22
    %24:vec2<u32> = or %17, %21
    %25:vec2<u32> = or %13, %24
    %26:vec2<u32> = or %9, %25
    %27:vec2<u32> = or %5, %26
    %result:vec2<u32> = add %27, %23
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_Vec4I32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %result:vec4<i32> = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %result:vec4<i32> = countTrailingZeros %arg
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Degrees_NoPolyfill) {
    Build(core::BuiltinFn::kDegrees, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = degrees %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.degrees = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Degrees_F32) {
    Build(core::BuiltinFn::kDegrees, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = degrees %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = mul %arg, 57.295780181884765625f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.degrees = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Degrees_F16) {
    Build(core::BuiltinFn::kDegrees, ty.f16(), Vector{ty.f16()});
    auto* src = R"(
%foo = func(%arg:f16):f16 {
  $B1: {
    %result:f16 = degrees %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f16):f16 {
  $B1: {
    %result:f16 = mul %arg, 57.28125h
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.degrees = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Degrees_Vec2F32) {
    Build(core::BuiltinFn::kDegrees, ty.vec2<f32>(), Vector{ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> {
  $B1: {
    %result:vec2<f32> = degrees %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> {
  $B1: {
    %result:vec2<f32> = mul %arg, 57.295780181884765625f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.degrees = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Degrees_Vec4F16) {
    Build(core::BuiltinFn::kDegrees, ty.vec4<f16>(), Vector{ty.vec4<f16>()});
    auto* src = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> {
  $B1: {
    %result:vec4<f16> = degrees %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> {
  $B1: {
    %result:vec4<f16> = mul %arg, 57.28125h
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.degrees = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_NoPolyfill) {
    Build(core::BuiltinFn::kExtractBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:u32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kNone;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_U32) {
    Build(core::BuiltinFn::kExtractBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:u32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:u32 = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_I32) {
    Build(core::BuiltinFn::kExtractBits, ty.i32(), Vector{ty.i32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:i32, %arg_1:u32, %arg_2:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:i32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32, %arg_1:u32, %arg_2:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:i32 = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_Vec2U32) {
    Build(core::BuiltinFn::kExtractBits, ty.vec2<u32>(),
          Vector{ty.vec2<u32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>, %arg_1:u32, %arg_2:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec2<u32> = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>, %arg_1:u32, %arg_2:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:vec2<u32> = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_Vec4I32) {
    Build(core::BuiltinFn::kExtractBits, ty.vec4<i32>(),
          Vector{ty.vec4<i32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>, %arg_1:u32, %arg_2:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec4<i32> = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>, %arg_1:u32, %arg_2:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:vec4<i32> = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_Full_U32) {
    Build(core::BuiltinFn::kExtractBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:u32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = add %5, %arg_2
    %7:u32 = min 32u, %6
    %8:u32 = sub 32u, %7
    %9:u32 = add %8, %5
    %10:u32 = construct %8
    %11:u32 = shl %arg, %10
    %12:bool = lt %8, 32u
    %13:u32 = select 0u, %11, %12
    %14:u32 = shr %13, 31u
    %15:u32 = shr %14, 1u
    %16:u32 = construct %9
    %17:u32 = shr %13, %16
    %18:bool = lt %9, 32u
    %result:u32 = select %15, %17, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_Full_I32) {
    Build(core::BuiltinFn::kExtractBits, ty.i32(), Vector{ty.i32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:i32, %arg_1:u32, %arg_2:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:i32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32, %arg_1:u32, %arg_2:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = add %5, %arg_2
    %7:u32 = min 32u, %6
    %8:u32 = sub 32u, %7
    %9:u32 = add %8, %5
    %10:u32 = construct %8
    %11:i32 = shl %arg, %10
    %12:bool = lt %8, 32u
    %13:i32 = select 0i, %11, %12
    %14:i32 = shr %13, 31u
    %15:i32 = shr %14, 1u
    %16:u32 = construct %9
    %17:i32 = shr %13, %16
    %18:bool = lt %9, 32u
    %result:i32 = select %15, %17, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_Full_Vec2U32) {
    Build(core::BuiltinFn::kExtractBits, ty.vec2<u32>(),
          Vector{ty.vec2<u32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>, %arg_1:u32, %arg_2:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec2<u32> = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>, %arg_1:u32, %arg_2:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = add %5, %arg_2
    %7:u32 = min 32u, %6
    %8:u32 = sub 32u, %7
    %9:u32 = add %8, %5
    %10:vec2<u32> = construct %8
    %11:vec2<u32> = shl %arg, %10
    %12:bool = lt %8, 32u
    %13:vec2<u32> = select vec2<u32>(0u), %11, %12
    %14:vec2<u32> = shr %13, vec2<u32>(31u)
    %15:vec2<u32> = shr %14, vec2<u32>(1u)
    %16:vec2<u32> = construct %9
    %17:vec2<u32> = shr %13, %16
    %18:bool = lt %9, 32u
    %result:vec2<u32> = select %15, %17, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_Full_Vec4I32) {
    Build(core::BuiltinFn::kExtractBits, ty.vec4<i32>(),
          Vector{ty.vec4<i32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>, %arg_1:u32, %arg_2:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec4<i32> = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>, %arg_1:u32, %arg_2:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:u32 = min %arg_1, 32u
    %6:u32 = add %5, %arg_2
    %7:u32 = min 32u, %6
    %8:u32 = sub 32u, %7
    %9:u32 = add %8, %5
    %10:vec4<u32> = construct %8
    %11:vec4<i32> = shl %arg, %10
    %12:bool = lt %8, 32u
    %13:vec4<i32> = select vec4<i32>(0i), %11, %12
    %14:vec4<i32> = shr %13, vec4<u32>(31u)
    %15:vec4<i32> = shr %14, vec4<u32>(1u)
    %16:vec4<u32> = construct %9
    %17:vec4<i32> = shr %13, %16
    %18:bool = lt %9, 32u
    %result:vec4<i32> = select %15, %17, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_NoPolyfill) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_U32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %3:u32 = and %arg, 4294901760u
    %4:bool = eq %3, 0u
    %5:u32 = select 16u, 0u, %4
    %6:u32 = shr %arg, %5
    %7:u32 = and %6, 65280u
    %8:bool = eq %7, 0u
    %9:u32 = select 8u, 0u, %8
    %10:u32 = shr %6, %9
    %11:u32 = and %10, 240u
    %12:bool = eq %11, 0u
    %13:u32 = select 4u, 0u, %12
    %14:u32 = shr %10, %13
    %15:u32 = and %14, 12u
    %16:bool = eq %15, 0u
    %17:u32 = select 2u, 0u, %16
    %18:u32 = shr %14, %17
    %19:u32 = and %18, 2u
    %20:bool = eq %19, 0u
    %21:u32 = select 1u, 0u, %20
    %22:u32 = or %17, %21
    %23:u32 = or %13, %22
    %24:u32 = or %9, %23
    %25:u32 = or %5, %24
    %26:bool = eq %18, 0u
    %result:u32 = select %25, 4294967295u, %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_I32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %result:i32 = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %3:u32 = bitcast %arg
    %4:u32 = complement %3
    %5:bool = lt %3, 2147483648u
    %6:u32 = select %4, %3, %5
    %7:u32 = and %6, 4294901760u
    %8:bool = eq %7, 0u
    %9:u32 = select 16u, 0u, %8
    %10:u32 = shr %6, %9
    %11:u32 = and %10, 65280u
    %12:bool = eq %11, 0u
    %13:u32 = select 8u, 0u, %12
    %14:u32 = shr %10, %13
    %15:u32 = and %14, 240u
    %16:bool = eq %15, 0u
    %17:u32 = select 4u, 0u, %16
    %18:u32 = shr %14, %17
    %19:u32 = and %18, 12u
    %20:bool = eq %19, 0u
    %21:u32 = select 2u, 0u, %20
    %22:u32 = shr %18, %21
    %23:u32 = and %22, 2u
    %24:bool = eq %23, 0u
    %25:u32 = select 1u, 0u, %24
    %26:u32 = or %21, %25
    %27:u32 = or %17, %26
    %28:u32 = or %13, %27
    %29:u32 = or %9, %28
    %30:bool = eq %22, 0u
    %31:u32 = select %29, 4294967295u, %30
    %result:i32 = bitcast %31
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_Vec2U32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = and %arg, vec2<u32>(4294901760u)
    %4:vec2<bool> = eq %3, vec2<u32>(0u)
    %5:vec2<u32> = select vec2<u32>(16u), vec2<u32>(0u), %4
    %6:vec2<u32> = shr %arg, %5
    %7:vec2<u32> = and %6, vec2<u32>(65280u)
    %8:vec2<bool> = eq %7, vec2<u32>(0u)
    %9:vec2<u32> = select vec2<u32>(8u), vec2<u32>(0u), %8
    %10:vec2<u32> = shr %6, %9
    %11:vec2<u32> = and %10, vec2<u32>(240u)
    %12:vec2<bool> = eq %11, vec2<u32>(0u)
    %13:vec2<u32> = select vec2<u32>(4u), vec2<u32>(0u), %12
    %14:vec2<u32> = shr %10, %13
    %15:vec2<u32> = and %14, vec2<u32>(12u)
    %16:vec2<bool> = eq %15, vec2<u32>(0u)
    %17:vec2<u32> = select vec2<u32>(2u), vec2<u32>(0u), %16
    %18:vec2<u32> = shr %14, %17
    %19:vec2<u32> = and %18, vec2<u32>(2u)
    %20:vec2<bool> = eq %19, vec2<u32>(0u)
    %21:vec2<u32> = select vec2<u32>(1u), vec2<u32>(0u), %20
    %22:vec2<u32> = or %17, %21
    %23:vec2<u32> = or %13, %22
    %24:vec2<u32> = or %9, %23
    %25:vec2<u32> = or %5, %24
    %26:vec2<bool> = eq %18, vec2<u32>(0u)
    %result:vec2<u32> = select %25, vec2<u32>(4294967295u), %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_Vec4I32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %result:vec4<i32> = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<u32> = bitcast %arg
    %4:vec4<u32> = complement %3
    %5:vec4<bool> = lt %3, vec4<u32>(2147483648u)
    %6:vec4<u32> = select %4, %3, %5
    %7:vec4<u32> = and %6, vec4<u32>(4294901760u)
    %8:vec4<bool> = eq %7, vec4<u32>(0u)
    %9:vec4<u32> = select vec4<u32>(16u), vec4<u32>(0u), %8
    %10:vec4<u32> = shr %6, %9
    %11:vec4<u32> = and %10, vec4<u32>(65280u)
    %12:vec4<bool> = eq %11, vec4<u32>(0u)
    %13:vec4<u32> = select vec4<u32>(8u), vec4<u32>(0u), %12
    %14:vec4<u32> = shr %10, %13
    %15:vec4<u32> = and %14, vec4<u32>(240u)
    %16:vec4<bool> = eq %15, vec4<u32>(0u)
    %17:vec4<u32> = select vec4<u32>(4u), vec4<u32>(0u), %16
    %18:vec4<u32> = shr %14, %17
    %19:vec4<u32> = and %18, vec4<u32>(12u)
    %20:vec4<bool> = eq %19, vec4<u32>(0u)
    %21:vec4<u32> = select vec4<u32>(2u), vec4<u32>(0u), %20
    %22:vec4<u32> = shr %18, %21
    %23:vec4<u32> = and %22, vec4<u32>(2u)
    %24:vec4<bool> = eq %23, vec4<u32>(0u)
    %25:vec4<u32> = select vec4<u32>(1u), vec4<u32>(0u), %24
    %26:vec4<u32> = or %21, %25
    %27:vec4<u32> = or %17, %26
    %28:vec4<u32> = or %13, %27
    %29:vec4<u32> = or %9, %28
    %30:vec4<bool> = eq %22, vec4<u32>(0u)
    %31:vec4<u32> = select %29, vec4<u32>(4294967295u), %30
    %result:vec4<i32> = bitcast %31
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_NoPolyfill) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_U32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %result:u32 = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 {
  $B1: {
    %3:u32 = and %arg, 65535u
    %4:bool = eq %3, 0u
    %5:u32 = select 0u, 16u, %4
    %6:u32 = shr %arg, %5
    %7:u32 = and %6, 255u
    %8:bool = eq %7, 0u
    %9:u32 = select 0u, 8u, %8
    %10:u32 = shr %6, %9
    %11:u32 = and %10, 15u
    %12:bool = eq %11, 0u
    %13:u32 = select 0u, 4u, %12
    %14:u32 = shr %10, %13
    %15:u32 = and %14, 3u
    %16:bool = eq %15, 0u
    %17:u32 = select 0u, 2u, %16
    %18:u32 = shr %14, %17
    %19:u32 = and %18, 1u
    %20:bool = eq %19, 0u
    %21:u32 = select 0u, 1u, %20
    %22:u32 = or %17, %21
    %23:u32 = or %13, %22
    %24:u32 = or %9, %23
    %25:u32 = or %5, %24
    %26:bool = eq %18, 0u
    %result:u32 = select %25, 4294967295u, %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_I32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %result:i32 = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 {
  $B1: {
    %3:u32 = bitcast %arg
    %4:u32 = and %3, 65535u
    %5:bool = eq %4, 0u
    %6:u32 = select 0u, 16u, %5
    %7:u32 = shr %3, %6
    %8:u32 = and %7, 255u
    %9:bool = eq %8, 0u
    %10:u32 = select 0u, 8u, %9
    %11:u32 = shr %7, %10
    %12:u32 = and %11, 15u
    %13:bool = eq %12, 0u
    %14:u32 = select 0u, 4u, %13
    %15:u32 = shr %11, %14
    %16:u32 = and %15, 3u
    %17:bool = eq %16, 0u
    %18:u32 = select 0u, 2u, %17
    %19:u32 = shr %15, %18
    %20:u32 = and %19, 1u
    %21:bool = eq %20, 0u
    %22:u32 = select 0u, 1u, %21
    %23:u32 = or %18, %22
    %24:u32 = or %14, %23
    %25:u32 = or %10, %24
    %26:u32 = or %6, %25
    %27:bool = eq %19, 0u
    %28:u32 = select %26, 4294967295u, %27
    %result:i32 = bitcast %28
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_Vec2U32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = and %arg, vec2<u32>(65535u)
    %4:vec2<bool> = eq %3, vec2<u32>(0u)
    %5:vec2<u32> = select vec2<u32>(0u), vec2<u32>(16u), %4
    %6:vec2<u32> = shr %arg, %5
    %7:vec2<u32> = and %6, vec2<u32>(255u)
    %8:vec2<bool> = eq %7, vec2<u32>(0u)
    %9:vec2<u32> = select vec2<u32>(0u), vec2<u32>(8u), %8
    %10:vec2<u32> = shr %6, %9
    %11:vec2<u32> = and %10, vec2<u32>(15u)
    %12:vec2<bool> = eq %11, vec2<u32>(0u)
    %13:vec2<u32> = select vec2<u32>(0u), vec2<u32>(4u), %12
    %14:vec2<u32> = shr %10, %13
    %15:vec2<u32> = and %14, vec2<u32>(3u)
    %16:vec2<bool> = eq %15, vec2<u32>(0u)
    %17:vec2<u32> = select vec2<u32>(0u), vec2<u32>(2u), %16
    %18:vec2<u32> = shr %14, %17
    %19:vec2<u32> = and %18, vec2<u32>(1u)
    %20:vec2<bool> = eq %19, vec2<u32>(0u)
    %21:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %20
    %22:vec2<u32> = or %17, %21
    %23:vec2<u32> = or %13, %22
    %24:vec2<u32> = or %9, %23
    %25:vec2<u32> = or %5, %24
    %26:vec2<bool> = eq %18, vec2<u32>(0u)
    %result:vec2<u32> = select %25, vec2<u32>(4294967295u), %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_Vec4I32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %result:vec4<i32> = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> {
  $B1: {
    %3:vec4<u32> = bitcast %arg
    %4:vec4<u32> = and %3, vec4<u32>(65535u)
    %5:vec4<bool> = eq %4, vec4<u32>(0u)
    %6:vec4<u32> = select vec4<u32>(0u), vec4<u32>(16u), %5
    %7:vec4<u32> = shr %3, %6
    %8:vec4<u32> = and %7, vec4<u32>(255u)
    %9:vec4<bool> = eq %8, vec4<u32>(0u)
    %10:vec4<u32> = select vec4<u32>(0u), vec4<u32>(8u), %9
    %11:vec4<u32> = shr %7, %10
    %12:vec4<u32> = and %11, vec4<u32>(15u)
    %13:vec4<bool> = eq %12, vec4<u32>(0u)
    %14:vec4<u32> = select vec4<u32>(0u), vec4<u32>(4u), %13
    %15:vec4<u32> = shr %11, %14
    %16:vec4<u32> = and %15, vec4<u32>(3u)
    %17:vec4<bool> = eq %16, vec4<u32>(0u)
    %18:vec4<u32> = select vec4<u32>(0u), vec4<u32>(2u), %17
    %19:vec4<u32> = shr %15, %18
    %20:vec4<u32> = and %19, vec4<u32>(1u)
    %21:vec4<bool> = eq %20, vec4<u32>(0u)
    %22:vec4<u32> = select vec4<u32>(0u), vec4<u32>(1u), %21
    %23:vec4<u32> = or %18, %22
    %24:vec4<u32> = or %14, %23
    %25:vec4<u32> = or %10, %24
    %26:vec4<u32> = or %6, %25
    %27:vec4<bool> = eq %19, vec4<u32>(0u)
    %28:vec4<u32> = select %26, vec4<u32>(4294967295u), %27
    %result:vec4<i32> = bitcast %28
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FwidthFine_NoPolyfill) {
    Build(core::BuiltinFn::kFwidthFine, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = fwidthFine %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.fwidth_fine = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_F32) {
    Build(core::BuiltinFn::kFwidthFine, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = fwidthFine %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:f32 = dpdxFine %arg
    %4:f32 = dpdyFine %arg
    %5:f32 = abs %3
    %6:f32 = abs %4
    %7:f32 = add %5, %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.fwidth_fine = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_Vector) {
    Build(core::BuiltinFn::kFwidthFine, ty.vec4<f32>(), Vector{ty.vec4<f32>()});
    auto* src = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %result:vec4<f32> = fwidthFine %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = dpdxFine %arg
    %4:vec4<f32> = dpdyFine %arg
    %5:vec4<f32> = abs %3
    %6:vec4<f32> = abs %4
    %7:vec4<f32> = add %5, %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.fwidth_fine = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_NoPolyfill) {
    Build(core::BuiltinFn::kInsertBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:u32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kNone;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_U32) {
    Build(core::BuiltinFn::kInsertBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:u32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:u32 = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_I32) {
    Build(core::BuiltinFn::kInsertBits, ty.i32(), Vector{ty.i32(), ty.i32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:i32, %arg_1:i32, %arg_2:u32, %arg_3:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:i32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32, %arg_1:i32, %arg_2:u32, %arg_3:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:i32 = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_Vec2U32) {
    Build(core::BuiltinFn::kInsertBits, ty.vec2<u32>(),
          Vector{ty.vec2<u32>(), ty.vec2<u32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>, %arg_1:vec2<u32>, %arg_2:u32, %arg_3:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:vec2<u32> = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>, %arg_1:vec2<u32>, %arg_2:u32, %arg_3:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:vec2<u32> = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_Vec4I32) {
    Build(core::BuiltinFn::kInsertBits, ty.vec4<i32>(),
          Vector{ty.vec4<i32>(), ty.vec4<i32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>, %arg_1:vec4<i32>, %arg_2:u32, %arg_3:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:vec4<i32> = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>, %arg_1:vec4<i32>, %arg_2:u32, %arg_3:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:vec4<i32> = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_Full_U32) {
    Build(core::BuiltinFn::kInsertBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:u32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = add %arg_2, %arg_3
    %7:u32 = shl 1u, %arg_2
    %8:bool = lt %arg_2, 32u
    %9:u32 = select 0u, %7, %8
    %10:u32 = shl 1u, %6
    %11:bool = lt %6, 32u
    %12:u32 = select 0u, %10, %11
    %13:u32 = sub %9, 1u
    %14:u32 = sub %12, 1u
    %15:u32 = xor %13, %14
    %16:u32 = construct %arg_2
    %17:u32 = shl %arg_1, %16
    %18:bool = lt %arg_2, 32u
    %19:u32 = select 0u, %17, %18
    %20:u32 = construct %15
    %21:u32 = and %19, %20
    %22:u32 = complement %15
    %23:u32 = construct %22
    %24:u32 = and %arg, %23
    %result:u32 = or %21, %24
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_Full_I32) {
    Build(core::BuiltinFn::kInsertBits, ty.i32(), Vector{ty.i32(), ty.i32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:i32, %arg_1:i32, %arg_2:u32, %arg_3:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:i32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32, %arg_1:i32, %arg_2:u32, %arg_3:u32):i32 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = add %arg_2, %arg_3
    %7:u32 = shl 1u, %arg_2
    %8:bool = lt %arg_2, 32u
    %9:u32 = select 0u, %7, %8
    %10:u32 = shl 1u, %6
    %11:bool = lt %6, 32u
    %12:u32 = select 0u, %10, %11
    %13:u32 = sub %9, 1u
    %14:u32 = sub %12, 1u
    %15:u32 = xor %13, %14
    %16:u32 = construct %arg_2
    %17:i32 = shl %arg_1, %16
    %18:bool = lt %arg_2, 32u
    %19:i32 = select 0i, %17, %18
    %20:i32 = construct %15
    %21:i32 = and %19, %20
    %22:u32 = complement %15
    %23:i32 = construct %22
    %24:i32 = and %arg, %23
    %result:i32 = or %21, %24
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_Full_Vec2U32) {
    Build(core::BuiltinFn::kInsertBits, ty.vec2<u32>(),
          Vector{ty.vec2<u32>(), ty.vec2<u32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>, %arg_1:vec2<u32>, %arg_2:u32, %arg_3:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:vec2<u32> = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>, %arg_1:vec2<u32>, %arg_2:u32, %arg_3:u32):vec2<u32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = add %arg_2, %arg_3
    %7:u32 = shl 1u, %arg_2
    %8:bool = lt %arg_2, 32u
    %9:u32 = select 0u, %7, %8
    %10:u32 = shl 1u, %6
    %11:bool = lt %6, 32u
    %12:u32 = select 0u, %10, %11
    %13:u32 = sub %9, 1u
    %14:u32 = sub %12, 1u
    %15:u32 = xor %13, %14
    %16:vec2<u32> = construct %arg_2
    %17:vec2<u32> = shl %arg_1, %16
    %18:bool = lt %arg_2, 32u
    %19:vec2<u32> = select vec2<u32>(0u), %17, %18
    %20:vec2<u32> = construct %15
    %21:vec2<u32> = and %19, %20
    %22:u32 = complement %15
    %23:vec2<u32> = construct %22
    %24:vec2<u32> = and %arg, %23
    %result:vec2<u32> = or %21, %24
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_Full_Vec4I32) {
    Build(core::BuiltinFn::kInsertBits, ty.vec4<i32>(),
          Vector{ty.vec4<i32>(), ty.vec4<i32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>, %arg_1:vec4<i32>, %arg_2:u32, %arg_3:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:vec4<i32> = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>, %arg_1:vec4<i32>, %arg_2:u32, %arg_3:u32):vec4<i32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:u32 = add %arg_2, %arg_3
    %7:u32 = shl 1u, %arg_2
    %8:bool = lt %arg_2, 32u
    %9:u32 = select 0u, %7, %8
    %10:u32 = shl 1u, %6
    %11:bool = lt %6, 32u
    %12:u32 = select 0u, %10, %11
    %13:u32 = sub %9, 1u
    %14:u32 = sub %12, 1u
    %15:u32 = xor %13, %14
    %16:vec4<u32> = construct %arg_2
    %17:vec4<i32> = shl %arg_1, %16
    %18:bool = lt %arg_2, 32u
    %19:vec4<i32> = select vec4<i32>(0i), %17, %18
    %20:vec4<i32> = construct %15
    %21:vec4<i32> = and %19, %20
    %22:u32 = complement %15
    %23:vec4<i32> = construct %22
    %24:vec4<i32> = and %arg, %23
    %result:vec4<i32> = or %21, %24
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kFull;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, TextureSampleBaseClampToEdge_2d_f32_NoPolyfill) {
    auto* texture_ty =
        ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    Build(core::BuiltinFn::kTextureSampleBaseClampToEdge, ty.vec4<f32>(),
          Vector{texture_ty, ty.sampler(), ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>):vec4<f32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec4<f32> = textureSampleBaseClampToEdge %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.texture_sample_base_clamp_to_edge_2d_f32 = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Radians_NoPolyfill) {
    Build(core::BuiltinFn::kRadians, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = radians %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.radians = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Radians_F32) {
    Build(core::BuiltinFn::kRadians, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = radians %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %result:f32 = mul %arg, 0.01745329238474369049f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.radians = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Radians_F16) {
    Build(core::BuiltinFn::kRadians, ty.f16(), Vector{ty.f16()});
    auto* src = R"(
%foo = func(%arg:f16):f16 {
  $B1: {
    %result:f16 = radians %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f16):f16 {
  $B1: {
    %result:f16 = mul %arg, 0.0174407958984375h
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.radians = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Radians_Vec2F32) {
    Build(core::BuiltinFn::kRadians, ty.vec2<f32>(), Vector{ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> {
  $B1: {
    %result:vec2<f32> = radians %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> {
  $B1: {
    %result:vec2<f32> = mul %arg, 0.01745329238474369049f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.radians = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Radians_Vec4F16) {
    Build(core::BuiltinFn::kRadians, ty.vec4<f16>(), Vector{ty.vec4<f16>()});
    auto* src = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> {
  $B1: {
    %result:vec4<f16> = radians %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> {
  $B1: {
    %result:vec4<f16> = mul %arg, 0.0174407958984375h
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.radians = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, TextureSampleBaseClampToEdge_2d_f32) {
    auto* texture_ty =
        ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    Build(core::BuiltinFn::kTextureSampleBaseClampToEdge, ty.vec4<f32>(),
          Vector{texture_ty, ty.sampler(), ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>):vec4<f32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %result:vec4<f32> = textureSampleBaseClampToEdge %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>):vec4<f32> {  # %arg_1: 'arg', %arg_2: 'arg'
  $B1: {
    %5:vec2<u32> = textureDimensions %arg
    %6:vec2<f32> = convert %5
    %7:vec2<f32> = div vec2<f32>(0.5f), %6
    %8:vec2<f32> = sub vec2<f32>(1.0f), %7
    %9:vec2<f32> = clamp %arg_2, %7, %8
    %result:vec4<f32> = textureSampleLevel %arg, %arg_1, %9, 0.0f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.texture_sample_base_clamp_to_edge_2d_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, TextureSampleBiasClampNonArray) {
    auto* texture_ty =
        ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    Build(core::BuiltinFn::kTextureSampleBias, ty.vec4<f32>(),
          Vector{texture_ty, ty.sampler(), ty.vec2<f32>(), ty.f32()});

    auto* src = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>, %arg_3:f32):vec4<f32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %result:vec4<f32> = textureSampleBias %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>, %arg_3:f32):vec4<f32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  $B1: {
    %6:f32 = clamp %arg_3, -16.0f, 15.9899997711181640625f
    %result:vec4<f32> = textureSampleBias %arg, %arg_1, %arg_2, %6
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.texture_sample_base_clamp_to_edge_2d_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, TextureSampleBiasClampWithArray) {
    auto* texture_ty =
        ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, ty.f32());
    Build(core::BuiltinFn::kTextureSampleBias, ty.vec4<f32>(),
          Vector{texture_ty, ty.sampler(), ty.vec2<f32>(), ty.i32(), ty.f32()});

    auto* src = R"(
%foo = func(%arg:texture_2d_array<f32>, %arg_1:sampler, %arg_2:vec2<f32>, %arg_3:i32, %arg_4:f32):vec4<f32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg', %arg_4: 'arg'
  $B1: {
    %result:vec4<f32> = textureSampleBias %arg, %arg_1, %arg_2, %arg_3, %arg_4
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:texture_2d_array<f32>, %arg_1:sampler, %arg_2:vec2<f32>, %arg_3:i32, %arg_4:f32):vec4<f32> {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg', %arg_4: 'arg'
  $B1: {
    %7:f32 = clamp %arg_4, -16.0f, 15.9899997711181640625f
    %result:vec4<f32> = textureSampleBias %arg, %arg_1, %arg_2, %arg_3, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.texture_sample_base_clamp_to_edge_2d_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Pack4xI8) {
    Build(core::BuiltinFn::kPack4XI8, ty.u32(), Vector{ty.vec4<i32>()});

    auto* src = R"(
%foo = func(%arg:vec4<i32>):u32 {
  $B1: {
    %result:u32 = pack4xI8 %arg
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:vec4<i32>):u32 {
  $B1: {
    %3:vec4<u32> = construct 0u, 8u, 16u, 24u
    %4:vec4<u32> = bitcast %arg
    %5:vec4<u32> = construct 255u
    %6:vec4<u32> = and %4, %5
    %7:vec4<u32> = shl %6, %3
    %8:vec4<u32> = construct 1u
    %result:u32 = dot %7, %8
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8 = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Pack4xU8) {
    Build(core::BuiltinFn::kPack4XU8, ty.u32(), Vector{ty.vec4<u32>()});

    auto* src = R"(
%foo = func(%arg:vec4<u32>):u32 {
  $B1: {
    %result:u32 = pack4xU8 %arg
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:vec4<u32>):u32 {
  $B1: {
    %3:vec4<u32> = construct 0u, 8u, 16u, 24u
    %4:vec4<u32> = construct 255u
    %5:vec4<u32> = and %arg, %4
    %6:vec4<u32> = shl %5, %3
    %7:vec4<u32> = construct 1u
    %result:u32 = dot %6, %7
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8 = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Pack4xI8Clamp) {
    Build(core::BuiltinFn::kPack4XI8Clamp, ty.u32(), Vector{ty.vec4<i32>()});

    auto* src = R"(
%foo = func(%arg:vec4<i32>):u32 {
  $B1: {
    %result:u32 = pack4xI8Clamp %arg
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:vec4<i32>):u32 {
  $B1: {
    %3:vec4<u32> = construct 0u, 8u, 16u, 24u
    %4:vec4<i32> = construct -128i
    %5:vec4<i32> = construct 127i
    %6:vec4<i32> = clamp %arg, %4, %5
    %7:vec4<u32> = bitcast %6
    %8:vec4<u32> = construct 255u
    %9:vec4<u32> = and %7, %8
    %10:vec4<u32> = shl %9, %3
    %11:vec4<u32> = construct 1u
    %result:u32 = dot %10, %11
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8 = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Pack4xU8Clamp) {
    Build(core::BuiltinFn::kPack4XU8Clamp, ty.u32(), Vector{ty.vec4<u32>()});

    auto* src = R"(
%foo = func(%arg:vec4<u32>):u32 {
  $B1: {
    %result:u32 = pack4xU8Clamp %arg
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:vec4<u32>):u32 {
  $B1: {
    %3:vec4<u32> = construct 0u, 8u, 16u, 24u
    %4:vec4<u32> = construct 0u
    %5:vec4<u32> = construct 255u
    %6:vec4<u32> = clamp %arg, %4, %5
    %7:vec4<u32> = shl %6, %3
    %8:vec4<u32> = construct 1u
    %result:u32 = dot %7, %8
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.pack_4xu8_clamp = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Unpack4xI8) {
    Build(core::BuiltinFn::kUnpack4XI8, ty.vec4<i32>(), Vector{ty.u32()});

    auto* src = R"(
%foo = func(%arg:u32):vec4<i32> {
  $B1: {
    %result:vec4<i32> = unpack4xI8 %arg
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:u32):vec4<i32> {
  $B1: {
    %3:vec4<u32> = construct 24u, 16u, 8u, 0u
    %4:vec4<u32> = construct %arg
    %5:vec4<u32> = shl %4, %3
    %6:vec4<i32> = bitcast %5
    %7:vec4<u32> = construct 24u
    %result:vec4<i32> = shr %6, %7
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8 = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Unpack4xU8) {
    Build(core::BuiltinFn::kUnpack4XU8, ty.vec4<u32>(), Vector{ty.u32()});

    auto* src = R"(
%foo = func(%arg:u32):vec4<u32> {
  $B1: {
    %result:vec4<u32> = unpack4xU8 %arg
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:u32):vec4<u32> {
  $B1: {
    %3:vec4<u32> = construct 0u, 8u, 16u, 24u
    %4:vec4<u32> = construct %arg
    %5:vec4<u32> = shr %4, %3
    %6:vec4<u32> = construct 255u
    %result:vec4<u32> = and %5, %6
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8 = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Dot4I8Packed) {
    Build(core::BuiltinFn::kDot4I8Packed, ty.i32(), Vector{ty.u32(), ty.u32()});

    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32):i32 {  # %arg_1: 'arg'
  $B1: {
    %result:i32 = dot4I8Packed %arg, %arg_1
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32):i32 {  # %arg_1: 'arg'
  $B1: {
    %4:vec4<u32> = construct 24u, 16u, 8u, 0u
    %5:vec4<u32> = construct %arg
    %6:vec4<u32> = shl %5, %4
    %7:vec4<i32> = bitcast %6
    %8:vec4<u32> = construct 24u
    %9:vec4<i32> = shr %7, %8
    %10:vec4<u32> = construct 24u, 16u, 8u, 0u
    %11:vec4<u32> = construct %arg_1
    %12:vec4<u32> = shl %11, %10
    %13:vec4<i32> = bitcast %12
    %14:vec4<u32> = construct 24u
    %15:vec4<i32> = shr %13, %14
    %result:i32 = dot %9, %15
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.dot_4x8_packed = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Dot4U8Packed) {
    Build(core::BuiltinFn::kDot4U8Packed, ty.u32(), Vector{ty.u32(), ty.u32()});

    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32):u32 {  # %arg_1: 'arg'
  $B1: {
    %result:u32 = dot4U8Packed %arg, %arg_1
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32):u32 {  # %arg_1: 'arg'
  $B1: {
    %4:vec4<u32> = construct 0u, 8u, 16u, 24u
    %5:vec4<u32> = construct %arg
    %6:vec4<u32> = shr %5, %4
    %7:vec4<u32> = construct 255u
    %8:vec4<u32> = and %6, %7
    %9:vec4<u32> = construct 0u, 8u, 16u, 24u
    %10:vec4<u32> = construct %arg_1
    %11:vec4<u32> = shr %10, %9
    %12:vec4<u32> = construct 255u
    %13:vec4<u32> = and %11, %12
    %result:u32 = dot %8, %13
    ret %result
  }
}
)";

    BuiltinPolyfillConfig config;
    config.dot_4x8_packed = true;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Reflect_NoPolyfill) {
    Build(core::BuiltinFn::kReflect, ty.vec2<f32>(), Vector{ty.vec2<f32>(), ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:vec2<f32>, %arg_1:vec2<f32>):vec2<f32> {  # %arg_1: 'arg'
  $B1: {
    %result:vec2<f32> = reflect %arg, %arg_1
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.reflect_vec2_f32 = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

// Reflect polyfill (reflect_vec2_f32) should only affect vec2<f32>
TEST_F(IR_BuiltinPolyfillTest, Reflect_Vec2F32) {
    Build(core::BuiltinFn::kReflect, ty.vec2<f32>(), Vector{ty.vec2<f32>(), ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:vec2<f32>, %arg_1:vec2<f32>):vec2<f32> {  # %arg_1: 'arg'
  $B1: {
    %result:vec2<f32> = reflect %arg, %arg_1
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<f32>, %arg_1:vec2<f32>):vec2<f32> {  # %arg_1: 'arg'
  $B1: {
    %4:f32 = dot %arg, %arg_1
    %5:f32 = mul -2.0f, %4
    %6:vec2<f32> = construct %5
    %7:vec2<f32> = mul %6, %arg_1
    %result:vec2<f32> = add %arg, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.reflect_vec2_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Reflect_Vec3F32) {
    Build(core::BuiltinFn::kReflect, ty.vec3<f32>(), Vector{ty.vec3<f32>(), ty.vec3<f32>()});
    auto* src = R"(
%foo = func(%arg:vec3<f32>, %arg_1:vec3<f32>):vec3<f32> {  # %arg_1: 'arg'
  $B1: {
    %result:vec3<f32> = reflect %arg, %arg_1
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.reflect_vec2_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Reflect_Vec4F32) {
    Build(core::BuiltinFn::kReflect, ty.vec4<f32>(), Vector{ty.vec4<f32>(), ty.vec4<f32>()});
    auto* src = R"(
%foo = func(%arg:vec4<f32>, %arg_1:vec4<f32>):vec4<f32> {  # %arg_1: 'arg'
  $B1: {
    %result:vec4<f32> = reflect %arg, %arg_1
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.reflect_vec2_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Reflect_Vec2F16) {
    Build(core::BuiltinFn::kReflect, ty.vec2<f16>(), Vector{ty.vec2<f16>(), ty.vec2<f16>()});
    auto* src = R"(
%foo = func(%arg:vec2<f16>, %arg_1:vec2<f16>):vec2<f16> {  # %arg_1: 'arg'
  $B1: {
    %result:vec2<f16> = reflect %arg, %arg_1
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.reflect_vec2_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Reflect_Vec3F16) {
    Build(core::BuiltinFn::kReflect, ty.vec3<f16>(), Vector{ty.vec3<f16>(), ty.vec3<f16>()});
    auto* src = R"(
%foo = func(%arg:vec3<f16>, %arg_1:vec3<f16>):vec3<f16> {  # %arg_1: 'arg'
  $B1: {
    %result:vec3<f16> = reflect %arg, %arg_1
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.reflect_vec2_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Reflect_Vec4F16) {
    Build(core::BuiltinFn::kReflect, ty.vec4<f16>(), Vector{ty.vec4<f16>(), ty.vec4<f16>()});
    auto* src = R"(
%foo = func(%arg:vec4<f16>, %arg_1:vec4<f16>):vec4<f16> {  # %arg_1: 'arg'
  $B1: {
    %result:vec4<f16> = reflect %arg, %arg_1
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.reflect_vec2_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Pack4x8snorm) {
    Build(core::BuiltinFn::kPack4X8Snorm, ty.u32(), Vector{ty.vec4<f32>()});
    auto* src = R"(
%foo = func(%arg:vec4<f32>):u32 {
  $B1: {
    %result:u32 = pack4x8snorm %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f32>):u32 {
  $B1: {
    %3:vec4<f32> = clamp %arg, vec4<f32>(-1.0f), vec4<f32>(1.0f)
    %4:vec4<f32> = mul vec4<f32>(127.0f), %3
    %5:vec4<f32> = add vec4<f32>(0.5f), %4
    %6:vec4<f32> = floor %5
    %7:vec4<i32> = convert %6
    %8:vec4<u32> = bitcast %7
    %9:vec4<u32> = and %8, vec4<u32>(255u)
    %10:vec4<u32> = construct 0u, 8u, 16u, 24u
    %11:vec4<u32> = shl %9, %10
    %12:u32 = access %11, 0u
    %13:u32 = access %11, 1u
    %14:u32 = access %11, 2u
    %15:u32 = access %11, 3u
    %16:u32 = or %14, %15
    %17:u32 = or %13, %16
    %18:u32 = or %12, %17
    ret %18
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8_norm = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Pack4x8unorm) {
    Build(core::BuiltinFn::kPack4X8Unorm, ty.u32(), Vector{ty.vec4<f32>()});
    auto* src = R"(
%foo = func(%arg:vec4<f32>):u32 {
  $B1: {
    %result:u32 = pack4x8unorm %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f32>):u32 {
  $B1: {
    %3:vec4<f32> = clamp %arg, vec4<f32>(0.0f), vec4<f32>(1.0f)
    %4:vec4<f32> = mul vec4<f32>(255.0f), %3
    %5:vec4<f32> = add vec4<f32>(0.5f), %4
    %6:vec4<f32> = floor %5
    %7:vec4<u32> = convert %6
    %8:vec4<u32> = and %7, vec4<u32>(255u)
    %9:vec4<u32> = construct 0u, 8u, 16u, 24u
    %10:vec4<u32> = shl %8, %9
    %11:u32 = access %10, 0u
    %12:u32 = access %10, 1u
    %13:u32 = access %10, 2u
    %14:u32 = access %10, 3u
    %15:u32 = or %13, %14
    %16:u32 = or %12, %15
    %17:u32 = or %11, %16
    ret %17
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8_norm = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Unpack4x8snorm) {
    Build(core::BuiltinFn::kUnpack4X8Snorm, ty.vec4<f32>(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):vec4<f32> {
  $B1: {
    %result:vec4<f32> = unpack4x8snorm %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):vec4<f32> {
  $B1: {
    %3:vec4<u32> = construct %arg
    %4:vec4<u32> = construct 24u, 16u, 8u, 0u
    %5:vec4<u32> = shl %3, %4
    %6:vec4<i32> = bitcast %5
    %7:vec4<i32> = shr %6, vec4<u32>(24u)
    %8:vec4<f32> = convert %7
    %9:vec4<f32> = div %8, vec4<f32>(127.0f)
    %10:vec4<f32> = max %9, vec4<f32>(-1.0f)
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8_norm = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Unpack4x8unorm) {
    Build(core::BuiltinFn::kUnpack4X8Unorm, ty.vec4<f32>(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):vec4<f32> {
  $B1: {
    %result:vec4<f32> = unpack4x8unorm %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):vec4<f32> {
  $B1: {
    %3:vec4<u32> = construct %arg
    %4:vec4<u32> = construct 0u, 8u, 16u, 24u
    %5:vec4<u32> = shr %3, %4
    %6:vec4<u32> = and %5, vec4<u32>(255u)
    %7:vec4<f32> = convert %6
    %8:vec4<f32> = div %7, vec4<f32>(255.0f)
    ret %8
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.pack_unpack_4x8_norm = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
