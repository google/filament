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

#include "src/tint/lang/spirv/writer/raise/builtin_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_BuiltinPolyfillTest, ArrayLength) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str_ty = ty.Struct(mod.symbols.New("Buffer"), {
                                                            {mod.symbols.New("a"), ty.i32()},
                                                            {mod.symbols.New("b"), ty.i32()},
                                                            {mod.symbols.New("arr"), arr},
                                                        });
    auto* var = b.Var("var", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, arr), var, 2_u);
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, access);
        b.Return(func, result);
    });

    auto* src = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

$B1: {  # root
  %var:ptr<storage, Buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %4:u32 = arrayLength %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

$B1: {  # root
  %var:ptr<storage, Buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %4:u32 = spirv.array_length %var, 2u
    ret %4
  }
}
)";
    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, ArrayLength_ViaLet_BeforeAccess) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str_ty = ty.Struct(mod.symbols.New("Buffer"), {
                                                            {mod.symbols.New("a"), ty.i32()},
                                                            {mod.symbols.New("b"), ty.i32()},
                                                            {mod.symbols.New("arr"), arr},
                                                        });
    auto* var = b.Var("var", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* let_a = b.Let("a", var);
        auto* let_b = b.Let("b", let_a);
        auto* access = b.Access(ty.ptr(storage, arr), let_b, 2_u);
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, access);
        b.Return(func, result);
    });

    auto* src = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

$B1: {  # root
  %var:ptr<storage, Buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %a:ptr<storage, Buffer, read_write> = let %var
    %b:ptr<storage, Buffer, read_write> = let %a
    %5:ptr<storage, array<i32>, read_write> = access %b, 2u
    %6:u32 = arrayLength %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

$B1: {  # root
  %var:ptr<storage, Buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %a:ptr<storage, Buffer, read_write> = let %var
    %b:ptr<storage, Buffer, read_write> = let %a
    %5:ptr<storage, array<i32>, read_write> = access %b, 2u
    %6:u32 = spirv.array_length %b, 2u
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, ArrayLength_ViaLet_AfterAccess) {
    auto* arr = ty.runtime_array(ty.i32());
    auto* str_ty = ty.Struct(mod.symbols.New("Buffer"), {
                                                            {mod.symbols.New("a"), ty.i32()},
                                                            {mod.symbols.New("b"), ty.i32()},
                                                            {mod.symbols.New("arr"), arr},
                                                        });
    auto* var = b.Var("var", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, arr), var, 2_u);
        auto* let_a = b.Let("a", access);
        auto* let_b = b.Let("b", let_a);
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, let_b);
        b.Return(func, result);
    });

    auto* src = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

$B1: {  # root
  %var:ptr<storage, Buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %a:ptr<storage, array<i32>, read_write> = let %3
    %b:ptr<storage, array<i32>, read_write> = let %a
    %6:u32 = arrayLength %b
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Buffer = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
  arr:array<i32> @offset(8)
}

$B1: {  # root
  %var:ptr<storage, Buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():u32 {
  $B2: {
    %3:ptr<storage, array<i32>, read_write> = access %var, 2u
    %a:ptr<storage, array<i32>, read_write> = let %3
    %b:ptr<storage, array<i32>, read_write> = let %a
    %6:u32 = spirv.array_length %var, 2u
    ret %6
  }
}
)";
    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicAdd_Storage) {
    auto* var = b.Var(ty.ptr(storage, ty.atomic(ty.i32())));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<storage, atomic<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicAdd %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<storage, atomic<i32>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_i_add %1, 1u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicAdd_Workgroup) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicAdd %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_i_add %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicAnd) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicAnd, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicAnd %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_and %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicCompareExchangeWeak) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* cmp = b.FunctionParam("cmp", ty.i32());
    auto* val = b.FunctionParam("val", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cmp, val});

    b.Append(func->Block(), [&] {
        auto* result_ty = core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32());
        auto* result =
            b.Call(result_ty, core::BuiltinFn::kAtomicCompareExchangeWeak, var, cmp, val);
        b.Return(func, b.Access(ty.i32(), result, 0_u));
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%cmp:i32, %val:i32):i32 {
  $B2: {
    %5:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %1, %cmp, %val
    %6:i32 = access %5, 0u
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%cmp:i32, %val:i32):i32 {
  $B2: {
    %5:i32 = spirv.atomic_compare_exchange %1, 2u, 0u, 0u, %val, %cmp
    %6:bool = eq %5, %cmp
    %7:__atomic_compare_exchange_result_i32 = construct %5, %6
    %8:i32 = access %7, 0u
    ret %8
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicExchange) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicExchange, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicExchange %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_exchange %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicLoad) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* func = b.Function("foo", ty.i32());

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    %3:i32 = atomicLoad %1
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    %3:i32 = spirv.atomic_load %1, 2u, 0u
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicMax_I32) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicMax, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicMax %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_s_max %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicMax_U32) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.u32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kAtomicMax, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = func(%arg1:u32):u32 {
  $B2: {
    %4:u32 = atomicMax %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = func(%arg1:u32):u32 {
  $B2: {
    %4:u32 = spirv.atomic_u_max %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicMin_I32) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicMin, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicMin %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_s_min %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicMin_U32) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.u32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kAtomicMin, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = func(%arg1:u32):u32 {
  $B2: {
    %4:u32 = atomicMin %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = func(%arg1:u32):u32 {
  $B2: {
    %4:u32 = spirv.atomic_u_min %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicOr) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicOr, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicOr %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_or %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicStore) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, var, arg1);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):void {
  $B2: {
    %4:void = atomicStore %1, %arg1
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):void {
  $B2: {
    %4:void = spirv.atomic_store %1, 2u, 0u, %arg1
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicSub) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicSub %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_i_sub %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, AtomicXor) {
    auto* var = mod.root_block->Append(b.Var(ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicXor, var, arg1);
        b.Return(func, result);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = atomicXor %1, %arg1
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%arg1:i32):i32 {
  $B2: {
    %4:i32 = spirv.atomic_xor %1, 2u, 0u, %arg1
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Dot_Vec4f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 {
  $B1: {
    %4:f32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 {
  $B1: {
    %4:f32 = spirv.dot %arg1, %arg2
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Dot_Vec2i) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<i32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 {
  $B1: {
    %4:i32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 {
  $B1: {
    %4:i32 = access %arg1, 0u
    %5:i32 = access %arg2, 0u
    %6:i32 = mul %4, %5
    %7:i32 = access %arg1, 1u
    %8:i32 = access %arg2, 1u
    %9:i32 = mul %7, %8
    %10:i32 = add %6, %9
    ret %10
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Dot_Vec4u) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<u32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 {
  $B1: {
    %4:u32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 {
  $B1: {
    %4:u32 = access %arg1, 0u
    %5:u32 = access %arg2, 0u
    %6:u32 = mul %4, %5
    %7:u32 = access %arg1, 1u
    %8:u32 = access %arg2, 1u
    %9:u32 = mul %7, %8
    %10:u32 = add %6, %9
    %11:u32 = access %arg1, 2u
    %12:u32 = access %arg2, 2u
    %13:u32 = mul %11, %12
    %14:u32 = add %10, %13
    %15:u32 = access %arg1, 3u
    %16:u32 = access %arg2, 3u
    %17:u32 = mul %15, %16
    %18:u32 = add %14, %17
    ret %18
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Dot4I8Packed) {
    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* arg2 = b.FunctionParam("arg2", ty.u32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kDot4I8Packed, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:u32, %arg2:u32):i32 {
  $B1: {
    %4:i32 = dot4I8Packed %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:u32, %arg2:u32):i32 {
  $B1: {
    %4:i32 = spirv.s_dot %arg1, %arg2, 0u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Dot4U8Packed) {
    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* arg2 = b.FunctionParam("arg2", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kDot4U8Packed, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:u32, %arg2:u32):u32 {
  $B1: {
    %4:u32 = dot4U8Packed %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:u32, %arg2:u32):u32 {
  $B1: {
    %4:u32 = spirv.u_dot %arg1, %arg2, 0u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Select_ScalarCondition_ScalarOperands) {
    auto* argf = b.FunctionParam("argf", ty.i32());
    auto* argt = b.FunctionParam("argt", ty.i32());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 {
  $B1: {
    %5:i32 = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 {
  $B1: {
    %5:i32 = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Select_VectorCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), core::BuiltinFn::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> {
  $B1: {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> {
  $B1: {
    %5:vec4<i32> = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Clamp_VectorOperands_DisabledScalarize) {
    auto* x = b.FunctionParam("x", ty.vec2<f32>());
    auto* low = b.FunctionParam("low", ty.vec2<f32>());
    auto* high = b.FunctionParam("high", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({x, low, high});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<f32>(), core::BuiltinFn::kClamp, x, low, high);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%x:vec2<f32>, %low:vec2<f32>, %high:vec2<f32>):vec2<f32> {
  $B1: {
    %5:vec2<f32> = clamp %x, %low, %high
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec2<f32>, %low:vec2<f32>, %high:vec2<f32>):vec2<f32> {
  $B1: {
    %5:vec2<f32> = clamp %x, %low, %high
    ret %5
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Select_ScalarCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), core::BuiltinFn::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> {
  $B1: {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> {
  $B1: {
    %5:vec4<bool> = construct %cond, %cond, %cond, %cond
    %6:vec4<i32> = spirv.select %5, %argt, %argf
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, Select_ScalarCondition_VectorOperands_Spv14) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), core::BuiltinFn::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> {
  $B1: {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> {
  $B1: {
    %5:vec4<i32> = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    PolyfillConfig config{.version = SpvVersion::kSpv14};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t, coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %coords:vec2<i32>, %lod:i32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureLoad %t, %coords, %lod
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %coords:vec2<i32>, %lod:i32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = spirv.image_fetch %t, %coords, 2u, %lod
    ret %5
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_BindingArray_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* sampled_texture = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* arr = b.FunctionParam("arr", ty.binding_array(sampled_texture, 2));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arr, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad,
                              b.Access(sampled_texture, arr, 0_u), coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arr:binding_array<texture_2d<f32>, 2>, %coords:vec2<i32>, %lod:i32):vec4<f32> {
  $B1: {
    %5:texture_2d<f32> = access %arr, 0u
    %6:vec4<f32> = textureLoad %5, %coords, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arr:binding_array<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, 2>, %coords:vec2<i32>, %lod:i32):vec4<f32> {
  $B1: {
    %5:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write> = access %arr, 0u
    %6:vec4<f32> = spirv.image_fetch %5, %coords, 2u, %lod
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_2DArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, array_idx, lod});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t, coords, array_idx, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %array_idx:i32, %lod:i32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureLoad %t, %coords, %array_idx, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %coords:vec2<i32>, %array_idx:i32, %lod:i32):vec4<f32> {
  $B1: {
    %6:vec3<i32> = construct %coords, %array_idx
    %7:vec4<f32> = spirv.image_fetch %t, %6, 2u, %lod
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_2DArray_IndexDifferentType) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.u32());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, array_idx, lod});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t, coords, array_idx, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %coords:vec2<i32>, %array_idx:u32, %lod:i32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureLoad %t, %coords, %array_idx, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %coords:vec2<i32>, %array_idx:u32, %lod:i32):vec4<f32> {
  $B1: {
    %6:i32 = convert %array_idx
    %7:vec3<i32> = construct %coords, %6
    %8:vec4<f32> = spirv.image_fetch %t, %7, 2u, %lod
    ret %8
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_Multisampled2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.multisampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* sample_idx = b.FunctionParam("sample_idx", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, coords, sample_idx});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, t, coords, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<f32>, %coords:vec2<i32>, %sample_idx:i32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureLoad %t, %coords, %sample_idx
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, multi_sampled, sampling_compatible, undefined, read_write>, %coords:vec2<i32>, %sample_idx:i32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = spirv.image_fetch %t, %coords, 64u, %sample_idx
    ret %5
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_Depth2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kTextureLoad, t, coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %coords:vec2<i32>, %lod:i32):f32 {
  $B1: {
    %5:f32 = textureLoad %t, %coords, %lod
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %coords:vec2<i32>, %lod:i32):f32 {
  $B1: {
    %5:vec4<f32> = spirv.image_fetch %t, %coords, 2u, %lod
    %6:f32 = access %5, 0u
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_Storage) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kR32Uint;
    auto* t = b.FunctionParam("t", ty.storage_texture(core::type::TextureDimension::k2d, format,
                                                      core::Access::kReadWrite));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec4<u32>());
    func->SetParams({t, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<u32>(), core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<r32uint, read_write>, %coords:vec2<i32>):vec4<u32> {
  $B1: {
    %4:vec4<u32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, r32uint, read_write>, %coords:vec2<i32>):vec4<u32> {
  $B1: {
    %4:vec4<u32> = spirv.image_read %t, %coords, 0u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_TexelBuffer) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kR32Uint;
    auto* t = b.FunctionParam("t", ty.texel_buffer(format, core::Access::kReadWrite));
    auto* coords = b.FunctionParam("coords", ty.u32());
    auto* func = b.Function("foo", ty.vec4<u32>());
    func->SetParams({t, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<u32>(), core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texel_buffer<r32uint, read_write>, %coords:u32):vec4<u32> {
  $B1: {
    %4:vec4<u32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, buffer, not_depth, non_arrayed, single_sampled, rw_op_compatible, r32uint, read_write>, %coords:u32):vec4<u32> {
  $B1: {
    %4:vec4<u32> = spirv.image_read %t, %coords, 0u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureStore_TexelBuffer) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kR32Uint;
    auto* t = b.FunctionParam("t", ty.texel_buffer(format, core::Access::kReadWrite));
    auto* coords = b.FunctionParam("coords", ty.u32());
    auto* texel = b.FunctionParam("texel", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, t, coords, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texel_buffer<r32uint, read_write>, %coords:u32, %texel:vec4<u32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, buffer, not_depth, non_arrayed, single_sampled, rw_op_compatible, r32uint, read_write>, %coords:u32, %texel:vec4<u32>):void {
  $B1: {
    %5:void = spirv.image_write %t, %coords, %texel, 0u
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureDimensions_TexelBuffer) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam(
        "t", ty.texel_buffer(core::TexelFormat::kR32Uint, core::Access::kReadWrite));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texel_buffer<r32uint, read_write>):u32 {
  $B1: {
    %3:u32 = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, buffer, not_depth, non_arrayed, single_sampled, rw_op_compatible, r32uint, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_size<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureLoad_Storage_Vulkan) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kR32Uint;
    auto* t = b.FunctionParam("t", ty.storage_texture(core::type::TextureDimension::k2d, format,
                                                      core::Access::kReadWrite));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec4<u32>());
    func->SetParams({t, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<u32>(), core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<r32uint, read_write>, %coords:vec2<i32>):vec4<u32> {
  $B1: {
    %4:vec4<u32> = textureLoad %t, %coords
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, r32uint, read_write>, %coords:vec2<i32>):vec4<u32> {
  $B1: {
    %4:vec4<u32> = spirv.image_read %t, %coords, 1024u
    ret %4
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSample_1D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>, %s:sampler, %coords:f32):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureSample %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:f32):vec4<f32> {
  $B1: {
    %5:spirv.sampled_image<spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 0u
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSample_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureSample %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 0u
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSample_2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, t, s, coords,
                              b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureSample %t, %s, %coords, vec2<i32>(1i)
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %6:vec4<f32> = spirv.image_sample_implicit_lod %5, %coords, 8u, vec2<i32>(1i)
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSample_2DArray_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSample, t, s, coords,
                              array_idx, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureSample %t, %s, %coords, %array_idx, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %array_idx:i32):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:f32 = convert %array_idx
    %8:vec3<f32> = construct %coords, %7
    %9:vec4<f32> = spirv.image_sample_implicit_lod %6, %8, 8u, vec2<i32>(1i)
    ret %9
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleBias_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, bias});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBias, t, s, coords, bias);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureSampleBias %t, %s, %coords, %bias
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, %coords, 1u, %bias
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleBias_2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBias, t, s, coords,
                              bias, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureSampleBias %t, %s, %coords, %bias, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %bias:f32):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_sample_implicit_lod %6, %coords, 9u, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleBias_2DArray_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleBias, t, s, coords,
                              array_idx, bias, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleBias %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %bias:f32):vec4<f32> {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_sample_implicit_lod %7, %9, 9u, %bias, vec2<i32>(1i)
    ret %10
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleCompare_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompare, t, s, coords, dref);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:f32 = textureSampleCompare %t, %s, %coords, %dref
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 0u
    ret %7
  }
}
)";
    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleCompare_2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompare, t, s, coords, dref,
                              b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:f32 = textureSampleCompare %t, %s, %coords, %dref, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:f32 = spirv.image_sample_dref_implicit_lod %6, %coords, %dref, 8u, vec2<i32>(1i)
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleCompare_2DArray_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, array_idx, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompare, t, s, coords,
                              array_idx, bias, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler_comparison, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 {
  $B1: {
    %7:f32 = textureSampleCompare %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:f32 = spirv.image_sample_dref_implicit_lod %7, %9, %bias, 8u, vec2<i32>(1i)
    ret %10
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, dref);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:f32 = textureSampleCompareLevel %t, %s, %coords, %dref
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:f32 = spirv.image_sample_dref_explicit_lod %6, %coords, %dref, 2u, 0.0f
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* dref = b.FunctionParam("dref", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, dref});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords,
                              dref, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:f32 = textureSampleCompareLevel %t, %s, %coords, %dref, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %dref:f32):f32 {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:f32 = spirv.image_sample_dref_explicit_lod %6, %coords, %dref, 10u, 0.0f, vec2<i32>(1i)
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleCompareLevel_2DArray_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* bias = b.FunctionParam("bias", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t, s, coords, array_idx, bias});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords,
                              array_idx, bias, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler_comparison, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 {
  $B1: {
    %7:f32 = textureSampleCompareLevel %t, %s, %coords, %array_idx, %bias, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %array_idx:i32, %bias:f32):f32 {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:f32 = spirv.image_sample_dref_explicit_lod %7, %9, %bias, 10u, 0.0f, vec2<i32>(1i)
    ret %10
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleGrad_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleGrad %t, %s, %coords, %ddx, %ddy
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:vec4<f32> = spirv.image_sample_explicit_lod %7, %coords, 4u, %ddx, %ddy
    ret %8
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleGrad_2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, ddx, ddy});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleGrad, t, s, coords,
                              ddx, ddy, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleGrad %t, %s, %coords, %ddx, %ddy, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:vec4<f32> = spirv.image_sample_explicit_lod %7, %coords, 12u, %ddx, %ddy, vec2<i32>(1i)
    ret %8
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleGrad_2DArray_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* ddx = b.FunctionParam("ddx", ty.vec2<f32>());
    auto* ddy = b.FunctionParam("ddy", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, ddx, ddy});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleGrad, t, s, coords,
                              array_idx, ddx, ddy, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %8:vec4<f32> = textureSampleGrad %t, %s, %coords, %array_idx, %ddx, %ddy, vec2<i32>(1i)
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %ddx:vec2<f32>, %ddy:vec2<f32>):vec4<f32> {
  $B1: {
    %8:spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %9:f32 = convert %array_idx
    %10:vec3<f32> = construct %coords, %9
    %11:vec4<f32> = spirv.image_sample_explicit_lod %8, %10, 12u, %ddx, %ddy, vec2<i32>(1i)
    ret %11
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleLevel_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleLevel, t, s, coords, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureSampleLevel %t, %s, %coords, %lod
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_sample_explicit_lod %6, %coords, 2u, %lod
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleLevel_2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleLevel, t, s, coords,
                              lod, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureSampleLevel %t, %s, %coords, %lod, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %lod:f32):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_sample_explicit_lod %6, %coords, 10u, %lod, vec2<i32>(1i)
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureSampleLevel_2DArray_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* lod = b.FunctionParam("lod", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureSampleLevel, t, s, coords,
                              array_idx, lod, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %lod:f32):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureSampleLevel %t, %s, %coords, %array_idx, %lod, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>, %array_idx:i32, %lod:f32):vec4<f32> {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_sample_explicit_lod %7, %9, 10u, %lod, vec2<i32>(1i)
    ret %10
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureGather_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* component = b.FunctionParam("component", ty.i32());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({component, t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, component, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%component:i32, %t:texture_2d<f32>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureGather %component, %t, %s, %coords
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%component:i32, %t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_gather %6, %coords, %component, 0u
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureGather_2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* component = b.FunctionParam("component", ty.i32());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, component, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, component, t, s,
                              coords, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %s:sampler, %component:i32, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureGather %component, %t, %s, %coords, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %component:i32, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_gather %6, %coords, %component, 8u, vec2<i32>(1i)
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureGather_2DArray_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* component = b.FunctionParam("component", ty.i32());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, component, coords, array_idx});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, component, t, s,
                              coords, array_idx, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>, %s:sampler, %component:i32, %coords:vec2<f32>, %array_idx:i32):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureGather %component, %t, %s, %coords, %array_idx, vec2<i32>(1i)
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %component:i32, %coords:vec2<f32>, %array_idx:i32):vec4<f32> {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_gather %7, %9, %component, 8u, vec2<i32>(1i)
    ret %10
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureGather_Depth2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGather, t, s, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:vec4<f32> = textureGather %t, %s, %coords
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler, %coords:vec2<f32>):vec4<f32> {
  $B1: {
    %5:spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %6:vec4<f32> = spirv.image_gather %5, %coords, 0u, 0u
    ret %6
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureGatherCompare_Depth2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, depth});

    b.Append(func->Block(), [&] {
        auto* result =
            b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGatherCompare, t, s, coords, depth);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureGatherCompare %t, %s, %coords, %depth
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_dref_gather %6, %coords, %depth, 0u
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureGatherCompare_Depth2D_Offset) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, depth});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                              depth, b.Splat<vec2<i32>>(1_i));
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):vec4<f32> {
  $B1: {
    %6:vec4<f32> = textureGatherCompare %t, %s, %coords, %depth, vec2<i32>(1i)
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %depth:f32):vec4<f32> {
  $B1: {
    %6:spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %7:vec4<f32> = spirv.image_dref_gather %6, %coords, %depth, 8u, vec2<i32>(1i)
    ret %7
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureGatherCompare_Depth2DArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2dArray));
    auto* s = b.FunctionParam("s", ty.comparison_sampler());
    auto* coords = b.FunctionParam("coords", ty.vec2<f32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.u32());
    auto* depth = b.FunctionParam("depth", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t, s, coords, array_idx, depth});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                              array_idx, depth);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array, %s:sampler_comparison, %coords:vec2<f32>, %array_idx:u32, %depth:f32):vec4<f32> {
  $B1: {
    %7:vec4<f32> = textureGatherCompare %t, %s, %coords, %array_idx, %depth
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>, %s:sampler_comparison, %coords:vec2<f32>, %array_idx:u32, %depth:f32):vec4<f32> {
  $B1: {
    %7:spirv.sampled_image<spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> = spirv.sampled_image<spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>> %t, %s
    %8:f32 = convert %array_idx
    %9:vec3<f32> = construct %coords, %8
    %10:vec4<f32> = spirv.image_dref_gather %7, %9, %depth, 0u
    ret %10
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureStore_2D) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kR32Uint;
    auto* t = b.FunctionParam(
        "t", ty.storage_texture(core::type::TextureDimension::k2d, format, core::Access::kWrite));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* texel = b.FunctionParam("texel", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, t, coords, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<r32uint, write>, %coords:vec2<i32>, %texel:vec4<u32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, r32uint, write>, %coords:vec2<i32>, %texel:vec4<u32>):void {
  $B1: {
    %5:void = spirv.image_write %t, %coords, %texel, 0u
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureStore_2D_Vulkan) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kR32Uint;
    auto* t = b.FunctionParam("t", ty.storage_texture(core::type::TextureDimension::k2d, format,
                                                      core::Access::kReadWrite));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* texel = b.FunctionParam("texel", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, t, coords, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<r32uint, read_write>, %coords:vec2<i32>, %texel:vec4<u32>):void {
  $B1: {
    %5:void = textureStore %t, %coords, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, 2d, not_depth, non_arrayed, single_sampled, rw_op_compatible, r32uint, read_write>, %coords:vec2<i32>, %texel:vec4<u32>):void {
  $B1: {
    %5:void = spirv.image_write %t, %coords, %texel, 1024u
    ret
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureStore_2DArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kRgba8Sint;
    auto* t = b.FunctionParam("t", ty.storage_texture(core::type::TextureDimension::k2dArray,
                                                      format, core::Access::kWrite));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.i32());
    auto* texel = b.FunctionParam("texel", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, array_idx, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, t, coords, array_idx, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<rgba8sint, write>, %coords:vec2<i32>, %array_idx:i32, %texel:vec4<i32>):void {
  $B1: {
    %6:void = textureStore %t, %coords, %array_idx, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<i32, 2d, not_depth, arrayed, single_sampled, rw_op_compatible, rgba8sint, write>, %coords:vec2<i32>, %array_idx:i32, %texel:vec4<i32>):void {
  $B1: {
    %6:vec3<i32> = construct %coords, %array_idx
    %7:void = spirv.image_write %t, %6, %texel, 0u
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureStore_2DArray_IndexDifferentType) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kRgba32Uint;
    auto* t = b.FunctionParam("t", ty.storage_texture(core::type::TextureDimension::k2dArray,
                                                      format, core::Access::kWrite));
    auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
    auto* array_idx = b.FunctionParam("array_idx", ty.u32());
    auto* texel = b.FunctionParam("texel", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t, coords, array_idx, texel});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, t, coords, array_idx, texel);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<rgba32uint, write>, %coords:vec2<i32>, %array_idx:u32, %texel:vec4<u32>):void {
  $B1: {
    %6:void = textureStore %t, %coords, %array_idx, %texel
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<u32, 2d, not_depth, arrayed, single_sampled, rw_op_compatible, rgba32uint, write>, %coords:vec2<i32>, %array_idx:u32, %texel:vec4<u32>):void {
  $B1: {
    %6:i32 = convert %array_idx
    %7:vec3<i32> = construct %coords, %6
    %8:void = spirv.image_write %t, %7, %texel, 0u
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureDimensions_2D_ImplicitLod) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = spirv.image_query_size_lod<u32> %t, 0u
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureDimensions_2D_ExplicitLod) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* lod = b.FunctionParam("lod", ty.i32());
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t, lod});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t, lod);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>, %lod:i32):vec2<u32> {
  $B1: {
    %4:vec2<u32> = textureDimensions %t, %lod
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>, %lod:i32):vec2<u32> {
  $B1: {
    %4:vec2<u32> = spirv.image_query_size_lod<u32> %t, %lod
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureDimensions_2DArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):vec2<u32> {
  $B1: {
    %3:vec3<u32> = spirv.image_query_size_lod<u32> %t, 0u
    %4:vec2<u32> = swizzle %3, xy
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureDimensions_Multisampled) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.multisampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, multi_sampled, sampling_compatible, undefined, read_write>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = spirv.image_query_size<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_1d) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 1d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_2d) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_2d_array) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_3d) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_3d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 3d, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_cube) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_cube<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, cube, not_depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_CubeArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam(
        "t", ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_cube_array<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, cube, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_depth_2d) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_depth_2dArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2dArray));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_depth_cube) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::kCube));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_cube):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, cube, depth, non_arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLevels_depth_CubeArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::kCubeArray));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_cube_array):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, cube, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_levels<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumSamples_ms_2d) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.multisampled_texture(core::type::TextureDimension::k2d, ty.i32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<i32>):u32 {
  $B1: {
    %3:u32 = textureNumSamples %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<i32, 2d, not_depth, non_arrayed, multi_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_samples<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumSamples_depth_ms_2d) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.depth_multisampled_texture(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_multisampled_2d):u32 {
  $B1: {
    %3:u32 = textureNumSamples %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, non_arrayed, multi_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:u32 = spirv.image_query_samples<u32> %t
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLayers_2DArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:vec3<u32> = spirv.image_query_size_lod<u32> %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLayers_CubeArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam(
        "t", ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_cube_array<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, cube, not_depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:vec3<u32> = spirv.image_query_size_lod<u32> %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLayers_Depth2DArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::k2dArray));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:vec3<u32> = spirv.image_query_size_lod<u32> %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLayers_DepthCubeArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.depth_texture(core::type::TextureDimension::kCubeArray));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_cube_array):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, cube, depth, arrayed, single_sampled, sampling_compatible, undefined, read_write>):u32 {
  $B1: {
    %3:vec3<u32> = spirv.image_query_size_lod<u32> %t, 0u
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, TextureNumLayers_Storage2DArray) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto format = core::TexelFormat::kR32Float;
    auto* t = b.FunctionParam("t", ty.storage_texture(core::type::TextureDimension::k2dArray,
                                                      format, core::Access::kWrite));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d_array<r32float, write>):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, 2d, not_depth, arrayed, single_sampled, rw_op_compatible, r32float, write>):u32 {
  $B1: {
    %3:vec3<u32> = spirv.image_query_size<u32> %t
    %4:u32 = access %3, 2u
    ret %4
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, QuantizeToF16_Scalar) {
    auto* arg = b.FunctionParam("arg", ty.f32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), core::BuiltinFn::kQuantizeToF16, arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:f32):f32 {
  $B1: {
    %3:f32 = quantizeToF16 %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, QuantizeToF16_Vector) {
    auto* arg = b.FunctionParam("arg", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kQuantizeToF16, arg);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = quantizeToF16 %arg
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg:vec4<f32>):vec4<f32> {
  $B1: {
    %3:f32 = access %arg, 0u
    %4:f32 = quantizeToF16 %3
    %5:f32 = access %arg, 1u
    %6:f32 = quantizeToF16 %5
    %7:f32 = access %arg, 2u
    %8:f32 = quantizeToF16 %7
    %9:f32 = access %arg, 3u
    %10:f32 = quantizeToF16 %9
    %11:vec4<f32> = construct %4, %6, %8, %10
    ret %11
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, InputAttachmentLoad) {
    capabilities = core::ir::Capability::kAllowNonCoreTypes;

    auto* t = b.FunctionParam("t", ty.input_attachment(ty.f32()));
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), core::BuiltinFn::kInputAttachmentLoad, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:input_attachment<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = inputAttachmentLoad %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:spirv.image<f32, subpass_data, not_depth, non_arrayed, single_sampled, rw_op_compatible, undefined, read_write>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = spirv.image_read %t, vec2<i32>(0i)
    ret %3
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupShuffle) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kSubgroupShuffle, 1_i, 1_i));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = subgroupShuffle 1i, 1i
    %a:i32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = bitcast 1i
    %3:i32 = subgroupShuffle 1i, %2
    %a:i32 = let %3
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupBroadcastConstSignedId) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kSubgroupBroadcast, 1_i, 1_i));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = subgroupBroadcast 1i, 1i
    %a:i32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = subgroupBroadcast 1i, 1u
    %a:i32 = let %2
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, QuadBroadcastConstSignedId) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kQuadBroadcast, 1_i, 1_i));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = quadBroadcast 1i, 1i
    %a:i32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = quadBroadcast 1i, 1u
    %a:i32 = let %2
    ret
  }
}
)";

    PolyfillConfig config;
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixLoad_Storage_ColMajor_F32) {
    auto* mat = ty.subgroup_matrix_result(ty.f32(), 8, 8);
    auto* p = b.FunctionParam<ptr<storage, array<f32, 256>>>("p");
    auto* func = b.Function("foo", mat);
    func->SetParams({p});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(mat, core::BuiltinFn::kSubgroupMatrixLoad, Vector{mat}, p, 64_u,
                                    true, 32_u);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %3:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>> %p, 64u, true, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %3:ptr<storage, f32, read_write> = access %p, 64u
    %4:subgroup_matrix_result<f32, 8, 8> = spirv.cooperative_matrix_load<subgroup_matrix_result<f32, 8, 8>> %3, 1u, 32u, 32u
    ret %4
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixLoad_Workgroup_RowMajor_U32) {
    auto* mat = ty.subgroup_matrix_result(ty.u32(), 8, 8);
    auto* p = b.FunctionParam<ptr<workgroup, array<u32, 256>>>("p");
    auto* func = b.Function("foo", mat);
    func->SetParams({p});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(mat, core::BuiltinFn::kSubgroupMatrixLoad, Vector{mat}, p, 64_u,
                                    false, 32_u);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>):subgroup_matrix_result<u32, 8, 8> {
  $B1: {
    %3:subgroup_matrix_result<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<u32, 8, 8>> %p, 64u, false, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>):subgroup_matrix_result<u32, 8, 8> {
  $B1: {
    %3:ptr<workgroup, u32, read_write> = access %p, 64u
    %4:subgroup_matrix_result<u32, 8, 8> = spirv.cooperative_matrix_load<subgroup_matrix_result<u32, 8, 8>> %3, 0u, 32u, 32u
    ret %4
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixLoad_Storage_ColMajor_I8) {
    auto* mat = ty.subgroup_matrix_result(ty.i8(), 8, 8);
    auto* p = b.FunctionParam<ptr<storage, array<i32, 256>>>("p");
    auto* func = b.Function("foo", mat);
    func->SetParams({p});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(mat, core::BuiltinFn::kSubgroupMatrixLoad, Vector{mat}, p, 64_u,
                                    true, 32_u);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, array<i32, 256>, read_write>):subgroup_matrix_result<i8, 8, 8> {
  $B1: {
    %3:subgroup_matrix_result<i8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<i8, 8, 8>> %p, 64u, true, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, array<i32, 256>, read_write>):subgroup_matrix_result<i8, 8, 8> {
  $B1: {
    %3:u32 = div 32u, 4u
    %4:u32 = div 64u, 4u
    %5:ptr<storage, i32, read_write> = access %p, %4
    %6:subgroup_matrix_result<i8, 8, 8> = spirv.cooperative_matrix_load<subgroup_matrix_result<i8, 8, 8>> %5, 1u, %3, 32u
    ret %6
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixLoad_Workgroup_RowMajor_U8) {
    auto* mat = ty.subgroup_matrix_result(ty.u8(), 8, 8);
    auto* p = b.FunctionParam<ptr<workgroup, array<u32, 256>>>("p");
    auto* func = b.Function("foo", mat);
    func->SetParams({p});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(mat, core::BuiltinFn::kSubgroupMatrixLoad, Vector{mat}, p, 64_u,
                                    false, 32_u);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>):subgroup_matrix_result<u8, 8, 8> {
  $B1: {
    %3:subgroup_matrix_result<u8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<u8, 8, 8>> %p, 64u, false, 32u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>):subgroup_matrix_result<u8, 8, 8> {
  $B1: {
    %3:u32 = div 32u, 4u
    %4:u32 = div 64u, 4u
    %5:ptr<workgroup, u32, read_write> = access %p, %4
    %6:subgroup_matrix_result<u8, 8, 8> = spirv.cooperative_matrix_load<subgroup_matrix_result<u8, 8, 8>> %5, 0u, %3, 32u
    ret %6
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixStore_Storage_ColMajor_F32) {
    auto* p = b.FunctionParam<ptr<storage, array<f32, 256>>>("p");
    auto* m = b.FunctionParam("m", ty.subgroup_matrix_result(ty.f32(), 8, 8));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({p, m});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kSubgroupMatrixStore, p, 64_u, m, true, 32_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>, %m:subgroup_matrix_result<f32, 8, 8>):void {
  $B1: {
    %4:void = subgroupMatrixStore %p, 64u, %m, true, 32u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, array<f32, 256>, read_write>, %m:subgroup_matrix_result<f32, 8, 8>):void {
  $B1: {
    %4:ptr<storage, f32, read_write> = access %p, 64u
    %5:void = spirv.cooperative_matrix_store %4, %m, 1u, 32u, 32u
    ret
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixStore_Workgroup_RowMajor_U32) {
    auto* p = b.FunctionParam<ptr<workgroup, array<u32, 256>>>("p");
    auto* m = b.FunctionParam("m", ty.subgroup_matrix_result(ty.u32(), 8, 8));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({p, m});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kSubgroupMatrixStore, p, 64_u, m, false, 32_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>, %m:subgroup_matrix_result<u32, 8, 8>):void {
  $B1: {
    %4:void = subgroupMatrixStore %p, 64u, %m, false, 32u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>, %m:subgroup_matrix_result<u32, 8, 8>):void {
  $B1: {
    %4:ptr<workgroup, u32, read_write> = access %p, 64u
    %5:void = spirv.cooperative_matrix_store %4, %m, 0u, 32u, 32u
    ret
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixStore_Storage_ColMajor_I8) {
    auto* p = b.FunctionParam<ptr<storage, array<i32, 256>>>("p");
    auto* m = b.FunctionParam("m", ty.subgroup_matrix_result(ty.i8(), 8, 8));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({p, m});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kSubgroupMatrixStore, p, 64_u, m, true, 32_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, array<i32, 256>, read_write>, %m:subgroup_matrix_result<i8, 8, 8>):void {
  $B1: {
    %4:void = subgroupMatrixStore %p, 64u, %m, true, 32u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, array<i32, 256>, read_write>, %m:subgroup_matrix_result<i8, 8, 8>):void {
  $B1: {
    %4:u32 = div 32u, 4u
    %5:u32 = div 64u, 4u
    %6:ptr<storage, i32, read_write> = access %p, %5
    %7:void = spirv.cooperative_matrix_store %6, %m, 1u, %4, 32u
    ret
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixStore_Workgroup_RowMajor_U8) {
    auto* p = b.FunctionParam<ptr<workgroup, array<u32, 256>>>("p");
    auto* m = b.FunctionParam("m", ty.subgroup_matrix_result(ty.u8(), 8, 8));
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({p, m});
    b.Append(func->Block(), [&] {
        b.Call<void>(core::BuiltinFn::kSubgroupMatrixStore, p, 64_u, m, false, 32_u);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>, %m:subgroup_matrix_result<u8, 8, 8>):void {
  $B1: {
    %4:void = subgroupMatrixStore %p, 64u, %m, false, 32u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<workgroup, array<u32, 256>, read_write>, %m:subgroup_matrix_result<u8, 8, 8>):void {
  $B1: {
    %4:u32 = div 32u, 4u
    %5:u32 = div 64u, 4u
    %6:ptr<workgroup, u32, read_write> = access %p, %5
    %7:void = spirv.cooperative_matrix_store %6, %m, 0u, %4, 32u
    ret
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_F32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.f32(), 4, 8));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.f32(), 8, 4));
    auto* result = ty.subgroup_matrix_result(ty.f32(), 8, 8);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.f32()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %4:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiply<f32> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %4:subgroup_matrix_result<f32, 8, 8> = construct
    %5:subgroup_matrix_result<f32, 8, 8> = spirv.cooperative_matrix_mul_add %left, %right, %4, 0u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_I32_U32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.i32(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.i32(), 2, 8));
    auto* result = ty.subgroup_matrix_result(ty.u32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.u32()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<u32, 2, 4> = subgroupMatrixMultiply<u32> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<u32, 2, 4> = construct
    %5:subgroup_matrix_result<u32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %4, 3u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_U32_I32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.u32(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.u32(), 2, 8));
    auto* result = ty.subgroup_matrix_result(ty.i32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.i32()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<u32, 8, 4>, %right:subgroup_matrix_right<u32, 2, 8>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<i32, 2, 4> = subgroupMatrixMultiply<i32> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<u32, 8, 4>, %right:subgroup_matrix_right<u32, 2, 8>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<i32, 2, 4> = construct
    %5:subgroup_matrix_result<i32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %4, 12u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_I32_I32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.i32(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.i32(), 2, 8));
    auto* result = ty.subgroup_matrix_result(ty.i32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.i32()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<i32, 2, 4> = subgroupMatrixMultiply<i32> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<i32, 2, 4> = construct
    %5:subgroup_matrix_result<i32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %4, 15u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_I8_I32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.i8(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.i8(), 2, 8));
    auto* result = ty.subgroup_matrix_result(ty.i32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.i32()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<i8, 8, 4>, %right:subgroup_matrix_right<i8, 2, 8>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<i32, 2, 4> = subgroupMatrixMultiply<i32> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<i8, 8, 4>, %right:subgroup_matrix_right<i8, 2, 8>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<i32, 2, 4> = construct
    %5:subgroup_matrix_result<i32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %4, 15u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiply_U8_U32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.u8(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.u8(), 2, 8));
    auto* result = ty.subgroup_matrix_result(ty.u32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right});
    b.Append(func->Block(), [&] {
        auto* call = b.CallExplicit(result, core::BuiltinFn::kSubgroupMatrixMultiply,
                                    Vector{ty.u32()}, left, right);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<u8, 8, 4>, %right:subgroup_matrix_right<u8, 2, 8>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<u32, 2, 4> = subgroupMatrixMultiply<u32> %left, %right
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<u8, 8, 4>, %right:subgroup_matrix_right<u8, 2, 8>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %4:subgroup_matrix_result<u32, 2, 4> = construct
    %5:subgroup_matrix_result<u32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %4, 0u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_F32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.f32(), 4, 8));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.f32(), 8, 4));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.f32(), 8, 8));
    auto* result = ty.subgroup_matrix_result(ty.f32(), 8, 8);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>, %acc:subgroup_matrix_result<f32, 8, 8>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %5:subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<f32, 4, 8>, %right:subgroup_matrix_right<f32, 8, 4>, %acc:subgroup_matrix_result<f32, 8, 8>):subgroup_matrix_result<f32, 8, 8> {
  $B1: {
    %5:subgroup_matrix_result<f32, 8, 8> = spirv.cooperative_matrix_mul_add %left, %right, %acc, 0u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_I32_U32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.i32(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.i32(), 2, 8));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.u32(), 2, 4));
    auto* result = ty.subgroup_matrix_result(ty.u32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>, %acc:subgroup_matrix_result<u32, 2, 4>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<u32, 2, 4> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>, %acc:subgroup_matrix_result<u32, 2, 4>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<u32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %acc, 3u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_U32_I32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.u32(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.u32(), 2, 8));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.i32(), 2, 4));
    auto* result = ty.subgroup_matrix_result(ty.i32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<u32, 8, 4>, %right:subgroup_matrix_right<u32, 2, 8>, %acc:subgroup_matrix_result<i32, 2, 4>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<i32, 2, 4> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<u32, 8, 4>, %right:subgroup_matrix_right<u32, 2, 8>, %acc:subgroup_matrix_result<i32, 2, 4>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<i32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %acc, 12u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_I32_I32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.i32(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.i32(), 2, 8));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.i32(), 2, 4));
    auto* result = ty.subgroup_matrix_result(ty.i32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>, %acc:subgroup_matrix_result<i32, 2, 4>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<i32, 2, 4> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<i32, 8, 4>, %right:subgroup_matrix_right<i32, 2, 8>, %acc:subgroup_matrix_result<i32, 2, 4>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<i32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %acc, 15u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_I8_I32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.i8(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.i8(), 2, 8));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.i32(), 2, 4));
    auto* result = ty.subgroup_matrix_result(ty.i32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<i8, 8, 4>, %right:subgroup_matrix_right<i8, 2, 8>, %acc:subgroup_matrix_result<i32, 2, 4>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<i32, 2, 4> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<i8, 8, 4>, %right:subgroup_matrix_right<i8, 2, 8>, %acc:subgroup_matrix_result<i32, 2, 4>):subgroup_matrix_result<i32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<i32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %acc, 15u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupMatrixMultiplyAccumulate_U8_U32) {
    auto* left = b.FunctionParam("left", ty.subgroup_matrix_left(ty.u8(), 8, 4));
    auto* right = b.FunctionParam("right", ty.subgroup_matrix_right(ty.u8(), 2, 8));
    auto* acc = b.FunctionParam("acc", ty.subgroup_matrix_result(ty.u32(), 2, 4));
    auto* result = ty.subgroup_matrix_result(ty.u32(), 2, 4);
    auto* func = b.Function("foo", result);
    func->SetParams({left, right, acc});
    b.Append(func->Block(), [&] {
        auto* call =
            b.Call(result, core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
        b.Return(func, call);
    });

    auto* src = R"(
%foo = func(%left:subgroup_matrix_left<u8, 8, 4>, %right:subgroup_matrix_right<u8, 2, 8>, %acc:subgroup_matrix_result<u32, 2, 4>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<u32, 2, 4> = subgroupMatrixMultiplyAccumulate %left, %right, %acc
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%left:subgroup_matrix_left<u8, 8, 4>, %right:subgroup_matrix_right<u8, 2, 8>, %acc:subgroup_matrix_result<u32, 2, 4>):subgroup_matrix_result<u32, 2, 4> {
  $B1: {
    %5:subgroup_matrix_result<u32, 2, 4> = spirv.cooperative_matrix_mul_add %left, %right, %acc, 0u
    ret %5
  }
}
)";

    PolyfillConfig config{.use_vulkan_memory_model = true};
    Run(BuiltinPolyfill, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupShuffleClamped_NoPolyfill) {
    auto* func = b.Function("foo", ty.void_());
    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* arg2 = b.FunctionParam("arg2", ty.i32());
    func->SetParams({arg1, arg2});

    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kSubgroupShuffle, arg1, arg2));
        b.Return(func);
    });
    auto* src = R"(
%foo = func(%arg1:i32, %arg2:i32):void {
  $B1: {
    %4:i32 = subgroupShuffle %arg1, %arg2
    %a:i32 = let %4
    ret
  }
}
)";

    auto* expect = R"(
%foo = func(%arg1:i32, %arg2:i32):void {
  $B1: {
    %4:u32 = bitcast %arg2
    %5:i32 = subgroupShuffle %arg1, %4
    %a:i32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    PolyfillConfig config;
    config.subgroup_shuffle_clamped = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupShuffleClamped_I32) {
    auto* func = b.Function("foo", ty.void_());
    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* arg2 = b.FunctionParam("arg2", ty.i32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kSubgroupShuffle, arg1, arg2));
        b.Return(func);
    });
    auto* src = R"(
%foo = func(%arg1:i32, %arg2:i32):void {
  $B1: {
    %4:i32 = subgroupShuffle %arg1, %arg2
    %a:i32 = let %4
    ret
  }
}
)";
    auto* expect = R"(
%foo = func(%arg1:i32, %arg2:i32):void {
  $B1: {
    %4:u32 = bitcast %arg2
    %5:u32 = and %4, 127u
    %6:i32 = subgroupShuffle %arg1, %5
    %a:i32 = let %6
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    PolyfillConfig config;
    config.subgroup_shuffle_clamped = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupShuffleClamped_U32) {
    auto* func = b.Function("foo", ty.void_());
    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* arg2 = b.FunctionParam("arg2", ty.u32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kSubgroupShuffle, arg1, arg2));
        b.Return(func);
    });
    auto* src = R"(
%foo = func(%arg1:u32, %arg2:u32):void {
  $B1: {
    %4:u32 = subgroupShuffle %arg1, %arg2
    %a:u32 = let %4
    ret
  }
}
)";
    auto* expect = R"(
%foo = func(%arg1:u32, %arg2:u32):void {
  $B1: {
    %4:u32 = and %arg2, 127u
    %5:u32 = subgroupShuffle %arg1, %4
    %a:u32 = let %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    PolyfillConfig config;
    config.subgroup_shuffle_clamped = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupShuffleClamped_F32) {
    auto* func = b.Function("foo", ty.void_());
    auto* arg1 = b.FunctionParam("arg1", ty.f32());
    auto* arg2 = b.FunctionParam("arg2", ty.u32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.f32(), core::BuiltinFn::kSubgroupShuffle, arg1, arg2));
        b.Return(func);
    });
    auto* src = R"(
%foo = func(%arg1:f32, %arg2:u32):void {
  $B1: {
    %4:f32 = subgroupShuffle %arg1, %arg2
    %a:f32 = let %4
    ret
  }
}
)";
    auto* expect = R"(
%foo = func(%arg1:f32, %arg2:u32):void {
  $B1: {
    %4:u32 = and %arg2, 127u
    %5:f32 = subgroupShuffle %arg1, %4
    %a:f32 = let %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    PolyfillConfig config;
    config.subgroup_shuffle_clamped = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_BuiltinPolyfillTest, SubgroupShuffleClamped_Vec2F32) {
    auto* func = b.Function("foo", ty.void_());
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.u32());
    func->SetParams({arg1, arg2});
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kSubgroupShuffle, arg1, arg2));
        b.Return(func);
    });
    auto* src = R"(
%foo = func(%arg1:vec2<f32>, %arg2:u32):void {
  $B1: {
    %4:vec2<f32> = subgroupShuffle %arg1, %arg2
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    auto* expect = R"(
%foo = func(%arg1:vec2<f32>, %arg2:u32):void {
  $B1: {
    %4:u32 = and %arg2, 127u
    %5:vec2<f32> = subgroupShuffle %arg1, %4
    %a:vec2<f32> = let %5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    PolyfillConfig config;
    config.subgroup_shuffle_clamped = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
