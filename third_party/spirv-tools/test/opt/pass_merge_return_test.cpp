// Copyright (c) 2017 Google Inc.
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
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using MergeReturnPassTest = PassTest<::testing::Test>;

TEST_F(MergeReturnPassTest, OneReturn) {
  const std::string before =
      R"(OpCapability Addresses
OpCapability Kernel
OpCapability GenericPointer
OpCapability Linkage
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %1 "simple_kernel"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %2 None %3
%4 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after = before;

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

TEST_F(MergeReturnPassTest, TwoReturnsNoValue) {
  const std::string before =
      R"(OpCapability Addresses
OpCapability Kernel
OpCapability GenericPointer
OpCapability Linkage
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %6 "simple_kernel"
%2 = OpTypeVoid
%3 = OpTypeBool
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpBranchConditional %4 %8 %9
%8 = OpLabel
OpReturn
%9 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Addresses
OpCapability Kernel
OpCapability GenericPointer
OpCapability Linkage
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %6 "simple_kernel"
%2 = OpTypeVoid
%3 = OpTypeBool
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpBranchConditional %4 %8 %9
%8 = OpLabel
OpBranch %10
%9 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

TEST_F(MergeReturnPassTest, TwoReturnsWithValues) {
  const std::string before =
      R"(OpCapability Linkage
OpCapability Kernel
OpMemoryModel Logical OpenCL
%1 = OpTypeInt 32 0
%2 = OpTypeBool
%3 = OpConstantFalse %2
%4 = OpConstant %1 0
%5 = OpConstant %1 1
%6 = OpTypeFunction %1
%7 = OpFunction %1 None %6
%8 = OpLabel
OpBranchConditional %3 %9 %10
%9 = OpLabel
OpReturnValue %4
%10 = OpLabel
OpReturnValue %5
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Linkage
OpCapability Kernel
OpMemoryModel Logical OpenCL
%1 = OpTypeInt 32 0
%2 = OpTypeBool
%3 = OpConstantFalse %2
%4 = OpConstant %1 0
%5 = OpConstant %1 1
%6 = OpTypeFunction %1
%7 = OpFunction %1 None %6
%8 = OpLabel
OpBranchConditional %3 %9 %10
%9 = OpLabel
OpBranch %11
%10 = OpLabel
OpBranch %11
%11 = OpLabel
%12 = OpPhi %1 %4 %9 %5 %10
OpReturnValue %12
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

TEST_F(MergeReturnPassTest, UnreachableReturnsNoValue) {
  const std::string before =
      R"(OpCapability Addresses
OpCapability Kernel
OpCapability GenericPointer
OpCapability Linkage
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %6 "simple_kernel"
%2 = OpTypeVoid
%3 = OpTypeBool
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpReturn
%8 = OpLabel
OpBranchConditional %4 %9 %10
%9 = OpLabel
OpReturn
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Addresses
OpCapability Kernel
OpCapability GenericPointer
OpCapability Linkage
OpMemoryModel Physical32 OpenCL
OpEntryPoint Kernel %6 "simple_kernel"
%2 = OpTypeVoid
%3 = OpTypeBool
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpBranch %11
%8 = OpLabel
OpBranchConditional %4 %9 %10
%9 = OpLabel
OpBranch %11
%10 = OpLabel
OpBranch %11
%11 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

TEST_F(MergeReturnPassTest, UnreachableReturnsWithValues) {
  const std::string before =
      R"(OpCapability Linkage
OpCapability Kernel
OpMemoryModel Logical OpenCL
%1 = OpTypeInt 32 0
%2 = OpTypeBool
%3 = OpConstantFalse %2
%4 = OpConstant %1 0
%5 = OpConstant %1 1
%6 = OpTypeFunction %1
%7 = OpFunction %1 None %6
%8 = OpLabel
%9 = OpIAdd %1 %4 %5
OpReturnValue %9
%10 = OpLabel
OpBranchConditional %3 %11 %12
%11 = OpLabel
OpReturnValue %4
%12 = OpLabel
OpReturnValue %5
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Linkage
OpCapability Kernel
OpMemoryModel Logical OpenCL
%1 = OpTypeInt 32 0
%2 = OpTypeBool
%3 = OpConstantFalse %2
%4 = OpConstant %1 0
%5 = OpConstant %1 1
%6 = OpTypeFunction %1
%7 = OpFunction %1 None %6
%8 = OpLabel
%9 = OpIAdd %1 %4 %5
OpBranch %13
%10 = OpLabel
OpBranchConditional %3 %11 %12
%11 = OpLabel
OpBranch %13
%12 = OpLabel
OpBranch %13
%13 = OpLabel
%14 = OpPhi %1 %9 %8 %4 %11 %5 %12
OpReturnValue %14
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

#ifdef SPIRV_EFFCEE
TEST_F(MergeReturnPassTest, StructuredControlFlowWithUnreachableMerge) {
  const std::string before =
      R"(
; CHECK: [[false:%\w+]] = OpConstantFalse
; CHECK: [[true:%\w+]] = OpConstantTrue
; CHECK: OpFunction
; CHECK: [[var:%\w+]] = OpVariable [[:%\w+]] Function [[false]]
; CHECK: OpSelectionMerge [[merge_lab:%\w+]]
; CHECK: OpBranchConditional [[cond:%\w+]] [[if_lab:%\w+]] [[then_lab:%\w+]]
; CHECK: [[if_lab]] = OpLabel
; CHECK-NEXT: OpStore [[var]] [[true]]
; CHECK-NEXT: OpBranch
; CHECK: [[then_lab]] = OpLabel
; CHECK-NEXT: OpStore [[var]] [[true]]
; CHECK-NEXT: OpBranch [[merge_lab]]
; CHECK: OpReturn
OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %6 "simple_shader"
%2 = OpTypeVoid
%3 = OpTypeBool
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %4 %8 %9
%8 = OpLabel
OpReturn
%9 = OpLabel
OpReturn
%10 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<MergeReturnPass>(before, false);
}

TEST_F(MergeReturnPassTest, StructuredControlFlowAddPhi) {
  const std::string before =
      R"(
; CHECK: [[false:%\w+]] = OpConstantFalse
; CHECK: [[true:%\w+]] = OpConstantTrue
; CHECK: OpFunction
; CHECK: [[var:%\w+]] = OpVariable [[:%\w+]] Function [[false]]
; CHECK: OpSelectionMerge [[merge_lab:%\w+]]
; CHECK: OpBranchConditional [[cond:%\w+]] [[if_lab:%\w+]] [[then_lab:%\w+]]
; CHECK: [[if_lab]] = OpLabel
; CHECK-NEXT: [[add:%\w+]] = OpIAdd [[type:%\w+]]
; CHECK-NEXT: OpBranch
; CHECK: [[then_lab]] = OpLabel
; CHECK-NEXT: OpStore [[var]] [[true]]
; CHECK-NEXT: OpBranch [[merge_lab]]
; CHECK: [[merge_lab]] = OpLabel
; CHECK-NEXT: [[phi:%\w+]] = OpPhi [[type]] [[add]] [[if_lab]] [[undef:%\w+]] [[then_lab]]
; CHECK: OpIAdd [[type]] [[phi]] [[phi]]
; CHECK: OpReturn
OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %6 "simple_shader"
%2 = OpTypeVoid
%3 = OpTypeBool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %4 %8 %9
%8 = OpLabel
%11 = OpIAdd %int %int_0 %int_0
OpBranch %10
%9 = OpLabel
OpReturn
%10 = OpLabel
%12 = OpIAdd %int %11 %11
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<MergeReturnPass>(before, false);
}

TEST_F(MergeReturnPassTest, StructuredControlDecoration) {
  const std::string before =
      R"(
; CHECK: OpDecorate [[dec_id:%\w+]] RelaxedPrecision
; CHECK: [[false:%\w+]] = OpConstantFalse
; CHECK: [[true:%\w+]] = OpConstantTrue
; CHECK: OpFunction
; CHECK: [[var:%\w+]] = OpVariable [[:%\w+]] Function [[false]]
; CHECK: OpSelectionMerge [[merge_lab:%\w+]]
; CHECK: OpBranchConditional [[cond:%\w+]] [[if_lab:%\w+]] [[then_lab:%\w+]]
; CHECK: [[if_lab]] = OpLabel
; CHECK-NEXT: [[dec_id]] = OpIAdd [[type:%\w+]]
; CHECK-NEXT: OpBranch
; CHECK: [[then_lab]] = OpLabel
; CHECK-NEXT: OpStore [[var]] [[true]]
; CHECK-NEXT: OpBranch [[merge_lab]]
; CHECK: [[merge_lab]] = OpLabel
; CHECK: OpReturn
OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %6 "simple_shader"
OpDecorate %11 RelaxedPrecision
%2 = OpTypeVoid
%3 = OpTypeBool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %4 %8 %9
%8 = OpLabel
%11 = OpIAdd %int %int_0 %int_0
OpBranch %10
%9 = OpLabel
OpReturn
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<MergeReturnPass>(before, false);
}

TEST_F(MergeReturnPassTest, StructuredControlDecoration2) {
  const std::string before =
      R"(
; CHECK: OpDecorate [[dec_id:%\w+]] RelaxedPrecision
; CHECK: [[false:%\w+]] = OpConstantFalse
; CHECK: [[true:%\w+]] = OpConstantTrue
; CHECK: OpFunction
; CHECK: [[var:%\w+]] = OpVariable [[:%\w+]] Function [[false]]
; CHECK: OpSelectionMerge [[merge_lab:%\w+]]
; CHECK: OpBranchConditional [[cond:%\w+]] [[if_lab:%\w+]] [[then_lab:%\w+]]
; CHECK: [[if_lab]] = OpLabel
; CHECK-NEXT: [[dec_id]] = OpIAdd [[type:%\w+]]
; CHECK-NEXT: OpBranch
; CHECK: [[then_lab]] = OpLabel
; CHECK-NEXT: OpStore [[var]] [[true]]
; CHECK-NEXT: OpBranch [[merge_lab]]
; CHECK: [[merge_lab]] = OpLabel
; CHECK-NEXT: [[phi:%\w+]] = OpPhi [[type]] [[dec_id]] [[if_lab]] [[undef:%\w+]] [[then_lab]]
; CHECK: OpIAdd [[type]] [[phi]] [[phi]]
; CHECK: OpReturn
OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %6 "simple_shader"
OpDecorate %11 RelaxedPrecision
%2 = OpTypeVoid
%3 = OpTypeBool
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%4 = OpConstantFalse %3
%1 = OpTypeFunction %2
%6 = OpFunction %2 None %1
%7 = OpLabel
OpSelectionMerge %10 None
OpBranchConditional %4 %8 %9
%8 = OpLabel
%11 = OpIAdd %int %int_0 %int_0
OpBranch %10
%9 = OpLabel
OpReturn
%10 = OpLabel
%12 = OpIAdd %int %11 %11
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<MergeReturnPass>(before, false);
}

TEST_F(MergeReturnPassTest, SplitBlockUsedInPhi) {
  const std::string before =
      R"(
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK: OpSelectionMerge [[merge1:%\w+]] None
; CHECK: [[merge1]] = OpLabel
; CHECK: OpBranchConditional %{{\w+}} %{{\w+}} [[old_merge:%\w+]]
; CHECK: [[old_merge]] = OpLabel
; CHECK-NEXT: OpSelectionMerge [[merge2:%\w+]]
; CHECK-NEXT: OpBranchConditional %false [[side_node:%\w+]] [[merge2]]
; CHECK: [[merge2]] = OpLabel
; CHECK-NEXT: OpPhi %bool %false [[old_merge]] %true [[side_node]]
               OpCapability Addresses
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "simple_shader"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
          %6 = OpTypeFunction %void
          %1 = OpFunction %void None %6
          %7 = OpLabel
               OpSelectionMerge %8 None
               OpBranchConditional %false %9 %8
          %9 = OpLabel
               OpReturn
          %8 = OpLabel
               OpSelectionMerge %10 None
               OpBranchConditional %false %11 %10
         %11 = OpLabel
               OpBranch %10
         %10 = OpLabel
         %12 = OpPhi %bool %false %8 %true %11
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<MergeReturnPass>(before, false);
}

// TODO(#1861): Reenable these test when the breaks from selection constructs
// are reenabled.
/*
TEST_F(MergeReturnPassTest, UpdateOrderWhenPredicating) {
  const std::string before =
      R"(
; CHECK: OpFunction
; CHECK: OpFunction
; CHECK: OpSelectionMerge [[m1:%\w+]] None
; CHECK-NOT: OpReturn
; CHECK: [[m1]] = OpLabel
; CHECK: OpSelectionMerge [[m2:%\w+]] None
; CHECK: OpSelectionMerge [[m3:%\w+]] None
; CHECK: OpSelectionMerge [[m4:%\w+]] None
; CHECK: OpLabel
; CHECK-NEXT: OpStore
; CHECK-NEXT: OpBranch [[m4]]
; CHECK: [[m4]] = OpLabel
; CHECK-NEXT: [[ld4:%\w+]] = OpLoad %bool
; CHECK-NEXT: OpBranchConditional [[ld4]] [[m3]]
; CHECK: [[m3]] = OpLabel
; CHECK-NEXT: [[ld3:%\w+]] = OpLoad %bool
; CHECK-NEXT: OpBranchConditional [[ld3]] [[m2]]
; CHECK: [[m2]] = OpLabel
               OpCapability SampledBuffer
               OpCapability StorageImageExtendedFormats
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "PS_DebugTiles"
               OpExecutionMode %1 OriginUpperLeft
               OpSource HLSL 600
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %bool = OpTypeBool
          %1 = OpFunction %void None %3
          %5 = OpLabel
          %6 = OpFunctionCall %void %7
               OpReturn
               OpFunctionEnd
          %7 = OpFunction %void None %3
          %8 = OpLabel
          %9 = OpUndef %bool
               OpSelectionMerge %10 None
               OpBranchConditional %9 %11 %10
         %11 = OpLabel
               OpReturn
         %10 = OpLabel
         %12 = OpUndef %bool
               OpSelectionMerge %13 None
               OpBranchConditional %12 %14 %15
         %15 = OpLabel
         %16 = OpUndef %bool
               OpSelectionMerge %17 None
               OpBranchConditional %16 %18 %17
         %18 = OpLabel
               OpReturn
         %17 = OpLabel
               OpBranch %13
         %14 = OpLabel
               OpReturn
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<MergeReturnPass>(before, false);
}
*/
#endif

TEST_F(MergeReturnPassTest, StructuredControlFlowBothMergeAndHeader) {
  const std::string before =
      R"(OpCapability Addresses
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "simple_shader"
          %2 = OpTypeVoid
          %3 = OpTypeBool
          %4 = OpTypeInt 32 0
          %5 = OpConstant %4 0
          %6 = OpConstantFalse %3
          %7 = OpTypeFunction %2
          %1 = OpFunction %2 None %7
          %8 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %6 %10 %11
         %10 = OpLabel
               OpReturn
         %11 = OpLabel
               OpBranch %9
          %9 = OpLabel
               OpLoopMerge %12 %13 None
               OpBranch %13
         %13 = OpLabel
         %14 = OpIAdd %4 %5 %5
               OpBranchConditional %6 %9 %12
         %12 = OpLabel
         %15 = OpIAdd %4 %14 %14
               OpReturn
               OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "simple_shader"
%void = OpTypeVoid
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%false = OpConstantFalse %bool
%7 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%1 = OpFunction %void None %7
%8 = OpLabel
%18 = OpVariable %_ptr_Function_bool Function %false
OpSelectionMerge %9 None
OpBranchConditional %false %10 %11
%10 = OpLabel
OpStore %18 %true
OpBranch %9
%11 = OpLabel
OpBranch %9
%9 = OpLabel
%23 = OpLoad %bool %18
OpSelectionMerge %22 None
OpBranchConditional %23 %22 %21
%21 = OpLabel
OpBranch %20
%20 = OpLabel
OpLoopMerge %12 %13 None
OpBranch %13
%13 = OpLabel
%14 = OpIAdd %uint %uint_0 %uint_0
OpBranchConditional %false %20 %12
%12 = OpLabel
%15 = OpIAdd %uint %14 %14
OpStore %18 %true
OpBranch %22
%22 = OpLabel
OpBranch %16
%16 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

// TODO(#1861): Reenable these test when the breaks from selection constructs
// are reenabled.
/*
TEST_F(MergeReturnPassTest, NestedSelectionMerge) {
  const std::string before =
      R"(
               OpCapability Addresses
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "simple_shader"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %false = OpConstantFalse %bool
          %7 = OpTypeFunction %void
          %1 = OpFunction %void None %7
          %8 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %false %10 %11
         %10 = OpLabel
               OpReturn
         %11 = OpLabel
               OpSelectionMerge %12 None
               OpBranchConditional %false %13 %14
         %13 = OpLabel
         %15 = OpIAdd %uint %uint_0 %uint_0
               OpBranch %12
         %14 = OpLabel
               OpReturn
         %12 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %16 = OpIAdd %uint %15 %15
               OpReturn
               OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "simple_shader"
%void = OpTypeVoid
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%false = OpConstantFalse %bool
%7 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%26 = OpUndef %uint
%1 = OpFunction %void None %7
%8 = OpLabel
%19 = OpVariable %_ptr_Function_bool Function %false
OpSelectionMerge %9 None
OpBranchConditional %false %10 %11
%10 = OpLabel
OpStore %19 %true
OpBranch %9
%11 = OpLabel
OpSelectionMerge %12 None
OpBranchConditional %false %13 %14
%13 = OpLabel
%15 = OpIAdd %uint %uint_0 %uint_0
OpBranch %12
%14 = OpLabel
OpStore %19 %true
OpBranch %12
%12 = OpLabel
%27 = OpPhi %uint %15 %13 %26 %14
%22 = OpLoad %bool %19
OpBranchConditional %22 %9 %21
%21 = OpLabel
OpBranch %9
%9 = OpLabel
%28 = OpPhi %uint %27 %21 %26 %10 %26 %12
%25 = OpLoad %bool %19
OpSelectionMerge %24 None
OpBranchConditional %25 %24 %23
%23 = OpLabel
%16 = OpIAdd %uint %28 %28
OpStore %19 %true
OpBranch %24
%24 = OpLabel
OpBranch %17
%17 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

// This is essentially the same as NestedSelectionMerge, except
// the order of the first branch is changed.  This is to make sure things
// work even if the order of the traversals change.
TEST_F(MergeReturnPassTest, NestedSelectionMerge2) {
  const std::string before =
      R"(      OpCapability Addresses
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "simple_shader"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %false = OpConstantFalse %bool
          %7 = OpTypeFunction %void
          %1 = OpFunction %void None %7
          %8 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %false %10 %11
         %11 = OpLabel
               OpReturn
         %10 = OpLabel
               OpSelectionMerge %12 None
               OpBranchConditional %false %13 %14
         %13 = OpLabel
         %15 = OpIAdd %uint %uint_0 %uint_0
               OpBranch %12
         %14 = OpLabel
               OpReturn
         %12 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %16 = OpIAdd %uint %15 %15
               OpReturn
               OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "simple_shader"
%void = OpTypeVoid
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%false = OpConstantFalse %bool
%7 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%26 = OpUndef %uint
%1 = OpFunction %void None %7
%8 = OpLabel
%19 = OpVariable %_ptr_Function_bool Function %false
OpSelectionMerge %9 None
OpBranchConditional %false %10 %11
%11 = OpLabel
OpStore %19 %true
OpBranch %9
%10 = OpLabel
OpSelectionMerge %12 None
OpBranchConditional %false %13 %14
%13 = OpLabel
%15 = OpIAdd %uint %uint_0 %uint_0
OpBranch %12
%14 = OpLabel
OpStore %19 %true
OpBranch %12
%12 = OpLabel
%27 = OpPhi %uint %15 %13 %26 %14
%25 = OpLoad %bool %19
OpBranchConditional %25 %9 %24
%24 = OpLabel
OpBranch %9
%9 = OpLabel
%28 = OpPhi %uint %27 %24 %26 %11 %26 %12
%23 = OpLoad %bool %19
OpSelectionMerge %22 None
OpBranchConditional %23 %22 %21
%21 = OpLabel
%16 = OpIAdd %uint %28 %28
OpStore %19 %true
OpBranch %22
%22 = OpLabel
OpBranch %17
%17 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

TEST_F(MergeReturnPassTest, NestedSelectionMerge3) {
  const std::string before =
      R"(      OpCapability Addresses
               OpCapability Shader
               OpCapability Linkage
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "simple_shader"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
      %false = OpConstantFalse %bool
          %7 = OpTypeFunction %void
          %1 = OpFunction %void None %7
          %8 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %false %10 %11
         %11 = OpLabel
               OpReturn
         %10 = OpLabel
         %12 = OpIAdd %uint %uint_0 %uint_0
               OpSelectionMerge %13 None
               OpBranchConditional %false %14 %15
         %14 = OpLabel
               OpBranch %13
         %15 = OpLabel
               OpReturn
         %13 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %16 = OpIAdd %uint %12 %12
               OpReturn
               OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Addresses
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "simple_shader"
%void = OpTypeVoid
%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%false = OpConstantFalse %bool
%7 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%26 = OpUndef %uint
%1 = OpFunction %void None %7
%8 = OpLabel
%19 = OpVariable %_ptr_Function_bool Function %false
OpSelectionMerge %9 None
OpBranchConditional %false %10 %11
%11 = OpLabel
OpStore %19 %true
OpBranch %9
%10 = OpLabel
%12 = OpIAdd %uint %uint_0 %uint_0
OpSelectionMerge %13 None
OpBranchConditional %false %14 %15
%14 = OpLabel
OpBranch %13
%15 = OpLabel
OpStore %19 %true
OpBranch %13
%13 = OpLabel
%25 = OpLoad %bool %19
OpBranchConditional %25 %9 %24
%24 = OpLabel
OpBranch %9
%9 = OpLabel
%27 = OpPhi %uint %12 %24 %26 %11 %26 %13
%23 = OpLoad %bool %19
OpSelectionMerge %22 None
OpBranchConditional %23 %22 %21
%21 = OpLabel
%16 = OpIAdd %uint %27 %27
OpStore %19 %true
OpBranch %22
%22 = OpLabel
OpBranch %17
%17 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}
*/

TEST_F(MergeReturnPassTest, NestedLoopMerge) {
  const std::string before =
      R"(               OpCapability SampledBuffer
               OpCapability StorageImageExtendedFormats
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %2 "CS"
               OpExecutionMode %2 LocalSize 8 8 1
               OpSource HLSL 600
               OpName %function "function"
       %uint = OpTypeInt 32 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
%_ptr_Function_uint = OpTypePointer Function %uint
 %_struct_13 = OpTypeStruct %v3uint %v3uint %v3uint %uint %uint %uint %uint %uint %uint
          %2 = OpFunction %void None %6
         %14 = OpLabel
         %15 = OpFunctionCall %void %function
               OpReturn
               OpFunctionEnd
   %function = OpFunction %void None %6
         %16 = OpLabel
         %17 = OpVariable %_ptr_Function_uint Function
         %18 = OpVariable %_ptr_Function_uint Function
               OpStore %17 %uint_0
               OpBranch %19
         %19 = OpLabel
         %20 = OpLoad %uint %17
         %21 = OpULessThan %bool %20 %uint_1
               OpLoopMerge %22 %23 DontUnroll
               OpBranchConditional %21 %24 %22
         %24 = OpLabel
               OpStore %18 %uint_1
               OpBranch %25
         %25 = OpLabel
         %26 = OpLoad %uint %18
         %27 = OpINotEqual %bool %26 %uint_0
               OpLoopMerge %28 %29 DontUnroll
               OpBranchConditional %27 %30 %28
         %30 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %true %32 %31
         %32 = OpLabel
               OpReturn
         %31 = OpLabel
               OpStore %18 %uint_1
               OpBranch %29
         %29 = OpLabel
               OpBranch %25
         %28 = OpLabel
               OpBranch %23
         %23 = OpLabel
         %33 = OpLoad %uint %17
         %34 = OpIAdd %uint %33 %uint_1
               OpStore %17 %34
               OpBranch %19
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability SampledBuffer
OpCapability StorageImageExtendedFormats
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %2 "CS"
OpExecutionMode %2 LocalSize 8 8 1
OpSource HLSL 600
OpName %function "function"
%uint = OpTypeInt 32 0
%void = OpTypeVoid
%6 = OpTypeFunction %void
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%v3uint = OpTypeVector %uint 3
%bool = OpTypeBool
%true = OpConstantTrue %bool
%_ptr_Function_uint = OpTypePointer Function %uint
%_struct_13 = OpTypeStruct %v3uint %v3uint %v3uint %uint %uint %uint %uint %uint %uint
%false = OpConstantFalse %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%2 = OpFunction %void None %6
%14 = OpLabel
%15 = OpFunctionCall %void %function
OpReturn
OpFunctionEnd
%function = OpFunction %void None %6
%16 = OpLabel
%38 = OpVariable %_ptr_Function_bool Function %false
%17 = OpVariable %_ptr_Function_uint Function
%18 = OpVariable %_ptr_Function_uint Function
OpStore %17 %uint_0
OpBranch %19
%19 = OpLabel
%20 = OpLoad %uint %17
%21 = OpULessThan %bool %20 %uint_1
OpLoopMerge %22 %23 DontUnroll
OpBranchConditional %21 %24 %22
%24 = OpLabel
OpStore %18 %uint_1
OpBranch %25
%25 = OpLabel
%26 = OpLoad %uint %18
%27 = OpINotEqual %bool %26 %uint_0
OpLoopMerge %28 %29 DontUnroll
OpBranchConditional %27 %30 %28
%30 = OpLabel
OpSelectionMerge %31 None
OpBranchConditional %true %32 %31
%32 = OpLabel
OpStore %38 %true
OpBranch %28
%31 = OpLabel
OpStore %18 %uint_1
OpBranch %29
%29 = OpLabel
OpBranch %25
%28 = OpLabel
%40 = OpLoad %bool %38
OpBranchConditional %40 %22 %39
%39 = OpLabel
OpBranch %23
%23 = OpLabel
%33 = OpLoad %uint %17
%34 = OpIAdd %uint %33 %uint_1
OpStore %17 %34
OpBranch %19
%22 = OpLabel
%43 = OpLoad %bool %38
OpSelectionMerge %42 None
OpBranchConditional %43 %42 %41
%41 = OpLabel
OpStore %38 %true
OpBranch %42
%42 = OpLabel
OpBranch %35
%35 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

TEST_F(MergeReturnPassTest, ReturnValueDecoration) {
  const std::string before =
      R"(OpCapability Linkage
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %11 "simple_shader"
OpDecorate %7 RelaxedPrecision
%12 = OpTypeVoid
%1 = OpTypeInt 32 0
%2 = OpTypeBool
%3 = OpConstantFalse %2
%4 = OpConstant %1 0
%5 = OpConstant %1 1
%6 = OpTypeFunction %1
%13 = OpTypeFunction %12
%11 = OpFunction %12 None %13
%l1 = OpLabel
OpReturn
OpFunctionEnd
%7 = OpFunction %1 None %6
%8 = OpLabel
OpBranchConditional %3 %9 %10
%9 = OpLabel
OpReturnValue %4
%10 = OpLabel
OpReturnValue %5
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Linkage
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %11 "simple_shader"
OpDecorate %7 RelaxedPrecision
OpDecorate %17 RelaxedPrecision
OpDecorate %18 RelaxedPrecision
%12 = OpTypeVoid
%1 = OpTypeInt 32 0
%2 = OpTypeBool
%3 = OpConstantFalse %2
%4 = OpConstant %1 0
%5 = OpConstant %1 1
%6 = OpTypeFunction %1
%13 = OpTypeFunction %12
%16 = OpTypePointer Function %1
%19 = OpTypePointer Function %2
%21 = OpConstantTrue %2
%11 = OpFunction %12 None %13
%14 = OpLabel
OpReturn
OpFunctionEnd
%7 = OpFunction %1 None %6
%8 = OpLabel
%20 = OpVariable %19 Function %3
%17 = OpVariable %16 Function
OpBranchConditional %3 %9 %10
%9 = OpLabel
OpStore %20 %21
OpStore %17 %4
OpBranch %15
%10 = OpLabel
OpStore %20 %21
OpStore %17 %5
OpBranch %15
%15 = OpLabel
%18 = OpLoad %1 %17
OpReturnValue %18
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<MergeReturnPass>(before, after, false, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
