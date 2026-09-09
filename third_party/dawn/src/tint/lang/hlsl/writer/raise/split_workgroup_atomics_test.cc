// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/raise/split_workgroup_atomics.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/decompose_access.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/atomic.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using SplitWorkgroupAtomicsTest = core::ir::transform::TransformTest;

// Workgroup variables without atomics are not modified.
TEST_F(SplitWorkgroupAtomicsTest, NoAtomics) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.u32()},
                                                       {mod.symbols.New("b"), ty.f32()},
                                                   });

    auto* var = b.Var("wg", ty.ptr(workgroup, str_ty));
    mod.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* ptr = b.Access(ty.ptr(workgroup, ty.u32()), var, 0_u);
        b.Store(ptr, 42_u);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %3, 42u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = src;
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

// Split a simple structure with one atomic member.
TEST_F(SplitWorkgroupAtomicsTest, SimpleStructWithAtomic) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("data"), ty.u32()},
                                            {mod.symbols.New("counter"), ty.atomic<u32>()},
                                        });

    auto* var = b.Var("wg", ty.ptr(workgroup, str_ty));
    mod.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        // Non-atomic access.
        auto* data_ptr = b.Access(ty.ptr(workgroup, ty.u32()), var, 0_u);
        b.Store(data_ptr, 1_u);

        // Atomic access.
        auto* atomic_ptr = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), var, 1_u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, atomic_ptr, 0_u);

        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  data:u32 @offset(0)
  counter:atomic<u32> @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %3, 1u
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u
    %5:void = atomicStore %4, 0u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  data:u32 @offset(0)
  counter:atomic<u32> @offset(4)
}

S_data = struct @align(4) {
  data:u32 @offset(0)
  counter:u32 @offset(4)
}

$B1: {  # root
  %counter:ptr<workgroup, atomic<u32>, read_write> = var undef
  %data:ptr<workgroup, S_data, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = access %data, 0u
    store %4, 1u
    %5:void = atomicStore %counter, 0u
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

// Preserve an outer array even when it contains only one element.
TEST_F(SplitWorkgroupAtomicsTest, ArrayOfOneStructPreservesArrayShape) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("counter"), ty.atomic<u32>()},
                                        });
    auto* var = b.Var("wg", ty.ptr(workgroup, ty.array(str_ty, 1)));
    mod.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* atomic = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), var, 0_u, 0_u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, atomic, 7_u);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  counter:atomic<u32> @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 1>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u, 0u
    %4:void = atomicStore %3, 7u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  counter:atomic<u32> @offset(0)
}

S_data = struct @align(4) {
  counter:u32 @offset(0)
}

$B1: {  # root
  %counter:ptr<workgroup, array<atomic<u32>, 1>, read_write> = var undef
  %data:ptr<workgroup, array<S_data, 1>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<workgroup, atomic<u32>, read_write> = access %counter, 0u
    %5:void = atomicStore %4, 7u
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

// Split each atomic member into its own array while preserving the struct array index.
TEST_F(SplitWorkgroupAtomicsTest, MultipleAtomicMembersPreserveStructArrayIndices) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("first"), ty.atomic<u32>()},
                                            {mod.symbols.New("second"), ty.atomic<u32>()},
                                        });
    auto* var = b.Var("wg", ty.ptr(workgroup, ty.array(str_ty, 3)));
    mod.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* first = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), var, 1_u, 0_u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, first, 10_u);

        auto* second = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), var, 2_u, 1_u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, second, 20_u);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  first:atomic<u32> @offset(0)
  second:atomic<u32> @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 3>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u, 0u
    %4:void = atomicStore %3, 10u
    %5:ptr<workgroup, atomic<u32>, read_write> = access %wg, 2u, 1u
    %6:void = atomicStore %5, 20u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  first:atomic<u32> @offset(0)
  second:atomic<u32> @offset(4)
}

S_data = struct @align(4) {
  first:u32 @offset(0)
  second:u32 @offset(4)
}

$B1: {  # root
  %first:ptr<workgroup, array<atomic<u32>, 3>, read_write> = var undef
  %second:ptr<workgroup, array<atomic<u32>, 3>, read_write> = var undef
  %data:ptr<workgroup, array<S_data, 3>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<workgroup, atomic<u32>, read_write> = access %first, 1u
    %6:void = atomicStore %5, 10u
    %7:ptr<workgroup, atomic<u32>, read_write> = access %second, 2u
    %8:void = atomicStore %7, 20u
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

// Preserve nested array dimensions and indices for each atomic leaf.
TEST_F(SplitWorkgroupAtomicsTest, NestedAtomicArraysPreserveDimensions) {
    auto* inner_ty =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("counter"), ty.atomic<u32>()},
                                            });
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("items"), ty.array(inner_ty, 3)},
                                        });
    auto* var = b.Var("wg", ty.ptr(workgroup, ty.array(str_ty, 2)));
    mod.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* atomic = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), var, 1_u, 0_u, 2_u, 0_u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, atomic, 9_u);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  counter:atomic<u32> @offset(0)
}

S = struct @align(4) {
  items:array<Inner, 3> @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 2>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u, 0u, 2u, 0u
    %4:void = atomicStore %3, 9u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  counter:atomic<u32> @offset(0)
}

S = struct @align(4) {
  items:array<Inner, 3> @offset(0)
}

Inner_data = struct @align(4) {
  counter:u32 @offset(0)
}

S_data = struct @align(4) {
  items:array<Inner_data, 3> @offset(0)
}

$B1: {  # root
  %items_counter:ptr<workgroup, array<array<atomic<u32>, 3>, 2>, read_write> = var undef
  %data:ptr<workgroup, array<S_data, 2>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<workgroup, atomic<u32>, read_write> = access %items_counter, 1u, 2u
    %5:void = atomicStore %4, 9u
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

// Normalize pointer aliases and chained accesses before splitting an atomic leaf.
TEST_F(SplitWorkgroupAtomicsTest, ChainedAccessAndPointerAliases) {
    auto* inner_ty =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("counter"), ty.atomic<u32>()},
                                            });
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("inner"), inner_ty},
                                                   });
    auto* var = b.Var("wg", ty.ptr(workgroup, str_ty));
    mod.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* var_alias = b.Let("var_alias", var);
        auto* inner = b.Access(ty.ptr(workgroup, inner_ty), var_alias, 0_u);
        auto* inner_alias = b.Let("inner_alias", inner);
        auto* atomic = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), inner_alias, 0_u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, atomic, 11_u);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  counter:atomic<u32> @offset(0)
}

S = struct @align(4) {
  inner:Inner @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %var_alias:ptr<workgroup, S, read_write> = let %wg
    %4:ptr<workgroup, Inner, read_write> = access %var_alias, 0u
    %inner_alias:ptr<workgroup, Inner, read_write> = let %4
    %6:ptr<workgroup, atomic<u32>, read_write> = access %inner_alias, 0u
    %7:void = atomicStore %6, 11u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  counter:atomic<u32> @offset(0)
}

S = struct @align(4) {
  inner:Inner @offset(0)
}

Inner_data = struct @align(4) {
  counter:u32 @offset(0)
}

S_data = struct @align(4) {
  inner:Inner_data @offset(0)
}

$B1: {  # root
  %inner_counter:ptr<workgroup, atomic<u32>, read_write> = var undef
  %data:ptr<workgroup, S_data, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:void = atomicStore %inner_counter, 11u
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

// Verify that f16 matrix data interoperates with workgroup access decomposition.
TEST_F(SplitWorkgroupAtomicsTest, F16MatrixDataInteroperatesWithDecomposeAccess) {
    mod.properties.Add(core::ir::Property::kAllow16BitFloats);

    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("x"), ty.mat4x4<f16>()},
                                                       {mod.symbols.New("y"), ty.atomic<u32>()},
                                                   });
    auto* var = b.Var("out", ty.ptr(workgroup, str_ty));
    mod.root_block->Append(var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* matrix_column = b.Access(ty.ptr(workgroup, ty.vec4h()), var, 0_u, 0_u);
        b.StoreVectorElement(matrix_column, 0_u, 0_h);

        auto* atomic = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), var, 1_u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, atomic, 0_u);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(8) {
  x:mat4x4<f16> @offset(0)
  y:atomic<u32> @offset(32)
}

$B1: {  # root
  %out:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, vec4<f16>, read_write> = access %out, 0u, 0u
    store_vector_element %3, 0u, 0.0h
    %4:ptr<workgroup, atomic<u32>, read_write> = access %out, 1u
    %5:void = atomicStore %4, 0u
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(8) {
  x:mat4x4<f16> @offset(0)
  y:atomic<u32> @offset(32)
}

S_data = struct @align(8) {
  x:mat4x4<f16> @offset(0)
  y:u32 @offset(32)
}

$B1: {  # root
  %y:ptr<workgroup, atomic<u32>, read_write> = var undef
  %data:ptr<workgroup, S_data, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<workgroup, vec4<f16>, read_write> = access %data, 0u, 0u
    store_vector_element %4, 0u, 0.0h
    %5:void = atomicStore %y, 0u
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    ASSERT_EQ(expect, str());

    core::ir::transform::DecomposeAccessConfig options{.workgroup = true};
    Run(core::ir::transform::DecomposeAccess, options);

    auto* decomposed =
        // The storage input and output remain unchanged. The workgroup data is lowered to a
        // layout-preserving u16 array, while the atomic member is split into a separate variable.
        R"(
S = struct @align(8) {
  x:mat4x4<f16> @offset(0)
  y:atomic<u32> @offset(32)
}

S_data = struct @align(8) {
  x:mat4x4<f16> @offset(0)
  y:u32 @offset(32)
}

$B1: {  # root
  %y:ptr<workgroup, atomic<u32>, read_write> = var undef
  %data:ptr<workgroup, array<u16, 20>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u16 = bitcast<u16> 0.0h
    %5:ptr<workgroup, u16, read_write> = access %data, 0u
    store %5, %4
    %6:void = atomicStore %y, 0u
    ret
  }
}
)";
    EXPECT_EQ(decomposed, str());
}

// Copy a shared structure from storage to workgroup memory one member at a time.
// Structures containing atomics cannot be loaded or stored as whole values.
TEST_F(SplitWorkgroupAtomicsTest, CopyStorageStructMembersToWorkgroup) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("data"), ty.u32()},
                                            {mod.symbols.New("counter"), ty.atomic<u32>()},
                                        });

    auto* storage_var = b.Var("storage_var", ty.ptr(storage, str_ty));
    storage_var->SetBindingPoint(0, 0);
    mod.root_block->Append(storage_var);
    auto* workgroup_var = b.Var("workgroup_var", ty.ptr(workgroup, str_ty));
    mod.root_block->Append(workgroup_var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* storage_data = b.Access(ty.ptr(storage, ty.u32()), storage_var, 0_u);
        auto* workgroup_data = b.Access(ty.ptr(workgroup, ty.u32()), workgroup_var, 0_u);
        b.Store(workgroup_data, b.Load(storage_data));

        auto* storage_atomic = b.Access(ty.ptr(storage, ty.atomic<u32>()), storage_var, 1_u);
        auto* workgroup_atomic = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), workgroup_var, 1_u);
        auto* atomic_value = b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, storage_atomic);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, workgroup_atomic, atomic_value);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  data:u32 @offset(0)
  counter:atomic<u32> @offset(4)
}

$B1: {  # root
  %storage_var:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %workgroup_var:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<storage, u32, read_write> = access %storage_var, 0u
    %5:ptr<workgroup, u32, read_write> = access %workgroup_var, 0u
    %6:u32 = load %4
    store %5, %6
    %7:ptr<storage, atomic<u32>, read_write> = access %storage_var, 1u
    %8:ptr<workgroup, atomic<u32>, read_write> = access %workgroup_var, 1u
    %9:u32 = atomicLoad %7
    %10:void = atomicStore %8, %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  data:u32 @offset(0)
  counter:atomic<u32> @offset(4)
}

S_data = struct @align(4) {
  data:u32 @offset(0)
  counter:u32 @offset(4)
}

$B1: {  # root
  %storage_var:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %counter:ptr<workgroup, atomic<u32>, read_write> = var undef
  %data:ptr<workgroup, S_data, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, u32, read_write> = access %storage_var, 0u
    %6:ptr<workgroup, u32, read_write> = access %data, 0u
    %7:u32 = load %5
    store %6, %7
    %8:ptr<storage, atomic<u32>, read_write> = access %storage_var, 1u
    %9:u32 = atomicLoad %8
    %10:void = atomicStore %counter, %9
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

// Copy a shared structure from workgroup memory to storage one member at a time.
// Structures containing atomics cannot be loaded or stored as whole values.
TEST_F(SplitWorkgroupAtomicsTest, CopyWorkgroupStructMembersToStorage) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("data"), ty.u32()},
                                            {mod.symbols.New("counter"), ty.atomic<u32>()},
                                        });

    auto* storage_var = b.Var("storage_var", ty.ptr(storage, str_ty));
    storage_var->SetBindingPoint(0, 0);
    mod.root_block->Append(storage_var);
    auto* workgroup_var = b.Var("workgroup_var", ty.ptr(workgroup, str_ty));
    mod.root_block->Append(workgroup_var);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* workgroup_data = b.Access(ty.ptr(workgroup, ty.u32()), workgroup_var, 0_u);
        auto* storage_data = b.Access(ty.ptr(storage, ty.u32()), storage_var, 0_u);
        b.Store(storage_data, b.Load(workgroup_data));

        auto* workgroup_atomic = b.Access(ty.ptr(workgroup, ty.atomic<u32>()), workgroup_var, 1_u);
        auto* storage_atomic = b.Access(ty.ptr(storage, ty.atomic<u32>()), storage_var, 1_u);
        auto* atomic_value = b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, workgroup_atomic);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, storage_atomic, atomic_value);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  data:u32 @offset(0)
  counter:atomic<u32> @offset(4)
}

$B1: {  # root
  %storage_var:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %workgroup_var:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = access %workgroup_var, 0u
    %5:ptr<storage, u32, read_write> = access %storage_var, 0u
    %6:u32 = load %4
    store %5, %6
    %7:ptr<workgroup, atomic<u32>, read_write> = access %workgroup_var, 1u
    %8:ptr<storage, atomic<u32>, read_write> = access %storage_var, 1u
    %9:u32 = atomicLoad %7
    %10:void = atomicStore %8, %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  data:u32 @offset(0)
  counter:atomic<u32> @offset(4)
}

S_data = struct @align(4) {
  data:u32 @offset(0)
  counter:u32 @offset(4)
}

$B1: {  # root
  %storage_var:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %counter:ptr<workgroup, atomic<u32>, read_write> = var undef
  %data:ptr<workgroup, S_data, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<workgroup, u32, read_write> = access %data, 0u
    %6:ptr<storage, u32, read_write> = access %storage_var, 0u
    %7:u32 = load %5
    store %6, %7
    %8:ptr<storage, atomic<u32>, read_write> = access %storage_var, 1u
    %9:u32 = atomicLoad %counter
    %10:void = atomicStore %8, %9
    ret
  }
}
)";
    Run(SplitWorkgroupAtomics);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
