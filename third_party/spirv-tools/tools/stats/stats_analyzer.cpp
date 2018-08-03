// Copyright (c) 2017 Google Inc.
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

#include "stats_analyzer.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "latest_version_spirv_header.h"
#include "source/comp/markv_model.h"
#include "source/enum_string_mapping.h"
#include "source/opcode.h"
#include "source/operand.h"
#include "source/spirv_constant.h"
#include "source/util/huffman_codec.h"

namespace {

using spvtools::SpirvStats;
using spvtools::utils::HuffmanCodec;

// Signals that the value is not in the coding scheme and a fallback method
// needs to be used.
const uint64_t kMarkvNoneOfTheAbove =
    spvtools::comp::MarkvModel::GetMarkvNoneOfTheAbove();

inline uint32_t CombineOpcodeAndNumOperands(uint32_t opcode,
                                            uint32_t num_operands) {
  return opcode | (num_operands << 16);
}

// Returns all SPIR-V v1.2 opcodes.
std::vector<uint32_t> GetAllOpcodes() {
  return std::vector<uint32_t>({
      SpvOpNop,
      SpvOpUndef,
      SpvOpSourceContinued,
      SpvOpSource,
      SpvOpSourceExtension,
      SpvOpName,
      SpvOpMemberName,
      SpvOpString,
      SpvOpLine,
      SpvOpExtension,
      SpvOpExtInstImport,
      SpvOpExtInst,
      SpvOpMemoryModel,
      SpvOpEntryPoint,
      SpvOpExecutionMode,
      SpvOpCapability,
      SpvOpTypeVoid,
      SpvOpTypeBool,
      SpvOpTypeInt,
      SpvOpTypeFloat,
      SpvOpTypeVector,
      SpvOpTypeMatrix,
      SpvOpTypeImage,
      SpvOpTypeSampler,
      SpvOpTypeSampledImage,
      SpvOpTypeArray,
      SpvOpTypeRuntimeArray,
      SpvOpTypeStruct,
      SpvOpTypeOpaque,
      SpvOpTypePointer,
      SpvOpTypeFunction,
      SpvOpTypeEvent,
      SpvOpTypeDeviceEvent,
      SpvOpTypeReserveId,
      SpvOpTypeQueue,
      SpvOpTypePipe,
      SpvOpTypeForwardPointer,
      SpvOpConstantTrue,
      SpvOpConstantFalse,
      SpvOpConstant,
      SpvOpConstantComposite,
      SpvOpConstantSampler,
      SpvOpConstantNull,
      SpvOpSpecConstantTrue,
      SpvOpSpecConstantFalse,
      SpvOpSpecConstant,
      SpvOpSpecConstantComposite,
      SpvOpSpecConstantOp,
      SpvOpFunction,
      SpvOpFunctionParameter,
      SpvOpFunctionEnd,
      SpvOpFunctionCall,
      SpvOpVariable,
      SpvOpImageTexelPointer,
      SpvOpLoad,
      SpvOpStore,
      SpvOpCopyMemory,
      SpvOpCopyMemorySized,
      SpvOpAccessChain,
      SpvOpInBoundsAccessChain,
      SpvOpPtrAccessChain,
      SpvOpArrayLength,
      SpvOpGenericPtrMemSemantics,
      SpvOpInBoundsPtrAccessChain,
      SpvOpDecorate,
      SpvOpMemberDecorate,
      SpvOpDecorationGroup,
      SpvOpGroupDecorate,
      SpvOpGroupMemberDecorate,
      SpvOpVectorExtractDynamic,
      SpvOpVectorInsertDynamic,
      SpvOpVectorShuffle,
      SpvOpCompositeConstruct,
      SpvOpCompositeExtract,
      SpvOpCompositeInsert,
      SpvOpCopyObject,
      SpvOpTranspose,
      SpvOpSampledImage,
      SpvOpImageSampleImplicitLod,
      SpvOpImageSampleExplicitLod,
      SpvOpImageSampleDrefImplicitLod,
      SpvOpImageSampleDrefExplicitLod,
      SpvOpImageSampleProjImplicitLod,
      SpvOpImageSampleProjExplicitLod,
      SpvOpImageSampleProjDrefImplicitLod,
      SpvOpImageSampleProjDrefExplicitLod,
      SpvOpImageFetch,
      SpvOpImageGather,
      SpvOpImageDrefGather,
      SpvOpImageRead,
      SpvOpImageWrite,
      SpvOpImage,
      SpvOpImageQueryFormat,
      SpvOpImageQueryOrder,
      SpvOpImageQuerySizeLod,
      SpvOpImageQuerySize,
      SpvOpImageQueryLod,
      SpvOpImageQueryLevels,
      SpvOpImageQuerySamples,
      SpvOpConvertFToU,
      SpvOpConvertFToS,
      SpvOpConvertSToF,
      SpvOpConvertUToF,
      SpvOpUConvert,
      SpvOpSConvert,
      SpvOpFConvert,
      SpvOpQuantizeToF16,
      SpvOpConvertPtrToU,
      SpvOpSatConvertSToU,
      SpvOpSatConvertUToS,
      SpvOpConvertUToPtr,
      SpvOpPtrCastToGeneric,
      SpvOpGenericCastToPtr,
      SpvOpGenericCastToPtrExplicit,
      SpvOpBitcast,
      SpvOpSNegate,
      SpvOpFNegate,
      SpvOpIAdd,
      SpvOpFAdd,
      SpvOpISub,
      SpvOpFSub,
      SpvOpIMul,
      SpvOpFMul,
      SpvOpUDiv,
      SpvOpSDiv,
      SpvOpFDiv,
      SpvOpUMod,
      SpvOpSRem,
      SpvOpSMod,
      SpvOpFRem,
      SpvOpFMod,
      SpvOpVectorTimesScalar,
      SpvOpMatrixTimesScalar,
      SpvOpVectorTimesMatrix,
      SpvOpMatrixTimesVector,
      SpvOpMatrixTimesMatrix,
      SpvOpOuterProduct,
      SpvOpDot,
      SpvOpIAddCarry,
      SpvOpISubBorrow,
      SpvOpUMulExtended,
      SpvOpSMulExtended,
      SpvOpAny,
      SpvOpAll,
      SpvOpIsNan,
      SpvOpIsInf,
      SpvOpIsFinite,
      SpvOpIsNormal,
      SpvOpSignBitSet,
      SpvOpLessOrGreater,
      SpvOpOrdered,
      SpvOpUnordered,
      SpvOpLogicalEqual,
      SpvOpLogicalNotEqual,
      SpvOpLogicalOr,
      SpvOpLogicalAnd,
      SpvOpLogicalNot,
      SpvOpSelect,
      SpvOpIEqual,
      SpvOpINotEqual,
      SpvOpUGreaterThan,
      SpvOpSGreaterThan,
      SpvOpUGreaterThanEqual,
      SpvOpSGreaterThanEqual,
      SpvOpULessThan,
      SpvOpSLessThan,
      SpvOpULessThanEqual,
      SpvOpSLessThanEqual,
      SpvOpFOrdEqual,
      SpvOpFUnordEqual,
      SpvOpFOrdNotEqual,
      SpvOpFUnordNotEqual,
      SpvOpFOrdLessThan,
      SpvOpFUnordLessThan,
      SpvOpFOrdGreaterThan,
      SpvOpFUnordGreaterThan,
      SpvOpFOrdLessThanEqual,
      SpvOpFUnordLessThanEqual,
      SpvOpFOrdGreaterThanEqual,
      SpvOpFUnordGreaterThanEqual,
      SpvOpShiftRightLogical,
      SpvOpShiftRightArithmetic,
      SpvOpShiftLeftLogical,
      SpvOpBitwiseOr,
      SpvOpBitwiseXor,
      SpvOpBitwiseAnd,
      SpvOpNot,
      SpvOpBitFieldInsert,
      SpvOpBitFieldSExtract,
      SpvOpBitFieldUExtract,
      SpvOpBitReverse,
      SpvOpBitCount,
      SpvOpDPdx,
      SpvOpDPdy,
      SpvOpFwidth,
      SpvOpDPdxFine,
      SpvOpDPdyFine,
      SpvOpFwidthFine,
      SpvOpDPdxCoarse,
      SpvOpDPdyCoarse,
      SpvOpFwidthCoarse,
      SpvOpEmitVertex,
      SpvOpEndPrimitive,
      SpvOpEmitStreamVertex,
      SpvOpEndStreamPrimitive,
      SpvOpControlBarrier,
      SpvOpMemoryBarrier,
      SpvOpAtomicLoad,
      SpvOpAtomicStore,
      SpvOpAtomicExchange,
      SpvOpAtomicCompareExchange,
      SpvOpAtomicCompareExchangeWeak,
      SpvOpAtomicIIncrement,
      SpvOpAtomicIDecrement,
      SpvOpAtomicIAdd,
      SpvOpAtomicISub,
      SpvOpAtomicSMin,
      SpvOpAtomicUMin,
      SpvOpAtomicSMax,
      SpvOpAtomicUMax,
      SpvOpAtomicAnd,
      SpvOpAtomicOr,
      SpvOpAtomicXor,
      SpvOpPhi,
      SpvOpLoopMerge,
      SpvOpSelectionMerge,
      SpvOpLabel,
      SpvOpBranch,
      SpvOpBranchConditional,
      SpvOpSwitch,
      SpvOpKill,
      SpvOpReturn,
      SpvOpReturnValue,
      SpvOpUnreachable,
      SpvOpLifetimeStart,
      SpvOpLifetimeStop,
      SpvOpGroupAsyncCopy,
      SpvOpGroupWaitEvents,
      SpvOpGroupAll,
      SpvOpGroupAny,
      SpvOpGroupBroadcast,
      SpvOpGroupIAdd,
      SpvOpGroupFAdd,
      SpvOpGroupFMin,
      SpvOpGroupUMin,
      SpvOpGroupSMin,
      SpvOpGroupFMax,
      SpvOpGroupUMax,
      SpvOpGroupSMax,
      SpvOpReadPipe,
      SpvOpWritePipe,
      SpvOpReservedReadPipe,
      SpvOpReservedWritePipe,
      SpvOpReserveReadPipePackets,
      SpvOpReserveWritePipePackets,
      SpvOpCommitReadPipe,
      SpvOpCommitWritePipe,
      SpvOpIsValidReserveId,
      SpvOpGetNumPipePackets,
      SpvOpGetMaxPipePackets,
      SpvOpGroupReserveReadPipePackets,
      SpvOpGroupReserveWritePipePackets,
      SpvOpGroupCommitReadPipe,
      SpvOpGroupCommitWritePipe,
      SpvOpEnqueueMarker,
      SpvOpEnqueueKernel,
      SpvOpGetKernelNDrangeSubGroupCount,
      SpvOpGetKernelNDrangeMaxSubGroupSize,
      SpvOpGetKernelWorkGroupSize,
      SpvOpGetKernelPreferredWorkGroupSizeMultiple,
      SpvOpRetainEvent,
      SpvOpReleaseEvent,
      SpvOpCreateUserEvent,
      SpvOpIsValidEvent,
      SpvOpSetUserEventStatus,
      SpvOpCaptureEventProfilingInfo,
      SpvOpGetDefaultQueue,
      SpvOpBuildNDRange,
      SpvOpImageSparseSampleImplicitLod,
      SpvOpImageSparseSampleExplicitLod,
      SpvOpImageSparseSampleDrefImplicitLod,
      SpvOpImageSparseSampleDrefExplicitLod,
      SpvOpImageSparseSampleProjImplicitLod,
      SpvOpImageSparseSampleProjExplicitLod,
      SpvOpImageSparseSampleProjDrefImplicitLod,
      SpvOpImageSparseSampleProjDrefExplicitLod,
      SpvOpImageSparseFetch,
      SpvOpImageSparseGather,
      SpvOpImageSparseDrefGather,
      SpvOpImageSparseTexelsResident,
      SpvOpNoLine,
      SpvOpAtomicFlagTestAndSet,
      SpvOpAtomicFlagClear,
      SpvOpImageSparseRead,
      SpvOpSizeOf,
      SpvOpTypePipeStorage,
      SpvOpConstantPipeStorage,
      SpvOpCreatePipeFromPipeStorage,
      SpvOpGetKernelLocalSizeForSubgroupCount,
      SpvOpGetKernelMaxNumSubgroups,
      SpvOpTypeNamedBarrier,
      SpvOpNamedBarrierInitialize,
      SpvOpMemoryNamedBarrier,
      SpvOpModuleProcessed,
      SpvOpExecutionModeId,
      SpvOpDecorateId,
      SpvOpSubgroupBallotKHR,
      SpvOpSubgroupFirstInvocationKHR,
      SpvOpSubgroupAllKHR,
      SpvOpSubgroupAnyKHR,
      SpvOpSubgroupAllEqualKHR,
      SpvOpSubgroupReadInvocationKHR,
  });
}

std::string GetVersionString(uint32_t word) {
  std::stringstream ss;
  ss << "Version " << SPV_SPIRV_VERSION_MAJOR_PART(word) << "."
     << SPV_SPIRV_VERSION_MINOR_PART(word);
  return ss.str();
}

std::string GetGeneratorString(uint32_t word) {
  return spvGeneratorStr(SPV_GENERATOR_TOOL_PART(word));
}

std::string GetOpcodeString(uint32_t word) {
  return spvOpcodeString(static_cast<SpvOp>(word));
}

std::string GetCapabilityString(uint32_t word) {
  return spvtools::CapabilityToString(static_cast<SpvCapability>(word));
}

template <class T>
std::string KeyIsLabel(T key) {
  std::stringstream ss;
  ss << key;
  return ss.str();
}

template <class Key>
std::unordered_map<Key, double> GetRecall(
    const std::unordered_map<Key, uint32_t>& hist, uint64_t total) {
  std::unordered_map<Key, double> freq;
  for (const auto& pair : hist) {
    const double frequency =
        static_cast<double>(pair.second) / static_cast<double>(total);
    freq.emplace(pair.first, frequency);
  }
  return freq;
}

template <class Key>
std::unordered_map<Key, double> GetPrevalence(
    const std::unordered_map<Key, uint32_t>& hist) {
  uint64_t total = 0;
  for (const auto& pair : hist) {
    total += pair.second;
  }

  return GetRecall(hist, total);
}

// Writes |freq| to |out| sorted by frequency in the following format:
// LABEL3 70%
// LABEL1 20%
// LABEL2 10%
// |label_from_key| is used to convert |Key| to label.
template <class Key>
void WriteFreq(std::ostream& out, const std::unordered_map<Key, double>& freq,
               std::string (*label_from_key)(Key), double threshold = 0.001) {
  std::vector<std::pair<Key, double>> sorted_freq(freq.begin(), freq.end());
  std::sort(sorted_freq.begin(), sorted_freq.end(),
            [](const std::pair<Key, double>& left,
               const std::pair<Key, double>& right) {
              return left.second > right.second;
            });

  for (const auto& pair : sorted_freq) {
    if (pair.second < threshold) break;
    out << label_from_key(pair.first) << " " << pair.second * 100.0 << "%"
        << std::endl;
  }
}

// Writes |hist| to |out| sorted by count in the following format:
// LABEL3 100
// LABEL1 50
// LABEL2 10
// |label_from_key| is used to convert |Key| to label.
template <class Key>
void WriteHist(std::ostream& out, const std::unordered_map<Key, uint32_t>& hist,
               std::string (*label_from_key)(Key)) {
  std::vector<std::pair<Key, uint32_t>> sorted_hist(hist.begin(), hist.end());
  std::sort(sorted_hist.begin(), sorted_hist.end(),
            [](const std::pair<Key, uint32_t>& left,
               const std::pair<Key, uint32_t>& right) {
              return left.second > right.second;
            });

  for (const auto& pair : sorted_hist) {
    out << label_from_key(pair.first) << " " << pair.second << std::endl;
  }
}

}  // namespace

StatsAnalyzer::StatsAnalyzer(const SpirvStats& stats) : stats_(stats) {
  num_modules_ = 0;
  for (const auto& pair : stats_.version_hist) {
    num_modules_ += pair.second;
  }

  version_freq_ = GetRecall(stats_.version_hist, num_modules_);
  generator_freq_ = GetRecall(stats_.generator_hist, num_modules_);
  capability_freq_ = GetRecall(stats_.capability_hist, num_modules_);
  extension_freq_ = GetRecall(stats_.extension_hist, num_modules_);
  opcode_freq_ = GetPrevalence(stats_.opcode_hist);
}

void StatsAnalyzer::WriteVersion(std::ostream& out) {
  WriteFreq(out, version_freq_, GetVersionString);
}

void StatsAnalyzer::WriteGenerator(std::ostream& out) {
  WriteFreq(out, generator_freq_, GetGeneratorString);
}

void StatsAnalyzer::WriteCapability(std::ostream& out) {
  WriteFreq(out, capability_freq_, GetCapabilityString);
}

void StatsAnalyzer::WriteExtension(std::ostream& out) {
  WriteFreq(out, extension_freq_, KeyIsLabel);
}

void StatsAnalyzer::WriteOpcode(std::ostream& out) {
  out << "Total unique opcodes used: " << opcode_freq_.size() << std::endl;
  WriteFreq(out, opcode_freq_, GetOpcodeString);
}

void StatsAnalyzer::WriteConstantLiterals(std::ostream& out) {
  out << "Constant literals" << std::endl;

  out << "Float 32" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.f32_constant_hist), KeyIsLabel);

  out << std::endl << "Float 64" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.f64_constant_hist), KeyIsLabel);

  out << std::endl << "Unsigned int 16" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.u16_constant_hist), KeyIsLabel);

  out << std::endl << "Signed int 16" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.s16_constant_hist), KeyIsLabel);

  out << std::endl << "Unsigned int 32" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.u32_constant_hist), KeyIsLabel);

  out << std::endl << "Signed int 32" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.s32_constant_hist), KeyIsLabel);

  out << std::endl << "Unsigned int 64" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.u64_constant_hist), KeyIsLabel);

  out << std::endl << "Signed int 64" << std::endl;
  WriteFreq(out, GetPrevalence(stats_.s64_constant_hist), KeyIsLabel);
}

void StatsAnalyzer::WriteOpcodeMarkov(std::ostream& out) {
  if (stats_.opcode_markov_hist.empty()) return;

  const std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>>&
      cue_to_hist = stats_.opcode_markov_hist[0];

  // Sort by prevalence of the opcodes in opcode_freq_ (descending).
  std::vector<std::pair<uint32_t, std::unordered_map<uint32_t, uint32_t>>>
      sorted_cue_to_hist(cue_to_hist.begin(), cue_to_hist.end());
  std::sort(
      sorted_cue_to_hist.begin(), sorted_cue_to_hist.end(),
      [this](const std::pair<uint32_t, std::unordered_map<uint32_t, uint32_t>>&
                 left,
             const std::pair<uint32_t, std::unordered_map<uint32_t, uint32_t>>&
                 right) {
        const double lf = opcode_freq_[left.first];
        const double rf = opcode_freq_[right.first];
        if (lf == rf) return right.first > left.first;
        return lf > rf;
      });

  for (const auto& kv : sorted_cue_to_hist) {
    const uint32_t cue = kv.first;
    const double kFrequentEnoughToAnalyze = 0.0001;
    if (opcode_freq_[cue] < kFrequentEnoughToAnalyze) continue;

    const std::unordered_map<uint32_t, uint32_t>& hist = kv.second;

    uint32_t total = 0;
    for (const auto& pair : hist) {
      total += pair.second;
    }

    std::vector<std::pair<uint32_t, uint32_t>> sorted_hist(hist.begin(),
                                                           hist.end());
    std::sort(sorted_hist.begin(), sorted_hist.end(),
              [](const std::pair<uint32_t, uint32_t>& left,
                 const std::pair<uint32_t, uint32_t>& right) {
                if (left.second == right.second)
                  return right.first > left.first;
                return left.second > right.second;
              });

    for (const auto& pair : sorted_hist) {
      const double prior = opcode_freq_[pair.first];
      const double posterior =
          static_cast<double>(pair.second) / static_cast<double>(total);
      out << GetOpcodeString(cue) << " -> " << GetOpcodeString(pair.first)
          << " " << posterior * 100 << "% (base rate " << prior * 100
          << "%, pair occurrences " << pair.second << ")" << std::endl;
    }
  }
}

void StatsAnalyzer::WriteCodegenOpcodeHist(std::ostream& out) {
  auto all_opcodes = GetAllOpcodes();

  // uint64_t is used because kMarkvNoneOfTheAbove is outside of uint32_t range.
  out << "std::map<uint64_t, uint32_t> GetOpcodeHist() {\n"
      << "  return std::map<uint64_t, uint32_t>({\n";

  uint32_t total = 0;
  for (const auto& kv : stats_.opcode_hist) {
    total += kv.second;
  }

  for (uint32_t opcode : all_opcodes) {
    const auto it = stats_.opcode_hist.find(opcode);
    const uint32_t count = it == stats_.opcode_hist.end() ? 0 : it->second;
    const double kMaxValue = 1000.0;
    uint32_t value = uint32_t(kMaxValue * double(count) / double(total));
    if (value == 0) value = 1;
    out << "    { SpvOp" << GetOpcodeString(opcode) << ", " << value << " },\n";
  }

  // Add kMarkvNoneOfTheAbove as a signal for unknown opcode.
  out << "    { kMarkvNoneOfTheAbove, " << 10 << " },\n";
  out << "  });\n}\n";
}

void StatsAnalyzer::WriteCodegenOpcodeAndNumOperandsHist(std::ostream& out) {
  out << "std::map<uint64_t, uint32_t> GetOpcodeAndNumOperandsHist() {\n"
      << "  return std::map<uint64_t, uint32_t>({\n";

  uint32_t total = 0;
  for (const auto& kv : stats_.opcode_and_num_operands_hist) {
    total += kv.second;
  }

  uint32_t left_out = 0;

  for (const auto& kv : stats_.opcode_and_num_operands_hist) {
    const uint32_t count = kv.second;
    const double kFrequentEnoughToAnalyze = 0.001;
    const uint32_t opcode_and_num_operands = kv.first;
    const uint32_t opcode = opcode_and_num_operands & 0xFFFF;
    const uint32_t num_operands = opcode_and_num_operands >> 16;

    if (opcode == SpvOpTypeStruct ||
        double(count) / double(total) < kFrequentEnoughToAnalyze) {
      left_out += count;
      continue;
    }

    out << "    { CombineOpcodeAndNumOperands(SpvOp"
        << spvOpcodeString(SpvOp(opcode)) << ", " << num_operands << "), "
        << count << " },\n";
  }

  // Heuristic.
  const uint32_t none_of_the_above = std::max(1, int(left_out + total * 0.01));
  out << "    { kMarkvNoneOfTheAbove, " << none_of_the_above << " },\n";
  out << "  });\n}\n";
}

void StatsAnalyzer::WriteCodegenOpcodeAndNumOperandsMarkovHuffmanCodecs(
    std::ostream& out) {
  out << "std::map<uint32_t, std::unique_ptr<HuffmanCodec<uint64_t>>>\n"
      << "GetOpcodeAndNumOperandsMarkovHuffmanCodecs() {\n"
      << "  std::map<uint32_t, std::unique_ptr<HuffmanCodec<uint64_t>>> "
      << "codecs;\n";

  for (const auto& kv : stats_.opcode_and_num_operands_markov_hist) {
    const uint32_t prev_opcode = kv.first;
    const double kFrequentEnoughToAnalyze = 0.001;
    if (opcode_freq_[prev_opcode] < kFrequentEnoughToAnalyze) continue;

    const std::unordered_map<uint32_t, uint32_t>& hist = kv.second;

    uint32_t total = 0;
    for (const auto& pair : hist) {
      total += pair.second;
    }

    uint32_t left_out = 0;

    std::map<uint64_t, uint32_t> processed_hist;
    for (const auto& pair : hist) {
      const uint32_t opcode_and_num_operands = pair.first;
      const uint32_t opcode = opcode_and_num_operands & 0xFFFF;

      if (opcode == SpvOpTypeStruct) continue;

      const uint32_t num_operands = opcode_and_num_operands >> 16;
      const uint32_t count = pair.second;
      const double posterior_freq = double(count) / double(total);

      if (opcode_freq_[opcode] < kFrequentEnoughToAnalyze &&
          posterior_freq < kFrequentEnoughToAnalyze) {
        left_out += count;
        continue;
      }
      processed_hist.emplace(CombineOpcodeAndNumOperands(opcode, num_operands),
                             count);
    }

    // Heuristic.
    processed_hist.emplace(kMarkvNoneOfTheAbove,
                           std::max(1, int(left_out + total * 0.01)));

    HuffmanCodec<uint64_t> codec(processed_hist);

    out << "  {\n";
    out << "    std::unique_ptr<HuffmanCodec<uint64_t>> "
        << "codec(new HuffmanCodec<uint64_t>";
    out << codec.SerializeToText(4);
    out << ");\n" << std::endl;
    out << "    codecs.emplace(SpvOp" << GetOpcodeString(prev_opcode)
        << ", std::move(codec));\n";
    out << "  }\n\n";
  }

  out << "  return codecs;\n}\n";
}

void StatsAnalyzer::WriteCodegenLiteralStringHuffmanCodecs(std::ostream& out) {
  out << "std::map<uint32_t, std::unique_ptr<HuffmanCodec<std::string>>>\n"
      << "GetLiteralStringHuffmanCodecs() {\n"
      << "  std::map<uint32_t, std::unique_ptr<HuffmanCodec<std::string>>> "
      << "codecs;\n";

  for (const auto& kv : stats_.literal_strings_hist) {
    const uint32_t opcode = kv.first;

    if (opcode == SpvOpName || opcode == SpvOpMemberName) continue;

    const double kOpcodeFrequentEnoughToAnalyze = 0.001;
    if (opcode_freq_[opcode] < kOpcodeFrequentEnoughToAnalyze) continue;

    const std::unordered_map<std::string, uint32_t>& hist = kv.second;

    uint32_t total = 0;
    for (const auto& pair : hist) {
      total += pair.second;
    }

    uint32_t left_out = 0;

    std::map<std::string, uint32_t> processed_hist;
    for (const auto& pair : hist) {
      const uint32_t count = pair.second;
      const double freq = double(count) / double(total);
      const double kStringFrequentEnoughToAnalyze = 0.001;
      if (freq < kStringFrequentEnoughToAnalyze) {
        left_out += count;
        continue;
      }
      processed_hist.emplace(pair.first, count);
    }

    // Heuristic.
    processed_hist.emplace("kMarkvNoneOfTheAbove",
                           std::max(1, int(left_out + total * 0.01)));

    HuffmanCodec<std::string> codec(processed_hist);

    out << "  {\n";
    out << "    std::unique_ptr<HuffmanCodec<std::string>> "
        << "codec(new HuffmanCodec<std::string>";
    out << codec.SerializeToText(4);
    out << ");\n" << std::endl;
    out << "    codecs.emplace(SpvOp" << spvOpcodeString(SpvOp(opcode))
        << ", std::move(codec));\n";
    out << "  }\n\n";
  }

  out << "  return codecs;\n}\n";
}

void StatsAnalyzer::WriteCodegenNonIdWordHuffmanCodecs(std::ostream& out) {
  out << "std::map<std::pair<uint32_t, uint32_t>, "
      << "std::unique_ptr<HuffmanCodec<uint64_t>>>\n"
      << "GetNonIdWordHuffmanCodecs() {\n"
      << "  std::map<std::pair<uint32_t, uint32_t>, "
      << "std::unique_ptr<HuffmanCodec<uint64_t>>> codecs;\n";

  for (const auto& kv : stats_.operand_slot_non_id_words_hist) {
    const auto& opcode_and_index = kv.first;
    const uint32_t opcode = opcode_and_index.first;
    const uint32_t index = opcode_and_index.second;

    const double kOpcodeFrequentEnoughToAnalyze = 0.001;
    if (opcode_freq_[opcode] < kOpcodeFrequentEnoughToAnalyze) continue;

    const std::map<uint32_t, uint32_t>& hist = kv.second;

    uint32_t total = 0;
    for (const auto& pair : hist) {
      total += pair.second;
    }

    uint32_t left_out = 0;

    std::map<uint64_t, uint32_t> processed_hist;
    for (const auto& pair : hist) {
      const uint32_t word = pair.first;
      const uint32_t count = pair.second;
      const double freq = double(count) / double(total);
      const double kWordFrequentEnoughToAnalyze = 0.003;
      if (freq < kWordFrequentEnoughToAnalyze) {
        left_out += count;
        continue;
      }
      processed_hist.emplace(word, count);
    }

    // Heuristic.
    processed_hist.emplace(kMarkvNoneOfTheAbove,
                           std::max(1, int(left_out + total * 0.01)));

    HuffmanCodec<uint64_t> codec(processed_hist);

    out << "  {\n";
    out << "    std::unique_ptr<HuffmanCodec<uint64_t>> "
        << "codec(new HuffmanCodec<uint64_t>";
    out << codec.SerializeToText(4);
    out << ");\n" << std::endl;
    out << "    codecs.emplace(std::pair<uint32_t, uint32_t>(SpvOp"
        << spvOpcodeString(SpvOp(opcode)) << ", " << index
        << "), std::move(codec));\n";
    out << "  }\n\n";
  }

  out << "  return codecs;\n}\n";
}

void StatsAnalyzer::WriteCodegenIdDescriptorHuffmanCodecs(std::ostream& out) {
  out << "std::map<std::pair<uint32_t, uint32_t>, "
      << "std::unique_ptr<HuffmanCodec<uint64_t>>>\n"
      << "GetIdDescriptorHuffmanCodecs() {\n"
      << "  std::map<std::pair<uint32_t, uint32_t>, "
      << "std::unique_ptr<HuffmanCodec<uint64_t>>> codecs;\n";

  std::unordered_set<uint32_t> descriptors_with_coding_scheme;

  for (const auto& kv : stats_.operand_slot_id_descriptor_hist) {
    const auto& opcode_and_index = kv.first;
    const uint32_t opcode = opcode_and_index.first;
    const uint32_t index = opcode_and_index.second;

    const double kOpcodeFrequentEnoughToAnalyze = 0.003;
    if (opcode_freq_[opcode] < kOpcodeFrequentEnoughToAnalyze) continue;

    const std::map<uint32_t, uint32_t>& hist = kv.second;

    uint32_t total = 0;
    for (const auto& pair : hist) {
      total += pair.second;
    }

    uint32_t left_out = 0;

    std::map<uint64_t, uint32_t> processed_hist;
    for (const auto& pair : hist) {
      const uint32_t descriptor = pair.first;
      const uint32_t count = pair.second;
      const double freq = double(count) / double(total);
      const double kDescriptorFrequentEnoughToAnalyze = 0.003;
      if (freq < kDescriptorFrequentEnoughToAnalyze) {
        left_out += count;
        continue;
      }
      processed_hist.emplace(descriptor, count);
      descriptors_with_coding_scheme.insert(descriptor);
    }

    // Heuristic.
    processed_hist.emplace(kMarkvNoneOfTheAbove,
                           std::max(1, int(left_out + total * 0.01)));

    HuffmanCodec<uint64_t> codec(processed_hist);

    out << "  {\n";
    out << "    std::unique_ptr<HuffmanCodec<uint64_t>> "
        << "codec(new HuffmanCodec<uint64_t>";
    out << codec.SerializeToText(4);
    out << ");\n" << std::endl;
    out << "    codecs.emplace(std::pair<uint32_t, uint32_t>(SpvOp"
        << spvOpcodeString(SpvOp(opcode)) << ", " << index
        << "), std::move(codec));\n";
    out << "  }\n\n";
  }

  out << "  return codecs;\n}\n";

  out << "\nstd::unordered_set<uint32_t> GetDescriptorsWithCodingScheme() {\n"
      << "  std::unordered_set<uint32_t> descriptors_with_coding_scheme = {\n";
  for (uint32_t descriptor : descriptors_with_coding_scheme) {
    out << "    " << descriptor << ",\n";
  }
  out << "  };\n";
  out << "  return descriptors_with_coding_scheme;\n}\n";
}
