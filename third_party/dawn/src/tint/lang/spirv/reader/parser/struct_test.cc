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

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {

TEST_F(SpirvParserDeathTest, Struct_Empty) {
    EXPECT_DEATH_IF_SUPPORTED(  //
        {
            auto assembly = Assemble(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %str = OpTypeStruct
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %str

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %str
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)");
            auto parsed = Parse(Slice(assembly.Get().data(), assembly.Get().size()));
            EXPECT_EQ(parsed, Success);
        },
        "empty structures are not supported");
}

TEST_F(SpirvParserTest, Struct_BasicDecl) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %str = OpTypeStruct %i32 %i32
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %str

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %str
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:i32 @offset(4)
}

%1 = func(%2:tint_symbol_2):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Struct_MultipleUses) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %str = OpTypeStruct %i32 %i32
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %str %str %str

        %foo = OpFunction %str None %fn_type
    %param_1 = OpFunctionParameter %str
    %param_2 = OpFunctionParameter %str
  %foo_start = OpLabel
               OpReturnValue %param_1
               OpFunctionEnd

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:i32 @offset(4)
}

%1 = func(%2:tint_symbol_2, %3:tint_symbol_2):tint_symbol_2 {
  $B1: {
    ret %2
  }
}
)");
}

TEST_F(SpirvParserTest, Struct_Nested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
      %inner = OpTypeStruct %i32 %i32
     %middle = OpTypeStruct %inner %inner
      %outer = OpTypeStruct %inner %middle %inner
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %outer

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %outer
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:i32 @offset(4)
}

tint_symbol_6 = struct @align(4) {
  tint_symbol_4:tint_symbol_2 @offset(0)
  tint_symbol_5:tint_symbol_2 @offset(8)
}

tint_symbol_9 = struct @align(4) {
  tint_symbol_3:tint_symbol_2 @offset(0)
  tint_symbol_7:tint_symbol_6 @offset(8)
  tint_symbol_8:tint_symbol_2 @offset(24)
}

%1 = func(%2:tint_symbol_9):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Struct_Offset) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %str 0 Offset 0
               OpMemberDecorate %str 1 Offset 4
               OpMemberDecorate %str 2 Offset 32
               OpMemberDecorate %str 3 Offset 64
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %str = OpTypeStruct %i32 %i32 %i32 %i32
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %void %str

        %foo = OpFunction %void None %fn_type
      %param = OpFunctionParameter %str
  %foo_start = OpLabel
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_4 = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:i32 @offset(4)
  tint_symbol_2:i32 @offset(32)
  tint_symbol_3:i32 @offset(64)
}

%1 = func(%2:tint_symbol_4):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Struct_Builtin) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpMemberDecorate %str 0 BuiltIn Position
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
        %str = OpTypeStruct %vec4f
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %str
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}
)");
}

TEST_F(SpirvParserTest, Struct_Builtin_WithInvariant) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpMemberDecorate %str 0 BuiltIn Position
               OpMemberDecorate %str 0 Invariant
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
        %str = OpTypeStruct %vec4f
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %str
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @invariant, @builtin(position)
}
)");
}

struct LocationCase {
    std::string spirv_decorations;
    std::string ir;
};

using LocationStructTest = SpirvParserTestWithParam<LocationCase>;

TEST_P(LocationStructTest, MemberDecorations) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
          )" + params.spirv_decorations +
                  R"(
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
        %str = OpTypeStruct %vec4f
    %fn_type = OpTypeFunction %void

%_ptr_Input = OpTypePointer Input %str
        %var = OpVariable %_ptr_Input Input

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              params.ir);
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    LocationStructTest,
    testing::Values(
        LocationCase{
            "OpMemberDecorate %str 0 Location 1 ",
            "tint_symbol:vec4<f32> @offset(0), @location(1)",
        },
        LocationCase{
            "OpMemberDecorate %str 0 Location 2 "
            "OpMemberDecorate %str 0 NoPerspective ",
            "tint_symbol:vec4<f32> @offset(0), @location(2), @interpolate(linear, center)",
        },
        LocationCase{
            "OpMemberDecorate %str 0 Location 3 "
            "OpMemberDecorate %str 0 Flat ",
            "tint_symbol:vec4<f32> @offset(0), @location(3), @interpolate(flat, center)",
        },
        LocationCase{
            "OpMemberDecorate %str 0 Location 4 "
            "OpMemberDecorate %str 0 Centroid ",
            "tint_symbol:vec4<f32> @offset(0), @location(4), @interpolate(perspective, centroid)",
        },
        LocationCase{
            "OpMemberDecorate %str 0 Location 5 "
            "OpMemberDecorate %str 0 Sample ",
            "tint_symbol:vec4<f32> @offset(0), @location(5), @interpolate(perspective, sample)",
        },
        LocationCase{
            "OpMemberDecorate %str 0 Location 6 "
            "OpMemberDecorate %str 0 NoPerspective "
            "OpMemberDecorate %str 0 Centroid ",
            "tint_symbol:vec4<f32> @offset(0), @location(6), @interpolate(linear, centroid)",
        }));

}  // namespace tint::spirv::reader
