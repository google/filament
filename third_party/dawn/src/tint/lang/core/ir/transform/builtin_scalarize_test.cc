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

#include "src/tint/lang/core/ir/transform/builtin_scalarize.h"

#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/sampled_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_BuiltinScalarizeTest = TransformTest;

TEST_F(IR_BuiltinScalarizeTest, Clamp_VectorOperands_Scalarize) {
    auto* x = b.FunctionParam("x", ty.vec2<f32>());
    auto* low = b.FunctionParam("low", ty.vec2<f32>());
    auto* high = b.FunctionParam("high", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({x, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), core::BuiltinFn::kClamp, x, low, high);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:vec2<f32>, %low:vec2<f32>, %high:vec2<f32>):vec2<f32> {
  $B1: {
    %5:vec2<f32> = clamp %x, %low, %high
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec2<f32>, %low:vec2<f32>, %high:vec2<f32>):vec2<f32> {
  $B1: {
    %5:f32 = access %x, 0u
    %6:f32 = access %low, 0u
    %7:f32 = access %high, 0u
    %8:f32 = clamp %5, %6, %7
    %9:f32 = access %x, 1u
    %10:f32 = access %low, 1u
    %11:f32 = access %high, 1u
    %12:f32 = clamp %9, %10, %11
    %13:vec2<f32> = construct %8, %12
    ret %13
  }
}
)";

    BuiltinScalarizeConfig config{.scalarize_clamp = true};
    Run(BuiltinScalarize, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinScalarizeTest, Clamp_VectorOperands_AlreadyScalarize) {
    auto* x = b.FunctionParam("x", ty.f32());
    auto* low = b.FunctionParam("low", ty.f32());
    auto* high = b.FunctionParam("high", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({x, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kClamp, x, low, high);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:f32, %low:f32, %high:f32):f32 {
  $B1: {
    %5:f32 = clamp %x, %low, %high
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:f32, %low:f32, %high:f32):f32 {
  $B1: {
    %5:f32 = clamp %x, %low, %high
    ret %5
  }
}
)";

    BuiltinScalarizeConfig config{.scalarize_clamp = true};
    Run(BuiltinScalarize, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinScalarizeTest, Clamp_VectorOperands_LeaveVectorized) {
    auto* x = b.FunctionParam("x", ty.vec2<f32>());
    auto* low = b.FunctionParam("low", ty.vec2<f32>());
    auto* high = b.FunctionParam("high", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({x, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), core::BuiltinFn::kClamp, x, low, high);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:vec2<f32>, %low:vec2<f32>, %high:vec2<f32>):vec2<f32> {
  $B1: {
    %5:vec2<f32> = clamp %x, %low, %high
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec2<f32>, %low:vec2<f32>, %high:vec2<f32>):vec2<f32> {
  $B1: {
    %5:vec2<f32> = clamp %x, %low, %high
    ret %5
  }
}
)";

    BuiltinScalarizeConfig config;
    Run(BuiltinScalarize, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinScalarizeTest, Max_VectorOperands_Scalarize) {
    auto* x = b.FunctionParam("x", ty.vec3<f32>());
    auto* y = b.FunctionParam("y", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({x, y});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), core::BuiltinFn::kMax, x, y);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:vec3<f32>, %y:vec3<f32>):vec3<f32> {
  $B1: {
    %4:vec3<f32> = max %x, %y
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec3<f32>, %y:vec3<f32>):vec3<f32> {
  $B1: {
    %4:f32 = access %x, 0u
    %5:f32 = access %y, 0u
    %6:f32 = max %4, %5
    %7:f32 = access %x, 1u
    %8:f32 = access %y, 1u
    %9:f32 = max %7, %8
    %10:f32 = access %x, 2u
    %11:f32 = access %y, 2u
    %12:f32 = max %10, %11
    %13:vec3<f32> = construct %6, %9, %12
    ret %13
  }
}
)";

    BuiltinScalarizeConfig config{.scalarize_max = true};
    Run(BuiltinScalarize, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinScalarizeTest, Max_VectorOperands_LeaveVectorized) {
    auto* x = b.FunctionParam("x", ty.vec3<f32>());
    auto* y = b.FunctionParam("y", ty.vec3<f32>());
    auto* func = b.Function("foo", ty.vec3<f32>());
    func->SetParams({x, y});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), core::BuiltinFn::kMax, x, y);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:vec3<f32>, %y:vec3<f32>):vec3<f32> {
  $B1: {
    %4:vec3<f32> = max %x, %y
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec3<f32>, %y:vec3<f32>):vec3<f32> {
  $B1: {
    %4:vec3<f32> = max %x, %y
    ret %4
  }
}
)";

    BuiltinScalarizeConfig config;
    Run(BuiltinScalarize, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinScalarizeTest, Min_VectorOperands_Scalarize) {
    auto* x = b.FunctionParam("x", ty.vec4<f16>());
    auto* y = b.FunctionParam("y", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({x, y});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), core::BuiltinFn::kMin, x, y);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:vec4<f16>, %y:vec4<f16>):vec4<f16> {
  $B1: {
    %4:vec4<f16> = min %x, %y
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec4<f16>, %y:vec4<f16>):vec4<f16> {
  $B1: {
    %4:f16 = access %x, 0u
    %5:f16 = access %y, 0u
    %6:f16 = min %4, %5
    %7:f16 = access %x, 1u
    %8:f16 = access %y, 1u
    %9:f16 = min %7, %8
    %10:f16 = access %x, 2u
    %11:f16 = access %y, 2u
    %12:f16 = min %10, %11
    %13:f16 = access %x, 3u
    %14:f16 = access %y, 3u
    %15:f16 = min %13, %14
    %16:vec4<f16> = construct %6, %9, %12, %15
    ret %16
  }
}
)";

    BuiltinScalarizeConfig config{.scalarize_min = true};
    Run(BuiltinScalarize, config);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinScalarizeTest, Min_VectorOperands_LeaveVectorized) {
    auto* x = b.FunctionParam("x", ty.vec4<f16>());
    auto* y = b.FunctionParam("y", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.vec4<f16>());
    func->SetParams({x, y});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f16>(), core::BuiltinFn::kMin, x, y);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:vec4<f16>, %y:vec4<f16>):vec4<f16> {
  $B1: {
    %4:vec4<f16> = min %x, %y
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec4<f16>, %y:vec4<f16>):vec4<f16> {
  $B1: {
    %4:vec4<f16> = min %x, %y
    ret %4
  }
}
)";

    BuiltinScalarizeConfig config;
    Run(BuiltinScalarize, config);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
