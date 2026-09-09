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

#include "src/tint/lang/wgsl/reader/lower/lower.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using Wgslreader_LowerTest = core::ir::transform::TransformTest;

TEST_F(Wgslreader_LowerTest, BuiltinConversion) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Call<wgsl::ir::BuiltinCall>(ty.i32(), wgsl::BuiltinFn::kMax, 1_i, 2_i);
        b.Return(f);
    });

    auto* src = R"(
%f = func():void {
  $B1: {
    %2:i32 = wgsl.max 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():void {
  $B1: {
    %2:i32 = max 1i, 2i
    ret
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

TEST_F(Wgslreader_LowerTest, WorkgroupUniformLoad) {
    auto* wgvar = b.Var("wgvar", ty.ptr<workgroup, i32>());
    mod.root_block->Append(wgvar);

    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        auto* result = b.Call<wgsl::ir::BuiltinCall>(
            ty.i32(), wgsl::BuiltinFn::kWorkgroupUniformLoad, wgvar->Result());
        b.Return(f, result);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:i32 = wgsl.workgroupUniformLoad %wgvar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, i32, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:i32 = load %wgvar
    %5:void = workgroupBarrier
    ret %4
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

TEST_F(Wgslreader_LowerTest, WorkgroupUniformLoadAtomic) {
    auto* wgvar = b.Var("wgvar", ty.ptr<workgroup, atomic<i32>>());
    mod.root_block->Append(wgvar);

    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        auto* result = b.Call<wgsl::ir::BuiltinCall>(
            ty.i32(), wgsl::BuiltinFn::kWorkgroupUniformLoad, wgvar->Result());
        b.Return(f, result);
    });

    auto* src = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:i32 = wgsl.workgroupUniformLoad %wgvar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %wgvar:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%f = func():i32 {
  $B2: {
    %3:void = workgroupBarrier
    %4:i32 = atomicLoad %wgvar
    %5:void = workgroupBarrier
    ret %4
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

// TODO(529415904): Remove these after fully transitioned
TEST_F(Wgslreader_LowerTest, SubgroupMatrixLoad_u8_const) {
    mod.properties.Add(core::ir::Property::kAllow8BitIntegers);
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u8(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.u32())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.CallExplicit<wgsl::ir::BuiltinCall>(mat_ty, wgsl::BuiltinFn::kSubgroupMatrixLoad,
                                              Vector<core::ir::TemplateParameter, 1>{mat_ty}, v,
                                              4_u, true, 8_u);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:subgroup_matrix_left<u8, 8, 8> = wgsl.subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>> %v, 4u, true, 8u
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:subgroup_matrix_left<u8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major> %v, 1u, 2u
    ret
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

TEST_F(Wgslreader_LowerTest, SubgroupMatrixLoad_f16) {
    mod.properties.Add(core::ir::Property::kAllow16BitFloats);
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f16(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.f16())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* stride = b.FunctionParam("stride", ty.u32());
    foo->SetParams({offset, stride});
    b.Append(foo->Block(), [&] {
        b.CallExplicit<wgsl::ir::BuiltinCall>(mat_ty, wgsl::BuiltinFn::kSubgroupMatrixLoad,
                                              Vector<core::ir::TemplateParameter, 1>{mat_ty}, v,
                                              offset, true, stride);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:subgroup_matrix_left<f16, 8, 8> = wgsl.subgroupMatrixLoad<subgroup_matrix_left<f16, 8, 8>> %v, %offset, true, %stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<f16>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:u32, %stride:u32):void {
  $B2: {
    %5:subgroup_matrix_left<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f16, 8, 8>, col_major> %v, %offset, %stride
    ret
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

TEST_F(Wgslreader_LowerTest, SubgroupMatrixStore_u8) {
    mod.properties.Add(core::ir::Property::kAllow8BitIntegers);
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u8(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.u32())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* m = b.FunctionParam("m", mat_ty);
    auto* offset = b.FunctionParam("offset", ty.i32());
    auto* stride = b.FunctionParam("stride", ty.i32());
    foo->SetParams({m, offset, stride});
    b.Append(foo->Block(), [&] {
        b.Call<wgsl::ir::BuiltinCall>(ty.void_(), wgsl::BuiltinFn::kSubgroupMatrixStore, v, offset,
                                      m, true, stride);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<u8, 8, 8>, %offset:i32, %stride:i32):void {
  $B2: {
    %6:void = wgsl.subgroupMatrixStore %v, %offset, %m, true, %stride
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<u8, 8, 8>, %offset:i32, %stride:i32):void {
  $B2: {
    %6:u32 = bitcast<u32> %offset
    %7:u32 = div %6, 4u
    %8:u32 = bitcast<u32> %stride
    %9:u32 = div %8, 4u
    %10:void = subgroupMatrixStore<col_major> %v, %7, %m, %9
    ret
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

TEST_F(Wgslreader_LowerTest, SubgroupMatrixStore_f32_const) {
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.f32())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* m = b.FunctionParam("m", mat_ty);
    foo->SetParams({m});
    b.Append(foo->Block(), [&] {
        b.Call<wgsl::ir::BuiltinCall>(ty.void_(), wgsl::BuiltinFn::kSubgroupMatrixStore, v, 4_i, m,
                                      false, 8_i);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f32, 8, 8>):void {
  $B2: {
    %4:void = wgsl.subgroupMatrixStore %v, 4i, %m, false, 8i
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%m:subgroup_matrix_left<f32, 8, 8>):void {
  $B2: {
    %4:void = subgroupMatrixStore<row_major> %v, 4u, %m, 8u
    ret
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::reader
