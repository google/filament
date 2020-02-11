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

#ifndef SOURCE_FUZZ_TRANSFORMATION_ADD_GLOBAL_VARIABLE_H_
#define SOURCE_FUZZ_TRANSFORMATION_ADD_GLOBAL_VARIABLE_H_

#include "source/fuzz/fact_manager.h"
#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

class TransformationAddGlobalVariable : public Transformation {
 public:
  explicit TransformationAddGlobalVariable(
      const protobufs::TransformationAddGlobalVariable& message);

  TransformationAddGlobalVariable(uint32_t fresh_id, uint32_t type_id,
                                  uint32_t initializer_id,
                                  bool value_is_arbitrary);

  // - |message_.fresh_id| must be fresh
  // - |message_.type_id| must be the id of a pointer type with Private storage
  //   class
  // - |message_.initializer_id| must either be 0 or the id of a constant whose
  //   type is the pointee type of |message_.type_id|
  bool IsApplicable(opt::IRContext* context,
                    const FactManager& fact_manager) const override;

  // Adds a global variable with Private storage class to the module, with type
  // |message_.type_id| and either no initializer or |message_.initializer_id|
  // as an initializer, depending on whether |message_.initializer_id| is 0.
  // The global variable has result id |message_.fresh_id|.
  void Apply(opt::IRContext* context, FactManager* fact_manager) const override;

  protobufs::Transformation ToMessage() const override;

 private:
  static bool PrivateGlobalsMustBeDeclaredInEntryPointInterfaces(
      opt::IRContext* context);

  protobufs::TransformationAddGlobalVariable message_;
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_TRANSFORMATION_ADD_GLOBAL_VARIABLE_H_
