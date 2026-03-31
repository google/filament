// Copyright 2025 The Khronos Group Inc.
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
#include "test/link/linker_fixture.h"

namespace spvtools {
namespace {

using FunctionVariants = spvtest::LinkerTest;

TEST_F(FunctionVariants, Dot4) {
  constexpr const char* const dot4_fp32 = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Int8
    OpCapability Int64
    OpCapability Linkage
    %1 = OpExtInstImport "OpenCL.std"
    OpMemoryModel Physical64 OpenCL
    OpEntryPoint Kernel %2 "dot4" %3
    OpExecutionMode %2 ContractionOff
    OpSource OpenCL_C 102000
    OpName %2 "dot4"
    OpName %3 "__spirv_BuiltInGlobalInvocationId"
    OpName %4 "entry"
    %5 = OpTypeFloat 32
    %6 = OpTypePointer CrossWorkgroup %5
    %7 = OpTypeInt 8 0
    %8 = OpTypePointer CrossWorkgroup %7
    %9 = OpTypeVoid
    %10 = OpTypeFunction %9 %6 %6 %8
    %11 = OpTypeInt 64 0
    %12 = OpTypeInt 32 0
    %13 = OpTypeVector %11 3
    %14 = OpTypePointer Input %13
    %15 = OpTypeVector %5 4
    %16 = OpConstant %11 30
    %17 = OpConstant %11 32
    %18 = OpConstant %12 2
    %3 = OpVariable %14 Input
    %2 = OpFunction %9 None %10
    %19 = OpFunctionParameter %6
    %20 = OpFunctionParameter %6
    %21 = OpFunctionParameter %8
    %4 = OpLabel
    %22 = OpLoad %13 %3 Aligned 1
    %23 = OpCompositeExtract %11 %22 0
    %24 = OpUConvert %12 %23
    %25 = OpShiftLeftLogical %12 %24 %18
    %26 = OpSConvert %11 %25
    %27 = OpExtInst %15 %1 vloadn %26 %19 4
    %28 = OpExtInst %15 %1 vloadn %26 %20 4
    %29 = OpDot %5 %27 %28
    %30 = OpShiftLeftLogical %11 %23 %17
    %31 = OpShiftRightArithmetic %11 %30 %16
    %32 = OpInBoundsPtrAccessChain %8 %21 %31
    %33 = OpBitcast %6 %32
    OpStore %33 %29 Aligned 4
    OpReturn
    OpFunctionEnd
  )";

  constexpr const char* const dot4_fp16 = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Int8
    OpCapability Int64
    OpCapability Float16
    OpCapability Linkage
    %1 = OpExtInstImport "OpenCL.std"
    OpMemoryModel Physical64 OpenCL
    OpEntryPoint Kernel %2 "dot4" %3
    OpExecutionMode %2 ContractionOff
    OpSource OpenCL_C 102000
    OpName %2 "dot4"
    OpName %3 "__spirv_BuiltInGlobalInvocationId"
    OpName %4 "entry"
    %5 = OpTypeFloat 16
    %6 = OpTypePointer CrossWorkgroup %5
    %7 = OpTypeInt 8 0
    %8 = OpTypePointer CrossWorkgroup %7
    %9 = OpTypeVoid
    %10 = OpTypeFunction %9 %6 %6 %8
    %11 = OpTypeInt 64 0
    %12 = OpTypeInt 32 0
    %13 = OpTypeVector %11 3
    %14 = OpTypePointer Input %13
    %15 = OpTypeVector %5 4
    %16 = OpConstant %11 31
    %17 = OpConstant %11 32
    %18 = OpConstant %12 2
    %3 = OpVariable %14 Input
    %2 = OpFunction %9 None %10
    %19 = OpFunctionParameter %6
    %20 = OpFunctionParameter %6
    %21 = OpFunctionParameter %8
    %4 = OpLabel
    %22 = OpLoad %13 %3 Aligned 1
    %23 = OpCompositeExtract %11 %22 0
    %24 = OpUConvert %12 %23
    %25 = OpShiftLeftLogical %12 %24 %18
    %26 = OpSConvert %11 %25
    %27 = OpExtInst %15 %1 vloadn %26 %19 4
    %28 = OpExtInst %15 %1 vloadn %26 %20 4
    %29 = OpDot %5 %27 %28
    %30 = OpShiftLeftLogical %11 %23 %17
    %31 = OpShiftRightArithmetic %11 %30 %16
    %32 = OpInBoundsPtrAccessChain %8 %21 %31
    %33 = OpBitcast %6 %32
    OpStore %33 %29 Aligned 4
    OpReturn
    OpFunctionEnd
  )";

  // clang-format off
  const std::vector<const char*> expected_lines = {
    "OpCapability FunctionVariantsINTEL",
    "OpCapability SpecConditionalINTEL",
    "OpCapability Kernel",
    "OpCapability Addresses",
    "OpCapability Int8",
    "OpCapability Int64",
    "OpCapability Linkage",
    "OpConditionalCapabilityINTEL %dot4_fp16_spv Float16",
    "OpExtension \"SPV_INTEL_function_variants\"",
    "OpConditionalEntryPointINTEL %dot4_fp32_spv Kernel %dot4 \"dot4\" %__spirv_BuiltInGlobalInvocationId",
    "OpConditionalEntryPointINTEL %dot4_fp16_spv Kernel %dot4_0 \"dot4\" %__spirv_BuiltInGlobalInvocationId_0",
    "OpModuleProcessed \"SPV_INTEL_function_variants registry version 0\"",
    "OpDecorate %ulong_30 ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %ulong_32 ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %uint_2 ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %__spirv_BuiltInGlobalInvocationId ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %dot4 ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %ulong_31 ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %ulong_32_0 ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %uint_2_0 ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %__spirv_BuiltInGlobalInvocationId_0 ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %dot4_0 ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %float ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %_ptr_CrossWorkgroup_float ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %18 ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %v4float ConditionalINTEL %dot4_fp32_spv",
    "OpDecorate %half ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %_ptr_CrossWorkgroup_half ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %22 ConditionalINTEL %dot4_fp16_spv",
    "OpDecorate %v4half ConditionalINTEL %dot4_fp16_spv",
    "%float = OpTypeFloat 32",
    "%_ptr_CrossWorkgroup_float = OpTypePointer CrossWorkgroup %float",
    "%18 = OpTypeFunction %void %_ptr_CrossWorkgroup_float %_ptr_CrossWorkgroup_float %_ptr_CrossWorkgroup_uchar",
    "%v4float = OpTypeVector %float 4",
    "%ulong_30 = OpConstant %ulong 30",
    "%ulong_32 = OpConstant %ulong 32",
    "%uint_2 = OpConstant %uint 2",
    "%__spirv_BuiltInGlobalInvocationId = OpVariable %_ptr_Input_v3ulong Input",
    "%bool = OpTypeBool",
    "%32 = OpSpecConstantArchitectureINTEL %bool 2 3 174 0",
    "%33 = OpSpecConstantTargetINTEL %bool 7",
    "%34 = OpSpecConstantTargetINTEL %bool 8",
    "%35 = OpSpecConstantCapabilitiesINTEL %bool Addresses Linkage Kernel Int64 Int8",
    "%36 = OpSpecConstantOp %bool LogicalOr %33 %34",
    "%37 = OpSpecConstantOp %bool LogicalAnd %35 %32",
    "%38 = OpSpecConstantOp %bool LogicalAnd %37 %36",
    "%half = OpTypeFloat 16",
    "%_ptr_CrossWorkgroup_half = OpTypePointer CrossWorkgroup %half",
    "%22 = OpTypeFunction %void %_ptr_CrossWorkgroup_half %_ptr_CrossWorkgroup_half %_ptr_CrossWorkgroup_uchar",
    "%v4half = OpTypeVector %half 4",
    "%ulong_31 = OpConstant %ulong 31",
    "%ulong_32_0 = OpConstant %ulong 32",
    "%uint_2_0 = OpConstant %uint 2",
    "%__spirv_BuiltInGlobalInvocationId_0 = OpVariable %_ptr_Input_v3ulong Input",
    "%39 = OpSpecConstantArchitectureINTEL %bool 2 3 174 4",
    "%40 = OpSpecConstantTargetINTEL %bool 7",
    "%41 = OpSpecConstantTargetINTEL %bool 8",
    "%42 = OpSpecConstantCapabilitiesINTEL %bool Addresses Linkage Kernel Float16 Int64 Int8",
    "%43 = OpSpecConstantOp %bool LogicalOr %40 %41",
    "%44 = OpSpecConstantOp %bool LogicalAnd %42 %39",
    "%dot4_fp16_spv = OpSpecConstantOp %bool LogicalAnd %44 %43",
    "%45 = OpSpecConstantOp %bool LogicalNot %dot4_fp16_spv",
    "%dot4_fp32_spv = OpSpecConstantOp %bool LogicalAnd %38 %45",
    "%dot4 = OpFunction %void None %18",
    "%56 = OpDot %float %54 %55",
    "%dot4_0 = OpFunction %void None %22",
    "%71 = OpDot %half %69 %70",
  };
  // clang-format on

  const std::string targets_csv =
      "module,target,features\n"
      "dot4_fp32.spv,7,\n"
      "dot4_fp32.spv,8,\n"
      "dot4_fp16.spv,7,\n"
      "dot4_fp16.spv,8,\n";
  const std::string architectures_csv =
      "module,category,family,op,architecture\n"
      "dot4_fp32.spv,2,3,174,0\n"
      "dot4_fp16.spv,2,3,174,4\n";
  const std::vector<std::string> sources = {dot4_fp32, dot4_fp16};
  const std::vector<std::string> in_files = {"dot4_fp32.spv", "dot4_fp16.spv"};

  LinkerOptions options;
  options.SetInFiles(in_files);
  options.SetFnVarTargetsCsv(targets_csv);
  options.SetFnVarArchitecturesCsv(architectures_csv);
  options.SetHasFnVarCapabilities(true);
  options.SetCreateLibrary(true);
  options.SetVerifyIds(true);

  spvtest::Binary linked_binary;
  spv_result_t res = AssembleAndLink(sources, &linked_binary, options);
  EXPECT_EQ(SPV_SUCCESS, res) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(), std::string());
  EXPECT_TRUE(Validate(linked_binary));

  std::string linked_asm;
  res = Disassemble(linked_binary, &linked_asm);
  EXPECT_EQ(SPV_SUCCESS, res) << GetErrorMessage();
  for (const auto& expected : expected_lines) {
    EXPECT_THAT(linked_asm, testing::HasSubstr(expected));
  }
}

TEST_F(FunctionVariants, FAddAsm) {
  constexpr const char* const foo_base = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Int8
    OpCapability Int64
    OpCapability Linkage
    %1 = OpExtInstImport "OpenCL.std"
    OpMemoryModel Physical64 OpenCL
    OpSource OpenCL_CPP 100000
    OpName %2 "foo"
    OpName %3 "add"
    OpName %4 "entry"
    OpName %5 "work"
    OpName %6 "call"
    OpName %7 "entry"
    OpDecorate %2 LinkageAttributes "foo" Export
    OpDecorate %5 LinkageAttributes "work" Export
    %8 = OpTypeFloat 32
    %9 = OpTypePointer Function %8
    %10 = OpTypeFunction %8 %9 %9 %9
    %11 = OpTypeInt 8 0
    %12 = OpTypePointer Function %11
    %13 = OpTypeInt 64 0
    %14 = OpConstant %13 28
    %15 = OpConstant %13 24
    %16 = OpConstant %13 20
    %17 = OpConstant %13 16
    %18 = OpConstant %13 12
    %19 = OpConstant %13 8
    %20 = OpConstant %13 4
    %2 = OpFunction %8 DontInline %10
    %21 = OpFunctionParameter %9
    %22 = OpFunctionParameter %9
    %23 = OpFunctionParameter %9
    %4 = OpLabel
    %24 = OpLoad %8 %21 Aligned 4
    %25 = OpLoad %8 %22 Aligned 4
    %3 = OpFAdd %8 %24 %25
    OpStore %23 %3 Aligned 4
    OpReturnValue %3
    OpFunctionEnd
    %5 = OpFunction %8 None %10
    %26 = OpFunctionParameter %9
    %27 = OpFunctionParameter %9
    %28 = OpFunctionParameter %9
    %7 = OpLabel
    %6 = OpFunctionCall %8 %2 %26 %27 %28
    %29 = OpFDiv %8 %6 %6
    %30 = OpFDiv %8 %29 %6
    OpReturnValue %30
    OpFunctionEnd
  )";

  constexpr const char* const foo_asm = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability AsmINTEL
    OpExtension "SPV_INTEL_inline_assembly"
    %1 = OpExtInstImport "OpenCL.std"
    OpMemoryModel Physical64 OpenCL
    OpSource OpenCL_CPP 100000
    OpName %2 "a"
    OpName %3 "b"
    OpName %4 "c"
    OpName %5 "foo"
    OpName %6 "add"
    OpName %7 "entry"
    OpDecorate %2 FuncParamAttr NoWrite
    OpDecorate %2 FuncParamAttr NoAlias
    OpDecorate %3 FuncParamAttr NoWrite
    OpDecorate %3 FuncParamAttr NoAlias
    OpDecorate %4 FuncParamAttr NoAlias
    OpDecorate %5 LinkageAttributes "foo" Export
    OpDecorate %8 SideEffectsINTEL
    %9 = OpTypeFloat 32
    %10 = OpTypePointer Function %9
    %11 = OpTypeFunction %9 %10 %10 %10
    %12 = OpTypeVoid
    %13 = OpTypeFunction %12
    %14 = OpAsmTargetINTEL "spirv64-unknown-unknown"
    %8 = OpAsmINTEL %12 %13 %14 "nop1" ""
    %5 = OpFunction %9 None %11
    %2 = OpFunctionParameter %10
    %3 = OpFunctionParameter %10
    %4 = OpFunctionParameter %10
    %7 = OpLabel
    %15 = OpLoad %9 %2 Aligned 4
    %16 = OpLoad %9 %3 Aligned 4
    %6 = OpFSub %9 %15 %16
    OpStore %4 %6 Aligned 4
    %17 = OpAsmCallINTEL %12 %8
    OpReturnValue %6
    OpFunctionEnd
	)";

  // same as foo_asm, just with OpFMul and a different assembly string in
  // OpAsmINTEL
  constexpr const char* const foo_asm2 = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability AsmINTEL
    OpExtension "SPV_INTEL_inline_assembly"
    %1 = OpExtInstImport "OpenCL.std"
    OpMemoryModel Physical64 OpenCL
    OpSource OpenCL_CPP 100000
    OpName %2 "a"
    OpName %3 "b"
    OpName %4 "c"
    OpName %5 "foo"
    OpName %6 "add"
    OpName %7 "entry"
    OpDecorate %2 FuncParamAttr NoWrite
    OpDecorate %2 FuncParamAttr NoAlias
    OpDecorate %3 FuncParamAttr NoWrite
    OpDecorate %3 FuncParamAttr NoAlias
    OpDecorate %4 FuncParamAttr NoAlias
    OpDecorate %5 LinkageAttributes "foo" Export
    OpDecorate %8 SideEffectsINTEL
    %9 = OpTypeFloat 32
    %10 = OpTypePointer Function %9
    %11 = OpTypeFunction %9 %10 %10 %10
    %12 = OpTypeVoid
    %13 = OpTypeFunction %12
    %14 = OpAsmTargetINTEL "spirv64-unknown-unknown"
    %8 = OpAsmINTEL %12 %13 %14 "nop2" ""
    %5 = OpFunction %9 None %11
    %2 = OpFunctionParameter %10
    %3 = OpFunctionParameter %10
    %4 = OpFunctionParameter %10
    %7 = OpLabel
    %15 = OpLoad %9 %2 Aligned 4
    %16 = OpLoad %9 %3 Aligned 4
    %6 = OpFMul %9 %15 %16
    OpStore %4 %6 Aligned 4
    %17 = OpAsmCallINTEL %12 %8
    OpReturnValue %6
    OpFunctionEnd
  )";

  // clang-format off
  const std::vector<const char*> expected_lines = {
    "OpCapability FunctionVariantsINTEL",
    "OpCapability SpecConditionalINTEL",
    "OpCapability Kernel",
    "OpCapability Addresses",
    "OpConditionalCapabilityINTEL %foo_spv Int8",
    "OpConditionalCapabilityINTEL %foo_spv Int64",
    "OpCapability Linkage",
    "OpConditionalCapabilityINTEL %2 AsmINTEL",
    "OpExtension \"SPV_INTEL_function_variants\"",
    "OpConditionalExtensionINTEL %2 \"SPV_INTEL_inline_assembly\"",
    "OpModuleProcessed \"SPV_INTEL_function_variants registry version 0\"",
    "OpDecorate %foo LinkageAttributes \"foo\" Export",
    "OpDecorate %work LinkageAttributes \"work\" Export",
    "OpDecorate %ulong_28 ConditionalINTEL %foo_spv",
    "OpDecorate %ulong_24 ConditionalINTEL %foo_spv",
    "OpDecorate %ulong_20 ConditionalINTEL %foo_spv",
    "OpDecorate %ulong_16 ConditionalINTEL %foo_spv",
    "OpDecorate %ulong_12 ConditionalINTEL %foo_spv",
    "OpDecorate %ulong_8 ConditionalINTEL %foo_spv",
    "OpDecorate %ulong_4 ConditionalINTEL %foo_spv",
    "OpDecorate %foo ConditionalINTEL %foo_spv",
    "OpDecorate %foo_0 LinkageAttributes \"foo\" Export",
    "OpDecorate %32 ConditionalINTEL %foo_asm_spv",
    "OpDecorate %31 ConditionalINTEL %foo_asm_spv",
    "OpDecorate %foo_0 ConditionalINTEL %foo_asm_spv",
    "OpDecorate %foo_1 LinkageAttributes \"foo\" Export",
    "OpDecorate %34 ConditionalINTEL %foo_asm2_spv",
    "OpDecorate %33 ConditionalINTEL %foo_asm2_spv",
    "OpDecorate %foo_1 ConditionalINTEL %foo_asm2_spv",
    "OpDecorate %call ConditionalINTEL %foo_spv",
    "OpDecorate %35 ConditionalINTEL %foo_asm_spv",
    "OpDecorate %36 ConditionalINTEL %foo_asm2_spv",
    "OpDecorate %uchar ConditionalINTEL %foo_spv",
    "OpDecorate %_ptr_Function_uchar ConditionalINTEL %foo_spv",
    "OpDecorate %ulong ConditionalINTEL %foo_spv",
    "OpDecorate %void ConditionalINTEL %2",
    "OpDecorate %41 ConditionalINTEL %2",
    "%ulong_28 = OpConstant %ulong 28",
    "%ulong_24 = OpConstant %ulong 24",
    "%ulong_20 = OpConstant %ulong 20",
    "%ulong_16 = OpConstant %ulong 16",
    "%ulong_12 = OpConstant %ulong 12",
    "%ulong_8 = OpConstant %ulong 8",
    "%ulong_4 = OpConstant %ulong 4",
    "%bool = OpTypeBool",
    "%46 = OpSpecConstantTargetINTEL %bool 4",
    "%47 = OpSpecConstantCapabilitiesINTEL %bool Addresses Linkage Kernel Int64 Int8",
    "%48 = OpSpecConstantOp %bool LogicalAnd %47 %46",
    "%31 = OpAsmINTEL %void %41 %32 \"nop1\" \"\"",
    "%49 = OpSpecConstantArchitectureINTEL %bool 1 1 170 1",
    "%50 = OpSpecConstantTargetINTEL %bool 4 9 10",
    "%51 = OpSpecConstantCapabilitiesINTEL %bool Addresses Linkage Kernel AsmINTEL",
    "%52 = OpSpecConstantOp %bool LogicalAnd %51 %49",
    "%foo_asm_spv = OpSpecConstantOp %bool LogicalAnd %52 %50",
    "%33 = OpAsmINTEL %void %41 %34 \"nop2\" \"\"",
    "%53 = OpSpecConstantArchitectureINTEL %bool 1 7 174 1",
    "%54 = OpSpecConstantArchitectureINTEL %bool 1 7 178 3",
    "%55 = OpSpecConstantArchitectureINTEL %bool 1 8 170 1",
    "%56 = OpSpecConstantArchitectureINTEL %bool 1 9 174 1",
    "%57 = OpSpecConstantArchitectureINTEL %bool 1 9 178 3",
    "%58 = OpSpecConstantTargetINTEL %bool 5 2 4 5",
    "%59 = OpSpecConstantTargetINTEL %bool 6 2 4 5",
    "%60 = OpSpecConstantCapabilitiesINTEL %bool Addresses Linkage Kernel AsmINTEL",
    "%61 = OpSpecConstantOp %bool LogicalAnd %53 %54",
    "%62 = OpSpecConstantOp %bool LogicalAnd %56 %57",
    "%63 = OpSpecConstantOp %bool LogicalOr %61 %55",
    "%64 = OpSpecConstantOp %bool LogicalOr %63 %62",
    "%65 = OpSpecConstantOp %bool LogicalOr %58 %59",
    "%66 = OpSpecConstantOp %bool LogicalAnd %60 %64",
    "%foo_asm2_spv = OpSpecConstantOp %bool LogicalAnd %66 %65",
    "%67 = OpSpecConstantOp %bool LogicalOr %foo_asm_spv %foo_asm2_spv",
    "%68 = OpSpecConstantOp %bool LogicalNot %67",
    "%foo_spv = OpSpecConstantOp %bool LogicalAnd %48 %68",
    "%2 = OpSpecConstantOp %bool LogicalOr %foo_asm_spv %foo_asm2_spv",
    "%foo = OpFunction %float DontInline %44",
    "%add = OpFAdd %float %72 %73",
    "%work = OpFunction %float None %44",
    "%call = OpFunctionCall %float %foo %74 %75 %76",
    "%35 = OpFunctionCall %float %foo_0 %74 %75 %76",
    "%36 = OpFunctionCall %float %foo_1 %74 %75 %76",
    "%77 = OpConditionalCopyObjectINTEL %float %foo_spv %call %foo_asm_spv %35 %foo_asm2_spv %36",
    "%78 = OpFDiv %float %77 %77",
    "%79 = OpFDiv %float %78 %77",
    "OpReturnValue %79",
    "%foo_0 = OpFunction %float None %44",
    "%add_0 = OpFSub %float %80 %81",
    "%82 = OpAsmCallINTEL %void %31",
    "%foo_1 = OpFunction %float None %44",
    "%add_1 = OpFMul %float %83 %84",
    "%85 = OpAsmCallINTEL %void %33",
  };
  // clang-format on

  const std::string targets_csv =
      "module,target,features\n"
      "foo.spv,04,\n"           // test leading zeros
      "foo_asm.spv,4,9/0010\n"  // test leading zeros
      "foo_asm2.spv,5,2/4/5\n"
      "foo_asm2.spv,6,2/4/5\n";
  const std::string architectures_csv =
      "module,category,family,op,architecture\n"
      "foo_asm.spv,1,1,170,1\n"
      "foo_asm2.spv,1,7,174,1\n"
      "foo_asm2.spv,1,7,178,3\n"
      "foo_asm2.spv,1,8,170,1\n"
      "foo_asm2.spv,1,9,174,1\n"
      "foo_asm2.spv,1,9,178,3\n";
  const std::vector<std::string> sources = {foo_base, foo_asm, foo_asm2};
  const std::vector<std::string> in_files = {"foo.spv", "foo_asm.spv",
                                             "foo_asm2.spv"};

  LinkerOptions options;
  options.SetInFiles(in_files);
  options.SetFnVarTargetsCsv(targets_csv);
  options.SetFnVarArchitecturesCsv(architectures_csv);
  options.SetHasFnVarCapabilities(true);
  options.SetCreateLibrary(true);
  options.SetVerifyIds(true);

  spvtest::Binary linked_binary;
  spv_result_t res = AssembleAndLink(sources, &linked_binary, options);
  EXPECT_EQ(SPV_SUCCESS, res) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(), std::string());
  EXPECT_TRUE(Validate(linked_binary));

  std::string linked_asm;
  res = Disassemble(linked_binary, &linked_asm);
  EXPECT_EQ(SPV_SUCCESS, res) << GetErrorMessage();
  for (const auto& expected : expected_lines) {
    EXPECT_THAT(linked_asm, testing::HasSubstr(expected));
  }
}

TEST_F(FunctionVariants, InvalidNumber1) {
  const std::string targets_csv =
      "module,target,features\n"
      "foo.spv,-4,9/10\n";
  const std::string architectures_csv = "";
  const std::vector<std::string> sources = {""};
  const std::vector<std::string> in_files = {"foo.spv"};

  LinkerOptions options;
  options.SetInFiles(in_files);
  options.SetFnVarTargetsCsv(targets_csv);
  options.SetFnVarArchitecturesCsv(architectures_csv);
  options.SetHasFnVarCapabilities(true);
  options.SetCreateLibrary(true);
  options.SetVerifyIds(true);

  spvtest::Binary linked_binary;
  spv_result_t res = AssembleAndLink(sources, &linked_binary, options);
  EXPECT_EQ(SPV_ERROR_FNVAR, res) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(), "ERROR: 0: Error converting -4 to target.");
}

TEST_F(FunctionVariants, InvalidNumber2) {
  const std::string targets_csv =
      "module,target,features\n"
      "foo.spv,4,9/-10\n";
  const std::string architectures_csv = "";
  const std::vector<std::string> sources = {""};
  const std::vector<std::string> in_files = {"foo.spv"};

  LinkerOptions options;
  options.SetInFiles(in_files);
  options.SetFnVarTargetsCsv(targets_csv);
  options.SetFnVarArchitecturesCsv(architectures_csv);
  options.SetHasFnVarCapabilities(true);
  options.SetCreateLibrary(true);
  options.SetVerifyIds(true);

  spvtest::Binary linked_binary;
  spv_result_t res = AssembleAndLink(sources, &linked_binary, options);
  EXPECT_EQ(SPV_ERROR_FNVAR, res) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(),
              "ERROR: 0: Error converting -10 in 9/-10 to target feature.");
}

TEST_F(FunctionVariants, InvalidNumber3) {
  const std::string targets_csv =
      "module,target,features\n"
      "foo.spv,4.0,9/10\n";
  const std::string architectures_csv = "";
  const std::vector<std::string> sources = {""};
  const std::vector<std::string> in_files = {"foo.spv"};

  LinkerOptions options;
  options.SetInFiles(in_files);
  options.SetFnVarTargetsCsv(targets_csv);
  options.SetFnVarArchitecturesCsv(architectures_csv);
  options.SetHasFnVarCapabilities(true);
  options.SetCreateLibrary(true);
  options.SetVerifyIds(true);

  spvtest::Binary linked_binary;
  spv_result_t res = AssembleAndLink(sources, &linked_binary, options);
  EXPECT_EQ(SPV_ERROR_FNVAR, res) << GetErrorMessage();
  EXPECT_THAT(GetErrorMessage(), "ERROR: 0: Error converting 4.0 to target.");
}

}  // namespace
}  // namespace spvtools
