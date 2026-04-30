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

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, ArrayLengthDirect) {
    auto* sb = b.Var("sb", ty.ptr<storage, array<i32>>());
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
void main() {
  uint v = 0u;
  sb.GetDimensions(v);
  uint len = (v / 4u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLengthInStruct) {
    auto* SB =
        ty.Struct(mod.symbols.New("SB"), {
                                             {mod.symbols.New("x"), ty.i32()},
                                             {mod.symbols.New("arr"), ty.runtime_array(ty.i32())},
                                         });

    auto* sb = b.Var("sb", ty.ptr(storage, SB));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                            b.Access(ty.ptr<storage, array<i32>>(), sb, 1_u)));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
void main() {
  uint v = 0u;
  sb.GetDimensions(v);
  uint len = ((v - 4u) / 4u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLengthOfStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("f"), ty.f32()},
                                                });

    auto* sb = b.Var("sb", ty.ptr(storage, ty.runtime_array(SB), core::Access::kRead));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer sb : register(t0);
void main() {
  uint v = 0u;
  sb.GetDimensions(v);
  uint len = (v / 4u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLengthArrayOfArrayOfStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("f"), ty.f32()},
                                                });
    auto* sb = b.Var("sb", ty.ptr(storage, ty.runtime_array(ty.array(SB, 4))));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
void main() {
  uint v = 0u;
  sb.GetDimensions(v);
  uint len = (v / 16u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLengthMultiple) {
    auto* sb = b.Var("sb", ty.ptr<storage, array<i32>>());
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Let("b", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Let("c", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
void main() {
  uint v = 0u;
  sb.GetDimensions(v);
  uint a = (v / 4u);
  uint v_1 = 0u;
  sb.GetDimensions(v_1);
  uint b = (v_1 / 4u);
  uint v_2 = 0u;
  sb.GetDimensions(v_2);
  uint c = (v_2 / 4u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLengthMultipleStorageBuffers) {
    auto* SB1 =
        ty.Struct(mod.symbols.New("SB1"), {
                                              {mod.symbols.New("x"), ty.i32()},
                                              {mod.symbols.New("arr1"), ty.runtime_array(ty.i32())},
                                          });
    auto* SB2 = ty.Struct(mod.symbols.New("SB2"),
                          {
                              {mod.symbols.New("x"), ty.i32()},
                              {mod.symbols.New("arr2"), ty.runtime_array(ty.vec4f())},
                          });
    auto* sb1 = b.Var("sb1", ty.ptr(storage, SB1));
    sb1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb1);

    auto* sb2 = b.Var("sb2", ty.ptr(storage, SB2));
    sb2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(sb2);

    auto* sb3 = b.Var("sb3", ty.ptr(storage, ty.runtime_array(ty.i32())));
    sb3->SetBindingPoint(0, 2);
    b.ir.root_block->Append(sb3);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("len1", b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                             b.Access(ty.ptr<storage, array<i32>>(), sb1, 1_u)));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kArrayLength,
                             b.Access(ty.ptr<storage, array<vec4<f32>>>(), sb2, 1_u)));
        b.Let("len3", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb3));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb1 : register(u0);
RWByteAddressBuffer sb2 : register(u1);
RWByteAddressBuffer sb3 : register(u2);
void main() {
  uint v = 0u;
  sb1.GetDimensions(v);
  uint len1 = ((v - 4u) / 4u);
  uint v_1 = 0u;
  sb2.GetDimensions(v_1);
  uint len2 = ((v_1 - 16u) / 16u);
  uint v_2 = 0u;
  sb3.GetDimensions(v_2);
  uint len3 = (v_2 / 4u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLength_Robustness) {
    auto* dst = b.Var("dest", ty.ptr(storage, ty.array<u32>()));
    dst->SetBindingPoint(0, 1);
    b.ir.root_block->Append(dst);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, ty.u32()), dst, 0_u);
        b.Store(access, 123_u);
        b.Return(func);
    });

    Options options;
    options.entry_point_name = "main";
    options.disable_robustness = false;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer dest : register(u1);
void main() {
  uint v = 0u;
  dest.GetDimensions(v);
  dest.Store((0u + (min(0u, ((v / 4u) - 1u)) * 4u)), 123u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLength_RobustnessAndArrayLengthFromUniform) {
    auto* dst = b.Var("dest", ty.ptr(storage, ty.array<u32>()));
    dst->SetBindingPoint(0, 1);
    b.ir.root_block->Append(dst);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, ty.u32()), dst, 0_u);
        b.Store(access, 123_u);
        b.Return(func);
    });

    Options options;
    options.entry_point_name = "main";
    options.disable_robustness = false;
    options.array_length_from_uniform.ubo_binding = {30, 0};
    options.array_length_from_uniform.bindpoint_to_size_index[{0, 1}] = 0;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_array_lengths_struct {
  uint tint_array_length_0_1;
};


RWByteAddressBuffer dest : register(u1);
cbuffer cbuffer_tint_storage_buffer_sizes : register(b0, space30) {
  uint4 tint_storage_buffer_sizes[1];
};
void main() {
  tint_array_lengths_struct v = {(tint_storage_buffer_sizes[0u].x / 4u)};
  dest.Store((0u + (min(0u, (v.tint_array_length_0_1 - 1u)) * 4u)), 123u);
}

)");
}

TEST_F(HlslWriterTest, ArrayLength_RobustnessAndArrayLengthFromImmediates) {
    auto* dst = b.Var("dest", ty.ptr(storage, ty.array<u32>()));
    dst->SetBindingPoint(0, 1);
    b.ir.root_block->Append(dst);
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, ty.u32()), dst, 0_u);
        b.Store(access, 123_u);
        b.Return(func);
    });

    Options options;
    options.entry_point_name = "main";
    options.disable_robustness = false;
    options.immediate_binding_point = BindingPoint{0, 30};
    options.array_length_from_uniform.buffer_sizes_offset = 16;  // Non-zero offset
    options.array_length_from_uniform.bindpoint_to_size_index[{0, 1}] = 0;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_array_lengths_struct {
  uint tint_array_length_0_1;
};


RWByteAddressBuffer dest : register(u1);
cbuffer cbuffer_tint_immediate_data : register(b30) {
  uint4 tint_immediate_data[2];
};
void main() {
  tint_array_lengths_struct v = {(tint_immediate_data[1u].x / 4u)};
  dest.Store((0u + (min(0u, (v.tint_array_length_0_1 - 1u)) * 4u)), 123u);
}

)");
}

// Test: Both ArrayLength AND ArrayOffset use immediates
TEST_F(HlslWriterTest, ArrayLengthAndOffset_BothFromImmediates) {
    auto* sb = b.Var("sb", ty.ptr(storage, ty.array<u32>()));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        // Test arrayLength
        auto* length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb);
        // Test storage buffer access (uses dynamic offset)
        auto* access = b.Access(ty.ptr(storage, ty.u32()), sb, 0_u);
        auto* value = b.Load(access);
        b.Let("len", length);
        b.Let("val", value);
        b.Return(func);
    });

    Options options{};
    options.immediate_binding_point = BindingPoint{0, 30};
    // ArrayLength uses immediate at offset 0 (first vec4)
    options.array_length_from_uniform.buffer_sizes_offset = 0;
    options.array_length_from_uniform.bindpoint_to_size_index[{0, 0}] = 0;
    // ArrayOffset uses immediate at offset 16 (second vec4, properly aligned)
    options.array_offset_from_uniform.buffer_offsets_offset = 16;
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 0}] = 0;

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_array_lengths_struct {
  uint tint_array_length_0_0;
};


RWByteAddressBuffer sb : register(u0);
cbuffer cbuffer_tint_immediate_data : register(b30) {
  uint4 tint_immediate_data[2];
};
[numthreads(1, 1, 1)]
void main() {
  tint_array_lengths_struct v = {(tint_immediate_data[0u].x / 4u)};
  uint len = v.tint_array_length_0_0;
  uint val = sb.Load(((0u + (min(0u, (v.tint_array_length_0_0 - 1u)) * 4u)) + tint_immediate_data[1u].x));
}

)");
}

// Test: ArrayLength from immediates, ArrayOffset from UBO (fallback)
TEST_F(HlslWriterTest, ArrayLengthFromImmediates_ArrayOffsetFromUBO) {
    auto* sb = b.Var("sb", ty.ptr(storage, ty.array<u32>()));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb);
        auto* access = b.Access(ty.ptr(storage, ty.u32()), sb, 0_u);
        auto* value = b.Load(access);
        b.Let("len", length);
        b.Let("val", value);
        b.Return(func);
    });

    Options options{};
    options.immediate_binding_point = BindingPoint{0, 30};
    // ArrayLength uses immediate
    options.array_length_from_uniform.buffer_sizes_offset = 0;
    options.array_length_from_uniform.bindpoint_to_size_index[{0, 0}] = 0;
    // ArrayOffset uses UBO (no buffer_offsets_offset set)
    options.array_offset_from_uniform.ubo_binding = {30, 1};
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 0}] = 0;

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_array_lengths_struct {
  uint tint_array_length_0_0;
};


RWByteAddressBuffer sb : register(u0);
cbuffer cbuffer_tint_immediate_data : register(b30) {
  uint4 tint_immediate_data[1];
};
cbuffer cbuffer_tint_storage_buffer_dynamic_offsets : register(b1, space30) {
  uint4 tint_storage_buffer_dynamic_offsets[1];
};
[numthreads(1, 1, 1)]
void main() {
  tint_array_lengths_struct v = {(tint_immediate_data[0u].x / 4u)};
  uint len = v.tint_array_length_0_0;
  uint val = sb.Load(((0u + (min(0u, (v.tint_array_length_0_0 - 1u)) * 4u)) + tint_storage_buffer_dynamic_offsets[0u].x));
}

)");
}

// Test: ArrayLength from UBO (fallback), ArrayOffset from immediates
TEST_F(HlslWriterTest, ArrayLengthFromUBO_ArrayOffsetFromImmediates) {
    auto* sb = b.Var("sb", ty.ptr(storage, ty.array<u32>()));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb);
        auto* access = b.Access(ty.ptr(storage, ty.u32()), sb, 0_u);
        auto* value = b.Load(access);
        b.Let("len", length);
        b.Let("val", value);
        b.Return(func);
    });

    Options options{};
    options.immediate_binding_point = BindingPoint{0, 30};
    // ArrayLength uses UBO (no buffer_sizes_offset set)
    options.array_length_from_uniform.ubo_binding = {30, 0};
    options.array_length_from_uniform.bindpoint_to_size_index[{0, 0}] = 0;
    // ArrayOffset uses immediate
    options.array_offset_from_uniform.buffer_offsets_offset = 0;
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 0}] = 0;

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_array_lengths_struct {
  uint tint_array_length_0_0;
};


RWByteAddressBuffer sb : register(u0);
cbuffer cbuffer_tint_immediate_data : register(b30) {
  uint4 tint_immediate_data[1];
};
cbuffer cbuffer_tint_storage_buffer_sizes : register(b0, space30) {
  uint4 tint_storage_buffer_sizes[1];
};
[numthreads(1, 1, 1)]
void main() {
  tint_array_lengths_struct v = {(tint_storage_buffer_sizes[0u].x / 4u)};
  uint len = v.tint_array_length_0_0;
  uint val = sb.Load(((0u + (min(0u, (v.tint_array_length_0_0 - 1u)) * 4u)) + tint_immediate_data[0u].x));
}

)");
}

// Test: Neither uses immediates - default UBO behavior for both
TEST_F(HlslWriterTest, ArrayLengthAndOffset_BothFromUBO) {
    auto* sb = b.Var("sb", ty.ptr(storage, ty.array<u32>()));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* length = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, sb);
        auto* access = b.Access(ty.ptr(storage, ty.u32()), sb, 0_u);
        auto* value = b.Load(access);
        b.Let("len", length);
        b.Let("val", value);
        b.Return(func);
    });

    Options options{};
    options.disable_robustness = true;  // Disable robustness to avoid bounds checking
    // Both use UBO (no immediate offsets set)
    options.array_length_from_uniform.ubo_binding = {30, 0};
    options.array_length_from_uniform.bindpoint_to_size_index[{0, 0}] = 0;
    options.array_offset_from_uniform.ubo_binding = {30, 1};
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 0}] = 0;

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_array_lengths_struct {
  uint tint_array_length_0_0;
};


RWByteAddressBuffer sb : register(u0);
cbuffer cbuffer_tint_storage_buffer_sizes : register(b0, space30) {
  uint4 tint_storage_buffer_sizes[1];
};
cbuffer cbuffer_tint_storage_buffer_dynamic_offsets : register(b1, space30) {
  uint4 tint_storage_buffer_dynamic_offsets[1];
};
[numthreads(1, 1, 1)]
void main() {
  tint_array_lengths_struct v = {(tint_storage_buffer_sizes[0u].x / 4u)};
  uint v_1 = v.tint_array_length_0_0;
  uint len = v_1;
  uint val = sb.Load((0u + tint_storage_buffer_dynamic_offsets[0u].x));
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
