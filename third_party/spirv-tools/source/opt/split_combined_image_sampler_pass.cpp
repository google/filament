// Copyright (c) 2025 Google LLC
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

#include "source/opt/split_combined_image_sampler_pass.h"

#include <algorithm>
#include <cassert>
#include <memory>

#include "source/opt/instruction.h"
#include "source/opt/ir_builder.h"
#include "source/opt/ir_context.h"
#include "source/opt/type_manager.h"
#include "source/opt/types.h"
#include "source/util/make_unique.h"
#include "source/util/string_utils.h"
#include "spirv/unified1/spirv.h"

namespace spvtools {
namespace opt {

#define CHECK(cond)                                          \
  {                                                          \
    if ((cond) != SPV_SUCCESS) return Pass::Status::Failure; \
  }

#define CHECK_STATUS(cond)                           \
  {                                                  \
    if (auto c = (cond); c != SPV_SUCCESS) return c; \
  }

IRContext::Analysis SplitCombinedImageSamplerPass::GetPreservedAnalyses() {
  return
      // def use manager is updated
      IRContext::kAnalysisDefUse

      // decorations are updated
      | IRContext::kAnalysisDecorations

      // control flow is not changed
      | IRContext::kAnalysisCFG           //
      | IRContext::kAnalysisLoopAnalysis  //
      | IRContext::kAnalysisStructuredCFG

      // type manager is updated
      | IRContext::kAnalysisTypes;
}

Pass::Status SplitCombinedImageSamplerPass::Process() {
  def_use_mgr_ = context()->get_def_use_mgr();
  type_mgr_ = context()->get_type_mgr();

  FindCombinedTextureSamplers();
  if (combined_types_to_remove_.empty() && !sampled_image_used_as_param_) {
    return Ok();
  }

  CHECK(RemapFunctions());
  CHECK(RemapVars());
  CHECK(RemoveDeadTypes());

  def_use_mgr_ = nullptr;
  type_mgr_ = nullptr;

  return Ok();
}

spvtools::DiagnosticStream SplitCombinedImageSamplerPass::Fail() {
  return std::move(
      spvtools::DiagnosticStream({}, consumer(), "", SPV_ERROR_INVALID_BINARY)
      << "split-combined-image-sampler: ");
}

void SplitCombinedImageSamplerPass::FindCombinedTextureSamplers() {
  for (auto& inst : context()->types_values()) {
    RegisterGlobal(inst.result_id());
    switch (inst.opcode()) {
      case spv::Op::OpTypeSampler:
        // Modules can't have duplicate sampler types.
        assert(!sampler_type_);
        sampler_type_ = &inst;
        break;

      case spv::Op::OpTypeSampledImage:
        if (!first_sampled_image_type_) {
          first_sampled_image_type_ = &inst;
        }
        combined_types_.insert(inst.result_id());
        def_use_mgr_->WhileEachUser(inst.result_id(), [&](Instruction* i) {
          sampled_image_used_as_param_ |=
              i->opcode() == spv::Op::OpTypeFunction;
          return !sampled_image_used_as_param_;
        });
        break;

      case spv::Op::OpTypeArray:
      case spv::Op::OpTypeRuntimeArray: {
        auto pointee_id = inst.GetSingleWordInOperand(0);
        if (combined_types_.find(pointee_id) != combined_types_.end()) {
          combined_types_.insert(inst.result_id());
          combined_types_to_remove_.push_back(inst.result_id());
        }
      } break;

      case spv::Op::OpTypePointer: {
        auto sc =
            static_cast<spv::StorageClass>(inst.GetSingleWordInOperand(0));
        if (sc == spv::StorageClass::UniformConstant) {
          auto pointee_id = inst.GetSingleWordInOperand(1);
          if (combined_types_.find(pointee_id) != combined_types_.end()) {
            combined_types_.insert(inst.result_id());
            combined_types_to_remove_.push_back(inst.result_id());
          }
        }
      } break;

      case spv::Op::OpVariable:
        if (combined_types_.find(inst.type_id()) != combined_types_.end()) {
          ordered_vars_.push_back(&inst);
        }
        break;

      default:
        break;
    }
  }
}

Instruction* SplitCombinedImageSamplerPass::GetSamplerType() {
  if (!sampler_type_) {
    analysis::Sampler s;
    uint32_t sampler_type_id = type_mgr_->GetTypeInstruction(&s);
    sampler_type_ = def_use_mgr_->GetDef(sampler_type_id);
    assert(first_sampled_image_type_);
    sampler_type_->InsertBefore(first_sampled_image_type_);
    RegisterNewGlobal(sampler_type_->result_id());
  }
  return sampler_type_;
}

spv_result_t SplitCombinedImageSamplerPass::RemapVars() {
  for (Instruction* var : ordered_vars_) {
    CHECK_STATUS(RemapVar(var));
  }
  return SPV_SUCCESS;
}

std::pair<Instruction*, Instruction*> SplitCombinedImageSamplerPass::SplitType(
    Instruction& combined_kind_type) {
  if (auto where = type_remap_.find(combined_kind_type.result_id());
      where != type_remap_.end()) {
    auto& type_remap = where->second;
    return {type_remap.image_kind_type, type_remap.sampler_kind_type};
  }

  switch (combined_kind_type.opcode()) {
    case spv::Op::OpTypeSampledImage: {
      auto* image_type =
          def_use_mgr_->GetDef(combined_kind_type.GetSingleWordInOperand(0));
      auto* sampler_type = GetSamplerType();
      type_remap_[combined_kind_type.result_id()] = {&combined_kind_type,
                                                     image_type, sampler_type};
      return {image_type, sampler_type};
      break;
    }
    case spv::Op::OpTypePointer: {
      auto sc = static_cast<spv::StorageClass>(
          combined_kind_type.GetSingleWordInOperand(0));
      if (sc == spv::StorageClass::UniformConstant) {
        auto* pointee =
            def_use_mgr_->GetDef(combined_kind_type.GetSingleWordInOperand(1));
        auto [image_pointee, sampler_pointee] = SplitType(*pointee);
        // These would be null if the pointee is an image type or a sampler
        // type. Don't decompose them. Currently this method does not check the
        // assumption that it is being only called on combined types. So code
        // this defensively.
        if (image_pointee && sampler_pointee) {
          auto* ptr_image = MakeUniformConstantPointer(image_pointee);
          auto* ptr_sampler = MakeUniformConstantPointer(sampler_pointee);
          type_remap_[combined_kind_type.result_id()] = {
              &combined_kind_type, ptr_image, ptr_sampler};
          return {ptr_image, ptr_sampler};
        }
      }
      break;
    }
    case spv::Op::OpTypeArray: {
      const auto* array_ty =
          type_mgr_->GetType(combined_kind_type.result_id())->AsArray();
      assert(array_ty);
      const auto* sampled_image_ty = array_ty->element_type()->AsSampledImage();
      assert(sampled_image_ty);

      const analysis::Type* image_ty = sampled_image_ty->image_type();
      assert(image_ty);
      analysis::Array array_image_ty(image_ty, array_ty->length_info());
      const uint32_t array_image_ty_id =
          type_mgr_->GetTypeInstruction(&array_image_ty);
      auto* array_image_ty_inst = def_use_mgr_->GetDef(array_image_ty_id);
      if (!IsKnownGlobal(array_image_ty_id)) {
        array_image_ty_inst->InsertBefore(&combined_kind_type);
        RegisterNewGlobal(array_image_ty_id);
        // GetTypeInstruction also updated the def-use manager.
      }

      analysis::Array sampler_array_ty(
          type_mgr_->GetType(GetSamplerType()->result_id()),
          array_ty->length_info());
      const uint32_t array_sampler_ty_id =
          type_mgr_->GetTypeInstruction(&sampler_array_ty);
      auto* array_sampler_ty_inst = def_use_mgr_->GetDef(array_sampler_ty_id);
      if (!IsKnownGlobal(array_sampler_ty_id)) {
        array_sampler_ty_inst->InsertBefore(&combined_kind_type);
        RegisterNewGlobal(array_sampler_ty_id);
        // GetTypeInstruction also updated the def-use manager.
      }
      return {array_image_ty_inst, array_sampler_ty_inst};
    }
    case spv::Op::OpTypeRuntimeArray: {
      // This is like the sized-array case, but there is no length parameter.
      auto* array_ty =
          type_mgr_->GetType(combined_kind_type.result_id())->AsRuntimeArray();
      assert(array_ty);
      auto* sampled_image_ty = array_ty->element_type()->AsSampledImage();
      assert(sampled_image_ty);

      const analysis::Type* image_ty = sampled_image_ty->image_type();
      assert(image_ty);
      analysis::RuntimeArray array_image_ty(image_ty);
      const uint32_t array_image_ty_id =
          type_mgr_->GetTypeInstruction(&array_image_ty);
      auto* array_image_ty_inst = def_use_mgr_->GetDef(array_image_ty_id);
      if (!IsKnownGlobal(array_image_ty_id)) {
        array_image_ty_inst->InsertBefore(&combined_kind_type);
        RegisterNewGlobal(array_image_ty_id);
        // GetTypeInstruction also updated the def-use manager.
      }

      analysis::RuntimeArray sampler_array_ty(
          type_mgr_->GetType(GetSamplerType()->result_id()));
      const uint32_t array_sampler_ty_id =
          type_mgr_->GetTypeInstruction(&sampler_array_ty);
      auto* array_sampler_ty_inst = def_use_mgr_->GetDef(array_sampler_ty_id);
      if (!IsKnownGlobal(array_sampler_ty_id)) {
        array_sampler_ty_inst->InsertBefore(&combined_kind_type);
        RegisterNewGlobal(array_sampler_ty_id);
        // GetTypeInstruction also updated the def-use manager.
      }
      return {array_image_ty_inst, array_sampler_ty_inst};
    }
    default:
      break;
  }
  return {nullptr, nullptr};
}

spv_result_t SplitCombinedImageSamplerPass::RemapVar(
    Instruction* combined_var) {
  InstructionBuilder builder(context(), combined_var,
                             IRContext::kAnalysisDefUse);

  // Create an image variable, and a sampler variable.
  auto* combined_var_type = def_use_mgr_->GetDef(combined_var->type_id());
  auto [ptr_image_ty, ptr_sampler_ty] = SplitType(*combined_var_type);
  assert(ptr_image_ty);
  assert(ptr_sampler_ty);
  Instruction* sampler_var = builder.AddVariable(
      ptr_sampler_ty->result_id(), SpvStorageClassUniformConstant);
  Instruction* image_var = builder.AddVariable(ptr_image_ty->result_id(),
                                               SpvStorageClassUniformConstant);
  modified_ = true;
  return RemapUses(combined_var, image_var, sampler_var);
}

spv_result_t SplitCombinedImageSamplerPass::RemapUses(
    Instruction* combined, Instruction* image_part, Instruction* sampler_part) {
  // The instructions to delete.
  std::unordered_set<Instruction*> dead_insts;
  // The insertion point should be updated before using this builder.
  // We needed *something* here.
  InstructionBuilder builder(context(), combined, IRContext::kAnalysisDefUse);

  // This code must maintain the SPIR-V "Data rule" about sampled image values:
  //  > All OpSampledImage instructions, or instructions that load an image or
  //  > sampler reference, must be in the same block in which their Result <id>
  //  > are consumed.
  //
  // When the code below inserts OpSampledImage instructions, it is always
  // either:
  // - in the same block as the previous OpSampledImage instruction it is
  //   replacing, or
  // - in the same block as the instruction using sampled image value it is
  //   replacing.
  //
  // Assuming that rule is already honoured by the module, these updates will
  // continue to honour the rule.

  // Represents a single use of a value to be remapped.
  struct RemapUse {
    uint32_t used_id;  // The ID that is being used.
    Instruction* user;
    uint32_t index;
    Instruction* image_part;    // The image part of the replacement.
    Instruction* sampler_part;  // The sampler part of the replacement.
  };
  // The work list of uses to be remapped.
  std::vector<RemapUse> uses;

  // Adds remap records for each use of a value to be remapped.
  // Also schedules the original value for deletion.
  auto add_remap = [this, &dead_insts, &uses](Instruction* combined_arg,
                                              Instruction* image_part_arg,
                                              Instruction* sampler_part_arg) {
    const uint32_t used_combined_id = combined_arg->result_id();

    def_use_mgr_->ForEachUse(
        combined_arg, [&](Instruction* user, uint32_t use_index) {
          uses.push_back({used_combined_id, user, use_index, image_part_arg,
                          sampler_part_arg});
        });
    dead_insts.insert(combined_arg);
  };

  add_remap(combined, image_part, sampler_part);

  // Use index-based iteration because we can add to the work list as we go
  // along, and reallocation would invalidate ordinary iterators.
  for (size_t use_index = 0; use_index < uses.size(); ++use_index) {
    auto& use = uses[use_index];
    switch (use.user->opcode()) {
      case spv::Op::OpCopyObject: {
        // Append the uses of this OpCopyObject to the work list.
        add_remap(use.user, image_part, sampler_part);
        break;
      }
      case spv::Op::OpLoad: {
        assert(use.index == 2 && "variable used as non-pointer index on load");
        Instruction* load = use.user;

        // Assume the loaded value is a sampled image.
        assert(def_use_mgr_->GetDef(load->type_id())->opcode() ==
               spv::Op::OpTypeSampledImage);

        // Create loads for the image part and sampler part.
        builder.SetInsertPoint(load);
        auto* image = builder.AddLoad(PointeeTypeId(use.image_part),
                                      use.image_part->result_id());
        auto* sampler = builder.AddLoad(PointeeTypeId(use.sampler_part),
                                        use.sampler_part->result_id());

        // Move decorations, such as RelaxedPrecision.
        auto* deco_mgr = context()->get_decoration_mgr();
        deco_mgr->CloneDecorations(load->result_id(), image->result_id());
        deco_mgr->CloneDecorations(load->result_id(), sampler->result_id());
        deco_mgr->RemoveDecorationsFrom(load->result_id());

        // Create a sampled image from the loads of the two parts.
        auto* sampled_image = builder.AddSampledImage(
            load->type_id(), image->result_id(), sampler->result_id());
        // Replace the original sampled image value with the new one.
        std::unordered_set<Instruction*> users;
        def_use_mgr_->ForEachUse(
            load, [&users, sampled_image](Instruction* user, uint32_t index) {
              user->SetOperand(index, {sampled_image->result_id()});
              users.insert(user);
            });
        for (auto* user : users) {
          def_use_mgr_->AnalyzeInstUse(user);
        }
        dead_insts.insert(load);
        break;
      }
      case spv::Op::OpDecorate: {
        assert(use.index == 0 && "variable used as non-target index");
        builder.SetInsertPoint(use.user);
        spv::Decoration deco{use.user->GetSingleWordInOperand(1)};
        std::vector<uint32_t> literals;
        for (uint32_t i = 2; i < use.user->NumInOperands(); i++) {
          literals.push_back(use.user->GetSingleWordInOperand(i));
        }
        builder.AddDecoration(use.image_part->result_id(), deco, literals);
        builder.AddDecoration(use.sampler_part->result_id(), deco, literals);
        // KillInst will delete names and decorations, so don't schedule a
        // deletion of this instruction.
        break;
      }
      case spv::Op::OpEntryPoint: {
        // The entry point lists variables in the shader interface, i.e.
        // module-scope variables referenced by the static call tree rooted
        // at the entry point. (It can be a proper superset).  Before SPIR-V
        // 1.4, only Input and Output variables are listed; in 1.4 and later,
        // module-scope variables in all storage classes are listed.
        // If a combined image+sampler is listed by the entry point, then
        // the separated image and sampler variables should be.
        assert(use.index >= 3 &&
               "variable used in OpEntryPoint but not as an interface ID");
        use.user->SetOperand(use.index, {use.image_part->result_id()});
        use.user->InsertOperand(
            use.user->NumOperands(),
            {SPV_OPERAND_TYPE_ID, {use.sampler_part->result_id()}});
        def_use_mgr_->AnalyzeInstUse(use.user);
        break;
      }
      case spv::Op::OpName: {
        // Synthesize new names from the old.
        const auto name = use.user->GetOperand(1).AsString();
        AddOpName(use.image_part->result_id(), name + "_image");
        AddOpName(use.sampler_part->result_id(), name + "_sampler");

        // KillInst will delete names and decorations, so don't schedule a
        // deletion of this instruction.
        break;
      }
      case spv::Op::OpFunctionCall: {
        // Replace each combined arg with two args: the image part, then the
        // sampler part.
        // The combined value could have been used twice in the argument list.
        // Moving things around now will invalidate the 'use' list above.
        // So don't trust the use index value.
        auto& call = *use.user;
        // The insert API only takes absolute arg IDs, not "in" arg IDs.
        const auto first_arg_operand_index = 3;  // Skip the callee ID
        for (uint32_t i = first_arg_operand_index; i < call.NumOperands();
             ++i) {
          if (use.used_id == call.GetSingleWordOperand(i)) {
            call.SetOperand(i, {use.sampler_part->result_id()});
            call.InsertOperand(
                i, {SPV_OPERAND_TYPE_ID, {use.image_part->result_id()}});
            ++i;
          }
        }
        def_use_mgr_->AnalyzeInstUse(&call);
        break;
      }
      case spv::Op::OpAccessChain:
      case spv::Op::OpInBoundsAccessChain: {
        auto* original_access_chain = use.user;
        builder.SetInsertPoint(original_access_chain);
        // It can only be the base pointer
        assert(use.index == 2);

        // Replace the original access chain with access chains for the image
        // part and the sampler part.
        std::vector<uint32_t> indices;
        for (uint32_t i = 3; i < original_access_chain->NumOperands(); i++) {
          indices.push_back(original_access_chain->GetSingleWordOperand(i));
        }

        auto [result_image_part_ty, result_sampler_part_ty] =
            SplitType(*def_use_mgr_->GetDef(original_access_chain->type_id()));
        auto* result_image_part = builder.AddOpcodeAccessChain(
            use.user->opcode(), result_image_part_ty->result_id(),
            use.image_part->result_id(), indices);
        auto* result_sampler_part = builder.AddOpcodeAccessChain(
            use.user->opcode(), result_sampler_part_ty->result_id(),
            use.sampler_part->result_id(), indices);

        // Remap uses of the original access chain.
        add_remap(original_access_chain, result_image_part,
                  result_sampler_part);
        break;
      }
      default: {
        uint32_t used_type_id = def_use_mgr_->GetDef(use.used_id)->type_id();
        auto* used_type = def_use_mgr_->GetDef(used_type_id);
        if (used_type->opcode() == spv::Op::OpTypeSampledImage) {
          // This value being used is a sampled image value.  But it's
          // being replaced, so recreate it here.
          // Example: used by OpImage, OpImageSampleExplicitLod, etc.
          builder.SetInsertPoint(use.user);
          auto* sampled_image =
              builder.AddSampledImage(used_type_id, use.image_part->result_id(),
                                      use.sampler_part->result_id());
          use.user->SetOperand(use.index, {sampled_image->result_id()});
          def_use_mgr_->AnalyzeInstUse(use.user);
          break;
        }
        return Fail() << "unhandled user: " << *use.user;
      }
    }
  }

  for (auto* inst : dead_insts) {
    KillInst(inst);
  }

  return SPV_SUCCESS;
}

spv_result_t SplitCombinedImageSamplerPass::RemapFunctions() {
  // Remap function types. A combined type can appear as a parameter, but not as
  // the return type.
  {
    std::unordered_set<Instruction*> dead_insts;
    for (auto& inst : context()->types_values()) {
      if (inst.opcode() != spv::Op::OpTypeFunction) {
        continue;
      }
      analysis::Function* f_ty =
          type_mgr_->GetType(inst.result_id())->AsFunction();
      std::vector<const analysis::Type*> new_params;
      for (const auto* param_ty : f_ty->param_types()) {
        const auto param_ty_id = type_mgr_->GetId(param_ty);
        if (combined_types_.find(param_ty_id) != combined_types_.end()) {
          auto* param_type = def_use_mgr_->GetDef(param_ty_id);
          auto [image_type, sampler_type] = SplitType(*param_type);
          assert(image_type);
          assert(sampler_type);
          // The image and sampler types must already exist, so there is no
          // need to move them to the right spot.
          new_params.push_back(type_mgr_->GetType(image_type->result_id()));
          new_params.push_back(type_mgr_->GetType(sampler_type->result_id()));
        } else {
          new_params.push_back(param_ty);
        }
      }
      if (new_params.size() != f_ty->param_types().size()) {
        // Replace this type.
        analysis::Function new_f_ty(f_ty->return_type(), new_params);
        const uint32_t new_f_ty_id = type_mgr_->GetTypeInstruction(&new_f_ty);
        std::unordered_set<Instruction*> users;
        def_use_mgr_->ForEachUse(
            &inst,
            [&users, new_f_ty_id](Instruction* user, uint32_t use_index) {
              user->SetOperand(use_index, {new_f_ty_id});
              users.insert(user);
            });
        for (auto* user : users) {
          def_use_mgr_->AnalyzeInstUse(user);
        }
        dead_insts.insert(&inst);
      }
    }
    for (auto* inst : dead_insts) {
      KillInst(inst);
    }
  }

  // Rewite OpFunctionParameter in function definitions.
  for (Function& fn : *context()->module()) {
    // Rewrite the function parameters and record their replacements.
    struct Replacement {
      Instruction* combined;
      Instruction* image;
      Instruction* sampler;
    };
    std::vector<Replacement> replacements;

    Function::RewriteParamFn rewriter =
        [&](std::unique_ptr<Instruction>&& param,
            std::back_insert_iterator<Function::ParamList>& appender) {
          if (combined_types_.count(param->type_id()) == 0) {
            appender = std::move(param);
            return;
          }

          // Replace this parameter with two new parameters.
          auto* combined_inst = param.release();
          auto* combined_type = def_use_mgr_->GetDef(combined_inst->type_id());
          auto [image_type, sampler_type] = SplitType(*combined_type);
          auto image_param = MakeUnique<Instruction>(
              context(), spv::Op::OpFunctionParameter, image_type->result_id(),
              context()->TakeNextId(), Instruction::OperandList{});
          auto sampler_param = MakeUnique<Instruction>(
              context(), spv::Op::OpFunctionParameter,
              sampler_type->result_id(), context()->TakeNextId(),
              Instruction::OperandList{});
          replacements.push_back(
              {combined_inst, image_param.get(), sampler_param.get()});
          appender = std::move(image_param);
          appender = std::move(sampler_param);
        };
    fn.RewriteParams(rewriter);

    for (auto& r : replacements) {
      modified_ = true;
      def_use_mgr_->AnalyzeInstDefUse(r.image);
      def_use_mgr_->AnalyzeInstDefUse(r.sampler);
      CHECK_STATUS(RemapUses(r.combined, r.image, r.sampler));
    }
  }
  return SPV_SUCCESS;
}

Instruction* SplitCombinedImageSamplerPass::MakeUniformConstantPointer(
    Instruction* pointee) {
  uint32_t ptr_id = type_mgr_->FindPointerToType(
      pointee->result_id(), spv::StorageClass::UniformConstant);
  auto* ptr = def_use_mgr_->GetDef(ptr_id);
  if (!IsKnownGlobal(ptr_id)) {
    // The pointer type was created at the end. Put it right after the
    // pointee.
    ptr->InsertBefore(pointee);
    pointee->InsertBefore(ptr);
    RegisterNewGlobal(ptr_id);
    // FindPointerToType also updated the def-use manager.
  }
  return ptr;
}

void SplitCombinedImageSamplerPass::AddOpName(uint32_t id,
                                              const std::string& name) {
  std::unique_ptr<Instruction> opname{new Instruction{
      context(),
      spv::Op::OpName,
      0u,
      0u,
      {{SPV_OPERAND_TYPE_ID, {id}},
       {SPV_OPERAND_TYPE_LITERAL_STRING,
        utils::MakeVector<spvtools::opt::Operand::OperandData>(name)}}}};

  context()->AddDebug2Inst(std::move(opname));
}

spv_result_t SplitCombinedImageSamplerPass::RemoveDeadTypes() {
  for (auto dead_type_id : combined_types_to_remove_) {
    if (auto* ty = def_use_mgr_->GetDef(dead_type_id)) {
      KillInst(ty);
    }
  }
  return SPV_SUCCESS;
}

void SplitCombinedImageSamplerPass::KillInst(Instruction* inst) {
  // IRContext::KillInst will remove associated debug instructions and
  // decorations. It will delete the object only if it is already in a list.
  const bool was_in_list = inst->IsInAList();
  context()->KillInst(inst);
  if (!was_in_list) {
    // Avoid leaking
    delete inst;
  }
  modified_ = true;
}

}  // namespace opt
}  // namespace spvtools
