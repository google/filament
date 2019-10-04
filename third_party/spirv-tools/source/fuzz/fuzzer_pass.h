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

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/fuzzer_context.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"

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

 private:
  opt::IRContext* ir_context_;
  FactManager* fact_manager_;
  FuzzerContext* fuzzer_context_;
  protobufs::TransformationSequence* transformations_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_PASS_H_
