// Copyright 2025 Google LLC
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

#include <array>
#include <iostream>

#include "gmock/gmock.h"
#include "source/operand.h"
#include "source/table2.h"
#include "spirv-tools/libspirv.h"
#include "test/unit_spirv.h"

using ::testing::ContainerEq;
using ::testing::ValuesIn;

namespace spvtools {
namespace {

struct OperandLookupCase {
  spv_operand_type_t type;
  std::string name;
  size_t name_length;
  uint32_t value;
  bool expect_pass = true;
};

std::ostream& operator<<(std::ostream& os, const OperandLookupCase& olc) {
  os << "OLC('" << spvOperandTypeStr(olc.type) << " '" << olc.name
     << "', len:" << olc.name_length << ", value:" << olc.value
     << ", expect pass? " << olc.expect_pass << ")";
  return os;
}

using OperandLookupTest = ::testing::TestWithParam<OperandLookupCase>;

TEST_P(OperandLookupTest, OperandLookup_ByName) {
  const OperandDesc* desc = nullptr;
  auto status = LookupOperand(GetParam().type, GetParam().name.data(),
                              GetParam().name_length, &desc);
  if (GetParam().expect_pass) {
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->value, GetParam().value);
  } else {
    EXPECT_NE(status, SPV_SUCCESS);
    EXPECT_EQ(desc, nullptr);
  }
}

TEST_P(OperandLookupTest, OperandLookup_ByValue_Success) {
  const OperandDesc* desc = nullptr;
  if (GetParam().expect_pass) {
    const auto value = GetParam().value;
    auto status = LookupOperand(GetParam().type, GetParam().value, &desc);
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->value, value);
  }
}

INSTANTIATE_TEST_SUITE_P(
    Samples, OperandLookupTest,
    ValuesIn(std::vector<OperandLookupCase>{
        {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, "Relaxed", 7, 0},
        // "None" is an alias for "Relaxed"
        {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, "None", 4, 0},
        // "NonPrivatePointer" is the canonical name that appeared
        // in an extension and incorporated in SPIR-V 1.5.
        {SPV_OPERAND_TYPE_MEMORY_ACCESS, "NonPrivatePointer", 17, 32},
        // "NonPrivatePointerKHR" is the name from the extension.
        {SPV_OPERAND_TYPE_MEMORY_ACCESS, "NonPrivatePointerKHR", 20, 32},
        // "NoAliasINTELMask" is only in an extension
        {SPV_OPERAND_TYPE_MEMORY_ACCESS, "NoAliasINTELMask", 16, 0x20000},
        {SPV_OPERAND_TYPE_RAY_FLAGS, "TerminateOnFirstHitKHR", 22, 4},
        {SPV_OPERAND_TYPE_FPENCODING, "BFloat16KHR", 11, 0},
        // Lookup on An optional operand type should match the base lookup.
        {SPV_OPERAND_TYPE_OPTIONAL_FPENCODING, "BFloat16KHR", 11, 0},
        // Lookup is type-specific.
        {SPV_OPERAND_TYPE_OPTIONAL_FPENCODING, "Relaxed", 7, 0, false},
        // Invalid string
        {SPV_OPERAND_TYPE_RAY_FLAGS, "does_not_exist", 14, 0, false},
        // Check lengths
        {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, "Relaxed", 6, 0, false},
        {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, "Relaxed", 7, 0, true},
        {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, "Relaxed|", 7, 0, true},
        {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, "Relaxed|", 8, 0, false},
    }));

TEST(OperandLookupSingleTest, OperandLookup_ByValue_Fails) {
  // This list may need adjusting over time.
  std::array<spv_operand_type_t, 3> types = {
      {SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID, SPV_OPERAND_TYPE_RAY_FLAGS,
       SPV_OPERAND_TYPE_FPENCODING}};
  std::array<uint32_t, 3> bad_values = {{99999, 37737, 110101}};
  for (auto type : types) {
    for (auto bad_value : bad_values) {
      const OperandDesc* desc = nullptr;
      auto status = LookupOperand(type, bad_value, &desc);
      EXPECT_NE(status, SPV_SUCCESS);
      ASSERT_EQ(desc, nullptr);
    }
  }
}

TEST(OperandLookupOperands, Sample) {
  // Check the operand list for a valid operand lookup.
  const OperandDesc* desc = nullptr;
  auto status = LookupOperand(SPV_OPERAND_TYPE_IMAGE, "Grad", 4, &desc);
  EXPECT_EQ(status, SPV_SUCCESS);
  ASSERT_NE(desc, nullptr);

  EXPECT_EQ(desc->operands_range.count(), 2u);

  auto operands = desc->operands();
  using vtype = std::vector<spv_operand_type_t>;

  EXPECT_THAT(vtype(operands.begin(), operands.end()),
              ContainerEq(vtype{SPV_OPERAND_TYPE_ID, SPV_OPERAND_TYPE_ID}));
}

}  // namespace
}  // namespace spvtools
