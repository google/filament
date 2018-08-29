// Copyright (c) 2018 Google Inc.
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

// Validation tests for WebGPU env specific checks

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using testing::HasSubstr;

using ValidateWebGPU = spvtest::ValidateBase<bool>;

TEST_F(ValidateWebGPU, OpUndefIsDisallowed) {
  std::string spirv = R"(
    OpCapability Shader
    OpCapability Linkage
    OpMemoryModel Logical GLSL450
    %float = OpTypeFloat 32
    %1 = OpUndef %float
)";

  CompileSuccessfully(spirv);

  // Control case: OpUndef is allowed in SPIR-V 1.3
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));

  // Control case: OpUndef is disallowed in the WebGPU env
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions(SPV_ENV_WEBGPU_0));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("OpUndef is disallowed"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
