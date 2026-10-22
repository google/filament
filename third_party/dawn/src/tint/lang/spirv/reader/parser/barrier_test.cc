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

TEST_F(SpirvParserTest, ControlBarrier_WorkgroupBarrier) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %helper "helper"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
   %uint_264 = OpConstant %uint 264
     %helper = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_2 %uint_2 %uint_264
               OpReturn
               OpFunctionEnd
     %main = OpFunction %void None %1
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )",
              R"(
%helper = func():void {
  $B1: {
    %2:void = workgroupBarrier
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ControlBarrier_StorageBarrier) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %helper "helper"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
    %uint_72 = OpConstant %uint 72
     %helper = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_2 %uint_2 %uint_72
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %1
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )",
              R"(
%helper = func():void {
  $B1: {
    %2:void = storageBarrier
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ControlBarrier_TextureBarrier) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %helper "helper"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
  %uint_2056 = OpConstant %uint 2056
     %helper = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_2 %uint_2 %uint_2056
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %1
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )",
              R"(
%helper = func():void {
  $B1: {
    %2:void = textureBarrier
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, ControlBarrier_WorkgroupAndTextureAndStorageBarrier) {
    // Check that we emit multiple adjacent barrier calls when the flags
    // are combined.
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
               OpName %helper "helper"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
   %uint_x948 = OpConstant %uint 0x948
     %helper = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_2 %uint_2 %uint_x948
               OpReturn
               OpFunctionEnd
     %main = OpFunction %void None %1
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
  )",
              R"(
%helper = func():void {
  $B1: {
    %2:void = workgroupBarrier
    %3:void = storageBarrier
    %4:void = textureBarrier
    ret
  }
}
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserDeathTest, ControlBarrier_ErrBarrierInvalidExecution) {
    auto* src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
     %uint_2 = OpConstant %uint 2
   %uint_264 = OpConstant %uint 264
       %main = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_3 %uint_2 %uint_264
               OpReturn
               OpFunctionEnd
  )";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

TEST_F(SpirvParserDeathTest, ControlBarrier_ErrBarrierSemanticsMissingAcquireRelease) {
    auto* src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_0 = OpConstant %uint 0
       %main = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_2 %uint_2 %uint_0
               OpReturn
               OpFunctionEnd
  )";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

TEST_F(SpirvParserDeathTest, ControlBarrier_ErrStorageBarrierInvalidMemory) {
    auto* src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
    %uint_72 = OpConstant %uint 72
       %main = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_2 %uint_1 %uint_72
               OpReturn
               OpFunctionEnd
  )";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

TEST_F(SpirvParserDeathTest, ControlBarrier_ErrTextureBarrierInvalidMemory) {
    auto* src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"
       %void = OpTypeVoid
          %1 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
  %uint_2056 = OpConstant %uint 2056
       %main = OpFunction %void None %1
          %4 = OpLabel
               OpControlBarrier %uint_2 %uint_1 %uint_2056
               OpReturn
               OpFunctionEnd
  )";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

}  // namespace
}  // namespace tint::spirv::reader
