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

#include "source/fuzz/fuzzer_pass_add_dead_continues.h"

#include "source/fuzz/transformation_add_dead_continue.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

FuzzerPassAddDeadContinues::FuzzerPassAddDeadContinues(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassAddDeadContinues::~FuzzerPassAddDeadContinues() = default;

void FuzzerPassAddDeadContinues::Apply() {
  // Consider every block in every function.
  for (auto& function : *GetIRContext()->module()) {
    for (auto& block : function) {
      // Make a transformation to add a dead continue from this node; if the
      // node turns out to be inappropriate (e.g. by not being in a loop) the
      // precondition for the transformation will fail and it will be ignored.
      //
      // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/2856): right
      //  now we completely ignore OpPhi instructions at continue targets.
      //  This will lead to interesting opportunities being missed.
      auto candidate_transformation = TransformationAddDeadContinue(
          block.id(), GetFuzzerContext()->ChooseEven(), {});
      // Probabilistically decide whether to apply the transformation in the
      // case that it is applicable.
      if (candidate_transformation.IsApplicable(GetIRContext(),
                                                *GetFactManager()) &&
          GetFuzzerContext()->ChoosePercentage(
              GetFuzzerContext()->GetChanceOfAddingDeadContinue())) {
        candidate_transformation.Apply(GetIRContext(), GetFactManager());
        *GetTransformations()->add_transformation() =
            candidate_transformation.ToMessage();
      }
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
