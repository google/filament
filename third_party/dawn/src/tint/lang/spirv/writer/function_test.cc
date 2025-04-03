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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, Function_Empty) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
)");
    // There is always an injected entry point if none exists in the source program.
    EXPECT_EQ(workgroup_info.x, 1u);
    EXPECT_EQ(workgroup_info.y, 1u);
    EXPECT_EQ(workgroup_info.z, 1u);
}

// Test that we do not emit the same function type more than once.
TEST_F(SpirvWriterTest, Function_DeduplicateType) {
    auto* func_a = b.Function("func_a", ty.void_());
    b.Append(func_a->Block(), [&] {  //
        b.Return(func_a);
    });
    auto* func_b = b.Function("func_b", ty.void_());
    b.Append(func_b->Block(), [&] {  //
        b.Return(func_b);
    });
    auto* func_c = b.Function("func_c", ty.void_());
    b.Append(func_c->Block(), [&] {  //
        b.Return(func_c);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function func_a
     %func_a = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function func_b
     %func_b = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function func_c
     %func_c = OpFunction %void None %3
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Compute) {
    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 32 4 1

               ; Debug Information
               OpName %main "main"                  ; id %1

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
)");
    EXPECT_EQ(workgroup_info.x, 32u);
    EXPECT_EQ(workgroup_info.y, 4u);
    EXPECT_EQ(workgroup_info.z, 1u);
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Fragment) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %main "main"                  ; id %1

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd
)");
    EXPECT_EQ(workgroup_info.x, 0u);
    EXPECT_EQ(workgroup_info.y, 0u);
    EXPECT_EQ(workgroup_info.z, 0u);
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Vertex) {
    auto* func = b.Function("main", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    func->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Zero<vec4<f32>>());
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint Vertex %main "main" %main_position_Output %main___point_size_Output

               ; Debug Information
               OpName %main_position_Output "main_position_Output"  ; id %1
               OpName %main___point_size_Output "main___point_size_Output"  ; id %5
               OpName %main_inner "main_inner"                              ; id %7
               OpName %main "main"                                          ; id %11

               ; Annotations
               OpDecorate %main_position_Output BuiltIn Position
               OpDecorate %main___point_size_Output BuiltIn PointSize

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main_position_Output = OpVariable %_ptr_Output_v4float Output  ; BuiltIn Position
%_ptr_Output_float = OpTypePointer Output %float
%main___point_size_Output = OpVariable %_ptr_Output_float Output    ; BuiltIn PointSize
          %8 = OpTypeFunction %v4float
         %10 = OpConstantNull %v4float
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_1 = OpConstant %float 1

               ; Function main_inner
 %main_inner = OpFunction %v4float None %8
          %9 = OpLabel
               OpReturnValue %10
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
         %15 = OpFunctionCall %v4float %main_inner
               OpStore %main_position_Output %15 None
               OpStore %main___point_size_Output %float_1 None
               OpReturn
               OpFunctionEnd
)");
    EXPECT_EQ(workgroup_info.x, 0u);
    EXPECT_EQ(workgroup_info.y, 0u);
    EXPECT_EQ(workgroup_info.z, 0u);
}

TEST_F(SpirvWriterTest, Function_EntryPoint_Multiple) {
    auto* f1 = b.ComputeFunction("main1", 32_u, 4_u, 1_u);
    b.Append(f1->Block(), [&] {  //
        b.Return(f1);
    });

    auto* f2 = b.ComputeFunction("main2", 8_u, 2_u, 16_u);
    b.Append(f2->Block(), [&] {  //
        b.Return(f2);
    });

    auto* f3 = b.Function("main3", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(f3->Block(), [&] {  //
        b.Return(f3);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint GLCompute %main1 "main1"
               OpEntryPoint GLCompute %main2 "main2"
               OpEntryPoint Fragment %main3 "main3"
               OpExecutionMode %main1 LocalSize 32 4 1
               OpExecutionMode %main2 LocalSize 8 2 16
               OpExecutionMode %main3 OriginUpperLeft

               ; Debug Information
               OpName %main1 "main1"                ; id %1
               OpName %main2 "main2"                ; id %5
               OpName %main3 "main3"                ; id %7

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main1
      %main1 = OpFunction %void None %3
          %4 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function main2
      %main2 = OpFunction %void None %3
          %6 = OpLabel
               OpReturn
               OpFunctionEnd

               ; Function main3
      %main3 = OpFunction %void None %3
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_ReturnValue) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {  //
        b.Return(func, 42_i);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %3 = OpTypeFunction %int
     %int_42 = OpConstant %int 42
       %void = OpTypeVoid
          %8 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %3
          %4 = OpLabel
               OpReturnValue %int_42
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_Parameters) {
    auto* i32 = ty.i32();
    auto* x = b.FunctionParam("x", i32);
    auto* y = b.FunctionParam("y", i32);
    auto* func = b.Function("foo", i32);
    func->SetParams({x, y});

    b.Append(func->Block(), [&] {
        auto* result = b.Add(i32, x, y);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %5 = OpTypeFunction %int %int %int
       %void = OpTypeVoid
         %10 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %int None %5
          %x = OpFunctionParameter %int
          %y = OpFunctionParameter %int
          %6 = OpLabel
          %7 = OpIAdd %int %x %y
               OpReturnValue %7
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_Call) {
    auto* i32 = ty.i32();
    auto* x = b.FunctionParam("x", i32);
    auto* y = b.FunctionParam("y", i32);
    auto* foo = b.Function("foo", i32);
    foo->SetParams({x, y});

    b.Append(foo->Block(), [&] {
        auto* result = b.Add(i32, x, y);
        b.Return(foo, result);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        auto* result = b.Call(i32, foo, 2_i, 3_i);
        b.Return(bar);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpFunctionCall %int %foo %int_2 %int_3");
}

TEST_F(SpirvWriterTest, Function_Call_Void) {
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        auto* result = b.Call(ty.void_(), foo);
        b.Return(bar);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpFunctionCall %void %foo");
}

TEST_F(SpirvWriterTest, Function_ShaderIO_VertexPointSize) {
    auto* func = b.Function("main", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    func->SetReturnBuiltin(core::BuiltinValue::kPosition);
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

    Options options;
    options.emit_vertex_point_size = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(
        R"(OpEntryPoint Vertex %main "main" %main_position_Output %main___point_size_Output)");
    EXPECT_INST(R"(
               OpDecorate %main_position_Output BuiltIn Position
               OpDecorate %main___point_size_Output BuiltIn PointSize
)");
    EXPECT_INST(R"(
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main_position_Output = OpVariable %_ptr_Output_v4float Output  ; BuiltIn Position
%_ptr_Output_float = OpTypePointer Output %float
%main___point_size_Output = OpVariable %_ptr_Output_float Output    ; BuiltIn PointSize
)");
    EXPECT_INST(R"(
       %main = OpFunction %void None %14
         %15 = OpLabel
         %16 = OpFunctionCall %v4float %main_inner
               OpStore %main_position_Output %16 None
               OpStore %main___point_size_Output %float_1 None
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_ShaderIO_F16_Input_WithCapability) {
    auto* input = b.FunctionParam("input", ty.vec4<f16>());
    input->SetLocation(1);
    auto* func = b.Function("main", ty.vec4<f32>(), core::ir::Function::PipelineStage::kFragment);
    func->SetReturnLocation(2);
    func->SetParams({input});
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Convert(ty.vec4<f32>(), input));
    });

    Options options;
    options.use_storage_input_output_16 = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST("OpCapability StorageInputOutput16");
    EXPECT_INST(R"(OpEntryPoint Fragment %main "main" %main_loc1_Input %main_loc2_Output)");
    EXPECT_INST("%main_loc1_Input = OpVariable %_ptr_Input_v4half Input");
    EXPECT_INST("%main_loc2_Output = OpVariable %_ptr_Output_v4float Output");
    EXPECT_INST(R"(
       %main = OpFunction %void None %16
         %17 = OpLabel
         %18 = OpLoad %v4half %main_loc1_Input None
         %19 = OpFunctionCall %v4float %main_inner %18
               OpStore %main_loc2_Output %19 None
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_ShaderIO_F16_Input_WithoutCapability) {
    auto* input = b.FunctionParam("input", ty.vec4<f16>());
    input->SetLocation(1);
    auto* func = b.Function("main", ty.vec4<f32>(), core::ir::Function::PipelineStage::kFragment);
    func->SetReturnLocation(2);
    func->SetParams({input});
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Convert(ty.vec4<f32>(), input));
    });

    Options options;
    options.use_storage_input_output_16 = false;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(OpEntryPoint Fragment %main "main" %main_loc1_Input %main_loc2_Output)");
    EXPECT_INST("%main_loc1_Input = OpVariable %_ptr_Input_v4float Input");
    EXPECT_INST("%main_loc2_Output = OpVariable %_ptr_Output_v4float Output");
    EXPECT_INST(R"(
       %main = OpFunction %void None %16
         %17 = OpLabel
         %18 = OpLoad %v4float %main_loc1_Input None
         %19 = OpFConvert %v4half %18
         %20 = OpFunctionCall %v4float %main_inner %19
               OpStore %main_loc2_Output %20 None
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_ShaderIO_F16_Output_WithCapability) {
    auto* input = b.FunctionParam("input", ty.vec4<f32>());
    input->SetLocation(1);
    auto* func = b.Function("main", ty.vec4<f16>(), core::ir::Function::PipelineStage::kFragment);
    func->SetReturnLocation(2);
    func->SetParams({input});
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Convert(ty.vec4<f16>(), input));
    });

    Options options;
    options.use_storage_input_output_16 = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST("OpCapability StorageInputOutput16");
    EXPECT_INST(R"(OpEntryPoint Fragment %main "main" %main_loc1_Input %main_loc2_Output)");
    EXPECT_INST("%main_loc1_Input = OpVariable %_ptr_Input_v4float Input");
    EXPECT_INST("%main_loc2_Output = OpVariable %_ptr_Output_v4half Output");
    EXPECT_INST(R"(
       %main = OpFunction %void None %16
         %17 = OpLabel
         %18 = OpLoad %v4float %main_loc1_Input None
         %19 = OpFunctionCall %v4half %main_inner %18
               OpStore %main_loc2_Output %19 None
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_ShaderIO_F16_Output_WithoutCapability) {
    auto* input = b.FunctionParam("input", ty.vec4<f32>());
    input->SetLocation(1);
    auto* func = b.Function("main", ty.vec4<f16>(), core::ir::Function::PipelineStage::kFragment);
    func->SetReturnLocation(2);
    func->SetParams({input});
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Convert(ty.vec4<f16>(), input));
    });

    Options options;
    options.use_storage_input_output_16 = false;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(OpEntryPoint Fragment %main "main" %main_loc1_Input %main_loc2_Output)");
    EXPECT_INST("%main_loc1_Input = OpVariable %_ptr_Input_v4float Input");
    EXPECT_INST("%main_loc2_Output = OpVariable %_ptr_Output_v4float Output");
    EXPECT_INST(R"(
       %main = OpFunction %void None %16
         %17 = OpLabel
         %18 = OpLoad %v4float %main_loc1_Input None
         %19 = OpFunctionCall %v4half %main_inner %18
         %20 = OpFConvert %v4float %19
               OpStore %main_loc2_Output %20 None
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_ShaderIO_DualSourceBlend) {
    auto* outputs =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {
                                                      mod.symbols.Register("a"),
                                                      ty.f32(),
                                                      core::IOAttributes{
                                                          /* location */ 0u,
                                                          /* index */ 0u,
                                                          /* color */ std::nullopt,
                                                          /* builtin */ std::nullopt,
                                                          /* interpolation */ std::nullopt,
                                                          /* invariant */ false,
                                                      },
                                                  },
                                                  {
                                                      mod.symbols.Register("b"),
                                                      ty.f32(),
                                                      core::IOAttributes{
                                                          /* location */ 0u,
                                                          /* index */ 1u,
                                                          /* color */ std::nullopt,
                                                          /* builtin */ std::nullopt,
                                                          /* interpolation */ std::nullopt,
                                                          /* invariant */ false,
                                                      },
                                                  },
                                              });

    auto* func = b.Function("main", outputs, core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(outputs, 0.5_f, 0.6_f));
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(
        R"(OpEntryPoint Fragment %main "main" %main_loc0_idx0_Output %main_loc0_idx1_Output)");
    EXPECT_INST(R"(
               OpDecorate %main_loc0_idx0_Output Location 0
               OpDecorate %main_loc0_idx0_Output Index 0
               OpDecorate %main_loc0_idx1_Output Location 0
               OpDecorate %main_loc0_idx1_Output Index 1
)");
    EXPECT_INST(R"(
%main_loc0_idx0_Output = OpVariable %_ptr_Output_float Output   ; Location 0, Index 0
%main_loc0_idx1_Output = OpVariable %_ptr_Output_float Output   ; Location 0, Index 1
    )");
    EXPECT_INST(R"(
       %main = OpFunction %void None %14
         %15 = OpLabel
         %16 = OpFunctionCall %Outputs %main_inner
         %17 = OpCompositeExtract %float %16 0
               OpStore %main_loc0_idx0_Output %17 None
         %18 = OpCompositeExtract %float %16 1
               OpStore %main_loc0_idx1_Output %18 None
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Function_PassMatrixByPointer) {
    auto* mat_ty = ty.mat3x3<f32>();
    auto* arr = mod.root_block->Append(b.Var("var", ty.ptr(private_, ty.array(mat_ty, 4))));

    auto* target = b.Function("target", mat_ty);
    auto* value_a = b.FunctionParam("value_a", mat_ty);
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* value_b = b.FunctionParam("value_b", mat_ty);
    target->SetParams({value_a, scalar, value_b});
    b.Append(target->Block(), [&] {
        auto* scale = b.Multiply(mat_ty, value_a, scalar);
        auto* sum = b.Add(mat_ty, scale, value_b);
        b.Return(target, sum);
    });

    auto* caller = b.Function("caller", mat_ty);
    b.Append(caller->Block(), [&] {
        auto* mat_ptr = ty.ptr(private_, mat_ty);
        auto* ma = b.Load(b.Access(mat_ptr, arr, 0_u));
        auto* mb = b.Load(b.Access(mat_ptr, arr, 1_u));
        auto* result = b.Call(mat_ty, target, ma, b.Constant(2_f), mb);
        b.Return(caller, result);
    });

    Options options;
    options.pass_matrix_by_pointer = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;

    EXPECT_INST(R"(
               ; Function target
     %target = OpFunction %mat3v3float None %15
         %12 = OpFunctionParameter %_ptr_Function_mat3v3float
     %scalar = OpFunctionParameter %float
         %14 = OpFunctionParameter %_ptr_Function_mat3v3float
         %16 = OpLabel
         %17 = OpLoad %mat3v3float %14 None
         %18 = OpLoad %mat3v3float %12 None
         %19 = OpMatrixTimesScalar %mat3v3float %18 %scalar
         %20 = OpCompositeExtract %v3float %19 0
         %21 = OpCompositeExtract %v3float %17 0
         %22 = OpFAdd %v3float %20 %21
         %23 = OpCompositeExtract %v3float %19 1
         %24 = OpCompositeExtract %v3float %17 1
         %25 = OpFAdd %v3float %23 %24
         %26 = OpCompositeExtract %v3float %19 2
         %27 = OpCompositeExtract %v3float %17 2
         %28 = OpFAdd %v3float %26 %27
         %29 = OpCompositeConstruct %mat3v3float %22 %25 %28
               OpReturnValue %29
               OpFunctionEnd

               ; Function caller
     %caller = OpFunction %mat3v3float None %31
         %32 = OpLabel
         %40 = OpVariable %_ptr_Function_mat3v3float Function
         %41 = OpVariable %_ptr_Function_mat3v3float Function
         %33 = OpAccessChain %_ptr_Private_mat3v3float %var %uint_0
         %36 = OpLoad %mat3v3float %33 None
         %37 = OpAccessChain %_ptr_Private_mat3v3float %var %uint_1
         %39 = OpLoad %mat3v3float %37 None
               OpStore %40 %36
               OpStore %41 %39
         %42 = OpFunctionCall %mat3v3float %target %40 %float_2 %41
               OpReturnValue %42
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, WorkgroupStorageSizeEmpty) {
    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_EQ(0u, workgroup_info.storage_size);
}

TEST_F(SpirvWriterTest, WorkgroupStorageSizeSimple) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.f32())));
    auto* var2 = mod.root_block->Append(b.Var("var2", ty.ptr(workgroup, ty.i32())));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Let("y", var2);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_EQ(32u, workgroup_info.storage_size);
}

TEST_F(SpirvWriterTest, WorkgroupStorageSizeCompoundTypes) {
    Vector members{
        ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(mod.symbols.New("b"), ty.array<i32, 4>(), 1u, 4u, 16u, 64u,
                                         core::IOAttributes{}),
    };

    // This struct should occupy 68 bytes. 4 from the i32 field, and another 64
    // from the 4-element array with 16-byte stride.
    auto* wg_struct_ty = ty.Struct(mod.symbols.New("WgStruct"), members);
    auto* str_var = mod.root_block->Append(b.Var("var_struct", ty.ptr(workgroup, wg_struct_ty)));

    // Plus another 4 bytes from this other workgroup-class f32.
    auto* f32_var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, ty.f32())));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", f32_var);
        b.Let("y", str_var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_EQ(96u, workgroup_info.storage_size);
}

TEST_F(SpirvWriterTest, WorkgroupStorageSizeAlignmentPadding) {
    // vec3<f32> has an alignment of 16 but a size of 12. We leverage this to test
    // that our padded size calculation for workgroup storage is accurate.
    auto* var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, ty.vec3<f32>())));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_EQ(16u, workgroup_info.storage_size);
}

TEST_F(SpirvWriterTest, WorkgroupStorageSizeStructAlignment) {
    // Per WGSL spec, a struct's size is the offset its last member plus the size
    // of its last member, rounded up to the alignment of its largest member. So
    // here the struct is expected to occupy 1024 bytes of workgroup storage.
    Vector members{
        ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 1024u, 4u,
                                         core::IOAttributes{}),
    };

    auto* wg_struct_ty = ty.Struct(mod.symbols.New("WgStruct"), members);
    auto* var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, wg_struct_ty)));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_EQ(1024u, workgroup_info.storage_size);
}

}  // namespace
}  // namespace tint::spirv::writer
