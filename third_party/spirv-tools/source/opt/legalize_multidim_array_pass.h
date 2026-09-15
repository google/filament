// Copyright (c) 2026 Google LLC
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

#ifndef SOURCE_OPT_LEGALIZE_MULTIDIM_ARRAY_PASS_H_
#define SOURCE_OPT_LEGALIZE_MULTIDIM_ARRAY_PASS_H_

#include "source/opt/pass.h"

namespace spvtools {
namespace opt {

// Pass to legalize multidimensional arrays of resources for Vulkan.
// It transforms multidimensional arrays into single-dimensional ones.
class LegalizeMultidimArrayPass : public Pass {
 public:
  const char* name() const override { return "legalize-multidim-array"; }
  Status Process() override;

 private:
  // Returns true if |var| is a multidimensional array of resources.
  bool IsMultidimArrayOfResources(Instruction* var);

  // Flattens the multidimensional array type of |var| and returns the new type
  // id.
  uint32_t FlattenArrayType(Instruction* var);

  // Rewrites all access chains that use |var|.
  bool RewriteAccessChains(Instruction* var, uint32_t old_ptr_type_id);

  // Returns true if all uses of |var| can be legalized.
  bool CanLegalize(Instruction* var);

  // Recursively checks if the uses of |inst| can be legalized.
  bool CheckUse(Instruction* inst, uint32_t max_depth);

  // Returns the dimensions of the array type |type_id|.
  void GetArrayDimensions(uint32_t type_id, std::vector<uint32_t>* dims,
                          uint32_t* element_type_id);
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_LEGALIZE_MULTIDIM_ARRAY_PASS_H_
