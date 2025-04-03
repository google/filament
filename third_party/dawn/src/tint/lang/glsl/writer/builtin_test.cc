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
#include "src/tint/lang/glsl/writer/helper_test.h"

#include "gtest/gtest.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, BuiltinGeneric) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kAbs, x);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int x = 1;
  int w = abs(x);
}
)");
}

TEST_F(GlslWriterTest, BuiltinSelectScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kSelect, x, y, true);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int x = 1;
  int y = 2;
  int w = mix(x, y, true);
}
)");
}

TEST_F(GlslWriterTest, BuiltinSelectVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Construct<vec2<i32>>(1_i, 2_i));
        auto* y = b.Let("y", b.Construct<vec2<i32>>(3_i, 4_i));
        auto* cmp = b.Construct<vec2<bool>>(true, false);

        auto* c = b.Call(ty.vec2<i32>(), core::BuiltinFn::kSelect, x, y, cmp);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  ivec2 x = ivec2(1, 2);
  ivec2 y = ivec2(3, 4);
  ivec2 w = mix(x, y, bvec2(true, false));
}
)");
}

TEST_F(GlslWriterTest, BuiltinStorageBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  memoryBarrierBuffer();
  barrier();
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  memoryBarrierImage();
  barrier();
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
}
)");
}

TEST_F(GlslWriterTest, BuiltinStorageAtomicCompareExchangeWeak) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct SB {
  vec4 padding;
  int a;
  uint b;
  uint tint_pad_0;
  uint tint_pad_1;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  SB inner;
} v_1;
void main() {
  int v_2 = atomicCompSwap(v_1.inner.a, 123, 345);
  atomic_compare_exchange_result_i32 x = atomic_compare_exchange_result_i32(v_2, (v_2 == 123));
}
)");
}

TEST_F(GlslWriterTest, BuiltinStorageAtomicCompareExchangeWeakDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak, var, 123_i, 345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  int inner;
} v_1;
void main() {
  int v_2 = atomicCompSwap(v_1.inner, 123, 345);
  atomic_compare_exchange_result_i32 x = atomic_compare_exchange_result_i32(v_2, (v_2 == 123));
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupAtomicLoad) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct SB {
  vec4 padding;
  int a;
  uint b;
};

shared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v.padding = vec4(0.0f);
    atomicExchange(v.a, 0);
    atomicExchange(v.b, 0u);
  }
  barrier();
  int x = atomicOr(v.a, 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo_inner(gl_LocalInvocationIndex);
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupAtomicSub) {
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
        b.Let("y", b.Call(ty.u32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<u32>, read_write>(), var, 2_u), 123_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct SB {
  vec4 padding;
  int a;
  uint b;
};

shared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v.padding = vec4(0.0f);
    atomicExchange(v.a, 0);
    atomicExchange(v.b, 0u);
  }
  barrier();
  int x = atomicAdd(v.a, -(123));
  uint y = atomicAdd(v.b, -(123u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo_inner(gl_LocalInvocationIndex);
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupAtomicCompareExchangeWeak) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct SB {
  vec4 padding;
  int a;
  uint b;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

shared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v.padding = vec4(0.0f);
    atomicExchange(v.a, 0);
    atomicExchange(v.b, 0u);
  }
  barrier();
  int v_1 = atomicCompSwap(v.a, 123, 345);
  atomic_compare_exchange_result_i32 x = atomic_compare_exchange_result_i32(v_1, (v_1 == 123));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo_inner(gl_LocalInvocationIndex);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_FloatToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  float a = 1.0f;
  float x = a;
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_IntToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int a = 1;
  float x = intBitsToFloat(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_UintToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint a = 1u;
  float x = uintBitsToFloat(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec3UintToVec3Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Splat<vec3<u32>>(1_u));
        b.Let("x", b.Bitcast<vec3<f32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uvec3 a = uvec3(1u);
  vec3 x = uintBitsToFloat(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_FloatToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  float a = 1.0f;
  int x = floatBitsToInt(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_FloatToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  float a = 1.0f;
  uint x = floatBitsToUint(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_UintToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint a = 1u;
  int x = int(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_IntToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int a = 1;
  uint x = uint(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_I32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec2 tint_bitcast_to_f16(int src) {
  return unpackFloat2x16(uint(src));
}
void main() {
  int a = 1;
  f16vec2 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F16ToI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

int tint_bitcast_from_f16(f16vec2 src) {
  return int(packFloat2x16(src));
}
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  int x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_U32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
void main() {
  uint a = 1u;
  f16vec2 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F16ToU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

uint tint_bitcast_from_f16(f16vec2 src) {
  return packFloat2x16(src);
}
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  uint x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_F32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec2 tint_bitcast_to_f16(float src) {
  return unpackFloat2x16(floatBitsToUint(src));
}
void main() {
  float a = 1.0f;
  f16vec2 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F16ToF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

float tint_bitcast_from_f16(f16vec2 src) {
  return uintBitsToFloat(packFloat2x16(src));
}
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  float x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2I32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 v = uvec2(src);
  return f16vec4(unpackFloat2x16(v.x), unpackFloat2x16(v.y));
}
void main() {
  ivec2 a = ivec2(1, 2);
  f16vec4 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec4F16ToVec2I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<i32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

ivec2 tint_bitcast_from_f16(f16vec4 src) {
  return ivec2(uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw)));
}
void main() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf);
  ivec2 x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2U32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<u32>>(1_u, 2_u));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
void main() {
  uvec2 a = uvec2(1u, 2u);
  f16vec4 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec4F16ToVec2U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<u32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

uvec2 tint_bitcast_from_f16(f16vec4 src) {
  return uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
}
void main() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf);
  uvec2 x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec4 tint_bitcast_to_f16(vec2 src) {
  return f16vec4(unpackFloat2x16(floatBitsToUint(src).x), unpackFloat2x16(floatBitsToUint(src).y));
}
void main() {
  vec2 a = vec2(1.0f, 2.0f);
  f16vec4 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec4F16ToVec2F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<f32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

vec2 tint_bitcast_from_f16(f16vec4 src) {
  return uintBitsToFloat(uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw)));
}
void main() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf);
  vec2 x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_1d) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32());
    auto* var = b.Var("v", handle, type, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<u32>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp sampler2D f_v;
void main() {
  uint x = uvec2(textureSize(f_v, 0)).x;
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_2d_WithoutLod) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* var = b.Var("v", handle, type, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_v;
void main() {
  uvec2 x = uvec2(textureSize(f_v, 0));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_2d_WithU32Lod) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* var = b.Var("v", handle, type, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var), 3_u));
        b.Return(func);
    });

    Options opts{};
    opts.bindings.texture_builtins_from_uniform.ubo_bindingpoint_ordering = {{0, 0}};
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform f_tint_symbol_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp sampler2D f_v;
void main() {
  uvec2 x = uvec2(textureSize(f_v, int(min(3u, (v_1.inner.tint_builtin_value_0 - 1u)))));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_2dArray) {
    auto* type = ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32());
    auto* var = b.Var("v", handle, type, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_v;
void main() {
  uvec2 x = uvec2(textureSize(f_v, 0).xy);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_Storage1D) {
    auto* type = ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Float,
                                    core::Access::kRead);
    auto* var = b.Var("v", handle, type, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, r32f) uniform highp readonly image2D f_v;
void main() {
  uvec2 x = uvec2(imageSize(f_v));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_DepthMultisampled) {
    auto* type = ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d);
    auto* var = b.Var("v", handle, type, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, b.Load(var)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DMS f_v;
void main() {
  uvec2 x = uvec2(textureSize(f_v));
}
)");
}

TEST_F(GlslWriterTest, CountOneBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kCountOneBits, 1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint x = uint(bitCount(1u));
}
)");
}

TEST_F(GlslWriterTest, ExtractBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kExtractBits, 1_u, 2_u, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int v = int(min(2u, 32u));
  uint x = bitfieldExtract(1u, v, int(min(3u, (32u - min(2u, 32u)))));
}
)");
}

TEST_F(GlslWriterTest, InsertBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kInsertBits, 1_u, 2_u, 3_u, 4_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int v = int(min(3u, 32u));
  uint x = bitfieldInsert(1u, 2u, v, int(min(4u, (32u - min(3u, 32u)))));
}
)");
}

TEST_F(GlslWriterTest, TextureNumLayers_2DArray) {
    auto* var =
        b.Var("v", handle, ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32()),
              core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_v;
void main() {
  uint x = uint(textureSize(f_v, 0).z);
}
)");
}

TEST_F(GlslWriterTest, TextureNumLayers_Depth2DArray) {
    auto* var =
        b.Var("v", handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray),
              core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_v;
void main() {
  uint x = uint(textureSize(f_v, 0).z);
}
)");
}

TEST_F(GlslWriterTest, TextureNumLayers_CubeArray) {
    auto* var =
        b.Var("v", handle, ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32()),
              core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArray f_v;
void main() {
  uint x = uint(textureSize(f_v, 0).z);
}
)");
}

TEST_F(GlslWriterTest, TextureNumLayers_DepthCubeArray) {
    auto* var = b.Var("v", handle,
                      ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray),
                      core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArray f_v;
void main() {
  uint x = uint(textureSize(f_v, 0).z);
}
)");
}

TEST_F(GlslWriterTest, TextureNumLayers_Storage2DArray) {
    auto* storage_ty = ty.storage_texture(core::type::TextureDimension::k2dArray,
                                          core::TexelFormat::kRg32Float, core::Access::kRead);

    auto* var = b.Var("v", handle, storage_ty, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp readonly image2DArray f_v;
void main() {
  uint x = uint(imageSize(f_v).z);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureLoad_1DF32) {
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
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp sampler2D f_t;
void main() {
  ivec2 v = ivec2(uvec2(1u, 0u));
  vec4 x = texelFetch(f_t, v, int(3u));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureLoad_2DLevelI32) {
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp isampler2D f_t;
void main() {
  ivec2 v = ivec2(uvec2(1u, 2u));
  ivec4 x = texelFetch(f_t, v, int(3u));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureLoad_3DLevelU32) {
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t;
void main() {
  vec4 x = texelFetch(f_t, ivec3(1, 2, 3), int(4u));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureLoad_Multisampled2DI32) {
    auto* t = b.Var(ty.ptr(handle, ty.Get<core::type::MultisampledTexture>(
                                       core::type::TextureDimension::k2d, ty.i32())));
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp isampler2DMS f_t;
void main() {
  ivec4 x = texelFetch(f_t, ivec2(1, 2), 3);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureLoad_Storage2D) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k2d,
                                          core::TexelFormat::kRg32Float, core::Access::kRead)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<i32>(), b.Value(1_i), b.Value(2_i));
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp readonly image2D f_v;
void main() {
  vec4 x = imageLoad(f_v, ivec2(1, 2));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureStore1D) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k1d,
                                          core::TexelFormat::kR32Float, core::Access::kWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    Options opts;
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

layout(binding = 0, r32f) uniform highp writeonly image2D f_v;
void main() {
  imageStore(f_v, ivec2(1, 0), vec4(0.5f, 0.0f, 0.0f, 1.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureStore3D) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k3d,
                                          core::TexelFormat::kR32Float, core::Access::kWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, r32f) uniform highp writeonly image3D f_v;
void main() {
  imageStore(f_v, ivec3(1, 2, 3), vec4(0.5f, 0.0f, 0.0f, 1.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureStoreArray) {
    auto* t = b.Var(
        ty.ptr(handle, ty.storage_texture(core::type::TextureDimension::k2dArray,
                                          core::TexelFormat::kRgba32Float, core::Access::kWrite)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, .4_f, .3_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, 3_u, value);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, rgba32f) uniform highp writeonly image2DArray f_v;
void main() {
  imageStore(f_v, ivec3(ivec2(1, 2), int(3u)), vec4(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinFMA_f32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Splat(ty.vec3<f32>(), 1_f);
        auto* y = b.Splat(ty.vec3<f32>(), 2_f);
        auto* z = b.Splat(ty.vec3<f32>(), 3_f);

        b.Let("x", b.Call(ty.vec3<f32>(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  vec3 x = ((vec3(1.0f) * vec3(2.0f)) + vec3(3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinFMA_f16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Splat(ty.vec3<f16>(), 1_h);
        auto* y = b.Splat(ty.vec3<f16>(), 2_h);
        auto* z = b.Splat(ty.vec3<f16>(), 3_h);

        b.Let("x", b.Call(ty.vec3<f16>(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;
#extension GL_AMD_gpu_shader_half_float: require

void main() {
  f16vec3 x = ((f16vec3(1.0hf) * f16vec3(2.0hf)) + f16vec3(3.0hf));
}
)");
}

TEST_F(GlslWriterTest, BuiltinArrayLength) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("b"), ty.array<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Access(ty.ptr<storage, array<u32>, read_write>(), var, 0_u);
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, ary));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_SB_ssbo {
  uint b[];
} v;
void main() {
  uint x = uint(v.b.length());
}
)");
}

TEST_F(GlslWriterTest, AnyScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAny, true));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  bool x = true;
}
)");
}

TEST_F(GlslWriterTest, AllScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAll, false));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  bool x = false;
}
)");
}

TEST_F(GlslWriterTest, DotI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec4<i32>(), 2_i));
        auto* y = b.Let("y", b.Splat(ty.vec4<i32>(), 3_i));
        b.Let("z", b.Call(ty.i32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

int tint_int_dot(ivec4 x, ivec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
void main() {
  ivec4 x = ivec4(2);
  ivec4 y = ivec4(3);
  int z = tint_int_dot(x, y);
}
)");
}

TEST_F(GlslWriterTest, DotU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec2<u32>(), 2_u));
        auto* y = b.Let("y", b.Splat(ty.vec2<u32>(), 3_u));
        b.Let("z", b.Call(ty.u32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uint tint_int_dot(uvec2 x, uvec2 y) {
  return ((x.x * y.x) + (x.y * y.y));
}
void main() {
  uvec2 x = uvec2(2u);
  uvec2 y = uvec2(3u);
  uint z = tint_int_dot(x, y);
}
)");
}

TEST_F(GlslWriterTest, Modf_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* whole = b.Access<f32>(result, 1_u);
        b.Return(func, b.Add<f32>(fract, whole));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct modf_result_f32 {
  float member_0;
  float whole;
};

float foo(float value) {
  modf_result_f32 v = modf_result_f32(0.0f, 0.0f);
  v.member_0 = modf(value, v.whole);
  modf_result_f32 v_1 = v;
  return (v_1.member_0 + v_1.whole);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Modf_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.vec4<f32>()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* whole = b.Access<vec4<f32>>(result, 1_u);
        b.Return(func, b.Add<vec4<f32>>(fract, whole));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct modf_result_vec4_f32 {
  vec4 member_0;
  vec4 whole;
};

vec4 foo(vec4 value) {
  modf_result_vec4_f32 v = modf_result_vec4_f32(vec4(0.0f), vec4(0.0f));
  v.member_0 = modf(value, v.whole);
  modf_result_vec4_f32 v_1 = v;
  return (v_1.member_0 + v_1.whole);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Frexp_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* exp = b.Access<i32>(result, 1_u);
        b.Return(func, b.Add<f32>(fract, b.Convert<f32>(exp)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct frexp_result_f32 {
  float member_0;
  int member_1;
};

float foo(float value) {
  frexp_result_f32 v = frexp_result_f32(0.0f, 0);
  v.member_0 = frexp(value, v.member_1);
  frexp_result_f32 v_1 = v;
  return (v_1.member_0 + float(v_1.member_1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Frexp_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.vec4<f32>()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* exp = b.Access<vec4<i32>>(result, 1_u);
        b.Return(func, b.Add<vec4<f32>>(fract, b.Convert<vec4<f32>>(exp)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct frexp_result_vec4_f32 {
  vec4 member_0;
  ivec4 member_1;
};

vec4 foo(vec4 value) {
  frexp_result_vec4_f32 v = frexp_result_vec4_f32(vec4(0.0f), ivec4(0));
  v.member_0 = frexp(value, v.member_1);
  frexp_result_vec4_f32 v_1 = v;
  return (v_1.member_0 + vec4(v_1.member_1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, AbsScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kAbs, 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint x = 2u;
}
)");
}

TEST_F(GlslWriterTest, AbsVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.vec2<u32>(), core::BuiltinFn::kAbs, b.Splat<vec2<u32>>(2_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uvec2 x = uvec2(2u);
}
)");
}

TEST_F(GlslWriterTest, BuiltinQuantizeToF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* v = b.Var("x", b.Zero(ty.vec2<f32>()));
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kQuantizeToF16, b.Load(v)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

vec2 tint_quantize_to_f16(vec2 val) {
  return unpackHalf2x16(packHalf2x16(val));
}
void main() {
  vec2 x = vec2(0.0f);
  vec2 a = tint_quantize_to_f16(x);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureGatherCompare_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  vec4 x = textureGather(f_t_s, vec2(1.0f, 2.0f), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureGatherCompare_Depth2dOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = textureGatherOffset(f_t_s, v, 3.0f, ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureGatherCompare_DepthCubeArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray)));
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
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArrayShadow f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 2.5f);
  vec4 x = textureGather(f_t_s, vec4(v, float(6u)), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureGatherCompare_Depth2dArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.comparison_sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<f32>(), 1_f, 2_f);
        auto* array_idx = b.Value(6_i);
        auto* depth_ref = b.Value(3_f);
        auto* offset = b.Composite(ty.vec2<i32>(), 4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGatherCompare, t, s, coords,
                                     array_idx, depth_ref, offset));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec4 x = textureGatherOffset(f_t_s, vec3(vec2(1.0f, 2.0f), float(6)), 3.0f, ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureGather_Alpha) {
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp isampler2D f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  ivec4 x = textureGather(f_t_s, v, int(3u));
}
)");
}
TEST_F(GlslWriterTest, BuiltinTextureGather_RedOffset) {
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp isampler2D f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  ivec4 x = textureGatherOffset(f_t_s, v, ivec2(1, 3), int(0u));
}
)");
}
TEST_F(GlslWriterTest, BuiltinTextureGather_GreenArray) {
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp isampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec3 v_1 = vec3(v, float(1u));
  ivec4 x = textureGather(f_t_s, v_1, int(1u));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureGather_BlueArrayOffset) {
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp isampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec3 v_1 = vec3(v, float(1));
  ivec4 x = textureGatherOffset(f_t_s, v_1, ivec2(1, 2), int(2u));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureGather_Depth) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  vec4 x = textureGather(f_t_s, vec2(1.0f, 2.0f), 0.0f);
}
)");
}
TEST_F(GlslWriterTest, BuiltinTextureGather_DepthOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  vec4 x = textureGatherOffset(f_t_s, vec2(1.0f, 2.0f), 0.0f, ivec2(3, 4));
}
)");
}
TEST_F(GlslWriterTest, BuiltinTextureGather_DepthArray) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = textureGather(f_t_s, vec3(v, float(4)), 0.0f);
}
)");
}
TEST_F(GlslWriterTest, BuiltinTextureGather_DepthArrayOffset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = textureGatherOffset(f_t_s, vec3(v, float(4u)), 0.0f, ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_1d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 x = texture(f_t_s, vec2(1.0f, 0.5f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_2d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 x = texture(f_t_s, vec2(1.0f, 2.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_2d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 x = textureOffset(f_t_s, vec2(1.0f, 2.0f), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_2d_Array) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = texture(f_t_s, vec3(v, float(4u)));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_2d_Array_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = textureOffset(f_t_s, vec3(v, float(4u)), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_3d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec4 x = texture(f_t_s, vec3(1.0f, 2.0f, 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_3d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec4 x = textureOffset(f_t_s, vec3(1.0f, 2.0f, 3.0f), ivec3(4, 5, 6));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_Cube) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp samplerCube f_t_s;
void main() {
  vec4 x = texture(f_t_s, vec3(1.0f, 2.0f, 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_Cube_Array) {
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArray f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec4 x = texture(f_t_s, vec4(v, float(4u)));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  float x = texture(f_t_s, vec3(vec2(1.0f, 2.0f), 0.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_Depth2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  float x = textureOffset(f_t_s, vec3(vec2(1.0f, 2.0f), 0.0f), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_Depth2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  float x = texture(f_t_s, vec4(v, float(4u), 0.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_Depth2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), 1_f, 2_f);
        auto* array_idx = b.Value(4_u);
        auto* offset = b.Composite<vec2<i32>>(4_i, 5_i);

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureSample, t, s, coords, array_idx, offset));
        b.Return(func);
    });

    ASSERT_TRUE(Generate({}, tint::ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 v_1 = vec4(v, float(4u), 0.0f);
  vec2 v_2 = dFdx(v);
  float x = textureGradOffset(f_t_s, v_1, v_2, dFdy(v), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSample_DepthCube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray)));
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArrayShadow f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  float x = texture(f_t_s, vec4(v, float(4u)), 0.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_2d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 x = texture(f_t_s, vec2(1.0f, 2.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_2d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 x = textureOffset(f_t_s, vec2(1.0f, 2.0f), ivec2(4, 5), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_2d_Array) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = texture(f_t_s, vec3(v, float(4u)), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_2d_Array_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = textureOffset(f_t_s, vec3(v, float(4u)), ivec2(4, 5), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_3d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec4 x = texture(f_t_s, vec3(1.0f, 2.0f, 3.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_3d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec4 x = textureOffset(f_t_s, vec3(1.0f, 2.0f, 3.0f), ivec3(4, 5, 6), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_Cube) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp samplerCube f_t_s;
void main() {
  vec4 x = texture(f_t_s, vec3(1.0f, 2.0f, 3.0f), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleBias_Cube_Array) {
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArray f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec4 x = texture(f_t_s, vec4(v, float(4u)), clamp(3.0f, -16.0f, 15.9899997711181640625f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_2d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 x = textureLod(f_t_s, vec2(1.0f, 2.0f), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_2d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 x = textureLodOffset(f_t_s, vec2(1.0f, 2.0f), 3.0f, ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_2d_Array) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = textureLod(f_t_s, vec3(v, float(4u)), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_2d_Array_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 x = textureLodOffset(f_t_s, vec3(v, float(4u)), 3.0f, ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_3d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec4 x = textureLod(f_t_s, vec3(1.0f, 2.0f, 3.0f), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_3d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec4 x = textureLodOffset(f_t_s, vec3(1.0f, 2.0f, 3.0f), 3.0f, ivec3(4, 5, 6));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_Cube) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp samplerCube f_t_s;
void main() {
  vec4 x = textureLod(f_t_s, vec3(1.0f, 2.0f, 3.0f), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_Cube_Array) {
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArray f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec4 x = textureLod(f_t_s, vec4(v, float(4u)), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_Depth2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  vec3 v = vec3(vec2(1.0f, 2.0f), 0.0f);
  float x = textureLod(f_t_s, v, float(3));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_Depth2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  vec3 v = vec3(vec2(1.0f, 2.0f), 0.0f);
  float x = textureLodOffset(f_t_s, v, float(3), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_Depth2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;
#extension GL_EXT_texture_shadow_lod: require

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 v_1 = vec4(v, float(4u), 0.0f);
  float x = textureLod(f_t_s, v_1, float(3u));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_Depth2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;
#extension GL_EXT_texture_shadow_lod: require

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 v_1 = vec4(v, float(4u), 0.0f);
  float x = textureLodOffset(f_t_s, v_1, float(3), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleLevel_DepthCube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray)));
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;
#extension GL_EXT_texture_shadow_lod: require

uniform highp samplerCubeArrayShadow f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec4 v_1 = vec4(v, float(4u));
  float x = textureLod(f_t_s, v_1, 0.0f, float(3u));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_2d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec2 v_1 = vec2(3.0f, 4.0f);
  vec4 x = textureGrad(f_t_s, v, v_1, vec2(5.0f, 6.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_2d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec2 v_1 = vec2(3.0f, 4.0f);
  vec4 x = textureGradOffset(f_t_s, v, v_1, vec2(5.0f, 6.0f), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_2d_Array) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec2 v_1 = vec2(3.0f, 4.0f);
  vec2 v_2 = vec2(5.0f, 6.0f);
  vec4 x = textureGrad(f_t_s, vec3(v, float(4u)), v_1, v_2);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_2d_Array_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArray f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec2 v_1 = vec2(3.0f, 4.0f);
  vec2 v_2 = vec2(5.0f, 6.0f);
  vec4 x = textureGradOffset(f_t_s, vec3(v, float(4u)), v_1, v_2, ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_3d) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec3 v_1 = vec3(3.0f, 4.0f, 5.0f);
  vec4 x = textureGrad(f_t_s, v, v_1, vec3(6.0f, 7.0f, 8.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_3d_Offset) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler3D f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec3 v_1 = vec3(3.0f, 4.0f, 5.0f);
  vec4 x = textureGradOffset(f_t_s, v, v_1, vec3(6.0f, 7.0f, 8.0f), ivec3(4, 5, 6));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_Cube) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp samplerCube f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec3 v_1 = vec3(3.0f, 4.0f, 5.0f);
  vec4 x = textureGrad(f_t_s, v, v_1, vec3(6.0f, 7.0f, 8.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleGrad_Cube_Array) {
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArray f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  vec3 v_1 = vec3(3.0f, 4.0f, 5.0f);
  vec3 v_2 = vec3(6.0f, 7.0f, 8.0f);
  vec4 x = textureGrad(f_t_s, vec4(v, float(4u)), v_1, v_2);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompare_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  float x = texture(f_t_s, vec3(vec2(1.0f, 2.0f), 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompare_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  float x = textureOffset(f_t_s, vec3(vec2(1.0f, 2.0f), 3.0f), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompare_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  float x = texture(f_t_s, vec4(v, float(4u), 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompare_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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

    ASSERT_TRUE(Generate({}, ast::PipelineStage::kFragment)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  vec4 v_1 = vec4(v, float(4u), 3.0f);
  vec2 v_2 = dFdx(v);
  float x = textureGradOffset(f_t_s, v_1, v_2, dFdy(v), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompare_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCube)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp samplerCubeShadow f_t_s;
void main() {
  float x = texture(f_t_s, vec4(vec3(1.0f, 2.0f, 3.0f), 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompare_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray)));
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArrayShadow f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  float x = texture(f_t_s, vec4(v, float(4u)), 3.0f);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompareLevel_2d) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  float x = texture(f_t_s, vec3(vec2(1.0f, 2.0f), 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompareLevel_2d_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DShadow f_t_s;
void main() {
  float x = textureOffset(f_t_s, vec3(vec2(1.0f, 2.0f), 3.0f), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompareLevel_2d_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  float x = texture(f_t_s, vec4(v, float(4u), 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompareLevel_2d_Array_Offset) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow f_t_s;
void main() {
  vec2 v = vec2(1.0f, 2.0f);
  float x = textureGradOffset(f_t_s, vec4(v, float(4u), 3.0f), vec2(0.0f), vec2(0.0f), ivec2(4, 5));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompareLevel_Cube) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(
            ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCube)));
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

uniform highp samplerCubeShadow f_t_s;
void main() {
  float x = texture(f_t_s, vec4(vec3(1.0f, 2.0f, 3.0f), 3.0f));
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureSampleCompareLevel_Cube_Array) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(
            handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray)));
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

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460
precision highp float;
precision highp int;

uniform highp samplerCubeArrayShadow f_t_s;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  float x = texture(f_t_s, vec4(v, float(4u)), 3.0f);
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
