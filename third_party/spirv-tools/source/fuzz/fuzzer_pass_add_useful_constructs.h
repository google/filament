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

#ifndef SOURCE_FUZZ_FUZZER_PASS_ADD_USEFUL_CONSTRUCTS_
#define SOURCE_FUZZ_FUZZER_PASS_ADD_USEFUL_CONSTRUCTS_

#include "source/fuzz/fuzzer_pass.h"

namespace spvtools {
namespace fuzz {

// An initial pass for adding useful ingredients to the module, such as boolean
// constants, if they are not present.
class FuzzerPassAddUsefulConstructs : public FuzzerPass {
 public:
  FuzzerPassAddUsefulConstructs(
      opt::IRContext* ir_context, FactManager* fact_manager,
      FuzzerContext* fuzzer_context,
      protobufs::TransformationSequence* transformations);

  ~FuzzerPassAddUsefulConstructs() override;

  void Apply() override;

 private:
  void MaybeAddIntConstant(uint32_t width, bool is_signed,
                           std::vector<uint32_t> data) const;

  void MaybeAddFloatConstant(uint32_t width, std::vector<uint32_t> data) const;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FUZZER_PASS_ADD_USEFUL_CONSTRUCTS_
