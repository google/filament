// Copyright (c) 2017 Google Inc.
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

#include <memory>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/opt/pass.h"
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

class DummyPass : public Pass {
 public:
  const char* name() const override { return "dummy-pass"; }
  Status Process() override { return Status::SuccessWithoutChange; }
};

using ::testing::UnorderedElementsAre;
using PassClassTest = PassTest<::testing::Test>;

TEST_F(PassClassTest, BasicVisitFromEntryPoint) {
  // Make sure we visit the entry point, and the function it calls.
  // Do not visit Dead or Exported.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main"
               OpName %10 "main"
               OpName %Dead "Dead"
               OpName %11 "Constant"
               OpName %ExportedFunc "ExportedFunc"
               OpDecorate %ExportedFunc LinkageAttributes "ExportedFunc" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
         %10 = OpFunction %void None %6
         %14 = OpLabel
         %15 = OpFunctionCall %void %11
         %16 = OpFunctionCall %void %11
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %void None %6
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
       %Dead = OpFunction %void None %6
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
%ExportedFunc = OpFunction %void None %7
         %20 = OpLabel
         %21 = OpFunctionCall %void %11
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;
  DummyPass testPass;
  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  testPass.ProcessEntryPointCallTree(mark_visited, localContext->module());
  EXPECT_THAT(processed, UnorderedElementsAre(10, 11));
}

TEST_F(PassClassTest, BasicVisitReachable) {
  // Make sure we visit the entry point, exported function, and the function
  // they call. Do not visit Dead.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main"
               OpName %10 "main"
               OpName %Dead "Dead"
               OpName %11 "Constant"
               OpName %12 "ExportedFunc"
               OpName %13 "Constant2"
               OpDecorate %12 LinkageAttributes "ExportedFunc" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
         %10 = OpFunction %void None %6
         %14 = OpLabel
         %15 = OpFunctionCall %void %11
         %16 = OpFunctionCall %void %11
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %void None %6
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
       %Dead = OpFunction %void None %6
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %void None %9
         %20 = OpLabel
         %21 = OpFunctionCall %void %13
               OpReturn
               OpFunctionEnd
         %13 = OpFunction %void None %6
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;

  DummyPass testPass;
  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  testPass.ProcessReachableCallTree(mark_visited, localContext.get());
  EXPECT_THAT(processed, UnorderedElementsAre(10, 11, 12, 13));
}

TEST_F(PassClassTest, BasicVisitOnlyOnce) {
  // Make sure we visit %11 only once, even if it is called from two different
  // functions.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main" %gl_FragColor
               OpName %10 "main"
               OpName %Dead "Dead"
               OpName %11 "Constant"
               OpName %12 "ExportedFunc"
               OpDecorate %12 LinkageAttributes "ExportedFunc" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
         %10 = OpFunction %void None %6
         %14 = OpLabel
         %15 = OpFunctionCall %void %11
         %16 = OpFunctionCall %void %12
               OpReturn
               OpFunctionEnd
         %11 = OpFunction %void None %6
         %18 = OpLabel
         %19 = OpFunctionCall %void %12
               OpReturn
               OpFunctionEnd
       %Dead = OpFunction %void None %6
         %20 = OpLabel
               OpReturn
               OpFunctionEnd
         %12 = OpFunction %void None %9
         %21 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;

  DummyPass testPass;
  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  testPass.ProcessReachableCallTree(mark_visited, localContext.get());
  EXPECT_THAT(processed, UnorderedElementsAre(10, 11, 12));
}

TEST_F(PassClassTest, BasicDontVisitExportedVariable) {
  // Make sure we only visit functions and not exported variables.
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %10 "main" %gl_FragColor
               OpExecutionMode %10 OriginUpperLeft
               OpSource GLSL 150
               OpName %10 "main"
               OpName %Dead "Dead"
               OpName %11 "Constant"
               OpName %12 "export_var"
               OpDecorate %12 LinkageAttributes "export_var" Export
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
  %float_1 = OpConstant %float 1
         %12 = OpVariable %float Output
         %10 = OpFunction %void None %6
         %14 = OpLabel
               OpStore %12 %float_1
               OpReturn
               OpFunctionEnd
)";
  // clang-format on

  std::unique_ptr<IRContext> localContext =
      BuildModule(SPV_ENV_UNIVERSAL_1_1, nullptr, text,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  EXPECT_NE(nullptr, localContext) << "Assembling failed for shader:\n"
                                   << text << std::endl;

  DummyPass testPass;
  std::vector<uint32_t> processed;
  Pass::ProcessFunction mark_visited = [&processed](Function* fp) {
    processed.push_back(fp->result_id());
    return false;
  };
  testPass.ProcessReachableCallTree(mark_visited, localContext.get());
  EXPECT_THAT(processed, UnorderedElementsAre(10));
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
