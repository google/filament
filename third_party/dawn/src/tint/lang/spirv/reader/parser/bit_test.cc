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

struct SpirvBitParam {
    std::string spv_name;
    std::string ir_name;
};
[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, SpirvBitParam c) {
    out << c.spv_name;
    return out;
}

using SpirvParser_BitwiseTest = SpirvParserTestWithParam<SpirvBitParam>;

TEST_P(SpirvParser_BitwiseTest, Scalar_SignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
        %one = OpConstant %int 1
        %two = OpConstant %int 2
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 1i, 2i
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Scalar_SignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %one %eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 1i, 8u
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Scalar_UnsignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %eight %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 8u, 1i
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Scalar_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %eight %nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 8u, 9u
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Scalar_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %eight %nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 8u, 9u
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Scalar_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %eight %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 8u, 1i
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Scalar_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %one %eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 1i, 8u
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Scalar_SignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 1i, 2i
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_SignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_SignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2one %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_UnsignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2eight %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2eight %v2nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2eight %v2nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2eight %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2one %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_BitwiseTest, Vector_SignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         SpirvParser_BitwiseTest,
                         testing::Values(SpirvBitParam{"BitwiseAnd", "bitwise_and"},  //
                                         SpirvBitParam{"BitwiseOr", "bitwise_or"},    //
                                         SpirvBitParam{"BitwiseXor", "bitwise_xor"}));

using SpirvParser_ShiftTest = SpirvParserTestWithParam<SpirvBitParam>;
TEST_P(SpirvParser_ShiftTest, Scalar_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %eight %nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 8u, 9u
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Scalar_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %eight %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 8u, 1i
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Scalar_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %one %nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 1i, 9u
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Scalar_SignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %uint %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir_name + R"(<u32> 1i, 2i
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Scalar_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %eight %nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 8u, 9u
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Scalar_UnsignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %eight %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 8u, 1i
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Scalar_SignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %one %nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 1i, 9u
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Scalar_SignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %int %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir_name + R"(<i32> 1i, 2i
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2eight %v2nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2eight %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2one %v2nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_SignedSigned_Unsigned) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2uint %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir_name + R"(<u32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2eight %v2nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_UnsignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2eight %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_SignedUnsigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2one %v2nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<i32>(1i), vec2<u32>(9u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_ShiftTest, Vector_SignedSigned_Signed) {
    auto& params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
    %v2eight = OpConstantComposite %v2uint %eight %eight
     %v2nine = OpConstantComposite %v2uint %nine %nine
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
               %1 = Op)" +
                  params.spv_name + R"( %v2int %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir_name + R"(<i32> vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    SpirvParser_ShiftTest,
    testing::Values(SpirvBitParam{"ShiftLeftLogical", "shift_left_logical"},  //
                    SpirvBitParam{"ShiftRightLogical", "shift_right_logical"},
                    SpirvBitParam{"ShiftRightArithmetic", "shift_right_arithmetic"}));

TEST_F(SpirvParserTest, Bitcast_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
        %two = OpConstant %float 2
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpBitcast %uint %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast 2.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Bitcast_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
     %v2uint = OpTypeVector %uint 2
    %v2float = OpTypeVector %float 2
      %eight = OpConstant %uint 8
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void

       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpBitcast %v2float %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = bitcast vec2<u32>(8u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Scalar_Signed_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %int %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.not<i32> 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Scalar_Signed_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %uint %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.not<u32> 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Scalar_Unsigned_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %int %eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.not<i32> 8u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Scalar_Unsigned_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %uint %eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.not<u32> 8u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Vector_Signed_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %v2int %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.not<i32> vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Vector_Signed_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %v2uint %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.not<u32> vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Vector_Unsigned_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %v2int %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.not<i32> vec2<u32>(8u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Not_Vector_Unsigned_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
      %v2int = OpTypeVector %int 2
        %one = OpConstant %int 1
      %eight = OpConstant %uint 8
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpNot %v2uint %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.not<u32> vec2<u32>(8u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, QuantizeToF16_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
        %one = OpConstant %float 1
    %v2float = OpTypeVector %float 2
      %v2one = OpConstantComposite %v2float %one %one
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpQuantizeToF16 %float %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = quantizeToF16 1.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, QuantizeToF16_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
      %float = OpTypeFloat 32
        %one = OpConstant %float 1
    %v2float = OpTypeVector %float 2
      %v2one = OpConstantComposite %v2float %one %one
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpQuantizeToF16 %v2float %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = quantizeToF16 vec2<f32>(1.0f)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Scalar_Signed_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %int %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_negate<i32> 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Scalar_Signed_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %uint %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_negate<u32> 1i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Scalar_Unsigned_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %int %eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_negate<i32> 8u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Scalar_Unsigned_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %uint %eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_negate<u32> 8u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Vector_Signed_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %v2int %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.s_negate<i32> vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Vector_Signed_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %v2uint %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.s_negate<u32> vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Vector_Unsigned_Signed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %v2int %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.s_negate<i32> vec2<u32>(8u)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, SNegate_Vector_Unsigned_Unsigned) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
      %v2one = OpConstantComposite %v2int %one %one
    %v2eight = OpConstantComposite %v2uint %eight %eight
    %void_fn = OpTypeFunction %void
       %main = OpFunction %void None %void_fn
 %main_start = OpLabel
          %1 = OpSNegate %v2uint %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.s_negate<u32> vec2<u32>(8u)
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
