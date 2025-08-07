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

#include "source/enum_set.h"
#include "source/table2.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::ElementsIn;

// Capabilities required by an Opcode.
struct ExpectedOpCodeCapabilities {
  spv::Op opcode;
  CapabilitySet capabilities;
};

using OpcodeTableCapabilitiesTest =
    ::testing::TestWithParam<ExpectedOpCodeCapabilities>;

TEST_P(OpcodeTableCapabilitiesTest, TableEntryMatchesExpectedCapabilities) {
  const spvtools::InstructionDesc* desc = nullptr;
  ASSERT_EQ(SPV_SUCCESS, spvtools::LookupOpcode(GetParam().opcode, &desc));
  auto caps = desc->capabilities();
  EXPECT_EQ(ElementsIn(GetParam().capabilities),
            ElementsIn(CapabilitySet(static_cast<uint32_t>(caps.size()),
                                     caps.data())));
}

INSTANTIATE_TEST_SUITE_P(
    TableRowTest, OpcodeTableCapabilitiesTest,
    // Spot-check a few opcodes.
    ::testing::Values(
        ExpectedOpCodeCapabilities{spv::Op::OpImageQuerySize,
                                   CapabilitySet{spv::Capability::Kernel,
                                                 spv::Capability::ImageQuery}},
        ExpectedOpCodeCapabilities{spv::Op::OpImageQuerySizeLod,
                                   CapabilitySet{spv::Capability::Kernel,
                                                 spv::Capability::ImageQuery}},
        ExpectedOpCodeCapabilities{spv::Op::OpImageQueryLevels,
                                   CapabilitySet{spv::Capability::Kernel,
                                                 spv::Capability::ImageQuery}},
        ExpectedOpCodeCapabilities{spv::Op::OpImageQuerySamples,
                                   CapabilitySet{spv::Capability::Kernel,
                                                 spv::Capability::ImageQuery}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpImageSparseSampleImplicitLod,
            CapabilitySet{spv::Capability::SparseResidency}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpCopyMemorySized,
            CapabilitySet{spv::Capability::Addresses,
                          spv::Capability::UntypedPointersKHR}},
        ExpectedOpCodeCapabilities{spv::Op::OpArrayLength,
                                   CapabilitySet{spv::Capability::Shader}},
        ExpectedOpCodeCapabilities{spv::Op::OpFunction, CapabilitySet()},
        ExpectedOpCodeCapabilities{spv::Op::OpConvertFToS, CapabilitySet()},
        ExpectedOpCodeCapabilities{
            spv::Op::OpEmitStreamVertex,
            CapabilitySet{spv::Capability::GeometryStreams}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpTypeNamedBarrier,
            CapabilitySet{spv::Capability::NamedBarrier}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpGetKernelMaxNumSubgroups,
            CapabilitySet{spv::Capability::SubgroupDispatch}},
        ExpectedOpCodeCapabilities{spv::Op::OpImageQuerySamples,
                                   CapabilitySet{spv::Capability::Kernel,
                                                 spv::Capability::ImageQuery}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpImageSparseSampleImplicitLod,
            CapabilitySet{spv::Capability::SparseResidency}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpCopyMemorySized,
            CapabilitySet{spv::Capability::Addresses,
                          spv::Capability::UntypedPointersKHR}},
        ExpectedOpCodeCapabilities{spv::Op::OpArrayLength,
                                   CapabilitySet{spv::Capability::Shader}},
        ExpectedOpCodeCapabilities{spv::Op::OpFunction, CapabilitySet()},
        ExpectedOpCodeCapabilities{spv::Op::OpConvertFToS, CapabilitySet()},
        ExpectedOpCodeCapabilities{
            spv::Op::OpEmitStreamVertex,
            CapabilitySet{spv::Capability::GeometryStreams}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpTypeNamedBarrier,
            CapabilitySet{spv::Capability::NamedBarrier}},
        ExpectedOpCodeCapabilities{
            spv::Op::OpGetKernelMaxNumSubgroups,
            CapabilitySet{spv::Capability::SubgroupDispatch}}));

}  // namespace
}  // namespace spvtools
