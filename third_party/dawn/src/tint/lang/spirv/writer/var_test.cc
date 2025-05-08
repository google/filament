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

#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, FunctionVar_NoInit) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("v", ty.ptr<function, i32>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
}

TEST_F(SpirvWriterTest, FunctionVar_WithInit) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        v->SetInitializer(b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpirvWriterTest, FunctionVar_DeclInsideBlock) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {
            auto* v = b.Var("v", ty.ptr<function, i32>());
            v->SetInitializer(b.Constant(42_i));
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %3
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
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        auto* result = b.Load(v);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("%result = OpLoad %int %v");
}

TEST_F(SpirvWriterTest, FunctionVar_Store) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        b.Store(v, 42_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpirvWriterTest, PrivateVar_NoInit) {
    mod.root_block->Append(b.Var("v", ty.ptr<private_, i32>()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private");
}

TEST_F(SpirvWriterTest, PrivateVar_WithInit) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
}

TEST_F(SpirvWriterTest, PrivateVar_LoadAndStore) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpStore %v %add");
}

TEST_F(SpirvWriterTest, WorkgroupVar) {
    mod.root_block->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
}

TEST_F(SpirvWriterTest, WorkgroupVar_LoadAndStore) {
    auto* v = mod.root_block->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpStore %v %add");
}

TEST_F(SpirvWriterTest, WorkgroupVar_ZeroInitializeWithExtension) {
    mod.root_block->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    Options opts{};
    opts.disable_workgroup_init = false;
    opts.use_zero_initialize_workgroup_memory_extension = true;

    // Create a writer with the zero_init_workgroup_memory flag set to `true`.
    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("%4 = OpConstantNull %int");
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup %4");
}

TEST_F(SpirvWriterTest, StorageVar_ReadOnly) {
    auto* v = b.Var("v", ty.ptr<storage, i32, read>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
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

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %1 Coherent
)");

    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
       %load = OpLoad %int %9 None
        %add = OpIAdd %int %load %int_1
         %16 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %16 %add None
)");
}

TEST_F(SpirvWriterTest, StorageVar_WithVulkan) {
    auto* v = b.Var("v", ty.ptr<storage, i32, read_write>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    Options opts;
    opts.use_vulkan_memory_model = true;

    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST(R"(               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability VulkanMemoryModelDeviceScope
               OpExtension "SPV_KHR_vulkan_memory_model"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %foo "foo"
               OpExecutionMode %foo LocalSize 1 1 1

               ; Debug Information
               OpMemberName %v_block 0 "inner"
               OpName %v_block "v_block"            ; id %3
               OpName %foo "foo"                    ; id %5
               OpName %load "load"                  ; id %13
               OpName %add "add"                    ; id %14

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

               ; Function foo
        %foo = OpFunction %void None %7
          %8 = OpLabel
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
       %load = OpLoad %int %9 NonPrivatePointer
        %add = OpIAdd %int %load %int_1
         %16 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %16 %add NonPrivatePointer
               OpReturn
               OpFunctionEnd)");
}

TEST_F(SpirvWriterTest, StorageVar_Workgroup_WithVulkan) {
    auto* v = b.Var("v", ty.ptr<workgroup, i32, read_write>());
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    Options opts;
    opts.use_vulkan_memory_model = true;

    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST(R"(               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability VulkanMemoryModelDeviceScope
               OpExtension "SPV_KHR_vulkan_memory_model"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %foo "foo" %foo_local_invocation_index_Input
               OpExecutionMode %foo LocalSize 1 1 1

               ; Debug Information
               OpName %v "v"                        ; id %1
               OpName %foo_local_invocation_index_Input "foo_local_invocation_index_Input"  ; id %4
               OpName %foo_inner "foo_inner"                                                ; id %7
               OpName %tint_local_index "tint_local_index"                                  ; id %9
               OpName %load "load"                                                          ; id %21
               OpName %add "add"                                                            ; id %22
               OpName %foo "foo"                                                            ; id %24

               ; Annotations
               OpDecorate %foo_local_invocation_index_Input BuiltIn LocalInvocationIndex

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
          %v = OpVariable %_ptr_Workgroup_int Workgroup
       %uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
%foo_local_invocation_index_Input = OpVariable %_ptr_Input_uint Input   ; BuiltIn LocalInvocationIndex
       %void = OpTypeVoid
         %10 = OpTypeFunction %void %uint
     %uint_1 = OpConstant %uint 1
       %bool = OpTypeBool
      %int_0 = OpConstant %int 0
     %uint_2 = OpConstant %uint 2
 %uint_24840 = OpConstant %uint 24840
      %int_1 = OpConstant %int 1
         %25 = OpTypeFunction %void

               ; Function foo_inner
  %foo_inner = OpFunction %void None %10
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
        %add = OpIAdd %int %load %int_1
               OpStore %v %add NonPrivatePointer
               OpReturn
               OpFunctionEnd

               ; Function foo
        %foo = OpFunction %void None %25
         %26 = OpLabel
         %27 = OpLoad %uint %foo_local_invocation_index_Input None
         %28 = OpFunctionCall %void %foo_inner %27
               OpReturn
               OpFunctionEnd)");
}

TEST_F(SpirvWriterTest, StorageVar_WriteOnly) {
    auto* v = b.Var("v", ty.ptr<storage, i32, write>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Store(v, 42_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v_block Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
               OpDecorate %1 NonReadable
)");
    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %9 %int_42 None
)");
}

TEST_F(SpirvWriterTest, UniformVar) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v_block Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
)");
    EXPECT_INST(R"(
    %v_block = OpTypeStruct %int                    ; Block
%_ptr_Uniform_v_block = OpTypePointer Uniform %v_block
          %1 = OpVariable %_ptr_Uniform_v_block Uniform     ; DescriptorSet 0, Binding 0, NonWritable
)");
}

TEST_F(SpirvWriterTest, UniformVar_Load) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_Uniform_int %1 %uint_0
       %load = OpLoad %int %9 None
)");
}

TEST_F(SpirvWriterTest, PushConstantVar) {
    auto* v = b.Var("v", ty.ptr<push_constant, i32>());
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v_block Block
)");
    EXPECT_INST(R"(
    %v_block = OpTypeStruct %int                    ; Block
%_ptr_PushConstant_v_block = OpTypePointer PushConstant %v_block
          %1 = OpVariable %_ptr_PushConstant_v_block PushConstant
)");
}

TEST_F(SpirvWriterTest, PushConstantVar_Load) {
    auto* v = b.Var("v", ty.ptr<push_constant, i32>());
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func, load);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
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

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeSampler
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0
)");
}

TEST_F(SpirvWriterTest, SamplerVar_Load) {
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle, ty.sampler(), core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%load = OpLoad %3 %v");
}

TEST_F(SpirvWriterTest, TextureVar) {
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0
)");
}

TEST_F(SpirvWriterTest, TextureVar_Load) {
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%load = OpLoad %3 %v");
}

TEST_F(SpirvWriterTest, ReadOnlyStorageTextureVar) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.storage_texture(core::type::TextureDimension::k2d, format, read),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v NonWritable
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0, NonWritable
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

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v Coherent
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0, Coherent
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

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v NonReadable
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant   ; DescriptorSet 0, Binding 0, NonReadable
)");
}

}  // namespace
}  // namespace tint::spirv::writer
