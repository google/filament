// Copyright (c) 2018 The Khronos Group Inc.
// Copyright (c) 2018 Valve Corporation
// Copyright (c) 2018 LunarG Inc.
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

#include "inst_bindless_check_pass.h"

namespace {

// Input Operand Indices
static const int kSpvImageSampleImageIdInIdx = 0;
static const int kSpvSampledImageImageIdInIdx = 0;
static const int kSpvSampledImageSamplerIdInIdx = 1;
static const int kSpvImageSampledImageIdInIdx = 0;
static const int kSpvLoadPtrIdInIdx = 0;
static const int kSpvAccessChainBaseIdInIdx = 0;
static const int kSpvAccessChainIndex0IdInIdx = 1;
static const int kSpvTypeArrayLengthIdInIdx = 1;
static const int kSpvConstantValueInIdx = 0;
static const int kSpvVariableStorageClassInIdx = 0;

}  // anonymous namespace

// Avoid unused variable warning/error on Linux
#ifndef NDEBUG
#define USE_ASSERT(x) assert(x)
#else
#define USE_ASSERT(x) ((void)(x))
#endif

namespace spvtools {
namespace opt {

uint32_t InstBindlessCheckPass::GenDebugReadLength(
    uint32_t var_id, InstructionBuilder* builder) {
  uint32_t desc_set_idx =
      var2desc_set_[var_id] + kDebugInputBindlessOffsetLengths;
  uint32_t desc_set_idx_id = builder->GetUintConstantId(desc_set_idx);
  uint32_t binding_idx_id = builder->GetUintConstantId(var2binding_[var_id]);
  return GenDebugDirectRead({desc_set_idx_id, binding_idx_id}, builder);
}

uint32_t InstBindlessCheckPass::GenDebugReadInit(uint32_t var_id,
                                                 uint32_t desc_idx_id,
                                                 InstructionBuilder* builder) {
  uint32_t binding_idx_id = builder->GetUintConstantId(var2binding_[var_id]);
  uint32_t u_desc_idx_id = GenUintCastCode(desc_idx_id, builder);
  // If desc index checking is not enabled, we know the offset of initialization
  // entries is 1, so we can avoid loading this value and just add 1 to the
  // descriptor set.
  if (!desc_idx_enabled_) {
    uint32_t desc_set_idx_id =
        builder->GetUintConstantId(var2desc_set_[var_id] + 1);
    return GenDebugDirectRead({desc_set_idx_id, binding_idx_id, u_desc_idx_id},
                              builder);
  } else {
    uint32_t desc_set_base_id =
        builder->GetUintConstantId(kDebugInputBindlessInitOffset);
    uint32_t desc_set_idx_id =
        builder->GetUintConstantId(var2desc_set_[var_id]);
    return GenDebugDirectRead(
        {desc_set_base_id, desc_set_idx_id, binding_idx_id, u_desc_idx_id},
        builder);
  }
}

uint32_t InstBindlessCheckPass::CloneOriginalReference(
    ref_analysis* ref, InstructionBuilder* builder) {
  // If original is image based, start by cloning descriptor load
  uint32_t new_image_id = 0;
  if (ref->desc_load_id != 0) {
    Instruction* desc_load_inst = get_def_use_mgr()->GetDef(ref->desc_load_id);
    Instruction* new_load_inst = builder->AddLoad(
        desc_load_inst->type_id(),
        desc_load_inst->GetSingleWordInOperand(kSpvLoadPtrIdInIdx));
    uid2offset_[new_load_inst->unique_id()] =
        uid2offset_[desc_load_inst->unique_id()];
    uint32_t new_load_id = new_load_inst->result_id();
    get_decoration_mgr()->CloneDecorations(desc_load_inst->result_id(),
                                           new_load_id);
    new_image_id = new_load_id;
    // Clone Image/SampledImage with new load, if needed
    if (ref->image_id != 0) {
      Instruction* image_inst = get_def_use_mgr()->GetDef(ref->image_id);
      if (image_inst->opcode() == SpvOp::SpvOpSampledImage) {
        Instruction* new_image_inst = builder->AddBinaryOp(
            image_inst->type_id(), SpvOpSampledImage, new_load_id,
            image_inst->GetSingleWordInOperand(kSpvSampledImageSamplerIdInIdx));
        uid2offset_[new_image_inst->unique_id()] =
            uid2offset_[image_inst->unique_id()];
        new_image_id = new_image_inst->result_id();
      } else {
        assert(image_inst->opcode() == SpvOp::SpvOpImage &&
               "expecting OpImage");
        Instruction* new_image_inst =
            builder->AddUnaryOp(image_inst->type_id(), SpvOpImage, new_load_id);
        uid2offset_[new_image_inst->unique_id()] =
            uid2offset_[image_inst->unique_id()];
        new_image_id = new_image_inst->result_id();
      }
      get_decoration_mgr()->CloneDecorations(ref->image_id, new_image_id);
    }
  }
  // Clone original reference
  std::unique_ptr<Instruction> new_ref_inst(ref->ref_inst->Clone(context()));
  uint32_t ref_result_id = ref->ref_inst->result_id();
  uint32_t new_ref_id = 0;
  if (ref_result_id != 0) {
    new_ref_id = TakeNextId();
    new_ref_inst->SetResultId(new_ref_id);
  }
  // Update new ref with new image if created
  if (new_image_id != 0)
    new_ref_inst->SetInOperand(kSpvImageSampleImageIdInIdx, {new_image_id});
  // Register new reference and add to new block
  Instruction* added_inst = builder->AddInstruction(std::move(new_ref_inst));
  uid2offset_[added_inst->unique_id()] =
      uid2offset_[ref->ref_inst->unique_id()];
  if (new_ref_id != 0)
    get_decoration_mgr()->CloneDecorations(ref_result_id, new_ref_id);
  return new_ref_id;
}

uint32_t InstBindlessCheckPass::GetImageId(Instruction* inst) {
  switch (inst->opcode()) {
    case SpvOp::SpvOpImageSampleImplicitLod:
    case SpvOp::SpvOpImageSampleExplicitLod:
    case SpvOp::SpvOpImageSampleDrefImplicitLod:
    case SpvOp::SpvOpImageSampleDrefExplicitLod:
    case SpvOp::SpvOpImageSampleProjImplicitLod:
    case SpvOp::SpvOpImageSampleProjExplicitLod:
    case SpvOp::SpvOpImageSampleProjDrefImplicitLod:
    case SpvOp::SpvOpImageSampleProjDrefExplicitLod:
    case SpvOp::SpvOpImageGather:
    case SpvOp::SpvOpImageDrefGather:
    case SpvOp::SpvOpImageQueryLod:
    case SpvOp::SpvOpImageSparseSampleImplicitLod:
    case SpvOp::SpvOpImageSparseSampleExplicitLod:
    case SpvOp::SpvOpImageSparseSampleDrefImplicitLod:
    case SpvOp::SpvOpImageSparseSampleDrefExplicitLod:
    case SpvOp::SpvOpImageSparseSampleProjImplicitLod:
    case SpvOp::SpvOpImageSparseSampleProjExplicitLod:
    case SpvOp::SpvOpImageSparseSampleProjDrefImplicitLod:
    case SpvOp::SpvOpImageSparseSampleProjDrefExplicitLod:
    case SpvOp::SpvOpImageSparseGather:
    case SpvOp::SpvOpImageSparseDrefGather:
    case SpvOp::SpvOpImageFetch:
    case SpvOp::SpvOpImageRead:
    case SpvOp::SpvOpImageQueryFormat:
    case SpvOp::SpvOpImageQueryOrder:
    case SpvOp::SpvOpImageQuerySizeLod:
    case SpvOp::SpvOpImageQuerySize:
    case SpvOp::SpvOpImageQueryLevels:
    case SpvOp::SpvOpImageQuerySamples:
    case SpvOp::SpvOpImageSparseFetch:
    case SpvOp::SpvOpImageSparseRead:
    case SpvOp::SpvOpImageWrite:
      return inst->GetSingleWordInOperand(kSpvImageSampleImageIdInIdx);
    default:
      break;
  }
  return 0;
}

Instruction* InstBindlessCheckPass::GetPointeeTypeInst(Instruction* ptr_inst) {
  uint32_t pte_ty_id = GetPointeeTypeId(ptr_inst);
  return get_def_use_mgr()->GetDef(pte_ty_id);
}

bool InstBindlessCheckPass::AnalyzeDescriptorReference(Instruction* ref_inst,
                                                       ref_analysis* ref) {
  ref->ref_inst = ref_inst;
  if (ref_inst->opcode() == SpvOpLoad || ref_inst->opcode() == SpvOpStore) {
    ref->desc_load_id = 0;
    ref->ptr_id = ref_inst->GetSingleWordInOperand(kSpvLoadPtrIdInIdx);
    Instruction* ptr_inst = get_def_use_mgr()->GetDef(ref->ptr_id);
    if (ptr_inst->opcode() != SpvOp::SpvOpAccessChain) return false;
    ref->var_id = ptr_inst->GetSingleWordInOperand(kSpvAccessChainBaseIdInIdx);
    Instruction* var_inst = get_def_use_mgr()->GetDef(ref->var_id);
    if (var_inst->opcode() != SpvOp::SpvOpVariable) return false;
    uint32_t storage_class =
        var_inst->GetSingleWordInOperand(kSpvVariableStorageClassInIdx);
    switch (storage_class) {
      case SpvStorageClassUniform:
      case SpvStorageClassUniformConstant:
      case SpvStorageClassStorageBuffer:
        break;
      default:
        return false;
        break;
    }
    Instruction* desc_type_inst = GetPointeeTypeInst(var_inst);
    switch (desc_type_inst->opcode()) {
      case SpvOpTypeArray:
      case SpvOpTypeRuntimeArray:
        // A load through a descriptor array will have at least 3 operands. We
        // do not want to instrument loads of descriptors here which are part of
        // an image-based reference.
        if (ptr_inst->NumInOperands() < 3) return false;
        ref->desc_idx_id =
            ptr_inst->GetSingleWordInOperand(kSpvAccessChainIndex0IdInIdx);
        break;
      default:
        ref->desc_idx_id = 0;
        break;
    }
    return true;
  }
  // Reference is not load or store. If not an image-based reference, return.
  ref->image_id = GetImageId(ref_inst);
  if (ref->image_id == 0) return false;
  Instruction* image_inst = get_def_use_mgr()->GetDef(ref->image_id);
  Instruction* desc_load_inst = nullptr;
  if (image_inst->opcode() == SpvOp::SpvOpSampledImage) {
    ref->desc_load_id =
        image_inst->GetSingleWordInOperand(kSpvSampledImageImageIdInIdx);
    desc_load_inst = get_def_use_mgr()->GetDef(ref->desc_load_id);
  } else if (image_inst->opcode() == SpvOp::SpvOpImage) {
    ref->desc_load_id =
        image_inst->GetSingleWordInOperand(kSpvImageSampledImageIdInIdx);
    desc_load_inst = get_def_use_mgr()->GetDef(ref->desc_load_id);
  } else {
    ref->desc_load_id = ref->image_id;
    desc_load_inst = image_inst;
    ref->image_id = 0;
  }
  if (desc_load_inst->opcode() != SpvOp::SpvOpLoad) {
    // TODO(greg-lunarg): Handle additional possibilities?
    return false;
  }
  ref->ptr_id = desc_load_inst->GetSingleWordInOperand(kSpvLoadPtrIdInIdx);
  Instruction* ptr_inst = get_def_use_mgr()->GetDef(ref->ptr_id);
  if (ptr_inst->opcode() == SpvOp::SpvOpVariable) {
    ref->desc_idx_id = 0;
    ref->var_id = ref->ptr_id;
  } else if (ptr_inst->opcode() == SpvOp::SpvOpAccessChain) {
    if (ptr_inst->NumInOperands() != 2) {
      assert(false && "unexpected bindless index number");
      return false;
    }
    ref->desc_idx_id =
        ptr_inst->GetSingleWordInOperand(kSpvAccessChainIndex0IdInIdx);
    ref->var_id = ptr_inst->GetSingleWordInOperand(kSpvAccessChainBaseIdInIdx);
    Instruction* var_inst = get_def_use_mgr()->GetDef(ref->var_id);
    if (var_inst->opcode() != SpvOpVariable) {
      assert(false && "unexpected bindless base");
      return false;
    }
  } else {
    // TODO(greg-lunarg): Handle additional possibilities?
    return false;
  }
  return true;
}

uint32_t InstBindlessCheckPass::FindStride(uint32_t ty_id,
                                           uint32_t stride_deco) {
  uint32_t stride = 0xdeadbeef;
  bool found = !get_decoration_mgr()->WhileEachDecoration(
      ty_id, stride_deco, [&stride](const Instruction& deco_inst) {
        stride = deco_inst.GetSingleWordInOperand(2u);
        return false;
      });
  USE_ASSERT(found && "stride not found");
  return stride;
}

uint32_t InstBindlessCheckPass::ByteSize(uint32_t ty_id) {
  analysis::TypeManager* type_mgr = context()->get_type_mgr();
  const analysis::Type* sz_ty = type_mgr->GetType(ty_id);
  if (sz_ty->kind() == analysis::Type::kPointer) {
    // Assuming PhysicalStorageBuffer pointer
    return 8;
  }
  uint32_t size = 1;
  if (sz_ty->kind() == analysis::Type::kMatrix) {
    const analysis::Matrix* m_ty = sz_ty->AsMatrix();
    size = m_ty->element_count() * size;
    uint32_t stride = FindStride(ty_id, SpvDecorationMatrixStride);
    if (stride != 0) return size * stride;
    sz_ty = m_ty->element_type();
  }
  if (sz_ty->kind() == analysis::Type::kVector) {
    const analysis::Vector* v_ty = sz_ty->AsVector();
    size = v_ty->element_count() * size;
    sz_ty = v_ty->element_type();
  }
  switch (sz_ty->kind()) {
    case analysis::Type::kFloat: {
      const analysis::Float* f_ty = sz_ty->AsFloat();
      size *= f_ty->width();
    } break;
    case analysis::Type::kInteger: {
      const analysis::Integer* i_ty = sz_ty->AsInteger();
      size *= i_ty->width();
    } break;
    default: { assert(false && "unexpected type"); } break;
  }
  size /= 8;
  return size;
}

uint32_t InstBindlessCheckPass::GenLastByteIdx(ref_analysis* ref,
                                               InstructionBuilder* builder) {
  // Find outermost buffer type and its access chain index
  Instruction* var_inst = get_def_use_mgr()->GetDef(ref->var_id);
  Instruction* desc_ty_inst = GetPointeeTypeInst(var_inst);
  uint32_t buff_ty_id;
  uint32_t ac_in_idx = 1;
  switch (desc_ty_inst->opcode()) {
    case SpvOpTypeArray:
    case SpvOpTypeRuntimeArray:
      buff_ty_id = desc_ty_inst->GetSingleWordInOperand(0);
      ++ac_in_idx;
      break;
    default:
      assert(desc_ty_inst->opcode() == SpvOpTypeStruct &&
             "unexpected descriptor type");
      buff_ty_id = desc_ty_inst->result_id();
      break;
  }
  // Process remaining access chain indices
  Instruction* ac_inst = get_def_use_mgr()->GetDef(ref->ptr_id);
  uint32_t curr_ty_id = buff_ty_id;
  uint32_t sum_id = 0;
  while (ac_in_idx < ac_inst->NumInOperands()) {
    uint32_t curr_idx_id = ac_inst->GetSingleWordInOperand(ac_in_idx);
    Instruction* curr_idx_inst = get_def_use_mgr()->GetDef(curr_idx_id);
    Instruction* curr_ty_inst = get_def_use_mgr()->GetDef(curr_ty_id);
    uint32_t curr_offset_id = 0;
    switch (curr_ty_inst->opcode()) {
      case SpvOpTypeArray:
      case SpvOpTypeRuntimeArray:
      case SpvOpTypeMatrix: {
        // Get array/matrix stride and multiply by current index
        uint32_t stride_deco = (curr_ty_inst->opcode() == SpvOpTypeMatrix)
                                   ? SpvDecorationMatrixStride
                                   : SpvDecorationArrayStride;
        uint32_t arr_stride = FindStride(curr_ty_id, stride_deco);
        uint32_t arr_stride_id = builder->GetUintConstantId(arr_stride);
        uint32_t curr_idx_32b_id = Gen32BitCvtCode(curr_idx_id, builder);
        Instruction* curr_offset_inst = builder->AddBinaryOp(
            GetUintId(), SpvOpIMul, arr_stride_id, curr_idx_32b_id);
        curr_offset_id = curr_offset_inst->result_id();
        // Get element type for next step
        curr_ty_id = curr_ty_inst->GetSingleWordInOperand(0);
      } break;
      case SpvOpTypeVector: {
        // Stride is size of component type
        uint32_t comp_ty_id = curr_ty_inst->GetSingleWordInOperand(0u);
        uint32_t vec_stride = ByteSize(comp_ty_id);
        uint32_t vec_stride_id = builder->GetUintConstantId(vec_stride);
        uint32_t curr_idx_32b_id = Gen32BitCvtCode(curr_idx_id, builder);
        Instruction* curr_offset_inst = builder->AddBinaryOp(
            GetUintId(), SpvOpIMul, vec_stride_id, curr_idx_32b_id);
        curr_offset_id = curr_offset_inst->result_id();
        // Get element type for next step
        curr_ty_id = comp_ty_id;
      } break;
      case SpvOpTypeStruct: {
        // Get buffer byte offset for the referenced member
        assert(curr_idx_inst->opcode() == SpvOpConstant &&
               "unexpected struct index");
        uint32_t member_idx = curr_idx_inst->GetSingleWordInOperand(0);
        uint32_t member_offset = 0xdeadbeef;
        bool found = !get_decoration_mgr()->WhileEachDecoration(
            curr_ty_id, SpvDecorationOffset,
            [&member_idx, &member_offset](const Instruction& deco_inst) {
              if (deco_inst.GetSingleWordInOperand(1u) != member_idx)
                return true;
              member_offset = deco_inst.GetSingleWordInOperand(3u);
              return false;
            });
        USE_ASSERT(found && "member offset not found");
        curr_offset_id = builder->GetUintConstantId(member_offset);
        // Get element type for next step
        curr_ty_id = curr_ty_inst->GetSingleWordInOperand(member_idx);
      } break;
      default: { assert(false && "unexpected non-composite type"); } break;
    }
    if (sum_id == 0)
      sum_id = curr_offset_id;
    else {
      Instruction* sum_inst =
          builder->AddBinaryOp(GetUintId(), SpvOpIAdd, sum_id, curr_offset_id);
      sum_id = sum_inst->result_id();
    }
    ++ac_in_idx;
  }
  // Add in offset of last byte of referenced object
  uint32_t bsize = ByteSize(curr_ty_id);
  uint32_t last = bsize - 1;
  uint32_t last_id = builder->GetUintConstantId(last);
  Instruction* sum_inst =
      builder->AddBinaryOp(GetUintId(), SpvOpIAdd, sum_id, last_id);
  return sum_inst->result_id();
}

void InstBindlessCheckPass::GenCheckCode(
    uint32_t check_id, uint32_t error_id, uint32_t offset_id,
    uint32_t length_id, uint32_t stage_idx, ref_analysis* ref,
    std::vector<std::unique_ptr<BasicBlock>>* new_blocks) {
  BasicBlock* back_blk_ptr = &*new_blocks->back();
  InstructionBuilder builder(
      context(), back_blk_ptr,
      IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);
  // Gen conditional branch on check_id. Valid branch generates original
  // reference. Invalid generates debug output and zero result (if needed).
  uint32_t merge_blk_id = TakeNextId();
  uint32_t valid_blk_id = TakeNextId();
  uint32_t invalid_blk_id = TakeNextId();
  std::unique_ptr<Instruction> merge_label(NewLabel(merge_blk_id));
  std::unique_ptr<Instruction> valid_label(NewLabel(valid_blk_id));
  std::unique_ptr<Instruction> invalid_label(NewLabel(invalid_blk_id));
  (void)builder.AddConditionalBranch(check_id, valid_blk_id, invalid_blk_id,
                                     merge_blk_id, SpvSelectionControlMaskNone);
  // Gen valid bounds branch
  std::unique_ptr<BasicBlock> new_blk_ptr(
      new BasicBlock(std::move(valid_label)));
  builder.SetInsertPoint(&*new_blk_ptr);
  uint32_t new_ref_id = CloneOriginalReference(ref, &builder);
  (void)builder.AddBranch(merge_blk_id);
  new_blocks->push_back(std::move(new_blk_ptr));
  // Gen invalid block
  new_blk_ptr.reset(new BasicBlock(std::move(invalid_label)));
  builder.SetInsertPoint(&*new_blk_ptr);
  uint32_t u_index_id = GenUintCastCode(ref->desc_idx_id, &builder);
  if (offset_id != 0) {
    // Buffer OOB
    uint32_t u_offset_id = GenUintCastCode(offset_id, &builder);
    uint32_t u_length_id = GenUintCastCode(length_id, &builder);
    GenDebugStreamWrite(uid2offset_[ref->ref_inst->unique_id()], stage_idx,
                        {error_id, u_index_id, u_offset_id, u_length_id},
                        &builder);
  } else if (buffer_bounds_enabled_) {
    // Uninitialized Descriptor - Return additional unused zero so all error
    // modes will use same debug stream write function
    uint32_t u_length_id = GenUintCastCode(length_id, &builder);
    GenDebugStreamWrite(
        uid2offset_[ref->ref_inst->unique_id()], stage_idx,
        {error_id, u_index_id, u_length_id, builder.GetUintConstantId(0)},
        &builder);
  } else {
    // Uninitialized Descriptor - Normal error return
    uint32_t u_length_id = GenUintCastCode(length_id, &builder);
    GenDebugStreamWrite(uid2offset_[ref->ref_inst->unique_id()], stage_idx,
                        {error_id, u_index_id, u_length_id}, &builder);
  }
  // Remember last invalid block id
  uint32_t last_invalid_blk_id = new_blk_ptr->GetLabelInst()->result_id();
  // Gen zero for invalid  reference
  uint32_t ref_type_id = ref->ref_inst->type_id();
  (void)builder.AddBranch(merge_blk_id);
  new_blocks->push_back(std::move(new_blk_ptr));
  // Gen merge block
  new_blk_ptr.reset(new BasicBlock(std::move(merge_label)));
  builder.SetInsertPoint(&*new_blk_ptr);
  // Gen phi of new reference and zero, if necessary, and replace the
  // result id of the original reference with that of the Phi. Kill original
  // reference.
  if (new_ref_id != 0) {
    Instruction* phi_inst = builder.AddPhi(
        ref_type_id, {new_ref_id, valid_blk_id, GetNullId(ref_type_id),
                      last_invalid_blk_id});
    context()->ReplaceAllUsesWith(ref->ref_inst->result_id(),
                                  phi_inst->result_id());
  }
  new_blocks->push_back(std::move(new_blk_ptr));
  context()->KillInst(ref->ref_inst);
}

void InstBindlessCheckPass::GenDescIdxCheckCode(
    BasicBlock::iterator ref_inst_itr,
    UptrVectorIterator<BasicBlock> ref_block_itr, uint32_t stage_idx,
    std::vector<std::unique_ptr<BasicBlock>>* new_blocks) {
  // Look for reference through indexed descriptor. If found, analyze and
  // save components. If not, return.
  ref_analysis ref;
  if (!AnalyzeDescriptorReference(&*ref_inst_itr, &ref)) return;
  Instruction* ptr_inst = get_def_use_mgr()->GetDef(ref.ptr_id);
  if (ptr_inst->opcode() != SpvOp::SpvOpAccessChain) return;
  // If index and bound both compile-time constants and index < bound,
  // return without changing
  Instruction* var_inst = get_def_use_mgr()->GetDef(ref.var_id);
  Instruction* desc_type_inst = GetPointeeTypeInst(var_inst);
  uint32_t length_id = 0;
  if (desc_type_inst->opcode() == SpvOpTypeArray) {
    length_id =
        desc_type_inst->GetSingleWordInOperand(kSpvTypeArrayLengthIdInIdx);
    Instruction* index_inst = get_def_use_mgr()->GetDef(ref.desc_idx_id);
    Instruction* length_inst = get_def_use_mgr()->GetDef(length_id);
    if (index_inst->opcode() == SpvOpConstant &&
        length_inst->opcode() == SpvOpConstant &&
        index_inst->GetSingleWordInOperand(kSpvConstantValueInIdx) <
            length_inst->GetSingleWordInOperand(kSpvConstantValueInIdx))
      return;
  } else if (!desc_idx_enabled_ ||
             desc_type_inst->opcode() != SpvOpTypeRuntimeArray) {
    return;
  }
  // Move original block's preceding instructions into first new block
  std::unique_ptr<BasicBlock> new_blk_ptr;
  MovePreludeCode(ref_inst_itr, ref_block_itr, &new_blk_ptr);
  InstructionBuilder builder(
      context(), &*new_blk_ptr,
      IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);
  new_blocks->push_back(std::move(new_blk_ptr));
  uint32_t error_id = builder.GetUintConstantId(kInstErrorBindlessBounds);
  // If length id not yet set, descriptor array is runtime size so
  // generate load of length from stage's debug input buffer.
  if (length_id == 0) {
    assert(desc_type_inst->opcode() == SpvOpTypeRuntimeArray &&
           "unexpected bindless type");
    length_id = GenDebugReadLength(ref.var_id, &builder);
  }
  // Generate full runtime bounds test code with true branch
  // being full reference and false branch being debug output and zero
  // for the referenced value.
  uint32_t desc_idx_32b_id = Gen32BitCvtCode(ref.desc_idx_id, &builder);
  uint32_t length_32b_id = Gen32BitCvtCode(length_id, &builder);
  Instruction* ult_inst = builder.AddBinaryOp(GetBoolId(), SpvOpULessThan,
                                              desc_idx_32b_id, length_32b_id);
  ref.desc_idx_id = desc_idx_32b_id;
  GenCheckCode(ult_inst->result_id(), error_id, 0u, length_id, stage_idx, &ref,
               new_blocks);
  // Move original block's remaining code into remainder/merge block and add
  // to new blocks
  BasicBlock* back_blk_ptr = &*new_blocks->back();
  MovePostludeCode(ref_block_itr, back_blk_ptr);
}

void InstBindlessCheckPass::GenDescInitCheckCode(
    BasicBlock::iterator ref_inst_itr,
    UptrVectorIterator<BasicBlock> ref_block_itr, uint32_t stage_idx,
    std::vector<std::unique_ptr<BasicBlock>>* new_blocks) {
  // Look for reference through descriptor. If not, return.
  ref_analysis ref;
  if (!AnalyzeDescriptorReference(&*ref_inst_itr, &ref)) return;
  // Determine if we can only do initialization check
  bool init_check = false;
  if (ref.desc_load_id != 0 || !buffer_bounds_enabled_) {
    init_check = true;
  } else {
    // For now, only do bounds check for non-aggregate types. Otherwise
    // just do descriptor initialization check.
    // TODO(greg-lunarg): Do bounds check for aggregate loads and stores
    Instruction* ref_ptr_inst = get_def_use_mgr()->GetDef(ref.ptr_id);
    Instruction* pte_type_inst = GetPointeeTypeInst(ref_ptr_inst);
    uint32_t pte_type_op = pte_type_inst->opcode();
    if (pte_type_op == SpvOpTypeArray || pte_type_op == SpvOpTypeRuntimeArray ||
        pte_type_op == SpvOpTypeStruct)
      init_check = true;
  }
  // If initialization check and not enabled, return
  if (init_check && !desc_init_enabled_) return;
  // Move original block's preceding instructions into first new block
  std::unique_ptr<BasicBlock> new_blk_ptr;
  MovePreludeCode(ref_inst_itr, ref_block_itr, &new_blk_ptr);
  InstructionBuilder builder(
      context(), &*new_blk_ptr,
      IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);
  new_blocks->push_back(std::move(new_blk_ptr));
  // If initialization check, use reference value of zero.
  // Else use the index of the last byte referenced.
  uint32_t ref_id = init_check ? builder.GetUintConstantId(0u)
                               : GenLastByteIdx(&ref, &builder);
  // Read initialization/bounds from debug input buffer. If index id not yet
  // set, binding is single descriptor, so set index to constant 0.
  if (ref.desc_idx_id == 0) ref.desc_idx_id = builder.GetUintConstantId(0u);
  uint32_t init_id = GenDebugReadInit(ref.var_id, ref.desc_idx_id, &builder);
  // Generate runtime initialization/bounds test code with true branch
  // being full reference and false branch being debug output and zero
  // for the referenced value.
  Instruction* ult_inst =
      builder.AddBinaryOp(GetBoolId(), SpvOpULessThan, ref_id, init_id);
  uint32_t error =
      init_check ? kInstErrorBindlessUninit : kInstErrorBindlessBuffOOB;
  uint32_t error_id = builder.GetUintConstantId(error);
  GenCheckCode(ult_inst->result_id(), error_id, init_check ? 0 : ref_id,
               init_check ? builder.GetUintConstantId(0u) : init_id, stage_idx,
               &ref, new_blocks);
  // Move original block's remaining code into remainder/merge block and add
  // to new blocks
  BasicBlock* back_blk_ptr = &*new_blocks->back();
  MovePostludeCode(ref_block_itr, back_blk_ptr);
}

void InstBindlessCheckPass::InitializeInstBindlessCheck() {
  // Initialize base class
  InitializeInstrument();
  // If runtime array length support enabled, create variable mappings. Length
  // support is always enabled if descriptor init check is enabled.
  if (desc_idx_enabled_ || buffer_bounds_enabled_)
    for (auto& anno : get_module()->annotations())
      if (anno.opcode() == SpvOpDecorate) {
        if (anno.GetSingleWordInOperand(1u) == SpvDecorationDescriptorSet)
          var2desc_set_[anno.GetSingleWordInOperand(0u)] =
              anno.GetSingleWordInOperand(2u);
        else if (anno.GetSingleWordInOperand(1u) == SpvDecorationBinding)
          var2binding_[anno.GetSingleWordInOperand(0u)] =
              anno.GetSingleWordInOperand(2u);
      }
}

Pass::Status InstBindlessCheckPass::ProcessImpl() {
  // Perform bindless bounds check on each entry point function in module
  InstProcessFunction pfn =
      [this](BasicBlock::iterator ref_inst_itr,
             UptrVectorIterator<BasicBlock> ref_block_itr, uint32_t stage_idx,
             std::vector<std::unique_ptr<BasicBlock>>* new_blocks) {
        return GenDescIdxCheckCode(ref_inst_itr, ref_block_itr, stage_idx,
                                   new_blocks);
      };
  bool modified = InstProcessEntryPointCallTree(pfn);
  if (desc_init_enabled_ || buffer_bounds_enabled_) {
    // Perform descriptor initialization check on each entry point function in
    // module
    pfn = [this](BasicBlock::iterator ref_inst_itr,
                 UptrVectorIterator<BasicBlock> ref_block_itr,
                 uint32_t stage_idx,
                 std::vector<std::unique_ptr<BasicBlock>>* new_blocks) {
      return GenDescInitCheckCode(ref_inst_itr, ref_block_itr, stage_idx,
                                  new_blocks);
    };
    modified |= InstProcessEntryPointCallTree(pfn);
  }
  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

Pass::Status InstBindlessCheckPass::Process() {
  InitializeInstBindlessCheck();
  return ProcessImpl();
}

}  // namespace opt
}  // namespace spvtools
