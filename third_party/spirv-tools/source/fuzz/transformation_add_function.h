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

#ifndef SOURCE_FUZZ_TRANSFORMATION_ADD_FUNCTION_H_
#define SOURCE_FUZZ_TRANSFORMATION_ADD_FUNCTION_H_

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

class TransformationAddFunction : public Transformation {
 public:
  explicit TransformationAddFunction(
      const protobufs::TransformationAddFunction& message);

  // Creates a transformation to add a non live-safe function.
  explicit TransformationAddFunction(
      const std::vector<protobufs::Instruction>& instructions);

  // Creates a transformation to add a live-safe function.
  TransformationAddFunction(
      const std::vector<protobufs::Instruction>& instructions,
      uint32_t loop_limiter_variable_id, uint32_t loop_limit_constant_id,
      const std::vector<protobufs::LoopLimiterInfo>& loop_limiters,
      uint32_t kill_unreachable_return_value_id,
      const std::vector<protobufs::AccessChainClampingInfo>&
          access_chain_clampers);

  // - |message_.instruction| must correspond to a sufficiently well-formed
  //   sequence of instructions that a function can be created from them
  // - If |message_.is_livesafe| holds then |message_| must contain suitable
  //   ingredients to make the function livesafe, and the function must only
  //   invoke other livesafe functions
  // - Adding the created function to the module must lead to a valid module.
  bool IsApplicable(opt::IRContext* context,
                    const FactManager& fact_manager) const override;

  // Adds the function defined by |message_.instruction| to the module, making
  // it livesafe if |message_.is_livesafe| holds.
  void Apply(opt::IRContext* context, FactManager* fact_manager) const override;

  protobufs::Transformation ToMessage() const override;

  // Helper method that returns the bound for indexing into a composite of type
  // |composite_type_inst|, i.e. the number of fields of a struct, the size of
  // an array, the number of components of a vector, or the number of columns of
  // a matrix.
  static uint32_t GetBoundForCompositeIndex(
      opt::IRContext* context, const opt::Instruction& composite_type_inst);

  // Helper method that, given composite type |composite_type_inst|, returns the
  // type of the sub-object at index |index_id|, which is required to be in-
  // bounds.
  static opt::Instruction* FollowCompositeIndex(
      opt::IRContext* context, const opt::Instruction& composite_type_inst,
      uint32_t index_id);

 private:
  // Attempts to create a function from the series of instructions in
  // |message_.instruction| and add it to |context|.
  //
  // Returns false if adding the function is not possible due to the messages
  // not respecting the basic structure of a function, e.g. if there is no
  // OpFunction instruction or no blocks; in this case |context| is left in an
  // indeterminate state.
  //
  // Otherwise returns true.  Whether |context| is valid after addition of the
  // function depends on the contents of |message_.instruction|.
  //
  // Intended usage:
  // - Perform a dry run of this method on a clone of a module, and use
  //   the validator to check whether the resulting module is valid.  Working
  //   on a clone means it does not matter if the function fails to be cleanly
  //   added, or leads to an invalid module.
  // - If the dry run succeeds, run the method on the real module of interest,
  //   to add the function.
  bool TryToAddFunction(opt::IRContext* context) const;

  // Should only be called if |message_.is_livesafe| holds.  Attempts to make
  // the function livesafe (see FactFunctionIsLivesafe for a definition).
  // Returns false if this is not possible, due to |message_| or |context| not
  // containing sufficient ingredients (such as types and fresh ids) to add
  // the instrumentation necessary to make the function livesafe.
  bool TryToMakeFunctionLivesafe(opt::IRContext* context,
                                 const FactManager& fact_manager) const;

  // A helper for TryToMakeFunctionLivesafe that tries to add loop-limiting
  // logic.
  bool TryToAddLoopLimiters(opt::IRContext* context,
                            opt::Function* added_function) const;

  // A helper for TryToMakeFunctionLivesafe that tries to replace OpKill and
  // OpUnreachable instructions into return instructions.
  bool TryToTurnKillOrUnreachableIntoReturn(
      opt::IRContext* context, opt::Function* added_function,
      opt::Instruction* kill_or_unreachable_inst) const;

  // A helper for TryToMakeFunctionLivesafe that tries to clamp access chain
  // indices so that they are guaranteed to be in-bounds.
  bool TryToClampAccessChainIndices(opt::IRContext* context,
                                    opt::Instruction* access_chain_inst) const;

  protobufs::TransformationAddFunction message_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_TRANSFORMATION_ADD_FUNCTION_H_
