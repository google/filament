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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

TEST_F(SpirvWriterTest, Constant_Bool) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", true);
        b.Var<private_, read_write>("v", false);
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%true = OpConstantTrue %bool");
    EXPECT_INST("%false = OpConstantFalse %bool");
}

TEST_F(SpirvWriterTest, Constant_I32) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", i32(42));
        b.Var<private_, read_write>("v", i32(-1));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int_42 = OpConstant %int 42");
    EXPECT_INST("%int_n1 = OpConstant %int -1");
}

TEST_F(SpirvWriterTest, Constant_U32) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", u32(42));
        b.Var<private_, read_write>("v", u32(4000000000));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%uint_42 = OpConstant %uint 42");
    EXPECT_INST("%uint_4000000000 = OpConstant %uint 4000000000");
}

TEST_F(SpirvWriterTest, Constant_F32) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", f32(42));
        b.Var<private_, read_write>("v", f32(-1));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%float_42 = OpConstant %float 42");
    EXPECT_INST("%float_n1 = OpConstant %float -1");
}

TEST_F(SpirvWriterTest, Constant_F16) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", f16(42));
        b.Var<private_, read_write>("v", f16(-1));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%half_0x1_5p_5 = OpConstant %half 0x1.5p+5");
    EXPECT_INST("%half_n0x1p_0 = OpConstant %half -0x1p+0");
}

TEST_F(SpirvWriterTest, Constant_Vec4Bool) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", b.Composite<vec4<bool>>(true, false, false, true));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantComposite %v4bool %true %false %false %true");
}

TEST_F(SpirvWriterTest, Constant_Vec2i) {
    b.Append(b.ir.root_block,
             [&] { b.Var<private_, read_write>("v", b.Composite<vec2<i32>>(42_i, -1_i)); });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantComposite %v2int %int_42 %int_n1");
}

TEST_F(SpirvWriterTest, Constant_Vec3u) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", b.Composite<vec3<u32>>(42_u, 0_u, 4000000000_u));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantComposite %v3uint %uint_42 %uint_0 %uint_4000000000");
}

TEST_F(SpirvWriterTest, Constant_Vec4f) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", b.Composite<vec4<f32>>(42_f, 0_f, 0.25_f, -1_f));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantComposite %v4float %float_42 %float_0 %float_0_25 %float_n1");
}

TEST_F(SpirvWriterTest, Constant_Vec2h) {
    b.Append(b.ir.root_block,
             [&] { b.Var<private_, read_write>("v", b.Composite<vec2<f16>>(42_h, 0.25_h)); });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantComposite %v2half %half_0x1_5p_5 %half_0x1pn2");
}

TEST_F(SpirvWriterTest, Constant_Mat2x3f) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v",
                                    b.Composite<mat2x3<f32>>(  //
                                        b.Composite<vec3<f32>>(42_f, -1_f, 0.25_f),
                                        b.Composite<vec3<f32>>(-42_f, 0_f, -0.25_f)));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
   %float_42 = OpConstant %float 42
   %float_n1 = OpConstant %float -1
 %float_0_25 = OpConstant %float 0.25
          %7 = OpConstantComposite %v3float %float_42 %float_n1 %float_0_25
  %float_n42 = OpConstant %float -42
    %float_0 = OpConstant %float 0
%float_n0_25 = OpConstant %float -0.25
         %11 = OpConstantComposite %v3float %float_n42 %float_0 %float_n0_25
          %6 = OpConstantComposite %mat2v3float %7 %11
)");
}

TEST_F(SpirvWriterTest, Constant_Mat4x2h) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", b.Composite<mat4x2<f16>>(                 //
                                             b.Composite<vec2<f16>>(42_h, -1_h),   //
                                             b.Composite<vec2<f16>>(0_h, 0.25_h),  //
                                             b.Composite<vec2<f16>>(-42_h, 1_h),   //
                                             b.Composite<vec2<f16>>(0.5_h, f16(-0))));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
%half_0x1_5p_5 = OpConstant %half 0x1.5p+5
%half_n0x1p_0 = OpConstant %half -0x1p+0
          %7 = OpConstantComposite %v2half %half_0x1_5p_5 %half_n0x1p_0
%half_0x0p_0 = OpConstant %half 0x0p+0
%half_0x1pn2 = OpConstant %half 0x1p-2
         %10 = OpConstantComposite %v2half %half_0x0p_0 %half_0x1pn2
%half_n0x1_5p_5 = OpConstant %half -0x1.5p+5
%half_0x1p_0 = OpConstant %half 0x1p+0
         %13 = OpConstantComposite %v2half %half_n0x1_5p_5 %half_0x1p_0
%half_0x1pn1 = OpConstant %half 0x1p-1
         %16 = OpConstantComposite %v2half %half_0x1pn1 %half_0x0p_0
          %6 = OpConstantComposite %mat4v2half %7 %10 %13 %16
)");
}

TEST_F(SpirvWriterTest, Constant_Array_I32) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", b.Composite<array<i32, 4>>(1_i, 2_i, 3_i, 4_i));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantComposite %_arr_int_uint_4 %int_1 %int_2 %int_3 %int_4");
}

TEST_F(SpirvWriterTest, Constant_Array_Array_I32) {
    b.Append(b.ir.root_block, [&] {
        auto* inner = b.Composite<array<i32, 4>>(1_i, 2_i, 3_i, 4_i);
        b.Var<private_, read_write>(
            "v", b.Composite<array<array<i32, 4>, 4>>(inner, inner, inner, inner));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpConstantComposite %_arr_int_uint_4 %int_1 %int_2 %int_3 %int_4
          %8 = OpConstantComposite %_arr__arr_int_uint_4_uint_4 %9 %9 %9 %9
)");
}

TEST_F(SpirvWriterTest, Constant_Array_LargeAllZero) {
    b.Append(b.ir.root_block,
             [&] { b.Var<private_, read_write>("v", b.Zero<array<i32, 65535>>()); });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantNull %_arr_int_uint_65535");
}

TEST_F(SpirvWriterTest, Constant_Struct) {
    b.Append(b.ir.root_block, [&] {
        auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                  {mod.symbols.New("a"), ty.i32()},
                                                                  {mod.symbols.New("b"), ty.u32()},
                                                                  {mod.symbols.New("c"), ty.f32()},
                                                              });
        b.Var<private_, read_write>("v", b.Composite(str_ty, 1_i, 2_u, 3_f));
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(" = OpConstantComposite %MyStruct %int_1 %uint_2 %float_3");
}

// Test that we do not emit the same constant more than once.
TEST_F(SpirvWriterTest, Constant_Deduplicate) {
    b.Append(b.ir.root_block, [&] {
        b.Var<private_, read_write>("v", 42_i);
        b.Var<private_, read_write>("v", 42_i);
        b.Var<private_, read_write>("v", 42_i);
    });
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%int_42 = OpConstant %int 42");
}

}  // namespace
}  // namespace tint::spirv::writer
