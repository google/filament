// Copyright (c) 2015-2020 The Khronos Group Inc.
// Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights
// reserved.
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

#include "source/operand.h"

#include <assert.h>
#include <string.h>

#include <algorithm>

#include "DebugInfo.h"
#include "OpenCLDebugInfo100.h"
#include "source/macro.h"
#include "source/opcode.h"
#include "source/spirv_constant.h"

// For now, assume unified1 contains up to SPIR-V 1.3 and no later
// SPIR-V version.
// TODO(dneto): Make one set of tables, but with version tags on a
// per-item basis. https://github.com/KhronosGroup/SPIRV-Tools/issues/1195

#include "operand.kinds-unified1.inc"
#include "spirv-tools/libspirv.h"

static const spv_operand_table_t kOperandTable = {
    ARRAY_SIZE(pygen_variable_OperandInfoTable),
    pygen_variable_OperandInfoTable};

spv_result_t spvOperandTableGet(spv_operand_table* pOperandTable,
                                spv_target_env) {
  if (!pOperandTable) return SPV_ERROR_INVALID_POINTER;

  *pOperandTable = &kOperandTable;
  return SPV_SUCCESS;
}

spv_result_t spvOperandTableNameLookup(spv_target_env,
                                       const spv_operand_table table,
                                       const spv_operand_type_t type,
                                       const char* name,
                                       const size_t nameLength,
                                       spv_operand_desc* pEntry) {
  if (!table) return SPV_ERROR_INVALID_TABLE;
  if (!name || !pEntry) return SPV_ERROR_INVALID_POINTER;

  for (uint64_t typeIndex = 0; typeIndex < table->count; ++typeIndex) {
    const auto& group = table->types[typeIndex];
    if (type != group.type) continue;
    for (uint64_t index = 0; index < group.count; ++index) {
      const auto& entry = group.entries[index];
      // We consider the current operand as available as long as
      // it is in the grammar.  It might not be *valid* to use,
      // but that should be checked by the validator, not by parsing.
      //
      // Exact match case
      if (nameLength == strlen(entry.name) &&
          !strncmp(entry.name, name, nameLength)) {
        *pEntry = &entry;
        return SPV_SUCCESS;
      }

      // Check the aliases. Ideally we would have a version of the table sorted
      // by name and then we could iterate between the lower and upper bounds to
      // restrict the amount comparisons. Fortunately, name-based lookups are
      // mostly restricted to the assembler.
      if (entry.numAliases > 0) {
        for (uint32_t aliasIndex = 0; aliasIndex < entry.numAliases;
             aliasIndex++) {
          const auto alias = entry.aliases[aliasIndex];
          const size_t aliasLength = strlen(alias);
          if (nameLength == aliasLength && !strncmp(name, alias, nameLength)) {
            *pEntry = &entry;
            return SPV_SUCCESS;
          }
        }
      }
    }
  }

  return SPV_ERROR_INVALID_LOOKUP;
}

spv_result_t spvOperandTableValueLookup(spv_target_env,
                                        const spv_operand_table table,
                                        const spv_operand_type_t type,
                                        const uint32_t value,
                                        spv_operand_desc* pEntry) {
  if (!table) return SPV_ERROR_INVALID_TABLE;
  if (!pEntry) return SPV_ERROR_INVALID_POINTER;

  spv_operand_desc_t needle = {"", value,   0,  nullptr, 0,  nullptr,
                               0,  nullptr, {}, ~0u,     ~0u};

  auto comp = [](const spv_operand_desc_t& lhs, const spv_operand_desc_t& rhs) {
    return lhs.value < rhs.value;
  };

  for (uint64_t typeIndex = 0; typeIndex < table->count; ++typeIndex) {
    const auto& group = table->types[typeIndex];
    if (type != group.type) continue;

    const auto beg = group.entries;
    const auto end = group.entries + group.count;

    // Assumes the underlying table is already sorted ascendingly according to
    // opcode value.
    auto it = std::lower_bound(beg, end, needle, comp);
    if (it != end && it->value == value) {
      // The current operand is considered available as long as
      // it is in the grammar.  It might not be *valid* to use,
      // but that should be checked by the validator, not by parsing.
      *pEntry = it;
      return SPV_SUCCESS;
    }
  }

  return SPV_ERROR_INVALID_LOOKUP;
}

const char* spvOperandTypeStr(spv_operand_type_t type) {
  switch (type) {
    case SPV_OPERAND_TYPE_ID:
    case SPV_OPERAND_TYPE_OPTIONAL_ID:
      return "ID";
    case SPV_OPERAND_TYPE_TYPE_ID:
      return "type ID";
    case SPV_OPERAND_TYPE_RESULT_ID:
      return "result ID";
    case SPV_OPERAND_TYPE_LITERAL_INTEGER:
    case SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER:
    case SPV_OPERAND_TYPE_OPTIONAL_LITERAL_NUMBER:
    case SPV_OPERAND_TYPE_LITERAL_FLOAT:
      return "literal number";
    case SPV_OPERAND_TYPE_OPTIONAL_TYPED_LITERAL_INTEGER:
      return "possibly multi-word literal integer";
    case SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER:
      return "possibly multi-word literal number";
    case SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER:
      return "extension instruction number";
    case SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER:
      return "OpSpecConstantOp opcode";
    case SPV_OPERAND_TYPE_LITERAL_STRING:
    case SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING:
      return "literal string";
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE:
      return "source language";
    case SPV_OPERAND_TYPE_EXECUTION_MODEL:
      return "execution model";
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL:
      return "addressing model";
    case SPV_OPERAND_TYPE_MEMORY_MODEL:
      return "memory model";
    case SPV_OPERAND_TYPE_EXECUTION_MODE:
      return "execution mode";
    case SPV_OPERAND_TYPE_STORAGE_CLASS:
      return "storage class";
    case SPV_OPERAND_TYPE_DIMENSIONALITY:
      return "dimensionality";
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE:
      return "sampler addressing mode";
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE:
      return "sampler filter mode";
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT:
      return "image format";
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE:
      return "floating-point fast math mode";
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE:
      return "floating-point rounding mode";
    case SPV_OPERAND_TYPE_LINKAGE_TYPE:
      return "linkage type";
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER:
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER:
      return "access qualifier";
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE:
      return "function parameter attribute";
    case SPV_OPERAND_TYPE_DECORATION:
      return "decoration";
    case SPV_OPERAND_TYPE_BUILT_IN:
      return "built-in";
    case SPV_OPERAND_TYPE_SELECTION_CONTROL:
      return "selection control";
    case SPV_OPERAND_TYPE_LOOP_CONTROL:
      return "loop control";
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL:
      return "function control";
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID:
      return "memory semantics ID";
    case SPV_OPERAND_TYPE_MEMORY_ACCESS:
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS:
      return "memory access";
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE:
      return "shading rate";
    case SPV_OPERAND_TYPE_SCOPE_ID:
      return "scope ID";
    case SPV_OPERAND_TYPE_GROUP_OPERATION:
      return "group operation";
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS:
      return "kernel enqeue flags";
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO:
      return "kernel profiling info";
    case SPV_OPERAND_TYPE_CAPABILITY:
      return "capability";
    case SPV_OPERAND_TYPE_RAY_FLAGS:
      return "ray flags";
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION:
      return "ray query intersection";
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE:
      return "ray query committed intersection type";
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE:
      return "ray query candidate intersection type";
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT:
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT:
      return "packed vector format";
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS:
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS:
      return "cooperative matrix operands";
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT:
      return "cooperative matrix layout";
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE:
      return "cooperative matrix use";
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE:
      return "tensor clamp mode";
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE:
      return "cooperative matrix reduce";
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS:
      return "tensor addressing operands";
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS:
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS:
      return "matrix multiply accumulate operands";
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER:
      return "initialization mode qualifier";
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER:
      return "host access qualifier";
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL:
      return "load cache control";
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL:
      return "store cache control";
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS:
      return "named maximum number of registers";
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS:
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS:
      return "raw access chain operands";
    case SPV_OPERAND_TYPE_IMAGE:
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE:
      return "image";
    case SPV_OPERAND_TYPE_OPTIONAL_CIV:
      return "context-insensitive value";
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS:
      return "debug info flags";
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING:
      return "debug base type encoding";
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE:
      return "debug composite type";
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER:
      return "debug type qualifier";
    case SPV_OPERAND_TYPE_DEBUG_OPERATION:
      return "debug operation";
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS:
      return "OpenCL.DebugInfo.100 debug info flags";
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING:
      return "OpenCL.DebugInfo.100 debug base type encoding";
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE:
      return "OpenCL.DebugInfo.100 debug composite type";
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER:
      return "OpenCL.DebugInfo.100 debug type qualifier";
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION:
      return "OpenCL.DebugInfo.100 debug operation";
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY:
      return "OpenCL.DebugInfo.100 debug imported entity";
    case SPV_OPERAND_TYPE_FPENCODING:
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING:
      return "FP encoding";

    // The next values are for values returned from an instruction, not actually
    // an operand.  So the specific strings don't matter.  But let's add them
    // for completeness and ease of testing.
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER:
      return "image channel order";
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE:
      return "image channel data type";

    case SPV_OPERAND_TYPE_FPDENORM_MODE:
      return "FP denorm mode";
    case SPV_OPERAND_TYPE_FPOPERATION_MODE:
      return "FP operation mode";
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES:
      return "quantization mode";
    case SPV_OPERAND_TYPE_OVERFLOW_MODES:
      return "overflow mode";
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT:
      return "cooperative vector matrix layout";
    case SPV_OPERAND_TYPE_COMPONENT_TYPE:
      return "component type";

    case SPV_OPERAND_TYPE_NONE:
      return "NONE";
    default:
      break;
  }
  return "unknown";
}

void spvPushOperandTypes(const spv_operand_type_t* types,
                         spv_operand_pattern_t* pattern) {
  const spv_operand_type_t* endTypes;
  for (endTypes = types; *endTypes != SPV_OPERAND_TYPE_NONE; ++endTypes) {
  }

  while (endTypes-- != types) {
    pattern->push_back(*endTypes);
  }
}

void spvPushOperandTypesForMask(spv_target_env env,
                                const spv_operand_table operandTable,
                                const spv_operand_type_t type,
                                const uint32_t mask,
                                spv_operand_pattern_t* pattern) {
  // Scan from highest bits to lowest bits because we will append in LIFO
  // fashion, and we need the operands for lower order bits to be consumed first
  for (uint32_t candidate_bit = (1u << 31u); candidate_bit;
       candidate_bit >>= 1) {
    if (candidate_bit & mask) {
      spv_operand_desc entry = nullptr;
      if (SPV_SUCCESS == spvOperandTableValueLookup(env, operandTable, type,
                                                    candidate_bit, &entry)) {
        spvPushOperandTypes(entry->operandTypes, pattern);
      }
    }
  }
}

bool spvOperandIsConcrete(spv_operand_type_t type) {
  if (spvIsIdType(type) || spvOperandIsConcreteMask(type)) {
    return true;
  }
  switch (type) {
    case SPV_OPERAND_TYPE_LITERAL_INTEGER:
    case SPV_OPERAND_TYPE_LITERAL_FLOAT:
    case SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER:
    case SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER:
    case SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER:
    case SPV_OPERAND_TYPE_LITERAL_STRING:
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE:
    case SPV_OPERAND_TYPE_EXECUTION_MODEL:
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL:
    case SPV_OPERAND_TYPE_MEMORY_MODEL:
    case SPV_OPERAND_TYPE_EXECUTION_MODE:
    case SPV_OPERAND_TYPE_STORAGE_CLASS:
    case SPV_OPERAND_TYPE_DIMENSIONALITY:
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE:
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE:
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT:
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER:
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE:
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE:
    case SPV_OPERAND_TYPE_LINKAGE_TYPE:
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER:
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE:
    case SPV_OPERAND_TYPE_DECORATION:
    case SPV_OPERAND_TYPE_BUILT_IN:
    case SPV_OPERAND_TYPE_GROUP_OPERATION:
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS:
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO:
    case SPV_OPERAND_TYPE_CAPABILITY:
    case SPV_OPERAND_TYPE_RAY_FLAGS:
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION:
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE:
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE:
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING:
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE:
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER:
    case SPV_OPERAND_TYPE_DEBUG_OPERATION:
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING:
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE:
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER:
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION:
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY:
    case SPV_OPERAND_TYPE_FPDENORM_MODE:
    case SPV_OPERAND_TYPE_FPOPERATION_MODE:
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES:
    case SPV_OPERAND_TYPE_OVERFLOW_MODES:
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT:
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT:
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE:
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER:
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER:
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL:
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL:
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS:
    case SPV_OPERAND_TYPE_FPENCODING:
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE:
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT:
    case SPV_OPERAND_TYPE_COMPONENT_TYPE:
      return true;
    default:
      break;
  }
  return false;
}

bool spvOperandIsConcreteMask(spv_operand_type_t type) {
  switch (type) {
    case SPV_OPERAND_TYPE_IMAGE:
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE:
    case SPV_OPERAND_TYPE_SELECTION_CONTROL:
    case SPV_OPERAND_TYPE_LOOP_CONTROL:
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL:
    case SPV_OPERAND_TYPE_MEMORY_ACCESS:
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE:
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS:
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS:
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS:
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS:
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS:
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE:
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS:
      return true;
    default:
      break;
  }
  return false;
}

bool spvOperandIsOptional(spv_operand_type_t type) {
  switch (type) {
    case SPV_OPERAND_TYPE_OPTIONAL_ID:
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE:
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS:
    case SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER:
    case SPV_OPERAND_TYPE_OPTIONAL_LITERAL_NUMBER:
    case SPV_OPERAND_TYPE_OPTIONAL_TYPED_LITERAL_INTEGER:
    case SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING:
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER:
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT:
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS:
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS:
    case SPV_OPERAND_TYPE_OPTIONAL_CIV:
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS:
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING:
      return true;
    default:
      break;
  }
  // Any variable operand is also optional.
  return spvOperandIsVariable(type);
}

bool spvOperandIsVariable(spv_operand_type_t type) {
  switch (type) {
    case SPV_OPERAND_TYPE_VARIABLE_ID:
    case SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER:
    case SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID:
    case SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER:
      return true;
    default:
      break;
  }
  return false;
}

bool spvExpandOperandSequenceOnce(spv_operand_type_t type,
                                  spv_operand_pattern_t* pattern) {
  switch (type) {
    case SPV_OPERAND_TYPE_VARIABLE_ID:
      pattern->push_back(type);
      pattern->push_back(SPV_OPERAND_TYPE_OPTIONAL_ID);
      return true;
    case SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER:
      pattern->push_back(type);
      pattern->push_back(SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER);
      return true;
    case SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID:
      // Represents Zero or more (Literal number, Id) pairs,
      // where the literal number must be a scalar integer.
      pattern->push_back(type);
      pattern->push_back(SPV_OPERAND_TYPE_ID);
      pattern->push_back(SPV_OPERAND_TYPE_OPTIONAL_TYPED_LITERAL_INTEGER);
      return true;
    case SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER:
      // Represents Zero or more (Id, Literal number) pairs.
      pattern->push_back(type);
      pattern->push_back(SPV_OPERAND_TYPE_LITERAL_INTEGER);
      pattern->push_back(SPV_OPERAND_TYPE_OPTIONAL_ID);
      return true;
    default:
      break;
  }
  return false;
}

spv_operand_type_t spvTakeFirstMatchableOperand(
    spv_operand_pattern_t* pattern) {
  assert(!pattern->empty());
  spv_operand_type_t result;
  do {
    result = pattern->back();
    pattern->pop_back();
  } while (spvExpandOperandSequenceOnce(result, pattern));
  return result;
}

spv_operand_pattern_t spvAlternatePatternFollowingImmediate(
    const spv_operand_pattern_t& pattern) {
  auto it =
      std::find(pattern.crbegin(), pattern.crend(), SPV_OPERAND_TYPE_RESULT_ID);
  if (it != pattern.crend()) {
    spv_operand_pattern_t alternatePattern(it - pattern.crbegin() + 2,
                                           SPV_OPERAND_TYPE_OPTIONAL_CIV);
    alternatePattern[1] = SPV_OPERAND_TYPE_RESULT_ID;
    return alternatePattern;
  }

  // No result-id found, so just expect CIVs.
  return {SPV_OPERAND_TYPE_OPTIONAL_CIV};
}

bool spvIsIdType(spv_operand_type_t type) {
  switch (type) {
    case SPV_OPERAND_TYPE_ID:
    case SPV_OPERAND_TYPE_TYPE_ID:
    case SPV_OPERAND_TYPE_RESULT_ID:
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID:
    case SPV_OPERAND_TYPE_SCOPE_ID:
      return true;
    default:
      return false;
  }
}

bool spvIsInIdType(spv_operand_type_t type) {
  if (!spvIsIdType(type)) {
    // If it is not an ID it cannot be an input ID.
    return false;
  }
  switch (type) {
    // Deny non-input IDs.
    case SPV_OPERAND_TYPE_TYPE_ID:
    case SPV_OPERAND_TYPE_RESULT_ID:
      return false;
    default:
      return true;
  }
}

std::function<bool(unsigned)> spvOperandCanBeForwardDeclaredFunction(
    spv::Op opcode) {
  std::function<bool(unsigned index)> out;
  if (spvOpcodeGeneratesType(opcode)) {
    // All types can use forward pointers.
    out = [](unsigned) { return true; };
    return out;
  }
  switch (opcode) {
    case spv::Op::OpExecutionMode:
    case spv::Op::OpExecutionModeId:
    case spv::Op::OpEntryPoint:
    case spv::Op::OpName:
    case spv::Op::OpMemberName:
    case spv::Op::OpSelectionMerge:
    case spv::Op::OpDecorate:
    case spv::Op::OpMemberDecorate:
    case spv::Op::OpDecorateId:
    case spv::Op::OpDecorateStringGOOGLE:
    case spv::Op::OpMemberDecorateStringGOOGLE:
    case spv::Op::OpBranch:
    case spv::Op::OpLoopMerge:
      out = [](unsigned) { return true; };
      break;
    case spv::Op::OpGroupDecorate:
    case spv::Op::OpGroupMemberDecorate:
    case spv::Op::OpBranchConditional:
    case spv::Op::OpSwitch:
      out = [](unsigned index) { return index != 0; };
      break;

    case spv::Op::OpFunctionCall:
      // The Function parameter.
      out = [](unsigned index) { return index == 2; };
      break;

    case spv::Op::OpPhi:
      out = [](unsigned index) { return index > 1; };
      break;

    case spv::Op::OpEnqueueKernel:
      // The Invoke parameter.
      out = [](unsigned index) { return index == 8; };
      break;

    case spv::Op::OpGetKernelNDrangeSubGroupCount:
    case spv::Op::OpGetKernelNDrangeMaxSubGroupSize:
      // The Invoke parameter.
      out = [](unsigned index) { return index == 3; };
      break;

    case spv::Op::OpGetKernelWorkGroupSize:
    case spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple:
      // The Invoke parameter.
      out = [](unsigned index) { return index == 2; };
      break;
    case spv::Op::OpTypeForwardPointer:
      out = [](unsigned index) { return index == 0; };
      break;
    case spv::Op::OpTypeArray:
      out = [](unsigned index) { return index == 1; };
      break;
    case spv::Op::OpCooperativeMatrixPerElementOpNV:
      out = [](unsigned index) { return index == 3; };
      break;
    case spv::Op::OpCooperativeMatrixReduceNV:
      out = [](unsigned index) { return index == 4; };
      break;
    case spv::Op::OpCooperativeMatrixLoadTensorNV:
      // approximate, due to variable operands
      out = [](unsigned index) { return index > 6; };
      break;
    default:
      out = [](unsigned) { return false; };
      break;
  }
  return out;
}

std::function<bool(unsigned)> spvDbgInfoExtOperandCanBeForwardDeclaredFunction(
    spv::Op opcode, spv_ext_inst_type_t ext_type, uint32_t key) {
  // The Vulkan debug info extended instruction set is non-semantic so allows no
  // forward references except if used through OpExtInstWithForwardRefsKHR.
  if (ext_type == SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100) {
    return [opcode](unsigned) {
      return opcode == spv::Op::OpExtInstWithForwardRefsKHR;
    };
  }

  // TODO(https://gitlab.khronos.org/spirv/SPIR-V/issues/532): Forward
  // references for debug info instructions are still in discussion. We must
  // update the following lines of code when we conclude the spec.
  std::function<bool(unsigned index)> out;
  if (ext_type == SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100) {
    switch (OpenCLDebugInfo100Instructions(key)) {
      case OpenCLDebugInfo100DebugFunction:
        out = [](unsigned index) { return index == 13; };
        break;
      case OpenCLDebugInfo100DebugTypeComposite:
        out = [](unsigned index) { return index >= 13; };
        break;
      default:
        out = [](unsigned) { return false; };
        break;
    }
  } else {
    switch (DebugInfoInstructions(key)) {
      case DebugInfoDebugFunction:
        out = [](unsigned index) { return index == 13; };
        break;
      case DebugInfoDebugTypeComposite:
        out = [](unsigned index) { return index >= 12; };
        break;
      default:
        out = [](unsigned) { return false; };
        break;
    }
  }
  return out;
}
