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

#ifndef SOURCE_FUZZ_FUZZER_PASS_H_
#define SOURCE_FUZZ_FUZZER_PASS_H_

#include <functional>
#include <vector>

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/fuzzer_context.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

// Interface for applying a pass of transformations to a module.
class FuzzerPass {
 public:
  FuzzerPass(opt::IRContext* ir_context, FactManager* fact_manager,
             FuzzerContext* fuzzer_context,
             protobufs::TransformationSequence* transformations);

  virtual ~FuzzerPass();

  // Applies the pass to the module |ir_context_|, assuming and updating
  // facts from |fact_manager_|, and using |fuzzer_context_| to guide the
  // process.  Appends to |transformations_| all transformations that were
  // applied during the pass.
  virtual void Apply() = 0;

 protected:
  opt::IRContext* GetIRContext() const { return ir_context_; }

  FactManager* GetFactManager() const { return fact_manager_; }

  FuzzerContext* GetFuzzerContext() const { return fuzzer_context_; }

  protobufs::TransformationSequence* GetTransformations() const {
    return transformations_;
  }

  // Returns all instructions that are *available* at |inst_it|, which is
  // required to be inside block |block| of function |function| - that is, all
  // instructions at global scope and all instructions that strictly dominate
  // |inst_it|.
  //
  // Filters said instructions to return only those that satisfy the
  // |instruction_is_relevant| predicate.  This, for instance, could ignore all
  // instructions that have a particular decoration.
  std::vector<opt::Instruction*> FindAvailableInstructions(
      const opt::Function& function, opt::BasicBlock* block,
      opt::BasicBlock::iterator inst_it,
      std::function<bool(opt::IRContext*, opt::Instruction*)>
          instruction_is_relevant);

  // A helper method that iterates through each instruction in each block, at
  // all times tracking an instruction descriptor that allows the latest
  // instruction to be located even if it has no result id.
  //
  // The code to manipulate the instruction descriptor is a bit fiddly, and the
  // point of this method is to avoiding having to duplicate it in multiple
  // transformation passes.
  //
  // The function |maybe_apply_transformation| is invoked for each instruction
  // |inst_it| in block |block| of function |function| that is encountered.  The
  // |instruction_descriptor| parameter to the function object allows |inst_it|
  // to be identified.
  //
  // The job of |maybe_apply_transformation| is to randomly decide whether to
  // try to apply some transformation, and then - if selected - to attempt to
  // apply it.
  void MaybeAddTransformationBeforeEachInstruction(
      std::function<
          void(const opt::Function& function, opt::BasicBlock* block,
               opt::BasicBlock::iterator inst_it,
               const protobufs::InstructionDescriptor& instruction_descriptor)>
          maybe_apply_transformation);

  // A generic helper for applying a transformation that should be applicable
  // by construction, and adding it to the sequence of applied transformations.
  template <typename TransformationType>
  void ApplyTransformation(const TransformationType& transformation) {
    assert(transformation.IsApplicable(GetIRContext(), *GetFactManager()) &&
           "Transformation should be applicable by construction.");
    transformation.Apply(GetIRContext(), GetFactManager());
    *GetTransformations()->add_transformation() = transformation.ToMessage();
  }

  // Returns the id of an OpTypeBool instruction.  If such an instruction does
  // not exist, a transformation is applied to add it.
  uint32_t FindOrCreateBoolType();

  // Returns the id of an OpTypeInt instruction, with width 32 and signedness
  // specified by |is_signed|.  If such an instruction does not exist, a
  // transformation is applied to add it.
  uint32_t FindOrCreate32BitIntegerType(bool is_signed);

  // Returns the id of an OpTypeFloat instruction, with width 32.  If such an
  // instruction does not exist, a transformation is applied to add it.
  uint32_t FindOrCreate32BitFloatType();

  // Returns the id of an OpTypeVector instruction, with |component_type_id|
  // (which must already exist) as its base type, and |component_count|
  // elements (which must be in the range [2, 4]).  If such an instruction does
  // not exist, a transformation is applied to add it.
  uint32_t FindOrCreateVectorType(uint32_t component_type_id,
                                  uint32_t component_count);

  // Returns the id of an OpTypeMatrix instruction, with |column_count| columns
  // and |row_count| rows (each of which must be in the range [2, 4]).  If the
  // float and vector types required to build this matrix type or the matrix
  // type itself do not exist, transformations are applied to add them.
  uint32_t FindOrCreateMatrixType(uint32_t column_count, uint32_t row_count);

  // Returns the id of an OpTypePointer instruction, with a 32-bit integer base
  // type of signedness specified by |is_signed|.  If the pointer type or
  // required integer base type do not exist, transformations are applied to add
  // them.
  uint32_t FindOrCreatePointerTo32BitIntegerType(bool is_signed,
                                                 SpvStorageClass storage_class);

  // Returns the id of an OpConstant instruction, with 32-bit integer type of
  // signedness specified by |is_signed|, with |word| as its value.  If either
  // the required integer type or the constant do not exist, transformations are
  // applied to add them.
  uint32_t FindOrCreate32BitIntegerConstant(uint32_t word, bool is_signed);

  // Returns the result id of an instruction of the form:
  //   %id = OpUndef %|type_id|
  // If no such instruction exists, a transformation is applied to add it.
  uint32_t FindOrCreateGlobalUndef(uint32_t type_id);

 private:
  opt::IRContext* ir_context_;
  FactManager* fact_manager_;
  FuzzerContext* fuzzer_context_;
  protobufs::TransformationSequence* transformations_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_PASS_H_
