// Copyright 2020 The Dawn & Tint Authors
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

#include "gmock/gmock.h"
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/parse.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::HasSubstr;

TEST_F(SpirvASTParserTest, Impl_Uint32VecEmpty) {
    std::vector<uint32_t> data;
    auto p = parser(data);
    EXPECT_FALSE(p->Parse());
    // TODO(dneto): What message?
}

TEST_F(SpirvASTParserTest, Impl_InvalidModuleFails) {
    auto invalid_spv = test::Assemble("%ty = OpTypeInt 3 0");
    auto p = parser(invalid_spv);
    EXPECT_FALSE(p->Parse());
    EXPECT_THAT(p->error(), HasSubstr("TypeInt cannot appear before the memory model instruction"));
    EXPECT_THAT(p->error(), HasSubstr("OpTypeInt 3 0"));
}

TEST_F(SpirvASTParserTest, Impl_GenericVulkanShader_SimpleMemoryModel) {
    auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
    auto p = parser(spv);
    EXPECT_TRUE(p->Parse());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, Impl_GenericVulkanShader_GLSL450MemoryModel) {
    auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
    auto p = parser(spv);
    EXPECT_TRUE(p->Parse());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, Impl_GenericVulkanShader_VulkanMemoryModel) {
    auto spv = test::Assemble(R"(
  OpCapability Shader
  OpCapability VulkanMemoryModelKHR
  OpExtension "SPV_KHR_vulkan_memory_model"
  OpMemoryModel Logical VulkanKHR
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
    auto p = parser(spv);
    EXPECT_TRUE(p->Parse());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, Impl_OpenCLKernel_Fails) {
    auto spv = test::Assemble(R"(
  OpCapability Kernel
  OpCapability Addresses
  OpMemoryModel Physical32 OpenCL
  OpEntryPoint Kernel %main "main"
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  OpReturn
  OpFunctionEnd
)");
    auto p = parser(spv);
    EXPECT_FALSE(p->Parse());
    EXPECT_THAT(p->error(), HasSubstr("Capability Kernel is not allowed"));
}

TEST_F(SpirvASTParserTest, Impl_Source_NoOpLine) {
    auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %5 = OpTypeInt 32 0
  %60 = OpConstantNull %5
  %main = OpFunction %void None %voidfn
  %1 = OpLabel
  OpReturn
  OpFunctionEnd
)");
    auto p = parser(spv);
    EXPECT_TRUE(p->Parse());
    EXPECT_TRUE(p->error().empty());
    // Use instruction counting.
    auto s5 = p->GetSourceForResultIdForTest(5);
    EXPECT_EQ(7u, s5.range.begin.line);
    EXPECT_EQ(0u, s5.range.begin.column);
    auto s60 = p->GetSourceForResultIdForTest(60);
    EXPECT_EQ(8u, s60.range.begin.line);
    EXPECT_EQ(0u, s60.range.begin.column);
    auto s1 = p->GetSourceForResultIdForTest(1);
    EXPECT_EQ(10u, s1.range.begin.line);
    EXPECT_EQ(0u, s1.range.begin.column);
}

TEST_F(SpirvASTParserTest, Impl_Source_WithOpLine_WithOpNoLine) {
    auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %15 = OpString "myfile"
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  OpLine %15 42 53
  %5 = OpTypeInt 32 0
  %60 = OpConstantNull %5
  OpNoLine
  %main = OpFunction %void None %voidfn
  %1 = OpLabel
  OpReturn
  OpFunctionEnd
)");
    auto p = parser(spv);
    EXPECT_TRUE(p->Parse());
    EXPECT_TRUE(p->error().empty());
    // Use the information from the OpLine that is still in scope.
    auto s5 = p->GetSourceForResultIdForTest(5);
    EXPECT_EQ(42u, s5.range.begin.line);
    EXPECT_EQ(53u, s5.range.begin.column);
    auto s60 = p->GetSourceForResultIdForTest(60);
    EXPECT_EQ(42u, s60.range.begin.line);
    EXPECT_EQ(53u, s60.range.begin.column);
    // After OpNoLine, revert back to instruction counting.
    auto s1 = p->GetSourceForResultIdForTest(1);
    EXPECT_EQ(14u, s1.range.begin.line);
    EXPECT_EQ(0u, s1.range.begin.column);
}

TEST_F(SpirvASTParserTest, Impl_Source_InvalidId) {
    auto spv = test::Assemble(R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1
  %15 = OpString "myfile"
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %main = OpFunction %void None %voidfn
  %1 = OpLabel
  OpReturn
  OpFunctionEnd
)");
    auto p = parser(spv);
    EXPECT_TRUE(p->Parse());
    EXPECT_TRUE(p->error().empty());
    auto s99 = p->GetSourceForResultIdForTest(99);
    EXPECT_EQ(0u, s99.range.begin.line);
    EXPECT_EQ(0u, s99.range.begin.column);
}

TEST_F(SpirvASTParserTest, Impl_IsValidIdentifier) {
    EXPECT_FALSE(ASTParser::IsValidIdentifier(""));  // empty
    EXPECT_FALSE(ASTParser::IsValidIdentifier("_"));
    EXPECT_FALSE(ASTParser::IsValidIdentifier("__"));
    EXPECT_TRUE(ASTParser::IsValidIdentifier("_x"));
    EXPECT_FALSE(ASTParser::IsValidIdentifier("9"));    // leading digit, but ok later
    EXPECT_FALSE(ASTParser::IsValidIdentifier(" "));    // leading space
    EXPECT_FALSE(ASTParser::IsValidIdentifier("a "));   // trailing space
    EXPECT_FALSE(ASTParser::IsValidIdentifier("a 1"));  // space in the middle
    EXPECT_FALSE(ASTParser::IsValidIdentifier("."));    // weird character

    // a simple identifier
    EXPECT_TRUE(ASTParser::IsValidIdentifier("A"));
    // each upper case letter
    EXPECT_TRUE(ASTParser::IsValidIdentifier("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    // each lower case letter
    EXPECT_TRUE(ASTParser::IsValidIdentifier("abcdefghijklmnopqrstuvwxyz"));
    EXPECT_TRUE(ASTParser::IsValidIdentifier("a0123456789"));  // each digit
    EXPECT_TRUE(ASTParser::IsValidIdentifier("x_"));           // has underscore
}

TEST_F(SpirvASTParserTest, Impl_FailOnNonFiniteLiteral) {
    auto spv = test::Assemble(R"(
                       OpCapability Shader
                       OpMemoryModel Logical GLSL450
                       OpEntryPoint Fragment %main "main" %out_var_SV_TARGET
                       OpExecutionMode %main OriginUpperLeft
                       OpSource HLSL 600
                       OpName %out_var_SV_TARGET "out.var.SV_TARGET"
                       OpName %main "main"
                       OpDecorate %out_var_SV_TARGET Location 0
              %float = OpTypeFloat 32
     %float_0x1p_128 = OpConstant %float -0x1p+128
            %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
               %void = OpTypeVoid
                  %9 = OpTypeFunction %void
  %out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
               %main = OpFunction %void None %9
                 %10 = OpLabel
                 %12 = OpCompositeConstruct %v4float %float_0x1p_128 %float_0x1p_128 %float_0x1p_128 %float_0x1p_128
                       OpStore %out_var_SV_TARGET %12
                       OpReturn
                       OpFunctionEnd

)");
    auto p = parser(spv);
    EXPECT_FALSE(p->Parse());
    EXPECT_THAT(p->error(), HasSubstr("value cannot be represented as 'f32': -inf"));
}

TEST_F(SpirvASTParserTest, BlendSrc) {
    auto spv = test::Assemble(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag_main "frag_main" %frag_main_loc0_idx0_Output %frag_main_loc0_idx1_Output
               OpExecutionMode %frag_main OriginUpperLeft
               OpName %frag_main_loc0_idx0_Output "frag_main_loc0_idx0_Output"
               OpName %frag_main_loc0_idx1_Output "frag_main_loc0_idx1_Output"
               OpName %frag_main_inner "frag_main_inner"
               OpMemberName %FragOutput 0 "color"
               OpMemberName %FragOutput 1 "blend"
               OpName %FragOutput "FragOutput"
               OpName %output "output"
               OpName %frag_main "frag_main"
               OpDecorate %frag_main_loc0_idx0_Output Location 0
               OpDecorate %frag_main_loc0_idx0_Output Index 0
               OpDecorate %frag_main_loc0_idx1_Output Location 0
               OpDecorate %frag_main_loc0_idx1_Output Index 1
               OpMemberDecorate %FragOutput 0 Offset 0
               OpMemberDecorate %FragOutput 1 Offset 16
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%frag_main_loc0_idx0_Output = OpVariable %_ptr_Output_v4float Output
%frag_main_loc0_idx1_Output = OpVariable %_ptr_Output_v4float Output
 %FragOutput = OpTypeStruct %v4float %v4float
          %8 = OpTypeFunction %FragOutput
%_ptr_Function_FragOutput = OpTypePointer Function %FragOutput
         %12 = OpConstantNull %FragOutput
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
         %17 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
     %uint_1 = OpConstant %uint 1
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
%frag_main_inner = OpFunction %FragOutput None %8
          %9 = OpLabel
     %output = OpVariable %_ptr_Function_FragOutput Function %12
         %13 = OpAccessChain %_ptr_Function_v4float %output %uint_0
               OpStore %13 %17 None
         %20 = OpAccessChain %_ptr_Function_v4float %output %uint_1
               OpStore %20 %17 None
         %22 = OpLoad %FragOutput %output None
               OpReturnValue %22
               OpFunctionEnd
  %frag_main = OpFunction %void None %25
         %26 = OpLabel
         %27 = OpFunctionCall %FragOutput %frag_main_inner
         %28 = OpCompositeExtract %v4float %27 0
               OpStore %frag_main_loc0_idx0_Output %28 None
         %29 = OpCompositeExtract %v4float %27 1
               OpStore %frag_main_loc0_idx1_Output %29 None
               OpReturn
               OpFunctionEnd
)");
    auto program = Parse(spv, {});
    auto errs = program.Diagnostics().Str();
    EXPECT_TRUE(program.IsValid()) << errs;
    EXPECT_EQ(program.Diagnostics().Count(), 0u) << errs;
    auto result = wgsl::writer::Generate(program);
    EXPECT_EQ(result, Success);
    EXPECT_EQ("\n" + result->wgsl, R"(
enable dual_source_blending;

struct FragOutput {
  /* @offset(0) */
  color : vec4f,
  /* @offset(16) */
  blend : vec4f,
}

var<private> frag_main_loc0_idx0_Output : vec4f;

var<private> frag_main_loc0_idx1_Output : vec4f;

const x_17 = vec4f(0.5f, 0.5f, 0.5f, 1.0f);

fn frag_main_inner() -> FragOutput {
  var output = FragOutput(vec4f(), vec4f());
  output.color = x_17;
  output.blend = x_17;
  let x_22 = output;
  return x_22;
}

fn frag_main_1() {
  let x_27 = frag_main_inner();
  frag_main_loc0_idx0_Output = x_27.color;
  frag_main_loc0_idx1_Output = x_27.blend;
  return;
}

struct frag_main_out {
  @location(0) @blend_src(0)
  frag_main_loc0_idx0_Output_1 : vec4f,
  @location(0) @blend_src(1)
  frag_main_loc0_idx1_Output_1 : vec4f,
}

@fragment
fn frag_main() -> frag_main_out {
  frag_main_1();
  return frag_main_out(frag_main_loc0_idx0_Output, frag_main_loc0_idx1_Output);
}
)");
}

TEST_F(SpirvASTParserTest, ClipDistances_ArraySize_1) {
    auto spv = test::Assemble(R"(
               OpCapability Shader
               OpCapability ClipDistance
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %main_position_Output %main_clip_distances_Output %main___point_size_Output
               OpName %main_position_Output "main_position_Output"
               OpName %main_clip_distances_Output "main_clip_distances_Output"
               OpName %main___point_size_Output "main___point_size_Output"
               OpName %main_inner "main_inner"
               OpMemberName %VertexOutputs 0 "position"
               OpMemberName %VertexOutputs 1 "clipDistance"
               OpName %VertexOutputs "VertexOutputs"
               OpName %main "main"
               OpDecorate %main_position_Output BuiltIn Position
               OpDecorate %_arr_float_uint_1 ArrayStride 4
               OpDecorate %main_clip_distances_Output BuiltIn ClipDistance
               OpDecorate %main___point_size_Output BuiltIn PointSize
               OpMemberDecorate %VertexOutputs 0 Offset 0
               OpMemberDecorate %VertexOutputs 1 Offset 16
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main_position_Output = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%_ptr_Output__arr_float_uint_1 = OpTypePointer Output %_arr_float_uint_1
%main_clip_distances_Output = OpVariable %_ptr_Output__arr_float_uint_1 Output
%_ptr_Output_float = OpTypePointer Output %float
%main___point_size_Output = OpVariable %_ptr_Output_float Output
%VertexOutputs = OpTypeStruct %v4float %_arr_float_uint_1
         %14 = OpTypeFunction %VertexOutputs
         %16 = OpConstantNull %VertexOutputs
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
 %main_inner = OpFunction %VertexOutputs None %14
         %15 = OpLabel
               OpReturnValue %16
               OpFunctionEnd
       %main = OpFunction %void None %19
         %20 = OpLabel
         %21 = OpFunctionCall %VertexOutputs %main_inner
         %22 = OpCompositeExtract %v4float %21 0
               OpStore %main_position_Output %22 None
         %23 = OpCompositeExtract %_arr_float_uint_1 %21 1
               OpStore %main_clip_distances_Output %23 None
               OpStore %main___point_size_Output %float_1 None
               OpReturn
               OpFunctionEnd
)");
    auto program = Parse(spv, {});
    auto errs = program.Diagnostics().Str();
    EXPECT_TRUE(program.IsValid()) << errs;
    EXPECT_EQ(program.Diagnostics().Count(), 0u) << errs;
    auto result = wgsl::writer::Generate(program);
    EXPECT_EQ(result, Success);
    EXPECT_EQ("\n" + result->wgsl, R"(
enable clip_distances;

alias Arr = array<f32, 1u>;

struct VertexOutputs {
  /* @offset(0) */
  position : vec4f,
  /* @offset(16) */
  clipDistance : Arr,
}

var<private> main_position_Output : vec4f;

var<private> main_clip_distances_Output : Arr;

fn main_inner() -> VertexOutputs {
  return VertexOutputs(vec4f(), array<f32, 1u>());
}

fn main_1() {
  let x_21 = main_inner();
  main_position_Output = x_21.position;
  main_clip_distances_Output = x_21.clipDistance;
  return;
}

struct main_out {
  @builtin(position)
  main_position_Output_1 : vec4f,
  @builtin(clip_distances)
  main_clip_distances_Output_1 : Arr,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(main_position_Output, main_clip_distances_Output);
}
)");
}

TEST_F(SpirvASTParserTest, ClipDistances_ArraySize_4) {
    auto spv = test::Assemble(R"(
               OpCapability Shader
               OpCapability ClipDistance
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %main_position_Output %main_clip_distances_Output %main___point_size_Output
               OpName %main_position_Output "main_position_Output"
               OpName %main_clip_distances_Output "main_clip_distances_Output"
               OpName %main___point_size_Output "main___point_size_Output"
               OpName %main_inner "main_inner"
               OpMemberName %VertexOutputs 0 "position"
               OpMemberName %VertexOutputs 1 "clipDistance"
               OpName %VertexOutputs "VertexOutputs"
               OpName %main "main"
               OpDecorate %main_position_Output BuiltIn Position
               OpDecorate %_arr_float_uint_1 ArrayStride 4
               OpDecorate %main_clip_distances_Output BuiltIn ClipDistance
               OpDecorate %main___point_size_Output BuiltIn PointSize
               OpMemberDecorate %VertexOutputs 0 Offset 0
               OpMemberDecorate %VertexOutputs 1 Offset 16
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main_position_Output = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 4
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%_ptr_Output__arr_float_uint_1 = OpTypePointer Output %_arr_float_uint_1
%main_clip_distances_Output = OpVariable %_ptr_Output__arr_float_uint_1 Output
%_ptr_Output_float = OpTypePointer Output %float
%main___point_size_Output = OpVariable %_ptr_Output_float Output
%VertexOutputs = OpTypeStruct %v4float %_arr_float_uint_1
         %14 = OpTypeFunction %VertexOutputs
         %16 = OpConstantNull %VertexOutputs
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
 %main_inner = OpFunction %VertexOutputs None %14
         %15 = OpLabel
               OpReturnValue %16
               OpFunctionEnd
       %main = OpFunction %void None %19
         %20 = OpLabel
         %21 = OpFunctionCall %VertexOutputs %main_inner
         %22 = OpCompositeExtract %v4float %21 0
               OpStore %main_position_Output %22 None
         %23 = OpCompositeExtract %_arr_float_uint_1 %21 1
               OpStore %main_clip_distances_Output %23 None
               OpStore %main___point_size_Output %float_1 None
               OpReturn
               OpFunctionEnd
)");
    auto program = Parse(spv, {});
    auto errs = program.Diagnostics().Str();
    EXPECT_TRUE(program.IsValid()) << errs;
    EXPECT_EQ(program.Diagnostics().Count(), 0u) << errs;
    auto result = wgsl::writer::Generate(program);
    EXPECT_EQ(result, Success);
    EXPECT_EQ("\n" + result->wgsl, R"(
enable clip_distances;

alias Arr = array<f32, 4u>;

struct VertexOutputs {
  /* @offset(0) */
  position : vec4f,
  /* @offset(16) */
  clipDistance : Arr,
}

var<private> main_position_Output : vec4f;

var<private> main_clip_distances_Output : Arr;

fn main_inner() -> VertexOutputs {
  return VertexOutputs(vec4f(), array<f32, 4u>());
}

fn main_1() {
  let x_21 = main_inner();
  main_position_Output = x_21.position;
  main_clip_distances_Output = x_21.clipDistance;
  return;
}

struct main_out {
  @builtin(position)
  main_position_Output_1 : vec4f,
  @builtin(clip_distances)
  main_clip_distances_Output_1 : Arr,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(main_position_Output, main_clip_distances_Output);
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
