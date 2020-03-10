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

#ifndef SOURCE_FUZZ_TRANSFORMATION_COMPOSITE_EXTRACT_H_
#define SOURCE_FUZZ_TRANSFORMATION_COMPOSITE_EXTRACT_H_

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

class TransformationCompositeExtract : public Transformation {
 public:
  explicit TransformationCompositeExtract(
      const protobufs::TransformationCompositeExtract& message);

  TransformationCompositeExtract(
      const protobufs::InstructionDescriptor& instruction_to_insert_before,
      uint32_t fresh_id, uint32_t composite_id, std::vector<uint32_t>&& index);

  // - |message_.fresh_id| must be available
  // - |message_.instruction_to_insert_before| must identify an instruction
  //   before which it is valid to place an OpCompositeExtract
  // - |message_.composite_id| must be the id of an instruction that defines
  //   a composite object, and this id must be available at the instruction
  //   identified by |message_.instruction_to_insert_before|
  // - |message_.index| must be a suitable set of indices for
  //   |message_.composite_id|, i.e. it must be possible to follow this chain
  //   of indices to reach a sub-object of |message_.composite_id|
  bool IsApplicable(opt::IRContext* context,
                    const FactManager& fact_manager) const override;

  // Adds an OpCompositeConstruct instruction before the instruction identified
  // by |message_.instruction_to_insert_before|, that extracts from
  // |message_.composite_id| via indices |message_.index| into
  // |message_.fresh_id|.  Generates a data synonym fact relating
  // |message_.fresh_id| to the extracted element.
  void Apply(opt::IRContext* context, FactManager* fact_manager) const override;

  protobufs::Transformation ToMessage() const override;

 private:
  protobufs::TransformationCompositeExtract message_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_TRANSFORMATION_COMPOSITE_EXTRACT_H_
