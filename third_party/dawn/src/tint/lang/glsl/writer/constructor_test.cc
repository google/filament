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

#include "gmock/gmock.h"
#include "src/tint/lang/glsl/writer/helper_test.h"

namespace tint::glsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(GlslWriterTest, Constructor_Type_Float_Large) {
    auto* func = b.Function("a", ty.f32());
    b.Append(func->Block(), [&] {  //
        // Use a number close to 1<<30 but whose decimal representation ends in 0.
        b.Return(func, b.Construct(ty.f32(), f32((1 << 30) - 4)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
float a() {
  return float(1073741824.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Float) {
    auto* func = b.Function("a", ty.f32());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.f32(), -1.2e-5_f));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
float a() {
  return float(-0.00001200000042445026f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_F16_Large) {
    auto* func = b.Function("a", ty.f16());
    b.Append(func->Block(), [&] {  //
        // Use a number close to 1<<16 but whose decimal representation ends in 0.
        b.Return(func, b.Construct(ty.f16(), f16((1 << 15) - 8)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

float16_t a() {
  return float16_t(32752.0hf);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_F16) {
    auto* func = b.Function("a", ty.f16());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.f16(), -1.2e-3_h));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

float16_t a() {
  return float16_t(-0.0011997222900390625hf);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Bool_False) {
    auto* func = b.Function("a", ty.bool_());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.bool_(), false));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a() {
  return bool(false);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Bool_True) {
    auto* func = b.Function("a", ty.bool_());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.bool_(), true));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bool a() {
  return bool(true);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Int) {
    auto* func = b.Function("a", ty.i32());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.i32(), -12345_i));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
int a() {
  return int(-12345);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Uint) {
    auto* func = b.Function("a", ty.u32());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.u32(), 12345_u));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uint a() {
  return uint(12345u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_F32) {
    auto* func = b.Function("a", ty.vec3<f32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.vec3<f32>(), 1.0_f, 2.0_f, 3.0_f));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
vec3 a() {
  return vec3(1.0f, 2.0f, 3.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_F16) {
    auto* func = b.Function("a", ty.vec3<f16>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.vec3<f16>(), 1_h, 2_h, 3_h));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16vec3 a() {
  return f16vec3(1.0hf, 2.0hf, 3.0hf);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_Empty_F32) {
    auto* func = b.Function("a", ty.vec3<f32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.vec3<f32>()));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
vec3 a() {
  return vec3(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_Empty_F16) {
    auto* func = b.Function("a", ty.vec3<f16>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.vec3<f16>()));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16vec3 a() {
  return f16vec3(0.0hf);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_SingleScalar_F32_Literal) {
    auto* func = b.Function("a", ty.vec3<f32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Splat(ty.vec3<f32>(), 2_f));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
vec3 a() {
  return vec3(2.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_SingleScalar_F16_Literal) {
    auto* func = b.Function("a", ty.vec3<f16>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Splat(ty.vec3<f16>(), 2_h));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16vec3 a() {
  return f16vec3(2.0hf);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_SingleScalar_F32_Var) {
    auto* func = b.Function("a", ty.vec3<f32>());
    b.Append(func->Block(), [&] {  //
        auto* v = b.Var("v", 2_f);
        b.Return(func, b.Construct(ty.vec3<f32>(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
vec3 a() {
  float v = 2.0f;
  return vec3(v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_SingleScalar_F16_Var) {
    auto* func = b.Function("a", ty.vec3<f16>());
    b.Append(func->Block(), [&] {  //
        auto* v = b.Var("v", 2_h);
        b.Return(func, b.Construct(ty.vec3<f16>(), v));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16vec3 a() {
  float16_t v = 2.0hf;
  return f16vec3(v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_SingleScalar_Bool) {
    auto* func = b.Function("a", ty.vec3<bool>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Splat(ty.vec3<bool>(), true));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
bvec3 a() {
  return bvec3(true);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_SingleScalar_Int) {
    auto* func = b.Function("a", ty.vec3<i32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Splat(ty.vec3<i32>(), 2_i));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
ivec3 a() {
  return ivec3(2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Vec_SingleScalar_UInt) {
    auto* func = b.Function("a", ty.vec3<u32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.vec3<u32>(), 2_u));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uvec3 a() {
  return uvec3(2u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_F32) {
    auto* func = b.Function("a", ty.mat2x3<f32>());
    b.Append(func->Block(), [&] {  //
        auto* v1 = b.Let("v1", b.Construct(ty.vec3<f32>(), 1_f, 2_f, 3_f));
        auto* v2 = b.Let("v2", b.Construct(ty.vec3<f32>(), 3_f, 4_f, 5_f));
        b.Return(func, b.Construct(ty.mat2x3<f32>(), v1, v2));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
mat2x3 a() {
  vec3 v1 = vec3(1.0f, 2.0f, 3.0f);
  vec3 v2 = vec3(3.0f, 4.0f, 5.0f);
  return mat2x3(v1, v2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_F16) {
    auto* func = b.Function("a", ty.mat2x3<f16>());
    b.Append(func->Block(), [&] {  //
        auto* v1 = b.Let("v1", b.Construct(ty.vec3<f16>(), 1_h, 2_h, 3_h));
        auto* v2 = b.Let("v2", b.Construct(ty.vec3<f16>(), 3_h, 4_h, 5_h));
        b.Return(func, b.Construct(ty.mat2x3<f16>(), v1, v2));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16mat2x3 a() {
  f16vec3 v1 = f16vec3(1.0hf, 2.0hf, 3.0hf);
  f16vec3 v2 = f16vec3(3.0hf, 4.0hf, 5.0hf);
  return f16mat2x3(v1, v2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_Complex_F32) {
    // mat4x4<f32>(
    //     vec4<f32>(2.0f, 3.0f, 4.0f, 8.0f),
    //     vec4<f32>(),
    //     vec4<f32>(7.0f),
    //     vec4<f32>(vec4<f32>(42.0f, 21.0f, 6.0f, -5.0f)),
    //   );
    auto* func = b.Function("a", ty.mat4x4<f32>());
    b.Append(func->Block(), [&] {  //
        auto* v1 = b.Construct(ty.vec4<f32>(), 2.0_f, 3.0_f, 4.0_f, 8.0_f);
        auto* v2 = b.Construct(ty.vec4<f32>());
        auto* v3 = b.Splat(ty.vec4<f32>(), 7.0_f);
        auto* v4 =
            b.Construct(ty.vec4<f32>(), b.Construct(ty.vec4<f32>(), 42.0_f, 21.0_f, 6.0_f, -5.0_f));
        b.Return(func, b.Construct(ty.mat4x4<f32>(), v1, v2, v3, v4));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
mat4 a() {
  vec4 v = vec4(2.0f, 3.0f, 4.0f, 8.0f);
  vec4 v_1 = vec4(0.0f);
  return mat4(v, v_1, vec4(7.0f), vec4(vec4(42.0f, 21.0f, 6.0f, -5.0f)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_Complex_F16) {
    // mat4x4<f16>(
    //     vec4<f16>(2.0h, 3.0h, 4.0h, 8.0h),
    //     vec4<f16>(),
    //     vec4<f16>(7.0h),
    //     vec4<f16>(vec4<f16>(42.0h, 21.0h, 6.0h, -5.0h)),
    //   );
    auto* func = b.Function("a", ty.mat4x4<f16>());
    b.Append(func->Block(), [&] {  //
        auto* v1 = b.Construct(ty.vec4<f16>(), 2.0_h, 3.0_h, 4.0_h, 8.0_h);
        auto* v2 = b.Construct(ty.vec4<f16>());
        auto* v3 = b.Splat(ty.vec4<f16>(), 7.0_h);
        auto* v4 =
            b.Construct(ty.vec4<f16>(), b.Construct(ty.vec4<f16>(), 42.0_h, 21.0_h, 6.0_h, -5.0_h));
        b.Return(func, b.Construct(ty.mat4x4<f16>(), v1, v2, v3, v4));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16mat4 a() {
  f16vec4 v = f16vec4(2.0hf, 3.0hf, 4.0hf, 8.0hf);
  f16vec4 v_1 = f16vec4(0.0hf);
  return f16mat4(v, v_1, f16vec4(7.0hf), f16vec4(f16vec4(42.0hf, 21.0hf, 6.0hf, -5.0hf)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_Empty_F32) {
    auto* func = b.Function("a", ty.mat2x3<f32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.mat2x3<f32>()));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
mat2x3 a() {
  return mat2x3(vec3(0.0f), vec3(0.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_Empty_F16) {
    auto* func = b.Function("a", ty.mat2x3<f16>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.mat2x3<f16>()));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16mat2x3 a() {
  return f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_Identity_F32) {
    // fn f() {
    //     var m_1: mat4x4<f32> = mat4x4<f32>();
    //     var m_2: mat4x4<f32> = mat4x4<f32>(m_1);
    // }

    auto* func = b.Function("a", ty.mat4x4<f32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.mat4x4<f32>(), b.Construct(ty.mat4x4<f32>())));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
mat4 a() {
  return mat4(mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Mat_Identity_F16) {
    // fn f() {
    //     var m_1: mat4x4<f16> = mat4x4<f16>();
    //     var m_2: mat4x4<f16> = mat4x4<f16>(m_1);
    // }

    auto* func = b.Function("a", ty.mat4x4<f16>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.mat4x4<f16>(), b.Construct(ty.mat4x4<f16>())));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require

f16mat4 a() {
  return f16mat4(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Array) {
    auto* func = b.Function("a", ty.array<vec3<f32>, 3>());
    b.Append(func->Block(), [&] {  //
        auto* v1 = b.Let("v1", b.Construct(ty.vec3<f32>(), 1_f, 2_f, 3_f));
        auto* v2 = b.Let("v2", b.Construct(ty.vec3<f32>(), 4_f, 5_f, 6_f));
        auto* v3 = b.Let("v3", b.Construct(ty.vec3<f32>(), 7_f, 8_f, 9_f));
        b.Return(func, b.Construct(ty.array<vec3<f32>, 3>(), v1, v2, v3));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
vec3[3] a() {
  vec3 v1 = vec3(1.0f, 2.0f, 3.0f);
  vec3 v2 = vec3(4.0f, 5.0f, 6.0f);
  vec3 v3 = vec3(7.0f, 8.0f, 9.0f);
  return vec3[3](v1, v2, v3);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Array_Empty) {
    auto* func = b.Function("a", ty.array<vec3<f32>, 3>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.array<vec3<f32>, 3>()));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
vec3[3] a() {
  return vec3[3](vec3(0.0f), vec3(0.0f), vec3(0.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Struct) {
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                    {mod.symbols.New("c"), ty.vec3<i32>()},
                                                });

    auto* func = b.Function("a", str);
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(str, 1_i, 2_f, b.Construct(ty.vec3<i32>(), 3_i, 4_i, 5_i)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct S {
  int a;
  float b;
  ivec3 c;
};

S a() {
  return S(1, 2.0f, ivec3(3, 4, 5));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Constructor_Type_Struct_Empty) {
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                    {mod.symbols.New("c"), ty.vec3<i32>()},
                                                });

    auto* func = b.Function("a", str);
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(str));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct S {
  int a;
  float b;
  ivec3 c;
};

S a() {
  return S(0, 0.0f, ivec3(0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
