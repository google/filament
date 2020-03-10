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

#include "source/fuzz/transformation_outline_function.h"

#include <set>

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

namespace {

std::map<uint32_t, uint32_t> PairSequenceToMap(
    const google::protobuf::RepeatedPtrField<protobufs::UInt32Pair>&
        pair_sequence) {
  std::map<uint32_t, uint32_t> result;
  for (auto& pair : pair_sequence) {
    result[pair.first()] = pair.second();
  }
  return result;
}

}  // namespace

TransformationOutlineFunction::TransformationOutlineFunction(
    const spvtools::fuzz::protobufs::TransformationOutlineFunction& message)
    : message_(message) {}

TransformationOutlineFunction::TransformationOutlineFunction(
    uint32_t entry_block, uint32_t exit_block,
    uint32_t new_function_struct_return_type_id, uint32_t new_function_type_id,
    uint32_t new_function_id, uint32_t new_function_region_entry_block,
    uint32_t new_caller_result_id, uint32_t new_callee_result_id,
    std::map<uint32_t, uint32_t>&& input_id_to_fresh_id,
    std::map<uint32_t, uint32_t>&& output_id_to_fresh_id) {
  message_.set_entry_block(entry_block);
  message_.set_exit_block(exit_block);
  message_.set_new_function_struct_return_type_id(
      new_function_struct_return_type_id);
  message_.set_new_function_type_id(new_function_type_id);
  message_.set_new_function_id(new_function_id);
  message_.set_new_function_region_entry_block(new_function_region_entry_block);
  message_.set_new_caller_result_id(new_caller_result_id);
  message_.set_new_callee_result_id(new_callee_result_id);
  for (auto& entry : input_id_to_fresh_id) {
    protobufs::UInt32Pair pair;
    pair.set_first(entry.first);
    pair.set_second(entry.second);
    *message_.add_input_id_to_fresh_id() = pair;
  }
  for (auto& entry : output_id_to_fresh_id) {
    protobufs::UInt32Pair pair;
    pair.set_first(entry.first);
    pair.set_second(entry.second);
    *message_.add_output_id_to_fresh_id() = pair;
  }
}

bool TransformationOutlineFunction::IsApplicable(
    opt::IRContext* context,
    const spvtools::fuzz::FactManager& /*unused*/) const {
  std::set<uint32_t> ids_used_by_this_transformation;

  // The various new ids used by the transformation must be fresh and distinct.

  if (!CheckIdIsFreshAndNotUsedByThisTransformation(
          message_.new_function_struct_return_type_id(), context,
          &ids_used_by_this_transformation)) {
    return false;
  }

  if (!CheckIdIsFreshAndNotUsedByThisTransformation(
          message_.new_function_type_id(), context,
          &ids_used_by_this_transformation)) {
    return false;
  }

  if (!CheckIdIsFreshAndNotUsedByThisTransformation(
          message_.new_function_id(), context,
          &ids_used_by_this_transformation)) {
    return false;
  }

  if (!CheckIdIsFreshAndNotUsedByThisTransformation(
          message_.new_function_region_entry_block(), context,
          &ids_used_by_this_transformation)) {
    return false;
  }

  if (!CheckIdIsFreshAndNotUsedByThisTransformation(
          message_.new_caller_result_id(), context,
          &ids_used_by_this_transformation)) {
    return false;
  }

  if (!CheckIdIsFreshAndNotUsedByThisTransformation(
          message_.new_callee_result_id(), context,
          &ids_used_by_this_transformation)) {
    return false;
  }

  for (auto& pair : message_.input_id_to_fresh_id()) {
    if (!CheckIdIsFreshAndNotUsedByThisTransformation(
            pair.second(), context, &ids_used_by_this_transformation)) {
      return false;
    }
  }

  for (auto& pair : message_.output_id_to_fresh_id()) {
    if (!CheckIdIsFreshAndNotUsedByThisTransformation(
            pair.second(), context, &ids_used_by_this_transformation)) {
      return false;
    }
  }

  // The entry and exit block ids must indeed refer to blocks.
  for (auto block_id : {message_.entry_block(), message_.exit_block()}) {
    auto block_label = context->get_def_use_mgr()->GetDef(block_id);
    if (!block_label || block_label->opcode() != SpvOpLabel) {
      return false;
    }
  }

  auto entry_block = context->cfg()->block(message_.entry_block());
  auto exit_block = context->cfg()->block(message_.exit_block());

  // The entry block cannot start with OpVariable - this would mean that
  // outlining would remove a variable from the function containing the region
  // being outlined.
  if (entry_block->begin()->opcode() == SpvOpVariable) {
    return false;
  }

  // For simplicity, we do not allow the entry block to be a loop header.
  if (entry_block->GetLoopMergeInst()) {
    return false;
  }

  // For simplicity, we do not allow the exit block to be a merge block or
  // continue target.
  if (fuzzerutil::IsMergeOrContinue(context, exit_block->id())) {
    return false;
  }

  // The entry block cannot start with OpPhi.  This is to keep the
  // transformation logic simple.  (Another transformation to split the OpPhis
  // from a block could be applied to avoid this scenario.)
  if (entry_block->begin()->opcode() == SpvOpPhi) {
    return false;
  }

  // The block must be in the same function.
  if (entry_block->GetParent() != exit_block->GetParent()) {
    return false;
  }

  // The entry block must dominate the exit block.
  auto dominator_analysis =
      context->GetDominatorAnalysis(entry_block->GetParent());
  if (!dominator_analysis->Dominates(entry_block, exit_block)) {
    return false;
  }

  // The exit block must post-dominate the entry block.
  auto postdominator_analysis =
      context->GetPostDominatorAnalysis(entry_block->GetParent());
  if (!postdominator_analysis->Dominates(exit_block, entry_block)) {
    return false;
  }

  // Find all the blocks dominated by |message_.entry_block| and post-dominated
  // by |message_.exit_block|.
  auto region_set = GetRegionBlocks(
      context, entry_block = context->cfg()->block(message_.entry_block()),
      exit_block = context->cfg()->block(message_.exit_block()));

  // Check whether |region_set| really is a single-entry single-exit region, and
  // also check whether structured control flow constructs and their merge
  // and continue constructs are either wholly in or wholly out of the region -
  // e.g. avoid the situation where the region contains the head of a loop but
  // not the loop's continue construct.
  //
  // This is achieved by going through every block in the function that contains
  // the region.
  for (auto& block : *entry_block->GetParent()) {
    if (&block == exit_block) {
      // It is OK (and typically expected) for the exit block of the region to
      // have successors outside the region.  It is also OK for the exit block
      // to head a structured control flow construct - the block containing the
      // call to the outlined function will end up heading this construct if
      // outlining takes place.
      continue;
    }

    if (region_set.count(&block) != 0) {
      // The block is in the region and is not the region's exit block.  Let's
      // see whether all of the block's successors are in the region.  If they
      // are not, the region is not single-entry single-exit.
      bool all_successors_in_region = true;
      block.WhileEachSuccessorLabel([&all_successors_in_region, context,
                                     &region_set](uint32_t successor) -> bool {
        if (region_set.count(context->cfg()->block(successor)) == 0) {
          all_successors_in_region = false;
          return false;
        }
        return true;
      });
      if (!all_successors_in_region) {
        return false;
      }
    }

    if (auto merge = block.GetMergeInst()) {
      // The block is a loop or selection header -- the header and its
      // associated merge block had better both be in the region or both be
      // outside the region.
      auto merge_block = context->cfg()->block(merge->GetSingleWordOperand(0));
      if (region_set.count(&block) != region_set.count(merge_block)) {
        return false;
      }
    }

    if (auto loop_merge = block.GetLoopMergeInst()) {
      // Similar to the above, but for the continue target of a loop.
      auto continue_target =
          context->cfg()->block(loop_merge->GetSingleWordOperand(1));
      if (continue_target != exit_block &&
          region_set.count(&block) != region_set.count(continue_target)) {
        return false;
      }
    }
  }

  // For each region input id, i.e. every id defined outside the region but
  // used inside the region, ...
  std::map<uint32_t, uint32_t> input_id_to_fresh_id_map =
      PairSequenceToMap(message_.input_id_to_fresh_id());
  for (auto id : GetRegionInputIds(context, region_set, exit_block)) {
    // There needs to be a corresponding fresh id to be used as a function
    // parameter.
    if (input_id_to_fresh_id_map.count(id) == 0) {
      return false;
    }
    // Furthermore, if the input id has pointer type it must be an OpVariable
    // or OpFunctionParameter.
    auto input_id_inst = context->get_def_use_mgr()->GetDef(id);
    if (context->get_def_use_mgr()
            ->GetDef(input_id_inst->type_id())
            ->opcode() == SpvOpTypePointer) {
      switch (input_id_inst->opcode()) {
        case SpvOpFunctionParameter:
        case SpvOpVariable:
          // These are OK.
          break;
        default:
          // Anything else is not OK.
          return false;
      }
    }
  }

  // For each region output id -- i.e. every id defined inside the region but
  // used outside the region -- there needs to be a corresponding fresh id that
  // can hold the value for this id computed in the outlined function.
  std::map<uint32_t, uint32_t> output_id_to_fresh_id_map =
      PairSequenceToMap(message_.output_id_to_fresh_id());
  for (auto id : GetRegionOutputIds(context, region_set, exit_block)) {
    if (output_id_to_fresh_id_map.count(id) == 0) {
      return false;
    }
  }

  return true;
}

void TransformationOutlineFunction::Apply(
    opt::IRContext* context, spvtools::fuzz::FactManager* fact_manager) const {
  // The entry block for the region before outlining.
  auto original_region_entry_block =
      context->cfg()->block(message_.entry_block());

  // The exit block for the region before outlining.
  auto original_region_exit_block =
      context->cfg()->block(message_.exit_block());

  // The single-entry single-exit region defined by |message_.entry_block| and
  // |message_.exit_block|.
  std::set<opt::BasicBlock*> region_blocks = GetRegionBlocks(
      context, original_region_entry_block, original_region_exit_block);

  // Input and output ids for the region being outlined.
  std::vector<uint32_t> region_input_ids =
      GetRegionInputIds(context, region_blocks, original_region_exit_block);
  std::vector<uint32_t> region_output_ids =
      GetRegionOutputIds(context, region_blocks, original_region_exit_block);

  // Maps from input and output ids to fresh ids.
  std::map<uint32_t, uint32_t> input_id_to_fresh_id_map =
      PairSequenceToMap(message_.input_id_to_fresh_id());
  std::map<uint32_t, uint32_t> output_id_to_fresh_id_map =
      PairSequenceToMap(message_.output_id_to_fresh_id());

  UpdateModuleIdBoundForFreshIds(context, input_id_to_fresh_id_map,
                                 output_id_to_fresh_id_map);

  // Construct a map that associates each output id with its type id.
  std::map<uint32_t, uint32_t> output_id_to_type_id;
  for (uint32_t output_id : region_output_ids) {
    output_id_to_type_id[output_id] =
        context->get_def_use_mgr()->GetDef(output_id)->type_id();
  }

  // The region will be collapsed to a single block that calls a function
  // containing the outlined region.  This block needs to end with whatever
  // the exit block of the region ended with before outlining.  We thus clone
  // the terminator of the region's exit block, and the merge instruction for
  // the block if there is one, so that we can append them to the end of the
  // collapsed block later.
  std::unique_ptr<opt::Instruction> cloned_exit_block_terminator =
      std::unique_ptr<opt::Instruction>(
          original_region_exit_block->terminator()->Clone(context));
  std::unique_ptr<opt::Instruction> cloned_exit_block_merge =
      original_region_exit_block->GetMergeInst()
          ? std::unique_ptr<opt::Instruction>(
                original_region_exit_block->GetMergeInst()->Clone(context))
          : nullptr;

  // Make a function prototype for the outlined function, which involves
  // figuring out its required type.
  std::unique_ptr<opt::Function> outlined_function =
      PrepareFunctionPrototype(region_input_ids, region_output_ids,
                               input_id_to_fresh_id_map, context, fact_manager);

  // If the original function was livesafe, the new function should also be
  // livesafe.
  if (fact_manager->FunctionIsLivesafe(
          original_region_entry_block->GetParent()->result_id())) {
    fact_manager->AddFactFunctionIsLivesafe(message_.new_function_id());
  }

  // Adapt the region to be outlined so that its input ids are replaced with the
  // ids of the outlined function's input parameters, and so that output ids
  // are similarly remapped.
  RemapInputAndOutputIdsInRegion(
      context, *original_region_exit_block, region_blocks, region_input_ids,
      region_output_ids, input_id_to_fresh_id_map, output_id_to_fresh_id_map);

  // Fill out the body of the outlined function according to the region that is
  // being outlined.
  PopulateOutlinedFunction(*original_region_entry_block,
                           *original_region_exit_block, region_blocks,
                           region_output_ids, output_id_to_fresh_id_map,
                           context, outlined_function.get(), fact_manager);

  // Collapse the region that has been outlined into a function down to a single
  // block that calls said function.
  ShrinkOriginalRegion(
      context, region_blocks, region_input_ids, region_output_ids,
      output_id_to_type_id, outlined_function->type_id(),
      std::move(cloned_exit_block_merge),
      std::move(cloned_exit_block_terminator), original_region_entry_block);

  // Add the outlined function to the module.
  context->module()->AddFunction(std::move(outlined_function));

  // Major surgery has been conducted on the module, so invalidate all analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationOutlineFunction::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_outline_function() = message_;
  return result;
}

std::vector<uint32_t> TransformationOutlineFunction::GetRegionInputIds(
    opt::IRContext* context, const std::set<opt::BasicBlock*>& region_set,
    opt::BasicBlock* region_exit_block) {
  std::vector<uint32_t> result;

  auto enclosing_function = region_exit_block->GetParent();

  // Consider each parameter of the function containing the region.
  enclosing_function->ForEachParam([context, &region_set, &result](
                                       opt::Instruction* function_parameter) {
    // Consider every use of the parameter.
    context->get_def_use_mgr()->WhileEachUse(
        function_parameter, [context, function_parameter, &region_set, &result](
                                opt::Instruction* use, uint32_t /*unused*/) {
          // Get the block, if any, in which the parameter is used.
          auto use_block = context->get_instr_block(use);
          // If the use is in a block that lies within the region, the
          // parameter is an input id for the region.
          if (use_block && region_set.count(use_block) != 0) {
            result.push_back(function_parameter->result_id());
            return false;
          }
          return true;
        });
  });

  // Consider all definitions in the function that might turn out to be input
  // ids.
  for (auto& block : *enclosing_function) {
    std::vector<opt::Instruction*> candidate_input_ids_for_block;
    if (region_set.count(&block) == 0) {
      // All instructions in blocks outside the region are candidate's for
      // generating input ids.
      for (auto& inst : block) {
        candidate_input_ids_for_block.push_back(&inst);
      }
    } else {
      // Blocks in the region cannot generate input ids.
      continue;
    }

    // Consider each candidate input id to check whether it is used in the
    // region.
    for (auto& inst : candidate_input_ids_for_block) {
      context->get_def_use_mgr()->WhileEachUse(
          inst,
          [context, &inst, region_exit_block, &region_set, &result](
              opt::Instruction* use, uint32_t /*unused*/) -> bool {

            // Find the block in which this id use occurs, recording the id as
            // an input id if the block is outside the region, with some
            // exceptions detailed below.
            auto use_block = context->get_instr_block(use);

            if (!use_block) {
              // There might be no containing block, e.g. if the use is in a
              // decoration.
              return true;
            }

            if (region_set.count(use_block) == 0) {
              // The use is not in the region: this does not make it an input
              // id.
              return true;
            }

            if (use_block == region_exit_block && use->IsBlockTerminator()) {
              // We do not regard uses in the exit block terminator as input
              // ids, as this terminator does not get outlined.
              return true;
            }

            result.push_back(inst->result_id());
            return false;
          });
    }
  }
  return result;
}

std::vector<uint32_t> TransformationOutlineFunction::GetRegionOutputIds(
    opt::IRContext* context, const std::set<opt::BasicBlock*>& region_set,
    opt::BasicBlock* region_exit_block) {
  std::vector<uint32_t> result;

  // Consider each block in the function containing the region.
  for (auto& block : *region_exit_block->GetParent()) {
    if (region_set.count(&block) == 0) {
      // Skip blocks that are not in the region.
      continue;
    }
    // Consider each use of each instruction defined in the block.
    for (auto& inst : block) {
      context->get_def_use_mgr()->WhileEachUse(
          &inst,
          [&region_set, context, &inst, region_exit_block, &result](
              opt::Instruction* use, uint32_t /*unused*/) -> bool {

            // Find the block in which this id use occurs, recording the id as
            // an output id if the block is outside the region, with some
            // exceptions detailed below.
            auto use_block = context->get_instr_block(use);

            if (!use_block) {
              // There might be no containing block, e.g. if the use is in a
              // decoration.
              return true;
            }

            if (region_set.count(use_block) != 0) {
              // The use is in the region.
              if (use_block != region_exit_block || !use->IsBlockTerminator()) {
                // Furthermore, the use is not in the terminator of the region's
                // exit block.
                return true;
              }
            }

            result.push_back(inst.result_id());
            return false;
          });
    }
  }
  return result;
}

std::set<opt::BasicBlock*> TransformationOutlineFunction::GetRegionBlocks(
    opt::IRContext* context, opt::BasicBlock* entry_block,
    opt::BasicBlock* exit_block) {
  auto enclosing_function = entry_block->GetParent();
  auto dominator_analysis = context->GetDominatorAnalysis(enclosing_function);
  auto postdominator_analysis =
      context->GetPostDominatorAnalysis(enclosing_function);

  std::set<opt::BasicBlock*> result;
  for (auto& block : *enclosing_function) {
    if (dominator_analysis->Dominates(entry_block, &block) &&
        postdominator_analysis->Dominates(exit_block, &block)) {
      result.insert(&block);
    }
  }
  return result;
}

std::unique_ptr<opt::Function>
TransformationOutlineFunction::PrepareFunctionPrototype(
    const std::vector<uint32_t>& region_input_ids,
    const std::vector<uint32_t>& region_output_ids,
    const std::map<uint32_t, uint32_t>& input_id_to_fresh_id_map,
    opt::IRContext* context, FactManager* fact_manager) const {
  uint32_t return_type_id = 0;
  uint32_t function_type_id = 0;

  // First, try to find an existing function type that is suitable.  This is
  // only possible if the region generates no output ids; if it generates output
  // ids we are going to make a new struct for those, and since that struct does
  // not exist there cannot already be a function type with this struct as its
  // return type.
  if (region_output_ids.empty()) {
    std::vector<uint32_t> return_and_parameter_types;
    opt::analysis::Void void_type;
    return_type_id = context->get_type_mgr()->GetId(&void_type);
    return_and_parameter_types.push_back(return_type_id);
    for (auto id : region_input_ids) {
      return_and_parameter_types.push_back(
          context->get_def_use_mgr()->GetDef(id)->type_id());
    }
    function_type_id =
        fuzzerutil::FindFunctionType(context, return_and_parameter_types);
  }

  // If no existing function type was found, we need to create one.
  if (function_type_id == 0) {
    assert(
        ((return_type_id == 0) == !region_output_ids.empty()) &&
        "We should only have set the return type if there are no output ids.");
    // If the region generates output ids, we need to make a struct with one
    // field per output id.
    if (!region_output_ids.empty()) {
      opt::Instruction::OperandList struct_member_types;
      for (uint32_t output_id : region_output_ids) {
        auto output_id_type =
            context->get_def_use_mgr()->GetDef(output_id)->type_id();
        struct_member_types.push_back({SPV_OPERAND_TYPE_ID, {output_id_type}});
      }
      // Add a new struct type to the module.
      context->module()->AddType(MakeUnique<opt::Instruction>(
          context, SpvOpTypeStruct, 0,
          message_.new_function_struct_return_type_id(),
          std::move(struct_member_types)));
      // The return type for the function is the newly-created struct.
      return_type_id = message_.new_function_struct_return_type_id();
    }
    assert(
        return_type_id != 0 &&
        "We should either have a void return type, or have created a struct.");

    // The region's input ids dictate the parameter types to the function.
    opt::Instruction::OperandList function_type_operands;
    function_type_operands.push_back({SPV_OPERAND_TYPE_ID, {return_type_id}});
    for (auto id : region_input_ids) {
      function_type_operands.push_back(
          {SPV_OPERAND_TYPE_ID,
           {context->get_def_use_mgr()->GetDef(id)->type_id()}});
    }
    // Add a new function type to the module, and record that this is the type
    // id for the new function.
    context->module()->AddType(MakeUnique<opt::Instruction>(
        context, SpvOpTypeFunction, 0, message_.new_function_type_id(),
        function_type_operands));
    function_type_id = message_.new_function_type_id();
  }

  // Create a new function with |message_.new_function_id| as the function id,
  // and the return type and function type prepared above.
  std::unique_ptr<opt::Function> outlined_function =
      MakeUnique<opt::Function>(MakeUnique<opt::Instruction>(
          context, SpvOpFunction, return_type_id, message_.new_function_id(),
          opt::Instruction::OperandList(
              {{spv_operand_type_t ::SPV_OPERAND_TYPE_LITERAL_INTEGER,
                {SpvFunctionControlMaskNone}},
               {spv_operand_type_t::SPV_OPERAND_TYPE_ID,
                {function_type_id}}})));

  // Add one parameter to the function for each input id, using the fresh ids
  // provided in |input_id_to_fresh_id_map|.
  for (auto id : region_input_ids) {
    outlined_function->AddParameter(MakeUnique<opt::Instruction>(
        context, SpvOpFunctionParameter,
        context->get_def_use_mgr()->GetDef(id)->type_id(),
        input_id_to_fresh_id_map.at(id), opt::Instruction::OperandList()));
    // If the input id is an arbitrary-valued variable, the same should be true
    // of the corresponding parameter.
    if (fact_manager->VariableValueIsArbitrary(id)) {
      fact_manager->AddFactValueOfVariableIsArbitrary(
          input_id_to_fresh_id_map.at(id));
    }
  }

  return outlined_function;
}

void TransformationOutlineFunction::UpdateModuleIdBoundForFreshIds(
    opt::IRContext* context,
    const std::map<uint32_t, uint32_t>& input_id_to_fresh_id_map,
    const std::map<uint32_t, uint32_t>& output_id_to_fresh_id_map) const {
  // Enlarge the module's id bound as needed to accommodate the various fresh
  // ids associated with the transformation.
  fuzzerutil::UpdateModuleIdBound(
      context, message_.new_function_struct_return_type_id());
  fuzzerutil::UpdateModuleIdBound(context, message_.new_function_type_id());
  fuzzerutil::UpdateModuleIdBound(context, message_.new_function_id());
  fuzzerutil::UpdateModuleIdBound(context,
                                  message_.new_function_region_entry_block());
  fuzzerutil::UpdateModuleIdBound(context, message_.new_caller_result_id());
  fuzzerutil::UpdateModuleIdBound(context, message_.new_callee_result_id());

  for (auto& entry : input_id_to_fresh_id_map) {
    fuzzerutil::UpdateModuleIdBound(context, entry.second);
  }

  for (auto& entry : output_id_to_fresh_id_map) {
    fuzzerutil::UpdateModuleIdBound(context, entry.second);
  }
}

void TransformationOutlineFunction::RemapInputAndOutputIdsInRegion(
    opt::IRContext* context, const opt::BasicBlock& original_region_exit_block,
    const std::set<opt::BasicBlock*>& region_blocks,
    const std::vector<uint32_t>& region_input_ids,
    const std::vector<uint32_t>& region_output_ids,
    const std::map<uint32_t, uint32_t>& input_id_to_fresh_id_map,
    const std::map<uint32_t, uint32_t>& output_id_to_fresh_id_map) const {
  // Change all uses of input ids inside the region to the corresponding fresh
  // ids that will ultimately be parameters of the outlined function.
  // This is done by considering each region input id in turn.
  for (uint32_t id : region_input_ids) {
    // We then consider each use of the input id.
    context->get_def_use_mgr()->ForEachUse(
        id, [context, id, &input_id_to_fresh_id_map, region_blocks](
                opt::Instruction* use, uint32_t operand_index) {
          // Find the block in which this use of the input id occurs.
          opt::BasicBlock* use_block = context->get_instr_block(use);
          // We want to rewrite the use id if its block occurs in the outlined
          // region.
          if (region_blocks.count(use_block) != 0) {
            // Rewrite this use of the input id.
            use->SetOperand(operand_index, {input_id_to_fresh_id_map.at(id)});
          }
        });
  }

  // Change each definition of a region output id to define the corresponding
  // fresh ids that will store intermediate value for the output ids.  Also
  // change all uses of the output id located in the outlined region.
  // This is done by considering each region output id in turn.
  for (uint32_t id : region_output_ids) {
    // First consider each use of the output id and update the relevant uses.
    context->get_def_use_mgr()->ForEachUse(
        id,
        [context, &original_region_exit_block, id, &output_id_to_fresh_id_map,
         region_blocks](opt::Instruction* use, uint32_t operand_index) {
          // Find the block in which this use of the output id occurs.
          auto use_block = context->get_instr_block(use);
          // We want to rewrite the use id if its block occurs in the outlined
          // region, with one exception: the terminator of the exit block of
          // the region is going to remain in the original function, so if the
          // use appears in such a terminator instruction we leave it alone.
          if (
              // The block is in the region ...
              region_blocks.count(use_block) != 0 &&
              // ... and the use is not in the terminator instruction of the
              // region's exit block.
              !(use_block == &original_region_exit_block &&
                use->IsBlockTerminator())) {
            // Rewrite this use of the output id.
            use->SetOperand(operand_index, {output_id_to_fresh_id_map.at(id)});
          }
        });

    // Now change the instruction that defines the output id so that it instead
    // defines the corresponding fresh id.  We do this after changing all the
    // uses so that the definition of the original id is still registered when
    // we analyse its uses.
    context->get_def_use_mgr()->GetDef(id)->SetResultId(
        output_id_to_fresh_id_map.at(id));
  }
}

void TransformationOutlineFunction::PopulateOutlinedFunction(
    const opt::BasicBlock& original_region_entry_block,
    const opt::BasicBlock& original_region_exit_block,
    const std::set<opt::BasicBlock*>& region_blocks,
    const std::vector<uint32_t>& region_output_ids,
    const std::map<uint32_t, uint32_t>& output_id_to_fresh_id_map,
    opt::IRContext* context, opt::Function* outlined_function,
    FactManager* fact_manager) const {
  // When we create the exit block for the outlined region, we use this pointer
  // to track of it so that we can manipulate it later.
  opt::BasicBlock* outlined_region_exit_block = nullptr;

  // The region entry block in the new function is identical to the entry block
  // of the region being outlined, except that it has
  // |message_.new_function_region_entry_block| as its id.
  std::unique_ptr<opt::BasicBlock> outlined_region_entry_block =
      MakeUnique<opt::BasicBlock>(MakeUnique<opt::Instruction>(
          context, SpvOpLabel, 0, message_.new_function_region_entry_block(),
          opt::Instruction::OperandList()));
  outlined_region_entry_block->SetParent(outlined_function);

  // If the original region's entry block was dead, the outlined region's entry
  // block is also dead.
  if (fact_manager->BlockIsDead(original_region_entry_block.id())) {
    fact_manager->AddFactBlockIsDead(outlined_region_entry_block->id());
  }

  if (&original_region_entry_block == &original_region_exit_block) {
    outlined_region_exit_block = outlined_region_entry_block.get();
  }

  for (auto& inst : original_region_entry_block) {
    outlined_region_entry_block->AddInstruction(
        std::unique_ptr<opt::Instruction>(inst.Clone(context)));
  }
  outlined_function->AddBasicBlock(std::move(outlined_region_entry_block));

  // We now go through the single-entry single-exit region defined by the entry
  // and exit blocks, adding clones of all blocks to the new function.

  // Consider every block in the enclosing function.
  auto enclosing_function = original_region_entry_block.GetParent();
  for (auto block_it = enclosing_function->begin();
       block_it != enclosing_function->end();) {
    // Skip the region's entry block - we already dealt with it above.
    if (region_blocks.count(&*block_it) == 0 ||
        &*block_it == &original_region_entry_block) {
      ++block_it;
      continue;
    }
    // Clone the block so that it can be added to the new function.
    auto cloned_block =
        std::unique_ptr<opt::BasicBlock>(block_it->Clone(context));

    // If this is the region's exit block, then the cloned block is the outlined
    // region's exit block.
    if (&*block_it == &original_region_exit_block) {
      assert(outlined_region_exit_block == nullptr &&
             "We should not yet have encountered the exit block.");
      outlined_region_exit_block = cloned_block.get();
    }

    cloned_block->SetParent(outlined_function);

    // Redirect any OpPhi operands whose predecessors are the original region
    // entry block to become the new function entry block.
    cloned_block->ForEachPhiInst([this](opt::Instruction* phi_inst) {
      for (uint32_t predecessor_index = 1;
           predecessor_index < phi_inst->NumInOperands();
           predecessor_index += 2) {
        if (phi_inst->GetSingleWordInOperand(predecessor_index) ==
            message_.entry_block()) {
          phi_inst->SetInOperand(predecessor_index,
                                 {message_.new_function_region_entry_block()});
        }
      }
    });

    outlined_function->AddBasicBlock(std::move(cloned_block));
    block_it = block_it.Erase();
  }
  assert(outlined_region_exit_block != nullptr &&
         "We should have encountered the region's exit block when iterating "
         "through the function");

  // We now need to adapt the exit block for the region - in the new function -
  // so that it ends with a return.

  // We first eliminate the merge instruction (if any) and the terminator for
  // the cloned exit block.
  for (auto inst_it = outlined_region_exit_block->begin();
       inst_it != outlined_region_exit_block->end();) {
    if (inst_it->opcode() == SpvOpLoopMerge ||
        inst_it->opcode() == SpvOpSelectionMerge) {
      inst_it = inst_it.Erase();
    } else if (inst_it->IsBlockTerminator()) {
      inst_it = inst_it.Erase();
    } else {
      ++inst_it;
    }
  }

  // We now add either OpReturn or OpReturnValue as the cloned exit block's
  // terminator.
  if (region_output_ids.empty()) {
    // The case where there are no region output ids is simple: we just add
    // OpReturn.
    outlined_region_exit_block->AddInstruction(MakeUnique<opt::Instruction>(
        context, SpvOpReturn, 0, 0, opt::Instruction::OperandList()));
  } else {
    // In the case where there are output ids, we add an OpCompositeConstruct
    // instruction to pack all the output values into a struct, and then an
    // OpReturnValue instruction to return this struct.
    opt::Instruction::OperandList struct_member_operands;
    for (uint32_t id : region_output_ids) {
      struct_member_operands.push_back(
          {SPV_OPERAND_TYPE_ID, {output_id_to_fresh_id_map.at(id)}});
    }
    outlined_region_exit_block->AddInstruction(MakeUnique<opt::Instruction>(
        context, SpvOpCompositeConstruct,
        message_.new_function_struct_return_type_id(),
        message_.new_callee_result_id(), struct_member_operands));
    outlined_region_exit_block->AddInstruction(MakeUnique<opt::Instruction>(
        context, SpvOpReturnValue, 0, 0,
        opt::Instruction::OperandList(
            {{SPV_OPERAND_TYPE_ID, {message_.new_callee_result_id()}}})));
  }

  outlined_function->SetFunctionEnd(MakeUnique<opt::Instruction>(
      context, SpvOpFunctionEnd, 0, 0, opt::Instruction::OperandList()));
}

void TransformationOutlineFunction::ShrinkOriginalRegion(
    opt::IRContext* context, std::set<opt::BasicBlock*>& region_blocks,
    const std::vector<uint32_t>& region_input_ids,
    const std::vector<uint32_t>& region_output_ids,
    const std::map<uint32_t, uint32_t>& output_id_to_type_id,
    uint32_t return_type_id,
    std::unique_ptr<opt::Instruction> cloned_exit_block_merge,
    std::unique_ptr<opt::Instruction> cloned_exit_block_terminator,
    opt::BasicBlock* original_region_entry_block) const {
  // Erase all blocks from the original function that are in the outlined
  // region, except for the region's entry block.
  //
  // In the process, identify all references to the exit block of the region,
  // as merge blocks, continue targets, or OpPhi predecessors, and rewrite them
  // to refer to the region entry block (the single block to which we are
  // shrinking the region).
  auto enclosing_function = original_region_entry_block->GetParent();
  for (auto block_it = enclosing_function->begin();
       block_it != enclosing_function->end();) {
    if (&*block_it == original_region_entry_block) {
      ++block_it;
    } else if (region_blocks.count(&*block_it) == 0) {
      // The block is not in the region.  Check whether it has the last block
      // of the region as an OpPhi predecessor, and if so change the
      // predecessor to be the first block of the region (i.e. the block
      // containing the call to what was outlined).
      assert(block_it->MergeBlockIdIfAny() != message_.exit_block() &&
             "Outlined region must not end with a merge block");
      assert(block_it->ContinueBlockIdIfAny() != message_.exit_block() &&
             "Outlined region must not end with a continue target");
      block_it->ForEachPhiInst([this](opt::Instruction* phi_inst) {
        for (uint32_t predecessor_index = 1;
             predecessor_index < phi_inst->NumInOperands();
             predecessor_index += 2) {
          if (phi_inst->GetSingleWordInOperand(predecessor_index) ==
              message_.exit_block()) {
            phi_inst->SetInOperand(predecessor_index, {message_.entry_block()});
          }
        }
      });
      ++block_it;
    } else {
      // The block is in the region and is not the region's entry block: kill
      // it.
      block_it = block_it.Erase();
    }
  }

  // Now erase all instructions from the region's entry block, as they have
  // been outlined.
  for (auto inst_it = original_region_entry_block->begin();
       inst_it != original_region_entry_block->end();) {
    inst_it = inst_it.Erase();
  }

  // Now we add a call to the outlined function to the region's entry block.
  opt::Instruction::OperandList function_call_operands;
  function_call_operands.push_back(
      {SPV_OPERAND_TYPE_ID, {message_.new_function_id()}});
  // The function parameters are the region input ids.
  for (auto input_id : region_input_ids) {
    function_call_operands.push_back({SPV_OPERAND_TYPE_ID, {input_id}});
  }

  original_region_entry_block->AddInstruction(MakeUnique<opt::Instruction>(
      context, SpvOpFunctionCall, return_type_id,
      message_.new_caller_result_id(), function_call_operands));

  // If there are output ids, the function call will return a struct.  For each
  // output id, we add an extract operation to pull the appropriate struct
  // member out into an output id.
  for (uint32_t index = 0; index < region_output_ids.size(); ++index) {
    uint32_t output_id = region_output_ids[index];
    original_region_entry_block->AddInstruction(MakeUnique<opt::Instruction>(
        context, SpvOpCompositeExtract, output_id_to_type_id.at(output_id),
        output_id,
        opt::Instruction::OperandList(
            {{SPV_OPERAND_TYPE_ID, {message_.new_caller_result_id()}},
             {SPV_OPERAND_TYPE_LITERAL_INTEGER, {index}}})));
  }

  // Finally, we terminate the block with the merge instruction (if any) that
  // used to belong to the region's exit block, and the terminator that used
  // to belong to the region's exit block.
  if (cloned_exit_block_merge != nullptr) {
    original_region_entry_block->AddInstruction(
        std::move(cloned_exit_block_merge));
  }
  original_region_entry_block->AddInstruction(
      std::move(cloned_exit_block_terminator));
}

}  // namespace fuzz
}  // namespace spvtools
