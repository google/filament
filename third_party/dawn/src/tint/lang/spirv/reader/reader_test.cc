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

#include "src/tint/lang/spirv/reader/reader.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/reader/common/helper_test.h"
#include "src/tint/lang/spirv/reader/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvReaderTest, UnsupportedExtension) {
    auto got = Run(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_variable_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)");
    ASSERT_NE(got, Success);
    EXPECT_EQ(got.Failure().reason,
              "SPIR-V extension 'SPV_KHR_variable_pointers' is not supported");
}

TEST_F(SpirvReaderTest, Load_VectorComponent) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
      %vec4u = OpTypeVector %u32 4
    %u32_ptr = OpTypePointer Function %u32
  %vec4u_ptr = OpTypePointer Function %vec4u
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
       %load = OpLoad %u32 %access
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    %3:u32 = load_vector_element %2, 2u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, Store_VectorComponent) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
      %vec4u = OpTypeVector %u32 4
    %u32_ptr = OpTypePointer Function %u32
  %vec4u_ptr = OpTypePointer Function %vec4u
      %u32_2 = OpConstant %u32 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %vec4u_ptr Function
     %access = OpAccessChain %u32_ptr %var %u32_2
               OpStore %access %u32_42
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec4<u32>, read_write> = var undef
    store_vector_element %2, 2u, 42u
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ShaderInputs) {
    auto got = Run(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %coord %colors
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %coord BuiltIn FragCoord
               OpDecorate %colors Location 1
               OpMemberDecorate %str 1 NoPerspective
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void
        %str = OpTypeStruct %vec4f %vec4f
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1

%_ptr_Input_vec4f = OpTypePointer Input %vec4f
  %_ptr_Input_str = OpTypePointer Input %str
      %coord = OpVariable %_ptr_Input_vec4f Input
     %colors = OpVariable %_ptr_Input_str Input

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
   %access_a = OpAccessChain %_ptr_Input_vec4f %colors %u32_0
   %access_b = OpAccessChain %_ptr_Input_vec4f %colors %u32_1
          %a = OpLoad %vec4f %access_a
          %b = OpLoad %vec4f %access_b
          %c = OpLoad %vec4f %coord
        %mul = OpFMul %vec4f %a %b
        %add = OpFAdd %vec4f %mul %c
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
tint_symbol_2 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0)
  tint_symbol_1:vec4<f32> @offset(16)
}

%main = @fragment func(%2:vec4<f32> [@position], %3:vec4<f32> [@location(1)], %4:vec4<f32> [@location(2), @interpolate(linear)]):void {
  $B1: {
    %5:tint_symbol_2 = construct %3, %4
    %6:vec4<f32> = access %5, 0u
    %7:vec4<f32> = access %5, 1u
    %8:vec4<f32> = mul %6, %7
    %9:vec4<f32> = add %8, %2
    ret
  }
}
)");
}

TEST_F(SpirvReaderTest, ShaderOutputs) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %depth %colors
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main DepthReplacing
               OpDecorate %depth BuiltIn FragDepth
               OpDecorate %colors Location 1
               OpMemberDecorate %str 1 NoPerspective
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void
        %str = OpTypeStruct %vec4f %vec4f
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1
     %f32_42 = OpConstant %f32 42.0
     %f32_n1 = OpConstant %f32 -1.0
   %f32_v4_a = OpConstantComposite %vec4f %f32_42 %f32_42 %f32_42 %f32_n1
   %f32_v4_b = OpConstantComposite %vec4f %f32_n1 %f32_n1 %f32_n1 %f32_42

%_ptr_Output_f32 = OpTypePointer Output %f32
%_ptr_Output_vec4f = OpTypePointer Output %vec4f
  %_ptr_Output_str = OpTypePointer Output %str
      %depth = OpVariable %_ptr_Output_f32 Output
     %colors = OpVariable %_ptr_Output_str Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
   %access_a = OpAccessChain %_ptr_Output_vec4f %colors %u32_0
   %access_b = OpAccessChain %_ptr_Output_vec4f %colors %u32_1
               OpStore %access_a %f32_v4_a
               OpStore %access_b %f32_v4_b
               OpStore %depth %f32_42
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
tint_symbol_2 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0)
  tint_symbol_1:vec4<f32> @offset(16)
}

tint_symbol_4 = struct @align(16) {
  tint_symbol_3:f32 @offset(0), @builtin(frag_depth)
  tint_symbol:vec4<f32> @offset(16), @location(1)
  tint_symbol_1:vec4<f32> @offset(32), @location(2), @interpolate(linear)
}

$B1: {  # root
  %1:ptr<private, f32, read_write> = var undef
  %2:ptr<private, tint_symbol_2, read_write> = var undef
}

%main_inner = func():void {
  $B2: {
    %4:ptr<private, vec4<f32>, read_write> = access %2, 0u
    %5:ptr<private, vec4<f32>, read_write> = access %2, 1u
    store %4, vec4<f32>(42.0f, 42.0f, 42.0f, -1.0f)
    store %5, vec4<f32>(-1.0f, -1.0f, -1.0f, 42.0f)
    store %1, 42.0f
    ret
  }
}
%main = @fragment func():tint_symbol_4 {
  $B3: {
    %7:void = call %main_inner
    %8:f32 = load %1
    %9:ptr<private, vec4<f32>, read_write> = access %2, 0u
    %10:vec4<f32> = load %9
    %11:ptr<private, vec4<f32>, read_write> = access %2, 1u
    %12:vec4<f32> = load %11
    %13:tint_symbol_4 = construct %8, %10, %12
    ret %13
  }
}
)");
}

TEST_F(SpirvReaderTest, ClipDistances) {
    auto got = Run(R"(
               OpCapability Shader
               OpCapability ClipDistance
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %main_position_Output %main_clip_distances_Output %main___point_size_Output
               OpName %main_position_Output "main_position_Output"
               OpName %main_clip_distances_Output "main_clip_distances_Output"
               OpName %main___point_size_Output "main___point_size_Output"
               OpName %main_inner "main_inner"
               OpMemberName %VertexOutputs 0 "position"
               OpMemberName %VertexOutputs 1 "clipDistance"
               OpName %VertexOutputs "VertexOutputs"
               OpName %main "main"
               OpDecorate %main_position_Output BuiltIn Position
               OpDecorate %_arr_float_uint_1 ArrayStride 4
               OpDecorate %main_clip_distances_Output BuiltIn ClipDistance
               OpDecorate %main___point_size_Output BuiltIn PointSize
               OpMemberDecorate %VertexOutputs 0 Offset 0
               OpMemberDecorate %VertexOutputs 1 Offset 16
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main_position_Output = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%_ptr_Output__arr_float_uint_1 = OpTypePointer Output %_arr_float_uint_1
%main_clip_distances_Output = OpVariable %_ptr_Output__arr_float_uint_1 Output
%_ptr_Output_float = OpTypePointer Output %float
%main___point_size_Output = OpVariable %_ptr_Output_float Output
%VertexOutputs = OpTypeStruct %v4float %_arr_float_uint_1
         %14 = OpTypeFunction %VertexOutputs
         %16 = OpConstantNull %VertexOutputs
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
 %main_inner = OpFunction %VertexOutputs None %14
         %15 = OpLabel
               OpReturnValue %16
               OpFunctionEnd
       %main = OpFunction %void None %19
         %20 = OpLabel
         %21 = OpFunctionCall %VertexOutputs %main_inner
         %22 = OpCompositeExtract %v4float %21 0
               OpStore %main_position_Output %22 None
         %23 = OpCompositeExtract %_arr_float_uint_1 %21 1
               OpStore %main_clip_distances_Output %23 None
               OpStore %main___point_size_Output %float_1 None
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
VertexOutputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clipDistance:array<f32, 1> @offset(16)
}

tint_symbol = struct @align(16) {
  main_position_Output:vec4<f32> @offset(0), @builtin(position)
  main_clip_distances_Output:array<f32, 1> @offset(16), @builtin(clip_distances)
}

$B1: {  # root
  %main_position_Output:ptr<private, vec4<f32>, read_write> = var undef
  %main_clip_distances_Output:ptr<private, array<f32, 1>, read_write> = var undef
  %main___point_size_Output:ptr<private, f32, read_write> = var 1.0f
}

%main_inner = func():VertexOutputs {
  $B2: {
    ret VertexOutputs(vec4<f32>(0.0f), array<f32, 1>(0.0f))
  }
}
%main_inner_1 = func():void {  # %main_inner_1: 'main_inner'
  $B3: {
    %6:VertexOutputs = call %main_inner
    %7:vec4<f32> = access %6, 0u
    store %main_position_Output, %7
    %8:array<f32, 1> = access %6, 1u
    store %main_clip_distances_Output, %8
    store %main___point_size_Output, 1.0f
    ret
  }
}
%main = @vertex func():tint_symbol {
  $B4: {
    %10:void = call %main_inner_1
    %11:vec4<f32> = load %main_position_Output
    %12:array<f32, 1> = load %main_clip_distances_Output
    %13:tint_symbol = construct %11, %12
    ret %13
  }
}
)");
}

TEST_F(SpirvReaderTest, ClipDistances_gl_PerVertex) {
    auto got = Run(R"(
               OpCapability Shader
               OpCapability ClipDistance
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_
               OpSource GLSL 450
               OpName %main "main"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_ClipDistance"
               OpName %_ ""
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn ClipDistance
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%gl_PerVertex = OpTypeStruct %v4float %_arr_float_uint_2
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
         %21 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpAccessChain %_ptr_Output_float %_ %int_1 %int_0
               OpStore %19 %float_0
         %20 = OpAccessChain %_ptr_Output_float %_ %int_1 %int_1
               OpStore %20 %float_0
         %23 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %23 %21
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
gl_PerVertex = struct @align(16) {
  gl_Position:vec4<f32> @offset(0)
  gl_ClipDistance:array<f32, 2> @offset(16)
}

tint_symbol = struct @align(16) {
  gl_Position:vec4<f32> @offset(0), @builtin(position)
  gl_ClipDistance:array<f32, 2> @offset(16), @builtin(clip_distances)
}

$B1: {  # root
  %1:ptr<private, gl_PerVertex, read_write> = var undef
}

%main_inner = func():void {
  $B2: {
    %3:ptr<private, f32, read_write> = access %1, 1i, 0i
    store %3, 0.0f
    %4:ptr<private, f32, read_write> = access %1, 1i, 1i
    store %4, 0.0f
    %5:ptr<private, vec4<f32>, read_write> = access %1, 0i
    store %5, vec4<f32>(0.0f)
    ret
  }
}
%main = @vertex func():tint_symbol {
  $B3: {
    %7:void = call %main_inner
    %8:ptr<private, vec4<f32>, read_write> = access %1, 0u
    %9:vec4<f32> = load %8
    %10:ptr<private, array<f32, 2>, read_write> = access %1, 1u
    %11:array<f32, 2> = load %10
    %12:tint_symbol = construct %9, %11
    ret %12
  }
}
)");
}

TEST_F(SpirvReaderTest, SampleMask) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %mask_in %mask_out
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %mask_in BuiltIn SampleMask
               OpDecorate %mask_out BuiltIn SampleMask
       %void = OpTypeVoid
    %fn_type = OpTypeFunction %void
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1
    %arr_u32 = OpTypeArray %u32 %u32_1

%_ptr_Input_u32 = OpTypePointer Input %u32
%_ptr_Input_arr_u32 = OpTypePointer Input %arr_u32
%_ptr_Output_u32 = OpTypePointer Output %u32
%_ptr_Output_arr_u32 = OpTypePointer Output %arr_u32
    %mask_in = OpVariable %_ptr_Input_arr_u32 Input
   %mask_out = OpVariable %_ptr_Output_arr_u32 Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
  %mask_in_0 = OpAccessChain %_ptr_Input_u32 %mask_in %u32_0
%mask_in_val = OpLoad %u32 %mask_in_0
   %plus_one = OpIAdd %u32 %mask_in_val %u32_1
 %mask_out_0 = OpAccessChain %_ptr_Output_u32 %mask_out %u32_0
               OpStore %mask_out_0 %plus_one
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
$B1: {  # root
  %1:ptr<private, array<u32, 1>, read_write> = var undef
}

%main_inner = func(%3:array<u32, 1>):void {
  $B2: {
    %4:u32 = access %3, 0u
    %5:u32 = add %4, 1u
    %6:ptr<private, u32, read_write> = access %1, 0u
    store %6, %5
    ret
  }
}
%main = @fragment func(%8:u32 [@sample_mask]):u32 [@sample_mask] {
  $B3: {
    %9:array<u32, 1> = construct %8
    %10:void = call %main_inner, %9
    %11:array<u32, 1> = load %1
    %12:u32 = access %11, 0u
    ret %12
  }
}
)");
}

TEST_F(SpirvReaderTest, BlendSrc) {
    auto got = Run(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag_main "frag_main" %frag_main_loc0_idx0_Output %frag_main_loc0_idx1_Output
               OpExecutionMode %frag_main OriginUpperLeft
               OpName %frag_main_loc0_idx0_Output "frag_main_loc0_idx0_Output"
               OpName %frag_main_loc0_idx1_Output "frag_main_loc0_idx1_Output"
               OpName %frag_main_inner "frag_main_inner"
               OpMemberName %FragOutput 0 "color"
               OpMemberName %FragOutput 1 "blend"
               OpName %FragOutput "FragOutput"
               OpName %output "output"
               OpName %frag_main "frag_main"
               OpDecorate %frag_main_loc0_idx0_Output Location 0
               OpDecorate %frag_main_loc0_idx0_Output Index 0
               OpDecorate %frag_main_loc0_idx1_Output Location 0
               OpDecorate %frag_main_loc0_idx1_Output Index 1
               OpMemberDecorate %FragOutput 0 Offset 0
               OpMemberDecorate %FragOutput 1 Offset 16
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%frag_main_loc0_idx0_Output = OpVariable %_ptr_Output_v4float Output
%frag_main_loc0_idx1_Output = OpVariable %_ptr_Output_v4float Output
 %FragOutput = OpTypeStruct %v4float %v4float
          %8 = OpTypeFunction %FragOutput
%_ptr_Function_FragOutput = OpTypePointer Function %FragOutput
         %12 = OpConstantNull %FragOutput
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
     %uint_1 = OpConstant %uint 1
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%frag_main_inner = OpFunction %FragOutput None %8
          %9 = OpLabel
     %output = OpVariable %_ptr_Function_FragOutput Function %12
         %13 = OpAccessChain %_ptr_Function_v4float %output %uint_0
               OpStore %13 %17 None
         %20 = OpAccessChain %_ptr_Function_v4float %output %uint_1
               OpStore %20 %17 None
         %22 = OpLoad %FragOutput %output None
               OpReturnValue %22
               OpFunctionEnd
  %frag_main = OpFunction %void None %25
         %26 = OpLabel
         %27 = OpFunctionCall %FragOutput %frag_main_inner
         %28 = OpCompositeExtract %v4float %27 0
               OpStore %frag_main_loc0_idx0_Output %28 None
         %29 = OpCompositeExtract %v4float %27 1
               OpStore %frag_main_loc0_idx1_Output %29 None
               OpReturn
               OpFunctionEnd
)");

    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
FragOutput = struct @align(16) {
  color:vec4<f32> @offset(0)
  blend:vec4<f32> @offset(16)
}

tint_symbol = struct @align(16) {
  frag_main_loc0_idx0_Output:vec4<f32> @offset(0), @location(0), @blend_src(0)
  frag_main_loc0_idx1_Output:vec4<f32> @offset(16), @location(0), @blend_src(1)
}

$B1: {  # root
  %frag_main_loc0_idx0_Output:ptr<private, vec4<f32>, read_write> = var undef
  %frag_main_loc0_idx1_Output:ptr<private, vec4<f32>, read_write> = var undef
}

%frag_main_inner = func():FragOutput {
  $B2: {
    %output:ptr<function, FragOutput, read_write> = var FragOutput(vec4<f32>(0.0f))
    %5:ptr<function, vec4<f32>, read_write> = access %output, 0u
    store %5, vec4<f32>(0.5f, 0.5f, 0.5f, 1.0f)
    %6:ptr<function, vec4<f32>, read_write> = access %output, 1u
    store %6, vec4<f32>(0.5f, 0.5f, 0.5f, 1.0f)
    %7:FragOutput = load %output
    ret %7
  }
}
%frag_main_inner_1 = func():void {  # %frag_main_inner_1: 'frag_main_inner'
  $B3: {
    %9:FragOutput = call %frag_main_inner
    %10:vec4<f32> = access %9, 0u
    store %frag_main_loc0_idx0_Output, %10
    %11:vec4<f32> = access %9, 1u
    store %frag_main_loc0_idx1_Output, %11
    ret
  }
}
%frag_main = @fragment func():tint_symbol {
  $B4: {
    %13:void = call %frag_main_inner_1
    %14:vec4<f32> = load %frag_main_loc0_idx0_Output
    %15:vec4<f32> = load %frag_main_loc0_idx1_Output
    %16:tint_symbol = construct %14, %15
    ret %16
  }
}
)");
}

TEST_F(SpirvReaderTest, MultipleEntryPoints) {
    auto got = Run(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %fs_main "fs_main" %coord %colors
               OpEntryPoint GLCompute %cs_main "cs_main"
               OpExecutionMode %fs_main OriginUpperLeft
               OpExecutionMode %cs_main LocalSize 1 1 1
               OpDecorate %coord BuiltIn FragCoord
               OpDecorate %colors Location 1
               OpMemberDecorate %str 1 NoPerspective
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void
        %str = OpTypeStruct %vec4f %vec4f
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1

%_ptr_Input_vec4f = OpTypePointer Input %vec4f
  %_ptr_Input_str = OpTypePointer Input %str
      %coord = OpVariable %_ptr_Input_vec4f Input
     %colors = OpVariable %_ptr_Input_str Input

       %fs_main = OpFunction %void None %fn_type
 %fs_main_start = OpLabel
   %access_a = OpAccessChain %_ptr_Input_vec4f %colors %u32_0
   %access_b = OpAccessChain %_ptr_Input_vec4f %colors %u32_1
          %a = OpLoad %vec4f %access_a
          %b = OpLoad %vec4f %access_b
          %c = OpLoad %vec4f %coord
        %mul = OpFMul %vec4f %a %b
        %add = OpFAdd %vec4f %mul %c
               OpReturn
               OpFunctionEnd

    %cs_main = OpFunction %void None %fn_type
 %cs_main_start = OpLabel
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
tint_symbol_2 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0)
  tint_symbol_1:vec4<f32> @offset(16)
}

%fs_main = @fragment func(%2:vec4<f32> [@position], %3:vec4<f32> [@location(1)], %4:vec4<f32> [@location(2), @interpolate(linear)]):void {
  $B1: {
    %5:tint_symbol_2 = construct %3, %4
    %6:vec4<f32> = access %5, 0u
    %7:vec4<f32> = access %5, 1u
    %8:vec4<f32> = mul %6, %7
    %9:vec4<f32> = add %8, %2
    ret
  }
}
%cs_main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

// In Vulkan it is valid to have a texture and sampler with the same set and binding, so we use the
// `ResolveBindingConflictsPass` SPIR-V tools pass to remove the conflicts.
// See crbug.com/435251397.
TEST_F(SpirvReaderTest, ResolveBindingConflicts) {
    auto got = Run(R"(
; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 40
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PS_Viewport "PS_Viewport" %in_var_TEXCOORD0 %out_var_SV_Target
               OpExecutionMode %PS_Viewport OriginUpperLeft
               OpSource HLSL 600
               OpName %type_constants_ "type.constants_"
               OpMemberName %type_constants_ 0 "constants"
               OpName %Constants "Constants"
               OpMemberName %Constants 0 "color"
               OpMemberName %Constants 1 "scale"
               OpMemberName %Constants 2 "padding"
               OpName %constants_ "constants_"
               OpName %type_2d_image "type.2d.image"
               OpName %tex "tex"
               OpName %type_sampler "type.sampler"
               OpName %samplerTex "samplerTex"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpName %PS_Viewport "PS_Viewport"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %constants_ DescriptorSet 0
               OpDecorate %constants_ Binding 0
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
               OpDecorate %samplerTex DescriptorSet 0
               OpDecorate %samplerTex Binding 0
               OpMemberDecorate %Constants 0 Offset 0
               OpMemberDecorate %Constants 1 Offset 16
               OpMemberDecorate %Constants 2 Offset 24
               OpMemberDecorate %type_constants_ 0 Offset 0
               OpDecorate %type_constants_ Block
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
  %Constants = OpTypeStruct %v4float %v2float %v2float
%type_constants_ = OpTypeStruct %Constants
%_ptr_Uniform_type_constants_ = OpTypePointer Uniform %type_constants_
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%type_sampled_image = OpTypeSampledImage %type_2d_image
       %bool = OpTypeBool
 %constants_ = OpVariable %_ptr_Uniform_type_constants_ Uniform
        %tex = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
 %samplerTex = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%PS_Viewport = OpFunction %void None %24
         %27 = OpLabel
         %28 = OpLoad %v2float %in_var_TEXCOORD0
         %29 = OpAccessChain %_ptr_Uniform_v4float %constants_ %int_0 %int_0
         %30 = OpLoad %v4float %29
         %31 = OpLoad %type_2d_image %tex
         %32 = OpLoad %type_sampler %samplerTex
         %33 = OpSampledImage %type_sampled_image %31 %32
         %34 = OpImageSampleImplicitLod %v4float %33 %28 None
         %35 = OpFMul %v4float %30 %34
         %36 = OpCompositeExtract %float %35 3
         %37 = OpFOrdLessThan %bool %36 %float_0
               OpSelectionMerge %38 None
               OpBranchConditional %37 %39 %38
         %39 = OpLabel
               OpKill
         %38 = OpLabel
               OpStore %out_var_SV_Target %35
               OpReturn
               OpFunctionEnd
)");
    ASSERT_EQ(got, Success);
    EXPECT_EQ(got, R"(
Constants = struct @align(16) {
  color:vec4<f32> @offset(0)
  scale:vec2<f32> @offset(16)
  padding:vec2<f32> @offset(24)
}

type.constants_ = struct @align(16) {
  constants:Constants @offset(0)
}

$B1: {  # root
  %constants_:ptr<uniform, type.constants_, read> = var undef @binding_point(0, 0)
  %tex:ptr<handle, texture_2d<f32>, read> = var undef @binding_point(0, 1)
  %samplerTex:ptr<handle, sampler, read> = var undef @binding_point(0, 2)
  %out.var.SV_Target:ptr<private, vec4<f32>, read_write> = var undef
}

%PS_Viewport_inner = func(%in.var.TEXCOORD0:vec2<f32>):void {
  $B2: {
    %7:ptr<uniform, vec4<f32>, read> = access %constants_, 0i, 0i
    %8:vec4<f32> = load %7
    %9:texture_2d<f32> = load %tex
    %10:sampler = load %samplerTex
    %11:vec4<f32> = textureSample %9, %10, %in.var.TEXCOORD0
    %12:vec4<f32> = mul %8, %11
    %13:f32 = access %12, 3u
    %14:bool = lt %13, 0.0f
    if %14 [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        discard
        ret
      }
      $B4: {  # false
        exit_if  # if_1
      }
    }
    store %out.var.SV_Target, %12
    ret
  }
}
%PS_Viewport = @fragment func(%in.var.TEXCOORD0_1:vec2<f32> [@location(0)]):vec4<f32> [@location(0)] {  # %in.var.TEXCOORD0_1: 'in.var.TEXCOORD0'
  $B5: {
    %17:void = call %PS_Viewport_inner, %in.var.TEXCOORD0_1
    %18:vec4<f32> = load %out.var.SV_Target
    ret %18
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
