// Copyright (c) 2020 Vasyl Teliman
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

#ifndef SOURCE_FUZZ_TRANSFORMATION_PERMUTE_FUNCTION_PARAMETERS_H_
#define SOURCE_FUZZ_TRANSFORMATION_PERMUTE_FUNCTION_PARAMETERS_H_

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/fuzz/transformation_context.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

class TransformationPermuteFunctionParameters : public Transformation {
 public:
  explicit TransformationPermuteFunctionParameters(
      protobufs::TransformationPermuteFunctionParameters message);

  TransformationPermuteFunctionParameters(
      uint32_t function_id, uint32_t function_type_fresh_id,
      const std::vector<uint32_t>& permutation);

  // - |function_id| is a valid non-entry-point OpFunction instruction
  // - |function_type_fresh_id| is a fresh id.
  //   New type is valid if:
  //     - it has the same number of operands as the old one
  //     - function's result type is the same as the old one
  //     - function's arguments are permuted according to |permutation| vector
  // - |permutation| is a set of [0..(n - 1)], where n is a number of arguments
  //   to the function
  bool IsApplicable(
      opt::IRContext* ir_context,
      const TransformationContext& transformation_context) const override;

  // - OpFunction instruction with |result_id == function_id| is changed.
  //   Its arguments are permuted according to the |permutation| vector
  // - Adjusts function's type to accommodate for permuted parameters.
  // - Calls to the function are adjusted accordingly
  void Apply(opt::IRContext* ir_context,
             TransformationContext* transformation_context) const override;

  std::unordered_set<uint32_t> GetFreshIds() const override;

  protobufs::Transformation ToMessage() const override;

 private:
  protobufs::TransformationPermuteFunctionParameters message_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_TRANSFORMATION_PERMUTE_FUNCTION_PARAMETERS_H_
