// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvParserTest, Phi_FromBlock) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpBranch %1
          %1 = OpLabel
          %2 = OpPhi %int %int_2 %main_start
          %3 = OpIAdd %int %2 %2
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 2i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_If_Undef) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpBranch %1
          %1 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %3 %4
          %3 = OpLabel
          %6 = OpUndef %int
               OpBranch %5
          %4 = OpLabel
          %7 = OpUndef %int
               OpBranch %5
          %5 = OpLabel
          %8 = OpPhi %int %6 %3 %7 %4
          %9 = OpIAdd %int %8 %8
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if 0i  # if_1
      }
      $B3: {  # false
        exit_if 0i  # if_1
      }
    }
    %3:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_If_ThenAndElse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpBranch %1
          %1 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %3 %4
          %3 = OpLabel
          %6 = OpIAdd %int %int_1 %int_2
               OpBranch %5
          %4 = OpLabel
          %7 = OpIAdd %int %int_3 %int_4
               OpBranch %5
          %5 = OpLabel
          %8 = OpPhi %int %6 %3 %7 %4
          %9 = OpIAdd %int %8 %8
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = spirv.add<i32> 1i, 2i
        exit_if %3  # if_1
      }
      $B3: {  # false
        %4:i32 = spirv.add<i32> 3i, 4i
        exit_if %4  # if_1
      }
    }
    %5:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_If_ThenNoElse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpBranch %1
          %1 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %3 %5
          %3 = OpLabel
          %6 = OpIAdd %int %int_1 %int_2
               OpBranch %5
          %5 = OpLabel
          %8 = OpPhi %int %int_2 %1 %6 %3
          %9 = OpIAdd %int %8 %8
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = spirv.add<i32> 1i, 2i
        exit_if %3  # if_1
      }
      $B3: {  # false
        exit_if 2i  # if_1
      }
    }
    %4:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_If_NoThenElse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpBranch %1
          %1 = OpLabel
               OpSelectionMerge %5 None
               OpBranchConditional %true %5 %3
          %3 = OpLabel
          %6 = OpIAdd %int %int_1 %int_2
               OpBranch %5
          %5 = OpLabel
          %8 = OpPhi %int %6 %3 %int_2 %1
          %9 = OpIAdd %int %8 %8
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if 2i  # if_1
      }
      $B3: {  # false
        %3:i32 = spirv.add<i32> 1i, 2i
        exit_if %3  # if_1
      }
    }
    %4:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Switch) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpSelectionMerge %13 None
               OpSwitch %int_0 %10 0 %11 1 %12
         %10 = OpLabel
               OpBranch %13
         %11 = OpLabel
               OpBranch %13
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %14 = OpPhi %int %int_0 %10 %int_1 %11 %int_2 %12
         %15 = OpIAdd %int %14 %14
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = switch 0i [c: (default, $B2), c: (0i, $B3), c: (1i, $B4)] {  # switch_1
      $B2: {  # case
        exit_switch 0i  # switch_1
      }
      $B3: {  # case
        exit_switch 1i  # switch_1
      }
      $B4: {  # case
        exit_switch 2i  # switch_1
      }
    }
    %3:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Switch_FromIfBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %bool = OpTypeBool
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %3
          %4 = OpLabel
               OpSelectionMerge %13 None
               OpSwitch %int_0 %10 0 %11 1 %12
         %10 = OpLabel
               OpBranch %13
         %11 = OpLabel
               OpBranch %13
         %12 = OpLabel
               OpSelectionMerge %20 None
               OpBranchConditional %true %20 %21
         %21 = OpLabel
               OpBranch %13
         %20 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %14 = OpPhi %int %int_0 %10 %int_1 %11 %int_2 %20 %int_3 %21
         %15 = OpIAdd %int %14 %14
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = switch 0i [c: (default, $B2), c: (0i, $B3), c: (1i, $B4)] {  # switch_1
      $B2: {  # case
        exit_switch 0i  # switch_1
      }
      $B3: {  # case
        exit_switch 1i  # switch_1
      }
      $B4: {  # case
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_switch 3i  # switch_1
          }
        }
        exit_switch 2i  # switch_1
      }
    }
    %3:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Switch_FromIfBreak_InDefault) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %bool = OpTypeBool
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %9
         %45 = OpLabel
               OpSelectionMerge %52 None
               OpSwitch %uint_0 %53
         %53 = OpLabel
               OpBranch %54
         %54 = OpLabel
               OpSelectionMerge %84 None
               OpBranchConditional %true %52 %84
         %84 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %85 = OpPhi %float %float_7 %54 %float_8 %84
        %100 = OpFAdd %float %85 %85
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = switch 0u [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        if true [t: $B3, f: $B4] {  # if_1
          $B3: {  # true
            exit_switch 7.0f  # switch_1
          }
          $B4: {  # false
            exit_if  # if_1
          }
        }
        exit_switch 8.0f  # switch_1
      }
    }
    %3:f32 = add %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Switch_FromIf_BothJumpToMerge_InDefault) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %bool = OpTypeBool
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %9
         %45 = OpLabel
               OpSelectionMerge %52 None
               OpSwitch %uint_0 %53
         %53 = OpLabel
               OpBranch %54
         %54 = OpLabel
               OpSelectionMerge %84 None
               OpBranchConditional %true %52 %52
         %84 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %85 = OpPhi %float %float_7 %54 %float_8 %84
        %100 = OpFAdd %float %85 %85
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = switch 0u [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %3:bool = or true, true
        if %3 [t: $B3, f: $B4] {  # if_1
          $B3: {  # true
            exit_switch 7.0f  # switch_1
          }
          $B4: {  # false
            unreachable
          }
        }
        exit_switch 8.0f  # switch_1
      }
    }
    %4:f32 = add %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Switch_FromIfBreakBoth_InDefault) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %bool = OpTypeBool
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %9
         %45 = OpLabel
               OpSelectionMerge %52 None
               OpSwitch %uint_0 %53
         %53 = OpLabel
               OpBranch %54
         %54 = OpLabel
               OpSelectionMerge %84 None
               OpBranchConditional %true %55 %56
         %55 = OpLabel
               OpBranch %52
         %56 = OpLabel
               OpBranch %52
         %84 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %85 = OpPhi %float %float_7 %55 %float_8 %56 %float_9 %84
        %100 = OpFAdd %float %85 %85
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = switch 0u [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        if true [t: $B3, f: $B4] {  # if_1
          $B3: {  # true
            exit_switch 7.0f  # switch_1
          }
          $B4: {  # false
            exit_switch 8.0f  # switch_1
          }
        }
        exit_switch 9.0f  # switch_1
      }
    }
    %3:f32 = add %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Loop_FromIfBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %bool = OpTypeBool
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %9
         %45 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %52 %55 None
               OpBranch %60
         %55 = OpLabel
               OpBranch %50
         %60 = OpLabel
               OpSelectionMerge %84 None
               OpBranchConditional %true %52 %84
         %84 = OpLabel
               OpBranch %52
         %52 = OpLabel
         %85 = OpPhi %float %float_7 %60 %float_8 %84
        %100 = OpFAdd %float %85 %85
               OpReturn
               OpFunctionEnd

)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop 7.0f  # loop_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        exit_loop 8.0f  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %3:f32 = add %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Loop_ContinueIsHeader) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %false %20
               OpLoopMerge %99 %20 None
               OpBranchConditional %101 %99 %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%2:bool): {  # body
        if %2 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            continue  # -> $B4
          }
        }
        unreachable
      }
      $B4: {  # continuing
        next_iteration false  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Loop_WithContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %false %30
               OpLoopMerge %99 %30 None
               OpBranchConditional %101 %99 %25
         %25 = OpLabel
               OpBranch %30
         %30 = OpLabel
        %102 = OpCopyObject %bool %101
               OpBranch %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%2:bool): {  # body
        if %2 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            continue  # -> $B4
          }
        }
        unreachable
      }
      $B4: {  # continuing
        %3:bool = let %2
        next_iteration false  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Loop_WithContinue_PhiInContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %30 None
               OpBranchConditional %true %24 %30
         %24 = OpLabel
               OpBranch %30
         %30 = OpLabel
        %101 = OpPhi %bool %true %24 %false %20
        %102 = OpCopyObject %bool %101
               OpBranchConditional %true %99 %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue true  # -> $B3
          }
          $B5: {  # false
            continue false  # -> $B3
          }
        }
        unreachable
      }
      $B3 (%2:bool): {  # continuing
        %3:bool = let %2
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Loop_WithMultiblockContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %false %31
               OpLoopMerge %99 %30 None
               OpBranchConditional %101 %99 %25
         %25 = OpLabel
               OpBranch %30
         %30 = OpLabel
        %102 = OpCopyObject %bool %101
               OpBranch %31
         %31 = OpLabel
               OpBranch %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%2:bool): {  # body
        if %2 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            continue  # -> $B4
          }
        }
        unreachable
      }
      $B4: {  # continuing
        %3:bool = let %2
        next_iteration false  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Loop_BranchConditionalBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpBranch %1
          %1 = OpLabel
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %50 %20
         %50 = OpLabel
          %6 = OpIAdd %int %int_1 %int_2
               OpBranch %99
         %20 = OpLabel
               OpBranch %1
         %99 = OpLabel
          %8 = OpPhi %int %6 %50
          %9 = OpIAdd %int %8 %8
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %3:i32 = spirv.add<i32> 1i, 2i
            exit_loop %3  # loop_1
          }
          $B5: {  # false
            continue  # -> $B3
          }
        }
        unreachable
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %4:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

// Phis must act as if they are simultaneously assigned. %101 and %102 should exchange values on
// each iteration, and never have the same value.
TEST_F(SpirvParserTest, Phi_SimultaneousAssignment) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %101 "default_true"
               OpName %102 "default_false"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %102 %20
        %102 = OpPhi %bool %false %10 %101 %20
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %99 %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true, false  # -> $B3
      }
      $B3 (%2:bool, %3:bool): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            continue  # -> $B4
          }
        }
        unreachable
      }
      $B4: {  # continuing
        next_iteration %3, %2  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_SingleBlockLoopIndex) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %2 "computed"
               OpName %3 "copied"
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %pty = OpTypePointer Private %uint
          %1 = OpVariable %pty Private
    %boolpty = OpTypePointer Private %bool
          %7 = OpVariable %boolpty Private
          %8 = OpVariable %boolpty Private
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpBranch %10
; Use an outer loop to show we put the new variable in the
; smallest enclosing scope.
         %10 = OpLabel
        %101 = OpLoad %bool %7
        %102 = OpLoad %bool %8
        %103 = OpIAdd %uint %uint_0 %uint_0
               OpLoopMerge %99 %89 None
               OpBranchConditional %101 %99 %20
         %20 = OpLabel
          %2 = OpPhi %uint %103 %10 %4 %20  ; gets computed value
          %3 = OpPhi %uint %uint_1 %10 %3 %20  ; gets itself
          %4 = OpIAdd %uint %2 %uint_1
               OpLoopMerge %79 %20 None
               OpBranchConditional %102 %79 %20
         %79 = OpLabel
               OpBranch %89
         %89 = OpLabel
               OpBranch %10
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var undef
  %2:ptr<private, bool, read_write> = var undef
  %3:ptr<private, bool, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %5:bool = load %2
        %6:bool = load %3
        %7:u32 = spirv.add<u32> 0u, 0u
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            loop [i: $B7, b: $B8, c: $B9] {  # loop_2
              $B7: {  # initializer
                next_iteration %7, 1u  # -> $B8
              }
              $B8 (%8:u32, %9:u32): {  # body
                %10:u32 = spirv.add<u32> %8, 1u
                if %6 [t: $B10, f: $B11] {  # if_2
                  $B10: {  # true
                    exit_loop  # loop_2
                  }
                  $B11: {  # false
                    continue  # -> $B9
                  }
                }
                unreachable
              }
              $B9: {  # continuing
                next_iteration %10, %9  # -> $B8
              }
            }
            continue  # -> $B4
          }
        }
        unreachable
      }
      $B4: {  # continuing
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_MultiBlockLoopIndex) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %pty = OpTypePointer Private %uint
          %1 = OpVariable %pty Private
    %boolpty = OpTypePointer Private %bool
          %7 = OpVariable %boolpty Private
          %8 = OpVariable %boolpty Private
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %5 = OpLabel
               OpBranch %10
; Use an outer loop to show we put the new variable in the
; smallest enclosing scope.
         %10 = OpLabel
        %101 = OpLoad %bool %7
        %102 = OpLoad %bool %8
               OpLoopMerge %99 %89 None
               OpBranchConditional %101 %99 %20
         %20 = OpLabel
          %2 = OpPhi %uint %uint_0 %10 %4 %30  ; gets computed value
          %3 = OpPhi %uint %uint_1 %10 %3 %30  ; gets itself
               OpLoopMerge %79 %30 None
               OpBranchConditional %102 %79 %30
         %30 = OpLabel  ; continue target for inner loop
          %4 = OpIAdd %uint %2 %uint_1
               OpBranch %20
         %79 = OpLabel  ; merge for inner loop
               OpBranch %89
         %89 = OpLabel  ; continue target for outer loop
               OpBranch %10
         %99 = OpLabel  ; merge for outer loop
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var undef
  %2:ptr<private, bool, read_write> = var undef
  %3:ptr<private, bool, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %5:bool = load %2
        %6:bool = load %3
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            loop [i: $B7, b: $B8, c: $B9] {  # loop_2
              $B7: {  # initializer
                next_iteration 0u, 1u  # -> $B8
              }
              $B8 (%7:u32, %8:u32): {  # body
                if %6 [t: $B10, f: $B11] {  # if_2
                  $B10: {  # true
                    exit_loop  # loop_2
                  }
                  $B11: {  # false
                    continue  # -> $B9
                  }
                }
                unreachable
              }
              $B9: {  # continuing
                %9:u32 = spirv.add<u32> %7, 1u
                next_iteration %9, %8  # -> $B8
              }
            }
            continue  # -> $B4
          }
        }
        unreachable
      }
      $B4: {  # continuing
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_ValueFromLoopBodyAndContinuing) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %pty = OpTypePointer Private %uint
          %1 = OpVariable %pty Private
    %boolpty = OpTypePointer Private %bool
         %17 = OpVariable %boolpty Private
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %9 = OpLabel
        %101 = OpLoad %bool %17
               OpBranch %10
; Use an outer loop to show we put the new variable in the
; smallest enclosing scope.
         %10 = OpLabel
               OpLoopMerge %99 %89 None
               OpBranch %20
         %20 = OpLabel
          %2 = OpPhi %uint %uint_0 %10 %4 %30  ; gets computed value
          %5 = OpPhi %uint %uint_1 %10 %7 %30
          %4 = OpIAdd %uint %2 %uint_1 ; define %4
          %6 = OpIAdd %uint %4 %uint_1 ; use %4
               OpLoopMerge %79 %30 None
               OpBranchConditional %101 %79 %30
         %30 = OpLabel
          %7 = OpIAdd %uint %4 %6 ; use %4 again
          %8 = OpCopyObject %uint %5 ; use %5
               OpBranchConditional %true %20 %79
         %79 = OpLabel
               OpBranch %89
         %89 = OpLabel
               OpBranchConditional %true %10 %99
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var undef
  %2:ptr<private, bool, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = load %2
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        loop [i: $B5, b: $B6, c: $B7] {  # loop_2
          $B5: {  # initializer
            next_iteration 0u, 1u  # -> $B6
          }
          $B6 (%5:u32, %6:u32): {  # body
            %7:u32 = spirv.add<u32> %5, 1u
            %8:u32 = spirv.add<u32> %7, 1u
            if %4 [t: $B8, f: $B9] {  # if_1
              $B8: {  # true
                exit_loop  # loop_2
              }
              $B9: {  # false
                continue  # -> $B7
              }
            }
            unreachable
          }
          $B7: {  # continuing
            %9:u32 = spirv.add<u32> %7, %8
            %10:u32 = let %6
            %11:bool = not true
            break_if %11 next_iteration: [ %7, %9 ]  # -> [t: exit_loop loop_2, f: $B6]
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %12:bool = not true
        break_if %12  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_FromElseAndThen) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %pty = OpTypePointer Private %uint
          %1 = OpVariable %pty Private
    %boolpty = OpTypePointer Private %bool
          %7 = OpVariable %boolpty Private
          %8 = OpVariable %boolpty Private
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %5 = OpLabel
        %101 = OpLoad %bool %7
        %102 = OpLoad %bool %8
               OpBranch %10
; Use an outer loop to show we put the new variable in the
; smallest enclosing scope.
         %10 = OpLabel
               OpLoopMerge %99 %89 None
               OpBranchConditional %101 %99 %20
         %20 = OpLabel ; if seleciton
               OpSelectionMerge %79 None
               OpBranchConditional %102 %30 %40
         %30 = OpLabel
               OpBranch %89
         %40 = OpLabel
               OpBranch %89
         %79 = OpLabel ; disconnected selection merge node
               OpBranch %89
         %89 = OpLabel
          %2 = OpPhi %uint %uint_0 %30 %uint_1 %40 %uint_3 %79
               OpStore %1 %2
               OpBranch %10
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var undef
  %2:ptr<private, bool, read_write> = var undef
  %3:ptr<private, bool, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:bool = load %2
    %6:bool = load %3
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            if %6 [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                continue 0u  # -> $B4
              }
              $B8: {  # false
                continue 1u  # -> $B4
              }
            }
            continue 3u  # -> $B4
          }
        }
        unreachable
      }
      $B4 (%7:u32): {  # continuing
        store %1, %7
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_FromHeaderAndThen) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %uint = OpTypeInt 32 0
        %pty = OpTypePointer Private %uint
          %1 = OpVariable %pty Private
    %boolpty = OpTypePointer Private %bool
          %7 = OpVariable %boolpty Private
          %8 = OpVariable %boolpty Private
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %5 = OpLabel
        %101 = OpLoad %bool %7
        %102 = OpLoad %bool %8
               OpBranch %10
; Use an outer loop to show we put the new variable in the
; smallest enclosing scope.
         %10 = OpLabel
               OpLoopMerge %99 %89 None
               OpBranchConditional %101 %99 %20
         %20 = OpLabel ; if seleciton
               OpSelectionMerge %79 None
               OpBranchConditional %102 %30 %89
         %30 = OpLabel
               OpBranch %89
         %79 = OpLabel ; disconnected selection merge node
               OpUnreachable
         %89 = OpLabel
          %2 = OpPhi %uint %uint_0 %20 %uint_1 %30
          %3 = OpPhi %uint %uint_1 %20 %uint_0 %30
          %4 = OpIAdd %uint %2 %3
               OpStore %1 %4
               OpBranch %10
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var undef
  %2:ptr<private, bool, read_write> = var undef
  %3:ptr<private, bool, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:bool = load %2
    %6:bool = load %3
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        if %5 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            if %6 [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                continue 1u, 0u  # -> $B4
              }
              $B8: {  # false
                continue 0u, 1u  # -> $B4
              }
            }
            unreachable
          }
        }
        unreachable
      }
      $B4 (%7:u32, %8:u32): {  # continuing
        %9:u32 = spirv.add<u32> %7, %8
        store %1, %9
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

// If the only use of a combinatorially computed ID is as the value in an OpPhi, then we still have
// to emit it.  The algorithm fix is to always count uses in Phis. This is the reduced case from the
// bug report.
//
// * The only use of %12 is in the phi.
// * The only use of %11 is in %12.
// * Both definitions need to be emitted to the output.
//
// https://crbug.com/215
TEST_F(SpirvParserTest, Phi_UseInPhiCountsAsUse) {
    EXPECT_IR(
        R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %11 = OpLogicalAnd %bool %true %true
         %12 = OpLogicalNot %bool %11  ;
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %99
         %20 = OpLabel
               OpBranch %99
         %99 = OpLabel
        %101 = OpPhi %bool %11 %10 %12 %20
        %102 = OpCopyObject %bool %101  ;; ensure a use of %101
               OpReturn
               OpFunctionEnd
)",
        R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = and true, true
    %3:bool = not %2
    %4:bool = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if %3  # if_1
      }
      $B3: {  # false
        exit_if %2  # if_1
      }
    }
    %5:bool = let %4
    ret
  }
}
)");
}

// Value %999 is defined deep in control flow, then we arrange for it to dominate the backedge of
// the outer loop. The %999 value is then fed back into the phi in the loop header.  So %999 needs
// to be hoisted out of the loop.  The phi assignment needs to use the hoisted variable. The hoisted
// variable needs to be placed such that its scope encloses that phi in the header of the outer
// loop. The compiler needs to "see" that there is an implicit use of %999 in the backedge block of
// that outer loop.
//
// https://crbug.com/1649
TEST_F(SpirvParserTest, Phi_PhiInLoopHeader_FedByHoistedVar_PhiUnused) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %999 %80
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %40 %50
         %40 = OpLabel
        %999 = OpCopyObject %bool %true
               OpBranch %60
         %50 = OpLabel
               OpReturn
         %60 = OpLabel ; if merge
               OpBranch %80
         %80 = OpLabel ; continue target
               OpBranch %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%2:bool): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            if true [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                %3:bool = let true
                continue %3  # -> $B4
              }
              $B8: {  # false
                exit_if  # if_2
              }
            }
            ret
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        unreachable
      }
      $B4 (%4:bool): {  # continuing
        next_iteration %4  # -> $B3
      }
    }
    ret
  }
}
)");
}

// Value %999 is defined deep in control flow, then we arrange for it to dominate the backedge of
// the outer loop. The %999 value is then fed back into the phi in the loop header.  So %999 needs
// to be hoisted out of the loop.  The phi assignment needs to use the hoisted variable. The hoisted
// variable needs to be placed such that its scope encloses that phi in the header of the outer
// loop. The compiler needs to "see" that there is an implicit use of %999 in the backedge block of
// that outer loop.
//
// https://crbug.com/1649
TEST_F(SpirvParserTest, Phi_PhiInLoopHeader_FedByHoistedVar_PhiUsed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %999 %80
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %40 %50
         %40 = OpLabel
        %999 = OpCopyObject %bool %true
               OpBranch %60
         %50 = OpLabel
               OpReturn
         %60 = OpLabel ; if merge
               OpBranch %80
         %80 = OpLabel ; continue target
               OpBranch %20
         %99 = OpLabel
       %1000 = OpCopyObject %bool %101
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%3:bool): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            if true [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                %4:bool = let true
                continue %4  # -> $B4
              }
              $B8: {  # false
                exit_if  # if_2
              }
            }
            ret
          }
          $B6: {  # false
            exit_loop %3  # loop_1
          }
        }
        unreachable
      }
      $B4 (%5:bool): {  # continuing
        next_iteration %5  # -> $B3
      }
    }
    %6:bool = let %2
    ret
  }
}
)");
}

// This is a reduction of one of the hard parts of test case
// vk-gl-cts/graphicsfuzz/stable-binarysearch-tree-false-if-discard-loop/1.spvasm
// In particular, see the data flow around %114 in that case.
//
// Here value %999 is is a *phi* defined deep in control flow, then we arrange for it to dominate
// the backedge of the outer loop. The %999 value is then fed back into the phi in the loop header.
// The variable generated to hold the %999 value needs to be placed such that its scope encloses
// that phi in the header of the outer loop. The compiler needs to "see" that there is an implicit
// use of %999 in the backedge block of that outer loop.
//
// https://crbug.com/1649
TEST_F(SpirvParserTest, Phi_PhiInLoopHeader_FedByPhi_PhiUnused) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %999 %80
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %99 %30
         %30 = OpLabel
               OpLoopMerge %70 %60 None
               OpBranch %40
         %40 = OpLabel
               OpBranchConditional %true %60 %50
         %50 = OpLabel
               OpBranch %60
         %60 = OpLabel ; inner continue
        %999 = OpPhi %bool %true %40 %false %50
               OpBranchConditional %true %70 %30
         %70 = OpLabel  ; inner merge
               OpBranch %80
         %80 = OpLabel ; outer continue target
               OpBranch %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%2:bool): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
          $B6: {  # false
            %3:bool = loop [b: $B7, c: $B8] {  # loop_2
              $B7: {  # body
                if true [t: $B9, f: $B10] {  # if_2
                  $B9: {  # true
                    continue true  # -> $B8
                  }
                  $B10: {  # false
                    continue false  # -> $B8
                  }
                }
                unreachable
              }
              $B8 (%4:bool): {  # continuing
                break_if true exit_loop: [ %4 ]  # -> [t: exit_loop loop_2, f: $B7]
              }
            }
            continue %3  # -> $B4
          }
        }
        unreachable
      }
      $B4 (%5:bool): {  # continuing
        next_iteration %5  # -> $B3
      }
    }
    ret
  }
}
)");
}

// This is a reduction of one of the hard parts of test case
// vk-gl-cts/graphicsfuzz/stable-binarysearch-tree-false-if-discard-loop/1.spvasm
// In particular, see the data flow around %114 in that case.
//
// Here value %999 is is a *phi* defined deep in control flow, then we arrange for it to dominate
// the backedge of the outer loop. The %999 value is then fed back into the phi in the loop header.
// The variable generated to hold the %999 value needs to be placed such that its scope encloses
// that phi in the header of the outer loop. The compiler needs to "see" that there is an implicit
// use of %999 in the backedge block of that outer loop.
//
// https://crbug.com/1649
TEST_F(SpirvParserTest, Phi_PhiInLoopHeader_FedByPhi_PhiUsed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
        %101 = OpPhi %bool %true %10 %999 %80
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %99 %30
         %30 = OpLabel
               OpLoopMerge %70 %60 None
               OpBranch %40
         %40 = OpLabel
               OpBranchConditional %true %60 %50
         %50 = OpLabel
               OpBranch %60
         %60 = OpLabel ; inner continue
        %999 = OpPhi %bool %true %40 %false %50
               OpBranchConditional %true %70 %30
         %70 = OpLabel  ; inner merge
               OpBranch %80
         %80 = OpLabel ; outer continue target
               OpBranch %20
         %99 = OpLabel
       %1000 = OpCopyObject %bool %101
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%3:bool): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_loop %3  # loop_1
          }
          $B6: {  # false
            %4:bool = loop [b: $B7, c: $B8] {  # loop_2
              $B7: {  # body
                if true [t: $B9, f: $B10] {  # if_2
                  $B9: {  # true
                    continue true  # -> $B8
                  }
                  $B10: {  # false
                    continue false  # -> $B8
                  }
                }
                unreachable
              }
              $B8 (%5:bool): {  # continuing
                break_if true exit_loop: [ %5 ]  # -> [t: exit_loop loop_2, f: $B7]
              }
            }
            continue %4  # -> $B4
          }
        }
        unreachable
      }
      $B4 (%6:bool): {  # continuing
        next_iteration %6  # -> $B3
      }
    }
    %7:bool = let %2
    ret
  }
}
)");
}

// A phi in an unreachable block may have no operands.
TEST_F(SpirvParserTest, Phi_UnreachableLoopMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %99
         %99 = OpLabel
               OpLoopMerge %101 %99 None
               OpBranch %99
        %101 = OpLabel
        %102 = OpPhi %uint
               OpUnreachable
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    unreachable
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_SwitchDefaultIsMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %67 = OpLabel
         %69 = OpIAdd %int %int_1 %int_1
               OpSelectionMerge %70 None
               OpSwitch %int_1 %70 0 %71
         %71 = OpLabel
         %82 = OpIAdd %int %69 %int_1
               OpBranch %70
         %70 = OpLabel
         %64 = OpPhi %int %69 %67 %82 %71
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    %3:i32 = switch 1i [c: (default, $B2), c: (0i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch %2  # switch_1
      }
      $B3: {  # case
        %4:i32 = spirv.add<i32> %2, 1i
        exit_switch %4  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_LoopWithIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %bool = OpTypeBool
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %11
         %33 = OpLabel
               OpBranch %40
         %40 = OpLabel
               OpLoopMerge %47 %43 None
               OpBranchConditional %true %48 %47
         %48 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpSelectionMerge %59 None
               OpBranchConditional %true %47 %59
         %59 = OpLabel
               OpBranch %43
         %43 = OpLabel
               OpBranchConditional %true %47 %40
         %47 = OpLabel
         %60 = OpPhi %float %float_0 %40 %float_1 %49 %float_2 %43
         %61 = OpFAdd %float %60 %60
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            if true [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                exit_loop 1.0f  # loop_1
              }
              $B7: {  # false
                exit_if  # if_2
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop 0.0f  # loop_1
          }
        }
        unreachable
      }
      $B3: {  # continuing
        break_if true exit_loop: [ 2.0f ]  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    %3:f32 = add %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_InLoopBody) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %bool = OpTypeBool
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
       %main = OpFunction %void None %7
         %29 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %35 %34 None
               OpBranch %36
         %36 = OpLabel
         %33 = OpPhi %float %float_1 %31 %float_0 %37
         %99 = OpFAdd %float %33 %33
               OpReturn
         %37 = OpLabel
               OpBranch %36
         %34 = OpLabel
               OpBranch %31
         %35 = OpLabel
               OpUnreachable
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:f32 = add 1.0f, 1.0f
        ret
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    unreachable
  }
}
)");
}

// This is testing that an `OpPhi` value defined inside a loop (like %23) is correctly propagated
// when accessed from an `OpPhi` outside of the look (like %45). These phi values should turn into
// results on the loop, and then accessed as such in the two adds (%48 and %49)
TEST_F(SpirvParserTest, Phi_Propagated) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
         %80 = OpTypeFunction %void
        %int = OpTypeInt 32 1
          %1 = OpConstant %int 1
          %2 = OpConstant %int 2
          %3 = OpConstant %int 3
          %4 = OpConstant %int 4
          %5 = OpConstant %int 5
       %bool = OpTypeBool
          %8 = OpConstantTrue %bool
       %main = OpFunction %void None %80
         %21 = OpLabel
               OpBranch %22

         ; 1st loop
         %22 = OpLabel
         %23 = OpPhi %int %1 %21 %2 %25
         %24 = OpIAdd %int %23 %23
               OpLoopMerge %30 %25 None
               OpBranchConditional %8 %25 %30
         %25 = OpLabel
               OpBranch %22

         ; loop merge && if start
         %30 = OpLabel
         %99 = OpCopyObject %int %2
               OpSelectionMerge %34 None
               OpBranchConditional %8 %35 %34
         %35 = OpLabel
               OpBranch %36

         ; 2nd loop
         %36 = OpLabel
         %37 = OpPhi %int %3 %35 %4 %39
         %40 = OpPhi %int %23 %35 %5 %39
         %41 = OpIAdd %int %37 %40
               OpLoopMerge %43 %39 None
               OpBranchConditional %8 %39 %43
         %39 = OpLabel
               OpBranch %36
         %43 = OpLabel
               OpBranch %34

         ; if merge
         %34 = OpLabel
         %45 = OpPhi %int %99 %30 %37 %43
         %46 = OpPhi %int %3 %30 %40 %43
         %47 = OpPhi %int %4 %30 %40 %43
         %48 = OpIAdd %int %45 %46
         %49 = OpIAdd %int %48 %47
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:i32 = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration 1i  # -> $B3
      }
      $B3 (%3:i32): {  # body
        %4:i32 = spirv.add<i32> %3, %3
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            continue  # -> $B4
          }
          $B6: {  # false
            exit_loop %3  # loop_1
          }
        }
        unreachable
      }
      $B4: {  # continuing
        next_iteration 2i  # -> $B3
      }
    }
    %5:i32 = let 2i
    %6:i32, %7:i32, %8:i32 = if true [t: $B7, f: $B8] {  # if_2
      $B7: {  # true
        %9:i32, %10:i32 = loop [i: $B9, b: $B10, c: $B11] {  # loop_2
          $B9: {  # initializer
            next_iteration 3i, %2  # -> $B10
          }
          $B10 (%11:i32, %12:i32): {  # body
            %13:i32 = spirv.add<i32> %11, %12
            if true [t: $B12, f: $B13] {  # if_3
              $B12: {  # true
                continue  # -> $B11
              }
              $B13: {  # false
                exit_loop %11, %12  # loop_2
              }
            }
            unreachable
          }
          $B11: {  # continuing
            next_iteration 4i, 5i  # -> $B10
          }
        }
        exit_if %9, %10, %10  # if_2
      }
      $B8: {  # false
        exit_if %5, 3i, 4i  # if_2
      }
    }
    %14:i32 = spirv.add<i32> %6, %7
    %15:i32 = spirv.add<i32> %14, %8
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Propagated_PropagatedPhiValue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
        %int = OpTypeInt 32 1
          %1 = OpConstant %int 1
          %2 = OpConstant %int 2
          %3 = OpConstant %int 3
          %4 = OpConstant %int 4
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %8
         %22 = OpLabel
               OpBranch %27
         %27 = OpLabel
         %28 = OpPhi %int %2 %22 %3 %30
         %31 = OpPhi %int %1 %22 %4 %30
               OpLoopMerge %35 %30 None
               OpBranchConditional %true %36 %35
         %36 = OpLabel
         %29 = OpCopyObject %int %1
               OpSelectionMerge %40 None
               OpBranchConditional %true %41 %40
         %41 = OpLabel
               OpBranch %35
         %40 = OpLabel
               OpBranch %30
         %30 = OpLabel
               OpBranch %27
         %35 = OpLabel
         %42 = OpPhi %int %28 %27 %29 %41
         %43 = OpIAdd %int %42 %42
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:i32 = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration 2i, 1i  # -> $B3
      }
      $B3 (%3:i32, %4:i32): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            %5:i32 = let 1i
            if true [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                exit_loop %5  # loop_1
              }
              $B8: {  # false
                exit_if  # if_2
              }
            }
            continue  # -> $B4
          }
          $B6: {  # false
            exit_loop %3  # loop_1
          }
        }
        unreachable
      }
      $B4: {  # continuing
        next_iteration 3i, 4i  # -> $B3
      }
    }
    %6:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

// The object in %29 is propagated up through the IF as a result and then is used to pass the value
// through the continue block as %6 to OpPhi %28 as a block param.
TEST_F(SpirvParserTest, Phi_Propagated_PropagatedPhiValue_BlockParam) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
        %int = OpTypeInt 32 1
          %1 = OpConstant %int 1
          %2 = OpConstant %int 2
          %3 = OpConstant %int 3
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %8
         %22 = OpLabel
               OpBranch %27
         %27 = OpLabel
         %28 = OpPhi %int %2 %22 %29 %30
         %31 = OpPhi %int %1 %22 %3 %30
               OpLoopMerge %35 %30 None
               OpBranchConditional %true %36 %35
         %36 = OpLabel
         %29 = OpCopyObject %int %1
               OpSelectionMerge %40 None
               OpBranchConditional %true %41 %40
         %41 = OpLabel
               OpBranch %35
         %40 = OpLabel
               OpBranch %30
         %30 = OpLabel
               OpBranch %27
         %35 = OpLabel
         %42 = OpPhi %int %28 %27 %29 %41
         %43 = OpIAdd %int %42 %42
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:i32 = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration 2i, 1i  # -> $B3
      }
      $B3 (%3:i32, %4:i32): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            %5:i32 = let 1i
            if true [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                exit_loop %5  # loop_1
              }
              $B8: {  # false
                exit_if  # if_2
              }
            }
            continue %5  # -> $B4
          }
          $B6: {  # false
            exit_loop %3  # loop_1
          }
        }
        unreachable
      }
      $B4 (%6:i32): {  # continuing
        next_iteration %6, 3i  # -> $B3
      }
    }
    %7:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Phi_Propagated_BreakIf) {
    EXPECT_IR(R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %9
         %35 = OpLabel
               OpBranch %39
         %39 = OpLabel
         %40 = OpPhi %bool %true %35 %40 %41
               OpLoopMerge %44 %41 None
               OpBranch %45
         %45 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %true %50 %49
         %50 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpBranch %41
         %41 = OpLabel
               OpBranchConditional %true %39 %44
         %44 = OpLabel
               OpBranch %55
         %55 = OpLabel
         %56 = OpPhi %bool %40 %44 %true %57
               OpLoopMerge %59 %57 None
               OpBranch %60
         %60 = OpLabel
               OpBranch %57
         %57 = OpLabel
               OpBranch %55
         %59 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:bool = loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration true  # -> $B3
      }
      $B3 (%3:bool): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_if  # if_1
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %4:bool = not true
        break_if %4 next_iteration: [ %3 ] exit_loop: [ %3 ]  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    loop [i: $B7, b: $B8, c: $B9] {  # loop_2
      $B7: {  # initializer
        next_iteration %2  # -> $B8
      }
      $B8 (%5:bool): {  # body
        continue  # -> $B9
      }
      $B9: {  # continuing
        next_iteration true  # -> $B8
      }
    }
    ret
  }
}
)");
}

// Verify that %258 correctly propagates up to %259.  %258 is defined in a selection construct
// headed by %211, and needs to be propagated out of that "if" first, and then propagated from the
// phi.
TEST_F(SpirvParserTest, Phi_Propagated_If) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
       %bool = OpTypeBool
          %4 = OpConstantTrue %bool
      %float = OpTypeFloat 32
          %1 = OpConstant %float 1
          %2 = OpConstant %float 2
          %3 = OpConstant %float 3
       %main = OpFunction %void None %10
         %58 = OpLabel
               OpBranch %202
        %202 = OpLabel
               OpSelectionMerge %209 None
               OpBranchConditional %4 %210 %211
        %210 = OpLabel
               OpBranch %209
        %211 = OpLabel
               OpSelectionMerge %218 None
               OpBranchConditional %4 %219 %220
        %219 = OpLabel
               OpKill
        %220 = OpLabel
               OpSelectionMerge %222 None
               OpBranchConditional %4 %223 %224
        %223 = OpLabel
               OpBranch %222
        %224 = OpLabel
               OpBranch %222
        %222 = OpLabel
        %258 = OpPhi %float %1 %223 %2 %224
               OpBranch %218
        %218 = OpLabel
               OpBranch %209
        %209 = OpLabel
        %259 = OpPhi %float %3 %210 %258 %218
        %263 = OpFAdd %float %259 %259
               OpBranch %182
        %182 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    %2:f32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if 3.0f  # if_1
      }
      $B3: {  # false
        %3:f32 = if true [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            discard
            ret
          }
          $B5: {  # false
            %4:f32 = if true [t: $B6, f: $B7] {  # if_3
              $B6: {  # true
                exit_if 1.0f  # if_3
              }
              $B7: {  # false
                exit_if 2.0f  # if_3
              }
            }
            exit_if %4  # if_2
          }
        }
        exit_if %3  # if_1
      }
    }
    %5:f32 = add %2, %2
    ret
  }
}
)");
}

// The %34 is ignored and we propagate an undef because the continuing is unreachable
TEST_F(SpirvParserTest, Phi_FromContinuing) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %16 = OpTypeFunction %void
          %1 = OpFunction %void None %16
         %17 = OpLabel
               OpSelectionMerge %31 None
               OpBranchConditional %true %32 %31
         %32 = OpLabel
         %33 = OpPhi %int %int_0 %17 %34 %35
               OpLoopMerge %37 %35 None
               OpBranchConditional %true %38 %37
         %38 = OpLabel
               OpKill
         %35 = OpLabel
         %34 = OpCopyObject %int %int_1
               OpBranch %32
         %37 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @fragment func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        loop [i: $B4, b: $B5, c: $B6] {  # loop_1
          $B4: {  # initializer
            next_iteration 0i  # -> $B5
          }
          $B5 (%2:i32): {  # body
            if true [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                discard
                ret
              }
              $B8: {  # false
                exit_loop  # loop_1
              }
            }
            unreachable
          }
          $B6: {  # continuing
            next_iteration undef  # -> $B5
          }
        }
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

// Bug: https://crbug.com/dawn/435621075
TEST_F(SpirvParserTest, Phi_Bug_435621075) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
       %void = OpTypeVoid
         %29 = OpTypeFunction %void
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
       %main = OpFunction %void None %29
         %33 = OpLabel
               OpBranch %35
         %35 = OpLabel
         %36 = OpPhi %float %float_0 %33 %37 %38
               OpLoopMerge %44 %38 None
               OpBranchConditional %true %45 %44
         %45 = OpLabel
         %37 = OpCopyObject %float %float_0
               OpSelectionMerge %48 None
               OpBranchConditional %true %49 %48
         %49 = OpLabel
               OpBranch %38
         %48 = OpLabel
               OpBranch %38
         %38 = OpLabel
               OpBranch %35
         %44 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration 0.0f  # -> $B3
      }
      $B3 (%2:f32): {  # body
        if true [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            %3:f32 = let 0.0f
            if true [t: $B7, f: $B8] {  # if_2
              $B7: {  # true
                continue %3  # -> $B4
              }
              $B8: {  # false
                exit_if  # if_2
              }
            }
            continue %3  # -> $B4
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        unreachable
      }
      $B4 (%4:f32): {  # continuing
        next_iteration %4  # -> $B3
      }
    }
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
