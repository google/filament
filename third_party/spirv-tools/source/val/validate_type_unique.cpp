// Copyright (c) 2017 Google Inc.
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

// Ensures type declarations are unique unless allowed by the specification.

#include "source/val/validate.h"

#include "source/diagnostic.h"
#include "source/opcode.h"
#include "source/val/instruction.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {

// Validates that type declarations are unique, unless multiple declarations
// of the same data type are allowed by the specification.
// (see section 2.8 Types and Variables)
// Doesn't do anything if SPV_VAL_ignore_type_decl_unique was declared in the
// module.
spv_result_t TypeUniquePass(ValidationState_t& _, const Instruction* inst) {
  if (_.HasExtension(Extension::kSPV_VALIDATOR_ignore_type_decl_unique))
    return SPV_SUCCESS;

  const SpvOp opcode = inst->opcode();

  if (spvOpcodeGeneratesType(opcode)) {
    if (opcode == SpvOpTypeArray || opcode == SpvOpTypeRuntimeArray ||
        opcode == SpvOpTypeStruct || opcode == SpvOpTypePointer) {
      // Duplicate declarations of aggregates are allowed.
      return SPV_SUCCESS;
    }

    if (!_.RegisterUniqueTypeDeclaration(inst)) {
      return _.diag(SPV_ERROR_INVALID_DATA, inst)
             << "Duplicate non-aggregate type declarations are not allowed."
             << " Opcode: " << spvOpcodeString(SpvOp(inst->opcode()))
             << " id: " << inst->id();
    }
  }

  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
