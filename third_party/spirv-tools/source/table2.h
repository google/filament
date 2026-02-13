// Copyright (c) 2025 The Khronos Group Inc.
// Copyright (c) 2025 Google LLC
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

#ifndef SOURCE_TABLE2_H_
#define SOURCE_TABLE2_H_

#include "source/latest_version_spirv_header.h"
#include "source/util/index_range.h"
#include "spirv-tools/libspirv.hpp"

// Define the objects that describe the grammatical structure of SPIR-V
// instructions and their operands. The objects are owned by static
// tables populated at C++ build time from the grammar files from SPIRV-Headers.
//
// Clients use freestanding methods to lookup an opcode or an operand, either
// by numeric value (in the binary), or by name.
//
// For historical reasons, the opcode lookup can also use a target enviroment
// enum to filter for opcodes supported in that environment.
//
// It should be very fast for the system loader to load (and possibly relocate)
// the static tables.  In particular, there should be very few global symbols
// with independent addresses. Prefer a very few large tables of items rather
// than dozens or hundreds of global symbols.
//
// The overall structure among containers (i.e. skipping scalar data members)
// is as follows:
//
//    An OperandDesc describes an operand.
//    An InstructionDesc desribes an instruction.
//    An ExtInstDesc describes an extended intruction.
//
//    Both OperandDesc and InstructionDesc have members:
//      - a name string
//      - array of alias strings
//      - array of spv::Capability      (as an enum)
//      - array of spv_operand_type_t   (as an enum)
//      - array of spvtools::Extension  (as an enum)
//      - a minVersion
//      - a lastVersion
//
//    An OperandDesc also has:
//      - a uint32_t value.
//
//    An InstructionDesc also has:
//      - a spv::Op opcode
//      - a bool hasResult
//      - a bool hasType
//      - a printing class
//
//    An ExtInstDesc has:
//      - a name
//      - array of spv::Capability      (as an enum)
//      - array of spv_operand_type_t   (as an enum)
//
// The arrays are represented by spans into a global static array, with one
// array for each of:
//      - null-terminated strings, for names
//      - arrays of null-terminated strings, for alias lists
//      - spv_operand_type_t
//      - spv::Capability
//      - spvtools::Extension
//
// Note: Currently alias lists never have more than one element.
// The data structures and code do not assume this.

// TODO(dneto): convert the tables for extended instructions
// Currently (as defined in table.h):
//    An spv_ext_inst_group_t has:
//      - array of spv_ext_inst_desc_t
//
//    An spv_ext_inst_desc_t has:
//      - a name string
//      - array of spv::Capability
//      - array of spv_operand_type_t

namespace spvtools {

#include "core_tables_header.inc"

using IndexRange = utils::IndexRange<uint32_t, uint32_t>;

// Describes a SPIR-V operand.
struct OperandDesc {
  const uint32_t value;

  const IndexRange operands_range;      // Indexes kOperandSpans
  const IndexRange name_range;          // Indexes kStrings
  const IndexRange aliases_range;       // Indexes kAliasSpans
  const IndexRange capabilities_range;  // Indexes kCapabilitySpans
  // A set of extensions that enable this feature. If empty then this operand
  // value is in core and its availability is subject to minVersion. The
  // assembler, binary parser, and disassembler ignore this rule, so you can
  // freely process invalid modules.
  const IndexRange extensions_range;  // Indexes kExtensionSpans
  // Minimal core SPIR-V version required for this feature, if without
  // extensions. ~0u means reserved for future use. ~0u and non-empty
  // extension lists means only available in extensions.
  const uint32_t minVersion = 0xFFFFFFFFu;
  const uint32_t lastVersion = 0xFFFFFFFFu;
  utils::Span<const spv_operand_type_t> operands() const;
  utils::Span<const char> name() const;
  utils::Span<const IndexRange> aliases() const;
  utils::Span<const spv::Capability> capabilities() const;
  utils::Span<const spvtools::Extension> extensions() const;

  constexpr OperandDesc(uint32_t v, IndexRange o, IndexRange n, IndexRange a,
                        IndexRange c, IndexRange e, uint32_t mv, uint32_t lv)
      : value(v),
        operands_range(o),
        name_range(n),
        aliases_range(a),
        capabilities_range(c),
        extensions_range(e),
        minVersion(mv),
        lastVersion(lv) {}

  constexpr OperandDesc(uint32_t v) : value(v) {}

  OperandDesc(const OperandDesc&) = delete;
  OperandDesc(OperandDesc&&) = delete;
};

// Describes an Instruction
struct InstructionDesc {
  const spv::Op opcode;
  const bool hasResult = false;
  const bool hasType = false;

  const IndexRange operands_range;      // Indexes kOperandSpans
  const IndexRange name_range;          // Indexes kStrings
  const IndexRange aliases_range;       // Indexes kAliasSpans
  const IndexRange capabilities_range;  // Indexes kCapbilitySpans
  // A set of extensions that enable this feature. If empty then this operand
  // value is in core and its availability is subject to minVersion. The
  // assembler, binary parser, and disassembler ignore this rule, so you can
  // freely process invalid modules.
  const IndexRange extensions_range;  // Indexes kExtensionSpans
  // Minimal core SPIR-V version required for this feature, if without
  // extensions. ~0u means reserved for future use. ~0u and non-empty
  // extension lists means only available in extensions.
  const uint32_t minVersion = 0xFFFFFFFFu;
  const uint32_t lastVersion = 0xFFFFFFFFu;
  // The printing class specifies what kind of instruction it is, e.g. what
  // section of the SPIR-V spec. E.g. kImage, kComposite
  const PrintingClass printingClass = PrintingClass::kReserved;
  // Returns the span of elements in the global grammar tables corresponding
  // to the privately-stored index ranges
  utils::Span<const spv_operand_type_t> operands() const;
  utils::Span<const char> name() const;
  utils::Span<const IndexRange> aliases() const;
  utils::Span<const spv::Capability> capabilities() const;
  utils::Span<const spvtools::Extension> extensions() const;

  constexpr InstructionDesc(spv::Op oc, bool hr, bool ht, IndexRange o,
                            IndexRange n, IndexRange a, IndexRange c,
                            IndexRange e, uint32_t mv, uint32_t lv,
                            PrintingClass pc)
      : opcode(oc),
        hasResult(hr),
        hasType(ht),
        operands_range(o),
        name_range(n),
        aliases_range(a),
        capabilities_range(c),
        extensions_range(e),
        minVersion(mv),
        lastVersion(lv),
        printingClass(pc) {}

  constexpr InstructionDesc(spv::Op oc) : opcode(oc) {}

  InstructionDesc(const InstructionDesc&) = delete;
  InstructionDesc(InstructionDesc&&) = delete;
};

// Describes an extended instruction
struct ExtInstDesc {
  const uint32_t value;
  const IndexRange operands_range;      // Indexes kOperandSpans
  const IndexRange name_range;          // Indexes kStrings
  const IndexRange capabilities_range;  // Indexes kCapbilitySpans
  // Returns the span of elements in the global grammar tables corresponding
  // to the privately-stored index ranges
  utils::Span<const spv_operand_type_t> operands() const;
  utils::Span<const char> name() const;
  utils::Span<const spv::Capability> capabilities() const;

  constexpr ExtInstDesc(uint32_t v, IndexRange o, IndexRange n, IndexRange c)
      : value(v), operands_range(o), name_range(n), capabilities_range(c) {}

  constexpr ExtInstDesc(uint32_t v) : value(v) {}

  ExtInstDesc(const ExtInstDesc&) = delete;
  ExtInstDesc(ExtInstDesc&&) = delete;
};

// Finds the instruction description by opcode name. The name should not
// have the "Op" prefix. On success, returns SPV_SUCCESS and updates *desc.
spv_result_t LookupOpcode(const char* name, const InstructionDesc** desc);
// Finds the instruction description by opcode value.
// On success, returns SPV_SUCCESS and updates *desc.
spv_result_t LookupOpcode(spv::Op opcode, const InstructionDesc** desc);

// Finds the instruction description by opcode name, without the "Op" prefix.
// A lookup will succeed if:
// - The instruction exists, and
// - Either the target environment supports the SPIR-V version of the
// instruction,
//   or the instruction is enabled by at least one extension,
//   or the instruction is enabled by at least one capability.,
// On success, returns SPV_SUCCESS and updates *desc.
spv_result_t LookupOpcodeForEnv(spv_target_env env, const char* name,
                                const InstructionDesc** desc);

// Finds the instruction description by opcode value.
// A lookup will succeed if:
// - The instruction exists, and
// - Either the target environment supports the SPIR-V version of the
// instruction,
//   or the instruction is enabled by at least one extension,
//   or the instruction is enabled by at least one capability.,
// On success, returns SPV_SUCCESS and updates *desc.
spv_result_t LookupOpcodeForEnv(spv_target_env env, spv::Op,
                                const InstructionDesc** desc);

spv_result_t LookupOperand(spv_operand_type_t type, const char* name,
                           size_t name_len, const OperandDesc** desc);
spv_result_t LookupOperand(spv_operand_type_t type, uint32_t operand,
                           const OperandDesc** desc);

// Finds the extended instruction description by opcode name.
// On success, returns SPV_SUCCESS and updates *desc.
spv_result_t LookupExtInst(spv_ext_inst_type_t type, const char* name,
                           const ExtInstDesc** desc);
// Finds the extended instruction description by opcode value.
// On success, returns SPV_SUCCESS and updates *desc.
spv_result_t LookupExtInst(spv_ext_inst_type_t type, uint32_t value,
                           const ExtInstDesc** desc);

// Finds Extension enum corresponding to |str|. Returns false if not found.
bool GetExtensionFromString(const char* str, Extension* extension);

// Returns text string corresponding to |extension|.
const char* ExtensionToString(Extension extension);

}  // namespace spvtools
#endif  // SOURCE_TABLE2_H_
