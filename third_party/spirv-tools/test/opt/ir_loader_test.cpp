// Copyright (c) 2016 Google Inc.
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

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "source/opt/build_module.h"
#include "source/opt/def_use_manager.h"
#include "source/opt/ir_context.h"
#include "spirv-tools/libspirv.hpp"

namespace spvtools {
namespace opt {
namespace {

using ::testing::ContainerEq;

constexpr uint32_t kOpLineOperandLineIndex = 1;

void DoRoundTripCheck(const std::string& text) {
  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text);
  ASSERT_NE(nullptr, context) << "Failed to assemble\n" << text;

  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, /* skip_nop = */ false);

  std::string disassembled_text;
  EXPECT_TRUE(t.Disassemble(binary, &disassembled_text));
  EXPECT_EQ(text, disassembled_text);
}

TEST(IrBuilder, RoundTrip) {
  // #version 310 es
  // int add(int a, int b) { return a + b; }
  // void main() { add(1, 2); }
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
               "OpSource ESSL 310\n"
               "OpSourceExtension \"GL_GOOGLE_cpp_style_line_directive\"\n"
               "OpSourceExtension \"GL_GOOGLE_include_directive\"\n"
               "OpName %main \"main\"\n"
               "OpName %add_i1_i1_ \"add(i1;i1;\"\n"
               "OpName %a \"a\"\n"
               "OpName %b \"b\"\n"
               "OpName %param \"param\"\n"
               "OpName %param_0 \"param\"\n"
       "%void = OpTypeVoid\n"
          "%9 = OpTypeFunction %void\n"
        "%int = OpTypeInt 32 1\n"
 "%_ptr_Function_int = OpTypePointer Function %int\n"
         "%12 = OpTypeFunction %int %_ptr_Function_int %_ptr_Function_int\n"
      "%int_1 = OpConstant %int 1\n"
      "%int_2 = OpConstant %int 2\n"
       "%main = OpFunction %void None %9\n"
         "%15 = OpLabel\n"
      "%param = OpVariable %_ptr_Function_int Function\n"
    "%param_0 = OpVariable %_ptr_Function_int Function\n"
               "OpStore %param %int_1\n"
               "OpStore %param_0 %int_2\n"
         "%16 = OpFunctionCall %int %add_i1_i1_ %param %param_0\n"
               "OpReturn\n"
               "OpFunctionEnd\n"
 "%add_i1_i1_ = OpFunction %int None %12\n"
          "%a = OpFunctionParameter %_ptr_Function_int\n"
          "%b = OpFunctionParameter %_ptr_Function_int\n"
         "%17 = OpLabel\n"
         "%18 = OpLoad %int %a\n"
         "%19 = OpLoad %int %b\n"
         "%20 = OpIAdd %int %18 %19\n"
               "OpReturnValue %20\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, RoundTripIncompleteBasicBlock) {
  DoRoundTripCheck(
      "%2 = OpFunction %1 None %3\n"
      "%4 = OpLabel\n"
      "OpNop\n");
}

TEST(IrBuilder, RoundTripIncompleteFunction) {
  DoRoundTripCheck("%2 = OpFunction %1 None %3\n");
}

TEST(IrBuilder, RoundTripFunctionPointer) {
  DoRoundTripCheck(
      "OpCapability Linkage\n"
      "OpCapability FunctionPointersINTEL\n"
      "OpName %some_function \"some_function\"\n"
      "OpName %ptr_to_function \"ptr_to_function\"\n"
      "OpDecorate %some_function LinkageAttributes \"some_function\" Import\n"
      "%float = OpTypeFloat 32\n"
      "%4 = OpTypeFunction %float %float\n"
      "%_ptr_Function_4 = OpTypePointer Function %4\n"
      "%ptr_to_function = OpConstantFunctionPointerINTEL %_ptr_Function_4 "
      "%some_function\n"
      "%some_function = OpFunction %float Const %4\n"
      "%6 = OpFunctionParameter %float\n"
      "OpFunctionEnd\n");
}
TEST(IrBuilder, KeepLineDebugInfo) {
  // #version 310 es
  // void main() {}
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
          "%3 = OpString \"minimal.vert\"\n"
               "OpSource ESSL 310\n"
               "OpName %main \"main\"\n"
               "OpLine %3 10 10\n"
       "%void = OpTypeVoid\n"
               "OpLine %3 100 100\n"
          "%5 = OpTypeFunction %void\n"
       "%main = OpFunction %void None %5\n"
               "OpLine %3 1 1\n"
               "OpNoLine\n"
               "OpLine %3 2 2\n"
               "OpLine %3 3 3\n"
          "%6 = OpLabel\n"
               "OpLine %3 4 4\n"
               "OpNoLine\n"
               "OpReturn\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, DistributeLineDebugInfo) {
  const std::string text =
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
               "OpSource ESSL 310\n"
       "%file = OpString \"test\"\n"
               "OpName %main \"main\"\n"
               "OpName %f_ \"f(\"\n"
               "OpName %gv1 \"gv1\"\n"
               "OpName %gv2 \"gv2\"\n"
               "OpName %lv1 \"lv1\"\n"
               "OpName %lv2 \"lv2\"\n"
               "OpName %lv1_0 \"lv1\"\n"
       "%void = OpTypeVoid\n"
         "%10 = OpTypeFunction %void\n"
               "OpLine %file 10 0\n"
      "%float = OpTypeFloat 32\n"
         "%12 = OpTypeFunction %float\n"
 "%_ptr_Private_float = OpTypePointer Private %float\n"
        "%gv1 = OpVariable %_ptr_Private_float Private\n"
   "%float_10 = OpConstant %float 10\n"
        "%gv2 = OpVariable %_ptr_Private_float Private\n"
  "%float_100 = OpConstant %float 100\n"
 "%_ptr_Function_float = OpTypePointer Function %float\n"
       "%main = OpFunction %void None %10\n"
         "%17 = OpLabel\n"
      "%lv1_0 = OpVariable %_ptr_Function_float Function\n"
               "OpStore %gv1 %float_10\n"
               "OpStore %gv2 %float_100\n"
               "OpLine %file 1 0\n"
               "OpNoLine\n"
               "OpLine %file 2 0\n"
         "%18 = OpLoad %float %gv1\n"
         "%19 = OpLoad %float %gv2\n"
         "%20 = OpFSub %float %18 %19\n"
               "OpStore %lv1_0 %20\n"
               "OpReturn\n"
               "OpFunctionEnd\n"
         "%f_ = OpFunction %float None %12\n"
         "%21 = OpLabel\n"
        "%lv1 = OpVariable %_ptr_Function_float Function\n"
        "%lv2 = OpVariable %_ptr_Function_float Function\n"
               "OpLine %file 3 0\n"
               "OpLine %file 4 0\n"
         "%22 = OpLoad %float %gv1\n"
         "%23 = OpLoad %float %gv2\n"
         "%24 = OpFAdd %float %22 %23\n"
               "OpStore %lv1 %24\n"
               "OpLine %file 5 0\n"
               "OpLine %file 6 0\n"
               "OpNoLine\n"
         "%25 = OpLoad %float %gv1\n"
         "%26 = OpLoad %float %gv2\n"
         "%27 = OpFMul %float %25 %26\n"
               "OpBranch %28\n"
         "%28 = OpLabel\n"
               "OpStore %lv2 %27\n"
         "%29 = OpLoad %float %lv1\n"
               "OpLine %file 7 0\n"
         "%30 = OpLoad %float %lv2\n"
         "%31 = OpFDiv %float %28 %29\n"
               "OpReturnValue %30\n"
               "OpFunctionEnd\n";
  // clang-format on

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ASSERT_NE(nullptr, context);

  struct LineInstrCheck {
    uint32_t id;
    std::vector<uint32_t> line_numbers;
  };
  const uint32_t kNoLine = 0;
  const LineInstrCheck line_checks[] = {
      {12, {10}},   {18, {1, kNoLine, 2}},
      {19, {2}},    {20, {2}},
      {22, {3, 4}}, {23, {4}},
      {24, {4}},    {25, {5, 6, kNoLine}},
      {26, {}},     {27, {}},
      {28, {}},     {29, {}},
      {30, {7}},    {31, {7}},
  };

  spvtools::opt::analysis::DefUseManager* def_use_mgr =
      context->get_def_use_mgr();
  for (const LineInstrCheck& check : line_checks) {
    auto& lines = def_use_mgr->GetDef(check.id)->dbg_line_insts();
    for (uint32_t i = 0; i < check.line_numbers.size(); ++i) {
      if (check.line_numbers[i] == kNoLine) {
        EXPECT_EQ(lines[i].opcode(), spv::Op::OpNoLine);
        continue;
      }
      EXPECT_EQ(lines[i].opcode(), spv::Op::OpLine);
      EXPECT_EQ(lines[i].GetSingleWordOperand(kOpLineOperandLineIndex),
                check.line_numbers[i]);
    }
  }
}

TEST(IrBuilder, BuildModule_WithoutExtraLines) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Vertex %main "main"
%file = OpString "my file"
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%main = OpFunction %void None %voidfn
%100 = OpLabel
%1 = OpFAdd %float %float_1 %float_1
OpLine %file 1 0
%2 = OpFMul %float %1 %1
%3 = OpFSub %float %2 %2
OpReturn
OpFunctionEnd
)";

  std::vector<uint32_t> binary;
  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_TRUE(t.Assemble(text, &binary,
                         SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS));

  // This is the function we're testing.
  std::unique_ptr<IRContext> context = BuildModule(
      SPV_ENV_UNIVERSAL_1_5, nullptr, binary.data(), binary.size(), false);
  ASSERT_NE(nullptr, context);

  spvtools::opt::analysis::DefUseManager* def_use_mgr =
      context->get_def_use_mgr();

  std::vector<spv::Op> opcodes;
  for (auto* inst = def_use_mgr->GetDef(1);
       inst && (inst->opcode() != spv::Op::OpFunctionEnd);
       inst = inst->NextNode()) {
    inst->ForEachInst(
        [&opcodes](spvtools::opt::Instruction* sub_inst) {
          opcodes.push_back(sub_inst->opcode());
        },
        true);
  }

  EXPECT_THAT(opcodes, ContainerEq(std::vector<spv::Op>{
                           spv::Op::OpFAdd, spv::Op::OpLine, spv::Op::OpFMul,
                           spv::Op::OpFSub, spv::Op::OpReturn}));
}

TEST(IrBuilder, BuildModule_WithExtraLines_IsDefault) {
  const std::string text = R"(OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Vertex %main "main"
%file = OpString "my file"
%void = OpTypeVoid
%voidfn = OpTypeFunction %void
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%main = OpFunction %void None %voidfn
%100 = OpLabel
%1 = OpFAdd %float %float_1 %float_1
OpLine %file 1 0
%2 = OpFMul %float %1 %1
%3 = OpFSub %float %2 %2
OpReturn
OpFunctionEnd
)";

  std::vector<uint32_t> binary;

  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  ASSERT_TRUE(t.Assemble(text, &binary,
                         SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS));

  // This is the function we're testing.
  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_5, nullptr, binary.data(), binary.size());

  spvtools::opt::analysis::DefUseManager* def_use_mgr =
      context->get_def_use_mgr();

  std::vector<spv::Op> opcodes;
  for (auto* inst = def_use_mgr->GetDef(1);
       inst && (inst->opcode() != spv::Op::OpFunctionEnd);
       inst = inst->NextNode()) {
    inst->ForEachInst(
        [&opcodes](spvtools::opt::Instruction* sub_inst) {
          opcodes.push_back(sub_inst->opcode());
        },
        true);
  }

  EXPECT_THAT(opcodes, ContainerEq(std::vector<spv::Op>{
                           spv::Op::OpFAdd, spv::Op::OpLine, spv::Op::OpFMul,
                           spv::Op::OpLine, spv::Op::OpFSub, spv::Op::OpLine,
                           spv::Op::OpReturn}));
}

TEST(IrBuilder, ConsumeDebugInfoInst) {
  // /* HLSL */
  //
  // struct VS_OUTPUT {
  //   float4 pos : SV_POSITION;
  //   float4 color : COLOR;
  // };
  //
  // VS_OUTPUT main(float4 pos : POSITION,
  //                float4 color : COLOR) {
  //   VS_OUTPUT vout;
  //   vout.pos = pos;
  //   vout.color = color;
  //   return vout;
  // }
  DoRoundTripCheck(R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %pos %color %gl_Position %out_var_COLOR
%7 = OpString "simple_vs.hlsl"
%8 = OpString "#line 1 \"simple_vs.hlsl\"
struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};

VS_OUTPUT main(float4 pos : POSITION,
               float4 color : COLOR) {
  VS_OUTPUT vout;
  vout.pos = pos;
  vout.color = color;
  return vout;
}
"
OpSource HLSL 600 %7 "#line 1 \"simple_vs.hlsl\"
struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};

VS_OUTPUT main(float4 pos : POSITION,
               float4 color : COLOR) {
  VS_OUTPUT vout;
  vout.pos = pos;
  vout.color = color;
  return vout;
}
"
%9 = OpString "struct VS_OUTPUT"
%10 = OpString "float"
%11 = OpString "pos : SV_POSITION"
%12 = OpString "color : COLOR"
%13 = OpString "VS_OUTPUT"
%14 = OpString "main"
%15 = OpString "VS_OUTPUT_main_v4f_v4f"
%16 = OpString "pos : POSITION"
%17 = OpString "color : COLOR"
%18 = OpString "vout"
OpName %out_var_COLOR "out.var.COLOR"
OpName %main "main"
OpName %VS_OUTPUT "VS_OUTPUT"
OpMemberName %VS_OUTPUT 0 "pos"
OpMemberName %VS_OUTPUT 1 "color"
OpName %pos "pos"
OpName %color "color"
OpName %vout "vout"
OpDecorate %gl_Position BuiltIn Position
OpDecorate %pos Location 0
OpDecorate %color Location 1
OpDecorate %out_var_COLOR Location 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_32 = OpConstant %int 32
%int_128 = OpConstant %int 128
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%31 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%VS_OUTPUT = OpTypeStruct %v4float %v4float
%_ptr_Function_VS_OUTPUT = OpTypePointer Function %VS_OUTPUT
OpLine %7 6 23
%pos = OpVariable %_ptr_Input_v4float Input
OpLine %7 7 23
%color = OpVariable %_ptr_Input_v4float Input
OpLine %7 2 16
%gl_Position = OpVariable %_ptr_Output_v4float Output
OpLine %7 3 18
%out_var_COLOR = OpVariable %_ptr_Output_v4float Output
%34 = OpExtInst %void %1 DebugSource %7 %8
%35 = OpExtInst %void %1 DebugCompilationUnit 2 4 %34 HLSL
%36 = OpExtInst %void %1 DebugTypeComposite %9 Structure %34 1 1 %35 %13 %int_128 FlagIsProtected|FlagIsPrivate %37 %38
%39 = OpExtInst %void %1 DebugTypeBasic %10 %int_32 Float
%40 = OpExtInst %void %1 DebugTypeVector %39 4
%37 = OpExtInst %void %1 DebugTypeMember %11 %40 %34 2 3 %36 %int_0 %int_128 FlagIsProtected|FlagIsPrivate
%38 = OpExtInst %void %1 DebugTypeMember %12 %40 %34 3 3 %36 %int_128 %int_128 FlagIsProtected|FlagIsPrivate
%41 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %36 %40 %40
%42 = OpExtInst %void %1 DebugExpression
%43 = OpExtInst %void %1 DebugFunction %14 %41 %34 6 1 %35 %15 FlagIsProtected|FlagIsPrivate 7 %main
%44 = OpExtInst %void %1 DebugLocalVariable %16 %40 %34 6 16 %43 FlagIsLocal 0
%45 = OpExtInst %void %1 DebugLocalVariable %17 %40 %34 7 16 %43 FlagIsLocal 1
%46 = OpExtInst %void %1 DebugLocalVariable %18 %36 %34 8 3 %43 FlagIsLocal
OpLine %7 6 1
%main = OpFunction %void None %31
%47 = OpLabel
%60 = OpExtInst %void %1 DebugScope %43
OpLine %7 8 13
%vout = OpVariable %_ptr_Function_VS_OUTPUT Function
%49 = OpExtInst %void %1 DebugDeclare %46 %vout %42
OpLine %7 9 14
%50 = OpLoad %v4float %pos
OpLine %7 9 3
%51 = OpAccessChain %_ptr_Function_v4float %vout %int_0
%52 = OpExtInst %void %1 DebugValue %46 %51 %42 %int_0
OpStore %51 %50
OpLine %7 10 16
%53 = OpLoad %v4float %color
OpLine %7 10 3
%54 = OpAccessChain %_ptr_Function_v4float %vout %int_1
%55 = OpExtInst %void %1 DebugValue %46 %54 %42 %int_1
OpStore %54 %53
OpLine %7 11 10
%56 = OpLoad %VS_OUTPUT %vout
OpLine %7 11 3
%57 = OpCompositeExtract %v4float %56 0
OpStore %gl_Position %57
%58 = OpCompositeExtract %v4float %56 1
OpStore %out_var_COLOR %58
%61 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
)");
}

TEST(IrBuilder, ConsumeDebugInfoLexicalScopeInst) {
  // /* HLSL */
  //
  // float4 func2(float arg2) {   // func2_block
  //   return float4(arg2, 0, 0, 0);
  // }
  //
  // float4 func1(float arg1) {   // func1_block
  //   if (arg1 > 1) {       // if_true_block
  //     return float4(0, 0, 0, 0);
  //   }
  //   return func2(arg1);   // if_merge_block
  // }
  //
  // float4 main(float pos : POSITION) : SV_POSITION {  // main
  //   return func1(pos);
  // }
  DoRoundTripCheck(R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %pos %gl_Position
%5 = OpString "block/block.hlsl"
%6 = OpString "#line 1 \"block/block.hlsl\"
float4 func2(float arg2) {
  return float4(arg2, 0, 0, 0);
}

float4 func1(float arg1) {
  if (arg1 > 1) {
    return float4(0, 0, 0, 0);
  }
  return func2(arg1);
}

float4 main(float pos : POSITION) : SV_POSITION {
  return func1(pos);
}
"
OpSource HLSL 600 %5 "#line 1 \"block/block.hlsl\"
float4 func2(float arg2) {
  return float4(arg2, 0, 0, 0);
}

float4 func1(float arg1) {
  if (arg1 > 1) {
    return float4(0, 0, 0, 0);
  }
  return func2(arg1);
}

float4 main(float pos : POSITION) : SV_POSITION {
  return func1(pos);
}
"
%7 = OpString "float"
%8 = OpString "main"
%9 = OpString "v4f_main_f"
%10 = OpString "v4f_func1_f"
%11 = OpString "v4f_func2_f"
%12 = OpString "pos : POSITION"
%13 = OpString "func1"
%14 = OpString "func2"
OpName %main "main"
OpName %pos "pos"
OpName %bb_entry "bb.entry"
OpName %param_var_arg1 "param.var.arg1"
OpName %func1 "func1"
OpName %arg1 "arg1"
OpName %bb_entry_0 "bb.entry"
OpName %param_var_arg2 "param.var.arg2"
OpName %if_true "if.true"
OpName %if_merge "if.merge"
OpName %func2 "func2"
OpName %arg2 "arg2"
OpName %bb_entry_1 "bb.entry"
OpDecorate %gl_Position BuiltIn Position
OpDecorate %pos Location 0
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%int_32 = OpConstant %int 32
%v4float = OpTypeVector %float 4
%32 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%36 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%38 = OpTypeFunction %v4float %_ptr_Function_float
%bool = OpTypeBool
OpLine %5 12 25
%pos = OpVariable %_ptr_Input_float Input
OpLine %5 12 37
%gl_Position = OpVariable %_ptr_Output_v4float Output
%40 = OpExtInst %void %1 DebugSource %5 %6
%41 = OpExtInst %void %1 DebugCompilationUnit 2 4 %40 HLSL
%42 = OpExtInst %void %1 DebugTypeBasic %7 %int_32 Float
%43 = OpExtInst %void %1 DebugTypeVector %42 4
%44 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %43 %42
%45 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %43 %42
%46 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %43 %42
%47 = OpExtInst %void %1 DebugFunction %8 %44 %40 12 1 %41 %9 FlagIsProtected|FlagIsPrivate 13 %main
%48 = OpExtInst %void %1 DebugFunction %13 %45 %40 5 1 %41 %10 FlagIsProtected|FlagIsPrivate 13 %func1
%49 = OpExtInst %void %1 DebugFunction %14 %46 %40 1 1 %41 %11 FlagIsProtected|FlagIsPrivate 13 %func2
%50 = OpExtInst %void %1 DebugLexicalBlock %40 6 17 %48
%51 = OpExtInst %void %1 DebugLexicalBlock %40 9 3 %48
OpLine %5 12 1
%main = OpFunction %void None %36
%bb_entry = OpLabel
%70 = OpExtInst %void %1 DebugScope %47
OpLine %5 13 16
%param_var_arg1 = OpVariable %_ptr_Function_float Function
%53 = OpLoad %float %pos
OpStore %param_var_arg1 %53
OpLine %5 13 10
%54 = OpFunctionCall %v4float %func1 %param_var_arg1
OpLine %5 13 3
OpStore %gl_Position %54
%71 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
OpLine %5 5 1
%func1 = OpFunction %v4float None %38
OpLine %5 5 20
%arg1 = OpFunctionParameter %_ptr_Function_float
%bb_entry_0 = OpLabel
%72 = OpExtInst %void %1 DebugScope %48
OpLine %5 9 16
%param_var_arg2 = OpVariable %_ptr_Function_float Function
OpLine %5 6 7
%57 = OpLoad %float %arg1
OpLine %5 6 12
%58 = OpFOrdGreaterThan %bool %57 %float_1
OpLine %5 6 17
%73 = OpExtInst %void %1 DebugNoScope
OpSelectionMerge %if_merge None
OpBranchConditional %58 %if_true %if_merge
%if_true = OpLabel
%74 = OpExtInst %void %1 DebugScope %50
OpLine %5 7 5
%75 = OpExtInst %void %1 DebugNoScope
OpReturnValue %32
%if_merge = OpLabel
%76 = OpExtInst %void %1 DebugScope %51
OpLine %5 9 16
%63 = OpLoad %float %arg1
OpStore %param_var_arg2 %63
OpLine %5 9 10
%64 = OpFunctionCall %v4float %func2 %param_var_arg2
OpLine %5 9 3
%77 = OpExtInst %void %1 DebugNoScope
OpReturnValue %64
OpFunctionEnd
OpLine %5 1 1
%func2 = OpFunction %v4float None %38
OpLine %5 1 20
%arg2 = OpFunctionParameter %_ptr_Function_float
%bb_entry_1 = OpLabel
%78 = OpExtInst %void %1 DebugScope %49
OpLine %5 2 17
%67 = OpLoad %float %arg2
%68 = OpCompositeConstruct %v4float %67 %float_0 %float_0 %float_0
OpLine %5 2 3
%79 = OpExtInst %void %1 DebugNoScope
OpReturnValue %68
OpFunctionEnd
)");
}

TEST(IrBuilder, ConsumeDebugInlinedAt) {
  // /* HLSL */
  //
  // float4 func2(float arg2) {   // func2_block
  //   return float4(arg2, 0, 0, 0);
  // }
  //
  // float4 func1(float arg1) {   // func1_block
  //   if (arg1 > 1) {       // if_true_block
  //     return float4(0, 0, 0, 0);
  //   }
  //   return func2(arg1);   // if_merge_block
  // }
  //
  // float4 main(float pos : POSITION) : SV_POSITION {  // main
  //   return func1(pos);
  // }
  //
  // TODO(https://gitlab.khronos.org/spirv/SPIR-V/issues/533): In the following
  // SPIRV code, we use DebugInfoNone to reference opted-out function from
  // DebugFunction similar to opted-out global variable for DebugGlobalVariable,
  // but this is not a part of the spec yet. We are still in discussion and we
  // must correct it if our decision is different.
  DoRoundTripCheck(R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %pos %gl_Position
%5 = OpString "block/block.hlsl"
%6 = OpString "#line 1 \"block/block.hlsl\"
float4 func2(float arg2) {
  return float4(arg2, 0, 0, 0);
}

float4 func1(float arg1) {
  if (arg1 > 1) {
    return float4(0, 0, 0, 0);
  }
  return func2(arg1);
}

float4 main(float pos : POSITION) : SV_POSITION {
  return func1(pos);
}
"
OpSource HLSL 600 %5 "#line 1 \"block/block.hlsl\"
float4 func2(float arg2) {
  return float4(arg2, 0, 0, 0);
}

float4 func1(float arg1) {
  if (arg1 > 1) {
    return float4(0, 0, 0, 0);
  }
  return func2(arg1);
}

float4 main(float pos : POSITION) : SV_POSITION {
  return func1(pos);
}
"
%7 = OpString "float"
%8 = OpString "main"
%9 = OpString "v4f_main_f"
%10 = OpString "v4f_func1_f"
%11 = OpString "v4f_func2_f"
%12 = OpString "pos : POSITION"
%13 = OpString "func1"
%14 = OpString "func2"
OpName %main "main"
OpName %pos "pos"
OpName %bb_entry "bb.entry"
OpName %if_true "if.true"
OpName %if_merge "if.merge"
OpDecorate %gl_Position BuiltIn Position
OpDecorate %pos Location 0
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%int_32 = OpConstant %int 32
%v4float = OpTypeVector %float 4
%24 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%28 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%30 = OpTypeFunction %v4float %_ptr_Function_float
%bool = OpTypeBool
OpLine %5 12 25
%pos = OpVariable %_ptr_Input_float Input
OpLine %5 12 37
%gl_Position = OpVariable %_ptr_Output_v4float Output
%32 = OpExtInst %void %1 DebugInfoNone
%33 = OpExtInst %void %1 DebugSource %5 %6
%34 = OpExtInst %void %1 DebugCompilationUnit 2 4 %33 HLSL
%35 = OpExtInst %void %1 DebugTypeBasic %7 %int_32 Float
%36 = OpExtInst %void %1 DebugTypeVector %35 4
%37 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %36 %35
%38 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %36 %35
%39 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %36 %35
%40 = OpExtInst %void %1 DebugFunction %8 %37 %33 12 1 %34 %9 FlagIsProtected|FlagIsPrivate 13 %main
%41 = OpExtInst %void %1 DebugFunction %13 %38 %33 5 1 %34 %10 FlagIsProtected|FlagIsPrivate 13 %32
%42 = OpExtInst %void %1 DebugFunction %14 %39 %33 1 1 %34 %11 FlagIsProtected|FlagIsPrivate 13 %32
%43 = OpExtInst %void %1 DebugLexicalBlock %33 12 49 %40
%44 = OpExtInst %void %1 DebugLexicalBlock %33 5 26 %41
%45 = OpExtInst %void %1 DebugLexicalBlock %33 1 26 %42
%46 = OpExtInst %void %1 DebugLexicalBlock %33 6 17 %44
%47 = OpExtInst %void %1 DebugLexicalBlock %33 9 3 %44
%48 = OpExtInst %void %1 DebugInlinedAt 9 %47
%49 = OpExtInst %void %1 DebugInlinedAt 13 %43
%50 = OpExtInst %void %1 DebugInlinedAt 13 %43 %48
OpLine %5 12 1
%main = OpFunction %void None %28
%bb_entry = OpLabel
%62 = OpExtInst %void %1 DebugScope %44 %49
OpLine %5 6 7
%52 = OpLoad %float %pos
OpLine %5 6 12
%53 = OpFOrdGreaterThan %bool %52 %float_1
OpLine %5 6 17
%63 = OpExtInst %void %1 DebugNoScope
OpSelectionMerge %if_merge None
OpBranchConditional %53 %if_true %if_merge
%if_true = OpLabel
%64 = OpExtInst %void %1 DebugScope %46 %49
OpLine %5 7 5
OpStore %gl_Position %24
%65 = OpExtInst %void %1 DebugNoScope
OpReturn
%if_merge = OpLabel
%66 = OpExtInst %void %1 DebugScope %45 %50
OpLine %5 2 17
%58 = OpLoad %float %pos
OpLine %5 2 10
%59 = OpCompositeConstruct %v4float %58 %float_0 %float_0 %float_0
%67 = OpExtInst %void %1 DebugScope %43
OpLine %5 13 3
OpStore %gl_Position %59
%68 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
)");
}

TEST(IrBuilder, DebugInfoInstInFunctionOutOfBlock) {
  // /* HLSL */
  //
  // float4 func2(float arg2) {   // func2_block
  //   return float4(arg2, 0, 0, 0);
  // }
  //
  // float4 func1(float arg1) {   // func1_block
  //   if (arg1 > 1) {       // if_true_block
  //     return float4(0, 0, 0, 0);
  //   }
  //   return func2(arg1);   // if_merge_block
  // }
  //
  // float4 main(float pos : POSITION) : SV_POSITION {  // main
  //   return func1(pos);
  // }
  const std::string text = R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %pos %gl_Position
%5 = OpString "block/block.hlsl"
%6 = OpString "#line 1 \"block/block.hlsl\"
float4 func2(float arg2) {
  return float4(arg2, 0, 0, 0);
}

float4 func1(float arg1) {
  if (arg1 > 1) {
    return float4(0, 0, 0, 0);
  }
  return func2(arg1);
}

float4 main(float pos : POSITION) : SV_POSITION {
  return func1(pos);
}
"
OpSource HLSL 600 %5 "#line 1 \"block/block.hlsl\"
float4 func2(float arg2) {
  return float4(arg2, 0, 0, 0);
}

float4 func1(float arg1) {
  if (arg1 > 1) {
    return float4(0, 0, 0, 0);
  }
  return func2(arg1);
}

float4 main(float pos : POSITION) : SV_POSITION {
  return func1(pos);
}
"
%7 = OpString "float"
%8 = OpString "main"
%9 = OpString "v4f_main_f"
%10 = OpString "v4f_func1_f"
%11 = OpString "v4f_func2_f"
%12 = OpString "pos : POSITION"
%13 = OpString "func1"
%14 = OpString "func2"
OpName %main "main"
OpName %pos "pos"
OpName %bb_entry "bb.entry"
OpName %param_var_arg1 "param.var.arg1"
OpName %func1 "func1"
OpName %arg1 "arg1"
OpName %bb_entry_0 "bb.entry"
OpName %param_var_arg2 "param.var.arg2"
OpName %if_true "if.true"
OpName %if_merge "if.merge"
OpName %func2 "func2"
OpName %arg2 "arg2"
OpName %bb_entry_1 "bb.entry"
OpDecorate %gl_Position BuiltIn Position
OpDecorate %pos Location 0
%float = OpTypeFloat 32
%int = OpTypeInt 32 1
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%int_32 = OpConstant %int 32
%v4float = OpTypeVector %float 4
%32 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%36 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%38 = OpTypeFunction %v4float %_ptr_Function_float
%bool = OpTypeBool
OpLine %5 12 25
%pos = OpVariable %_ptr_Input_float Input
OpLine %5 12 37
%gl_Position = OpVariable %_ptr_Output_v4float Output
%40 = OpExtInst %void %1 DebugSource %5 %6
%41 = OpExtInst %void %1 DebugCompilationUnit 2 4 %40 HLSL
%42 = OpExtInst %void %1 DebugTypeBasic %7 %int_32 Float
%43 = OpExtInst %void %1 DebugTypeVector %42 4
%44 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %43 %42
%45 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %43 %42
%46 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %43 %42
%47 = OpExtInst %void %1 DebugFunction %8 %44 %40 12 1 %41 %9 FlagIsProtected|FlagIsPrivate 13 %main
%48 = OpExtInst %void %1 DebugFunction %13 %45 %40 5 1 %41 %10 FlagIsProtected|FlagIsPrivate 13 %func1
%49 = OpExtInst %void %1 DebugFunction %14 %46 %40 1 1 %41 %11 FlagIsProtected|FlagIsPrivate 13 %func2
%50 = OpExtInst %void %1 DebugLexicalBlock %40 6 17 %48
%51 = OpExtInst %void %1 DebugLexicalBlock %40 9 3 %48
OpLine %5 12 1
%main = OpFunction %void None %36
%bb_entry = OpLabel
%70 = OpExtInst %void %1 DebugScope %47
OpLine %5 13 16
%param_var_arg1 = OpVariable %_ptr_Function_float Function
%53 = OpLoad %float %pos
OpStore %param_var_arg1 %53
OpLine %5 13 10
%54 = OpFunctionCall %v4float %func1 %param_var_arg1
OpLine %5 13 3
OpStore %gl_Position %54
%71 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
OpLine %5 5 1
%func1 = OpFunction %v4float None %38
OpLine %5 5 20
%arg1 = OpFunctionParameter %_ptr_Function_float
%bb_entry_0 = OpLabel
%72 = OpExtInst %void %1 DebugScope %48
OpLine %5 9 16
%param_var_arg2 = OpVariable %_ptr_Function_float Function
OpLine %5 6 7
%57 = OpLoad %float %arg1
OpLine %5 6 12
%58 = OpFOrdGreaterThan %bool %57 %float_1
OpLine %5 6 17
%73 = OpExtInst %void %1 DebugNoScope
OpSelectionMerge %if_merge None
OpBranchConditional %58 %if_true %if_merge
%if_true = OpLabel
%74 = OpExtInst %void %1 DebugScope %50
OpLine %5 7 5
%75 = OpExtInst %void %1 DebugNoScope
OpReturnValue %32
%if_merge = OpLabel
%76 = OpExtInst %void %1 DebugScope %51
OpLine %5 9 16
%63 = OpLoad %float %arg1
OpStore %param_var_arg2 %63
OpLine %5 9 10
%64 = OpFunctionCall %v4float %func2 %param_var_arg2
OpLine %5 9 3
%77 = OpExtInst %void %1 DebugNoScope
OpReturnValue %64
OpFunctionEnd
OpLine %5 1 1
%func2 = OpFunction %v4float None %38
OpLine %5 1 20
%arg2 = OpFunctionParameter %_ptr_Function_float
%bb_entry_1 = OpLabel
%78 = OpExtInst %void %1 DebugScope %49
OpLine %5 2 17
%67 = OpLoad %float %arg2
%68 = OpCompositeConstruct %v4float %67 %float_0 %float_0 %float_0
OpLine %5 2 3
%79 = OpExtInst %void %1 DebugNoScope
OpReturnValue %68
OpFunctionEnd
)";

  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text);
  ASSERT_NE(nullptr, context);

  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, /* skip_nop = */ false);

  std::string disassembled_text;
  EXPECT_TRUE(t.Disassemble(binary, &disassembled_text));
  EXPECT_EQ(text, disassembled_text);
}

TEST(IrBuilder, DebugInfoInstInFunctionOutOfBlock2) {
  // /* HLSL */
  //
  // struct VS_OUTPUT {
  //   float4 pos : SV_POSITION;
  //   float4 color : COLOR;
  // };
  //
  // VS_OUTPUT main(float4 pos : POSITION,
  //                float4 color : COLOR) {
  //   VS_OUTPUT vout;
  //   vout.pos = pos;
  //   vout.color = color;
  //   return vout;
  // }
  const std::string text = R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %in_var_POSITION %in_var_COLOR %gl_Position %out_var_COLOR
%7 = OpString "vs.hlsl"
OpSource HLSL 600 %7 "#line 1 \"vs.hlsl\"
struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};

VS_OUTPUT main(float4 pos : POSITION,
               float4 color : COLOR) {
  VS_OUTPUT vout;
  vout.pos = pos;
  vout.color = color;
  return vout;
}
"
%8 = OpString "#line 1 \"vs.hlsl\"
struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};

VS_OUTPUT main(float4 pos : POSITION,
               float4 color : COLOR) {
  VS_OUTPUT vout;
  vout.pos = pos;
  vout.color = color;
  return vout;
}
"
%9 = OpString "VS_OUTPUT"
%10 = OpString "float"
%11 = OpString "src.main"
%12 = OpString "pos"
%13 = OpString "color"
%14 = OpString "vout"
OpName %in_var_POSITION "in.var.POSITION"
OpName %in_var_COLOR "in.var.COLOR"
OpName %out_var_COLOR "out.var.COLOR"
OpName %main "main"
OpName %param_var_pos "param.var.pos"
OpName %param_var_color "param.var.color"
OpName %VS_OUTPUT "VS_OUTPUT"
OpMemberName %VS_OUTPUT 0 "pos"
OpMemberName %VS_OUTPUT 1 "color"
OpName %src_main "src.main"
OpName %pos "pos"
OpName %color "color"
OpName %bb_entry "bb.entry"
OpName %vout "vout"
OpDecorate %gl_Position BuiltIn Position
OpDecorate %in_var_POSITION Location 0
OpDecorate %in_var_COLOR Location 1
OpDecorate %out_var_COLOR Location 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%uint_256 = OpConstant %uint 256
%uint_0 = OpConstant %uint 0
%uint_128 = OpConstant %uint 128
%36 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%VS_OUTPUT = OpTypeStruct %v4float %v4float
%38 = OpTypeFunction %VS_OUTPUT %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Function_VS_OUTPUT = OpTypePointer Function %VS_OUTPUT
OpLine %7 6 29
%in_var_POSITION = OpVariable %_ptr_Input_v4float Input
OpLine %7 7 31
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
OpLine %7 2 16
%gl_Position = OpVariable %_ptr_Output_v4float Output
OpLine %7 3 18
%out_var_COLOR = OpVariable %_ptr_Output_v4float Output
%40 = OpExtInst %void %1 DebugExpression
%41 = OpExtInst %void %1 DebugSource %7 %8
%42 = OpExtInst %void %1 DebugCompilationUnit 1 4 %41 HLSL
%43 = OpExtInst %void %1 DebugTypeComposite %9 Structure %41 1 1 %42 %9 %uint_256 FlagIsProtected|FlagIsPrivate %44 %45
%46 = OpExtInst %void %1 DebugTypeBasic %10 %uint_32 Float
%47 = OpExtInst %void %1 DebugTypeVector %46 4
%48 = OpExtInst %void %1 DebugTypeFunction FlagIsProtected|FlagIsPrivate %43 %47 %47
%49 = OpExtInst %void %1 DebugFunction %11 %48 %41 6 1 %42 %11 FlagIsProtected|FlagIsPrivate 7 %src_main
%50 = OpExtInst %void %1 DebugLocalVariable %12 %47 %41 6 23 %49 FlagIsLocal 0
%51 = OpExtInst %void %1 DebugLocalVariable %13 %47 %41 7 23 %49 FlagIsLocal 1
%52 = OpExtInst %void %1 DebugLexicalBlock %41 7 38 %49
%53 = OpExtInst %void %1 DebugLocalVariable %14 %43 %41 8 13 %52 FlagIsLocal
%44 = OpExtInst %void %1 DebugTypeMember %12 %47 %41 2 3 %43 %uint_0 %uint_128 FlagIsProtected|FlagIsPrivate
%45 = OpExtInst %void %1 DebugTypeMember %13 %47 %41 3 3 %43 %uint_128 %uint_128 FlagIsProtected|FlagIsPrivate
OpLine %7 6 1
%main = OpFunction %void None %36
%54 = OpLabel
%74 = OpExtInst %void %1 DebugScope %42
OpLine %7 6 23
%param_var_pos = OpVariable %_ptr_Function_v4float Function
OpLine %7 7 23
%param_var_color = OpVariable %_ptr_Function_v4float Function
OpLine %7 6 23
%56 = OpLoad %v4float %in_var_POSITION
OpStore %param_var_pos %56
OpLine %7 7 23
%57 = OpLoad %v4float %in_var_COLOR
OpStore %param_var_color %57
OpLine %7 6 1
%58 = OpFunctionCall %VS_OUTPUT %src_main %param_var_pos %param_var_color
OpLine %7 6 11
%59 = OpCompositeExtract %v4float %58 0
OpLine %7 2 16
OpStore %gl_Position %59
OpLine %7 6 11
%60 = OpCompositeExtract %v4float %58 1
OpLine %7 3 18
OpStore %out_var_COLOR %60
%75 = OpExtInst %void %1 DebugNoScope
OpReturn
OpFunctionEnd
OpLine %7 6 1
%src_main = OpFunction %VS_OUTPUT None %38
%76 = OpExtInst %void %1 DebugScope %49
OpLine %7 6 23
%pos = OpFunctionParameter %_ptr_Function_v4float
OpLine %7 7 23
%color = OpFunctionParameter %_ptr_Function_v4float
%63 = OpExtInst %void %1 DebugDeclare %50 %pos %40
%64 = OpExtInst %void %1 DebugDeclare %51 %color %40
%77 = OpExtInst %void %1 DebugNoScope
%bb_entry = OpLabel
%78 = OpExtInst %void %1 DebugScope %52
OpLine %7 8 13
%vout = OpVariable %_ptr_Function_VS_OUTPUT Function
%67 = OpExtInst %void %1 DebugDeclare %53 %vout %40
OpLine %7 9 14
%68 = OpLoad %v4float %pos
OpLine %7 9 3
%69 = OpAccessChain %_ptr_Function_v4float %vout %int_0
OpStore %69 %68
OpLine %7 10 16
%70 = OpLoad %v4float %color
OpLine %7 10 3
%71 = OpAccessChain %_ptr_Function_v4float %vout %int_1
OpStore %71 %70
OpLine %7 11 10
%72 = OpLoad %VS_OUTPUT %vout
OpLine %7 11 3
%79 = OpExtInst %void %1 DebugNoScope
OpReturnValue %72
OpFunctionEnd
)";

  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text);
  ASSERT_NE(nullptr, context);

  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, /* skip_nop = */ false);

  std::string disassembled_text;
  EXPECT_TRUE(t.Disassemble(binary, &disassembled_text));
  EXPECT_EQ(text, disassembled_text);
}

TEST(IrBuilder, DebugInfoForTerminationInsts) {
  // Check that DebugScope instructions for termination instructions are
  // preserved.
  DoRoundTripCheck(R"(OpCapability Shader
%1 = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%3 = OpString "simple_vs.hlsl"
OpSource HLSL 600 %3
OpName %main "main"
%void = OpTypeVoid
%5 = OpTypeFunction %void
%6 = OpExtInst %void %1 DebugSource %3
%7 = OpExtInst %void %1 DebugCompilationUnit 2 4 %6 HLSL
%main = OpFunction %void None %5
%8 = OpLabel
%20 = OpExtInst %void %1 DebugScope %7
OpBranch %10
%21 = OpExtInst %void %1 DebugNoScope
%10 = OpLabel
%22 = OpExtInst %void %1 DebugScope %7
OpKill
%23 = OpExtInst %void %1 DebugNoScope
%14 = OpLabel
%24 = OpExtInst %void %1 DebugScope %7
OpUnreachable
%25 = OpExtInst %void %1 DebugNoScope
%17 = OpLabel
%26 = OpExtInst %void %1 DebugScope %7
OpReturn
%27 = OpExtInst %void %1 DebugNoScope
OpFunctionEnd
)");
}

TEST(IrBuilder, LocalGlobalVariables) {
  // #version 310 es
  //
  // float gv1 = 10.;
  // float gv2 = 100.;
  //
  // float f() {
  //   float lv1 = gv1 + gv2;
  //   float lv2 = gv1 * gv2;
  //   return lv1 / lv2;
  // }
  //
  // void main() {
  //   float lv1 = gv1 - gv2;
  // }
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
               "OpSource ESSL 310\n"
               "OpName %main \"main\"\n"
               "OpName %f_ \"f(\"\n"
               "OpName %gv1 \"gv1\"\n"
               "OpName %gv2 \"gv2\"\n"
               "OpName %lv1 \"lv1\"\n"
               "OpName %lv2 \"lv2\"\n"
               "OpName %lv1_0 \"lv1\"\n"
       "%void = OpTypeVoid\n"
         "%10 = OpTypeFunction %void\n"
      "%float = OpTypeFloat 32\n"
         "%12 = OpTypeFunction %float\n"
 "%_ptr_Private_float = OpTypePointer Private %float\n"
        "%gv1 = OpVariable %_ptr_Private_float Private\n"
   "%float_10 = OpConstant %float 10\n"
        "%gv2 = OpVariable %_ptr_Private_float Private\n"
  "%float_100 = OpConstant %float 100\n"
 "%_ptr_Function_float = OpTypePointer Function %float\n"
       "%main = OpFunction %void None %10\n"
         "%17 = OpLabel\n"
      "%lv1_0 = OpVariable %_ptr_Function_float Function\n"
               "OpStore %gv1 %float_10\n"
               "OpStore %gv2 %float_100\n"
         "%18 = OpLoad %float %gv1\n"
         "%19 = OpLoad %float %gv2\n"
         "%20 = OpFSub %float %18 %19\n"
               "OpStore %lv1_0 %20\n"
               "OpReturn\n"
               "OpFunctionEnd\n"
         "%f_ = OpFunction %float None %12\n"
         "%21 = OpLabel\n"
        "%lv1 = OpVariable %_ptr_Function_float Function\n"
        "%lv2 = OpVariable %_ptr_Function_float Function\n"
         "%22 = OpLoad %float %gv1\n"
         "%23 = OpLoad %float %gv2\n"
         "%24 = OpFAdd %float %22 %23\n"
               "OpStore %lv1 %24\n"
         "%25 = OpLoad %float %gv1\n"
         "%26 = OpLoad %float %gv2\n"
         "%27 = OpFMul %float %25 %26\n"
               "OpStore %lv2 %27\n"
         "%28 = OpLoad %float %lv1\n"
         "%29 = OpLoad %float %lv2\n"
         "%30 = OpFDiv %float %28 %29\n"
               "OpReturnValue %30\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, OpUndefOutsideFunction) {
  // #version 310 es
  // void main() {}
  const std::string text =
      // clang-format off
               "OpMemoryModel Logical GLSL450\n"
        "%int = OpTypeInt 32 1\n"
       "%uint = OpTypeInt 32 0\n"
      "%float = OpTypeFloat 32\n"
          "%4 = OpUndef %int\n"
     "%int_10 = OpConstant %int 10\n"
          "%6 = OpUndef %uint\n"
       "%bool = OpTypeBool\n"
          "%8 = OpUndef %float\n"
     "%double = OpTypeFloat 64\n";
  // clang-format on

  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text);
  ASSERT_NE(nullptr, context);

  const auto opundef_count = std::count_if(
      context->module()->types_values_begin(),
      context->module()->types_values_end(), [](const Instruction& inst) {
        return inst.opcode() == spv::Op::OpUndef;
      });
  EXPECT_EQ(3, opundef_count);

  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, /* skip_nop = */ false);

  std::string disassembled_text;
  EXPECT_TRUE(t.Disassemble(binary, &disassembled_text));
  EXPECT_EQ(text, disassembled_text);
}

TEST(IrBuilder, OpUndefInBasicBlock) {
  DoRoundTripCheck(
      // clang-format off
               "OpMemoryModel Logical GLSL450\n"
               "OpName %main \"main\"\n"
       "%void = OpTypeVoid\n"
       "%uint = OpTypeInt 32 0\n"
     "%double = OpTypeFloat 64\n"
          "%5 = OpTypeFunction %void\n"
       "%main = OpFunction %void None %5\n"
          "%6 = OpLabel\n"
          "%7 = OpUndef %uint\n"
          "%8 = OpUndef %double\n"
               "OpReturn\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, KeepLineDebugInfoBeforeType) {
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
               "OpLine %1 1 1\n"
               "OpNoLine\n"
       "%void = OpTypeVoid\n"
               "OpLine %1 2 2\n"
          "%3 = OpTypeFunction %void\n");
  // clang-format on
}

TEST(IrBuilder, KeepLineDebugInfoBeforeLabel) {
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
       "%void = OpTypeVoid\n"
          "%3 = OpTypeFunction %void\n"
       "%4 = OpFunction %void None %3\n"
          "%5 = OpLabel\n"
   "OpBranch %6\n"
               "OpLine %1 1 1\n"
               "OpLine %1 2 2\n"
          "%6 = OpLabel\n"
               "OpBranch %7\n"
               "OpLine %1 100 100\n"
          "%7 = OpLabel\n"
               "OpReturn\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, KeepLineDebugInfoBeforeFunctionEnd) {
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
       "%void = OpTypeVoid\n"
          "%3 = OpTypeFunction %void\n"
       "%4 = OpFunction %void None %3\n"
               "OpLine %1 1 1\n"
               "OpLine %1 2 2\n"
               "OpFunctionEnd\n");
  // clang-format on
}

TEST(IrBuilder, KeepModuleProcessedInRightPlace) {
  DoRoundTripCheck(
      // clang-format off
               "OpCapability Shader\n"
               "OpMemoryModel Logical GLSL450\n"
          "%1 = OpString \"minimal.vert\"\n"
               "OpName %void \"void\"\n"
               "OpModuleProcessed \"Made it faster\"\n"
               "OpModuleProcessed \".. and smaller\"\n"
       "%void = OpTypeVoid\n");
  // clang-format on
}

// Checks the given |error_message| is reported when trying to build a module
// from the given |assembly|.
void DoErrorMessageCheck(const std::string& assembly,
                         const std::string& error_message, uint32_t line_num) {
  auto consumer = [error_message, line_num](spv_message_level_t, const char*,
                                            const spv_position_t& position,
                                            const char* m) {
    EXPECT_EQ(error_message, m);
    EXPECT_EQ(line_num, position.line);
  };

  SpirvTools t(SPV_ENV_UNIVERSAL_1_1);
  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, std::move(consumer), assembly);
  EXPECT_EQ(nullptr, context);
}

TEST(IrBuilder, FunctionInsideFunction) {
  DoErrorMessageCheck("%2 = OpFunction %1 None %3\n%5 = OpFunction %4 None %6",
                      "function inside function", 2);
}

TEST(IrBuilder, MismatchOpFunctionEnd) {
  DoErrorMessageCheck("OpFunctionEnd",
                      "OpFunctionEnd without corresponding OpFunction", 1);
}

TEST(IrBuilder, OpFunctionEndInsideBasicBlock) {
  DoErrorMessageCheck(
      "%2 = OpFunction %1 None %3\n"
      "%4 = OpLabel\n"
      "OpFunctionEnd",
      "OpFunctionEnd inside basic block", 3);
}

TEST(IrBuilder, BasicBlockOutsideFunction) {
  DoErrorMessageCheck("OpCapability Shader\n%1 = OpLabel",
                      "OpLabel outside function", 2);
}

TEST(IrBuilder, OpLabelInsideBasicBlock) {
  DoErrorMessageCheck(
      "%2 = OpFunction %1 None %3\n"
      "%4 = OpLabel\n"
      "%5 = OpLabel",
      "OpLabel inside basic block", 3);
}

TEST(IrBuilder, TerminatorOutsideFunction) {
  DoErrorMessageCheck("OpReturn", "terminator instruction outside function", 1);
}

TEST(IrBuilder, TerminatorOutsideBasicBlock) {
  DoErrorMessageCheck("%2 = OpFunction %1 None %3\nOpReturn",
                      "terminator instruction outside basic block", 2);
}

TEST(IrBuilder, NotAllowedInstAppearingInFunction) {
  DoErrorMessageCheck("%2 = OpFunction %1 None %3\n%5 = OpVariable %4 Function",
                      "Non-OpFunctionParameter (opcode: 59) found inside "
                      "function but outside basic block",
                      2);
}

TEST(IrBuilder, UniqueIds) {
  const std::string text =
      // clang-format off
               "OpCapability Shader\n"
          "%1 = OpExtInstImport \"GLSL.std.450\"\n"
               "OpMemoryModel Logical GLSL450\n"
               "OpEntryPoint Vertex %main \"main\"\n"
               "OpSource ESSL 310\n"
               "OpName %main \"main\"\n"
               "OpName %f_ \"f(\"\n"
               "OpName %gv1 \"gv1\"\n"
               "OpName %gv2 \"gv2\"\n"
               "OpName %lv1 \"lv1\"\n"
               "OpName %lv2 \"lv2\"\n"
               "OpName %lv1_0 \"lv1\"\n"
       "%void = OpTypeVoid\n"
         "%10 = OpTypeFunction %void\n"
      "%float = OpTypeFloat 32\n"
         "%12 = OpTypeFunction %float\n"
 "%_ptr_Private_float = OpTypePointer Private %float\n"
        "%gv1 = OpVariable %_ptr_Private_float Private\n"
   "%float_10 = OpConstant %float 10\n"
        "%gv2 = OpVariable %_ptr_Private_float Private\n"
  "%float_100 = OpConstant %float 100\n"
 "%_ptr_Function_float = OpTypePointer Function %float\n"
       "%main = OpFunction %void None %10\n"
         "%17 = OpLabel\n"
      "%lv1_0 = OpVariable %_ptr_Function_float Function\n"
               "OpStore %gv1 %float_10\n"
               "OpStore %gv2 %float_100\n"
         "%18 = OpLoad %float %gv1\n"
         "%19 = OpLoad %float %gv2\n"
         "%20 = OpFSub %float %18 %19\n"
               "OpStore %lv1_0 %20\n"
               "OpReturn\n"
               "OpFunctionEnd\n"
         "%f_ = OpFunction %float None %12\n"
         "%21 = OpLabel\n"
        "%lv1 = OpVariable %_ptr_Function_float Function\n"
        "%lv2 = OpVariable %_ptr_Function_float Function\n"
         "%22 = OpLoad %float %gv1\n"
         "%23 = OpLoad %float %gv2\n"
         "%24 = OpFAdd %float %22 %23\n"
               "OpStore %lv1 %24\n"
         "%25 = OpLoad %float %gv1\n"
         "%26 = OpLoad %float %gv2\n"
         "%27 = OpFMul %float %25 %26\n"
               "OpStore %lv2 %27\n"
         "%28 = OpLoad %float %lv1\n"
         "%29 = OpLoad %float %lv2\n"
         "%30 = OpFDiv %float %28 %29\n"
               "OpReturnValue %30\n"
               "OpFunctionEnd\n";
  // clang-format on

  std::unique_ptr<IRContext> context =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text);
  ASSERT_NE(nullptr, context);

  std::unordered_set<uint32_t> ids;
  context->module()->ForEachInst([&ids](const Instruction* inst) {
    EXPECT_TRUE(ids.insert(inst->unique_id()).second);
  });
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
