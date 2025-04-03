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

#include "src/tint/lang/spirv/reader/lower/builtins.h"

#include <string>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvReader_BuiltinsTest = core::ir::transform::TransformTest;

TEST_F(SpirvReader_BuiltinsTest, Normalize_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kNormalize, 10_f);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.normalize 10.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = sign 10.0f
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Normalize_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kNormalize,
                                       b.Splat(ty.vec2<f32>(), 10_f));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.normalize vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = normalize vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat2x2f) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.mat2x2<f32>(), spirv::BuiltinFn::kInverse,
                                       b.Construct(ty.mat2x2<f32>(), b.Splat(ty.vec2<f32>(), 10_f),
                                                   b.Splat(ty.vec2<f32>(), 20_f)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = construct vec2<f32>(10.0f), vec2<f32>(20.0f)
    %3:mat2x2<f32> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = construct vec2<f32>(10.0f), vec2<f32>(20.0f)
    %3:f32 = determinant %2
    %4:f32 = div 1.0f, %3
    %5:f32 = negation %4
    %6:f32 = access %2, 0u, 0u
    %7:f32 = access %2, 0u, 1u
    %8:f32 = access %2, 1u, 0u
    %9:f32 = access %2, 1u, 1u
    %10:f32 = mul %4, %9
    %11:f32 = mul %5, %7
    %12:f32 = mul %5, %8
    %13:f32 = mul %4, %6
    %14:vec2<f32> = construct %10, %11
    %15:vec2<f32> = construct %12, %13
    %16:mat2x2<f32> = construct %14, %15
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat2x2h) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.mat2x2<f16>(), spirv::BuiltinFn::kInverse,
                                       b.Construct(ty.mat2x2<f16>(), b.Splat(ty.vec2<f16>(), 10_h),
                                                   b.Splat(ty.vec2<f16>(), 20_h)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f16> = construct vec2<f16>(10.0h), vec2<f16>(20.0h)
    %3:mat2x2<f16> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f16> = construct vec2<f16>(10.0h), vec2<f16>(20.0h)
    %3:f16 = determinant %2
    %4:f16 = div 1.0h, %3
    %5:f16 = negation %4
    %6:f16 = access %2, 0u, 0u
    %7:f16 = access %2, 0u, 1u
    %8:f16 = access %2, 1u, 0u
    %9:f16 = access %2, 1u, 1u
    %10:f16 = mul %4, %9
    %11:f16 = mul %5, %7
    %12:f16 = mul %5, %8
    %13:f16 = mul %4, %6
    %14:vec2<f16> = construct %10, %11
    %15:vec2<f16> = construct %12, %13
    %16:mat2x2<f16> = construct %14, %15
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat3x3f) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(
            ty.mat3x3<f32>(), spirv::BuiltinFn::kInverse,
            b.Construct(ty.mat3x3<f32>(), b.Splat(ty.vec3<f32>(), 10_f),
                        b.Splat(ty.vec3<f32>(), 20_f), b.Splat(ty.vec3<f32>(), 30_f)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f32> = construct vec3<f32>(10.0f), vec3<f32>(20.0f), vec3<f32>(30.0f)
    %3:mat3x3<f32> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f32> = construct vec3<f32>(10.0f), vec3<f32>(20.0f), vec3<f32>(30.0f)
    %3:f32 = determinant %2
    %4:f32 = div 1.0f, %3
    %5:f32 = access %2, 0u, 0u
    %6:f32 = access %2, 0u, 1u
    %7:f32 = access %2, 0u, 2u
    %8:f32 = access %2, 1u, 0u
    %9:f32 = access %2, 1u, 1u
    %10:f32 = access %2, 1u, 2u
    %11:f32 = access %2, 2u, 0u
    %12:f32 = access %2, 2u, 1u
    %13:f32 = access %2, 2u, 2u
    %14:f32 = mul %9, %13
    %15:f32 = mul %10, %12
    %16:f32 = sub %14, %15
    %17:f32 = mul %7, %12
    %18:f32 = mul %6, %13
    %19:f32 = sub %17, %18
    %20:f32 = mul %6, %10
    %21:f32 = mul %7, %9
    %22:f32 = sub %20, %21
    %23:f32 = mul %10, %11
    %24:f32 = mul %8, %13
    %25:f32 = sub %23, %24
    %26:f32 = mul %5, %13
    %27:f32 = mul %7, %11
    %28:f32 = sub %26, %27
    %29:f32 = mul %7, %8
    %30:f32 = mul %5, %10
    %31:f32 = sub %29, %30
    %32:f32 = mul %8, %12
    %33:f32 = mul %9, %11
    %34:f32 = sub %32, %33
    %35:f32 = mul %6, %11
    %36:f32 = mul %5, %12
    %37:f32 = sub %35, %36
    %38:f32 = mul %5, %9
    %39:f32 = mul %6, %8
    %40:f32 = sub %38, %39
    %41:vec3<f32> = construct %16, %19, %22
    %42:vec3<f32> = construct %25, %28, %31
    %43:vec3<f32> = construct %34, %37, %40
    %44:mat3x3<f32> = construct %41, %42, %43
    %45:mat3x3<f32> = mul %4, %44
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat3x3h) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(
            ty.mat3x3<f16>(), spirv::BuiltinFn::kInverse,
            b.Construct(ty.mat3x3<f16>(), b.Splat(ty.vec3<f16>(), 10_h),
                        b.Splat(ty.vec3<f16>(), 20_h), b.Splat(ty.vec3<f16>(), 30_h)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f16> = construct vec3<f16>(10.0h), vec3<f16>(20.0h), vec3<f16>(30.0h)
    %3:mat3x3<f16> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f16> = construct vec3<f16>(10.0h), vec3<f16>(20.0h), vec3<f16>(30.0h)
    %3:f16 = determinant %2
    %4:f16 = div 1.0h, %3
    %5:f16 = access %2, 0u, 0u
    %6:f16 = access %2, 0u, 1u
    %7:f16 = access %2, 0u, 2u
    %8:f16 = access %2, 1u, 0u
    %9:f16 = access %2, 1u, 1u
    %10:f16 = access %2, 1u, 2u
    %11:f16 = access %2, 2u, 0u
    %12:f16 = access %2, 2u, 1u
    %13:f16 = access %2, 2u, 2u
    %14:f16 = mul %9, %13
    %15:f16 = mul %10, %12
    %16:f16 = sub %14, %15
    %17:f16 = mul %7, %12
    %18:f16 = mul %6, %13
    %19:f16 = sub %17, %18
    %20:f16 = mul %6, %10
    %21:f16 = mul %7, %9
    %22:f16 = sub %20, %21
    %23:f16 = mul %10, %11
    %24:f16 = mul %8, %13
    %25:f16 = sub %23, %24
    %26:f16 = mul %5, %13
    %27:f16 = mul %7, %11
    %28:f16 = sub %26, %27
    %29:f16 = mul %7, %8
    %30:f16 = mul %5, %10
    %31:f16 = sub %29, %30
    %32:f16 = mul %8, %12
    %33:f16 = mul %9, %11
    %34:f16 = sub %32, %33
    %35:f16 = mul %6, %11
    %36:f16 = mul %5, %12
    %37:f16 = sub %35, %36
    %38:f16 = mul %5, %9
    %39:f16 = mul %6, %8
    %40:f16 = sub %38, %39
    %41:vec3<f16> = construct %16, %19, %22
    %42:vec3<f16> = construct %25, %28, %31
    %43:vec3<f16> = construct %34, %37, %40
    %44:mat3x3<f16> = construct %41, %42, %43
    %45:mat3x3<f16> = mul %4, %44
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat4x4f) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(
            ty.mat4x4<f32>(), spirv::BuiltinFn::kInverse,
            b.Construct(ty.mat4x4<f32>(), b.Splat(ty.vec4<f32>(), 10_f),
                        b.Splat(ty.vec4<f32>(), 20_f), b.Splat(ty.vec4<f32>(), 30_f),
                        b.Splat(ty.vec4<f32>(), 40_f)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f32> = construct vec4<f32>(10.0f), vec4<f32>(20.0f), vec4<f32>(30.0f), vec4<f32>(40.0f)
    %3:mat4x4<f32> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f32> = construct vec4<f32>(10.0f), vec4<f32>(20.0f), vec4<f32>(30.0f), vec4<f32>(40.0f)
    %3:f32 = determinant %2
    %4:f32 = div 1.0f, %3
    %5:f32 = access %2, 0u, 0u
    %6:f32 = access %2, 0u, 1u
    %7:f32 = access %2, 0u, 2u
    %8:f32 = access %2, 0u, 3u
    %9:f32 = access %2, 1u, 0u
    %10:f32 = access %2, 1u, 1u
    %11:f32 = access %2, 1u, 2u
    %12:f32 = access %2, 1u, 3u
    %13:f32 = access %2, 2u, 0u
    %14:f32 = access %2, 2u, 1u
    %15:f32 = access %2, 2u, 2u
    %16:f32 = access %2, 2u, 3u
    %17:f32 = access %2, 3u, 0u
    %18:f32 = access %2, 3u, 1u
    %19:f32 = access %2, 3u, 2u
    %20:f32 = access %2, 3u, 3u
    %21:f32 = mul %15, %20
    %22:f32 = mul %16, %19
    %23:f32 = sub %21, %22
    %24:f32 = mul %14, %20
    %25:f32 = mul %16, %18
    %26:f32 = sub %24, %25
    %27:f32 = mul %14, %19
    %28:f32 = mul %15, %18
    %29:f32 = sub %27, %28
    %30:f32 = mul %11, %20
    %31:f32 = mul %12, %19
    %32:f32 = sub %30, %31
    %33:f32 = mul %10, %20
    %34:f32 = mul %12, %18
    %35:f32 = sub %33, %34
    %36:f32 = mul %10, %19
    %37:f32 = mul %11, %18
    %38:f32 = sub %36, %37
    %39:f32 = mul %11, %16
    %40:f32 = mul %12, %15
    %41:f32 = sub %39, %40
    %42:f32 = mul %10, %16
    %43:f32 = mul %12, %14
    %44:f32 = sub %42, %43
    %45:f32 = mul %10, %15
    %46:f32 = mul %11, %14
    %47:f32 = sub %45, %46
    %48:f32 = mul %13, %20
    %49:f32 = mul %16, %17
    %50:f32 = sub %48, %49
    %51:f32 = mul %13, %19
    %52:f32 = mul %15, %17
    %53:f32 = sub %51, %52
    %54:f32 = mul %9, %20
    %55:f32 = mul %12, %17
    %56:f32 = sub %54, %55
    %57:f32 = mul %9, %19
    %58:f32 = mul %11, %17
    %59:f32 = sub %57, %58
    %60:f32 = mul %9, %16
    %61:f32 = mul %12, %13
    %62:f32 = sub %60, %61
    %63:f32 = mul %9, %15
    %64:f32 = mul %11, %13
    %65:f32 = sub %63, %64
    %66:f32 = mul %13, %18
    %67:f32 = mul %14, %17
    %68:f32 = sub %66, %67
    %69:f32 = mul %9, %18
    %70:f32 = mul %10, %17
    %71:f32 = sub %69, %70
    %72:f32 = mul %9, %14
    %73:f32 = mul %10, %13
    %74:f32 = sub %72, %73
    %75:f32 = negation %6
    %76:f32 = mul %10, %23
    %77:f32 = mul %11, %26
    %78:f32 = mul %12, %29
    %79:f32 = sub %76, %77
    %80:f32 = add %79, %78
    %81:f32 = mul %75, %23
    %82:f32 = mul %7, %26
    %83:f32 = mul %8, %29
    %84:f32 = add %81, %82
    %85:f32 = sub %84, %83
    %86:f32 = mul %6, %32
    %87:f32 = mul %7, %35
    %88:f32 = mul %8, %38
    %89:f32 = sub %86, %87
    %90:f32 = add %89, %88
    %91:f32 = mul %75, %41
    %92:f32 = mul %7, %44
    %93:f32 = mul %8, %47
    %94:f32 = add %91, %92
    %95:f32 = sub %94, %93
    %96:f32 = negation %9
    %97:f32 = negation %5
    %98:f32 = mul %96, %23
    %99:f32 = mul %11, %50
    %100:f32 = mul %12, %53
    %101:f32 = add %98, %99
    %102:f32 = sub %101, %100
    %103:f32 = mul %5, %23
    %104:f32 = mul %7, %50
    %105:f32 = mul %8, %53
    %106:f32 = sub %103, %104
    %107:f32 = add %106, %105
    %108:f32 = mul %97, %32
    %109:f32 = mul %7, %56
    %110:f32 = mul %8, %59
    %111:f32 = add %108, %109
    %112:f32 = sub %111, %110
    %113:f32 = mul %5, %41
    %114:f32 = mul %7, %62
    %115:f32 = mul %8, %65
    %116:f32 = sub %113, %114
    %117:f32 = add %116, %115
    %118:f32 = mul %9, %26
    %119:f32 = mul %10, %50
    %120:f32 = mul %12, %68
    %121:f32 = sub %118, %119
    %122:f32 = add %121, %120
    %123:f32 = mul %97, %26
    %124:f32 = mul %6, %50
    %125:f32 = mul %8, %68
    %126:f32 = add %123, %124
    %127:f32 = sub %126, %125
    %128:f32 = mul %5, %35
    %129:f32 = mul %6, %56
    %130:f32 = mul %8, %71
    %131:f32 = sub %128, %129
    %132:f32 = add %131, %130
    %133:f32 = mul %97, %44
    %134:f32 = mul %6, %62
    %135:f32 = mul %8, %74
    %136:f32 = add %133, %134
    %137:f32 = sub %136, %135
    %138:f32 = mul %96, %29
    %139:f32 = mul %10, %53
    %140:f32 = mul %11, %68
    %141:f32 = add %138, %139
    %142:f32 = sub %141, %140
    %143:f32 = mul %5, %29
    %144:f32 = mul %6, %53
    %145:f32 = mul %7, %68
    %146:f32 = sub %143, %144
    %147:f32 = add %146, %145
    %148:f32 = mul %97, %38
    %149:f32 = mul %6, %59
    %150:f32 = mul %7, %71
    %151:f32 = add %148, %149
    %152:f32 = sub %151, %150
    %153:f32 = mul %5, %47
    %154:f32 = mul %6, %65
    %155:f32 = mul %7, %74
    %156:f32 = sub %153, %154
    %157:f32 = add %156, %155
    %158:vec3<f32> = construct %80, %85, %90, %95
    %159:vec3<f32> = construct %102, %107, %112, %117
    %160:vec3<f32> = construct %122, %127, %132, %137
    %161:vec3<f32> = construct %142, %147, %152, %157
    %162:mat4x4<f32> = construct %158, %159, %160, %161
    %163:mat4x4<f32> = mul %4, %162
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat4x4h) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(
            ty.mat4x4<f16>(), spirv::BuiltinFn::kInverse,
            b.Construct(ty.mat4x4<f16>(), b.Splat(ty.vec4<f16>(), 10_h),
                        b.Splat(ty.vec4<f16>(), 20_h), b.Splat(ty.vec4<f16>(), 30_h),
                        b.Splat(ty.vec4<f16>(), 40_h)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f16> = construct vec4<f16>(10.0h), vec4<f16>(20.0h), vec4<f16>(30.0h), vec4<f16>(40.0h)
    %3:mat4x4<f16> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f16> = construct vec4<f16>(10.0h), vec4<f16>(20.0h), vec4<f16>(30.0h), vec4<f16>(40.0h)
    %3:f16 = determinant %2
    %4:f16 = div 1.0h, %3
    %5:f16 = access %2, 0u, 0u
    %6:f16 = access %2, 0u, 1u
    %7:f16 = access %2, 0u, 2u
    %8:f16 = access %2, 0u, 3u
    %9:f16 = access %2, 1u, 0u
    %10:f16 = access %2, 1u, 1u
    %11:f16 = access %2, 1u, 2u
    %12:f16 = access %2, 1u, 3u
    %13:f16 = access %2, 2u, 0u
    %14:f16 = access %2, 2u, 1u
    %15:f16 = access %2, 2u, 2u
    %16:f16 = access %2, 2u, 3u
    %17:f16 = access %2, 3u, 0u
    %18:f16 = access %2, 3u, 1u
    %19:f16 = access %2, 3u, 2u
    %20:f16 = access %2, 3u, 3u
    %21:f16 = mul %15, %20
    %22:f16 = mul %16, %19
    %23:f16 = sub %21, %22
    %24:f16 = mul %14, %20
    %25:f16 = mul %16, %18
    %26:f16 = sub %24, %25
    %27:f16 = mul %14, %19
    %28:f16 = mul %15, %18
    %29:f16 = sub %27, %28
    %30:f16 = mul %11, %20
    %31:f16 = mul %12, %19
    %32:f16 = sub %30, %31
    %33:f16 = mul %10, %20
    %34:f16 = mul %12, %18
    %35:f16 = sub %33, %34
    %36:f16 = mul %10, %19
    %37:f16 = mul %11, %18
    %38:f16 = sub %36, %37
    %39:f16 = mul %11, %16
    %40:f16 = mul %12, %15
    %41:f16 = sub %39, %40
    %42:f16 = mul %10, %16
    %43:f16 = mul %12, %14
    %44:f16 = sub %42, %43
    %45:f16 = mul %10, %15
    %46:f16 = mul %11, %14
    %47:f16 = sub %45, %46
    %48:f16 = mul %13, %20
    %49:f16 = mul %16, %17
    %50:f16 = sub %48, %49
    %51:f16 = mul %13, %19
    %52:f16 = mul %15, %17
    %53:f16 = sub %51, %52
    %54:f16 = mul %9, %20
    %55:f16 = mul %12, %17
    %56:f16 = sub %54, %55
    %57:f16 = mul %9, %19
    %58:f16 = mul %11, %17
    %59:f16 = sub %57, %58
    %60:f16 = mul %9, %16
    %61:f16 = mul %12, %13
    %62:f16 = sub %60, %61
    %63:f16 = mul %9, %15
    %64:f16 = mul %11, %13
    %65:f16 = sub %63, %64
    %66:f16 = mul %13, %18
    %67:f16 = mul %14, %17
    %68:f16 = sub %66, %67
    %69:f16 = mul %9, %18
    %70:f16 = mul %10, %17
    %71:f16 = sub %69, %70
    %72:f16 = mul %9, %14
    %73:f16 = mul %10, %13
    %74:f16 = sub %72, %73
    %75:f16 = negation %6
    %76:f16 = mul %10, %23
    %77:f16 = mul %11, %26
    %78:f16 = mul %12, %29
    %79:f16 = sub %76, %77
    %80:f16 = add %79, %78
    %81:f16 = mul %75, %23
    %82:f16 = mul %7, %26
    %83:f16 = mul %8, %29
    %84:f16 = add %81, %82
    %85:f16 = sub %84, %83
    %86:f16 = mul %6, %32
    %87:f16 = mul %7, %35
    %88:f16 = mul %8, %38
    %89:f16 = sub %86, %87
    %90:f16 = add %89, %88
    %91:f16 = mul %75, %41
    %92:f16 = mul %7, %44
    %93:f16 = mul %8, %47
    %94:f16 = add %91, %92
    %95:f16 = sub %94, %93
    %96:f16 = negation %9
    %97:f16 = negation %5
    %98:f16 = mul %96, %23
    %99:f16 = mul %11, %50
    %100:f16 = mul %12, %53
    %101:f16 = add %98, %99
    %102:f16 = sub %101, %100
    %103:f16 = mul %5, %23
    %104:f16 = mul %7, %50
    %105:f16 = mul %8, %53
    %106:f16 = sub %103, %104
    %107:f16 = add %106, %105
    %108:f16 = mul %97, %32
    %109:f16 = mul %7, %56
    %110:f16 = mul %8, %59
    %111:f16 = add %108, %109
    %112:f16 = sub %111, %110
    %113:f16 = mul %5, %41
    %114:f16 = mul %7, %62
    %115:f16 = mul %8, %65
    %116:f16 = sub %113, %114
    %117:f16 = add %116, %115
    %118:f16 = mul %9, %26
    %119:f16 = mul %10, %50
    %120:f16 = mul %12, %68
    %121:f16 = sub %118, %119
    %122:f16 = add %121, %120
    %123:f16 = mul %97, %26
    %124:f16 = mul %6, %50
    %125:f16 = mul %8, %68
    %126:f16 = add %123, %124
    %127:f16 = sub %126, %125
    %128:f16 = mul %5, %35
    %129:f16 = mul %6, %56
    %130:f16 = mul %8, %71
    %131:f16 = sub %128, %129
    %132:f16 = add %131, %130
    %133:f16 = mul %97, %44
    %134:f16 = mul %6, %62
    %135:f16 = mul %8, %74
    %136:f16 = add %133, %134
    %137:f16 = sub %136, %135
    %138:f16 = mul %96, %29
    %139:f16 = mul %10, %53
    %140:f16 = mul %11, %68
    %141:f16 = add %138, %139
    %142:f16 = sub %141, %140
    %143:f16 = mul %5, %29
    %144:f16 = mul %6, %53
    %145:f16 = mul %7, %68
    %146:f16 = sub %143, %144
    %147:f16 = add %146, %145
    %148:f16 = mul %97, %38
    %149:f16 = mul %6, %59
    %150:f16 = mul %7, %71
    %151:f16 = add %148, %149
    %152:f16 = sub %151, %150
    %153:f16 = mul %5, %47
    %154:f16 = mul %6, %65
    %155:f16 = mul %7, %74
    %156:f16 = sub %153, %154
    %157:f16 = add %156, %155
    %158:vec3<f16> = construct %80, %85, %90, %95
    %159:vec3<f16> = construct %102, %107, %112, %117
    %160:vec3<f16> = construct %122, %127, %132, %137
    %161:vec3<f16> = construct %142, %147, %152, %157
    %162:mat4x4<f16> = construct %158, %159, %160, %161
    %163:mat4x4<f16> = mul %4, %162
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_UnsignedArg) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Constant(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.sign<i32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = sign %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_UnsignedResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Constant(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.sign<u32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = sign 10i
    %3:u32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_UnsignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Constant(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.sign<u32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = sign %2
    %4:u32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_SignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Constant(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.sign<i32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = sign 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_UnsignedArg) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<u32>(), (10_u)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.sign<i32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = sign %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_UnsignedResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.sign<u32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = sign vec2<i32>(10i)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_UnsignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.sign<u32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = sign %2
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_SignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kSign,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.sign<i32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = sign vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

struct SpirvReaderParams {
    spirv::BuiltinFn fn;
    std::string spv_name;
    std::string wgsl_name;
};
[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, SpirvReaderParams c) {
    out << c.spv_name;
    return out;
}

using SpirvReader_BuiltinsTest_OneParamSigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, UnsignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( %2
    %4:u32 = bitcast %3
    %5:vec2<i32> = bitcast vec2<u32>(10u)
    %6:vec2<i32> = )" +
                  params.wgsl_name + R"( %5
    %7:vec2<u32> = bitcast %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, UnsignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( %2
    %4:vec2<i32> = bitcast vec2<u32>(10u)
    %5:vec2<i32> = )" +
                  params.wgsl_name + R"( %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 10i
    %3:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, SignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 10i
    %3:u32 = bitcast %2
    %4:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i)
    %5:vec2<u32> = bitcast %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsTest_OneParamSigned,
                         ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kAbs, "abs", "abs"},
                                           SpirvReaderParams{spirv::BuiltinFn::kFindSMsb,
                                                             "find_s_msb", "firstLeadingBit"}));

using SpirvReader_BuiltinsTest_OneParamUnsigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, UnsignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 10u
    %3:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, UnsignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 10u
    %3:i32 = bitcast %2
    %4:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u)
    %5:vec2<i32> = bitcast %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( %2
    %4:i32 = bitcast %3
    %5:vec2<u32> = bitcast vec2<i32>(10i)
    %6:vec2<u32> = )" +
                  params.wgsl_name + R"( %5
    %7:vec2<i32> = bitcast %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, SignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( %2
    %4:vec2<u32> = bitcast vec2<i32>(10i)
    %5:vec2<u32> = )" +
                  params.wgsl_name + R"( %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsTest_OneParamUnsigned,
                         ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kFindUMsb,
                                                             "find_u_msb", "firstLeadingBit"}));

using SpirvReader_BuiltinsTest_TwoParamSigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, UnsignedToUnsigned) {
    auto& params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 10_u, 15_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<u32>(), 15_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u, 15u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u), vec2<u32>(15u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = bitcast 15u
    %4:i32 = )" + params.wgsl_name +
                  R"( %2, %3
    %5:u32 = bitcast %4
    %6:vec2<i32> = bitcast vec2<u32>(10u)
    %7:vec2<i32> = bitcast vec2<u32>(15u)
    %8:vec2<i32> = )" +
                  params.wgsl_name + R"( %6, %7
    %9:vec2<u32> = bitcast %8
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_i, 15_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<i32>(), 15_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i, 15i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i), vec2<i32>(15i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 10i, 15i
    %3:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i), vec2<i32>(15i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, MixedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i, 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( 10i, %2
    %4:u32 = bitcast %3
    %5:vec2<i32> = bitcast vec2<u32>(10u)
    %6:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i), %5
    %7:vec2<u32> = bitcast %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, MixedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u, 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( %2, 10i
    %4:vec2<i32> = bitcast vec2<u32>(10u)
    %5:vec2<i32> = )" +
                  params.wgsl_name + R"( %4, vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_BuiltinsTest_TwoParamSigned,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kSMax, "s_max", "max"},
                      SpirvReaderParams{spirv::BuiltinFn::kSMin, "s_min", "min"}));

using SpirvReader_BuiltinsTest_TwoParamUnsigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, UnsignedToUnsigned) {
    auto& params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 10_u, 15_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<u32>(), 15_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u, 15u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u), vec2<u32>(15u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 10u, 15u
    %3:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u), vec2<u32>(15u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_i, 15_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<i32>(), 15_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i, 15i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i), vec2<i32>(15i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 15i
    %4:u32 = )" + params.wgsl_name +
                  R"( %2, %3
    %5:i32 = bitcast %4
    %6:vec2<u32> = bitcast vec2<i32>(10i)
    %7:vec2<u32> = bitcast vec2<i32>(15i)
    %8:vec2<u32> = )" +
                  params.wgsl_name + R"( %6, %7
    %9:vec2<i32> = bitcast %8
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, MixedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i, 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( %2, 10u
    %4:vec2<u32> = bitcast vec2<i32>(10i)
    %5:vec2<u32> = )" +
                  params.wgsl_name + R"( %4, vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, MixedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u, 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( 10u, %2
    %4:i32 = bitcast %3
    %5:vec2<u32> = bitcast vec2<i32>(10i)
    %6:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u), %5
    %7:vec2<i32> = bitcast %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_BuiltinsTest_TwoParamUnsigned,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kUMax, "u_max", "max"},
                      SpirvReaderParams{spirv::BuiltinFn::kUMin, "u_min", "min"}));

TEST_F(SpirvReader_BuiltinsTest, SClamp_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSClamp,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_u,
                                               15_u, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kSClamp, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<u32>(), 15_u),
            b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_clamp<u32> 10u, 15u, 10u
    %3:vec2<u32> = spirv.s_clamp<u32> vec2<u32>(10u), vec2<u32>(15u), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = bitcast 15u
    %4:i32 = bitcast 10u
    %5:i32 = clamp %2, %3, %4
    %6:u32 = bitcast %5
    %7:vec2<i32> = bitcast vec2<u32>(10u)
    %8:vec2<i32> = bitcast vec2<u32>(15u)
    %9:vec2<i32> = bitcast vec2<u32>(10u)
    %10:vec2<i32> = clamp %7, %8, %9
    %11:vec2<u32> = bitcast %10
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SClamp_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSClamp,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_i,
                                               15_i, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kSClamp, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<i32>(), 15_i),
            b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_clamp<i32> 10i, 15i, 10i
    %3:vec2<i32> = spirv.s_clamp<i32> vec2<i32>(10i), vec2<i32>(15i), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = clamp 10i, 15i, 10i
    %3:vec2<i32> = clamp vec2<i32>(10i), vec2<i32>(15i), vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SClamp_MixedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSClamp,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_i,
                                               10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kSClamp, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<u32>(), 10_u),
            b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_clamp<u32> 10i, 10u, 10i
    %3:vec2<u32> = spirv.s_clamp<u32> vec2<i32>(10i), vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = clamp 10i, %2, 10i
    %4:u32 = bitcast %3
    %5:vec2<i32> = bitcast vec2<u32>(10u)
    %6:vec2<i32> = clamp vec2<i32>(10i), %5, vec2<i32>(10i)
    %7:vec2<u32> = bitcast %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SClamp_MixedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSClamp,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_u,
                                               10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kSClamp, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<i32>(), 10_i),
            b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_clamp<i32> 10u, 10i, 10u
    %3:vec2<i32> = spirv.s_clamp<i32> vec2<u32>(10u), vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = bitcast 10u
    %4:i32 = clamp %2, 10i, %3
    %5:vec2<i32> = bitcast vec2<u32>(10u)
    %6:vec2<i32> = bitcast vec2<u32>(10u)
    %7:vec2<i32> = clamp %5, vec2<i32>(10i), %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kUClamp,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_u,
                                               15_u, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kUClamp, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<u32>(), 15_u),
            b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.u_clamp<u32> 10u, 15u, 10u
    %3:vec2<u32> = spirv.u_clamp<u32> vec2<u32>(10u), vec2<u32>(15u), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = clamp 10u, 15u, 10u
    %3:vec2<u32> = clamp vec2<u32>(10u), vec2<u32>(15u), vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kUClamp,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_i,
                                               15_i, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kUClamp, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<i32>(), 15_i),
            b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.u_clamp<i32> 10i, 15i, 10i
    %3:vec2<i32> = spirv.u_clamp<i32> vec2<i32>(10i), vec2<i32>(15i), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 15i
    %4:u32 = bitcast 10i
    %5:u32 = clamp %2, %3, %4
    %6:i32 = bitcast %5
    %7:vec2<u32> = bitcast vec2<i32>(10i)
    %8:vec2<u32> = bitcast vec2<i32>(15i)
    %9:vec2<u32> = bitcast vec2<i32>(10i)
    %10:vec2<u32> = clamp %7, %8, %9
    %11:vec2<i32> = bitcast %10
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_MixedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kUClamp,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_i,
                                               10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kUClamp, Vector<const core::type::Type*, 1>{ty.u32()},
            b.Splat(ty.vec2<i32>(), 10_i), b.Splat(ty.vec2<u32>(), 10_u),
            b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.u_clamp<u32> 10i, 10u, 10i
    %3:vec2<u32> = spirv.u_clamp<u32> vec2<i32>(10i), vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 10i
    %4:u32 = clamp %2, 10u, %3
    %5:vec2<u32> = bitcast vec2<i32>(10i)
    %6:vec2<u32> = bitcast vec2<i32>(10i)
    %7:vec2<u32> = clamp %5, vec2<u32>(10u), %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_MixedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kUClamp,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_u,
                                               10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kUClamp, Vector<const core::type::Type*, 1>{ty.i32()},
            b.Splat(ty.vec2<u32>(), 10_u), b.Splat(ty.vec2<i32>(), 10_i),
            b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.u_clamp<i32> 10u, 10i, 10u
    %3:vec2<i32> = spirv.u_clamp<i32> vec2<u32>(10u), vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = clamp 10u, %2, 10u
    %4:i32 = bitcast %3
    %5:vec2<u32> = bitcast vec2<i32>(10i)
    %6:vec2<u32> = clamp vec2<u32>(10u), %5, vec2<u32>(10u)
    %7:vec2<i32> = bitcast %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.find_i_lsb<i32> 10i
    %3:vec2<i32> = spirv.find_i_lsb<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = firstTrailingBit 10i
    %3:vec2<i32> = firstTrailingBit vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.find_i_lsb<u32> 10u
    %3:vec2<u32> = spirv.find_i_lsb<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = firstTrailingBit 10u
    %3:vec2<u32> = firstTrailingBit vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_SignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.find_i_lsb<u32> 10i
    %3:vec2<u32> = spirv.find_i_lsb<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = firstTrailingBit 10i
    %3:u32 = bitcast %2
    %4:vec2<i32> = firstTrailingBit vec2<i32>(10i)
    %5:vec2<u32> = bitcast %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_UnsignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kFindILsb,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.find_i_lsb<i32> 10u
    %3:vec2<i32> = spirv.find_i_lsb<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = firstTrailingBit 10u
    %3:i32 = bitcast %2
    %4:vec2<u32> = firstTrailingBit vec2<u32>(10u)
    %5:vec2<i32> = bitcast %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Refract_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kRefract, 50_f, 60_f, 70_f);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.refract 50.0f, 60.0f, 70.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = construct 50.0f, 0.0f
    %3:vec2<f32> = construct 60.0f, 0.0f
    %4:vec2<f32> = refract %2, %3, 70.0f
    %5:f32 = swizzle %4, x
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Refract_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kRefract,
                                       b.Splat(ty.vec2<f32>(), 10_f), b.Splat(ty.vec2<f32>(), 20_f),
                                       70_f);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.refract vec2<f32>(10.0f), vec2<f32>(20.0f), 70.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = refract vec2<f32>(10.0f), vec2<f32>(20.0f), 70.0f
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FaceForward_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFaceForward, 50_f, 60_f, 70_f);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.face_forward 50.0f, 60.0f, 70.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = negation 50.0f
    %3:f32 = mul 60.0f, 70.0f
    %4:bool = lt %3, 0.0f
    %5:f32 = select %2, 50.0f, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FaceForward_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kFaceForward,
                                       b.Splat(ty.vec2<f32>(), 10_f), b.Splat(ty.vec2<f32>(), 20_f),
                                       b.Splat(ty.vec2<f32>(), 30_f));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.face_forward vec2<f32>(10.0f), vec2<f32>(20.0f), vec2<f32>(30.0f)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = faceForward vec2<f32>(10.0f), vec2<f32>(20.0f), vec2<f32>(30.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Reflect_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kReflect, 50_f, 60_f);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.reflect 50.0f, 60.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = mul 50.0f, 60.0f
    %3:f32 = mul %2, 60.0f
    %4:f32 = mul %3, 2.0f
    %5:f32 = sub 50.0f, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Reflect_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kReflect,
                                       b.Splat(ty.vec2<f32>(), 10_f),
                                       b.Splat(ty.vec2<f32>(), 20_f));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.reflect vec2<f32>(10.0f), vec2<f32>(20.0f)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = reflect vec2<f32>(10.0f), vec2<f32>(20.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Ldexp_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kLdexp, 50_f, 10_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.ldexp 50.0f, 10i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = ldexp 50.0f, 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Ldexp_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kLdexp, 50_f, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.ldexp 50.0f, 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:f32 = ldexp 50.0f, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Ldexp_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kLdexp,
                                       b.Splat(ty.vec2<f32>(), 50_f),
                                       b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.ldexp vec2<f32>(50.0f), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<f32> = ldexp vec2<f32>(50.0f), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
TEST_F(SpirvReader_BuiltinsTest, Ldexp_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kLdexp,
                                       b.Splat(ty.vec2<f32>(), 50_f),
                                       b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.ldexp vec2<f32>(50.0f), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = ldexp vec2<f32>(50.0f), vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Modf_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, f32>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kModf, 50_f, v);
        b.Let(b.Multiply(ty.f32(), res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:f32 = spirv.modf 50.0f, %2
    %4:f32 = mul %3, %3
    %5:f32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:__modf_result_f32 = modf 50.0f
    %4:f32 = access %3, 1u
    store %2, %4
    %5:f32 = access %3, 0u
    %6:f32 = mul %5, %5
    %7:f32 = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Modf_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, vec2<f32>>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kModf,
                                                   b.Splat(ty.vec2<f32>(), 50_f), v);
        b.Let(b.Multiply(ty.vec2<f32>(), res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<f32>, read_write> = var undef
    %3:vec2<f32> = spirv.modf vec2<f32>(50.0f), %2
    %4:vec2<f32> = mul %3, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__modf_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  whole:vec2<f32> @offset(8)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<f32>, read_write> = var undef
    %3:__modf_result_vec2_f32 = modf vec2<f32>(50.0f)
    %4:vec2<f32> = access %3, 1u
    store %2, %4
    %5:vec2<f32> = access %3, 0u
    %6:vec2<f32> = mul %5, %5
    %7:vec2<f32> = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, i32>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFrexp, 50_f, v);
        b.Let(b.Multiply(ty.f32(), res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:f32 = spirv.frexp 50.0f, %2
    %4:f32 = mul %3, %3
    %5:f32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:__frexp_result_f32 = frexp 50.0f
    %4:i32 = access %3, 1u
    store %2, %4
    %5:f32 = access %3, 0u
    %6:f32 = mul %5, %5
    %7:f32 = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_ScalarUnSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, u32>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFrexp, 50_f, v);
        b.Let(b.Multiply(ty.f32(), res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    %3:f32 = spirv.frexp 50.0f, %2
    %4:f32 = mul %3, %3
    %5:f32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    %3:__frexp_result_f32 = frexp 50.0f
    %4:i32 = access %3, 1u
    %5:u32 = bitcast %4
    store %2, %5
    %6:f32 = access %3, 0u
    %7:f32 = mul %6, %6
    %8:f32 = let %7
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, vec2<i32>>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kFrexp,
                                                   b.Splat(ty.vec2<f32>(), 50_f), v);
        b.Let(b.Multiply(ty.vec2<f32>(), res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<i32>, read_write> = var undef
    %3:vec2<f32> = spirv.frexp vec2<f32>(50.0f), %2
    %4:vec2<f32> = mul %3, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<i32>, read_write> = var undef
    %3:__frexp_result_vec2_f32 = frexp vec2<f32>(50.0f)
    %4:vec2<i32> = access %3, 1u
    store %2, %4
    %5:vec2<f32> = access %3, 0u
    %6:vec2<f32> = mul %5, %5
    %7:vec2<f32> = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, vec2<u32>>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kFrexp,
                                                   b.Splat(ty.vec2<f32>(), 50_f), v);
        b.Let(b.Multiply(ty.vec2<f32>(), res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:vec2<f32> = spirv.frexp vec2<f32>(50.0f), %2
    %4:vec2<f32> = mul %3, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:__frexp_result_vec2_f32 = frexp vec2<f32>(50.0f)
    %4:vec2<i32> = access %3, 1u
    %5:vec2<u32> = bitcast %4
    store %2, %5
    %6:vec2<f32> = access %3, 0u
    %7:vec2<f32> = mul %6, %6
    %8:vec2<f32> = let %7
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = countOneBits 10u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_UnsignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = countOneBits 10u
    %3:i32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_SignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.u32()}, 10_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 10i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = countOneBits 10i
    %3:u32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.i32()}, 10_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 10i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = countOneBits 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = countOneBits vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_UnsignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<u32>(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = countOneBits vec2<u32>(10u)
    %3:vec2<i32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_SignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = countOneBits vec2<i32>(10i)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitCount,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat(ty.vec2<i32>(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = countOneBits vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Int_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldInsert, 10_i, 20_i,
                                       10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 10i, 20i, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = insertBits 10i, 20i, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Int_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldInsert, 10_i, 20_i,
                                       10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 10i, 20i, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:i32 = insertBits 10i, 20i, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_IntVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<i32>>(10_i), b.Splat<vec2<i32>>(20_i), 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(10i), vec2<i32>(20i), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = insertBits vec2<i32>(10i), vec2<i32>(20i), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_IntVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<i32>>(10_i), b.Splat<vec2<i32>>(20_i), 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(10i), vec2<i32>(20i), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:vec2<i32> = insertBits vec2<i32>(10i), vec2<i32>(20i), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Uint_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldInsert, 10_u, 20_u,
                                       10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = insertBits 10u, 20u, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Uint_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldInsert, 10_u, 20_u,
                                       10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:u32 = insertBits 10u, 20u, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_UintVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<u32>>(10_u), b.Splat<vec2<u32>>(20_u), 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u), vec2<u32>(20u), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = insertBits vec2<u32>(10u), vec2<u32>(20u), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_UintVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<u32>>(10_u), b.Splat<vec2<u32>>(20_u), 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u), vec2<u32>(20u), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:vec2<u32> = insertBits vec2<u32>(10u), vec2<u32>(20u), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Uint_SignedOffsetAndUnsignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldInsert, 10_u, 20_u,
                                       10_i, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10i, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = insertBits 10u, 20u, %2, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Int_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldSExtract, 10_i, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = extractBits 10i, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Int_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldSExtract, 10_i, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:i32 = extractBits 10i, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_IntVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = extractBits vec2<i32>(10i), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_IntVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:vec2<i32> = extractBits vec2<i32>(10i), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_IntVector_SignedOffsetAndUnsignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_i, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i), 10i, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:vec2<i32> = extractBits vec2<i32>(10i), %2, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Uint_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldSExtract, 10_u, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = extractBits %2, 10u, 20u
    %4:u32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Uint_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldSExtract, 10_u, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:u32 = bitcast 10i
    %4:u32 = bitcast 20i
    %5:i32 = extractBits %2, %3, %4
    %6:u32 = bitcast %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_UintVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = extractBits %2, 10u, 20u
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_UintVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:u32 = bitcast 10i
    %4:u32 = bitcast 20i
    %5:vec2<i32> = extractBits %2, %3, %4
    %6:vec2<u32> = bitcast %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_UintVector_SignedOffsetAndUnsignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_i, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u), 10i, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:u32 = bitcast 10i
    %4:vec2<i32> = extractBits %2, %3, 20u
    %5:vec2<u32> = bitcast %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Uint_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldUExtract, 10_u, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = extractBits 10u, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Uint_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldUExtract, 10_u, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:u32 = extractBits 10u, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_UintVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = extractBits vec2<u32>(10u), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_UintVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 20i
    %4:vec2<u32> = extractBits vec2<u32>(10u), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_UintVector_UnsignedOffsetAndSignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_u, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u), 10u, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 20i
    %3:vec2<u32> = extractBits vec2<u32>(10u), 10u, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Int_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldUExtract, 10_i, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = extractBits %2, 10u, 20u
    %4:i32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Int_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldUExtract, 10_i, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:u32 = bitcast 10i
    %4:u32 = bitcast 20i
    %5:u32 = extractBits %2, %3, %4
    %6:i32 = bitcast %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_IntVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(10i)
    %3:vec2<u32> = extractBits %2, 10u, 20u
    %4:vec2<i32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_IntVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(10i)
    %3:u32 = bitcast 10i
    %4:u32 = bitcast 20i
    %5:vec2<u32> = extractBits %2, %3, %4
    %6:vec2<i32> = bitcast %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_IntVector_UnsignedOffsetAndSignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_u, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i), 10u, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(10i)
    %3:u32 = bitcast 20i
    %4:vec2<u32> = extractBits %2, 10u, %3
    %5:vec2<i32> = bitcast %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

struct BinaryCase {
    spirv::BuiltinFn fn;
    std::string ir;
};

using SpirvReader_BuiltinsMixedSignTest = core::ir::transform::TransformTestWithParam<BinaryCase>;

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 50_i, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.ir + R"(<i32> 50i, 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.ir +
                  R"( 50i, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_u, 50_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.ir + R"(<i32> 10u, 50i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 50i
    %3:u32 = )" + params.ir +
                  R"( 10u, %2
    %4:i32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.ir + R"(<i32> 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.ir +
                  R"( 10u, 20u
    %3:i32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 50_i, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.ir + R"(<u32> 50i, 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.ir +
                  R"( 50i, %2
    %4:u32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 10_u, 50_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.ir + R"(<u32> 10u, 50i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 50i
    %3:u32 = )" + params.ir +
                  R"( 10u, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 50_i, 60_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.ir + R"(<u32> 50i, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.ir +
                  R"( 50i, 60i
    %3:u32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat<vec2<i32>>(50_i), b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> vec2<i32>(50i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = )" +
                  params.ir +
                  R"( vec2<i32>(50i), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat<vec2<u32>>(10_u), b.Splat<vec2<i32>>(50_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> vec2<u32>(10u), vec2<i32>(50i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(50i)
    %3:vec2<u32> = )" +
                  params.ir +
                  R"( vec2<u32>(10u), %2
    %4:vec2<i32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat<vec2<u32>>(10_u), b.Splat<vec2<u32>>(20_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> vec2<u32>(10u), vec2<u32>(20u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = )" +
                  params.ir +
                  R"( vec2<u32>(10u), vec2<u32>(20u)
    %3:vec2<i32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat<vec2<i32>>(50_i), b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> vec2<i32>(50i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = )" +
                  params.ir +
                  R"( vec2<i32>(50i), %2
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat<vec2<u32>>(10_u), b.Splat<vec2<i32>>(50_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> vec2<u32>(10u), vec2<i32>(50i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(50i)
    %3:vec2<u32> = )" +
                  params.ir +
                  R"( vec2<u32>(10u), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat<vec2<i32>>(50_i), b.Splat<vec2<i32>>(60_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> vec2<i32>(50i), vec2<i32>(60i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  params.ir +
                  R"( vec2<i32>(50i), vec2<i32>(60i)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsMixedSignTest,
                         testing::Values(BinaryCase{spirv::BuiltinFn::kAdd, "add"},
                                         BinaryCase{spirv::BuiltinFn::kSub, "sub"},
                                         BinaryCase{spirv::BuiltinFn::kMul, "mul"}));

struct SignedBinaryCase {
    spirv::BuiltinFn fn;
    std::string ir;
    std::string wgsl;
};

using SpirvReader_BuiltinsSignedTest =
    core::ir::transform::TransformTestWithParam<SignedBinaryCase>;

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 50_i, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.ir + R"(<i32> 50i, 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl +
                  R"( 50i, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_u, 50_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.ir + R"(<i32> 10u, 50i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl +
                  R"( %2, 50i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<const core::type::Type*, 1>{ty.i32()}, 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.ir + R"(<i32> 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = bitcast 20u
    %4:i32 = )" + params.wgsl +
                  R"( %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 50_i, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.ir + R"(<u32> 50i, 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl +
                  R"( 50i, %2
    %4:u32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 10_u, 50_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.ir + R"(<u32> 10u, 50i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:i32 = )" + params.wgsl +
                  R"( %2, 50i
    %4:u32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<const core::type::Type*, 1>{ty.u32()}, 50_i, 60_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.ir + R"(<u32> 50i, 60i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl +
                  R"( 50i, 60i
    %3:u32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat<vec2<i32>>(50_i), b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> vec2<i32>(50i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = )" +
                  params.wgsl +
                  R"( vec2<i32>(50i), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat<vec2<u32>>(10_u), b.Splat<vec2<i32>>(50_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> vec2<u32>(10u), vec2<i32>(50i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = )" +
                  params.wgsl +
                  R"( %2, vec2<i32>(50i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.i32()},
                                               b.Splat<vec2<u32>>(10_u), b.Splat<vec2<u32>>(20_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> vec2<u32>(10u), vec2<u32>(20u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = bitcast vec2<u32>(20u)
    %4:vec2<i32> = )" +
                  params.wgsl +
                  R"( %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat<vec2<i32>>(50_i), b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> vec2<i32>(50i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = )" +
                  params.wgsl +
                  R"( vec2<i32>(50i), %2
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat<vec2<u32>>(10_u), b.Splat<vec2<i32>>(50_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> vec2<u32>(10u), vec2<i32>(50i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<i32> = )" +
                  params.wgsl +
                  R"( %2, vec2<i32>(50i)
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn,
                                               Vector<const core::type::Type*, 1>{ty.u32()},
                                               b.Splat<vec2<i32>>(50_i), b.Splat<vec2<i32>>(60_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> vec2<i32>(50i), vec2<i32>(60i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  params.wgsl +
                  R"( vec2<i32>(50i), vec2<i32>(60i)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsSignedTest,
                         testing::Values(SignedBinaryCase{spirv::BuiltinFn::kSDiv, "s_div", "div"},
                                         SignedBinaryCase{spirv::BuiltinFn::kSMod, "s_mod",
                                                          "mod"}));

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kConvertFToS,
                                               Vector{ty.i32()}, 10_f);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.convert_f_to_s<i32> 10.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = convert 10.0f
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kConvertFToS,
                                               Vector{ty.u32()}, 10_f);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.convert_f_to_s<u32> 10.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = convert 10.0f
    %3:u32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kConvertFToS,
                                               Vector{ty.i32()}, b.Splat<vec2<f32>>(10_f));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.convert_f_to_s<i32> vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = convert vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kConvertFToS,
                                               Vector{ty.u32()}, b.Splat<vec2<f32>>(10_f));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.convert_f_to_s<u32> vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = convert vec2<f32>(10.0f)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertSToF,
                                               Vector{ty.f32()}, 10_i);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_s_to_f<f32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = convert 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertSToF,
                                               Vector{ty.f32()}, 10_u);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_s_to_f<f32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 10u
    %3:f32 = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kConvertSToF,
                                               Vector{ty.f32()}, b.Splat<vec2<i32>>(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_s_to_f<f32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = convert vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kConvertSToF,
                                               Vector{ty.f32()}, b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_s_to_f<f32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(10u)
    %3:vec2<f32> = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertUToF,
                                               Vector{ty.f32()}, 10_i);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_u_to_f<f32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 10i
    %3:f32 = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertUToF,
                                               Vector{ty.f32()}, 10_u);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_u_to_f<f32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = convert 10u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kConvertUToF,
                                               Vector{ty.f32()}, b.Splat<vec2<i32>>(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_u_to_f<f32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(10i)
    %3:vec2<f32> = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kConvertUToF,
                                               Vector{ty.f32()}, b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_u_to_f<f32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = convert vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

using SpirvReader_BitwiseTest = core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn, Vector{ty.i32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn, Vector{ty.i32()}, 1_i, 8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 1i, 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:i32 = )" + params.wgsl_name +
                  R"( 1i, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn, Vector{ty.i32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = )" + params.wgsl_name +
                  R"( 8u, %2
    %4:i32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), params.fn, Vector{ty.i32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 8u, 9u
    %3:i32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn, Vector{ty.u32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn, Vector{ty.u32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = )" + params.wgsl_name +
                  R"( 8u, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn, Vector{ty.u32()}, 1_i, 8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 1i, 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:i32 = )" + params.wgsl_name +
                  R"( 1i, %2
    %4:u32 = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), params.fn, Vector{ty.u32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 1i, 2i
    %3:u32 = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn, Vector{ty.i32()},
                                               b.Splat<vec2<i32>>(1_i), b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn, Vector{ty.i32()},
                                               b.Splat<vec2<i32>>(1_i), b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(1i), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn, Vector{ty.i32()},
                                               b.Splat<vec2<u32>>(8_u), b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(8u), %2
    %4:vec2<i32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), params.fn, Vector{ty.i32()},
                                               b.Splat<vec2<u32>>(8_u), b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    %3:vec2<i32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn, Vector{ty.u32()},
                                               b.Splat<vec2<u32>>(8_u), b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn, Vector{ty.u32()},
                                               b.Splat<vec2<u32>>(8_u), b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(8u), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn, Vector{ty.u32()},
                                               b.Splat<vec2<i32>>(1_i), b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(1i), %2
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), params.fn, Vector{ty.u32()},
                                               b.Splat<vec2<i32>>(1_i), b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_BitwiseTest,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kBitwiseAnd, "bitwise_and", "and"},
                      SpirvReaderParams{spirv::BuiltinFn::kBitwiseOr, "bitwise_or", "or"},
                      SpirvReaderParams{spirv::BuiltinFn::kBitwiseXor, "bitwise_xor", "xor"}));

using SpirvReader_IntegerTest = core::ir::transform::TransformTestWithParam<SpirvReaderParams>;
TEST_P(SpirvReader_IntegerTest, Scalar_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + param.wgsl_name +
                  R"( 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Scalar_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 1_i, 8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 1i, 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:bool = )" + param.wgsl_name +
                  R"( 1i, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Scalar_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:bool = )" + param.wgsl_name +
                  R"( 8u, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Scalar_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + param.wgsl_name +
                  R"( 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<i32>>(1_i),
                                       b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<i32>>(1_i),
                                       b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<i32>(1i), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<u32>>(8_u),
                                       b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<u32>(8u), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<u32>>(8_u),
                                       b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_IntegerTest,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kEqual, "equal", "eq"},
                      SpirvReaderParams{spirv::BuiltinFn::kNotEqual, "not_equal", "neq"}));

using SpirvReader_SignedIntegerTest =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;
TEST_P(SpirvReader_SignedIntegerTest, Scalar_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + param.wgsl_name +
                  R"( 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Scalar_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 1_i, 8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 1i, 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:bool = )" + param.wgsl_name +
                  R"( 1i, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Scalar_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:bool = )" + param.wgsl_name +
                  R"( %2, 1i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Scalar_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:i32 = bitcast 9u
    %4:bool = )" + param.wgsl_name +
                  R"( %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<i32>>(1_i),
                                       b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<i32>>(1_i),
                                       b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<i32>(1i), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<u32>>(8_u),
                                       b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<bool> = )" +
                  param.wgsl_name + R"( %2, vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<u32>>(8_u),
                                       b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<i32> = bitcast vec2<u32>(9u)
    %4:vec2<bool> = )" +
                  param.wgsl_name + R"( %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_SignedIntegerTest,
    ::testing::Values(
        SpirvReaderParams{spirv::BuiltinFn::kSGreaterThan, "s_greater_than", "gt"},
        SpirvReaderParams{spirv::BuiltinFn::kSGreaterThanEqual, "s_greater_than_equal", "gte"},
        SpirvReaderParams{spirv::BuiltinFn::kSLessThan, "s_less_than", "lt"},
        SpirvReaderParams{spirv::BuiltinFn::kSLessThanEqual, "s_less_than_equal", "lte"}));

using SpirvReader_UnsignedIntegerTest =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;
TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = bitcast 2i
    %4:bool = )" + param.wgsl_name +
                  R"( %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 1_i, 8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 1i, 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:bool = )" + param.wgsl_name +
                  R"( %2, 8u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:bool = )" + param.wgsl_name +
                  R"( 8u, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
               param.spv_name + R"( 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + param.wgsl_name +
                  R"( 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<i32>>(1_i),
                                       b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = bitcast vec2<i32>(2i)
    %4:vec2<bool> = )" +
                  param.wgsl_name + R"( %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<i32>>(1_i),
                                       b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<bool> = )" +
                  param.wgsl_name + R"( %2, vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<u32>>(8_u),
                                       b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<u32>(8u), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, b.Splat<vec2<u32>>(8_u),
                                       b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
               param.spv_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  param.wgsl_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_UnsignedIntegerTest,
    ::testing::Values(
        SpirvReaderParams{spirv::BuiltinFn::kUGreaterThan, "u_greater_than", "gt"},
        SpirvReaderParams{spirv::BuiltinFn::kUGreaterThanEqual, "u_greater_than_equal", "gte"},
        SpirvReaderParams{spirv::BuiltinFn::kULessThan, "u_less_than", "lt"},
        SpirvReaderParams{spirv::BuiltinFn::kULessThanEqual, "u_less_than_equal", "lte"}));

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_left_logical<u32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = shl 8u, 9u
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_left_logical<u32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = shl 8u, %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, 1_i, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_left_logical<u32> 1i, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = shl 1i, 9u
    %3:u32 = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_left_logical<u32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 2i
    %3:i32 = shl 1i, %2
    %4:u32 = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_left_logical<i32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = shl 8u, 9u
    %3:i32 = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_left_logical<i32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = shl 8u, %2
    %4:i32 = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, 1_i, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_left_logical<i32> 1i, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = shl 1i, 9u
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_left_logical<i32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 2i
    %3:i32 = shl 1i, %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_left_logical<u32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = shl vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_left_logical<u32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = shl vec2<u32>(8u), %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_left_logical<u32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = shl vec2<i32>(1i), vec2<u32>(9u)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_left_logical<u32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(2i)
    %3:vec2<i32> = shl vec2<i32>(1i), %2
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_left_logical<i32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = shl vec2<u32>(8u), vec2<u32>(9u)
    %3:vec2<i32> = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_left_logical<i32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = shl vec2<u32>(8u), %2
    %4:vec2<i32> = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_left_logical<i32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = shl vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_left_logical<i32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(2i)
    %3:vec2<i32> = shl vec2<i32>(1i), %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_logical<u32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = shr 8u, 9u
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_logical<u32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = shr 8u, %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, 1_i, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_logical<u32> 1i, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = shr %2, 9u
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_logical<u32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = bitcast 2i
    %4:u32 = shr %2, %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_logical<i32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = shr 8u, 9u
    %3:i32 = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_logical<i32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = shr 8u, %2
    %4:i32 = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, 1_i, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_logical<i32> 1i, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = shr %2, 9u
    %4:i32 = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_logical<i32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:u32 = bitcast 2i
    %4:u32 = shr %2, %3
    %5:i32 = bitcast %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_logical<u32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = shr vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_logical<u32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = shr vec2<u32>(8u), %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_logical<u32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = shr %2, vec2<u32>(9u)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.u32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_logical<u32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = bitcast vec2<i32>(2i)
    %4:vec2<u32> = shr %2, %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_logical<i32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = shr vec2<u32>(8u), vec2<u32>(9u)
    %3:vec2<i32> = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<u32>>(8_u),
                                               b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_logical<i32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = shr vec2<u32>(8u), %2
    %4:vec2<i32> = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_logical<i32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = shr %2, vec2<u32>(9u)
    %4:vec2<i32> = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector{ty.i32()}, b.Splat<vec2<i32>>(1_i),
                                               b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_logical<i32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(1i)
    %3:vec2<u32> = bitcast vec2<i32>(2i)
    %4:vec2<u32> = shr %2, %3
    %5:vec2<i32> = bitcast %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.u32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_arithmetic<u32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:i32 = shr %2, 9u
    %4:u32 = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.u32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_arithmetic<u32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:u32 = bitcast 1i
    %4:i32 = shr %2, %3
    %5:u32 = bitcast %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.u32()}, 1_i, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_arithmetic<u32> 1i, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = shr 1i, 9u
    %3:u32 = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.u32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.shift_right_arithmetic<u32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 2i
    %3:i32 = shr 1i, %2
    %4:u32 = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.i32()}, 8_u, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_arithmetic<i32> 8u, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:i32 = shr %2, 9u
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.i32()}, 8_u, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_arithmetic<i32> 8u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:u32 = bitcast 1i
    %4:i32 = shr %2, %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.i32()}, 1_i, 9_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_arithmetic<i32> 1i, 9u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = shr 1i, 9u
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector{ty.i32()}, 1_i, 2_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.shift_right_arithmetic<i32> 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 2i
    %3:i32 = shr 1i, %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.u32()},
            b.Splat<vec2<u32>>(8_u), b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_arithmetic<u32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<i32> = shr %2, vec2<u32>(9u)
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.u32()},
            b.Splat<vec2<u32>>(8_u), b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_arithmetic<u32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<u32> = bitcast vec2<i32>(1i)
    %4:vec2<i32> = shr %2, %3
    %5:vec2<u32> = bitcast %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.u32()},
            b.Splat<vec2<i32>>(1_i), b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_arithmetic<u32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = shr vec2<i32>(1i), vec2<u32>(9u)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<u32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.u32()},
            b.Splat<vec2<i32>>(1_i), b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.shift_right_arithmetic<u32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(2i)
    %3:vec2<i32> = shr vec2<i32>(1i), %2
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.i32()},
            b.Splat<vec2<u32>>(8_u), b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_arithmetic<i32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<i32> = shr %2, vec2<u32>(9u)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.i32()},
            b.Splat<vec2<u32>>(8_u), b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_arithmetic<i32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<u32> = bitcast vec2<i32>(1i)
    %4:vec2<i32> = shr %2, %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.i32()},
            b.Splat<vec2<i32>>(1_i), b.Splat<vec2<u32>>(9_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_arithmetic<i32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = shr vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2<i32>(), spirv::BuiltinFn::kShiftRightArithmetic, Vector{ty.i32()},
            b.Splat<vec2<i32>>(1_i), b.Splat<vec2<i32>>(2_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.shift_right_arithmetic<i32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast vec2<i32>(2i)
    %3:vec2<i32> = shr vec2<i32>(1i), %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Scalar_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kNot, Vector{ty.i32()},
                                               1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.not<i32> 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = complement 1i
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Scalar_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kNot, Vector{ty.u32()},
                                               1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.not<u32> 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = complement 1i
    %3:u32 = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Scalar_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kNot, Vector{ty.i32()},
                                               8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.not<i32> 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = complement 8u
    %3:i32 = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kNot,
                                               Vector{ty.i32()}, b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.not<i32> vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = complement vec2<i32>(1i)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kNot,
                                               Vector{ty.u32()}, b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.not<u32> vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = complement vec2<i32>(1i)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kNot,
                                               Vector{ty.i32()}, b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.not<i32> vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = complement vec2<u32>(8u)
    %3:vec2<i32> = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Unsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kNot,
                                               Vector{ty.u32()}, b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.not<u32> vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = complement vec2<u32>(8u)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.i32()}, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_negate<i32> 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = negation 1i
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.u32()}, 1_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_negate<u32> 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = negation 1i
    %3:u32 = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.i32()}, 8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_negate<i32> 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:i32 = negation %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Unsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.u32()}, 8_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_negate<u32> 8u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast 8u
    %3:i32 = negation %2
    %4:u32 = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.i32()}, b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.s_negate<i32> vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = negation vec2<i32>(1i)
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.u32()}, b.Splat<vec2<i32>>(1_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.s_negate<u32> vec2<i32>(1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = negation vec2<i32>(1i)
    %3:vec2<u32> = bitcast %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<i32>(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.i32()}, b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.s_negate<i32> vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<i32> = negation %2
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Unsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2<u32>(), spirv::BuiltinFn::kSNegate,
                                               Vector{ty.u32()}, b.Splat<vec2<u32>>(8_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.s_negate<u32> vec2<u32>(8u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast vec2<u32>(8u)
    %3:vec2<i32> = negation %2
    %4:vec2<u32> = bitcast %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FMod_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFMod, 1_f, 2_f);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.f_mod 1.0f, 2.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = div 1.0f, 2.0f
    %3:f32 = floor %2
    %4:f32 = mul 2.0f, %3
    %5:f32 = sub 1.0f, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FMod_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kFMod,
                                       b.Splat<vec2<f32>>(1_f), b.Splat<vec2<f32>>(2_f));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.f_mod vec2<f32>(1.0f), vec2<f32>(2.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = div vec2<f32>(1.0f), vec2<f32>(2.0f)
    %3:vec2<f32> = floor %2
    %4:vec2<f32> = mul vec2<f32>(2.0f), %3
    %5:vec2<f32> = sub vec2<f32>(1.0f), %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Select_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kSelect, true, 1_f, 2_f);
        b.Return(ep);
    });
    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.select true, 1.0f, 2.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = select 2.0f, 1.0f, true
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
TEST_F(SpirvReader_BuiltinsTest, Select_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<f32>(), spirv::BuiltinFn::kSelect,
                                       b.Splat<vec2<bool>>(false), b.Splat<vec2<f32>>(1_f),
                                       b.Splat<vec2<f32>>(2_f));
        b.Return(ep);
    });
    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.select vec2<bool>(false), vec2<f32>(1.0f), vec2<f32>(2.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = select vec2<f32>(2.0f), vec2<f32>(1.0f), vec2<bool>(false)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, OuterProduct_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        // Call the OuterProduct builtin function
        b.Call<spirv::ir::BuiltinCall>(ty.mat2x4<f32>(), spirv::BuiltinFn::kOuterProduct,
                                       b.Splat<vec4<f32>>(1_f), b.Splat<vec2<f32>>(2_f));
        b.Return(ep);
    });

    // Expected SPIR-V source code
    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x4<f32> = spirv.outer_product vec4<f32>(1.0f), vec2<f32>(2.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    // Run the test
    Run(Builtins);

    // Updated expected expanded SPIR-V code after lowering
    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = access vec2<f32>(2.0f), 0u
    %3:f32 = access vec4<f32>(1.0f), 0u
    %4:f32 = mul %3, %2
    %5:f32 = access vec4<f32>(1.0f), 1u
    %6:f32 = mul %5, %2
    %7:f32 = access vec4<f32>(1.0f), 2u
    %8:f32 = mul %7, %2
    %9:f32 = access vec4<f32>(1.0f), 3u
    %10:f32 = mul %9, %2
    %11:vec4<f32> = construct %4, %6, %8, %10
    %12:f32 = access vec2<f32>(2.0f), 1u
    %13:f32 = access vec4<f32>(1.0f), 0u
    %14:f32 = mul %13, %12
    %15:f32 = access vec4<f32>(1.0f), 1u
    %16:f32 = mul %15, %12
    %17:f32 = access vec4<f32>(1.0f), 2u
    %18:f32 = mul %17, %12
    %19:f32 = access vec4<f32>(1.0f), 3u
    %20:f32 = mul %19, %12
    %21:vec4<f32> = construct %14, %16, %18, %20
    %22:mat2x4<f32> = construct %11, %21
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
