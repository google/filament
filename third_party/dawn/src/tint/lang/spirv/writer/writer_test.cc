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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"
#include "src/tint/utils/internal_limits.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, ModuleHeader) {
    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] { b.Return(eb); });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpCapability Shader");
    EXPECT_INST("OpMemoryModel Logical GLSL450");
}

TEST_F(SpirvWriterTest, ModuleHeader_VulkanMemoryModel) {
    Options opts;
    opts.extensions.use_vulkan_memory_model = true;

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] { b.Return(eb); });

    auto result = Generate(opts);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpExtension \"SPV_KHR_vulkan_memory_model\"");
    EXPECT_INST("OpCapability VulkanMemoryModel");
    EXPECT_INST("OpCapability VulkanMemoryModelDeviceScope");
    EXPECT_INST("OpMemoryModel Logical Vulkan");
}

TEST_F(SpirvWriterTest, CanGenerate_SubgroupMatrixRequiresVulkanMemoryModel) {
    core::ir::Var* v = nullptr;
    b.Append(mod.root_block,
             [&] { v = b.Var(ty.ptr<private_>(ty.subgroup_matrix_result(ty.f32(), 8, 8))); });

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {
        b.Let("x", v);
        b.Return(ep);
    });

    Options options;
    options.extensions.use_vulkan_memory_model = false;
    options.entry_point_name = "main";
    auto result = Generate(options);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr("using subgroup matrices requires the Vulkan Memory Model"));
}

TEST_F(SpirvWriterTest, EntryPoint_InputLocation_ExceedsMax) {
    auto* f = b.FragmentFunction("my_func", ty.void_());
    auto* p = b.FunctionParam("p", ty.f32());
    p->SetLocation(4096);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    Options options;
    options.entry_point_name = "my_func";
    auto res = Generate(options);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, "location(4096) exceeds the maximum allowed value of '4095'");
}

TEST_F(SpirvWriterTest, EntryPoint_OutputLocation_ExceedsMax) {
    auto* f = b.FragmentFunction("my_func", ty.f32());
    f->SetReturnLocation(4096);

    b.Append(f->Block(), [&] { b.Return(f, 1.0_f); });

    Options options;
    options.entry_point_name = "my_func";
    auto res = Generate(options);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, "location(4096) exceeds the maximum allowed value of '4095'");
}

TEST_F(SpirvWriterTest, Unreachable) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(true);
            b.Append(ifelse->True(), [&] {  //
                b.Continue(loop);
            });
            b.Append(ifelse->False(), [&] {  //
                b.Continue(loop);
            });
            b.Unreachable();

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %11
         %10 = OpLabel
               OpBranch %6
         %11 = OpLabel
               OpBranch %6
          %9 = OpLabel
               OpUnreachable
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we fail gracefully when a function has too many parameters.
// See crbug.com/354748060.
TEST_F(SpirvWriterTest, TooManyFunctionParameters) {
    Vector<core::ir::FunctionParam*, 256> params;
    Vector<core::ir::Value*, 256> args;
    for (uint32_t i = 0; i < 256; i++) {
        params.Push(b.FunctionParam(ty.i32()));
        args.Push(b.Zero(ty.i32()));
    }
    auto* func = b.Function("foo", ty.void_());
    func->SetParams(std::move(params));
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] {
        b.Call(func, args);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason,
                testing::HasSubstr(
                    "Function 'foo' has more than 255 parameters after running Tint transforms"));
}

TEST_F(SpirvWriterTest, EntryPointName_Remapped) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "my_entry_point";
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"my_entry_point\"");
}

TEST_F(SpirvWriterTest, EntryPointName_NotRemapped) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"main\"");
}

TEST_F(SpirvWriterTest, EntryPoint_FunctionVar_Spirv1p3) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("x", 0_u);
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"main\"\n");
}

TEST_F(SpirvWriterTest, EntryPoint_FunctionVar_Spirv1p4) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Var("x", 0_u);
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    options.spirv_version = SpvVersion::kSpv14;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"main\"\n");
}

TEST_F(SpirvWriterTest, EntryPoint_StorageVar_Spirv1p3) {
    auto* v = b.Var("v", core::AddressSpace::kStorage, ty.u32(), core::Access::kReadWrite);
    mod.root_block->Append(v);
    v->SetBindingPoint(0, 0);
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Load(v);
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"main\"\n");
}

TEST_F(SpirvWriterTest, EntryPoint_StorageVar_Spirv1p4) {
    auto* v = b.Var("v", core::AddressSpace::kStorage, ty.u32(), core::Access::kReadWrite);
    mod.root_block->Append(v);
    v->SetBindingPoint(0, 0);
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Load(v);
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    options.spirv_version = SpvVersion::kSpv14;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"main\" %1");
}

TEST_F(SpirvWriterTest, EntryPoint_StorageVar_CalledFunction_Spirv1p4) {
    auto* v = b.Var("v", core::AddressSpace::kStorage, ty.u32(), core::Access::kReadWrite);
    mod.root_block->Append(v);
    v->SetBindingPoint(0, 0);
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(v);
        b.Return(foo);
    });
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Call(foo);
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    options.spirv_version = SpvVersion::kSpv14;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"main\" %1");
}

TEST_F(SpirvWriterTest, Spv14_CopyLogical) {
    auto* ssbo = b.Var("ssbo", core::AddressSpace::kStorage, ty.array(ty.u32(), 4),
                       core::Access::kReadWrite);
    mod.root_block->Append(ssbo);
    ssbo->SetBindingPoint(0, 0);
    auto* wg = b.Var("wg", core::AddressSpace::kWorkgroup, ty.array(ty.u32(), 4),
                     core::Access::kReadWrite);
    mod.root_block->Append(wg);
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        auto* load = b.Load(ssbo);
        b.Store(wg, load);
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    options.spirv_version = SpvVersion::kSpv14;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Annotations
               OpDecorate %_arr_uint_uint_4 ArrayStride 4
               OpMemberDecorate %ssbo_block_tint_explicit_layout 0 Offset 0
               OpDecorate %ssbo_block_tint_explicit_layout Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
               OpDecorate %1 Coherent
)");
    EXPECT_INST(R"(
               ; Types, variables and constants
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_uint_uint_4 = OpTypeArray %uint %uint_4       ; ArrayStride 4
%ssbo_block_tint_explicit_layout = OpTypeStruct %_arr_uint_uint_4   ; Block
%_ptr_StorageBuffer_ssbo_block_tint_explicit_layout = OpTypePointer StorageBuffer %ssbo_block_tint_explicit_layout
          %1 = OpVariable %_ptr_StorageBuffer_ssbo_block_tint_explicit_layout StorageBuffer     ; DescriptorSet 0, Binding 0, Coherent
%_arr_uint_uint_4_0 = OpTypeArray %uint %uint_4
%_ptr_Workgroup__arr_uint_uint_4_0 = OpTypePointer Workgroup %_arr_uint_uint_4_0
         %wg = OpVariable %_ptr_Workgroup__arr_uint_uint_4_0 Workgroup
)");
    EXPECT_INST(R"(
         %27 = OpAccessChain %_ptr_StorageBuffer__arr_uint_uint_4 %1 %uint_0
         %30 = OpLoad %_arr_uint_uint_4 %27 None
         %31 = OpCopyLogical %_arr_uint_uint_4_0 %30
               OpStore %wg %31 None
               OpReturn
)");
}

TEST_F(SpirvWriterTest, StripAllNames) {
    auto* str = ty.Struct(mod.symbols.New("MyStruct"), {
                                                           {mod.symbols.Register("a"), ty.i32()},
                                                           {mod.symbols.Register("b"), ty.vec4i()},
                                                       });
    auto* func = b.ComputeFunction("main");
    auto* idx = b.FunctionParam("idx", ty.u32());
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);
    func->AppendParam(idx);
    b.Append(func->Block(), [&] {  //
        auto* var = b.Var("str", ty.ptr<function>(str));
        auto* val = b.Load(var);
        mod.SetName(val, "val");
        auto* a = b.Access<i32>(val, 0_u);
        mod.SetName(a, "a");
        b.Return(func);
    });

    Options options;
    options.strip_all_names = true;
    options.remapped_entry_point_name = "tint_entry_point";
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpEntryPoint GLCompute %16 "tint_entry_point" %gl_LocalInvocationIndex
               OpExecutionMode %16 LocalSize 1 1 1

               ; Annotations
               OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
%gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input    ; BuiltIn LocalInvocationIndex
       %void = OpTypeVoid
          %7 = OpTypeFunction %void %uint
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
 %_struct_11 = OpTypeStruct %int %v4int
%_ptr_Function__struct_11 = OpTypePointer Function %_struct_11
         %14 = OpConstantNull %_struct_11
         %17 = OpTypeFunction %void

               ; Function 4
          %4 = OpFunction %void None %7
          %6 = OpFunctionParameter %uint
          %8 = OpLabel
          %9 = OpVariable %_ptr_Function__struct_11 Function %14
         %15 = OpLoad %_struct_11 %9 None
               OpReturn
               OpFunctionEnd

               ; Function 16
         %16 = OpFunction %void None %17
         %18 = OpLabel
         %19 = OpLoad %uint %gl_LocalInvocationIndex None
         %20 = OpFunctionCall %void %4 %19
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, WorkgroupStorageSize_OverflowAfterAlign) {
    auto* var = mod.root_block->Append(b.Var<workgroup, array<u32, 0x3FFFFFFFu>>("a"));
    auto* foo = b.ComputeFunction("main", 64_u, 1_u, 1_u);
    b.Append(foo->Block(), [&] {  //
        b.Load(b.Access<ptr<workgroup, u32>>(var, 0_u));
        b.Return(foo);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_EQ(workgroup_info.storage_size, 0x100000000ull);
}

TEST_F(SpirvWriterTest, CanGenerate_StructMemberPadding_TooLarge) {
    ty.Get<core::type::Struct>(
        mod.symbols.New("S"),
        tint::Vector{ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                      4u, core::IOAttributes{}),
                     ty.Get<core::type::StructMember>(
                         mod.symbols.New("b"), ty.i32(), 1u,
                         static_cast<uint32_t>(tint::internal_limits::kMaxStructMemberPadding + 4),
                         4u, 4u, core::IOAttributes{})},
        8u /* size */);

    auto* ep = b.ComputeFunction("main");
    b.Append(ep->Block(), [&] { b.Return(ep); });

    Options options;
    auto result = Generate(options);
    ASSERT_NE(result, Success);
    EXPECT_THAT(result.Failure().reason, testing::HasSubstr("is larger than the maximum"));
}

TEST_F(SpirvWriterTest, PolyfillPixelCenter) {
    auto* position = b.FunctionParam("position", ty.vec4f());
    position->SetBuiltin(core::BuiltinValue::kPosition);

    auto* ep = b.FragmentFunction("main", ty.void_());
    ep->SetParams({position});

    b.Append(ep->Block(), [&] {
        b.Let("p", position);
        b.Return(ep);
    });

    Options options;
    options.polyfill_pixel_center = 0;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
       %main = OpFunction %void None %12
         %13 = OpLabel
         %14 = OpLoad %v4float %main_position_Input None
         %15 = OpVectorShuffle %v2float %14 %14 0 1
         %17 = OpExtInst %v2float %18 Floor %15
         %19 = OpFAdd %v2float %17 %20
         %22 = OpExtInst %v4float %18 InterpolateAtOffset %main_loc0_Input %23
         %24 = OpCompositeExtract %float %22 2
         %25 = OpCompositeExtract %float %22 3
         %26 = OpFDiv %float %24 %25
         %27 = OpFDiv %float %float_1 %25
         %29 = OpCompositeConstruct %v4float %19 %26 %27
         %30 = OpFunctionCall %void %main_inner %29
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, MaximalReconvergence_Compute) {
    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    ep->Block()->Append(b.Return(ep));

    Options options;
    options.extensions.use_maximal_reconvergence = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpExtension "SPV_KHR_maximal_reconvergence"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main MaximallyReconvergesKHR
)");
}

TEST_F(SpirvWriterTest, MaximalReconvergence_Fragment) {
    auto* ep = b.FragmentFunction("main", ty.void_());
    ep->Block()->Append(b.Return(ep));

    Options options;
    options.extensions.use_maximal_reconvergence = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpExtension "SPV_KHR_maximal_reconvergence"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main MaximallyReconvergesKHR
)");
}

TEST_F(SpirvWriterTest, MaximalReconvergence_Vertex) {
    auto* ep = b.Function("main", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnAttributes({.builtin = core::BuiltinValue::kPosition});
    ep->Block()->Append(b.Return(ep, b.Zero(ty.vec4f())));

    Options options;
    options.extensions.use_maximal_reconvergence = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpExtension "SPV_KHR_maximal_reconvergence"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %main_position_Output %main___point_size_Output
               OpExecutionMode %main MaximallyReconvergesKHR
)");
}

TEST_F(SpirvWriterTest, SubgroupUniformControlFlow_Compute) {
    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    ep->Block()->Append(b.Return(ep));

    Options options;
    options.extensions.use_subgroup_uniform_control_flow = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpExtension "SPV_KHR_subgroup_uniform_control_flow"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main SubgroupUniformControlFlowKHR
)");
}

TEST_F(SpirvWriterTest, SubgroupUniformControlFlow_Fragment) {
    auto* ep = b.FragmentFunction("main", ty.void_());
    ep->Block()->Append(b.Return(ep));

    Options options;
    options.extensions.use_subgroup_uniform_control_flow = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpExtension "SPV_KHR_subgroup_uniform_control_flow"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main SubgroupUniformControlFlowKHR
)");
}

TEST_F(SpirvWriterTest, SubgroupUniformControlFlow_Vertex) {
    auto* ep = b.Function("main", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnAttributes({.builtin = core::BuiltinValue::kPosition});
    ep->Block()->Append(b.Return(ep, b.Zero(ty.vec4f())));

    Options options;
    options.extensions.use_subgroup_uniform_control_flow = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(
        R"(              OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %main_position_Output %main___point_size_Output

)");
}

TEST_F(SpirvWriterTest, BufferView_Vec2h_Via_U16) {
    mod.properties.Add(core::ir::Property::kAllowBufferTypes);
    auto* v = b.Var("v", ty.ptr(storage, ty.unsized_buffer()));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.vec2h()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.vec2h()}, v, 0_u);
        b.Store(view, b.Zero(ty.vec2h()));
        b.Return(ep);
    });

    Options options;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(OpCapability Int16)");
    EXPECT_INST(R"(OpTypeInt 16 0)");
}

TEST_F(SpirvWriterTest, Alignment_Load_Workgroup) {
    auto* v = b.Var("v", ty.ptr(workgroup, ty.u32()));
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* ld = b.Load(v);
        ld->SetAlignment(16);
        b.Return(ep);
    });

    Options options;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpLoad %uint %v None");
}

TEST_F(SpirvWriterTest, Alignment_Load) {
    auto* v = b.Var("v", ty.ptr(storage, ty.u32()));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* ld = b.Load(v);
        ld->SetAlignment(16);
        b.Return(ep);
    });

    Options options;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpLoad %uint %9 Aligned 16");
}

TEST_F(SpirvWriterTest, Alignment_LoadVectorElement) {
    auto* v = b.Var("v", ty.ptr(storage, ty.vec4u()));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* ld = b.LoadVectorElement(v, 2_u);
        ld->SetAlignment(8);
        b.Return(ep);
    });

    Options options;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpLoad %uint %13 Aligned 8");
}

TEST_F(SpirvWriterTest, Alignment_Store) {
    auto* v = b.Var("v", ty.ptr(storage, ty.u32()));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* st = b.Store(v, 0_u);
        st->SetAlignment(16);
        b.Return(ep);
    });

    Options options;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpStore %9 %uint_0 Aligned 16");
}

TEST_F(SpirvWriterTest, Alignment_StoreVectorElement) {
    auto* v = b.Var("v", ty.ptr(storage, ty.vec4u()));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* st = b.StoreVectorElement(v, 2_u, 0_u);
        st->SetAlignment(8);
        b.Return(ep);
    });

    Options options;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpStore %13 %uint_0 Aligned 8");
}

TEST_F(SpirvWriterTest, Alignment_CooperativeMatrixLoad) {
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.f32())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* ld = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, v, 0_u,
            8_u);
        ld->SetAlignment(64);
        b.Return(ep);
    });

    Options options;
    options.extensions.use_vulkan_memory_model = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpCooperativeMatrixLoadKHR %28 %25 %uint_0 %23 Aligned|NonPrivatePointer 64");
}

TEST_F(SpirvWriterTest, Alignment_CooperativeMatrixStore) {
    auto* mat_ty = ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8, 8);
    auto* v = b.Var("v", ty.ptr(storage, ty.runtime_array(ty.f32())));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("main", 1_u, 1_u, 1_u);
    b.Append(ep->Block(), [&] {
        auto* m = b.Construct(mat_ty);
        auto* st = b.CallExplicit(
            ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
            Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, v, 0_u, m, 8_u);
        st->SetAlignment(64);
        b.Return(ep);
    });

    Options options;
    options.extensions.use_vulkan_memory_model = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpCooperativeMatrixStoreKHR %28 %10 %uint_0 %26 Aligned|NonPrivatePointer 64");
}

}  // namespace
}  // namespace tint::spirv::writer
