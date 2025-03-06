// Copyright (c) 2021 The Khronos Group Inc.
// Copyright (c) 2021 Valve Corporation
// Copyright (c) 2021 LunarG Inc.
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

#include <vector>

#include "gmock/gmock.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using InterpFixupTest = PassTest<::testing::Test>;

using ::testing::HasSubstr;

TEST_F(InterpFixupTest, FixInterpAtSample) {
  const std::string text = R"(
               OpCapability Shader
               OpCapability InterpolationFunction
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %i_vPositionOs %_entryPointOutput
               OpExecutionMode %MainPs OriginUpperLeft
               OpSource HLSL 500
               OpName %MainPs "MainPs"
               OpName %i_vPositionOs "i.vPositionOs"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %i_vPositionOs Location 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %float_0 = OpConstant %float 0
         %10 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_4 = OpConstant %uint 4
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Input_v4float = OpTypePointer Input %v4float
%i_vPositionOs = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
     %MainPs = OpFunction %void None %6
         %19 = OpLabel
         %20 = OpLoad %v4float %i_vPositionOs
               OpBranch %21
         %21 = OpLabel
         %22 = OpPhi %v4float %10 %19 %23 %24
         %25 = OpPhi %uint %uint_0 %19 %26 %24
         %27 = OpULessThan %bool %25 %uint_4
               OpLoopMerge %28 %24 None
               OpBranchConditional %27 %24 %28
         %24 = OpLabel
         %29 = OpExtInst %v4float %1 InterpolateAtSample %20 %25
;CHECK:  %29 = OpExtInst %v4float %1 InterpolateAtSample %i_vPositionOs %25
         %30 = OpCompositeExtract %float %29 0
         %31 = OpCompositeExtract %float %22 0
         %32 = OpFAdd %float %31 %30
         %23 = OpCompositeInsert %v4float %32 %22 0
         %26 = OpIAdd %uint %25 %int_1
               OpBranch %21
         %28 = OpLabel
               OpStore %_entryPointOutput %22
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<InterpFixupPass>(text, false);
}

TEST_F(InterpFixupTest, FixInterpAtCentroid) {
  const std::string text = R"(
               OpCapability Shader
               OpCapability InterpolationFunction
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %i_vPositionOs %_entryPointOutput
               OpExecutionMode %MainPs OriginUpperLeft
               OpSource HLSL 500
               OpName %MainPs "MainPs"
               OpName %i_vPositionOs "i.vPositionOs"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %i_vPositionOs Location 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %float_0 = OpConstant %float 0
         %10 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%i_vPositionOs = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
     %MainPs = OpFunction %void None %6
         %13 = OpLabel
         %14 = OpLoad %v4float %i_vPositionOs
         %15 = OpExtInst %v4float %1 InterpolateAtCentroid %14
;CHECK:  %15 = OpExtInst %v4float %1 InterpolateAtCentroid %i_vPositionOs
         %16 = OpCompositeExtract %float %15 0
         %17 = OpCompositeInsert %v4float %16 %10 0
               OpStore %_entryPointOutput %17
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<InterpFixupPass>(text, false);
}

TEST_F(InterpFixupTest, FixInterpAtOffset) {
  const std::string text = R"(
               OpCapability Shader
               OpCapability InterpolationFunction
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %i_vPositionOs %_entryPointOutput
               OpExecutionMode %MainPs OriginUpperLeft
               OpSource HLSL 500
               OpName %MainPs "MainPs"
               OpName %i_vPositionOs "i.vPositionOs"
               OpName %_entryPointOutput "@entryPointOutput"
               OpDecorate %i_vPositionOs Location 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %float_0 = OpConstant %float 0
         %10 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
    %v2float = OpTypeVector %float 2
%float_0_0625 = OpConstant %float 0.0625
         %13 = OpConstantComposite %v2float %float_0_0625 %float_0_0625
%_ptr_Input_v4float = OpTypePointer Input %v4float
%i_vPositionOs = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
     %MainPs = OpFunction %void None %6
         %16 = OpLabel
         %17 = OpLoad %v4float %i_vPositionOs
         %18 = OpExtInst %v4float %1 InterpolateAtOffset %17 %13
;CHECK:  %18 = OpExtInst %v4float %1 InterpolateAtOffset %i_vPositionOs %13
         %19 = OpCompositeExtract %float %18 0
         %20 = OpCompositeInsert %v4float %19 %10 0
               OpStore %_entryPointOutput %20
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<InterpFixupPass>(text, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
