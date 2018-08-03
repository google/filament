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
#include <unordered_set>
#include <utility>
#include <vector>

#include "source/diagnostic.h"
#include "source/instruction.h"
#include "source/message.h"
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
bool idUsage::isValid<SpvOpMemberName>(const spv_instruction_t* inst,
                                       const spv_opcode_desc) {
  auto typeIndex = 1;
  auto type = module_.FindDef(inst->words[typeIndex]);
  if (!type || SpvOpTypeStruct != type->opcode()) {
    DIAG(type) << "OpMemberName Type <id> '"
               << module_.getIdName(inst->words[typeIndex])
               << "' is not a struct type.";
    return false;
  }
  auto memberIndex = 2;
  auto member = inst->words[memberIndex];
  auto memberCount = (uint32_t)(type->words().size() - 2);
  if (memberCount <= member) {
    DIAG(module_.FindDef(member))
        << "OpMemberName Member <id> '"
        << module_.getIdName(inst->words[memberIndex])
        << "' index is larger than Type <id> '" << module_.getIdName(type->id())
        << "'s member count.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpLine>(const spv_instruction_t* inst,
                                 const spv_opcode_desc) {
  auto fileIndex = 1;
  auto file = module_.FindDef(inst->words[fileIndex]);
  if (!file || SpvOpString != file->opcode()) {
    DIAG(file) << "OpLine Target <id> '"
               << module_.getIdName(inst->words[fileIndex])
               << "' is not an OpString.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpDecorate>(const spv_instruction_t* inst,
                                     const spv_opcode_desc) {
  auto decorationIndex = 2;
  auto decoration = inst->words[decorationIndex];
  if (decoration == SpvDecorationSpecId) {
    auto targetIndex = 1;
    auto target = module_.FindDef(inst->words[targetIndex]);
    if (!target || !spvOpcodeIsScalarSpecConstant(target->opcode())) {
      DIAG(target) << "OpDecorate SpectId decoration target <id> '"
                   << module_.getIdName(inst->words[decorationIndex])
                   << "' is not a scalar specialization constant.";
      return false;
    }
  }
  // TODO: Add validations for all decorations.
  return true;
}

template <>
bool idUsage::isValid<SpvOpMemberDecorate>(const spv_instruction_t* inst,
                                           const spv_opcode_desc) {
  auto structTypeIndex = 1;
  auto structType = module_.FindDef(inst->words[structTypeIndex]);
  if (!structType || SpvOpTypeStruct != structType->opcode()) {
    DIAG(structType) << "OpMemberDecorate Structure type <id> '"
                     << module_.getIdName(inst->words[structTypeIndex])
                     << "' is not a struct type.";
    return false;
  }
  auto memberIndex = 2;
  auto member = inst->words[memberIndex];
  auto memberCount = static_cast<uint32_t>(structType->words().size() - 2);
  if (memberCount < member) {
    DIAG(structType) << "Index " << member
                     << " provided in OpMemberDecorate for struct <id> "
                     << module_.getIdName(inst->words[structTypeIndex])
                     << " is out of bounds. The structure has " << memberCount
                     << " members. Largest valid index is " << memberCount - 1
                     << ".";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpDecorationGroup>(const spv_instruction_t* inst,
                                            const spv_opcode_desc) {
  auto decorationGroupIndex = 1;
  auto decorationGroup = module_.FindDef(inst->words[decorationGroupIndex]);

  for (auto pair : decorationGroup->uses()) {
    auto use = pair.first;
    if (use->opcode() != SpvOpDecorate && use->opcode() != SpvOpGroupDecorate &&
        use->opcode() != SpvOpGroupMemberDecorate &&
        use->opcode() != SpvOpName) {
      DIAG(decorationGroup) << "Result id of OpDecorationGroup can only "
                            << "be targeted by OpName, OpGroupDecorate, "
                            << "OpDecorate, and OpGroupMemberDecorate";
      return false;
    }
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpGroupDecorate>(const spv_instruction_t* inst,
                                          const spv_opcode_desc) {
  auto decorationGroupIndex = 1;
  auto decorationGroup = module_.FindDef(inst->words[decorationGroupIndex]);
  if (!decorationGroup || SpvOpDecorationGroup != decorationGroup->opcode()) {
    DIAG(decorationGroup) << "OpGroupDecorate Decoration group <id> '"
                          << module_.getIdName(
                                 inst->words[decorationGroupIndex])
                          << "' is not a decoration group.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpGroupMemberDecorate>(const spv_instruction_t* inst,
                                                const spv_opcode_desc) {
  auto decorationGroupIndex = 1;
  auto decorationGroup = module_.FindDef(inst->words[decorationGroupIndex]);
  if (!decorationGroup || SpvOpDecorationGroup != decorationGroup->opcode()) {
    DIAG(decorationGroup) << "OpGroupMemberDecorate Decoration group <id> '"
                          << module_.getIdName(
                                 inst->words[decorationGroupIndex])
                          << "' is not a decoration group.";
    return false;
  }
  // Grammar checks ensures that the number of arguments to this instruction
  // is an odd number: 1 decoration group + (id,literal) pairs.
  for (size_t i = 2; i + 1 < inst->words.size(); i = i + 2) {
    const uint32_t struct_id = inst->words[i];
    const uint32_t index = inst->words[i + 1];
    auto struct_instr = module_.FindDef(struct_id);
    if (!struct_instr || SpvOpTypeStruct != struct_instr->opcode()) {
      DIAG(struct_instr) << "OpGroupMemberDecorate Structure type <id> '"
                         << module_.getIdName(struct_id)
                         << "' is not a struct type.";
      return false;
    }
    const uint32_t num_struct_members =
        static_cast<uint32_t>(struct_instr->words().size() - 2);
    if (index >= num_struct_members) {
      DIAG(struct_instr)
          << "Index " << index
          << " provided in OpGroupMemberDecorate for struct <id> "
          << module_.getIdName(struct_id)
          << " is out of bounds. The structure has " << num_struct_members
          << " members. Largest valid index is " << num_struct_members - 1
          << ".";
      return false;
    }
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpEntryPoint>(const spv_instruction_t* inst,
                                       const spv_opcode_desc) {
  auto entryPointIndex = 2;
  auto entryPoint = module_.FindDef(inst->words[entryPointIndex]);
  if (!entryPoint || SpvOpFunction != entryPoint->opcode()) {
    DIAG(entryPoint) << "OpEntryPoint Entry Point <id> '"
                     << module_.getIdName(inst->words[entryPointIndex])
                     << "' is not a function.";
    return false;
  }
  // don't check kernel function signatures
  const SpvExecutionModel executionModel = SpvExecutionModel(inst->words[1]);
  if (executionModel != SpvExecutionModelKernel) {
    // TODO: Check the entry point signature is void main(void), may be subject
    // to change
    auto entryPointType = module_.FindDef(entryPoint->words()[4]);
    if (!entryPointType || 3 != entryPointType->words().size()) {
      DIAG(entryPoint) << "OpEntryPoint Entry Point <id> '"
                       << module_.getIdName(inst->words[entryPointIndex])
                       << "'s function parameter count is not zero.";
      return false;
    }
  }

  auto returnType = module_.FindDef(entryPoint->type_id());
  if (!returnType || SpvOpTypeVoid != returnType->opcode()) {
    DIAG(entryPoint) << "OpEntryPoint Entry Point <id> '"
                     << module_.getIdName(inst->words[entryPointIndex])
                     << "'s function return type is not void.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpExecutionMode>(const spv_instruction_t* inst,
                                          const spv_opcode_desc) {
  auto entryPointIndex = 1;
  auto entryPointID = inst->words[entryPointIndex];
  auto found =
      std::find(entry_points_.cbegin(), entry_points_.cend(), entryPointID);
  if (found == entry_points_.cend()) {
    DIAG(module_.FindDef(entryPointID))
        << "OpExecutionMode Entry Point <id> '"
        << module_.getIdName(inst->words[entryPointIndex])
        << "' is not the Entry Point "
           "operand of an OpEntryPoint.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypeVector>(const spv_instruction_t* inst,
                                       const spv_opcode_desc) {
  auto componentIndex = 2;
  auto componentType = module_.FindDef(inst->words[componentIndex]);
  if (!componentType || !spvOpcodeIsScalarType(componentType->opcode())) {
    DIAG(componentType) << "OpTypeVector Component Type <id> '"
                        << module_.getIdName(inst->words[componentIndex])
                        << "' is not a scalar type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypeMatrix>(const spv_instruction_t* inst,
                                       const spv_opcode_desc) {
  auto columnTypeIndex = 2;
  auto columnType = module_.FindDef(inst->words[columnTypeIndex]);
  if (!columnType || SpvOpTypeVector != columnType->opcode()) {
    DIAG(columnType) << "OpTypeMatrix Column Type <id> '"
                     << module_.getIdName(inst->words[columnTypeIndex])
                     << "' is not a vector.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypeSampler>(const spv_instruction_t*,
                                        const spv_opcode_desc) {
  // OpTypeSampler takes no arguments in Rev31 and beyond.
  return true;
}

// True if the integer constant is > 0. constWords are words of the
// constant-defining instruction (either OpConstant or
// OpSpecConstant). typeWords are the words of the constant's-type-defining
// OpTypeInt.
bool aboveZero(const std::vector<uint32_t>& constWords,
               const std::vector<uint32_t>& typeWords) {
  const uint32_t width = typeWords[2];
  const bool is_signed = typeWords[3] > 0;
  const uint32_t loWord = constWords[3];
  if (width > 32) {
    // The spec currently doesn't allow integers wider than 64 bits.
    const uint32_t hiWord = constWords[4];  // Must exist, per spec.
    if (is_signed && (hiWord >> 31)) return false;
    return (loWord | hiWord) > 0;
  } else {
    if (is_signed && (loWord >> 31)) return false;
    return loWord > 0;
  }
}

template <>
bool idUsage::isValid<SpvOpTypeArray>(const spv_instruction_t* inst,
                                      const spv_opcode_desc) {
  auto elementTypeIndex = 2;
  auto elementType = module_.FindDef(inst->words[elementTypeIndex]);
  if (!elementType || !spvOpcodeGeneratesType(elementType->opcode())) {
    DIAG(elementType) << "OpTypeArray Element Type <id> '"
                      << module_.getIdName(inst->words[elementTypeIndex])
                      << "' is not a type.";
    return false;
  }
  auto lengthIndex = 3;
  auto length = module_.FindDef(inst->words[lengthIndex]);
  if (!length || !spvOpcodeIsConstant(length->opcode())) {
    DIAG(length) << "OpTypeArray Length <id> '"
                 << module_.getIdName(inst->words[lengthIndex])
                 << "' is not a scalar constant type.";
    return false;
  }

  // NOTE: Check the initialiser value of the constant
  auto constInst = length->words();
  auto constResultTypeIndex = 1;
  auto constResultType = module_.FindDef(constInst[constResultTypeIndex]);
  if (!constResultType || SpvOpTypeInt != constResultType->opcode()) {
    DIAG(length) << "OpTypeArray Length <id> '"
                 << module_.getIdName(inst->words[lengthIndex])
                 << "' is not a constant integer type.";
    return false;
  }

  switch (length->opcode()) {
    case SpvOpSpecConstant:
    case SpvOpConstant:
      if (aboveZero(length->words(), constResultType->words())) break;
    // Else fall through!
    case SpvOpConstantNull: {
      DIAG(length) << "OpTypeArray Length <id> '"
                   << module_.getIdName(inst->words[lengthIndex])
                   << "' default value must be at least 1.";
      return false;
    }
    case SpvOpSpecConstantOp:
      // Assume it's OK, rather than try to evaluate the operation.
      break;
    default:
      assert(0 && "bug in spvOpcodeIsConstant() or result type isn't int");
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypeRuntimeArray>(const spv_instruction_t* inst,
                                             const spv_opcode_desc) {
  auto elementTypeIndex = 2;
  auto elementType = module_.FindDef(inst->words[elementTypeIndex]);
  if (!elementType || !spvOpcodeGeneratesType(elementType->opcode())) {
    DIAG(elementType) << "OpTypeRuntimeArray Element Type <id> '"
                      << module_.getIdName(inst->words[elementTypeIndex])
                      << "' is not a type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypeStruct>(const spv_instruction_t* inst,
                                       const spv_opcode_desc) {
  ValidationState_t& vstate = const_cast<ValidationState_t&>(module_);
  const uint32_t struct_id = inst->words[1];
  auto structType = module_.FindDef(struct_id);
  for (size_t memberTypeIndex = 2; memberTypeIndex < inst->words.size();
       ++memberTypeIndex) {
    auto memberTypeId = inst->words[memberTypeIndex];
    auto memberType = module_.FindDef(memberTypeId);
    if (!memberType || !spvOpcodeGeneratesType(memberType->opcode())) {
      DIAG(memberType) << "OpTypeStruct Member Type <id> '"
                       << module_.getIdName(inst->words[memberTypeIndex])
                       << "' is not a type.";
      return false;
    }
    if (SpvOpTypeStruct == memberType->opcode() &&
        module_.IsStructTypeWithBuiltInMember(memberTypeId)) {
      DIAG(memberType)
          << "Structure <id> " << module_.getIdName(memberTypeId)
          << " contains members with BuiltIn decoration. Therefore this "
             "structure may not be contained as a member of another structure "
             "type. Structure <id> "
          << module_.getIdName(struct_id) << " contains structure <id> "
          << module_.getIdName(memberTypeId) << ".";
      return false;
    }
    if (module_.IsForwardPointer(memberTypeId)) {
      if (memberType->opcode() != SpvOpTypePointer) {
        DIAG(memberType) << "Found a forward reference to a non-pointer "
                            "type in OpTypeStruct instruction.";
        return false;
      }
      // If we're dealing with a forward pointer:
      // Find out the type that the pointer is pointing to (must be struct)
      // word 3 is the <id> of the type being pointed to.
      auto typePointingTo = module_.FindDef(memberType->words()[3]);
      if (typePointingTo && typePointingTo->opcode() != SpvOpTypeStruct) {
        // Forward declared operands of a struct may only point to a struct.
        DIAG(memberType)
            << "A forward reference operand in an OpTypeStruct must be an "
               "OpTypePointer that points to an OpTypeStruct. "
               "Found OpTypePointer that points to Op"
            << spvOpcodeString(static_cast<SpvOp>(typePointingTo->opcode()))
            << ".";
        return false;
      }
    }
  }
  std::unordered_set<uint32_t> built_in_members;
  for (auto decoration : vstate.id_decorations(struct_id)) {
    if (decoration.dec_type() == SpvDecorationBuiltIn &&
        decoration.struct_member_index() != Decoration::kInvalidMember) {
      built_in_members.insert(decoration.struct_member_index());
    }
  }
  int num_struct_members = static_cast<int>(inst->words.size() - 2);
  int num_builtin_members = static_cast<int>(built_in_members.size());
  if (num_builtin_members > 0 && num_builtin_members != num_struct_members) {
    DIAG(structType)
        << "When BuiltIn decoration is applied to a structure-type member, "
           "all members of that structure type must also be decorated with "
           "BuiltIn (No allowed mixing of built-in variables and "
           "non-built-in variables within a single structure). Structure id "
        << struct_id << " does not meet this requirement.";
    return false;
  }
  if (num_builtin_members > 0) {
    vstate.RegisterStructTypeWithBuiltInMember(struct_id);
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypePointer>(const spv_instruction_t* inst,
                                        const spv_opcode_desc) {
  auto typeIndex = 3;
  auto type = module_.FindDef(inst->words[typeIndex]);
  if (!type || !spvOpcodeGeneratesType(type->opcode())) {
    DIAG(type) << "OpTypePointer Type <id> '"
               << module_.getIdName(inst->words[typeIndex])
               << "' is not a type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypeFunction>(const spv_instruction_t* inst,
                                         const spv_opcode_desc) {
  auto returnTypeIndex = 2;
  auto returnType = module_.FindDef(inst->words[returnTypeIndex]);
  if (!returnType || !spvOpcodeGeneratesType(returnType->opcode())) {
    DIAG(returnType) << "OpTypeFunction Return Type <id> '"
                     << module_.getIdName(inst->words[returnTypeIndex])
                     << "' is not a type.";
    return false;
  }
  size_t num_args = 0;
  for (size_t paramTypeIndex = 3; paramTypeIndex < inst->words.size();
       ++paramTypeIndex, ++num_args) {
    auto paramType = module_.FindDef(inst->words[paramTypeIndex]);
    if (!paramType || !spvOpcodeGeneratesType(paramType->opcode())) {
      DIAG(paramType) << "OpTypeFunction Parameter Type <id> '"
                      << module_.getIdName(inst->words[paramTypeIndex])
                      << "' is not a type.";
      return false;
    }
  }
  const uint32_t num_function_args_limit =
      module_.options()->universal_limits_.max_function_args;
  if (num_args > num_function_args_limit) {
    DIAG(returnType) << "OpTypeFunction may not take more than "
                     << num_function_args_limit
                     << " arguments. OpTypeFunction <id> '"
                     << module_.getIdName(inst->words[1]) << "' has "
                     << num_args << " arguments.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpTypePipe>(const spv_instruction_t*,
                                     const spv_opcode_desc) {
  // OpTypePipe has no ID arguments.
  return true;
}

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

template <>
bool idUsage::isValid<SpvOpFunction>(const spv_instruction_t* inst,
                                     const spv_opcode_desc) {
  const auto* thisInst = module_.FindDef(inst->words[2u]);
  if (!thisInst) return false;

  for (uint32_t entryId : module_.FunctionEntryPoints(thisInst->id())) {
    const Function* thisFunc = module_.function(thisInst->id());
    assert(thisFunc);
    const auto* models = module_.GetExecutionModels(entryId);
    if (models) {
      assert(models->size());
      for (auto model : *models) {
        std::string reason;
        if (!thisFunc->IsCompatibleWithExecutionModel(model, &reason)) {
          DIAG(module_.FindDef(inst->words[2]))
              << "OpEntryPoint Entry Point <id> '" << module_.getIdName(entryId)
              << "'s callgraph contains function <id> "
              << module_.getIdName(thisInst->id())
              << ", which cannot be used with the current execution model:\n"
              << reason;
          return false;
        }
      }
    }
  }

  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType) return false;
  auto functionTypeIndex = 4;
  auto functionType = module_.FindDef(inst->words[functionTypeIndex]);
  if (!functionType || SpvOpTypeFunction != functionType->opcode()) {
    DIAG(functionType) << "OpFunction Function Type <id> '"
                       << module_.getIdName(inst->words[functionTypeIndex])
                       << "' is not a function type.";
    return false;
  }
  auto returnType = module_.FindDef(functionType->words()[2]);
  assert(returnType);
  if (returnType->id() != resultType->id()) {
    DIAG(resultType) << "OpFunction Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' does not match the Function Type <id> '"
                     << module_.getIdName(resultType->id())
                     << "'s return type.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpFunctionParameter>(const spv_instruction_t* inst,
                                              const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType) return false;
  // NOTE: Find OpFunction & ensure OpFunctionParameter is not out of place.
  size_t paramIndex = 0;
  assert(firstInst < inst && "Invalid instruction pointer");
  while (firstInst != --inst) {
    if (SpvOpFunction == inst->opcode) {
      break;
    } else if (SpvOpFunctionParameter == inst->opcode) {
      paramIndex++;
    }
  }
  auto functionType = module_.FindDef(inst->words[4]);
  assert(functionType);
  if (paramIndex >= functionType->words().size() - 3) {
    DIAG(module_.FindDef(inst->words[0]))
        << "Too many OpFunctionParameters for " << inst->words[2]
        << ": expected " << functionType->words().size() - 3
        << " based on the function's type";
    return false;
  }
  auto paramType = module_.FindDef(functionType->words()[paramIndex + 3]);
  assert(paramType);
  if (resultType->id() != paramType->id()) {
    DIAG(resultType) << "OpFunctionParameter Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "' does not match the OpTypeFunction parameter "
                        "type of the same index.";
    return false;
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpFunctionCall>(const spv_instruction_t* inst,
                                         const spv_opcode_desc) {
  auto resultTypeIndex = 1;
  auto resultType = module_.FindDef(inst->words[resultTypeIndex]);
  if (!resultType) return false;
  auto functionIndex = 3;
  auto function = module_.FindDef(inst->words[functionIndex]);
  if (!function || SpvOpFunction != function->opcode()) {
    DIAG(function) << "OpFunctionCall Function <id> '"
                   << module_.getIdName(inst->words[functionIndex])
                   << "' is not a function.";
    return false;
  }
  auto returnType = module_.FindDef(function->type_id());
  assert(returnType);
  if (returnType->id() != resultType->id()) {
    DIAG(resultType) << "OpFunctionCall Result Type <id> '"
                     << module_.getIdName(inst->words[resultTypeIndex])
                     << "'s type does not match Function <id> '"
                     << module_.getIdName(returnType->id())
                     << "'s return type.";
    return false;
  }
  auto functionType = module_.FindDef(function->words()[4]);
  assert(functionType);
  auto functionCallArgCount = inst->words.size() - 4;
  auto functionParamCount = functionType->words().size() - 3;
  if (functionParamCount != functionCallArgCount) {
    DIAG(module_.FindDef(inst->words.back()))
        << "OpFunctionCall Function <id>'s parameter count does not match "
           "the argument count.";
    return false;
  }
  for (size_t argumentIndex = 4, paramIndex = 3;
       argumentIndex < inst->words.size(); argumentIndex++, paramIndex++) {
    auto argument = module_.FindDef(inst->words[argumentIndex]);
    if (!argument) return false;
    auto argumentType = module_.FindDef(argument->type_id());
    assert(argumentType);
    auto parameterType = module_.FindDef(functionType->words()[paramIndex]);
    assert(parameterType);
    if (argumentType->id() != parameterType->id()) {
      DIAG(argument) << "OpFunctionCall Argument <id> '"
                     << module_.getIdName(inst->words[argumentIndex])
                     << "'s type does not match Function <id> '"
                     << module_.getIdName(parameterType->id())
                     << "'s parameter type.";
      return false;
    }
  }
  return true;
}

template <>
bool idUsage::isValid<SpvOpPhi>(const spv_instruction_t* inst,
                                const spv_opcode_desc /*opcodeEntry*/) {
  auto thisInst = module_.FindDef(inst->words[2]);
  SpvOp typeOp = module_.GetIdOpcode(thisInst->type_id());
  if (!spvOpcodeGeneratesType(typeOp)) {
    DIAG(thisInst) << "OpPhi's type <id> "
                   << module_.getIdName(thisInst->type_id())
                   << " is not a type instruction.";
    return false;
  }

  auto block = thisInst->block();
  size_t numInOps = inst->words.size() - 3;
  if (numInOps % 2 != 0) {
    DIAG(thisInst)
        << "OpPhi does not have an equal number of incoming values and "
           "basic blocks.";
    return false;
  }

  // Create a uniqued vector of predecessor ids for comparison against
  // incoming values. OpBranchConditional %cond %label %label produces two
  // predecessors in the CFG.
  std::vector<uint32_t> predIds;
  std::transform(block->predecessors()->begin(), block->predecessors()->end(),
                 std::back_inserter(predIds),
                 [](const BasicBlock* b) { return b->id(); });
  std::sort(predIds.begin(), predIds.end());
  predIds.erase(std::unique(predIds.begin(), predIds.end()), predIds.end());

  size_t numEdges = numInOps / 2;
  if (numEdges != predIds.size()) {
    DIAG(thisInst) << "OpPhi's number of incoming blocks (" << numEdges
                   << ") does not match block's predecessor count ("
                   << block->predecessors()->size() << ").";
    return false;
  }

  for (size_t i = 3; i < inst->words.size(); ++i) {
    auto incId = inst->words[i];
    if (i % 2 == 1) {
      // Incoming value type must match the phi result type.
      auto incTypeId = module_.GetTypeId(incId);
      if (thisInst->type_id() != incTypeId) {
        DIAG(thisInst) << "OpPhi's result type <id> "
                       << module_.getIdName(thisInst->type_id())
                       << " does not match incoming value <id> "
                       << module_.getIdName(incId) << " type <id> "
                       << module_.getIdName(incTypeId) << ".";
        return false;
      }
    } else {
      if (module_.GetIdOpcode(incId) != SpvOpLabel) {
        DIAG(thisInst) << "OpPhi's incoming basic block <id> "
                       << module_.getIdName(incId) << " is not an OpLabel.";
        return false;
      }

      // Incoming basic block must be an immediate predecessor of the phi's
      // block.
      if (!std::binary_search(predIds.begin(), predIds.end(), incId)) {
        DIAG(thisInst) << "OpPhi's incoming basic block <id> "
                       << module_.getIdName(incId)
                       << " is not a predecessor of <id> "
                       << module_.getIdName(block->id()) << ".";
        return false;
      }
    }
  }

  return true;
}

template <>
bool idUsage::isValid<SpvOpBranchConditional>(const spv_instruction_t* inst,
                                              const spv_opcode_desc) {
  const size_t numOperands = inst->words.size() - 1;
  const size_t condOperandIndex = 1;
  const size_t targetTrueIndex = 2;
  const size_t targetFalseIndex = 3;

  // num_operands is either 3 or 5 --- if 5, the last two need to be literal
  // integers
  if (numOperands != 3 && numOperands != 5) {
    Instruction* fake_inst = nullptr;
    DIAG(fake_inst) << "OpBranchConditional requires either 3 or 5 parameters";
    return false;
  }

  bool ret = true;

  // grab the condition operand and check that it is a bool
  const auto condOp = module_.FindDef(inst->words[condOperandIndex]);
  if (!condOp || !module_.IsBoolScalarType(condOp->type_id())) {
    DIAG(condOp)
        << "Condition operand for OpBranchConditional must be of boolean type";
    ret = false;
  }

  // target operands must be OpLabel
  // note that we don't need to check that the target labels are in the same
  // function,
  // PerformCfgChecks already checks for that
  const auto targetOpTrue = module_.FindDef(inst->words[targetTrueIndex]);
  if (!targetOpTrue || SpvOpLabel != targetOpTrue->opcode()) {
    DIAG(targetOpTrue)
        << "The 'True Label' operand for OpBranchConditional must be the "
           "ID of an OpLabel instruction";
    ret = false;
  }

  const auto targetOpFalse = module_.FindDef(inst->words[targetFalseIndex]);
  if (!targetOpFalse || SpvOpLabel != targetOpFalse->opcode()) {
    DIAG(targetOpFalse)
        << "The 'False Label' operand for OpBranchConditional must be the "
           "ID of an OpLabel instruction";
    ret = false;
  }

  return ret;
}

template <>
bool idUsage::isValid<SpvOpReturnValue>(const spv_instruction_t* inst,
                                        const spv_opcode_desc) {
  auto valueIndex = 1;
  auto value = module_.FindDef(inst->words[valueIndex]);
  if (!value || !value->type_id()) {
    DIAG(value) << "OpReturnValue Value <id> '"
                << module_.getIdName(inst->words[valueIndex])
                << "' does not represent a value.";
    return false;
  }
  auto valueType = module_.FindDef(value->type_id());
  if (!valueType || SpvOpTypeVoid == valueType->opcode()) {
    DIAG(value) << "OpReturnValue value's type <id> '"
                << module_.getIdName(value->type_id())
                << "' is missing or void.";
    return false;
  }

  const bool uses_variable_pointer =
      module_.features().variable_pointers ||
      module_.features().variable_pointers_storage_buffer;

  if (addressingModel == SpvAddressingModelLogical &&
      SpvOpTypePointer == valueType->opcode() && !uses_variable_pointer &&
      !module_.options()->relax_logical_pointer) {
    DIAG(value)
        << "OpReturnValue value's type <id> '"
        << module_.getIdName(value->type_id())
        << "' is a pointer, which is invalid in the Logical addressing model.";
    return false;
  }

  // NOTE: Find OpFunction
  const spv_instruction_t* function = inst - 1;
  while (firstInst != function) {
    if (SpvOpFunction == function->opcode) break;
    function--;
  }
  if (SpvOpFunction != function->opcode) {
    DIAG(value) << "OpReturnValue is not in a basic block.";
    return false;
  }
  auto returnType = module_.FindDef(function->words[1]);
  if (!returnType || returnType->id() != valueType->id()) {
    DIAG(value) << "OpReturnValue Value <id> '"
                << module_.getIdName(inst->words[valueIndex])
                << "'s type does not match OpFunction's return type.";
    return false;
  }
  return true;
}

#undef DIAG

bool idUsage::isValid(const spv_instruction_t* inst) {
  spv_opcode_desc opcodeEntry = nullptr;
  if (spvOpcodeTableValueLookup(targetEnv, opcodeTable, inst->opcode,
                                &opcodeEntry))
    return false;
#define CASE(OpCode) \
  case Spv##OpCode:  \
    return isValid<Spv##OpCode>(inst, opcodeEntry);
  switch (inst->opcode) {
    CASE(OpMemberName)
    CASE(OpLine)
    CASE(OpDecorate)
    CASE(OpMemberDecorate)
    CASE(OpDecorationGroup)
    CASE(OpGroupDecorate)
    CASE(OpGroupMemberDecorate)
    CASE(OpEntryPoint)
    CASE(OpExecutionMode)
    CASE(OpTypeVector)
    CASE(OpTypeMatrix)
    CASE(OpTypeSampler)
    CASE(OpTypeArray)
    CASE(OpTypeRuntimeArray)
    CASE(OpTypeStruct)
    CASE(OpTypePointer)
    CASE(OpTypeFunction)
    CASE(OpTypePipe)
    CASE(OpConstantTrue)
    CASE(OpConstantFalse)
    CASE(OpConstantComposite)
    CASE(OpConstantSampler)
    CASE(OpConstantNull)
    CASE(OpSpecConstantTrue)
    CASE(OpSpecConstantFalse)
    CASE(OpSpecConstantComposite)
    CASE(OpSampledImage)
    CASE(OpFunction)
    CASE(OpFunctionParameter)
    CASE(OpFunctionCall)
    // Other composite opcodes are validated in validate_composites.cpp.
    // Arithmetic opcodes are validated in validate_arithmetics.cpp.
    // Bitwise opcodes are validated in validate_bitwise.cpp.
    // Logical opcodes are validated in validate_logicals.cpp.
    // Derivative opcodes are validated in validate_derivatives.cpp.
    CASE(OpPhi)
    // OpBranch is validated in validate_cfg.cpp.
    // See tests in test/val/val_cfg_test.cpp.
    CASE(OpBranchConditional)
    CASE(OpReturnValue)
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

  _.RegisterInstruction(inst);

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
