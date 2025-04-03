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

#ifndef LIBSPIRV_OPT_SPLIT_COMBINED_IMAGE_SAMPLER_PASS_H_
#define LIBSPIRV_OPT_SPLIT_COMBINED_IMAGE_SAMPLER_PASS_H_

#include <unordered_map>
#include <utility>
#include <vector>

#include "source/diagnostic.h"
#include "source/opt/decoration_manager.h"
#include "source/opt/def_use_manager.h"
#include "source/opt/pass.h"
#include "source/opt/type_manager.h"

namespace spvtools {
namespace opt {

// Replaces each combined-image sampler variable with an image variable
// and a sampler variable. Similar for function parameters.
//
// Copy the descriptor set and binding number. Vulkan allows this, surprisingly.
class SplitCombinedImageSamplerPass : public Pass {
 public:
  virtual ~SplitCombinedImageSamplerPass() override = default;
  const char* name() const override { return "split-combined-image-sampler"; }
  IRContext::Analysis GetPreservedAnalyses() override;
  Status Process() override;

 private:
  // Records failure for the current module, and returns a stream
  // that can be used to provide user error information to the message
  // consumer.
  spvtools::DiagnosticStream Fail();

  // Find variables that contain combined texture-samplers, or arrays of them.
  // Also populate known_globals_.
  void FindCombinedTextureSamplers();

  // Returns the sampler type. If it does not yet exist, then it is created
  // and placed before the first sampled image type.
  Instruction* GetSamplerType();

  // Remaps function types and function declarations.  Each
  // pointer-to-sampled-image-type operand is replaced with a pair of
  // pointer-to-image-type and pointer-to-sampler-type pair.
  // Updates the def-use manager and type manager.
  spv_result_t RemapFunctions();
  // Remap resource variables.
  // Updates the def-use manager.
  spv_result_t RemapVars();
  // Remap a single resource variable for combined var.
  // Updates the def-use manager and the decorations manager.
  spv_result_t RemapVar(Instruction* combined_var);
  // Transitively remaps uses of the combined object with uses of the
  // decomposed image and sampler parts.  The combined object can be sampled
  // image value, a pointer to one, an array of one, or a pointer to an array
  // of one. The image and sampler parts have corresponding shapes.
  // Updates the def-use manager and the decorations manager.
  spv_result_t RemapUses(Instruction* combined, Instruction* image_part,
                         Instruction* sampler_part);
  // Removes types that are no longer referenced.
  spv_result_t RemoveDeadTypes();

  // Returns the type instruction for a UniformConstant pointer to the given
  // pointee type. If it does not yet exist, the new type instruction is created
  // and placed immediately after the pointee type instruction. Updates def-use
  // and type managers, and the set of known globals.
  Instruction* MakeUniformConstantPointer(Instruction* pointee);

  // Returns the ID of the pointee type for a pointer value instruction.
  uint32_t PointeeTypeId(Instruction* ptr_value) {
    auto* ptr_ty = def_use_mgr_->GetDef(ptr_value->type_id());
    assert(ptr_ty->opcode() == spv::Op::OpTypePointer);
    return ptr_ty->GetSingleWordInOperand(1);
  }

  // Creates a new OpName instruction mapping the given name to the given
  // string, and adds it to the module at the end of the OpName and OpMemberName
  // section.
  void AddOpName(uint32_t id, const std::string& name);

  // Cached from the IRContext. Valid while Process() is running.
  analysis::DefUseManager* def_use_mgr_ = nullptr;
  // Cached from the IRContext. Valid while Process() is running.
  analysis::TypeManager* type_mgr_ = nullptr;

  // Did processing modify the module?
  bool modified_ = false;
  Pass::Status Ok() {
    return modified_ ? Pass::Status::SuccessWithChange
                     : Pass::Status::SuccessWithoutChange;
  }

  // The first OpTypeSampledImage instruction in the module, if one exists.
  Instruction* first_sampled_image_type_ = nullptr;
  // An OpTypeSampler instruction, if one existed already, or if we created one.
  Instruction* sampler_type_ = nullptr;

  // The known types and module-scope values.
  // We use this to know when a new such value was created.
  std::unordered_set<uint32_t> known_globals_;
  bool IsKnownGlobal(uint32_t id) const {
    return known_globals_.find(id) != known_globals_.end();
  }
  void RegisterGlobal(uint32_t id) { known_globals_.insert(id); }
  void RegisterNewGlobal(uint32_t id) {
    modified_ = true;
    RegisterGlobal(id);
  }

  // Deletes an instruction and associated debug and decoration instructions.
  // Updates the def-use manager.
  void KillInst(Instruction* inst);

  // Combined types.  The known combined sampled-image type,
  // and recursively pointers or arrays of them.
  std::unordered_set<uint32_t> combined_types_;
  // The pre-existing types this pass should remove: pointer to
  // combined type, array of combined type, pointer to array of combined type.
  std::vector<uint32_t> combined_types_to_remove_;
  // Is an OpTypeSampledImage used as a function parameter? Those should be
  // transformed.
  bool sampled_image_used_as_param_ = false;

  // Remaps a combined-kind type to corresponding sampler-kind and image-kind
  // of type.
  struct TypeRemapInfo {
    // The instruction for the combined type, pointer to combined type,
    // or point to array of combined type.
    Instruction* combined_kind_type;
    // The corresponding image type, with the same shape of indirection as the
    // combined_kind_type.
    Instruction* image_kind_type;
    // The corresponding sampler type, with the same shape of indirection as the
    // combined_kind_type.
    Instruction* sampler_kind_type;
  };
  // Maps the ID of a combined-image-sampler type kind to its corresponding
  // split parts.
  std::unordered_map<uint32_t, TypeRemapInfo> type_remap_;

  // Returns the image-like and sampler-like types of the same indirection shape
  // as the given combined-like type.  If combined_kind_type is not a combined
  // type or a pointer to one, or an array of one or a pointer to an array of
  // one, then returns a pair of null pointer. Either both components are
  // non-null, or both components are null. Updates the def-use manager and the
  // type manager if new instructions are created.
  std::pair<Instruction*, Instruction*> SplitType(
      Instruction& combined_kind_type);

  // The combined-image-sampler variables to be replaced.
  std::vector<Instruction*> ordered_vars_;
};
}  // namespace opt
}  // namespace spvtools
#endif  // LIBSPIRV_OPT_SPLIT_COMBINED_IMAGE_SAMPLER_PASS_H_
