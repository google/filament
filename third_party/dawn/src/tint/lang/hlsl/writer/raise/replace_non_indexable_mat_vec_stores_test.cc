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

#include "src/tint/lang/hlsl/writer/raise/replace_non_indexable_mat_vec_stores.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterReplaceNonIndexableMatVecStoresTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, Vector) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.vec3<f32>()));
        b.StoreVectorElement(v, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(v, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, vec3<f32>, read_write> = var undef
    %5:u32 = load %dyn_index
    store_vector_element %v, %5, 1.0f
    store_vector_element %v, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, vec3<f32>, read_write> = var undef
    %5:u32 = load %dyn_index
    %6:vec3<f32> = load %v
    %7:vec3<f32> = construct 1.0f
    %8:vec3<f32> = construct %5
    %9:vec3<f32> = construct 0i, 1i, 2i
    %10:vec3<bool> = eq %8, %9
    %11:vec3<f32> = select %6, %7, %10
    store %v, %11
    store_vector_element %v, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, VectorInStruct) {
    auto* vec_ty = ty.vec3<f32>();
    auto* struct_ty = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("v"), vec_ty}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(struct_ty));
        auto* access = b.Access(ty.ptr<function>(vec_ty), v, 0_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  v:vec3<f32> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec3<f32>, read_write> = access %v, 0u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  v:vec3<f32> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec3<f32>, read_write> = access %v, 0u
    %6:u32 = load %dyn_index
    %7:vec3<f32> = load %5
    %8:vec3<f32> = construct 1.0f
    %9:vec3<f32> = construct %6
    %10:vec3<f32> = construct 0i, 1i, 2i
    %11:vec3<bool> = eq %9, %10
    %12:vec3<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, VectorInArray) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* vec_ty = ty.vec3<f32>();
        auto* v = b.Var("v", ty.ptr<function>(ty.array(vec_ty, 8)));
        auto* access = b.Access(ty.ptr<function>(vec_ty), v, 0_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, array<vec3<f32>, 8>, read_write> = var undef
    %5:ptr<function, vec3<f32>, read_write> = access %v, 0u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, array<vec3<f32>, 8>, read_write> = var undef
    %5:ptr<function, vec3<f32>, read_write> = access %v, 0u
    %6:u32 = load %dyn_index
    %7:vec3<f32> = load %5
    %8:vec3<f32> = construct 1.0f
    %9:vec3<f32> = construct %6
    %10:vec3<f32> = construct 0i, 1i, 2i
    %11:vec3<bool> = eq %9, %10
    %12:vec3<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, VectorInArrayInStruct) {
    auto* vec_ty = ty.vec3<f32>();
    auto* struct_ty =
        ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("v"), ty.array(vec_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(struct_ty));
        auto* access = b.Access(ty.ptr<function>(vec_ty), v, 0_u, 0_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  v:array<vec3<f32>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec3<f32>, read_write> = access %v, 0u, 0u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  v:array<vec3<f32>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec3<f32>, read_write> = access %v, 0u, 0u
    %6:u32 = load %dyn_index
    %7:vec3<f32> = load %5
    %8:vec3<f32> = construct 1.0f
    %9:vec3<f32> = construct %6
    %10:vec3<f32> = construct 0i, 1i, 2i
    %11:vec3<bool> = eq %9, %10
    %12:vec3<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, VectorByFunc) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* get_dynamic = b.Function("get_dynamic", ty.u32());
    b.Append(get_dynamic->Block(), [&] { b.Return(get_dynamic, b.Load(dyn_index)); });
    auto* get_static = b.Function("get_static", ty.u32());
    b.Append(get_static->Block(), [&] { b.Return(get_static, 0_u); });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function>(ty.vec3<f32>()));
        b.StoreVectorElement(v, b.Call(get_dynamic), 1_f);
        // Will be transformed because we assume functions return a dynamic value
        b.StoreVectorElement(v, b.Call(get_static), 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%get_dynamic = func():u32 {
  $B2: {
    %3:u32 = load %dyn_index
    ret %3
  }
}
%get_static = func():u32 {
  $B3: {
    ret 0u
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %v:ptr<function, vec3<f32>, read_write> = var undef
    %7:u32 = call %get_dynamic
    store_vector_element %v, %7, 1.0f
    %8:u32 = call %get_static
    store_vector_element %v, %8, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%get_dynamic = func():u32 {
  $B2: {
    %3:u32 = load %dyn_index
    ret %3
  }
}
%get_static = func():u32 {
  $B3: {
    ret 0u
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %v:ptr<function, vec3<f32>, read_write> = var undef
    %7:u32 = call %get_dynamic
    %8:vec3<f32> = load %v
    %9:vec3<f32> = construct 1.0f
    %10:vec3<f32> = construct %7
    %11:vec3<f32> = construct 0i, 1i, 2i
    %12:vec3<bool> = eq %10, %11
    %13:vec3<f32> = select %8, %9, %12
    store %v, %13
    %14:u32 = call %get_static
    %15:vec3<f32> = load %v
    %16:vec3<f32> = construct 1.0f
    %17:vec3<f32> = construct %14
    %18:vec3<f32> = construct 0i, 1i, 2i
    %19:vec3<bool> = eq %17, %18
    %20:vec3<f32> = select %15, %16, %19
    store %v, %20
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, Vector_ViaPointer) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.vec3<f32>()));
        auto* p = b.Let("p", v);
        b.StoreVectorElement(p, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(p, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, vec3<f32>, read_write> = var undef
    %p:ptr<function, vec3<f32>, read_write> = let %v
    %6:u32 = load %dyn_index
    store_vector_element %p, %6, 1.0f
    store_vector_element %p, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, vec3<f32>, read_write> = var undef
    %5:u32 = load %dyn_index
    %6:vec3<f32> = load %v
    %7:vec3<f32> = construct 1.0f
    %8:vec3<f32> = construct %5
    %9:vec3<f32> = construct 0i, 1i, 2i
    %10:vec3<bool> = eq %8, %9
    %11:vec3<f32> = select %6, %7, %10
    store %v, %11
    store_vector_element %v, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, Vector_PrivateVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<private_>(ty.vec3<f32>()));
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        b.StoreVectorElement(v, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(v, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<private, vec3<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:u32 = load %dyn_index
    store_vector_element %v, %5, 1.0f
    store_vector_element %v, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<private, vec3<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:u32 = load %dyn_index
    %6:vec3<f32> = load %v
    %7:vec3<f32> = construct 1.0f
    %8:vec3<f32> = construct %5
    %9:vec3<f32> = construct 0i, 1i, 2i
    %10:vec3<bool> = eq %8, %9
    %11:vec3<f32> = select %6, %7, %10
    store %v, %11
    store_vector_element %v, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, Vector_StorageVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<storage>(ty.vec3<f32>()));
    v->SetBindingPoint(0, 1);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        b.StoreVectorElement(v, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(v, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<storage, vec3<f32>, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:u32 = load %dyn_index
    store_vector_element %v, %5, 1.0f
    store_vector_element %v, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, Vector_WorkgroupVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<workgroup>(ty.vec3<f32>()));
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        b.StoreVectorElement(v, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(v, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<workgroup, vec3<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:u32 = load %dyn_index
    store_vector_element %v, %5, 1.0f
    store_vector_element %v, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElement) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.mat2x4<f32>()));
        auto* access = b.Access(ty.ptr<function, vec4<f32>>(), v, 1_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 1u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 1u
    %6:u32 = load %dyn_index
    %7:vec4<f32> = load %5
    %8:vec4<f32> = construct 1.0f
    %9:vec4<f32> = construct %6
    %10:vec4<f32> = construct 0i, 1i, 2i, 3i
    %11:vec4<bool> = eq %9, %10
    %12:vec4<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElementInStruct) {
    auto* mat_ty = ty.mat2x4<f32>();
    auto* struct_ty = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("v"), mat_ty}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(struct_ty));
        auto* access = b.Access(ty.ptr<function, vec4<f32>>(), v, 0_u, 1_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  v:mat2x4<f32> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 0u, 1u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  v:mat2x4<f32> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 0u, 1u
    %6:u32 = load %dyn_index
    %7:vec4<f32> = load %5
    %8:vec4<f32> = construct 1.0f
    %9:vec4<f32> = construct %6
    %10:vec4<f32> = construct 0i, 1i, 2i, 3i
    %11:vec4<bool> = eq %9, %10
    %12:vec4<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElementInArray) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.array(ty.mat2x4<f32>(), 8)));
        auto* access = b.Access(ty.ptr<function, vec4<f32>>(), v, 7_u, 1_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, array<mat2x4<f32>, 8>, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 7u, 1u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, array<mat2x4<f32>, 8>, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 7u, 1u
    %6:u32 = load %dyn_index
    %7:vec4<f32> = load %5
    %8:vec4<f32> = construct 1.0f
    %9:vec4<f32> = construct %6
    %10:vec4<f32> = construct 0i, 1i, 2i, 3i
    %11:vec4<bool> = eq %9, %10
    %12:vec4<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElementInArrayInStruct) {
    auto* mat_ty = ty.mat2x4<f32>();
    auto* struct_ty =
        ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("v"), ty.array(mat_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(struct_ty));
        auto* access = b.Access(ty.ptr<function, vec4<f32>>(), v, 0_u, 7_u, 1_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  v:array<mat2x4<f32>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 0u, 7u, 1u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  v:array<mat2x4<f32>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 0u, 7u, 1u
    %6:u32 = load %dyn_index
    %7:vec4<f32> = load %5
    %8:vec4<f32> = construct 1.0f
    %9:vec4<f32> = construct %6
    %10:vec4<f32> = construct 0i, 1i, 2i, 3i
    %11:vec4<bool> = eq %9, %10
    %12:vec4<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

// MatrixElementByFunc
TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElementByFunc) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* get_dynamic = b.Function("get_dynamic", ty.u32());
    b.Append(get_dynamic->Block(), [&] { b.Return(get_dynamic, b.Load(dyn_index)); });
    auto* get_static = b.Function("get_static", ty.u32());
    b.Append(get_static->Block(), [&] { b.Return(get_static, 0_u); });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function>(ty.mat2x4<f32>()));
        auto* access = b.Access(ty.ptr<function, vec4<f32>>(), v, 1_u);
        b.StoreVectorElement(access, b.Call(get_dynamic), 1_f);
        // Will be transformed because we assume functions return a dynamic value
        b.StoreVectorElement(access, b.Call(get_static), 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%get_dynamic = func():u32 {
  $B2: {
    %3:u32 = load %dyn_index
    ret %3
  }
}
%get_static = func():u32 {
  $B3: {
    ret 0u
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %7:ptr<function, vec4<f32>, read_write> = access %v, 1u
    %8:u32 = call %get_dynamic
    store_vector_element %7, %8, 1.0f
    %9:u32 = call %get_static
    store_vector_element %7, %9, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%get_dynamic = func():u32 {
  $B2: {
    %3:u32 = load %dyn_index
    ret %3
  }
}
%get_static = func():u32 {
  $B3: {
    ret 0u
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %7:ptr<function, vec4<f32>, read_write> = access %v, 1u
    %8:u32 = call %get_dynamic
    %9:vec4<f32> = load %7
    %10:vec4<f32> = construct 1.0f
    %11:vec4<f32> = construct %8
    %12:vec4<f32> = construct 0i, 1i, 2i, 3i
    %13:vec4<bool> = eq %11, %12
    %14:vec4<f32> = select %9, %10, %13
    store %7, %14
    %15:u32 = call %get_static
    %16:vec4<f32> = load %7
    %17:vec4<f32> = construct 1.0f
    %18:vec4<f32> = construct %15
    %19:vec4<f32> = construct 0i, 1i, 2i, 3i
    %20:vec4<bool> = eq %18, %19
    %21:vec4<f32> = select %16, %17, %20
    store %7, %21
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElement_ViaPointer) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.mat2x4<f32>()));
        auto* access = b.Access(ty.ptr<function, vec4<f32>>(), v, 1_u);
        auto* p = b.Let("p", access);
        b.StoreVectorElement(p, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(p, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 1u
    %p:ptr<function, vec4<f32>, read_write> = let %5
    %7:u32 = load %dyn_index
    store_vector_element %p, %7, 1.0f
    store_vector_element %p, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %v, 1u
    %6:u32 = load %dyn_index
    %7:vec4<f32> = load %5
    %8:vec4<f32> = construct 1.0f
    %9:vec4<f32> = construct %6
    %10:vec4<f32> = construct 0i, 1i, 2i, 3i
    %11:vec4<bool> = eq %9, %10
    %12:vec4<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElement_PrivateVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<private_>(ty.mat2x4<f32>()));
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* access = b.Access(ty.ptr<private_, vec4<f32>>(), v, 1_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<private, mat2x4<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:ptr<private, vec4<f32>, read_write> = access %v, 1u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<private, mat2x4<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:ptr<private, vec4<f32>, read_write> = access %v, 1u
    %6:u32 = load %dyn_index
    %7:vec4<f32> = load %5
    %8:vec4<f32> = construct 1.0f
    %9:vec4<f32> = construct %6
    %10:vec4<f32> = construct 0i, 1i, 2i, 3i
    %11:vec4<bool> = eq %9, %10
    %12:vec4<f32> = select %7, %8, %11
    store %5, %12
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElement_StorageVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<storage>(ty.mat2x4<f32>()));
    v->SetBindingPoint(0, 1);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* access = b.Access(ty.ptr<storage, vec4<f32>>(), v, 1_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<storage, mat2x4<f32>, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:ptr<storage, vec4<f32>, read_write> = access %v, 1u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixElement_WorkgroupVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<workgroup>(ty.mat2x4<f32>()));
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* access = b.Access(ty.ptr<workgroup, vec4<f32>>(), v, 1_u);
        b.StoreVectorElement(access, b.Load(dyn_index), 1_f);
        b.StoreVectorElement(access, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<workgroup, mat2x4<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:ptr<workgroup, vec4<f32>, read_write> = access %v, 1u
    %6:u32 = load %dyn_index
    store_vector_element %5, %6, 1.0f
    store_vector_element %5, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumn) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.mat2x4<f32>()));
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<function, vec4<f32>>(), v, b.Load(dyn_index));
        b.Store(access0, vec);
        auto* access1 = b.Access(ty.ptr<function, vec4<f32>>(), v, static_index);
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, %6
    store %7, %5
    %8:ptr<function, vec4<f32>, read_write> = access %v, %static_index
    store %8, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, %6
    switch %6 [c: (0u, $B3), c: (1u, $B4), c: (default, $B5)] {  # switch_1
      $B3: {  # case
        %8:ptr<function, vec4<f32>, read_write> = access %v, 0u
        store %8, %5
        exit_switch  # switch_1
      }
      $B4: {  # case
        %9:ptr<function, vec4<f32>, read_write> = access %v, 1u
        store %9, %5
        exit_switch  # switch_1
      }
      $B5: {  # case
        exit_switch  # switch_1
      }
    }
    %10:ptr<function, vec4<f32>, read_write> = access %v, %static_index
    store %10, %5
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumnInStruct) {
    auto* mat_ty = ty.mat2x4<f32>();
    auto* struct_ty = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("v"), mat_ty}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(struct_ty));
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<function, vec4<f32>>(), v, 0_u, b.Load(dyn_index));
        b.Store(access0, vec);
        auto* access1 = b.Access(ty.ptr<function, vec4<f32>>(), v, 0_u, static_index);
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  v:mat2x4<f32> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, 0u, %6
    store %7, %5
    %8:ptr<function, vec4<f32>, read_write> = access %v, 0u, %static_index
    store %8, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  v:mat2x4<f32> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, 0u, %6
    %8:ptr<function, mat2x4<f32>, read_write> = access %v, 0u
    switch %6 [c: (0u, $B3), c: (1u, $B4), c: (default, $B5)] {  # switch_1
      $B3: {  # case
        %9:ptr<function, vec4<f32>, read_write> = access %8, 0u
        store %9, %5
        exit_switch  # switch_1
      }
      $B4: {  # case
        %10:ptr<function, vec4<f32>, read_write> = access %8, 1u
        store %10, %5
        exit_switch  # switch_1
      }
      $B5: {  # case
        exit_switch  # switch_1
      }
    }
    %11:ptr<function, vec4<f32>, read_write> = access %v, 0u, %static_index
    store %11, %5
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumnInArray) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.array(ty.mat2x4<f32>(), 8)));
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<function, vec4<f32>>(), v, 7_u, b.Load(dyn_index));
        b.Store(access0, vec);
        auto* access1 = b.Access(ty.ptr<function, vec4<f32>>(), v, 7_u, static_index);
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, array<mat2x4<f32>, 8>, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, 7u, %6
    store %7, %5
    %8:ptr<function, vec4<f32>, read_write> = access %v, 7u, %static_index
    store %8, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, array<mat2x4<f32>, 8>, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, 7u, %6
    %8:ptr<function, mat2x4<f32>, read_write> = access %v, 7u
    switch %6 [c: (0u, $B3), c: (1u, $B4), c: (default, $B5)] {  # switch_1
      $B3: {  # case
        %9:ptr<function, vec4<f32>, read_write> = access %8, 0u
        store %9, %5
        exit_switch  # switch_1
      }
      $B4: {  # case
        %10:ptr<function, vec4<f32>, read_write> = access %8, 1u
        store %10, %5
        exit_switch  # switch_1
      }
      $B5: {  # case
        exit_switch  # switch_1
      }
    }
    %11:ptr<function, vec4<f32>, read_write> = access %v, 7u, %static_index
    store %11, %5
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumnInArrayInStruct) {
    auto* mat_ty = ty.mat2x4<f32>();
    auto* struct_ty =
        ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("v"), ty.array(mat_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(struct_ty));
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<function, vec4<f32>>(), v, 0_u, 7_u, b.Load(dyn_index));
        b.Store(access0, vec);
        auto* access1 = b.Access(ty.ptr<function, vec4<f32>>(), v, 0_u, 7_u, static_index);
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(16) {
  v:array<mat2x4<f32>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, 0u, 7u, %6
    store %7, %5
    %8:ptr<function, vec4<f32>, read_write> = access %v, 0u, 7u, %static_index
    store %8, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(16) {
  v:array<mat2x4<f32>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, S, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, 0u, 7u, %6
    %8:ptr<function, mat2x4<f32>, read_write> = access %v, 0u, 7u
    switch %6 [c: (0u, $B3), c: (1u, $B4), c: (default, $B5)] {  # switch_1
      $B3: {  # case
        %9:ptr<function, vec4<f32>, read_write> = access %8, 0u
        store %9, %5
        exit_switch  # switch_1
      }
      $B4: {  # case
        %10:ptr<function, vec4<f32>, read_write> = access %8, 1u
        store %10, %5
        exit_switch  # switch_1
      }
      $B5: {  # case
        exit_switch  # switch_1
      }
    }
    %11:ptr<function, vec4<f32>, read_write> = access %v, 0u, 7u, %static_index
    store %11, %5
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumnByFunc) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* get_dynamic = b.Function("get_dynamic", ty.u32());
    b.Append(get_dynamic->Block(), [&] { b.Return(get_dynamic, b.Load(dyn_index)); });
    auto* get_static = b.Function("get_static", ty.u32());
    b.Append(get_static->Block(), [&] { b.Return(get_static, 0_u); });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function>(ty.mat2x4<f32>()));
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<function, vec4<f32>>(), v, b.Call(get_dynamic));
        b.Store(access0, vec);
        // Will be transformed because we assume functions return a dynamic value
        auto* access1 = b.Access(ty.ptr<function, vec4<f32>>(), v, b.Call(get_static));
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%get_dynamic = func():u32 {
  $B2: {
    %3:u32 = load %dyn_index
    ret %3
  }
}
%get_static = func():u32 {
  $B3: {
    ret 0u
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %7:vec4<f32> = construct 0.0f
    %8:u32 = call %get_dynamic
    %9:ptr<function, vec4<f32>, read_write> = access %v, %8
    store %9, %7
    %10:u32 = call %get_static
    %11:ptr<function, vec4<f32>, read_write> = access %v, %10
    store %11, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%get_dynamic = func():u32 {
  $B2: {
    %3:u32 = load %dyn_index
    ret %3
  }
}
%get_static = func():u32 {
  $B3: {
    ret 0u
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %7:vec4<f32> = construct 0.0f
    %8:u32 = call %get_dynamic
    %9:ptr<function, vec4<f32>, read_write> = access %v, %8
    switch %8 [c: (0u, $B5), c: (1u, $B6), c: (default, $B7)] {  # switch_1
      $B5: {  # case
        %10:ptr<function, vec4<f32>, read_write> = access %v, 0u
        store %10, %7
        exit_switch  # switch_1
      }
      $B6: {  # case
        %11:ptr<function, vec4<f32>, read_write> = access %v, 1u
        store %11, %7
        exit_switch  # switch_1
      }
      $B7: {  # case
        exit_switch  # switch_1
      }
    }
    %12:u32 = call %get_static
    %13:ptr<function, vec4<f32>, read_write> = access %v, %12
    switch %12 [c: (0u, $B8), c: (1u, $B9), c: (default, $B10)] {  # switch_2
      $B8: {  # case
        %14:ptr<function, vec4<f32>, read_write> = access %v, 0u
        store %14, %7
        exit_switch  # switch_2
      }
      $B9: {  # case
        %15:ptr<function, vec4<f32>, read_write> = access %v, 1u
        store %15, %7
        exit_switch  # switch_2
      }
      $B10: {  # case
        exit_switch  # switch_2
      }
    }
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumn_ViaPointer) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.mat2x4<f32>()));
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<function, vec4<f32>>(), v, b.Load(dyn_index));
        auto* p0 = b.Let("p0", access0);
        b.Store(p0, vec);
        auto* access1 = b.Access(ty.ptr<function, vec4<f32>>(), v, static_index);
        auto* p1 = b.Let("p1", access1);
        b.Store(p1, vec);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, %6
    %p0:ptr<function, vec4<f32>, read_write> = let %7
    store %p0, %5
    %9:ptr<function, vec4<f32>, read_write> = access %v, %static_index
    %p1:ptr<function, vec4<f32>, read_write> = let %9
    store %p1, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<function, vec4<f32>, read_write> = access %v, %6
    switch %6 [c: (0u, $B3), c: (1u, $B4), c: (default, $B5)] {  # switch_1
      $B3: {  # case
        %8:ptr<function, vec4<f32>, read_write> = access %v, 0u
        store %8, %5
        exit_switch  # switch_1
      }
      $B4: {  # case
        %9:ptr<function, vec4<f32>, read_write> = access %v, 1u
        store %9, %5
        exit_switch  # switch_1
      }
      $B5: {  # case
        exit_switch  # switch_1
      }
    }
    %10:ptr<function, vec4<f32>, read_write> = access %v, %static_index
    store %10, %5
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumn_PrivateVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<private_>(ty.mat2x4<f32>()));
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<private_, vec4<f32>>(), v, b.Load(dyn_index));
        b.Store(access0, vec);
        auto* access1 = b.Access(ty.ptr<private_, vec4<f32>>(), v, static_index);
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<private, mat2x4<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<private, vec4<f32>, read_write> = access %v, %6
    store %7, %5
    %8:ptr<private, vec4<f32>, read_write> = access %v, %static_index
    store %8, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<private, mat2x4<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<private, vec4<f32>, read_write> = access %v, %6
    switch %6 [c: (0u, $B3), c: (1u, $B4), c: (default, $B5)] {  # switch_1
      $B3: {  # case
        %8:ptr<private, vec4<f32>, read_write> = access %v, 0u
        store %8, %5
        exit_switch  # switch_1
      }
      $B4: {  # case
        %9:ptr<private, vec4<f32>, read_write> = access %v, 1u
        store %9, %5
        exit_switch  # switch_1
      }
      $B5: {  # case
        exit_switch  # switch_1
      }
    }
    %10:ptr<private, vec4<f32>, read_write> = access %v, %static_index
    store %10, %5
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumn_StorageVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<storage>(ty.mat2x4<f32>()));
    v->SetBindingPoint(0, 1);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<storage, vec4<f32>>(), v, b.Load(dyn_index));
        b.Store(access0, vec);
        auto* access1 = b.Access(ty.ptr<storage, vec4<f32>>(), v, static_index);
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<storage, mat2x4<f32>, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<storage, vec4<f32>, read_write> = access %v, %6
    store %7, %5
    %8:ptr<storage, vec4<f32>, read_write> = access %v, %static_index
    store %8, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumn_WorkgroupVar) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    auto* v = b.Var("v", ty.ptr<workgroup>(ty.mat2x4<f32>()));
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* vec = b.Construct(ty.vec4<f32>(), 0_f);
        auto* access0 = b.Access(ty.ptr<workgroup, vec4<f32>>(), v, b.Load(dyn_index));
        b.Store(access0, vec);
        auto* access1 = b.Access(ty.ptr<workgroup, vec4<f32>>(), v, static_index);
        b.Store(access1, vec);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
  %v:ptr<workgroup, mat2x4<f32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %5:vec4<f32> = construct 0.0f
    %6:u32 = load %dyn_index
    %7:ptr<workgroup, vec4<f32>, read_write> = access %v, %6
    store %7, %5
    %8:ptr<workgroup, vec4<f32>, read_write> = access %v, %static_index
    store %8, %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterReplaceNonIndexableMatVecStoresTest, MatrixColumnAndElement) {
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* static_index = b.Let("static_index", 0_u);
        auto* v = b.Var("v", ty.ptr<function>(ty.mat2x4<f32>()));
        auto* access0 = b.Access(ty.ptr<function, vec4<f32>>(), v, b.Load(dyn_index));
        b.StoreVectorElement(access0, b.Load(dyn_index), 1_f);
        auto* access1 = b.Access(ty.ptr<function, vec4<f32>>(), v, static_index);
        b.StoreVectorElement(access1, static_index, 1_f);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:u32 = load %dyn_index
    %6:ptr<function, vec4<f32>, read_write> = access %v, %5
    %7:u32 = load %dyn_index
    store_vector_element %6, %7, 1.0f
    %8:ptr<function, vec4<f32>, read_write> = access %v, %static_index
    store_vector_element %8, %static_index, 1.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %static_index:u32 = let 0u
    %v:ptr<function, mat2x4<f32>, read_write> = var undef
    %5:u32 = load %dyn_index
    %6:ptr<function, vec4<f32>, read_write> = access %v, %5
    %7:u32 = load %dyn_index
    switch %5 [c: (0u, $B3), c: (1u, $B4), c: (default, $B5)] {  # switch_1
      $B3: {  # case
        %8:ptr<function, vec4<f32>, read_write> = access %v, 0u
        %9:vec4<f32> = load %8
        %10:vec4<f32> = construct 1.0f
        %11:vec4<f32> = construct %7
        %12:vec4<f32> = construct 0i, 1i, 2i, 3i
        %13:vec4<bool> = eq %11, %12
        %14:vec4<f32> = select %9, %10, %13
        store %8, %14
        exit_switch  # switch_1
      }
      $B4: {  # case
        %15:ptr<function, vec4<f32>, read_write> = access %v, 1u
        %16:vec4<f32> = load %15
        %17:vec4<f32> = construct 1.0f
        %18:vec4<f32> = construct %7
        %19:vec4<f32> = construct 0i, 1i, 2i, 3i
        %20:vec4<bool> = eq %18, %19
        %21:vec4<f32> = select %16, %17, %20
        store %15, %21
        exit_switch  # switch_1
      }
      $B5: {  # case
        exit_switch  # switch_1
      }
    }
    %22:ptr<function, vec4<f32>, read_write> = access %v, %static_index
    store_vector_element %22, %static_index, 1.0f
    ret
  }
}
)";

    Run(ReplaceNonIndexableMatVecStores);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
