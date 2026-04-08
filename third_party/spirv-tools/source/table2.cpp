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

// Compressed grammar tables.

#include "source/table2.h"

#include <algorithm>
#include <array>
#include <cstring>

#include "source/extensions.h"
#include "source/latest_version_spirv_header.h"
#include "source/spirv_constant.h"
#include "source/spirv_target_env.h"
#include "spirv-tools/libspirv.hpp"

namespace spvtools {
namespace {

// This is used in the source for the generated tables.
constexpr inline IndexRange IR(uint32_t first, uint32_t count) {
  return IndexRange{first, count};
}

struct NameIndex {
  // Location of the null-terminated name in the global string table kStrings.
  IndexRange name;
  // Index of this name's entry in the corresponding by-value table.
  uint32_t index;
};

struct NameValue {
  // Location of the null-terminated name in the global string table kStrings.
  IndexRange name;
  // Enum value in the binary format.
  uint32_t value;
};

// The generated include file contains variables:
//
//   std::array<NameIndex,...> kOperandNames:
//      Operand names and index, ordered by (operand kind, name)
//      The index part is the named entry's index in kOperandsByValue array.
//      Aliases are included as their own entries.
//
//   std::array<OperandDesc, ...> kOperandsByValue:
//      Operand descriptions, ordered by (operand kind, operand enum value).
//
//   std::array<NameIndex,...> kInstructionNames:
//      Instruction names and index, ordered by (name, value)
//      The index part is the named entry's index in kInstructionDesc array.
//      Aliases are included as their own entries.
//
//   std::array<InstructionDesc, ...> kInstructionDesc
//      Instruction descriptions, ordered by opcode.
//
//   const char kStrings[]
//      Array of characters, referenced by IndexRanges elsewhere.
//      Each IndexRange denotes a string.
//
//   const IndexRange kAliasSpans[]
//      Array of IndexRanges, where each represents a string by referencing
//      the kStrings table.
//      This array contains all sequences of alias strings used in the grammar.
//      This table is referenced by an IndexRange elsewhere, i.e. by the
//      'aliases' field of an instruction or operand description.
//
//   const spv::Capability kCapabilitySpans[]
//      Array of capabilities, referenced by IndexRanges elsewhere.
//      Contains all sequences of capabilities used in the grammar.
//
//   const spvtools::Extension kExtensionSpans[] = {
//      Array of extensions, referenced by IndexRanges elsewhere.
//      Contains all sequences of extensions used in the grammar.
//
//   const spv_operand_type_t kOperandSpans[] = {
//      Array of operand types, referenced by IndexRanges elsewhere.
//      Contains all sequences of operand types used in the grammar.

// Maps an operand kind to NameValue entries for that kind.
// The result is an IndexRange into kOperandNames, and are sorted
// by string name within that span.
IndexRange OperandNameRangeForKind(spv_operand_type_t type);

// Maps an operand kind to possible operands for that kind.
// The result is an IndexRange into kOperandsByValue, and the operands
// are sorted by value within that span.
IndexRange OperandByValueRangeForKind(spv_operand_type_t type);

// Maps an extended instruction set kind to NameValue entries for that kind.
// The result is an IndexRange into kExtIntNames, and are sorted
// by string name within that span.
IndexRange ExtInstNameRangeForKind(spv_ext_inst_type_t type);

// Maps an extended instruction set kind to possible operands for that kind.
// The result is an IndexRange into kExtInstByValue, and the instructions
// are sorted by opcode value within that span.
IndexRange ExtInstByValueRangeForKind(spv_ext_inst_type_t type);

// Returns the name of an extension, as an index into kStrings
IndexRange ExtensionToIndexRange(Extension extension);

#include "core_tables_body.inc"

// Returns a pointer to the null-terminated C-style string in the global
// strings table, as referenced by 'ir'.  Assumes the given range is valid.
const char* getChars(IndexRange ir) {
  assert(ir.first() < sizeof(kStrings));
  return ir.apply(kStrings).data();
}

}  // anonymous namespace

utils::Span<const spv_operand_type_t> OperandDesc::operands() const {
  return operands_range.apply(kOperandSpans);
}
utils::Span<const char> OperandDesc::name() const {
  return name_range.apply(kStrings);
}
utils::Span<const IndexRange> OperandDesc::aliases() const {
  return name_range.apply(kAliasSpans);
}
utils::Span<const spv::Capability> OperandDesc::capabilities() const {
  return capabilities_range.apply(kCapabilitySpans);
}
utils::Span<const spvtools::Extension> OperandDesc::extensions() const {
  return extensions_range.apply(kExtensionSpans);
}

utils::Span<const spv_operand_type_t> InstructionDesc::operands() const {
  return operands_range.apply(kOperandSpans);
}
utils::Span<const char> InstructionDesc::name() const {
  return name_range.apply(kStrings);
}
utils::Span<const IndexRange> InstructionDesc::aliases() const {
  return name_range.apply(kAliasSpans);
}
utils::Span<const spv::Capability> InstructionDesc::capabilities() const {
  return capabilities_range.apply(kCapabilitySpans);
}
utils::Span<const spvtools::Extension> InstructionDesc::extensions() const {
  return extensions_range.apply(kExtensionSpans);
}

utils::Span<const spv_operand_type_t> ExtInstDesc::operands() const {
  return operands_range.apply(kOperandSpans);
}
utils::Span<const char> ExtInstDesc::name() const {
  return name_range.apply(kStrings);
}
utils::Span<const spv::Capability> ExtInstDesc::capabilities() const {
  return capabilities_range.apply(kCapabilitySpans);
}

spv_result_t LookupOpcode(spv::Op opcode, const InstructionDesc** desc) {
  // Metaphor: Look for the needle in the haystack.
  const InstructionDesc needle(opcode);
  auto where = std::lower_bound(
      kInstructionDesc.begin(), kInstructionDesc.end(), needle,
      [&](const InstructionDesc& lhs, const InstructionDesc& rhs) {
        return uint32_t(lhs.opcode) < uint32_t(rhs.opcode);
      });
  if (where != kInstructionDesc.end() && where->opcode == opcode) {
    *desc = &*where;
    return SPV_SUCCESS;
  }
  return SPV_ERROR_INVALID_LOOKUP;
}

spv_result_t LookupOpcode(const char* name, const InstructionDesc** desc) {
  // The comparison function knows to use 'name' string to compare against
  // when the value is kSentinel.
  const auto kSentinel = uint32_t(-1);
  const NameIndex needle{{}, kSentinel};
  auto less = [&](const NameIndex& lhs, const NameIndex& rhs) {
    const char* lhs_chars = lhs.index == kSentinel ? name : getChars(lhs.name);
    const char* rhs_chars = rhs.index == kSentinel ? name : getChars(rhs.name);
    return std::strcmp(lhs_chars, rhs_chars) < 0;
  };

  auto where = std::lower_bound(kInstructionNames.begin(),
                                kInstructionNames.end(), needle, less);
  if (where != kInstructionNames.end() &&
      std::strcmp(getChars(where->name), name) == 0) {
    *desc = &kInstructionDesc[where->index];
    return SPV_SUCCESS;
  }
  return SPV_ERROR_INVALID_LOOKUP;
}

namespace {
template <typename KEY_TYPE>
spv_result_t LookupOpcodeForEnvInternal(spv_target_env env, KEY_TYPE key,
                                        const InstructionDesc** desc) {
  const InstructionDesc* desc_proxy;
  auto status = LookupOpcode(key, &desc_proxy);
  if (status != SPV_SUCCESS) {
    return status;
  }
  const auto& entry = *desc_proxy;
  const auto version = spvVersionForTargetEnv(env);
  if ((version >= entry.minVersion && version <= entry.lastVersion) ||
      entry.extensions_range.count() > 0 ||
      entry.capabilities_range.count() > 0) {
    *desc = desc_proxy;
    return SPV_SUCCESS;
  }
  return SPV_ERROR_INVALID_LOOKUP;
}
}  // namespace

spv_result_t LookupOpcodeForEnv(spv_target_env env, const char* name,
                                const InstructionDesc** desc) {
  return LookupOpcodeForEnvInternal(env, name, desc);
}

spv_result_t LookupOpcodeForEnv(spv_target_env env, spv::Op opcode,
                                const InstructionDesc** desc) {
  return LookupOpcodeForEnvInternal(env, opcode, desc);
}

spv_result_t LookupOperand(spv_operand_type_t type, uint32_t value,
                           const OperandDesc** desc) {
  auto ir = OperandByValueRangeForKind(type);
  if (ir.empty()) {
    return SPV_ERROR_INVALID_LOOKUP;
  }

  auto span = ir.apply(kOperandsByValue.data());

  // Metaphor: Look for the needle in the haystack.
  // The operand value is the first member.
  const OperandDesc needle{value};
  auto where =
      std::lower_bound(span.begin(), span.end(), needle,
                       [&](const OperandDesc& lhs, const OperandDesc& rhs) {
                         return lhs.value < rhs.value;
                       });
  if (where != span.end() && where->value == value) {
    *desc = &*where;
    return SPV_SUCCESS;
  }
  return SPV_ERROR_INVALID_LOOKUP;
}

spv_result_t LookupOperand(spv_operand_type_t type, const char* name,
                           size_t name_len, const OperandDesc** desc) {
  auto ir = OperandNameRangeForKind(type);
  if (ir.empty()) {
    return SPV_ERROR_INVALID_LOOKUP;
  }

  auto span = ir.apply(kOperandNames.data());

  // The comparison function knows to use (name, name_len) as the
  // string to compare against when the value is kSentinel.
  const auto kSentinel = uint32_t(-1);
  const NameIndex needle{{}, kSentinel};
  // The strings in the global string table are null-terminated, and the count
  // reflects that.  So always deduct 1 from its length.
  auto less = [&](const NameIndex& lhs, const NameIndex& rhs) {
    const char* lhs_chars = lhs.index == kSentinel ? name : getChars(lhs.name);
    const char* rhs_chars = rhs.index == kSentinel ? name : getChars(rhs.name);
    const auto content_cmp = std::strncmp(lhs_chars, rhs_chars, name_len);
    if (content_cmp != 0) {
      return content_cmp < 0;
    }
    const auto lhs_len =
        lhs.index == kSentinel ? name_len : lhs.name.count() - 1;
    const auto rhs_len =
        rhs.index == kSentinel ? name_len : rhs.name.count() - 1;
    return lhs_len < rhs_len;
  };

  auto where = std::lower_bound(span.begin(), span.end(), needle, less);
  if (where != span.end() && where->name.count() - 1 == name_len &&
      std::strncmp(getChars(where->name), name, name_len) == 0) {
    *desc = &kOperandsByValue[where->index];
    return SPV_SUCCESS;
  }
  return SPV_ERROR_INVALID_LOOKUP;
}

spv_result_t LookupExtInst(spv_ext_inst_type_t type, const char* name,
                           const ExtInstDesc** desc) {
  auto ir = ExtInstNameRangeForKind(type);
  if (ir.empty()) {
    return SPV_ERROR_INVALID_LOOKUP;
  }

  auto span = ir.apply(kExtInstNames.data());

  // The comparison function knows to use 'name' string to compare against
  // when the value is kSentinel.
  const auto kSentinel = uint32_t(-1);
  const NameIndex needle{{}, kSentinel};
  auto less = [&](const NameIndex& lhs, const NameIndex& rhs) {
    const char* lhs_chars = lhs.index == kSentinel ? name : getChars(lhs.name);
    const char* rhs_chars = rhs.index == kSentinel ? name : getChars(rhs.name);
    return std::strcmp(lhs_chars, rhs_chars) < 0;
  };

  auto where = std::lower_bound(span.begin(), span.end(), needle, less);
  if (where != span.end() && std::strcmp(getChars(where->name), name) == 0) {
    *desc = &kExtInstByValue[where->index];
    return SPV_SUCCESS;
  }
  return SPV_ERROR_INVALID_LOOKUP;
}

// Finds the extended instruction description by opcode value.
// On success, returns SPV_SUCCESS and updates *desc.
spv_result_t LookupExtInst(spv_ext_inst_type_t type, uint32_t value,
                           const ExtInstDesc** desc) {
  auto ir = ExtInstByValueRangeForKind(type);
  if (ir.empty()) {
    return SPV_ERROR_INVALID_LOOKUP;
  }

  auto span = ir.apply(kExtInstByValue.data());

  // Metaphor: Look for the needle in the haystack.
  // The operand value is the first member.
  const ExtInstDesc needle(value);
  auto where =
      std::lower_bound(span.begin(), span.end(), needle,
                       [&](const ExtInstDesc& lhs, const ExtInstDesc& rhs) {
                         return lhs.value < rhs.value;
                       });
  if (where != span.end() && where->value == value) {
    *desc = &*where;
    return SPV_SUCCESS;
  }
  return SPV_ERROR_INVALID_LOOKUP;
}

const char* ExtensionToString(Extension extension) {
  return getChars(ExtensionToIndexRange(extension));
}

bool GetExtensionFromString(const char* name, Extension* extension) {
  // The comparison function knows to use 'name' string to compare against
  // when the value is kSentinel.
  const auto kSentinel = uint32_t(-1);
  const NameValue needle{{}, kSentinel};
  auto less = [&](const NameValue& lhs, const NameValue& rhs) {
    const char* lhs_chars = lhs.value == kSentinel ? name : getChars(lhs.name);
    const char* rhs_chars = rhs.value == kSentinel ? name : getChars(rhs.name);
    return std::strcmp(lhs_chars, rhs_chars) < 0;
  };

  auto where = std::lower_bound(kExtensionNames.begin(), kExtensionNames.end(),
                                needle, less);
  if (where != kExtensionNames.end() &&
      std::strcmp(getChars(where->name), name) == 0) {
    *extension = static_cast<Extension>(where->value);
    return true;
  }
  return false;
}

// This is dirty copy of the spirv.hpp11 function
// TODO - Use a generated version of this function
const char* StorageClassToString(spv::StorageClass value) {
  switch (value) {
    case spv::StorageClass::UniformConstant:
      return "UniformConstant";
    case spv::StorageClass::Input:
      return "Input";
    case spv::StorageClass::Uniform:
      return "Uniform";
    case spv::StorageClass::Output:
      return "Output";
    case spv::StorageClass::Workgroup:
      return "Workgroup";
    case spv::StorageClass::CrossWorkgroup:
      return "CrossWorkgroup";
    case spv::StorageClass::Private:
      return "Private";
    case spv::StorageClass::Function:
      return "Function";
    case spv::StorageClass::Generic:
      return "Generic";
    case spv::StorageClass::PushConstant:
      return "PushConstant";
    case spv::StorageClass::AtomicCounter:
      return "AtomicCounter";
    case spv::StorageClass::Image:
      return "Image";
    case spv::StorageClass::StorageBuffer:
      return "StorageBuffer";
    case spv::StorageClass::TileImageEXT:
      return "TileImageEXT";
    case spv::StorageClass::TileAttachmentQCOM:
      return "TileAttachmentQCOM";
    case spv::StorageClass::NodePayloadAMDX:
      return "NodePayloadAMDX";
    case spv::StorageClass::CallableDataKHR:
      return "CallableDataKHR";
    case spv::StorageClass::IncomingCallableDataKHR:
      return "IncomingCallableDataKHR";
    case spv::StorageClass::RayPayloadKHR:
      return "RayPayloadKHR";
    case spv::StorageClass::HitAttributeKHR:
      return "HitAttributeKHR";
    case spv::StorageClass::IncomingRayPayloadKHR:
      return "IncomingRayPayloadKHR";
    case spv::StorageClass::ShaderRecordBufferKHR:
      return "ShaderRecordBufferKHR";
    case spv::StorageClass::PhysicalStorageBuffer:
      return "PhysicalStorageBuffer";
    case spv::StorageClass::HitObjectAttributeNV:
      return "HitObjectAttributeNV";
    case spv::StorageClass::TaskPayloadWorkgroupEXT:
      return "TaskPayloadWorkgroupEXT";
    case spv::StorageClass::CodeSectionINTEL:
      return "CodeSectionINTEL";
    case spv::StorageClass::DeviceOnlyINTEL:
      return "DeviceOnlyINTEL";
    case spv::StorageClass::HostOnlyINTEL:
      return "HostOnlyINTEL";
    default:
      return "Unknown";
  }
}

}  // namespace spvtools
