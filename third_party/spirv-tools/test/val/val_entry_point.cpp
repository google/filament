// Copyright (c) 2019 Samsung Inc
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
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

using ValidateEntryPoints = spvtest::ValidateBase<bool>;

TEST_F(ValidateEntryPoints, DuplicateEntryPoints) {
  const std::string body = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "foo"
OpEntryPoint GLCompute %4 "foo"
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%20 = OpLabel
OpReturn
OpFunctionEnd
%4 = OpFunction %1 None %2
%21 = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Entry points cannot share the same name"));
}

TEST_F(ValidateEntryPoints, UniqueEntryPoints) {
  const std::string body = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %3 "foo"
OpEntryPoint GLCompute %4 "foo2"
%1 = OpTypeVoid
%2 = OpTypeFunction %1
%3 = OpFunction %1 None %2
%20 = OpLabel
OpReturn
OpFunctionEnd
%4 = OpFunction %1 None %2
%21 = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(body);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

}  // namespace
}  // namespace spvtools
