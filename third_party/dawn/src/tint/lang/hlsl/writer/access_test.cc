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

#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, AccessArray) {
    auto* func = b.ComputeFunction("a");

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<array<f32, 3>>());
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float v[3] = (float[3])0;
  float x = v[1u];
}

)");
}

TEST_F(HlslWriterTest, AccessStruct) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::IOAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* f = b.ComputeFunction("a");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero(strct));
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 1_u)));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
  float b;
};


[numthreads(1, 1, 1)]
void a() {
  S v = (S)0;
  float x = v.b;
}

)");
}

TEST_F(HlslWriterTest, AccessVector) {
    auto* func = b.ComputeFunction("a");

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<f32>>());
        b.Let("x", b.LoadVectorElement(v, 1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float3 v = (0.0f).xxx;
  float x = v.y;
}

)");
}

TEST_F(HlslWriterTest, AccessMatrix) {
    auto* func = b.ComputeFunction("a");

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<mat4x4<f32>>());
        auto* v1 = b.Access(ty.ptr<function, vec4<f32>>(), v, 1_u);
        b.Let("x", b.LoadVectorElement(v1, 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float4x4 v = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float x = v[1u].z;
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVectorElementConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, 1_u, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int4 vec = (int(0)).xxxx;
  vec.y = int(42);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVectorElementDynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, idx, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(int idx) {
  int4 vec = (int(0)).xxxx;
  vec[min(uint(idx), 3u)] = int(42);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, AccessNested) {
    Vector members_a{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("e"), ty.array<f32, 3>(), 1u, 4u, 4u, 12u,
                                         core::IOAttributes{}),
    };
    auto* a_strct = ty.Struct(b.ir.symbols.New("A"), std::move(members_a));

    Vector members_s{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), a_strct, 2u, 8u, 8u, 16u,
                                         core::IOAttributes{}),
    };
    auto* s_strct = ty.Struct(b.ir.symbols.New("S"), std::move(members_s));

    auto* f = b.ComputeFunction("a");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero(s_strct));
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 2_u, 1_u, 1_i)));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct A {
  int d;
  float e[3];
};

struct S {
  int a;
  float b;
  A c;
};


[numthreads(1, 1, 1)]
void a() {
  S v = (S)0;
  float x = v.c.e[1u];
}

)");
}

TEST_F(HlslWriterTest, AccessSwizzle) {
    auto* f = b.ComputeFunction("a");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<f32>>());
        b.Let("b", b.Swizzle(ty.f32(), b.Load(v), {1u}));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float3 v = (0.0f).xxx;
  float b = v.y;
}

)");
}

TEST_F(HlslWriterTest, AccessSwizzleMulti) {
    auto* f = b.ComputeFunction("a");

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec4<f32>>());
        b.Let("b", b.Swizzle(ty.vec4<f32>(), b.Load(v), {3u, 2u, 1u, 0u}));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float4 v = (0.0f).xxxx;
  float4 b = v.wzyx;
}

)");
}

TEST_F(HlslWriterTest, AccessStorageVector) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
void foo() {
  float4 a = asfloat(v.Load4(0u));
  float b = asfloat(v.Load(0u));
  float c = asfloat(v.Load(4u));
  float d = asfloat(v.Load(8u));
  float e = asfloat(v.Load(12u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageVectorF16) {
    auto* var = b.Var<storage, vec4<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
void foo() {
  vector<float16_t, 4> a = v.Load<vector<float16_t, 4> >(0u);
  float16_t b = v.Load<float16_t>(0u);
  float16_t c = v.Load<float16_t>(2u);
  float16_t d = v.Load<float16_t>(4u);
  float16_t e = v.Load<float16_t>(6u);
}

)");
}

TEST_F(HlslWriterTest, AccessStorageMatrix) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
float4x4 v_1(uint offset) {
  return float4x4(asfloat(v.Load4((offset + 0u))), asfloat(v.Load4((offset + 16u))), asfloat(v.Load4((offset + 32u))), asfloat(v.Load4((offset + 48u))));
}

void foo() {
  float4x4 a = v_1(0u);
  float4 b = asfloat(v.Load4(48u));
  float c = asfloat(v.Load(24u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
typedef float3 ary_ret[5];
ary_ret v_1(uint offset) {
  float3 a[5] = (float3[5])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      a[v_3] = asfloat(v.Load3((offset + (v_3 * 16u))));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float3 v_4[5] = a;
  return v_4;
}

void foo() {
  float3 a[5] = v_1(0u);
  float3 b = asfloat(v.Load3(48u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  int a;
  float b;
};


ByteAddressBuffer v : register(t0);
SB v_1(uint offset) {
  SB v_2 = {asint(v.Load((offset + 0u))), asfloat(v.Load((offset + 4u)))};
  return v_2;
}

void foo() {
  SB a = v_1(0u);
  float b = asfloat(v.Load(4u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageNested) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("s"), ty.mat3x3<f32>()},
                                                {mod.symbols.New("t"), ty.array<vec3<f32>, 5>()},
                                            });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(),
                                                var, 1_u, 1_u, 1_u, 3_u),
                                       2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct Inner {
  float3x3 s;
  float3 t[5];
};

struct Outer {
  float x;
  Inner y;
};

struct SB {
  int a;
  Outer b;
};


ByteAddressBuffer v : register(t0);
typedef float3 ary_ret[5];
ary_ret v_1(uint offset) {
  float3 a[5] = (float3[5])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      a[v_3] = asfloat(v.Load3((offset + (v_3 * 16u))));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float3 v_4[5] = a;
  return v_4;
}

float3x3 v_5(uint offset) {
  return float3x3(asfloat(v.Load3((offset + 0u))), asfloat(v.Load3((offset + 16u))), asfloat(v.Load3((offset + 32u))));
}

Inner v_6(uint offset) {
  float3x3 v_7 = v_5((offset + 0u));
  float3 v_8[5] = v_1((offset + 48u));
  Inner v_9 = {v_7, v_8};
  return v_9;
}

Outer v_10(uint offset) {
  float v_11 = asfloat(v.Load((offset + 0u)));
  Inner v_12 = v_6((offset + 16u));
  Outer v_13 = {v_11, v_12};
  return v_13;
}

SB v_14(uint offset) {
  int v_15 = asint(v.Load((offset + 0u)));
  Outer v_16 = v_10((offset + 16u));
  SB v_17 = {v_15, v_16};
  return v_17;
}

void foo() {
  SB a = v_14(0u);
  float b = asfloat(v.Load(136u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageStoreVector) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 0_u, 2_f);
        b.StoreVectorElement(var, 1_u, 4_f);
        b.StoreVectorElement(var, 2_u, 8_f);
        b.StoreVectorElement(var, 3_u, 16_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(0u, asuint(2.0f));
  v.Store(4u, asuint(4.0f));
  v.Store(8u, asuint(8.0f));
  v.Store(12u, asuint(16.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessDirectVariable) {
    auto* var1 = b.Var<storage, vec4<f32>, core::Access::kRead>("v1");
    var1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var1);

    auto* var2 = b.Var<storage, vec4<f32>, core::Access::kRead>("v2");
    var2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(var2);

    auto* p = b.FunctionParam("x", ty.ptr<storage, vec4<f32>, core::Access::kRead>());
    auto* bar = b.Function("bar", ty.void_());
    bar->SetParams({p});
    b.Append(bar->Block(), [&] {
        b.Let("a", b.LoadVectorElement(p, 1_u));
        b.Return(bar);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(bar, var1);
        b.Call(bar, var2);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v1 : register(t0);
ByteAddressBuffer v2 : register(t1);
void bar() {
  float a = asfloat(v1.Load(4u));
}

void bar_1() {
  float a = asfloat(v2.Load(4u));
}

void foo() {
  bar();
  bar_1();
}

)");
}

TEST_F(HlslWriterTest, AccessChainFromUnnamedAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("d"), ty.u32()},
                                                      });
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Inner},
                                                });

    auto* var = b.Var("v", storage, ty.array(sb, 4), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(storage, sb, core::Access::kReadWrite), var, 2_u);
        auto* y = b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(), 1_u);
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite), y->Result(),
                                   1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  uint b = v.Load(32u);
}

)");
}

TEST_F(HlslWriterTest, AccessChainFromLetAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                      });
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Inner},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y = b.Let(
            "y", b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(), 1_u));
        auto* z = b.Let(
            "z", b.Access(ty.ptr(storage, ty.f32(), core::Access::kReadWrite), y->Result(), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  float a = asfloat(v.Load(4u));
}

)");
}

TEST_F(HlslWriterTest, AccessComplexDynamicAccessChain) {
    auto* S1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.array(S1, 3)},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.runtime_array(S2)},
                                                });

    auto* var = b.Var("sb", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* i = b.Load(b.Var("i", 4_i));
        auto* j = b.Load(b.Var("j", 1_u));
        auto* k = b.Load(b.Var("k", 2_i));
        // let x : f32 = sb.b[i].b[j].b[k];
        b.Let("x",
              b.LoadVectorElement(
                  b.Access(ty.ptr<storage, vec3<f32>, read_write>(), var, 1_u, i, 1_u, j, 1_u), k));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
void foo() {
  int i = int(4);
  uint j = 1u;
  uint v = j;
  int k = int(2);
  int v_1 = k;
  uint v_2 = 0u;
  sb.GetDimensions(v_2);
  uint v_3 = (((v_2 - 16u) / 128u) - 1u);
  uint v_4 = (min(uint(i), v_3) * 128u);
  float x = asfloat(sb.Load((((48u + v_4) + (min(v, 2u) * 32u)) + (min(uint(v_1), 2u) * 4u))));
}

)");
}

TEST_F(HlslWriterTest, AccessComplexDynamicAccessChainSplit) {
    auto* S1 = ty.Struct(mod.symbols.New("S1"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });
    auto* S2 = ty.Struct(mod.symbols.New("S2"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.array(S1, 3)},
                                                    {mod.symbols.New("c"), ty.i32()},
                                                });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.runtime_array(S2)},
                                                });

    auto* var = b.Var("sb", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* j = b.Load(b.Var("j", 1_u));
        b.Let("x", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, read_write>(), var, 1_u,
                                                4_u, 1_u, j, 1_u),
                                       2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
void foo() {
  uint j = 1u;
  uint v = 0u;
  sb.GetDimensions(v);
  float x = asfloat(sb.Load(((56u + (min(4u, (((v - 16u) / 128u) - 1u)) * 128u)) + (min(j, 2u) * 32u))));
}

)");
}

TEST_F(HlslWriterTest, AccessUniformChainFromUnnamedAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("d"), ty.u32()},
                                                      });

    tint::Vector<const core::type::StructMember*, 2> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                  ty.i32()->Size(), core::IOAttributes{}));
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("b"), Inner, 1u, 16u, 16u,
                                                  Inner->Size(), core::IOAttributes{}));
    auto* sb = ty.Struct(mod.symbols.New("SB"), members);

    auto* var = b.Var("v", uniform, ty.array(sb, 4), core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, sb, core::Access::kRead), var, 2_u);
        auto* y = b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(), 1_u);
        b.Let("b",
              b.Load(b.Access(ty.ptr(uniform, ty.u32(), core::Access::kRead), y->Result(), 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[8];
};
void foo() {
  uint b = v[5u].y;
}

)");
}

TEST_F(HlslWriterTest, AccessUniformChainFromLetAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                      });
    tint::Vector<const core::type::StructMember*, 2> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                  ty.i32()->Size(), core::IOAttributes{}));
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("b"), Inner, 1u, 16u, 16u,
                                                  Inner->Size(), core::IOAttributes{}));
    auto* sb = ty.Struct(mod.symbols.New("SB"), members);

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y =
            b.Let("y", b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(), 1_u));
        auto* z =
            b.Let("z", b.Access(ty.ptr(uniform, ty.f32(), core::Access::kRead), y->Result(), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[2];
};
void foo() {
  float a = asfloat(v[1u].x);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformScalar) {
    auto* var = b.Var<uniform, f32, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
void foo() {
  float a = asfloat(v[0u].x);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformScalarF16) {
    auto* var = b.Var<uniform, f16, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
void foo() {
  float16_t a = float16_t(f16tof32(v[0u].x));
}

)");
}

TEST_F(HlslWriterTest, AccessUniformVector) {
    auto* var = b.Var<uniform, vec4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
void foo() {
  float4 a = asfloat(v[0u]);
  float b = asfloat(v[0u].x);
  float c = asfloat(v[0u].y);
  float d = asfloat(v[0u].z);
  float e = asfloat(v[0u].w);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformVectorF16) {
    auto* var = b.Var<uniform, vec4<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Var("x", 1_u);
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, b.Load(x)));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v_1 = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v_1 & mask));
  float2 t_high = f16tof32(((v_1 >> shift) & mask));
  float16_t v_2 = float16_t(t_low.x);
  float16_t v_3 = float16_t(t_high.x);
  float16_t v_4 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_2, v_3, v_4, float16_t(t_high.y));
}

void foo() {
  uint x = 1u;
  vector<float16_t, 4> a = tint_bitcast_to_f16(v[0u].xy);
  float16_t b = float16_t(f16tof32(v[0u].x));
  uint v_5 = (min(x, 3u) * 2u);
  uint v_6 = v[(v_5 / 16u)][((v_5 % 16u) / 4u)];
  float16_t c = float16_t(f16tof32((v_6 >> ((((v_5 % 4u) == 0u)) ? (0u) : (16u)))));
  float16_t d = float16_t(f16tof32(v[0u].y));
  float16_t e = float16_t(f16tof32((v[0u].y >> 16u)));
}

)");
}

TEST_F(HlslWriterTest, AccessUniformMatrix) {
    auto* var = b.Var<uniform, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[4];
};
float4x4 v_1(uint start_byte_offset) {
  return float4x4(asfloat(v[(start_byte_offset / 16u)]), asfloat(v[((16u + start_byte_offset) / 16u)]), asfloat(v[((32u + start_byte_offset) / 16u)]), asfloat(v[((48u + start_byte_offset) / 16u)]));
}

void foo() {
  float4x4 a = v_1(0u);
  float4 b = asfloat(v[3u]);
  float c = asfloat(v[1u].z);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformMatrix2x3) {
    auto* var = b.Var<uniform, mat2x3<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[2];
};
float2x3 v_1(uint start_byte_offset) {
  return float2x3(asfloat(v[(start_byte_offset / 16u)].xyz), asfloat(v[((16u + start_byte_offset) / 16u)].xyz));
}

void foo() {
  float2x3 a = v_1(0u);
  float3 b = asfloat(v[1u].xyz);
  float c = asfloat(v[1u].z);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformMat2x3F16) {
    auto* var = b.Var<uniform, mat2x3<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr(uniform, ty.vec3<f16>()), var, 1_u)));
        b.Let("c", b.LoadVectorElement(b.Access(ty.ptr(uniform, ty.vec3<f16>()), var, 1_u), 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v_1 = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v_1 & mask));
  float2 t_high = f16tof32(((v_1 >> shift) & mask));
  float16_t v_2 = float16_t(t_low.x);
  float16_t v_3 = float16_t(t_high.x);
  float16_t v_4 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_2, v_3, v_4, float16_t(t_high.y));
}

matrix<float16_t, 2, 3> v_5(uint start_byte_offset) {
  uint4 v_6 = v[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = v[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_7, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz);
}

void foo() {
  matrix<float16_t, 2, 3> a = v_5(0u);
  vector<float16_t, 3> b = tint_bitcast_to_f16(v[0u].zw).xyz;
  float16_t c = float16_t(f16tof32(v[0u].w));
}

)");
}
TEST_F(HlslWriterTest, AccessUniformMatrix3x2) {
    auto* var = b.Var<uniform, mat3x2<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u), 1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[2];
};
float3x2 v_1(uint start_byte_offset) {
  uint4 v_2 = v[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = v[((8u + start_byte_offset) / 16u)];
  float2 v_5 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint4 v_6 = v[((16u + start_byte_offset) / 16u)];
  return float3x2(v_3, v_5, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))));
}

void foo() {
  float3x2 a = v_1(0u);
  float2 b = asfloat(v[0u].zw);
  float c = asfloat(v[0u].w);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformMatrix2x2) {
    auto* var = b.Var<uniform, mat2x2<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec2<f32>, core::Access::kRead>(), var, 1_u), 1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
float2x2 v_1(uint start_byte_offset) {
  uint4 v_2 = v[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = v[((8u + start_byte_offset) / 16u)];
  return float2x2(v_3, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

void foo() {
  float2x2 a = v_1(0u);
  float2 b = asfloat(v[0u].zw);
  float c = asfloat(v[0u].w);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformMatrix2x2F16) {
    auto* var = b.Var<uniform, mat2x2<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec2<f16>, core::Access::kRead>(), var, 1_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec2<f16>, core::Access::kRead>(), var, 1_u), 1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v_1 = src;
  float t_low = f16tof32((v_1 & 65535u));
  float t_high = f16tof32(((v_1 >> 16u) & 65535u));
  float16_t v_2 = float16_t(t_low);
  return vector<float16_t, 2>(v_2, float16_t(t_high));
}

matrix<float16_t, 2, 2> v_3(uint start_byte_offset) {
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(v[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return matrix<float16_t, 2, 2>(v_4, tint_bitcast_to_f16(v[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}

void foo() {
  matrix<float16_t, 2, 2> a = v_3(0u);
  vector<float16_t, 2> b = tint_bitcast_to_f16(v[0u].y);
  float16_t c = float16_t(f16tof32((v[0u].y >> 16u)));
}

)");
}

TEST_F(HlslWriterTest, AccessUniformArray) {
    auto* var = b.Var<uniform, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[5];
};
typedef float3 ary_ret[5];
ary_ret v_1(uint start_byte_offset) {
  float3 a[5] = (float3[5])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      a[v_3] = asfloat(v[((start_byte_offset + (v_3 * 16u)) / 16u)].xyz);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float3 v_4[5] = a;
  return v_4;
}

void foo() {
  float3 a[5] = v_1(0u);
  float3 b = asfloat(v[3u].xyz);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformArrayF16) {
    auto* var = b.Var<uniform, array<vec3<f16>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f16>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[3];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v_1 = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v_1 & mask));
  float2 t_high = f16tof32(((v_1 >> shift) & mask));
  float16_t v_2 = float16_t(t_low.x);
  float16_t v_3 = float16_t(t_high.x);
  float16_t v_4 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_2, v_3, v_4, float16_t(t_high.y));
}

typedef vector<float16_t, 3> ary_ret[5];
ary_ret v_5(uint start_byte_offset) {
  vector<float16_t, 3> a[5] = (vector<float16_t, 3>[5])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 5u)) {
        break;
      }
      uint4 v_8 = v[((start_byte_offset + (v_7 * 8u)) / 16u)];
      a[v_7] = tint_bitcast_to_f16(((((((start_byte_offset + (v_7 * 8u)) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz;
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  vector<float16_t, 3> v_9[5] = a;
  return v_9;
}

void foo() {
  vector<float16_t, 3> a[5] = v_5(0u);
  vector<float16_t, 3> b = tint_bitcast_to_f16(v[1u].zw).xyz;
}

)");
}

TEST_F(HlslWriterTest, AccessUniformArrayWhichCanHaveSizesOtherThenFive) {
    auto* var = b.Var<uniform, array<vec3<f32>, 42>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[42];
};
typedef float3 ary_ret[42];
ary_ret v_1(uint start_byte_offset) {
  float3 a[42] = (float3[42])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 42u)) {
        break;
      }
      a[v_3] = asfloat(v[((start_byte_offset + (v_3 * 16u)) / 16u)].xyz);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float3 v_4[42] = a;
  return v_4;
}

void foo() {
  float3 a[42] = v_1(0u);
  float3 b = asfloat(v[3u].xyz);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  int a;
  float b;
};


cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
SB v_1(uint start_byte_offset) {
  SB v_2 = {asint(v[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(v[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)])};
  return v_2;
}

void foo() {
  SB a = v_1(0u);
  float b = asfloat(v[0u].y);
}

)");
}

TEST_F(HlslWriterTest, AccessUniformStructF16) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f16()},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, f16, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  int a;
  float16_t b;
};


cbuffer cbuffer_v : register(b0) {
  uint4 v[1];
};
SB v_1(uint start_byte_offset) {
  int v_2 = asint(v[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  uint v_3 = v[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  SB v_4 = {v_2, float16_t(f16tof32((v_3 >> (((((4u + start_byte_offset) % 4u) == 0u)) ? (0u) : (16u)))))};
  return v_4;
}

void foo() {
  SB a = v_1(0u);
  float16_t b = float16_t(f16tof32(v[0u].y));
}

)");
}

TEST_F(HlslWriterTest, AccessUniformStructNested) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("s"), ty.mat3x3<f32>()},
                                                {mod.symbols.New("t"), ty.array<vec3<f32>, 5>()},
                                            });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(),
                                                var, 1_u, 1_u, 1_u, 3_u),
                                       2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct Inner {
  float3x3 s;
  float3 t[5];
};

struct Outer {
  float x;
  Inner y;
};

struct SB {
  int a;
  Outer b;
};


cbuffer cbuffer_v : register(b0) {
  uint4 v[10];
};
typedef float3 ary_ret[5];
ary_ret v_1(uint start_byte_offset) {
  float3 a[5] = (float3[5])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      a[v_3] = asfloat(v[((start_byte_offset + (v_3 * 16u)) / 16u)].xyz);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float3 v_4[5] = a;
  return v_4;
}

float3x3 v_5(uint start_byte_offset) {
  return float3x3(asfloat(v[(start_byte_offset / 16u)].xyz), asfloat(v[((16u + start_byte_offset) / 16u)].xyz), asfloat(v[((32u + start_byte_offset) / 16u)].xyz));
}

Inner v_6(uint start_byte_offset) {
  float3x3 v_7 = v_5(start_byte_offset);
  float3 v_8[5] = v_1((48u + start_byte_offset));
  Inner v_9 = {v_7, v_8};
  return v_9;
}

Outer v_10(uint start_byte_offset) {
  float v_11 = asfloat(v[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  Inner v_12 = v_6((16u + start_byte_offset));
  Outer v_13 = {v_11, v_12};
  return v_13;
}

SB v_14(uint start_byte_offset) {
  int v_15 = asint(v[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  Outer v_16 = v_10((16u + start_byte_offset));
  SB v_17 = {v_15, v_16};
  return v_17;
}

void foo() {
  SB a = v_14(0u);
  float b = asfloat(v[8u].z);
}

)");
}

TEST_F(HlslWriterTest, AccessStoreScalar) {
    auto* var = b.Var<storage, f32, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, 2_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(0u, asuint(2.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreScalarF16) {
    auto* var = b.Var<storage, f16, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, 2_h);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store<float16_t>(0u, float16_t(2.0h));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVectorElement) {
    auto* var = b.Var<storage, vec3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 1_u, 2_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(4u, asuint(2.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVectorElementF16) {
    auto* var = b.Var<storage, vec3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 1_u, 2_h);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store<float16_t>(2u, float16_t(2.0h));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVector) {
    auto* var = b.Var<storage, vec3<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Composite(ty.vec3<f32>(), 2_f, 3_f, 4_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store3(0u, asuint(float3(2.0f, 3.0f, 4.0f)));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVectorF16) {
    auto* var = b.Var<storage, vec3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Composite(ty.vec3<f16>(), 2_h, 3_h, 4_h));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store<vector<float16_t, 3> >(0u, vector<float16_t, 3>(float16_t(2.0h), float16_t(3.0h), float16_t(4.0h)));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreMatrixElement) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(
            b.Access(ty.ptr<storage, vec4<f32>, core::Access::kReadWrite>(), var, 1_u), 2_u, 5_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(24u, asuint(5.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreMatrixElementF16) {
    auto* var = b.Var<storage, mat3x2<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(
            b.Access(ty.ptr<storage, vec2<f16>, core::Access::kReadWrite>(), var, 2_u), 1_u, 5_h);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store<float16_t>(10u, float16_t(5.0h));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreMatrixColumn) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, vec4<f32>, core::Access::kReadWrite>(), var, 1_u),
                b.Splat<vec4<f32>>(5_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store4(16u, asuint((5.0f).xxxx));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreMatrixColumnF16) {
    auto* var = b.Var<storage, mat2x3<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, vec3<f16>, core::Access::kReadWrite>(), var, 1_u),
                b.Splat<vec3<f16>>(5_h));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store<vector<float16_t, 3> >(8u, (float16_t(5.0h)).xxx);
}

)");
}

TEST_F(HlslWriterTest, AccessStoreMatrix) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero<mat4x4<f32>>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float4x4 obj) {
  v.Store4((offset + 0u), asuint(obj[0u]));
  v.Store4((offset + 16u), asuint(obj[1u]));
  v.Store4((offset + 32u), asuint(obj[2u]));
  v.Store4((offset + 48u), asuint(obj[3u]));
}

void foo() {
  v_1(0u, float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreMatrixF16) {
    auto* var = b.Var<storage, mat4x4<f16>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(var, b.Zero<mat4x4<f16>>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, matrix<float16_t, 4, 4> obj) {
  v.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  v.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  v.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  v.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

void foo() {
  v_1(0u, matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreArrayElement) {
    auto* var = b.Var<storage, array<f32, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 3_u), 1_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(12u, asuint(1.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreArrayElementF16) {
    auto* var = b.Var<storage, array<f16, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f16, core::Access::kReadWrite>(), var, 3_u), 1_h);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store<float16_t>(6u, float16_t(1.0h));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 5>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Let("ary", b.Zero<array<vec3<f32>, 5>>());
        b.Store(var, ary);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float3 obj[5]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      v.Store3((offset + (v_3 * 16u)), asuint(obj[v_3]));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void foo() {
  float3 ary[5] = (float3[5])0;
  v_1(0u, ary);
}

)");
}

TEST_F(HlslWriterTest, AccessStoreStructMember) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 1_u), 3_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(4u, asuint(3.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreStructMemberF16) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f16()},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f16, core::Access::kReadWrite>(), var, 1_u), 3_h);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store<float16_t>(4u, float16_t(3.0h));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreStructNested) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("s"), ty.mat3x3<f32>()},
                                                {mod.symbols.New("t"), ty.array<vec3<f32>, 5>()},
                                            });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Store(b.Access(ty.ptr<storage, f32, core::Access::kReadWrite>(), var, 1_u, 0_u), 2_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(16u, asuint(2.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessStoreStruct) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("s"), ty.f32()},
                                                          {mod.symbols.New("t"), ty.vec3<f32>()},
                                                      });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Let("s", b.Zero(SB));
        b.Store(var, s);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct Inner {
  float s;
  float3 t;
};

struct Outer {
  float x;
  Inner y;
};

struct SB {
  int a;
  Outer b;
};


RWByteAddressBuffer v : register(u0);
void v_1(uint offset, Inner obj) {
  v.Store((offset + 0u), asuint(obj.s));
  v.Store3((offset + 16u), asuint(obj.t));
}

void v_2(uint offset, Outer obj) {
  v.Store((offset + 0u), asuint(obj.x));
  Inner v_3 = obj.y;
  v_1((offset + 16u), v_3);
}

void v_4(uint offset, SB obj) {
  v.Store((offset + 0u), asuint(obj.a));
  Outer v_5 = obj.b;
  v_2((offset + 16u), v_5);
}

void foo() {
  SB s = (SB)0;
  v_4(0u, s);
}

)");
}

TEST_F(HlslWriterTest, AccessStoreStructComplex) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("s"), ty.mat3x3<f32>()},
                                                {mod.symbols.New("t"), ty.array<vec3<f32>, 5>()},
                                            });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", storage, SB, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Let("s", b.Zero(SB));
        b.Store(var, s);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct Inner {
  float3x3 s;
  float3 t[5];
};

struct Outer {
  float x;
  Inner y;
};

struct SB {
  int a;
  Outer b;
};


RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float3 obj[5]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      v.Store3((offset + (v_3 * 16u)), asuint(obj[v_3]));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void v_4(uint offset, float3x3 obj) {
  v.Store3((offset + 0u), asuint(obj[0u]));
  v.Store3((offset + 16u), asuint(obj[1u]));
  v.Store3((offset + 32u), asuint(obj[2u]));
}

void v_5(uint offset, Inner obj) {
  v_4((offset + 0u), obj.s);
  float3 v_6[5] = obj.t;
  v_1((offset + 48u), v_6);
}

void v_7(uint offset, Outer obj) {
  v.Store((offset + 0u), asuint(obj.x));
  Inner v_8 = obj.y;
  v_5((offset + 16u), v_8);
}

void v_9(uint offset, SB obj) {
  v.Store((offset + 0u), asuint(obj.a));
  Outer v_10 = obj.b;
  v_7((offset + 16u), v_10);
}

void foo() {
  SB s = (SB)0;
  v_9(0u, s);
}

)");
}

TEST_F(HlslWriterTest, AccessChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.vec3<f32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(storage, ty.vec3<f32>(), core::Access::kReadWrite), var, 1_u);
        b.Let("b", b.LoadVectorElement(x, 1_u));
        b.Let("c", b.LoadVectorElement(x, 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  float b = asfloat(v.Load(20u));
  float c = asfloat(v.Load(24u));
}

)");
}

TEST_F(HlslWriterTest, UniformAccessChainReused) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("c"), ty.f32()},
                                                    {mod.symbols.New("d"), ty.vec3<f32>()},
                                                });

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, ty.vec3<f32>(), core::Access::kRead), var, 1_u);
        b.Let("b", b.LoadVectorElement(x, 1_u));
        b.Let("c", b.LoadVectorElement(x, 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v : register(b0) {
  uint4 v[2];
};
void foo() {
  float b = asfloat(v[1u].y);
  float c = asfloat(v[1u].z);
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
