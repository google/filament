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

#include "src/tint/lang/glsl/writer/raise/bitcast_polyfill.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_BitcastPolyfillTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_BitcastPolyfillTest, FloatToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:f32 = bitcast %a
    %x:f32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %x:f32 = let %a
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, IntToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:f32 = bitcast %a
    %x:f32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:f32 = glsl.intBitsToFloat %a
    %x:f32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, UintToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:f32 = bitcast %a
    %x:f32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:f32 = glsl.uintBitsToFloat %a
    %x:f32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec3UintToVec3Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Splat<vec3<u32>>(1_u));
        b.Let("x", b.Bitcast<vec3<f32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec3<u32> = let vec3<u32>(1u)
    %3:vec3<f32> = bitcast %a
    %x:vec3<f32> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec3<u32> = let vec3<u32>(1u)
    %3:vec3<f32> = glsl.uintBitsToFloat %a
    %x:vec3<f32> = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, FloatToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:i32 = bitcast %a
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:i32 = glsl.floatBitsToInt %a
    %x:i32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, FloatToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:u32 = bitcast %a
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:u32 = glsl.floatBitsToUint %a
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, UintToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:i32 = bitcast %a
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:i32 = convert %a
    %x:i32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, IntToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:u32 = bitcast %a
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:u32 = convert %a
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, I32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:vec2<f16> = bitcast %a
    %x:vec2<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:i32):vec2<f16> {
  $B2: {
    %7:u32 = convert %src
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    ret %8
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F16ToI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:i32 = bitcast %a
    %x:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:i32 = call %tint_bitcast_from_f16, %a
    %x:i32 = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):i32 {
  $B2: {
    %8:u32 = glsl.packFloat2x16 %src
    %9:i32 = convert %8
    ret %9
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, U32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:vec2<f16> = bitcast %a
    %x:vec2<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:u32):vec2<f16> {
  $B2: {
    %7:vec2<f16> = glsl.unpackFloat2x16 %src
    ret %7
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F16ToU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:u32 = bitcast %a
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:u32 = call %tint_bitcast_from_f16, %a
    %x:u32 = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):u32 {
  $B2: {
    %8:u32 = glsl.packFloat2x16 %src
    ret %8
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, F32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:vec2<f16> = bitcast %a
    %x:vec2<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:f32):vec2<f16> {
  $B2: {
    %7:u32 = glsl.floatBitsToUint %src
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    ret %8
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F16ToF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:f32 = bitcast %a
    %x:f32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:f32 = call %tint_bitcast_from_f16, %a
    %x:f32 = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):f32 {
  $B2: {
    %8:u32 = glsl.packFloat2x16 %src
    %9:f32 = glsl.uintBitsToFloat %8
    ret %9
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2I32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:vec2<i32> = let %2
    %4:vec4<f16> = bitcast %a
    %x:vec4<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:vec2<i32> = let %2
    %4:vec4<f16> = call %tint_bitcast_to_f16, %a
    %x:vec4<f16> = let %4
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<i32>):vec4<f16> {
  $B2: {
    %8:vec2<u32> = convert %src
    %9:u32 = swizzle %8, x
    %10:vec2<f16> = glsl.unpackFloat2x16 %9
    %11:u32 = swizzle %8, y
    %12:vec2<f16> = glsl.unpackFloat2x16 %11
    %13:vec4<f16> = construct %10, %12
    ret %13
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec4F16ToVec2I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<i32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<i32> = bitcast %a
    %x:vec2<i32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<i32> = call %tint_bitcast_from_f16, %a
    %x:vec2<i32> = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec4<f16>):vec2<i32> {
  $B2: {
    %8:vec2<f16> = swizzle %src, xy
    %9:u32 = glsl.packFloat2x16 %8
    %10:vec2<f16> = swizzle %src, zw
    %11:u32 = glsl.packFloat2x16 %10
    %12:vec2<u32> = construct %9, %11
    %13:vec2<i32> = convert %12
    ret %13
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2U32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<u32>>(1_u, 2_u));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<u32> = construct 1u, 2u
    %a:vec2<u32> = let %2
    %4:vec4<f16> = bitcast %a
    %x:vec4<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<u32> = construct 1u, 2u
    %a:vec2<u32> = let %2
    %4:vec4<f16> = call %tint_bitcast_to_f16, %a
    %x:vec4<f16> = let %4
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<u32>):vec4<f16> {
  $B2: {
    %8:u32 = swizzle %src, x
    %9:vec2<f16> = glsl.unpackFloat2x16 %8
    %10:u32 = swizzle %src, y
    %11:vec2<f16> = glsl.unpackFloat2x16 %10
    %12:vec4<f16> = construct %9, %11
    ret %12
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec4F16ToVec2U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<u32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<u32> = bitcast %a
    %x:vec2<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<u32> = call %tint_bitcast_from_f16, %a
    %x:vec2<u32> = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec4<f16>):vec2<u32> {
  $B2: {
    %8:vec2<f16> = swizzle %src, xy
    %9:u32 = glsl.packFloat2x16 %8
    %10:vec2<f16> = swizzle %src, zw
    %11:u32 = glsl.packFloat2x16 %10
    %12:vec2<u32> = construct %9, %11
    ret %12
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f32> = construct 1.0f, 2.0f
    %a:vec2<f32> = let %2
    %4:vec4<f16> = bitcast %a
    %x:vec4<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f32> = construct 1.0f, 2.0f
    %a:vec2<f32> = let %2
    %4:vec4<f16> = call %tint_bitcast_to_f16, %a
    %x:vec4<f16> = let %4
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<f32>):vec4<f16> {
  $B2: {
    %8:vec2<u32> = glsl.floatBitsToUint %src
    %9:u32 = swizzle %8, x
    %10:vec2<f16> = glsl.unpackFloat2x16 %9
    %11:u32 = swizzle %8, y
    %12:vec2<f16> = glsl.unpackFloat2x16 %11
    %13:vec4<f16> = construct %10, %12
    ret %13
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec4F16ToVec2F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<f32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<f32> = bitcast %a
    %x:vec2<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<f32> = call %tint_bitcast_from_f16, %a
    %x:vec2<f32> = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec4<f16>):vec2<f32> {
  $B2: {
    %8:vec2<f16> = swizzle %src, xy
    %9:u32 = glsl.packFloat2x16 %8
    %10:vec2<f16> = swizzle %src, zw
    %11:u32 = glsl.packFloat2x16 %10
    %12:vec2<u32> = construct %9, %11
    %13:vec2<f32> = glsl.uintBitsToFloat %12
    ret %13
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
