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

#include "source/val/validate.h"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stack>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "source/diagnostic.h"
#include "source/instruction.h"
#include "source/opcode.h"
#include "source/operand.h"
#include "source/spirv_validator_options.h"
#include "source/val/function.h"
#include "source/val/validation_state.h"
#include "spirv-tools/libspirv.h"

namespace spvtools {
namespace val {
namespace {

class idUsage {
 public:
  idUsage(spv_const_context context, const spv_instruction_t* pInsts,
          const uint64_t instCountArg, const SpvMemoryModel memoryModelArg,
          const SpvAddressingModel addressingModelArg,
          const ValidationState_t& module,
          const std::vector<uint32_t>& entry_points, spv_position positionArg,
          const MessageConsumer& consumer)
      : targetEnv(context->target_env),
        opcodeTable(context->opcode_table),
        operandTable(context->operand_table),
        extInstTable(context->ext_inst_table),
        firstInst(pInsts),
        instCount(instCountArg),
        memoryModel(memoryModelArg),
        addressingModel(addressingModelArg),
        position(positionArg),
        consumer_(consumer),
        module_(module),
        entry_points_(entry_points) {}

  bool isValid(const spv_instruction_t* inst);

  template <SpvOp>
  bool isValid(const spv_instruction_t* inst, const spv_opcode_desc);

 private:
  const spv_target_env targetEnv;
  const spv_opcode_table opcodeTable;
  const spv_operand_table operandTable;
  const spv_ext_inst_table extInstTable;
  const spv_instruction_t* const firstInst;
  const uint64_t instCount;
  const SpvMemoryModel memoryModel;
  const SpvAddressingModel addressingModel;
  spv_position position;
  const MessageConsumer& consumer_;
  const ValidationState_t& module_;
  std::vector<uint32_t> entry_points_;
};

#define DIAG(inst)                                                          \
  position->index = inst ? inst->LineNum() : -1;                            \
  std::string disassembly;                                                  \
  if (inst) {                                                               \
    disassembly = module_.Disassemble(                                      \
        inst->words().data(), static_cast<uint16_t>(inst->words().size())); \
  }                                                                         \
  DiagnosticStream helper(*position, consumer_, disassembly,                \
                          SPV_ERROR_INVALID_DIAGNOSTIC);                    \
  helper

template <>
bool idUsage::isValid<SpvOpConstantTrue>(const spv_instruction_t* inst,
                                         const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || SpvOpTypeBool != resultType->opcode()) {
    DIAG(resultType) << "OpConstantTrue Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' is not a boolean type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpConstantFalse>(const spv_instruction_t* inst,
                                          const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || SpvOpTypeBool != resultType->opcode()) {
    DIAG(resultType) << "OpConstantFalse Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' is not a boolean type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpConstantComposite>(const spv_instruction_t* inst,
                                              const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || !spvOpcodeIsComposite(resultType->opcode())) {
    DIAG(resultType) << "OpConstantComposite Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' is not a composite type.";
    return false;
  }

  auto constituentCount = inst->words.size() - 3;
  switch (resultType->opcode()) {
    case SpvOpTypeVector: {
      auto componentCount = resultType->words()[3];
      if (componentCount != constituentCount) {
        // TODO: Output ID's on diagnostic
        DIAG(module_.FindDef(inst->words.back()))
            << "OpConstantComposite Constituent <id> count does not match "
               "Result Type <id> '"
            << module_.getIdName(resultType->id())
            << "'s vector component count.";
        return false;
      }
      auto componentType = module_.FindDef(resultType->words()[2]);
      assert(componentType);
      for (size_t constituentIndex = 3; constituentIndex < inst->words.size();
           constituentIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        if (!constituent ||
            !spvOpcodeIsConstantOrUndef(constituent->opcode())) {
          DIAG(constituent) << "OpConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant or undef.";
          return false;
        }
        auto constituentResultType = module_.FindDef(constituent->type_id());
        if (!constituentResultType ||
            componentType->opcode() != constituentResultType->opcode()) {
          DIAG(constituent) << "OpConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "'s type does not match Result Type <id> '"
                            << module_.getIdName(resultType->id())
                            << "'s vector element type.";
          return false;
        }
      }
    } break;
    case SpvOpTypeMatrix: {
      auto columnCount = resultType->words()[3];
      if (columnCount != constituentCount) {
        // TODO: Output ID's on diagnostic
        DIAG(module_.FindDef(inst->words.back()))
            << "OpConstantComposite Constituent <id> count does not match "
               "Result Type <id> '"
            << module_.getIdName(resultType->id()) << "'s matrix column count.";
        return false;
      }

      auto columnType = module_.FindDef(resultType->words()[2]);
      assert(columnType);
      auto componentCount = columnType->words()[3];
      auto componentType = module_.FindDef(columnType->words()[2]);
      assert(componentType);

      for (size_t constituentIndex = 3; constituentIndex < inst->words.size();
           constituentIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        if (!constituent || !(SpvOpConstantComposite == constituent->opcode() ||
                              SpvOpUndef == constituent->opcode())) {
          // The message says "... or undef" because the spec does not say
          // undef is a constant.
          DIAG(constituent) << "OpConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant composite or undef.";
          return false;
        }
        auto vector = module_.FindDef(constituent->type_id());
        assert(vector);
        if (columnType->opcode() != vector->opcode()) {
          DIAG(constituent) << "OpConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' type does not match Result Type <id> '"
                            << module_.getIdName(resultType->id())
                            << "'s matrix column type.";
          return false;
        }
        auto vectorComponentType = module_.FindDef(vector->words()[2]);
        assert(vectorComponentType);
        if (componentType->id() != vectorComponentType->id()) {
          DIAG(constituent)
              << "OpConstantComposite Constituent <id> '"
              << module_.getIdName(inst->words[constituentIndex])
              << "' component type does not match Result Type <id> '"
              << module_.getIdName(resultType->id())
              << "'s matrix column component type.";
          return false;
        }
        if (componentCount != vector->words()[3]) {
          DIAG(constituent)
              << "OpConstantComposite Constituent <id> '"
              << module_.getIdName(inst->words[constituentIndex])
              << "' vector component count does not match Result Type <id> '"
              << module_.getIdName(resultType->id())
              << "'s vector component count.";
          return false;
        }
      }
    } break;
    case SpvOpTypeArray: {
      auto elementType = module_.FindDef(resultType->words()[2]);
      assert(elementType);
      auto length = module_.FindDef(resultType->words()[3]);
      assert(length);
      if (length->words()[3] != constituentCount) {
        DIAG(module_.FindDef(inst->words.back()))
            << "OpConstantComposite Constituent count does not match "
               "Result Type <id> '"
            << module_.getIdName(resultType->id()) << "'s array length.";
        return false;
      }
      for (size_t constituentIndex = 3; constituentIndex < inst->words.size();
           constituentIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        if (!constituent ||
            !spvOpcodeIsConstantOrUndef(constituent->opcode())) {
          DIAG(constituent) << "OpConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant or undef.";
          return false;
        }
        auto constituentType = module_.FindDef(constituent->type_id());
        assert(constituentType);
        if (elementType->id() != constituentType->id()) {
          DIAG(constituent) << "OpConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "'s type does not match Result Type <id> '"
                            << module_.getIdName(resultType->id())
                            << "'s array element type.";
          return false;
        }
      }
    } break;
    case SpvOpTypeStruct: {
      auto memberCount = resultType->words().size() - 2;
      if (memberCount != constituentCount) {
        DIAG(resultType) << "OpConstantComposite Constituent <id> '"
                         << module_.getIdName(inst->words[resultTypeIndex])
                         << "' count does not match Result Type <id> '"
                         << module_.getIdName(resultType->id())
                         << "'s struct member count.";
        return false;
      }
      for (uint32_t constituentIndex = 3, memberIndex = 2;
           constituentIndex < inst->words.size();
           constituentIndex++, memberIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        if (!constituent ||
            !spvOpcodeIsConstantOrUndef(constituent->opcode())) {
          DIAG(constituent) << "OpConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant or undef.";
          return false;
        }
        auto constituentType = module_.FindDef(constituent->type_id());
        assert(constituentType);

        auto memberType = module_.FindDef(resultType->words()[memberIndex]);
        assert(memberType);
        if (memberType->id() != constituentType->id()) {
          DIAG(constituent)
              << "OpConstantComposite Constituent <id> '"
              << module_.getIdName(inst->words[constituentIndex])
              << "' type does not match the Result Type <id> '"
              << module_.getIdName(resultType->id()) << "'s member type.";
          return false;
        }
      }
    } break;
    default: { assert(0 && "Unreachable!"); } break;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpConstantSampler>(const spv_instruction_t* inst,
                                            const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || SpvOpTypeSampler != resultType->opcode()) {
    DIAG(resultType) << "OpConstantSampler Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' is not a sampler type.";
    return false;
  }
  return true;
}

// True if instruction defines a type that can have a null value, as defined by
// the SPIR-V spec.  Tracks composite-type components through module to check
// nullability transitively.
bool IsTypeNullable(const std::vector<uint32_t>& instruction,
                    const ValidationState_t& module) {
  uint16_t opcode;
  uint16_t word_count;
  spvOpcodeSplit(instruction[0], &word_count, &opcode);
  switch (static_cast<SpvOp>(opcode)) {
    case SpvOpTypeBool:
    case SpvOpTypeInt:
    case SpvOpTypeFloat:
    case SpvOpTypePointer:
    case SpvOpTypeEvent:
    case SpvOpTypeDeviceEvent:
    case SpvOpTypeReserveId:
    case SpvOpTypeQueue:
      return true;
    case SpvOpTypeArray:
    case SpvOpTypeMatrix:
    case SpvOpTypeVector: {
      auto base_type = module.FindDef(instruction[2]);
      return base_type && IsTypeNullable(base_type->words(), module);
    }
    case SpvOpTypeStruct: {
      for (size_t elementIndex = 2; elementIndex < instruction.size();
           ++elementIndex) {
        auto element = module.FindDef(instruction[elementIndex]);
        if (!element || !IsTypeNullable(element->words(), module)) return false;
      }
      return true;
    }
    default:
      return false;
  }
}

template <>
bool idUsage::isValid<SpvOpConstantNull>(const spv_instruction_t* inst,
                                         const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || !IsTypeNullable(resultType->words(), module_)) {
    DIAG(resultType) << "OpConstantNull Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' cannot have a null value.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpSpecConstantTrue>(const spv_instruction_t* inst,
                                             const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || SpvOpTypeBool != resultType->opcode()) {
    DIAG(resultType) << "OpSpecConstantTrue Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' is not a boolean type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpSpecConstantFalse>(const spv_instruction_t* inst,
                                              const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || SpvOpTypeBool != resultType->opcode()) {
    DIAG(resultType) << "OpSpecConstantFalse Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' is not a boolean type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpSampledImage>(const spv_instruction_t* inst,
                                         const spv_opcode_desc) {
  auto resultTypeIndex = 2;
  auto resultID = inst->words[resultTypeIndex];
  auto sampledImageInstr = module_.FindDef(resultID);
  // We need to validate 2 things:
  // * All OpSampledImage instructions must be in the same block in which their
  // Result <id> are consumed.
  // * Result <id> from OpSampledImage instructions must not appear as operands
  // to OpPhi instructions or OpSelect instructions, or any instructions other
  // than the image lookup and query instructions specified to take an operand
  // whose type is OpTypeSampledImage.
  std::vector<uint32_t> consumers = module_.getSampledImageConsumers(resultID);
  if (!consumers.empty()) {
    for (auto consumer_id : consumers) {
      auto consumer_instr = module_.FindDef(consumer_id);
      auto consumer_opcode = consumer_instr->opcode();
      if (consumer_instr->block() != sampledImageInstr->block()) {
        DIAG(sampledImageInstr)
            << "All OpSampledImage instructions must be in the same block in "
               "which their Result <id> are consumed. OpSampledImage Result "
               "Type <id> '"
            << module_.getIdName(resultID)
            << "' has a consumer in a different basic "
               "block. The consumer instruction <id> is '"
            << module_.getIdName(consumer_id) << "'.";
        return false;
      }
      // TODO: The following check is incomplete. We should also check that the
      // Sampled Image is not used by instructions that should not take
      // SampledImage as an argument. We could find the list of valid
      // instructions by scanning for "Sampled Image" in the operand description
      // field in the grammar file.
      if (consumer_opcode == SpvOpPhi || consumer_opcode == SpvOpSelect) {
        DIAG(sampledImageInstr)
            << "Result <id> from OpSampledImage instruction must not appear as "
               "operands of Op"
            << spvOpcodeString(static_cast<SpvOp>(consumer_opcode)) << "."
            << " Found result <id> '" << module_.getIdName(resultID)
            << "' as an operand of <id> '" << module_.getIdName(consumer_id)
            << "'.";
        return false;
      }
    }
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpSpecConstantComposite>(const spv_instruction_t* inst,
                                                  const spv_opcode_desc) {
  // The result type must be a composite type.
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType || !spvOpcodeIsComposite(resultType->opcode())) {
    DIAG(resultType) << "OpSpecConstantComposite Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' is not a composite type.";
    return false;
  }
  // Validation checks differ based on the type of composite type.
  auto constituentCount = inst->words.size() - 3;
  switch (resultType->opcode()) {
    // For Vectors, the following must be met:
    // * Number of constituents in the result type and the vector must match.
    // * All the components of the vector must have the same type (or specialize
    // to the same type). OpConstant and OpSpecConstant are allowed.
    // To check that condition, we check each supplied value argument's type
    // against the element type of the result type.
    case SpvOpTypeVector: {
      auto componentCount = resultType->words()[3];
      if (componentCount != constituentCount) {
        DIAG(module_.FindDef(inst->words.back()))
            << "OpSpecConstantComposite Constituent <id> count does not match "
               "Result Type <id> '"
            << module_.getIdName(resultType->id())
            << "'s vector component count.";
        return false;
      }
      auto componentType = module_.FindDef(resultType->words()[2]);
      assert(componentType);
      for (size_t constituentIndex = 3; constituentIndex < inst->words.size();
           constituentIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        if (!constituent ||
            !spvOpcodeIsConstantOrUndef(constituent->opcode())) {
          DIAG(constituent) << "OpSpecConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant or undef.";
          return false;
        }
        auto constituentResultType = module_.FindDef(constituent->type_id());
        if (!constituentResultType ||
            componentType->opcode() != constituentResultType->opcode()) {
          DIAG(constituent) << "OpSpecConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "'s type does not match Result Type <id> '"
                            << module_.getIdName(resultType->id())
                            << "'s vector element type.";
          return false;
        }
      }
      break;
    }
    case SpvOpTypeMatrix: {
      auto columnCount = resultType->words()[3];
      if (columnCount != constituentCount) {
        DIAG(module_.FindDef(inst->words.back()))
            << "OpSpecConstantComposite Constituent <id> count does not match "
               "Result Type <id> '"
            << module_.getIdName(resultType->id()) << "'s matrix column count.";
        return false;
      }

      auto columnType = module_.FindDef(resultType->words()[2]);
      assert(columnType);
      auto componentCount = columnType->words()[3];
      auto componentType = module_.FindDef(columnType->words()[2]);
      assert(componentType);

      for (size_t constituentIndex = 3; constituentIndex < inst->words.size();
           constituentIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        auto constituentOpCode = constituent->opcode();
        if (!constituent || !(SpvOpSpecConstantComposite == constituentOpCode ||
                              SpvOpConstantComposite == constituentOpCode ||
                              SpvOpUndef == constituentOpCode)) {
          // The message says "... or undef" because the spec does not say
          // undef is a constant.
          DIAG(constituent) << "OpSpecConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant composite or undef.";
          return false;
        }
        auto vector = module_.FindDef(constituent->type_id());
        assert(vector);
        if (columnType->opcode() != vector->opcode()) {
          DIAG(constituent) << "OpSpecConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' type does not match Result Type <id> '"
                            << module_.getIdName(resultType->id())
                            << "'s matrix column type.";
          return false;
        }
        auto vectorComponentType = module_.FindDef(vector->words()[2]);
        assert(vectorComponentType);
        if (componentType->id() != vectorComponentType->id()) {
          DIAG(constituent)
              << "OpSpecConstantComposite Constituent <id> '"
              << module_.getIdName(inst->words[constituentIndex])
              << "' component type does not match Result Type <id> '"
              << module_.getIdName(resultType->id())
              << "'s matrix column component type.";
          return false;
        }
        if (componentCount != vector->words()[3]) {
          DIAG(constituent)
              << "OpSpecConstantComposite Constituent <id> '"
              << module_.getIdName(inst->words[constituentIndex])
              << "' vector component count does not match Result Type <id> '"
              << module_.getIdName(resultType->id())
              << "'s vector component count.";
          return false;
        }
      }
      break;
    }
    case SpvOpTypeArray: {
      auto elementType = module_.FindDef(resultType->words()[2]);
      assert(elementType);
      auto length = module_.FindDef(resultType->words()[3]);
      assert(length);
      if (length->words()[3] != constituentCount) {
        DIAG(module_.FindDef(inst->words.back()))
            << "OpSpecConstantComposite Constituent count does not match "
               "Result Type <id> '"
            << module_.getIdName(resultType->id()) << "'s array length.";
        return false;
      }
      for (size_t constituentIndex = 3; constituentIndex < inst->words.size();
           constituentIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        if (!constituent ||
            !spvOpcodeIsConstantOrUndef(constituent->opcode())) {
          DIAG(constituent) << "OpSpecConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant or undef.";
          return false;
        }
        auto constituentType = module_.FindDef(constituent->type_id());
        assert(constituentType);
        if (elementType->id() != constituentType->id()) {
          DIAG(constituent) << "OpSpecConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "'s type does not match Result Type <id> '"
                            << module_.getIdName(resultType->id())
                            << "'s array element type.";
          return false;
        }
      }
      break;
    }
    case SpvOpTypeStruct: {
      auto memberCount = resultType->words().size() - 2;
      if (memberCount != constituentCount) {
        DIAG(resultType) << "OpSpecConstantComposite Constituent <id> '"
                         << module_.getIdName(inst->words[resultTypeIndex])
                         << "' count does not match Result Type <id> '"
                         << module_.getIdName(resultType->id())
                         << "'s struct member count.";
        return false;
      }
      for (uint32_t constituentIndex = 3, memberIndex = 2;
           constituentIndex < inst->words.size();
           constituentIndex++, memberIndex++) {
        auto constituent = module_.FindDef(inst->words[constituentIndex]);
        if (!constituent ||
            !spvOpcodeIsConstantOrUndef(constituent->opcode())) {
          DIAG(constituent) << "OpSpecConstantComposite Constituent <id> '"
                            << module_.getIdName(inst->words[constituentIndex])
                            << "' is not a constant or undef.";
          return false;
        }
        auto constituentType = module_.FindDef(constituent->type_id());
        assert(constituentType);

        auto memberType = module_.FindDef(resultType->words()[memberIndex]);
        assert(memberType);
        if (memberType->id() != constituentType->id()) {
          DIAG(constituent)
              << "OpSpecConstantComposite Constituent <id> '"
              << module_.getIdName(inst->words[constituentIndex])
              << "' type does not match the Result Type <id> '"
              << module_.getIdName(resultType->id()) << "'s member type.";
          return false;
        }
      }
      break;
    }
    default: { assert(0 && "Unreachable!"); } break;
  }
  return true;
}

bool idUsage::isValid(const spv_instruction_t* inst) {
  spv_opcode_desc opcodeEntry = nullptr;
  if (spvOpcodeTableValueLookup(targetEnv, opcodeTable, inst->opcode,
                                &opcodeEntry))
    return false;
#define CASE(OpCode) \
  case Spv##OpCode:  \
    return isValid<Spv##OpCode>(inst, opcodeEntry);
  switch (inst->opcode) {
    CASE(OpConstantTrue)
    CASE(OpConstantFalse)
    CASE(OpConstantComposite)
    CASE(OpConstantSampler)
    CASE(OpConstantNull)
    CASE(OpSpecConstantTrue)
    CASE(OpSpecConstantFalse)
    CASE(OpSpecConstantComposite)
    CASE(OpSampledImage)
    // Other composite opcodes are validated in validate_composites.cpp.
    // Arithmetic opcodes are validated in validate_arithmetics.cpp.
    // Bitwise opcodes are validated in validate_bitwise.cpp.
    // Logical opcodes are validated in validate_logicals.cpp.
    // Derivative opcodes are validated in validate_derivatives.cpp.
    default:
      return true;
  }
#undef TODO
#undef CASE
}

}  // namespace

spv_result_t UpdateIdUse(ValidationState_t& _, const Instruction* inst) {
  for (auto& operand : inst->operands()) {
    const spv_operand_type_t& type = operand.type;
    const uint32_t operand_id = inst->word(operand.offset);
    if (spvIsIdType(type) && type != SPV_OPERAND_TYPE_RESULT_ID) {
      if (auto def = _.FindDef(operand_id))
        def->RegisterUse(inst, operand.offset);
    }
  }

  return SPV_SUCCESS;
}

/// This function checks all ID definitions dominate their use in the CFG.
///
/// This function will iterate over all ID definitions that are defined in the
/// functions of a module and make sure that the definitions appear in a
/// block that dominates their use.
///
/// NOTE: This function does NOT check module scoped functions which are
/// checked during the initial binary parse in the IdPass below
spv_result_t CheckIdDefinitionDominateUse(const ValidationState_t& _) {
  std::unordered_set<const Instruction*> phi_instructions;
  for (const auto& definition : _.all_definitions()) {
    // Check only those definitions defined in a function
    if (const Function* func = definition.second->function()) {
      if (const BasicBlock* block = definition.second->block()) {
        if (!block->reachable()) continue;
        // If the Id is defined within a block then make sure all references to
        // that Id appear in a blocks that are dominated by the defining block
        for (auto& use_index_pair : definition.second->uses()) {
          const Instruction* use = use_index_pair.first;
          if (const BasicBlock* use_block = use->block()) {
            if (use_block->reachable() == false) continue;
            if (use->opcode() == SpvOpPhi) {
              phi_instructions.insert(use);
            } else if (!block->dominates(*use->block())) {
              return _.diag(SPV_ERROR_INVALID_ID, use_block->label())
                     << "ID " << _.getIdName(definition.first)
                     << " defined in block " << _.getIdName(block->id())
                     << " does not dominate its use in block "
                     << _.getIdName(use_block->id());
            }
          }
        }
      } else {
        // If the Ids defined within a function but not in a block(i.e. function
        // parameters, block ids), then make sure all references to that Id
        // appear within the same function
        for (auto use : definition.second->uses()) {
          const Instruction* inst = use.first;
          if (inst->function() && inst->function() != func) {
            return _.diag(SPV_ERROR_INVALID_ID, _.FindDef(func->id()))
                   << "ID " << _.getIdName(definition.first)
                   << " used in function "
                   << _.getIdName(inst->function()->id())
                   << " is used outside of it's defining function "
                   << _.getIdName(func->id());
          }
        }
      }
    }
    // NOTE: Ids defined outside of functions must appear before they are used
    // This check is being performed in the IdPass function
  }

  // Check all OpPhi parent blocks are dominated by the variable's defining
  // blocks
  for (const Instruction* phi : phi_instructions) {
    if (phi->block()->reachable() == false) continue;
    for (size_t i = 3; i < phi->operands().size(); i += 2) {
      const Instruction* variable = _.FindDef(phi->word(i));
      const BasicBlock* parent =
          phi->function()->GetBlock(phi->word(i + 1)).first;
      if (variable->block() && parent->reachable() &&
          !variable->block()->dominates(*parent)) {
        return _.diag(SPV_ERROR_INVALID_ID, phi)
               << "In OpPhi instruction " << _.getIdName(phi->id()) << ", ID "
               << _.getIdName(variable->id())
               << " definition does not dominate its parent "
               << _.getIdName(parent->id());
      }
    }
  }

  return SPV_SUCCESS;
}

// Performs SSA validation on the IDs of an instruction. The
// can_have_forward_declared_ids  functor should return true if the
// instruction operand's ID can be forward referenced.
spv_result_t IdPass(ValidationState_t& _, Instruction* inst) {
  auto can_have_forward_declared_ids =
      spvOperandCanBeForwardDeclaredFunction(inst->opcode());

  // Keep track of a result id defined by this instruction.  0 means it
  // does not define an id.
  uint32_t result_id = 0;

  for (unsigned i = 0; i < inst->operands().size(); i++) {
    const spv_parsed_operand_t& operand = inst->operand(i);
    const spv_operand_type_t& type = operand.type;
    // We only care about Id operands, which are a single word.
    const uint32_t operand_word = inst->word(operand.offset);

    auto ret = SPV_ERROR_INTERNAL;
    switch (type) {
      case SPV_OPERAND_TYPE_RESULT_ID:
        // NOTE: Multiple Id definitions are being checked by the binary parser.
        //
        // Defer undefined-forward-reference removal until after we've analyzed
        // the remaining operands to this instruction.  Deferral only matters
        // for OpPhi since it's the only case where it defines its own forward
        // reference.  Other instructions that can have forward references
        // either don't define a value or the forward reference is to a function
        // Id (and hence defined outside of a function body).
        result_id = operand_word;
        // NOTE: The result Id is added (in RegisterInstruction) *after* all of
        // the other Ids have been checked to avoid premature use in the same
        // instruction.
        ret = SPV_SUCCESS;
        break;
      case SPV_OPERAND_TYPE_ID:
      case SPV_OPERAND_TYPE_TYPE_ID:
      case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID:
      case SPV_OPERAND_TYPE_SCOPE_ID:
        if (_.IsDefinedId(operand_word)) {
          ret = SPV_SUCCESS;
        } else if (can_have_forward_declared_ids(i)) {
          ret = _.ForwardDeclareId(operand_word);
        } else {
          ret = _.diag(SPV_ERROR_INVALID_ID, inst)
                << "ID " << _.getIdName(operand_word)
                << " has not been defined";
        }
        break;
      default:
        ret = SPV_SUCCESS;
        break;
    }
    if (SPV_SUCCESS != ret) return ret;
  }
  if (result_id) _.RemoveIfForwardDeclared(result_id);

  return SPV_SUCCESS;
}

spv_result_t spvValidateInstructionIDs(const spv_instruction_t* pInsts,
                                       const uint64_t instCount,
                                       const ValidationState_t& state,
                                       spv_position position) {
  idUsage idUsage(state.context(), pInsts, instCount, state.memory_model(),
                  state.addressing_model(), state, state.entry_points(),
                  position, state.context()->consumer);
  for (uint64_t instIndex = 0; instIndex < instCount; ++instIndex) {
    if (!idUsage.isValid(&pInsts[instIndex])) return SPV_ERROR_INVALID_ID;
  }
  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
