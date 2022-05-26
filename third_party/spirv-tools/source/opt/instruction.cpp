// Copyright (c) 2016 Google Inc.
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

#include "source/opt/instruction.h"

#include <initializer_list>

#include "OpenCLDebugInfo100.h"
#include "source/disassemble.h"
#include "source/opt/fold.h"
#include "source/opt/ir_context.h"
#include "source/opt/reflect.h"

namespace spvtools {
namespace opt {

namespace {
// Indices used to get particular operands out of instructions using InOperand.
const uint32_t kTypeImageDimIndex = 1;
const uint32_t kLoadBaseIndex = 0;
const uint32_t kPointerTypeStorageClassIndex = 0;
const uint32_t kVariableStorageClassIndex = 0;
const uint32_t kTypeImageSampledIndex = 5;

// Constants for OpenCL.DebugInfo.100 / NonSemantic.Shader.DebugInfo.100
// extension instructions.
const uint32_t kExtInstSetIdInIdx = 0;
const uint32_t kExtInstInstructionInIdx = 1;
const uint32_t kDebugScopeNumWords = 7;
const uint32_t kDebugScopeNumWordsWithoutInlinedAt = 6;
const uint32_t kDebugNoScopeNumWords = 5;

// Number of operands of an OpBranchConditional instruction
// with weights.
const uint32_t kOpBranchConditionalWithWeightsNumOperands = 5;
}  // namespace

Instruction::Instruction(IRContext* c)
    : utils::IntrusiveNodeBase<Instruction>(),
      context_(c),
      opcode_(SpvOpNop),
      has_type_id_(false),
      has_result_id_(false),
      unique_id_(c->TakeNextUniqueId()),
      dbg_scope_(kNoDebugScope, kNoInlinedAt) {}

Instruction::Instruction(IRContext* c, SpvOp op)
    : utils::IntrusiveNodeBase<Instruction>(),
      context_(c),
      opcode_(op),
      has_type_id_(false),
      has_result_id_(false),
      unique_id_(c->TakeNextUniqueId()),
      dbg_scope_(kNoDebugScope, kNoInlinedAt) {}

Instruction::Instruction(IRContext* c, const spv_parsed_instruction_t& inst,
                         std::vector<Instruction>&& dbg_line)
    : utils::IntrusiveNodeBase<Instruction>(),
      context_(c),
      opcode_(static_cast<SpvOp>(inst.opcode)),
      has_type_id_(inst.type_id != 0),
      has_result_id_(inst.result_id != 0),
      unique_id_(c->TakeNextUniqueId()),
      dbg_line_insts_(std::move(dbg_line)),
      dbg_scope_(kNoDebugScope, kNoInlinedAt) {
  for (uint32_t i = 0; i < inst.num_operands; ++i) {
    const auto& current_payload = inst.operands[i];
    operands_.emplace_back(
        current_payload.type, inst.words + current_payload.offset,
        inst.words + current_payload.offset + current_payload.num_words);
  }
  assert((!IsLineInst() || dbg_line.empty()) &&
         "Op(No)Line attaching to Op(No)Line found");
}

Instruction::Instruction(IRContext* c, const spv_parsed_instruction_t& inst,
                         const DebugScope& dbg_scope)
    : utils::IntrusiveNodeBase<Instruction>(),
      context_(c),
      opcode_(static_cast<SpvOp>(inst.opcode)),
      has_type_id_(inst.type_id != 0),
      has_result_id_(inst.result_id != 0),
      unique_id_(c->TakeNextUniqueId()),
      dbg_scope_(dbg_scope) {
  for (uint32_t i = 0; i < inst.num_operands; ++i) {
    const auto& current_payload = inst.operands[i];
    operands_.emplace_back(
        current_payload.type, inst.words + current_payload.offset,
        inst.words + current_payload.offset + current_payload.num_words);
  }
}

Instruction::Instruction(IRContext* c, SpvOp op, uint32_t ty_id,
                         uint32_t res_id, const OperandList& in_operands)
    : utils::IntrusiveNodeBase<Instruction>(),
      context_(c),
      opcode_(op),
      has_type_id_(ty_id != 0),
      has_result_id_(res_id != 0),
      unique_id_(c->TakeNextUniqueId()),
      operands_(),
      dbg_scope_(kNoDebugScope, kNoInlinedAt) {
  if (has_type_id_) {
    operands_.emplace_back(spv_operand_type_t::SPV_OPERAND_TYPE_TYPE_ID,
                           std::initializer_list<uint32_t>{ty_id});
  }
  if (has_result_id_) {
    operands_.emplace_back(spv_operand_type_t::SPV_OPERAND_TYPE_RESULT_ID,
                           std::initializer_list<uint32_t>{res_id});
  }
  operands_.insert(operands_.end(), in_operands.begin(), in_operands.end());
}

Instruction::Instruction(Instruction&& that)
    : utils::IntrusiveNodeBase<Instruction>(),
      context_(that.context_),
      opcode_(that.opcode_),
      has_type_id_(that.has_type_id_),
      has_result_id_(that.has_result_id_),
      unique_id_(that.unique_id_),
      operands_(std::move(that.operands_)),
      dbg_line_insts_(std::move(that.dbg_line_insts_)),
      dbg_scope_(that.dbg_scope_) {
  for (auto& i : dbg_line_insts_) {
    i.dbg_scope_ = that.dbg_scope_;
  }
}

Instruction& Instruction::operator=(Instruction&& that) {
  context_ = that.context_;
  opcode_ = that.opcode_;
  has_type_id_ = that.has_type_id_;
  has_result_id_ = that.has_result_id_;
  unique_id_ = that.unique_id_;
  operands_ = std::move(that.operands_);
  dbg_line_insts_ = std::move(that.dbg_line_insts_);
  dbg_scope_ = that.dbg_scope_;
  return *this;
}

Instruction* Instruction::Clone(IRContext* c) const {
  Instruction* clone = new Instruction(c);
  clone->opcode_ = opcode_;
  clone->has_type_id_ = has_type_id_;
  clone->has_result_id_ = has_result_id_;
  clone->unique_id_ = c->TakeNextUniqueId();
  clone->operands_ = operands_;
  clone->dbg_line_insts_ = dbg_line_insts_;
  for (auto& i : clone->dbg_line_insts_) {
    i.unique_id_ = c->TakeNextUniqueId();
    if (i.IsDebugLineInst()) i.SetResultId(c->TakeNextId());
  }
  clone->dbg_scope_ = dbg_scope_;
  return clone;
}

uint32_t Instruction::GetSingleWordOperand(uint32_t index) const {
  const auto& words = GetOperand(index).words;
  assert(words.size() == 1 && "expected the operand only taking one word");
  return words.front();
}

uint32_t Instruction::NumInOperandWords() const {
  uint32_t size = 0;
  for (uint32_t i = TypeResultIdCount(); i < operands_.size(); ++i)
    size += static_cast<uint32_t>(operands_[i].words.size());
  return size;
}

bool Instruction::HasBranchWeights() const {
  if (opcode_ == SpvOpBranchConditional &&
      NumOperands() == kOpBranchConditionalWithWeightsNumOperands) {
    return true;
  }

  return false;
}

void Instruction::ToBinaryWithoutAttachedDebugInsts(
    std::vector<uint32_t>* binary) const {
  const uint32_t num_words = 1 + NumOperandWords();
  binary->push_back((num_words << 16) | static_cast<uint16_t>(opcode_));
  for (const auto& operand : operands_) {
    binary->insert(binary->end(), operand.words.begin(), operand.words.end());
  }
}

void Instruction::ReplaceOperands(const OperandList& new_operands) {
  operands_.clear();
  operands_.insert(operands_.begin(), new_operands.begin(), new_operands.end());
}

bool Instruction::IsReadOnlyLoad() const {
  if (IsLoad()) {
    Instruction* address_def = GetBaseAddress();
    if (!address_def) {
      return false;
    }

    if (address_def->opcode() == SpvOpVariable) {
      if (address_def->IsReadOnlyPointer()) {
        return true;
      }
    }

    if (address_def->opcode() == SpvOpLoad) {
      const analysis::Type* address_type =
          context()->get_type_mgr()->GetType(address_def->type_id());
      if (address_type->AsSampledImage() != nullptr) {
        const auto* image_type =
            address_type->AsSampledImage()->image_type()->AsImage();
        if (image_type->sampled() == 1) {
          return true;
        }
      }
    }
  }
  return false;
}

Instruction* Instruction::GetBaseAddress() const {
  uint32_t base = GetSingleWordInOperand(kLoadBaseIndex);
  Instruction* base_inst = context()->get_def_use_mgr()->GetDef(base);
  bool done = false;
  while (!done) {
    switch (base_inst->opcode()) {
      case SpvOpAccessChain:
      case SpvOpInBoundsAccessChain:
      case SpvOpPtrAccessChain:
      case SpvOpInBoundsPtrAccessChain:
      case SpvOpImageTexelPointer:
      case SpvOpCopyObject:
        // All of these instructions have the base pointer use a base pointer
        // in in-operand 0.
        base = base_inst->GetSingleWordInOperand(0);
        base_inst = context()->get_def_use_mgr()->GetDef(base);
        break;
      default:
        done = true;
        break;
    }
  }
  return base_inst;
}

bool Instruction::IsReadOnlyPointer() const {
  if (context()->get_feature_mgr()->HasCapability(SpvCapabilityShader))
    return IsReadOnlyPointerShaders();
  else
    return IsReadOnlyPointerKernel();
}

bool Instruction::IsVulkanStorageImage() const {
  if (opcode() != SpvOpTypePointer) {
    return false;
  }

  uint32_t storage_class =
      GetSingleWordInOperand(kPointerTypeStorageClassIndex);
  if (storage_class != SpvStorageClassUniformConstant) {
    return false;
  }

  Instruction* base_type =
      context()->get_def_use_mgr()->GetDef(GetSingleWordInOperand(1));

  // Unpack the optional layer of arraying.
  if (base_type->opcode() == SpvOpTypeArray ||
      base_type->opcode() == SpvOpTypeRuntimeArray) {
    base_type = context()->get_def_use_mgr()->GetDef(
        base_type->GetSingleWordInOperand(0));
  }

  if (base_type->opcode() != SpvOpTypeImage) {
    return false;
  }

  if (base_type->GetSingleWordInOperand(kTypeImageDimIndex) == SpvDimBuffer) {
    return false;
  }

  // Check if the image is sampled.  If we do not know for sure that it is,
  // then assume it is a storage image.
  return base_type->GetSingleWordInOperand(kTypeImageSampledIndex) != 1;
}

bool Instruction::IsVulkanSampledImage() const {
  if (opcode() != SpvOpTypePointer) {
    return false;
  }

  uint32_t storage_class =
      GetSingleWordInOperand(kPointerTypeStorageClassIndex);
  if (storage_class != SpvStorageClassUniformConstant) {
    return false;
  }

  Instruction* base_type =
      context()->get_def_use_mgr()->GetDef(GetSingleWordInOperand(1));

  // Unpack the optional layer of arraying.
  if (base_type->opcode() == SpvOpTypeArray ||
      base_type->opcode() == SpvOpTypeRuntimeArray) {
    base_type = context()->get_def_use_mgr()->GetDef(
        base_type->GetSingleWordInOperand(0));
  }

  if (base_type->opcode() != SpvOpTypeImage) {
    return false;
  }

  if (base_type->GetSingleWordInOperand(kTypeImageDimIndex) == SpvDimBuffer) {
    return false;
  }

  // Check if the image is sampled.  If we know for sure that it is,
  // then return true.
  return base_type->GetSingleWordInOperand(kTypeImageSampledIndex) == 1;
}

bool Instruction::IsVulkanStorageTexelBuffer() const {
  if (opcode() != SpvOpTypePointer) {
    return false;
  }

  uint32_t storage_class =
      GetSingleWordInOperand(kPointerTypeStorageClassIndex);
  if (storage_class != SpvStorageClassUniformConstant) {
    return false;
  }

  Instruction* base_type =
      context()->get_def_use_mgr()->GetDef(GetSingleWordInOperand(1));

  // Unpack the optional layer of arraying.
  if (base_type->opcode() == SpvOpTypeArray ||
      base_type->opcode() == SpvOpTypeRuntimeArray) {
    base_type = context()->get_def_use_mgr()->GetDef(
        base_type->GetSingleWordInOperand(0));
  }

  if (base_type->opcode() != SpvOpTypeImage) {
    return false;
  }

  if (base_type->GetSingleWordInOperand(kTypeImageDimIndex) != SpvDimBuffer) {
    return false;
  }

  // Check if the image is sampled.  If we do not know for sure that it is,
  // then assume it is a storage texel buffer.
  return base_type->GetSingleWordInOperand(kTypeImageSampledIndex) != 1;
}

bool Instruction::IsVulkanStorageBuffer() const {
  // Is there a difference between a "Storage buffer" and a "dynamic storage
  // buffer" in SPIR-V and do we care about the difference?
  if (opcode() != SpvOpTypePointer) {
    return false;
  }

  Instruction* base_type =
      context()->get_def_use_mgr()->GetDef(GetSingleWordInOperand(1));

  // Unpack the optional layer of arraying.
  if (base_type->opcode() == SpvOpTypeArray ||
      base_type->opcode() == SpvOpTypeRuntimeArray) {
    base_type = context()->get_def_use_mgr()->GetDef(
        base_type->GetSingleWordInOperand(0));
  }

  if (base_type->opcode() != SpvOpTypeStruct) {
    return false;
  }

  uint32_t storage_class =
      GetSingleWordInOperand(kPointerTypeStorageClassIndex);
  if (storage_class == SpvStorageClassUniform) {
    bool is_buffer_block = false;
    context()->get_decoration_mgr()->ForEachDecoration(
        base_type->result_id(), SpvDecorationBufferBlock,
        [&is_buffer_block](const Instruction&) { is_buffer_block = true; });
    return is_buffer_block;
  } else if (storage_class == SpvStorageClassStorageBuffer) {
    bool is_block = false;
    context()->get_decoration_mgr()->ForEachDecoration(
        base_type->result_id(), SpvDecorationBlock,
        [&is_block](const Instruction&) { is_block = true; });
    return is_block;
  }
  return false;
}

bool Instruction::IsVulkanStorageBufferVariable() const {
  if (opcode() != SpvOpVariable) {
    return false;
  }

  uint32_t storage_class = GetSingleWordInOperand(kVariableStorageClassIndex);
  if (storage_class == SpvStorageClassStorageBuffer ||
      storage_class == SpvStorageClassUniform) {
    Instruction* var_type = context()->get_def_use_mgr()->GetDef(type_id());
    return var_type != nullptr && var_type->IsVulkanStorageBuffer();
  }

  return false;
}

bool Instruction::IsVulkanUniformBuffer() const {
  if (opcode() != SpvOpTypePointer) {
    return false;
  }

  uint32_t storage_class =
      GetSingleWordInOperand(kPointerTypeStorageClassIndex);
  if (storage_class != SpvStorageClassUniform) {
    return false;
  }

  Instruction* base_type =
      context()->get_def_use_mgr()->GetDef(GetSingleWordInOperand(1));

  // Unpack the optional layer of arraying.
  if (base_type->opcode() == SpvOpTypeArray ||
      base_type->opcode() == SpvOpTypeRuntimeArray) {
    base_type = context()->get_def_use_mgr()->GetDef(
        base_type->GetSingleWordInOperand(0));
  }

  if (base_type->opcode() != SpvOpTypeStruct) {
    return false;
  }

  bool is_block = false;
  context()->get_decoration_mgr()->ForEachDecoration(
      base_type->result_id(), SpvDecorationBlock,
      [&is_block](const Instruction&) { is_block = true; });
  return is_block;
}

bool Instruction::IsReadOnlyPointerShaders() const {
  if (type_id() == 0) {
    return false;
  }

  Instruction* type_def = context()->get_def_use_mgr()->GetDef(type_id());
  if (type_def->opcode() != SpvOpTypePointer) {
    return false;
  }

  uint32_t storage_class =
      type_def->GetSingleWordInOperand(kPointerTypeStorageClassIndex);

  switch (storage_class) {
    case SpvStorageClassUniformConstant:
      if (!type_def->IsVulkanStorageImage() &&
          !type_def->IsVulkanStorageTexelBuffer()) {
        return true;
      }
      break;
    case SpvStorageClassUniform:
      if (!type_def->IsVulkanStorageBuffer()) {
        return true;
      }
      break;
    case SpvStorageClassPushConstant:
    case SpvStorageClassInput:
      return true;
    default:
      break;
  }

  bool is_nonwritable = false;
  context()->get_decoration_mgr()->ForEachDecoration(
      result_id(), SpvDecorationNonWritable,
      [&is_nonwritable](const Instruction&) { is_nonwritable = true; });
  return is_nonwritable;
}

bool Instruction::IsReadOnlyPointerKernel() const {
  if (type_id() == 0) {
    return false;
  }

  Instruction* type_def = context()->get_def_use_mgr()->GetDef(type_id());
  if (type_def->opcode() != SpvOpTypePointer) {
    return false;
  }

  uint32_t storage_class =
      type_def->GetSingleWordInOperand(kPointerTypeStorageClassIndex);

  return storage_class == SpvStorageClassUniformConstant;
}

uint32_t Instruction::GetTypeComponent(uint32_t element) const {
  uint32_t subtype = 0;
  switch (opcode()) {
    case SpvOpTypeStruct:
      subtype = GetSingleWordInOperand(element);
      break;
    case SpvOpTypeArray:
    case SpvOpTypeRuntimeArray:
    case SpvOpTypeVector:
    case SpvOpTypeMatrix:
      // These types all have uniform subtypes.
      subtype = GetSingleWordInOperand(0u);
      break;
    default:
      break;
  }

  return subtype;
}

void Instruction::UpdateLexicalScope(uint32_t scope) {
  dbg_scope_.SetLexicalScope(scope);
  for (auto& i : dbg_line_insts_) {
    i.dbg_scope_.SetLexicalScope(scope);
  }
  if (!IsLineInst() &&
      context()->AreAnalysesValid(IRContext::kAnalysisDebugInfo)) {
    context()->get_debug_info_mgr()->AnalyzeDebugInst(this);
  }
}

void Instruction::UpdateDebugInlinedAt(uint32_t new_inlined_at) {
  dbg_scope_.SetInlinedAt(new_inlined_at);
  for (auto& i : dbg_line_insts_) {
    i.dbg_scope_.SetInlinedAt(new_inlined_at);
  }
  if (!IsLineInst() &&
      context()->AreAnalysesValid(IRContext::kAnalysisDebugInfo)) {
    context()->get_debug_info_mgr()->AnalyzeDebugInst(this);
  }
}

void Instruction::ClearDbgLineInsts() {
  if (context()->AreAnalysesValid(IRContext::kAnalysisDefUse)) {
    auto def_use_mgr = context()->get_def_use_mgr();
    for (auto& l_inst : dbg_line_insts_) def_use_mgr->ClearInst(&l_inst);
  }
  clear_dbg_line_insts();
}

void Instruction::UpdateDebugInfoFrom(const Instruction* from) {
  if (from == nullptr) return;
  ClearDbgLineInsts();
  if (!from->dbg_line_insts().empty())
    AddDebugLine(&from->dbg_line_insts().back());
  SetDebugScope(from->GetDebugScope());
  if (!IsLineInst() &&
      context()->AreAnalysesValid(IRContext::kAnalysisDebugInfo)) {
    context()->get_debug_info_mgr()->AnalyzeDebugInst(this);
  }
}

void Instruction::AddDebugLine(const Instruction* inst) {
  dbg_line_insts_.push_back(*inst);
  dbg_line_insts_.back().unique_id_ = context()->TakeNextUniqueId();
  if (inst->IsDebugLineInst())
    dbg_line_insts_.back().SetResultId(context_->TakeNextId());
  if (context()->AreAnalysesValid(IRContext::kAnalysisDefUse))
    context()->get_def_use_mgr()->AnalyzeInstDefUse(&dbg_line_insts_.back());
}

bool Instruction::IsDebugLineInst() const {
  NonSemanticShaderDebugInfo100Instructions ext_opt = GetShader100DebugOpcode();
  return ((ext_opt == NonSemanticShaderDebugInfo100DebugLine) ||
          (ext_opt == NonSemanticShaderDebugInfo100DebugNoLine));
}

bool Instruction::IsLineInst() const { return IsLine() || IsNoLine(); }

bool Instruction::IsLine() const {
  if (opcode() == SpvOpLine) return true;
  NonSemanticShaderDebugInfo100Instructions ext_opt = GetShader100DebugOpcode();
  return ext_opt == NonSemanticShaderDebugInfo100DebugLine;
}

bool Instruction::IsNoLine() const {
  if (opcode() == SpvOpNoLine) return true;
  NonSemanticShaderDebugInfo100Instructions ext_opt = GetShader100DebugOpcode();
  return ext_opt == NonSemanticShaderDebugInfo100DebugNoLine;
}

Instruction* Instruction::InsertBefore(std::unique_ptr<Instruction>&& inst) {
  inst.get()->InsertBefore(this);
  return inst.release();
}

Instruction* Instruction::InsertBefore(
    std::vector<std::unique_ptr<Instruction>>&& list) {
  Instruction* first_node = list.front().get();
  for (auto& inst : list) {
    inst.release()->InsertBefore(this);
  }
  list.clear();
  return first_node;
}

bool Instruction::IsValidBasePointer() const {
  uint32_t tid = type_id();
  if (tid == 0) {
    return false;
  }

  Instruction* type = context()->get_def_use_mgr()->GetDef(tid);
  if (type->opcode() != SpvOpTypePointer) {
    return false;
  }

  auto feature_mgr = context()->get_feature_mgr();
  if (feature_mgr->HasCapability(SpvCapabilityAddresses)) {
    // TODO: The rules here could be more restrictive.
    return true;
  }

  if (opcode() == SpvOpVariable || opcode() == SpvOpFunctionParameter) {
    return true;
  }

  // With variable pointers, there are more valid base pointer objects.
  // Variable pointers implicitly declares Variable pointers storage buffer.
  SpvStorageClass storage_class =
      static_cast<SpvStorageClass>(type->GetSingleWordInOperand(0));
  if ((feature_mgr->HasCapability(SpvCapabilityVariablePointersStorageBuffer) &&
       storage_class == SpvStorageClassStorageBuffer) ||
      (feature_mgr->HasCapability(SpvCapabilityVariablePointers) &&
       storage_class == SpvStorageClassWorkgroup)) {
    switch (opcode()) {
      case SpvOpPhi:
      case SpvOpSelect:
      case SpvOpFunctionCall:
      case SpvOpConstantNull:
        return true;
      default:
        break;
    }
  }

  uint32_t pointee_type_id = type->GetSingleWordInOperand(1);
  Instruction* pointee_type_inst =
      context()->get_def_use_mgr()->GetDef(pointee_type_id);

  if (pointee_type_inst->IsOpaqueType()) {
    return true;
  }
  return false;
}

OpenCLDebugInfo100Instructions Instruction::GetOpenCL100DebugOpcode() const {
  if (opcode() != SpvOpExtInst) {
    return OpenCLDebugInfo100InstructionsMax;
  }

  if (!context()->get_feature_mgr()->GetExtInstImportId_OpenCL100DebugInfo()) {
    return OpenCLDebugInfo100InstructionsMax;
  }

  if (GetSingleWordInOperand(kExtInstSetIdInIdx) !=
      context()->get_feature_mgr()->GetExtInstImportId_OpenCL100DebugInfo()) {
    return OpenCLDebugInfo100InstructionsMax;
  }

  return OpenCLDebugInfo100Instructions(
      GetSingleWordInOperand(kExtInstInstructionInIdx));
}

NonSemanticShaderDebugInfo100Instructions Instruction::GetShader100DebugOpcode()
    const {
  if (opcode() != SpvOpExtInst) {
    return NonSemanticShaderDebugInfo100InstructionsMax;
  }

  if (!context()->get_feature_mgr()->GetExtInstImportId_Shader100DebugInfo()) {
    return NonSemanticShaderDebugInfo100InstructionsMax;
  }

  if (GetSingleWordInOperand(kExtInstSetIdInIdx) !=
      context()->get_feature_mgr()->GetExtInstImportId_Shader100DebugInfo()) {
    return NonSemanticShaderDebugInfo100InstructionsMax;
  }

  return NonSemanticShaderDebugInfo100Instructions(
      GetSingleWordInOperand(kExtInstInstructionInIdx));
}

CommonDebugInfoInstructions Instruction::GetCommonDebugOpcode() const {
  if (opcode() != SpvOpExtInst) {
    return CommonDebugInfoInstructionsMax;
  }

  const uint32_t opencl_set_id =
      context()->get_feature_mgr()->GetExtInstImportId_OpenCL100DebugInfo();
  const uint32_t shader_set_id =
      context()->get_feature_mgr()->GetExtInstImportId_Shader100DebugInfo();

  if (!opencl_set_id && !shader_set_id) {
    return CommonDebugInfoInstructionsMax;
  }

  const uint32_t used_set_id = GetSingleWordInOperand(kExtInstSetIdInIdx);

  if (used_set_id != opencl_set_id && used_set_id != shader_set_id) {
    return CommonDebugInfoInstructionsMax;
  }

  return CommonDebugInfoInstructions(
      GetSingleWordInOperand(kExtInstInstructionInIdx));
}

bool Instruction::IsValidBaseImage() const {
  uint32_t tid = type_id();
  if (tid == 0) {
    return false;
  }

  Instruction* type = context()->get_def_use_mgr()->GetDef(tid);
  return (type->opcode() == SpvOpTypeImage ||
          type->opcode() == SpvOpTypeSampledImage);
}

bool Instruction::IsOpaqueType() const {
  if (opcode() == SpvOpTypeStruct) {
    bool is_opaque = false;
    ForEachInOperand([&is_opaque, this](const uint32_t* op_id) {
      Instruction* type_inst = context()->get_def_use_mgr()->GetDef(*op_id);
      is_opaque |= type_inst->IsOpaqueType();
    });
    return is_opaque;
  } else if (opcode() == SpvOpTypeArray) {
    uint32_t sub_type_id = GetSingleWordInOperand(0);
    Instruction* sub_type_inst =
        context()->get_def_use_mgr()->GetDef(sub_type_id);
    return sub_type_inst->IsOpaqueType();
  } else {
    return opcode() == SpvOpTypeRuntimeArray ||
           spvOpcodeIsBaseOpaqueType(opcode());
  }
}

bool Instruction::IsFoldable() const {
  return IsFoldableByFoldScalar() ||
         context()->get_instruction_folder().HasConstFoldingRule(this);
}

bool Instruction::IsFoldableByFoldScalar() const {
  const InstructionFolder& folder = context()->get_instruction_folder();
  if (!folder.IsFoldableOpcode(opcode())) {
    return false;
  }

  Instruction* type = context()->get_def_use_mgr()->GetDef(type_id());
  if (!folder.IsFoldableType(type)) {
    return false;
  }

  // Even if the type of the instruction is foldable, its operands may not be
  // foldable (e.g., comparisons of 64bit types).  Check that all operand types
  // are foldable before accepting the instruction.
  return WhileEachInOperand([&folder, this](const uint32_t* op_id) {
    Instruction* def_inst = context()->get_def_use_mgr()->GetDef(*op_id);
    Instruction* def_inst_type =
        context()->get_def_use_mgr()->GetDef(def_inst->type_id());
    return folder.IsFoldableType(def_inst_type);
  });
}

bool Instruction::IsFloatingPointFoldingAllowed() const {
  // TODO: Add the rules for kernels.  For now it will be pessimistic.
  // For now, do not support capabilities introduced by SPV_KHR_float_controls.
  if (!context_->get_feature_mgr()->HasCapability(SpvCapabilityShader) ||
      context_->get_feature_mgr()->HasCapability(SpvCapabilityDenormPreserve) ||
      context_->get_feature_mgr()->HasCapability(
          SpvCapabilityDenormFlushToZero) ||
      context_->get_feature_mgr()->HasCapability(
          SpvCapabilitySignedZeroInfNanPreserve) ||
      context_->get_feature_mgr()->HasCapability(
          SpvCapabilityRoundingModeRTZ) ||
      context_->get_feature_mgr()->HasCapability(
          SpvCapabilityRoundingModeRTE)) {
    return false;
  }

  bool is_nocontract = false;
  context_->get_decoration_mgr()->WhileEachDecoration(
      result_id(), SpvDecorationNoContraction,
      [&is_nocontract](const Instruction&) {
        is_nocontract = true;
        return false;
      });
  return !is_nocontract;
}

std::string Instruction::PrettyPrint(uint32_t options) const {
  // Convert the module to binary.
  std::vector<uint32_t> module_binary;
  context()->module()->ToBinary(&module_binary, /* skip_nop = */ false);

  // Convert the instruction to binary. This is used to identify the correct
  // stream of words to output from the module.
  std::vector<uint32_t> inst_binary;
  ToBinaryWithoutAttachedDebugInsts(&inst_binary);

  // Do not generate a header.
  return spvInstructionBinaryToText(
      context()->grammar().target_env(), inst_binary.data(), inst_binary.size(),
      module_binary.data(), module_binary.size(),
      options | SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
}

std::ostream& operator<<(std::ostream& str, const Instruction& inst) {
  str << inst.PrettyPrint();
  return str;
}

void Instruction::Dump() const {
  std::cerr << "Instruction #" << unique_id() << "\n" << *this << "\n";
}

bool Instruction::IsOpcodeCodeMotionSafe() const {
  switch (opcode_) {
    case SpvOpNop:
    case SpvOpUndef:
    case SpvOpLoad:
    case SpvOpAccessChain:
    case SpvOpInBoundsAccessChain:
    case SpvOpArrayLength:
    case SpvOpVectorExtractDynamic:
    case SpvOpVectorInsertDynamic:
    case SpvOpVectorShuffle:
    case SpvOpCompositeConstruct:
    case SpvOpCompositeExtract:
    case SpvOpCompositeInsert:
    case SpvOpCopyObject:
    case SpvOpTranspose:
    case SpvOpConvertFToU:
    case SpvOpConvertFToS:
    case SpvOpConvertSToF:
    case SpvOpConvertUToF:
    case SpvOpUConvert:
    case SpvOpSConvert:
    case SpvOpFConvert:
    case SpvOpQuantizeToF16:
    case SpvOpBitcast:
    case SpvOpSNegate:
    case SpvOpFNegate:
    case SpvOpIAdd:
    case SpvOpFAdd:
    case SpvOpISub:
    case SpvOpFSub:
    case SpvOpIMul:
    case SpvOpFMul:
    case SpvOpUDiv:
    case SpvOpSDiv:
    case SpvOpFDiv:
    case SpvOpUMod:
    case SpvOpSRem:
    case SpvOpSMod:
    case SpvOpFRem:
    case SpvOpFMod:
    case SpvOpVectorTimesScalar:
    case SpvOpMatrixTimesScalar:
    case SpvOpVectorTimesMatrix:
    case SpvOpMatrixTimesVector:
    case SpvOpMatrixTimesMatrix:
    case SpvOpOuterProduct:
    case SpvOpDot:
    case SpvOpIAddCarry:
    case SpvOpISubBorrow:
    case SpvOpUMulExtended:
    case SpvOpSMulExtended:
    case SpvOpAny:
    case SpvOpAll:
    case SpvOpIsNan:
    case SpvOpIsInf:
    case SpvOpLogicalEqual:
    case SpvOpLogicalNotEqual:
    case SpvOpLogicalOr:
    case SpvOpLogicalAnd:
    case SpvOpLogicalNot:
    case SpvOpSelect:
    case SpvOpIEqual:
    case SpvOpINotEqual:
    case SpvOpUGreaterThan:
    case SpvOpSGreaterThan:
    case SpvOpUGreaterThanEqual:
    case SpvOpSGreaterThanEqual:
    case SpvOpULessThan:
    case SpvOpSLessThan:
    case SpvOpULessThanEqual:
    case SpvOpSLessThanEqual:
    case SpvOpFOrdEqual:
    case SpvOpFUnordEqual:
    case SpvOpFOrdNotEqual:
    case SpvOpFUnordNotEqual:
    case SpvOpFOrdLessThan:
    case SpvOpFUnordLessThan:
    case SpvOpFOrdGreaterThan:
    case SpvOpFUnordGreaterThan:
    case SpvOpFOrdLessThanEqual:
    case SpvOpFUnordLessThanEqual:
    case SpvOpFOrdGreaterThanEqual:
    case SpvOpFUnordGreaterThanEqual:
    case SpvOpShiftRightLogical:
    case SpvOpShiftRightArithmetic:
    case SpvOpShiftLeftLogical:
    case SpvOpBitwiseOr:
    case SpvOpBitwiseXor:
    case SpvOpBitwiseAnd:
    case SpvOpNot:
    case SpvOpBitFieldInsert:
    case SpvOpBitFieldSExtract:
    case SpvOpBitFieldUExtract:
    case SpvOpBitReverse:
    case SpvOpBitCount:
    case SpvOpSizeOf:
      return true;
    default:
      return false;
  }
}

bool Instruction::IsScalarizable() const {
  if (spvOpcodeIsScalarizable(opcode())) {
    return true;
  }

  if (opcode() == SpvOpExtInst) {
    uint32_t instSetId =
        context()->get_feature_mgr()->GetExtInstImportId_GLSLstd450();

    if (GetSingleWordInOperand(kExtInstSetIdInIdx) == instSetId) {
      switch (GetSingleWordInOperand(kExtInstInstructionInIdx)) {
        case GLSLstd450Round:
        case GLSLstd450RoundEven:
        case GLSLstd450Trunc:
        case GLSLstd450FAbs:
        case GLSLstd450SAbs:
        case GLSLstd450FSign:
        case GLSLstd450SSign:
        case GLSLstd450Floor:
        case GLSLstd450Ceil:
        case GLSLstd450Fract:
        case GLSLstd450Radians:
        case GLSLstd450Degrees:
        case GLSLstd450Sin:
        case GLSLstd450Cos:
        case GLSLstd450Tan:
        case GLSLstd450Asin:
        case GLSLstd450Acos:
        case GLSLstd450Atan:
        case GLSLstd450Sinh:
        case GLSLstd450Cosh:
        case GLSLstd450Tanh:
        case GLSLstd450Asinh:
        case GLSLstd450Acosh:
        case GLSLstd450Atanh:
        case GLSLstd450Atan2:
        case GLSLstd450Pow:
        case GLSLstd450Exp:
        case GLSLstd450Log:
        case GLSLstd450Exp2:
        case GLSLstd450Log2:
        case GLSLstd450Sqrt:
        case GLSLstd450InverseSqrt:
        case GLSLstd450Modf:
        case GLSLstd450FMin:
        case GLSLstd450UMin:
        case GLSLstd450SMin:
        case GLSLstd450FMax:
        case GLSLstd450UMax:
        case GLSLstd450SMax:
        case GLSLstd450FClamp:
        case GLSLstd450UClamp:
        case GLSLstd450SClamp:
        case GLSLstd450FMix:
        case GLSLstd450Step:
        case GLSLstd450SmoothStep:
        case GLSLstd450Fma:
        case GLSLstd450Frexp:
        case GLSLstd450Ldexp:
        case GLSLstd450FindILsb:
        case GLSLstd450FindSMsb:
        case GLSLstd450FindUMsb:
        case GLSLstd450NMin:
        case GLSLstd450NMax:
        case GLSLstd450NClamp:
          return true;
        default:
          return false;
      }
    }
  }
  return false;
}

bool Instruction::IsOpcodeSafeToDelete() const {
  if (context()->IsCombinatorInstruction(this)) {
    return true;
  }

  switch (opcode()) {
    case SpvOpDPdx:
    case SpvOpDPdy:
    case SpvOpFwidth:
    case SpvOpDPdxFine:
    case SpvOpDPdyFine:
    case SpvOpFwidthFine:
    case SpvOpDPdxCoarse:
    case SpvOpDPdyCoarse:
    case SpvOpFwidthCoarse:
    case SpvOpImageQueryLod:
      return true;
    default:
      return false;
  }
}

bool Instruction::IsNonSemanticInstruction() const {
  if (!HasResultId()) return false;
  if (opcode() != SpvOpExtInst) return false;

  auto import_inst =
      context()->get_def_use_mgr()->GetDef(GetSingleWordInOperand(0));
  std::string import_name = import_inst->GetInOperand(0).AsString();
  return import_name.find("NonSemantic.") == 0;
}

void DebugScope::ToBinary(uint32_t type_id, uint32_t result_id,
                          uint32_t ext_set,
                          std::vector<uint32_t>* binary) const {
  uint32_t num_words = kDebugScopeNumWords;
  CommonDebugInfoInstructions dbg_opcode = CommonDebugInfoDebugScope;
  if (GetLexicalScope() == kNoDebugScope) {
    num_words = kDebugNoScopeNumWords;
    dbg_opcode = CommonDebugInfoDebugNoScope;
  } else if (GetInlinedAt() == kNoInlinedAt) {
    num_words = kDebugScopeNumWordsWithoutInlinedAt;
  }
  std::vector<uint32_t> operands = {
      (num_words << 16) | static_cast<uint16_t>(SpvOpExtInst),
      type_id,
      result_id,
      ext_set,
      static_cast<uint32_t>(dbg_opcode),
  };
  binary->insert(binary->end(), operands.begin(), operands.end());
  if (GetLexicalScope() != kNoDebugScope) {
    binary->push_back(GetLexicalScope());
    if (GetInlinedAt() != kNoInlinedAt) binary->push_back(GetInlinedAt());
  }
}

}  // namespace opt
}  // namespace spvtools
