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

#include "src/tint/api/common/bindings.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, FunctionVar_NoInit) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Var("v", ty.ptr<function, i32>());
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
}

TEST_F(SpirvWriterTest, FunctionVar_WithInit) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        v->SetInitializer(b.Constant(42_i));
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpirvWriterTest, FunctionVar_DeclInsideBlock) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {
            auto* v = b.Var("v", ty.ptr<function, i32>());
            v->SetInitializer(b.Constant(42_i));
            b.ExitIf(i);
        });
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
       %main = OpFunction %void None %3
          %4 = OpLabel
          %v = OpVariable %_ptr_Function_int Function
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %5
          %6 = OpLabel
               OpStore %v %int_42
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, FunctionVar_Load) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        auto* result = b.Load(v);
        b.Return(func);
        mod.SetName(result, "result");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("%result = OpLoad %int %v");
}

TEST_F(SpirvWriterTest, FunctionVar_Store) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        b.Store(v, 42_i);
        b.Return(func);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpirvWriterTest, PrivateVar_NoInit) {
    core::ir::Var* v = nullptr;
    mod.root_block->Append(v = b.Var("v", ty.ptr<private_, i32>()));

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private");
}

TEST_F(SpirvWriterTest, PrivateVar_WithInit) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
}

TEST_F(SpirvWriterTest, PrivateVar_LoadAndStore) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpBitcast %uint %load");
    EXPECT_INST("OpBitcast %uint %int_1");
    EXPECT_INST("OpIAdd %uint %11 %12");
    EXPECT_INST("OpBitcast %int %14");
    EXPECT_INST("OpStore %v %15 None");
}

TEST_F(SpirvWriterTest, WorkgroupVar) {
    core::ir::Var* v = nullptr;
    mod.root_block->Append(v = b.Var("v", ty.ptr<workgroup, i32>()));

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
}

TEST_F(SpirvWriterTest, WorkgroupVar_LoadAndStore) {
    auto* v = mod.root_block->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpBitcast %uint %load");
    EXPECT_INST("OpBitcast %uint %int_1");
    EXPECT_INST("OpIAdd %uint %21 %22");
    EXPECT_INST("OpBitcast %int %24");
    EXPECT_INST("OpStore %v %25 None");
}

TEST_F(SpirvWriterTest, WorkgroupVar_ZeroInitializeWithExtension) {
    core::ir::Var* v = nullptr;
    mod.root_block->Append(v = b.Var("v", ty.ptr<workgroup, i32>()));

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", v);
        b.Return(eb);
    });

    Options opts{};
    opts.disable_workgroup_init = false;
    opts.extensions.use_zero_initialize_workgroup_memory = true;

    // Create a writer with the zero_init_workgroup_memory flag set to `true`.
    auto result = Generate(opts);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%4 = OpConstantNull %int");
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup %4");
}

TEST_F(SpirvWriterTest, StorageVar_ReadOnly) {
    auto* v = b.Var("v", ty.ptr<storage, i32, read>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %v_block Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
               OpDecorate %1 NonWritable
)");
    EXPECT_INST(R"(
    %v_block = OpTypeStruct %int                    ; Block
%_ptr_StorageBuffer_v_block = OpTypePointer StorageBuffer %v_block
          %1 = OpVariable %_ptr_StorageBuffer_v_block StorageBuffer     ; DescriptorSet 0, Binding 0, NonWritable
)");
}

TEST_F(SpirvWriterTest, StorageVar_LoadAndStore) {
    auto* v = b.Var("v", ty.ptr<storage, i32, read_write>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %1 Coherent
)");

    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
       %load = OpLoad %int %9 None
         %14 = OpBitcast %uint %load
         %15 = OpBitcast %uint %int_1
         %17 = OpIAdd %uint %14 %15
         %18 = OpBitcast %int %17
         %19 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %19 %18 None
)");
}

TEST_F(SpirvWriterTest, StorageVar_WithVulkan) {
    auto* v = b.Var("v", ty.ptr<storage, i32, read_write>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    Options opts;
    opts.extensions.use_vulkan_memory_model = true;

    auto result = Generate(opts);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability VulkanMemoryModelDeviceScope
               OpExtension "SPV_KHR_vulkan_memory_model"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpMemberName %v_block 0 "inner"
               OpName %v_block "v_block"            ; id %3
               OpName %main "main"                  ; id %5
               OpName %load "load"                  ; id %13

               ; Annotations
               OpMemberDecorate %v_block 0 Offset 0
               OpDecorate %v_block Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0

               ; Types, variables and constants
        %int = OpTypeInt 32 1
    %v_block = OpTypeStruct %int                    ; Block
%_ptr_StorageBuffer_v_block = OpTypePointer StorageBuffer %v_block
          %1 = OpVariable %_ptr_StorageBuffer_v_block StorageBuffer     ; DescriptorSet 0, Binding 0
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %int_1 = OpConstant %int 1

               ; Function main
       %main = OpFunction %void None %7
          %8 = OpLabel
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
       %load = OpLoad %int %9 NonPrivatePointer
         %14 = OpBitcast %uint %load
         %15 = OpBitcast %uint %int_1
         %17 = OpIAdd %uint %14 %15
         %18 = OpBitcast %int %17
         %19 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %19 %18 NonPrivatePointer
               OpReturn
               OpFunctionEnd)");
}

TEST_F(SpirvWriterTest, StorageVar_Workgroup_WithVulkan) {
    auto* v = b.Var("v", ty.ptr<workgroup, i32, read_write>());
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    Options opts;
    opts.extensions.use_vulkan_memory_model = true;

    auto result = Generate(opts);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability VulkanMemoryModelDeviceScope
               OpExtension "SPV_KHR_vulkan_memory_model"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %main "main" %main_local_invocation_index_Input
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %v "v"                        ; id %1
               OpName %main_local_invocation_index_Input "main_local_invocation_index_Input"    ; id %4
               OpName %main_inner "main_inner"                                                  ; id %7
               OpName %tint_local_index "tint_local_index"                                      ; id %9
               OpName %load "load"                                                              ; id %20
               OpName %main "main"                                                              ; id %27

               ; Annotations
               OpDecorate %main_local_invocation_index_Input BuiltIn LocalInvocationIndex

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
          %v = OpVariable %_ptr_Workgroup_int Workgroup
       %uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
%main_local_invocation_index_Input = OpVariable %_ptr_Input_uint Input  ; BuiltIn LocalInvocationIndex
       %void = OpTypeVoid
         %10 = OpTypeFunction %void %uint
     %uint_1 = OpConstant %uint 1
       %bool = OpTypeBool
     %uint_2 = OpConstant %uint 2
 %uint_24840 = OpConstant %uint 24840
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
         %28 = OpTypeFunction %void

               ; Function main_inner
 %main_inner = OpFunction %void None %10
%tint_local_index = OpFunctionParameter %uint
         %11 = OpLabel
         %12 = OpULessThan %bool %tint_local_index %uint_1
               OpSelectionMerge %15 None
               OpBranchConditional %12 %16 %15
         %16 = OpLabel
               OpStore %v %int_0 NonPrivatePointer
               OpBranch %15
         %15 = OpLabel
               OpControlBarrier %uint_2 %uint_2 %uint_24840
       %load = OpLoad %int %v NonPrivatePointer
         %21 = OpBitcast %uint %load
         %22 = OpBitcast %uint %int_1
         %24 = OpIAdd %uint %21 %22
         %25 = OpBitcast %int %24
               OpStore %v %25 NonPrivatePointer
               OpReturn
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %28
         %29 = OpLabel
         %30 = OpLoad %uint %main_local_invocation_index_Input None
         %31 = OpFunctionCall %void %main_inner %30
               OpReturn
               OpFunctionEnd)");
}

TEST_F(SpirvWriterTest, UniformVar) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %_arr_v4uint_uint_1 ArrayStride 16
               OpMemberDecorate %v_block_tint_explicit_layout 0 Offset 0
               OpDecorate %v_block_tint_explicit_layout Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
               OpDecorate %1 NonWritable
)");
    EXPECT_INST(R"(
%_arr_v4uint_uint_1 = OpTypeArray %v4uint %uint_1   ; ArrayStride 16
%v_block_tint_explicit_layout = OpTypeStruct %_arr_v4uint_uint_1    ; Block
%_ptr_Uniform_v_block_tint_explicit_layout = OpTypePointer Uniform %v_block_tint_explicit_layout
          %1 = OpVariable %_ptr_Uniform_v_block_tint_explicit_layout Uniform    ; DescriptorSet 0, Binding 0, NonWritable
)");
}

TEST_F(SpirvWriterTest, UniformVar_Load) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
         %12 = OpAccessChain %_ptr_Uniform_v4uint %1 %uint_0 %uint_0
         %15 = OpLoad %v4uint %12 None
         %16 = OpCompositeExtract %uint %15 0
         %18 = OpBitcast %int %16
)");
}

TEST_F(SpirvWriterTest, ImmediateVar) {
    auto* v = b.Var("v", ty.ptr<immediate, i32>());
    mod.root_block->Append(v);

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %v_block Block
)");
    EXPECT_INST(R"(
    %v_block = OpTypeStruct %int                    ; Block
%_ptr_PushConstant_v_block = OpTypePointer PushConstant %v_block
          %1 = OpVariable %_ptr_PushConstant_v_block PushConstant
)");
}

TEST_F(SpirvWriterTest, ImmedaiteVar_Load) {
    auto* v = b.Var("v", ty.ptr<immediate, i32>());
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func, load);
        mod.SetName(load, "load");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
          %8 = OpAccessChain %_ptr_PushConstant_int %1 %uint_0
       %load = OpLoad %int %8 None
               OpReturnValue %load
)");
}

TEST_F(SpirvWriterTest, SamplerVar) {
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle, ty.sampler(), core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeSampler
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0
)");
    EXPECT_INST("%load = OpLoad %3 %v");
}

TEST_F(SpirvWriterTest, TextureVar) {
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0, RelaxedPrecision
)");
    EXPECT_INST("%load = OpLoad %3 %v");
}

TEST_F(SpirvWriterTest, TextureVar_TextureParamTextureLoad_NoDva) {
    auto* tex =
        b.Var("tex", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    mod.root_block->Append(tex);

    auto* t = b.FunctionParam("texparam",
                              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t});
    b.Append(fn->Block(), [&] {
        b.Let("p",
              b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, t, b.Splat(ty.vec2u(), 0_u), 0_u));
        b.Return(fn);
    });

    auto* fn2 = b.ComputeFunction("main");
    b.Append(fn2->Block(), [&] {
        auto* t2 = b.Load(tex);
        b.Call(ty.void_(), fn, t2);
        b.Return(fn2);
    });

    Options opts{};
    opts.workarounds.dva_transform_handle = false;

    auto result = Generate(opts);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpFunctionParameter");
}

TEST_F(SpirvWriterTest, TextureVar_TextureParamTextureLoad_ExternalDva) {
    auto* tex =
        b.Var("tex", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    mod.root_block->Append(tex);

    auto* ex_tex = b.Var("ex_tex", handle, ty.external_texture(), core::Access::kRead);
    ex_tex->SetBindingPoint(0, 1);
    mod.root_block->Append(ex_tex);

    auto* fn = b.Function("f", ty.void_());
    auto* t = b.FunctionParam("texparam",
                              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    fn->SetParams({t});
    b.Append(fn->Block(), [&] {
        b.Let("p",
              b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, t, b.Splat(ty.vec2u(), 0_u), 0_u));
        b.Return(fn);
    });

    auto* ex_fn = b.Function("ex_f", ty.void_());
    auto* ex_t = b.FunctionParam("external_tex", ty.external_texture());
    ex_fn->SetParams({ex_t});
    b.Append(ex_fn->Block(), [&] {
        b.Let("p",
              b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, ex_t, b.Splat(ty.vec2u(), 0_u)));
        b.Return(ex_fn);
    });

    auto* fn2 = b.ComputeFunction("main");
    b.Append(fn2->Block(), [&] {
        auto* t2 = b.Load(tex);
        auto* t3 = b.Load(ex_tex);
        b.Call(ty.void_(), fn, t2);
        b.Call(ty.void_(), ex_fn, t3);
        b.Return(fn2);
    });

    Options opts{};
    opts.workarounds.dva_transform_handle = false;
    opts.bindings.external_texture.emplace(
        tint::BindingPoint{0, 1},
        ExternalMultiplanarTexture{tint::BindingPoint{2, 0}, tint::BindingPoint{3, 0}});

    auto result = Generate(opts);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%texparam = OpFunctionParameter");
    // Consider a EXPECT_NOT_INST macro.
    ASSERT_TRUE(output_.find("%external_tex_params = OpFunctionParameter") == std::string::npos)
        << output_;
}

TEST_F(SpirvWriterTest, TextureVar_TextureParamTextureLoad_Dva) {
    auto* tex =
        b.Var("tex", handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              core::Access::kRead);
    tex->SetBindingPoint(0, 0);
    mod.root_block->Append(tex);

    auto* t = b.FunctionParam("texparam",
                              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({t});
    b.Append(fn->Block(), [&] {
        b.Let("p",
              b.Call(ty.vec4f(), core::BuiltinFn::kTextureLoad, t, b.Splat(ty.vec2u(), 0_u), 0_u));
        b.Return(fn);
    });

    auto* fn2 = b.ComputeFunction("main");
    b.Append(fn2->Block(), [&] {
        auto* t2 = b.Load(tex);
        b.Call(ty.void_(), fn, t2);
        b.Return(fn2);
    });

    Options opts{};
    opts.workarounds.dva_transform_handle = true;

    auto result = Generate(opts);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    // Consider a EXPECT_NOT_INST macro.
    ASSERT_TRUE(output_.find("OpFunctionParameter") == std::string::npos) << output_;
}

TEST_F(SpirvWriterTest, ReadOnlyStorageTextureVar) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.storage_texture(core::type::TextureDimension::k2d, format, read),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Load(v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v NonWritable
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0, NonWritable, RelaxedPrecision
)");
}

TEST_F(SpirvWriterTest, ReadWriteStorageTextureVar) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* v =
        b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                          ty.storage_texture(core::type::TextureDimension::k2d, format, read_write),
                          core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Load(v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v Coherent
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0, Coherent, RelaxedPrecision
)");
}

TEST_F(SpirvWriterTest, WriteOnlyStorageTextureVar) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* v =
        b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                          ty.storage_texture(core::type::TextureDimension::k2d, format, write),
                          core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Load(v);
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v NonReadable
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0, NonReadable, RelaxedPrecision
)");
}

}  // namespace
}  // namespace tint::spirv::writer
