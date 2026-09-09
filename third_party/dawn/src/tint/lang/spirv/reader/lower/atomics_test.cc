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

TEST_F(SpirvReader_AtomicsTest, ArrayStore_CopiedObject_AfterAtomicOp) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr<workgroup, array<u32, 4>, read_write>()); });

    b.Append(f->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 1_u, 0_u,
                                       2_u);

        auto* l = b.Let(wg);
        a = b.Access(ty.ptr<workgroup, u32, read_write>(), l, 1_i);
        b.Load(a);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 1i
    %4:void = spirv.atomic_store %3, 1u, 0u, 2u
    %5:ptr<workgroup, array<u32, 4>, read_write> = let %wg
    %6:ptr<workgroup, u32, read_write> = access %5, 1i
    %7:u32 = load %6
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
    %4:void = atomicStore %3, 2u
    %5:ptr<workgroup, array<atomic<u32>, 4>, read_write> = let %wg
    %6:ptr<workgroup, atomic<u32>, read_write> = access %5, 1i
    %7:u32 = atomicLoad %6
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

TEST_F(SpirvReader_AtomicsTest, FunctionParam) {
    auto* c = b.Function("c", ty.void_());
    auto* p = b.FunctionParam("param", ty.ptr(workgroup, ty.array<u32, 4>(), read_write));
    c->SetParams({p});

    b.Append(c->Block(), [&] {
        auto* a = b.Access(ty.ptr<workgroup, u32, read_write>(), p, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 2_u, 0_u,
                                       1_u);

        b.Return(c);
    });

    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr(workgroup, ty.array<u32, 4>(), read_write)); });

    b.Append(f->Block(), [&] {  //
        b.Call(ty.void_(), c, wg);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%c = func(%param:ptr<workgroup, array<u32, 4>, read_write>):void {
  $B2: {
    %4:ptr<workgroup, u32, read_write> = access %param, 1i
    %5:void = spirv.atomic_store %4, 2u, 0u, 1u
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:void = call %c, %wg
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

%c = func(%param:ptr<workgroup, array<atomic<u32>, 4>, read_write>):void {
  $B2: {
    %4:ptr<workgroup, atomic<u32>, read_write> = access %param, 1i
    %5:void = atomicStore %4, 1u
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:void = call %c, %wg
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

TEST_F(SpirvReader_AtomicsTest, AtomicExchange) {
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
                                       2_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicExchange, wg_i32, 1_u,
                                       0_u, 3_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicExchange, wg_u32, 1_u,
                                       0_u, 4_u);
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
    %8:u32 = spirv.atomic_exchange %7, 1u, 0u, 2u
    %9:i32 = spirv.atomic_exchange %wg_i32, 1u, 0u, 3i
    %10:u32 = spirv.atomic_exchange %wg_u32, 1u, 0u, 4u
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
    %6:i32 = atomicExchange %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicExchange %7, 2u
    %9:i32 = atomicExchange %wg_i32, 3i
    %10:u32 = atomicExchange %wg_u32, 4u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicCompareExchange) {
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
                                       0_u, 0_u, 2_i, 3_i);

        auto* a1 = b.Access(ty.ptr<storage, u32, read_write>(), sg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicCompareExchange, a1, 1_u,
                                       0_u, 0_u, 4_u, 5_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicCompareExchange, wg_i32,
                                       1_u, 0_u, 0_u, 6_i, 7_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicCompareExchange, wg_u32,
                                       1_u, 0_u, 0_u, 8_u, 9_u);
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
    %6:i32 = spirv.atomic_compare_exchange %5, 1u, 0u, 0u, 2i, 3i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_compare_exchange %7, 1u, 0u, 0u, 4u, 5u
    %9:i32 = spirv.atomic_compare_exchange %wg_i32, 1u, 0u, 0u, 6i, 7i
    %10:u32 = spirv.atomic_compare_exchange %wg_u32, 1u, 0u, 0u, 8u, 9u
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

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

__atomic_compare_exchange_result_u32 = struct @align(4) {
  old_value:u32 @offset(0)
  exchanged:bool @offset(4)
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
    %6:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %5, 3i, 2i
    %7:i32 = access %6, 0u
    %8:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %9:__atomic_compare_exchange_result_u32 = atomicCompareExchangeWeak %8, 5u, 4u
    %10:u32 = access %9, 0u
    %11:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %wg_i32, 7i, 6i
    %12:i32 = access %11, 0u
    %13:__atomic_compare_exchange_result_u32 = atomicCompareExchangeWeak %wg_u32, 9u, 8u
    %14:u32 = access %13, 0u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicLoad) {
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
    %6:i32 = atomicLoad %5
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:u32 = atomicLoad %7
    %9:i32 = atomicLoad %wg_i32
    %10:u32 = atomicLoad %wg_u32
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicStore) {
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
                                       2_u);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, wg_i32, 1_u, 0_u,
                                       3_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, wg_u32, 1_u, 0_u,
                                       4_u);
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
    %8:void = spirv.atomic_store %7, 1u, 0u, 2u
    %9:void = spirv.atomic_store %wg_i32, 1u, 0u, 3i
    %10:void = spirv.atomic_store %wg_u32, 1u, 0u, 4u
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
    %6:void = atomicStore %5, 1i
    %7:ptr<storage, atomic<u32>, read_write> = access %sb, 1u
    %8:void = atomicStore %7, 2u
    %9:void = atomicStore %wg_i32, 3i
    %10:void = atomicStore %wg_u32, 4u
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AtomicDecrement) {
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
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIDecrement, a1, 4_u, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIDecrement, wg_i32, 3_u,
                                       0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIDecrement, wg_u32, 2_u,
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
    %8:u32 = spirv.atomic_i_decrement %7, 4u, 0u
    %9:i32 = spirv.atomic_i_decrement %wg_i32, 3u, 0u
    %10:u32 = spirv.atomic_i_decrement %wg_u32, 2u, 0u
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

TEST_F(SpirvReader_AtomicsTest, AtomicIncrement) {
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
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIIncrement, a1, 4_u, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIIncrement, wg_i32, 3_u,
                                       0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIIncrement, wg_u32, 2_u,
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
    %8:u32 = spirv.atomic_i_increment %7, 4u, 0u
    %9:i32 = spirv.atomic_i_increment %wg_i32, 3u, 0u
    %10:u32 = spirv.atomic_i_increment %wg_u32, 2u, 0u
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

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_Scalar) {
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
$B1: {  # root
  %wg:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:u32 = atomicAdd %wg, 0u
    %5:void = atomicStore %wg, 0u
    %6:u32 = atomicLoad %wg
    %7:u32 = let %6
    %8:u32 = atomicLoad %wg
    store %b, %8
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_Struct) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.u32()},
                                               });

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());
        auto* l1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 4_u);

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
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 4u
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
S = struct @align(4) {
  a:u32 @offset(0)
}

S_atomic = struct @align(4) {
  a:atomic<u32> @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %5:u32 = atomicAdd %4, 4u
    %6:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %7:void = atomicStore %6, 0u
    %8:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %9:u32 = atomicLoad %8
    %10:u32 = let %9
    %11:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %12:u32 = atomicLoad %11
    store %b, %12
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_NestedStruct) {
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
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 4_u);
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
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 4u
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
S0 = struct @align(4) {
  a:u32 @offset(0)
}

S1 = struct @align(4) {
  s0:S0 @offset(0)
}

S0_atomic = struct @align(4) {
  a:atomic<u32> @offset(0)
}

S1_atomic = struct @align(4) {
  s0:S0_atomic @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S1_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u, 0u
    %5:u32 = atomicAdd %4, 4u
    %6:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u, 0u
    %7:void = atomicStore %6, 0u
    %8:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u, 0u
    %9:u32 = atomicLoad %8
    %10:u32 = let %9
    %11:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u, 0u
    %12:u32 = atomicLoad %11
    store %b, %12
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_StructMultipleAtomics) {
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
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 3_u);
        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l2, 1_u, 0_u, 4_u);
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
    %7:u32 = spirv.atomic_i_add %6, 1u, 0u, 3u
    %8:ptr<workgroup, u32, read_write> = access %wg, 1u
    %9:u32 = spirv.atomic_i_add %8, 1u, 0u, 4u
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
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
  c:u32 @offset(8)
}

S_atomic = struct @align(4) {
  a:atomic<u32> @offset(0)
  b:atomic<u32> @offset(4)
  c:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %d:ptr<function, u32, read_write> = var undef
    %e:ptr<function, u32, read_write> = var undef
    %f:ptr<function, u32, read_write> = var undef
    %6:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %7:u32 = atomicAdd %6, 3u
    %8:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u
    %9:u32 = atomicAdd %8, 4u
    %10:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %11:void = atomicStore %10, 0u
    %12:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %13:u32 = atomicLoad %12
    %14:u32 = let %13
    %15:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %16:u32 = atomicLoad %15
    store %d, %16
    %17:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %18:void = atomicStore %17, 0u
    %19:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u
    %20:u32 = atomicLoad %19
    %21:u32 = let %20
    %22:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1u
    %23:u32 = atomicLoad %22
    store %e, %23
    %24:ptr<workgroup, u32, read_write> = access %wg, 2u
    store %24, 0u
    %25:ptr<workgroup, u32, read_write> = access %wg, 2u
    %26:u32 = load %25
    %27:u32 = let %26
    %28:ptr<workgroup, u32, read_write> = access %wg, 2u
    %29:u32 = load %28
    store %f, %29
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_ArrayOfScalar) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block,
             [&] { wg = b.Var("wg", ty.ptr<workgroup, array<u32, 4>, read_write>()); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, u32, read_write>());

        auto* l1 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 4_u);
        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_i);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_i);
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
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 4u
    %6:ptr<workgroup, u32, read_write> = access %wg, 1i
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0i
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 2i
    %11:u32 = load %10
    store %b, %11
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
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1i
    %5:u32 = atomicAdd %4, 4u
    %6:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1i
    %7:void = atomicStore %6, 0u
    %8:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0i
    %9:u32 = atomicLoad %8
    %10:u32 = let %9
    %11:ptr<workgroup, atomic<u32>, read_write> = access %wg, 2i
    %12:u32 = atomicLoad %11
    store %b, %12
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_ArrayOfStruct) {
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
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 9_u);
        auto* l2 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 1_i, 0_u);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_i, 0_u);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 2_i, 0_u);
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
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 9u
    %6:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0i, 0u
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 2i, 0u
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

S_atomic = struct @align(4) {
  a:atomic<u32> @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S_atomic, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1i, 0u
    %5:u32 = atomicAdd %4, 9u
    %6:ptr<workgroup, atomic<u32>, read_write> = access %wg, 1i, 0u
    %7:void = atomicStore %6, 0u
    %8:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0i, 0u
    %9:u32 = atomicLoad %8
    %10:u32 = let %9
    %11:ptr<workgroup, atomic<u32>, read_write> = access %wg, 2i, 0u
    %12:u32 = atomicLoad %11
    store %b, %12
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_StructOfArray) {
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
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 3_u);
        auto* l2 = b.Access(ty.ptr<storage, u32, read_write>(), wg, 0_u, 2_i);
        b.Store(l2, 0_u);

        auto* l3 = b.Access(ty.ptr<storage, u32, read_write>(), wg, 0_u, 3_i);
        auto* v1 = b.Load(l3);
        b.Let(v1);

        auto* l4 = b.Access(ty.ptr<storage, u32, read_write>(), wg, 0_u, 1_i);
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
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 3u
    %6:ptr<storage, u32, read_write> = access %sg, 0u, 2i
    store %6, 0u
    %7:ptr<storage, u32, read_write> = access %sg, 0u, 3i
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<storage, u32, read_write> = access %sg, 0u, 1i
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:array<u32> @offset(0)
}

S_atomic = struct @align(4) {
  a:array<atomic<u32>> @offset(0)
}

$B1: {  # root
  %sg:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<storage, atomic<u32>, read_write> = access %sg, 0u, 4i
    %5:u32 = atomicAdd %4, 3u
    %6:ptr<storage, atomic<u32>, read_write> = access %sg, 0u, 2i
    %7:void = atomicStore %6, 0u
    %8:ptr<storage, atomic<u32>, read_write> = access %sg, 0u, 3i
    %9:u32 = atomicLoad %8
    %10:u32 = let %9
    %11:ptr<storage, atomic<u32>, read_write> = access %sg, 0u, 1i
    %12:u32 = atomicLoad %11
    store %b, %12
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceAssignsAndDecls_Let) {
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
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, p1, 1_u, 0_u, 8_u);
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
    %7:u32 = spirv.atomic_i_add %6, 1u, 0u, 8u
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
S = struct @align(4) {
  i:u32 @offset(0)
}

S_atomic = struct @align(4) {
  i:atomic<u32> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<storage, S_atomic, read_write> = let %s
    %5:ptr<storage, atomic<u32>, read_write> = access %4, 0u
    %6:ptr<storage, atomic<u32>, read_write> = let %5
    %7:u32 = atomicAdd %6, 8u
    %8:void = atomicStore %6, 0u
    %9:u32 = atomicLoad %6
    %10:u32 = let %9
    %11:u32 = atomicLoad %6
    store %b, %11
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceBitcastArgument_Scalar) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, ty.u32(), read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, f32, read_write>());

        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, wg, 1_u, 0_u, 3_u);
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
    %4:u32 = spirv.atomic_i_add %wg, 1u, 0u, 3u
    store %wg, 0u
    %5:u32 = load %wg
    %6:f32 = bitcast<f32> %5
    store %b, %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
$B1: {  # root
  %wg:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, f32, read_write> = var undef
    %4:u32 = atomicAdd %wg, 3u
    %5:void = atomicStore %wg, 0u
    %6:u32 = atomicLoad %wg
    %7:f32 = bitcast<f32> %6
    store %b, %7
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ReplaceBitcastArgument_Struct) {
    auto* f = b.ComputeFunction("main");

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("a"), ty.u32()},
                                               });
    core::ir::Var* wg = nullptr;
    b.Append(mod.root_block, [&] { wg = b.Var("wg", ty.ptr(workgroup, sb, read_write)); });

    b.Append(f->Block(), [&] {  //
        auto* b_ = b.Var("b", ty.ptr<function, f32, read_write>());

        auto* a0 = b.Access(ty.ptr<workgroup, u32, read_write>(), wg, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, a0, 1_u, 0_u, 2_u);

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
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 2u
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0u
    %8:u32 = load %7
    %9:f32 = bitcast<f32> %8
    store %b, %9
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

S_atomic = struct @align(4) {
  a:atomic<u32> @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S_atomic, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, f32, read_write> = var undef
    %4:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %5:u32 = atomicAdd %4, 2u
    %6:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %7:void = atomicStore %6, 0u
    %8:ptr<workgroup, atomic<u32>, read_write> = access %wg, 0u
    %9:u32 = atomicLoad %8
    %10:f32 = bitcast<f32> %9
    store %b, %10
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, FunctionParam_AnotherCallWithNonAtomicUse) {
    core::ir::Var* wg_atomic = nullptr;
    core::ir::Var* wg_nonatomic = nullptr;
    b.Append(mod.root_block, [&] {
        wg_atomic = b.Var("wg_atomic", ty.ptr<workgroup, u32>());
        wg_nonatomic = b.Var("wg_nonatomic", ty.ptr<workgroup, u32>());
    });

    auto* f_atomic = b.Function("f_atomic", ty.u32());
    b.Append(f_atomic->Block(), [&] {
        auto* p = b.FunctionParam("param", ty.ptr<workgroup, u32>());
        f_atomic->SetParams({p});

        auto* ret =
            b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicLoad, p, 1_u, 0_u);
        b.Return(f_atomic, ret);
    });

    auto* f_nonatomic = b.Function("f_nonatomic", ty.u32());
    b.Append(f_nonatomic->Block(), [&] {
        auto* p = b.FunctionParam("param", ty.ptr<workgroup, u32>());
        f_nonatomic->SetParams({p});

        auto* ret = b.Load(p);
        b.Return(f_nonatomic, ret);
    });

    auto* main = b.ComputeFunction("main");
    b.Append(main->Block(), [&] {  //
        b.Call(ty.u32(), f_atomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_nonatomic);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %wg_atomic:ptr<workgroup, u32, read_write> = var undef
  %wg_nonatomic:ptr<workgroup, u32, read_write> = var undef
}

%f_atomic = func(%param:ptr<workgroup, u32, read_write>):u32 {
  $B2: {
    %5:u32 = spirv.atomic_load %param, 1u, 0u
    ret %5
  }
}
%f_nonatomic = func(%param_1:ptr<workgroup, u32, read_write>):u32 {  # %param_1: 'param'
  $B3: {
    %8:u32 = load %param_1
    ret %8
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %10:u32 = call %f_atomic, %wg_atomic
    %11:u32 = call %f_nonatomic, %wg_atomic
    %12:u32 = call %f_nonatomic, %wg_atomic
    %13:u32 = call %f_nonatomic, %wg_nonatomic
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
$B1: {  # root
  %wg_atomic:ptr<workgroup, atomic<u32>, read_write> = var undef
  %wg_nonatomic:ptr<workgroup, u32, read_write> = var undef
}

%f_atomic = func(%param:ptr<workgroup, atomic<u32>, read_write>):u32 {
  $B2: {
    %5:u32 = atomicLoad %param
    ret %5
  }
}
%f_nonatomic = func(%param_1:ptr<workgroup, u32, read_write>):u32 {  # %param_1: 'param'
  $B3: {
    %8:u32 = load %param_1
    ret %8
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %10:u32 = call %f_atomic, %wg_atomic
    %11:u32 = call %f_nonatomic_1, %wg_atomic
    %13:u32 = call %f_nonatomic_1, %wg_atomic
    %14:u32 = call %f_nonatomic, %wg_nonatomic
    ret
  }
}
%f_nonatomic_1 = func(%param_2:ptr<workgroup, atomic<u32>, read_write>):u32 {  # %f_nonatomic_1: 'f_nonatomic', %param_2: 'param'
  $B5: {
    %16:u32 = atomicLoad %param_2
    ret %16
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, FunctionParam_MixedCalls) {
    core::ir::Var* wg_atomic = nullptr;
    core::ir::Var* wg_nonatomic = nullptr;
    b.Append(mod.root_block, [&] {
        wg_atomic = b.Var("wg_atomic", ty.ptr<workgroup, u32>());
        wg_nonatomic = b.Var("wg_nonatomic", ty.ptr<workgroup, u32>());
    });

    auto* f_atomic = b.Function("f_atomic", ty.u32());
    b.Append(f_atomic->Block(), [&] {
        auto* p = b.FunctionParam("param", ty.ptr<workgroup, u32>());
        f_atomic->SetParams({p});

        auto* ret =
            b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicLoad, p, 1_u, 0_u);
        b.Return(f_atomic, ret);
    });

    auto* f_nonatomic = b.Function("f_nonatomic", ty.u32());
    b.Append(f_nonatomic->Block(), [&] {
        auto* p1 = b.FunctionParam("param1", ty.ptr<workgroup, u32>());
        auto* p2 = b.FunctionParam("param2", ty.ptr<workgroup, u32>());
        f_nonatomic->SetParams({p1, p2});

        auto* one = b.Load(p1);
        auto* two = b.Load(p2);
        b.Return(f_nonatomic, b.Add(one, two));
    });

    auto* main = b.ComputeFunction("main");
    b.Append(main->Block(), [&] {  //
        b.Call(ty.u32(), f_atomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_atomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_atomic, wg_nonatomic);
        b.Call(ty.u32(), f_nonatomic, wg_nonatomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_nonatomic, wg_nonatomic);

        // Duplicate the calls to make sure the functions don't duplicate
        b.Call(ty.u32(), f_nonatomic, wg_atomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_atomic, wg_nonatomic);
        b.Call(ty.u32(), f_nonatomic, wg_nonatomic, wg_atomic);
        b.Call(ty.u32(), f_nonatomic, wg_nonatomic, wg_nonatomic);
        b.Return(main);
    });

    auto* src = R"(
$B1: {  # root
  %wg_atomic:ptr<workgroup, u32, read_write> = var undef
  %wg_nonatomic:ptr<workgroup, u32, read_write> = var undef
}

%f_atomic = func(%param:ptr<workgroup, u32, read_write>):u32 {
  $B2: {
    %5:u32 = spirv.atomic_load %param, 1u, 0u
    ret %5
  }
}
%f_nonatomic = func(%param1:ptr<workgroup, u32, read_write>, %param2:ptr<workgroup, u32, read_write>):u32 {
  $B3: {
    %9:u32 = load %param1
    %10:u32 = load %param2
    %11:u32 = add %9, %10
    ret %11
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %13:u32 = call %f_atomic, %wg_atomic
    %14:u32 = call %f_nonatomic, %wg_atomic, %wg_atomic
    %15:u32 = call %f_nonatomic, %wg_atomic, %wg_nonatomic
    %16:u32 = call %f_nonatomic, %wg_nonatomic, %wg_atomic
    %17:u32 = call %f_nonatomic, %wg_nonatomic, %wg_nonatomic
    %18:u32 = call %f_nonatomic, %wg_atomic, %wg_atomic
    %19:u32 = call %f_nonatomic, %wg_atomic, %wg_nonatomic
    %20:u32 = call %f_nonatomic, %wg_nonatomic, %wg_atomic
    %21:u32 = call %f_nonatomic, %wg_nonatomic, %wg_nonatomic
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
$B1: {  # root
  %wg_atomic:ptr<workgroup, atomic<u32>, read_write> = var undef
  %wg_nonatomic:ptr<workgroup, u32, read_write> = var undef
}

%f_atomic = func(%param:ptr<workgroup, atomic<u32>, read_write>):u32 {
  $B2: {
    %5:u32 = atomicLoad %param
    ret %5
  }
}
%f_nonatomic = func(%param1:ptr<workgroup, u32, read_write>, %param2:ptr<workgroup, u32, read_write>):u32 {
  $B3: {
    %9:u32 = load %param1
    %10:u32 = load %param2
    %11:u32 = add %9, %10
    ret %11
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B4: {
    %13:u32 = call %f_atomic, %wg_atomic
    %14:u32 = call %f_nonatomic_1, %wg_atomic, %wg_atomic
    %16:u32 = call %f_nonatomic_2, %wg_atomic, %wg_nonatomic
    %18:u32 = call %f_nonatomic_3, %wg_nonatomic, %wg_atomic
    %20:u32 = call %f_nonatomic, %wg_nonatomic, %wg_nonatomic
    %21:u32 = call %f_nonatomic_1, %wg_atomic, %wg_atomic
    %22:u32 = call %f_nonatomic_2, %wg_atomic, %wg_nonatomic
    %23:u32 = call %f_nonatomic_3, %wg_nonatomic, %wg_atomic
    %24:u32 = call %f_nonatomic, %wg_nonatomic, %wg_nonatomic
    ret
  }
}
%f_nonatomic_1 = func(%param1_1:ptr<workgroup, atomic<u32>, read_write>, %param2_1:ptr<workgroup, atomic<u32>, read_write>):u32 {  # %f_nonatomic_1: 'f_nonatomic', %param1_1: 'param1', %param2_1: 'param2'
  $B5: {
    %27:u32 = atomicLoad %param1_1
    %28:u32 = atomicLoad %param2_1
    %29:u32 = add %27, %28
    ret %29
  }
}
%f_nonatomic_2 = func(%param1_2:ptr<workgroup, atomic<u32>, read_write>, %param2_2:ptr<workgroup, u32, read_write>):u32 {  # %f_nonatomic_2: 'f_nonatomic', %param1_2: 'param1', %param2_2: 'param2'
  $B6: {
    %32:u32 = atomicLoad %param1_2
    %33:u32 = load %param2_2
    %34:u32 = add %32, %33
    ret %34
  }
}
%f_nonatomic_3 = func(%param1_3:ptr<workgroup, u32, read_write>, %param2_3:ptr<workgroup, atomic<u32>, read_write>):u32 {  # %f_nonatomic_3: 'f_nonatomic', %param1_3: 'param1', %param2_3: 'param2'
  $B7: {
    %37:u32 = load %param1_3
    %38:u32 = atomicLoad %param2_3
    %39:u32 = add %37, %38
    ret %39
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, ArrayLength) {
    auto* f = b.ComputeFunction("main");

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block,
             [&] {  //
                 buffer = b.Var("buffer", ty.ptr<storage, array<u32>, read_write>());
                 buffer->SetBindingPoint(0u, 0u);
             });

    b.Append(f->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kAtomicStore, a, 1_u, 0_u,
                                       1_u);
        b.Call<core::ir::CoreBuiltinCall>(ty.u32(), core::BuiltinFn::kArrayLength, buffer);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i
    %4:void = spirv.atomic_store %3, 1u, 0u, 1u
    %5:u32 = arrayLength %buffer
    ret
  }
}
)";
    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<atomic<u32>>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i
    %4:void = atomicStore %3, 1u
    %5:u32 = arrayLength %buffer
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, RewriteLoad_SameAccessChain) {
    auto* f = b.ComputeFunction("main");
    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block,
             [&] {  //
                 buffer = b.Var("buffer", ty.ptr<storage, array<u32>, read_write>());
                 buffer->SetBindingPoint(0u, 0u);
             });

    b.Append(f->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i);
        b.Let("x", b.Load(a));

        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicCompareExchange, a,
                                           1_u, 0_u, 0_u, 0_u, 0_u);
            b.ExitIf(if_);
        });

        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i
    %4:u32 = load %3
    %x:u32 = let %4
    if true [t: $B3] {  # if_1
      $B3: {  # true
        %6:u32 = spirv.atomic_compare_exchange %3, 1u, 0u, 0u, 0u, 0u
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    ASSERT_EQ(src, str());
    Run(Atomics);

    auto* expect = R"(
__atomic_compare_exchange_result_u32 = struct @align(4) {
  old_value:u32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, array<atomic<u32>>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i
    %4:u32 = atomicLoad %3
    %x:u32 = let %4
    if true [t: $B3] {  # if_1
      $B3: {  # true
        %6:__atomic_compare_exchange_result_u32 = atomicCompareExchangeWeak %3, 0u, 0u
        %7:u32 = access %6, 0u
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, StoreStructContainingAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("member_a"), ty.f32()},
                                                   {mod.symbols.New("member_b"), ty.u32()},
                                               });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, sb, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i);
        auto* atomic_add = b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd,
                                                          l1, 1_u, 0_u, 1_u);
        auto* construct = b.Construct(sb, 1.0_f, atomic_add);
        b.Store(buffer, construct);
        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:S = construct 1.0f, %4
    store %buffer, %5
    ret
  }
}
)";

    auto* expect = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i
    %4:u32 = atomicAdd %3, 1u
    %5:S = construct 1.0f, %4
    %6:ptr<storage, f32, read_write> = access %buffer, 0u
    %7:f32 = access %5, 0u
    store %6, %7
    %8:ptr<storage, atomic<u32>, read_write> = access %buffer, 1u
    %9:u32 = access %5, 1u
    %10:void = atomicStore %8, %9
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, LoadStructContainingAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("member_a"), ty.f32()},
                                                   {mod.symbols.New("member_b"), ty.u32()},
                                               });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, sb, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        auto* load = b.Load(buffer);
        auto* ext_a = b.Access(ty.f32(), load, 0_u);
        auto* ext_b = b.Access(ty.u32(), load, 1_u);

        auto* local_a = b.Var("local_a", ty.ptr<function, f32, read_write>());
        auto* local_b = b.Var("local_b", ty.ptr<function, u32, read_write>());
        b.Store(local_a, ext_a);
        b.Store(local_b, ext_b);

        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:S = load %buffer
    %6:f32 = access %5, 0u
    %7:u32 = access %5, 1u
    %local_a:ptr<function, f32, read_write> = var undef
    %local_b:ptr<function, u32, read_write> = var undef
    store %local_a, %6
    store %local_b, %7
    ret
  }
}
)";

    auto* expect = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, S_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i
    %4:u32 = atomicAdd %3, 1u
    %5:ptr<storage, f32, read_write> = access %buffer, 0u
    %6:f32 = load %5
    %7:ptr<storage, atomic<u32>, read_write> = access %buffer, 1u
    %8:u32 = atomicLoad %7
    %9:S = construct %6, %8
    %10:f32 = access %9, 0u
    %11:u32 = access %9, 1u
    %local_a:ptr<function, f32, read_write> = var undef
    %local_b:ptr<function, u32, read_write> = var undef
    store %local_a, %10
    store %local_b, %11
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, LoadArrayOfAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* arr_ty = ty.array(ty.u32(), 4_u);

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, arr_ty, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        auto* load = b.Load(buffer);
        auto* ext = b.Access(ty.u32(), load, 0_u);

        auto* local = b.Var("local", ty.ptr<function, u32, read_write>());
        b.Store(local, ext);

        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:array<u32, 4> = load %buffer
    %6:u32 = access %5, 0u
    %local:ptr<function, u32, read_write> = var undef
    store %local, %6
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<atomic<u32>, 4>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i
    %4:u32 = atomicAdd %3, 1u
    %5:ptr<storage, atomic<u32>, read_write> = access %buffer, 0u
    %6:u32 = atomicLoad %5
    %7:ptr<storage, atomic<u32>, read_write> = access %buffer, 1u
    %8:u32 = atomicLoad %7
    %9:ptr<storage, atomic<u32>, read_write> = access %buffer, 2u
    %10:u32 = atomicLoad %9
    %11:ptr<storage, atomic<u32>, read_write> = access %buffer, 3u
    %12:u32 = atomicLoad %11
    %13:array<u32, 4> = construct %6, %8, %10, %12
    %14:u32 = access %13, 0u
    %local:ptr<function, u32, read_write> = var undef
    store %local, %14
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, StoreArrayOfAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* arr_ty = ty.array(ty.u32(), 4_u);

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, arr_ty, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        auto* construct = b.Construct(arr_ty, 0_u, 1_u, 2_u, 3_u);
        b.Store(buffer, construct);

        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:array<u32, 4> = construct 0u, 1u, 2u, 3u
    store %buffer, %5
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<atomic<u32>, 4>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i
    %4:u32 = atomicAdd %3, 1u
    %5:array<u32, 4> = construct 0u, 1u, 2u, 3u
    %6:ptr<storage, atomic<u32>, read_write> = access %buffer, 0u
    %7:u32 = access %5, 0u
    %8:void = atomicStore %6, %7
    %9:ptr<storage, atomic<u32>, read_write> = access %buffer, 1u
    %10:u32 = access %5, 1u
    %11:void = atomicStore %9, %10
    %12:ptr<storage, atomic<u32>, read_write> = access %buffer, 2u
    %13:u32 = access %5, 2u
    %14:void = atomicStore %12, %13
    %15:ptr<storage, atomic<u32>, read_write> = access %buffer, 3u
    %16:u32 = access %5, 3u
    %17:void = atomicStore %15, %16
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, LoadArrayOfStructContainingAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("member_a"), ty.f32()},
                                                   {mod.symbols.New("member_b"), ty.u32()},
                                               });
    auto* arr_ty = ty.array(sb, 4_u);

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, arr_ty, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        // Load the whole array
        auto* load = b.Load(buffer);
        // Extract member_a of element 2 (non-atomic)
        auto* ext_a = b.Access(ty.f32(), load, 2_u, 0_u);
        // Extract member_b of element 2 (atomic)
        auto* ext_b = b.Access(ty.u32(), load, 2_u, 1_u);

        auto* local_a = b.Var("local_a", ty.ptr<function, f32, read_write>());
        auto* local_b = b.Var("local_b", ty.ptr<function, u32, read_write>());
        b.Store(local_a, ext_a);
        b.Store(local_b, ext_b);

        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, array<S, 4>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i, 1u
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:array<S, 4> = load %buffer
    %6:f32 = access %5, 2u, 0u
    %7:u32 = access %5, 2u, 1u
    %local_a:ptr<function, f32, read_write> = var undef
    %local_b:ptr<function, u32, read_write> = var undef
    store %local_a, %6
    store %local_b, %7
    ret
  }
}
)";

    auto* expect = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, array<S_atomic, 4>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i, 1u
    %4:u32 = atomicAdd %3, 1u
    %5:ptr<storage, S_atomic, read_write> = access %buffer, 0u
    %6:ptr<storage, f32, read_write> = access %5, 0u
    %7:f32 = load %6
    %8:ptr<storage, atomic<u32>, read_write> = access %5, 1u
    %9:u32 = atomicLoad %8
    %10:S = construct %7, %9
    %11:ptr<storage, S_atomic, read_write> = access %buffer, 1u
    %12:ptr<storage, f32, read_write> = access %11, 0u
    %13:f32 = load %12
    %14:ptr<storage, atomic<u32>, read_write> = access %11, 1u
    %15:u32 = atomicLoad %14
    %16:S = construct %13, %15
    %17:ptr<storage, S_atomic, read_write> = access %buffer, 2u
    %18:ptr<storage, f32, read_write> = access %17, 0u
    %19:f32 = load %18
    %20:ptr<storage, atomic<u32>, read_write> = access %17, 1u
    %21:u32 = atomicLoad %20
    %22:S = construct %19, %21
    %23:ptr<storage, S_atomic, read_write> = access %buffer, 3u
    %24:ptr<storage, f32, read_write> = access %23, 0u
    %25:f32 = load %24
    %26:ptr<storage, atomic<u32>, read_write> = access %23, 1u
    %27:u32 = atomicLoad %26
    %28:S = construct %25, %27
    %29:array<S, 4> = construct %10, %16, %22, %28
    %30:f32 = access %29, 2u, 0u
    %31:u32 = access %29, 2u, 1u
    %local_a:ptr<function, f32, read_write> = var undef
    %local_b:ptr<function, u32, read_write> = var undef
    store %local_a, %30
    store %local_b, %31
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, LoadNestedStructContainingAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("member_a"), ty.f32()},
                                                          {mod.symbols.New("member_b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("member_a"), ty.i32()},
                                                          {mod.symbols.New("member_b"), inner},
                                                      });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        // Load the whole outer struct
        auto* load = b.Load(buffer);
        // Extract non-atomic member_a from outer
        auto* ext_a = b.Access(ty.i32(), load, 0_u);
        // Extract non-atomic member_a from inner
        auto* ext_ba = b.Access(ty.f32(), load, 1_u, 0_u);
        // Extract atomic member_b from inner
        auto* ext_bb = b.Access(ty.u32(), load, 1_u, 1_u);

        auto* local_a = b.Var("local_a", ty.ptr<function, i32, read_write>());
        auto* local_ba = b.Var("local_ba", ty.ptr<function, f32, read_write>());
        auto* local_bb = b.Var("local_bb", ty.ptr<function, u32, read_write>());
        b.Store(local_a, ext_a);
        b.Store(local_ba, ext_ba);
        b.Store(local_bb, ext_bb);

        b.Return(f);
    });

    auto* src = R"(
Inner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

Outer = struct @align(4) {
  member_a_1:i32 @offset(0)
  member_b_1:Inner @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1u, 1u
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:Outer = load %buffer
    %6:i32 = access %5, 0u
    %7:f32 = access %5, 1u, 0u
    %8:u32 = access %5, 1u, 1u
    %local_a:ptr<function, i32, read_write> = var undef
    %local_ba:ptr<function, f32, read_write> = var undef
    %local_bb:ptr<function, u32, read_write> = var undef
    store %local_a, %6
    store %local_ba, %7
    store %local_bb, %8
    ret
  }
}
)";

    auto* expect = R"(
Inner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

Outer = struct @align(4) {
  member_a_1:i32 @offset(0)
  member_b_1:Inner @offset(4)
}

Inner_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

Outer_atomic = struct @align(4) {
  member_a_1:i32 @offset(0)
  member_b_1:Inner_atomic @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1u, 1u
    %4:u32 = atomicAdd %3, 1u
    %5:ptr<storage, i32, read_write> = access %buffer, 0u
    %6:i32 = load %5
    %7:ptr<storage, Inner_atomic, read_write> = access %buffer, 1u
    %8:ptr<storage, f32, read_write> = access %7, 0u
    %9:f32 = load %8
    %10:ptr<storage, atomic<u32>, read_write> = access %7, 1u
    %11:u32 = atomicLoad %10
    %12:Inner = construct %9, %11
    %13:Outer = construct %6, %12
    %14:i32 = access %13, 0u
    %15:f32 = access %13, 1u, 0u
    %16:u32 = access %13, 1u, 1u
    %local_a:ptr<function, i32, read_write> = var undef
    %local_ba:ptr<function, f32, read_write> = var undef
    %local_bb:ptr<function, u32, read_write> = var undef
    store %local_a, %14
    store %local_ba, %15
    store %local_bb, %16
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, LoadStructContainingAtomicAndNonAtomicStruct) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* atomic_inner =
        ty.Struct(mod.symbols.New("AtomicInner"), {
                                                      {mod.symbols.New("member_a"), ty.f32()},
                                                      {mod.symbols.New("member_b"), ty.u32()},
                                                  });
    auto* non_atomic_inner =
        ty.Struct(mod.symbols.New("NonAtomicInner"), {
                                                         {mod.symbols.New("member_x"), ty.f32()},
                                                         {mod.symbols.New("member_y"), ty.u32()},
                                                     });
    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("first"), atomic_inner},
                                                {mod.symbols.New("second"), non_atomic_inner},
                                            });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        // Load the whole outer struct
        auto* load = b.Load(buffer);
        // Extract non-atomic member_a from first
        auto* ext_a = b.Access(ty.f32(), load, 0_u, 0_u);
        // Extract atomic member_b from first
        auto* ext_b = b.Access(ty.u32(), load, 0_u, 1_u);
        // Extract second struct (non-atomic struct value)
        auto* ext_second = b.Access(non_atomic_inner, load, 1_u);

        auto* local_a = b.Var("local_a", ty.ptr<function, f32, read_write>());
        auto* local_b = b.Var("local_b", ty.ptr<function, u32, read_write>());
        auto* local_second = b.Var("local_second", ty.ptr(function, non_atomic_inner, read_write));
        b.Store(local_a, ext_a);
        b.Store(local_b, ext_b);
        b.Store(local_second, ext_second);

        b.Return(f);
    });

    auto* src = R"(
AtomicInner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

NonAtomicInner = struct @align(4) {
  member_x:f32 @offset(0)
  member_y:u32 @offset(4)
}

Outer = struct @align(4) {
  first:AtomicInner @offset(0)
  second:NonAtomicInner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 0u, 1u
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:Outer = load %buffer
    %6:f32 = access %5, 0u, 0u
    %7:u32 = access %5, 0u, 1u
    %8:NonAtomicInner = access %5, 1u
    %local_a:ptr<function, f32, read_write> = var undef
    %local_b:ptr<function, u32, read_write> = var undef
    %local_second:ptr<function, NonAtomicInner, read_write> = var undef
    store %local_a, %6
    store %local_b, %7
    store %local_second, %8
    ret
  }
}
)";

    auto* expect = R"(
AtomicInner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

NonAtomicInner = struct @align(4) {
  member_x:f32 @offset(0)
  member_y:u32 @offset(4)
}

Outer = struct @align(4) {
  first:AtomicInner @offset(0)
  second:NonAtomicInner @offset(8)
}

AtomicInner_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

Outer_atomic = struct @align(4) {
  first:AtomicInner_atomic @offset(0)
  second:NonAtomicInner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 0u, 1u
    %4:u32 = atomicAdd %3, 1u
    %5:ptr<storage, AtomicInner_atomic, read_write> = access %buffer, 0u
    %6:ptr<storage, f32, read_write> = access %5, 0u
    %7:f32 = load %6
    %8:ptr<storage, atomic<u32>, read_write> = access %5, 1u
    %9:u32 = atomicLoad %8
    %10:AtomicInner = construct %7, %9
    %11:ptr<storage, NonAtomicInner, read_write> = access %buffer, 1u
    %12:NonAtomicInner = load %11
    %13:Outer = construct %10, %12
    %14:f32 = access %13, 0u, 0u
    %15:u32 = access %13, 0u, 1u
    %16:NonAtomicInner = access %13, 1u
    %local_a:ptr<function, f32, read_write> = var undef
    %local_b:ptr<function, u32, read_write> = var undef
    %local_second:ptr<function, NonAtomicInner, read_write> = var undef
    store %local_a, %14
    store %local_b, %15
    store %local_second, %16
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, StoreArrayOfStructContainingAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* sb = ty.Struct(mod.symbols.New("S"), {
                                                   {mod.symbols.New("member_a"), ty.f32()},
                                                   {mod.symbols.New("member_b"), ty.u32()},
                                               });
    auto* arr_ty = ty.array(sb, 2_u);

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, arr_ty, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_i, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        auto* s0 = b.Construct(sb, 1.0_f, 2_u);
        auto* s1 = b.Construct(sb, 3.0_f, 4_u);
        auto* construct = b.Construct(arr_ty, s0, s1);
        b.Store(buffer, construct);

        b.Return(f);
    });

    auto* src = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, array<S, 2>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1i, 1u
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:S = construct 1.0f, 2u
    %6:S = construct 3.0f, 4u
    %7:array<S, 2> = construct %5, %6
    store %buffer, %7
    ret
  }
}
)";

    auto* expect = R"(
S = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

S_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, array<S_atomic, 2>, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1i, 1u
    %4:u32 = atomicAdd %3, 1u
    %5:S = construct 1.0f, 2u
    %6:S = construct 3.0f, 4u
    %7:array<S, 2> = construct %5, %6
    %8:ptr<storage, S_atomic, read_write> = access %buffer, 0u
    %9:S = access %7, 0u
    %10:ptr<storage, f32, read_write> = access %8, 0u
    %11:f32 = access %9, 0u
    store %10, %11
    %12:ptr<storage, atomic<u32>, read_write> = access %8, 1u
    %13:u32 = access %9, 1u
    %14:void = atomicStore %12, %13
    %15:ptr<storage, S_atomic, read_write> = access %buffer, 1u
    %16:S = access %7, 1u
    %17:ptr<storage, f32, read_write> = access %15, 0u
    %18:f32 = access %16, 0u
    store %17, %18
    %19:ptr<storage, atomic<u32>, read_write> = access %15, 1u
    %20:u32 = access %16, 1u
    %21:void = atomicStore %19, %20
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, StoreNestedStructContainingAtomic) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("member_a"), ty.f32()},
                                                          {mod.symbols.New("member_b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("member_a"), ty.i32()},
                                                          {mod.symbols.New("member_b"), inner},
                                                      });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        auto* inner_val = b.Construct(inner, 1.0_f, 2_u);
        auto* outer_val = b.Construct(outer, 1_i, inner_val);
        b.Store(buffer, outer_val);

        b.Return(f);
    });

    auto* src = R"(
Inner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

Outer = struct @align(4) {
  member_a_1:i32 @offset(0)
  member_b_1:Inner @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 1u, 1u
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:Inner = construct 1.0f, 2u
    %6:Outer = construct 1i, %5
    store %buffer, %6
    ret
  }
}
)";

    auto* expect = R"(
Inner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

Outer = struct @align(4) {
  member_a_1:i32 @offset(0)
  member_b_1:Inner @offset(4)
}

Inner_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

Outer_atomic = struct @align(4) {
  member_a_1:i32 @offset(0)
  member_b_1:Inner_atomic @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 1u, 1u
    %4:u32 = atomicAdd %3, 1u
    %5:Inner = construct 1.0f, 2u
    %6:Outer = construct 1i, %5
    %7:ptr<storage, i32, read_write> = access %buffer, 0u
    %8:i32 = access %6, 0u
    store %7, %8
    %9:ptr<storage, Inner_atomic, read_write> = access %buffer, 1u
    %10:Inner = access %6, 1u
    %11:ptr<storage, f32, read_write> = access %9, 0u
    %12:f32 = access %10, 0u
    store %11, %12
    %13:ptr<storage, atomic<u32>, read_write> = access %9, 1u
    %14:u32 = access %10, 1u
    %15:void = atomicStore %13, %14
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, StoreStructContainingAtomicAndNonAtomicStruct) {
    auto* f = b.ComputeFunction("main", 64_u, 1_u, 1_u);

    auto* atomic_inner =
        ty.Struct(mod.symbols.New("AtomicInner"), {
                                                      {mod.symbols.New("member_a"), ty.f32()},
                                                      {mod.symbols.New("member_b"), ty.u32()},
                                                  });
    auto* non_atomic_inner =
        ty.Struct(mod.symbols.New("NonAtomicInner"), {
                                                         {mod.symbols.New("member_x"), ty.f32()},
                                                         {mod.symbols.New("member_y"), ty.u32()},
                                                     });
    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("first"), atomic_inner},
                                                {mod.symbols.New("second"), non_atomic_inner},
                                            });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* l1 = b.Access(ty.ptr<storage, u32, read_write>(), buffer, 0_u, 1_u);
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kAtomicIAdd, l1, 1_u, 0_u, 1_u);

        auto* first_val = b.Construct(atomic_inner, 1.0_f, 2_u);
        auto* second_val = b.Construct(non_atomic_inner, 3.0_f, 4_u);
        auto* outer_val = b.Construct(outer, first_val, second_val);
        b.Store(buffer, outer_val);

        b.Return(f);
    });

    auto* src = R"(
AtomicInner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

NonAtomicInner = struct @align(4) {
  member_x:f32 @offset(0)
  member_y:u32 @offset(4)
}

Outer = struct @align(4) {
  first:AtomicInner @offset(0)
  second:NonAtomicInner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %buffer, 0u, 1u
    %4:u32 = spirv.atomic_i_add %3, 1u, 0u, 1u
    %5:AtomicInner = construct 1.0f, 2u
    %6:NonAtomicInner = construct 3.0f, 4u
    %7:Outer = construct %5, %6
    store %buffer, %7
    ret
  }
}
)";

    auto* expect = R"(
AtomicInner = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:u32 @offset(4)
}

NonAtomicInner = struct @align(4) {
  member_x:f32 @offset(0)
  member_y:u32 @offset(4)
}

Outer = struct @align(4) {
  first:AtomicInner @offset(0)
  second:NonAtomicInner @offset(8)
}

AtomicInner_atomic = struct @align(4) {
  member_a:f32 @offset(0)
  member_b:atomic<u32> @offset(4)
}

Outer_atomic = struct @align(4) {
  first:AtomicInner_atomic @offset(0)
  second:NonAtomicInner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(64u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<u32>, read_write> = access %buffer, 0u, 1u
    %4:u32 = atomicAdd %3, 1u
    %5:AtomicInner = construct 1.0f, 2u
    %6:NonAtomicInner = construct 3.0f, 4u
    %7:Outer = construct %5, %6
    %8:ptr<storage, AtomicInner_atomic, read_write> = access %buffer, 0u
    %9:AtomicInner = access %7, 0u
    %10:ptr<storage, f32, read_write> = access %8, 0u
    %11:f32 = access %9, 0u
    store %10, %11
    %12:ptr<storage, atomic<u32>, read_write> = access %8, 1u
    %13:u32 = access %9, 1u
    %14:void = atomicStore %12, %13
    %15:ptr<storage, NonAtomicInner, read_write> = access %buffer, 1u
    %16:NonAtomicInner = access %7, 1u
    store %15, %16
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AccessNonAtomicMatrixInAtomicStruct) {
    auto* f = b.ComputeFunction("main");

    auto* inner = ty.Struct(mod.symbols.Register("Inner"),
                            {
                                {mod.symbols.Register("xform"), ty.mat4x4<f32>()},
                            });

    auto* outer = ty.Struct(mod.symbols.Register("Outer"),
                            {
                                {mod.symbols.Register("particle_count"), ty.i32()},
                                {mod.symbols.Register("data"), ty.array(inner, 4_u)},
                            });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* cnt_ptr = b.Access(ty.ptr<storage, i32, read_write>(), buffer, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIAdd, cnt_ptr, 1_u, 0_u,
                                       1_i);

        auto* mat_ptr =
            b.Access(ty.ptr<storage, vec4<f32>, read_write>(), buffer, 1_u, 0_u, 0_u, 3_u);
        b.Load(mat_ptr);

        b.Return(f);
    });

    auto* src = R"(
Inner = struct @align(16) {
  xform:mat4x4<f32> @offset(0)
}

Outer = struct @align(16) {
  particle_count:i32 @offset(0)
  data:array<Inner, 4> @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, i32, read_write> = access %buffer, 0u
    %4:i32 = spirv.atomic_i_add %3, 1u, 0u, 1i
    %5:ptr<storage, vec4<f32>, read_write> = access %buffer, 1u, 0u, 0u, 3u
    %6:vec4<f32> = load %5
    ret
  }
}
)";

    auto* expect = R"(
Inner = struct @align(16) {
  xform:mat4x4<f32> @offset(0)
}

Outer = struct @align(16) {
  particle_count:i32 @offset(0)
  data:array<Inner, 4> @offset(16)
}

Outer_atomic = struct @align(16) {
  particle_count:atomic<i32> @offset(0)
  data:array<Inner, 4> @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<i32>, read_write> = access %buffer, 0u
    %4:i32 = atomicAdd %3, 1i
    %5:ptr<storage, vec4<f32>, read_write> = access %buffer, 1u, 0u, 0u, 3u
    %6:vec4<f32> = load %5
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

TEST_F(SpirvReader_AtomicsTest, AccessNonAtomicVectorInAtomicStruct) {
    mod.properties.Add(core::ir::Property::kAllowVectorElementPointer);

    auto* f = b.ComputeFunction("main");

    auto* inner = ty.Struct(mod.symbols.Register("Inner"),
                            {
                                {mod.symbols.Register("vector"), ty.vec4<f32>()},
                            });

    auto* outer = ty.Struct(mod.symbols.Register("Outer"),
                            {
                                {mod.symbols.Register("particle_count"), ty.i32()},
                                {mod.symbols.Register("data"), ty.array(inner, 4_u)},
                            });

    core::ir::Var* buffer = nullptr;
    b.Append(mod.root_block, [&] {
        buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
        buffer->SetBindingPoint(0, 0);
    });

    b.Append(f->Block(), [&] {
        auto* cnt_ptr = b.Access(ty.ptr<storage, i32, read_write>(), buffer, 0_u);
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kAtomicIAdd, cnt_ptr, 1_u, 0_u,
                                       1_i);

        auto* vec_ptr = b.Access(ty.ptr<storage, f32, read_write>(), buffer, 1_u, 0_u, 0_u, 3_u);
        b.Load(vec_ptr);

        b.Return(f);
    });

    auto* src = R"(
Inner = struct @align(16) {
  vector:vec4<f32> @offset(0)
}

Outer = struct @align(16) {
  particle_count:i32 @offset(0)
  data:array<Inner, 4> @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, i32, read_write> = access %buffer, 0u
    %4:i32 = spirv.atomic_i_add %3, 1u, 0u, 1i
    %5:ptr<storage, f32, read_write> = access %buffer, 1u, 0u, 0u, 3u
    %6:f32 = load %5
    ret
  }
}
)";

    auto* expect = R"(
Inner = struct @align(16) {
  vector:vec4<f32> @offset(0)
}

Outer = struct @align(16) {
  particle_count:i32 @offset(0)
  data:array<Inner, 4> @offset(16)
}

Outer_atomic = struct @align(16) {
  particle_count:atomic<i32> @offset(0)
  data:array<Inner, 4> @offset(16)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_atomic, read_write> = var undef @binding_point(0, 0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, atomic<i32>, read_write> = access %buffer, 0u
    %4:i32 = atomicAdd %3, 1i
    %5:ptr<storage, f32, read_write> = access %buffer, 1u, 0u, 0u, 3u
    %6:f32 = load %5
    ret
  }
}
)";

    ASSERT_EQ(src, str());
    Run(Atomics);
    ASSERT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
