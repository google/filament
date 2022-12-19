// Copyright (c) 2015-2016 The Khronos Group Inc.
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

// Assembler tests for instructions in the "Extension Instruction" section
// of the SPIR-V spec.

#include <string>
#include <tuple>
#include <vector>

#include "gmock/gmock.h"
#include "source/latest_version_glsl_std_450_header.h"
#include "source/latest_version_opencl_std_header.h"
#include "source/util/string_utils.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::Concatenate;
using spvtest::MakeInstruction;
using utils::MakeVector;
using spvtest::TextToBinaryTest;
using ::testing::Combine;
using ::testing::Eq;
using ::testing::Values;
using ::testing::ValuesIn;

// Returns a generator of common Vulkan environment values to be tested.
std::vector<spv_target_env> CommonVulkanEnvs() {
  return {SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1, SPV_ENV_UNIVERSAL_1_2,
          SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_0,    SPV_ENV_VULKAN_1_1};
}

TEST_F(TextToBinaryTest, InvalidExtInstImportName) {
  EXPECT_THAT(CompileFailure("%1 = OpExtInstImport \"Haskell.std\""),
              Eq("Invalid extended instruction import 'Haskell.std'"));
}

TEST_F(TextToBinaryTest, InvalidImportId) {
  EXPECT_THAT(CompileFailure("%1 = OpTypeVoid\n"
                             "%2 = OpExtInst %1 %1"),
              Eq("Invalid extended instruction import Id 2"));
}

TEST_F(TextToBinaryTest, InvalidImportInstruction) {
  const std::string input = R"(%1 = OpTypeVoid
                               %2 = OpExtInstImport "OpenCL.std"
                               %3 = OpExtInst %1 %2 not_in_the_opencl)";
  EXPECT_THAT(CompileFailure(input),
              Eq("Invalid extended instruction name 'not_in_the_opencl'."));
}

TEST_F(TextToBinaryTest, MultiImport) {
  const std::string input = R"(%2 = OpExtInstImport "OpenCL.std"
                               %2 = OpExtInstImport "OpenCL.std")";
  EXPECT_THAT(CompileFailure(input),
              Eq("Import Id is being defined a second time"));
}

TEST_F(TextToBinaryTest, TooManyArguments) {
  const std::string input = R"(%opencl = OpExtInstImport "OpenCL.std"
                               %2 = OpExtInst %float %opencl cos %x %oops")";
  EXPECT_THAT(CompileFailure(input), Eq("Expected '=', found end of stream."));
}

TEST_F(TextToBinaryTest, ExtInstFromTwoDifferentImports) {
  const std::string input = R"(%1 = OpExtInstImport "OpenCL.std"
%2 = OpExtInstImport "GLSL.std.450"
%4 = OpExtInst %3 %1 native_sqrt %5
%7 = OpExtInst %6 %2 MatrixInverse %8
)";

  // Make sure it assembles correctly.
  EXPECT_THAT(
      CompiledInstructions(input),
      Eq(Concatenate({
          MakeInstruction(spv::Op::OpExtInstImport, {1},
                          MakeVector("OpenCL.std")),
          MakeInstruction(spv::Op::OpExtInstImport, {2},
                          MakeVector("GLSL.std.450")),
          MakeInstruction(
              spv::Op::OpExtInst,
              {3, 4, 1, uint32_t(OpenCLLIB::Entrypoints::Native_sqrt), 5}),
          MakeInstruction(spv::Op::OpExtInst,
                          {6, 7, 2, uint32_t(GLSLstd450MatrixInverse), 8}),
      })));

  // Make sure it disassembles correctly.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(input), Eq(input));
}

// A test case for assembling into words in an instruction.
struct AssemblyCase {
  std::string input;
  std::vector<uint32_t> expected;
};

using ExtensionAssemblyTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<std::tuple<spv_target_env, AssemblyCase>>>;

TEST_P(ExtensionAssemblyTest, Samples) {
  const spv_target_env& env = std::get<0>(GetParam());
  const AssemblyCase& ac = std::get<1>(GetParam());

  // Check that it assembles correctly.
  EXPECT_THAT(CompiledInstructions(ac.input, env), Eq(ac.expected));
}

using ExtensionRoundTripTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<std::tuple<spv_target_env, AssemblyCase>>>;

TEST_P(ExtensionRoundTripTest, Samples) {
  const spv_target_env& env = std::get<0>(GetParam());
  const AssemblyCase& ac = std::get<1>(GetParam());

  // Check that it assembles correctly.
  EXPECT_THAT(CompiledInstructions(ac.input, env), Eq(ac.expected));

  // Check round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(ac.input,
                                          SPV_BINARY_TO_TEXT_OPTION_NONE, env),
              Eq(ac.input))
      << "target env: " << spvTargetEnvDescription(env) << "\n";
}

// SPV_KHR_shader_ballot

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_shader_ballot, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpCapability SubgroupBallotKHR\n",
             MakeInstruction(spv::Op::OpCapability,
                             {uint32_t(spv::Capability::SubgroupBallotKHR)})},
            {"%2 = OpSubgroupBallotKHR %1 %3\n",
             MakeInstruction(spv::Op::OpSubgroupBallotKHR, {1, 2, 3})},
            {"%2 = OpSubgroupFirstInvocationKHR %1 %3\n",
             MakeInstruction(spv::Op::OpSubgroupFirstInvocationKHR, {1, 2, 3})},
            {"OpDecorate %1 BuiltIn SubgroupEqMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupEqMaskKHR)})},
            {"OpDecorate %1 BuiltIn SubgroupGeMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupGeMaskKHR)})},
            {"OpDecorate %1 BuiltIn SubgroupGtMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupGtMaskKHR)})},
            {"OpDecorate %1 BuiltIn SubgroupLeMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupLeMaskKHR)})},
            {"OpDecorate %1 BuiltIn SubgroupLtMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupLtMaskKHR)})},
        })));

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_shader_ballot_vulkan_1_1, ExtensionRoundTripTest,
    // In SPIR-V 1.3 and Vulkan 1.1 we can drop the KHR suffix on the
    // builtin enums.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_1),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpCapability SubgroupBallotKHR\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::SubgroupBallotKHR})},
            {"%2 = OpSubgroupBallotKHR %1 %3\n",
             MakeInstruction(spv::Op::OpSubgroupBallotKHR, {1, 2, 3})},
            {"%2 = OpSubgroupFirstInvocationKHR %1 %3\n",
             MakeInstruction(spv::Op::OpSubgroupFirstInvocationKHR, {1, 2, 3})},
            {"OpDecorate %1 BuiltIn SubgroupEqMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupEqMask)})},
            {"OpDecorate %1 BuiltIn SubgroupGeMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupGeMask)})},
            {"OpDecorate %1 BuiltIn SubgroupGtMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupGtMask)})},
            {"OpDecorate %1 BuiltIn SubgroupLeMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupLeMask)})},
            {"OpDecorate %1 BuiltIn SubgroupLtMask\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, uint32_t(spv::Decoration::BuiltIn),
                              uint32_t(spv::BuiltIn::SubgroupLtMask)})},
        })));

// The old builtin names (with KHR suffix) still work in the assembler, and
// map to the enums without the KHR.
INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_shader_ballot_vulkan_1_1_alias_check, ExtensionAssemblyTest,
    // In SPIR-V 1.3 and Vulkan 1.1 we can drop the KHR suffix on the
    // builtin enums.
    Combine(Values(SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_1),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpDecorate %1 BuiltIn SubgroupEqMaskKHR\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::SubgroupEqMask})},
                {"OpDecorate %1 BuiltIn SubgroupGeMaskKHR\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::SubgroupGeMask})},
                {"OpDecorate %1 BuiltIn SubgroupGtMaskKHR\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::SubgroupGtMask})},
                {"OpDecorate %1 BuiltIn SubgroupLeMaskKHR\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::SubgroupLeMask})},
                {"OpDecorate %1 BuiltIn SubgroupLtMaskKHR\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::SubgroupLtMask})},
            })));

// SPV_KHR_shader_draw_parameters

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_shader_draw_parameters, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(ValuesIn(CommonVulkanEnvs()),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpCapability DrawParameters\n",
                 MakeInstruction(spv::Op::OpCapability,
                                 {(uint32_t)spv::Capability::DrawParameters})},
                {"OpDecorate %1 BuiltIn BaseVertex\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::BaseVertex})},
                {"OpDecorate %1 BuiltIn BaseInstance\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::BaseInstance})},
                {"OpDecorate %1 BuiltIn DrawIndex\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::DrawIndex})},
            })));

// SPV_KHR_subgroup_vote

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_subgroup_vote, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(ValuesIn(CommonVulkanEnvs()),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpCapability SubgroupVoteKHR\n",
                 MakeInstruction(spv::Op::OpCapability,
                                 {(uint32_t)spv::Capability::SubgroupVoteKHR})},
                {"%2 = OpSubgroupAnyKHR %1 %3\n",
                 MakeInstruction(spv::Op::OpSubgroupAnyKHR, {1, 2, 3})},
                {"%2 = OpSubgroupAllKHR %1 %3\n",
                 MakeInstruction(spv::Op::OpSubgroupAllKHR, {1, 2, 3})},
                {"%2 = OpSubgroupAllEqualKHR %1 %3\n",
                 MakeInstruction(spv::Op::OpSubgroupAllEqualKHR, {1, 2, 3})},
            })));

// SPV_KHR_16bit_storage

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_16bit_storage, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(
        ValuesIn(CommonVulkanEnvs()),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpCapability StorageBuffer16BitAccess\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::StorageUniformBufferBlock16})},
            {"OpCapability StorageBuffer16BitAccess\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::StorageBuffer16BitAccess})},
            {"OpCapability UniformAndStorageBuffer16BitAccess\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)
                      spv::Capability::UniformAndStorageBuffer16BitAccess})},
            {"OpCapability UniformAndStorageBuffer16BitAccess\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::StorageUniform16})},
            {"OpCapability StoragePushConstant16\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::StoragePushConstant16})},
            {"OpCapability StorageInputOutput16\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::StorageInputOutput16})},
        })));

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_16bit_storage_alias_check, ExtensionAssemblyTest,
    Combine(
        ValuesIn(CommonVulkanEnvs()),
        ValuesIn(std::vector<AssemblyCase>{
            // The old name maps to the new enum.
            {"OpCapability StorageUniformBufferBlock16\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::StorageBuffer16BitAccess})},
            // The new name maps to the old enum.
            {"OpCapability UniformAndStorageBuffer16BitAccess\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::StorageUniform16})},
        })));

// SPV_KHR_device_group

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_device_group, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(ValuesIn(CommonVulkanEnvs()),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpCapability DeviceGroup\n",
                 MakeInstruction(spv::Op::OpCapability,
                                 {(uint32_t)spv::Capability::DeviceGroup})},
                {"OpDecorate %1 BuiltIn DeviceIndex\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::DeviceIndex})},
            })));

// SPV_KHR_8bit_storage

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_8bit_storage, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(ValuesIn(CommonVulkanEnvs()),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpCapability StorageBuffer8BitAccess\n",
                 MakeInstruction(
                     spv::Op::OpCapability,
                     {(uint32_t)spv::Capability::StorageBuffer8BitAccess})},
                {"OpCapability UniformAndStorageBuffer8BitAccess\n",
                 MakeInstruction(
                     spv::Op::OpCapability,
                     {(uint32_t)
                          spv::Capability::UniformAndStorageBuffer8BitAccess})},
                {"OpCapability StoragePushConstant8\n",
                 MakeInstruction(
                     spv::Op::OpCapability,
                     {(uint32_t)spv::Capability::StoragePushConstant8})},
            })));

// SPV_KHR_multiview

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_multiview, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                   SPV_ENV_VULKAN_1_0),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpCapability MultiView\n",
                 MakeInstruction(spv::Op::OpCapability,
                                 {(uint32_t)spv::Capability::MultiView})},
                {"OpDecorate %1 BuiltIn ViewIndex\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::ViewIndex})},
            })));

// SPV_AMD_shader_explicit_vertex_parameter

#define PREAMBLE \
  "%1 = OpExtInstImport \"SPV_AMD_shader_explicit_vertex_parameter\"\n"
INSTANTIATE_TEST_SUITE_P(
    SPV_AMD_shader_explicit_vertex_parameter, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {PREAMBLE "%3 = OpExtInst %2 %1 InterpolateAtVertexAMD %4 %5\n",
             Concatenate(
                 {MakeInstruction(
                      spv::Op::OpExtInstImport, {1},
                      MakeVector("SPV_AMD_shader_explicit_vertex_parameter")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 1, 4, 5})})},
        })));
#undef PREAMBLE

// SPV_AMD_shader_trinary_minmax

#define PREAMBLE "%1 = OpExtInstImport \"SPV_AMD_shader_trinary_minmax\"\n"
INSTANTIATE_TEST_SUITE_P(
    SPV_AMD_shader_trinary_minmax, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {PREAMBLE "%3 = OpExtInst %2 %1 FMin3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 1, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 UMin3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 2, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 SMin3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 3, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 FMax3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 4, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 UMax3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 5, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 SMax3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 6, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 FMid3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 7, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 UMid3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 8, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 SMid3AMD %4 %5 %6\n",
             Concatenate(
                 {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                  MakeVector("SPV_AMD_shader_trinary_minmax")),
                  MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 9, 4, 5, 6})})},
        })));
#undef PREAMBLE

// SPV_AMD_gcn_shader

#define PREAMBLE "%1 = OpExtInstImport \"SPV_AMD_gcn_shader\"\n"
INSTANTIATE_TEST_SUITE_P(
    SPV_AMD_gcn_shader, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {PREAMBLE "%3 = OpExtInst %2 %1 CubeFaceIndexAMD %4\n",
             Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                          MakeVector("SPV_AMD_gcn_shader")),
                          MakeInstruction(spv::Op::OpExtInst,
                                          {2, 3, 1, 1, 4})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 CubeFaceCoordAMD %4\n",
             Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                          MakeVector("SPV_AMD_gcn_shader")),
                          MakeInstruction(spv::Op::OpExtInst,
                                          {2, 3, 1, 2, 4})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 TimeAMD\n",
             Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                          MakeVector("SPV_AMD_gcn_shader")),
                          MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, 3})})},
        })));
#undef PREAMBLE

// SPV_AMD_shader_ballot

#define PREAMBLE "%1 = OpExtInstImport \"SPV_AMD_shader_ballot\"\n"
INSTANTIATE_TEST_SUITE_P(
    SPV_AMD_shader_ballot, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {PREAMBLE "%3 = OpExtInst %2 %1 SwizzleInvocationsAMD %4 %5\n",
             Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                          MakeVector("SPV_AMD_shader_ballot")),
                          MakeInstruction(spv::Op::OpExtInst,
                                          {2, 3, 1, 1, 4, 5})})},
            {PREAMBLE
             "%3 = OpExtInst %2 %1 SwizzleInvocationsMaskedAMD %4 %5\n",
             Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                          MakeVector("SPV_AMD_shader_ballot")),
                          MakeInstruction(spv::Op::OpExtInst,
                                          {2, 3, 1, 2, 4, 5})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 WriteInvocationAMD %4 %5 %6\n",
             Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                          MakeVector("SPV_AMD_shader_ballot")),
                          MakeInstruction(spv::Op::OpExtInst,
                                          {2, 3, 1, 3, 4, 5, 6})})},
            {PREAMBLE "%3 = OpExtInst %2 %1 MbcntAMD %4\n",
             Concatenate({MakeInstruction(spv::Op::OpExtInstImport, {1},
                                          MakeVector("SPV_AMD_shader_ballot")),
                          MakeInstruction(spv::Op::OpExtInst,
                                          {2, 3, 1, 4, 4})})},
        })));
#undef PREAMBLE

// SPV_KHR_variable_pointers

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_variable_pointers, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpCapability VariablePointers\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::VariablePointers})},
            {"OpCapability VariablePointersStorageBuffer\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::VariablePointersStorageBuffer})},
        })));

// SPV_KHR_vulkan_memory_model

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_vulkan_memory_model, ExtensionRoundTripTest,
    // We'll get coverage over operand tables by trying the universal
    // environments, and at least one specific environment.
    //
    // Note: SPV_KHR_vulkan_memory_model adds scope enum value QueueFamilyKHR.
    // Scope enums are used in ID definitions elsewhere, that don't know they
    // are using particular enums.  So the assembler doesn't support assembling
    // those enums names into the corresponding values.  So there is no asm/dis
    // tests for those enums.
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpCapability VulkanMemoryModel\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::VulkanMemoryModelKHR})},
            {"OpCapability VulkanMemoryModelDeviceScope\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::VulkanMemoryModelDeviceScopeKHR})},
            {"OpMemoryModel Logical Vulkan\n",
             MakeInstruction(spv::Op::OpMemoryModel,
                             {(uint32_t)spv::AddressingModel::Logical,
                              (uint32_t)spv::MemoryModel::VulkanKHR})},
            {"OpStore %1 %2 MakePointerAvailable %3\n",
             MakeInstruction(
                 spv::Op::OpStore,
                 {1, 2,
                  (uint32_t)spv::MemoryAccessMask::MakePointerAvailableKHR,
                  3})},
            {"OpStore %1 %2 Volatile|MakePointerAvailable %3\n",
             MakeInstruction(
                 spv::Op::OpStore,
                 {1, 2,
                  int(spv::MemoryAccessMask::MakePointerAvailableKHR) |
                      int(spv::MemoryAccessMask::Volatile),
                  3})},
            {"OpStore %1 %2 Aligned|MakePointerAvailable 4 %3\n",
             MakeInstruction(
                 spv::Op::OpStore,
                 {1, 2,
                  int(spv::MemoryAccessMask::MakePointerAvailableKHR) |
                      int(spv::MemoryAccessMask::Aligned),
                  4, 3})},
            {"OpStore %1 %2 MakePointerAvailable|NonPrivatePointer %3\n",
             MakeInstruction(
                 spv::Op::OpStore,
                 {1, 2,
                  int(spv::MemoryAccessMask::MakePointerAvailableKHR) |
                      int(spv::MemoryAccessMask::NonPrivatePointerKHR),
                  3})},
            {"%2 = OpLoad %1 %3 MakePointerVisible %4\n",
             MakeInstruction(
                 spv::Op::OpLoad,
                 {1, 2, 3,
                  (uint32_t)spv::MemoryAccessMask::MakePointerVisibleKHR, 4})},
            {"%2 = OpLoad %1 %3 Volatile|MakePointerVisible %4\n",
             MakeInstruction(
                 spv::Op::OpLoad,
                 {1, 2, 3,
                  int(spv::MemoryAccessMask::MakePointerVisibleKHR) |
                      int(spv::MemoryAccessMask::Volatile),
                  4})},
            {"%2 = OpLoad %1 %3 Aligned|MakePointerVisible 8 %4\n",
             MakeInstruction(
                 spv::Op::OpLoad,
                 {1, 2, 3,
                  int(spv::MemoryAccessMask::MakePointerVisibleKHR) |
                      int(spv::MemoryAccessMask::Aligned),
                  8, 4})},
            {"%2 = OpLoad %1 %3 MakePointerVisible|NonPrivatePointer "
             "%4\n",
             MakeInstruction(
                 spv::Op::OpLoad,
                 {1, 2, 3,
                  int(spv::MemoryAccessMask::MakePointerVisibleKHR) |
                      int(spv::MemoryAccessMask::NonPrivatePointerKHR),
                  4})},
            {"OpCopyMemory %1 %2 "
             "MakePointerAvailable|"
             "MakePointerVisible|"
             "NonPrivatePointer "
             "%3 %4\n",
             MakeInstruction(
                 spv::Op::OpCopyMemory,
                 {1, 2,
                  (int(spv::MemoryAccessMask::MakePointerVisibleKHR) |
                   int(spv::MemoryAccessMask::MakePointerAvailableKHR) |
                   int(spv::MemoryAccessMask::NonPrivatePointerKHR)),
                  3, 4})},
            {"OpCopyMemorySized %1 %2 %3 "
             "MakePointerAvailable|"
             "MakePointerVisible|"
             "NonPrivatePointer "
             "%4 %5\n",
             MakeInstruction(
                 spv::Op::OpCopyMemorySized,
                 {1, 2, 3,
                  (int(spv::MemoryAccessMask::MakePointerVisibleKHR) |
                   int(spv::MemoryAccessMask::MakePointerAvailableKHR) |
                   int(spv::MemoryAccessMask::NonPrivatePointerKHR)),
                  4, 5})},
            // Image operands
            {"OpImageWrite %1 %2 %3 MakeTexelAvailable "
             "%4\n",
             MakeInstruction(
                 spv::Op::OpImageWrite,
                 {1, 2, 3, int(spv::ImageOperandsMask::MakeTexelAvailableKHR),
                  4})},
            {"OpImageWrite %1 %2 %3 MakeTexelAvailable|NonPrivateTexel "
             "%4\n",
             MakeInstruction(
                 spv::Op::OpImageWrite,
                 {1, 2, 3,
                  int(spv::ImageOperandsMask::MakeTexelAvailableKHR) |
                      int(spv::ImageOperandsMask::NonPrivateTexelKHR),
                  4})},
            {"OpImageWrite %1 %2 %3 "
             "MakeTexelAvailable|NonPrivateTexel|VolatileTexel "
             "%4\n",
             MakeInstruction(
                 spv::Op::OpImageWrite,
                 {1, 2, 3,
                  int(spv::ImageOperandsMask::MakeTexelAvailableKHR) |
                      int(spv::ImageOperandsMask::NonPrivateTexelKHR) |
                      int(spv::ImageOperandsMask::VolatileTexelKHR),
                  4})},
            {"%2 = OpImageRead %1 %3 %4 MakeTexelVisible "
             "%5\n",
             MakeInstruction(spv::Op::OpImageRead,
                             {1, 2, 3, 4,
                              int(spv::ImageOperandsMask::MakeTexelVisibleKHR),
                              5})},
            {"%2 = OpImageRead %1 %3 %4 "
             "MakeTexelVisible|NonPrivateTexel "
             "%5\n",
             MakeInstruction(
                 spv::Op::OpImageRead,
                 {1, 2, 3, 4,
                  int(spv::ImageOperandsMask::MakeTexelVisibleKHR) |
                      int(spv::ImageOperandsMask::NonPrivateTexelKHR),
                  5})},
            {"%2 = OpImageRead %1 %3 %4 "
             "MakeTexelVisible|NonPrivateTexel|VolatileTexel "
             "%5\n",
             MakeInstruction(
                 spv::Op::OpImageRead,
                 {1, 2, 3, 4,
                  int(spv::ImageOperandsMask::MakeTexelVisibleKHR) |
                      int(spv::ImageOperandsMask::NonPrivateTexelKHR) |
                      int(spv::ImageOperandsMask::VolatileTexelKHR),
                  5})},

            // Memory semantics ID values are numbers put into a SPIR-V
            // constant integer referenced by Id. There is no token for
            // them, and so no assembler or disassembler support required.
            // Similar for Scope ID.
        })));

// SPV_GOOGLE_decorate_string

// Now that OpDecorateString is the preferred spelling for
// OpDecorateStringGOOGLE use that name in round trip tests, and the GOOGLE
// name in an assembly-only test.

INSTANTIATE_TEST_SUITE_P(
    SPV_GOOGLE_decorate_string, ExtensionRoundTripTest,
    Combine(
        // We'll get coverage over operand tables by trying the universal
        // environments, and at least one specific environment.
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_UNIVERSAL_1_2, SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpDecorateString %1 UserSemantic \"ABC\"\n",
             MakeInstruction(spv::Op::OpDecorateStringGOOGLE,
                             {1, (uint32_t)spv::Decoration::HlslSemanticGOOGLE},
                             MakeVector("ABC"))},
            {"OpDecorateString %1 UserSemantic \"ABC\"\n",
             MakeInstruction(spv::Op::OpDecorateString,
                             {1, (uint32_t)spv::Decoration::UserSemantic},
                             MakeVector("ABC"))},
            {"OpMemberDecorateString %1 3 UserSemantic \"DEF\"\n",
             MakeInstruction(spv::Op::OpMemberDecorateStringGOOGLE,
                             {1, 3, (uint32_t)spv::Decoration::UserSemantic},
                             MakeVector("DEF"))},
            {"OpMemberDecorateString %1 3 UserSemantic \"DEF\"\n",
             MakeInstruction(spv::Op::OpMemberDecorateString,
                             {1, 3, (uint32_t)spv::Decoration::UserSemantic},
                             MakeVector("DEF"))},
        })));

INSTANTIATE_TEST_SUITE_P(
    SPV_GOOGLE_decorate_string, ExtensionAssemblyTest,
    Combine(
        // We'll get coverage over operand tables by trying the universal
        // environments, and at least one specific environment.
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_UNIVERSAL_1_2, SPV_ENV_VULKAN_1_0),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpDecorateStringGOOGLE %1 HlslSemanticGOOGLE \"ABC\"\n",
             MakeInstruction(spv::Op::OpDecorateStringGOOGLE,
                             {1, (uint32_t)spv::Decoration::HlslSemanticGOOGLE},
                             MakeVector("ABC"))},
            {"OpMemberDecorateStringGOOGLE %1 3 HlslSemanticGOOGLE \"DEF\"\n",
             MakeInstruction(spv::Op::OpMemberDecorateStringGOOGLE,
                             {1, 3,
                              (uint32_t)spv::Decoration::HlslSemanticGOOGLE},
                             MakeVector("DEF"))},
        })));

// SPV_GOOGLE_hlsl_functionality1

// Now that CounterBuffer is the preferred spelling for HlslCounterBufferGOOGLE,
// use that name in round trip tests, and the GOOGLE name in an assembly-only
// test.
INSTANTIATE_TEST_SUITE_P(
    SPV_GOOGLE_hlsl_functionality1, ExtensionRoundTripTest,
    Combine(
        // We'll get coverage over operand tables by trying the universal
        // environments, and at least one specific environment.
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_UNIVERSAL_1_2, SPV_ENV_VULKAN_1_0),
        // HlslSemanticGOOGLE is tested in SPV_GOOGLE_decorate_string, since
        // they are coupled together.
        ValuesIn(std::vector<AssemblyCase>{
            {"OpDecorateId %1 CounterBuffer %2\n",
             MakeInstruction(
                 spv::Op::OpDecorateId,
                 {1, (uint32_t)spv::Decoration::HlslCounterBufferGOOGLE, 2})},
            {"OpDecorateId %1 CounterBuffer %2\n",
             MakeInstruction(spv::Op::OpDecorateId,
                             {1, (uint32_t)spv::Decoration::CounterBuffer, 2})},
        })));

INSTANTIATE_TEST_SUITE_P(
    SPV_GOOGLE_hlsl_functionality1, ExtensionAssemblyTest,
    Combine(
        // We'll get coverage over operand tables by trying the universal
        // environments, and at least one specific environment.
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_UNIVERSAL_1_2, SPV_ENV_VULKAN_1_0),
        // HlslSemanticGOOGLE is tested in SPV_GOOGLE_decorate_string, since
        // they are coupled together.
        ValuesIn(std::vector<AssemblyCase>{
            {"OpDecorateId %1 HlslCounterBufferGOOGLE %2\n",
             MakeInstruction(
                 spv::Op::OpDecorateId,
                 {1, (uint32_t)spv::Decoration::HlslCounterBufferGOOGLE, 2})},
        })));

// SPV_NV_viewport_array2

INSTANTIATE_TEST_SUITE_P(
    SPV_NV_viewport_array2, ExtensionRoundTripTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                   SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3,
                   SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpExtension \"SPV_NV_viewport_array2\"\n",
                 MakeInstruction(spv::Op::OpExtension,
                                 MakeVector("SPV_NV_viewport_array2"))},
                // The EXT and NV extensions have the same token number for this
                // capability.
                {"OpCapability ShaderViewportIndexLayerEXT\n",
                 MakeInstruction(
                     spv::Op::OpCapability,
                     {(uint32_t)spv::Capability::ShaderViewportIndexLayerNV})},
                // Check the new capability's token number
                {"OpCapability ShaderViewportIndexLayerEXT\n",
                 MakeInstruction(spv::Op::OpCapability, {5254})},
                // Decorations
                {"OpDecorate %1 ViewportRelativeNV\n",
                 MakeInstruction(
                     spv::Op::OpDecorate,
                     {1, (uint32_t)spv::Decoration::ViewportRelativeNV})},
                {"OpDecorate %1 BuiltIn ViewportMaskNV\n",
                 MakeInstruction(spv::Op::OpDecorate,
                                 {1, (uint32_t)spv::Decoration::BuiltIn,
                                  (uint32_t)spv::BuiltIn::ViewportMaskNV})},
            })));

// SPV_NV_shader_subgroup_partitioned

INSTANTIATE_TEST_SUITE_P(
    SPV_NV_shader_subgroup_partitioned, ExtensionRoundTripTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_1),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpExtension \"SPV_NV_shader_subgroup_partitioned\"\n",
             MakeInstruction(spv::Op::OpExtension,
                             MakeVector("SPV_NV_shader_subgroup_partitioned"))},
            {"OpCapability GroupNonUniformPartitionedNV\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::GroupNonUniformPartitionedNV})},
            // Check the new capability's token number
            {"OpCapability GroupNonUniformPartitionedNV\n",
             MakeInstruction(spv::Op::OpCapability, {5297})},
            {"%2 = OpGroupNonUniformPartitionNV %1 %3\n",
             MakeInstruction(spv::Op::OpGroupNonUniformPartitionNV, {1, 2, 3})},
            // Check the new instruction's token number
            {"%2 = OpGroupNonUniformPartitionNV %1 %3\n",
             MakeInstruction(static_cast<spv::Op>(5296), {1, 2, 3})},
            // Check the new group operations
            {"%2 = OpGroupIAdd %1 %3 PartitionedReduceNV %4\n",
             MakeInstruction(
                 spv::Op::OpGroupIAdd,
                 {1, 2, 3, (uint32_t)spv::GroupOperation::PartitionedReduceNV,
                  4})},
            {"%2 = OpGroupIAdd %1 %3 PartitionedReduceNV %4\n",
             MakeInstruction(spv::Op::OpGroupIAdd, {1, 2, 3, 6, 4})},
            {"%2 = OpGroupIAdd %1 %3 PartitionedInclusiveScanNV %4\n",
             MakeInstruction(
                 spv::Op::OpGroupIAdd,
                 {1, 2, 3,
                  (uint32_t)spv::GroupOperation::PartitionedInclusiveScanNV,
                  4})},
            {"%2 = OpGroupIAdd %1 %3 PartitionedInclusiveScanNV %4\n",
             MakeInstruction(spv::Op::OpGroupIAdd, {1, 2, 3, 7, 4})},
            {"%2 = OpGroupIAdd %1 %3 PartitionedExclusiveScanNV %4\n",
             MakeInstruction(
                 spv::Op::OpGroupIAdd,
                 {1, 2, 3,
                  (uint32_t)spv::GroupOperation::PartitionedExclusiveScanNV,
                  4})},
            {"%2 = OpGroupIAdd %1 %3 PartitionedExclusiveScanNV %4\n",
             MakeInstruction(spv::Op::OpGroupIAdd, {1, 2, 3, 8, 4})},
        })));

// SPV_EXT_descriptor_indexing

INSTANTIATE_TEST_SUITE_P(
    SPV_EXT_descriptor_indexing, ExtensionRoundTripTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
               SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_0,
               SPV_ENV_VULKAN_1_1),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpExtension \"SPV_EXT_descriptor_indexing\"\n",
             MakeInstruction(spv::Op::OpExtension,
                             MakeVector("SPV_EXT_descriptor_indexing"))},
            // Check capabilities, by name
            {"OpCapability ShaderNonUniform\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::ShaderNonUniformEXT})},
            {"OpCapability RuntimeDescriptorArray\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::RuntimeDescriptorArrayEXT})},
            {"OpCapability InputAttachmentArrayDynamicIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  InputAttachmentArrayDynamicIndexingEXT})},
            {"OpCapability UniformTexelBufferArrayDynamicIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  UniformTexelBufferArrayDynamicIndexingEXT})},
            {"OpCapability StorageTexelBufferArrayDynamicIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  StorageTexelBufferArrayDynamicIndexingEXT})},
            {"OpCapability UniformBufferArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  UniformBufferArrayNonUniformIndexingEXT})},
            {"OpCapability SampledImageArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  SampledImageArrayNonUniformIndexingEXT})},
            {"OpCapability StorageBufferArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  StorageBufferArrayNonUniformIndexingEXT})},
            {"OpCapability StorageImageArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  StorageImageArrayNonUniformIndexingEXT})},
            {"OpCapability InputAttachmentArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::
                                  InputAttachmentArrayNonUniformIndexingEXT})},
            {"OpCapability UniformTexelBufferArrayNonUniformIndexing\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::
                      UniformTexelBufferArrayNonUniformIndexingEXT})},
            {"OpCapability StorageTexelBufferArrayNonUniformIndexing\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::
                      StorageTexelBufferArrayNonUniformIndexingEXT})},
            // Check capabilities, by number
            {"OpCapability ShaderNonUniform\n",
             MakeInstruction(spv::Op::OpCapability, {5301})},
            {"OpCapability RuntimeDescriptorArray\n",
             MakeInstruction(spv::Op::OpCapability, {5302})},
            {"OpCapability InputAttachmentArrayDynamicIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5303})},
            {"OpCapability UniformTexelBufferArrayDynamicIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5304})},
            {"OpCapability StorageTexelBufferArrayDynamicIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5305})},
            {"OpCapability UniformBufferArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5306})},
            {"OpCapability SampledImageArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5307})},
            {"OpCapability StorageBufferArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5308})},
            {"OpCapability StorageImageArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5309})},
            {"OpCapability InputAttachmentArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5310})},
            {"OpCapability UniformTexelBufferArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5311})},
            {"OpCapability StorageTexelBufferArrayNonUniformIndexing\n",
             MakeInstruction(spv::Op::OpCapability, {5312})},

            // Check the decoration token
            {"OpDecorate %1 NonUniform\n",
             MakeInstruction(spv::Op::OpDecorate,
                             {1, (uint32_t)spv::Decoration::NonUniformEXT})},
            {"OpDecorate %1 NonUniform\n",
             MakeInstruction(spv::Op::OpDecorate, {1, 5300})},
        })));

// SPV_KHR_linkonce_odr

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_linkonce_odr, ExtensionRoundTripTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_3, SPV_ENV_VULKAN_1_0,
               SPV_ENV_VULKAN_1_1, SPV_ENV_VULKAN_1_2),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpExtension \"SPV_KHR_linkonce_odr\"\n",
             MakeInstruction(spv::Op::OpExtension,
                             MakeVector("SPV_KHR_linkonce_odr"))},
            {"OpDecorate %1 LinkageAttributes \"foobar\" LinkOnceODR\n",
             MakeInstruction(
                 spv::Op::OpDecorate,
                 Concatenate({{1, (uint32_t)spv::Decoration::LinkageAttributes},
                              MakeVector("foobar"),
                              {(uint32_t)spv::LinkageType::LinkOnceODR}}))},
        })));

// SPV_KHR_expect_assume

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_expect_assume, ExtensionRoundTripTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_3,
                   SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1, SPV_ENV_VULKAN_1_2),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpExtension \"SPV_KHR_expect_assume\"\n",
                 MakeInstruction(spv::Op::OpExtension,
                                 MakeVector("SPV_KHR_expect_assume"))},
                {"OpAssumeTrueKHR %1\n",
                 MakeInstruction(spv::Op::OpAssumeTrueKHR, {1})}})));
// SPV_KHR_subgroup_uniform_control_flow

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_subgroup_uniform_control_flow, ExtensionRoundTripTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_3,
                   SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1, SPV_ENV_VULKAN_1_2),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpExtension \"SPV_KHR_subgroup_uniform_control_flow\"\n",
                 MakeInstruction(
                     spv::Op::OpExtension,
                     MakeVector("SPV_KHR_subgroup_uniform_control_flow"))},
                {"OpExecutionMode %1 SubgroupUniformControlFlowKHR\n",
                 MakeInstruction(spv::Op::OpExecutionMode,
                                 {1, (uint32_t)spv::ExecutionMode::
                                         SubgroupUniformControlFlowKHR})},
            })));

// SPV_KHR_integer_dot_product

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_integer_dot_product, ExtensionRoundTripTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_5,
               SPV_ENV_UNIVERSAL_1_6, SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1,
               SPV_ENV_VULKAN_1_2, SPV_ENV_VULKAN_1_3),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpExtension \"SPV_KHR_integer_dot_product\"\n",
             MakeInstruction(spv::Op::OpExtension,
                             MakeVector("SPV_KHR_integer_dot_product"))},
            {"OpCapability DotProductInputAll\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::DotProductInputAllKHR})},
            {"OpCapability DotProductInput4x8Bit\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::DotProductInput4x8BitKHR})},
            {"OpCapability DotProductInput4x8BitPacked\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::DotProductInput4x8BitPackedKHR})},
            {"OpCapability DotProduct\n",
             MakeInstruction(spv::Op::OpCapability,
                             {(uint32_t)spv::Capability::DotProductKHR})},
            {"%2 = OpSDot %1 %3 %4\n",
             MakeInstruction(spv::Op::OpSDotKHR, {1, 2, 3, 4})},
            {"%2 = OpSDot %1 %3 %4 PackedVectorFormat4x8Bit\n",
             MakeInstruction(
                 spv::Op::OpSDotKHR,
                 {1, 2, 3, 4,
                  (uint32_t)
                      spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR})},
            {"%2 = OpUDot %1 %3 %4\n",
             MakeInstruction(spv::Op::OpUDotKHR, {1, 2, 3, 4})},
            {"%2 = OpUDot %1 %3 %4 PackedVectorFormat4x8Bit\n",
             MakeInstruction(
                 spv::Op::OpUDotKHR,
                 {1, 2, 3, 4,
                  (uint32_t)
                      spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR})},
            {"%2 = OpSUDot %1 %3 %4\n",
             MakeInstruction(spv::Op::OpSUDotKHR, {1, 2, 3, 4})},
            {"%2 = OpSUDot %1 %3 %4 PackedVectorFormat4x8Bit\n",
             MakeInstruction(
                 spv::Op::OpSUDotKHR,
                 {1, 2, 3, 4,
                  (uint32_t)
                      spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR})},
            {"%2 = OpSDotAccSat %1 %3 %4 %5\n",
             MakeInstruction(spv::Op::OpSDotAccSatKHR, {1, 2, 3, 4, 5})},
            {"%2 = OpSDotAccSat %1 %3 %4 %5 PackedVectorFormat4x8Bit\n",
             MakeInstruction(
                 spv::Op::OpSDotAccSatKHR,
                 {1, 2, 3, 4, 5,
                  (uint32_t)
                      spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR})},
            {"%2 = OpUDotAccSat %1 %3 %4 %5\n",
             MakeInstruction(spv::Op::OpUDotAccSatKHR, {1, 2, 3, 4, 5})},
            {"%2 = OpUDotAccSat %1 %3 %4 %5 PackedVectorFormat4x8Bit\n",
             MakeInstruction(
                 spv::Op::OpUDotAccSatKHR,
                 {1, 2, 3, 4, 5,
                  (uint32_t)
                      spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR})},
            {"%2 = OpSUDotAccSat %1 %3 %4 %5\n",
             MakeInstruction(spv::Op::OpSUDotAccSatKHR, {1, 2, 3, 4, 5})},
            {"%2 = OpSUDotAccSat %1 %3 %4 %5 PackedVectorFormat4x8Bit\n",
             MakeInstruction(
                 spv::Op::OpSUDotAccSatKHR,
                 {1, 2, 3, 4, 5,
                  (uint32_t)
                      spv::PackedVectorFormat::PackedVectorFormat4x8BitKHR})},
        })));

// SPV_KHR_bit_instructions

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_bit_instructions, ExtensionRoundTripTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_5,
                   SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1, SPV_ENV_VULKAN_1_2),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpExtension \"SPV_KHR_bit_instructions\"\n",
                 MakeInstruction(spv::Op::OpExtension,
                                 MakeVector("SPV_KHR_bit_instructions"))},
                {"OpCapability BitInstructions\n",
                 MakeInstruction(spv::Op::OpCapability,
                                 {(uint32_t)spv::Capability::BitInstructions})},
            })));

// SPV_KHR_uniform_group_instructions

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_uniform_group_instructions, ExtensionRoundTripTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_5,
               SPV_ENV_UNIVERSAL_1_6, SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1,
               SPV_ENV_VULKAN_1_2, SPV_ENV_VULKAN_1_3),
        ValuesIn(std::vector<AssemblyCase>{
            {"OpExtension \"SPV_KHR_uniform_group_instructions\"\n",
             MakeInstruction(spv::Op::OpExtension,
                             MakeVector("SPV_KHR_uniform_group_instructions"))},
            {"OpCapability GroupUniformArithmeticKHR\n",
             MakeInstruction(
                 spv::Op::OpCapability,
                 {(uint32_t)spv::Capability::GroupUniformArithmeticKHR})},
            {"%2 = OpGroupIMulKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupIMulKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
            {"%2 = OpGroupFMulKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupFMulKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
            {"%2 = OpGroupBitwiseAndKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupBitwiseAndKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
            {"%2 = OpGroupBitwiseOrKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupBitwiseOrKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
            {"%2 = OpGroupBitwiseXorKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupBitwiseXorKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
            {"%2 = OpGroupLogicalAndKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupLogicalAndKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
            {"%2 = OpGroupLogicalOrKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupLogicalOrKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
            {"%2 = OpGroupLogicalXorKHR %1 %3 Reduce %4\n",
             MakeInstruction(spv::Op::OpGroupLogicalXorKHR,
                             {1, 2, 3, (uint32_t)spv::GroupOperation::Reduce,
                              4})},
        })));

// SPV_KHR_subgroup_rotate

INSTANTIATE_TEST_SUITE_P(
    SPV_KHR_subgroup_rotate, ExtensionRoundTripTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_6,
                   SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1, SPV_ENV_VULKAN_1_2,
                   SPV_ENV_VULKAN_1_3, SPV_ENV_OPENCL_2_1),
            ValuesIn(std::vector<AssemblyCase>{
                {"OpExtension \"SPV_KHR_subgroup_rotate\"\n",
                 MakeInstruction(spv::Op::OpExtension,
                                 MakeVector("SPV_KHR_subgroup_rotate"))},
                {"OpCapability GroupNonUniformRotateKHR\n",
                 MakeInstruction(
                     spv::Op::OpCapability,
                     {(uint32_t)spv::Capability::GroupNonUniformRotateKHR})},
                {"%2 = OpGroupNonUniformRotateKHR %1 %3 %4 %5\n",
                 MakeInstruction(spv::Op::OpGroupNonUniformRotateKHR,
                                 {1, 2, 3, 4, 5})},
                {"%2 = OpGroupNonUniformRotateKHR %1 %3 %4 %5 %6\n",
                 MakeInstruction(spv::Op::OpGroupNonUniformRotateKHR,
                                 {1, 2, 3, 4, 5, 6})},
            })));

}  // namespace
}  // namespace spvtools
