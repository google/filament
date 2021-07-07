// Copyright (c) 2020 Google LLC
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

#include "source/opt/debug_info_manager.h"

#include <memory>
#include <string>
#include <vector>

#include "effcee/effcee.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "source/opt/build_module.h"
#include "source/opt/instruction.h"
#include "spirv-tools/libspirv.hpp"

// Constants for OpenCL.DebugInfo.100 extension instructions.

static const uint32_t kDebugFunctionOperandFunctionIndex = 13;
static const uint32_t kDebugInlinedAtOperandLineIndex = 4;
static const uint32_t kDebugInlinedAtOperandScopeIndex = 5;
static const uint32_t kDebugInlinedAtOperandInlinedIndex = 6;
static const uint32_t kOpLineInOperandFileIndex = 0;
static const uint32_t kOpLineInOperandLineIndex = 1;
static const uint32_t kOpLineInOperandColumnIndex = 2;

namespace spvtools {
namespace opt {
namespace analysis {
namespace {

TEST(DebugInfoManager, GetDebugInlinedAt) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR
               OpExecutionMode %main OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void main(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "main"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %main
        %100 = OpExtInst %void %1 DebugInlinedAt 7 %22
       %main = OpFunction %void None %27
         %28 = OpLabel
         %31 = OpLoad %float %in_var_COLOR
               OpStore %100 %31
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  DebugInfoManager manager(context.get());

  EXPECT_EQ(manager.GetDebugInlinedAt(150), nullptr);
  EXPECT_EQ(manager.GetDebugInlinedAt(31), nullptr);
  EXPECT_EQ(manager.GetDebugInlinedAt(22), nullptr);

  auto* inst = manager.GetDebugInlinedAt(100);
  EXPECT_EQ(inst->GetSingleWordOperand(kDebugInlinedAtOperandLineIndex), 7);
  EXPECT_EQ(inst->GetSingleWordOperand(kDebugInlinedAtOperandScopeIndex), 22);
}

TEST(DebugInfoManager, CreateDebugInlinedAt) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR
               OpExecutionMode %main OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void main(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "main"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %main
        %100 = OpExtInst %void %1 DebugInlinedAt 7 %22
       %main = OpFunction %void None %27
         %28 = OpLabel
         %31 = OpLoad %float %in_var_COLOR
               OpStore %100 %31
               OpReturn
               OpFunctionEnd
  )";

  DebugScope scope(22U, 0U);

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  DebugInfoManager manager(context.get());

  uint32_t inlined_at_id = manager.CreateDebugInlinedAt(nullptr, scope);
  auto* inlined_at = manager.GetDebugInlinedAt(inlined_at_id);
  EXPECT_NE(inlined_at, nullptr);
  EXPECT_EQ(inlined_at->GetSingleWordOperand(kDebugInlinedAtOperandLineIndex),
            1);
  EXPECT_EQ(inlined_at->GetSingleWordOperand(kDebugInlinedAtOperandScopeIndex),
            22);
  EXPECT_EQ(inlined_at->NumOperands(), kDebugInlinedAtOperandScopeIndex + 1);

  const uint32_t line_number = 77U;
  Instruction line(context.get(), SpvOpLine);
  line.SetInOperands({
      {spv_operand_type_t::SPV_OPERAND_TYPE_ID, {5U}},
      {spv_operand_type_t::SPV_OPERAND_TYPE_LITERAL_INTEGER, {line_number}},
      {spv_operand_type_t::SPV_OPERAND_TYPE_LITERAL_INTEGER, {0U}},
  });

  inlined_at_id = manager.CreateDebugInlinedAt(&line, scope);
  inlined_at = manager.GetDebugInlinedAt(inlined_at_id);
  EXPECT_NE(inlined_at, nullptr);
  EXPECT_EQ(inlined_at->GetSingleWordOperand(kDebugInlinedAtOperandLineIndex),
            line_number);
  EXPECT_EQ(inlined_at->GetSingleWordOperand(kDebugInlinedAtOperandScopeIndex),
            22);
  EXPECT_EQ(inlined_at->NumOperands(), kDebugInlinedAtOperandScopeIndex + 1);

  scope.SetInlinedAt(100U);
  inlined_at_id = manager.CreateDebugInlinedAt(&line, scope);
  inlined_at = manager.GetDebugInlinedAt(inlined_at_id);
  EXPECT_NE(inlined_at, nullptr);
  EXPECT_EQ(inlined_at->GetSingleWordOperand(kDebugInlinedAtOperandLineIndex),
            line_number);
  EXPECT_EQ(inlined_at->GetSingleWordOperand(kDebugInlinedAtOperandScopeIndex),
            22);
  EXPECT_EQ(inlined_at->NumOperands(), kDebugInlinedAtOperandInlinedIndex + 1);
  EXPECT_EQ(
      inlined_at->GetSingleWordOperand(kDebugInlinedAtOperandInlinedIndex),
      100U);
}

TEST(DebugInfoManager, GetDebugInfoNone) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR
               OpExecutionMode %main OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void main(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "main"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %main
         %12 = OpExtInst %void %1 DebugInfoNone
         %25 = OpExtInst %void %1 DebugLocalVariable %24 %18 %15 1 20 %22 FlagIsLocal 0
       %main = OpFunction %void None %27
         %28 = OpLabel
        %100 = OpVariable %_ptr_Function_float Function
         %31 = OpLoad %float %in_var_COLOR
               OpStore %100 %31
         %36 = OpExtInst %void %1 DebugDeclare %25 %100 %13
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  DebugInfoManager manager(context.get());

  Instruction* debug_info_none_inst = manager.GetDebugInfoNone();
  EXPECT_NE(debug_info_none_inst, nullptr);
  EXPECT_EQ(debug_info_none_inst->GetOpenCL100DebugOpcode(),
            OpenCLDebugInfo100DebugInfoNone);
  EXPECT_EQ(debug_info_none_inst->PreviousNode(), nullptr);
}

TEST(DebugInfoManager, CreateDebugInfoNone) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR
               OpExecutionMode %main OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void main(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "main"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %main
         %25 = OpExtInst %void %1 DebugLocalVariable %24 %18 %15 1 20 %22 FlagIsLocal 0
       %main = OpFunction %void None %27
         %28 = OpLabel
        %100 = OpVariable %_ptr_Function_float Function
         %31 = OpLoad %float %in_var_COLOR
               OpStore %100 %31
         %36 = OpExtInst %void %1 DebugDeclare %25 %100 %13
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  DebugInfoManager manager(context.get());

  Instruction* debug_info_none_inst = manager.GetDebugInfoNone();
  EXPECT_NE(debug_info_none_inst, nullptr);
  EXPECT_EQ(debug_info_none_inst->GetOpenCL100DebugOpcode(),
            OpenCLDebugInfo100DebugInfoNone);
  EXPECT_EQ(debug_info_none_inst->PreviousNode(), nullptr);
}

TEST(DebugInfoManager, GetDebugFunction) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %200 "200" %in_var_COLOR
               OpExecutionMode %200 OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void 200(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "200"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %200 "200"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %200
         %25 = OpExtInst %void %1 DebugLocalVariable %24 %18 %15 1 20 %22 FlagIsLocal 0
       %200 = OpFunction %void None %27
         %28 = OpLabel
        %100 = OpVariable %_ptr_Function_float Function
         %31 = OpLoad %float %in_var_COLOR
               OpStore %100 %31
         %36 = OpExtInst %void %1 DebugDeclare %25 %100 %13
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  DebugInfoManager manager(context.get());

  EXPECT_EQ(manager.GetDebugFunction(100), nullptr);
  EXPECT_EQ(manager.GetDebugFunction(150), nullptr);

  Instruction* dbg_fn = manager.GetDebugFunction(200);

  EXPECT_EQ(dbg_fn->GetOpenCL100DebugOpcode(), OpenCLDebugInfo100DebugFunction);
  EXPECT_EQ(dbg_fn->GetSingleWordOperand(kDebugFunctionOperandFunctionIndex),
            200);
}

TEST(DebugInfoManager, GetDebugFunction_InlinedAway) {
  // struct PS_INPUT
  // {
  //   float4 iColor : COLOR;
  // };
  //
  // struct PS_OUTPUT
  // {
  //   float4 oColor : SV_Target0;
  // };
  //
  // float4 foo(float4 ic)
  // {
  //   float4 c = ic / 2.0;
  //   return c;
  // }
  //
  // PS_OUTPUT MainPs(PS_INPUT i)
  // {
  //   PS_OUTPUT ps_output;
  //   float4 ic = i.iColor;
  //   ps_output.oColor = foo(ic);
  //   return ps_output;
  // }
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPs "MainPs" %in_var_COLOR %out_var_SV_Target0
               OpExecutionMode %MainPs OriginUpperLeft
         %15 = OpString "foo2.frag"
         %19 = OpString "PS_OUTPUT"
         %23 = OpString "float"
         %26 = OpString "oColor"
         %28 = OpString "PS_INPUT"
         %31 = OpString "iColor"
         %33 = OpString "foo"
         %37 = OpString "c"
         %39 = OpString "ic"
         %42 = OpString "src.MainPs"
         %47 = OpString "ps_output"
         %50 = OpString "i"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPs "MainPs"
               OpDecorate %in_var_COLOR Location 0
               OpDecorate %out_var_SV_Target0 Location 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
   %uint_128 = OpConstant %uint 128
     %uint_0 = OpConstant %uint 0
         %52 = OpTypeFunction %void
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
  %float_0_5 = OpConstant %float 0.5
        %130 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
        %115 = OpExtInst %void %1 DebugInfoNone
         %49 = OpExtInst %void %1 DebugExpression
         %17 = OpExtInst %void %1 DebugSource %15
         %18 = OpExtInst %void %1 DebugCompilationUnit 1 4 %17 HLSL
         %21 = OpExtInst %void %1 DebugTypeComposite %19 Structure %17 6 1 %18 %19 %uint_128 FlagIsProtected|FlagIsPrivate %22
         %24 = OpExtInst %void %1 DebugTypeBasic %23 %uint_32 Float
         %25 = OpExtInst %void %1 DebugTypeVector %24 4
         %22 = OpExtInst %void %1 DebugTypeMember %26 %25 %17 8 5 %21 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %29 = OpExtInst %void %1 DebugTypeComposite %28 Structure %17 1 1 %18 %28 %uint_128 FlagIsProtected|FlagIsPrivate %30
         %30 = OpExtInst %void %1 DebugTypeMember %31 %25 %17 3 5 %29 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
         %32 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %25 %25
         %34 = OpExtInst %void %1 DebugFunction %33 %32 %17 11 1 %18 %33 FlagIsProtected|FlagIsPrivate 12 %115
         %36 = OpExtInst %void %1 DebugLexicalBlock %17 12 1 %34
         %38 = OpExtInst %void %1 DebugLocalVariable %37 %25 %17 13 12 %36 FlagIsLocal
         %41 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %21 %29
         %43 = OpExtInst %void %1 DebugFunction %42 %41 %17 17 1 %18 %42 FlagIsProtected|FlagIsPrivate 18 %115
         %45 = OpExtInst %void %1 DebugLexicalBlock %17 18 1 %43
         %46 = OpExtInst %void %1 DebugLocalVariable %39 %25 %17 20 12 %45 FlagIsLocal
         %48 = OpExtInst %void %1 DebugLocalVariable %47 %21 %17 19 15 %45 FlagIsLocal
        %107 = OpExtInst %void %1 DebugInlinedAt 21 %45
     %MainPs = OpFunction %void None %52
         %53 = OpLabel
         %57 = OpLoad %v4float %in_var_COLOR
        %131 = OpExtInst %void %1 DebugScope %45
               OpLine %15 20 12
        %117 = OpExtInst %void %1 DebugValue %46 %57 %49
        %132 = OpExtInst %void %1 DebugScope %36 %107
               OpLine %15 13 19
        %112 = OpFMul %v4float %57 %130
               OpLine %15 13 12
        %116 = OpExtInst %void %1 DebugValue %38 %112 %49
        %133 = OpExtInst %void %1 DebugScope %45
        %128 = OpExtInst %void %1 DebugValue %48 %112 %49 %int_0
        %134 = OpExtInst %void %1 DebugNoScope
               OpStore %out_var_SV_Target0 %112
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  DebugInfoManager manager(context.get());

  EXPECT_EQ(manager.GetDebugFunction(115), nullptr);
}

TEST(DebugInfoManager, CloneDebugInlinedAt) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR
               OpExecutionMode %main OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void main(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "main"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %main
        %100 = OpExtInst %void %1 DebugInlinedAt 7 %22
       %main = OpFunction %void None %27
         %28 = OpLabel
         %31 = OpLoad %float %in_var_COLOR
               OpStore %100 %31
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  DebugInfoManager manager(context.get());

  EXPECT_EQ(manager.CloneDebugInlinedAt(150), nullptr);
  EXPECT_EQ(manager.CloneDebugInlinedAt(22), nullptr);

  auto* inst = manager.CloneDebugInlinedAt(100);
  EXPECT_EQ(inst->GetSingleWordOperand(kDebugInlinedAtOperandLineIndex), 7);
  EXPECT_EQ(inst->GetSingleWordOperand(kDebugInlinedAtOperandScopeIndex), 22);
  EXPECT_EQ(inst->NumOperands(), kDebugInlinedAtOperandScopeIndex + 1);

  Instruction* before_100 = nullptr;
  for (auto it = context->module()->ext_inst_debuginfo_begin();
       it != context->module()->ext_inst_debuginfo_end(); ++it) {
    if (it->result_id() == 100) break;
    before_100 = &*it;
  }
  EXPECT_NE(inst, before_100);

  inst = manager.CloneDebugInlinedAt(100, manager.GetDebugInlinedAt(100));
  EXPECT_EQ(inst->GetSingleWordOperand(kDebugInlinedAtOperandLineIndex), 7);
  EXPECT_EQ(inst->GetSingleWordOperand(kDebugInlinedAtOperandScopeIndex), 22);
  EXPECT_EQ(inst->NumOperands(), kDebugInlinedAtOperandScopeIndex + 1);

  before_100 = nullptr;
  for (auto it = context->module()->ext_inst_debuginfo_begin();
       it != context->module()->ext_inst_debuginfo_end(); ++it) {
    if (it->result_id() == 100) break;
    before_100 = &*it;
  }
  EXPECT_EQ(inst, before_100);
}

TEST(DebugInfoManager, KillDebugDeclares) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR
               OpExecutionMode %main OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void main(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "main"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %main
         %12 = OpExtInst %void %1 DebugInfoNone
         %25 = OpExtInst %void %1 DebugLocalVariable %24 %18 %15 1 20 %22 FlagIsLocal 0
       %main = OpFunction %void None %27
         %28 = OpLabel
        %100 = OpVariable %_ptr_Function_float Function
         %31 = OpLoad %float %in_var_COLOR
               OpStore %100 %31
         %36 = OpExtInst %void %1 DebugDeclare %25 %100 %13
         %37 = OpExtInst %void %1 DebugDeclare %25 %100 %13
         %38 = OpExtInst %void %1 DebugDeclare %25 %100 %13
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* dbg_info_mgr = context->get_debug_info_mgr();
  auto* def_use_mgr = context->get_def_use_mgr();

  EXPECT_TRUE(dbg_info_mgr->IsVariableDebugDeclared(100));
  EXPECT_EQ(def_use_mgr->GetDef(36)->GetOpenCL100DebugOpcode(),
            OpenCLDebugInfo100DebugDeclare);
  EXPECT_EQ(def_use_mgr->GetDef(37)->GetOpenCL100DebugOpcode(),
            OpenCLDebugInfo100DebugDeclare);
  EXPECT_EQ(def_use_mgr->GetDef(38)->GetOpenCL100DebugOpcode(),
            OpenCLDebugInfo100DebugDeclare);

  dbg_info_mgr->KillDebugDeclares(100);
  EXPECT_EQ(def_use_mgr->GetDef(36), nullptr);
  EXPECT_EQ(def_use_mgr->GetDef(37), nullptr);
  EXPECT_EQ(def_use_mgr->GetDef(38), nullptr);
  EXPECT_FALSE(dbg_info_mgr->IsVariableDebugDeclared(100));
}

TEST(DebugInfoManager, AddDebugValueForDecl) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %in_var_COLOR
               OpExecutionMode %main OriginUpperLeft
          %5 = OpString "ps.hlsl"
         %14 = OpString "#line 1 \"ps.hlsl\"
void main(float in_var_color : COLOR) {
  float color = in_var_color;
}
"
         %17 = OpString "float"
         %21 = OpString "main"
         %24 = OpString "color"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %main "main"
               OpDecorate %in_var_COLOR Location 0
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%in_var_COLOR = OpVariable %_ptr_Input_float Input
         %13 = OpExtInst %void %1 DebugExpression
         %15 = OpExtInst %void %1 DebugSource %5 %14
         %16 = OpExtInst %void %1 DebugCompilationUnit 1 4 %15 HLSL
         %18 = OpExtInst %void %1 DebugTypeBasic %17 %uint_32 Float
         %20 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %18 %18
         %22 = OpExtInst %void %1 DebugFunction %21 %20 %15 1 1 %16 %21 FlagIsProtected|FlagIsPrivate 1 %main
         %12 = OpExtInst %void %1 DebugInfoNone
         %25 = OpExtInst %void %1 DebugLocalVariable %24 %18 %15 1 20 %22 FlagIsLocal 0
       %main = OpFunction %void None %27
         %28 = OpLabel
        %100 = OpVariable %_ptr_Function_float Function
         %31 = OpLoad %float %in_var_COLOR
        %101 = OpExtInst %void %1 DebugScope %22
               OpLine %5 13 7
               OpStore %100 %31
               OpNoLine
        %102 = OpExtInst %void %1 DebugNoScope
         %36 = OpExtInst %void %1 DebugDeclare %25 %100 %13
               OpReturn
               OpFunctionEnd
  )";

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  auto* def_use_mgr = context->get_def_use_mgr();
  auto* dbg_decl = def_use_mgr->GetDef(36);
  EXPECT_EQ(dbg_decl->GetOpenCL100DebugOpcode(),
            OpenCLDebugInfo100DebugDeclare);

  auto* dbg_info_mgr = context->get_debug_info_mgr();
  Instruction* store = dbg_decl->PreviousNode();
  auto* dbg_value =
      dbg_info_mgr->AddDebugValueForDecl(dbg_decl, 100, dbg_decl, store);

  EXPECT_EQ(dbg_value->GetOpenCL100DebugOpcode(), OpenCLDebugInfo100DebugValue);
  EXPECT_EQ(dbg_value->dbg_line_inst()->GetSingleWordInOperand(
                kOpLineInOperandFileIndex),
            5);
  EXPECT_EQ(dbg_value->dbg_line_inst()->GetSingleWordInOperand(
                kOpLineInOperandLineIndex),
            13);
  EXPECT_EQ(dbg_value->dbg_line_inst()->GetSingleWordInOperand(
                kOpLineInOperandColumnIndex),
            7);
}

}  // namespace
}  // namespace analysis
}  // namespace opt
}  // namespace spvtools
