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

#include "gmock/gmock.h"
#include "src/tint/lang/msl/validate/validate.h"
#include "src/tint/lang/msl/writer/helper_test.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(MslWriterTest, WorkgroupAllocations_NoAllocations) {
    auto* var_a = b.Var("a", ty.ptr<workgroup, i32>());
    auto* var_b = b.Var("b", ty.ptr<workgroup, i32>());
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    // No allocations, but still needs an entry in the map.
    auto* bar = b.ComputeFunction("entry");
    b.Append(bar->Block(), [&] { b.Return(bar); });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
}
)");

    EXPECT_THAT(output_.workgroup_allocations, testing::ElementsAre());
}

TEST_F(MslWriterTest, WorkgroupAllocations) {
    auto* var_a = b.Var("a", ty.ptr<workgroup, i32>());
    auto* var_b = b.Var("b", ty.ptr<workgroup, i32>());
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* foo = b.ComputeFunction("entry");
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_a, b.Add(load_a, load_b));
        b.Return(foo);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

struct tint_module_vars_struct {
  threadgroup int* a;
  threadgroup int* b;
};

struct tint_symbol_2 {
  int tint_symbol;
  int tint_symbol_1;
};

void entry_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  if ((tint_local_index < 1u)) {
    (*tint_module_vars.a) = 0;
    (*tint_module_vars.b) = 0;
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  (*tint_module_vars.a) = as_type<int>((as_type<uint>((*tint_module_vars.a)) + as_type<uint>((*tint_module_vars.b))));
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_2* v [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=(&(*v).tint_symbol), .b=(&(*v).tint_symbol_1)};
  entry_inner(tint_local_index, tint_module_vars);
}
)");

    EXPECT_THAT(output_.workgroup_allocations, testing::ElementsAre(8u));
}

TEST_F(MslWriterTest, WorkgroupStorageSize_OverflowAfterAlign) {
    auto* var = mod.root_block->Append(b.Var<workgroup, array<u32, 0x3FFFFFFFu>>("a"));
    auto* foo = b.ComputeFunction("entry", 64_u, 1_u, 1_u);
    b.Append(foo->Block(), [&] {  //
        b.Load(b.Access<ptr<workgroup, u32>>(var, 0_u));
        b.Return(foo);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_module_vars_struct {
  threadgroup tint_array<uint, 1073741823>* a;
};

struct tint_symbol_1 {
  tint_array<uint, 1073741823> tint_symbol;
};

void entry_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint const v_1 = v;
      if ((v_1 >= 1073741823u)) {
        break;
      }
      (*tint_module_vars.a)[v_1] = 0u;
      {
        v = (v_1 + 64u);
      }
    }
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
}

[[max_total_threads_per_threadgroup(64)]]
kernel void entry(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_1* v_2 [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=(&(*v_2).tint_symbol)};
  entry_inner(tint_local_index, tint_module_vars);
}
)");

    EXPECT_EQ(output_.workgroup_info.storage_size, 0x100000000ull);
}

TEST_F(MslWriterTest, NeedsStorageBufferSizes_False) {
    auto* var = b.Var("a", ty.ptr<storage, array<u32>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* foo = b.ComputeFunction("entry");
    b.Append(foo->Block(), [&] {
        b.Store(b.Access<ptr<storage, u32>>(var, 0_u), 42_u);
        b.Return(foo);
    });

    Options options;
    options.immediate_binding_point = tint::BindingPoint{0, 30};
    options.array_length_from_constants.bindpoint_to_size_index[{0u, 0u}] = 0u;
    options.array_length_from_constants.buffer_sizes_offset = 64u;
    options.disable_robustness = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_immediate_data_struct {
  tint_array<uint4, 1> tint_storage_buffer_sizes;
};

struct tint_module_vars_struct {
  device tint_array<uint, 1>* a;
  const constant tint_immediate_data_struct* tint_immediate_data;
};

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device tint_array<uint, 1>* a [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=a};
  (*tint_module_vars.a)[0u] = 42u;
}
)");
    EXPECT_FALSE(output_.needs_storage_buffer_sizes);
}

TEST_F(MslWriterTest, NeedsStorageBufferSizes_True) {
    auto* var = b.Var("a", ty.ptr<storage, array<u32>>());
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* foo = b.ComputeFunction("entry");
    b.Append(foo->Block(), [&] {
        auto* length = b.Call<u32>(core::BuiltinFn::kArrayLength, var);
        b.Store(b.Access<ptr<storage, u32>>(var, 0_u), length);
        b.Return(foo);
    });

    Options options;
    options.immediate_binding_point = tint::BindingPoint{0, 30};
    options.array_length_from_constants.bindpoint_to_size_index[{0u, 0u}] = 0u;
    options.array_length_from_constants.buffer_sizes_offset = 64u;
    options.disable_robustness = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_immediate_data_struct {
  /* 0x0000 */ tint_array<int8_t, 64> tint_pad;
  /* 0x0040 */ tint_array<uint4, 1> tint_storage_buffer_sizes;
};

struct tint_module_vars_struct {
  device tint_array<uint, 1>* a;
  const constant tint_immediate_data_struct* tint_immediate_data;
};

struct tint_array_lengths_struct {
  uint tint_array_length_0_0;
};

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device tint_array<uint, 1>* a [[buffer(0)]], const constant tint_immediate_data_struct* tint_immediate_data [[buffer(30)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.a=a, .tint_immediate_data=tint_immediate_data};
  (*tint_module_vars.a)[0u] = tint_array_lengths_struct{.tint_array_length_0_0=((*tint_module_vars.tint_immediate_data).tint_storage_buffer_sizes[0u].x / 4u)}.tint_array_length_0_0;
}
)");
    EXPECT_TRUE(output_.needs_storage_buffer_sizes);
}

TEST_F(MslWriterTest, StripAllNames) {
    auto* str = ty.Struct(mod.symbols.New("MyStruct"), {
                                                           {mod.symbols.Register("a"), ty.i32()},
                                                           {mod.symbols.Register("b"), ty.vec4i()},
                                                       });
    auto* foo = b.Function("foo", ty.u32());
    auto* param = b.FunctionParam("param", ty.u32());
    foo->AppendParam(param);
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, param);
    });

    auto* func = b.ComputeFunction("entry");
    auto* idx = b.FunctionParam("idx", ty.u32());
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);
    func->AppendParam(idx);
    b.Append(func->Block(), [&] {  //
        auto* var = b.Var("str", ty.ptr<function>(str));
        auto* val = b.Load(var);
        mod.SetName(val, "val");
        auto* a = b.Access<i32>(val, 0_u);
        mod.SetName(a, "a");
        b.Let("let", b.Call<u32>(foo, idx));
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "tint_entry_point";
    options.strip_all_names = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_struct {
  int tint_member;
  int4 tint_member_1;
};

uint v(uint v_1) {
  return v_1;
}

void v_2(uint v_3) {
  tint_struct v_4 = {};
  uint const v_5 = v(v_3);
}

[[max_total_threads_per_threadgroup(1)]]
kernel void tint_entry_point(uint v_7 [[thread_index_in_threadgroup]]) {
  v_2(v_7);
}
)");
}

TEST_F(MslWriterTest, VertexPulling) {
    auto* ep = b.Function("entry", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    auto* attr = b.FunctionParam<vec4<f32>>("attr");
    attr->SetLocation(1);
    ep->SetParams({attr});
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, attr);
    });

    VertexPullingConfig vertex_pulling_config;
    vertex_pulling_config.pulling_group = 4u;
    vertex_pulling_config.vertex_state = {
        {{4, VertexStepMode::kVertex, {{VertexFormat::kFloat32, 0, 1}}}}};
    ArrayLengthOptions array_length_config;
    array_length_config.buffer_sizes_offset = 64u;
    array_length_config.bindpoint_to_size_index.insert({BindingPoint{0u, 1u}, 0u});
    Options options;
    options.bindings.storage.emplace(BindingPoint{4u, 0u}, BindingPoint{0u, 1u});
    options.vertex_pulling_config = std::move(vertex_pulling_config);
    options.immediate_binding_point = BindingPoint{0, 30};
    options.array_length_from_constants = std::move(array_length_config);

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_immediate_data_struct {
  /* 0x0000 */ tint_array<int8_t, 64> tint_pad;
  /* 0x0040 */ tint_array<uint4, 1> tint_storage_buffer_sizes;
};

struct tint_module_vars_struct {
  const device tint_array<uint, 1>* tint_vertex_buffer_0;
  const constant tint_immediate_data_struct* tint_immediate_data;
};

struct tint_array_lengths_struct {
  uint tint_array_length_0_1;
};

struct entry_outputs {
  float4 tint_symbol [[position]];
};

float4 entry_inner(uint tint_vertex_index, tint_module_vars_struct tint_module_vars) {
  return float4(as_type<float>((*tint_module_vars.tint_vertex_buffer_0)[min(tint_vertex_index, (tint_array_lengths_struct{.tint_array_length_0_1=((*tint_module_vars.tint_immediate_data).tint_storage_buffer_sizes[0u].x / 4u)}.tint_array_length_0_1 - 1u))]), 0.0f, 0.0f, 1.0f);
}

vertex entry_outputs entry(uint tint_vertex_index [[vertex_id]], const device tint_array<uint, 1>* tint_vertex_buffer_0 [[buffer(1)]], const constant tint_immediate_data_struct* tint_immediate_data [[buffer(30)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.tint_vertex_buffer_0=tint_vertex_buffer_0, .tint_immediate_data=tint_immediate_data};
  entry_outputs tint_wrapper_result = {};
  tint_wrapper_result.tint_symbol = entry_inner(tint_vertex_index, tint_module_vars);
  return tint_wrapper_result;
}
)");
}

TEST_F(MslWriterTest, CanGenerate_TexelBufferUnsupported) {
    auto* buffer_ty = ty.texel_buffer(core::TexelFormat::kRgba8Unorm, core::Access::kRead);
    auto* var = b.Var("buf", ty.ptr<handle>(buffer_ty));
    mod.root_block->Append(var);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Let("x", var);
        b.Return(ep);
    });

    Options options;
    options.entry_point_name = "main";
    auto result = Generate(options);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("texel buffers are not supported by the MSL backend"));
}

TEST_F(MslWriterTest, AtomicStoreMax_Supported) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.Register("a"), ty.atomic(ty.u64())},
                                         });
    auto* var = b.Var("sb", ty.ptr<storage, read_write>(sb));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, read_write>(ty.atomic(ty.u64())), var, 0_u);
        b.Call<void>(core::BuiltinFn::kAtomicStoreMax, access,
                     b.Bitcast(ty.u64(), b.Splat(ty.vec2u(), 1_u)));
        b.Return(func);
    });

    Options options;
    options.entry_point_name = "main";
    auto result = Generate(options, validate::MslVersion::kMsl_2_4);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

struct SB {
  /* 0x0000 */ atomic_ulong a;
};

struct tint_module_vars_struct {
  device SB* sb;
};

[[max_total_threads_per_threadgroup(1)]]
kernel void v(device SB* sb [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.sb=sb};
  atomic_max_explicit((&(*tint_module_vars.sb).a), as_type<ulong>(uint2(1u)), memory_order_relaxed);
}
)");
}

TEST_F(MslWriterTest, AtomicStoreMin_Supported) {
    auto* sb =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.Register("a"), ty.atomic(ty.u64())},
                                         });
    auto* var = b.Var("sb", ty.ptr<storage, read_write>(sb));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, read_write>(ty.atomic(ty.u64())), var, 0_u);
        b.Call<void>(core::BuiltinFn::kAtomicStoreMin, access,
                     b.Bitcast(ty.u64(), b.Splat(ty.vec2u(), 1_u)));
        b.Return(func);
    });

    Options options;
    options.entry_point_name = "main";
    auto result = Generate(options, validate::MslVersion::kMsl_2_4);
    ASSERT_EQ(result, Success);
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

struct SB {
  /* 0x0000 */ atomic_ulong a;
};

struct tint_module_vars_struct {
  device SB* sb;
};

[[max_total_threads_per_threadgroup(1)]]
kernel void v(device SB* sb [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.sb=sb};
  atomic_min_explicit((&(*tint_module_vars.sb).a), as_type<ulong>(uint2(1u)), memory_order_relaxed);
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
