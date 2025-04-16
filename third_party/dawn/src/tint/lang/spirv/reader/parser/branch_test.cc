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

TEST_F(SpirvParserTest, Branch_Forward) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpBranch %bb_2
       %bb_2 = OpLabel
          %2 = OpCopyObject %i32 %two
               OpBranch %bb_3
       %bb_3 = OpLabel
          %3 = OpCopyObject %i32 %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:i32 = let 2i
    %4:i32 = let 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %10 = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_false
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpReturn
   %bb_false = OpLabel
          %3 = OpCopyObject %i32 %three
               OpBranch %bb_merge
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = let 2i
        ret
      }
      $B3: {  # false
        %4:i32 = let 3i
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Empty) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %true %99 %99
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = or true, true
    if %2 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        unreachable
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_TrueToMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_merge %bb_false
   %bb_false = OpLabel
          %3 = OpCopyObject %i32 %three
               OpBranch %bb_merge
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        %3:i32 = let 3i
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_FalseToMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_merge
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpReturn
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = let 2i
        ret
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

TEST_F(SpirvParserTest, BranchConditional_TrueMatchesFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpBranchConditional %true %bb_true %bb_true
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:bool = or true, true
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = let 2i
        ret
      }
      $B3: {  # false
        unreachable
      }
    }
    unreachable
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Nested_FalseExit) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_merge
    %bb_true = OpLabel
               OpBranchConditional %false %99 %bb_merge
         %99 = OpLabel
          %2 = OpCopyObject %i32 %two
               OpBranch %bb_merge
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        if false [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            %3:i32 = let 2i
            exit_if  # if_2
          }
          $B5: {  # false
            exit_if  # if_2
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

TEST_F(SpirvParserTest, BranchConditional_Nested_TrueExit) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_merge
    %bb_true = OpLabel
               OpBranchConditional %false %bb_merge %99
         %99 = OpLabel
          %2 = OpCopyObject %i32 %two
               OpBranch %bb_merge
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        if false [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            exit_if  # if_2
          }
          $B5: {  # false
            %3:i32 = let 2i
            exit_if  # if_2
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

TEST_F(SpirvParserTest, BranchConditional_TrueBranchesToFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %20 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %false %40 %30
         %30 = OpLabel
         %50 = OpIAdd %i32 %one %one
               OpReturn
         %40 = OpLabel
         %51 = OpIAdd %i32 %two %two
               OpBranch %30 ; backtrack, but does dominate %30
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if false [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %2:i32 = spirv.add<i32> 2i, 2i
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    if true [t: $B4, f: $B5] {  # if_2
      $B4: {  # true
        %3:i32 = spirv.add<i32> 1i, 1i
        ret
      }
      $B5: {  # false
        unreachable
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Hoisting) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_false
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpBranch %bb_merge
   %bb_false = OpLabel
               OpReturn
   %bb_merge = OpLabel
          %3 = OpIAdd %i32 %2 %2
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = let 2i
        exit_if %4  # if_1
      }
      $B3: {  # false
        ret
      }
    }
    %5:i32 = spirv.add<i32> %3, %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_HoistingIntoNested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
        %502 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %20 %30
         %20 = OpLabel
        %200 = OpCopyObject %i32 %two
               OpBranch %50
         %30 = OpLabel
               OpReturn
         %50 = OpLabel
               OpSelectionMerge %60 None
               OpBranchConditional %true %70 %80
         %70 = OpLabel
        %201 = OpIAdd %i32 %200 %200
               OpBranch %60
         %80 = OpLabel
               OpReturn
         %60 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = let 2i
        exit_if %3  # if_1
      }
      $B3: {  # false
        ret
      }
    }
    if true [t: $B4, f: $B5] {  # if_2
      $B4: {  # true
        %4:i32 = spirv.add<i32> %2, %2
        exit_if  # if_2
      }
      $B5: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_HoistingIntoParentNested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %1 = OpLabel
               OpSelectionMerge %90 None
               OpBranchConditional %true %10 %15
         %15 = OpLabel
               OpReturn
         %10 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %20 %30
         %20 = OpLabel
        %200 = OpCopyObject %i32 %two
               OpBranch %50
         %30 = OpLabel
               OpReturn
         %50 = OpLabel
               OpSelectionMerge %60 None
               OpBranchConditional %true %70 %80
         %70 = OpLabel
        %201 = OpIAdd %i32 %200 %200
               OpBranch %60
         %80 = OpLabel
               OpReturn
         %60 = OpLabel
               OpBranch %90
         %90 = OpLabel
        %202 = OpIAdd %i32 %200 %200
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = if true [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            %4:i32 = let 2i
            exit_if %4  # if_2
          }
          $B5: {  # false
            ret
          }
        }
        if true [t: $B6, f: $B7] {  # if_3
          $B6: {  # true
            %5:i32 = spirv.add<i32> %3, %3
            exit_if  # if_3
          }
          $B7: {  # false
            ret
          }
        }
        exit_if %3  # if_1
      }
      $B3: {  # false
        ret
      }
    }
    %6:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_DuplicateTrue_UsedValue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpBranchConditional %true %bb_true %bb_true
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
         %22 = OpCopyObject %i32 %2
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:bool = or true, true
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = let 2i
        %5:i32 = let %4
        ret
      }
      $B3: {  # false
        unreachable
      }
    }
    unreachable
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_DuplicateTrue_Premerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %true %14 %15
         %14 = OpLabel
               OpBranch %16
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
          %1 = OpCopyObject %i32 %one
          %2 = OpCopyObject %i32 %1
               OpBranch %13
         %13 = OpLabel
          %3 = OpCopyObject %i32 %1
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    %2:i32 = if true [t: $B4, f: $B5] {  # if_2
      $B4: {  # true
        %3:i32 = let 1i
        %4:i32 = let %3
        exit_if %3  # if_2
      }
      $B5: {  # false
        unreachable
      }
    }
    %5:i32 = let %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %98 20 %20
         %20 = OpLabel
         %21 = OpIAdd %i32 %one %two
               OpBranch %99
         %98 = OpLabel
         %22 = OpIAdd %i32 %two %two
               OpBranch %99
         %99 = OpLabel
         %23 = OpIAdd %i32 %three %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 2i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        %2:i32 = spirv.add<i32> 2i, 2i
        exit_switch  # switch_1
      }
      $B3: {  # case
        %3:i32 = spirv.add<i32> 1i, 2i
        exit_switch  # switch_1
      }
    }
    %4:i32 = spirv.add<i32> 3i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_DefaultIsMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %99 20 %20
         %20 = OpLabel
         %21 = OpIAdd %i32 %two %two
               OpBranch %99
         %99 = OpLabel
         %98 = OpIAdd %i32 %two %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 2i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        %2:i32 = spirv.add<i32> 2i, 2i
        exit_switch  # switch_1
      }
    }
    %3:i32 = spirv.add<i32> 2i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_DefaultIsCase) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %20 20 %20
         %20 = OpLabel
         %21 = OpIAdd %i32 %two %two
               OpBranch %99
         %99 = OpLabel
         %98 = OpIAdd %i32 %two %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 2i [c: (20i default, $B2)] {  # switch_1
      $B2: {  # case
        %2:i32 = spirv.add<i32> 2i, 2i
        exit_switch  # switch_1
      }
    }
    %3:i32 = spirv.add<i32> 2i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_MuliselectorCase) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %50 20 %20 30 %20
         %20 = OpLabel
         %21 = OpIAdd %i32 %two %two
               OpBranch %99
         %50 = OpLabel
               OpReturn
         %99 = OpLabel
         %98 = OpIAdd %i32 %two %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 2i [c: (default, $B2), c: (20i 30i, $B3)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        %2:i32 = spirv.add<i32> 2i, 2i
        exit_switch  # switch_1
      }
    }
    %3:i32 = spirv.add<i32> 2i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_OnlyDefault) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %20
         %20 = OpLabel
         %51 = OpIAdd %i32 %one %two
               OpBranch %99
         %99 = OpLabel
         %52 = OpIAdd %i32 %two %two
               OpReturn
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 2i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %2:i32 = spirv.add<i32> 1i, 2i
        exit_switch  # switch_1
      }
    }
    %3:i32 = spirv.add<i32> 2i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_IfBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
       %bool = OpTypeBool
        %one = OpConstant %u32 1
        %two = OpConstant %u32 2
      %three = OpConstant %u32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %50 20 %20 50 %50
         %99 = OpLabel
               OpReturn
         %20 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %true %99 %40 ; break-if
         %40 = OpLabel
               OpBranch %49
         %49 = OpLabel
        %101 = OpIAdd %u32 %two %three
               OpBranch %99
         %50 = OpLabel
               OpSelectionMerge %79 None
               OpBranchConditional %true %60 %99 ; break-unless
         %60 = OpLabel
               OpBranch %79
         %79 = OpLabel ; dominated by 60, so must follow 60
        %100 = OpIAdd %u32 %one %two
               OpBranch %99
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 2u [c: (50u default, $B2), c: (20u, $B3)] {  # switch_1
      $B2: {  # case
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_switch  # switch_1
          }
        }
        %2:u32 = spirv.add<u32> 1u, 2u
        exit_switch  # switch_1
      }
      $B3: {  # case
        if true [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            exit_switch  # switch_1
          }
          $B7: {  # false
            exit_if  # if_2
          }
        }
        %3:u32 = spirv.add<u32> 2u, 3u
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_Nest_If_In_Case) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
       %bool = OpTypeBool
        %one = OpConstant %u32 1
        %two = OpConstant %u32 2
      %three = OpConstant %u32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %50 20 %20 50 %50
         %99 = OpLabel
               OpReturn
         %20 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %true %30 %40
         %49 = OpLabel
               OpBranch %99
         %30 = OpLabel
               OpBranch %49
         %40 = OpLabel
               OpBranch %49
         %50 = OpLabel
               OpSelectionMerge %79 None
               OpBranchConditional %false %60 %70
         %79 = OpLabel
               OpBranch %99
         %60 = OpLabel
               OpBranch %79
         %70 = OpLabel
               OpBranch %79
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 2u [c: (50u default, $B2), c: (20u, $B3)] {  # switch_1
      $B2: {  # case
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        exit_switch  # switch_1
      }
      $B3: {  # case
        if true [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            exit_if  # if_2
          }
          $B7: {  # false
            exit_if  # if_2
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_HoistFromDefault) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
       %bool = OpTypeBool
        %one = OpConstant %u32 1
        %two = OpConstant %u32 2
      %three = OpConstant %u32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %20
         %20 = OpLabel
        %100 = OpCopyObject %u32 %one
               OpBranch %99
         %99 = OpLabel
        %102 = OpIAdd %u32 %100 %100
               OpReturn
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = switch 2u [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        %3:u32 = let 1u
        exit_switch %3  # switch_1
      }
    }
    %4:u32 = spirv.add<u32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_HoistFromCase) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
       %bool = OpTypeBool
        %one = OpConstant %u32 1
        %two = OpConstant %u32 2
      %three = OpConstant %u32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %50 20 %20
         %20 = OpLabel
        %100 = OpCopyObject %u32 %one
               OpBranch %99
         %50 = OpLabel
               OpReturn
         %99 = OpLabel
        %102 = OpIAdd %u32 %100 %100
               OpReturn
               OpFunctionEnd
  )",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = switch 2u [c: (default, $B2), c: (20u, $B3)] {  # switch_1
      $B2: {  # case
        ret
      }
      $B3: {  # case
        %3:u32 = let 1u
        exit_switch %3  # switch_1
      }
    }
    %4:u32 = spirv.add<u32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserDeathTest, Switch_Fallthrough) {
    auto src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %two %80 20 %20 30 %30
         %20 = OpLabel
         %50 = OpIAdd %i32 %one %two
               OpBranch %30 ; fall through
         %30 = OpLabel
         %51 = OpIAdd %i32 %two %two
               OpBranch %99
         %80 = OpLabel
         %52 = OpIAdd %i32 %three %two
               OpBranch %99
         %99 = OpLabel
         %53 = OpIAdd %i32 %one %three
               OpReturn
               OpFunctionEnd
)";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

TEST_F(SpirvParserTest, Switch_IfBreakInCase) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %one %50 20 %20 50 %50
         %99 = OpLabel
               OpReturn
         %20 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %true %99 %40 ; break-if
         %40 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpBranch %99
         %50 = OpLabel
               OpSelectionMerge %79 None
               OpBranchConditional %true %60 %99 ; break-unless
         %60 = OpLabel
               OpBranch %79
         %79 = OpLabel ; dominated by 60, so must follow 60
               OpBranch %99
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (50i default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
      $B3: {  # case
        if true [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            exit_switch  # switch_1
          }
          $B7: {  # false
            exit_if  # if_2
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_CaseCanBeMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %99
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (20i default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
          %1 = OpIAdd %i32 %one %two
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %20 %99
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 2i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Infinite_Branch) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %20 None
               OpBranch %20
         %99 = OpLabel
               OpReturn
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
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Infinite_BranchConditional) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %20 %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:bool = or true, true
        if %2 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BranchConditional) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %40 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranch %40
         %40 = OpLabel
               OpBranch %20 ; back edge
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Branch) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %40 None
               OpBranch %30
         %30 = OpLabel
               OpBranchConditional %true %40 %99
         %40 = OpLabel
               OpBranch %20
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BranchMergeOrContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %30 = OpIAdd %i32 %one %two
               OpBranch %20
         %20 = OpLabel
         %31 = OpIAdd %i32 %two %three
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %99 %20
         %99 = OpLabel
         %32 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 2i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 3i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %4:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_HeaderHasBreakIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99 ; like While
         %30 = OpLabel ; trivial body
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_HeaderHasBreakUnless) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %99 %30 ; has break-unless
         %30 = OpLabel ; trivial body
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            exit_loop  # loop_1
          }
          $B5: {  # false
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyHasBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranch %99 ; break
         %50 = OpLabel
               OpBranch %20
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
            exit_loop  # loop_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyHasBreakIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranchConditional %false %99 %40 ; break-if
         %40 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                exit_loop  # loop_1
              }
              $B7: {  # false
                continue  # -> $B3
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyHasBreakUnless) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranchConditional %false %40 %99 ; break-unless
         %40 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                exit_loop  # loop_1
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %false %40 %45 ; nested if
         %40 = OpLabel
               OpBranch %49
         %45 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                exit_if  # if_2
              }
              $B7: {  # false
                exit_if  # if_2
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyIfBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %false %40 %49 ; nested if
         %40 = OpLabel
               OpBranch %99   ; break from nested if
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                exit_loop  # loop_1
              }
              $B7: {  # false
                exit_if  # if_2
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyHasContinueIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranchConditional %false %50 %40 ; continue-if
         %40 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                continue  # -> $B3
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyHasContinueUnless) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranchConditional %false %40 %50 ; continue-unless
         %40 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                continue  # -> $B3
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Body_If_Continue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %false %40 %49 ; nested if
         %40 = OpLabel
               OpBranch %50   ; continue from nested if
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                exit_if  # if_2
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Body_Switch) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpSelectionMerge %49 None
               OpSwitch %one %49 40 %40 45 %45 ; fully nested switch
         %40 = OpLabel
               OpBranch %49
         %45 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            switch 1i [c: (default, $B6), c: (40i, $B7), c: (45i, $B8)] {  # switch_1
              $B6: {  # case
                exit_switch  # switch_1
              }
              $B7: {  # case
                exit_switch  # switch_1
              }
              $B8: {  # case
                exit_switch  # switch_1
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Body_Switch_CaseContinues) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpSelectionMerge %49 None
               OpSwitch %one %49 40 %40 45 %45
         %40 = OpLabel
               OpBranch %50   ; continue bypasses switch merge
         %45 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %20
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
            switch 1i [c: (default, $B6), c: (40i, $B7), c: (45i, $B8)] {  # switch_1
              $B6: {  # case
                exit_switch  # switch_1
              }
              $B7: {  # case
                continue  # -> $B3
              }
              $B8: {  # case
                exit_switch  # switch_1
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Continue_Sequence) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %60
         %60 = OpLabel
               OpBranch %20
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Continue_ContainsIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpSelectionMerge %89 None
               OpBranchConditional %true %60 %70
         %89 = OpLabel
               OpBranch %20 ; backedge
         %60 = OpLabel
               OpBranch %89
         %70 = OpLabel
               OpBranch %89
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        if true [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            exit_if  # if_2
          }
          $B7: {  # false
            exit_if  # if_2
          }
        }
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Continue_HasBreakIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranchConditional %false %99 %20
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if false  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Continue_HasBreakUnless) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranchConditional %false %20 %99
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %2:bool = not false
        break_if %2  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Loop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpLoopMerge %49 %40 None
               OpBranchConditional %false %35 %49
         %35 = OpLabel
               OpBranch %37
         %37 = OpLabel
               OpBranch %40
         %40 = OpLabel ; inner loop's continue
               OpBranch %30 ; backedge
         %49 = OpLabel ; inner loop's merge
               OpBranch %50
         %50 = OpLabel ; outer loop's continue
               OpBranch %20 ; outer loop's backege
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
            loop [b: $B6, c: $B7] {  # loop_2
              $B6: {  # body
                if false [t: $B8, f: $B9] {  # if_2
                  $B8: {  # true
                    continue  # -> $B7
                  }
                  $B9: {  # false
                    exit_loop  # loop_2
                  }
                }
                continue  # -> $B7
              }
              $B7: {  # continuing
                next_iteration  # -> $B6
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Loop_InnerBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpLoopMerge %49 %40 None
               OpBranchConditional %false %35 %49
         %35 = OpLabel
               OpBranchConditional %true %49 %37 ; break to inner merge
         %37 = OpLabel
               OpBranch %40
         %40 = OpLabel ; inner loop's continue
               OpBranch %30 ; backedge
         %49 = OpLabel ; inner loop's merge
               OpBranch %50
         %50 = OpLabel ; outer loop's continue
               OpBranch %20 ; outer loop's backege
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
            loop [b: $B6, c: $B7] {  # loop_2
              $B6: {  # body
                if false [t: $B8, f: $B9] {  # if_2
                  $B8: {  # true
                    if true [t: $B10, f: $B11] {  # if_3
                      $B10: {  # true
                        exit_loop  # loop_2
                      }
                      $B11: {  # false
                        continue  # -> $B7
                      }
                    }
                    exit_if  # if_2
                  }
                  $B9: {  # false
                    exit_loop  # loop_2
                  }
                }
                continue  # -> $B7
              }
              $B7: {  # continuing
                next_iteration  # -> $B6
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Loop_InnerContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpLoopMerge %49 %40 None
               OpBranchConditional %false %35 %49
         %35 = OpLabel
               OpBranchConditional %true %37 %49 ; continue to inner continue target
         %37 = OpLabel
               OpBranch %40
         %40 = OpLabel ; inner loop's continue
               OpBranch %30 ; backedge
         %49 = OpLabel ; inner loop's merge
               OpBranch %50
         %50 = OpLabel ; outer loop's continue
               OpBranch %20 ; outer loop's backege
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
            loop [b: $B6, c: $B7] {  # loop_2
              $B6: {  # body
                if false [t: $B8, f: $B9] {  # if_2
                  $B8: {  # true
                    if true [t: $B10, f: $B11] {  # if_3
                      $B10: {  # true
                        continue  # -> $B7
                      }
                      $B11: {  # false
                        exit_loop  # loop_2
                      }
                    }
                    exit_if  # if_2
                  }
                  $B9: {  # false
                    exit_loop  # loop_2
                  }
                }
                continue  # -> $B7
              }
              $B7: {  # continuing
                next_iteration  # -> $B6
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Loop_InnerContinueBreaks) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpLoopMerge %49 %40 None
               OpBranchConditional %false %35 %49
         %35 = OpLabel
               OpBranch %37
         %37 = OpLabel
               OpBranch %40
         %40 = OpLabel ; inner loop's continue
               OpBranchConditional %true %30 %49 ; backedge and inner break
         %49 = OpLabel ; inner loop's merge
               OpBranch %50
         %50 = OpLabel ; outer loop's continue
               OpBranch %20 ; outer loop's backege
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
            loop [b: $B6, c: $B7] {  # loop_2
              $B6: {  # body
                if false [t: $B8, f: $B9] {  # if_2
                  $B8: {  # true
                    continue  # -> $B7
                  }
                  $B9: {  # false
                    exit_loop  # loop_2
                  }
                }
                continue  # -> $B7
              }
              $B7: {  # continuing
                %2:bool = not true
                break_if %2  # -> [t: exit_loop loop_2, f: $B6]
              }
            }
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_MergeBlockIsLoop) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %20 %50
         %20 = OpLabel
               OpBranch %50
         ; %50 is the merge block for the selection starting at 10,
         ; and its own continue target.
         %50 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %50 %99
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    loop [b: $B4, c: $B5] {  # loop_1
      $B4: {  # body
        if true [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            continue  # -> $B5
          }
          $B7: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B5
      }
      $B5: {  # continuing
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, MergeIsAlsoMultiBlockLoopHeader) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %20 %50
         %20 = OpLabel
               OpBranch %50
         ; %50 is the merge block for the selection starting at 10,
         ; and a loop block header but not its own continue target.
         %50 = OpLabel
               OpLoopMerge %99 %60 None
               OpBranchConditional %true %60 %99
         %60 = OpLabel
               OpBranch %50
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    loop [b: $B4, c: $B5] {  # loop_1
      $B4: {  # body
        if true [t: $B6, f: $B7] {  # if_2
          $B6: {  # true
            continue  # -> $B5
          }
          $B7: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B5
      }
      $B5: {  # continuing
        next_iteration  # -> $B4
      }
    }
    ret
  }
}
)");
}

// Exercises the hard case where we a single OpBranchConditional has both
// IfBreak and Forward edges, within the true-branch clause.
TEST_F(SpirvParserTest, IfBreak_FromThen_ForwardWithinThen) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %two
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %50
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %three
               OpBranchConditional %false %99 %30 ; kIfBreak with kForward
         %30 = OpLabel ; still in then clause
         %36 = OpIAdd %i32 %one %three
               OpBranch %99
         %50 = OpLabel ; else clause
         %37 = OpIAdd %i32 %three %one
               OpBranch %99
         %99 = OpLabel
         %38 = OpIAdd %i32 %three %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 2i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = spirv.add<i32> 2i, 3i
        if false [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            exit_if  # if_2
          }
          $B5: {  # false
            %4:i32 = spirv.add<i32> 1i, 3i
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
      $B3: {  # false
        %5:i32 = spirv.add<i32> 3i, 1i
        exit_if  # if_1
      }
    }
    %6:i32 = spirv.add<i32> 3i, 2i
    ret
  }
}
)");
}

// Exercises the hard case where we a single OpBranchConditional has both
// IfBreak and Forward edges, within the false-branch clause.
TEST_F(SpirvParserTest, IfBreak_FromElse_ForwardWithinElse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %35 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %50
         %20 = OpLabel
         %36 = OpIAdd %i32 %two %two
               OpBranch %99
         %50 = OpLabel ; else clause
         %37 = OpIAdd %i32 %three %three
               OpBranchConditional %false %99 %80 ; kIfBreak with kForward
         %80 = OpLabel ; still in then clause
         %38 = OpIAdd %i32 %one %two
               OpBranch %99
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = spirv.add<i32> 2i, 2i
        exit_if  # if_1
      }
      $B3: {  # false
        %4:i32 = spirv.add<i32> 3i, 3i
        if false [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            exit_if  # if_2
          }
          $B5: {  # false
            %5:i32 = spirv.add<i32> 1i, 2i
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
    }
    %6:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

// This is a combination of the previous two, but also adding a premerge.
// We have IfBreak and Forward edges from the same OpBranchConditional, and
// this occurs in the true-branch clause, the false-branch clause, and within
// the premerge clause.  Flow guards have to be sprinkled in lots of places.
TEST_F(SpirvParserTest, IfBreak_FromThenAndElseWithForward_Premerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %35 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               OpBranchConditional %true %20 %50
         %20 = OpLabel ; then
         %36 = OpIAdd %i32 %two %two
               OpBranchConditional %false %21 %99 ; kForward and kIfBreak
         %21 = OpLabel ; still in then clause
         %37 = OpIAdd %i32 %three %three
               OpBranch %80 ; to premerge
         %50 = OpLabel ; else clause
         %38 = OpIAdd %i32 %one %two
               OpBranchConditional %false %99 %51 ; kIfBreak with kForward
         %51 = OpLabel ; still in else clause
         %39 = OpIAdd %i32 %two %three
               OpBranch %80 ; to premerge
         %80 = OpLabel ; premerge
         %41 = OpIAdd %i32 %three %two
               OpBranchConditional %true %81 %99
         %81 = OpLabel ; premerge
         %42 = OpIAdd %i32 %three %one
               OpBranch %99
         %99 = OpLabel
         %43 = OpIAdd %i32 %two %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    %execute_premerge:ptr<function, bool, read_write> = var true
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = spirv.add<i32> 2i, 2i
        if false [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            %5:i32 = spirv.add<i32> 3i, 3i
            exit_if  # if_2
          }
          $B5: {  # false
            store %execute_premerge, false
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
      $B3: {  # false
        %6:i32 = spirv.add<i32> 1i, 2i
        if false [t: $B6, f: $B7] {  # if_3
          $B6: {  # true
            store %execute_premerge, false
            exit_if  # if_3
          }
          $B7: {  # false
            %7:i32 = spirv.add<i32> 2i, 3i
            exit_if  # if_3
          }
        }
        exit_if  # if_1
      }
    }
    %8:bool = load %execute_premerge
    if %8 [t: $B8, f: $B9] {  # if_4
      $B8: {  # true
        %9:i32 = spirv.add<i32> 3i, 2i
        if true [t: $B10, f: $B11] {  # if_5
          $B10: {  # true
            %10:i32 = spirv.add<i32> 3i, 1i
            exit_if  # if_5
          }
          $B11: {  # false
            exit_if  # if_5
          }
        }
        exit_if  # if_4
      }
      $B9: {  # false
        unreachable
      }
    }
    %11:i32 = spirv.add<i32> 2i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_SingleBlock_BothBackedge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %35 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %36 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %20 %20
         %99 = OpLabel
         %37 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:bool = or true, true
        if %4 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %5:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_SingleBlock_UnconditionalBackege) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %35 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %36 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %20 None
               OpBranch %20
         %99 = OpLabel
         %37 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %4:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Unconditional_Body_SingleBlockContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %35 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %36 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %50 None
               OpBranch %30
         %30 = OpLabel
         %37 = OpIAdd %i32 %three %three
               OpBranch %50
         %50 = OpLabel
         %38 = OpIAdd %i32 %one %two
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:i32 = spirv.add<i32> 1i, 2i
        next_iteration  # -> $B2
      }
    }
    %6:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Unconditional_Body_MultiBlockContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %50 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranch %50
         %50 = OpLabel
         %37 = OpIAdd %i32 %one %three
               OpBranch %60
         %60 = OpLabel
         %38 = OpIAdd %i32 %one %two
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:i32 = spirv.add<i32> 1i, 3i
        %6:i32 = spirv.add<i32> 1i, 2i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_Unconditional_Body_ContinueNestIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %50 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranch %50
         %50 = OpLabel ; continue target; also if-header
         %37 = OpIAdd %i32 %one %two
               OpSelectionMerge %80 None
               OpBranchConditional %false %60 %80
         %60 = OpLabel
         %38 = OpIAdd %i32 %two %three
               OpBranch %80
         %80 = OpLabel
         %39 = OpIAdd %i32 %one %three
               OpBranch %20
         %99 = OpLabel
         %41 = OpIAdd %i32 %three %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:i32 = spirv.add<i32> 1i, 2i
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %6:i32 = spirv.add<i32> 2i, 3i
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %7:i32 = spirv.add<i32> 1i, 3i
        next_iteration  # -> $B2
      }
    }
    %8:i32 = spirv.add<i32> 3i, 2i
    ret
  }
}
)");
}

// Test case where both branches exit. e.g both go to merge.
TEST_F(SpirvParserTest, Loop_Never) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %99 %99
         %80 = OpLabel ; continue target
         %35 = OpIAdd %i32 %two %two
               OpBranch %20
         %99 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        %3:bool = or true, true
        if %3 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = spirv.add<i32> 2i, 2i
        next_iteration  # -> $B2
      }
    }
    %5:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_TrueToBody_FalseBreaks) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %36 = OpIAdd %i32 %three %three
               OpBranch %20
         %99 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %3:i32 = spirv.add<i32> 2i, 2i
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    %5:i32 = spirv.add<i32> 1i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_FalseToBody_TrueBreaks) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %36 = OpIAdd %i32 %three %three
               OpBranch %20
         %99 = OpLabel
         %37 = OpIAdd %i32 %one %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %3:i32 = spirv.add<i32> 2i, 2i
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    %5:i32 = spirv.add<i32> 1i, 3i
    ret
  }
}
)");
}

// By construction, it has to come from nested code.
TEST_F(SpirvParserTest, Loop_NestedIfContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %40 %50
         %40 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %80 ; continue edge
         %50 = OpLabel ; inner selection merge
         %35 = OpIAdd %i32 %two %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %36 = OpIAdd %i32 %three %three
               OpBranch %20
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
            %2:i32 = spirv.add<i32> 1i, 1i
            continue  # -> $B3
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %3:i32 = spirv.add<i32> 2i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyAlwaysBreaks) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %99 ; break is here
         %80 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %20 ; backedge
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        %3:i32 = spirv.add<i32> 2i, 2i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

// The else-branch has a continue but it's skipped because it's from a
// block that immediately precedes the continue construct.
TEST_F(SpirvParserTest, Loop_BodyConditionallyBreaks_FromTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranchConditional %true %99 %80
         %80 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %20 ; backedge
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %3:i32 = spirv.add<i32> 2i, 2i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

// The else-branch has a continue but it's skipped because it's from a
// block that immediately precedes the continue construct.
TEST_F(SpirvParserTest, Loop_BodyConditionallyBreaks_FromFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranchConditional %true %80 %99
         %80 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %20 ; backedge
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %3:i32 = spirv.add<i32> 2i, 2i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyConditionallyBreaks_FromTrue_Early) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranchConditional %true %99 %70
         %70 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %80
         %80 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranch %20 ; backedge
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            %3:i32 = spirv.add<i32> 2i, 2i
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Loop_BodyConditionallyBreaks_FromFalse_Early) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranchConditional %true %70 %99
         %70 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %80
         %80 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranch %20 ; backedge
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %3:i32 = spirv.add<i32> 2i, 2i
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_Case_SintValue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               ; SPIR-V assembler doesn't support negative literals in switch
               OpSwitch %one %99 20 %20 2000000000 %30 !4000000000 %40
         %20 = OpLabel
         %35 = OpIAdd %i32 %one %one
               OpBranch %99
         %30 = OpLabel
         %36 = OpIAdd %i32 %two %two
               OpBranch %99
         %40 = OpLabel
         %37 = OpIAdd %i32 %three %three
               OpBranch %99
         %99 = OpLabel
         %38 = OpIAdd %i32 %one %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    switch 1i [c: (default, $B2), c: (20i, $B3), c: (2000000000i, $B4), c: (-294967296i, $B5)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        %3:i32 = spirv.add<i32> 1i, 1i
        exit_switch  # switch_1
      }
      $B4: {  # case
        %4:i32 = spirv.add<i32> 2i, 2i
        exit_switch  # switch_1
      }
      $B5: {  # case
        %5:i32 = spirv.add<i32> 3i, 3i
        exit_switch  # switch_1
      }
    }
    %6:i32 = spirv.add<i32> 1i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Switch_Case_UintValue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
       %bool = OpTypeBool
        %one = OpConstant %u32 1
        %two = OpConstant %u32 2
      %three = OpConstant %u32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %u32 %one %one
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %20 2000000000 %30 50 %40
         %20 = OpLabel
         %35 = OpIAdd %u32 %two %two
               OpBranch %99
         %30 = OpLabel
         %36 = OpIAdd %u32 %three %three
               OpBranch %99
         %40 = OpLabel
         %37 = OpIAdd %u32 %one %two
               OpBranch %99
         %99 = OpLabel
         %38 = OpIAdd %u32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.add<u32> 1u, 1u
    switch 1u [c: (default, $B2), c: (20u, $B3), c: (2000000000u, $B4), c: (50u, $B5)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        %3:u32 = spirv.add<u32> 2u, 2u
        exit_switch  # switch_1
      }
      $B4: {  # case
        %4:u32 = spirv.add<u32> 3u, 3u
        exit_switch  # switch_1
      }
      $B5: {  # case
        %5:u32 = spirv.add<u32> 1u, 2u
        exit_switch  # switch_1
      }
    }
    %6:u32 = spirv.add<u32> 2u, 3u
    ret
  }
}
)");
}

// When the break is not last in its case, we must emit a 'break'
TEST_F(SpirvParserTest, Branch_SwitchBreak_NotLastInCase) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpSelectionMerge %50 None
               OpBranchConditional %true %40 %50
         %40 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranch %99 ; branch to merge. Not last in case
         %50 = OpLabel ; inner merge
         %37 = OpIAdd %i32 %one %two
               OpBranch %99
         %99 = OpLabel
         %38 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    switch 1i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        %3:i32 = spirv.add<i32> 2i, 2i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %4:i32 = spirv.add<i32> 3i, 3i
            exit_switch  # switch_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %5:i32 = spirv.add<i32> 1i, 2i
        exit_switch  # switch_1
      }
    }
    %6:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");

    //     auto* expect = R"(var_1 = 1u;
    // switch(42u) {
    //   case 20u: {
    //     var_1 = 20u;
    //     if (false) {
    //       var_1 = 40u;
    //       break;
    //     }
    //     var_1 = 50u;
    //   }
    //   default: {
    //   }
    // }
    // var_1 = 7u;
    // return;
}

TEST_F(SpirvParserTest, Branch_LoopBreak_MultiBlockLoop_FromBody) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %99 ; break is here
         %80 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranch %20 ; backedge
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        %3:i32 = spirv.add<i32> 2i, 2i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest,
       Branch_LoopBreak_MultiBlockLoop_FromContinueConstructEnd_Conditional_BreakIf) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %80
         %80 = OpLabel ; continue target
         %34 = OpIAdd %i32 %one %one
               OpBranchConditional %true %99 %20  ; exit, and backedge
         %99 = OpLabel
               OpReturn
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
        %2:i32 = spirv.add<i32> 1i, 1i
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Branch_LoopBreak_MultiBlockLoop_FromContinueConstructEnd_Conditional) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %80
         %80 = OpLabel ; continue target
         %34 = OpIAdd %i32 %one %one
               OpBranchConditional %true %20 %99  ; backedge, and exit
         %99 = OpLabel
               OpReturn
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
        %2:i32 = spirv.add<i32> 1i, 1i
        %3:bool = not true
        break_if %3  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Branch_LoopBreak_FromContinueConstructTail) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %50 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %60
         %60 = OpLabel
               OpBranchConditional %true %20 %99
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %2:bool = not true
        break_if %2  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Branch_LoopContinue_LastInLoopConstruct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %80 ; continue edge from last block before continue target
         %80 = OpLabel ; continue target
         %35 = OpIAdd %i32 %two %two
               OpBranch %20
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %2:i32 = spirv.add<i32> 1i, 1i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %3:i32 = spirv.add<i32> 2i, 2i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

// By construction, it has to come from nested code.
TEST_F(SpirvParserTest, Branch_LoopContinue_BeforeLast) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %40 %50
         %40 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %80 ; continue edge
         %50 = OpLabel ; inner selection merge
         %35 = OpIAdd %i32 %two %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %36 = OpIAdd %i32 %three %three
               OpBranch %20
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
            %2:i32 = spirv.add<i32> 1i, 1i
            continue  # -> $B3
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %3:i32 = spirv.add<i32> 2i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Branch_LoopContinue_FromSwitch) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %79 None
               OpSwitch %one %79 40 %40
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranch %80 ; continue edge
         %79 = OpLabel ; switch merge
         %38 = OpIAdd %i32 %three %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %39 = OpIAdd %i32 %two %three
               OpBranch %20
         %99 = OpLabel
         %41 = OpIAdd %i32 %three %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        switch 1i [c: (default, $B4), c: (40i, $B5)] {  # switch_1
          $B4: {  # case
            exit_switch  # switch_1
          }
          $B5: {  # case
            %5:i32 = spirv.add<i32> 1i, 2i
            continue  # -> $B3
          }
        }
        %6:i32 = spirv.add<i32> 3i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %7:i32 = spirv.add<i32> 2i, 3i
        next_iteration  # -> $B2
      }
    }
    %8:i32 = spirv.add<i32> 3i, 1i
    ret
  }
}
)");
}

// When unconditional, the if-break must be last in the then clause.
TEST_F(SpirvParserTest, Branch_IfBreak_FromThen) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %true %30 %99
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %99
         %99 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %2:i32 = spirv.add<i32> 1i, 1i
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    %3:i32 = spirv.add<i32> 2i, 2i
    ret
  }
}
)");
}

// When unconditional, the if-break must be last in the else clause.
TEST_F(SpirvParserTest, Branch_IfBreak_FromElse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %true %99 %30
         %30 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %99
         %99 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        %2:i32 = spirv.add<i32> 1i, 1i
        exit_if  # if_1
      }
    }
    %3:i32 = spirv.add<i32> 2i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Back_SingleBlock_LoopBreak_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %99 %20
         %99 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %4:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Back_SingleBlock_LoopBreak_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %20 None
               OpBranchConditional %true %20 %99
         %99 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %4:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Back_MultiBlock_LoopBreak_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %80
         %80 = OpLabel
               OpBranchConditional %true %99 %20
         %99 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    %4:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Back_MultiBlock_LoopBreak_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %80
         %80 = OpLabel
               OpBranchConditional %true %20 %99
         %99 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %4:bool = not true
        break_if %4  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    %5:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

// When the break is not last in its case, we must emit a 'break'
TEST_F(SpirvParserTest, BranchConditional_SwitchBreak_SwitchBreak_NotLastInCase) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpSelectionMerge %50 None
               OpBranchConditional %true %40 %50
         %40 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranchConditional %false %99 %99 ; branch to merge. Not last in case
         %50 = OpLabel ; inner merge
         %37 = OpIAdd %i32 %one %two
               OpBranch %99
         %99 = OpLabel
         %38 = OpIAdd %i32 %one %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    switch 1i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        %3:i32 = spirv.add<i32> 2i, 2i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %4:i32 = spirv.add<i32> 3i, 3i
            %5:bool = or false, true
            if %5 [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                exit_switch  # switch_1
              }
              $B7: {  # false
                unreachable
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %6:i32 = spirv.add<i32> 1i, 2i
        exit_switch  # switch_1
      }
    }
    %7:i32 = spirv.add<i32> 1i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_SwitchBreak_Continue_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %79 None
               OpSwitch %one %79 40 %40
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranchConditional %true %80 %79 ; break; continue on true
         %79 = OpLabel
         %38 = OpIAdd %i32 %one %three
               OpBranch %80
         %80 = OpLabel ; continue target
         %39 = OpIAdd %i32 %two %three
               OpBranch %20
         %99 = OpLabel ; loop merge
         %41 = OpIAdd %i32 %three %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        switch 1i [c: (default, $B4), c: (40i, $B5)] {  # switch_1
          $B4: {  # case
            exit_switch  # switch_1
          }
          $B5: {  # case
            %5:i32 = spirv.add<i32> 1i, 2i
            if true [t: $B6, f: $B7] {  # if_1
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                exit_switch  # switch_1
              }
            }
            exit_switch  # switch_1
          }
        }
        %6:i32 = spirv.add<i32> 1i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %7:i32 = spirv.add<i32> 2i, 3i
        next_iteration  # -> $B2
      }
    }
    %8:i32 = spirv.add<i32> 3i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_SwitchBreak_Continue_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %79 None
               OpSwitch %one %79 40 %40
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranchConditional %true %79 %80 ; break; continue on false
         %79 = OpLabel
         %38 = OpIAdd %i32 %one %three
               OpBranch %80
         %80 = OpLabel ; continue target
         %39 = OpIAdd %i32 %two %three
               OpBranch %20
         %99 = OpLabel ; loop merge
         %41 = OpIAdd %i32 %three %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        switch 1i [c: (default, $B4), c: (40i, $B5)] {  # switch_1
          $B4: {  # case
            exit_switch  # switch_1
          }
          $B5: {  # case
            %5:i32 = spirv.add<i32> 1i, 2i
            if true [t: $B6, f: $B7] {  # if_1
              $B6: {  # true
                exit_switch  # switch_1
              }
              $B7: {  # false
                continue  # -> $B3
              }
            }
            exit_switch  # switch_1
          }
        }
        %6:i32 = spirv.add<i32> 1i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %7:i32 = spirv.add<i32> 2i, 3i
        next_iteration  # -> $B2
      }
    }
    %8:i32 = spirv.add<i32> 3i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_SwitchBreak_Forward_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranchConditional %true %30 %99 ; break; forward on true
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranch %99
         %99 = OpLabel ; switch merge
         %37 = OpIAdd %i32 %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    switch 1i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        %3:i32 = spirv.add<i32> 2i, 2i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %4:i32 = spirv.add<i32> 3i, 3i
            exit_switch  # switch_1
          }
          $B5: {  # false
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    %5:i32 = spirv.add<i32> 1i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_SwitchBreak_Forward_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpBranchConditional %true %99 %30 ; break; forward on false
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranch %99
         %99 = OpLabel ; switch merge
         %37 = OpIAdd %i32 %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    switch 1i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        %3:i32 = spirv.add<i32> 2i, 2i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_switch  # switch_1
          }
          $B5: {  # false
            %4:i32 = spirv.add<i32> 3i, 3i
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    %5:i32 = spirv.add<i32> 1i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_LoopBreak_SingleBlock_LoopBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %99 %99
         %80 = OpLabel ; continue target
         %36 = OpIAdd %i32 %three %three
               OpBranch %20
         %99 = OpLabel
         %37 = OpIAdd %i32 %one %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:bool = or true, true
        if %4 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    %6:i32 = spirv.add<i32> 1i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_LoopBreak_MultiBlock_LoopBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranchConditional %true %99 %99
         %80 = OpLabel ; continue target
         %37 = OpIAdd %i32 %one %two
               OpBranch %20
         %99 = OpLabel
         %38 = OpIAdd %i32 %one %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        %5:bool = or true, true
        if %5 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 2i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 1i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_LoopBreak_Continue_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %25
         ; Need this extra selection to make another block between
         ; %30 and the continue target, so we actually induce a Continue
         ; statement to exist.
         %25 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %false %30 %40
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               ; break; continue on true
               OpBranchConditional %false %80 %99
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %38 = OpIAdd %i32 %one %three
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %4:i32 = spirv.add<i32> 3i, 3i
            if false [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                exit_loop  # loop_1
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %5:i32 = spirv.add<i32> 1i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 3i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_LoopBreak_Continue_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %25
         ; Need this extra selection to make another block between
         ; %30 and the continue target, so we actually induce a Continue
         ; statement to exist.
         %25 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %false %30 %40
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               ; break; continue on false
               OpBranchConditional %true %99 %80
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %38 = OpIAdd %i32 %one %three
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %4:i32 = spirv.add<i32> 3i, 3i
            if true [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                exit_loop  # loop_1
              }
              $B7: {  # false
                continue  # -> $B3
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %5:i32 = spirv.add<i32> 1i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 3i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_LoopBreak_Forward_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               ; break; forward on true
               OpBranchConditional %true %40 %99
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %38 = OpIAdd %i32 %one %three
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %5:i32 = spirv.add<i32> 1i, 2i
            continue  # -> $B3
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 3i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_LoopBreak_Forward_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               ; break; forward on false
               OpBranchConditional %true %99 %40
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %38 = OpIAdd %i32 %one %three
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
          $B5: {  # false
            %5:i32 = spirv.add<i32> 1i, 2i
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 3i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Continue_Continue_FromHeader) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranchConditional %true %80 %80 ; to continue
         %80 = OpLabel ; continue target
         %36 = OpIAdd %i32 %three %three
               OpBranch %20
         %99 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:bool = or true, true
        if %4 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %5:i32 = spirv.add<i32> 3i, 3i
        next_iteration  # -> $B2
      }
    }
    %6:i32 = spirv.add<i32> 1i, 2i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Continue_Continue_AfterHeader_Unconditional) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpBranchConditional %true %80 %80 ; to continue
         %80 = OpLabel ; continue target
         %37 = OpIAdd %i32 %one %two
               OpBranch %20
         %99 = OpLabel
         %38 = OpIAdd %i32 %one %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        %5:bool = or true, true
        if %5 [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            unreachable
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 2i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 1i, 3i
    ret
  }
}
)");
}

// Create an intervening block so we actually require a "continue" statement
TEST_F(SpirvParserTest, BranchConditional_Continue_Continue_AfterHeader_Conditional) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %50 None
               OpBranchConditional %false %40 %50
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranchConditional %true %80 %80 ; to continue
         %50 = OpLabel ; merge for selection
         %38 = OpIAdd %i32 %one %three
               OpBranch %80
         %80 = OpLabel ; continue target
         %39 = OpIAdd %i32 %two %three
               OpBranch %20
         %99 = OpLabel
         %41 = OpIAdd %i32 %three %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %5:i32 = spirv.add<i32> 1i, 2i
            %6:bool = or true, true
            if %6 [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                unreachable
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %7:i32 = spirv.add<i32> 1i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %8:i32 = spirv.add<i32> 2i, 3i
        next_iteration  # -> $B2
      }
    }
    %9:i32 = spirv.add<i32> 3i, 1i
    ret
  }
}
)");
}

// Like the previous tests, but with an empty continuing clause.
TEST_F(SpirvParserTest,
       BranchConditional_Continue_Continue_AfterHeader_Conditional_EmptyContinuing) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %50 None
               OpBranchConditional %false %40 %50
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranchConditional %false %80 %80 ; to continue
         %50 = OpLabel ; merge for selection
         %38 = OpIAdd %i32 %one %three
               OpBranch %80
         %80 = OpLabel ; continue target
               ; no statements here.
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %5:i32 = spirv.add<i32> 1i, 2i
            %6:bool = or false, true
            if %6 [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                unreachable
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %7:i32 = spirv.add<i32> 1i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    %8:i32 = spirv.add<i32> 2i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_LoopContinue_FromSwitch) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %79 None
               OpSwitch %one %79 40 %40
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranchConditional %false %80 %80; dup continue edge
         %79 = OpLabel ; switch merge
         %38 = OpIAdd %i32 %one %three
               OpBranch %80
         %80 = OpLabel ; continue target
         %39 = OpIAdd %i32 %two %three
               OpBranch %20
         %99 = OpLabel
         %41 = OpIAdd %i32 %three %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        switch 1i [c: (default, $B4), c: (40i, $B5)] {  # switch_1
          $B4: {  # case
            exit_switch  # switch_1
          }
          $B5: {  # case
            %5:i32 = spirv.add<i32> 1i, 2i
            %6:bool = or false, true
            if %6 [t: $B6, f: $B7] {  # if_1
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                unreachable
              }
            }
            exit_switch  # switch_1
          }
        }
        %7:i32 = spirv.add<i32> 1i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %8:i32 = spirv.add<i32> 2i, 3i
        next_iteration  # -> $B2
      }
    }
    %9:i32 = spirv.add<i32> 3i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Continue_IfBreak_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %50 None
               OpBranchConditional %false %40 %50
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               ; true to if's merge;  false to continue
               OpBranchConditional %true %50 %80
         %50 = OpLabel ; merge for selection
         %38 = OpIAdd %i32 %one %three
               OpBranch %80
         %80 = OpLabel ; continue target
         %39 = OpIAdd %i32 %two %one
               OpBranch %20
         %99 = OpLabel
         %41 = OpIAdd %i32 %three %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %5:i32 = spirv.add<i32> 1i, 2i
            if true [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                exit_if  # if_2
              }
              $B7: {  # false
                continue  # -> $B3
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %6:i32 = spirv.add<i32> 1i, 3i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %7:i32 = spirv.add<i32> 2i, 1i
        next_iteration  # -> $B2
      }
    }
    %8:i32 = spirv.add<i32> 3i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Continue_IfBreak_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpSelectionMerge %50 None
               OpBranchConditional %false %40 %50
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               ; false to if's merge;  true to continue
               OpBranchConditional %true %80 %50
         %50 = OpLabel ; merge for selection
         %38 = OpIAdd %i32 %one %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %39 = OpIAdd %i32 %two %three
               OpBranch %20
         %99 = OpLabel
         %41 = OpIAdd %i32 %three %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if false [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %5:i32 = spirv.add<i32> 1i, 2i
            if true [t: $B6, f: $B7] {  # if_2
              $B6: {  # true
                continue  # -> $B3
              }
              $B7: {  # false
                exit_if  # if_2
              }
            }
            exit_if  # if_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        %6:i32 = spirv.add<i32> 1i, 2i
        continue  # -> $B3
      }
      $B3: {  # continuing
        %7:i32 = spirv.add<i32> 2i, 3i
        next_iteration  # -> $B2
      }
    }
    %8:i32 = spirv.add<i32> 3i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Continue_Forward_OnTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               ; continue; forward on true
               OpBranchConditional %true %40 %80
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %38 = OpIAdd %i32 %one %three
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            %5:i32 = spirv.add<i32> 1i, 2i
            continue  # -> $B3
          }
          $B5: {  # false
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 3i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 2i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Continue_Forward_OnFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranch %20
         %20 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpLoopMerge %99 %80 None
               OpBranch %30
         %30 = OpLabel
         %36 = OpIAdd %i32 %three %three
               ; continue; forward on true
               OpBranchConditional %true %80 %40
         %40 = OpLabel
         %37 = OpIAdd %i32 %one %two
               OpBranch %80
         %80 = OpLabel ; continue target
         %38 = OpIAdd %i32 %one %three
               OpBranch %20
         %99 = OpLabel
         %39 = OpIAdd %i32 %two %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        %3:i32 = spirv.add<i32> 2i, 2i
        %4:i32 = spirv.add<i32> 3i, 3i
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
          $B5: {  # false
            %5:i32 = spirv.add<i32> 1i, 2i
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        %6:i32 = spirv.add<i32> 1i, 3i
        next_iteration  # -> $B2
      }
    }
    %7:i32 = spirv.add<i32> 2i, 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_IfBreak_IfBreak_Same) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpSelectionMerge %99 None
               OpBranchConditional %true %99 %99
         %20 = OpLabel ; dead
         %35 = OpIAdd %i32 %two %two
               OpBranch %99
         %99 = OpLabel
         %36 = OpIAdd %i32 %three %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    %3:bool = or true, true
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        unreachable
      }
    }
    %4:i32 = spirv.add<i32> 3i, 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Forward_Forward_Same) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %34 = OpIAdd %i32 %one %one
               OpBranchConditional %true %99 %99; forward
         %99 = OpLabel
         %35 = OpIAdd %i32 %two %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.add<i32> 1i, 1i
    %3:bool = or true, true
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = spirv.add<i32> 2i, 2i
        ret
      }
      $B3: {  # false
        unreachable
      }
    }
    unreachable
  }
}
)");
}

// crbug.com/tint/243
TEST_F(SpirvParserTest, IfSelection_TrueBranch_LoopBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %99 %90 None
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %true %99 %30 ; true branch breaking is ok
         %30 = OpLabel
               OpBranch %40
         %40 = OpLabel ; selection merge
               OpBranch %90
         %90 = OpLabel ; continue target
               OpBranch %10 ; backedge
         %99 = OpLabel ; loop merge
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
            exit_loop  # loop_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

// crbug.com/tint/243
TEST_F(SpirvParserTest, TrueBranch_LoopContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %99 %90 None
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %true %90 %30 ; true branch continue is ok
         %30 = OpLabel
               OpBranch %40
         %40 = OpLabel ; selection merge
               OpBranch %90
         %90 = OpLabel ; continue target
               OpBranch %10 ; backedge
         %99 = OpLabel ; loop merge
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
            continue  # -> $B3
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

// crbug.com/tint/243
TEST_F(SpirvParserTest, TrueBranch_SwitchBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %20
         %20 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %true %99 %30 ; true branch switch break is ok
         %30 = OpLabel
               OpBranch %40
         %40 = OpLabel ; if-selection merge
               OpBranch %99
         %99 = OpLabel ; switch merge
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_switch  # switch_1
          }
          $B5: {  # false
            exit_if  # if_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

// crbug.com/tint/243
TEST_F(SpirvParserTest, FalseBranch_LoopBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %99 %90 None
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %true %30 %99 ; false branch breaking is ok
         %30 = OpLabel
               OpBranch %40
         %40 = OpLabel ; selection merge
               OpBranch %90
         %90 = OpLabel ; continue target
               OpBranch %10 ; backedge
         %99 = OpLabel ; loop merge
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
            exit_if  # if_1
          }
          $B5: {  # false
            exit_loop  # loop_1
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

// crbug.com/tint/243
TEST_F(SpirvParserTest, FalseBranch_LoopContinue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %99 %90 None
               OpBranch %20
         %20 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %true %30 %90 ; false branch continue is ok
         %30 = OpLabel
               OpBranch %40
         %40 = OpLabel ; selection merge
               OpBranch %90
         %90 = OpLabel ; continue target
               OpBranch %10 ; backedge
         %99 = OpLabel ; loop merge
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
            exit_if  # if_1
          }
          $B5: {  # false
            continue  # -> $B3
          }
        }
        continue  # -> $B3
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

// crbug.com/tint/243
TEST_F(SpirvParserTest, FalseBranch_SwitchBreak) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %one %99 20 %20
         %20 = OpLabel
               OpSelectionMerge %40 None
               OpBranchConditional %true %30 %99 ; false branch switch break is ok
         %30 = OpLabel
               OpBranch %40
         %40 = OpLabel ; if-selection merge
               OpBranch %99
         %99 = OpLabel ; switch merge
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    switch 1i [c: (default, $B2), c: (20i, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        if true [t: $B4, f: $B5] {  # if_1
          $B4: {  # true
            exit_if  # if_1
          }
          $B5: {  # false
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
