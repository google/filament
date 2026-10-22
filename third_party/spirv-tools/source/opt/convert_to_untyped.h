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

#ifndef LIBSPIRV_OPT_CONVERT_TO_UNTYPED_H_
#define LIBSPIRV_OPT_CONVERT_TO_UNTYPED_H_

#include <unordered_map>
#include <utility>

#include "pass.h"

namespace spvtools {
namespace opt {

class ConvertToUntyped : public Pass {
 public:
  const char* name() const override { return "convert-to-untyped"; }
  Status Process() override;

 private:
  // Adds the untyped pointer extension and capability if they are not present.
  void AddUntypedPointerCapability();

  // Updates the module by converting instructions and replacing uses.
  //
  // The TypeManager is not updated as the mutations occur and all instructions
  // continue to use old ids (mapped to correctly typed pointers) until after
  // conversions. Then all operands are updated (and unneeded instructions
  // removed).
  //
  // Returns true if conversions were made.
  bool ConvertPointers();

  // Returns true if sc is supports untyped pointers.
  bool SupportedStorageClass(spv::StorageClass sc);
  bool SupportedStorageClass(uint32_t sc);

  // Returns true if inst needs conversion.
  bool ShouldConvert(const Instruction* inst);

  // Returns true if the module has unsupported features.
  bool HasUnsupportedFeatures();

  // Determines whether workgroup storage class can be converted.
  void CheckWorkgroup();

  // Converts or updates inst.
  void Convert(Instruction* inst);

  // Converts OpTypePointer to OpTypeUntypedPointerKHR
  // All undecorated pointers in each storage class are de-duplicated.
  Instruction* ConvertPointerType(Instruction* inst);

  // Converts OpVariable to OpUntypedVariableKHR
  void ConvertVariable(Instruction* inst);

  // Converts access chains to equivalent untyped version.
  void ConvertAccessChain(Instruction* inst);

  // Converts OpArrayLength to OpUntypedArrayLengthKHR.
  void ConvertArrayLength(Instruction* inst);

  // Converts OpCopyMemory.
  // If the copy is a matrix or the column of a matrix, it is converted to loads
  // and stores element-wise.
  // Otherwise, converts the copy to a load and store.
  void ConvertCopyMemory(Instruction* inst);

  // Updates cooperative matrix load/store strides to be in bytes.
  void UpdateCooperativeMatrixLoadStore(Instruction* inst);

  uint32_t NextId();

  // Returns (row_major, matrix_stride) for the inst.
  std::pair<bool, uint32_t> MatrixStride(Instruction* inst);

  // Memory access operand conversion (from OpCopyMemory)
  // Converts operands into appropriate memory access operands for OpStore.
  std::vector<Operand> StoreOperands(const std::vector<Operand>& operands,
                                     uint32_t align);
  // Converts operands into appropriate memory access operands for OpLoad.
  std::vector<Operand> LoadOperands(const std::vector<Operand>& operands,
                                    uint32_t align);

  // Tracks the de-duplicated pointers per storage class.
  std::unordered_map<uint32_t, Instruction*> base_ptrs_{};
  // Tracks result id to its replacement result id
  std::unordered_map<uint32_t, uint32_t> remapped_ids_{};
  // Converted instructions that need deleted.
  std::vector<Instruction*> to_delete_{};

  bool support_workgroup_ = false;
};

}  // namespace opt
}  // namespace spvtools
#endif  // LIBSPIRV_OPT_CONVERT_TO_UNTYPED_H_
