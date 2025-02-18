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

#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/binary.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_BinaryPolyfillTest : public TransformTest {
  protected:
    /// Helper to build a function that executes a binary instruction.
    /// @param op the binary operation
    /// @param result_ty the result type of the builtin call
    /// @param lhs_ty the type of the LHS
    /// @param rhs_ty the type of the RHS
    void Build(BinaryOp op,
               const core::type::Type* result_ty,
               const core::type::Type* lhs_ty,
               const core::type::Type* rhs_ty) {
        Vector<FunctionParam*, 4> args;
        args.Push(b.FunctionParam("lhs", lhs_ty));
        args.Push(b.FunctionParam("rhs", rhs_ty));
        auto* func = b.Function("foo", result_ty);
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            auto* result = b.Binary(op, result_ty, args[0], args[1]);
            b.Return(func, result);
            mod.SetName(result, "result");
        });
    }
};

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_NoPolyfill) {
    Build(BinaryOp::kShiftLeft, ty.i32(), ty.i32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %result:i32 = shl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = false;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_NoPolyfill) {
    Build(BinaryOp::kShiftRight, ty.i32(), ty.i32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %result:i32 = shr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = false;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_I32) {
    Build(BinaryOp::kShiftLeft, ty.i32(), ty.i32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %result:i32 = shl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %4:u32 = and %rhs, 31u
    %result:i32 = shl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_U32) {
    Build(BinaryOp::kShiftLeft, ty.u32(), ty.u32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %result:u32 = shl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %4:u32 = and %rhs, 31u
    %result:u32 = shl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_Vec2I32) {
    Build(BinaryOp::kShiftLeft, ty.vec2<i32>(), ty.vec2<i32>(), ty.vec2<u32>());
    auto* src = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<u32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = shl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<u32>):vec2<i32> {
  $B1: {
    %4:vec2<u32> = and %rhs, vec2<u32>(31u)
    %result:vec2<i32> = shl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_Vec3U32) {
    Build(BinaryOp::kShiftLeft, ty.vec3<u32>(), ty.vec3<u32>(), ty.vec3<u32>());
    auto* src = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %result:vec3<u32> = shl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %4:vec3<u32> = and %rhs, vec3<u32>(31u)
    %result:vec3<u32> = shl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_I32) {
    Build(BinaryOp::kShiftRight, ty.i32(), ty.i32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %result:i32 = shr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:u32):i32 {
  $B1: {
    %4:u32 = and %rhs, 31u
    %result:i32 = shr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_U32) {
    Build(BinaryOp::kShiftRight, ty.u32(), ty.u32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %result:u32 = shr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %4:u32 = and %rhs, 31u
    %result:u32 = shr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_Vec2I32) {
    Build(BinaryOp::kShiftRight, ty.vec2<i32>(), ty.vec2<i32>(), ty.vec2<u32>());
    auto* src = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<u32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = shr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<u32>):vec2<i32> {
  $B1: {
    %4:vec2<u32> = and %rhs, vec2<u32>(31u)
    %result:vec2<i32> = shr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_Vec3U32) {
    Build(BinaryOp::kShiftRight, ty.vec3<u32>(), ty.vec3<u32>(), ty.vec3<u32>());
    auto* src = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %result:vec3<u32> = shr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %4:vec3<u32> = and %rhs, vec3<u32>(31u)
    %result:vec3<u32> = shr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Divide_NoPolyfill) {
    Build(BinaryOp::kDivide, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %result:i32 = div %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = false;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Modulo_NoPolyfill) {
    Build(BinaryOp::kModulo, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %result:i32 = mod %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = false;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Divide_I32) {
    Build(BinaryOp::kDivide, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %result:i32 = div %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %result:i32 = call %tint_div_i32, %lhs, %rhs
    ret %result
  }
}
%tint_div_i32 = func(%lhs_1:i32, %rhs_1:i32):i32 {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:bool = eq %rhs_1, 0i
    %9:bool = eq %lhs_1, -2147483648i
    %10:bool = eq %rhs_1, -1i
    %11:bool = and %9, %10
    %12:bool = or %8, %11
    %13:i32 = select %rhs_1, 1i, %12
    %14:i32 = div %lhs_1, %13
    ret %14
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Divide_U32) {
    Build(BinaryOp::kDivide, ty.u32(), ty.u32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %result:u32 = div %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %result:u32 = call %tint_div_u32, %lhs, %rhs
    ret %result
  }
}
%tint_div_u32 = func(%lhs_1:u32, %rhs_1:u32):u32 {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:bool = eq %rhs_1, 0u
    %9:u32 = select %rhs_1, 1u, %8
    %10:u32 = div %lhs_1, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Divide_Vec2I32) {
    Build(BinaryOp::kDivide, ty.vec2<i32>(), ty.vec2<i32>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = div %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = call %tint_div_v2i32, %lhs, %rhs
    ret %result
  }
}
%tint_div_v2i32 = func(%lhs_1:vec2<i32>, %rhs_1:vec2<i32>):vec2<i32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:vec2<bool> = eq %rhs_1, vec2<i32>(0i)
    %9:vec2<bool> = eq %lhs_1, vec2<i32>(-2147483648i)
    %10:vec2<bool> = eq %rhs_1, vec2<i32>(-1i)
    %11:vec2<bool> = and %9, %10
    %12:vec2<bool> = or %8, %11
    %13:vec2<i32> = select %rhs_1, vec2<i32>(1i), %12
    %14:vec2<i32> = div %lhs_1, %13
    ret %14
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Divide_Vec3U32) {
    Build(BinaryOp::kDivide, ty.vec3<u32>(), ty.vec3<u32>(), ty.vec3<u32>());
    auto* src = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %result:vec3<u32> = div %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %result:vec3<u32> = call %tint_div_v3u32, %lhs, %rhs
    ret %result
  }
}
%tint_div_v3u32 = func(%lhs_1:vec3<u32>, %rhs_1:vec3<u32>):vec3<u32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:vec3<bool> = eq %rhs_1, vec3<u32>(0u)
    %9:vec3<u32> = select %rhs_1, vec3<u32>(1u), %8
    %10:vec3<u32> = div %lhs_1, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Modulo_I32) {
    Build(BinaryOp::kModulo, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %result:i32 = mod %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 {
  $B1: {
    %result:i32 = call %tint_mod_i32, %lhs, %rhs
    ret %result
  }
}
%tint_mod_i32 = func(%lhs_1:i32, %rhs_1:i32):i32 {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:bool = eq %rhs_1, 0i
    %9:bool = eq %lhs_1, -2147483648i
    %10:bool = eq %rhs_1, -1i
    %11:bool = and %9, %10
    %12:bool = or %8, %11
    %13:i32 = select %rhs_1, 1i, %12
    %14:i32 = div %lhs_1, %13
    %15:i32 = mul %14, %13
    %16:i32 = sub %lhs_1, %15
    ret %16
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Modulo_U32) {
    Build(BinaryOp::kModulo, ty.u32(), ty.u32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %result:u32 = mod %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 {
  $B1: {
    %result:u32 = call %tint_mod_u32, %lhs, %rhs
    ret %result
  }
}
%tint_mod_u32 = func(%lhs_1:u32, %rhs_1:u32):u32 {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:bool = eq %rhs_1, 0u
    %9:u32 = select %rhs_1, 1u, %8
    %10:u32 = div %lhs_1, %9
    %11:u32 = mul %10, %9
    %12:u32 = sub %lhs_1, %11
    ret %12
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Modulo_Vec2I32) {
    Build(BinaryOp::kModulo, ty.vec2<i32>(), ty.vec2<i32>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = mod %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = call %tint_mod_v2i32, %lhs, %rhs
    ret %result
  }
}
%tint_mod_v2i32 = func(%lhs_1:vec2<i32>, %rhs_1:vec2<i32>):vec2<i32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:vec2<bool> = eq %rhs_1, vec2<i32>(0i)
    %9:vec2<bool> = eq %lhs_1, vec2<i32>(-2147483648i)
    %10:vec2<bool> = eq %rhs_1, vec2<i32>(-1i)
    %11:vec2<bool> = and %9, %10
    %12:vec2<bool> = or %8, %11
    %13:vec2<i32> = select %rhs_1, vec2<i32>(1i), %12
    %14:vec2<i32> = div %lhs_1, %13
    %15:vec2<i32> = mul %14, %13
    %16:vec2<i32> = sub %lhs_1, %15
    ret %16
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Modulo_Vec3U32) {
    Build(BinaryOp::kModulo, ty.vec3<u32>(), ty.vec3<u32>(), ty.vec3<u32>());
    auto* src = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %result:vec3<u32> = mod %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> {
  $B1: {
    %result:vec3<u32> = call %tint_mod_v3u32, %lhs, %rhs
    ret %result
  }
}
%tint_mod_v3u32 = func(%lhs_1:vec3<u32>, %rhs_1:vec3<u32>):vec3<u32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %8:vec3<bool> = eq %rhs_1, vec3<u32>(0u)
    %9:vec3<u32> = select %rhs_1, vec3<u32>(1u), %8
    %10:vec3<u32> = div %lhs_1, %9
    %11:vec3<u32> = mul %10, %9
    %12:vec3<u32> = sub %lhs_1, %11
    ret %12
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Divide_Scalar_Vector) {
    Build(BinaryOp::kDivide, ty.vec4<i32>(), ty.i32(), ty.vec4<i32>());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %result:vec4<i32> = div %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = construct %lhs
    %result:vec4<i32> = call %tint_div_v4i32, %4, %rhs
    ret %result
  }
}
%tint_div_v4i32 = func(%lhs_1:vec4<i32>, %rhs_1:vec4<i32>):vec4<i32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %9:vec4<bool> = eq %rhs_1, vec4<i32>(0i)
    %10:vec4<bool> = eq %lhs_1, vec4<i32>(-2147483648i)
    %11:vec4<bool> = eq %rhs_1, vec4<i32>(-1i)
    %12:vec4<bool> = and %10, %11
    %13:vec4<bool> = or %9, %12
    %14:vec4<i32> = select %rhs_1, vec4<i32>(1i), %13
    %15:vec4<i32> = div %lhs_1, %14
    ret %15
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Divide_Vector_Scalar) {
    Build(BinaryOp::kDivide, ty.vec4<i32>(), ty.vec4<i32>(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:vec4<i32>, %rhs:i32):vec4<i32> {
  $B1: {
    %result:vec4<i32> = div %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec4<i32>, %rhs:i32):vec4<i32> {
  $B1: {
    %4:vec4<i32> = construct %rhs
    %result:vec4<i32> = call %tint_div_v4i32, %lhs, %4
    ret %result
  }
}
%tint_div_v4i32 = func(%lhs_1:vec4<i32>, %rhs_1:vec4<i32>):vec4<i32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %9:vec4<bool> = eq %rhs_1, vec4<i32>(0i)
    %10:vec4<bool> = eq %lhs_1, vec4<i32>(-2147483648i)
    %11:vec4<bool> = eq %rhs_1, vec4<i32>(-1i)
    %12:vec4<bool> = and %10, %11
    %13:vec4<bool> = or %9, %12
    %14:vec4<i32> = select %rhs_1, vec4<i32>(1i), %13
    %15:vec4<i32> = div %lhs_1, %14
    ret %15
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Modulo_Scalar_Vector) {
    Build(BinaryOp::kModulo, ty.vec4<i32>(), ty.i32(), ty.vec4<i32>());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %result:vec4<i32> = mod %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:vec4<i32>):vec4<i32> {
  $B1: {
    %4:vec4<i32> = construct %lhs
    %result:vec4<i32> = call %tint_mod_v4i32, %4, %rhs
    ret %result
  }
}
%tint_mod_v4i32 = func(%lhs_1:vec4<i32>, %rhs_1:vec4<i32>):vec4<i32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %9:vec4<bool> = eq %rhs_1, vec4<i32>(0i)
    %10:vec4<bool> = eq %lhs_1, vec4<i32>(-2147483648i)
    %11:vec4<bool> = eq %rhs_1, vec4<i32>(-1i)
    %12:vec4<bool> = and %10, %11
    %13:vec4<bool> = or %9, %12
    %14:vec4<i32> = select %rhs_1, vec4<i32>(1i), %13
    %15:vec4<i32> = div %lhs_1, %14
    %16:vec4<i32> = mul %15, %14
    %17:vec4<i32> = sub %lhs_1, %16
    ret %17
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, Modulo_Vector_Scalar) {
    Build(BinaryOp::kModulo, ty.vec4<i32>(), ty.vec4<i32>(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:vec4<i32>, %rhs:i32):vec4<i32> {
  $B1: {
    %result:vec4<i32> = mod %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec4<i32>, %rhs:i32):vec4<i32> {
  $B1: {
    %4:vec4<i32> = construct %rhs
    %result:vec4<i32> = call %tint_mod_v4i32, %lhs, %4
    ret %result
  }
}
%tint_mod_v4i32 = func(%lhs_1:vec4<i32>, %rhs_1:vec4<i32>):vec4<i32> {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %9:vec4<bool> = eq %rhs_1, vec4<i32>(0i)
    %10:vec4<bool> = eq %lhs_1, vec4<i32>(-2147483648i)
    %11:vec4<bool> = eq %rhs_1, vec4<i32>(-1i)
    %12:vec4<bool> = and %10, %11
    %13:vec4<bool> = or %9, %12
    %14:vec4<i32> = select %rhs_1, vec4<i32>(1i), %13
    %15:vec4<i32> = div %lhs_1, %14
    %16:vec4<i32> = mul %15, %14
    %17:vec4<i32> = sub %lhs_1, %16
    ret %17
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, DivMod_MultipleUses) {
    {
        Vector<FunctionParam*, 4> args;
        args.Push(b.FunctionParam("lhs", ty.i32()));
        args.Push(b.FunctionParam("rhs", ty.i32()));
        auto* func = b.Function("foo_i32", ty.void_());
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            b.Binary(BinaryOp::kDivide, ty.i32(), args[0], args[1]);
            b.Binary(BinaryOp::kDivide, ty.i32(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.i32(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.i32(), args[0], args[1]);
            b.Return(func);
        });
    }
    {
        Vector<FunctionParam*, 4> args;
        args.Push(b.FunctionParam("lhs", ty.u32()));
        args.Push(b.FunctionParam("rhs", ty.u32()));
        auto* func = b.Function("foo_u32", ty.void_());
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            b.Binary(BinaryOp::kDivide, ty.u32(), args[0], args[1]);
            b.Binary(BinaryOp::kDivide, ty.u32(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.u32(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.u32(), args[0], args[1]);
            b.Return(func);
        });
    }
    {
        Vector<FunctionParam*, 4> args;
        args.Push(b.FunctionParam("lhs", ty.vec4<i32>()));
        args.Push(b.FunctionParam("rhs", ty.vec4<i32>()));
        auto* func = b.Function("foo_vec4i", ty.void_());
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            b.Binary(BinaryOp::kDivide, ty.vec4<i32>(), args[0], args[1]);
            b.Binary(BinaryOp::kDivide, ty.vec4<i32>(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.vec4<i32>(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.vec4<i32>(), args[0], args[1]);
            b.Return(func);
        });
    }
    {
        Vector<FunctionParam*, 4> args;
        args.Push(b.FunctionParam("lhs", ty.vec4<u32>()));
        args.Push(b.FunctionParam("rhs", ty.vec4<u32>()));
        auto* func = b.Function("foo_vec4u", ty.void_());
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            b.Binary(BinaryOp::kDivide, ty.vec4<u32>(), args[0], args[1]);
            b.Binary(BinaryOp::kDivide, ty.vec4<u32>(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.vec4<u32>(), args[0], args[1]);
            b.Binary(BinaryOp::kModulo, ty.vec4<u32>(), args[0], args[1]);
            b.Return(func);
        });
    }

    auto* src = R"(
%foo_i32 = func(%lhs:i32, %rhs:i32):void {
  $B1: {
    %4:i32 = div %lhs, %rhs
    %5:i32 = div %lhs, %rhs
    %6:i32 = mod %lhs, %rhs
    %7:i32 = mod %lhs, %rhs
    ret
  }
}
%foo_u32 = func(%lhs_1:u32, %rhs_1:u32):void {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %11:u32 = div %lhs_1, %rhs_1
    %12:u32 = div %lhs_1, %rhs_1
    %13:u32 = mod %lhs_1, %rhs_1
    %14:u32 = mod %lhs_1, %rhs_1
    ret
  }
}
%foo_vec4i = func(%lhs_2:vec4<i32>, %rhs_2:vec4<i32>):void {  # %lhs_2: 'lhs', %rhs_2: 'rhs'
  $B3: {
    %18:vec4<i32> = div %lhs_2, %rhs_2
    %19:vec4<i32> = div %lhs_2, %rhs_2
    %20:vec4<i32> = mod %lhs_2, %rhs_2
    %21:vec4<i32> = mod %lhs_2, %rhs_2
    ret
  }
}
%foo_vec4u = func(%lhs_3:vec4<u32>, %rhs_3:vec4<u32>):void {  # %lhs_3: 'lhs', %rhs_3: 'rhs'
  $B4: {
    %25:vec4<u32> = div %lhs_3, %rhs_3
    %26:vec4<u32> = div %lhs_3, %rhs_3
    %27:vec4<u32> = mod %lhs_3, %rhs_3
    %28:vec4<u32> = mod %lhs_3, %rhs_3
    ret
  }
}
)";
    auto* expect = R"(
%foo_i32 = func(%lhs:i32, %rhs:i32):void {
  $B1: {
    %4:i32 = call %tint_div_i32, %lhs, %rhs
    %6:i32 = call %tint_div_i32, %lhs, %rhs
    %7:i32 = call %tint_mod_i32, %lhs, %rhs
    %9:i32 = call %tint_mod_i32, %lhs, %rhs
    ret
  }
}
%foo_u32 = func(%lhs_1:u32, %rhs_1:u32):void {  # %lhs_1: 'lhs', %rhs_1: 'rhs'
  $B2: {
    %13:u32 = call %tint_div_u32, %lhs_1, %rhs_1
    %15:u32 = call %tint_div_u32, %lhs_1, %rhs_1
    %16:u32 = call %tint_mod_u32, %lhs_1, %rhs_1
    %18:u32 = call %tint_mod_u32, %lhs_1, %rhs_1
    ret
  }
}
%foo_vec4i = func(%lhs_2:vec4<i32>, %rhs_2:vec4<i32>):void {  # %lhs_2: 'lhs', %rhs_2: 'rhs'
  $B3: {
    %22:vec4<i32> = call %tint_div_v4i32, %lhs_2, %rhs_2
    %24:vec4<i32> = call %tint_div_v4i32, %lhs_2, %rhs_2
    %25:vec4<i32> = call %tint_mod_v4i32, %lhs_2, %rhs_2
    %27:vec4<i32> = call %tint_mod_v4i32, %lhs_2, %rhs_2
    ret
  }
}
%foo_vec4u = func(%lhs_3:vec4<u32>, %rhs_3:vec4<u32>):void {  # %lhs_3: 'lhs', %rhs_3: 'rhs'
  $B4: {
    %31:vec4<u32> = call %tint_div_v4u32, %lhs_3, %rhs_3
    %33:vec4<u32> = call %tint_div_v4u32, %lhs_3, %rhs_3
    %34:vec4<u32> = call %tint_mod_v4u32, %lhs_3, %rhs_3
    %36:vec4<u32> = call %tint_mod_v4u32, %lhs_3, %rhs_3
    ret
  }
}
%tint_div_i32 = func(%lhs_4:i32, %rhs_4:i32):i32 {  # %lhs_4: 'lhs', %rhs_4: 'rhs'
  $B5: {
    %39:bool = eq %rhs_4, 0i
    %40:bool = eq %lhs_4, -2147483648i
    %41:bool = eq %rhs_4, -1i
    %42:bool = and %40, %41
    %43:bool = or %39, %42
    %44:i32 = select %rhs_4, 1i, %43
    %45:i32 = div %lhs_4, %44
    ret %45
  }
}
%tint_mod_i32 = func(%lhs_5:i32, %rhs_5:i32):i32 {  # %lhs_5: 'lhs', %rhs_5: 'rhs'
  $B6: {
    %48:bool = eq %rhs_5, 0i
    %49:bool = eq %lhs_5, -2147483648i
    %50:bool = eq %rhs_5, -1i
    %51:bool = and %49, %50
    %52:bool = or %48, %51
    %53:i32 = select %rhs_5, 1i, %52
    %54:i32 = div %lhs_5, %53
    %55:i32 = mul %54, %53
    %56:i32 = sub %lhs_5, %55
    ret %56
  }
}
%tint_div_u32 = func(%lhs_6:u32, %rhs_6:u32):u32 {  # %lhs_6: 'lhs', %rhs_6: 'rhs'
  $B7: {
    %59:bool = eq %rhs_6, 0u
    %60:u32 = select %rhs_6, 1u, %59
    %61:u32 = div %lhs_6, %60
    ret %61
  }
}
%tint_mod_u32 = func(%lhs_7:u32, %rhs_7:u32):u32 {  # %lhs_7: 'lhs', %rhs_7: 'rhs'
  $B8: {
    %64:bool = eq %rhs_7, 0u
    %65:u32 = select %rhs_7, 1u, %64
    %66:u32 = div %lhs_7, %65
    %67:u32 = mul %66, %65
    %68:u32 = sub %lhs_7, %67
    ret %68
  }
}
%tint_div_v4i32 = func(%lhs_8:vec4<i32>, %rhs_8:vec4<i32>):vec4<i32> {  # %lhs_8: 'lhs', %rhs_8: 'rhs'
  $B9: {
    %71:vec4<bool> = eq %rhs_8, vec4<i32>(0i)
    %72:vec4<bool> = eq %lhs_8, vec4<i32>(-2147483648i)
    %73:vec4<bool> = eq %rhs_8, vec4<i32>(-1i)
    %74:vec4<bool> = and %72, %73
    %75:vec4<bool> = or %71, %74
    %76:vec4<i32> = select %rhs_8, vec4<i32>(1i), %75
    %77:vec4<i32> = div %lhs_8, %76
    ret %77
  }
}
%tint_mod_v4i32 = func(%lhs_9:vec4<i32>, %rhs_9:vec4<i32>):vec4<i32> {  # %lhs_9: 'lhs', %rhs_9: 'rhs'
  $B10: {
    %80:vec4<bool> = eq %rhs_9, vec4<i32>(0i)
    %81:vec4<bool> = eq %lhs_9, vec4<i32>(-2147483648i)
    %82:vec4<bool> = eq %rhs_9, vec4<i32>(-1i)
    %83:vec4<bool> = and %81, %82
    %84:vec4<bool> = or %80, %83
    %85:vec4<i32> = select %rhs_9, vec4<i32>(1i), %84
    %86:vec4<i32> = div %lhs_9, %85
    %87:vec4<i32> = mul %86, %85
    %88:vec4<i32> = sub %lhs_9, %87
    ret %88
  }
}
%tint_div_v4u32 = func(%lhs_10:vec4<u32>, %rhs_10:vec4<u32>):vec4<u32> {  # %lhs_10: 'lhs', %rhs_10: 'rhs'
  $B11: {
    %91:vec4<bool> = eq %rhs_10, vec4<u32>(0u)
    %92:vec4<u32> = select %rhs_10, vec4<u32>(1u), %91
    %93:vec4<u32> = div %lhs_10, %92
    ret %93
  }
}
%tint_mod_v4u32 = func(%lhs_11:vec4<u32>, %rhs_11:vec4<u32>):vec4<u32> {  # %lhs_11: 'lhs', %rhs_11: 'rhs'
  $B12: {
    %96:vec4<bool> = eq %rhs_11, vec4<u32>(0u)
    %97:vec4<u32> = select %rhs_11, vec4<u32>(1u), %96
    %98:vec4<u32> = div %lhs_11, %97
    %99:vec4<u32> = mul %98, %97
    %100:vec4<u32> = sub %lhs_11, %99
    ret %100
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.int_div_mod = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
