// Copyright (c) 2019 Google LLC.
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

#include <vector>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using DecomposeInitializedVariablesTest = PassTest<::testing::Test>;

std::string single_entry_header = R"(OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint Vertex %1 "shader"
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%4 = OpConstantNull %uint
%void = OpTypeVoid
%6 = OpTypeFunction %void
)";

std::string GetFunctionTest(std::string body) {
  auto result = single_entry_header;
  result += "%_ptr_Function_uint = OpTypePointer Function %uint\n";
  result += "%1 = OpFunction %void None %6\n";
  result += "%8 = OpLabel\n";
  result += body + "\n";
  result += "OpReturn\n";
  result += "OpFunctionEnd\n";
  return result;
}

TEST_F(DecomposeInitializedVariablesTest, FunctionChanged) {
  std::string input = "%9 = OpVariable %_ptr_Function_uint Function %uint_1";
  std::string expected = R"(%9 = OpVariable %_ptr_Function_uint Function
OpStore %9 %uint_1)";

  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      GetFunctionTest(input), GetFunctionTest(expected),
      /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, FunctionUnchanged) {
  std::string input = "%9 = OpVariable %_ptr_Function_uint Function";

  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      GetFunctionTest(input), GetFunctionTest(input), /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, FunctionMultipleVariables) {
  std::string input = R"(%9 = OpVariable %_ptr_Function_uint Function %uint_1
%10 = OpVariable %_ptr_Function_uint Function %4)";
  std::string expected = R"(%9 = OpVariable %_ptr_Function_uint Function
%10 = OpVariable %_ptr_Function_uint Function
OpStore %9 %uint_1
OpStore %10 %4)";

  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      GetFunctionTest(input), GetFunctionTest(expected),
      /* skip_nop = */ false);
}

std::string GetGlobalTest(std::string storage_class, bool initialized,
                          bool decomposed) {
  auto result = single_entry_header;

  result += "%_ptr_" + storage_class + "_uint = OpTypePointer " +
            storage_class + " %uint\n";
  if (initialized) {
    result += "%8 = OpVariable %_ptr_" + storage_class + "_uint " +
              storage_class + " %4\n";
  } else {
    result += "%8 = OpVariable %_ptr_" + storage_class + "_uint " +
              storage_class + "\n";
  }
  result += R"(%1 = OpFunction %void None %9
%9 = OpLabel
)";
  if (decomposed) result += "OpStore %8 %4\n";
  result += R"(OpReturn
OpFunctionEnd
)";
  return result;
}

TEST_F(DecomposeInitializedVariablesTest, PrivateChanged) {
  std::string input = GetGlobalTest("Private", true, false);
  std::string expected = GetGlobalTest("Private", false, true);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, expected, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, PrivateUnchanged) {
  std::string input = GetGlobalTest("Private", false, false);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, input, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, OutputChanged) {
  std::string input = GetGlobalTest("Output", true, false);
  std::string expected = GetGlobalTest("Output", false, true);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, expected, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, OutputUnchanged) {
  std::string input = GetGlobalTest("Output", false, false);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, input, /* skip_nop = */ false);
}

std::string multiple_entry_header = R"(OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint Vertex %1 "vertex"
OpEntryPoint Fragment %2 "fragment"
%uint = OpTypeInt 32 0
%4 = OpConstantNull %uint
%void = OpTypeVoid
%6 = OpTypeFunction %void
)";

std::string GetGlobalMultipleEntryTest(std::string storage_class,
                                       bool initialized, bool decomposed) {
  auto result = multiple_entry_header;
  result += "%_ptr_" + storage_class + "_uint = OpTypePointer " +
            storage_class + " %uint\n";
  if (initialized) {
    result += "%8 = OpVariable %_ptr_" + storage_class + "_uint " +
              storage_class + " %4\n";
  } else {
    result += "%8 = OpVariable %_ptr_" + storage_class + "_uint " +
              storage_class + "\n";
  }
  result += R"(%1 = OpFunction %void None %9
%9 = OpLabel
)";
  if (decomposed) result += "OpStore %8 %4\n";
  result += R"(OpReturn
OpFunctionEnd
%2 = OpFunction %void None %10
%10 = OpLabel
)";
  if (decomposed) result += "OpStore %8 %4\n";
  result += R"(OpReturn
OpFunctionEnd
)";

  return result;
}

TEST_F(DecomposeInitializedVariablesTest, PrivateMultipleEntryChanged) {
  std::string input = GetGlobalMultipleEntryTest("Private", true, false);
  std::string expected = GetGlobalMultipleEntryTest("Private", false, true);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, expected, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, PrivateMultipleEntryUnchanged) {
  std::string input = GetGlobalMultipleEntryTest("Private", false, false);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, input, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, OutputMultipleEntryChanged) {
  std::string input = GetGlobalMultipleEntryTest("Output", true, false);
  std::string expected = GetGlobalMultipleEntryTest("Output", false, true);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, expected, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, OutputMultipleEntryUnchanged) {
  std::string input = GetGlobalMultipleEntryTest("Output", false, false);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, input, /* skip_nop = */ false);
}

std::string GetGlobalWithNonEntryPointTest(std::string storage_class,
                                           bool initialized, bool decomposed) {
  auto result = single_entry_header;
  result += "%_ptr_" + storage_class + "_uint = OpTypePointer " +
            storage_class + " %uint\n";
  if (initialized) {
    result += "%8 = OpVariable %_ptr_" + storage_class + "_uint " +
              storage_class + " %4\n";
  } else {
    result += "%8 = OpVariable %_ptr_" + storage_class + "_uint " +
              storage_class + "\n";
  }
  result += R"(%1 = OpFunction %void None %9
%9 = OpLabel
)";
  if (decomposed) result += "OpStore %8 %4\n";
  result += R"(OpReturn
OpFunctionEnd
%10 = OpFunction %void None %11
%11 = OpLabel
OpReturn
OpFunctionEnd
)";

  return result;
}

TEST_F(DecomposeInitializedVariablesTest, PrivateWithNonEntryPointChanged) {
  std::string input = GetGlobalWithNonEntryPointTest("Private", true, false);
  std::string expected = GetGlobalWithNonEntryPointTest("Private", false, true);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, expected, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, PrivateWithNonEntryPointUnchanged) {
  std::string input = GetGlobalWithNonEntryPointTest("Private", false, false);
  //  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, input, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, OutputWithNonEntryPointChanged) {
  std::string input = GetGlobalWithNonEntryPointTest("Output", true, false);
  std::string expected = GetGlobalWithNonEntryPointTest("Output", false, true);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, expected, /* skip_nop = */ false);
}

TEST_F(DecomposeInitializedVariablesTest, OutputWithNonEntryPointUnchanged) {
  std::string input = GetGlobalWithNonEntryPointTest("Output", false, false);
  //  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<DecomposeInitializedVariablesPass>(
      input, input, /* skip_nop = */ false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
