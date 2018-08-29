// Copyright (c) 2017 The Khronos Group Inc.
// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG Inc.
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

#include "source/opt/pass.h"

#include "source/opt/iterator.h"

namespace spvtools {
namespace opt {

namespace {

const uint32_t kEntryPointFunctionIdInIdx = 1;
const uint32_t kTypePointerTypeIdInIdx = 1;

}  // namespace

Pass::Pass() : consumer_(nullptr), context_(nullptr), already_run_(false) {}

void Pass::AddCalls(Function* func, std::queue<uint32_t>* todo) {
  for (auto bi = func->begin(); bi != func->end(); ++bi)
    for (auto ii = bi->begin(); ii != bi->end(); ++ii)
      if (ii->opcode() == SpvOpFunctionCall)
        todo->push(ii->GetSingleWordInOperand(0));
}

bool Pass::ProcessEntryPointCallTree(ProcessFunction& pfn, Module* module) {
  // Map from function's result id to function
  std::unordered_map<uint32_t, Function*> id2function;
  for (auto& fn : *module) id2function[fn.result_id()] = &fn;

  // Collect all of the entry points as the roots.
  std::queue<uint32_t> roots;
  for (auto& e : module->entry_points())
    roots.push(e.GetSingleWordInOperand(kEntryPointFunctionIdInIdx));
  return ProcessCallTreeFromRoots(pfn, id2function, &roots);
}

bool Pass::ProcessReachableCallTree(ProcessFunction& pfn,
                                    IRContext* irContext) {
  // Map from function's result id to function
  std::unordered_map<uint32_t, Function*> id2function;
  for (auto& fn : *irContext->module()) id2function[fn.result_id()] = &fn;

  std::queue<uint32_t> roots;

  // Add all entry points since they can be reached from outside the module.
  for (auto& e : irContext->module()->entry_points())
    roots.push(e.GetSingleWordInOperand(kEntryPointFunctionIdInIdx));

  // Add all exported functions since they can be reached from outside the
  // module.
  for (auto& a : irContext->annotations()) {
    // TODO: Handle group decorations as well.  Currently not generate by any
    // front-end, but could be coming.
    if (a.opcode() == SpvOp::SpvOpDecorate) {
      if (a.GetSingleWordOperand(1) ==
          SpvDecoration::SpvDecorationLinkageAttributes) {
        uint32_t lastOperand = a.NumOperands() - 1;
        if (a.GetSingleWordOperand(lastOperand) ==
            SpvLinkageType::SpvLinkageTypeExport) {
          uint32_t id = a.GetSingleWordOperand(0);
          if (id2function.count(id) != 0) roots.push(id);
        }
      }
    }
  }

  return ProcessCallTreeFromRoots(pfn, id2function, &roots);
}

bool Pass::ProcessCallTreeFromRoots(
    ProcessFunction& pfn,
    const std::unordered_map<uint32_t, Function*>& id2function,
    std::queue<uint32_t>* roots) {
  // Process call tree
  bool modified = false;
  std::unordered_set<uint32_t> done;

  while (!roots->empty()) {
    const uint32_t fi = roots->front();
    roots->pop();
    if (done.insert(fi).second) {
      Function* fn = id2function.at(fi);
      modified = pfn(fn) || modified;
      AddCalls(fn, roots);
    }
  }
  return modified;
}

Pass::Status Pass::Run(IRContext* ctx) {
  if (already_run_) {
    return Status::Failure;
  }
  already_run_ = true;

  context_ = ctx;
  Pass::Status status = Process();
  context_ = nullptr;

  if (status == Status::SuccessWithChange) {
    ctx->InvalidateAnalysesExceptFor(GetPreservedAnalyses());
  }
  assert(ctx->IsConsistent());
  return status;
}

uint32_t Pass::GetPointeeTypeId(const Instruction* ptrInst) const {
  const uint32_t ptrTypeId = ptrInst->type_id();
  const Instruction* ptrTypeInst = get_def_use_mgr()->GetDef(ptrTypeId);
  return ptrTypeInst->GetSingleWordInOperand(kTypePointerTypeIdInIdx);
}

}  // namespace opt
}  // namespace spvtools
