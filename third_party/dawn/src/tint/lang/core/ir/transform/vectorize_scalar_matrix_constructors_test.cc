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

#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/matrix.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_VectorizeScalarMatrixConstructorsTest = TransformTest;

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, NoModify_NoOperands) {
    auto* mat = ty.mat3x3<f32>();
    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func():mat3x3<f32> {
  $B1: {
    %2:mat3x3<f32> = construct
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, NoModify_Identity) {
    auto* mat = ty.mat3x3<f32>();
    auto* value = b.FunctionParam("value", mat);
    auto* func = b.Function("foo", mat);
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, value);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%value:mat3x3<f32>):mat3x3<f32> {
  $B1: {
    %3:mat3x3<f32> = construct %value
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, NoModify_Vectors) {
    auto* mat = ty.mat3x3<f32>();
    auto* v1 = b.FunctionParam("v1", mat->ColumnType());
    auto* v2 = b.FunctionParam("v2", mat->ColumnType());
    auto* v3 = b.FunctionParam("v3", mat->ColumnType());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:vec3<f32>, %v2:vec3<f32>, %v3:vec3<f32>):mat3x3<f32> {
  $B1: {
    %5:mat3x3<f32> = construct %v1, %v2, %v3
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat2x2) {
    auto* mat = ty.mat2x2<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32):mat2x2<f32> {
  $B1: {
    %6:mat2x2<f32> = construct %v1, %v2, %v3, %v4
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32):mat2x2<f32> {
  $B1: {
    %6:vec2<f32> = construct %v1, %v2
    %7:vec2<f32> = construct %v3, %v4
    %8:mat2x2<f32> = construct %6, %7
    ret %8
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat2x3) {
    auto* mat = ty.mat2x3<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32):mat2x3<f32> {
  $B1: {
    %8:mat2x3<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32):mat2x3<f32> {
  $B1: {
    %8:vec3<f32> = construct %v1, %v2, %v3
    %9:vec3<f32> = construct %v4, %v5, %v6
    %10:mat2x3<f32> = construct %8, %9
    ret %10
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat2x4) {
    auto* mat = ty.mat2x4<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* v7 = b.FunctionParam("v7", ty.f32());
    auto* v8 = b.FunctionParam("v8", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6, v7, v8});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6, v7, v8);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32):mat2x4<f32> {
  $B1: {
    %10:mat2x4<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6, %v7, %v8
    ret %10
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32):mat2x4<f32> {
  $B1: {
    %10:vec4<f32> = construct %v1, %v2, %v3, %v4
    %11:vec4<f32> = construct %v5, %v6, %v7, %v8
    %12:mat2x4<f32> = construct %10, %11
    ret %12
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat3x2) {
    auto* mat = ty.mat3x2<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32):mat3x2<f32> {
  $B1: {
    %8:mat3x2<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32):mat3x2<f32> {
  $B1: {
    %8:vec2<f32> = construct %v1, %v2
    %9:vec2<f32> = construct %v3, %v4
    %10:vec2<f32> = construct %v5, %v6
    %11:mat3x2<f32> = construct %8, %9, %10
    ret %11
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat3x3) {
    auto* mat = ty.mat3x3<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* v7 = b.FunctionParam("v7", ty.f32());
    auto* v8 = b.FunctionParam("v8", ty.f32());
    auto* v9 = b.FunctionParam("v9", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6, v7, v8, v9});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32):mat3x3<f32> {
  $B1: {
    %11:mat3x3<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6, %v7, %v8, %v9
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32):mat3x3<f32> {
  $B1: {
    %11:vec3<f32> = construct %v1, %v2, %v3
    %12:vec3<f32> = construct %v4, %v5, %v6
    %13:vec3<f32> = construct %v7, %v8, %v9
    %14:mat3x3<f32> = construct %11, %12, %13
    ret %14
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat3x4) {
    auto* mat = ty.mat3x4<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* v7 = b.FunctionParam("v7", ty.f32());
    auto* v8 = b.FunctionParam("v8", ty.f32());
    auto* v9 = b.FunctionParam("v9", ty.f32());
    auto* v10 = b.FunctionParam("v10", ty.f32());
    auto* v11 = b.FunctionParam("v11", ty.f32());
    auto* v12 = b.FunctionParam("v12", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32, %v10:f32, %v11:f32, %v12:f32):mat3x4<f32> {
  $B1: {
    %14:mat3x4<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6, %v7, %v8, %v9, %v10, %v11, %v12
    ret %14
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32, %v10:f32, %v11:f32, %v12:f32):mat3x4<f32> {
  $B1: {
    %14:vec4<f32> = construct %v1, %v2, %v3, %v4
    %15:vec4<f32> = construct %v5, %v6, %v7, %v8
    %16:vec4<f32> = construct %v9, %v10, %v11, %v12
    %17:mat3x4<f32> = construct %14, %15, %16
    ret %17
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat4x2) {
    auto* mat = ty.mat4x2<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* v7 = b.FunctionParam("v7", ty.f32());
    auto* v8 = b.FunctionParam("v8", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6, v7, v8});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6, v7, v8);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32):mat4x2<f32> {
  $B1: {
    %10:mat4x2<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6, %v7, %v8
    ret %10
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32):mat4x2<f32> {
  $B1: {
    %10:vec2<f32> = construct %v1, %v2
    %11:vec2<f32> = construct %v3, %v4
    %12:vec2<f32> = construct %v5, %v6
    %13:vec2<f32> = construct %v7, %v8
    %14:mat4x2<f32> = construct %10, %11, %12, %13
    ret %14
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat4x3) {
    auto* mat = ty.mat4x3<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* v7 = b.FunctionParam("v7", ty.f32());
    auto* v8 = b.FunctionParam("v8", ty.f32());
    auto* v9 = b.FunctionParam("v9", ty.f32());
    auto* v10 = b.FunctionParam("v10", ty.f32());
    auto* v11 = b.FunctionParam("v11", ty.f32());
    auto* v12 = b.FunctionParam("v12", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32, %v10:f32, %v11:f32, %v12:f32):mat4x3<f32> {
  $B1: {
    %14:mat4x3<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6, %v7, %v8, %v9, %v10, %v11, %v12
    ret %14
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32, %v10:f32, %v11:f32, %v12:f32):mat4x3<f32> {
  $B1: {
    %14:vec3<f32> = construct %v1, %v2, %v3
    %15:vec3<f32> = construct %v4, %v5, %v6
    %16:vec3<f32> = construct %v7, %v8, %v9
    %17:vec3<f32> = construct %v10, %v11, %v12
    %18:mat4x3<f32> = construct %14, %15, %16, %17
    ret %18
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat4x4) {
    auto* mat = ty.mat4x4<f32>();
    auto* v1 = b.FunctionParam("v1", ty.f32());
    auto* v2 = b.FunctionParam("v2", ty.f32());
    auto* v3 = b.FunctionParam("v3", ty.f32());
    auto* v4 = b.FunctionParam("v4", ty.f32());
    auto* v5 = b.FunctionParam("v5", ty.f32());
    auto* v6 = b.FunctionParam("v6", ty.f32());
    auto* v7 = b.FunctionParam("v7", ty.f32());
    auto* v8 = b.FunctionParam("v8", ty.f32());
    auto* v9 = b.FunctionParam("v9", ty.f32());
    auto* v10 = b.FunctionParam("v10", ty.f32());
    auto* v11 = b.FunctionParam("v11", ty.f32());
    auto* v12 = b.FunctionParam("v12", ty.f32());
    auto* v13 = b.FunctionParam("v13", ty.f32());
    auto* v14 = b.FunctionParam("v14", ty.f32());
    auto* v15 = b.FunctionParam("v15", ty.f32());
    auto* v16 = b.FunctionParam("v16", ty.f32());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16});
    b.Append(func->Block(), [&] {
        auto* construct =
            b.Construct(mat, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32, %v10:f32, %v11:f32, %v12:f32, %v13:f32, %v14:f32, %v15:f32, %v16:f32):mat4x4<f32> {
  $B1: {
    %18:mat4x4<f32> = construct %v1, %v2, %v3, %v4, %v5, %v6, %v7, %v8, %v9, %v10, %v11, %v12, %v13, %v14, %v15, %v16
    ret %18
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f32, %v2:f32, %v3:f32, %v4:f32, %v5:f32, %v6:f32, %v7:f32, %v8:f32, %v9:f32, %v10:f32, %v11:f32, %v12:f32, %v13:f32, %v14:f32, %v15:f32, %v16:f32):mat4x4<f32> {
  $B1: {
    %18:vec4<f32> = construct %v1, %v2, %v3, %v4
    %19:vec4<f32> = construct %v5, %v6, %v7, %v8
    %20:vec4<f32> = construct %v9, %v10, %v11, %v12
    %21:vec4<f32> = construct %v13, %v14, %v15, %v16
    %22:mat4x4<f32> = construct %18, %19, %20, %21
    ret %22
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VectorizeScalarMatrixConstructorsTest, Mat3x3_F16) {
    auto* mat = ty.mat3x3<f16>();
    auto* v1 = b.FunctionParam("v1", ty.f16());
    auto* v2 = b.FunctionParam("v2", ty.f16());
    auto* v3 = b.FunctionParam("v3", ty.f16());
    auto* v4 = b.FunctionParam("v4", ty.f16());
    auto* v5 = b.FunctionParam("v5", ty.f16());
    auto* v6 = b.FunctionParam("v6", ty.f16());
    auto* v7 = b.FunctionParam("v7", ty.f16());
    auto* v8 = b.FunctionParam("v8", ty.f16());
    auto* v9 = b.FunctionParam("v9", ty.f16());
    auto* func = b.Function("foo", mat);
    func->SetParams({v1, v2, v3, v4, v5, v6, v7, v8, v9});
    b.Append(func->Block(), [&] {
        auto* construct = b.Construct(mat, v1, v2, v3, v4, v5, v6, v7, v8, v9);
        b.Return(func, construct->Result());
    });

    auto* src = R"(
%foo = func(%v1:f16, %v2:f16, %v3:f16, %v4:f16, %v5:f16, %v6:f16, %v7:f16, %v8:f16, %v9:f16):mat3x3<f16> {
  $B1: {
    %11:mat3x3<f16> = construct %v1, %v2, %v3, %v4, %v5, %v6, %v7, %v8, %v9
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v1:f16, %v2:f16, %v3:f16, %v4:f16, %v5:f16, %v6:f16, %v7:f16, %v8:f16, %v9:f16):mat3x3<f16> {
  $B1: {
    %11:vec3<f16> = construct %v1, %v2, %v3
    %12:vec3<f16> = construct %v4, %v5, %v6
    %13:vec3<f16> = construct %v7, %v8, %v9
    %14:mat3x3<f16> = construct %11, %12, %13
    ret %14
  }
}
)";

    Run(VectorizeScalarMatrixConstructors);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
