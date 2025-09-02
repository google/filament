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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/sampler_kind.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

#include "gtest/gtest.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, BuiltinSelectScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kSelect, x, y, true);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int x = int(1);
  int y = int(2);
  int w = ((true) ? (y) : (x));
}

)");
}

TEST_F(HlslWriterTest, BuiltinSelectVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Construct<vec2<i32>>(1_i, 2_i));
        auto* y = b.Let("y", b.Construct<vec2<i32>>(3_i, 4_i));
        auto* cmp = b.Construct<vec2<bool>>(true, false);

        auto* c = b.Call(ty.vec2<i32>(), core::BuiltinFn::kSelect, x, y, cmp);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int2 x = int2(int(1), int(2));
  int2 y = int2(int(3), int(4));
  int2 w = ((bool2(true, false)) ? (y) : (x));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTrunc) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* val = b.Var("v", b.Zero(ty.f32()));

        auto* v = b.Load(val);
        auto* t = b.Call(ty.f32(), core::BuiltinFn::kTrunc, v);

        b.Let("val", t);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float v = 0.0f;
  float v_1 = v;
  float val = (((v_1 < 0.0f)) ? (ceil(v_1)) : (floor(v_1)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTruncVec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* val = b.Var("v", b.Splat(ty.vec3<f32>(), 2_f));

        auto* v = b.Load(val);
        auto* t = b.Call(ty.vec3<f32>(), core::BuiltinFn::kTrunc, v);

        b.Let("val", t);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float3 v = (2.0f).xxx;
  float3 v_1 = v;
  float3 val = (((v_1 < (0.0f).xxx)) ? (ceil(v_1)) : (floor(v_1)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTruncF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* val = b.Var("v", b.Zero(ty.f16()));

        auto* v = b.Load(val);
        auto* t = b.Call(ty.f16(), core::BuiltinFn::kTrunc, v);

        b.Let("val", t);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float16_t v = float16_t(0.0h);
  float16_t v_1 = v;
  float16_t val = (((v_1 < float16_t(0.0h))) ? (ceil(v_1)) : (floor(v_1)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicStore) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore,
               b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedExchange(int(16u), int(123), v_1);
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicStoreDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, var, 123_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedExchange(int(0u), int(123), v_1);
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicLoad) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedOr(int(16u), int(0), v_1);
  int x = v_1;
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicLoadDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedOr(int(0u), int(0), v_1);
  int x = v_1;
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicSub) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedAdd(int(16u), (int(0) - int(123)), v_1);
  int x = v_1;
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicSubDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, var, 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedAdd(int(0u), (int(0) - int(123)), v_1);
  int x = v_1;
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x",
              b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                     core::BuiltinFn::kAtomicCompareExchangeWeak,
                     b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i, 345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedCompareExchange(int(16u), int(123), int(345), v_1);
  int v_2 = v_1;
  atomic_compare_exchange_result_i32 x = {v_2, (v_2 == int(123))};
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageAtomicCompareExchangeWeakDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak, var, 123_i, 345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.InterlockedCompareExchange(int(0u), int(123), int(345), v_1);
  int v_2 = v_1;
  atomic_compare_exchange_result_i32 x = {v_2, (v_2 == int(123))};
}

)");
}

struct AtomicData {
    core::BuiltinFn fn;
    const char* interlock;
};
[[maybe_unused]] std::ostream& operator<<(std::ostream& out, const AtomicData& data) {
    out << data.interlock;
    return out;
}

using HlslBuiltinAtomic = HlslWriterTestWithParam<AtomicData>;
TEST_P(HlslBuiltinAtomic, IndirectAccess) {
    auto param = GetParam();
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), param.fn,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.)" + std::string(param.interlock) +
                                R"((int(16u), int(123), v_1);
  int x = v_1;
}

)");
}

TEST_P(HlslBuiltinAtomic, DirectAccess) {
    auto param = GetParam();
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), param.fn, var, 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  int v_1 = int(0);
  v.)" + std::string(param.interlock) +
                                R"((int(0u), int(123), v_1);
  int x = v_1;
}

)");
}

INSTANTIATE_TEST_SUITE_P(HlslWriterTest,
                         HlslBuiltinAtomic,
                         testing::Values(AtomicData{core::BuiltinFn::kAtomicAdd, "InterlockedAdd"},
                                         AtomicData{core::BuiltinFn::kAtomicMax, "InterlockedMax"},
                                         AtomicData{core::BuiltinFn::kAtomicMin, "InterlockedMin"},
                                         AtomicData{core::BuiltinFn::kAtomicAnd, "InterlockedAnd"},
                                         AtomicData{core::BuiltinFn::kAtomicOr, "InterlockedOr"},
                                         AtomicData{core::BuiltinFn::kAtomicXor, "InterlockedXor"},
                                         AtomicData{core::BuiltinFn::kAtomicExchange,
                                                    "InterlockedExchange"}));

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicStore) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore,
               b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = int(0);
    InterlockedExchange(v.a, int(0), v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = int(0);
  InterlockedExchange(v.a, int(123), v_3);
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicLoad) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = int(0);
    InterlockedExchange(v.a, int(0), v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = int(0);
  InterlockedOr(v.a, int(0), v_3);
  int x = v_3;
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicSub) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<u32>, read_write>(), var, 2_u), 123_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = int(0);
    InterlockedExchange(v.a, int(0), v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = int(0);
  InterlockedAdd(v.a, (int(0) - int(123)), v_3);
  int x = v_3;
  uint v_4 = 0u;
  InterlockedAdd(v.b, (0u - 123u), v_4);
  uint x_1 = v_4;
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i,
                          345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = int(0);
    InterlockedExchange(v.a, int(0), v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = int(0);
  InterlockedCompareExchange(v.a, int(123), int(345), v_3);
  int v_4 = v_3;
  atomic_compare_exchange_result_i32 x = {v_4, (v_4 == int(123))};
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

using HlslBuiltinWorkgroupAtomic = HlslWriterTestWithParam<AtomicData>;
TEST_P(HlslBuiltinWorkgroupAtomic, Access) {
    auto param = GetParam();
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");

    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), param.fn, var, 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    int v_1 = int(0);
    InterlockedExchange(v, int(0), v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_2 = int(0);
  )" + std::string(param.interlock) +
                                R"((v, int(123), v_2);
  int x = v_2;
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

INSTANTIATE_TEST_SUITE_P(HlslWriterTest,
                         HlslBuiltinWorkgroupAtomic,
                         testing::Values(AtomicData{core::BuiltinFn::kAtomicAdd, "InterlockedAdd"},
                                         AtomicData{core::BuiltinFn::kAtomicMax, "InterlockedMax"},
                                         AtomicData{core::BuiltinFn::kAtomicMin, "InterlockedMin"},
                                         AtomicData{core::BuiltinFn::kAtomicAnd, "InterlockedAnd"},
                                         AtomicData{core::BuiltinFn::kAtomicOr, "InterlockedOr"},
                                         AtomicData{core::BuiltinFn::kAtomicXor, "InterlockedXor"},
                                         AtomicData{core::BuiltinFn::kAtomicExchange,
                                                    "InterlockedExchange"}));

TEST_F(HlslWriterTest, BuiltinSignScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.f16(), core::BuiltinFn::kSign, 1_h));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float16_t x = float16_t(sign(float16_t(1.0h)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinSignVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.vec3<f32>(), core::BuiltinFn::kSign,
                          b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float3 x = float3(sign(float3(1.0f, 2.0f, 3.0f)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  DeviceMemoryBarrierWithGroupSync();
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  DeviceMemoryBarrierWithGroupSync();
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  GroupMemoryBarrierWithGroupSync();
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLevels1D) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture1D<float4> t) {
  uint2 v = (0u).xx;
  t.GetDimensions(0u, v.x, v.y);
  uint d = v.y;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLevels2D) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2D<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(0u, v.x, v.y, v.z);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLevels3D) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture3D<float4> t) {
  uint4 v = (0u).xxxx;
  t.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint d = v.w;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension1D) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureDimensions, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture1D<float4> t) {
  uint v = 0u;
  t.GetDimensions(v);
  uint d = v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension2D) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2D<float4> t) {
  uint2 v = (0u).xx;
  t.GetDimensions(v.x, v.y);
  uint2 d = v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension2dLOD) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t, 1_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2D<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(uint(int(1)), v.x, v.y, v.z);
  uint2 d = v.xy;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension3D) {
    auto* t = b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.vec3<u32>(), core::BuiltinFn::kTextureDimensions, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture3D<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v.x, v.y, v.z);
  uint3 d = v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLayers2dArray) {
    auto* t =
        b.FunctionParam("t", ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2DArray<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v.x, v.y, v.z);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLayersCubeArray) {
    auto* t = b.FunctionParam(
        "t", ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(TextureCubeArray<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v.x, v.y, v.z);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumSamples) {
    auto* t =
        b.FunctionParam("t", ty.depth_multisampled_texture(core::type::TextureDimension::k2d));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2DMS<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v.x, v.y, v.z);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_1DF32) {
    auto* t =
        b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_u);
        auto* level = b.Value(3_u);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture1D<float4> v : register(t0);
void foo() {
  int v_1 = int(1u);
  float4 x = v.Load(int2(v_1, int(3u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_2DLevelI32) {
    auto* t =
        b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<u32>(), 1_u, 2_u);
        auto* level = b.Value(3_u);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<int4> v : register(t0);
void foo() {
  int2 v_1 = int2(uint2(1u, 2u));
  int4 x = v.Load(int3(v_1, int(3u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_3DLevelU32) {
    auto* t =
        b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i);
        auto* level = b.Value(4_u);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
void foo() {
  float4 x = v.Load(int4(int3(int(1), int(2), int(3)), int(4u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_Multisampled2DI32) {
    auto* t =
        b.Var(ty.ptr(handle, ty.multisampled_texture(core::type::TextureDimension::k2d, ty.i32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* sample_idx = b.Value(3_i);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, sample_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DMS<int4> v : register(t0);
void foo() {
  int4 x = v.Load(int2(int(1), int(2)), int(3));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_Depth2DLevelF32) {
    auto* t = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<i32>(), b.Value(1_i), b.Value(2_i));
        auto* level = b.Value(3_u);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
void foo() {
  int2 v_1 = int2(int(1), int(2));
  float x = v.Load(int3(v_1, int(3u))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_Depth2DArrayLevelF32) {
    auto* t = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* array_idx = b.Value(3_u);
        auto* sample_idx = b.Value(4_i);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, array_idx, sample_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
void foo() {
  float x = v.Load(int4(int2(int(1), int(2)), int(3u), int(4))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_DepthMultisampledF32) {
    auto* t =
        b.Var(ty.ptr(handle, ty.depth_multisampled_texture(core::type::TextureDimension::k2d)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* sample_idx = b.Value(3_u);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, sample_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DMS<float4> v : register(t0);
void foo() {
  float x = v.Load(int2(int(1), int(2)), int(3u)).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureStore1D) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k1d,
                                          core::TexelFormat::kR32Float, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWTexture1D<float4> v : register(u0);
void foo() {
  v[int(1)] = float4(0.5f, 0.0f, 0.0f, 1.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureStore3D) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k3d,
                                          core::TexelFormat::kR32Float, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWTexture3D<float4> v : register(u0);
void foo() {
  v[int3(int(1), int(2), int(3))] = float4(0.5f, 0.0f, 0.0f, 1.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureStoreArray) {
    auto* t = b.Var(ty.ptr(
        handle, ty.storage_texture(core::type::TextureDimension::k2dArray,
                                   core::TexelFormat::kRgba32Float, core::Access::kReadWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, .4_f, .3_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, 3_u, value);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWTexture2DArray<float4> v : register(u0);
void foo() {
  v[int3(int2(int(1), int(2)), int(3u))] = float4(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureGatherCompare_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* depth_ref = b.Value(3_f);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords, depth_ref));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float4 x = v.GatherCmp(v_1, float2(1.0f, 2.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureGatherCompare_Depth2dOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* depth_ref = b.Value(3_f);
        auto* offset = b.Construct(ty.vec2<i32>(), b.Value(4_i), b.Value(5_i));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                                     depth_ref, offset));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.GatherCmp(v_1, v_2, 3.0f, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureGatherCompare_DepthCubeArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(2.5_f));
        auto* array_idx = b.Value(6_u);
        auto* depth_ref = b.Value(3_f);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                                     array_idx, depth_ref));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 2.5f);
  float4 x = v.GatherCmp(v_1, float4(v_2, float(6u)), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureGatherCompare_Depth2dArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(6_i);
        auto* depth_ref = b.Value(3_f);
        auto* offset = b.Construct(ty.vec2<i32>(), b.Value(4_i), b.Value(5_i));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                                     array_idx, depth_ref, offset));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  int2 v_3 = int2(int(4), int(5));
  float4 x = v.GatherCmp(v_1, float3(v_2, float(int(6))), 3.0f, v_3);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureGather_Alpha) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 3_u, t, s, coords));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<int4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  int4 x = v.GatherAlpha(v_1, float2(1.0f, 2.0f));
}

)");
}
TEST_F(HlslWriterTest, BuiltinTextureGather_RedOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(1_i, 3_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 0_u, t, s, coords, offset));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<int4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  int4 x = v.GatherRed(v_1, float2(1.0f, 2.0f), int2(int(1), int(3)));
}

)");
}
TEST_F(HlslWriterTest, BuiltinTextureGather_GreenArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(1_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 1_u, t, s, coords, array_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<int4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  int4 x = v.GatherGreen(v_1, float3(v_2, float(1u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureGather_BlueArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.i32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(1_i);
        auto* offset = b.Composite<vec2<i32>>(1_i, 2_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureGather, 2_u, t, s, coords, array_idx,
                                     offset));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<int4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  int4 x = v.GatherBlue(v_1, float3(v_2, float(int(1))), int2(int(1), int(2)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureGather_Depth) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Gather(v_1, float2(1.0f, 2.0f));
}

)");
}
TEST_F(HlslWriterTest, BuiltinTextureGather_DepthOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(3_i, 4_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords, offset));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Gather(v_1, float2(1.0f, 2.0f), int2(int(3), int(4)));
}

)");
}
TEST_F(HlslWriterTest, BuiltinTextureGather_DepthArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords, array_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.Gather(v_1, float3(v_2, float(int(4))));
}

)");
}
TEST_F(HlslWriterTest, BuiltinTextureGather_DepthArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords, array_idx, offset));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.Gather(v_1, float3(v_2, float(4u)), int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinQuantizeToF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* v = b.Var("x", b.Zero(ty.vec2<f32>()));
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kQuantizeToF16, b.Load(v)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 x = (0.0f).xx;
  float2 a = f16tof32(f32tof16(x));
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack2x16Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec2<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack2X16Float, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 u = (2.0f).xx;
  uint2 v = f32tof16(u);
  uint a = (v.x | (v.y << 16u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack2x16Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Float, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  float2 a = f16tof32(uint2((v & 65535u), (v >> 16u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack2x16Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec2<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack2X16Snorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 u = (2.0f).xx;
  int2 v = (int2(round((clamp(u, (-1.0f).xx, (1.0f).xx) * 32767.0f))) & (int(65535)).xx);
  uint a = asuint((v.x | (v.y << 16u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack2x16Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Snorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  int v = int(u);
  float2 a = clamp((float2((int2((v << 16u), v) >> (16u).xx)) / 32767.0f), (-1.0f).xx, (1.0f).xx);
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack2x16Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec2<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack2X16Unorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 u = (2.0f).xx;
  uint2 v = uint2(round((clamp(u, (0.0f).xx, (1.0f).xx) * 65535.0f)));
  uint a = (v.x | (v.y << 16u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack2x16Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Unorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  float2 a = (float2(uint2((v & 65535u), (v >> 16u))) / 65535.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4x8Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4X8Snorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float4 u = (2.0f).xxxx;
  int4 v = (int4(round((clamp(u, (-1.0f).xxxx, (1.0f).xxxx) * 127.0f))) & (int(255)).xxxx);
  uint a = asuint((v.x | ((v.y << 8u) | ((v.z << 16u) | (v.w << 24u)))));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4x8Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Snorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  int v = int(u);
  float4 a = clamp((float4((int4((v << 24u), (v << 16u), (v << 8u), v) >> (24u).xxxx)) / 127.0f), (-1.0f).xxxx, (1.0f).xxxx);
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4x8Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<f32>(), 2_f));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4X8Unorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float4 u = (2.0f).xxxx;
  uint4 v = uint4(round((clamp(u, (0.0f).xxxx, (1.0f).xxxx) * 255.0f)));
  uint a = (v.x | ((v.y << 8u) | ((v.z << 16u) | (v.w << 24u))));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4x8Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Unorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  float4 a = (float4(uint4((v & 255u), ((v >> 8u) & 255u), ((v >> 16u) & 255u), (v >> 24u))) / 255.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4xI8CorePolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<i32>(), 2_i));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XI8, b.Load(u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_pack_unpack_4x8 = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int4 u = (int(2)).xxxx;
  int4 v = u;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = ((asuint(v) & uint4((255u).xxxx)) << v_1);
  uint a = dot(v_2, uint4((1u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xI8CorePolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<i32>(), core::BuiltinFn::kUnpack4XI8, b.Load(u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_pack_unpack_4x8 = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  uint4 v_1 = uint4(24u, 16u, 8u, 0u);
  int4 v_2 = asint((uint4((v).xxxx) << v_1));
  int4 a = (v_2 >> uint4((24u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4xI8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<i32>(), 2_i));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XI8, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int4 u = (int(2)).xxxx;
  uint a = uint(pack_s8(u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xI8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<i32>(), core::BuiltinFn::kUnpack4XI8, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  int4 a = unpack_s8s32(int8_t4_packed(u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4xU8CorePolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<u32>(), 2_u));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XU8, b.Load(u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_pack_unpack_4x8 = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint4 u = (2u).xxxx;
  uint4 v = u;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = ((v & uint4((255u).xxxx)) << v_1);
  uint a = dot(v_2, uint4((1u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xU8CorePolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<u32>(), core::BuiltinFn::kUnpack4XU8, b.Load(u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_pack_unpack_4x8 = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = (uint4((v).xxxx) >> v_1);
  uint4 a = (v_2 & uint4((255u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4xU8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<u32>(), 2_u));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XU8, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint4 u = (2u).xxxx;
  uint a = uint(pack_u8(u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xU8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<u32>(), core::BuiltinFn::kUnpack4XU8, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint4 a = unpack_u8u32(uint8_t4_packed(u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinDot4U8PackedPolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kDot4U8Packed, b.Load(u), u32(3_u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_dot_4x8_packed = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = (uint4((v).xxxx) >> v_1);
  uint4 v_3 = (v_2 & uint4((255u).xxxx));
  uint4 v_4 = uint4(0u, 8u, 16u, 24u);
  uint4 v_5 = (uint4((3u).xxxx) >> v_4);
  uint a = dot(v_3, (v_5 & uint4((255u).xxxx)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinDot4U8Packed) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kDot4U8Packed, b.Load(u), u32(3_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint accumulator = 0u;
  uint a = dot4add_u8packed(u, 3u, accumulator);
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4xU8ClampPolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<u32>(), 2_u));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XU8Clamp, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint4 u = (2u).xxxx;
  uint4 v = u;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = uint4((0u).xxxx);
  uint4 v_3 = (clamp(v, v_2, uint4((255u).xxxx)) << v_1);
  uint a = dot(v_3, uint4((1u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4xI8ClampPolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<i32>(), 2_i));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XI8Clamp, b.Load(u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_pack_unpack_4x8 = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int4 u = (int(2)).xxxx;
  int4 v = u;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  int4 v_2 = int4((int(-128)).xxxx);
  uint4 v_3 = asuint(clamp(v, v_2, int4((int(127)).xxxx)));
  uint4 v_4 = ((v_3 & uint4((255u).xxxx)) << v_1);
  uint a = dot(v_4, uint4((1u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinPack4xI8Clamp) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", b.Splat(ty.vec4<i32>(), 2_i));
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kPack4XI8Clamp, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int4 u = (int(2)).xxxx;
  uint a = uint(pack_clamp_s8(u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinDot4I8PackedPolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kDot4I8Packed, b.Load(u), u32(3_u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_dot_4x8_packed = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  uint4 v_1 = uint4(24u, 16u, 8u, 0u);
  int4 v_2 = asint((uint4((v).xxxx) << v_1));
  int4 v_3 = (v_2 >> uint4((24u).xxxx));
  uint4 v_4 = uint4(24u, 16u, 8u, 0u);
  int4 v_5 = asint((uint4((3u).xxxx) << v_4));
  int a = dot(v_3, (v_5 >> uint4((24u).xxxx)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinDot4I8Packed) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.i32(), core::BuiltinFn::kDot4I8Packed, b.Load(u), u32(3_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  int accumulator = int(0);
  int a = dot4add_i8packed(u, 3u, accumulator);
}

)");
}

TEST_F(HlslWriterTest, BuiltinAsinh) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", .25_f);
        b.Let("a", b.Call(ty.f32(), core::BuiltinFn::kAsinh, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float u = 0.25f;
  float v = u;
  float a = log((v + sqrt(((v * v) + 1.0f))));
}

)");
}

TEST_F(HlslWriterTest, BuiltinAcosh) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 1.25_h);
        b.Let("a", b.Call(ty.f16(), core::BuiltinFn::kAcosh, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float16_t u = float16_t(1.25h);
  float16_t v = u;
  float16_t a = log((v + sqrt(((v * v) - float16_t(1.0h)))));
}

)");
}

TEST_F(HlslWriterTest, BuiltinAtanh) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", .25_f);
        b.Let("a", b.Call(ty.f32(), core::BuiltinFn::kAtanh, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float u = 0.25f;
  float v = u;
  float a = (log(((1.0f + v) / (1.0f - v))) * 0.5f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinSubgroupBallot) {
    auto* func = b.ComputeFunction("foo");

    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.vec4<u32>(), core::BuiltinFn::kSubgroupBallot, true));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  uint4 x = WaveActiveBallot(true);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_1d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_f);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture1D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Sample(v_1, 1.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Sample(v_1, float2(1.0f, 2.0f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Sample(v_1, float2(1.0f, 2.0f), int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.Sample(v_1, float3(v_2, float(4u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.Sample(v_1, float3(v_2, float(4u)), int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Sample(v_1, float3(1.0f, 2.0f, 3.0f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Sample(v_1, float3(1.0f, 2.0f, 3.0f), int3(int(4), int(5), int(6)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCube<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.Sample(v_1, float3(1.0f, 2.0f, 3.0f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float4 x = v.Sample(v_1, float4(v_2, float(4u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleBias(v_1, float2(1.0f, 2.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleBias(v_1, float2(1.0f, 2.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f), int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.SampleBias(v_1, float3(v_2, float(4u)), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, array_idx,
                                     3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.SampleBias(v_1, float3(v_2, float(4u)), clamp(3.0f, -16.0f, 15.9899997711181640625f), int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleBias(v_1, float3(1.0f, 2.0f, 3.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleBias(v_1, float3(1.0f, 2.0f, 3.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f), int3(int(4), int(5), int(6)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCube<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleBias(v_1, float3(1.0f, 2.0f, 3.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleBias_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleBias, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float4 x = v.SampleBias(v_1, float4(v_2, float(4u)), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompare_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float x = v.SampleCmp(v_1, float2(1.0f, 2.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompare_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float x = v.SampleCmp(v_1, float2(1.0f, 2.0f), 3.0f, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompare_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.SampleCmp(v_1, float3(v_2, float(4u)), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompare_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, array_idx, 3_f,
                               offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.SampleCmp(v_1, float3(v_2, float(4u)), 3.0f, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompare_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCube)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCube v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float x = v.SampleCmp(v_1, float3(1.0f, 2.0f, 3.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompare_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureSampleCompare, t, s, coords, array_idx, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float x = v.SampleCmp(v_1, float4(v_2, float(4u)), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompareLevel_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float x = v.SampleCmpLevelZero(v_1, float2(1.0f, 2.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompareLevel_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float x = v.SampleCmpLevelZero(v_1, float2(1.0f, 2.0f), 3.0f, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompareLevel_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, array_idx,
                               3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.SampleCmpLevelZero(v_1, float3(v_2, float(4u)), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompareLevel_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, array_idx,
                               3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.SampleCmpLevelZero(v_1, float3(v_2, float(4u)), 3.0f, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompareLevel_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCube)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCube v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float x = v.SampleCmpLevelZero(v_1, float3(1.0f, 2.0f, 3.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleCompareLevel_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleCompareLevel, t, s, coords, array_idx,
                               3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray v : register(t0);
SamplerComparisonState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float x = v.SampleCmpLevelZero(v_1, float4(v_2, float(4u)), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(5_f), b.Value(6_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float2 v_3 = float2(3.0f, 4.0f);
  float4 x = v.SampleGrad(v_1, v_2, v_3, float2(5.0f, 6.0f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(5_f), b.Value(6_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy,
                                     offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float2 v_3 = float2(3.0f, 4.0f);
  float4 x = v.SampleGrad(v_1, v_2, v_3, float2(5.0f, 6.0f), int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(5_f), b.Value(6_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, array_idx,
                                     ddx, ddy));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float2 v_3 = float2(3.0f, 4.0f);
  float2 v_4 = float2(5.0f, 6.0f);
  float4 x = v.SampleGrad(v_1, float3(v_2, float(4u)), v_3, v_4);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* ddx = b.Construct(ty.vec2<f32>(), b.Value(3_f), b.Value(4_f));
        auto* ddy = b.Construct(ty.vec2<f32>(), b.Value(5_f), b.Value(6_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, array_idx,
                                     ddx, ddy, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float2 v_3 = float2(3.0f, 4.0f);
  float2 v_4 = float2(5.0f, 6.0f);
  float4 x = v.SampleGrad(v_1, float3(v_2, float(4u)), v_3, v_4, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float3 v_3 = float3(3.0f, 4.0f, 5.0f);
  float4 x = v.SampleGrad(v_1, v_2, v_3, float3(6.0f, 7.0f, 8.0f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy,
                                     offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float3 v_3 = float3(3.0f, 4.0f, 5.0f);
  float4 x = v.SampleGrad(v_1, v_2, v_3, float3(6.0f, 7.0f, 8.0f), int3(int(4), int(5), int(6)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, ddx, ddy));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCube<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float3 v_3 = float3(3.0f, 4.0f, 5.0f);
  float4 x = v.SampleGrad(v_1, v_2, v_3, float3(6.0f, 7.0f, 8.0f));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleGrad_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);
        auto* ddx = b.Construct(ty.vec3<f32>(), b.Value(3_f), b.Value(4_f), b.Value(5_f));
        auto* ddy = b.Construct(ty.vec3<f32>(), b.Value(6_f), b.Value(7_f), b.Value(8_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleGrad, t, s, coords, array_idx,
                                     ddx, ddy));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float3 v_3 = float3(3.0f, 4.0f, 5.0f);
  float3 v_4 = float3(6.0f, 7.0f, 8.0f);
  float4 x = v.SampleGrad(v_1, float4(v_2, float(4u)), v_3, v_4);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float x = v.Sample(v_1, float2(1.0f, 2.0f)).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_Depth2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float x = v.Sample(v_1, float2(1.0f, 2.0f), int2(int(4), int(5))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_Depth2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.Sample(v_1, float3(v_2, float(4u))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_Depth2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.Sample(v_1, float3(v_2, float(4u)), int2(int(4), int(5))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSample_DepthCube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float x = v.Sample(v_1, float4(v_2, float(4u))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleLevel(v_1, float2(1.0f, 2.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleLevel(v_1, float2(1.0f, 2.0f), 3.0f, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx,
                                     3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.SampleLevel(v_1, float3(v_2, float(4u)), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx,
                                     3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float4 x = v.SampleLevel(v_1, float3(v_2, float(4u)), 3.0f, int2(int(4), int(5)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_3d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleLevel(v_1, float3(1.0f, 2.0f, 3.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_3d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex =
            b.Var(ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* offset = b.Composite<vec3<i32>>(4_i, 5_i, 6_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x",
              b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleLevel(v_1, float3(1.0f, 2.0f, 3.0f), 3.0f, int3(int(4), int(5), int(6)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCube<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float4 x = v.SampleLevel(v_1, float3(1.0f, 2.0f, 3.0f), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32())));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx,
                                     3_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray<float4> v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float4 x = v.SampleLevel(v_1, float4(v_2, float(4u)), 3.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.SampleLevel(v_1, v_2, float(int(3))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_Depth2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, 3_i, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float x = v.SampleLevel(v_1, v_2, float(int(3)), int2(int(4), int(5))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_Depth2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float3 v_3 = float3(v_2, float(4u));
  float x = v.SampleLevel(v_1, v_3, float(3u)).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_Depth2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx, 3_i,
                               offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float2 v_2 = float2(1.0f, 2.0f);
  float3 v_3 = float3(v_2, float(4u));
  float x = v.SampleLevel(v_1, v_3, float(int(3)), int2(int(4), int(5))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureSampleLevel_DepthCube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::kCubeArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec3<f32>(), b.Value(1_f), b.Value(2_f), b.Value(3_f));
        auto* array_idx = b.Value(4_u);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSampleLevel, t, s, coords, array_idx, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
TextureCubeArray v : register(t0);
SamplerState v_1 : register(s1);
void foo() {
  float3 v_2 = float3(1.0f, 2.0f, 3.0f);
  float4 v_3 = float4(v_2, float(4u));
  float x = v.SampleLevel(v_1, v_3, float(3u)).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinReflect_Vec2f32_NoPolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* vec_ty = ty.vec2<f32>();
        auto* x = b.Let("x", b.MatchWidth(1_f, vec_ty));
        auto* y = b.Let("y", b.MatchWidth(2_f, vec_ty));

        auto* c = b.Call(vec_ty, core::BuiltinFn::kReflect, x, y);
        b.Let("w", c);
        b.Return(func);
    });

    tint::hlsl::writer::Options options;
    options.polyfill_reflect_vec2_f32 = false;
    ASSERT_TRUE(Generate(options)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 x = (1.0f).xx;
  float2 y = (2.0f).xx;
  float2 w = reflect(x, y);
}

)");
}

// The generated HLSL must effectively be emitted as:
//      x + (-2.0 * dot(x,y) * y)
// Rather than:
//      x - 2.0 * dot(x,y) * y
// See crbug.com/tint/1798
TEST_F(HlslWriterTest, BuiltinReflect_Vec2f32_Polyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* vec_ty = ty.vec2<f32>();
        auto* x = b.Let("x", b.MatchWidth(1_f, vec_ty));
        auto* y = b.Let("y", b.MatchWidth(2_f, vec_ty));

        auto* c = b.Call(vec_ty, core::BuiltinFn::kReflect, x, y);
        b.Let("w", c);
        b.Return(func);
    });

    tint::hlsl::writer::Options options;
    options.polyfill_reflect_vec2_f32 = true;
    ASSERT_TRUE(Generate(options)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 x = (1.0f).xx;
  float2 y = (2.0f).xx;
  float2 w = (x + (float2(((-2.0f * dot(x, y))).xx) * y));
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
