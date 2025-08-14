// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/lower/transpose_row_major.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class SpirvReader_TransposeRowMajorTest : public core::ir::transform::TransformTest {
    void SetUp() override { capabilities.Add(core::ir::Capability::kAllowNonCoreTypes); }
};

TEST_F(SpirvReader_TransposeRowMajorTest, ReadUniformMatrix) {
    // struct S {
    //   @offset(16) @row_major m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x3<f32> = s.m;
    // }

    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<uniform, mat2x3<f32>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat2x3<f32>, read> = access %s, 0u
    %4:mat2x3<f32> = load %3
    %x:mat2x3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<uniform, S_1, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %s, 0u
    %4:mat3x2<f32> = load %3
    %5:mat2x3<f32> = transpose %4
    %x:mat2x3<f32> = let %5
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ReadUniformColumn) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : vec3<f32> = s.m[1];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<uniform, vec3<f32>>>(var, 0_u, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %s, 0u, 1u
    %4:vec3<f32> = load %3
    %x:vec3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<uniform, S_1, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %s, 0u
    %4:vec3<f32> = call %tint_load_row_major_column, %3, 1u
    %x:vec3<f32> = let %4
    ret
  }
}
%tint_load_row_major_column = func(%7:ptr<uniform, mat3x2<f32>, read>, %8:u32):vec3<f32> {
  $B3: {
    %9:ptr<uniform, vec2<f32>, read> = access %7, 0u
    %10:f32 = load_vector_element %9, %8
    %11:ptr<uniform, vec2<f32>, read> = access %7, 1u
    %12:f32 = load_vector_element %11, %8
    %13:ptr<uniform, vec2<f32>, read> = access %7, 2u
    %14:f32 = load_vector_element %13, %8
    %15:vec3<f32> = construct %10, %12, %14
    ret %15
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ReadUniformElement) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   let x : f32 = s.m[col_idx].z;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* col = b.Access<ptr<uniform, vec3<f32>>>(var, 0_u, 1_u);
        b.Let("x", b.LoadVectorElement(col, 2_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %s, 0u, 1u
    %4:f32 = load_vector_element %3, 2u
    %x:f32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<uniform, S_1, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %s, 0u
    %4:ptr<uniform, vec2<f32>, read> = access %3, 2u
    %5:f32 = load_vector_element %4, 1u
    %x:f32 = let %5
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ReadUniformSwizzle) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   let x : vec2<f32> = s.m[1].zx;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access<ptr<uniform, vec3<f32>>>(var, 0_u, 1_u);
        auto* l = b.Load(a);
        b.Let("x", b.Swizzle(ty.vec2<f32>(), l, Vector{2u, 0u}));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %s, 0u, 1u
    %4:vec3<f32> = load %3
    %5:vec2<f32> = swizzle %4, zx
    %x:vec2<f32> = let %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<uniform, S_1, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %s, 0u
    %4:vec3<f32> = call %tint_load_row_major_column, %3, 1u
    %6:vec2<f32> = swizzle %4, zx
    %x:vec2<f32> = let %6
    ret
  }
}
%tint_load_row_major_column = func(%8:ptr<uniform, mat3x2<f32>, read>, %9:u32):vec3<f32> {
  $B3: {
    %10:ptr<uniform, vec2<f32>, read> = access %8, 0u
    %11:f32 = load_vector_element %10, %9
    %12:ptr<uniform, vec2<f32>, read> = access %8, 1u
    %13:f32 = load_vector_element %12, %9
    %14:ptr<uniform, vec2<f32>, read> = access %8, 2u
    %15:f32 = load_vector_element %14, %9
    %16:vec3<f32> = construct %11, %13, %15
    ret %16
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, WriteStorageMatrix) {
    // struct S {
    //   @offset(8)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.m = mat2x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 8u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* m = b.Construct(ty.mat2x3<f32>(), 1_f, 2_f, 3_f, 4_f, 5_f, 6_f);
        auto* a = b.Access<ptr<storage, mat2x3<f32>, read_write>>(var, 0_u);
        b.Store(a, m);
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:mat2x3<f32> = construct 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f
    %4:ptr<storage, mat2x3<f32>, read_write> = access %s, 0u
    store %4, %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(8)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:mat2x3<f32> = construct 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f
    %4:ptr<storage, mat3x2<f32>, read_write> = access %s, 0u
    %5:mat3x2<f32> = transpose %3
    store %4, %5
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, WriteStorageColumn) {
    // struct S {
    //   @offset(8)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   s.m[1] = vec3<f32>(1.0, 2.0, 3.0);
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 8u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access<ptr<storage, vec3<f32>, read_write>>(var, 0_u, 1_u);
        auto* c = b.Construct(ty.vec3<f32>(), 1_f, 2_f, 3_f);
        b.Store(a, c);
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    store %3, %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(8)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat3x2<f32>, read_write> = access %s, 0u
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    %5:void = call %tint_store_row_major_column, %3, 1u, %4
    ret
  }
}
%tint_store_row_major_column = func(%7:ptr<storage, mat3x2<f32>, read_write>, %8:u32, %9:vec3<f32>):void {
  $B3: {
    %10:f32 = access %9, 0u
    %11:ptr<storage, vec2<f32>, read_write> = access %7, 0u
    store_vector_element %11, %8, %10
    %12:f32 = access %9, 1u
    %13:ptr<storage, vec2<f32>, read_write> = access %7, 1u
    store_vector_element %13, %8, %12
    %14:f32 = access %9, 2u
    %15:ptr<storage, vec2<f32>, read_write> = access %7, 2u
    store_vector_element %15, %8, %14
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, WriteStorageElement_Accessor) {
    // struct S {
    //   @offset(8)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   s.m[1].z = 1.0;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 8u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access<ptr<storage, vec3<f32>>>(var, 0_u, 1_u);
        b.StoreVectorElement(a, 2_u, 1_f);
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u
    store_vector_element %3, 2u, 1.0f
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(8)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat3x2<f32>, read_write> = access %s, 0u
    %4:ptr<storage, vec2<f32>, read_write> = access %3, 2u
    store_vector_element %4, 1u, 1.0f
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ExtractFromLoadedStruct) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   let row_idx : i32 = 2i;
    //   let load = s;
    //   let m : mat2x3<f32> = load.m;
    //   let c : vec3<f32> = load.m[col_idx];
    //   let e : vec3<f32> = load.m[col_idx][row_idx];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* ls = b.Load(var);
        auto* load = b.Let("load", ls);
        b.Let("m", b.Access(ty.mat2x3<f32>(), load, 0_u));
        b.Let("c", b.Access(ty.vec3<f32>(), load, 0_u, 1_u));
        b.Let("e", b.Access(ty.f32(), load, 0_u, 1_u, 2_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S = load %s
    %load:S = let %3
    %5:mat2x3<f32> = access %load, 0u
    %m:mat2x3<f32> = let %5
    %7:vec3<f32> = access %load, 0u, 1u
    %c:vec3<f32> = let %7
    %9:f32 = access %load, 0u, 1u, 2u
    %e:f32 = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<uniform, S_1, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S_1 = load %s
    %load:S_1 = let %3
    %5:mat3x2<f32> = access %load, 0u
    %6:mat2x3<f32> = transpose %5
    %m:mat2x3<f32> = let %6
    %8:mat3x2<f32> = access %load, 0u
    %9:mat2x3<f32> = transpose %8
    %10:vec3<f32> = access %9, 1u
    %c:vec3<f32> = let %10
    %12:mat3x2<f32> = access %load, 0u
    %13:mat2x3<f32> = transpose %12
    %14:f32 = access %13, 1u, 2u
    %e:f32 = let %14
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ExtractFromLoadedVector) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   let row_idx : i32 = 2i;
    //   let load = s;
    //   let m : mat2x3<f32> = load.m;
    //   let c : vec3<f32> = load.m[col_idx];
    //   let e : vec3<f32> = c[row_idx];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* ls = b.Load(var);
        auto* load = b.Let("load", ls);
        b.Let("m", b.Access(ty.mat2x3<f32>(), load, 0_u));
        auto* c = b.Let("c", b.Access(ty.vec3<f32>(), load, 0_u, 1_u));
        b.Let("e", b.Access(ty.f32(), c, 2_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S = load %s
    %load:S = let %3
    %5:mat2x3<f32> = access %load, 0u
    %m:mat2x3<f32> = let %5
    %7:vec3<f32> = access %load, 0u, 1u
    %c:vec3<f32> = let %7
    %9:f32 = access %c, 2u
    %e:f32 = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<uniform, S_1, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S_1 = load %s
    %load:S_1 = let %3
    %5:mat3x2<f32> = access %load, 0u
    %6:mat2x3<f32> = transpose %5
    %m:mat2x3<f32> = let %6
    %8:mat3x2<f32> = access %load, 0u
    %9:mat2x3<f32> = transpose %8
    %10:vec3<f32> = access %9, 1u
    %c:vec3<f32> = let %10
    %12:f32 = access %c, 2u
    %e:f32 = let %12
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, InsertInStructConstructor) {
    // struct S {
    //   @offset(0) @row_major m1 : mat2x3<f32>,
    //   @offset(32) m2 : mat4x2<f32>,
    //   @offset(64) @row_major m3 : mat4x2<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let m1 = mat2x3<f32>();
    //   let m2 = mat4x2<f32>();
    //   s = S(m, m2, m2);
    // }
    auto* matrix_member_0 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat2x3<f32>(), 0u, 0u, 24u, 24u, core::IOAttributes{});
    matrix_member_0->SetRowMajor();

    auto* matrix_member_1 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat4x2<f32>(), 1u, 32u, 32u, 32u, core::IOAttributes{});

    auto* matrix_member_2 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat4x2<f32>(), 2u, 64u, 32u, 32u, core::IOAttributes{});
    matrix_member_2->SetRowMajor();

    auto* strct =
        ty.Struct(mod.symbols.New("S"), Vector{matrix_member_0, matrix_member_1, matrix_member_2});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* m1 = b.Construct(ty.mat2x3<f32>());
        auto* m2 = b.Construct(ty.mat4x2<f32>());

        b.Store(var, b.Construct(strct, m1, m2, m2));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major
  m_1:mat4x2<f32> @offset(32)
  m_2:mat4x2<f32> @offset(64), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:mat2x3<f32> = construct
    %4:mat4x2<f32> = construct
    %5:S = construct %3, %4, %4
    store %s, %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(32) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major
  m_1:mat4x2<f32> @offset(32)
  m_2:mat4x2<f32> @offset(64), @row_major
}

S_1 = struct @align(32) {
  m:mat3x2<f32> @offset(0)
  m_1:mat4x2<f32> @offset(32)
  m_2:mat2x4<f32> @offset(64)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:mat2x3<f32> = construct
    %4:mat4x2<f32> = construct
    %5:mat3x2<f32> = transpose %3
    %6:mat2x4<f32> = transpose %4
    %7:S_1 = construct %5, %4, %6
    store %s, %7
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DeeplyNested) {
    // struct Inner {
    //   @offset(0)
    //   @row_major
    //   m : mat4x3<f32>,
    // };
    // struct Outer {
    //   @offset(0)
    //   arr : array<Inner, 4>,
    // };
    // @group(0) @binding(0) var<storage, read_write> buffer : Outer;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let m = buffer.arr[1].m;
    //   buffer.arr[0].m[3] = m[2];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat4x3<f32>(),
                                                           0u, 16u, 24u, 48u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* inner_strct = ty.Struct(mod.symbols.New("Inner"), Vector{matrix_member});

    auto* outer_strct =
        ty.Struct(mod.symbols.New("Outer"), {{mod.symbols.New("arr"), ty.array(inner_strct, 4_u)}});

    auto* var = b.Var("s", ty.ptr(storage, outer_strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* m = b.Let("m", b.Load(b.Access<ptr<storage, mat4x3<f32>>>(var, 0_u, 1_u, 0_u)));
        auto* ptr = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 0_u, 0_u, 3_u);
        b.Store(ptr, b.Access(ty.vec3<f32>(), m, 2_u));
        b.Return(f);
    });

    auto* before = R"(
Inner = struct @align(24) {
  m:mat4x3<f32> @offset(16) @size(48), @row_major
}

Outer = struct @align(24) {
  arr:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat4x3<f32>, read_write> = access %s, 0u, 1u, 0u
    %4:mat4x3<f32> = load %3
    %m:mat4x3<f32> = let %4
    %6:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 0u, 0u, 3u
    %7:vec3<f32> = access %m, 2u
    store %6, %7
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
Inner = struct @align(24) {
  m:mat4x3<f32> @offset(16) @size(48), @row_major
}

Outer = struct @align(24) {
  arr:array<Inner, 4> @offset(0)
}

Inner_1 = struct @align(24) {
  m:mat3x4<f32> @offset(16)
}

Outer_1 = struct @align(24) {
  arr:array<Inner_1, 4> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, Outer_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 1u, 0u
    %4:mat3x4<f32> = load %3
    %5:mat4x3<f32> = transpose %4
    %m:mat4x3<f32> = let %5
    %7:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 0u, 0u
    %8:vec3<f32> = access %m, 2u
    %9:void = call %tint_store_row_major_column, %7, 3u, %8
    ret
  }
}
%tint_store_row_major_column = func(%11:ptr<storage, mat3x4<f32>, read_write>, %12:u32, %13:vec3<f32>):void {
  $B3: {
    %14:f32 = access %13, 0u
    %15:ptr<storage, vec4<f32>, read_write> = access %11, 0u
    store_vector_element %15, %12, %14
    %16:f32 = access %13, 1u
    %17:ptr<storage, vec4<f32>, read_write> = access %11, 1u
    store_vector_element %17, %12, %16
    %18:f32 = access %13, 2u
    %19:ptr<storage, vec4<f32>, read_write> = access %11, 2u
    store_vector_element %19, %12, %18
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, MultipleColumnHelpers) {
    // struct S {
    //   @offset(0) @row_major m1 : mat2x3<f32>,
    //   @offset(32) @row_major m2 : mat4x2<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // var<private> ps : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   ps.m1[0] = s.m1[1];
    //   ps.m1[1] = s.m1[0];
    //   ps.m2[2] = s.m2[3];
    //   ps.m2[3] = s.m2[2];
    //
    //   s.m1[0] = ps.m1[0];
    //   s.m1[1] = ps.m1[1];
    //   s.m2[2] = ps.m2[2];
    //   s.m2[3] = ps.m2[3];
    // }
    auto* matrix_member_0 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat2x3<f32>(), 0u, 0u, 24u, 24u, core::IOAttributes{});
    matrix_member_0->SetRowMajor();

    auto* matrix_member_1 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat4x2<f32>(), 1u, 32u, 24u, 48u, core::IOAttributes{});
    matrix_member_1->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member_0, matrix_member_1});

    auto* s = b.Var("s", ty.ptr(storage, strct, read_write));
    s->SetBindingPoint(0, 0);
    mod.root_block->Append(s);

    auto* ps = b.Var("ps", ty.ptr(private_, strct));
    mod.root_block->Append(ps);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* sm10 = b.Access(ty.ptr<storage, vec3<f32>, read_write>(), s, 0_u, 0_u);
        auto* sm11 = b.Access(ty.ptr<storage, vec3<f32>, read_write>(), s, 0_u, 1_u);
        auto* sm22 = b.Access(ty.ptr<storage, vec2<f32>, read_write>(), s, 1_u, 2_u);
        auto* sm23 = b.Access(ty.ptr<storage, vec2<f32>, read_write>(), s, 1_u, 3_u);

        auto* psm10 = b.Access(ty.ptr<private_, vec3<f32>, read_write>(), ps, 0_u, 0_u);
        auto* psm11 = b.Access(ty.ptr<private_, vec3<f32>, read_write>(), ps, 0_u, 1_u);
        auto* psm22 = b.Access(ty.ptr<private_, vec2<f32>, read_write>(), ps, 1_u, 2_u);
        auto* psm23 = b.Access(ty.ptr<private_, vec2<f32>, read_write>(), ps, 1_u, 3_u);

        b.Store(psm10, b.Load(sm11));
        b.Store(psm11, b.Load(sm10));
        b.Store(psm22, b.Load(sm23));
        b.Store(psm23, b.Load(sm22));

        b.Store(sm10, b.Load(psm10));
        b.Store(sm11, b.Load(psm11));
        b.Store(sm22, b.Load(psm22));
        b.Store(sm23, b.Load(psm23));

        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major
  m_1:mat4x2<f32> @offset(32) @size(48), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %ps:ptr<private, S, read_write> = var undef
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 0u
    %5:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u
    %6:ptr<storage, vec2<f32>, read_write> = access %s, 1u, 2u
    %7:ptr<storage, vec2<f32>, read_write> = access %s, 1u, 3u
    %8:ptr<private, vec3<f32>, read_write> = access %ps, 0u, 0u
    %9:ptr<private, vec3<f32>, read_write> = access %ps, 0u, 1u
    %10:ptr<private, vec2<f32>, read_write> = access %ps, 1u, 2u
    %11:ptr<private, vec2<f32>, read_write> = access %ps, 1u, 3u
    %12:vec3<f32> = load %5
    store %8, %12
    %13:vec3<f32> = load %4
    store %9, %13
    %14:vec2<f32> = load %7
    store %10, %14
    %15:vec2<f32> = load %6
    store %11, %15
    %16:vec3<f32> = load %8
    store %4, %16
    %17:vec3<f32> = load %9
    store %5, %17
    %18:vec2<f32> = load %10
    store %6, %18
    %19:vec2<f32> = load %11
    store %7, %19
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major
  m_1:mat4x2<f32> @offset(32) @size(48), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(0)
  m_1:mat2x4<f32> @offset(32) @size(48)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
  %ps:ptr<private, S_1, read_write> = var undef
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<storage, mat3x2<f32>, read_write> = access %s, 0u
    %5:ptr<storage, mat3x2<f32>, read_write> = access %s, 0u
    %6:ptr<storage, mat2x4<f32>, read_write> = access %s, 1u
    %7:ptr<storage, mat2x4<f32>, read_write> = access %s, 1u
    %8:ptr<private, mat3x2<f32>, read_write> = access %ps, 0u
    %9:ptr<private, mat3x2<f32>, read_write> = access %ps, 0u
    %10:ptr<private, mat2x4<f32>, read_write> = access %ps, 1u
    %11:ptr<private, mat2x4<f32>, read_write> = access %ps, 1u
    %12:vec3<f32> = call %tint_load_row_major_column, %5, 1u
    %14:void = call %tint_store_row_major_column, %8, 0u, %12
    %16:vec3<f32> = call %tint_load_row_major_column, %4, 0u
    %17:void = call %tint_store_row_major_column, %9, 1u, %16
    %18:vec2<f32> = call %tint_load_row_major_column_1, %7, 3u
    %20:void = call %tint_store_row_major_column_1, %10, 2u, %18
    %22:vec2<f32> = call %tint_load_row_major_column_1, %6, 2u
    %23:void = call %tint_store_row_major_column_1, %11, 3u, %22
    %24:vec3<f32> = call %tint_load_row_major_column_2, %8, 0u
    %26:void = call %tint_store_row_major_column_2, %4, 0u, %24
    %28:vec3<f32> = call %tint_load_row_major_column_2, %9, 1u
    %29:void = call %tint_store_row_major_column_2, %5, 1u, %28
    %30:vec2<f32> = call %tint_load_row_major_column_3, %10, 2u
    %32:void = call %tint_store_row_major_column_3, %6, 2u, %30
    %34:vec2<f32> = call %tint_load_row_major_column_3, %11, 3u
    %35:void = call %tint_store_row_major_column_3, %7, 3u, %34
    ret
  }
}
%tint_store_row_major_column_1 = func(%36:ptr<private, mat2x4<f32>, read_write>, %37:u32, %38:vec2<f32>):void {  # %tint_store_row_major_column_1: 'tint_store_row_major_column'
  $B3: {
    %39:f32 = access %38, 0u
    %40:ptr<private, vec4<f32>, read_write> = access %36, 0u
    store_vector_element %40, %37, %39
    %41:f32 = access %38, 1u
    %42:ptr<private, vec4<f32>, read_write> = access %36, 1u
    store_vector_element %42, %37, %41
    ret
  }
}
%tint_load_row_major_column_3 = func(%43:ptr<private, mat2x4<f32>, read_write>, %44:u32):vec2<f32> {  # %tint_load_row_major_column_3: 'tint_load_row_major_column'
  $B4: {
    %45:ptr<private, vec4<f32>, read_write> = access %43, 0u
    %46:f32 = load_vector_element %45, %44
    %47:ptr<private, vec4<f32>, read_write> = access %43, 1u
    %48:f32 = load_vector_element %47, %44
    %49:vec2<f32> = construct %46, %48
    ret %49
  }
}
%tint_store_row_major_column = func(%50:ptr<private, mat3x2<f32>, read_write>, %51:u32, %52:vec3<f32>):void {
  $B5: {
    %53:f32 = access %52, 0u
    %54:ptr<private, vec2<f32>, read_write> = access %50, 0u
    store_vector_element %54, %51, %53
    %55:f32 = access %52, 1u
    %56:ptr<private, vec2<f32>, read_write> = access %50, 1u
    store_vector_element %56, %51, %55
    %57:f32 = access %52, 2u
    %58:ptr<private, vec2<f32>, read_write> = access %50, 2u
    store_vector_element %58, %51, %57
    ret
  }
}
%tint_load_row_major_column_2 = func(%59:ptr<private, mat3x2<f32>, read_write>, %60:u32):vec3<f32> {  # %tint_load_row_major_column_2: 'tint_load_row_major_column'
  $B6: {
    %61:ptr<private, vec2<f32>, read_write> = access %59, 0u
    %62:f32 = load_vector_element %61, %60
    %63:ptr<private, vec2<f32>, read_write> = access %59, 1u
    %64:f32 = load_vector_element %63, %60
    %65:ptr<private, vec2<f32>, read_write> = access %59, 2u
    %66:f32 = load_vector_element %65, %60
    %67:vec3<f32> = construct %62, %64, %66
    ret %67
  }
}
%tint_load_row_major_column_1 = func(%68:ptr<storage, mat2x4<f32>, read_write>, %69:u32):vec2<f32> {  # %tint_load_row_major_column_1: 'tint_load_row_major_column'
  $B7: {
    %70:ptr<storage, vec4<f32>, read_write> = access %68, 0u
    %71:f32 = load_vector_element %70, %69
    %72:ptr<storage, vec4<f32>, read_write> = access %68, 1u
    %73:f32 = load_vector_element %72, %69
    %74:vec2<f32> = construct %71, %73
    ret %74
  }
}
%tint_store_row_major_column_3 = func(%75:ptr<storage, mat2x4<f32>, read_write>, %76:u32, %77:vec2<f32>):void {  # %tint_store_row_major_column_3: 'tint_store_row_major_column'
  $B8: {
    %78:f32 = access %77, 0u
    %79:ptr<storage, vec4<f32>, read_write> = access %75, 0u
    store_vector_element %79, %76, %78
    %80:f32 = access %77, 1u
    %81:ptr<storage, vec4<f32>, read_write> = access %75, 1u
    store_vector_element %81, %76, %80
    ret
  }
}
%tint_load_row_major_column = func(%82:ptr<storage, mat3x2<f32>, read_write>, %83:u32):vec3<f32> {
  $B9: {
    %84:ptr<storage, vec2<f32>, read_write> = access %82, 0u
    %85:f32 = load_vector_element %84, %83
    %86:ptr<storage, vec2<f32>, read_write> = access %82, 1u
    %87:f32 = load_vector_element %86, %83
    %88:ptr<storage, vec2<f32>, read_write> = access %82, 2u
    %89:f32 = load_vector_element %88, %83
    %90:vec3<f32> = construct %85, %87, %89
    ret %90
  }
}
%tint_store_row_major_column_2 = func(%91:ptr<storage, mat3x2<f32>, read_write>, %92:u32, %93:vec3<f32>):void {  # %tint_store_row_major_column_2: 'tint_store_row_major_column'
  $B10: {
    %94:f32 = access %93, 0u
    %95:ptr<storage, vec2<f32>, read_write> = access %91, 0u
    store_vector_element %95, %92, %94
    %96:f32 = access %93, 1u
    %97:ptr<storage, vec2<f32>, read_write> = access %91, 1u
    store_vector_element %97, %92, %96
    %98:f32 = access %93, 2u
    %99:ptr<storage, vec2<f32>, read_write> = access %91, 2u
    store_vector_element %99, %92, %98
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, PreserveMatrixStride) {
    capabilities.Add(core::ir::Capability::kAllowStructMatrixDecorations);

    // struct S {
    //   @offset(0)
    //   @stride(32)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x3<f32> = s.m;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 0u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();
    matrix_member->SetMatrixStride(32);

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<uniform, mat2x3<f32>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major, @matrix_stride(32)
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat2x3<f32>, read> = access %s, 0u
    %4:mat2x3<f32> = load %3
    %x:mat2x3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major, @matrix_stride(32)
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(0), @matrix_stride(32)
}

$B1: {  # root
  %s:ptr<uniform, S_1, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %s, 0u
    %4:mat3x2<f32> = load %3
    %5:mat2x3<f32> = transpose %4
    %x:mat2x3<f32> = let %5
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ArrayOfMatrix_ReadWholeArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(32) array<mat2x3<f32>, 4>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : array<mat2x3<f32>, 4> = s.arr;
    // }
    auto* matrix_member =
        ty.Get<core::type::StructMember>(mod.symbols.New("arr"), ty.array(ty.mat2x3<f32>(), 4), 0u,
                                         0u, 32u, 128u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<storage, array<mat2x3<f32>, 4>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  arr:array<mat2x3<f32>, 4> @offset(0), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<mat2x3<f32>, 4>, read_write> = access %s, 0u
    %4:array<mat2x3<f32>, 4> = load %3
    %x:array<mat2x3<f32>, 4> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(32) {
  arr:array<mat2x3<f32>, 4> @offset(0), @row_major
}

S_1 = struct @align(32) {
  arr:array<mat3x2<f32>, 4> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<mat3x2<f32>, 4>, read_write> = access %s, 0u
    %4:array<mat3x2<f32>, 4> = load %3
    %5:array<mat2x3<f32>, 4> = call %tint_transpose_row_major_array, %4
    %x:array<mat2x3<f32>, 4> = let %5
    ret
  }
}
%tint_transpose_row_major_array = func(%8:array<mat3x2<f32>, 4>):array<mat2x3<f32>, 4> {
  $B3: {
    %9:ptr<function, array<mat2x3<f32>, 4>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %11:bool = gte %idx, 4u
        if %11 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %12:mat3x2<f32> = access %8, %idx
        %13:mat2x3<f32> = transpose %12
        %14:ptr<function, mat2x3<f32>, read_write> = access %9, %idx
        store %14, %13
        continue  # -> $B6
      }
      $B6: {  # continuing
        %15:u32 = add %idx, 1u
        next_iteration %15  # -> $B5
      }
    }
    %16:array<mat2x3<f32>, 4> = load %9
    ret %16
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ArrayOfMatrix_WriteWholeArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(32) array<mat2x3<f32>, 4>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.arr = array<mat2x3<f32>, 4>();
    // }
    auto* matrix_member =
        ty.Get<core::type::StructMember>(mod.symbols.New("arr"), ty.array(ty.mat2x3<f32>(), 4), 0u,
                                         0u, 32u, 128u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access(ty.ptr(storage, ty.array(ty.mat2x3<f32>(), 4)), var, 0_u);
        b.Store(a, b.Construct(ty.array<mat2x3<f32>, 4>()));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  arr:array<mat2x3<f32>, 4> @offset(0), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<mat2x3<f32>, 4>, read_write> = access %s, 0u
    %4:array<mat2x3<f32>, 4> = construct
    store %3, %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(32) {
  arr:array<mat2x3<f32>, 4> @offset(0), @row_major
}

S_1 = struct @align(32) {
  arr:array<mat3x2<f32>, 4> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<mat3x2<f32>, 4>, read_write> = access %s, 0u
    %4:array<mat2x3<f32>, 4> = construct
    %5:array<mat3x2<f32>, 4> = call %tint_transpose_row_major_array, %4
    store %3, %5
    ret
  }
}
%tint_transpose_row_major_array = func(%7:array<mat2x3<f32>, 4>):array<mat3x2<f32>, 4> {
  $B3: {
    %8:ptr<function, array<mat3x2<f32>, 4>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %10:bool = gte %idx, 4u
        if %10 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %11:mat2x3<f32> = access %7, %idx
        %12:mat3x2<f32> = transpose %11
        %13:ptr<function, mat3x2<f32>, read_write> = access %8, %idx
        store %13, %12
        continue  # -> $B6
      }
      $B6: {  # continuing
        %14:u32 = add %idx, 1u
        next_iteration %14  # -> $B5
      }
    }
    %15:array<mat3x2<f32>, 4> = load %8
    ret %15
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ArrayOfMatrix_NestedArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(128) array<@stride(32) array<mat2x3<f32>, 4>, 5>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x = s.arr;
    //   s.arr = array<array<mat2x3<f32>, 4, 5>>();
    //   s.arr[0] = x[1];
    //   s.arr[1][2] = x[2][3];
    //   s.arr[2][3][1] = x[4][3][1];
    //   s.arr[4][2][0][1] = x[1][3][0][2];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.array(ty.array(ty.mat2x3<f32>(), 4), 5), 0u, 0u, 32u, 640u,
        core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* sarr = b.Access<ptr<storage, array<array<mat2x3<f32>, 4>, 5>, read_write>>(var, 0_u);
        auto* x = b.Let("x", b.Load(sarr));

        b.Store(sarr, b.Construct(ty.array<array<mat2x3<f32>, 4>, 5>()));

        auto* sarr0 = b.Access<ptr<storage, array<mat2x3<f32>, 4>, read_write>>(sarr, 0_u);
        b.Store(sarr0, b.Access(ty.array<mat2x3<f32>, 4>(), x, 1_u));

        auto* sarr12 = b.Access<ptr<storage, mat2x3<f32>, read_write>>(sarr, 1_u, 2_u);
        b.Store(sarr12, b.Access(ty.mat2x3<f32>(), x, 2_u, 3_u));

        auto* sarr231 = b.Access<ptr<storage, vec3<f32>, read_write>>(sarr, 2_u, 3_u, 1_u);
        b.Store(sarr231, b.Access(ty.vec3<f32>(), x, 4_u, 3_u, 1_u));

        auto* sarr420 = b.Access<ptr<storage, vec3<f32>, read_write>>(sarr, 4_u, 2_u, 0_u);
        b.StoreVectorElement(sarr420, 1_u, b.Access(ty.f32(), x, 1_u, 3_u, 0_u, 2_u));

        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  m:array<array<mat2x3<f32>, 4>, 5> @offset(0), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<array<mat2x3<f32>, 4>, 5>, read_write> = access %s, 0u
    %4:array<array<mat2x3<f32>, 4>, 5> = load %3
    %x:array<array<mat2x3<f32>, 4>, 5> = let %4
    %6:array<array<mat2x3<f32>, 4>, 5> = construct
    store %3, %6
    %7:ptr<storage, array<mat2x3<f32>, 4>, read_write> = access %3, 0u
    %8:array<mat2x3<f32>, 4> = access %x, 1u
    store %7, %8
    %9:ptr<storage, mat2x3<f32>, read_write> = access %3, 1u, 2u
    %10:mat2x3<f32> = access %x, 2u, 3u
    store %9, %10
    %11:ptr<storage, vec3<f32>, read_write> = access %3, 2u, 3u, 1u
    %12:vec3<f32> = access %x, 4u, 3u, 1u
    store %11, %12
    %13:ptr<storage, vec3<f32>, read_write> = access %3, 4u, 2u, 0u
    %14:f32 = access %x, 1u, 3u, 0u, 2u
    store_vector_element %13, 1u, %14
    ret
  }
}
)";

    EXPECT_EQ(before, str());

    auto* after = R"(
S = struct @align(32) {
  m:array<array<mat2x3<f32>, 4>, 5> @offset(0), @row_major
}

S_1 = struct @align(32) {
  m:array<array<mat3x2<f32>, 4>, 5> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<array<mat3x2<f32>, 4>, 5>, read_write> = access %s, 0u
    %4:array<array<mat3x2<f32>, 4>, 5> = load %3
    %5:array<array<mat2x3<f32>, 4>, 5> = call %tint_transpose_row_major_array, %4
    %x:array<array<mat2x3<f32>, 4>, 5> = let %5
    %8:array<array<mat2x3<f32>, 4>, 5> = construct
    %9:array<array<mat3x2<f32>, 4>, 5> = call %tint_transpose_row_major_array_1, %8
    store %3, %9
    %11:ptr<storage, array<mat3x2<f32>, 4>, read_write> = access %3, 0u
    %12:array<mat2x3<f32>, 4> = access %x, 1u
    %13:array<mat3x2<f32>, 4> = call %tint_transpose_row_major_array_2, %12
    store %11, %13
    %15:ptr<storage, mat3x2<f32>, read_write> = access %3, 1u, 2u
    %16:mat2x3<f32> = access %x, 2u, 3u
    %17:mat3x2<f32> = transpose %16
    store %15, %17
    %18:ptr<storage, mat3x2<f32>, read_write> = access %3, 2u, 3u
    %19:vec3<f32> = access %x, 4u, 3u, 1u
    %20:void = call %tint_store_row_major_column, %18, 1u, %19
    %22:ptr<storage, mat3x2<f32>, read_write> = access %3, 4u, 2u
    %23:ptr<storage, vec2<f32>, read_write> = access %22, 1u
    %24:f32 = access %x, 1u, 3u, 0u, 2u
    store_vector_element %23, 0u, %24
    ret
  }
}
%tint_transpose_row_major_array = func(%25:array<array<mat3x2<f32>, 4>, 5>):array<array<mat2x3<f32>, 4>, 5> {
  $B3: {
    %26:ptr<function, array<array<mat2x3<f32>, 4>, 5>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %28:bool = gte %idx, 5u
        if %28 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %29:array<mat3x2<f32>, 4> = access %25, %idx
        %30:array<mat2x3<f32>, 4> = call %tint_transpose_row_major_array_3, %29
        %32:ptr<function, array<mat2x3<f32>, 4>, read_write> = access %26, %idx
        store %32, %30
        continue  # -> $B6
      }
      $B6: {  # continuing
        %33:u32 = add %idx, 1u
        next_iteration %33  # -> $B5
      }
    }
    %34:array<array<mat2x3<f32>, 4>, 5> = load %26
    ret %34
  }
}
%tint_transpose_row_major_array_3 = func(%35:array<mat3x2<f32>, 4>):array<mat2x3<f32>, 4> {  # %tint_transpose_row_major_array_3: 'tint_transpose_row_major_array'
  $B8: {
    %36:ptr<function, array<mat2x3<f32>, 4>, read_write> = var undef
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %38:bool = gte %idx_1, 4u
        if %38 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %39:mat3x2<f32> = access %35, %idx_1
        %40:mat2x3<f32> = transpose %39
        %41:ptr<function, mat2x3<f32>, read_write> = access %36, %idx_1
        store %41, %40
        continue  # -> $B11
      }
      $B11: {  # continuing
        %42:u32 = add %idx_1, 1u
        next_iteration %42  # -> $B10
      }
    }
    %43:array<mat2x3<f32>, 4> = load %36
    ret %43
  }
}
%tint_transpose_row_major_array_1 = func(%44:array<array<mat2x3<f32>, 4>, 5>):array<array<mat3x2<f32>, 4>, 5> {  # %tint_transpose_row_major_array_1: 'tint_transpose_row_major_array'
  $B13: {
    %45:ptr<function, array<array<mat3x2<f32>, 4>, 5>, read_write> = var undef
    loop [i: $B14, b: $B15, c: $B16] {  # loop_3
      $B14: {  # initializer
        next_iteration 0u  # -> $B15
      }
      $B15 (%idx_2:u32): {  # body
        %47:bool = gte %idx_2, 5u
        if %47 [t: $B17] {  # if_3
          $B17: {  # true
            exit_loop  # loop_3
          }
        }
        %48:array<mat2x3<f32>, 4> = access %44, %idx_2
        %49:array<mat3x2<f32>, 4> = call %tint_transpose_row_major_array_2, %48
        %50:ptr<function, array<mat3x2<f32>, 4>, read_write> = access %45, %idx_2
        store %50, %49
        continue  # -> $B16
      }
      $B16: {  # continuing
        %51:u32 = add %idx_2, 1u
        next_iteration %51  # -> $B15
      }
    }
    %52:array<array<mat3x2<f32>, 4>, 5> = load %45
    ret %52
  }
}
%tint_transpose_row_major_array_2 = func(%53:array<mat2x3<f32>, 4>):array<mat3x2<f32>, 4> {  # %tint_transpose_row_major_array_2: 'tint_transpose_row_major_array'
  $B18: {
    %54:ptr<function, array<mat3x2<f32>, 4>, read_write> = var undef
    loop [i: $B19, b: $B20, c: $B21] {  # loop_4
      $B19: {  # initializer
        next_iteration 0u  # -> $B20
      }
      $B20 (%idx_3:u32): {  # body
        %56:bool = gte %idx_3, 4u
        if %56 [t: $B22] {  # if_4
          $B22: {  # true
            exit_loop  # loop_4
          }
        }
        %57:mat2x3<f32> = access %53, %idx_3
        %58:mat3x2<f32> = transpose %57
        %59:ptr<function, mat3x2<f32>, read_write> = access %54, %idx_3
        store %59, %58
        continue  # -> $B21
      }
      $B21: {  # continuing
        %60:u32 = add %idx_3, 1u
        next_iteration %60  # -> $B20
      }
    }
    %61:array<mat3x2<f32>, 4> = load %54
    ret %61
  }
}
%tint_store_row_major_column = func(%62:ptr<storage, mat3x2<f32>, read_write>, %63:u32, %64:vec3<f32>):void {
  $B23: {
    %65:f32 = access %64, 0u
    %66:ptr<storage, vec2<f32>, read_write> = access %62, 0u
    store_vector_element %66, %63, %65
    %67:f32 = access %64, 1u
    %68:ptr<storage, vec2<f32>, read_write> = access %62, 1u
    store_vector_element %68, %63, %67
    %69:f32 = access %64, 2u
    %70:ptr<storage, vec2<f32>, read_write> = access %62, 2u
    store_vector_element %70, %63, %69
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, ArrayOfMatrix_RuntimeSizedArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(128) array<mat4x3<f32>>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.arr[1] = s.arr[0];
    //   s.arr[2][3] = s.arr[1][2];
    //   s.arr[3][2][1] = s.arr[4][3][2];
    // }
    auto* matrix_member =
        ty.Get<core::type::StructMember>(mod.symbols.New("arr"), ty.runtime_array(ty.mat4x3<f32>()),
                                         0u, 0u, 32u, 128u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* sarr1 = b.Access(ty.ptr(storage, ty.mat4x3<f32>(), read_write), var, 0_u, 1_u);
        b.Store(sarr1,
                b.Load(b.Access(ty.ptr(storage, ty.mat4x3<f32>(), read_write), var, 0_u, 0_u)));

        auto* sarr23 = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 2_u, 3_u);
        b.Store(sarr23,
                b.Load(b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 1_u, 2_u)));

        auto* sarr32 = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 2_u, 3_u);
        auto* sarr43 = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 2_u, 3_u);
        b.StoreVectorElement(sarr32, 1_u, b.LoadVectorElement(sarr43, 2_u));

        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  arr:array<mat4x3<f32>> @offset(0) @size(128), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat4x3<f32>, read_write> = access %s, 0u, 1u
    %4:ptr<storage, mat4x3<f32>, read_write> = access %s, 0u, 0u
    %5:mat4x3<f32> = load %4
    store %3, %5
    %6:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 2u, 3u
    %7:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u, 2u
    %8:vec3<f32> = load %7
    store %6, %8
    %9:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 2u, 3u
    %10:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 2u, 3u
    %11:f32 = load_vector_element %10, 2u
    store_vector_element %9, 1u, %11
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(32) {
  arr:array<mat4x3<f32>> @offset(0) @size(128), @row_major
}

S_1 = struct @align(32) {
  arr:array<mat3x4<f32>> @offset(0) @size(128)
}

$B1: {  # root
  %s:ptr<storage, S_1, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 1u
    %4:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 0u
    %5:mat3x4<f32> = load %4
    %6:mat4x3<f32> = transpose %5
    %7:mat3x4<f32> = transpose %6
    store %3, %7
    %8:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 2u
    %9:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 1u
    %10:vec3<f32> = call %tint_load_row_major_column, %9, 2u
    %12:void = call %tint_store_row_major_column, %8, 3u, %10
    %14:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 2u
    %15:ptr<storage, vec4<f32>, read_write> = access %14, 1u
    %16:ptr<storage, mat3x4<f32>, read_write> = access %s, 0u, 2u
    %17:ptr<storage, vec4<f32>, read_write> = access %16, 2u
    %18:f32 = load_vector_element %17, 3u
    store_vector_element %15, 3u, %18
    ret
  }
}
%tint_load_row_major_column = func(%19:ptr<storage, mat3x4<f32>, read_write>, %20:u32):vec3<f32> {
  $B3: {
    %21:ptr<storage, vec4<f32>, read_write> = access %19, 0u
    %22:f32 = load_vector_element %21, %20
    %23:ptr<storage, vec4<f32>, read_write> = access %19, 1u
    %24:f32 = load_vector_element %23, %20
    %25:ptr<storage, vec4<f32>, read_write> = access %19, 2u
    %26:f32 = load_vector_element %25, %20
    %27:vec3<f32> = construct %22, %24, %26
    ret %27
  }
}
%tint_store_row_major_column = func(%28:ptr<storage, mat3x4<f32>, read_write>, %29:u32, %30:vec3<f32>):void {
  $B4: {
    %31:f32 = access %30, 0u
    %32:ptr<storage, vec4<f32>, read_write> = access %28, 0u
    store_vector_element %32, %29, %31
    %33:f32 = access %30, 1u
    %34:ptr<storage, vec4<f32>, read_write> = access %28, 1u
    store_vector_element %34, %29, %33
    %35:f32 = access %30, 2u
    %36:ptr<storage, vec4<f32>, read_write> = access %28, 2u
    store_vector_element %36, %29, %35
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, FunctionConstruct) {
    // struct S {
    //   @offset(16) @row_major m : mat2x3<f32>,
    // };
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   var<function> s = S(mat2x3(vec3f(0.f), vec3(1.f)));
    //   let x : mat2x3<f32> = s.m;
    // }

    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* init = b.Construct(
            strct, b.Composite(ty.mat2x3<f32>(), b.Splat<vec3<f32>>(0_f), b.Splat<vec3<f32>>(1_f)));
        auto* var = b.Var("s", ty.ptr<function>(strct));
        var->SetInitializer(init->Result());

        b.Let("x", b.Load(b.Access<ptr<function, mat2x3<f32>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:S = construct mat2x3<f32>(vec3<f32>(0.0f), vec3<f32>(1.0f))
    %s:ptr<function, S, read_write> = var %2
    %4:ptr<function, mat2x3<f32>, read_write> = access %s, 0u
    %5:mat2x3<f32> = load %4
    %x:mat2x3<f32> = let %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x2<f32> = transpose mat2x3<f32>(vec3<f32>(0.0f), vec3<f32>(1.0f))
    %3:S_1 = construct %2
    %s:ptr<function, S_1, read_write> = var %3
    %5:ptr<function, mat3x2<f32>, read_write> = access %s, 0u
    %6:mat3x2<f32> = load %5
    %7:mat2x3<f32> = transpose %6
    %x:mat2x3<f32> = let %7
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, FunctionConstant) {
    // struct S {
    //   @offset(16) @row_major m : mat2x3<f32>,
    // };
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   var<function> s = S(mat2x3(vec3f(0.f), vec3(1.f)));
    //   let x : mat2x3<f32> = s.m;
    // }

    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* init = b.Composite(
            strct, b.Composite(ty.mat2x3<f32>(), b.Splat<vec3<f32>>(0_f), b.Splat<vec3<f32>>(1_f)));
        auto* var = b.Var("s", ty.ptr<function>(strct));
        var->SetInitializer(init);

        b.Let("x", b.Load(b.Access<ptr<function, mat2x3<f32>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %s:ptr<function, S, read_write> = var S(mat2x3<f32>(vec3<f32>(0.0f), vec3<f32>(1.0f)))
    %3:ptr<function, mat2x3<f32>, read_write> = access %s, 0u
    %4:mat2x3<f32> = load %3
    %x:mat2x3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %s:ptr<function, S_1, read_write> = var S_1(mat3x2<f32>(vec2<f32>(0.0f, 1.0f)))
    %3:ptr<function, mat3x2<f32>, read_write> = access %s, 0u
    %4:mat3x2<f32> = load %3
    %5:mat2x3<f32> = transpose %4
    %x:mat2x3<f32> = let %5
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, PrivateConstant) {
    // struct S {
    //   @offset(16) @row_major m : mat2x3<f32>,
    // };
    //
    // var<private> s = S(mat2x3(vec3f(0.f), vec3(1.f)));
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x3<f32> = s.m;
    // }

    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    core::ir::Var* var = nullptr;
    b.Append(mod.root_block, [&] {
        auto* init =
            b.Composite(strct, b.Composite(ty.mat2x3<f32>(), b.Composite<vec3<f32>>(0_f, 1_f, 2_f),
                                           b.Composite<vec3<f32>>(3_f, 4_f, 5_f)));
        var = b.Var("s", ty.ptr<private_>(strct));
        var->SetInitializer(init);
    });

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<private_, mat2x3<f32>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<private, S, read_write> = var S(mat2x3<f32>(vec3<f32>(0.0f, 1.0f, 2.0f), vec3<f32>(3.0f, 4.0f, 5.0f)))
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat2x3<f32>, read_write> = access %s, 0u
    %4:mat2x3<f32> = load %3
    %x:mat2x3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

S_1 = struct @align(24) {
  m:mat3x2<f32> @offset(16)
}

$B1: {  # root
  %s:ptr<private, S_1, read_write> = var S_1(mat3x2<f32>(vec2<f32>(0.0f, 3.0f), vec2<f32>(1.0f, 4.0f), vec2<f32>(2.0f, 5.0f)))
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat3x2<f32>, read_write> = access %s, 0u
    %4:mat3x2<f32> = load %3
    %5:mat2x3<f32> = transpose %4
    %x:mat2x3<f32> = let %5
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
