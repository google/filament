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

#include "src/tint/lang/spirv/reader/ast_parser/parse.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::spirv::reader::ast_parser {
namespace {

class SpirvReaderRowMajorMatrixTest : public testing::Test {
  public:
    std::string Run(std::string decorations, std::string types, std::string body) {
        auto spv = test::Assemble(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %foo "foo"
               OpExecutionMode %foo LocalSize 1 1 1
               OpDecorate %buffer DescriptorSet 0
               OpDecorate %buffer Binding 0
               OpDecorate %S Block

          )" + decorations + R"(

         %u32 = OpTypeInt 32 0
       %u32_0 = OpConstant %u32 0
       %u32_1 = OpConstant %u32 1
       %u32_2 = OpConstant %u32 2
       %u32_4 = OpConstant %u32 4

         %f32 = OpTypeFloat 32
       %f32_2 = OpConstant %f32 2.0

       %vec3f = OpTypeVector %f32 3
     %mat2x3f = OpTypeMatrix %vec3f 2
 %arr_mat2x3f = OpTypeArray %mat2x3f %u32_4

           )" + types + R"(

    %_ptr_Storage_f32 = OpTypePointer StorageBuffer %f32
  %_ptr_Storage_vec3f = OpTypePointer StorageBuffer %vec3f
%_ptr_Storage_mat2x3f = OpTypePointer StorageBuffer %mat2x3f
%_ptr_Storage_arr_mat2x3f = OpTypePointer StorageBuffer %arr_mat2x3f
%_ptr_Storage_S = OpTypePointer StorageBuffer %S
      %buffer = OpVariable %_ptr_Storage_S StorageBuffer

        %void = OpTypeVoid
   %func_type = OpTypeFunction %void
         %foo = OpFunction %void None %func_type
  %func_start = OpLabel

           )" + body + R"(

               OpReturn
               OpFunctionEnd
)");
        auto program = Parse(spv, {});
        auto errs = program.Diagnostics().Str();
        EXPECT_TRUE(program.IsValid()) << errs;
        EXPECT_EQ(program.Diagnostics().Count(), 0u) << errs;
        auto result = wgsl::writer::Generate(program);
        EXPECT_EQ(result, Success);
        return "\n" + result->wgsl;
    }
};

TEST_F(SpirvReaderRowMajorMatrixTest, LoadMatrix_DefaultStride) {
    auto result = Run(R"(
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 8
               OpMemberDecorate %S 1 Offset 64)",
                      R"(
          %S = OpTypeStruct %mat2x3f %vec3f)",
                      R"(
      %m_ptr = OpAccessChain %_ptr_Storage_mat2x3f %buffer %u32_0
          %m = OpLoad %mat2x3f %m_ptr)");

    EXPECT_EQ(result, R"(
struct S {
  /* @offset(0) */
  field0 : mat3x2<f32>,
  @size(32)
  padding_0 : u32,
  /* @offset(64) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn foo_1() {
  let x_23 = transpose(x_2.field0);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

TEST_F(SpirvReaderRowMajorMatrixTest, LoadMatrix_CustomStride) {
    auto result = Run(R"(
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 32
               OpMemberDecorate %S 1 Offset 128)",
                      R"(
          %S = OpTypeStruct %mat2x3f %vec3f)",
                      R"(
      %m_ptr = OpAccessChain %_ptr_Storage_mat2x3f %buffer %u32_0
          %m = OpLoad %mat2x3f %m_ptr)");

    EXPECT_EQ(result, R"(
struct strided_arr {
  @size(32)
  el : vec2<f32>,
}

struct S {
  /* @offset(0) */
  field0 : array<strided_arr, 3u>,
  @size(32)
  padding_0 : u32,
  /* @offset(128) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn arr_to_mat3x2_stride_32(arr : array<strided_arr, 3u>) -> mat3x2<f32> {
  return mat3x2<f32>(arr[0u].el, arr[1u].el, arr[2u].el);
}

fn foo_1() {
  let x_23 = transpose(arr_to_mat3x2_stride_32(x_2.field0));
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

TEST_F(SpirvReaderRowMajorMatrixTest, LoadColumn) {
    auto result = Run(R"(
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 8
               OpMemberDecorate %S 1 Offset 64)",
                      R"(
          %S = OpTypeStruct %mat2x3f %vec3f)",
                      R"(
      %c_ptr = OpAccessChain %_ptr_Storage_vec3f %buffer %u32_0 %u32_1
          %c = OpLoad %vec3f %c_ptr)");

    EXPECT_EQ(result, R"(
fn tint_load_row_major_column(tint_from : ptr<storage, mat3x2<f32>, read_write>, tint_idx : u32) -> vec3<f32> {
  return vec3<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx], tint_from[2][tint_idx]);
}

struct S {
  /* @offset(0) */
  field0 : mat3x2<f32>,
  @size(32)
  padding_0 : u32,
  /* @offset(64) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn foo_1() {
  let x_23 = tint_load_row_major_column(&(x_2.field0), u32(1u));
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

TEST_F(SpirvReaderRowMajorMatrixTest, LoadElement) {
    auto result = Run(R"(
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 8
               OpMemberDecorate %S 1 Offset 64)",
                      R"(
          %S = OpTypeStruct %mat2x3f %vec3f)",
                      R"(
      %e_ptr = OpAccessChain %_ptr_Storage_f32 %buffer %u32_0 %u32_1 %u32_2
          %e = OpLoad %f32 %e_ptr)");

    EXPECT_EQ(result, R"(
struct S {
  /* @offset(0) */
  field0 : mat3x2<f32>,
  @size(32)
  padding_0 : u32,
  /* @offset(64) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn foo_1() {
  let x_23 = x_2.field0[2u][1u];
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

TEST_F(SpirvReaderRowMajorMatrixTest, StoreMatrix) {
    auto result = Run(R"(
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 8
               OpMemberDecorate %S 1 Offset 64)",
                      R"(
          %S = OpTypeStruct %mat2x3f %vec3f)",
                      R"(
      %m_ptr = OpAccessChain %_ptr_Storage_mat2x3f %buffer %u32_0
          %m = OpLoad %mat2x3f %m_ptr
        %mul = OpMatrixTimesScalar %mat2x3f %m %f32_2
               OpStore %m_ptr %mul)");

    EXPECT_EQ(result, R"(
struct S {
  /* @offset(0) */
  field0 : mat3x2<f32>,
  @size(32)
  padding_0 : u32,
  /* @offset(64) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn foo_1() {
  x_2.field0 = transpose((transpose(x_2.field0) * 2.0f));
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

TEST_F(SpirvReaderRowMajorMatrixTest, StoreColumn) {
    auto result = Run(R"(
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 8
               OpMemberDecorate %S 1 Offset 64)",
                      R"(
          %S = OpTypeStruct %mat2x3f %vec3f)",
                      R"(
      %c_ptr = OpAccessChain %_ptr_Storage_vec3f %buffer %u32_0 %u32_1
          %c = OpLoad %vec3f %c_ptr
        %mul = OpVectorTimesScalar %vec3f %c %f32_2
               OpStore %c_ptr %mul)");

    EXPECT_EQ(result, R"(
fn tint_load_row_major_column(tint_from : ptr<storage, mat3x2<f32>, read_write>, tint_idx : u32) -> vec3<f32> {
  return vec3<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx], tint_from[2][tint_idx]);
}

fn tint_store_row_major_column(tint_to : ptr<storage, mat3x2<f32>, read_write>, tint_idx : u32, tint_col : vec3<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
  tint_to[2][tint_idx] = tint_col[2];
}

struct S {
  /* @offset(0) */
  field0 : mat3x2<f32>,
  @size(32)
  padding_0 : u32,
  /* @offset(64) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn foo_1() {
  tint_store_row_major_column(&(x_2.field0), u32(1u), (tint_load_row_major_column(&(x_2.field0), u32(1u)) * 2.0f));
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

TEST_F(SpirvReaderRowMajorMatrixTest, StoreElement) {
    auto result = Run(R"(
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 8
               OpMemberDecorate %S 1 Offset 64)",
                      R"(
          %S = OpTypeStruct %mat2x3f %vec3f)",
                      R"(
      %e_ptr = OpAccessChain %_ptr_Storage_f32 %buffer %u32_0 %u32_1 %u32_2
          %e = OpLoad %f32 %e_ptr
        %mul = OpFMul %f32 %e %f32_2
               OpStore %e_ptr %mul)");

    EXPECT_EQ(result, R"(
struct S {
  /* @offset(0) */
  field0 : mat3x2<f32>,
  @size(32)
  padding_0 : u32,
  /* @offset(64) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn foo_1() {
  x_2.field0[2u][1u] = (x_2.field0[2u][1u] * 2.0f);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

TEST_F(SpirvReaderRowMajorMatrixTest, ArrayOfMatrix) {
    auto result = Run(R"(
               OpDecorate %arr_mat2x3f ArrayStride 32
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RowMajor
               OpMemberDecorate %S 0 MatrixStride 8
               OpMemberDecorate %S 1 Offset 128)",
                      R"(
          %S = OpTypeStruct %arr_mat2x3f %vec3f)",
                      R"(
    %arr_ptr = OpAccessChain %_ptr_Storage_arr_mat2x3f %buffer %u32_0
        %arr = OpLoad %arr_mat2x3f %arr_ptr
     %m1_ptr = OpAccessChain %_ptr_Storage_mat2x3f %buffer %u32_0 %u32_1
          %m = OpLoad %mat2x3f %m1_ptr
     %m2_ptr = OpAccessChain %_ptr_Storage_mat2x3f %buffer %u32_0 %u32_2
               OpStore %m2_ptr %m
    %arr_mod = OpCompositeInsert %arr_mat2x3f %m %arr 0
               OpStore %arr_ptr %arr_mod
          )");

    EXPECT_EQ(result, R"(
struct strided_arr {
  @size(32)
  el : mat3x2<f32>,
}

fn tint_transpose_array(tint_from : array<strided_arr, 4u>) -> array<mat2x3<f32>, 4u> {
  var tint_result : array<mat2x3<f32>, 4u>;
  for(var i = 0u; (i < 4u); i++) {
    tint_result[i] = transpose(tint_from[i].el);
  }
  return tint_result;
}

fn tint_transpose_array_1(tint_from : array<mat2x3<f32>, 4u>) -> array<strided_arr, 4u> {
  var tint_result_1 : array<strided_arr, 4u>;
  for(var i_1 = 0u; (i_1 < 4u); i_1++) {
    tint_result_1[i_1].el = transpose(tint_from[i_1]);
  }
  return tint_result_1;
}

alias Arr = array<mat2x3f, 4u>;

struct S {
  /* @offset(0) */
  field0 : array<strided_arr, 4u>,
  /* @offset(128) */
  field1 : vec3f,
}

@group(0) @binding(0) var<storage, read_write> x_2 : S;

fn foo_1() {
  let x_23 = tint_transpose_array(x_2.field0);
  let x_25 = transpose(x_2.field0[1u].el);
  x_2.field0[2u].el = transpose(x_25);
  var x_27_1 = x_23;
  x_27_1[0u] = x_25;
  x_2.field0 = tint_transpose_array_1(x_27_1);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn foo() {
  foo_1();
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
