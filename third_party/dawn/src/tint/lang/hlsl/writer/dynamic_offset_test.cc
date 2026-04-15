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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

namespace tint::hlsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(HlslWriterTest, DynamicOffset_RobustnessAndDynamicOffsetFromImmediates) {
    auto* sb = b.Var("sb", ty.ptr(storage, ty.i32()));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* value = b.Load(sb);
        b.Let("result", value);
        b.Return(func);
    });

    Options options{};
    options.disable_robustness = false;
    options.immediate_binding_point = BindingPoint{0, 30};
    options.array_offset_from_uniform.buffer_offsets_offset = 16;
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 0}] = 0;

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
cbuffer cbuffer_tint_immediate_data : register(b30) {
  uint4 tint_immediate_data[2];
};
[numthreads(1, 1, 1)]
void main() {
  int result = asint(sb.Load((0u + tint_immediate_data[1u].x)));
}

)");
}

TEST_F(HlslWriterTest, DynamicOffset_AtomicWithImmediates) {
    auto* sb = b.Var("sb", ty.ptr(storage, ty.atomic(ty.u32()), core::Access::kReadWrite));
    sb->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kAtomicAdd, sb, b.Constant(1_u));
        b.Let("out", result);
        b.Return(func);
    });

    Options options{};
    options.immediate_binding_point = BindingPoint{0, 30};
    options.array_offset_from_uniform.buffer_offsets_offset = 0;
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 0}] = 0;

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb : register(u0);
cbuffer cbuffer_tint_immediate_data : register(b30) {
  uint4 tint_immediate_data[1];
};
[numthreads(1, 1, 1)]
void main() {
  uint v = 0u;
  sb.InterlockedAdd((0u + tint_immediate_data[0u].x), 1u, v);
  uint v_1 = v;
}

)");
}

TEST_F(HlslWriterTest, DynamicOffset_MultipleBuffersWithImmediates) {
    auto* sb1 = b.Var("sb1", ty.ptr(storage, ty.i32()));
    sb1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(sb1);

    auto* sb2 = b.Var("sb2", ty.ptr(storage, ty.i32()));
    sb2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(sb2);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    b.Append(func->Block(), [&] {
        auto* val1 = b.Load(sb1);
        auto* val2 = b.Load(sb2);
        auto* sum = b.Add(val1, val2);
        b.Let("result", sum);
        b.Return(func);
    });

    Options options{};
    options.immediate_binding_point = BindingPoint{0, 30};
    options.array_offset_from_uniform.buffer_offsets_offset = 0;
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 0}] = 0;
    options.array_offset_from_uniform.bindpoint_to_offset_index[{0, 1}] = 1;

    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure().reason << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer sb1 : register(u0);
RWByteAddressBuffer sb2 : register(u1);
cbuffer cbuffer_tint_immediate_data : register(b30) {
  uint4 tint_immediate_data[1];
};
[numthreads(1, 1, 1)]
void main() {
  int result = asint((asuint(asint(sb1.Load((0u + tint_immediate_data[0u].x)))) + asuint(asint(sb2.Load((0u + tint_immediate_data[0u].y))))));
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
