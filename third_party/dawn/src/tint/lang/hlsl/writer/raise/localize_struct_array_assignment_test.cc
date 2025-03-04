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

#include "src/tint/lang/hlsl/writer/raise/localize_struct_array_assignment.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterLocalizeStructArrayAssignmentTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructArray) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(inner_s_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s1 : OuterS;
        // s1.a1[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s1 = b.Var("s1", ty.ptr<function>(outer_s_ty));
        auto* access = b.Access(ty.ptr<function>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    %8:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %9:array<InnerS, 8> = load %8
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5
    store %11, %7
    %12:array<InnerS, 8> = load %tint_array_copy
    store %8, %12
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructArray_SplitAccess) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto array_ty = ty.array(inner_s_ty, 8);
    auto* outer_s_ty = ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), array_ty}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s1 : OuterS;
        // s1.a1[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s1 = b.Var("s1", ty.ptr<function>(outer_s_ty));
        auto* access1 = b.Access(ty.ptr<function>(array_ty), s1, 0_u);
        auto* access2 = b.Access(ty.ptr<function>(inner_s_ty), access1, b.Load(dyn_index));
        b.Store(access2, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %6:u32 = load %dyn_index
    %7:ptr<function, InnerS, read_write> = access %5, %6
    %8:InnerS = load %v
    store %7, %8
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %6:u32 = load %dyn_index
    %7:ptr<function, InnerS, read_write> = access %5, %6
    %8:InnerS = load %v
    %9:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %10:array<InnerS, 8> = load %9
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %10
    %12:ptr<function, InnerS, read_write> = access %tint_array_copy, %6
    store %12, %8
    %13:array<InnerS, 8> = load %tint_array_copy
    store %9, %13
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructStructArray) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* s1_ty =
        ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a"), ty.array(inner_s_ty, 8)}});
    auto* outer_s_ty = ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("s1"), s1_ty}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s1 : OuterS;
        // s1.s2.a[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s1 = b.Var("s1", ty.ptr<function>(outer_s_ty));
        auto* access = b.Access(ty.ptr<function>(inner_s_ty), s1, 0_u, 0_u, b.Load(dyn_index));
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  s1:S1 @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s1, 0u, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  s1:S1 @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s1, 0u, 0u, %5
    %7:InnerS = load %v
    %8:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u, 0u
    %9:array<InnerS, 8> = load %8
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5
    store %11, %7
    %12:array<InnerS, 8> = load %tint_array_copy
    store %8, %12
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructStructArray_SplitAccess) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* array_ty = ty.array(inner_s_ty, 8);
    auto* s1_ty = ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a"), array_ty}});
    auto* outer_s_ty = ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("s1"), s1_ty}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s1 : OuterS;
        // s1.s2.a[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s1 = b.Var("s1", ty.ptr<function>(outer_s_ty));
        // auto* access = b.Access(ty.ptr<function>(inner_s_ty), s1, 0_u, 0_u, b.Load(dyn_index));
        auto* access1 = b.Access(ty.ptr<function>(s1_ty), s1, 0_u);
        auto* access2 = b.Access(ty.ptr<function>(array_ty), access1, 0_u);
        auto* access3 = b.Access(ty.ptr<function>(inner_s_ty), access2, b.Load(dyn_index));

        b.Store(access3, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  s1:S1 @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:ptr<function, S1, read_write> = access %s1, 0u
    %6:ptr<function, array<InnerS, 8>, read_write> = access %5, 0u
    %7:u32 = load %dyn_index
    %8:ptr<function, InnerS, read_write> = access %6, %7
    %9:InnerS = load %v
    store %8, %9
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  s1:S1 @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:ptr<function, S1, read_write> = access %s1, 0u
    %6:ptr<function, array<InnerS, 8>, read_write> = access %5, 0u
    %7:u32 = load %dyn_index
    %8:ptr<function, InnerS, read_write> = access %6, %7
    %9:InnerS = load %v
    %10:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u, 0u
    %11:array<InnerS, 8> = load %10
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %11
    %13:ptr<function, InnerS, read_write> = access %tint_array_copy, %7
    store %13, %9
    %14:array<InnerS, 8> = load %tint_array_copy
    store %10, %14
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructArrayStructArray) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* s1_ty =
        ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a2"), ty.array(inner_s_ty, 8)}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(s1_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s : OuterS;
        // s.a1[dyn_index].a2[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s = b.Var("s", ty.ptr<function>(outer_s_ty));
        auto* i = b.Load(dyn_index);
        auto* access = b.Access(ty.ptr<function>(inner_s_ty), s, 0_u, i, 0_u, i);
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a2:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  a1:array<S1, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s, 0u, %5, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a2:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  a1:array<S1, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s, 0u, %5, 0u, %5
    %7:InnerS = load %v
    %8:ptr<function, array<S1, 8>, read_write> = access %s, 0u
    %9:array<S1, 8> = load %8
    %tint_array_copy:ptr<function, array<S1, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5, 0u, %5
    store %11, %7
    %12:array<S1, 8> = load %tint_array_copy
    store %8, %12
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructArrayStructArray_SplitAccess) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* array_inner_s_ty = ty.array(inner_s_ty, 8);
    auto* s1_ty = ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a2"), array_inner_s_ty}});
    auto* array_s1_ty = ty.array(s1_ty, 8);
    auto* outer_s_ty = ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), array_s1_ty}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s : OuterS;
        // s.a1[dyn_index].a2[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s = b.Var("s", ty.ptr<function>(outer_s_ty));
        auto* access1 = b.Access(ty.ptr<function>(array_s1_ty), s, 0_u);
        auto* access2 = b.Access(ty.ptr<function>(s1_ty), access1, b.Load(dyn_index));
        auto* access3 = b.Access(ty.ptr<function>(array_inner_s_ty), access2, 0_u);
        auto* access4 = b.Access(ty.ptr<function>(inner_s_ty), access3, b.Load(dyn_index));
        b.Store(access4, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a2:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  a1:array<S1, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:ptr<function, array<S1, 8>, read_write> = access %s, 0u
    %6:u32 = load %dyn_index
    %7:ptr<function, S1, read_write> = access %5, %6
    %8:ptr<function, array<InnerS, 8>, read_write> = access %7, 0u
    %9:u32 = load %dyn_index
    %10:ptr<function, InnerS, read_write> = access %8, %9
    %11:InnerS = load %v
    store %10, %11
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a2:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  a1:array<S1, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:ptr<function, array<S1, 8>, read_write> = access %s, 0u
    %6:u32 = load %dyn_index
    %7:ptr<function, S1, read_write> = access %5, %6
    %8:ptr<function, array<InnerS, 8>, read_write> = access %7, 0u
    %9:u32 = load %dyn_index
    %10:ptr<function, InnerS, read_write> = access %8, %9
    %11:InnerS = load %v
    %12:ptr<function, array<S1, 8>, read_write> = access %s, 0u
    %13:array<S1, 8> = load %12
    %tint_array_copy:ptr<function, array<S1, 8>, read_write> = var, %13
    %15:ptr<function, InnerS, read_write> = access %tint_array_copy, %6, 0u, %9
    store %15, %11
    %16:array<S1, 8> = load %tint_array_copy
    store %12, %16
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructArrayStructArray_DynIndexSecondAccess) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* s1_ty =
        ty.Struct(mod.symbols.New("S1"), {{mod.symbols.New("a2"), ty.array(inner_s_ty, 8)}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(s1_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s : OuterS;
        // s.a1[3].a2[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s = b.Var("s", ty.ptr<function>(outer_s_ty));
        auto* i = b.Load(dyn_index);
        auto* access = b.Access(ty.ptr<function>(inner_s_ty), s, 0_u, 3_u, 0_u, i);
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a2:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  a1:array<S1, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s, 0u, 3u, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

S1 = struct @align(4) {
  a2:array<InnerS, 8> @offset(0)
}

OuterS = struct @align(4) {
  a1:array<S1, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s, 0u, 3u, 0u, %5
    %7:InnerS = load %v
    %8:ptr<function, array<InnerS, 8>, read_write> = access %s, 0u, 3u, 0u
    %9:array<InnerS, 8> = load %8
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5
    store %11, %7
    %12:array<InnerS, 8> = load %tint_array_copy
    store %8, %12
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StructArrayArray_DynIndexSecondAccess) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty = ty.Struct(mod.symbols.New("OuterS"),
                                 {{mod.symbols.New("a"), ty.array(ty.array(inner_s_ty, 8), 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var s : OuterS;
        // s.a[3][dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s = b.Var("s", ty.ptr<function>(outer_s_ty));
        auto* i = b.Load(dyn_index);
        auto* access = b.Access(ty.ptr<function>(inner_s_ty), s, 0_u, 3_u, i);
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a:array<array<InnerS, 8>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s, 0u, 3u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a:array<array<InnerS, 8>, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s, 0u, 3u, %5
    %7:InnerS = load %v
    %8:ptr<function, array<InnerS, 8>, read_write> = access %s, 0u, 3u
    %9:array<InnerS, 8> = load %8
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5
    store %11, %7
    %12:array<InnerS, 8> = load %tint_array_copy
    store %8, %12
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, Multiple) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(inner_s_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s1 = b.Var("s1", ty.ptr<function>(outer_s_ty));
        // Multiple access and loads
        auto* access1 = b.Access(ty.ptr<function>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        b.Store(access1, b.Load(v));
        auto* access2 = b.Access(ty.ptr<function>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        b.Store(access2, b.Load(v));
        auto* access3 = b.Access(ty.ptr<function>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        auto* access4 = b.Access(ty.ptr<function>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        b.Store(access3, b.Load(v));
        b.Store(access4, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    %8:u32 = load %dyn_index
    %9:ptr<function, InnerS, read_write> = access %s1, 0u, %8
    %10:InnerS = load %v
    store %9, %10
    %11:u32 = load %dyn_index
    %12:ptr<function, InnerS, read_write> = access %s1, 0u, %11
    %13:u32 = load %dyn_index
    %14:ptr<function, InnerS, read_write> = access %s1, 0u, %13
    %15:InnerS = load %v
    store %12, %15
    %16:InnerS = load %v
    store %14, %16
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s1:ptr<function, OuterS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    %8:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %9:array<InnerS, 8> = load %8
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5
    store %11, %7
    %12:array<InnerS, 8> = load %tint_array_copy
    store %8, %12
    %13:u32 = load %dyn_index
    %14:ptr<function, InnerS, read_write> = access %s1, 0u, %13
    %15:InnerS = load %v
    %16:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %17:array<InnerS, 8> = load %16
    %tint_array_copy_1:ptr<function, array<InnerS, 8>, read_write> = var, %17  # %tint_array_copy_1: 'tint_array_copy'
    %19:ptr<function, InnerS, read_write> = access %tint_array_copy_1, %13
    store %19, %15
    %20:array<InnerS, 8> = load %tint_array_copy_1
    store %16, %20
    %21:u32 = load %dyn_index
    %22:ptr<function, InnerS, read_write> = access %s1, 0u, %21
    %23:u32 = load %dyn_index
    %24:ptr<function, InnerS, read_write> = access %s1, 0u, %23
    %25:InnerS = load %v
    %26:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %27:array<InnerS, 8> = load %26
    %tint_array_copy_2:ptr<function, array<InnerS, 8>, read_write> = var, %27  # %tint_array_copy_2: 'tint_array_copy'
    %29:ptr<function, InnerS, read_write> = access %tint_array_copy_2, %21
    store %29, %25
    %30:array<InnerS, 8> = load %tint_array_copy_2
    store %26, %30
    %31:InnerS = load %v
    %32:ptr<function, array<InnerS, 8>, read_write> = access %s1, 0u
    %33:array<InnerS, 8> = load %32
    %tint_array_copy_3:ptr<function, array<InnerS, 8>, read_write> = var, %33  # %tint_array_copy_3: 'tint_array_copy'
    %35:ptr<function, InnerS, read_write> = access %tint_array_copy_3, %23
    store %35, %31
    %36:array<InnerS, 8> = load %tint_array_copy_3
    store %32, %36
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

// Should also transform for private variables (above test function variables)
TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, PrivateVar) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(inner_s_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    // var<private> s1 : OuterS;
    auto* s1 = b.Var("s1", ty.ptr<private_>(outer_s_ty));
    mod.root_block->Append(s1);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // s1.a1[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* access = b.Access(ty.ptr<private_>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
  %s1:ptr<private, OuterS, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<private, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
  %s1:ptr<private, OuterS, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<private, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    %8:ptr<private, array<InnerS, 8>, read_write> = access %s1, 0u
    %9:array<InnerS, 8> = load %8
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5
    store %11, %7
    %12:array<InnerS, 8> = load %tint_array_copy
    store %8, %12
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

// Should also transform if store is via a pointer
TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, ViaPointer) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(inner_s_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    // var<private> s1 : OuterS;
    auto* s1 = b.Var("s1", ty.ptr<private_>(outer_s_ty));
    mod.root_block->Append(s1);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // let p = &(s1.a1[dyn_index]);
        // *p = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* access = b.Access(ty.ptr<private_>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        auto* p = b.Let("p", access);
        b.Store(p, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
  %s1:ptr<private, OuterS, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<private, InnerS, read_write> = access %s1, 0u, %5
    %p:ptr<private, InnerS, read_write> = let %6
    %8:InnerS = load %v
    store %p, %8
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
  %s1:ptr<private, OuterS, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<private, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    %8:ptr<private, array<InnerS, 8>, read_write> = access %s1, 0u
    %9:array<InnerS, 8> = load %8
    %tint_array_copy:ptr<function, array<InnerS, 8>, read_write> = var, %9
    %11:ptr<function, InnerS, read_write> = access %tint_array_copy, %5
    store %11, %7
    %12:array<InnerS, 8> = load %tint_array_copy
    store %8, %12
    ret
  }
}
)";

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

// No change for storage vars
TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, StorageVar) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(inner_s_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    // var<storage> s1 : OuterS;
    auto* s1 = b.Var("s1", ty.ptr<storage>(outer_s_ty));
    s1->SetBindingPoint(0, 1);
    mod.root_block->Append(s1);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // s1.a1[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* access = b.Access(ty.ptr<storage>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
  %s1:ptr<storage, OuterS, read_write> = var @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<storage, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

// No change for workgroup vars
TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, WorkgroupVar) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a1"), ty.array(inner_s_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);
    // var<workgroup> s1 : OuterS;
    auto* s1 = b.Var("s1", ty.ptr<workgroup>(outer_s_ty));
    mod.root_block->Append(s1);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // s1.a1[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* access = b.Access(ty.ptr<workgroup>(inner_s_ty), s1, 0_u, b.Load(dyn_index));
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a1:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
  %s1:ptr<workgroup, OuterS, read_write> = var
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<workgroup, InnerS, read_write> = access %s1, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

// FXC only fails if the top-level type is a struct
TEST_F(HlslWriterLocalizeStructArrayAssignmentTest, ArrayStructArray) {
    auto* inner_s_ty = ty.Struct(mod.symbols.New("InnerS"), {{mod.symbols.New("v"), ty.i32()}});
    auto* outer_s_ty =
        ty.Struct(mod.symbols.New("OuterS"), {{mod.symbols.New("a"), ty.array(inner_s_ty, 8)}});
    auto* dyn_index = b.Var("dyn_index", ty.ptr<uniform, u32>());
    dyn_index->SetBindingPoint(0, 0);
    mod.root_block->Append(dyn_index);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        // var v : InnerS;
        // var as : array<OuterS, 2>;
        // as[dyn_index].a[dyn_index] = v;
        auto* v = b.Var("v", ty.ptr<function>(inner_s_ty));
        auto* s = b.Var("s", ty.ptr<function>(ty.array(outer_s_ty, 2)));
        auto* i = b.Load(dyn_index);
        auto* access = b.Access(ty.ptr<function>(inner_s_ty), s, i, 0_u, i);
        b.Store(access, b.Load(v));
        b.Return(func);
    });

    auto* src = R"(
InnerS = struct @align(4) {
  v:i32 @offset(0)
}

OuterS = struct @align(4) {
  a:array<InnerS, 8> @offset(0)
}

$B1: {  # root
  %dyn_index:ptr<uniform, u32, read> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %v:ptr<function, InnerS, read_write> = var
    %s:ptr<function, array<OuterS, 2>, read_write> = var
    %5:u32 = load %dyn_index
    %6:ptr<function, InnerS, read_write> = access %s, %5, 0u, %5
    %7:InnerS = load %v
    store %6, %7
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(LocalizeStructArrayAssignment);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
