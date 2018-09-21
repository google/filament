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

bool idUsage::isValid(const spv_instruction_t* inst) {
  spv_opcode_desc opcodeEntry = nullptr;
  if (spvOpcodeTableValueLookup(targetEnv, opcodeTable, inst->opcode,
                                &opcodeEntry))
    return false;
#define CASE(OpCode) \
  case Spv##OpCode:  \
    return isValid<Spv##OpCode>(inst, opcodeEntry);
  switch (inst->opcode) {
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
  std::vector<const Instruction*> phi_instructions;
  std::unordered_set<uint32_t> phi_ids;
  for (const auto& inst : _.ordered_instructions()) {
    if (inst.id() == 0) continue;
    if (const Function* func = inst.function()) {
      if (const BasicBlock* block = inst.block()) {
        if (!block->reachable()) continue;
        // If the Id is defined within a block then make sure all references to
        // that Id appear in a blocks that are dominated by the defining block
        for (auto& use_index_pair : inst.uses()) {
          const Instruction* use = use_index_pair.first;
          if (const BasicBlock* use_block = use->block()) {
            if (use_block->reachable() == false) continue;
            if (use->opcode() == SpvOpPhi) {
              if (phi_ids.insert(use->id()).second) {
                phi_instructions.push_back(use);
              }
            } else if (!block->dominates(*use->block())) {
              return _.diag(SPV_ERROR_INVALID_ID, use_block->label())
                     << "ID " << _.getIdName(inst.id()) << " defined in block "
                     << _.getIdName(block->id())
                     << " does not dominate its use in block "
                     << _.getIdName(use_block->id());
            }
          }
        }
      } else {
        // If the Ids defined within a function but not in a block(i.e. function
        // parameters, block ids), then make sure all references to that Id
        // appear within the same function
        for (auto use : inst.uses()) {
          const Instruction* user = use.first;
          if (user->function() && user->function() != func) {
            return _.diag(SPV_ERROR_INVALID_ID, _.FindDef(func->id()))
                   << "ID " << _.getIdName(inst.id()) << " used in function "
                   << _.getIdName(user->function()->id())
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
