// Copyright (c) 2023 Google LLC.
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

#include "tools/objdump/extract_source.h"

#include <gtest/gtest.h>

#include <string>

#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"
#include "spirv-tools/libspirv.hpp"
#include "tools/util/cli_consumer.h"

namespace {

constexpr auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;

std::pair<bool, std::unordered_map<std::string, std::string>> ExtractSource(
    const std::string& spv_source) {
  std::unique_ptr<spvtools::opt::IRContext> ctx = spvtools::BuildModule(
      kDefaultEnvironment, spvtools::utils::CLIMessageConsumer, spv_source,
      spvtools::SpirvTools::kDefaultAssembleOption |
          SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  std::vector<uint32_t> binary;
  ctx->module()->ToBinary(&binary, /* skip_nop = */ false);
  std::unordered_map<std::string, std::string> output;
  bool result = ExtractSourceFromModule(binary, &output);
  return std::make_pair(result, std::move(output));
}

}  // namespace

TEST(ExtractSourceTest, no_debug) {
  std::string source = R"(
           OpCapability Shader
           OpCapability Linkage
           OpMemoryModel Logical GLSL450
   %void = OpTypeVoid
      %2 = OpTypeFunction %void
   %bool = OpTypeBool
      %4 = OpUndef %bool
      %5 = OpFunction %void None %2
      %6 = OpLabel
           OpReturn
           OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 0);
}

TEST(ExtractSourceTest, SimpleSource) {
  std::string source = R"(
      OpCapability Shader
      OpMemoryModel Logical GLSL450
      OpEntryPoint GLCompute %1 "compute_1"
      OpExecutionMode %1 LocalSize 1 1 1
 %2 = OpString "compute.hlsl"
      OpSource HLSL 660 %2 "[numthreads(1, 1, 1)] void compute_1(){ }"
      OpName %1 "compute_1"
 %3 = OpTypeVoid
 %4 = OpTypeFunction %3
 %1 = OpFunction %3 None %4
 %5 = OpLabel
      OpLine %2 1 41
      OpReturn
      OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 1);
  ASSERT_TRUE(result["compute.hlsl"] ==
              "[numthreads(1, 1, 1)] void compute_1(){ }");
}

TEST(ExtractSourceTest, SourceContinued) {
  std::string source = R"(
      OpCapability Shader
      OpMemoryModel Logical GLSL450
      OpEntryPoint GLCompute %1 "compute_1"
      OpExecutionMode %1 LocalSize 1 1 1
 %2 = OpString "compute.hlsl"
      OpSource HLSL 660 %2 "[numthreads(1, 1, 1)] "
      OpSourceContinued "void compute_1(){ }"
      OpName %1 "compute_1"
 %3 = OpTypeVoid
 %4 = OpTypeFunction %3
 %1 = OpFunction %3 None %4
 %5 = OpLabel
      OpLine %2 1 41
      OpReturn
      OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 1);
  ASSERT_TRUE(result["compute.hlsl"] ==
              "[numthreads(1, 1, 1)] void compute_1(){ }");
}

TEST(ExtractSourceTest, OnlyFilename) {
  std::string source = R"(
      OpCapability Shader
      OpMemoryModel Logical GLSL450
      OpEntryPoint GLCompute %1 "compute_1"
      OpExecutionMode %1 LocalSize 1 1 1
 %2 = OpString "compute.hlsl"
      OpSource HLSL 660 %2
      OpName %1 "compute_1"
 %3 = OpTypeVoid
 %4 = OpTypeFunction %3
 %1 = OpFunction %3 None %4
 %5 = OpLabel
      OpLine %2 1 41
      OpReturn
      OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 1);
  ASSERT_TRUE(result["compute.hlsl"] == "");
}

TEST(ExtractSourceTest, MultipleFiles) {
  std::string source = R"(
      OpCapability Shader
      OpMemoryModel Logical GLSL450
      OpEntryPoint GLCompute %1 "compute_1"
      OpExecutionMode %1 LocalSize 1 1 1
 %2 = OpString "compute1.hlsl"
 %3 = OpString "compute2.hlsl"
      OpSource HLSL 660 %2 "some instruction"
      OpSource HLSL 660 %3 "some other instruction"
      OpName %1 "compute_1"
 %4 = OpTypeVoid
 %5 = OpTypeFunction %4
 %1 = OpFunction %4 None %5
 %6 = OpLabel
      OpLine %2 1 41
      OpReturn
      OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 2);
  ASSERT_TRUE(result["compute1.hlsl"] == "some instruction");
  ASSERT_TRUE(result["compute2.hlsl"] == "some other instruction");
}

TEST(ExtractSourceTest, MultilineCode) {
  std::string source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "compute_1"
               OpExecutionMode %1 LocalSize 1 1 1
          %2 = OpString "compute.hlsl"
               OpSource HLSL 660 %2 "[numthreads(1, 1, 1)]
void compute_1() {
}
"
               OpName %1 "compute_1"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %1 = OpFunction %3 None %4
          %5 = OpLabel
               OpLine %2 3 1
               OpReturn
               OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 1);
  ASSERT_TRUE(result["compute.hlsl"] ==
              "[numthreads(1, 1, 1)]\nvoid compute_1() {\n}\n");
}

TEST(ExtractSourceTest, EmptyFilename) {
  std::string source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "compute_1"
               OpExecutionMode %1 LocalSize 1 1 1
          %2 = OpString ""
               OpSource HLSL 660 %2 "void compute(){}"
               OpName %1 "compute_1"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %1 = OpFunction %3 None %4
          %5 = OpLabel
               OpLine %2 3 1
               OpReturn
               OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 1);
  ASSERT_TRUE(result["unnamed-0.hlsl"] == "void compute(){}");
}

TEST(ExtractSourceTest, EscapeEscaped) {
  std::string source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "compute"
               OpExecutionMode %1 LocalSize 1 1 1
          %2 = OpString "compute.hlsl"
               OpSource HLSL 660 %2 "// check \" escape removed"
               OpName %1 "compute"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %1 = OpFunction %3 None %4
          %5 = OpLabel
               OpLine %2 6 1
               OpReturn
               OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 1);
  ASSERT_TRUE(result["compute.hlsl"] == "// check \" escape removed");
}

TEST(ExtractSourceTest, OpSourceWithNoSource) {
  std::string source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "compute"
               OpExecutionMode %1 LocalSize 1 1 1
          %2 = OpString "compute.hlsl"
               OpSource HLSL 660 %2
               OpName %1 "compute"
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %1 = OpFunction %3 None %4
          %5 = OpLabel
               OpLine %2 6 1
               OpReturn
               OpFunctionEnd
  )";

  auto[success, result] = ExtractSource(source);
  ASSERT_TRUE(success);
  ASSERT_TRUE(result.size() == 1);
  ASSERT_TRUE(result["compute.hlsl"] == "");
}
