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

#ifndef SOURCE_FUZZ_FUZZER_PASS_DONATE_MODULES_H_
#define SOURCE_FUZZ_FUZZER_PASS_DONATE_MODULES_H_

#include <vector>

#include "source/fuzz/fuzzer_pass.h"
#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

// A fuzzer pass that randomly adds code from other SPIR-V modules to the module
// being transformed.
class FuzzerPassDonateModules : public FuzzerPass {
 public:
  FuzzerPassDonateModules(
      opt::IRContext* ir_context, FactManager* fact_manager,
      FuzzerContext* fuzzer_context,
      protobufs::TransformationSequence* transformations,
      const std::vector<fuzzerutil::ModuleSupplier>& donor_suppliers);

  ~FuzzerPassDonateModules();

  void Apply() override;

  // Donates the global declarations and functions of |donor_ir_context| into
  // the fuzzer pass's IR context.  |make_livesafe| dictates whether the
  // functions of the donated module will be made livesafe (see
  // FactFunctionIsLivesafe).
  void DonateSingleModule(opt::IRContext* donor_ir_context, bool make_livesafe);

 private:
  // Adapts a storage class coming from a donor module so that it will work
  // in a recipient module, e.g. by changing Uniform to Private.
  static SpvStorageClass AdaptStorageClass(SpvStorageClass donor_storage_class);

  // Identifies all external instruction set imports in |donor_ir_context| and
  // populates |original_id_to_donated_id| with a mapping from the donor's id
  // for such an import to a corresponding import in the recipient.  Aborts if
  // no such corresponding import is available.
  void HandleExternalInstructionImports(
      opt::IRContext* donor_ir_context,
      std::map<uint32_t, uint32_t>* original_id_to_donated_id);

  // Considers all types, globals, constants and undefs in |donor_ir_context|.
  // For each instruction, uses |original_to_donated_id| to map its result id to
  // either (1) the id of an existing identical instruction in the recipient, or
  // (2) to a fresh id, in which case the instruction is also added to the
  // recipient (with any operand ids that it uses being remapped via
  // |original_id_to_donated_id|).
  void HandleTypesAndValues(
      opt::IRContext* donor_ir_context,
      std::map<uint32_t, uint32_t>* original_id_to_donated_id);

  // Assumes that |donor_ir_context| does not exhibit recursion.  Considers the
  // functions in |donor_ir_context|'s call graph in a reverse-topologically-
  // sorted order (leaves-to-root), adding each function to the recipient
  // module, rewritten to use fresh ids and using |original_id_to_donated_id| to
  // remap ids.  The |make_livesafe| argument captures whether the functions in
  // the module are required to be made livesafe before being added to the
  // recipient.
  void HandleFunctions(opt::IRContext* donor_ir_context,
                       std::map<uint32_t, uint32_t>* original_id_to_donated_id,
                       bool make_livesafe);

  // Returns the ids of all functions in |context| in a topological order in
  // relation to the call graph of |context|, which is assumed to be recursion-
  // free.
  static std::vector<uint32_t> GetFunctionsInCallGraphTopologicalOrder(
      opt::IRContext* context);

  // Functions that supply SPIR-V modules
  std::vector<fuzzerutil::ModuleSupplier> donor_suppliers_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_PASS_DONATE_MODULES_H_
