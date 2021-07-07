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

#include "source/val/construct.h"

#include <cassert>
#include <cstddef>
#include <unordered_set>

#include "source/val/function.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

Construct::Construct(ConstructType construct_type, BasicBlock* entry,
                     BasicBlock* exit, std::vector<Construct*> constructs)
    : type_(construct_type),
      corresponding_constructs_(constructs),
      entry_block_(entry),
      exit_block_(exit) {}

ConstructType Construct::type() const { return type_; }

const std::vector<Construct*>& Construct::corresponding_constructs() const {
  return corresponding_constructs_;
}
std::vector<Construct*>& Construct::corresponding_constructs() {
  return corresponding_constructs_;
}

bool ValidateConstructSize(ConstructType type, size_t size) {
  switch (type) {
    case ConstructType::kSelection:
      return size == 0;
    case ConstructType::kContinue:
      return size == 1;
    case ConstructType::kLoop:
      return size == 1;
    case ConstructType::kCase:
      return size >= 1;
    default:
      assert(1 == 0 && "Type not defined");
  }
  return false;
}

void Construct::set_corresponding_constructs(
    std::vector<Construct*> constructs) {
  assert(ValidateConstructSize(type_, constructs.size()));
  corresponding_constructs_ = constructs;
}

const BasicBlock* Construct::entry_block() const { return entry_block_; }
BasicBlock* Construct::entry_block() { return entry_block_; }

const BasicBlock* Construct::exit_block() const { return exit_block_; }
BasicBlock* Construct::exit_block() { return exit_block_; }

void Construct::set_exit(BasicBlock* block) { exit_block_ = block; }

Construct::ConstructBlockSet Construct::blocks(Function* function) const {
  auto header = entry_block();
  auto merge = exit_block();
  assert(header);
  int header_depth = function->GetBlockDepth(const_cast<BasicBlock*>(header));
  ConstructBlockSet construct_blocks;
  std::unordered_set<BasicBlock*> corresponding_headers;
  for (auto& other : corresponding_constructs()) {
    // The corresponding header can be the same block as this construct's
    // header for loops with no loop construct. In those cases, don't add the
    // loop header as it prevents finding any blocks in the construct.
    if (type() != ConstructType::kContinue || other->entry_block() != header) {
      corresponding_headers.insert(other->entry_block());
    }
  }
  std::vector<BasicBlock*> stack;
  stack.push_back(const_cast<BasicBlock*>(header));
  while (!stack.empty()) {
    BasicBlock* block = stack.back();
    stack.pop_back();

    if (merge == block && ExitBlockIsMergeBlock()) {
      // Merge block is not part of the construct.
      continue;
    }

    if (corresponding_headers.count(block)) {
      // Entered a corresponding construct.
      continue;
    }

    int block_depth = function->GetBlockDepth(block);
    if (block_depth < header_depth) {
      // Broke to outer construct.
      continue;
    }

    // In a loop, the continue target is at a depth of the loop construct + 1.
    // A selection construct nested directly within the loop construct is also
    // at the same depth. It is valid, however, to branch directly to the
    // continue target from within the selection construct.
    if (block != header && block_depth == header_depth &&
        type() == ConstructType::kSelection &&
        block->is_type(kBlockTypeContinue)) {
      // Continued to outer construct.
      continue;
    }

    if (!construct_blocks.insert(block).second) continue;

    if (merge != block) {
      for (auto succ : *block->successors()) {
        // All blocks in the construct must be dominated by the header.
        if (header->dominates(*succ)) {
          stack.push_back(succ);
        }
      }
    }
  }

  return construct_blocks;
}

bool Construct::IsStructuredExit(ValidationState_t& _, BasicBlock* dest) const {
  // Structured Exits:
  // - Selection:
  //  - branch to its merge
  //  - branch to nearest enclosing loop merge or continue
  //  - branch to nearest enclosing switch selection merge
  // - Loop:
  //  - branch to its merge
  //  - branch to its continue
  // - Continue:
  //  - branch to loop header
  //  - branch to loop merge
  //
  // Note: we will never see a case construct here.
  assert(type() != ConstructType::kCase);
  if (type() == ConstructType::kLoop) {
    auto header = entry_block();
    auto terminator = header->terminator();
    auto index = terminator - &_.ordered_instructions()[0];
    auto merge_inst = &_.ordered_instructions()[index - 1];
    auto merge_block_id = merge_inst->GetOperandAs<uint32_t>(0u);
    auto continue_block_id = merge_inst->GetOperandAs<uint32_t>(1u);
    if (dest->id() == merge_block_id || dest->id() == continue_block_id) {
      return true;
    }
  } else if (type() == ConstructType::kContinue) {
    auto loop_construct = corresponding_constructs()[0];
    auto header = loop_construct->entry_block();
    auto terminator = header->terminator();
    auto index = terminator - &_.ordered_instructions()[0];
    auto merge_inst = &_.ordered_instructions()[index - 1];
    auto merge_block_id = merge_inst->GetOperandAs<uint32_t>(0u);
    if (dest == header || dest->id() == merge_block_id) {
      return true;
    }
  } else {
    assert(type() == ConstructType::kSelection);
    if (dest == exit_block()) {
      return true;
    }

    // The next block in the traversal is either:
    //  i.  The header block that declares |block| as its merge block.
    //  ii. The immediate dominator of |block|.
    auto NextBlock = [](const BasicBlock* block) -> const BasicBlock* {
      for (auto& use : block->label()->uses()) {
        if ((use.first->opcode() == SpvOpLoopMerge ||
             use.first->opcode() == SpvOpSelectionMerge) &&
            use.second == 1)
          return use.first->block();
      }
      return block->immediate_dominator();
    };

    bool seen_switch = false;
    auto header = entry_block();
    auto block = NextBlock(header);
    while (block) {
      auto terminator = block->terminator();
      auto index = terminator - &_.ordered_instructions()[0];
      auto merge_inst = &_.ordered_instructions()[index - 1];
      if (merge_inst->opcode() == SpvOpLoopMerge ||
          (header->terminator()->opcode() != SpvOpSwitch &&
           merge_inst->opcode() == SpvOpSelectionMerge &&
           terminator->opcode() == SpvOpSwitch)) {
        auto merge_target = merge_inst->GetOperandAs<uint32_t>(0u);
        auto merge_block = merge_inst->function()->GetBlock(merge_target).first;
        if (merge_block->dominates(*header)) {
          block = NextBlock(block);
          continue;
        }

        if ((!seen_switch || merge_inst->opcode() == SpvOpLoopMerge) &&
            dest->id() == merge_target) {
          return true;
        } else if (merge_inst->opcode() == SpvOpLoopMerge) {
          auto continue_target = merge_inst->GetOperandAs<uint32_t>(1u);
          if (dest->id() == continue_target) {
            return true;
          }
        }

        if (terminator->opcode() == SpvOpSwitch) {
          seen_switch = true;
        }

        // Hit an enclosing loop and didn't break or continue.
        if (merge_inst->opcode() == SpvOpLoopMerge) return false;
      }

      block = NextBlock(block);
    }
  }

  return false;
}

}  // namespace val
}  // namespace spvtools
