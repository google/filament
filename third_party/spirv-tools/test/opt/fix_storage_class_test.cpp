// Copyright (c) 2019 Google LLC
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
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using FixStorageClassTest = PassTest<::testing::Test>;

TEST_F(FixStorageClassTest, FixAccessChain) {
  const std::string text = R"(
; CHECK: OpAccessChain %_ptr_Workgroup_float
; CHECK: OpAccessChain %_ptr_Uniform_float
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "testMain" %gl_GlobalInvocationID %gl_LocalInvocationID %gl_WorkGroupID
               OpExecutionMode %1 LocalSize 8 8 1
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 0
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%ptr = OpTypePointer Function %_arr_float_uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
  %_struct_5 = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Workgroup__struct_5 = OpTypePointer Workgroup %_struct_5
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %30 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
          %6 = OpVariable %_ptr_Workgroup__struct_5 Workgroup
          %8 = OpVariable %_ptr_Uniform__struct_7 Uniform
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
          %1 = OpFunction %void None %30
         %38 = OpLabel
         %44 = OpLoad %v3uint %gl_LocalInvocationID
         %50 = OpAccessChain %_ptr_Function_float %6 %int_0 %int_0 %int_0
         %51 = OpLoad %float %50
         %52 = OpFMul %float %float_2 %51
               OpStore %50 %52
         %55 = OpLoad %float %50
         %59 = OpCompositeExtract %uint %44 0
         %60 = OpAccessChain %_ptr_Uniform_float %8 %int_0 %59
               OpStore %60 %55
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixStorageClassTest, FixLinkedAccessChain) {
  const std::string text = R"(
; CHECK: OpAccessChain %_ptr_Workgroup__arr_float_uint_10
; CHECK: OpAccessChain %_ptr_Workgroup_float
; CHECK: OpAccessChain %_ptr_Uniform_float
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "testMain" %gl_GlobalInvocationID %gl_LocalInvocationID %gl_WorkGroupID
               OpExecutionMode %1 LocalSize 8 8 1
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_ptr_Function__arr_float_uint_10 = OpTypePointer Function %_arr_float_uint_10
%_ptr = OpTypePointer Function %_arr_float_uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
 %_struct_17 = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Workgroup__struct_17 = OpTypePointer Workgroup %_struct_17
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
         %27 = OpVariable %_ptr_Workgroup__struct_17 Workgroup
          %5 = OpVariable %_ptr_Uniform__struct_7 Uniform
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
          %1 = OpFunction %void None %23
         %28 = OpLabel
         %29 = OpLoad %v3uint %gl_LocalInvocationID
         %30 = OpAccessChain %_ptr_Function__arr_float_uint_10 %27 %int_0 %int_0
         %31 = OpAccessChain %_ptr_Function_float %30 %int_0
         %32 = OpLoad %float %31
         %33 = OpFMul %float %float_2 %32
               OpStore %31 %33
         %34 = OpLoad %float %31
         %35 = OpCompositeExtract %uint %29 0
         %36 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %35
               OpStore %36 %34
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixStorageClassTest, FixCopyObject) {
  const std::string text = R"(
; CHECK: OpCopyObject %_ptr_Workgroup__struct_17
; CHECK: OpAccessChain %_ptr_Workgroup_float
; CHECK: OpAccessChain %_ptr_Uniform_float
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "testMain" %gl_GlobalInvocationID %gl_LocalInvocationID %gl_WorkGroupID
               OpExecutionMode %1 LocalSize 8 8 1
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 0
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%ptr = OpTypePointer Function %_arr_float_uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
  %_struct_17 = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Workgroup__struct_17 = OpTypePointer Workgroup %_struct_17
%_ptr_Function__struct_17 = OpTypePointer Function %_struct_17
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %30 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
          %6 = OpVariable %_ptr_Workgroup__struct_17 Workgroup
          %8 = OpVariable %_ptr_Uniform__struct_7 Uniform
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
          %1 = OpFunction %void None %30
         %38 = OpLabel
         %44 = OpLoad %v3uint %gl_LocalInvocationID
         %cp = OpCopyObject %_ptr_Function__struct_17 %6
         %50 = OpAccessChain %_ptr_Function_float %cp %int_0 %int_0 %int_0
         %51 = OpLoad %float %50
         %52 = OpFMul %float %float_2 %51
               OpStore %50 %52
         %55 = OpLoad %float %50
         %59 = OpCompositeExtract %uint %44 0
         %60 = OpAccessChain %_ptr_Uniform_float %8 %int_0 %59
               OpStore %60 %55
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixStorageClassTest, FixPhiInSelMerge) {
  const std::string text = R"(
; CHECK: OpPhi %_ptr_Workgroup__struct_19
; CHECK: OpAccessChain %_ptr_Workgroup_float
; CHECK: OpAccessChain %_ptr_Uniform_float
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "testMain" %gl_GlobalInvocationID %gl_LocalInvocationID %gl_WorkGroupID
               OpExecutionMode %1 LocalSize 8 8 1
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_ptr_Function__arr_float_uint_10 = OpTypePointer Function %_arr_float_uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
 %_struct_19 = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Workgroup__struct_19 = OpTypePointer Workgroup %_struct_19
%_ptr_Function__struct_19 = OpTypePointer Function %_struct_19
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
         %28 = OpVariable %_ptr_Workgroup__struct_19 Workgroup
         %29 = OpVariable %_ptr_Workgroup__struct_19 Workgroup
          %5 = OpVariable %_ptr_Uniform__struct_7 Uniform
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
          %1 = OpFunction %void None %25
         %30 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %true %32 %31
         %32 = OpLabel
               OpBranch %31
         %31 = OpLabel
         %33 = OpPhi %_ptr_Function__struct_19 %28 %30 %29 %32
         %34 = OpLoad %v3uint %gl_LocalInvocationID
         %35 = OpAccessChain %_ptr_Function_float %33 %int_0 %int_0 %int_0
         %36 = OpLoad %float %35
         %37 = OpFMul %float %float_2 %36
               OpStore %35 %37
         %38 = OpLoad %float %35
         %39 = OpCompositeExtract %uint %34 0
         %40 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %39
               OpStore %40 %38
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixStorageClassTest, FixPhiInLoop) {
  const std::string text = R"(
; CHECK: OpPhi %_ptr_Workgroup__struct_19
; CHECK: OpAccessChain %_ptr_Workgroup_float
; CHECK: OpAccessChain %_ptr_Uniform_float
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "testMain" %gl_GlobalInvocationID %gl_LocalInvocationID %gl_WorkGroupID
               OpExecutionMode %1 LocalSize 8 8 1
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_ptr_Function__arr_float_uint_10 = OpTypePointer Function %_arr_float_uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
 %_struct_19 = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Workgroup__struct_19 = OpTypePointer Workgroup %_struct_19
%_ptr_Function__struct_19 = OpTypePointer Function %_struct_19
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
         %28 = OpVariable %_ptr_Workgroup__struct_19 Workgroup
         %29 = OpVariable %_ptr_Workgroup__struct_19 Workgroup
          %5 = OpVariable %_ptr_Uniform__struct_7 Uniform
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
          %1 = OpFunction %void None %25
         %30 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %true %32 %31
         %32 = OpLabel
               OpBranch %31
         %31 = OpLabel
         %33 = OpPhi %_ptr_Function__struct_19 %28 %30 %29 %32
         %34 = OpLoad %v3uint %gl_LocalInvocationID
         %35 = OpAccessChain %_ptr_Function_float %33 %int_0 %int_0 %int_0
         %36 = OpLoad %float %35
         %37 = OpFMul %float %float_2 %36
               OpStore %35 %37
         %38 = OpLoad %float %35
         %39 = OpCompositeExtract %uint %34 0
         %40 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %39
               OpStore %40 %38
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixStorageClassTest, DontChangeFunctionCalls) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "testMain"
OpExecutionMode %1 LocalSize 8 8 1
OpDecorate %2 DescriptorSet 0
OpDecorate %2 Binding 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
%_ptr_Uniform_int = OpTypePointer Uniform %int
%void = OpTypeVoid
%8 = OpTypeFunction %void
%9 = OpTypeFunction %_ptr_Uniform_int %_ptr_Function_int
%10 = OpVariable %_ptr_Workgroup_int Workgroup
%2 = OpVariable %_ptr_Uniform_int Uniform
%1 = OpFunction %void None %8
%11 = OpLabel
%12 = OpFunctionCall %_ptr_Uniform_int %13 %10
OpReturn
OpFunctionEnd
%13 = OpFunction %_ptr_Uniform_int None %9
%14 = OpFunctionParameter %_ptr_Function_int
%15 = OpLabel
OpReturnValue %2
OpFunctionEnd
)";

  SinglePassRunAndCheck<FixStorageClass>(text, text, false, false);
}

TEST_F(FixStorageClassTest, FixSelect) {
  const std::string text = R"(
; CHECK: OpSelect %_ptr_Workgroup__struct_19
; CHECK: OpAccessChain %_ptr_Workgroup_float
; CHECK: OpAccessChain %_ptr_Uniform_float
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "testMain" %gl_GlobalInvocationID %gl_LocalInvocationID %gl_WorkGroupID
               OpExecutionMode %1 LocalSize 8 8 1
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_ptr_Function__arr_float_uint_10 = OpTypePointer Function %_arr_float_uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
 %_struct_19 = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Workgroup__struct_19 = OpTypePointer Workgroup %_struct_19
%_ptr_Function__struct_19 = OpTypePointer Function %_struct_19
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
         %28 = OpVariable %_ptr_Workgroup__struct_19 Workgroup
         %29 = OpVariable %_ptr_Workgroup__struct_19 Workgroup
          %5 = OpVariable %_ptr_Uniform__struct_7 Uniform
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
          %1 = OpFunction %void None %25
         %30 = OpLabel
         %33 = OpSelect %_ptr_Function__struct_19 %true %28 %29
         %34 = OpLoad %v3uint %gl_LocalInvocationID
         %35 = OpAccessChain %_ptr_Function_float %33 %int_0 %int_0 %int_0
         %36 = OpLoad %float %35
         %37 = OpFMul %float %float_2 %36
               OpStore %35 %37
         %38 = OpLoad %float %35
         %39 = OpCompositeExtract %uint %34 0
         %40 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %39
               OpStore %40 %38
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixStorageClassTest, BitCast) {
  const std::string text = R"(OpCapability VariablePointersStorageBuffer
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%_ptr_Output_void = OpTypePointer Output %void
%_ptr_Private__ptr_Output_void = OpTypePointer Private %_ptr_Output_void
%6 = OpVariable %_ptr_Private__ptr_Output_void Private
%1 = OpFunction %void Inline %3
%7 = OpLabel
%8 = OpBitcast %_ptr_Output_void %6
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<FixStorageClass>(text, text, false);
}

TEST_F(FixStorageClassTest, FixLinkedAccessChain2) {
  // This case is similar to FixLinkedAccessChain.  The difference is that the
  // first OpAccessChain instruction starts as workgroup storage class.  Only
  // the second one needs to change.
  const std::string text = R"(
; CHECK: OpAccessChain %_ptr_Workgroup__arr_float_uint_10
; CHECK: OpAccessChain %_ptr_Workgroup_float
; CHECK: OpAccessChain %_ptr_Uniform_float
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "testMain" %gl_GlobalInvocationID %gl_LocalInvocationID %gl_WorkGroupID
               OpExecutionMode %1 LocalSize 8 8 1
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
               OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
               OpDecorate %5 DescriptorSet 0
               OpDecorate %5 Binding 0
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_2 = OpConstant %float 2
       %uint = OpTypeInt 32 0
    %uint_10 = OpConstant %uint 10
%_arr_float_uint_10 = OpTypeArray %float %uint_10
%_ptr_Workgroup__arr_float_uint_10 = OpTypePointer Workgroup %_arr_float_uint_10
%_ptr = OpTypePointer Function %_arr_float_uint_10
%_arr__arr_float_uint_10_uint_10 = OpTypeArray %_arr_float_uint_10 %uint_10
 %_struct_17 = OpTypeStruct %_arr__arr_float_uint_10_uint_10
%_ptr_Workgroup__struct_17 = OpTypePointer Workgroup %_struct_17
%_runtimearr_float = OpTypeRuntimeArray %float
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
         %27 = OpVariable %_ptr_Workgroup__struct_17 Workgroup
          %5 = OpVariable %_ptr_Uniform__struct_7 Uniform
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_LocalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_WorkGroupID = OpVariable %_ptr_Input_v3uint Input
          %1 = OpFunction %void None %23
         %28 = OpLabel
         %29 = OpLoad %v3uint %gl_LocalInvocationID
         %30 = OpAccessChain %_ptr_Workgroup__arr_float_uint_10 %27 %int_0 %int_0
         %31 = OpAccessChain %_ptr_Function_float %30 %int_0
         %32 = OpLoad %float %31
         %33 = OpFMul %float %float_2 %32
               OpStore %31 %33
         %34 = OpLoad %float %31
         %35 = OpCompositeExtract %uint %29 0
         %36 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %35
               OpStore %36 %34
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixStorageClassTest, AllowImageFormatMismatch) {
  const std::string text = R"(OpCapability Shader
OpCapability SampledBuffer
OpCapability ImageBuffer
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource HLSL 600
OpName %type_buffer_image "type.buffer.image"
OpName %Buf "Buf"
OpName %main "main"
OpName %src_main "src.main"
OpName %bb_entry "bb.entry"
OpName %type_buffer_image_0 "type.buffer.image"
OpName %b "b"
OpDecorate %Buf DescriptorSet 0
OpDecorate %Buf Binding 0
%float = OpTypeFloat 32
%type_buffer_image = OpTypeImage %float Buffer 2 0 0 2 Rgba16f
%_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
%void = OpTypeVoid
%11 = OpTypeFunction %void
%type_buffer_image_0 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
%_ptr_Function_type_buffer_image_0 = OpTypePointer Function %type_buffer_image_0
%Buf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
%main = OpFunction %void None %11
%13 = OpLabel
%14 = OpFunctionCall %void %src_main
OpReturn
OpFunctionEnd
%src_main = OpFunction %void None %11
%bb_entry = OpLabel
%b = OpVariable %_ptr_Function_type_buffer_image_0 Function
%15 = OpLoad %type_buffer_image %Buf
OpStore %b %15
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<FixStorageClass>(text, text, false, false);
}

using FixTypeTest = PassTest<::testing::Test>;

TEST_F(FixTypeTest, FixAccessChain) {
  const std::string text = R"(
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform_S %A %int_0 %uint_0
; CHECK: [[ac2:%\w+]] = OpAccessChain %_ptr_Uniform_T [[ac1]] %int_0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource HLSL 600
               OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
               OpName %S "S"
               OpMemberName %S 0 "t"
               OpName %T "T"
               OpMemberName %T 0 "a"
               OpName %A "A"
               OpName %type_ACSBuffer_counter "type.ACSBuffer.counter"
               OpMemberName %type_ACSBuffer_counter 0 "counter"
               OpName %counter_var_A "counter.var.A"
               OpName %main "main"
               OpName %S_0 "S"
               OpMemberName %S_0 0 "t"
               OpName %T_0 "T"
               OpMemberName %T_0 0 "a"
               OpDecorate %A DescriptorSet 0
               OpDecorate %A Binding 0
               OpDecorate %counter_var_A DescriptorSet 0
               OpDecorate %counter_var_A Binding 1
               OpMemberDecorate %T 0 Offset 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_S BufferBlock
               OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
               OpDecorate %type_ACSBuffer_counter BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
          %T = OpTypeStruct %int
          %S = OpTypeStruct %T
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
%type_ACSBuffer_counter = OpTypeStruct %int
%_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
        %T_0 = OpTypeStruct %int
        %S_0 = OpTypeStruct %T_0
%_ptr_Function_S_0 = OpTypePointer Function %S_0
%_ptr_Uniform_S = OpTypePointer Uniform %S
%_ptr_Uniform_T = OpTypePointer Uniform %T
         %22 = OpTypeFunction %T_0 %_ptr_Function_S_0
%_ptr_Function_T_0 = OpTypePointer Function %T_0
          %A = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
%counter_var_A = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
       %main = OpFunction %void None %18
         %24 = OpLabel
         %25 = OpVariable %_ptr_Function_T_0 Function
         %26 = OpVariable %_ptr_Function_S_0 Function
         %27 = OpAccessChain %_ptr_Uniform_S %A %int_0 %uint_0
         %28 = OpAccessChain %_ptr_Function_T_0 %27 %int_0
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixTypeTest, FixLoad) {
  const std::string text = R"(
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform_S %A %int_0 %uint_0
; CHECK: [[ac2:%\w+]] = OpAccessChain %_ptr_Uniform_T [[ac1]] %int_0
; CHECK: [[ld:%\w+]] = OpLoad %T [[ac2]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource HLSL 600
               OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
               OpName %S "S"
               OpMemberName %S 0 "t"
               OpName %T "T"
               OpMemberName %T 0 "a"
               OpName %A "A"
               OpName %type_ACSBuffer_counter "type.ACSBuffer.counter"
               OpMemberName %type_ACSBuffer_counter 0 "counter"
               OpName %counter_var_A "counter.var.A"
               OpName %main "main"
               OpName %S_0 "S"
               OpMemberName %S_0 0 "t"
               OpName %T_0 "T"
               OpMemberName %T_0 0 "a"
               OpDecorate %A DescriptorSet 0
               OpDecorate %A Binding 0
               OpDecorate %counter_var_A DescriptorSet 0
               OpDecorate %counter_var_A Binding 1
               OpMemberDecorate %T 0 Offset 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_S BufferBlock
               OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
               OpDecorate %type_ACSBuffer_counter BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
          %T = OpTypeStruct %int
          %S = OpTypeStruct %T
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
%type_ACSBuffer_counter = OpTypeStruct %int
%_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
        %T_0 = OpTypeStruct %int
        %S_0 = OpTypeStruct %T_0
%_ptr_Function_S_0 = OpTypePointer Function %S_0
%_ptr_Uniform_S = OpTypePointer Uniform %S
%_ptr_Uniform_T = OpTypePointer Uniform %T
         %22 = OpTypeFunction %T_0 %_ptr_Function_S_0
%_ptr_Function_T_0 = OpTypePointer Function %T_0
          %A = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
%counter_var_A = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
       %main = OpFunction %void None %18
         %24 = OpLabel
         %25 = OpVariable %_ptr_Function_T_0 Function
         %26 = OpVariable %_ptr_Function_S_0 Function
         %27 = OpAccessChain %_ptr_Uniform_S %A %int_0 %uint_0
         %28 = OpAccessChain %_ptr_Uniform_T %27 %int_0
         %29 = OpLoad %T_0 %28
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixTypeTest, FixStore) {
  const std::string text = R"(
; CHECK: [[ld:%\w+]] = OpLoad %T
; CHECK: OpStore
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource HLSL 600
               OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
               OpName %S "S"
               OpMemberName %S 0 "t"
               OpName %T "T"
               OpMemberName %T 0 "a"
               OpName %A "A"
               OpName %type_ACSBuffer_counter "type.ACSBuffer.counter"
               OpMemberName %type_ACSBuffer_counter 0 "counter"
               OpName %counter_var_A "counter.var.A"
               OpName %main "main"
               OpName %S_0 "S"
               OpMemberName %S_0 0 "t"
               OpName %T_0 "T"
               OpMemberName %T_0 0 "a"
               OpDecorate %A DescriptorSet 0
               OpDecorate %A Binding 0
               OpDecorate %counter_var_A DescriptorSet 0
               OpDecorate %counter_var_A Binding 1
               OpMemberDecorate %T 0 Offset 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_S BufferBlock
               OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
               OpDecorate %type_ACSBuffer_counter BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
          %T = OpTypeStruct %int
          %S = OpTypeStruct %T
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
%type_ACSBuffer_counter = OpTypeStruct %int
%_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
        %T_0 = OpTypeStruct %int
        %S_0 = OpTypeStruct %T_0
%_ptr_Function_S_0 = OpTypePointer Function %S_0
%_ptr_Uniform_S = OpTypePointer Uniform %S
%_ptr_Uniform_T = OpTypePointer Uniform %T
         %22 = OpTypeFunction %T_0 %_ptr_Function_S_0
%_ptr_Function_T_0 = OpTypePointer Function %T_0
          %A = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
%counter_var_A = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
       %main = OpFunction %void None %18
         %24 = OpLabel
         %25 = OpVariable %_ptr_Function_T_0 Function
         %26 = OpVariable %_ptr_Function_S_0 Function
         %27 = OpAccessChain %_ptr_Uniform_S %A %int_0 %uint_0
         %28 = OpAccessChain %_ptr_Uniform_T %27 %int_0
         %29 = OpLoad %T %28
               OpStore %25 %29
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixTypeTest, FixSelect) {
  const std::string text = R"(
; CHECK: OpSelect %_ptr_Uniform__struct_3
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_runtimearr__struct_3 ArrayStride 4
               OpMemberDecorate %_struct_5 0 Offset 0
               OpDecorate %_struct_5 BufferBlock
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
  %_struct_3 = OpTypeStruct %uint
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
  %_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
 %_struct_12 = OpTypeStruct %uint
%_ptr_Function__struct_12 = OpTypePointer Function %_struct_12
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %bool = OpTypeBool
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
          %2 = OpVariable %_ptr_Uniform__struct_5 Uniform
          %1 = OpFunction %void None %11
         %17 = OpLabel
         %18 = OpAccessChain %_ptr_Uniform_uint %2 %uint_0 %uint_0 %uint_0
         %19 = OpLoad %uint %18
         %20 = OpSGreaterThan %bool %19 %uint_0
         %21 = OpAccessChain %_ptr_Uniform__struct_3 %2 %uint_0 %uint_0
         %22 = OpAccessChain %_ptr_Uniform__struct_3 %2 %uint_0 %uint_1
         %23 = OpSelect %_ptr_Function__struct_12 %20 %21 %22
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

TEST_F(FixTypeTest, FixPhiInLoop) {
  const std::string text = R"(
; CHECK: [[ac_init:%\w+]] = OpAccessChain %_ptr_Uniform__struct_3
; CHECK: [[ac_phi:%\w+]] = OpPhi %_ptr_Uniform__struct_3 [[ac_init]] {{%\w+}} [[ac_update:%\w+]] {{%\w+}}
; CHECK: [[ac_update]] = OpPtrAccessChain %_ptr_Uniform__struct_3 [[ac_phi]] %int_1
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpSource HLSL 600
               OpDecorate %2 DescriptorSet 0
               OpDecorate %2 Binding 0
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_runtimearr__struct_3 ArrayStride 4
               OpMemberDecorate %_struct_5 0 Offset 0
               OpDecorate %_struct_5 BufferBlock
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
  %_struct_3 = OpTypeStruct %int
  %_struct_9 = OpTypeStruct %int
%_runtimearr__struct_3 = OpTypeRuntimeArray %_struct_3
  %_struct_5 = OpTypeStruct %_runtimearr__struct_3
%_ptr_Uniform__struct_5 = OpTypePointer Uniform %_struct_5
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
       %bool = OpTypeBool
%_ptr_Uniform__struct_3 = OpTypePointer Uniform %_struct_3
%_ptr_Function__struct_9 = OpTypePointer Function %_struct_9
          %2 = OpVariable %_ptr_Uniform__struct_5 Uniform
          %1 = OpFunction %void None %12
         %16 = OpLabel
         %17 = OpAccessChain %_ptr_Uniform__struct_3 %2 %int_0 %int_0
               OpBranch %18
         %18 = OpLabel
         %20 = OpPhi %_ptr_Function__struct_9 %17 %16 %21 %22
         %23 = OpUndef %bool
               OpLoopMerge %24 %22 None
               OpBranchConditional %23 %22 %24
         %22 = OpLabel
         %21 = OpPtrAccessChain %_ptr_Function__struct_9 %20 %int_1
               OpBranch %18
         %24 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<FixStorageClass>(text, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
