// Copyright (c) 2025 The Khronos Group Inc.
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

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;
using ::testing::internal::ParamGenerator;

// clang-format off
#define MEMORY_SEMANTICS_OPERANDS   \
    OpMemoryBarrier,                \
    OpControlBarrier,               \
    OpAtomicLoad,                   \
    OpAtomicStore,                  \
    OpAtomicExchange,               \
    OpAtomicIIncrement,             \
    OpAtomicIDecrement,             \
    OpAtomicIAdd,                   \
    OpAtomicISub,                   \
    OpAtomicSMin,                   \
    OpAtomicSMax,                   \
    OpAtomicUMin,                   \
    OpAtomicUMax,                   \
    OpAtomicAnd,                    \
    OpAtomicOr,                     \
    OpAtomicXor,                    \
    OpAtomicCompareExchangeEqual,   \
    OpAtomicCompareExchangeUnequal

enum Operand { MEMORY_SEMANTICS_OPERANDS };
const Operand Operands[] = { MEMORY_SEMANTICS_OPERANDS };
const size_t OperandsCount = sizeof(Operands) / sizeof(Operand);
#undef MEMORY_SEMANTICS_OPERANDS
// clang-format on

enum Value {
  None = uint32_t(spv::MemorySemanticsMask::MaskNone),
  Acquire = uint32_t(spv::MemorySemanticsMask::Acquire),
  Release = uint32_t(spv::MemorySemanticsMask::Release),
  AcqRel = uint32_t(spv::MemorySemanticsMask::AcquireRelease),
  SeqCst = uint32_t(spv::MemorySemanticsMask::SequentiallyConsistent),
  Uniform = uint32_t(spv::MemorySemanticsMask::UniformMemory),
  Subgroup = uint32_t(spv::MemorySemanticsMask::SubgroupMemory),
  Workgroup = uint32_t(spv::MemorySemanticsMask::WorkgroupMemory),
  CrossWorkgroup = uint32_t(spv::MemorySemanticsMask::CrossWorkgroupMemory),
  AtomicCounter = uint32_t(spv::MemorySemanticsMask::AtomicCounterMemory),
  Image = uint32_t(spv::MemorySemanticsMask::ImageMemory),
  Output = uint32_t(spv::MemorySemanticsMask::OutputMemory),
  Available = uint32_t(spv::MemorySemanticsMask::MakeAvailable),
  Visible = uint32_t(spv::MemorySemanticsMask::MakeVisible),
  Volatile = uint32_t(spv::MemorySemanticsMask::Volatile)
};

struct TestResult {
  explicit TestResult(spv_result_t in_result = SPV_SUCCESS,
                      const char* in_vuid = nullptr,
                      const char* in_error = nullptr)
      : result(in_result), vuid(in_vuid), error(in_error) {}
  spv_result_t result;
  const char* vuid;
  const char* error;
};

template <typename T, typename... Ts>
ParamGenerator<T> ValuesInExcept(const T* items, const size_t count,
                                 const Ts... skip) {
  std::vector<T> filtered;
  std::initializer_list<T> excluded = {skip...};
  std::copy_if(items, items + count, std::back_inserter(filtered),
               [&excluded](const T& value) {
                 return std::all_of(
                     excluded.begin(), excluded.end(),
                     [&value](const T& other) { return value != other; });
               });
  return ValuesIn(filtered);
}

std::string GenerateInstruction(const Operand operand) {
  switch (operand) {
    case OpMemoryBarrier:
      return "OpMemoryBarrier %scope %semantics";
    case OpControlBarrier:
      return "OpControlBarrier %uint_2 %scope %semantics";
    case OpAtomicLoad:
      return "%result = OpAtomicLoad %uint %var %scope %semantics";
    case OpAtomicStore:
      return "OpAtomicStore %var %scope %semantics %uint_1";
    case OpAtomicExchange:
      return "%result = OpAtomicExchange %uint %var %scope %semantics %uint_1";
    case OpAtomicCompareExchangeEqual:
      return "%result = OpAtomicCompareExchange %uint %var %scope %semantics "
             "%semantics_min %uint_1 %uint_0";
    case OpAtomicCompareExchangeUnequal:
      return "%result = OpAtomicCompareExchange %uint %var %scope "
             "%semantics_max %semantics %uint_1 %uint_0";
    case OpAtomicIIncrement:
      return "%result = OpAtomicIIncrement %uint %var %scope %semantics";
    case OpAtomicIDecrement:
      return "%result = OpAtomicIDecrement %uint %var %scope %semantics";
    case OpAtomicIAdd:
      return "%result = OpAtomicIAdd %uint %var %scope %semantics %uint_1";
    case OpAtomicISub:
      return "%result = OpAtomicISub %uint %var %scope %semantics %uint_1";
    case OpAtomicSMin:
      return "%result = OpAtomicSMin %uint %var %scope %semantics %uint_1";
    case OpAtomicUMin:
      return "%result = OpAtomicUMin %uint %var %scope %semantics %uint_1";
    case OpAtomicSMax:
      return "%result = OpAtomicSMax %uint %var %scope %semantics %uint_1";
    case OpAtomicUMax:
      return "%result = OpAtomicUMax %uint %var %scope %semantics %uint_1";
    case OpAtomicAnd:
      return "%result = OpAtomicAnd %uint %var %scope %semantics %uint_1";
    case OpAtomicOr:
      return "%result = OpAtomicOr %uint %var %scope %semantics %uint_1";
    case OpAtomicXor:
      return "%result = OpAtomicXor %uint %var %scope %semantics %uint_1";
    default:
      return "";
  }
}

std::string GenerateVulkanCode(const std::string instruction,
                               const uint32_t semantics,
                               const uint32_t semantics2 = 0) {
  std::ostringstream ss;
  ss << R"(
OpCapability Shader
OpCapability VulkanMemoryModel
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical Vulkan
OpEntryPoint GLCompute %main "main" %var
OpExecutionMode %main LocalSize 32 1 1
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%func = OpTypeFunction %void
%uint_ptr = OpTypePointer Workgroup %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_4 = OpConstant %uint 4
%scope = OpConstant %uint 5
%semantics = OpConstant %uint )"
     << semantics << R"(
%semantics2 = OpConstant %uint )"
     << semantics2 << R"(
%semantics_min = OpConstant %uint )"
     << (semantics & Volatile) << R"(
%semantics_max = OpConstant %uint )"
     << (32712 | (semantics & Volatile)) << R"(
%var = OpVariable %uint_ptr Workgroup
%main = OpFunction %void None %func
%label = OpLabel
)" << instruction
     << R"(
OpReturn
OpFunctionEnd)";

  return ss.str();
}

using VulkanMemorySemantics =
    spvtest::ValidateBase<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t,
                                     uint32_t, Operand, TestResult>>;
using VulkanUnequalMemorySemantics = spvtest::ValidateBase<
    std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
               uint32_t, uint32_t, bool, uint32_t, TestResult>>;

INSTANTIATE_TEST_SUITE_P(
    ErrorMultipleMemoryOrderBits, VulkanMemorySemantics,
    Combine(
        Values(Acquire | Release, Acquire | AcqRel, Release | AcqRel,
               Acquire | SeqCst, Release | SeqCst, AcqRel | SeqCst),
        Values(None, Uniform | Workgroup | Image | Output),
        Values(None, Available, Visible, Available | Visible),
        Values(None, Volatile),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        ValuesIn(Operands),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-MemorySemantics-10865",
            "Memory Semantics must have at most one non-relaxed memory order "
            "bit set"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorSequentiallyConsistentMemoryOrder, VulkanMemorySemantics,
    Combine(
        Values(SeqCst), Values(None, Uniform | Workgroup | Image | Output),
        Values(None, Available, Visible, Available | Visible),
        Values(None, Volatile),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        ValuesIn(Operands),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-MemorySemantics-10866",
            "Memory Semantics with SequentiallyConsistent memory order must "
            "not be used in the Vulkan API"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorAtomicStoreWithAcquireMemoryOrder, VulkanMemorySemantics,
    Combine(Values(Acquire, AcqRel),
            Values(None, Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Visible, Available, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpAtomicStore),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10867",
                "MemorySemantics must not use Acquire or AcquireRelease "
                "memory order with AtomicStore"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorAtomicLoadWithReleaseMemoryOrder, VulkanMemorySemantics,
    Combine(Values(Release, AcqRel),
            Values(None, Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Available, Visible, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpAtomicLoad),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10868",
                "MemorySemantics must not use Release or AcquireRelease "
                "memory order with AtomicLoad"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMemoryBarrierWithRelaxedMemoryOrder, VulkanMemorySemantics,
    Combine(Values(None),
            Values(None, Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Available, Visible, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpMemoryBarrier),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10869",
                "MemorySemantics must not use Relaxed memory order with "
                "MemoryBarrier"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorNonRelaxedSemanticsWithoutStorageClass, VulkanMemorySemantics,
    Combine(Values(Acquire, Release, AcqRel), Values(None),
            Values(None, Available, Visible, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpAtomicLoad,
                           OpAtomicStore),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10870",
                "Memory Semantics with a non-relaxed memory order (Acquire, "
                "Release, or AcquireRelease) must have at least one "
                "Vulkan-supported storage class semantics bit set "
                "(UniformMemory, WorkgroupMemory, ImageMemory, "
                "or OutputMemory)"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorNonRelaxedSemanticsWithoutStorageClassLoad, VulkanMemorySemantics,
    Combine(Values(Acquire), Values(None),
            Values(None, Available, Visible, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpAtomicLoad),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10870",
                "Memory Semantics with a non-relaxed memory order (Acquire, "
                "Release, or AcquireRelease) must have at least one "
                "Vulkan-supported storage class semantics bit set "
                "(UniformMemory, WorkgroupMemory, ImageMemory, or "
                "OutputMemory)"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorNonRelaxedSemanticsWithoutStorageClassStore, VulkanMemorySemantics,
    Combine(Values(Release), Values(None),
            Values(None, Available, Visible, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpAtomicStore),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10870",
                "Memory Semantics with a non-relaxed memory order (Acquire, "
                "Release, or AcquireRelease) must have at least one "
                "Vulkan-supported storage class semantics bit set "
                "(UniformMemory, WorkgroupMemory, ImageMemory, or "
                "OutputMemory)"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorRelaxedSemanticsWithStorageClass, VulkanMemorySemantics,
    Combine(
        Values(None),
        Values(Uniform, Workgroup, Image, Output,
               Uniform | Workgroup | Image | Output),
        Values(None, Available, Visible, Available | Visible),
        Values(None, Volatile),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        ValuesInExcept(Operands, OperandsCount, OpMemoryBarrier),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-MemorySemantics-10871",
            "Memory Semantics with at least one Vulkan-supported storage class "
            "semantics bit set (UniformMemory, WorkgroupMemory, ImageMemory, "
            "or OutputMemory) must use a non-relaxed memory order (Acquire, "
            "Release, or AcquireRelease)"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMakeAvailableWithRelaxedMemoryOrder, VulkanMemorySemantics,
    Combine(Values(None), Values(None), Values(Available, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpMemoryBarrier),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10872",
                "Memory Semantics with MakeAvailable bit set must use Release "
                "or AcquireRelease memory order"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMakeAvailableWithAcquireMemoryOrder, VulkanMemorySemantics,
    Combine(Values(Acquire),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(Available, Available | Visible), Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpAtomicStore),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10872",
                "Memory Semantics with MakeAvailable bit set must use Release "
                "or AcquireRelease memory order"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMakeVisibleWithRelaxedMemoryOrder, VulkanMemorySemantics,
    Combine(Values(None), Values(None), Values(Visible), Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpMemoryBarrier),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10873",
                "Memory Semantics with MakeVisible bit set must use Acquire "
                "or AcquireRelease memory order"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMakeVisibleWithReleaseMemoryOrder, VulkanMemorySemantics,
    Combine(Values(Release),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(Visible, Available | Visible), Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpAtomicLoad),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10873",
                "Memory Semantics with MakeVisible bit set must use Acquire "
                "or AcquireRelease memory order"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorVolatileBarrierWithRelaxedSemantics, VulkanMemorySemantics,
    Combine(Values(None), Values(None), Values(None), Values(Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpControlBarrier),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10874",
                "Memory Semantics with Volatile bit set must not be used with "
                "barrier instructions"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorVolatileBarrierWithNonRelaxedSemantics, VulkanMemorySemantics,
    Combine(Values(Acquire, Acquire | Visible, Release, Release | Available,
                   AcqRel, AcqRel | Visible, AcqRel | Available,
                   AcqRel | Available | Visible),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None), Values(Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpControlBarrier, OpMemoryBarrier),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-MemorySemantics-10874",
                "Memory Semantics with Volatile bit set must not be used with "
                "barrier instructions"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorCompareExchangeUnequalSemanticsWithRelease, VulkanMemorySemantics,
    Combine(Values(Release, AcqRel, AcqRel | Visible),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Available), Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpAtomicCompareExchangeUnequal),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-UnequalMemorySemantics-10875",
                "AtomicCompareExchange Unequal Memory Semantics must not use "
                "Release or AcquireRelease memory order"))));

INSTANTIATE_TEST_SUITE_P(
    SuccessAtomicsRelaxed, VulkanMemorySemantics,
    Combine(Values(None), Values(None), Values(None), Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpMemoryBarrier,
                           OpControlBarrier),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SuccessAtomicsAcquire, VulkanMemorySemantics,
    Combine(Values(Acquire),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Visible), Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpMemoryBarrier,
                           OpControlBarrier, OpAtomicStore),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SuccessAtomicsRelease, VulkanMemorySemantics,
    Combine(Values(Release),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Available), Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpMemoryBarrier,
                           OpControlBarrier, OpAtomicLoad,
                           OpAtomicCompareExchangeUnequal),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SuccessAtomicsAcqRel, VulkanMemorySemantics,
    Combine(Values(AcqRel),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Available, Visible, Available | Visible),
            Values(None, Volatile),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            ValuesInExcept(Operands, OperandsCount, OpMemoryBarrier,
                           OpControlBarrier, OpAtomicLoad, OpAtomicStore,
                           OpAtomicCompareExchangeUnequal),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SuccessBarriersRelaxed, VulkanMemorySemantics,
    Combine(Values(None), Values(None), Values(None), Values(None),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpControlBarrier), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SuccessBarriersNonRelaxed, VulkanMemorySemantics,
    Combine(Values(Acquire, Acquire | Visible, Release, Release | Available,
                   AcqRel, AcqRel | Available, AcqRel | Visible,
                   AcqRel | Available | Visible),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None), Values(None),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(OpControlBarrier, OpMemoryBarrier), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ErrorMemoryOrderTooWeak, VulkanUnequalMemorySemantics,
    Combine(Values(None), Values(None), Values(None),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(Acquire),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None, Visible),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(true, false), Values(None, Volatile),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-UnequalMemorySemantics-10876",
                "AtomicCompareExchange Unequal Memory Semantics must not use a "
                "stronger memory order than the corresponding Equal Memory "
                "Semantics"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMissingStorageClassSemanticsFlags, VulkanUnequalMemorySemantics,
    Combine(
        Values(Acquire, Acquire | Visible, Release, Release | Available, AcqRel,
               AcqRel | Visible, AcqRel | Available,
               AcqRel | Available | Visible),
        Values(Uniform | Workgroup), Values(None),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        Values(Acquire), Values(Uniform | Image, Output), Values(None, Visible),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        Values(true, false), Values(None, Volatile),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-UnequalMemorySemantics-10877",
            "AtomicCompareExchange Unequal Memory Semantics must not have any "
            "Vulkan-supported storage class semantics bit set (UniformMemory, "
            "WorkgroupMemory, ImageMemory, or OutputMemory) unless this bit is "
            "also set in the corresponding Equal Memory Semantics"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMissingMakeVisibleFlag, VulkanUnequalMemorySemantics,
    Combine(Values(Acquire, Release, Release | Available, AcqRel,
                   AcqRel | Available),
            Values(Uniform | Workgroup | Image | Output), Values(None),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(Acquire),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(Visible),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(true, false), Values(None, Volatile),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "VUID-StandaloneSpirv-UnequalMemorySemantics-10878",
                "AtomicCompareExchange Unequal Memory Semantics must not have "
                "MakeVisible bit set unless this bit is also set in the "
                "corresponding Equal Memory Semantics"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMismatchingVolatileFlagsRelaxedAndRelaxed,
    VulkanUnequalMemorySemantics,
    Combine(
        Values(None), Values(None), Values(None),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter), Values(None),
        Values(None), Values(None),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter), Values(false),
        Values(None, Volatile),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-UnequalMemorySemantics-10879",
            "AtomicCompareExchange Unequal Memory Semantics must have Volatile "
            "bit set if and only if this bit is also set in the corresponding "
            "Equal Memory Semantics"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMismatchingVolatileFlagsNonRelaxedAndRelaxed,
    VulkanUnequalMemorySemantics,
    Combine(
        Values(Acquire, Acquire | Visible, Release, Release | Available, AcqRel,
               AcqRel | Visible, AcqRel | Available,
               AcqRel | Available | Visible),
        Values(Uniform, Workgroup, Image, Output,
               Uniform | Workgroup | Image | Output),
        Values(None), Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        Values(None), Values(None), Values(None),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter), Values(false),
        Values(None, Volatile),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-UnequalMemorySemantics-10879",
            "AtomicCompareExchange Unequal Memory Semantics must have Volatile "
            "bit set if and only if this bit is also set in the corresponding "
            "Equal Memory Semantics"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMismatchingVolatileFlagsNonRelaxedAndAcquire,
    VulkanUnequalMemorySemantics,
    Combine(
        Values(Acquire, Acquire | Visible, Release, Release | Available, AcqRel,
               AcqRel | Visible, AcqRel | Available,
               AcqRel | Available | Visible),
        Values(Uniform | Workgroup | Image | Output), Values(None),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        Values(Acquire),
        Values(Uniform, Workgroup, Image, Output,
               Uniform | Workgroup | Image | Output),
        Values(None), Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        Values(false), Values(None, Volatile),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-UnequalMemorySemantics-10879",
            "AtomicCompareExchange Unequal Memory Semantics must have Volatile "
            "bit set if and only if this bit is also set in the corresponding "
            "Equal Memory Semantics"))));

INSTANTIATE_TEST_SUITE_P(
    ErrorMismatchingVolatileFlagsNonRelaxedAndAcquireVisible,
    VulkanUnequalMemorySemantics,
    Combine(
        Values(Acquire, AcqRel, AcqRel | Available),
        Values(Uniform | Workgroup | Image | Output), Values(Visible),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
        Values(Acquire),
        Values(Uniform, Workgroup, Image, Output,
               Uniform | Workgroup | Image | Output),
        Values(Visible),
        Values(None, Subgroup | CrossWorkgroup | AtomicCounter), Values(false),
        Values(None, Volatile),
        Values(TestResult(
            SPV_ERROR_INVALID_DATA,
            "VUID-StandaloneSpirv-UnequalMemorySemantics-10879",
            "AtomicCompareExchange Unequal Memory Semantics must have Volatile "
            "bit set if and only if this bit is also set in the corresponding "
            "Equal Memory Semantics"))));

INSTANTIATE_TEST_SUITE_P(
    SuccessNonRelaxedAndAcquire, VulkanUnequalMemorySemantics,
    Combine(Values(Acquire, Acquire | Visible, Release, Release | Available,
                   AcqRel, AcqRel | Visible, AcqRel | Available,
                   AcqRel | Available | Visible),
            Values(Uniform | Workgroup | Image | Output), Values(None),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(Acquire),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(None),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(true), Values(None, Volatile), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SuccessNonRelaxedAndAcquireVisible, VulkanUnequalMemorySemantics,
    Combine(Values(Acquire, AcqRel, AcqRel | Available),
            Values(Uniform | Workgroup | Image | Output), Values(Visible),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(Acquire),
            Values(Uniform, Workgroup, Image, Output,
                   Uniform | Workgroup | Image | Output),
            Values(Visible),
            Values(None, Subgroup | CrossWorkgroup | AtomicCounter),
            Values(true), Values(None, Volatile), Values(TestResult())));

TEST_P(VulkanMemorySemantics, Case) {
  const uint32_t semantics = std::get<0>(GetParam()) | std::get<1>(GetParam()) |
                             std::get<2>(GetParam()) | std::get<3>(GetParam()) |
                             std::get<4>(GetParam());
  const Operand operand = std::get<5>(GetParam());
  const TestResult& result = std::get<6>(GetParam());
  const std::string instruction = GenerateInstruction(operand);

  CompileSuccessfully(GenerateVulkanCode(instruction, semantics),
                      SPV_ENV_VULKAN_1_4);
  ASSERT_EQ(result.result, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  if (result.vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(result.vuid));
  }
  if (result.error) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(result.error));
  }
}

TEST_P(VulkanUnequalMemorySemantics, Case) {
  const uint32_t equal_volatile = std::get<9>(GetParam());
  const uint32_t unequal_volatile =
      std::get<8>(GetParam()) ? equal_volatile : Volatile ^ equal_volatile;
  const uint32_t equal = std::get<0>(GetParam()) | std::get<1>(GetParam()) |
                         std::get<2>(GetParam()) | std::get<3>(GetParam()) |
                         equal_volatile;
  const uint32_t unequal = std::get<4>(GetParam()) | std::get<5>(GetParam()) |
                           std::get<6>(GetParam()) | std::get<7>(GetParam()) |
                           unequal_volatile;
  const TestResult& result = std::get<10>(GetParam());
  const std::string instruction =
      "%result = OpAtomicCompareExchange %uint %var %scope %semantics "
      "%semantics2 %uint_1 %uint_0";

  CompileSuccessfully(GenerateVulkanCode(instruction, equal, unequal),
                      SPV_ENV_VULKAN_1_4);
  ASSERT_EQ(result.result, ValidateInstructions(SPV_ENV_VULKAN_1_4));
  if (result.vuid) {
    EXPECT_THAT(getDiagnosticString(), AnyVUID(result.vuid));
  }
  if (result.error) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(result.error));
  }
}

}  // namespace
}  // namespace val
}  // namespace spvtools
