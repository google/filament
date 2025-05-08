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

#include "src/tint/lang/spirv/reader/lower/atomics.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvReader_AtomicsTest = core::ir::transform::TransformTest;

TEST_F(SpirvReader_AtomicsTest, ArrayStore) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr<workgroup, array<u32, 4>, read_write>()); });

    b.Append(f->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 1i
    %4:void = spirv.atomic_store %3, 1u, 0u, 1u
    ret
  }
}
)";
    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<atomic<u32>, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1i
    %4:void = atomicStore %3, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ArrayStore_CopiedObject) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr<workgroup, array<u32, 4>, read_write>()); });

    b.Append(f->Block(), [&] {  //
        auto* l = b.Let(wg);
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), l, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 1_u, 0_u,
                                       2_u);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, array<u32, 4>, read_write> = let %wg
    %4:ptr<workgroup, u32, read_write> = access %3, 1i
    %5:void = spirv.atomic_store %4, 1u, 0u, 2u
    ret
  }
}
)";
    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<atomic<u32>, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, array<atomic<u32>, 4>, read_write> = let %wg
    %4:ptr<workgroup, atomic<u32>, read_write> = access %3, 1i
    %5:void = atomicStore %4, 2u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ArrayNested) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] {
        wg = b.Var("wg", ty.ptr<workgroup, array<array<array<u32, 1>, 2>, 3>, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_i, 1_i, 0_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 2_u, 0_u,
                                       1_u);
        b.Return(f);
    });
    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<array<array<u32, 1>, 2>, 3>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 2i, 1i, 0i
    %4:void = spirv.atomic_store %3, 2u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<array<array<atomic<u32>, 1>, 2>, 3>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %wg, 2i, 1i, 0i
    %4:void = atomicStore %3, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, FlatSingleAtomic) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("x"), ty.i32()},
                                                   {mod.symbols.New("a"), ty.u32()},
                                                   {mod.symbols.New("y"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<workgroup, i32, read_write>(), wg, 0_u);
        b.Store(a0, 0_i);
        auto* a1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a1, 2_u, 0_u,
                                       0_u);
        auto* a2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_u);
        b.Store(a2, 0_u);
        b.Return(f);
    });
    auto* src = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  y:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 0u
    store %3, 0i
    %4:ptr<workgroup, u32, read_write> = access %wg, 1u
    %5:void = spirv.atomic_store %4, 2u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 2u
    store %6, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  y:u32 @offset(8)
}

S_atomic = struct @align(4) {
  x:i32 @offset(0)
  a:atomic<u32> @offset(4)
  y:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 0u
    store %3, 0i
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u
    %5:void = atomicStore %4, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 2u
    store %6, 0u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, FlatMultipleAtomics) {
    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("x"), ty.i32()},
                                                   {mod.symbols.New("a"), ty.u32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    auto* f = b.ComputeFunction("main");
    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<workgroup, i32, read_write>(), wg, 0_u);
        b.Store(a0, 0_i);
        auto* a1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a1, 2_u, 0_u,
                                       0_u);
        auto* a2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a2, 2_u, 0_u,
                                       0_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  b:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 0u
    store %3, 0i
    %4:ptr<workgroup, u32, read_write> = access %wg, 1u
    %5:void = spirv.atomic_store %4, 2u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 2u
    %7:void = spirv.atomic_store %6, 2u, 0u, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  b:u32 @offset(8)
}

S_atomic = struct @align(4) {
  x:i32 @offset(0)
  a:atomic<u32> @offset(4)
  b:atomic<u32> @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 0u
    store %3, 0i
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u
    %5:void = atomicStore %4, 0u
    %6:ptr<workgroup, atomic<u32>, read_write> = access %wg, 2u
    %7:void = atomicStore %6, 0u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, Nested) {
    auto* f = b.ComputeFunction("main");

    auto* s0 = ty.Struct(mod.symbols.New("S0"), {
                                                    {mod.symbols.New("x"), ty.i32()},
                                                    {mod.symbols.New("a"), ty.u32()},
                                                    {mod.symbols.New("y"), ty.i32()},
                                                    {mod.symbols.New("z"), ty.i32()},
                                                });
    auto* s1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("x"), ty.i32()},
                                                    {mod.symbols.New("a"), s0},
                                                    {mod.symbols.New("y"), ty.i32()},
                                                    {mod.symbols.New("z"), ty.i32()},
                                                });
    auto* s2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("x"), ty.i32()},
                                                    {mod.symbols.New("y"), ty.i32()},
                                                    {mod.symbols.New("z"), ty.i32()},
                                                    {mod.symbols.New("a"), s1},
                                                });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, s2, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<workgroup, i32, read_write>(), wg, 3_u, 1_u, 0_u);
        b.Store(a0, 0_i);

        auto* a1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 3_u, 1_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a1, 2_u, 0_u,
                                       0_u);

        auto* a2 = b.Access(ty.ptr<workgroup, i32, read_write>(), wg, 3_u, 1_u, 2_u);
        b.Store(a2, 0_i);
        b.Return(f);
    });

    auto* src = R"(
S0 = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  y:i32 @offset(8)
  z:i32 @offset(12)
}

S1 = struct @align(4) {
  x_1:i32 @offset(0)
  a_1:S0 @offset(4)
  y_1:i32 @offset(20)
  z_1:i32 @offset(24)
}

S2 = struct @align(4) {
  x_2:i32 @offset(0)
  y_2:i32 @offset(4)
  z_2:i32 @offset(8)
  a_2:S1 @offset(12)
}

$B1: {  # root
  %wg:ptr<workgroup, S2, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 3u, 1u, 0u
    store %3, 0i
    %4:ptr<workgroup, u32, read_write> = access %wg, 3u, 1u, 1u
    %5:void = spirv.atomic_store %4, 2u, 0u, 0u
    %6:ptr<workgroup, i32, read_write> = access %wg, 3u, 1u, 2u
    store %6, 0i
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S0 = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  y:i32 @offset(8)
  z:i32 @offset(12)
}

S1 = struct @align(4) {
  x_1:i32 @offset(0)
  a_1:S0 @offset(4)
  y_1:i32 @offset(20)
  z_1:i32 @offset(24)
}

S2 = struct @align(4) {
  x_2:i32 @offset(0)
  y_2:i32 @offset(4)
  z_2:i32 @offset(8)
  a_2:S1 @offset(12)
}

S0_atomic = struct @align(4) {
  x:i32 @offset(0)
  a:atomic<u32> @offset(4)
  y:i32 @offset(8)
  z:i32 @offset(12)
}

S1_atomic = struct @align(4) {
  x_1:i32 @offset(0)
  a_1:S0_atomic @offset(4)
  y_1:i32 @offset(20)
  z_1:i32 @offset(24)
}

S2_atomic = struct @align(4) {
  x_2:i32 @offset(0)
  y_2:i32 @offset(4)
  z_2:i32 @offset(8)
  a_2:S1_atomic @offset(12)
}

$B1: {  # root
  %wg:ptr<workgroup, S2_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 3u, 1u, 0u
    store %3, 0i
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 3u, 1u, 1u
    %5:void = atomicStore %4, 0u
    %6:ptr<workgroup, i32, read_write> = access %wg, 3u, 1u, 2u
    store %6, 0i
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ArrayOfStruct) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("x"), ty.i32()},
                                                   {mod.symbols.New("a"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr(workgroup, ty.array(sb, 10), read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 4_i, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 2_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 10>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 4i, 1u
    %4:void = spirv.atomic_store %3, 2u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
}

S_atomic = struct @align(4) {
  x:i32 @offset(0)
  a:atomic<u32> @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S_atomic, 10>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %wg, 4i, 1u
    %4:void = atomicStore %3, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ArrayOfStruct_Let) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("x"), ty.i32()},
                                                   {mod.symbols.New("a"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr(workgroup, ty.array(sb, 10), read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* l = b.Let(wg);
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), l, 4_i, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 2_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 10>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, array<S, 10>, read_write> = let %wg
    %4:ptr<workgroup, u32, read_write> = access %3, 4i, 1u
    %5:void = spirv.atomic_store %4, 2u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
}

S_atomic = struct @align(4) {
  x:i32 @offset(0)
  a:atomic<u32> @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S_atomic, 10>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, array<S_atomic, 10>, read_write> = let %wg
    %4:ptr<workgroup, atomic<u32>, read_write> = access %3, 4i, 1u
    %5:void = atomicStore %4, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, StructOfArray) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("x"), ty.i32()},
                                                   {mod.symbols.New("a"), ty.array(ty.u32(), 10)},
                                                   {mod.symbols.New("y"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_u, 4_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 2_u, 0_u,
                                       1_u);
        b.Return(f);
    });
    auto* src = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:array<u32, 10> @offset(4)
  y:u32 @offset(44)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 1u, 4i
    %4:void = spirv.atomic_store %3, 2u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:array<u32, 10> @offset(4)
  y:u32 @offset(44)
}

S_atomic = struct @align(4) {
  x:i32 @offset(0)
  a:array<atomic<u32>, 10> @offset(4)
  y:u32 @offset(44)
}

$B1: {  # root
  %wg:ptr<workgroup, S_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u, 4i
    %4:void = atomicStore %3, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicAdd) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIAdd, a0, 1_u, 0_u, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, a1, 1_u, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIAdd, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_add %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_add %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_i_add %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_i_add %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  a:atomic<i32> @offset(0)
  b:atomic<u32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, atomic<i32>, read_write> = var undef
  %wg_u32:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, atomic<i32>, read_write> = access %sb, 0u
    %6:i32 = atomicAdd %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicAdd %7, 1u
    %9:i32 = atomicAdd %wg_i32, 1i
    %10:u32 = atomicAdd %wg_u32, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicSub) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicISub, a0, 1_u, 0_u, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicISub, a1, 1_u, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicISub, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicISub, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_sub %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_sub %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_i_sub %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_i_sub %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  a:atomic<i32> @offset(0)
  b:atomic<u32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, atomic<i32>, read_write> = var undef
  %wg_u32:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, atomic<i32>, read_write> = access %sb, 0u
    %6:i32 = atomicSub %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicSub %7, 1u
    %9:i32 = atomicSub %wg_i32, 1i
    %10:u32 = atomicSub %wg_u32, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicAnd) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicAnd, a0, 1_u, 0_u, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicAnd, a1, 1_u, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicAnd, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicAnd, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_and %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_and %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_and %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_and %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  a:atomic<i32> @offset(0)
  b:atomic<u32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, atomic<i32>, read_write> = var undef
  %wg_u32:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, atomic<i32>, read_write> = access %sb, 0u
    %6:i32 = atomicAnd %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicAnd %7, 1u
    %9:i32 = atomicAnd %wg_i32, 1i
    %10:u32 = atomicAnd %wg_u32, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicOr) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicOr, a0, 1_u, 0_u, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicOr, a1, 1_u, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicOr, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicOr, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });
    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_or %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_or %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_or %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_or %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  a:atomic<i32> @offset(0)
  b:atomic<u32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, atomic<i32>, read_write> = var undef
  %wg_u32:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, atomic<i32>, read_write> = access %sb, 0u
    %6:i32 = atomicOr %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicOr %7, 1u
    %9:i32 = atomicOr %wg_i32, 1i
    %10:u32 = atomicOr %wg_u32, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicXor) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicXor, a0, 1_u, 0_u, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicXor, a1, 1_u, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicXor, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicXor, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_xor %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_xor %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_xor %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_xor %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  a:atomic<i32> @offset(0)
  b:atomic<u32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, atomic<i32>, read_write> = var undef
  %wg_u32:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, atomic<i32>, read_write> = access %sb, 0u
    %6:i32 = atomicXor %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicXor %7, 1u
    %9:i32 = atomicXor %wg_i32, 1i
    %10:u32 = atomicXor %wg_u32, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicMax) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicSMax, a0, 1_u, 0_u, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicUMax, a1, 1_u, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicSMax, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicUMax, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_s_max %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_u_max %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_s_max %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_u_max %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  a:atomic<i32> @offset(0)
  b:atomic<u32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, atomic<i32>, read_write> = var undef
  %wg_u32:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, atomic<i32>, read_write> = access %sb, 0u
    %6:i32 = atomicMax %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicMax %7, 1u
    %9:i32 = atomicMax %wg_i32, 1i
    %10:u32 = atomicMax %wg_u32, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicMin) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicSMin, a0, 1_u, 0_u, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicUMin, a1, 1_u, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicSMin, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicUMin, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_s_min %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_u_min %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_s_min %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_u_min %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  a:atomic<i32> @offset(0)
  b:atomic<u32> @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, atomic<i32>, read_write> = var undef
  %wg_u32:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, atomic<i32>, read_write> = access %sb, 0u
    %6:i32 = atomicMin %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicMin %7, 1u
    %9:i32 = atomicMin %wg_i32, 1i
    %10:u32 = atomicMin %wg_u32, 1u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_AtomicExchange) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicExchange, a0, 1_u, 0_u,
                                       1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicExchange, a1, 1_u, 0_u,
                                       1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicExchange, wg_i32, 1_u,
                                       0_u, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicExchange, wg_u32, 1_u,
                                       0_u, 1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_exchange %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_exchange %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_exchange %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_exchange %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_AtomicCompareExchange) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicCompareExchange, a0, 1_u,
                                       0_u, 0_u, 1_i, 1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicCompareExchange, a1, 1_u,
                                       0_u, 0_u, 1_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicCompareExchange, wg_i32,
                                       1_u, 0_u, 0_u, 1_i, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicCompareExchange, wg_u32,
                                       1_u, 0_u, 0_u, 1_u, 1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_compare_exchange %5, 1u, 0u, 0u, 1i, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_compare_exchange %7, 1u, 0u, 0u, 1u, 1u
    %9:i32 = spirv.atomic_compare_exchange %wg_i32, 1u, 0u, 0u, 1i, 1i
    %10:u32 = spirv.atomic_compare_exchange %wg_u32, 1u, 0u, 0u, 1u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_AtomicLoad) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicLoad, a0, 1_u, 0_u);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicLoad, a1, 1_u, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicLoad, wg_i32, 1_u, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicLoad, wg_u32, 1_u, 0_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_load %5, 1u, 0u
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_load %7, 1u, 0u
    %9:i32 = spirv.atomic_load %wg_i32, 1u, 0u
    %10:u32 = spirv.atomic_load %wg_u32, 1u, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_AtomicStore) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a0, 1_u, 0_u,
                                       1_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a1, 1_u, 0_u,
                                       1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, wg_i32, 1_u, 0_u,
                                       1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, wg_u32, 1_u, 0_u,
                                       1_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:void = spirv.atomic_store %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:void = spirv.atomic_store %7, 1u, 0u, 1u
    %9:void = spirv.atomic_store %wg_i32, 1u, 0u, 1i
    %10:void = spirv.atomic_store %wg_u32, 1u, 0u, 1u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_AtomicDecrement) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIDecrement, a0, 1_u, 0_u);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIDecrement, a1, 1_u, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIDecrement, wg_i32, 1_u,
                                       0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIDecrement, wg_u32, 1_u,
                                       0_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_decrement %5, 1u, 0u
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_decrement %7, 1u, 0u
    %9:i32 = spirv.atomic_i_decrement %wg_i32, 1u, 0u
    %10:u32 = spirv.atomic_i_decrement %wg_u32, 1u, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_AtomicIncrement) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.i32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                               });

    core::ir::Var* wg_u32 = nullptr;
    core::ir::Var* wg_i32 = nullptr;
    core::ir::Var* sg = nullptr;
    b.Append(mod.root_block, [&] {
        sg = b.Var("sb", ty.ptr(storage, sb, read_write));
        sg->SetBindingPoint(0, 0);

        wg_i32 = b.Var("wg_i32", ty.ptr<workgroup, i32, read_write>());
        wg_u32 = b.Var("wg_u32", ty.ptr<workgroup, u32, read_write>());
    });

    b.Append(f->Block(), [&] {  //
        auto* a0 = b.Access(ty.ptr<storage, i32, read_write>(), sg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIIncrement, a0, 1_u, 0_u);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIIncrement, a1, 1_u, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIIncrement, wg_i32, 1_u,
                                       0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIIncrement, wg_u32, 1_u,
                                       0_u);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_increment %5, 1u, 0u
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_increment %7, 1u, 0u
    %9:i32 = spirv.atomic_i_increment %wg_i32, 1u, 0u
    %10:u32 = spirv.atomic_i_increment %wg_u32, 1u, 0u
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_Scalar) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, ty.u32(), read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* v = b.Var("b", ty.ptr(function, ty.u32(), read_write));
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, wg, 1_u, 0_u, 0_u);
        b.Store(wg, 0_u);
        auto* l0 = b.Load(wg);
        b.Let(l0);
        auto* l1 = b.Load(wg);
        b.Store(v, l1);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:u32 = spirv.atomic_i_add %wg, 1u, 0u, 0u
    store %wg, 0u
    %5:u32 = load %wg
    %6:u32 = let %5
    %7:u32 = load %wg
    store %b, %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_Struct) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());
        auto* l1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 0_u);

        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        auto* v2 = b.Load(l4);
        b.Store(b_, v2);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0u
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 0u
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_NestedStruct) {
    auto* f = b.ComputeFunction("main");

    auto* s0 = ty.Struct(mod.symbols.New("S0"), {
                                                    {mod.symbols.New("a"), ty.u32()},
                                                });
    auto* s1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("s0"), s0},
                                                });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, s1, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());

        auto* l1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 0_u);
        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u, 0_u);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u, 0_u);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u, 0_u);
        auto* v2 = b.Load(l4);
        b.Store(b_, v2);

        b.Return(f);
    });

    auto* src = R"(
S0 = struct @align(4) {
  a:u32 @offset(0)
}

S1 = struct @align(4) {
  s0:S0 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S1, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_StructMultipleAtomics) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.u32()},
                                                   {mod.symbols.New("b"), ty.u32()},
                                                   {mod.symbols.New("c"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* d_ = b.Var("d", ty.ptr<function, u32, read_write>());
        auto* e = b.Var("e", ty.ptr<function, u32, read_write>());
        auto* f_1 = b.Var("f", ty.ptr<function, u32, read_write>());

        auto* l1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 0_u);
        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l2, 1_u, 0_u, 0_u);
        auto* l3 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Store(l3, 0_u);

        auto* l4 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        auto* v1 = b.Load(l4);
        b.Let(v1);

        auto* l5 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        auto* v2 = b.Load(l5);
        b.Store(d_, v2);

        auto* l6 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Store(l6, 0_u);

        auto* l7 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_u);
        auto* v3 = b.Load(l7);
        b.Let(v3);

        auto* l8 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_u);
        auto* v4 = b.Load(l8);
        b.Store(e, v4);

        auto* l9 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_u);
        b.Store(l9, 0_u);

        auto* l10 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_u);
        auto* v5 = b.Load(l10);
        b.Let(v5);

        auto* l11 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_u);
        auto* v6 = b.Load(l11);
        b.Store(f_1, v6);

        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
  c:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %d:ptr<function, u32, read_write> = var undef
    %e:ptr<function, u32, read_write> = var undef
    %f:ptr<function, u32, read_write> = var undef
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u
    %7:u32 = spirv.atomic_i_add %6, 1u, 0u, 0u
    %8:ptr<workgroup, u32, read_write> = access %wg, 1u
    %9:u32 = spirv.atomic_i_add %8, 1u, 0u, 0u
    %10:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %10, 0u
    %11:ptr<workgroup, u32, read_write> = access %wg, 0u
    %12:u32 = load %11
    %13:u32 = let %12
    %14:ptr<workgroup, u32, read_write> = access %wg, 0u
    %15:u32 = load %14
    store %d, %15
    %16:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %16, 0u
    %17:ptr<workgroup, u32, read_write> = access %wg, 1u
    %18:u32 = load %17
    %19:u32 = let %18
    %20:ptr<workgroup, u32, read_write> = access %wg, 1u
    %21:u32 = load %20
    store %e, %21
    %22:ptr<workgroup, u32, read_write> = access %wg, 2u
    store %22, 0u
    %23:ptr<workgroup, u32, read_write> = access %wg, 2u
    %24:u32 = load %23
    %25:u32 = let %24
    %26:ptr<workgroup, u32, read_write> = access %wg, 2u
    %27:u32 = load %26
    store %f, %27
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_ArrayOfScalar) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr<workgroup, array<u32, 4>, read_write>()); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());

        auto* l1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 0_u);
        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        auto* v2 = b.Load(l4);
        b.Store(b_, v2);

        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 1i
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 1i
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 1i
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 1i
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_ArrayOfStruct) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr(workgroup, ty.array(sb, 4), read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());

        auto* l1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 0_u);
        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i, 0_u);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i, 0_u);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i, 0_u);
        auto* v2 = b.Load(l4);
        b.Store(b_, v2);

        b.Return(f);
    });
    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_StructOfArray) {
    auto* f = b.ComputeFunction("main");

    auto* sb =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.runtime_array(ty.u32())},
                                        });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] {
        wg = b.Var("sg", ty.ptr(storage, sb, read_write));
        wg->SetBindingPoint(0, 1);
    });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());

        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), wg, 0_u, 4_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 0_u);
        auto* l2 = b.Access(ty.ptr<storage, u32, read_write>(), wg, 0_u, 4_i);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<storage, u32, read_write>(), wg, 0_u, 4_i);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<storage, u32, read_write>(), wg, 0_u, 4_i);
        auto* v2 = b.Load(l4);
        b.Store(b_, v2);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<u32> @offset(0)
}

$B1: {  # root
  %sg:ptr<storage, S, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    store %6, 0u
    %7:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceAssignsAndDecls_Let) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("i"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] {
        wg = b.Var("s", ty.ptr(storage, sb, read_write));
        wg->SetBindingPoint(0, 1);
    });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());

        auto* p0 = b.Let(wg);
        auto* a = b.Access(ty.ptr<storage, u32, read_write>(), p0, 0_u);
        auto* p1 = b.Let(a);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, p1, 1_u, 0_u, 0_u);
        b.Store(p1, 0_u);

        auto* v1 = b.Load(p1);
        b.Let(v1);

        auto* v2 = b.Load(p1);
        b.Store(b_, v2);

        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  i:u32 @offset(0)
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<storage, S, read_write> = let %s
    %5:ptr<storage, u32, read_write> = access %4, 0u
    %6:ptr<storage, u32, read_write> = let %5
    %7:u32 = spirv.atomic_i_add %6, 1u, 0u, 0u
    store %6, 0u
    %8:u32 = load %6
    %9:u32 = let %8
    %10:u32 = load %6
    store %b, %10
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceBitcastArgument_Scalar) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, ty.u32(), read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, f32, read_write>());

        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, wg, 1_u, 0_u, 0_u);
        b.Store(wg, 0_u);

        auto* v1 = b.Load(wg);
        auto* bc = b.Bitcast(ty.f32(), v1);
        b.Store(b_, bc);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, f32, read_write> = var undef
    %4:u32 = spirv.atomic_i_add %wg, 1u, 0u, 0u
    store %wg, 0u
    %5:u32 = load %wg
    %6:f32 = bitcast %5
    store %b, %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, DISABLED_ReplaceBitcastArgument_Struct) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.u32()},
                                               });
    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, f32, read_write>());

        auto* a0 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, a0, 1_u, 0_u, 0_u);

        auto* a1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Store(a1, 0_u);

        auto* a2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        auto* v1 = b.Load(a2);
        auto* bc = b.Bitcast(ty.f32(), v1);
        b.Store(b_, bc);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, f32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0u
    %8:u32 = load %7
    %9:f32 = bitcast %8
    store %b, %9
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
UNIMPLEMENTED
)";
    ASSERT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
