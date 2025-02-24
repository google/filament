// Copyright 2020 The Dawn & Tint Authors
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
#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest_Assign = TestHelper;

TEST_F(HlslASTPrinterTest_Assign, Emit_Assign) {
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.i32())),
             Decl(Var("rhs", ty.i32())),
             Assign("lhs", "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void fn() {
  int lhs = 0;
  int rhs = 0;
  lhs = rhs;
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Vector_Assign_LetIndex) {
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.vec3<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Let("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void set_vector_element(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void fn() {
  float3 lhs = float3(0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  uint index = 0u;
  set_vector_element(lhs, index, rhs);
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Vector_Assign_ConstIndex) {
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.vec3<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Const("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void fn() {
  float3 lhs = float3(0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  lhs[0u] = rhs;
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Vector_Assign_DynamicIndex) {
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.vec3<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Var("index", ty.u32())),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void set_vector_element(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void fn() {
  float3 lhs = float3(0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  uint index = 0u;
  set_vector_element(lhs, index, rhs);
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_Assign_Vector_LetIndex) {
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.vec2<f32>())),
             Decl(Let("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void set_matrix_column(inout float4x2 mat, int col, float2 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
    case 2: mat[2] = val; break;
    case 3: mat[3] = val; break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2 rhs = float2(0.0f, 0.0f);
  uint index = 0u;
  set_matrix_column(lhs, index, rhs);
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_Assign_Vector_ConstIndex) {
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.vec2<f32>())),
             Decl(Const("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2 rhs = float2(0.0f, 0.0f);
  lhs[0u] = rhs;
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_Assign_Vector_DynamicIndex) {
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.vec2<f32>())),
             Decl(Var("index", ty.u32())),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void set_matrix_column(inout float4x2 mat, int col, float2 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
    case 2: mat[2] = val; break;
    case 3: mat[3] = val; break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2 rhs = float2(0.0f, 0.0f);
  uint index = 0u;
  set_matrix_column(lhs, index, rhs);
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_Assign_Scalar_LetIndices) {
    auto* col = IndexAccessor("lhs", "col");
    auto* el = IndexAccessor(col, "row");
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Let("col", ty.u32(), Expr(0_u))),
             Decl(Let("row", ty.u32(), Expr(1_u))),
             Assign(el, "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void set_matrix_scalar(inout float4x2 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xx == int2(0, 1)) ? val.xx : mat[0];
      break;
    case 1:
      mat[1] = (row.xx == int2(0, 1)) ? val.xx : mat[1];
      break;
    case 2:
      mat[2] = (row.xx == int2(0, 1)) ? val.xx : mat[2];
      break;
    case 3:
      mat[3] = (row.xx == int2(0, 1)) ? val.xx : mat[3];
      break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  uint col = 0u;
  uint row = 1u;
  set_matrix_scalar(lhs, col, row, rhs);
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_Assign_Scalar_ConstIndices) {
    auto* col = IndexAccessor("lhs", "col");
    auto* el = IndexAccessor(col, "row");
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Const("col", ty.u32(), Expr(0_u))),
             Decl(Const("row", ty.u32(), Expr(1_u))),
             Assign(el, "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  lhs[0u][1u] = rhs;
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_Assign_Scalar_DynamicIndices) {
    auto* col = IndexAccessor("lhs", "col");
    auto* el = IndexAccessor(col, "row");
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Var("col", ty.u32())),
             Decl(Var("row", ty.u32())),
             Assign(el, "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void set_matrix_scalar(inout float4x2 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xx == int2(0, 1)) ? val.xx : mat[0];
      break;
    case 1:
      mat[1] = (row.xx == int2(0, 1)) ? val.xx : mat[1];
      break;
    case 2:
      mat[2] = (row.xx == int2(0, 1)) ? val.xx : mat[2];
      break;
    case 3:
      mat[3] = (row.xx == int2(0, 1)) ? val.xx : mat[3];
      break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  uint col = 0u;
  uint row = 0u;
  set_matrix_scalar(lhs, col, row, rhs);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Assignment to composites of f16
// See crbug.com/tint/2146
////////////////////////////////////////////////////////////////////////////////
TEST_F(HlslASTPrinterTest_Assign, Emit_Vector_f16_Assign) {
    Enable(wgsl::Extension::kF16);

    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.vec3<f16>())),
             Decl(Var("rhs", ty.f16())),
             Decl(Let("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.Result(),
              R"(void set_vector_element(inout vector<float16_t, 3> vec, int idx, float16_t val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void fn() {
  vector<float16_t, 3> lhs = vector<float16_t, 3>(float16_t(0.0h), float16_t(0.0h), float16_t(0.0h));
  float16_t rhs = float16_t(0.0h);
  uint index = 0u;
  set_vector_element(lhs, index, rhs);
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_f16_Assign_Vector) {
    Enable(wgsl::Extension::kF16);

    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f16>())),
             Decl(Var("rhs", ty.vec2<f16>())),
             Decl(Let("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(
        gen.Result(),
        R"(void set_matrix_column(inout matrix<float16_t, 4, 2> mat, int col, vector<float16_t, 2> val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
    case 2: mat[2] = val; break;
    case 3: mat[3] = val; break;
  }
}

void fn() {
  matrix<float16_t, 4, 2> lhs = matrix<float16_t, 4, 2>(float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h));
  vector<float16_t, 2> rhs = vector<float16_t, 2>(float16_t(0.0h), float16_t(0.0h));
  uint index = 0u;
  set_matrix_column(lhs, index, rhs);
}
)");
}

TEST_F(HlslASTPrinterTest_Assign, Emit_Matrix_f16_Assign_Scalar) {
    Enable(wgsl::Extension::kF16);

    auto* col = IndexAccessor("lhs", "col");
    auto* el = IndexAccessor(col, "row");
    Func("fn", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.mat4x2<f16>())),
             Decl(Var("rhs", ty.f16())),
             Decl(Let("col", ty.u32(), Expr(0_u))),
             Decl(Let("row", ty.u32(), Expr(1_u))),
             Assign(el, "rhs"),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(
        gen.Result(),
        R"(void set_matrix_scalar(inout matrix<float16_t, 4, 2> mat, int col, int row, float16_t val) {
  switch (col) {
    case 0:
      mat[0] = (row.xx == int2(0, 1)) ? val.xx : mat[0];
      break;
    case 1:
      mat[1] = (row.xx == int2(0, 1)) ? val.xx : mat[1];
      break;
    case 2:
      mat[2] = (row.xx == int2(0, 1)) ? val.xx : mat[2];
      break;
    case 3:
      mat[3] = (row.xx == int2(0, 1)) ? val.xx : mat[3];
      break;
  }
}

void fn() {
  matrix<float16_t, 4, 2> lhs = matrix<float16_t, 4, 2>(float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h));
  float16_t rhs = float16_t(0.0h);
  uint col = 0u;
  uint row = 1u;
  set_matrix_scalar(lhs, col, row, rhs);
}
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
