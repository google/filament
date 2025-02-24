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

#include "src/tint/lang/spirv/writer/raise/pass_matrix_by_pointer.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_PassMatrixByPointerTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_PassMatrixByPointerTest, NoModify_ArrayValue) {
    auto* arr_ty = ty.array<f32, 4u>();
    auto* arr = mod.root_block->Append(b.Var("var", ty.ptr(private_, arr_ty)));

    auto* target = b.Function("target", ty.f32());
    auto* value = b.FunctionParam("value", arr_ty);
    target->SetParams({value});
    b.Append(target->Block(), [&] {
        auto* access = b.Access(ty.f32(), value, 1_i);
        b.Return(target, access);
    });

    auto* caller = b.Function("caller", ty.f32());
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(ty.f32(), target, b.Load(arr));
        b.Return(caller, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, array<f32, 4>, read_write> = var
}

%target = func(%value:array<f32, 4>):f32 {
  $B2: {
    %4:f32 = access %value, 1i
    ret %4
  }
}
%caller = func():f32 {
  $B3: {
    %6:array<f32, 4> = load %var
    %7:f32 = call %target, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, NoModify_MatrixPointer) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* mat = mod.root_block->Append(b.Var("var", ty.ptr(private_, mat_ty)));

    auto* target = b.Function("target", ty.vec3<f32>());
    auto* value = b.FunctionParam("value", ty.ptr(private_, mat_ty));
    target->SetParams({value});
    b.Append(target->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, vec3<f32>>(), value, 1_i);
        b.Return(target, b.Load(access));
    });

    auto* caller = b.Function("caller", ty.vec3<f32>());
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(ty.vec3<f32>(), target, mat);
        b.Return(caller, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%target = func(%value:ptr<private, mat3x3<f32>, read_write>):vec3<f32> {
  $B2: {
    %4:ptr<private, vec3<f32>, read_write> = access %value, 1i
    %5:vec3<f32> = load %4
    ret %5
  }
}
%caller = func():vec3<f32> {
  $B3: {
    %7:vec3<f32> = call %target, %var
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, MatrixValuePassedToBuiltin) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* mat = mod.root_block->Append(b.Var("var", ty.ptr(private_, mat_ty)));

    auto* caller = b.Function("caller", ty.f32());
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kDeterminant, b.Load(mat));
        b.Return(caller, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%caller = func():f32 {
  $B2: {
    %3:mat3x3<f32> = load %var
    %4:f32 = determinant %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, SingleMatrixValue) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* mat = mod.root_block->Append(b.Var("var", ty.ptr(private_, mat_ty)));

    auto* target = b.Function("target", mat_ty);
    auto* value = b.FunctionParam("value", mat_ty);
    target->SetParams({value});
    b.Append(target->Block(), [&] {
        auto* scale = b.Multiply(mat_ty, value, b.Constant(2_f));
        b.Return(target, scale);
    });

    auto* caller = b.Function("caller", mat_ty);
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(mat));
        b.Return(caller, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%target = func(%value:mat3x3<f32>):mat3x3<f32> {
  $B2: {
    %4:mat3x3<f32> = mul %value, 2.0f
    ret %4
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %6:mat3x3<f32> = load %var
    %7:mat3x3<f32> = call %target, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%target = func(%3:ptr<function, mat3x3<f32>, read_write>):mat3x3<f32> {
  $B2: {
    %4:mat3x3<f32> = load %3
    %5:mat3x3<f32> = mul %4, 2.0f
    ret %5
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %7:mat3x3<f32> = load %var
    %8:ptr<function, mat3x3<f32>, read_write> = var, %7
    %9:mat3x3<f32> = call %target, %8
    ret %9
  }
}
)";

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, MultipleMatrixValues) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* arr = mod.root_block->Append(b.Var("var", ty.ptr(private_, ty.array(mat_ty, 4))));

    auto* target = b.Function("target", mat_ty);
    auto* value_a = b.FunctionParam("value_a", mat_ty);
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* value_b = b.FunctionParam("value_b", mat_ty);
    target->SetParams({value_a, scalar, value_b});
    b.Append(target->Block(), [&] {
        auto* scale = b.Multiply(mat_ty, value_a, scalar);
        auto* sum = b.Add(mat_ty, scale, value_b);
        b.Return(target, sum);
    });

    auto* caller = b.Function("caller", mat_ty);
    b.Append(caller->Block(), [&] {
        auto* mat_ptr = ty.ptr(private_, mat_ty);
        auto* ma = b.Load(b.Access(mat_ptr, arr, 0_u));
        auto* mb = b.Load(b.Access(mat_ptr, arr, 1_u));
        auto* result = b.Call(mat_ty, target, ma, b.Constant(2_f), mb);
        b.Return(caller, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, array<mat3x3<f32>, 4>, read_write> = var
}

%target = func(%value_a:mat3x3<f32>, %scalar:f32, %value_b:mat3x3<f32>):mat3x3<f32> {
  $B2: {
    %6:mat3x3<f32> = mul %value_a, %scalar
    %7:mat3x3<f32> = add %6, %value_b
    ret %7
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %9:ptr<private, mat3x3<f32>, read_write> = access %var, 0u
    %10:mat3x3<f32> = load %9
    %11:ptr<private, mat3x3<f32>, read_write> = access %var, 1u
    %12:mat3x3<f32> = load %11
    %13:mat3x3<f32> = call %target, %10, 2.0f, %12
    ret %13
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %var:ptr<private, array<mat3x3<f32>, 4>, read_write> = var
}

%target = func(%3:ptr<function, mat3x3<f32>, read_write>, %scalar:f32, %5:ptr<function, mat3x3<f32>, read_write>):mat3x3<f32> {
  $B2: {
    %6:mat3x3<f32> = load %5
    %7:mat3x3<f32> = load %3
    %8:mat3x3<f32> = mul %7, %scalar
    %9:mat3x3<f32> = add %8, %6
    ret %9
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %11:ptr<private, mat3x3<f32>, read_write> = access %var, 0u
    %12:mat3x3<f32> = load %11
    %13:ptr<private, mat3x3<f32>, read_write> = access %var, 1u
    %14:mat3x3<f32> = load %13
    %15:ptr<function, mat3x3<f32>, read_write> = var, %12
    %16:ptr<function, mat3x3<f32>, read_write> = var, %14
    %17:mat3x3<f32> = call %target, %15, 2.0f, %16
    ret %17
  }
}
)";

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, MultipleParamUses) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* mat = mod.root_block->Append(b.Var("var", ty.ptr(private_, mat_ty)));

    auto* target = b.Function("target", mat_ty);
    auto* value = b.FunctionParam("value", mat_ty);
    target->SetParams({value});
    b.Append(target->Block(), [&] {
        auto* add = b.Add(mat_ty, value, value);
        auto* mul = b.Multiply(mat_ty, add, value);
        b.Return(target, mul);
    });

    auto* caller = b.Function("caller", mat_ty);
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(mat));
        b.Return(caller, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%target = func(%value:mat3x3<f32>):mat3x3<f32> {
  $B2: {
    %4:mat3x3<f32> = add %value, %value
    %5:mat3x3<f32> = mul %4, %value
    ret %5
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %7:mat3x3<f32> = load %var
    %8:mat3x3<f32> = call %target, %7
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%target = func(%3:ptr<function, mat3x3<f32>, read_write>):mat3x3<f32> {
  $B2: {
    %4:mat3x3<f32> = load %3
    %5:mat3x3<f32> = add %4, %4
    %6:mat3x3<f32> = mul %5, %4
    ret %6
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %8:mat3x3<f32> = load %var
    %9:ptr<function, mat3x3<f32>, read_write> = var, %8
    %10:mat3x3<f32> = call %target, %9
    ret %10
  }
}
)";

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, MultipleCallsites) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* mat = mod.root_block->Append(b.Var("var", ty.ptr(private_, mat_ty)));

    auto* target = b.Function("target", mat_ty);
    auto* value = b.FunctionParam("value", mat_ty);
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    target->SetParams({value, scalar});
    b.Append(target->Block(), [&] {
        auto* scale = b.Multiply(mat_ty, value, scalar);
        b.Return(target, scale);
    });

    auto* caller_a = b.Function("caller_a", mat_ty);
    b.Append(caller_a->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(mat), b.Constant(2_f));
        b.Return(caller_a, result);
    });

    auto* caller_b = b.Function("caller_b", mat_ty);
    b.Append(caller_b->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(mat), b.Constant(3_f));
        b.Return(caller_b, result);
    });

    auto* caller_c = b.Function("caller_c", mat_ty);
    b.Append(caller_c->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(mat), b.Constant(4_f));
        b.Return(caller_c, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%target = func(%value:mat3x3<f32>, %scalar:f32):mat3x3<f32> {
  $B2: {
    %5:mat3x3<f32> = mul %value, %scalar
    ret %5
  }
}
%caller_a = func():mat3x3<f32> {
  $B3: {
    %7:mat3x3<f32> = load %var
    %8:mat3x3<f32> = call %target, %7, 2.0f
    ret %8
  }
}
%caller_b = func():mat3x3<f32> {
  $B4: {
    %10:mat3x3<f32> = load %var
    %11:mat3x3<f32> = call %target, %10, 3.0f
    ret %11
  }
}
%caller_c = func():mat3x3<f32> {
  $B5: {
    %13:mat3x3<f32> = load %var
    %14:mat3x3<f32> = call %target, %13, 4.0f
    ret %14
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %var:ptr<private, mat3x3<f32>, read_write> = var
}

%target = func(%3:ptr<function, mat3x3<f32>, read_write>, %scalar:f32):mat3x3<f32> {
  $B2: {
    %5:mat3x3<f32> = load %3
    %6:mat3x3<f32> = mul %5, %scalar
    ret %6
  }
}
%caller_a = func():mat3x3<f32> {
  $B3: {
    %8:mat3x3<f32> = load %var
    %9:ptr<function, mat3x3<f32>, read_write> = var, %8
    %10:mat3x3<f32> = call %target, %9, 2.0f
    ret %10
  }
}
%caller_b = func():mat3x3<f32> {
  $B4: {
    %12:mat3x3<f32> = load %var
    %13:ptr<function, mat3x3<f32>, read_write> = var, %12
    %14:mat3x3<f32> = call %target, %13, 3.0f
    ret %14
  }
}
%caller_c = func():mat3x3<f32> {
  $B5: {
    %16:mat3x3<f32> = load %var
    %17:ptr<function, mat3x3<f32>, read_write> = var, %16
    %18:mat3x3<f32> = call %target, %17, 4.0f
    ret %18
  }
}
)";

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, MatrixInArray) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* arr_ty = ty.array(mat_ty, 2);
    auto* arr = mod.root_block->Append(b.Var("var", ty.ptr(private_, arr_ty)));

    auto* target = b.Function("target", mat_ty);
    auto* value = b.FunctionParam("value", arr_ty);
    target->SetParams({value});
    b.Append(target->Block(), [&] {
        auto* ma = b.Access(mat_ty, value, 0_u);
        auto* mb = b.Access(mat_ty, value, 1_u);
        auto* add = b.Add(mat_ty, ma, mb);
        b.Return(target, add);
    });

    auto* caller = b.Function("caller", mat_ty);
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(arr));
        b.Return(caller, result);
    });

    auto* src = R"(
$B1: {  # root
  %var:ptr<private, array<mat3x3<f32>, 2>, read_write> = var
}

%target = func(%value:array<mat3x3<f32>, 2>):mat3x3<f32> {
  $B2: {
    %4:mat3x3<f32> = access %value, 0u
    %5:mat3x3<f32> = access %value, 1u
    %6:mat3x3<f32> = add %4, %5
    ret %6
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %8:array<mat3x3<f32>, 2> = load %var
    %9:mat3x3<f32> = call %target, %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %var:ptr<private, array<mat3x3<f32>, 2>, read_write> = var
}

%target = func(%3:ptr<function, array<mat3x3<f32>, 2>, read_write>):mat3x3<f32> {
  $B2: {
    %4:array<mat3x3<f32>, 2> = load %3
    %5:mat3x3<f32> = access %4, 0u
    %6:mat3x3<f32> = access %4, 1u
    %7:mat3x3<f32> = add %5, %6
    ret %7
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %9:array<mat3x3<f32>, 2> = load %var
    %10:ptr<function, array<mat3x3<f32>, 2>, read_write> = var, %9
    %11:mat3x3<f32> = call %target, %10
    ret %11
  }
}
)";

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, MatrixInStruct) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("m"), mat_ty},
                                                              {mod.symbols.New("s"), ty.f32()},
                                                          });
    auto* structure = mod.root_block->Append(b.Var("var", ty.ptr(private_, str_ty)));

    auto* target = b.Function("target", mat_ty);
    auto* value = b.FunctionParam("value", str_ty);
    target->SetParams({value});
    b.Append(target->Block(), [&] {
        auto* m = b.Access(mat_ty, value, 0_u);
        auto* s = b.Access(ty.f32(), value, 1_u);
        auto* mul = b.Multiply(mat_ty, m, s);
        b.Return(target, mul);
    });

    auto* caller = b.Function("caller", mat_ty);
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(structure));
        b.Return(caller, result);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  m:mat3x3<f32> @offset(0)
  s:f32 @offset(48)
}

$B1: {  # root
  %var:ptr<private, MyStruct, read_write> = var
}

%target = func(%value:MyStruct):mat3x3<f32> {
  $B2: {
    %4:mat3x3<f32> = access %value, 0u
    %5:f32 = access %value, 1u
    %6:mat3x3<f32> = mul %4, %5
    ret %6
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %8:MyStruct = load %var
    %9:mat3x3<f32> = call %target, %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  m:mat3x3<f32> @offset(0)
  s:f32 @offset(48)
}

$B1: {  # root
  %var:ptr<private, MyStruct, read_write> = var
}

%target = func(%3:ptr<function, MyStruct, read_write>):mat3x3<f32> {
  $B2: {
    %4:MyStruct = load %3
    %5:mat3x3<f32> = access %4, 0u
    %6:f32 = access %4, 1u
    %7:mat3x3<f32> = mul %5, %6
    ret %7
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %9:MyStruct = load %var
    %10:ptr<function, MyStruct, read_write> = var, %9
    %11:mat3x3<f32> = call %target, %10
    ret %11
  }
}
)";

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PassMatrixByPointerTest, MatrixArrayOfStructOfArray) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("m"), ty.array(mat_ty, 2)},
                                                   {mod.symbols.New("s"), ty.f32()},
                                               });
    auto* arr_ty = ty.array(str_ty, 4);
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(private_, arr_ty)));

    auto* target = b.Function("target", mat_ty);
    auto* value = b.FunctionParam("value", arr_ty);
    target->SetParams({value});
    b.Append(target->Block(), [&] {
        auto* ma = b.Access(mat_ty, value, 2_u, 0_u, 0_u);
        auto* mb = b.Access(mat_ty, value, 2_u, 0_u, 1_u);
        auto* s = b.Access(ty.f32(), value, 2_u, 1_u);
        auto* add = b.Add(mat_ty, ma, mb);
        auto* mul = b.Multiply(mat_ty, add, s);
        b.Return(target, mul);
    });

    auto* caller = b.Function("caller", mat_ty);
    b.Append(caller->Block(), [&] {
        auto* result = b.Call(mat_ty, target, b.Load(var));
        b.Return(caller, result);
    });

    auto* src = R"(
MyStruct = struct @align(16) {
  m:array<mat3x3<f32>, 2> @offset(0)
  s:f32 @offset(96)
}

$B1: {  # root
  %var:ptr<private, array<MyStruct, 4>, read_write> = var
}

%target = func(%value:array<MyStruct, 4>):mat3x3<f32> {
  $B2: {
    %4:mat3x3<f32> = access %value, 2u, 0u, 0u
    %5:mat3x3<f32> = access %value, 2u, 0u, 1u
    %6:f32 = access %value, 2u, 1u
    %7:mat3x3<f32> = add %4, %5
    %8:mat3x3<f32> = mul %7, %6
    ret %8
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %10:array<MyStruct, 4> = load %var
    %11:mat3x3<f32> = call %target, %10
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(16) {
  m:array<mat3x3<f32>, 2> @offset(0)
  s:f32 @offset(96)
}

$B1: {  # root
  %var:ptr<private, array<MyStruct, 4>, read_write> = var
}

%target = func(%3:ptr<function, array<MyStruct, 4>, read_write>):mat3x3<f32> {
  $B2: {
    %4:array<MyStruct, 4> = load %3
    %5:mat3x3<f32> = access %4, 2u, 0u, 0u
    %6:mat3x3<f32> = access %4, 2u, 0u, 1u
    %7:f32 = access %4, 2u, 1u
    %8:mat3x3<f32> = add %5, %6
    %9:mat3x3<f32> = mul %8, %7
    ret %9
  }
}
%caller = func():mat3x3<f32> {
  $B3: {
    %11:array<MyStruct, 4> = load %var
    %12:ptr<function, array<MyStruct, 4>, read_write> = var, %11
    %13:mat3x3<f32> = call %target, %12
    ret %13
  }
}
)";

    Run(PassMatrixByPointer);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
