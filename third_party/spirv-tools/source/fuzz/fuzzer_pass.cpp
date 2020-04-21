// Copyright (c) 2019 Google LLC
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

#include "source/fuzz/fuzzer_pass.h"

#include "source/fuzz/instruction_descriptor.h"
#include "source/fuzz/transformation_add_constant_scalar.h"
#include "source/fuzz/transformation_add_global_undef.h"
#include "source/fuzz/transformation_add_type_boolean.h"
#include "source/fuzz/transformation_add_type_float.h"
#include "source/fuzz/transformation_add_type_int.h"
#include "source/fuzz/transformation_add_type_matrix.h"
#include "source/fuzz/transformation_add_type_pointer.h"
#include "source/fuzz/transformation_add_type_vector.h"

namespace spvtools {
namespace fuzz {

FuzzerPass::FuzzerPass(opt::IRContext* ir_context, FactManager* fact_manager,
                       FuzzerContext* fuzzer_context,
                       protobufs::TransformationSequence* transformations)
    : ir_context_(ir_context),
      fact_manager_(fact_manager),
      fuzzer_context_(fuzzer_context),
      transformations_(transformations) {}

FuzzerPass::~FuzzerPass() = default;

std::vector<opt::Instruction*> FuzzerPass::FindAvailableInstructions(
    const opt::Function& function, opt::BasicBlock* block,
    opt::BasicBlock::iterator inst_it,
    std::function<bool(opt::IRContext*, opt::Instruction*)>
        instruction_is_relevant) {
  // TODO(afd) The following is (relatively) simple, but may end up being
  //  prohibitively inefficient, as it walks the whole dominator tree for
  //  every instruction that is considered.

  std::vector<opt::Instruction*> result;
  // Consider all global declarations
  for (auto& global : GetIRContext()->module()->types_values()) {
    if (instruction_is_relevant(GetIRContext(), &global)) {
      result.push_back(&global);
    }
  }

  // Consider all previous instructions in this block
  for (auto prev_inst_it = block->begin(); prev_inst_it != inst_it;
       ++prev_inst_it) {
    if (instruction_is_relevant(GetIRContext(), &*prev_inst_it)) {
      result.push_back(&*prev_inst_it);
    }
  }

  // Walk the dominator tree to consider all instructions from dominating
  // blocks
  auto dominator_analysis = GetIRContext()->GetDominatorAnalysis(&function);
  for (auto next_dominator = dominator_analysis->ImmediateDominator(block);
       next_dominator != nullptr;
       next_dominator =
           dominator_analysis->ImmediateDominator(next_dominator)) {
    for (auto& dominating_inst : *next_dominator) {
      if (instruction_is_relevant(GetIRContext(), &dominating_inst)) {
        result.push_back(&dominating_inst);
      }
    }
  }
  return result;
}

void FuzzerPass::MaybeAddTransformationBeforeEachInstruction(
    std::function<
        void(const opt::Function& function, opt::BasicBlock* block,
             opt::BasicBlock::iterator inst_it,
             const protobufs::InstructionDescriptor& instruction_descriptor)>
        maybe_apply_transformation) {
  // Consider every block in every function.
  for (auto& function : *GetIRContext()->module()) {
    for (auto& block : function) {
      // We now consider every instruction in the block, randomly deciding
      // whether to apply a transformation before it.

      // In order for transformations to insert new instructions, they need to
      // be able to identify the instruction to insert before.  We describe an
      // instruction via its opcode, 'opc', a base instruction 'base' that has a
      // result id, and the number of instructions with opcode 'opc' that we
      // should skip when searching from 'base' for the desired instruction.
      // (An instruction that has a result id is represented by its own opcode,
      // itself as 'base', and a skip-count of 0.)
      std::vector<std::tuple<uint32_t, SpvOp, uint32_t>>
          base_opcode_skip_triples;

      // The initial base instruction is the block label.
      uint32_t base = block.id();

      // Counts the number of times we have seen each opcode since we reset the
      // base instruction.
      std::map<SpvOp, uint32_t> skip_count;

      // Consider every instruction in the block.  The label is excluded: it is
      // only necessary to consider it as a base in case the first instruction
      // in the block does not have a result id.
      for (auto inst_it = block.begin(); inst_it != block.end(); ++inst_it) {
        if (inst_it->HasResultId()) {
          // In the case that the instruction has a result id, we use the
          // instruction as its own base, and clear the skip counts we have
          // collected.
          base = inst_it->result_id();
          skip_count.clear();
        }
        const SpvOp opcode = inst_it->opcode();

        // Invoke the provided function, which might apply a transformation.
        maybe_apply_transformation(
            function, &block, inst_it,
            MakeInstructionDescriptor(
                base, opcode,
                skip_count.count(opcode) ? skip_count.at(opcode) : 0));

        if (!inst_it->HasResultId()) {
          skip_count[opcode] =
              skip_count.count(opcode) ? skip_count.at(opcode) + 1 : 1;
        }
      }
    }
  }
}

uint32_t FuzzerPass::FindOrCreateBoolType() {
  opt::analysis::Bool bool_type;
  auto existing_id = GetIRContext()->get_type_mgr()->GetId(&bool_type);
  if (existing_id) {
    return existing_id;
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(TransformationAddTypeBoolean(result));
  return result;
}

uint32_t FuzzerPass::FindOrCreate32BitIntegerType(bool is_signed) {
  opt::analysis::Integer int_type(32, is_signed);
  auto existing_id = GetIRContext()->get_type_mgr()->GetId(&int_type);
  if (existing_id) {
    return existing_id;
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(TransformationAddTypeInt(result, 32, is_signed));
  return result;
}

uint32_t FuzzerPass::FindOrCreate32BitFloatType() {
  opt::analysis::Float float_type(32);
  auto existing_id = GetIRContext()->get_type_mgr()->GetId(&float_type);
  if (existing_id) {
    return existing_id;
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(TransformationAddTypeFloat(result, 32));
  return result;
}

uint32_t FuzzerPass::FindOrCreateVectorType(uint32_t component_type_id,
                                            uint32_t component_count) {
  assert(component_count >= 2 && component_count <= 4 &&
         "Precondition: component count must be in range [2, 4].");
  opt::analysis::Type* component_type =
      GetIRContext()->get_type_mgr()->GetType(component_type_id);
  assert(component_type && "Precondition: the component type must exist.");
  opt::analysis::Vector vector_type(component_type, component_count);
  auto existing_id = GetIRContext()->get_type_mgr()->GetId(&vector_type);
  if (existing_id) {
    return existing_id;
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(
      TransformationAddTypeVector(result, component_type_id, component_count));
  return result;
}

uint32_t FuzzerPass::FindOrCreateMatrixType(uint32_t column_count,
                                            uint32_t row_count) {
  assert(column_count >= 2 && column_count <= 4 &&
         "Precondition: column count must be in range [2, 4].");
  assert(row_count >= 2 && row_count <= 4 &&
         "Precondition: row count must be in range [2, 4].");
  uint32_t column_type_id =
      FindOrCreateVectorType(FindOrCreate32BitFloatType(), row_count);
  opt::analysis::Type* column_type =
      GetIRContext()->get_type_mgr()->GetType(column_type_id);
  opt::analysis::Matrix matrix_type(column_type, column_count);
  auto existing_id = GetIRContext()->get_type_mgr()->GetId(&matrix_type);
  if (existing_id) {
    return existing_id;
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(
      TransformationAddTypeMatrix(result, column_type_id, column_count));
  return result;
}

uint32_t FuzzerPass::FindOrCreatePointerTo32BitIntegerType(
    bool is_signed, SpvStorageClass storage_class) {
  auto uint32_type_id = FindOrCreate32BitIntegerType(is_signed);
  opt::analysis::Pointer pointer_type(
      GetIRContext()->get_type_mgr()->GetType(uint32_type_id), storage_class);
  auto existing_id = GetIRContext()->get_type_mgr()->GetId(&pointer_type);
  if (existing_id) {
    return existing_id;
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(
      TransformationAddTypePointer(result, storage_class, uint32_type_id));
  return result;
}

uint32_t FuzzerPass::FindOrCreate32BitIntegerConstant(uint32_t word,
                                                      bool is_signed) {
  auto uint32_type_id = FindOrCreate32BitIntegerType(is_signed);
  opt::analysis::IntConstant int_constant(
      GetIRContext()->get_type_mgr()->GetType(uint32_type_id)->AsInteger(),
      {word});
  auto existing_constant =
      GetIRContext()->get_constant_mgr()->FindConstant(&int_constant);
  if (existing_constant) {
    return GetIRContext()
        ->get_constant_mgr()
        ->GetDefiningInstruction(existing_constant)
        ->result_id();
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(
      TransformationAddConstantScalar(result, uint32_type_id, {word}));
  return result;
}

uint32_t FuzzerPass::FindOrCreateGlobalUndef(uint32_t type_id) {
  for (auto& inst : GetIRContext()->types_values()) {
    if (inst.opcode() == SpvOpUndef && inst.type_id() == type_id) {
      return inst.result_id();
    }
  }
  auto result = GetFuzzerContext()->GetFreshId();
  ApplyTransformation(TransformationAddGlobalUndef(result, type_id));
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
