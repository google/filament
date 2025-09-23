// Copyright (c) 2015-2016 The Khronos Group Inc.
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

#ifndef SOURCE_ASSEMBLY_GRAMMAR_H_
#define SOURCE_ASSEMBLY_GRAMMAR_H_

#include "source/enum_set.h"
#include "source/latest_version_spirv_header.h"
#include "source/operand.h"
#include "source/table.h"
#include "source/util/span.h"
#include "spirv-tools/libspirv.h"

namespace spvtools {

// Encapsulates the grammar to use for SPIR-V assembly.
// Contains methods to query for valid instructions and operands.
class AssemblyGrammar {
 public:
  explicit AssemblyGrammar(const spv_const_context context)
      : target_env_(context->target_env) {}

  // Returns the SPIR-V target environment.
  spv_target_env target_env() const { return target_env_; }

  // Removes capabilities not available in the current target environment and
  // returns the rest.
  // TODO(crbug.com/266223071) Remove this.
  CapabilitySet filterCapsAgainstTargetEnv(const spv::Capability* cap_array,
                                           uint32_t count) const;
  // Removes capabilities not available in the current target environment and
  // returns the rest.
  CapabilitySet filterCapsAgainstTargetEnv(
      const spvtools::utils::Span<const spv::Capability>& caps) const {
    return filterCapsAgainstTargetEnv(caps.begin(),
                                      static_cast<uint32_t>(caps.size()));
  }

  // Finds operand entry in the grammar table and returns its name.
  // Returns "Unknown" if not found.
  const char* lookupOperandName(spv_operand_type_t type,
                                uint32_t operand) const;

  // Finds the opcode for the given OpSpecConstantOp opcode name. The name
  // should not have the "Op" prefix.  For example, "IAdd" corresponds to
  // the integer add opcode for OpSpecConstantOp.  On success, returns
  // SPV_SUCCESS and sends the discovered operation code through the opcode
  // parameter.  On failure, returns SPV_ERROR_INVALID_LOOKUP.
  spv_result_t lookupSpecConstantOpcode(const char* name,
                                        spv::Op* opcode) const;

  // Returns SPV_SUCCESS if the given opcode is valid as the opcode operand
  // to OpSpecConstantOp.
  spv_result_t lookupSpecConstantOpcode(spv::Op opcode) const;

  // Parses a mask expression string for the given operand type.
  //
  // A mask expression is a sequence of one or more terms separated by '|',
  // where each term is a named enum value for a given type. No whitespace
  // is permitted.
  //
  // On success, the value is written to pValue, and SPV_SUCCESS is returned.
  // The operand type is defined by the type parameter, and the text to be
  // parsed is defined by the textValue parameter.
  spv_result_t parseMaskOperand(const spv_operand_type_t type,
                                const char* textValue, uint32_t* pValue) const;

  // Inserts the operands expected after the given typed mask onto the end
  // of the given pattern.
  //
  // Each set bit in the mask represents zero or more operand types that
  // should be appended onto the pattern. Operands for a less significant
  // bit must always match before operands for a more significant bit, so
  // the operands for a less significant bit must appear closer to the end
  // of the pattern stack.
  //
  // If a set bit is unknown, then we assume it has no operands.
  void pushOperandTypesForMask(const spv_operand_type_t type,
                               const uint32_t mask,
                               spv_operand_pattern_t* pattern) const;

 private:
  const spv_target_env target_env_;
};

}  // namespace spvtools

#endif  // SOURCE_ASSEMBLY_GRAMMAR_H_
