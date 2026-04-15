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

#include "src/tint/lang/wgsl/ir/atomic_vec2u_to_from_u64.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"

namespace tint::wgsl::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using AtomicVec2uToFromU64Test = core::ir::transform::TransformTest;

TEST_F(AtomicVec2uToFromU64Test, NoChange_ToU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<u32>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u32>, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kToU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, NoChange_FromU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<u32>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u32>, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kFromU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, Simple_ToU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<vec2<u32>>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<vec2<u32>>, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u64>, read_write> = var undef @binding_point(0, 0)
}

)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kToU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, Simple_FromU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<u64>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u64>, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, atomic<vec2<u32>>, read_write> = var undef @binding_point(0, 0)
}

)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kFromU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, Struct_ToU64) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("a"), ty.atomic<vec2<u32>>()}});
    auto* var = b.Var("v", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
S = struct @align(8) {
  a:atomic<vec2<u32>> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(8) {
  a:atomic<vec2<u32>> @offset(0)
}

S_a64 = struct @align(8) {
  a:atomic<u64> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S_a64, read_write> = var undef @binding_point(0, 0)
}

)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kToU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, Struct_FromU64) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("a"), ty.atomic<u64>()}});
    auto* var = b.Var("v", ty.ptr(storage, str_ty));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
S = struct @align(8) {
  a:atomic<u64> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(8) {
  a:atomic<u64> @offset(0)
}

S_a64 = struct @align(8) {
  a:atomic<vec2<u32>> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, S_a64, read_write> = var undef @binding_point(0, 0)
}

)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kFromU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, Array_ToU64) {
    auto* var = b.Var("v", ty.ptr(storage, ty.array(ty.atomic<vec2<u32>>(), 4)));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<atomic<vec2<u32>>, 4>, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<atomic<u64>, 4>, read_write> = var undef @binding_point(0, 0)
}

)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kToU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, Array_FromU64) {
    auto* var = b.Var("v", ty.ptr(storage, ty.array(ty.atomic<u64>(), 4)));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, array<atomic<u64>, 4>, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, array<atomic<vec2<u32>>, 4>, read_write> = var undef @binding_point(0, 0)
}

)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kFromU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, AtomicStoreMax_ToU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<vec2<u32>>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call<wgsl::ir::BuiltinCall>(ty.void_(), wgsl::BuiltinFn::kAtomicStoreMax, var->Result(),
                                      b.Construct(ty.vec2u(), b.Constant(u32(1))));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<vec2<u32>>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:void = wgsl.atomicStoreMax %v, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u64>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:u64 = bitcast<u64> %3
    %5:void = atomicStoreMax %v, %4
    ret
  }
}
)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kToU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, AtomicStoreMax_FromU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<u64>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* val_vec2u = b.Construct(ty.vec2u(), b.Constant(u32(1)));
        auto* val_u64 = b.Bitcast(ty.u64(), val_vec2u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStoreMax, var->Result(), val_u64);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u64>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:u64 = bitcast<u64> %3
    %5:void = atomicStoreMax %v, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, atomic<vec2<u32>>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:void = wgsl.atomicStoreMax %v, %3
    ret
  }
}
)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kFromU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, AtomicStoreMin_ToU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<vec2<u32>>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call<wgsl::ir::BuiltinCall>(ty.void_(), wgsl::BuiltinFn::kAtomicStoreMin, var->Result(),
                                      b.Construct(ty.vec2u(), b.Constant(u32(1))));
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<vec2<u32>>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:void = wgsl.atomicStoreMin %v, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u64>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:u64 = bitcast<u64> %3
    %5:void = atomicStoreMin %v, %4
    ret
  }
}
)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kToU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, AtomicStoreMin_FromU64) {
    auto* var = b.Var("v", ty.ptr<storage, atomic<u64>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* val_vec2u = b.Construct(ty.vec2u(), b.Constant(u32(1)));
        auto* val_u64 = b.Bitcast(ty.u64(), val_vec2u);
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStoreMin, var->Result(), val_u64);
        b.Return(f);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<storage, atomic<u64>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:u64 = bitcast<u64> %3
    %5:void = atomicStoreMin %v, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<storage, atomic<vec2<u32>>, read_write> = var undef @binding_point(0, 0)
}

%f = func():void {
  $B2: {
    %3:vec2<u32> = construct 1u
    %4:void = wgsl.atomicStoreMin %v, %3
    ret
  }
}
)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kFromU64); });

    EXPECT_EQ(expect, str());
}

TEST_F(AtomicVec2uToFromU64Test, AtomicStoreMin_NoBinding) {
    auto* f = b.Function("f", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr<storage, atomic<vec2u>>());
    f->AppendParam(param);
    b.Append(f->Block(), [&] {
        auto* val_vec2u = b.Construct(ty.vec2u(), b.Constant(u32(1)));
        b.Call<wgsl::ir::BuiltinCall>(ty.void_(), wgsl::BuiltinFn::kAtomicStoreMin, param,
                                      val_vec2u);
        b.Return(f);
    });

    auto* src = R"(
%f = func(%param:ptr<storage, atomic<vec2<u32>>, read_write>):void {
  $B1: {
    %3:vec2<u32> = construct 1u
    %4:void = wgsl.atomicStoreMin %param, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func(%param:ptr<storage, atomic<u64>, read_write>):void {
  $B1: {
    %3:vec2<u32> = construct 1u
    %4:u64 = bitcast<u64> %3
    %5:void = atomicStoreMin %param, %4
    ret
  }
}
)";

    Run([](auto& mod2) { return AtomicVec2uToFromU64(mod2, AtomicVec2uU64Direction::kToU64); });

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::ir::transform
