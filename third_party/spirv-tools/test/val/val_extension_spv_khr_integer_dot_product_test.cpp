// Copyright (c) 2020 Google Inc.
// Copyright (c) 2021 Arm Ltd.
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

#include <ostream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/extensions.h"
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Values;

struct Case {
  std::vector<std::string> caps;
  std::string inst;
  std::string result_type;
  std::string op0_type;
  std::string op1_type;
  std::string acc_type;  // can be empty
  bool packed;
  std::string expected_error;  // empty for no error.
};

inline std::ostream& operator<<(std::ostream& out, Case c) {
  out << "\nSPV_KHR_integer_dot_product Case{{";
  bool first = true;
  for (const auto& cap : c.caps) {
    if (!first) {
      out << " ";
    }
    first = false;
    out << cap;
  }
  out << "} ";
  out << c.inst << " ";
  out << c.result_type << " ";
  out << c.op0_type << " ";
  out << c.op1_type << " ";
  out << "'" << c.acc_type << "' ";
  out << (c.packed ? "packed " : "unpacked ");
  out << "err'" << c.expected_error << "'";
  return out;
}

std::string AssemblyForCase(const Case& c) {
  std::ostringstream ss;
  ss << "OpCapability Shader\n";
  for (auto& cap : c.caps) {
    ss << "OpCapability " << cap << "\n";
  }
  ss << R"(
  OpExtension "SPV_KHR_integer_dot_product"
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
  OpExecutionMode %main LocalSize 1 1 1

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1

  %v2uint = OpTypeVector %uint 2
  %v3uint = OpTypeVector %uint 3
  %v4uint = OpTypeVector %uint 4
  %v2int = OpTypeVector %int 2
  %v3int = OpTypeVector %int 3
  %v4int = OpTypeVector %int 4

  %uint_0 = OpConstant %uint 0
  %uint_1 = OpConstant %uint 1
  %int_0 = OpConstant %int 0
  %int_1 = OpConstant %int 1

  %v2uint_0 = OpConstantComposite %v2uint %uint_0 %uint_0
  %v2uint_1 = OpConstantComposite %v2uint %uint_1 %uint_1
  %v3uint_0 = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0
  %v3uint_1 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
  %v4uint_0 = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_0
  %v4uint_1 = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_1 %uint_1

  %v2int_0 = OpConstantComposite %v2int %int_0 %int_0
  %v2int_1 = OpConstantComposite %v2int %int_1 %int_1
  %v3int_0 = OpConstantComposite %v3int %int_0 %int_0 %int_0
  %v3int_1 = OpConstantComposite %v3int %int_1 %int_1 %int_1
  %v4int_0 = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
  %v4int_1 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
)";

  bool use8bit = false;
  for (auto& cap : c.caps) {
    if (cap == "DotProductInput4x8BitKHR") {
      use8bit = true;
    }
    if (cap == "Int8") {
      use8bit = true;
    }
  }
  if (use8bit) {
    ss << R"(
         %uchar = OpTypeInt 8 0
         %char = OpTypeInt 8 1

         %v4uchar = OpTypeVector %uchar 4
         %v4char = OpTypeVector %char 4

         %uchar_0 = OpConstant %uchar 0
         %uchar_1 = OpConstant %uchar 1
         %char_0 = OpConstant %char 0
         %char_1 = OpConstant %char 1

         %v4uchar_0 = OpConstantComposite %v4uchar %uchar_0 %uchar_0 %uchar_0 %uchar_0
         %v4uchar_1 = OpConstantComposite %v4uchar %uchar_1 %uchar_1 %uchar_1 %uchar_1
         %v4char_0 = OpConstantComposite %v4char %char_0 %char_0 %char_0 %char_0
         %v4char_1 = OpConstantComposite %v4char %char_1 %char_1 %char_1 %char_1

         )";
  }

  ss << R"(

  %main = OpFunction %void None %voidfn
  %entry = OpLabel
  %result = )"
     << c.inst << " " << c.result_type << " ";
  ss << c.op0_type << "_0 ";
  ss << c.op1_type << "_1 ";
  if (!c.acc_type.empty()) {
    ss << c.acc_type << "_0 ";
  }
  if (c.packed) {
    ss << "PackedVectorFormat4x8BitKHR";
  }
  ss << "\nOpReturn\nOpFunctionEnd\n\n";
  return ss.str();
}

using ValidateSpvKHRIntegerDotProduct = spvtest::ValidateBase<Case>;

TEST_P(ValidateSpvKHRIntegerDotProduct, Valid) {
  const auto& c = GetParam();
  const auto& assembly = AssemblyForCase(c);
  CompileSuccessfully(assembly);
  if (c.expected_error.empty()) {
    EXPECT_EQ(SPV_SUCCESS, ValidateInstructions()) << getDiagnosticString();
  } else {
    EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
    EXPECT_THAT(getDiagnosticString(), HasSubstr(c.expected_error));
  }
}

// UDot
INSTANTIATE_TEST_SUITE_P(
    Valid_UDot, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpUDotKHR",
                           "%uint",
                           "%v2uint",
                           "%v2uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpUDotKHR",
                           "%uint",
                           "%v3uint",
                           "%v3uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpUDotKHR",
                           "%uint",
                           "%v4uint",
                           "%v4uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpUDotKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpUDotKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpUDotKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpUDotKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpUDotKHR",
                           "%uchar",  // matches packed component type
                           "%uint",
                           "%uint",
                           "",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpUDotKHR",
                           "%uint",
                           "%uint",
                           "%uint",
                           "",
                           true,
                           ""}));

// SDot result signed args signed signed
INSTANTIATE_TEST_SUITE_P(
    Valid_SDot_signed_signed_signed, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%v2int",
                           "%v2int",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%v3int",
                           "%v3int",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%v4int",
                           "%v4int",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4char",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4char",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4char",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4char",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSDotKHR",
                           "%char",  // matches packed component type
                           "%int",
                           "%int",
                           "",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%int",
                           "%int",
                           "",
                           true,
                           ""}));

// SDot result unsigned args signed unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SDot_unsigned_signed_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%uint",
                           "%v2int",
                           "%v2uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%uint",
                           "%v3int",
                           "%v3uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%uint",
                           "%v4int",
                           "%v4uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotKHR",
                           "%uchar",  // match width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotKHR",
                           "%uint",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotKHR",
                           "%uchar",  // match width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotKHR",
                           "%uint",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSDotKHR",
                           "%uchar",  // matches packed component type
                           "%int",
                           "%uint",
                           "",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSDotKHR",
                           "%uint",
                           "%int",
                           "%uint",
                           "",
                           true,
                           ""}));

// SDot result signed args signed unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SDot_signed_signed_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%v2int",
                           "%v2uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%v3int",
                           "%v3uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%v4int",
                           "%v4uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSDotKHR",
                           "%char",  // matches packed component type
                           "%int",
                           "%uint",
                           "",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSDotKHR",
                           "%int",
                           "%int",
                           "%uint",
                           "",
                           true,
                           ""}));

// SUDot result signed args unsigned unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SUDot_signed_unsigned_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%v2uint",
                           "%v2uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%v3uint",
                           "%v3uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%v4uint",
                           "%v4uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotKHR",
                           "%char",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotKHR",
                           "%int",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotKHR",
                           "%char",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotKHR",
                           "%int",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSUDotKHR",
                           "%char",  // matches packed component type
                           "%uint",
                           "%uint",
                           "",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%uint",
                           "%uint",
                           "",
                           true,
                           ""}));

// SUDot result signed args signed unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SUDot_signed_signed_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%v2int",
                           "%v2uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%v3int",
                           "%v3uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%v4int",
                           "%v4uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSUDotKHR",
                           "%char",  // matches packed component type
                           "%int",
                           "%uint",
                           "",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSUDotKHR",
                           "%int",
                           "%int",
                           "%uint",
                           "",
                           true,
                           ""}));

// SUDot result unsigned args unsigned unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SUDot_unsigned_unsigned_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%uint",
                           "%v2uint",
                           "%v2uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%uint",
                           "%v3uint",
                           "%v3uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotKHR",
                           "%uint",
                           "%v4uint",
                           "%v4uint",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSUDotKHR",
                           "%uchar",  // matches packed component type
                           "%uint",
                           "%uint",
                           "",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSUDotKHR",
                           "%uint",
                           "%uint",
                           "%uint",
                           "",
                           true,
                           ""}));

// UDotAccSat
INSTANTIATE_TEST_SUITE_P(
    Valid_UDotAccSat, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpUDotAccSatKHR",
                           "%uint",
                           "%v2uint",
                           "%v2uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpUDotAccSatKHR",
                           "%uint",
                           "%v3uint",
                           "%v3uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpUDotAccSatKHR",
                           "%uint",
                           "%v4uint",
                           "%v4uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpUDotAccSatKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpUDotAccSatKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpUDotAccSatKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "%uchar",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpUDotAccSatKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpUDotAccSatKHR",
                           "%uchar",  // matches packed component type
                           "%uint",
                           "%uint",
                           "%uchar",
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpUDotAccSatKHR",
                           "%uint",
                           "%uint",
                           "%uint",
                           "%uint",
                           true,
                           ""}));

// SDotAccSat result signed args signed signed
INSTANTIATE_TEST_SUITE_P(
    Valid_SDotAccSat_signed_signed_signed, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%v2int",
                           "%v2int",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%v3int",
                           "%v3int",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%v4int",
                           "%v4int",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotAccSatKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4char",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotAccSatKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4char",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotAccSatKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4char",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotAccSatKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4char",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSDotAccSatKHR",
                           "%char",  // matches packed component type
                           "%int",
                           "%int",
                           "%char",  // matches packed component type
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%int",
                           "%int",
                           "%int",
                           true,
                           ""}));

// SDotAccSat result unsigned args signed unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SDotAccSat_unsigned_signed_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%uint",
                           "%v2int",
                           "%v2uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%uint",
                           "%v3int",
                           "%v3uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%uint",
                           "%v4int",
                           "%v4uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotAccSatKHR",
                           "%uchar",  // match width
                           "%v4char",
                           "%v4uchar",
                           "%uchar",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotAccSatKHR",
                           "%uint",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "%uint",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotAccSatKHR",
                           "%uchar",  // match width
                           "%v4char",
                           "%v4uchar",
                           "%uchar",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotAccSatKHR",
                           "%uint",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "%uint",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSDotAccSatKHR",
                           "%uchar",  // matches packed component type
                           "%int",
                           "%uint",
                           "%uchar",  // matches packed component type
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSDotAccSatKHR",
                           "%uint",
                           "%int",
                           "%uint",
                           "%uint",
                           true,
                           ""}));

// SDotAccSat result signed args signed unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SDotAccSat_signed_signed_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%v2int",
                           "%v2uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%v3int",
                           "%v3uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%v4int",
                           "%v4uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotAccSatKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSDotAccSatKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotAccSatKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSDotAccSatKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSDotAccSatKHR",
                           "%char",  // matches packed component type
                           "%int",
                           "%uint",
                           "%char",  // matches packed component type
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSDotAccSatKHR",
                           "%int",
                           "%int",
                           "%uint",
                           "%int",
                           true,
                           ""}));

// SUDotAccSat result signed args unsigned unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SUDotAccSat_signed_unsigned_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%v2uint",
                           "%v2uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%v3uint",
                           "%v3uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%v4uint",
                           "%v4uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotAccSatKHR",
                           "%char",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotAccSatKHR",
                           "%int",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotAccSatKHR",
                           "%char",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSUDotAccSatKHR",
                           "%char",  // matches packed component type
                           "%uint",
                           "%uint",
                           "%char",  // matches packed component type
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%uint",
                           "%uint",
                           "%int",
                           true,
                           ""}));

// SUDotAccSat result signed args signed unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SUDotAccSat_signed_signed_unsigned, ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%v2int",
                           "%v2uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%v3int",
                           "%v3uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%v4int",
                           "%v4uint",
                           "%int",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotAccSatKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotAccSatKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotAccSatKHR",
                           "%char",  // match width
                           "%v4char",
                           "%v4uchar",
                           "%char",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",  // wider width
                           "%v4char",
                           "%v4uchar",
                           "%int",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSUDotAccSatKHR",
                           "%char",  // matches packed component type
                           "%int",
                           "%uint",
                           "%char",  // matches packed component type
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSUDotAccSatKHR",
                           "%int",
                           "%int",
                           "%uint",
                           "%int",
                           true,
                           ""}));

// SUDotAccSat result unsigned args unsigned unsigned
INSTANTIATE_TEST_SUITE_P(
    Valid_SUDotAccSat_unsigned_unsigned_unsigned,
    ValidateSpvKHRIntegerDotProduct,
    ::testing::Values(Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%uint",
                           "%v2uint",
                           "%v2uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%uint",
                           "%v3uint",
                           "%v3uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR"},
                           "OpSUDotAccSatKHR",
                           "%uint",
                           "%v4uint",
                           "%v4uint",
                           "%uint",
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotAccSatKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "%uchar",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInputAllKHR", "Int8"},
                           "OpSUDotAccSatKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "%uint",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotAccSatKHR",
                           "%uchar",  // match width
                           "%v4uchar",
                           "%v4uchar",
                           "%uchar",  // match width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitKHR"},
                           "OpSUDotAccSatKHR",
                           "%uint",  // wider width
                           "%v4uchar",
                           "%v4uchar",
                           "%uint",  // wider width
                           false,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR",
                            "Int8"},
                           "OpSUDotAccSatKHR",
                           "%uchar",  // matches packed component type
                           "%uint",
                           "%uint",
                           "%uchar",  // matches packed component type
                           true,
                           ""},
                      Case{{"DotProductKHR", "DotProductInput4x8BitPackedKHR"},
                           "OpSUDotAccSatKHR",
                           "%uint",
                           "%uint",
                           "%uint",
                           "%uint",
                           true,
                           ""}));

using ValidateSpvKHRIntegerDotProductSimple = ::testing::Test;

TEST(ValidateSpvKHRIntegerDotProductSimple, DISABLED_RequiresExtension) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple, DISABLED_Invalid_ResultTooNarrow) {
  // Test across all the instructions.
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_UDot_OperandTypesMatch) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_SDot_OperandTypesMatchExceptSignedness) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_SUDot_OperandTypesMatchExceptSignedness) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_UDotAccSat_OperandTypesMatch) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_SDotAccSat_OperandTypesMatchExceptSignedness) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_SUDotAccSat_OperandTypesMatchExceptSignedness) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_UDot_RequiresUnsigned) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_SUDot_RequiresUnsignedSecondArg) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_UDotAccSat_RequiresUnsigned) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_SUDotAccSat_RequiresUnsignedSecondArg) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_VectorOperandsDisallowPackedFormat) {
  FAIL();
}

TEST(ValidateSpvKHRIntegerDotProductSimple,
     DISABLED_Invalid_ScalarOperandsRequirePackedFormat) {
  FAIL();
}

// TODO(dneto): Test valid cases with other scalar integer types
// TODO(dneto): Test valid cases of length-8 vectors
// TODO(dneto): Test valid cases of length-16 vectors

}  // namespace
}  // namespace val
}  // namespace spvtools
