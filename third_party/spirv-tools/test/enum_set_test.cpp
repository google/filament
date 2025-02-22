// Copyright (c) 2016 Google Inc.
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

#include <algorithm>
#include <array>
#include <random>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::ElementsIn;
using ::testing::Eq;
using ::testing::Values;
using ::testing::ValuesIn;

enum class TestEnum : uint32_t {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5,
  EIGHT = 8,
  TWENTY = 20,
  TWENTY_FOUR = 24,
  THIRTY = 30,
  ONE_HUNDRED = 100,
  ONE_HUNDRED_FIFTY = 150,
  TWO_HUNDRED = 200,
  THREE_HUNDRED = 300,
  FOUR_HUNDRED = 400,
  FIVE_HUNDRED = 500,
  SIX_HUNDRED = 600,
};

constexpr std::array kCapabilities{
    spv::Capability::Matrix,
    spv::Capability::Shader,
    spv::Capability::Geometry,
    spv::Capability::Tessellation,
    spv::Capability::Addresses,
    spv::Capability::Linkage,
    spv::Capability::Kernel,
    spv::Capability::Vector16,
    spv::Capability::Float16Buffer,
    spv::Capability::Float16,
    spv::Capability::Float64,
    spv::Capability::Int64,
    spv::Capability::Int64Atomics,
    spv::Capability::ImageBasic,
    spv::Capability::ImageReadWrite,
    spv::Capability::ImageMipmap,
    spv::Capability::Pipes,
    spv::Capability::Groups,
    spv::Capability::DeviceEnqueue,
    spv::Capability::LiteralSampler,
    spv::Capability::AtomicStorage,
    spv::Capability::Int16,
    spv::Capability::TessellationPointSize,
    spv::Capability::GeometryPointSize,
    spv::Capability::ImageGatherExtended,
    spv::Capability::StorageImageMultisample,
    spv::Capability::UniformBufferArrayDynamicIndexing,
    spv::Capability::SampledImageArrayDynamicIndexing,
    spv::Capability::StorageBufferArrayDynamicIndexing,
    spv::Capability::StorageImageArrayDynamicIndexing,
    spv::Capability::ClipDistance,
    spv::Capability::CullDistance,
    spv::Capability::ImageCubeArray,
    spv::Capability::SampleRateShading,
    spv::Capability::ImageRect,
    spv::Capability::SampledRect,
    spv::Capability::GenericPointer,
    spv::Capability::Int8,
    spv::Capability::InputAttachment,
    spv::Capability::SparseResidency,
    spv::Capability::MinLod,
    spv::Capability::Sampled1D,
    spv::Capability::Image1D,
    spv::Capability::SampledCubeArray,
    spv::Capability::SampledBuffer,
    spv::Capability::ImageBuffer,
    spv::Capability::ImageMSArray,
    spv::Capability::StorageImageExtendedFormats,
    spv::Capability::ImageQuery,
    spv::Capability::DerivativeControl,
    spv::Capability::InterpolationFunction,
    spv::Capability::TransformFeedback,
    spv::Capability::GeometryStreams,
    spv::Capability::StorageImageReadWithoutFormat,
    spv::Capability::StorageImageWriteWithoutFormat,
    spv::Capability::MultiViewport,
    spv::Capability::SubgroupDispatch,
    spv::Capability::NamedBarrier,
    spv::Capability::PipeStorage,
    spv::Capability::GroupNonUniform,
    spv::Capability::GroupNonUniformVote,
    spv::Capability::GroupNonUniformArithmetic,
    spv::Capability::GroupNonUniformBallot,
    spv::Capability::GroupNonUniformShuffle,
    spv::Capability::GroupNonUniformShuffleRelative,
    spv::Capability::GroupNonUniformClustered,
    spv::Capability::GroupNonUniformQuad,
    spv::Capability::ShaderLayer,
    spv::Capability::ShaderViewportIndex,
    spv::Capability::UniformDecoration,
    spv::Capability::CoreBuiltinsARM,
    spv::Capability::FragmentShadingRateKHR,
    spv::Capability::SubgroupBallotKHR,
    spv::Capability::DrawParameters,
    spv::Capability::WorkgroupMemoryExplicitLayoutKHR,
    spv::Capability::WorkgroupMemoryExplicitLayout8BitAccessKHR,
    spv::Capability::WorkgroupMemoryExplicitLayout16BitAccessKHR,
    spv::Capability::SubgroupVoteKHR,
    spv::Capability::StorageBuffer16BitAccess,
    spv::Capability::StorageUniformBufferBlock16,
    spv::Capability::StorageUniform16,
    spv::Capability::UniformAndStorageBuffer16BitAccess,
    spv::Capability::StoragePushConstant16,
    spv::Capability::StorageInputOutput16,
    spv::Capability::DeviceGroup,
    spv::Capability::MultiView,
    spv::Capability::VariablePointersStorageBuffer,
    spv::Capability::VariablePointers,
    spv::Capability::AtomicStorageOps,
    spv::Capability::SampleMaskPostDepthCoverage,
    spv::Capability::StorageBuffer8BitAccess,
    spv::Capability::UniformAndStorageBuffer8BitAccess,
    spv::Capability::StoragePushConstant8,
    spv::Capability::DenormPreserve,
    spv::Capability::DenormFlushToZero,
    spv::Capability::SignedZeroInfNanPreserve,
    spv::Capability::RoundingModeRTE,
    spv::Capability::RoundingModeRTZ,
    spv::Capability::RayQueryProvisionalKHR,
    spv::Capability::RayQueryKHR,
    spv::Capability::RayTraversalPrimitiveCullingKHR,
    spv::Capability::RayTracingKHR,
    spv::Capability::Float16ImageAMD,
    spv::Capability::ImageGatherBiasLodAMD,
    spv::Capability::FragmentMaskAMD,
    spv::Capability::StencilExportEXT,
    spv::Capability::ImageReadWriteLodAMD,
    spv::Capability::Int64ImageEXT,
    spv::Capability::ShaderClockKHR,
    spv::Capability::SampleMaskOverrideCoverageNV,
    spv::Capability::GeometryShaderPassthroughNV,
    spv::Capability::ShaderViewportIndexLayerEXT,
    spv::Capability::ShaderViewportIndexLayerNV,
    spv::Capability::ShaderViewportMaskNV,
    spv::Capability::ShaderStereoViewNV,
    spv::Capability::PerViewAttributesNV,
    spv::Capability::FragmentFullyCoveredEXT,
    spv::Capability::MeshShadingNV,
    spv::Capability::ImageFootprintNV,
    spv::Capability::MeshShadingEXT,
    spv::Capability::FragmentBarycentricKHR,
    spv::Capability::FragmentBarycentricNV,
    spv::Capability::ComputeDerivativeGroupQuadsNV,
    spv::Capability::FragmentDensityEXT,
    spv::Capability::ShadingRateNV,
    spv::Capability::GroupNonUniformPartitionedNV,
    spv::Capability::ShaderNonUniform,
    spv::Capability::ShaderNonUniformEXT,
    spv::Capability::RuntimeDescriptorArray,
    spv::Capability::RuntimeDescriptorArrayEXT,
    spv::Capability::InputAttachmentArrayDynamicIndexing,
    spv::Capability::InputAttachmentArrayDynamicIndexingEXT,
    spv::Capability::UniformTexelBufferArrayDynamicIndexing,
    spv::Capability::UniformTexelBufferArrayDynamicIndexingEXT,
    spv::Capability::StorageTexelBufferArrayDynamicIndexing,
    spv::Capability::StorageTexelBufferArrayDynamicIndexingEXT,
    spv::Capability::UniformBufferArrayNonUniformIndexing,
    spv::Capability::UniformBufferArrayNonUniformIndexingEXT,
    spv::Capability::SampledImageArrayNonUniformIndexing,
    spv::Capability::SampledImageArrayNonUniformIndexingEXT,
    spv::Capability::StorageBufferArrayNonUniformIndexing,
    spv::Capability::StorageBufferArrayNonUniformIndexingEXT,
    spv::Capability::StorageImageArrayNonUniformIndexing,
    spv::Capability::StorageImageArrayNonUniformIndexingEXT,
    spv::Capability::InputAttachmentArrayNonUniformIndexing,
    spv::Capability::InputAttachmentArrayNonUniformIndexingEXT,
    spv::Capability::UniformTexelBufferArrayNonUniformIndexing,
    spv::Capability::UniformTexelBufferArrayNonUniformIndexingEXT,
    spv::Capability::StorageTexelBufferArrayNonUniformIndexing,
    spv::Capability::StorageTexelBufferArrayNonUniformIndexingEXT,
    spv::Capability::RayTracingNV,
    spv::Capability::RayTracingMotionBlurNV,
    spv::Capability::VulkanMemoryModel,
    spv::Capability::VulkanMemoryModelKHR,
    spv::Capability::VulkanMemoryModelDeviceScope,
    spv::Capability::VulkanMemoryModelDeviceScopeKHR,
    spv::Capability::PhysicalStorageBufferAddresses,
    spv::Capability::PhysicalStorageBufferAddressesEXT,
    spv::Capability::ComputeDerivativeGroupLinearNV,
    spv::Capability::RayTracingProvisionalKHR,
    spv::Capability::CooperativeMatrixNV,
    spv::Capability::FragmentShaderSampleInterlockEXT,
    spv::Capability::FragmentShaderShadingRateInterlockEXT,
    spv::Capability::ShaderSMBuiltinsNV,
    spv::Capability::FragmentShaderPixelInterlockEXT,
    spv::Capability::DemoteToHelperInvocation,
    spv::Capability::DemoteToHelperInvocationEXT,
    spv::Capability::RayTracingOpacityMicromapEXT,
    spv::Capability::ShaderInvocationReorderNV,
    spv::Capability::BindlessTextureNV,
    spv::Capability::SubgroupShuffleINTEL,
    spv::Capability::SubgroupBufferBlockIOINTEL,
    spv::Capability::SubgroupImageBlockIOINTEL,
    spv::Capability::SubgroupImageMediaBlockIOINTEL,
    spv::Capability::RoundToInfinityINTEL,
    spv::Capability::FloatingPointModeINTEL,
    spv::Capability::IntegerFunctions2INTEL,
    spv::Capability::FunctionPointersINTEL,
    spv::Capability::IndirectReferencesINTEL,
    spv::Capability::AsmINTEL,
    spv::Capability::AtomicFloat32MinMaxEXT,
    spv::Capability::AtomicFloat64MinMaxEXT,
    spv::Capability::AtomicFloat16MinMaxEXT,
    spv::Capability::VectorComputeINTEL,
    spv::Capability::VectorAnyINTEL,
    spv::Capability::ExpectAssumeKHR,
    spv::Capability::SubgroupAvcMotionEstimationINTEL,
    spv::Capability::SubgroupAvcMotionEstimationIntraINTEL,
    spv::Capability::SubgroupAvcMotionEstimationChromaINTEL,
    spv::Capability::VariableLengthArrayINTEL,
    spv::Capability::FunctionFloatControlINTEL,
    spv::Capability::FPGAMemoryAttributesINTEL,
    spv::Capability::FPFastMathModeINTEL,
    spv::Capability::ArbitraryPrecisionIntegersINTEL,
    spv::Capability::ArbitraryPrecisionFloatingPointINTEL,
    spv::Capability::UnstructuredLoopControlsINTEL,
    spv::Capability::FPGALoopControlsINTEL,
    spv::Capability::KernelAttributesINTEL,
    spv::Capability::FPGAKernelAttributesINTEL,
    spv::Capability::FPGAMemoryAccessesINTEL,
    spv::Capability::FPGAClusterAttributesINTEL,
    spv::Capability::LoopFuseINTEL,
    spv::Capability::FPGADSPControlINTEL,
    spv::Capability::MemoryAccessAliasingINTEL,
    spv::Capability::FPGAInvocationPipeliningAttributesINTEL,
    spv::Capability::FPGABufferLocationINTEL,
    spv::Capability::ArbitraryPrecisionFixedPointINTEL,
    spv::Capability::USMStorageClassesINTEL,
    spv::Capability::RuntimeAlignedAttributeINTEL,
    spv::Capability::IOPipesINTEL,
    spv::Capability::BlockingPipesINTEL,
    spv::Capability::FPGARegINTEL,
    spv::Capability::DotProductInputAll,
    spv::Capability::DotProductInputAllKHR,
    spv::Capability::DotProductInput4x8Bit,
    spv::Capability::DotProductInput4x8BitKHR,
    spv::Capability::DotProductInput4x8BitPacked,
    spv::Capability::DotProductInput4x8BitPackedKHR,
    spv::Capability::DotProduct,
    spv::Capability::DotProductKHR,
    spv::Capability::RayCullMaskKHR,
    spv::Capability::BitInstructions,
    spv::Capability::GroupNonUniformRotateKHR,
    spv::Capability::AtomicFloat32AddEXT,
    spv::Capability::AtomicFloat64AddEXT,
    spv::Capability::LongCompositesINTEL,
    spv::Capability::OptNoneINTEL,
    spv::Capability::AtomicFloat16AddEXT,
    spv::Capability::DebugInfoModuleINTEL,
    spv::Capability::SplitBarrierINTEL,
    spv::Capability::GroupUniformArithmeticKHR,
    spv::Capability::Max,
};

namespace {
std::vector<TestEnum> enumerateValuesFromToWithStep(size_t start, size_t end,
                                                    size_t step) {
  assert(end > start && "end > start");
  std::vector<TestEnum> orderedValues;
  for (size_t i = start; i < end; i += step) {
    orderedValues.push_back(static_cast<TestEnum>(i));
  }
  return orderedValues;
}

EnumSet<TestEnum> createSetUnorderedInsertion(
    const std::vector<TestEnum>& values) {
  std::vector shuffledValues(values.cbegin(), values.cend());
  std::mt19937 rng(0);
  std::shuffle(shuffledValues.begin(), shuffledValues.end(), rng);
  EnumSet<TestEnum> set;
  for (auto value : shuffledValues) {
    set.insert(value);
  }
  return set;
}
}  // namespace

TEST(EnumSet, IsEmpty1) {
  EnumSet<TestEnum> set;
  EXPECT_TRUE(set.empty());
  set.insert(TestEnum::ZERO);
  EXPECT_FALSE(set.empty());
}

TEST(EnumSet, IsEmpty2) {
  EnumSet<TestEnum> set;
  EXPECT_TRUE(set.empty());
  set.insert(TestEnum::ONE_HUNDRED_FIFTY);
  EXPECT_FALSE(set.empty());
}

TEST(EnumSet, IsEmpty3) {
  EnumSet<TestEnum> set(TestEnum::FOUR);
  EXPECT_FALSE(set.empty());
}

TEST(EnumSet, IsEmpty4) {
  EnumSet<TestEnum> set(TestEnum::THREE_HUNDRED);
  EXPECT_FALSE(set.empty());
}

TEST(EnumSetHasAnyOf, EmptySetEmptyQuery) {
  const EnumSet<TestEnum> set;
  const EnumSet<TestEnum> empty;
  EXPECT_TRUE(set.HasAnyOf(empty));
  EXPECT_TRUE(EnumSet<TestEnum>().HasAnyOf(EnumSet<TestEnum>()));
}

TEST(EnumSetHasAnyOf, MaskSetEmptyQuery) {
  EnumSet<TestEnum> set;
  const EnumSet<TestEnum> empty;
  set.insert(TestEnum::FIVE);
  set.insert(TestEnum::EIGHT);
  EXPECT_TRUE(set.HasAnyOf(empty));
}

TEST(EnumSetHasAnyOf, OverflowSetEmptyQuery) {
  EnumSet<TestEnum> set;
  const EnumSet<TestEnum> empty;
  set.insert(TestEnum::TWO_HUNDRED);
  set.insert(TestEnum::THREE_HUNDRED);
  EXPECT_TRUE(set.HasAnyOf(empty));
}

TEST(EnumSetHasAnyOf, EmptyQuery) {
  EnumSet<TestEnum> set;
  const EnumSet<TestEnum> empty;
  set.insert(TestEnum::FIVE);
  set.insert(TestEnum::EIGHT);
  set.insert(TestEnum::TWO_HUNDRED);
  set.insert(TestEnum::THREE_HUNDRED);
  EXPECT_TRUE(set.HasAnyOf(empty));
}

TEST(EnumSetHasAnyOf, EmptyQueryAlwaysTrue) {
  EnumSet<TestEnum> set;
  const EnumSet<TestEnum> empty;
  EXPECT_TRUE(set.HasAnyOf(empty));
  set.insert(TestEnum::FIVE);
  EXPECT_TRUE(set.HasAnyOf(empty));

  EXPECT_TRUE(
      EnumSet<TestEnum>(TestEnum::ONE_HUNDRED).HasAnyOf(EnumSet<TestEnum>()));
}

TEST(EnumSetHasAnyOf, ReflexiveMask) {
  EnumSet<TestEnum> set(TestEnum::THREE);
  set.insert(TestEnum::TWENTY_FOUR);
  set.insert(TestEnum::THIRTY);
  EXPECT_TRUE(set.HasAnyOf(set));
}

TEST(EnumSetHasAnyOf, ReflexiveOverflow) {
  EnumSet<TestEnum> set(TestEnum::TWO_HUNDRED);
  set.insert(TestEnum::TWO_HUNDRED);
  set.insert(TestEnum::FOUR_HUNDRED);
  EXPECT_TRUE(set.HasAnyOf(set));
}

TEST(EnumSetHasAnyOf, Reflexive) {
  EnumSet<TestEnum> set(TestEnum::THREE);
  set.insert(TestEnum::TWENTY_FOUR);
  set.insert(TestEnum::THREE_HUNDRED);
  set.insert(TestEnum::FOUR_HUNDRED);
  EXPECT_TRUE(set.HasAnyOf(set));
}

TEST(EnumSetHasAnyOf, EmptySetHasNone) {
  EnumSet<TestEnum> set;
  EnumSet<TestEnum> items;
  for (uint32_t i = 0; i < 200; ++i) {
    TestEnum enumValue = static_cast<TestEnum>(i);
    items.insert(enumValue);
    EXPECT_FALSE(set.HasAnyOf(items));
    EXPECT_FALSE(set.HasAnyOf(EnumSet<TestEnum>(enumValue)));
  }
}

TEST(EnumSetHasAnyOf, MaskSetMaskQuery) {
  EnumSet<TestEnum> set(TestEnum::ZERO);
  EnumSet<TestEnum> items(TestEnum::ONE);
  EXPECT_FALSE(set.HasAnyOf(items));
  set.insert(TestEnum::TWO);
  items.insert(TestEnum::THREE);
  EXPECT_FALSE(set.HasAnyOf(items));
  set.insert(TestEnum::THREE);
  EXPECT_TRUE(set.HasAnyOf(items));
  set.insert(TestEnum::FOUR);
  EXPECT_TRUE(set.HasAnyOf(items));
}

TEST(EnumSetHasAnyOf, OverflowSetOverflowQuery) {
  EnumSet<TestEnum> set(TestEnum::ONE_HUNDRED);
  EnumSet<TestEnum> items(TestEnum::TWO_HUNDRED);
  EXPECT_FALSE(set.HasAnyOf(items));
  set.insert(TestEnum::THREE_HUNDRED);
  items.insert(TestEnum::FOUR_HUNDRED);
  EXPECT_FALSE(set.HasAnyOf(items));
  set.insert(TestEnum::TWO_HUNDRED);
  EXPECT_TRUE(set.HasAnyOf(items));
  set.insert(TestEnum::FIVE_HUNDRED);
  EXPECT_TRUE(set.HasAnyOf(items));
}

TEST(EnumSetHasAnyOf, GeneralCase) {
  EnumSet<TestEnum> set(TestEnum::ZERO);
  EnumSet<TestEnum> items(TestEnum::ONE_HUNDRED);
  EXPECT_FALSE(set.HasAnyOf(items));
  set.insert(TestEnum::THREE_HUNDRED);
  items.insert(TestEnum::FOUR);
  EXPECT_FALSE(set.HasAnyOf(items));
  set.insert(TestEnum::FIVE);
  items.insert(TestEnum::FIVE_HUNDRED);
  EXPECT_FALSE(set.HasAnyOf(items));
  set.insert(TestEnum::FIVE_HUNDRED);
  EXPECT_TRUE(set.HasAnyOf(items));
  EXPECT_FALSE(set.HasAnyOf(EnumSet<TestEnum>(TestEnum::TWENTY)));
  EXPECT_FALSE(set.HasAnyOf(EnumSet<TestEnum>(TestEnum::SIX_HUNDRED)));
  EXPECT_TRUE(set.HasAnyOf(EnumSet<TestEnum>(TestEnum::FIVE)));
  EXPECT_TRUE(set.HasAnyOf(EnumSet<TestEnum>(TestEnum::THREE_HUNDRED)));
  EXPECT_TRUE(set.HasAnyOf(EnumSet<TestEnum>(TestEnum::ZERO)));
}

TEST(EnumSet, DefaultIsEmpty) {
  EnumSet<TestEnum> set;
  for (uint32_t i = 0; i < 1000; ++i) {
    EXPECT_FALSE(set.contains(static_cast<TestEnum>(i)));
  }
}

TEST(EnumSet, EqualityCompareEmpty) {
  EnumSet<TestEnum> set1;
  EnumSet<TestEnum> set2;

  EXPECT_TRUE(set1 == set2);
  EXPECT_FALSE(set1 != set2);
}

TEST(EnumSet, EqualityCompareSame) {
  EnumSet<TestEnum> set1;
  EnumSet<TestEnum> set2;

  set1.insert(TestEnum::ONE);
  set1.insert(TestEnum::TWENTY);
  set2.insert(TestEnum::TWENTY);
  set2.insert(TestEnum::ONE);

  EXPECT_TRUE(set1 == set2);
  EXPECT_FALSE(set1 != set2);
}

TEST(EnumSet, EqualityCompareDifferent) {
  EnumSet<TestEnum> set1;
  EnumSet<TestEnum> set2;

  set1.insert(TestEnum::ONE);
  set1.insert(TestEnum::TWENTY);
  set2.insert(TestEnum::FIVE);
  set2.insert(TestEnum::ONE);

  EXPECT_FALSE(set1 == set2);
  EXPECT_TRUE(set1 != set2);
}

TEST(EnumSet, ConstructFromIterators) {
  auto orderedValues = enumerateValuesFromToWithStep(0, 2, /* step= */ 1);
  EnumSet<TestEnum> set1 = createSetUnorderedInsertion(orderedValues);

  EnumSet<TestEnum> set2(orderedValues.cbegin(), orderedValues.cend());

  EXPECT_EQ(set1, set2);
}

TEST(EnumSet, InsertUsingIteratorRange) {
  auto orderedValues = enumerateValuesFromToWithStep(0, 2, /* step= */ 1);
  EnumSet<TestEnum> set1 = createSetUnorderedInsertion(orderedValues);

  EnumSet<TestEnum> set2;
  set2.insert(orderedValues.cbegin(), orderedValues.cend());

  EXPECT_EQ(set1, set2);
}

TEST(CapabilitySet, RangeBasedLoopOrderIsEnumOrder) {
  auto orderedValues = enumerateValuesFromToWithStep(0, 2, /* step= */ 1);
  auto set = createSetUnorderedInsertion(orderedValues);

  size_t index = 0;
  for (auto value : set) {
    ASSERT_THAT(value, Eq(orderedValues[index]));
    index++;
  }
}

TEST(CapabilitySet, ConstructSingleMemberMatrix) {
  CapabilitySet s(spv::Capability::Matrix);
  EXPECT_TRUE(s.contains(spv::Capability::Matrix));
  EXPECT_FALSE(s.contains(spv::Capability::Shader));
  EXPECT_FALSE(s.contains(static_cast<spv::Capability>(1000)));
}

TEST(CapabilitySet, ConstructSingleMemberMaxInMask) {
  CapabilitySet s(static_cast<spv::Capability>(63));
  EXPECT_FALSE(s.contains(spv::Capability::Matrix));
  EXPECT_FALSE(s.contains(spv::Capability::Shader));
  EXPECT_TRUE(s.contains(static_cast<spv::Capability>(63)));
  EXPECT_FALSE(s.contains(static_cast<spv::Capability>(64)));
  EXPECT_FALSE(s.contains(static_cast<spv::Capability>(1000)));
}

TEST(CapabilitySet, ConstructSingleMemberMinOverflow) {
  // Check the first one that forces overflow beyond the mask.
  CapabilitySet s(static_cast<spv::Capability>(64));
  EXPECT_FALSE(s.contains(spv::Capability::Matrix));
  EXPECT_FALSE(s.contains(spv::Capability::Shader));
  EXPECT_FALSE(s.contains(static_cast<spv::Capability>(63)));
  EXPECT_TRUE(s.contains(static_cast<spv::Capability>(64)));
  EXPECT_FALSE(s.contains(static_cast<spv::Capability>(1000)));
}

TEST(CapabilitySet, ConstructSingleMemberMaxOverflow) {
  // Check the max 32-bit signed int.
  CapabilitySet s(static_cast<spv::Capability>(0x7fffffffu));
  EXPECT_FALSE(s.contains(spv::Capability::Matrix));
  EXPECT_FALSE(s.contains(spv::Capability::Shader));
  EXPECT_FALSE(s.contains(static_cast<spv::Capability>(1000)));
  EXPECT_TRUE(s.contains(static_cast<spv::Capability>(0x7fffffffu)));
}

TEST(CapabilitySet, AddEnum) {
  CapabilitySet s(spv::Capability::Shader);
  s.insert(spv::Capability::Kernel);
  s.insert(static_cast<spv::Capability>(42));
  EXPECT_FALSE(s.contains(spv::Capability::Matrix));
  EXPECT_TRUE(s.contains(spv::Capability::Shader));
  EXPECT_TRUE(s.contains(spv::Capability::Kernel));
  EXPECT_TRUE(s.contains(static_cast<spv::Capability>(42)));
}

TEST(CapabilitySet, InsertReturnsIteratorToInserted) {
  CapabilitySet set;

  auto[it, inserted] = set.insert(spv::Capability::Kernel);

  EXPECT_TRUE(inserted);
  EXPECT_EQ(*it, spv::Capability::Kernel);
}

TEST(CapabilitySet, InsertReturnsIteratorToElementOnDoubleInsertion) {
  CapabilitySet set;
  EXPECT_FALSE(set.contains(spv::Capability::Shader));
  {
    auto[it, inserted] = set.insert(spv::Capability::Shader);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(*it, spv::Capability::Shader);
  }
  EXPECT_TRUE(set.contains(spv::Capability::Shader));

  auto[it, inserted] = set.insert(spv::Capability::Shader);

  EXPECT_FALSE(inserted);
  EXPECT_EQ(*it, spv::Capability::Shader);
  EXPECT_TRUE(set.contains(spv::Capability::Shader));
}

TEST(CapabilitySet, InsertWithHintWorks) {
  CapabilitySet set;
  EXPECT_FALSE(set.contains(spv::Capability::Shader));

  auto it = set.insert(set.begin(), spv::Capability::Shader);

  EXPECT_EQ(*it, spv::Capability::Shader);
  EXPECT_TRUE(set.contains(spv::Capability::Shader));
}

TEST(CapabilitySet, InsertWithEndHintWorks) {
  CapabilitySet set;
  EXPECT_FALSE(set.contains(spv::Capability::Shader));

  auto it = set.insert(set.end(), spv::Capability::Shader);

  EXPECT_EQ(*it, spv::Capability::Shader);
  EXPECT_TRUE(set.contains(spv::Capability::Shader));
}

TEST(CapabilitySet, IteratorCanBeCopied) {
  CapabilitySet set;
  set.insert(spv::Capability::Matrix);
  set.insert(spv::Capability::Shader);
  set.insert(spv::Capability::Geometry);
  set.insert(spv::Capability::Float64);
  set.insert(spv::Capability::Float16);

  auto a = set.begin();
  ++a;
  auto b = a;

  EXPECT_EQ(*b, *a);
  ++b;
  EXPECT_NE(*b, *a);

  ++a;
  EXPECT_EQ(*b, *a);

  ++a;
  EXPECT_NE(*b, *a);
}

TEST(CapabilitySet, IteratorBeginToEndPostfix) {
  auto orderedValues = enumerateValuesFromToWithStep(0, 100, /* step= */ 1);
  auto set = createSetUnorderedInsertion(orderedValues);

  size_t index = 0;
  for (auto it = set.cbegin(); it != set.cend(); it++, index++) {
    EXPECT_EQ(*it, orderedValues[index]);
  }
}

TEST(CapabilitySet, IteratorBeginToEndPrefix) {
  auto orderedValues = enumerateValuesFromToWithStep(0, 100, /* step= */ 1);
  auto set = createSetUnorderedInsertion(orderedValues);

  size_t index = 0;
  for (auto it = set.cbegin(); it != set.cend(); ++it, index++) {
    EXPECT_EQ(*it, orderedValues[index]);
  }
}

TEST(CapabilitySet, IteratorBeginToEndPrefixStep) {
  auto orderedValues = enumerateValuesFromToWithStep(0, 100, /* step= */ 8);
  auto set = createSetUnorderedInsertion(orderedValues);

  size_t index = 0;
  for (auto it = set.cbegin(); it != set.cend(); ++it, index++) {
    ASSERT_EQ(*it, orderedValues[index]);
  }
}

TEST(CapabilitySet, IteratorBeginOnEmpty) {
  CapabilitySet set;

  auto begin = set.begin();
  auto end = set.end();
  ASSERT_EQ(begin, end);
}

TEST(CapabilitySet, IteratorBeginOnSingleNonZeroValue) {
  CapabilitySet set;
  set.insert(spv::Capability::Shader);

  auto begin = set.begin();
  auto end = set.end();

  ASSERT_NE(begin, end);
  ASSERT_EQ(*begin, spv::Capability::Shader);
}

TEST(CapabilitySet, IteratorForLoopNonZeroValue) {
  CapabilitySet set;
  set.insert(spv::Capability::Shader);
  set.insert(spv::Capability::Tessellation);

  auto begin = set.begin();
  auto end = set.end();

  ASSERT_NE(begin, end);
  ASSERT_EQ(*begin, spv::Capability::Shader);

  begin++;
  ASSERT_NE(begin, end);
  ASSERT_EQ(*begin, spv::Capability::Tessellation);

  begin++;
  ASSERT_EQ(begin, end);
}

TEST(CapabilitySet, IteratorPastEnd) {
  CapabilitySet set;
  set.insert(spv::Capability::Shader);

  auto begin = set.begin();
  auto end = set.end();

  ASSERT_NE(begin, end);
  ASSERT_EQ(*begin, spv::Capability::Shader);

  begin++;
  ASSERT_EQ(begin, end);

  begin++;
  ASSERT_EQ(begin, end);
}

TEST(CapabilitySet, CompatibleWithSTLFind) {
  CapabilitySet set;
  set.insert(spv::Capability::Matrix);
  set.insert(spv::Capability::Shader);
  set.insert(spv::Capability::Geometry);
  set.insert(spv::Capability::Tessellation);
  set.insert(spv::Capability::Addresses);
  set.insert(spv::Capability::Linkage);
  set.insert(spv::Capability::Kernel);
  set.insert(spv::Capability::Vector16);
  set.insert(spv::Capability::Float16Buffer);
  set.insert(spv::Capability::Float64);

  {
    auto it = std::find(set.cbegin(), set.cend(), spv::Capability::Vector16);
    ASSERT_NE(it, set.end());
    ASSERT_EQ(*it, spv::Capability::Vector16);
  }

  {
    auto it = std::find(set.cbegin(), set.cend(), spv::Capability::Float16);
    ASSERT_EQ(it, set.end());
  }
}

TEST(CapabilitySet, CompatibleWithSTLForEach) {
  auto orderedValues = enumerateValuesFromToWithStep(0, 100, /* step= */ 15);
  auto set = createSetUnorderedInsertion(orderedValues);

  size_t index = 0;
  std::for_each(set.cbegin(), set.cend(), [&](auto item) {
    ASSERT_EQ(item, orderedValues[index]);
    index++;
  });
}

TEST(CapabilitySet, InitializerListEmpty) {
  CapabilitySet s{};
  for (uint32_t i = 0; i < 1000; i++) {
    EXPECT_FALSE(s.contains(static_cast<spv::Capability>(i)));
  }
}

TEST(CapabilitySet, LargeSetHasInsertedElements) {
  CapabilitySet set;
  for (auto c : kCapabilities) {
    EXPECT_FALSE(set.contains(c));
  }

  for (auto c : kCapabilities) {
    set.insert(c);
    EXPECT_TRUE(set.contains(c));
  }

  for (auto c : kCapabilities) {
    EXPECT_TRUE(set.contains(c));
  }
}

TEST(CapabilitySet, LargeSetHasUnsortedInsertedElements) {
  std::vector shuffledCapabilities(kCapabilities.cbegin(),
                                   kCapabilities.cend());
  std::mt19937 rng(0);
  std::shuffle(shuffledCapabilities.begin(), shuffledCapabilities.end(), rng);
  CapabilitySet set;
  for (auto c : shuffledCapabilities) {
    EXPECT_FALSE(set.contains(c));
  }

  for (auto c : shuffledCapabilities) {
    set.insert(c);
    EXPECT_TRUE(set.contains(c));
  }

  for (auto c : shuffledCapabilities) {
    EXPECT_TRUE(set.contains(c));
  }
}

TEST(CapabilitySet, LargeSetHasUnsortedRemovedElement) {
  std::vector shuffledCapabilities(kCapabilities.cbegin(),
                                   kCapabilities.cend());
  std::mt19937 rng(0);
  std::shuffle(shuffledCapabilities.begin(), shuffledCapabilities.end(), rng);
  CapabilitySet set;
  for (auto c : shuffledCapabilities) {
    set.insert(c);
    EXPECT_TRUE(set.contains(c));
  }

  for (auto c : kCapabilities) {
    set.erase(c);
  }

  for (auto c : shuffledCapabilities) {
    EXPECT_FALSE(set.contains(c));
  }
}

struct ForEachCase {
  CapabilitySet capabilities;
  std::vector<spv::Capability> expected;
};

using CapabilitySetForEachTest = ::testing::TestWithParam<ForEachCase>;

TEST_P(CapabilitySetForEachTest, CallsAsExpected) {
  EXPECT_THAT(ElementsIn(GetParam().capabilities), Eq(GetParam().expected));
}

TEST_P(CapabilitySetForEachTest, CopyConstructor) {
  CapabilitySet copy(GetParam().capabilities);
  EXPECT_THAT(ElementsIn(copy), Eq(GetParam().expected));
}

TEST_P(CapabilitySetForEachTest, MoveConstructor) {
  // We need a writable copy to move from.
  CapabilitySet copy(GetParam().capabilities);
  CapabilitySet moved(std::move(copy));
  EXPECT_THAT(ElementsIn(moved), Eq(GetParam().expected));
}

TEST_P(CapabilitySetForEachTest, OperatorEquals) {
  CapabilitySet assigned = GetParam().capabilities;
  EXPECT_THAT(ElementsIn(assigned), Eq(GetParam().expected));
}

TEST_P(CapabilitySetForEachTest, OperatorEqualsSelfAssign) {
  CapabilitySet assigned{GetParam().capabilities};
  assigned = assigned;  // NOLINT
  EXPECT_THAT(ElementsIn(assigned), Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    Samples, CapabilitySetForEachTest,
    ValuesIn(std::vector<ForEachCase>{
        {{}, {}},
        {{spv::Capability::Matrix}, {spv::Capability::Matrix}},
        {{spv::Capability::Kernel, spv::Capability::Shader},
         {spv::Capability::Shader, spv::Capability::Kernel}},
        {{static_cast<spv::Capability>(999)},
         {static_cast<spv::Capability>(999)}},
        {{static_cast<spv::Capability>(0x7fffffff)},
         {static_cast<spv::Capability>(0x7fffffff)}},
        // Mixture and out of order
        {{static_cast<spv::Capability>(0x7fffffff),
          static_cast<spv::Capability>(100), spv::Capability::Shader,
          spv::Capability::Matrix},
         {spv::Capability::Matrix, spv::Capability::Shader,
          static_cast<spv::Capability>(100),
          static_cast<spv::Capability>(0x7fffffff)}},
    }));

using BoundaryTestWithParam = ::testing::TestWithParam<spv::Capability>;

TEST_P(BoundaryTestWithParam, InsertedContains) {
  CapabilitySet set;
  set.insert(GetParam());
  EXPECT_TRUE(set.contains(GetParam()));
}

INSTANTIATE_TEST_SUITE_P(
    Samples, BoundaryTestWithParam,
    Values(static_cast<spv::Capability>(0), static_cast<spv::Capability>(63),
           static_cast<spv::Capability>(64), static_cast<spv::Capability>(65),
           static_cast<spv::Capability>(127), static_cast<spv::Capability>(128),
           static_cast<spv::Capability>(129)));

}  // namespace
}  // namespace spvtools
