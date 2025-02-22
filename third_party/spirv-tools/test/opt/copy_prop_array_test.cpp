// Copyright (c) 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "gmock/gmock.h"
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"

namespace spvtools {
namespace opt {
namespace {

using CopyPropArrayPassTest = PassTest<::testing::Test>;

TEST_F(CopyPropArrayPassTest, BasicPropagateArray) {
  const std::string before =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
; CHECK: OpFunction
; CHECK: OpLabel
; CHECK: OpVariable
; CHECK: OpAccessChain
; CHECK: [[new_address:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
; CHECK: [[element_ptr:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[new_address]] %24
; CHECK: [[load:%\w+]] = OpLoad %v4float [[element_ptr]]
; CHECK: OpStore %out_var_SV_Target [[load]]
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 1
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %23 %35
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, BasicPropagateArrayWithName) {
  const std::string before =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %local "local"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
; CHECK: OpFunction
; CHECK: OpLabel
; CHECK: OpVariable
; CHECK: OpAccessChain
; CHECK: [[new_address:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
; CHECK: [[element_ptr:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[new_address]] %24
; CHECK: [[load:%\w+]] = OpLoad %v4float [[element_ptr]]
; CHECK: OpStore %out_var_SV_Target [[load]]
%main = OpFunction %void None %13
%22 = OpLabel
%local = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 1
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %local %35
%36 = OpAccessChain %_ptr_Function_v4float %local %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

// Propagate 2d array.  This test identifying a copy through multiple levels.
// Also has to traverse multiple OpAccessChains.
TEST_F(CopyPropArrayPassTest, Propagate2DArray) {
  const std::string text =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_2 ArrayStride 16
OpDecorate %_arr__arr_v4float_uint_2_uint_2 ArrayStride 32
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr__arr_v4float_uint_2_uint_2 = OpTypeArray %_arr_v4float_uint_2 %uint_2
%type_MyCBuffer = OpTypeStruct %_arr__arr_v4float_uint_2_uint_2
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%14 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_2_0 = OpTypeArray %v4float %uint_2
%_arr__arr_v4float_uint_2_0_uint_2 = OpTypeArray %_arr_v4float_uint_2_0 %uint_2
%_ptr_Function__arr__arr_v4float_uint_2_0_uint_2 = OpTypePointer Function %_arr__arr_v4float_uint_2_0_uint_2
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 = OpTypePointer Uniform %_arr__arr_v4float_uint_2_uint_2
%_ptr_Function__arr_v4float_uint_2_0 = OpTypePointer Function %_arr_v4float_uint_2_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
; CHECK: OpFunction
; CHECK: OpLabel
; CHECK: OpVariable
; CHECK: OpVariable
; CHECK: OpAccessChain
; CHECK: [[new_address:%\w+]] = OpAccessChain %_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 %MyCBuffer %int_0
%main = OpFunction %void None %14
%25 = OpLabel
%26 = OpVariable %_ptr_Function__arr_v4float_uint_2_0 Function
%27 = OpVariable %_ptr_Function__arr__arr_v4float_uint_2_0_uint_2 Function
%28 = OpLoad %int %in_var_INDEX
%29 = OpAccessChain %_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 %MyCBuffer %int_0
%30 = OpLoad %_arr__arr_v4float_uint_2_uint_2 %29
%31 = OpCompositeExtract %_arr_v4float_uint_2 %30 0
%32 = OpCompositeExtract %v4float %31 0
%33 = OpCompositeExtract %v4float %31 1
%34 = OpCompositeConstruct %_arr_v4float_uint_2_0 %32 %33
%35 = OpCompositeExtract %_arr_v4float_uint_2 %30 1
%36 = OpCompositeExtract %v4float %35 0
%37 = OpCompositeExtract %v4float %35 1
%38 = OpCompositeConstruct %_arr_v4float_uint_2_0 %36 %37
%39 = OpCompositeConstruct %_arr__arr_v4float_uint_2_0_uint_2 %34 %38
; CHECK: OpStore
OpStore %27 %39
%40 = OpAccessChain %_ptr_Function__arr_v4float_uint_2_0 %27 %28
%42 = OpAccessChain %_ptr_Function_v4float %40 %28
%43 = OpLoad %v4float %42
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_2 [[new_address]] %28
; CHECK: [[ac2:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[ac1]] %28
; CHECK: [[load:%\w+]] = OpLoad %v4float [[ac2]]
; CHECK: OpStore %out_var_SV_Target [[load]]
OpStore %out_var_SV_Target %43
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(text, false);
}

// Propagate 2d array.  This test identifying a copy through multiple levels.
// Also has to traverse multiple OpAccessChains.
TEST_F(CopyPropArrayPassTest, Propagate2DArrayWithMultiLevelExtract) {
  const std::string text =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_2 ArrayStride 16
OpDecorate %_arr__arr_v4float_uint_2_uint_2 ArrayStride 32
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr__arr_v4float_uint_2_uint_2 = OpTypeArray %_arr_v4float_uint_2 %uint_2
%type_MyCBuffer = OpTypeStruct %_arr__arr_v4float_uint_2_uint_2
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%14 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_2_0 = OpTypeArray %v4float %uint_2
%_arr__arr_v4float_uint_2_0_uint_2 = OpTypeArray %_arr_v4float_uint_2_0 %uint_2
%_ptr_Function__arr__arr_v4float_uint_2_0_uint_2 = OpTypePointer Function %_arr__arr_v4float_uint_2_0_uint_2
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 = OpTypePointer Uniform %_arr__arr_v4float_uint_2_uint_2
%_ptr_Function__arr_v4float_uint_2_0 = OpTypePointer Function %_arr_v4float_uint_2_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
; CHECK: OpFunction
; CHECK: OpLabel
; CHECK: OpVariable
; CHECK: OpVariable
; CHECK: OpAccessChain
; CHECK: [[new_address:%\w+]] = OpAccessChain %_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 %MyCBuffer %int_0
%main = OpFunction %void None %14
%25 = OpLabel
%26 = OpVariable %_ptr_Function__arr_v4float_uint_2_0 Function
%27 = OpVariable %_ptr_Function__arr__arr_v4float_uint_2_0_uint_2 Function
%28 = OpLoad %int %in_var_INDEX
%29 = OpAccessChain %_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 %MyCBuffer %int_0
%30 = OpLoad %_arr__arr_v4float_uint_2_uint_2 %29
%32 = OpCompositeExtract %v4float %30 0 0
%33 = OpCompositeExtract %v4float %30 0 1
%34 = OpCompositeConstruct %_arr_v4float_uint_2_0 %32 %33
%36 = OpCompositeExtract %v4float %30 1 0
%37 = OpCompositeExtract %v4float %30 1 1
%38 = OpCompositeConstruct %_arr_v4float_uint_2_0 %36 %37
%39 = OpCompositeConstruct %_arr__arr_v4float_uint_2_0_uint_2 %34 %38
; CHECK: OpStore
OpStore %27 %39
%40 = OpAccessChain %_ptr_Function__arr_v4float_uint_2_0 %27 %28
%42 = OpAccessChain %_ptr_Function_v4float %40 %28
%43 = OpLoad %v4float %42
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_2 [[new_address]] %28
; CHECK: [[ac2:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[ac1]] %28
; CHECK: [[load:%\w+]] = OpLoad %v4float [[ac2]]
; CHECK: OpStore %out_var_SV_Target [[load]]
OpStore %out_var_SV_Target %43
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(text, false);
}

// Test decomposing an object when we need to "rewrite" a store.
TEST_F(CopyPropArrayPassTest, DecomposeObjectForArrayStore) {
  const std::string text =
      R"(               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_MyCBuffer "type.MyCBuffer"
               OpMemberName %type_MyCBuffer 0 "Data"
               OpName %MyCBuffer "MyCBuffer"
               OpName %main "main"
               OpName %in_var_INDEX "in.var.INDEX"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpDecorate %_arr_v4float_uint_2 ArrayStride 16
               OpDecorate %_arr__arr_v4float_uint_2_uint_2 ArrayStride 32
               OpMemberDecorate %type_MyCBuffer 0 Offset 0
               OpDecorate %type_MyCBuffer Block
               OpDecorate %in_var_INDEX Flat
               OpDecorate %in_var_INDEX Location 0
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %MyCBuffer DescriptorSet 0
               OpDecorate %MyCBuffer Binding 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr__arr_v4float_uint_2_uint_2 = OpTypeArray %_arr_v4float_uint_2 %uint_2
%type_MyCBuffer = OpTypeStruct %_arr__arr_v4float_uint_2_uint_2
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_2_0 = OpTypeArray %v4float %uint_2
%_arr__arr_v4float_uint_2_0_uint_2 = OpTypeArray %_arr_v4float_uint_2_0 %uint_2
%_ptr_Function__arr__arr_v4float_uint_2_0_uint_2 = OpTypePointer Function %_arr__arr_v4float_uint_2_0_uint_2
      %int_0 = OpConstant %int 0
%_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 = OpTypePointer Uniform %_arr__arr_v4float_uint_2_uint_2
%_ptr_Function__arr_v4float_uint_2_0 = OpTypePointer Function %_arr_v4float_uint_2_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
  %MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %14
         %25 = OpLabel
         %26 = OpVariable %_ptr_Function__arr_v4float_uint_2_0 Function
         %27 = OpVariable %_ptr_Function__arr__arr_v4float_uint_2_0_uint_2 Function
         %28 = OpLoad %int %in_var_INDEX
         %29 = OpAccessChain %_ptr_Uniform__arr__arr_v4float_uint_2_uint_2 %MyCBuffer %int_0
         %30 = OpLoad %_arr__arr_v4float_uint_2_uint_2 %29
         %31 = OpCompositeExtract %_arr_v4float_uint_2 %30 0
         %32 = OpCompositeExtract %v4float %31 0
         %33 = OpCompositeExtract %v4float %31 1
         %34 = OpCompositeConstruct %_arr_v4float_uint_2_0 %32 %33
         %35 = OpCompositeExtract %_arr_v4float_uint_2 %30 1
         %36 = OpCompositeExtract %v4float %35 0
         %37 = OpCompositeExtract %v4float %35 1
         %38 = OpCompositeConstruct %_arr_v4float_uint_2_0 %36 %37
         %39 = OpCompositeConstruct %_arr__arr_v4float_uint_2_0_uint_2 %34 %38
               OpStore %27 %39
; CHECK: [[access_chain:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_2
         %40 = OpAccessChain %_ptr_Function__arr_v4float_uint_2_0 %27 %28
; CHECK: [[load:%\w+]] = OpLoad %_arr_v4float_uint_2 [[access_chain]]
         %41 = OpLoad %_arr_v4float_uint_2_0 %40
; CHECK: [[extract1:%\w+]] = OpCompositeExtract %v4float [[load]] 0
; CHECK: [[extract2:%\w+]] = OpCompositeExtract %v4float [[load]] 1
; CHECK: [[construct:%\w+]] = OpCompositeConstruct %_arr_v4float_uint_2_0 [[extract1]] [[extract2]]
; CHECK: OpStore %26 [[construct]]
               OpStore %26 %41
         %42 = OpAccessChain %_ptr_Function_v4float %26 %28
         %43 = OpLoad %v4float %42
               OpStore %out_var_SV_Target %43
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(text, false);
}

// Test decomposing an object when we need to "rewrite" a store.
TEST_F(CopyPropArrayPassTest, DecomposeObjectForStructStore) {
  const std::string text =
      R"(               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_MyCBuffer "type.MyCBuffer"
               OpMemberName %type_MyCBuffer 0 "Data"
               OpName %MyCBuffer "MyCBuffer"
               OpName %main "main"
               OpName %in_var_INDEX "in.var.INDEX"
               OpName %out_var_SV_Target "out.var.SV_Target"
               OpMemberDecorate %type_MyCBuffer 0 Offset 0
               OpDecorate %type_MyCBuffer Block
               OpDecorate %in_var_INDEX Flat
               OpDecorate %in_var_INDEX Location 0
               OpDecorate %out_var_SV_Target Location 0
               OpDecorate %MyCBuffer DescriptorSet 0
               OpDecorate %MyCBuffer Binding 0
; CHECK: OpDecorate [[decorated_type:%\w+]] GLSLPacked
               OpDecorate %struct GLSLPacked
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
; CHECK: [[decorated_type]] = OpTypeStruct
%struct = OpTypeStruct %float %uint
%_arr_struct_uint_2 = OpTypeArray %struct %uint_2
%type_MyCBuffer = OpTypeStruct %_arr_struct_uint_2
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
; CHECK: [[struct:%\w+]] = OpTypeStruct %float %uint
%struct_0 = OpTypeStruct %float %uint
%_arr_struct_0_uint_2 = OpTypeArray %struct_0 %uint_2
%_ptr_Function__arr_struct_0_uint_2 = OpTypePointer Function %_arr_struct_0_uint_2
      %int_0 = OpConstant %int 0
%_ptr_Uniform__arr_struct_uint_2 = OpTypePointer Uniform %_arr_struct_uint_2
; CHECK: [[decorated_ptr:%\w+]] = OpTypePointer Uniform [[decorated_type]]
%_ptr_Function_struct_0 = OpTypePointer Function %struct_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
  %MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %14
         %25 = OpLabel
         %26 = OpVariable %_ptr_Function_struct_0 Function
         %27 = OpVariable %_ptr_Function__arr_struct_0_uint_2 Function
         %28 = OpLoad %int %in_var_INDEX
         %29 = OpAccessChain %_ptr_Uniform__arr_struct_uint_2 %MyCBuffer %int_0
         %30 = OpLoad %_arr_struct_uint_2 %29
         %31 = OpCompositeExtract %struct %30 0
         %32 = OpCompositeExtract %v4float %31 0
         %33 = OpCompositeExtract %v4float %31 1
         %34 = OpCompositeConstruct %struct_0 %32 %33
         %35 = OpCompositeExtract %struct %30 1
         %36 = OpCompositeExtract %float %35 0
         %37 = OpCompositeExtract %uint %35 1
         %38 = OpCompositeConstruct %struct_0 %36 %37
         %39 = OpCompositeConstruct %_arr_struct_0_uint_2 %34 %38
               OpStore %27 %39
; CHECK: [[access_chain:%\w+]] = OpAccessChain [[decorated_ptr]]
         %40 = OpAccessChain %_ptr_Function_struct_0 %27 %28
; CHECK: [[load:%\w+]] = OpLoad [[decorated_type]] [[access_chain]]
         %41 = OpLoad %struct_0 %40
; CHECK: [[extract1:%\w+]] = OpCompositeExtract %float [[load]] 0
; CHECK: [[extract2:%\w+]] = OpCompositeExtract %uint [[load]] 1
; CHECK: [[construct:%\w+]] = OpCompositeConstruct [[struct]] [[extract1]] [[extract2]]
; CHECK: OpStore %26 [[construct]]
               OpStore %26 %41
         %42 = OpAccessChain %_ptr_Function_v4float %26 %28
         %43 = OpLoad %v4float %42
               OpStore %out_var_SV_Target %43
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(text, false);
}

TEST_F(CopyPropArrayPassTest, CopyViaInserts) {
  const std::string before =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
; CHECK: OpFunction
; CHECK: OpLabel
; CHECK: OpVariable
; CHECK: OpAccessChain
; CHECK: [[new_address:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
; CHECK: [[element_ptr:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[new_address]] %24
; CHECK: [[load:%\w+]] = OpLoad %v4float [[element_ptr]]
; CHECK: OpStore %out_var_SV_Target [[load]]
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%undef = OpUndef %_arr_v4float_uint_8_0
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%i0 = OpCompositeInsert %_arr_v4float_uint_8_0 %27 %undef 0
%28 = OpCompositeExtract %v4float %26 1
%i1 = OpCompositeInsert %_arr_v4float_uint_8_0 %28 %i0 1
%29 = OpCompositeExtract %v4float %26 2
%i2 = OpCompositeInsert %_arr_v4float_uint_8_0 %29 %i1 2
%30 = OpCompositeExtract %v4float %26 3
%i3 = OpCompositeInsert %_arr_v4float_uint_8_0 %30 %i2 3
%31 = OpCompositeExtract %v4float %26 4
%i4 = OpCompositeInsert %_arr_v4float_uint_8_0 %31 %i3 4
%32 = OpCompositeExtract %v4float %26 5
%i5 = OpCompositeInsert %_arr_v4float_uint_8_0 %32 %i4 5
%33 = OpCompositeExtract %v4float %26 6
%i6 = OpCompositeInsert %_arr_v4float_uint_8_0 %33 %i5 6
%34 = OpCompositeExtract %v4float %26 7
%i7 = OpCompositeInsert %_arr_v4float_uint_8_0 %34 %i6 7
OpStore %23 %i7
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, IsomorphicTypes1) {
  const std::string before =
      R"(
; CHECK: [[int:%\w+]] = OpTypeInt 32 0
; CHECK: [[s1:%\w+]] = OpTypeStruct [[int]]
; CHECK: [[s2:%\w+]] = OpTypeStruct [[s1]]
; CHECK: [[a1:%\w+]] = OpTypeArray [[s2]]
; CHECK: [[s3:%\w+]] = OpTypeStruct [[a1]]
; CHECK: [[p_s3:%\w+]] = OpTypePointer Uniform [[s3]]
; CHECK: [[global_var:%\w+]] = OpVariable [[p_s3]] Uniform
; CHECK: [[p_a1:%\w+]] = OpTypePointer Uniform [[a1]]
; CHECK: [[p_s2:%\w+]] = OpTypePointer Uniform [[s2]]
; CHECK: [[ac1:%\w+]] = OpAccessChain [[p_a1]] [[global_var]] %uint_0
; CHECK: [[ac2:%\w+]] = OpAccessChain [[p_s2]] [[ac1]] %uint_0
; CHECK: [[ld:%\w+]] = OpLoad [[s2]] [[ac2]]
; CHECK: [[ex:%\w+]] = OpCompositeExtract [[s1]] [[ld]]
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "PS_main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource HLSL 600
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 101
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
  %s1 = OpTypeStruct %uint
  %s2 = OpTypeStruct %s1
%a1 = OpTypeArray %s2 %uint_1
  %s3 = OpTypeStruct %a1
 %s1_1 = OpTypeStruct %uint
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
 %s1_0 = OpTypeStruct %uint
 %s2_0 = OpTypeStruct %s1_0
%a1_0 = OpTypeArray %s2_0 %uint_1
 %s3_0 = OpTypeStruct %a1_0
%p_s3 = OpTypePointer Uniform %s3
%p_s3_0 = OpTypePointer Function %s3_0
          %3 = OpVariable %p_s3 Uniform
%p_a1_0 = OpTypePointer Function %a1_0
%p_s2_0 = OpTypePointer Function %s2_0
          %2 = OpFunction %void None %13
         %20 = OpLabel
         %21 = OpVariable %p_a1_0 Function
         %22 = OpLoad %s3 %3
         %23 = OpCompositeExtract %a1 %22 0
         %24 = OpCompositeExtract %s2 %23 0
         %25 = OpCompositeExtract %s1 %24 0
         %26 = OpCompositeExtract %uint %25 0
         %27 = OpCompositeConstruct %s1_0 %26
         %32 = OpCompositeConstruct %s2_0 %27
         %28 = OpCompositeConstruct %a1_0 %32
               OpStore %21 %28
         %29 = OpAccessChain %p_s2_0 %21 %uint_0
         %30 = OpLoad %s2 %29
         %31 = OpCompositeExtract %s1 %30 0
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, IsomorphicTypes2) {
  const std::string before =
      R"(
; CHECK: [[int:%\w+]] = OpTypeInt 32 0
; CHECK: [[s1:%\w+]] = OpTypeStruct [[int]]
; CHECK: [[s2:%\w+]] = OpTypeStruct [[s1]]
; CHECK: [[a1:%\w+]] = OpTypeArray [[s2]]
; CHECK: [[s3:%\w+]] = OpTypeStruct [[a1]]
; CHECK: [[p_s3:%\w+]] = OpTypePointer Uniform [[s3]]
; CHECK: [[global_var:%\w+]] = OpVariable [[p_s3]] Uniform
; CHECK: [[p_s2:%\w+]] = OpTypePointer Uniform [[s2]]
; CHECK: [[p_s1:%\w+]] = OpTypePointer Uniform [[s1]]
; CHECK: [[ac1:%\w+]] = OpAccessChain [[p_s2]] [[global_var]] %uint_0 %uint_0
; CHECK: [[ac2:%\w+]] = OpAccessChain [[p_s1]] [[ac1]] %uint_0
; CHECK: [[ld:%\w+]] = OpLoad [[s1]] [[ac2]]
; CHECK: [[ex:%\w+]] = OpCompositeExtract [[int]] [[ld]]
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "PS_main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource HLSL 600
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 101
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
  %_struct_6 = OpTypeStruct %uint
  %_struct_7 = OpTypeStruct %_struct_6
%_arr__struct_7_uint_1 = OpTypeArray %_struct_7 %uint_1
  %_struct_9 = OpTypeStruct %_arr__struct_7_uint_1
 %_struct_10 = OpTypeStruct %uint
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
 %_struct_15 = OpTypeStruct %uint
%_arr__struct_15_uint_1 = OpTypeArray %_struct_15 %uint_1
%_ptr_Uniform__struct_9 = OpTypePointer Uniform %_struct_9
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
          %3 = OpVariable %_ptr_Uniform__struct_9 Uniform
%_ptr_Function__arr__struct_15_uint_1 = OpTypePointer Function %_arr__struct_15_uint_1
          %2 = OpFunction %void None %13
         %20 = OpLabel
         %21 = OpVariable %_ptr_Function__arr__struct_15_uint_1 Function
         %22 = OpLoad %_struct_9 %3
         %23 = OpCompositeExtract %_arr__struct_7_uint_1 %22 0
         %24 = OpCompositeExtract %_struct_7 %23 0
         %25 = OpCompositeExtract %_struct_6 %24 0
         %26 = OpCompositeExtract %uint %25 0
         %27 = OpCompositeConstruct %_struct_15 %26
         %28 = OpCompositeConstruct %_arr__struct_15_uint_1 %27
               OpStore %21 %28
         %29 = OpAccessChain %_ptr_Function__struct_15 %21 %uint_0
         %30 = OpLoad %_struct_15 %29
         %31 = OpCompositeExtract %uint %30 0
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, IsomorphicTypes3) {
  const std::string before =
      R"(
; CHECK: [[int:%\w+]] = OpTypeInt 32 0
; CHECK: [[s1:%\w+]] = OpTypeStruct [[int]]
; CHECK: [[s2:%\w+]] = OpTypeStruct [[s1]]
; CHECK: [[a1:%\w+]] = OpTypeArray [[s2]]
; CHECK: [[s3:%\w+]] = OpTypeStruct [[a1]]
; CHECK: [[s1_1:%\w+]] = OpTypeStruct [[int]]
; CHECK: [[p_s3:%\w+]] = OpTypePointer Uniform [[s3]]
; CHECK: [[p_s1_1:%\w+]] = OpTypePointer Function [[s1_1]]
; CHECK: [[global_var:%\w+]] = OpVariable [[p_s3]] Uniform
; CHECK: [[p_s2:%\w+]] = OpTypePointer Uniform [[s2]]
; CHECK: [[p_s1:%\w+]] = OpTypePointer Uniform [[s1]]
; CHECK: [[var:%\w+]] = OpVariable [[p_s1_1]] Function
; CHECK: [[ac1:%\w+]] = OpAccessChain [[p_s2]] [[global_var]] %uint_0 %uint_0
; CHECK: [[ac2:%\w+]] = OpAccessChain [[p_s1]] [[ac1]] %uint_0
; CHECK: [[ld:%\w+]] = OpLoad [[s1]] [[ac2]]
; CHECK: [[ex:%\w+]] = OpCompositeExtract [[int]] [[ld]]
; CHECK: [[copy:%\w+]] = OpCompositeConstruct [[s1_1]] [[ex]]
; CHECK: OpStore [[var]] [[copy]]
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "PS_main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource HLSL 600
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 101
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
  %_struct_6 = OpTypeStruct %uint
  %_struct_7 = OpTypeStruct %_struct_6
%_arr__struct_7_uint_1 = OpTypeArray %_struct_7 %uint_1
  %_struct_9 = OpTypeStruct %_arr__struct_7_uint_1
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
 %_struct_15 = OpTypeStruct %uint
 %_struct_10 = OpTypeStruct %uint
%_arr__struct_15_uint_1 = OpTypeArray %_struct_15 %uint_1
%_ptr_Uniform__struct_9 = OpTypePointer Uniform %_struct_9
%_ptr_Function__struct_15 = OpTypePointer Function %_struct_15
          %3 = OpVariable %_ptr_Uniform__struct_9 Uniform
%_ptr_Function__arr__struct_15_uint_1 = OpTypePointer Function %_arr__struct_15_uint_1
          %2 = OpFunction %void None %13
         %20 = OpLabel
         %21 = OpVariable %_ptr_Function__arr__struct_15_uint_1 Function
        %var = OpVariable %_ptr_Function__struct_15 Function
         %22 = OpLoad %_struct_9 %3
         %23 = OpCompositeExtract %_arr__struct_7_uint_1 %22 0
         %24 = OpCompositeExtract %_struct_7 %23 0
         %25 = OpCompositeExtract %_struct_6 %24 0
         %26 = OpCompositeExtract %uint %25 0
         %27 = OpCompositeConstruct %_struct_15 %26
         %28 = OpCompositeConstruct %_arr__struct_15_uint_1 %27
               OpStore %21 %28
         %29 = OpAccessChain %_ptr_Function__struct_15 %21 %uint_0
         %30 = OpLoad %_struct_15 %29
               OpStore %var %30
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, BadMergingTwoObjects) {
  // The second element in the |OpCompositeConstruct| is from a different
  // object.
  const std::string text =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpName %type_ConstBuf "type.ConstBuf"
OpMemberName %type_ConstBuf 0 "TexSizeU"
OpMemberName %type_ConstBuf 1 "TexSizeV"
OpName %ConstBuf "ConstBuf"
OpName %main "main"
OpMemberDecorate %type_ConstBuf 0 Offset 0
OpMemberDecorate %type_ConstBuf 1 Offset 8
OpDecorate %type_ConstBuf Block
OpDecorate %ConstBuf DescriptorSet 0
OpDecorate %ConstBuf Binding 2
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%type_ConstBuf = OpTypeStruct %v2float %v2float
%_ptr_Uniform_type_ConstBuf = OpTypePointer Uniform %type_ConstBuf
%void = OpTypeVoid
%9 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%int_0 = OpConstant %uint 0
%uint_2 = OpConstant %uint 2
%_arr_v2float_uint_2 = OpTypeArray %v2float %uint_2
%_ptr_Function__arr_v2float_uint_2 = OpTypePointer Function %_arr_v2float_uint_2
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%ConstBuf = OpVariable %_ptr_Uniform_type_ConstBuf Uniform
%main = OpFunction %void None %9
%24 = OpLabel
%25 = OpVariable %_ptr_Function__arr_v2float_uint_2 Function
%27 = OpAccessChain %_ptr_Uniform_v2float %ConstBuf %int_0
%28 = OpLoad %v2float %27
%29 = OpAccessChain %_ptr_Uniform_v2float %ConstBuf %int_0
%30 = OpLoad %v2float %29
%31 = OpFNegate %v2float %30
%37 = OpCompositeConstruct %_arr_v2float_uint_2 %28 %31
OpStore %25 %37
OpReturn
OpFunctionEnd
)";

  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(CopyPropArrayPassTest, SecondElementNotContained) {
  // The second element in the |OpCompositeConstruct| is not a memory object.
  // Make sure no change happends.
  const std::string text =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpName %type_ConstBuf "type.ConstBuf"
OpMemberName %type_ConstBuf 0 "TexSizeU"
OpMemberName %type_ConstBuf 1 "TexSizeV"
OpName %ConstBuf "ConstBuf"
OpName %main "main"
OpMemberDecorate %type_ConstBuf 0 Offset 0
OpMemberDecorate %type_ConstBuf 1 Offset 8
OpDecorate %type_ConstBuf Block
OpDecorate %ConstBuf DescriptorSet 0
OpDecorate %ConstBuf Binding 2
OpDecorate %ConstBuf2 DescriptorSet 1
OpDecorate %ConstBuf2 Binding 2
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%type_ConstBuf = OpTypeStruct %v2float %v2float
%_ptr_Uniform_type_ConstBuf = OpTypePointer Uniform %type_ConstBuf
%void = OpTypeVoid
%9 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%int_0 = OpConstant %uint 0
%int_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%_arr_v2float_uint_2 = OpTypeArray %v2float %uint_2
%_ptr_Function__arr_v2float_uint_2 = OpTypePointer Function %_arr_v2float_uint_2
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%ConstBuf = OpVariable %_ptr_Uniform_type_ConstBuf Uniform
%ConstBuf2 = OpVariable %_ptr_Uniform_type_ConstBuf Uniform
%main = OpFunction %void None %9
%24 = OpLabel
%25 = OpVariable %_ptr_Function__arr_v2float_uint_2 Function
%27 = OpAccessChain %_ptr_Uniform_v2float %ConstBuf %int_0
%28 = OpLoad %v2float %27
%29 = OpAccessChain %_ptr_Uniform_v2float %ConstBuf2 %int_1
%30 = OpLoad %v2float %29
%37 = OpCompositeConstruct %_arr_v2float_uint_2 %28 %30
OpStore %25 %37
OpReturn
OpFunctionEnd
)";

  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}
// This test will place a load before the store.  We cannot propagate in this
// case.
TEST_F(CopyPropArrayPassTest, LoadBeforeStore) {
  const std::string text =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%38 = OpAccessChain %_ptr_Function_v4float %23 %24
%39 = OpLoad %v4float %36
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 1
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %23 %35
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

// This test will place a load where it is not dominated by the store.  We
// cannot propagate in this case.
TEST_F(CopyPropArrayPassTest, LoadNotDominated) {
  const std::string text =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%bool = OpTypeBool
%true = OpConstantTrue %bool
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
OpSelectionMerge %merge None
OpBranchConditional %true %if %else
%if = OpLabel
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 1
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %23 %35
%38 = OpAccessChain %_ptr_Function_v4float %23 %24
%39 = OpLoad %v4float %36
OpBranch %merge
%else = OpLabel
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpBranch %merge
%merge = OpLabel
%phi = OpPhi %out_var_SV_Target %39 %if %37 %else
OpStore %out_var_SV_Target %phi
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

// This test has a partial store to the variable.  We cannot propagate in this
// case.
TEST_F(CopyPropArrayPassTest, PartialStore) {
  const std::string text =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%f0 = OpConstant %float 0
%v4const = OpConstantComposite %v4float %f0 %f0 %f0 %f0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 1
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %23 %35
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
      OpStore %36 %v4const
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

// This test does not have a proper copy of an object.  We cannot propagate in
// this case.
TEST_F(CopyPropArrayPassTest, NotACopy) {
  const std::string text =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%f0 = OpConstant %float 0
%v4const = OpConstantComposite %v4float %f0 %f0 %f0 %f0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 0
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %23 %35
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(CopyPropArrayPassTest, BadCopyViaInserts1) {
  const std::string text =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%undef = OpUndef %_arr_v4float_uint_8_0
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%i0 = OpCompositeInsert %_arr_v4float_uint_8_0 %27 %undef 0
%28 = OpCompositeExtract %v4float %26 1
%i1 = OpCompositeInsert %_arr_v4float_uint_8_0 %28 %i0 1
%29 = OpCompositeExtract %v4float %26 2
%i2 = OpCompositeInsert %_arr_v4float_uint_8_0 %29 %i1 3
%30 = OpCompositeExtract %v4float %26 3
%i3 = OpCompositeInsert %_arr_v4float_uint_8_0 %30 %i2 3
%31 = OpCompositeExtract %v4float %26 4
%i4 = OpCompositeInsert %_arr_v4float_uint_8_0 %31 %i3 4
%32 = OpCompositeExtract %v4float %26 5
%i5 = OpCompositeInsert %_arr_v4float_uint_8_0 %32 %i4 5
%33 = OpCompositeExtract %v4float %26 6
%i6 = OpCompositeInsert %_arr_v4float_uint_8_0 %33 %i5 6
%34 = OpCompositeExtract %v4float %26 7
%i7 = OpCompositeInsert %_arr_v4float_uint_8_0 %34 %i6 7
OpStore %23 %i7
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(CopyPropArrayPassTest, BadCopyViaInserts2) {
  const std::string text =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%undef = OpUndef %_arr_v4float_uint_8_0
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%i0 = OpCompositeInsert %_arr_v4float_uint_8_0 %27 %undef 0
%28 = OpCompositeExtract %v4float %26 1
%i1 = OpCompositeInsert %_arr_v4float_uint_8_0 %28 %i0 1
%29 = OpCompositeExtract %v4float %26 3
%i2 = OpCompositeInsert %_arr_v4float_uint_8_0 %29 %i1 2
%30 = OpCompositeExtract %v4float %26 3
%i3 = OpCompositeInsert %_arr_v4float_uint_8_0 %30 %i2 3
%31 = OpCompositeExtract %v4float %26 4
%i4 = OpCompositeInsert %_arr_v4float_uint_8_0 %31 %i3 4
%32 = OpCompositeExtract %v4float %26 5
%i5 = OpCompositeInsert %_arr_v4float_uint_8_0 %32 %i4 5
%33 = OpCompositeExtract %v4float %26 6
%i6 = OpCompositeInsert %_arr_v4float_uint_8_0 %33 %i5 6
%34 = OpCompositeExtract %v4float %26 7
%i7 = OpCompositeInsert %_arr_v4float_uint_8_0 %34 %i6 7
OpStore %23 %i7
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(CopyPropArrayPassTest, BadCopyViaInserts3) {
  const std::string text =
      R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%undef = OpUndef %_arr_v4float_uint_8_0
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%28 = OpCompositeExtract %v4float %26 1
%i1 = OpCompositeInsert %_arr_v4float_uint_8_0 %28 %undef 1
%29 = OpCompositeExtract %v4float %26 2
%i2 = OpCompositeInsert %_arr_v4float_uint_8_0 %29 %i1 2
%30 = OpCompositeExtract %v4float %26 3
%i3 = OpCompositeInsert %_arr_v4float_uint_8_0 %30 %i2 3
%31 = OpCompositeExtract %v4float %26 4
%i4 = OpCompositeInsert %_arr_v4float_uint_8_0 %31 %i3 4
%32 = OpCompositeExtract %v4float %26 5
%i5 = OpCompositeInsert %_arr_v4float_uint_8_0 %32 %i4 5
%33 = OpCompositeExtract %v4float %26 6
%i6 = OpCompositeInsert %_arr_v4float_uint_8_0 %33 %i5 6
%34 = OpCompositeExtract %v4float %26 7
%i7 = OpCompositeInsert %_arr_v4float_uint_8_0 %34 %i6 7
OpStore %23 %i7
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  auto result = SinglePassRunAndDisassemble<CopyPropagateArrays>(
      text, /* skip_nop = */ true, /* do_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(CopyPropArrayPassTest, AtomicAdd) {
  const std::string before = R"(OpCapability SampledBuffer
OpCapability StorageImageExtendedFormats
OpCapability ImageBuffer
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %2 "min" %gl_GlobalInvocationID
OpExecutionMode %2 LocalSize 64 1 1
OpSource HLSL 600
OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
OpDecorate %4 DescriptorSet 4
OpDecorate %4 Binding 70
%uint = OpTypeInt 32 0
%6 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
%_ptr_UniformConstant_6 = OpTypePointer UniformConstant %6
%_ptr_Function_6 = OpTypePointer Function %6
%void = OpTypeVoid
%10 = OpTypeFunction %void
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Image_uint = OpTypePointer Image %uint
%4 = OpVariable %_ptr_UniformConstant_6 UniformConstant
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%2 = OpFunction %void None %10
%17 = OpLabel
%16 = OpVariable %_ptr_Function_6 Function
%18 = OpLoad %6 %4
OpStore %16 %18
%19 = OpImageTexelPointer %_ptr_Image_uint %16 %uint_0 %uint_0
%20 = OpAtomicIAdd %uint %19 %uint_1 %uint_0 %uint_1
OpReturn
OpFunctionEnd
)";

  const std::string after = R"(OpCapability SampledBuffer
OpCapability StorageImageExtendedFormats
OpCapability ImageBuffer
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %2 "min" %gl_GlobalInvocationID
OpExecutionMode %2 LocalSize 64 1 1
OpSource HLSL 600
OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
OpDecorate %4 DescriptorSet 4
OpDecorate %4 Binding 70
%uint = OpTypeInt 32 0
%6 = OpTypeImage %uint Buffer 0 0 0 2 R32ui
%_ptr_UniformConstant_6 = OpTypePointer UniformConstant %6
%_ptr_Function_6 = OpTypePointer Function %6
%void = OpTypeVoid
%10 = OpTypeFunction %void
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Image_uint = OpTypePointer Image %uint
%4 = OpVariable %_ptr_UniformConstant_6 UniformConstant
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%2 = OpFunction %void None %10
%17 = OpLabel
%16 = OpVariable %_ptr_Function_6 Function
%18 = OpLoad %6 %4
OpStore %16 %18
%19 = OpImageTexelPointer %_ptr_Image_uint %4 %uint_0 %uint_0
%20 = OpAtomicIAdd %uint %19 %uint_1 %uint_0 %uint_1
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<CopyPropagateArrays>(before, after, true, true);
}

TEST_F(CopyPropArrayPassTest, IndexIsNullConstnat) {
  const std::string text = R"(
; CHECK: [[var:%\w+]] = OpVariable {{%\w+}} Uniform
; CHECK: [[null:%\w+]] = OpConstantNull %uint
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform__arr_uint_uint_1 [[var]] %uint_0 %uint_0
; CHECK: OpAccessChain %_ptr_Uniform_uint [[ac1]] [[null]]
; CHECK-NEXT: OpReturn
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 600
               OpDecorate %myCBuffer DescriptorSet 0
               OpDecorate %myCBuffer Binding 0
               OpDecorate %_arr_v4float_uint_1 ArrayStride 16
               OpMemberDecorate %MyConstantBuffer 0 Offset 0
               OpMemberDecorate %type_myCBuffer 0 Offset 0
               OpDecorate %type_myCBuffer Block
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
%_arr_v4float_uint_1 = OpTypeArray %uint %uint_1
%MyConstantBuffer = OpTypeStruct %_arr_v4float_uint_1
%type_myCBuffer = OpTypeStruct %MyConstantBuffer
%_ptr_Uniform_type_myCBuffer = OpTypePointer Uniform %type_myCBuffer
%_arr_v4float_uint_1_0 = OpTypeArray %uint %uint_1
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %uint
%_ptr_Uniform_MyConstantBuffer = OpTypePointer Uniform %MyConstantBuffer
  %myCBuffer = OpVariable %_ptr_Uniform_type_myCBuffer Uniform
%_ptr_Function__arr_v4float_uint_1_0 = OpTypePointer Function %_arr_v4float_uint_1_0
         %23 = OpConstantNull %uint
       %main = OpFunction %void None %19
         %24 = OpLabel
         %25 = OpVariable %_ptr_Function__arr_v4float_uint_1_0 Function
         %26 = OpAccessChain %_ptr_Uniform_MyConstantBuffer %myCBuffer %int_0
         %27 = OpLoad %MyConstantBuffer %26
         %28 = OpCompositeExtract %_arr_v4float_uint_1 %27 0
         %29 = OpCompositeExtract %uint %28 0
         %30 = OpCompositeConstruct %_arr_v4float_uint_1_0 %29
               OpStore %25 %30
         %31 = OpAccessChain %_ptr_Function_v4float %25 %23
               OpReturn
               OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<CopyPropagateArrays>(text, true);
}

TEST_F(CopyPropArrayPassTest, DebugDeclare) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
%file_name = OpString "test"
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%uint_32 = OpConstant %uint 32
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
%null_expr = OpExtInst %void %ext DebugExpression
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_tf
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main

; CHECK: [[deref:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugOperation Deref
; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal

; CHECK: [[deref_expr:%\w+]] = OpExtInst %void [[ext]] DebugExpression [[deref]]
; CHECK: OpAccessChain
; CHECK: [[newptr:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[newptr]] [[deref_expr]]
; CHECK: [[element_ptr:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[newptr]] %24
; CHECK: [[load:%\w+]] = OpLoad %v4float [[element_ptr]]
; CHECK: OpStore %out_var_SV_Target [[load]]
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 1
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %23 %35
%decl = OpExtInst %void %ext DebugDeclare %dbg_f %23 %null_expr
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, DebugValue) {
  const std::string before =
      R"(OpCapability Shader
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_INDEX %out_var_SV_Target
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 600
%file_name = OpString "test"
%float_name = OpString "float"
%main_name = OpString "main"
%f_name = OpString "f"
OpName %type_MyCBuffer "type.MyCBuffer"
OpMemberName %type_MyCBuffer 0 "Data"
OpName %MyCBuffer "MyCBuffer"
OpName %main "main"
OpName %in_var_INDEX "in.var.INDEX"
OpName %out_var_SV_Target "out.var.SV_Target"
OpDecorate %_arr_v4float_uint_8 ArrayStride 16
OpMemberDecorate %type_MyCBuffer 0 Offset 0
OpDecorate %type_MyCBuffer Block
OpDecorate %in_var_INDEX Flat
OpDecorate %in_var_INDEX Location 0
OpDecorate %out_var_SV_Target Location 0
OpDecorate %MyCBuffer DescriptorSet 0
OpDecorate %MyCBuffer Binding 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%uint = OpTypeInt 32 0
%uint_8 = OpConstant %uint 8
%uint_32 = OpConstant %uint 32
%_arr_v4float_uint_8 = OpTypeArray %v4float %uint_8
%type_MyCBuffer = OpTypeStruct %_arr_v4float_uint_8
%_ptr_Uniform_type_MyCBuffer = OpTypePointer Uniform %type_MyCBuffer
%void = OpTypeVoid
%13 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_arr_v4float_uint_8_0 = OpTypeArray %v4float %uint_8
%_ptr_Function__arr_v4float_uint_8_0 = OpTypePointer Function %_arr_v4float_uint_8_0
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_v4float_uint_8 = OpTypePointer Uniform %_arr_v4float_uint_8
%_ptr_Function_v4float = OpTypePointer Function %v4float
%MyCBuffer = OpVariable %_ptr_Uniform_type_MyCBuffer Uniform
%in_var_INDEX = OpVariable %_ptr_Input_int Input
%out_var_SV_Target = OpVariable %_ptr_Output_v4float Output

; CHECK: [[deref:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugOperation Deref
; CHECK: [[deref_expr:%\w+]] = OpExtInst %void [[ext]] DebugExpression [[deref]]
%deref = OpExtInst %void %ext DebugOperation Deref
%expr = OpExtInst %void %ext DebugExpression %deref
%src = OpExtInst %void %ext DebugSource %file_name
%cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
%main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_tf
%dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main

; CHECK: [[dbg_f:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable
%dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%main = OpFunction %void None %13
%22 = OpLabel
%23 = OpVariable %_ptr_Function__arr_v4float_uint_8_0 Function
%24 = OpLoad %int %in_var_INDEX
%25 = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
%26 = OpLoad %_arr_v4float_uint_8 %25
%27 = OpCompositeExtract %v4float %26 0
%28 = OpCompositeExtract %v4float %26 1
%29 = OpCompositeExtract %v4float %26 2
%30 = OpCompositeExtract %v4float %26 3
%31 = OpCompositeExtract %v4float %26 4
%32 = OpCompositeExtract %v4float %26 5
%33 = OpCompositeExtract %v4float %26 6
%34 = OpCompositeExtract %v4float %26 7
%35 = OpCompositeConstruct %_arr_v4float_uint_8_0 %27 %28 %29 %30 %31 %32 %33 %34
OpStore %23 %35

; CHECK: OpAccessChain
; CHECK: [[newptr:%\w+]] = OpAccessChain %_ptr_Uniform__arr_v4float_uint_8 %MyCBuffer %int_0
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_f]] [[newptr]] [[deref_expr]]
; CHECK: [[element_ptr:%\w+]] = OpAccessChain %_ptr_Uniform_v4float [[newptr]] %24
; CHECK: [[load:%\w+]] = OpLoad %v4float [[element_ptr]]
; CHECK: OpStore %out_var_SV_Target [[load]]
%decl = OpExtInst %void %ext DebugValue %dbg_f %23 %expr
%36 = OpAccessChain %_ptr_Function_v4float %23 %24
%37 = OpLoad %v4float %36
OpStore %out_var_SV_Target %37
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, FunctionDeclaration) {
  // Make sure the pass works with a function declaration that is called.
  const std::string text = R"(OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpCapability Int8
%1 = OpExtInstImport "OpenCL.std"
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %2 "_Z23julia__1166_kernel_77094Bool"
OpExecutionMode %2 ContractionOff
OpSource Unknown 0
OpDecorate %3 LinkageAttributes "julia_error_7712" Import
%void = OpTypeVoid
%5 = OpTypeFunction %void
%3 = OpFunction %void None %5
OpFunctionEnd
%2 = OpFunction %void None %5
%6 = OpLabel
%7 = OpFunctionCall %void %3
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<CopyPropagateArrays>(text, text, false);
}

// Since Spir-V 1.4, resources that are used by a shader must be on the
// OpEntryPoint instruction with the inputs and outputs. This test ensures that
// this does not stop the pass from working.
TEST_F(CopyPropArrayPassTest, EntryPointUser) {
  const std::string before = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main" %g_rwTexture3d
OpExecutionMode %main LocalSize 256 1 1
OpSource HLSL 660
OpName %type_3d_image "type.3d.image"
OpName %g_rwTexture3d "g_rwTexture3d"
OpName %main "main"
OpDecorate %g_rwTexture3d DescriptorSet 0
OpDecorate %g_rwTexture3d Binding 0
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%10 = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
%type_3d_image = OpTypeImage %uint 3D 2 0 0 2 R32ui
%_ptr_UniformConstant_type_3d_image = OpTypePointer UniformConstant %type_3d_image
%void = OpTypeVoid
%13 = OpTypeFunction %void
%_ptr_Function_type_3d_image = OpTypePointer Function %type_3d_image
%_ptr_Image_uint = OpTypePointer Image %uint
%g_rwTexture3d = OpVariable %_ptr_UniformConstant_type_3d_image UniformConstant
%main = OpFunction %void None %13
%16 = OpLabel
%17 = OpVariable %_ptr_Function_type_3d_image Function
%18 = OpLoad %type_3d_image %g_rwTexture3d
OpStore %17 %18
; CHECK: %19 = OpImageTexelPointer %_ptr_Image_uint %g_rwTexture3d %10 %uint_0
%19 = OpImageTexelPointer %_ptr_Image_uint %17 %10 %uint_0
%20 = OpAtomicIAdd %uint %19 %uint_1 %uint_0 %uint_1
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

// As per SPIRV spec, struct cannot be indexed with non-constant indices
// through OpAccessChain, only arrays.
// The copy-propagate-array pass tries to remove superfluous copies when the
// original array could be indexed instead of the copy.
//
// This test verifies we handle this case:
//  struct SRC { int field1; ...; int fieldN }
//  int tmp_arr[N] = { SRC.field1, ..., SRC.fieldN }
//  return tmp_arr[index];
//
// In such case, we cannot optimize the access: this array was added to allow
// dynamic indexing in the struct.
TEST_F(CopyPropArrayPassTest, StructIndexCannotBecomeDynamic) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %1 "main"
OpDecorate %2 DescriptorSet 0
OpDecorate %2 Binding 0
OpMemberDecorate %_struct_3 0 Offset 0
OpDecorate %_struct_3 Block
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_struct_3 = OpTypeStruct %v4float
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
%uint = OpTypeInt 32 0
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_uint = OpTypePointer Function %uint
%13 = OpTypeFunction %v4float %_ptr_Function_uint
%uint_1 = OpConstant %uint 1
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
%_ptr_Function__arr_v4float_uint_1 = OpTypePointer Function %_arr_v4float_uint_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%2 = OpVariable %_ptr_Uniform__struct_3 Uniform
%19 = OpUndef %v4float
%1 = OpFunction %void None %11
%20 = OpLabel
OpReturn
OpFunctionEnd
%21 = OpFunction %v4float None %13
%22 = OpFunctionParameter %_ptr_Function_uint
%23 = OpLabel
%24 = OpVariable %_ptr_Function__arr_v4float_uint_1 Function
%25 = OpAccessChain %_ptr_Uniform_v4float %2 %int_0
%26 = OpLoad %v4float %25
%27 = OpCompositeConstruct %_arr_v4float_uint_1 %26
OpStore %24 %27
%28 = OpLoad %uint %22
%29 = OpAccessChain %_ptr_Function_v4float %24 %28
OpReturnValue %19
OpFunctionEnd
)";

  SinglePassRunAndCheck<CopyPropagateArrays>(text, text, false);
}

// If the size of an array used in an OpCompositeInsert is not known at compile
// time, then we should not propagate the array, because we do not have a single
// array that represents the final value.
TEST_F(CopyPropArrayPassTest, SpecConstSizedArray) {
  const std::string text = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %2 "main"
OpExecutionMode %2 OriginUpperLeft
%void = OpTypeVoid
%4 = OpTypeFunction %void
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%7 = OpSpecConstant %uint 32
%_arr_int_7 = OpTypeArray %int %7
%int_63 = OpConstant %int 63
%uint_0 = OpConstant %uint 0
%bool = OpTypeBool
%int_0 = OpConstant %int 0
%int_587202566 = OpConstant %int 587202566
%false = OpConstantFalse %bool
%_ptr_Function__arr_int_7 = OpTypePointer Function %_arr_int_7
%16 = OpUndef %_arr_int_7
%2 = OpFunction %void None %4
%17 = OpLabel
%18 = OpVariable %_ptr_Function__arr_int_7 Function
%19 = OpCompositeInsert %_arr_int_7 %int_0 %16 0
OpStore %18 %19
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<CopyPropagateArrays>(text, text, false);
}

TEST_F(CopyPropArrayPassTest, InterpolateFunctions) {
  const std::string before = R"(OpCapability InterpolationFunction
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 680
OpName %in_var_COLOR "in.var.COLOR"
OpName %main "main"
OpName %offset "offset"
OpDecorate %in_var_COLOR Location 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%void = OpTypeVoid
%19 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %19
%20 = OpLabel
%45 = OpVariable %_ptr_Function_v4float Function
%25 = OpLoad %v4float %in_var_COLOR
OpStore %45 %25
; CHECK: OpExtInst %v4float %1 InterpolateAtCentroid %in_var_COLOR
%52 = OpExtInst %v4float %1 InterpolateAtCentroid %45
; CHECK: OpExtInst %v4float %1 InterpolateAtSample %in_var_COLOR %int_0
%54 = OpExtInst %v4float %1 InterpolateAtSample %45 %int_0
%offset = OpCompositeConstruct %v2float %float_0 %float_0
; CHECK: OpExtInst %v4float %1 InterpolateAtOffset %in_var_COLOR %offset
%56 = OpExtInst %v4float %1 InterpolateAtOffset %45 %offset
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, InterpolateMultiPropagation) {
  const std::string before = R"(OpCapability InterpolationFunction
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 680
OpName %in_var_COLOR "in.var.COLOR"
OpName %main "main"
OpName %param_var_color "param.var.color"
OpDecorate %in_var_COLOR Location 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%void = OpTypeVoid
%19 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %19
%20 = OpLabel
%45 = OpVariable %_ptr_Function_v4float Function
%param_var_color = OpVariable %_ptr_Function_v4float Function
%25 = OpLoad %v4float %in_var_COLOR
OpStore %param_var_color %25
; CHECK: OpExtInst %v4float %1 InterpolateAtCentroid %in_var_COLOR
%52 = OpExtInst %v4float %1 InterpolateAtCentroid %param_var_color
%49 = OpLoad %v4float %param_var_color
OpStore %45 %49
; CHECK: OpExtInst %v4float %1 InterpolateAtCentroid %in_var_COLOR
%54 = OpExtInst %v4float %1 InterpolateAtCentroid %45
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, PropagateScalar) {
  const std::string before = R"(OpCapability InterpolationFunction
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_SV_InstanceID
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 680
OpName %in_var_SV_InstanceID "in.var.SV_InstanceID"
OpName %main "main"
OpDecorate %in_var_SV_InstanceID Location 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_float = OpTypePointer Input %float
%void = OpTypeVoid
%19 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_SV_InstanceID = OpVariable %_ptr_Input_float Input
%main = OpFunction %void None %19
%20 = OpLabel
%45 = OpVariable %_ptr_Function_float Function
%25 = OpLoad %v4float %in_var_SV_InstanceID
OpStore %45 %25
; CHECK: OpExtInst %v4float %1 InterpolateAtCentroid %in_var_SV_InstanceID
%52 = OpExtInst %v4float %1 InterpolateAtCentroid %45
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, false);
}

TEST_F(CopyPropArrayPassTest, StoreToAccessChain) {
  const std::string before = R"(OpCapability InterpolationFunction
OpCapability MeshShadingEXT
OpExtension "SPV_EXT_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshEXT %1 "main" %2 %3
OpExecutionMode %1 LocalSize 128 1 1
OpExecutionMode %1 OutputTrianglesEXT
OpExecutionMode %1 OutputVertices 64
OpExecutionMode %1 OutputPrimitivesEXT 126
OpDecorate %3 Flat
OpDecorate %3 Location 2
%uint = OpTypeInt 32 0
%uint_4 = OpConstant %uint 4
%uint_32 = OpConstant %uint 32
%_arr_uint_uint_32 = OpTypeArray %uint %uint_32
%_struct_8 = OpTypeStruct %_arr_uint_uint_32
%_ptr_TaskPayloadWorkgroupEXT__struct_8 = OpTypePointer TaskPayloadWorkgroupEXT %_struct_8
%uint_64 = OpConstant %uint 64
%_arr_uint_uint_64 = OpTypeArray %uint %uint_64
%_ptr_Output__arr_uint_uint_64 = OpTypePointer Output %_arr_uint_uint_64
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function__arr_uint_uint_32 = OpTypePointer Function %_arr_uint_uint_32
%_ptr_Output_uint = OpTypePointer Output %uint
%2 = OpVariable %_ptr_TaskPayloadWorkgroupEXT__struct_8 TaskPayloadWorkgroupEXT
%3 = OpVariable %_ptr_Output__arr_uint_uint_64 Output
%1 = OpFunction %void None %14
%18 = OpLabel
%19 = OpVariable %_ptr_Function__arr_uint_uint_32 Function
%20 = OpLoad %_struct_8 %2
%21 = OpCompositeExtract %_arr_uint_uint_32 %20 0
; CHECK: %28 = OpAccessChain %_ptr_TaskPayloadWorkgroupEXT__arr_uint_uint_32 %2 %uint_0
OpStore %19 %21
; CHECK: %22 = OpAccessChain %_ptr_TaskPayloadWorkgroupEXT_uint %28 %uint_4
%22 = OpAccessChain %_ptr_Function_uint %19 %uint_4
%23 = OpLoad %uint %22
%24 = OpAccessChain %_ptr_Output_uint %3 %uint_4
OpStore %24 %23
OpReturn
OpFunctionEnd
)";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SinglePassRunAndMatch<CopyPropagateArrays>(before, true);
}
}  // namespace
}  // namespace opt
}  // namespace spvtools
