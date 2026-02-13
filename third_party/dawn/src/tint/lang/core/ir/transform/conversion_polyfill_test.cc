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

#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ConversionPolyfillTest : public TransformTest {
  protected:
    /// Helper to build a function that executes a convert instruction.
    /// @param src_ty the type of the source
    /// @param res_ty the type of the result
    void Build(const core::type::Type* src_ty, const core::type::Type* res_ty) {
        auto* func = b.Function("foo", res_ty);
        auto* src = b.FunctionParam("src", src_ty);
        func->SetParams({src});
        b.Append(func->Block(), [&] {
            auto* result = b.Convert(res_ty, src);
            b.Return(func, result);
            mod.SetName(result, "result");
        });
    }
};

// No change expected in this direction.
TEST_F(IR_ConversionPolyfillTest, I32_to_F32) {
    Build(ty.i32(), ty.f32());
    auto* src = R"(
%foo = func(%src:i32):f32 {
  $B1: {
    %result:f32 = convert %src
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

// No change expected in this direction.
TEST_F(IR_ConversionPolyfillTest, U32_to_F32) {
    Build(ty.u32(), ty.f32());
    auto* src = R"(
%foo = func(%src:u32):f32 {
  $B1: {
    %result:f32 = convert %src
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_I32_NoPolyfill) {
    Build(ty.f32(), ty.i32());
    auto* src = R"(
%foo = func(%src:f32):i32 {
  $B1: {
    %result:i32 = convert %src
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = false;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_I32) {
    Build(ty.f32(), ty.i32());
    auto* src = R"(
%foo = func(%src:f32):i32 {
  $B1: {
    %result:i32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f32):i32 {
  $B1: {
    %result:i32 = call %tint_f32_to_i32, %src
    ret %result
  }
}
%tint_f32_to_i32 = func(%value:f32):i32 {
  $B2: {
    %6:f32 = clamp %value, -2147483648.0f, 2147483520.0f
    %7:i32 = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_U32) {
    Build(ty.f32(), ty.u32());
    auto* src = R"(
%foo = func(%src:f32):u32 {
  $B1: {
    %result:u32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f32):u32 {
  $B1: {
    %result:u32 = call %tint_f32_to_u32, %src
    ret %result
  }
}
%tint_f32_to_u32 = func(%value:f32):u32 {
  $B2: {
    %6:f32 = clamp %value, 0.0f, 4294967040.0f
    %7:u32 = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_I32_Vec2) {
    Build(ty.vec2<f32>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%src:vec2<f32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f32>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = call %tint_v2f32_to_v2i32, %src
    ret %result
  }
}
%tint_v2f32_to_v2i32 = func(%value:vec2<f32>):vec2<i32> {
  $B2: {
    %6:vec2<f32> = clamp %value, vec2<f32>(-2147483648.0f), vec2<f32>(2147483520.0f)
    %7:vec2<i32> = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_U32_Vec3) {
    Build(ty.vec2<f32>(), ty.vec2<u32>());
    auto* src = R"(
%foo = func(%src:vec2<f32>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f32>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = call %tint_v2f32_to_v2u32, %src
    ret %result
  }
}
%tint_v2f32_to_v2u32 = func(%value:vec2<f32>):vec2<u32> {
  $B2: {
    %6:vec2<f32> = clamp %value, vec2<f32>(0.0f), vec2<f32>(4294967040.0f)
    %7:vec2<u32> = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_I32) {
    Build(ty.f16(), ty.i32());
    auto* src = R"(
%foo = func(%src:f16):i32 {
  $B1: {
    %result:i32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f16):i32 {
  $B1: {
    %result:i32 = call %tint_f16_to_i32, %src
    ret %result
  }
}
%tint_f16_to_i32 = func(%value:f16):i32 {
  $B2: {
    %6:f16 = clamp %value, -65504.0h, 65504.0h
    %7:i32 = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_U32) {
    Build(ty.f16(), ty.u32());
    auto* src = R"(
%foo = func(%src:f16):u32 {
  $B1: {
    %result:u32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f16):u32 {
  $B1: {
    %result:u32 = call %tint_f16_to_u32, %src
    ret %result
  }
}
%tint_f16_to_u32 = func(%value:f16):u32 {
  $B2: {
    %6:f16 = clamp %value, 0.0h, 65504.0h
    %7:u32 = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_I32_Vec2) {
    Build(ty.vec2<f16>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%src:vec2<f16>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f16>):vec2<i32> {
  $B1: {
    %result:vec2<i32> = call %tint_v2f16_to_v2i32, %src
    ret %result
  }
}
%tint_v2f16_to_v2i32 = func(%value:vec2<f16>):vec2<i32> {
  $B2: {
    %6:vec2<f16> = clamp %value, vec2<f16>(-65504.0h), vec2<f16>(65504.0h)
    %7:vec2<i32> = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_U32_Vec3) {
    Build(ty.vec2<f16>(), ty.vec2<u32>());
    auto* src = R"(
%foo = func(%src:vec2<f16>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f16>):vec2<u32> {
  $B1: {
    %result:vec2<u32> = call %tint_v2f16_to_v2u32, %src
    ret %result
  }
}
%tint_v2f16_to_v2u32 = func(%value:vec2<f16>):vec2<u32> {
  $B2: {
    %6:vec2<f16> = clamp %value, vec2<f16>(0.0h), vec2<f16>(65504.0h)
    %7:vec2<u32> = convert %6
    ret %7
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
