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

#include "src/tint/lang/msl/writer/raise/packed_vec3.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

using MslWriter_PackedVec3Test = core::ir::transform::TransformTest;

TEST_F(MslWriter_PackedVec3Test, NoModify_PrivateVar) {
    auto* var = b.Var<private_, vec3<u32>>("v");
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<u32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, vec3<u32>, read_write> = var undef
}

%foo = func():vec3<u32> {
  $B2: {
    %3:vec3<u32> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, NoModify_Vec2) {
    auto* var = b.Var<uniform, vec2<u32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, vec2<u32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec2<u32> {
  $B2: {
    %3:vec2<u32> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, NoModify_Mat3x2) {
    auto* var = b.Var<uniform, mat3x2<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.mat3x2<f32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, mat3x2<f32>, read> = var undef @binding_point(0, 0)
}

%foo = func():mat3x2<f32> {
  $B2: {
    %3:mat3x2<f32> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, NoModify_ArrayOfVec4) {
    auto* var = b.Var<uniform, array<vec4<u32>, 3>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.array<vec4<u32>, 3>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 3>, read> = var undef @binding_point(0, 0)
}

%foo = func():array<vec4<u32>, 3> {
  $B2: {
    %3:array<vec4<u32>, 3> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, WorkgroupVar_Vec3) {
    auto* var = b.Var<workgroup, vec3<u32>>("v");
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<u32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, vec3<u32>, read_write> = var undef
}

%foo = func():vec3<u32> {
  $B2: {
    %3:vec3<u32> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, __packed_vec3<u32>, read_write> = var undef
}

%foo = func():vec3<u32> {
  $B2: {
    %3:__packed_vec3<u32> = load %v
    %4:vec3<u32> = msl.convert %3
    ret %4
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, UniformVar_Vec3) {
    auto* var = b.Var<uniform, vec3<u32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<u32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec3<u32> {
  $B2: {
    %3:vec3<u32> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, __packed_vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo = func():vec3<u32> {
  $B2: {
    %3:__packed_vec3<u32> = load %v
    %4:vec3<u32> = msl.convert %3
    ret %4
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Vec3_LoadVector) {
    auto* var = b.Var<storage, vec3<u32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<u32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<u32> {
  $B2: {
    %3:vec3<u32> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, __packed_vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<u32> {
  $B2: {
    %3:__packed_vec3<u32> = load %v
    %4:vec3<u32> = msl.convert %3
    ret %4
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Vec3_LoadElement) {
    auto* var = b.Var<storage, vec3<u32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {  //
        auto* el_0 = b.LoadVectorElement(var, 0_u);
        auto* el_1 = b.LoadVectorElement(var, 1_u);
        b.Return(func, b.Add<u32>(el_0, el_1));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = load_vector_element %v, 0u
    %4:u32 = load_vector_element %v, 1u
    %5:u32 = add %3, %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, __packed_vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = load_vector_element %v, 0u
    %4:u32 = load_vector_element %v, 1u
    %5:u32 = add %3, %4
    ret %5
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Vec3_StoreVector) {
    auto* var = b.Var<storage, vec3<u32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.vec3<u32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<u32>):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, __packed_vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<u32>):void {
  $B2: {
    %4:__packed_vec3<u32> = msl.convert %value
    store %v, %4
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Vec3_StoreElement) {
    auto* var = b.Var<storage, vec3<u32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.u32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.StoreVectorElement(var, 0_u, value);
        b.StoreVectorElement(var, 1_u, value);
        b.StoreVectorElement(var, 2_u, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:u32):void {
  $B2: {
    store_vector_element %v, 0u, %value
    store_vector_element %v, 1u, %value
    store_vector_element %v, 2u, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, __packed_vec3<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:u32):void {
  $B2: {
    store_vector_element %v, 0u, %value
    store_vector_element %v, 1u, %value
    store_vector_element %v, 2u, %value
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Mat4x3_LoadMatrix) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.mat4x3<f32>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():mat4x3<f32> {
  $B2: {
    %3:mat4x3<f32> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():mat4x3<f32> {
  $B2: {
    %3:array<tint_packed_vec3_f32_array_element, 4> = load %v
    %4:__packed_vec3<f32> = access %3, 0u, 0u
    %5:vec3<f32> = msl.convert %4
    %6:__packed_vec3<f32> = access %3, 1u, 0u
    %7:vec3<f32> = msl.convert %6
    %8:__packed_vec3<f32> = access %3, 2u, 0u
    %9:vec3<f32> = msl.convert %8
    %10:__packed_vec3<f32> = access %3, 3u, 0u
    %11:vec3<f32> = msl.convert %10
    %12:mat4x3<f32> = construct %5, %7, %9, %11
    ret %12
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Mat4x3_LoadColumn) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<f32>());
    b.Append(func->Block(), [&] {  //
        auto* col_0 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u));
        auto* col_1 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u));
        auto* col_2 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 2_u));
        auto* col_3 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 3_u));
        b.Return(func,
                 b.Add<vec3<f32>>(b.Add<vec3<f32>>(b.Add<vec3<f32>>(col_0, col_1), col_2), col_3));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<f32> {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    %4:vec3<f32> = load %3
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    %6:vec3<f32> = load %5
    %7:ptr<storage, vec3<f32>, read_write> = access %v, 2u
    %8:vec3<f32> = load %7
    %9:ptr<storage, vec3<f32>, read_write> = access %v, 3u
    %10:vec3<f32> = load %9
    %11:vec3<f32> = add %4, %6
    %12:vec3<f32> = add %11, %8
    %13:vec3<f32> = add %12, %10
    ret %13
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<f32> {
  $B2: {
    %3:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %4:__packed_vec3<f32> = load %3
    %5:vec3<f32> = msl.convert %4
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %7:__packed_vec3<f32> = load %6
    %8:vec3<f32> = msl.convert %7
    %9:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 2u, 0u
    %10:__packed_vec3<f32> = load %9
    %11:vec3<f32> = msl.convert %10
    %12:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 3u, 0u
    %13:__packed_vec3<f32> = load %12
    %14:vec3<f32> = msl.convert %13
    %15:vec3<f32> = add %5, %8
    %16:vec3<f32> = add %15, %11
    %17:vec3<f32> = add %16, %14
    ret %17
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Mat4x3_LoadElement) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {  //
        auto* el_0 = b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u), 1_u);
        auto* el_1 = b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u), 1_u);
        auto* el_2 = b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 2_u), 2_u);
        auto* el_3 = b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 3_u), 2_u);
        b.Return(func, b.Add<f32>(b.Add<f32>(b.Add<f32>(el_0, el_1), el_2), el_3));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    %4:f32 = load_vector_element %3, 1u
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    %6:f32 = load_vector_element %5, 1u
    %7:ptr<storage, vec3<f32>, read_write> = access %v, 2u
    %8:f32 = load_vector_element %7, 2u
    %9:ptr<storage, vec3<f32>, read_write> = access %v, 3u
    %10:f32 = load_vector_element %9, 2u
    %11:f32 = add %4, %6
    %12:f32 = add %11, %8
    %13:f32 = add %12, %10
    ret %13
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %4:f32 = load_vector_element %3, 1u
    %5:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %6:f32 = load_vector_element %5, 1u
    %7:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 2u, 0u
    %8:f32 = load_vector_element %7, 2u
    %9:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 3u, 0u
    %10:f32 = load_vector_element %9, 2u
    %11:f32 = add %4, %6
    %12:f32 = add %11, %8
    %13:f32 = add %12, %10
    ret %13
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Mat4x3_StoreMatrix) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.mat4x3<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:mat4x3<f32>):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:mat4x3<f32>):void {
  $B2: {
    %4:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %5:vec3<f32> = access %value, 0u
    %6:__packed_vec3<f32> = msl.convert %5
    store %4, %6
    %7:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %8:vec3<f32> = access %value, 1u
    %9:__packed_vec3<f32> = msl.convert %8
    store %7, %9
    %10:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 2u, 0u
    %11:vec3<f32> = access %value, 2u
    %12:__packed_vec3<f32> = msl.convert %11
    store %10, %12
    %13:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 3u, 0u
    %14:vec3<f32> = access %value, 3u
    %15:__packed_vec3<f32> = msl.convert %14
    store %13, %15
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Mat4x3_StoreColumn) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.vec3<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u), value);
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u), value);
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 2_u), value);
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 3_u), value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<f32>):void {
  $B2: {
    %4:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    store %4, %value
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    store %5, %value
    %6:ptr<storage, vec3<f32>, read_write> = access %v, 2u
    store %6, %value
    %7:ptr<storage, vec3<f32>, read_write> = access %v, 3u
    store %7, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<f32>):void {
  $B2: {
    %4:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %5:__packed_vec3<f32> = msl.convert %value
    store %4, %5
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %7:__packed_vec3<f32> = msl.convert %value
    store %6, %7
    %8:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 2u, 0u
    %9:__packed_vec3<f32> = msl.convert %value
    store %8, %9
    %10:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 3u, 0u
    %11:__packed_vec3<f32> = msl.convert %value
    store %10, %11
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Mat4x3_StoreElement) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.StoreVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u), 1_u, value);
        b.StoreVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u), 1_u, value);
        b.StoreVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 2_u), 2_u, value);
        b.StoreVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 3_u), 2_u, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:f32):void {
  $B2: {
    %4:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    store_vector_element %4, 1u, %value
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    store_vector_element %5, 1u, %value
    %6:ptr<storage, vec3<f32>, read_write> = access %v, 2u
    store_vector_element %6, 2u, %value
    %7:ptr<storage, vec3<f32>, read_write> = access %v, 3u
    store_vector_element %7, 2u, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:f32):void {
  $B2: {
    %4:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    store_vector_element %4, 1u, %value
    %5:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    store_vector_element %5, 1u, %value
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 2u, 0u
    store_vector_element %6, 2u, %value
    %7:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 3u, 0u
    store_vector_element %7, 2u, %value
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Mat2x3_F16_LoadMatrix) {
    auto* var = b.Var<storage, mat2x3<f16>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.mat2x3<f16>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat2x3<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():mat2x3<f16> {
  $B2: {
    %3:mat2x3<f16> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f16_array_element = struct @align(8) {
  packed:__packed_vec3<f16> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f16_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():mat2x3<f16> {
  $B2: {
    %3:array<tint_packed_vec3_f16_array_element, 2> = load %v
    %4:__packed_vec3<f16> = access %3, 0u, 0u
    %5:vec3<f16> = msl.convert %4
    %6:__packed_vec3<f16> = access %3, 1u, 0u
    %7:vec3<f16> = msl.convert %6
    %8:mat2x3<f16> = construct %5, %7
    ret %8
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_LoadArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 2>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.array<vec3<f32>, 2>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f32>, 2> {
  $B2: {
    %3:array<vec3<f32>, 2> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f32>, 2> {
  $B2: {
    %3:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3, %v
    ret %3
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {
  $B3: {
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 0u, 0u
    %7:__packed_vec3<f32> = load %6
    %8:vec3<f32> = msl.convert %7
    %9:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 1u, 0u
    %10:__packed_vec3<f32> = load %9
    %11:vec3<f32> = msl.convert %10
    %12:array<vec3<f32>, 2> = construct %8, %11
    ret %12
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_LoadArray_LargeCount) {
    auto* var = b.Var<storage, array<vec3<f32>, 1024>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.array<vec3<f32>, 1024>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f32>, 1024> {
  $B2: {
    %3:array<vec3<f32>, 1024> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f32>, 1024> {
  $B2: {
    %3:array<vec3<f32>, 1024> = call %tint_load_array_packed_vec3, %v
    ret %3
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<tint_packed_vec3_f32_array_element, 1024>, read_write>):array<vec3<f32>, 1024> {
  $B3: {
    %6:ptr<function, array<vec3<f32>, 1024>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %8:bool = gte %idx, 1024u
        if %8 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %9:ptr<function, vec3<f32>, read_write> = access %6, %idx
        %10:ptr<storage, __packed_vec3<f32>, read_write> = access %from, %idx, 0u
        %11:__packed_vec3<f32> = load %10
        %12:vec3<f32> = msl.convert %11
        store %9, %12
        continue  # -> $B6
      }
      $B6: {  # continuing
        %13:u32 = add %idx, 1u
        next_iteration %13  # -> $B5
      }
    }
    %14:array<vec3<f32>, 1024> = load %6
    ret %14
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_LoadVector) {
    auto* var = b.Var<storage, array<vec3<f32>, 2>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<f32>());
    b.Append(func->Block(), [&] {  //
        auto* el_0 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u));
        auto* el_1 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u));
        b.Return(func, b.Add<vec3<f32>>(el_0, el_1));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<f32> {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    %4:vec3<f32> = load %3
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    %6:vec3<f32> = load %5
    %7:vec3<f32> = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<f32> {
  $B2: {
    %3:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %4:__packed_vec3<f32> = load %3
    %5:vec3<f32> = msl.convert %4
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %7:__packed_vec3<f32> = load %6
    %8:vec3<f32> = msl.convert %7
    %9:vec3<f32> = add %5, %8
    ret %9
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_LoadElement) {
    auto* var = b.Var<storage, array<vec3<f32>, 2>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {  //
        auto* el_0 = b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u), 2_u);
        auto* el_1 = b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u), 2_u);
        b.Return(func, b.Add<f32>(el_0, el_1));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    %4:f32 = load_vector_element %3, 2u
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    %6:f32 = load_vector_element %5, 2u
    %7:f32 = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():f32 {
  $B2: {
    %3:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %4:f32 = load_vector_element %3, 2u
    %5:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %6:f32 = load_vector_element %5, 2u
    %7:f32 = add %4, %6
    ret %7
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_StoreArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 2>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.array<vec3<f32>, 2>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<vec3<f32>, 2>):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<vec3<f32>, 2>):void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %v, %value
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>, %value_1:array<vec3<f32>, 2>):void {  # %value_1: 'value'
  $B3: {
    %8:vec3<f32> = access %value_1, 0u
    %9:ptr<storage, __packed_vec3<f32>, read_write> = access %to, 0u, 0u
    %10:__packed_vec3<f32> = msl.convert %8
    store %9, %10
    %11:vec3<f32> = access %value_1, 1u
    %12:ptr<storage, __packed_vec3<f32>, read_write> = access %to, 1u, 0u
    %13:__packed_vec3<f32> = msl.convert %11
    store %12, %13
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_StoreArray_LargeCount) {
    auto* var = b.Var<storage, array<vec3<f32>, 1024>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.array<vec3<f32>, 1024>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<vec3<f32>, 1024>):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<vec3<f32>, 1024>):void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %v, %value
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, array<tint_packed_vec3_f32_array_element, 1024>, read_write>, %value_1:array<vec3<f32>, 1024>):void {  # %value_1: 'value'
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %9:bool = gte %idx, 1024u
        if %9 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %10:vec3<f32> = access %value_1, %idx
        %11:ptr<storage, __packed_vec3<f32>, read_write> = access %to, %idx, 0u
        %12:__packed_vec3<f32> = msl.convert %10
        store %11, %12
        continue  # -> $B6
      }
      $B6: {  # continuing
        %13:u32 = add %idx, 1u
        next_iteration %13  # -> $B5
      }
    }
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_StoreVector) {
    auto* var = b.Var<storage, array<vec3<f32>, 2>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.vec3<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u), value);
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u), value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<f32>):void {
  $B2: {
    %4:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    store %4, %value
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    store %5, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<f32>):void {
  $B2: {
    %4:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %5:__packed_vec3<f32> = msl.convert %value
    store %4, %5
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %7:__packed_vec3<f32> = msl.convert %value
    store %6, %7
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_StoreElement) {
    auto* var = b.Var<storage, array<vec3<f32>, 2>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.StoreVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u), 2_u, value);
        b.StoreVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u), 2_u, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:f32):void {
  $B2: {
    %4:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    store_vector_element %4, 2u, %value
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    store_vector_element %5, 2u, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:f32):void {
  $B2: {
    %4:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    store_vector_element %4, 2u, %value
    %5:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    store_vector_element %5, 2u, %value
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Array_F16_LoadArray) {
    auto* var = b.Var<storage, array<vec3<f16>, 2>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.array<vec3<f16>, 2>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f16>, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f16>, 2> {
  $B2: {
    %3:array<vec3<f16>, 2> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f16_array_element = struct @align(8) {
  packed:__packed_vec3<f16> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f16_array_element, 2>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f16>, 2> {
  $B2: {
    %3:array<vec3<f16>, 2> = call %tint_load_array_packed_vec3, %v
    ret %3
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<tint_packed_vec3_f16_array_element, 2>, read_write>):array<vec3<f16>, 2> {
  $B3: {
    %6:ptr<storage, __packed_vec3<f16>, read_write> = access %from, 0u, 0u
    %7:__packed_vec3<f16> = load %6
    %8:vec3<f16> = msl.convert %7
    %9:ptr<storage, __packed_vec3<f16>, read_write> = access %from, 1u, 0u
    %10:__packed_vec3<f16> = load %9
    %11:vec3<f16> = msl.convert %10
    %12:array<vec3<f16>, 2> = construct %8, %11
    ret %12
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_NestedArray_LoadOuter) {
    auto* var = b.Var<storage, array<array<vec3<f32>, 2>, 3>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.array<array<vec3<f32>, 2>, 3>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<array<vec3<f32>, 2>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<array<vec3<f32>, 2>, 3> {
  $B2: {
    %3:array<array<vec3<f32>, 2>, 3> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<array<vec3<f32>, 2>, 3> {
  $B2: {
    %3:array<array<vec3<f32>, 2>, 3> = call %tint_load_array_packed_vec3, %v
    ret %3
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 3>, read_write>):array<array<vec3<f32>, 2>, 3> {
  $B3: {
    %6:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %from, 0u
    %7:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3_1, %6
    %9:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %from, 1u
    %10:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3_1, %9
    %11:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %from, 2u
    %12:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3_1, %11
    %13:array<array<vec3<f32>, 2>, 3> = construct %7, %10, %12
    ret %13
  }
}
%tint_load_array_packed_vec3_1 = func(%from_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {  # %from_1: 'from'
  $B4: {
    %15:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 0u, 0u
    %16:__packed_vec3<f32> = load %15
    %17:vec3<f32> = msl.convert %16
    %18:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 1u, 0u
    %19:__packed_vec3<f32> = load %18
    %20:vec3<f32> = msl.convert %19
    %21:array<vec3<f32>, 2> = construct %17, %20
    ret %21
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_NestedArray_LoadOuter_LargeCount) {
    auto* var = b.Var<storage, array<array<vec3<f32>, 2>, 1024>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.array<array<vec3<f32>, 2>, 1024>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<array<vec3<f32>, 2>, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<array<vec3<f32>, 2>, 1024> {
  $B2: {
    %3:array<array<vec3<f32>, 2>, 1024> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<array<vec3<f32>, 2>, 1024> {
  $B2: {
    %3:array<array<vec3<f32>, 2>, 1024> = call %tint_load_array_packed_vec3, %v
    ret %3
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 1024>, read_write>):array<array<vec3<f32>, 2>, 1024> {
  $B3: {
    %6:ptr<function, array<array<vec3<f32>, 2>, 1024>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %8:bool = gte %idx, 1024u
        if %8 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %9:ptr<function, array<vec3<f32>, 2>, read_write> = access %6, %idx
        %10:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %from, %idx
        %11:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3_1, %10
        store %9, %11
        continue  # -> $B6
      }
      $B6: {  # continuing
        %13:u32 = add %idx, 1u
        next_iteration %13  # -> $B5
      }
    }
    %14:array<array<vec3<f32>, 2>, 1024> = load %6
    ret %14
  }
}
%tint_load_array_packed_vec3_1 = func(%from_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {  # %from_1: 'from'
  $B8: {
    %16:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 0u, 0u
    %17:__packed_vec3<f32> = load %16
    %18:vec3<f32> = msl.convert %17
    %19:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 1u, 0u
    %20:__packed_vec3<f32> = load %19
    %21:vec3<f32> = msl.convert %20
    %22:array<vec3<f32>, 2> = construct %18, %21
    ret %22
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_NestedArray_LoadInner) {
    auto* var = b.Var<storage, array<array<vec3<f32>, 2>, 3>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.array<vec3<f32>, 2>());
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(b.Access(ty.ptr<storage, array<vec3<f32>, 2>>(), var, 1_u)));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<array<vec3<f32>, 2>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f32>, 2> {
  $B2: {
    %3:ptr<storage, array<vec3<f32>, 2>, read_write> = access %v, 1u
    %4:array<vec3<f32>, 2> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():array<vec3<f32>, 2> {
  $B2: {
    %3:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %v, 1u
    %4:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3, %3
    ret %4
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {
  $B3: {
    %7:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 0u, 0u
    %8:__packed_vec3<f32> = load %7
    %9:vec3<f32> = msl.convert %8
    %10:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 1u, 0u
    %11:__packed_vec3<f32> = load %10
    %12:vec3<f32> = msl.convert %11
    %13:array<vec3<f32>, 2> = construct %9, %12
    ret %13
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_NestedArray_StoreOuter) {
    auto* var = b.Var<storage, array<array<vec3<f32>, 2>, 3>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.array<array<vec3<f32>, 2>, 3>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<array<vec3<f32>, 2>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<array<vec3<f32>, 2>, 3>):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<array<vec3<f32>, 2>, 3>):void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %v, %value
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 3>, read_write>, %value_1:array<array<vec3<f32>, 2>, 3>):void {  # %value_1: 'value'
  $B3: {
    %8:array<vec3<f32>, 2> = access %value_1, 0u
    %9:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %to, 0u
    %10:void = call %tint_store_array_packed_vec3_1, %9, %8
    %12:array<vec3<f32>, 2> = access %value_1, 1u
    %13:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %to, 1u
    %14:void = call %tint_store_array_packed_vec3_1, %13, %12
    %15:array<vec3<f32>, 2> = access %value_1, 2u
    %16:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %to, 2u
    %17:void = call %tint_store_array_packed_vec3_1, %16, %15
    ret
  }
}
%tint_store_array_packed_vec3_1 = func(%to_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>, %value_2:array<vec3<f32>, 2>):void {  # %to_1: 'to', %value_2: 'value'
  $B4: {
    %20:vec3<f32> = access %value_2, 0u
    %21:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 0u, 0u
    %22:__packed_vec3<f32> = msl.convert %20
    store %21, %22
    %23:vec3<f32> = access %value_2, 1u
    %24:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 1u, 0u
    %25:__packed_vec3<f32> = msl.convert %23
    store %24, %25
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_NestedArray_StoreOuter_LargeCount) {
    auto* var = b.Var<storage, array<array<vec3<f32>, 2>, 1024>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.array<array<vec3<f32>, 2>, 1024>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<array<vec3<f32>, 2>, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<array<vec3<f32>, 2>, 1024>):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:array<array<vec3<f32>, 2>, 1024>):void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %v, %value
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 1024>, read_write>, %value_1:array<array<vec3<f32>, 2>, 1024>):void {  # %value_1: 'value'
  $B3: {
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %9:bool = gte %idx, 1024u
        if %9 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %10:array<vec3<f32>, 2> = access %value_1, %idx
        %11:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %to, %idx
        %12:void = call %tint_store_array_packed_vec3_1, %11, %10
        continue  # -> $B6
      }
      $B6: {  # continuing
        %14:u32 = add %idx, 1u
        next_iteration %14  # -> $B5
      }
    }
    ret
  }
}
%tint_store_array_packed_vec3_1 = func(%to_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>, %value_2:array<vec3<f32>, 2>):void {  # %to_1: 'to', %value_2: 'value'
  $B8: {
    %17:vec3<f32> = access %value_2, 0u
    %18:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 0u, 0u
    %19:__packed_vec3<f32> = msl.convert %17
    store %18, %19
    %20:vec3<f32> = access %value_2, 1u
    %21:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 1u, 0u
    %22:__packed_vec3<f32> = msl.convert %20
    store %21, %22
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_RuntimeArray_LoadVector) {
    auto* var = b.Var<storage, array<vec3<f32>>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<f32>());
    b.Append(func->Block(), [&] {  //
        auto* el_0 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u));
        auto* el_1 = b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u));
        b.Return(func, b.Add<vec3<f32>>(el_0, el_1));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<f32> {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    %4:vec3<f32> = load %3
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    %6:vec3<f32> = load %5
    %7:vec3<f32> = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():vec3<f32> {
  $B2: {
    %3:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u, 0u
    %4:__packed_vec3<f32> = load %3
    %5:vec3<f32> = msl.convert %4
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %7:__packed_vec3<f32> = load %6
    %8:vec3<f32> = msl.convert %7
    %9:vec3<f32> = add %5, %8
    ret %9
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_LoadStruct) {
    auto* s =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("vec"), ty.vec3<u32>()},
                                            {mod.symbols.Register("mat"), ty.mat4x3<f32>()},
                                            {mod.symbols.Register("arr"), ty.array<vec3<f32>, 2>()},
                                        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", s);
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():S {
  $B2: {
    %3:S = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<u32> @offset(0)
  mat:array<tint_packed_vec3_f32_array_element, 4> @offset(16)
  arr:array<tint_packed_vec3_f32_array_element, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

%foo = func():S {
  $B2: {
    %3:S = call %tint_load_struct_packed_vec3, %v
    ret %3
  }
}
%tint_load_struct_packed_vec3 = func(%from:ptr<storage, S_packed_vec3, read_write>):S {
  $B3: {
    %6:ptr<storage, __packed_vec3<u32>, read_write> = access %from, 0u
    %7:__packed_vec3<u32> = load %6
    %8:vec3<u32> = msl.convert %7
    %9:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = access %from, 1u
    %10:array<tint_packed_vec3_f32_array_element, 4> = load %9
    %11:__packed_vec3<f32> = access %10, 0u, 0u
    %12:vec3<f32> = msl.convert %11
    %13:__packed_vec3<f32> = access %10, 1u, 0u
    %14:vec3<f32> = msl.convert %13
    %15:__packed_vec3<f32> = access %10, 2u, 0u
    %16:vec3<f32> = msl.convert %15
    %17:__packed_vec3<f32> = access %10, 3u, 0u
    %18:vec3<f32> = msl.convert %17
    %19:mat4x3<f32> = construct %12, %14, %16, %18
    %20:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %from, 2u
    %21:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3, %20
    %23:S = construct %8, %19, %21
    ret %23
  }
}
%tint_load_array_packed_vec3 = func(%from_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {  # %from_1: 'from'
  $B4: {
    %25:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 0u, 0u
    %26:__packed_vec3<f32> = load %25
    %27:vec3<f32> = msl.convert %26
    %28:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 1u, 0u
    %29:__packed_vec3<f32> = load %28
    %30:vec3<f32> = msl.convert %29
    %31:array<vec3<f32>, 2> = construct %27, %30
    ret %31
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_LoadMembers) {
    auto* s =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("vec"), ty.vec3<u32>()},
                                            {mod.symbols.Register("mat"), ty.mat4x3<f32>()},
                                            {mod.symbols.Register("arr"), ty.array<vec3<f32>, 2>()},
                                        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Load(b.Access(ty.ptr<storage, vec3<u32>>(), var, 0_u));
        b.Load(b.Access(ty.ptr<storage, mat4x3<f32>>(), var, 1_u));
        b.Load(b.Access(ty.ptr<storage, array<vec3<f32>, 2>>(), var, 2_u));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, vec3<u32>, read_write> = access %v, 0u
    %4:vec3<u32> = load %3
    %5:ptr<storage, mat4x3<f32>, read_write> = access %v, 1u
    %6:mat4x3<f32> = load %5
    %7:ptr<storage, array<vec3<f32>, 2>, read_write> = access %v, 2u
    %8:array<vec3<f32>, 2> = load %7
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<u32> @offset(0)
  mat:array<tint_packed_vec3_f32_array_element, 4> @offset(16)
  arr:array<tint_packed_vec3_f32_array_element, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, __packed_vec3<u32>, read_write> = access %v, 0u
    %4:__packed_vec3<u32> = load %3
    %5:vec3<u32> = msl.convert %4
    %6:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = access %v, 1u
    %7:array<tint_packed_vec3_f32_array_element, 4> = load %6
    %8:__packed_vec3<f32> = access %7, 0u, 0u
    %9:vec3<f32> = msl.convert %8
    %10:__packed_vec3<f32> = access %7, 1u, 0u
    %11:vec3<f32> = msl.convert %10
    %12:__packed_vec3<f32> = access %7, 2u, 0u
    %13:vec3<f32> = msl.convert %12
    %14:__packed_vec3<f32> = access %7, 3u, 0u
    %15:vec3<f32> = msl.convert %14
    %16:mat4x3<f32> = construct %9, %11, %13, %15
    %17:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %v, 2u
    %18:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3, %17
    ret
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {
  $B3: {
    %21:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 0u, 0u
    %22:__packed_vec3<f32> = load %21
    %23:vec3<f32> = msl.convert %22
    %24:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 1u, 0u
    %25:__packed_vec3<f32> = load %24
    %26:vec3<f32> = msl.convert %25
    %27:array<vec3<f32>, 2> = construct %23, %26
    ret %27
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_StoreStruct) {
    auto* s =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("vec"), ty.vec3<f32>()},
                                            {mod.symbols.Register("mat"), ty.mat4x3<f32>()},
                                            {mod.symbols.Register("arr"), ty.array<vec3<f32>, 2>()},
                                        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", s);
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<f32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:S):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<f32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<f32> @offset(0)
  mat:array<tint_packed_vec3_f32_array_element, 4> @offset(16)
  arr:array<tint_packed_vec3_f32_array_element, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:S):void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %v, %value
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, S_packed_vec3, read_write>, %value_1:S):void {  # %value_1: 'value'
  $B3: {
    %8:vec3<f32> = access %value_1, 0u
    %9:ptr<storage, __packed_vec3<f32>, read_write> = access %to, 0u
    %10:__packed_vec3<f32> = msl.convert %8
    store %9, %10
    %11:mat4x3<f32> = access %value_1, 1u
    %12:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = access %to, 1u
    %13:ptr<storage, __packed_vec3<f32>, read_write> = access %12, 0u, 0u
    %14:vec3<f32> = access %11, 0u
    %15:__packed_vec3<f32> = msl.convert %14
    store %13, %15
    %16:ptr<storage, __packed_vec3<f32>, read_write> = access %12, 1u, 0u
    %17:vec3<f32> = access %11, 1u
    %18:__packed_vec3<f32> = msl.convert %17
    store %16, %18
    %19:ptr<storage, __packed_vec3<f32>, read_write> = access %12, 2u, 0u
    %20:vec3<f32> = access %11, 2u
    %21:__packed_vec3<f32> = msl.convert %20
    store %19, %21
    %22:ptr<storage, __packed_vec3<f32>, read_write> = access %12, 3u, 0u
    %23:vec3<f32> = access %11, 3u
    %24:__packed_vec3<f32> = msl.convert %23
    store %22, %24
    %25:array<vec3<f32>, 2> = access %value_1, 2u
    %26:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %to, 2u
    %27:void = call %tint_store_array_packed_vec3_1, %26, %25
    ret
  }
}
%tint_store_array_packed_vec3_1 = func(%to_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>, %value_2:array<vec3<f32>, 2>):void {  # %to_1: 'to', %value_2: 'value'
  $B4: {
    %31:vec3<f32> = access %value_2, 0u
    %32:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 0u, 0u
    %33:__packed_vec3<f32> = msl.convert %31
    store %32, %33
    %34:vec3<f32> = access %value_2, 1u
    %35:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 1u, 0u
    %36:__packed_vec3<f32> = msl.convert %34
    store %35, %36
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_StoreMembers) {
    auto* s =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("vec"), ty.vec3<f32>()},
                                            {mod.symbols.Register("mat"), ty.mat4x3<f32>()},
                                            {mod.symbols.Register("arr"), ty.array<vec3<f32>, 2>()},
                                        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", ty.vec3<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 0_u), value);
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u, 3_u), value);
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), var, 2_u, 1_u), value);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<f32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<f32>):void {
  $B2: {
    %4:ptr<storage, vec3<f32>, read_write> = access %v, 0u
    store %4, %value
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 1u, 3u
    store %5, %value
    %6:ptr<storage, vec3<f32>, read_write> = access %v, 2u, 1u
    store %6, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<f32> @offset(0)
  mat:mat4x3<f32> @offset(16)
  arr:array<vec3<f32>, 2> @offset(80)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<f32> @offset(0)
  mat:array<tint_packed_vec3_f32_array_element, 4> @offset(16)
  arr:array<tint_packed_vec3_f32_array_element, 2> @offset(80)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:vec3<f32>):void {
  $B2: {
    %4:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 0u
    %5:__packed_vec3<f32> = msl.convert %value
    store %4, %5
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 3u, 0u
    %7:__packed_vec3<f32> = msl.convert %value
    store %6, %7
    %8:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 2u, 1u, 0u
    %9:__packed_vec3<f32> = msl.convert %value
    store %8, %9
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_WithUnpackedMembers_LoadStruct) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.Register("u32"), ty.u32()},
                            {mod.symbols.Register("vec3"), ty.vec3<u32>()},
                            {mod.symbols.Register("vec4"), ty.vec4<u32>()},
                            {mod.symbols.Register("mat3"), ty.mat4x3<f32>()},
                            {mod.symbols.Register("mat2"), ty.mat3x2<f32>()},
                            {mod.symbols.Register("arr3"), ty.array<vec3<f32>, 2>()},
                            {mod.symbols.Register("arr4"), ty.array<vec4<f32>, 2>()},
                        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", s);
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:mat4x3<f32> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<vec3<f32>, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():S {
  $B2: {
    %3:S = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:mat4x3<f32> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<vec3<f32>, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  u32:u32 @offset(0)
  vec3:__packed_vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:array<tint_packed_vec3_f32_array_element, 4> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<tint_packed_vec3_f32_array_element, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

%foo = func():S {
  $B2: {
    %3:S = call %tint_load_struct_packed_vec3, %v
    ret %3
  }
}
%tint_load_struct_packed_vec3 = func(%from:ptr<storage, S_packed_vec3, read_write>):S {
  $B3: {
    %6:ptr<storage, u32, read_write> = access %from, 0u
    %7:u32 = load %6
    %8:ptr<storage, __packed_vec3<u32>, read_write> = access %from, 1u
    %9:__packed_vec3<u32> = load %8
    %10:vec3<u32> = msl.convert %9
    %11:ptr<storage, vec4<u32>, read_write> = access %from, 2u
    %12:vec4<u32> = load %11
    %13:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = access %from, 3u
    %14:array<tint_packed_vec3_f32_array_element, 4> = load %13
    %15:__packed_vec3<f32> = access %14, 0u, 0u
    %16:vec3<f32> = msl.convert %15
    %17:__packed_vec3<f32> = access %14, 1u, 0u
    %18:vec3<f32> = msl.convert %17
    %19:__packed_vec3<f32> = access %14, 2u, 0u
    %20:vec3<f32> = msl.convert %19
    %21:__packed_vec3<f32> = access %14, 3u, 0u
    %22:vec3<f32> = msl.convert %21
    %23:mat4x3<f32> = construct %16, %18, %20, %22
    %24:ptr<storage, mat3x2<f32>, read_write> = access %from, 4u
    %25:mat3x2<f32> = load %24
    %26:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %from, 5u
    %27:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3, %26
    %29:ptr<storage, array<vec4<f32>, 2>, read_write> = access %from, 6u
    %30:array<vec4<f32>, 2> = load %29
    %31:S = construct %7, %10, %12, %23, %25, %27, %30
    ret %31
  }
}
%tint_load_array_packed_vec3 = func(%from_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {  # %from_1: 'from'
  $B4: {
    %33:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 0u, 0u
    %34:__packed_vec3<f32> = load %33
    %35:vec3<f32> = msl.convert %34
    %36:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 1u, 0u
    %37:__packed_vec3<f32> = load %36
    %38:vec3<f32> = msl.convert %37
    %39:array<vec3<f32>, 2> = construct %35, %38
    ret %39
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_WithUnpackedMembers_LoadMembers) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.Register("u32"), ty.u32()},
                            {mod.symbols.Register("vec3"), ty.vec3<u32>()},
                            {mod.symbols.Register("vec4"), ty.vec4<u32>()},
                            {mod.symbols.Register("mat3"), ty.mat4x3<f32>()},
                            {mod.symbols.Register("mat2"), ty.mat3x2<f32>()},
                            {mod.symbols.Register("arr3"), ty.array<vec3<f32>, 2>()},
                            {mod.symbols.Register("arr4"), ty.array<vec4<f32>, 2>()},
                        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Load(b.Access(ty.ptr<storage, u32>(), var, 0_u));
        b.Load(b.Access(ty.ptr<storage, vec3<u32>>(), var, 1_u));
        b.Load(b.Access(ty.ptr<storage, vec4<u32>>(), var, 2_u));
        b.Load(b.Access(ty.ptr<storage, mat4x3<f32>>(), var, 3_u));
        b.Load(b.Access(ty.ptr<storage, mat3x2<f32>>(), var, 4_u));
        b.Load(b.Access(ty.ptr<storage, array<vec3<f32>, 2>>(), var, 5_u));
        b.Load(b.Access(ty.ptr<storage, array<vec4<f32>, 2>>(), var, 6_u));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:mat4x3<f32> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<vec3<f32>, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %5:ptr<storage, vec3<u32>, read_write> = access %v, 1u
    %6:vec3<u32> = load %5
    %7:ptr<storage, vec4<u32>, read_write> = access %v, 2u
    %8:vec4<u32> = load %7
    %9:ptr<storage, mat4x3<f32>, read_write> = access %v, 3u
    %10:mat4x3<f32> = load %9
    %11:ptr<storage, mat3x2<f32>, read_write> = access %v, 4u
    %12:mat3x2<f32> = load %11
    %13:ptr<storage, array<vec3<f32>, 2>, read_write> = access %v, 5u
    %14:array<vec3<f32>, 2> = load %13
    %15:ptr<storage, array<vec4<f32>, 2>, read_write> = access %v, 6u
    %16:array<vec4<f32>, 2> = load %15
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:mat4x3<f32> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<vec3<f32>, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  u32:u32 @offset(0)
  vec3:__packed_vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:array<tint_packed_vec3_f32_array_element, 4> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<tint_packed_vec3_f32_array_element, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %v, 0u
    %4:u32 = load %3
    %5:ptr<storage, __packed_vec3<u32>, read_write> = access %v, 1u
    %6:__packed_vec3<u32> = load %5
    %7:vec3<u32> = msl.convert %6
    %8:ptr<storage, vec4<u32>, read_write> = access %v, 2u
    %9:vec4<u32> = load %8
    %10:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = access %v, 3u
    %11:array<tint_packed_vec3_f32_array_element, 4> = load %10
    %12:__packed_vec3<f32> = access %11, 0u, 0u
    %13:vec3<f32> = msl.convert %12
    %14:__packed_vec3<f32> = access %11, 1u, 0u
    %15:vec3<f32> = msl.convert %14
    %16:__packed_vec3<f32> = access %11, 2u, 0u
    %17:vec3<f32> = msl.convert %16
    %18:__packed_vec3<f32> = access %11, 3u, 0u
    %19:vec3<f32> = msl.convert %18
    %20:mat4x3<f32> = construct %13, %15, %17, %19
    %21:ptr<storage, mat3x2<f32>, read_write> = access %v, 4u
    %22:mat3x2<f32> = load %21
    %23:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %v, 5u
    %24:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3, %23
    %26:ptr<storage, array<vec4<f32>, 2>, read_write> = access %v, 6u
    %27:array<vec4<f32>, 2> = load %26
    ret
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {
  $B3: {
    %29:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 0u, 0u
    %30:__packed_vec3<f32> = load %29
    %31:vec3<f32> = msl.convert %30
    %32:ptr<storage, __packed_vec3<f32>, read_write> = access %from, 1u, 0u
    %33:__packed_vec3<f32> = load %32
    %34:vec3<f32> = msl.convert %33
    %35:array<vec3<f32>, 2> = construct %31, %34
    ret %35
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_WithUnpackedMembers_StoreStruct) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.Register("u32"), ty.u32()},
                            {mod.symbols.Register("vec3"), ty.vec3<u32>()},
                            {mod.symbols.Register("vec4"), ty.vec4<u32>()},
                            {mod.symbols.Register("mat3"), ty.mat4x3<f32>()},
                            {mod.symbols.Register("mat2"), ty.mat3x2<f32>()},
                            {mod.symbols.Register("arr3"), ty.array<vec3<f32>, 2>()},
                            {mod.symbols.Register("arr4"), ty.array<vec4<f32>, 2>()},
                        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    auto* value = b.FunctionParam("value", s);
    func->SetParams({value});
    b.Append(func->Block(), [&] {  //
        b.Store(var, value);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:mat4x3<f32> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<vec3<f32>, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:S):void {
  $B2: {
    store %v, %value
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:mat4x3<f32> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<vec3<f32>, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  u32:u32 @offset(0)
  vec3:__packed_vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(32)
  mat3:array<tint_packed_vec3_f32_array_element, 4> @offset(48)
  mat2:mat3x2<f32> @offset(112)
  arr3:array<tint_packed_vec3_f32_array_element, 2> @offset(144)
  arr4:array<vec4<f32>, 2> @offset(176)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%value:S):void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %v, %value
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, S_packed_vec3, read_write>, %value_1:S):void {  # %value_1: 'value'
  $B3: {
    %8:u32 = access %value_1, 0u
    %9:ptr<storage, u32, read_write> = access %to, 0u
    store %9, %8
    %10:vec3<u32> = access %value_1, 1u
    %11:ptr<storage, __packed_vec3<u32>, read_write> = access %to, 1u
    %12:__packed_vec3<u32> = msl.convert %10
    store %11, %12
    %13:vec4<u32> = access %value_1, 2u
    %14:ptr<storage, vec4<u32>, read_write> = access %to, 2u
    store %14, %13
    %15:mat4x3<f32> = access %value_1, 3u
    %16:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = access %to, 3u
    %17:ptr<storage, __packed_vec3<f32>, read_write> = access %16, 0u, 0u
    %18:vec3<f32> = access %15, 0u
    %19:__packed_vec3<f32> = msl.convert %18
    store %17, %19
    %20:ptr<storage, __packed_vec3<f32>, read_write> = access %16, 1u, 0u
    %21:vec3<f32> = access %15, 1u
    %22:__packed_vec3<f32> = msl.convert %21
    store %20, %22
    %23:ptr<storage, __packed_vec3<f32>, read_write> = access %16, 2u, 0u
    %24:vec3<f32> = access %15, 2u
    %25:__packed_vec3<f32> = msl.convert %24
    store %23, %25
    %26:ptr<storage, __packed_vec3<f32>, read_write> = access %16, 3u, 0u
    %27:vec3<f32> = access %15, 3u
    %28:__packed_vec3<f32> = msl.convert %27
    store %26, %28
    %29:mat3x2<f32> = access %value_1, 4u
    %30:ptr<storage, mat3x2<f32>, read_write> = access %to, 4u
    store %30, %29
    %31:array<vec3<f32>, 2> = access %value_1, 5u
    %32:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %to, 5u
    %33:void = call %tint_store_array_packed_vec3_1, %32, %31
    %35:array<vec4<f32>, 2> = access %value_1, 6u
    %36:ptr<storage, array<vec4<f32>, 2>, read_write> = access %to, 6u
    store %36, %35
    ret
  }
}
%tint_store_array_packed_vec3_1 = func(%to_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>, %value_2:array<vec3<f32>, 2>):void {  # %to_1: 'to', %value_2: 'value'
  $B4: {
    %39:vec3<f32> = access %value_2, 0u
    %40:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 0u, 0u
    %41:__packed_vec3<f32> = msl.convert %39
    store %40, %41
    %42:vec3<f32> = access %value_2, 1u
    %43:ptr<storage, __packed_vec3<f32>, read_write> = access %to_1, 1u, 0u
    %44:__packed_vec3<f32> = msl.convert %42
    store %43, %44
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_Struct_NonDefaultOffset) {
    auto* s = ty.Struct(
        mod.symbols.New("S"),
        Vector{
            ty.Get<core::type::StructMember>(mod.symbols.Register("u32"), ty.u32(), /* index */ 0u,
                                             /* offset */ 0u, /* align */ 4u, /* size */ 4u,
                                             core::IOAttributes{}),
            ty.Get<core::type::StructMember>(
                mod.symbols.Register("vec3"), ty.vec3<u32>(), /* index */ 1u,
                /* offset */ 16u, /* align */ 4u, /* size */ 12u, core::IOAttributes{}),
            ty.Get<core::type::StructMember>(
                mod.symbols.Register("vec4"), ty.vec4<u32>(), /* index */ 2u,
                /* offset */ 64u, /* align */ 4u, /* size */ 16u, core::IOAttributes{}),
            ty.Get<core::type::StructMember>(
                mod.symbols.Register("mat3"), ty.mat4x3<f32>(), /* index */ 3u,
                /* offset */ 128u, /* align */ 16u, /* size */ 64u, core::IOAttributes{}),
            ty.Get<core::type::StructMember>(
                mod.symbols.Register("mat2"), ty.mat3x2<f32>(), /* index */ 4u,
                /* offset */ 256u, /* align */ 8u, /* size */ 24u, core::IOAttributes{}),
            ty.Get<core::type::StructMember>(
                mod.symbols.Register("arr3"), ty.array<vec3<f32>, 2>(), /* index */ 5u,
                /* offset */ 1024u, /* align */ 16u, /* size */ 32u, core::IOAttributes{}),
        });

    auto* var = b.Var("v", storage, s);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(64)
  mat3:mat4x3<f32> @offset(128)
  mat2:mat3x2<f32> @offset(256)
  arr3:array<vec3<f32>, 2> @offset(1024)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  u32:u32 @offset(0)
  vec3:vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(64)
  mat3:mat4x3<f32> @offset(128)
  mat2:mat3x2<f32> @offset(256)
  arr3:array<vec3<f32>, 2> @offset(1024)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  u32:u32 @offset(0)
  vec3:__packed_vec3<u32> @offset(16)
  vec4:vec4<u32> @offset(64)
  mat3:array<tint_packed_vec3_f32_array_element, 4> @offset(128)
  mat2:mat3x2<f32> @offset(256)
  arr3:array<tint_packed_vec3_f32_array_element, 2> @offset(1024)
}

$B1: {  # root
  %v:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
}

)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_DeeplyNestedType) {
    auto* s = ty.Struct(mod.symbols.New("S"),
                        {
                            {mod.symbols.Register("vec3"), ty.vec3<u32>()},
                            {mod.symbols.Register("arr"), ty.array<mat2x3<f32>, 11>()},
                        });
    auto* outer = ty.array(s, 16);

    auto* var = b.Var("v", storage, outer);
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Let("load_outer_array", b.Load(var));
        b.Let("load_matrix_column",
              b.Load(b.Access(ty.ptr<storage, vec3<f32>>(), var, 7_u, 1_u, 3_u, 1_u)));
        b.Store(var, b.Construct(outer));
        b.StoreVectorElement(b.Access(ty.ptr<storage, vec3<f32>>(), var, 6_u, 1_u, 4_u, 0_u), 2_u,
                             42_f);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  vec3:vec3<u32> @offset(0)
  arr:array<mat2x3<f32>, 11> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<S, 16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:array<S, 16> = load %v
    %load_outer_array:array<S, 16> = let %3
    %5:ptr<storage, vec3<f32>, read_write> = access %v, 7u, 1u, 3u, 1u
    %6:vec3<f32> = load %5
    %load_matrix_column:vec3<f32> = let %6
    %8:array<S, 16> = construct
    store %v, %8
    %9:ptr<storage, vec3<f32>, read_write> = access %v, 6u, 1u, 4u, 0u
    store_vector_element %9, 2u, 42.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec3:vec3<u32> @offset(0)
  arr:array<mat2x3<f32>, 11> @offset(16)
}

tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  vec3:__packed_vec3<u32> @offset(0)
  arr:array<array<tint_packed_vec3_f32_array_element, 2>, 11> @offset(16)
}

$B1: {  # root
  %v:ptr<storage, array<S_packed_vec3, 16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:array<S, 16> = call %tint_load_array_packed_vec3, %v
    %load_outer_array:array<S, 16> = let %3
    %6:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 7u, 1u, 3u, 1u, 0u
    %7:__packed_vec3<f32> = load %6
    %8:vec3<f32> = msl.convert %7
    %load_matrix_column:vec3<f32> = let %8
    %10:array<S, 16> = construct
    %11:void = call %tint_store_array_packed_vec3, %v, %10
    %13:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 6u, 1u, 4u, 0u, 0u
    store_vector_element %13, 2u, 42.0f
    ret
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<storage, array<S_packed_vec3, 16>, read_write>):array<S, 16> {
  $B3: {
    %15:ptr<function, array<S, 16>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %17:bool = gte %idx, 16u
        if %17 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %18:ptr<function, S, read_write> = access %15, %idx
        %19:ptr<storage, S_packed_vec3, read_write> = access %from, %idx
        %20:S = call %tint_load_struct_packed_vec3, %19
        store %18, %20
        continue  # -> $B6
      }
      $B6: {  # continuing
        %22:u32 = add %idx, 1u
        next_iteration %22  # -> $B5
      }
    }
    %23:array<S, 16> = load %15
    ret %23
  }
}
%tint_load_struct_packed_vec3 = func(%from_1:ptr<storage, S_packed_vec3, read_write>):S {  # %from_1: 'from'
  $B8: {
    %25:ptr<storage, __packed_vec3<u32>, read_write> = access %from_1, 0u
    %26:__packed_vec3<u32> = load %25
    %27:vec3<u32> = msl.convert %26
    %28:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 11>, read_write> = access %from_1, 1u
    %29:array<mat2x3<f32>, 11> = call %tint_load_array_packed_vec3_1, %28
    %31:S = construct %27, %29
    ret %31
  }
}
%tint_load_array_packed_vec3_1 = func(%from_2:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 11>, read_write>):array<mat2x3<f32>, 11> {  # %from_2: 'from'
  $B9: {
    %33:ptr<function, array<mat2x3<f32>, 11>, read_write> = var undef
    loop [i: $B10, b: $B11, c: $B12] {  # loop_2
      $B10: {  # initializer
        next_iteration 0u  # -> $B11
      }
      $B11 (%idx_1:u32): {  # body
        %35:bool = gte %idx_1, 11u
        if %35 [t: $B13] {  # if_2
          $B13: {  # true
            exit_loop  # loop_2
          }
        }
        %36:ptr<function, mat2x3<f32>, read_write> = access %33, %idx_1
        %37:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %from_2, %idx_1
        %38:array<tint_packed_vec3_f32_array_element, 2> = load %37
        %39:__packed_vec3<f32> = access %38, 0u, 0u
        %40:vec3<f32> = msl.convert %39
        %41:__packed_vec3<f32> = access %38, 1u, 0u
        %42:vec3<f32> = msl.convert %41
        %43:mat2x3<f32> = construct %40, %42
        store %36, %43
        continue  # -> $B12
      }
      $B12: {  # continuing
        %44:u32 = add %idx_1, 1u
        next_iteration %44  # -> $B11
      }
    }
    %45:array<mat2x3<f32>, 11> = load %33
    ret %45
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, array<S_packed_vec3, 16>, read_write>, %value:array<S, 16>):void {
  $B14: {
    loop [i: $B15, b: $B16, c: $B17] {  # loop_3
      $B15: {  # initializer
        next_iteration 0u  # -> $B16
      }
      $B16 (%idx_2:u32): {  # body
        %49:bool = gte %idx_2, 16u
        if %49 [t: $B18] {  # if_3
          $B18: {  # true
            exit_loop  # loop_3
          }
        }
        %50:S = access %value, %idx_2
        %51:ptr<storage, S_packed_vec3, read_write> = access %to, %idx_2
        %52:void = call %tint_store_array_packed_vec3_1, %51, %50
        continue  # -> $B17
      }
      $B17: {  # continuing
        %54:u32 = add %idx_2, 1u
        next_iteration %54  # -> $B16
      }
    }
    ret
  }
}
%tint_store_array_packed_vec3_1 = func(%to_1:ptr<storage, S_packed_vec3, read_write>, %value_1:S):void {  # %to_1: 'to', %value_1: 'value'
  $B19: {
    %57:vec3<u32> = access %value_1, 0u
    %58:ptr<storage, __packed_vec3<u32>, read_write> = access %to_1, 0u
    %59:__packed_vec3<u32> = msl.convert %57
    store %58, %59
    %60:array<mat2x3<f32>, 11> = access %value_1, 1u
    %61:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 11>, read_write> = access %to_1, 1u
    %62:void = call %tint_store_array_packed_vec3_2, %61, %60
    ret
  }
}
%tint_store_array_packed_vec3_2 = func(%to_2:ptr<storage, array<array<tint_packed_vec3_f32_array_element, 2>, 11>, read_write>, %value_2:array<mat2x3<f32>, 11>):void {  # %to_2: 'to', %value_2: 'value'
  $B20: {
    loop [i: $B21, b: $B22, c: $B23] {  # loop_4
      $B21: {  # initializer
        next_iteration 0u  # -> $B22
      }
      $B22 (%idx_3:u32): {  # body
        %67:bool = gte %idx_3, 11u
        if %67 [t: $B24] {  # if_4
          $B24: {  # true
            exit_loop  # loop_4
          }
        }
        %68:mat2x3<f32> = access %value_2, %idx_3
        %69:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = access %to_2, %idx_3
        %70:ptr<storage, __packed_vec3<f32>, read_write> = access %69, 0u, 0u
        %71:vec3<f32> = access %68, 0u
        %72:__packed_vec3<f32> = msl.convert %71
        store %70, %72
        %73:ptr<storage, __packed_vec3<f32>, read_write> = access %69, 1u, 0u
        %74:vec3<f32> = access %68, 1u
        %75:__packed_vec3<f32> = msl.convert %74
        store %73, %75
        continue  # -> $B23
      }
      $B23: {  # continuing
        %76:u32 = add %idx_3, 1u
        next_iteration %76  # -> $B22
      }
    }
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_PointerInLet) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* mat_ptr = b.Let("mat", var);
        auto* col_ptr = b.Let("col", b.Access(ty.ptr<storage, vec3<f32>>(), mat_ptr, 1_u));
        auto* col = b.Load(col_ptr);
        b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), mat_ptr, 2_u), col);
        b.StoreVectorElement(col_ptr, 2_u, 42_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %mat:ptr<storage, mat4x3<f32>, read_write> = let %v
    %4:ptr<storage, vec3<f32>, read_write> = access %mat, 1u
    %col:ptr<storage, vec3<f32>, read_write> = let %4
    %6:vec3<f32> = load %col
    %7:ptr<storage, vec3<f32>, read_write> = access %mat, 2u
    store %7, %6
    store_vector_element %col, 2u, 42.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %mat:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = let %v
    %4:ptr<storage, __packed_vec3<f32>, read_write> = access %mat, 1u, 0u
    %col:ptr<storage, __packed_vec3<f32>, read_write> = let %4
    %6:__packed_vec3<f32> = load %col
    %7:vec3<f32> = msl.convert %6
    %8:ptr<storage, __packed_vec3<f32>, read_write> = access %mat, 2u, 0u
    %9:__packed_vec3<f32> = msl.convert %7
    store %8, %9
    store_vector_element %col, 2u, 42.0f
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_PointerInFunctionParameter) {
    auto* var = b.Var<storage, mat4x3<f32>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* bar = b.Function("bar", ty.void_());
    {
        auto* mat_ptr = b.FunctionParam("mat", (ty.ptr<storage, mat4x3<f32>>()));
        auto* col_ptr = b.FunctionParam("col", (ty.ptr<storage, vec3<f32>>()));
        bar->SetParams({mat_ptr, col_ptr});
        b.Append(bar->Block(), [&] {  //
            auto* col = b.Load(col_ptr);
            b.Store(b.Access(ty.ptr<storage, vec3<f32>>(), mat_ptr, 2_u), col);
            b.StoreVectorElement(col_ptr, 2_u, 42_f);
            b.Return(bar);
        });
    }

    auto* foo = b.Function("foo", ty.void_());
    {
        b.Append(foo->Block(), [&] {  //
            auto* mat_ptr = b.Let("mat", var);
            auto* col_ptr = b.Let("col", b.Access(ty.ptr<storage, vec3<f32>>(), var, 1_u));
            b.Call(bar, mat_ptr, col_ptr);
            b.Return(foo);
        });
    }

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, mat4x3<f32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%mat:ptr<storage, mat4x3<f32>, read_write>, %col:ptr<storage, vec3<f32>, read_write>):void {
  $B2: {
    %5:vec3<f32> = load %col
    %6:ptr<storage, vec3<f32>, read_write> = access %mat, 2u
    store %6, %5
    store_vector_element %col, 2u, 42.0f
    ret
  }
}
%foo = func():void {
  $B3: {
    %mat_1:ptr<storage, mat4x3<f32>, read_write> = let %v  # %mat_1: 'mat'
    %9:ptr<storage, vec3<f32>, read_write> = access %v, 1u
    %col_1:ptr<storage, vec3<f32>, read_write> = let %9  # %col_1: 'col'
    %11:void = call %bar, %mat_1, %col_1
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%mat:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write>, %col:ptr<storage, __packed_vec3<f32>, read_write>):void {
  $B2: {
    %5:__packed_vec3<f32> = load %col
    %6:vec3<f32> = msl.convert %5
    %7:ptr<storage, __packed_vec3<f32>, read_write> = access %mat, 2u, 0u
    %8:__packed_vec3<f32> = msl.convert %6
    store %7, %8
    store_vector_element %col, 2u, 42.0f
    ret
  }
}
%foo = func():void {
  $B3: {
    %mat_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 4>, read_write> = let %v  # %mat_1: 'mat'
    %11:ptr<storage, __packed_vec3<f32>, read_write> = access %v, 1u, 0u
    %col_1:ptr<storage, __packed_vec3<f32>, read_write> = let %11  # %col_1: 'col'
    %13:void = call %bar, %mat_1, %col_1
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, StorageVar_ArrayLengthBuiltinCall) {
    auto* var = b.Var<storage, array<vec3<f32>>>("v");
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* foo = b.Function("foo", ty.u32());
    b.Append(foo->Block(), [&] {  //
        auto* len = b.Call<u32>(core::BuiltinFn::kArrayLength, var);
        b.Return(foo, len);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<vec3<f32>>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = arrayLength %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, array<tint_packed_vec3_f32_array_element>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:u32 = arrayLength %v
    ret %3
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, MultipleAddressSpaces_LoadArray) {
    auto* uvar = b.Var<uniform, array<vec3<f32>, 2>>("u");
    uvar->SetBindingPoint(0, 0);
    mod.root_block->Append(uvar);
    auto* svar = b.Var<storage, array<vec3<f32>, 2>>("s");
    svar->SetBindingPoint(0, 1);
    mod.root_block->Append(svar);
    auto* wvar = b.Var<workgroup, array<vec3<f32>, 2>>("w");
    mod.root_block->Append(wvar);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Let("u_load", b.Load(uvar));
        b.Let("s_load", b.Load(svar));
        b.Let("w_load", b.Load(wvar));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %u:ptr<uniform, array<vec3<f32>, 2>, read> = var undef @binding_point(0, 0)
  %s:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 1)
  %w:ptr<workgroup, array<vec3<f32>, 2>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %5:array<vec3<f32>, 2> = load %u
    %u_load:array<vec3<f32>, 2> = let %5
    %7:array<vec3<f32>, 2> = load %s
    %s_load:array<vec3<f32>, 2> = let %7
    %9:array<vec3<f32>, 2> = load %w
    %w_load:array<vec3<f32>, 2> = let %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %u:ptr<uniform, array<tint_packed_vec3_f32_array_element, 2>, read> = var undef @binding_point(0, 0)
  %s:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 1)
  %w:ptr<workgroup, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %5:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3, %u
    %u_load:array<vec3<f32>, 2> = let %5
    %8:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3_1, %s
    %s_load:array<vec3<f32>, 2> = let %8
    %11:array<vec3<f32>, 2> = call %tint_load_array_packed_vec3_2, %w
    %w_load:array<vec3<f32>, 2> = let %11
    ret
  }
}
%tint_load_array_packed_vec3 = func(%from:ptr<uniform, array<tint_packed_vec3_f32_array_element, 2>, read>):array<vec3<f32>, 2> {
  $B3: {
    %15:ptr<uniform, __packed_vec3<f32>, read> = access %from, 0u, 0u
    %16:__packed_vec3<f32> = load %15
    %17:vec3<f32> = msl.convert %16
    %18:ptr<uniform, __packed_vec3<f32>, read> = access %from, 1u, 0u
    %19:__packed_vec3<f32> = load %18
    %20:vec3<f32> = msl.convert %19
    %21:array<vec3<f32>, 2> = construct %17, %20
    ret %21
  }
}
%tint_load_array_packed_vec3_1 = func(%from_1:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {  # %from_1: 'from'
  $B4: {
    %23:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 0u, 0u
    %24:__packed_vec3<f32> = load %23
    %25:vec3<f32> = msl.convert %24
    %26:ptr<storage, __packed_vec3<f32>, read_write> = access %from_1, 1u, 0u
    %27:__packed_vec3<f32> = load %26
    %28:vec3<f32> = msl.convert %27
    %29:array<vec3<f32>, 2> = construct %25, %28
    ret %29
  }
}
%tint_load_array_packed_vec3_2 = func(%from_2:ptr<workgroup, array<tint_packed_vec3_f32_array_element, 2>, read_write>):array<vec3<f32>, 2> {  # %from_2: 'from'
  $B5: {
    %31:ptr<workgroup, __packed_vec3<f32>, read_write> = access %from_2, 0u, 0u
    %32:__packed_vec3<f32> = load %31
    %33:vec3<f32> = msl.convert %32
    %34:ptr<workgroup, __packed_vec3<f32>, read_write> = access %from_2, 1u, 0u
    %35:__packed_vec3<f32> = load %34
    %36:vec3<f32> = msl.convert %35
    %37:array<vec3<f32>, 2> = construct %33, %36
    ret %37
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, MultipleAddressSpaces_StoreArray) {
    auto* svar = b.Var<storage, array<vec3<f32>, 2>>("s");
    svar->SetBindingPoint(0, 0);
    mod.root_block->Append(svar);
    auto* wvar = b.Var<workgroup, array<vec3<f32>, 2>>("w");
    mod.root_block->Append(wvar);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Store(svar, b.Zero<array<vec3<f32>, 2>>());
        b.Store(wvar, b.Zero<array<vec3<f32>, 2>>());
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %s:ptr<storage, array<vec3<f32>, 2>, read_write> = var undef @binding_point(0, 0)
  %w:ptr<workgroup, array<vec3<f32>, 2>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    store %s, array<vec3<f32>, 2>(vec3<f32>(0.0f))
    store %w, array<vec3<f32>, 2>(vec3<f32>(0.0f))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_packed_vec3_f32_array_element = struct @align(16) {
  packed:__packed_vec3<f32> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef @binding_point(0, 0)
  %w:ptr<workgroup, array<tint_packed_vec3_f32_array_element, 2>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %s, array<vec3<f32>, 2>(vec3<f32>(0.0f))
    %6:void = call %tint_store_array_packed_vec3_1, %w, array<vec3<f32>, 2>(vec3<f32>(0.0f))
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, array<tint_packed_vec3_f32_array_element, 2>, read_write>, %value:array<vec3<f32>, 2>):void {
  $B3: {
    %10:vec3<f32> = access %value, 0u
    %11:ptr<storage, __packed_vec3<f32>, read_write> = access %to, 0u, 0u
    %12:__packed_vec3<f32> = msl.convert %10
    store %11, %12
    %13:vec3<f32> = access %value, 1u
    %14:ptr<storage, __packed_vec3<f32>, read_write> = access %to, 1u, 0u
    %15:__packed_vec3<f32> = msl.convert %13
    store %14, %15
    ret
  }
}
%tint_store_array_packed_vec3_1 = func(%to_1:ptr<workgroup, array<tint_packed_vec3_f32_array_element, 2>, read_write>, %value_1:array<vec3<f32>, 2>):void {  # %to_1: 'to', %value_1: 'value'
  $B4: {
    %18:vec3<f32> = access %value_1, 0u
    %19:ptr<workgroup, __packed_vec3<f32>, read_write> = access %to_1, 0u, 0u
    %20:__packed_vec3<f32> = msl.convert %18
    store %19, %20
    %21:vec3<f32> = access %value_1, 1u
    %22:ptr<workgroup, __packed_vec3<f32>, read_write> = access %to_1, 1u, 0u
    %23:__packed_vec3<f32> = msl.convert %21
    store %22, %23
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, MultipleAddressSpaces_LoadStruct) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("vec"), ty.vec3<u32>()},
                                                  {mod.symbols.Register("u"), ty.u32()},
                                              });

    auto* uvar = b.Var("u", ty.ptr<uniform>(s));
    uvar->SetBindingPoint(0, 0);
    mod.root_block->Append(uvar);
    auto* svar = b.Var("s", ty.ptr<storage>(s));
    svar->SetBindingPoint(0, 1);
    mod.root_block->Append(svar);
    auto* wvar = b.Var("w", ty.ptr<workgroup>(s));
    mod.root_block->Append(wvar);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Let("u_load", b.Load(uvar));
        b.Let("s_load", b.Load(svar));
        b.Let("w_load", b.Load(wvar));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:u32 @offset(12)
}

$B1: {  # root
  %u:ptr<uniform, S, read> = var undef @binding_point(0, 0)
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 1)
  %w:ptr<workgroup, S, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %5:S = load %u
    %u_load:S = let %5
    %7:S = load %s
    %s_load:S = let %7
    %9:S = load %w
    %w_load:S = let %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:u32 @offset(12)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<u32> @offset(0)
  u:u32 @offset(12)
}

$B1: {  # root
  %u:ptr<uniform, S_packed_vec3, read> = var undef @binding_point(0, 0)
  %s:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 1)
  %w:ptr<workgroup, S_packed_vec3, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %5:S = call %tint_load_struct_packed_vec3, %u
    %u_load:S = let %5
    %8:S = call %tint_load_struct_packed_vec3_1, %s
    %s_load:S = let %8
    %11:S = call %tint_load_struct_packed_vec3_2, %w
    %w_load:S = let %11
    ret
  }
}
%tint_load_struct_packed_vec3 = func(%from:ptr<uniform, S_packed_vec3, read>):S {
  $B3: {
    %15:ptr<uniform, __packed_vec3<u32>, read> = access %from, 0u
    %16:__packed_vec3<u32> = load %15
    %17:vec3<u32> = msl.convert %16
    %18:ptr<uniform, u32, read> = access %from, 1u
    %19:u32 = load %18
    %20:S = construct %17, %19
    ret %20
  }
}
%tint_load_struct_packed_vec3_1 = func(%from_1:ptr<storage, S_packed_vec3, read_write>):S {  # %from_1: 'from'
  $B4: {
    %22:ptr<storage, __packed_vec3<u32>, read_write> = access %from_1, 0u
    %23:__packed_vec3<u32> = load %22
    %24:vec3<u32> = msl.convert %23
    %25:ptr<storage, u32, read_write> = access %from_1, 1u
    %26:u32 = load %25
    %27:S = construct %24, %26
    ret %27
  }
}
%tint_load_struct_packed_vec3_2 = func(%from_2:ptr<workgroup, S_packed_vec3, read_write>):S {  # %from_2: 'from'
  $B5: {
    %29:ptr<workgroup, __packed_vec3<u32>, read_write> = access %from_2, 0u
    %30:__packed_vec3<u32> = load %29
    %31:vec3<u32> = msl.convert %30
    %32:ptr<workgroup, u32, read_write> = access %from_2, 1u
    %33:u32 = load %32
    %34:S = construct %31, %33
    ret %34
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, MultipleAddressSpaces_StoreStruct) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("vec"), ty.vec3<u32>()},
                                                  {mod.symbols.Register("u"), ty.u32()},
                                              });

    auto* svar = b.Var("s", ty.ptr<storage>(s));
    svar->SetBindingPoint(0, 0);
    mod.root_block->Append(svar);
    auto* wvar = b.Var("w", ty.ptr<workgroup>(s));
    mod.root_block->Append(wvar);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Store(svar, b.Zero(s));
        b.Store(wvar, b.Zero(s));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:u32 @offset(12)
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %w:ptr<workgroup, S, read_write> = var undef
}

%foo = func():void {
  $B2: {
    store %s, S(vec3<u32>(0u), 0u)
    store %w, S(vec3<u32>(0u), 0u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:u32 @offset(12)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<u32> @offset(0)
  u:u32 @offset(12)
}

$B1: {  # root
  %s:ptr<storage, S_packed_vec3, read_write> = var undef @binding_point(0, 0)
  %w:ptr<workgroup, S_packed_vec3, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %4:void = call %tint_store_array_packed_vec3, %s, S(vec3<u32>(0u), 0u)
    %6:void = call %tint_store_array_packed_vec3_1, %w, S(vec3<u32>(0u), 0u)
    ret
  }
}
%tint_store_array_packed_vec3 = func(%to:ptr<storage, S_packed_vec3, read_write>, %value:S):void {
  $B3: {
    %10:vec3<u32> = access %value, 0u
    %11:ptr<storage, __packed_vec3<u32>, read_write> = access %to, 0u
    %12:__packed_vec3<u32> = msl.convert %10
    store %11, %12
    %13:u32 = access %value, 1u
    %14:ptr<storage, u32, read_write> = access %to, 1u
    store %14, %13
    ret
  }
}
%tint_store_array_packed_vec3_1 = func(%to_1:ptr<workgroup, S_packed_vec3, read_write>, %value_1:S):void {  # %to_1: 'to', %value_1: 'value'
  $B4: {
    %17:vec3<u32> = access %value_1, 0u
    %18:ptr<workgroup, __packed_vec3<u32>, read_write> = access %to_1, 0u
    %19:__packed_vec3<u32> = msl.convert %17
    store %18, %19
    %20:u32 = access %value_1, 1u
    %21:ptr<workgroup, u32, read_write> = access %to_1, 1u
    store %21, %20
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, AtomicOnPackedStructMember) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("vec"), ty.vec3<u32>()},
                                                  {mod.symbols.Register("u"), ty.atomic<u32>()},
                                              });

    auto* var = b.Var("v", ty.ptr<workgroup>(s));
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {  //
        auto* p = b.Access<ptr<workgroup, atomic<u32>>>(var, 1_u);
        auto* result = b.Call<u32>(core::BuiltinFn::kAtomicLoad, p);
        b.Return(func, result);
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:atomic<u32> @offset(12)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %v, 1u
    %4:u32 = atomicLoad %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:atomic<u32> @offset(12)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<u32> @offset(0)
  u:atomic<u32> @offset(12)
}

$B1: {  # root
  %v:ptr<workgroup, S_packed_vec3, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %v, 1u
    %4:u32 = atomicLoad %3
    ret %4
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, AtomicOnPackedStructMember_ViaLet) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("vec"), ty.vec3<u32>()},
                                                  {mod.symbols.Register("u"), ty.atomic<u32>()},
                                              });

    auto* var = b.Var("v", ty.ptr<workgroup>(s));
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {  //
        auto* p = b.Let("p", b.Access<ptr<workgroup, atomic<u32>>>(var, 1_u));
        auto* result = b.Call<u32>(core::BuiltinFn::kAtomicLoad, p);
        b.Return(func, result);
    });

    auto* src = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:atomic<u32> @offset(12)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %v, 1u
    %p:ptr<workgroup, atomic<u32>, read_write> = let %3
    %5:u32 = atomicLoad %p
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  vec:vec3<u32> @offset(0)
  u:atomic<u32> @offset(12)
}

S_packed_vec3 = struct @align(16) {
  vec:__packed_vec3<u32> @offset(0)
  u:atomic<u32> @offset(12)
}

$B1: {  # root
  %v:ptr<workgroup, S_packed_vec3, read_write> = var undef
}

%foo = func():u32 {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %v, 1u
    %p:ptr<workgroup, atomic<u32>, read_write> = let %3
    %5:u32 = atomicLoad %p
    ret %5
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

// Workgroup is the only address space that requires packed types that supports bool types.
// These are rewritten as packed_vec3<u32> types since MSL does not support packed bool vectors.
TEST_F(MslWriter_PackedVec3Test, WorkgroupVar_Vec3_Bool) {
    auto* var = b.Var<workgroup, vec3<bool>>("v");
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.vec3<bool>());
    b.Append(func->Block(), [&] {  //
        b.Store(var, b.Zero<vec3<bool>>());
        b.Return(func, b.Load(var));
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, vec3<bool>, read_write> = var undef
}

%foo = func():vec3<bool> {
  $B2: {
    store %v, vec3<bool>(false)
    %3:vec3<bool> = load %v
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, __packed_vec3<u32>, read_write> = var undef
}

%foo = func():vec3<bool> {
  $B2: {
    %3:vec3<u32> = convert vec3<bool>(false)
    %4:__packed_vec3<u32> = msl.convert %3
    store %v, %4
    %5:__packed_vec3<u32> = load %v
    %6:vec3<u32> = msl.convert %5
    %7:vec3<bool> = convert %6
    ret %7
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

// Workgroup is the only address space that requires packed types that supports bool types.
// These are rewritten as packed_vec3<u32> types since MSL does not support packed bool vectors.
TEST_F(MslWriter_PackedVec3Test, WorkgroupVar_Vec3_Bool_VectorElementLoadAndStore) {
    auto* var = b.Var<workgroup, vec3<bool>>("v");
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* el = b.LoadVectorElement(var, 0_u);
        b.StoreVectorElement(var, 1_u, el);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, vec3<bool>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:bool = load_vector_element %v, 0u
    store_vector_element %v, 1u, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, __packed_vec3<u32>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:u32 = load_vector_element %v, 0u
    %4:bool = convert %3
    %5:u32 = convert %4
    store_vector_element %v, 1u, %5
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_PackedVec3Test, WorkgroupVar_Struct_Vec3_Bool_VectorElementLoadAndStore) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("data"), ty.vec3<bool>()},
                                              });
    auto* var = b.Var("v", ty.ptr<workgroup>(s));
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        auto* ptr = b.Access(ty.ptr<workgroup, vec3<bool>>(), var, 0_u);
        auto* el = b.LoadVectorElement(ptr, 0_u);
        b.StoreVectorElement(ptr, 1_u, el);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  data:vec3<bool> @offset(0)
}

$B1: {  # root
  %v:ptr<workgroup, S, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, vec3<bool>, read_write> = access %v, 0u
    %4:bool = load_vector_element %3, 0u
    store_vector_element %3, 1u, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  data:vec3<bool> @offset(0)
}

S_packed_vec3 = struct @align(16) {
  data:__packed_vec3<u32> @offset(0)
}

$B1: {  # root
  %v:ptr<workgroup, S_packed_vec3, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:ptr<workgroup, __packed_vec3<u32>, read_write> = access %v, 0u
    %4:u32 = load_vector_element %3, 0u
    %5:bool = convert %4
    %6:u32 = convert %5
    store_vector_element %3, 1u, %6
    ret
  }
}
)";

    Run(PackedVec3);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
