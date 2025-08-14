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
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT of THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/api/tint.h"

#include <gtest/gtest.h>

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif

namespace tint {
namespace {

using ApiTest = testing::Test;

#if TINT_BUILD_SPV_READER && TINT_BUILD_WGSL_WRITER
TEST_F(ApiTest, SpirvToWgsl) {
    std::string spirv_asm = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %depth %colors
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main DepthReplacing
               OpDecorate %depth BuiltIn FragDepth
               OpDecorate %colors Location 1
               OpMemberDecorate %str 1 NoPerspective
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void
        %str = OpTypeStruct %vec4f %vec4f
        %u32 = OpTypeInt 32 0
      %u32_0 = OpConstant %u32 0
      %u32_1 = OpConstant %u32 1
     %f32_42 = OpConstant %f32 42.0
     %f32_n1 = OpConstant %f32 -1.0
   %f32_v4_a = OpConstantComposite %vec4f %f32_42 %f32_42 %f32_42 %f32_n1
   %f32_v4_b = OpConstantComposite %vec4f %f32_n1 %f32_n1 %f32_n1 %f32_42

%_ptr_Output_f32 = OpTypePointer Output %f32
%_ptr_Output_vec4f = OpTypePointer Output %vec4f
  %_ptr_Output_str = OpTypePointer Output %str
      %depth = OpVariable %_ptr_Output_f32 Output
     %colors = OpVariable %_ptr_Output_str Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
   %access_a = OpAccessChain %_ptr_Output_vec4f %colors %u32_0
   %access_b = OpAccessChain %_ptr_Output_vec4f %colors %u32_1
               OpStore %access_a %f32_v4_a
               OpStore %access_b %f32_v4_b
               OpStore %depth %f32_42
               OpReturn
               OpFunctionEnd
    )";

    StringStream errors;
    std::vector<uint32_t> spirv_binary;
    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
    tools.SetMessageConsumer([&errors](spv_message_level_t, const char*,
                                       const spv_position_t& position, const char* message) {
        errors << "assembly error:" << position.line << ":" << position.column << ": " << message;
    });
    ASSERT_TRUE(tools.Assemble(spirv_asm, &spirv_binary)) << errors.str();

    auto result = tint::SpirvToWgsl(spirv_binary);
    ASSERT_EQ(result, tint::Success) << result.Failure();
    EXPECT_EQ(result.Get(), R"(var<private> v : f32;

struct tint_symbol_2 {
  tint_symbol : vec4<f32>,
  tint_symbol_1 : vec4<f32>,
}

var<private> v_1 : tint_symbol_2;

fn main_inner() {
  v_1.tint_symbol = vec4<f32>(42.0f, 42.0f, 42.0f, -1.0f);
  v_1.tint_symbol_1 = vec4<f32>(-1.0f, -1.0f, -1.0f, 42.0f);
  v = 42.0f;
}

struct tint_symbol_4 {
  @builtin(frag_depth)
  tint_symbol_3 : f32,
  @location(1u)
  tint_symbol : vec4<f32>,
  @location(2u) @interpolate(linear)
  tint_symbol_1 : vec4<f32>,
}

@fragment
fn main() -> tint_symbol_4 {
  main_inner();
  return tint_symbol_4(v, v_1.tint_symbol, v_1.tint_symbol_1);
}
)");
}
#endif

}  // namespace
}  // namespace tint
