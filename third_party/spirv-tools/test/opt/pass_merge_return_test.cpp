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
               OpBranchConditional %false %14 %15
         %14 = OpLabel
         %16 = OpIAdd %uint %uint_0 %uint_0
               OpBranch %12
         %15 = OpLabel
               OpReturn
         %12 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %17 = OpIAdd %uint %16 %16
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
%24 = OpUndef %uint
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
%25 = OpPhi %uint %15 %13 %24 %14
OpBranch %9
%9 = OpLabel
%26 = OpPhi %uint %25 %12 %24 %10
%23 = OpLoad %bool %19
OpSelectionMerge %22 None
OpBranchConditional %23 %22 %21
%21 = OpLabel
%16 = OpIAdd %uint %26 %26
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

// This is essentially the same as NestedSelectionMerge, except
// the order of the first branch is changed.  This is to make sure things
// work even if the order of the traversals change.
TEST_F(MergeReturnPassTest, NestedSelectionMerge2) {
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
         %11 = OpLabel
               OpReturn
         %10 = OpLabel
               OpSelectionMerge %12 None
               OpBranchConditional %false %14 %15
         %14 = OpLabel
         %16 = OpIAdd %uint %uint_0 %uint_0
               OpBranch %12
         %15 = OpLabel
               OpReturn
         %12 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %17 = OpIAdd %uint %16 %16
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
%24 = OpUndef %uint
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
%25 = OpPhi %uint %15 %13 %24 %14
OpBranch %9
%9 = OpLabel
%26 = OpPhi %uint %25 %12 %24 %11
%23 = OpLoad %bool %19
OpSelectionMerge %22 None
OpBranchConditional %23 %22 %21
%21 = OpLabel
%16 = OpIAdd %uint %26 %26
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
         %11 = OpLabel
               OpReturn
         %10 = OpLabel
         %16 = OpIAdd %uint %uint_0 %uint_0
               OpSelectionMerge %12 None
               OpBranchConditional %false %14 %15
         %14 = OpLabel
               OpBranch %12
         %15 = OpLabel
               OpReturn
         %12 = OpLabel
               OpBranch %9
          %9 = OpLabel
         %17 = OpIAdd %uint %16 %16
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
%24 = OpUndef %uint
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
OpBranch %9
%9 = OpLabel
%25 = OpPhi %uint %12 %13 %24 %11
%23 = OpLoad %bool %19
OpSelectionMerge %22 None
OpBranchConditional %23 %22 %21
%21 = OpLabel
%16 = OpIAdd %uint %25 %25
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

}  // namespace
}  // namespace opt
}  // namespace spvtools
