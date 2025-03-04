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

#include "src/tint/lang/hlsl/writer/raise/binary_polyfill.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriter_BinaryPolyfillTest = core::ir::transform::TransformTest;

TEST_F(HlslWriter_BinaryPolyfillTest, ModF32) {
    auto* x = b.FunctionParam<f32>("x");
    auto* y = b.FunctionParam<f32>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Modulo(ty.f32(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:f32, %y:f32):void {
  $B1: {
    %4:f32 = mod %x, %y
    %a:f32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:f32, %y:f32):void {
  $B1: {
    %4:f32 = div %x, %y
    %5:f32 = let %4
    %6:f32 = trunc %5
    %7:f32 = mul %6, %y
    %8:f32 = sub %x, %7
    %a:f32 = let %8
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, ModF16) {
    auto* x = b.FunctionParam<f16>("x");
    auto* y = b.FunctionParam<f16>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Modulo(ty.f16(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:f16, %y:f16):void {
  $B1: {
    %4:f16 = mod %x, %y
    %a:f16 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:f16, %y:f16):void {
  $B1: {
    %4:f16 = div %x, %y
    %5:f16 = let %4
    %6:f16 = trunc %5
    %7:f16 = mul %6, %y
    %8:f16 = sub %x, %7
    %a:f16 = let %8
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, ModF32Vec3) {
    auto* x = b.FunctionParam<vec3<f32>>("x");
    auto* y = b.FunctionParam<vec3<f32>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Modulo(ty.vec3<f32>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:vec3<f32>, %y:vec3<f32>):void {
  $B1: {
    %4:vec3<f32> = mod %x, %y
    %a:vec3<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec3<f32>, %y:vec3<f32>):void {
  $B1: {
    %4:vec3<f32> = div %x, %y
    %5:vec3<f32> = let %4
    %6:vec3<f32> = trunc %5
    %7:vec3<f32> = mul %6, %y
    %8:vec3<f32> = sub %x, %7
    %a:vec3<f32> = let %8
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, ModF16Vec3) {
    auto* x = b.FunctionParam<vec3<f16>>("x");
    auto* y = b.FunctionParam<vec3<f16>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Modulo(ty.vec3<f16>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:vec3<f16>, %y:vec3<f16>):void {
  $B1: {
    %4:vec3<f16> = mod %x, %y
    %a:vec3<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec3<f16>, %y:vec3<f16>):void {
  $B1: {
    %4:vec3<f16> = div %x, %y
    %5:vec3<f16> = let %4
    %6:vec3<f16> = trunc %5
    %7:vec3<f16> = mul %6, %y
    %8:vec3<f16> = sub %x, %7
    %a:vec3<f16> = let %8
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, MulVecMatF32) {
    auto* x = b.FunctionParam<vec3<f32>>("x");
    auto* y = b.FunctionParam<mat3x3<f32>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Multiply(ty.vec3<f32>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:vec3<f32>, %y:mat3x3<f32>):void {
  $B1: {
    %4:vec3<f32> = mul %x, %y
    %a:vec3<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec3<f32>, %y:mat3x3<f32>):void {
  $B1: {
    %4:vec3<f32> = hlsl.mul %y, %x
    %a:vec3<f32> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, MulVecMatF16) {
    auto* x = b.FunctionParam<vec3<f16>>("x");
    auto* y = b.FunctionParam<mat3x3<f16>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Multiply(ty.vec3<f16>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:vec3<f16>, %y:mat3x3<f16>):void {
  $B1: {
    %4:vec3<f16> = mul %x, %y
    %a:vec3<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec3<f16>, %y:mat3x3<f16>):void {
  $B1: {
    %4:vec3<f16> = hlsl.mul %y, %x
    %a:vec3<f16> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, MulMatVecF32) {
    auto* x = b.FunctionParam<mat3x3<f32>>("x");
    auto* y = b.FunctionParam<vec3<f32>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Multiply(ty.vec3<f32>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:mat3x3<f32>, %y:vec3<f32>):void {
  $B1: {
    %4:vec3<f32> = mul %x, %y
    %a:vec3<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:mat3x3<f32>, %y:vec3<f32>):void {
  $B1: {
    %4:vec3<f32> = hlsl.mul %y, %x
    %a:vec3<f32> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, MulMatVecF16) {
    auto* x = b.FunctionParam<mat3x3<f16>>("x");
    auto* y = b.FunctionParam<vec3<f16>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Multiply(ty.vec3<f16>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:mat3x3<f16>, %y:vec3<f16>):void {
  $B1: {
    %4:vec3<f16> = mul %x, %y
    %a:vec3<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:mat3x3<f16>, %y:vec3<f16>):void {
  $B1: {
    %4:vec3<f16> = hlsl.mul %y, %x
    %a:vec3<f16> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, MulMatMat32) {
    auto* x = b.FunctionParam<mat3x3<f32>>("x");
    auto* y = b.FunctionParam<mat3x3<f32>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Multiply(ty.mat3x3<f32>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:mat3x3<f32>, %y:mat3x3<f32>):void {
  $B1: {
    %4:mat3x3<f32> = mul %x, %y
    %a:mat3x3<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:mat3x3<f32>, %y:mat3x3<f32>):void {
  $B1: {
    %4:mat3x3<f32> = hlsl.mul %y, %x
    %a:mat3x3<f32> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BinaryPolyfillTest, MulMatMat16) {
    auto* x = b.FunctionParam<mat3x3<f16>>("x");
    auto* y = b.FunctionParam<mat3x3<f16>>("y");
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Multiply(ty.mat3x3<f16>(), x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:mat3x3<f16>, %y:mat3x3<f16>):void {
  $B1: {
    %4:mat3x3<f16> = mul %x, %y
    %a:mat3x3<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:mat3x3<f16>, %y:mat3x3<f16>):void {
  $B1: {
    %4:mat3x3<f16> = hlsl.mul %y, %x
    %a:mat3x3<f16> = let %4
    ret
  }
}
)";

    Run(BinaryPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
