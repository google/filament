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
#include "source/spirv_target_env.h"
#include "source/table2.h"
#include "test/unit_spirv.h"

using ::testing::ContainerEq;
using ::testing::ValuesIn;

namespace spvtools {
namespace {

struct OpcodeLookupCase {
  std::string name;
  uint32_t opcode;
  bool expect_pass = true;
};

std::ostream& operator<<(std::ostream& os, const OpcodeLookupCase& olc) {
  os << "OLC('" << olc.name << "', " << olc.opcode << ", expect pass? "
     << olc.expect_pass << ")";
  return os;
}

using OpcodeLookupTest = ::testing::TestWithParam<OpcodeLookupCase>;

TEST_P(OpcodeLookupTest, OpcodeLookup_ByName) {
  const InstructionDesc* desc = nullptr;
  auto status = LookupOpcode(GetParam().name.data(), &desc);
  if (GetParam().expect_pass) {
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(static_cast<uint32_t>(desc->opcode), GetParam().opcode);
  } else {
    EXPECT_NE(status, SPV_SUCCESS);
    EXPECT_EQ(desc, nullptr);
  }
}

TEST_P(OpcodeLookupTest, OpcodeLookup_ByOpcode_Success) {
  const InstructionDesc* desc = nullptr;
  if (GetParam().expect_pass) {
    spv::Op opcode = static_cast<spv::Op>(GetParam().opcode);
    auto status = LookupOpcode(opcode, &desc);
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->opcode, opcode);
  }
}

INSTANTIATE_TEST_SUITE_P(Samples, OpcodeLookupTest,
                         ValuesIn(std::vector<OpcodeLookupCase>{
                             {"Nop", 0},
                             {"WritePipe", 275},
                             {"TypeAccelerationStructureKHR", 5341},
                             {"TypeAccelerationStructureNV", 5341},
                             {"does not exist", 0, false},
                             {"CopyLogical", 400},
                             {"FPGARegINTEL", 5949},
                             {"SubgroupMatrixMultiplyAccumulateINTEL", 6237},
                         }));

TEST(OpcodeLookupSingleTest, OpcodeLookup_ByOpcode_Fails) {
  // This list may need adjusting over time.
  std::array<uint32_t, 3> bad_opcodes = {{99999, 37737, 110101}};
  for (auto bad_opcode : bad_opcodes) {
    const InstructionDesc* desc = nullptr;
    spv::Op opcode = static_cast<spv::Op>(bad_opcode);
    auto status = LookupOpcode(opcode, &desc);
    EXPECT_NE(status, SPV_SUCCESS);
    ASSERT_EQ(desc, nullptr);
  }
}

struct OpcodeLookupEnvCase {
  std::string name;
  uint32_t opcode;
  spv_target_env env = SPV_ENV_UNIVERSAL_1_0;
  bool expect_pass = true;
};

std::ostream& operator<<(std::ostream& os, const OpcodeLookupEnvCase& olec) {
  os << "OLC('" << olec.name << "', " << olec.opcode << ", env "
     << spvTargetEnvDescription(olec.env) << ", expect pass? "
     << olec.expect_pass << ")";
  return os;
}

using OpcodeLookupEnvTest = ::testing::TestWithParam<OpcodeLookupEnvCase>;

TEST_P(OpcodeLookupEnvTest, OpcodeLookupForEnv_ByName) {
  const InstructionDesc* desc = nullptr;
  auto status =
      LookupOpcodeForEnv(GetParam().env, GetParam().name.data(), &desc);
  if (GetParam().expect_pass) {
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(static_cast<uint32_t>(desc->opcode), GetParam().opcode);
  } else {
    EXPECT_NE(status, SPV_SUCCESS);
    EXPECT_EQ(desc, nullptr);
  }
}

TEST_P(OpcodeLookupEnvTest, OpcodeLookupForEnv_ByOpcode) {
  const InstructionDesc* desc = nullptr;
  spv::Op opcode = static_cast<spv::Op>(GetParam().opcode);
  auto status = LookupOpcodeForEnv(GetParam().env, opcode, &desc);
  if (GetParam().expect_pass) {
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->opcode, opcode);
  } else {
    // Skip nonsense cases created for the lookup-by-name case.
    if (GetParam().name != "does not exist") {
      EXPECT_NE(status, SPV_SUCCESS);
      EXPECT_EQ(desc, nullptr);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(Samples, OpcodeLookupEnvTest,
                         ValuesIn(std::vector<OpcodeLookupEnvCase>{
                             {"Nop", 0},
                             {"WritePipe", 275},
                             {"TypeAccelerationStructureKHR", 5341},
                             {"TypeAccelerationStructureNV", 5341},
                             {"does not exist", 0, SPV_ENV_UNIVERSAL_1_0,
                              false},
                             {"CopyLogical", 400, SPV_ENV_UNIVERSAL_1_0, false},
                             {"CopyLogical", 400, SPV_ENV_UNIVERSAL_1_3, false},
                             {"CopyLogical", 400, SPV_ENV_UNIVERSAL_1_4, true},
                             {"FPGARegINTEL", 5949},
                             {"SubgroupMatrixMultiplyAccumulateINTEL", 6237},
                         }));

TEST(OpcodeLookupExtInstTest, Operands) {
  // The SPIR-V spec grammar has a single rule for OpExtInst, where the last
  // item is "sequence of Ids".  SPIRV-Tools handles it differently. It drops
  // that last item, and instead specifies those operands as operands of the
  // extended instruction enum, such as 'cos'.
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/233
  // Test the exact sequence of operand types extracted for OpExtInst.
  const InstructionDesc* desc = nullptr;
  auto status = LookupOpcode("ExtInst", &desc);
  EXPECT_EQ(status, SPV_SUCCESS);
  ASSERT_NE(desc, nullptr);

  EXPECT_EQ(desc->operands_range.count(), 4u);

  auto operands = desc->operands();
  using vtype = std::vector<spv_operand_type_t>;

  EXPECT_THAT(
      vtype(operands.begin(), operands.end()),
      ContainerEq(vtype{SPV_OPERAND_TYPE_TYPE_ID, SPV_OPERAND_TYPE_RESULT_ID,
                        SPV_OPERAND_TYPE_ID,
                        SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER}));
}

// Test printingClass

struct OpcodePrintingClassCase {
  std::string name;
  PrintingClass expected;
};

std::ostream& operator<<(std::ostream& os,
                         const OpcodePrintingClassCase& opcc) {
  os << "OPCC('" << opcc.name << "', " << static_cast<int>(opcc.expected)
     << ")";
  return os;
}

using OpcodePrintingClassTest =
    ::testing::TestWithParam<OpcodePrintingClassCase>;

TEST_P(OpcodePrintingClassTest, OpcodeLookup_ByName) {
  const InstructionDesc* desc = nullptr;
  auto status = LookupOpcode(GetParam().name.data(), &desc);
  EXPECT_EQ(status, SPV_SUCCESS);
  ASSERT_NE(desc, nullptr);
  EXPECT_EQ(desc->printingClass, GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(
    Samples, OpcodePrintingClassTest,
    ValuesIn(std::vector<OpcodePrintingClassCase>{
        {"ConstantFunctionPointerINTEL", PrintingClass::k_exclude},
        {"Nop", PrintingClass::kMiscellaneous},
        {"SourceContinued", PrintingClass::kDebug},
        {"Decorate", PrintingClass::kAnnotation},
        {"Extension", PrintingClass::kExtension},
        {"MemoryModel", PrintingClass::kMode_Setting},
        {"Variable", PrintingClass::kMemory},
        {"CooperativeMatrixPerElementOpNV", PrintingClass::kFunction},
        {"SampledImage", PrintingClass::kImage},
        {"ConvertFToU", PrintingClass::kConversion},
        {"VectorExtractDynamic", PrintingClass::kComposite},
        {"IAdd", PrintingClass::kArithmetic},
        {"ShiftRightLogical", PrintingClass::kBit},
        {"Any", PrintingClass::kRelational_and_Logical},
        {"DPdx", PrintingClass::kDerivative},
        {"Branch", PrintingClass::kControl_Flow},
        {"AtomicLoad", PrintingClass::kAtomic},
        {"ControlBarrier", PrintingClass::kBarrier},
        {"GroupAll", PrintingClass::kGroup},
        {"EnqueueMarker", PrintingClass::kDevice_Side_Enqueue},
        {"ReadPipe", PrintingClass::kPipe},
        {"GroupNonUniformElect", PrintingClass::kNon_Uniform},
        // Skipping "Reserved" because it's probably an
        // unstable class.
    }));

}  // namespace
}  // namespace spvtools
