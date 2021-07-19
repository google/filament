// Copyright (c) 2020 Google Inc.
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

// Tests for OpExtension validator rules.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/enum_string_mapping.h"
#include "source/extensions.h"
#include "source/spirv_target_env.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateSpvKHRLinkOnceODR = spvtest::ValidateBase<bool>;

TEST_F(ValidateSpvKHRLinkOnceODR, Valid) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpExtension "SPV_KHR_linkonce_odr"
    OpMemoryModel Physical32 OpenCL
    OpDecorate %var LinkageAttributes "foobar" LinkOnceODR

    %uint = OpTypeInt 32 0
    %ptr = OpTypePointer CrossWorkgroup %uint
    %var = OpVariable %ptr CrossWorkgroup

)";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateSpvKHRLinkOnceODR, RequiresExtension) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpMemoryModel Physical32 OpenCL
    OpDecorate %var LinkageAttributes "foobar" LinkOnceODR

    %uint = OpTypeInt 32 0
    %ptr = OpTypePointer CrossWorkgroup %uint
    %var = OpVariable %ptr CrossWorkgroup
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("4th operand of Decorate: operand LinkOnceODR(2) requires one "
                "of these extensions: SPV_KHR_linkonce_odr \n"
                "  OpDecorate %1 LinkageAttributes \"foobar\" LinkOnceODR\n"));
}

TEST_F(ValidateSpvKHRLinkOnceODR, RequiresLinkageCapability) {
  const std::string str = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpExtension "SPV_KHR_linkonce_odr"
    OpMemoryModel Physical32 OpenCL
    OpDecorate %var LinkageAttributes "foobar" LinkOnceODR

    %uint = OpTypeInt 32 0
    %ptr = OpTypePointer CrossWorkgroup %uint
    %var = OpVariable %ptr CrossWorkgroup
)";
  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Operand 2 of Decorate requires one of these capabilities: Linkage \n"
          "  OpDecorate %1 LinkageAttributes \"foobar\" LinkOnceODR"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
