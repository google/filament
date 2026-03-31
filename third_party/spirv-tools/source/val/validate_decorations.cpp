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

#include <algorithm>
#include <cassert>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "source/diagnostic.h"
#include "source/opcode.h"
#include "source/spirv_constant.h"
#include "source/spirv_target_env.h"
#include "source/spirv_validator_options.h"
#include "source/util/string_utils.h"
#include "source/val/validate_scopes.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

// Distinguish between row and column major matrix layouts.
enum MatrixLayout { kRowMajor, kColumnMajor };

// A functor for hashing a pair of integers.
struct PairHash {
  std::size_t operator()(const std::pair<uint32_t, uint32_t> pair) const {
    const uint32_t a = pair.first;
    const uint32_t b = pair.second;
    const uint32_t rotated_b = (b >> 2) | ((b & 3) << 30);
    return a ^ rotated_b;
  }
};

// Struct member layout attributes that are inherited through arrays.
struct LayoutConstraints {
  explicit LayoutConstraints(
      MatrixLayout the_majorness = MatrixLayout::kColumnMajor,
      uint32_t stride = 0)
      : majorness(the_majorness), matrix_stride(stride) {}
  MatrixLayout majorness;
  uint32_t matrix_stride;
};

// A type for mapping (struct id, member id) to layout constraints.
using MemberConstraints = std::unordered_map<std::pair<uint32_t, uint32_t>,
                                             LayoutConstraints, PairHash>;

// Returns the array stride of the given array type.
uint32_t GetArrayStride(uint32_t array_id, ValidationState_t& vstate) {
  for (auto& decoration : vstate.id_decorations(array_id)) {
    if (spv::Decoration::ArrayStride == decoration.dec_type()) {
      return decoration.params()[0];
    }
  }
  return 0;
}

// Returns true if the given structure type has a Block decoration.
bool isBlock(uint32_t struct_id, ValidationState_t& vstate) {
  const auto& decorations = vstate.id_decorations(struct_id);
  return std::any_of(decorations.begin(), decorations.end(),
                     [](const Decoration& d) {
                       return spv::Decoration::Block == d.dec_type();
                     });
}

// Returns true if the given ID has the Import LinkageAttributes decoration.
bool hasImportLinkageAttribute(uint32_t id, ValidationState_t& vstate) {
  const auto& decorations = vstate.id_decorations(id);
  return std::any_of(
      decorations.begin(), decorations.end(), [](const Decoration& d) {
        return spv::Decoration::LinkageAttributes == d.dec_type() &&
               d.params().size() >= 2u &&
               spv::LinkageType(d.params().back()) == spv::LinkageType::Import;
      });
}

// Returns a vector of all members of a structure.
std::vector<uint32_t> getStructMembers(uint32_t struct_id,
                                       ValidationState_t& vstate) {
  const auto inst = vstate.FindDef(struct_id);
  return std::vector<uint32_t>(inst->words().begin() + 2, inst->words().end());
}

// Returns a vector of all members of a structure that have specific type.
std::vector<uint32_t> getStructMembers(uint32_t struct_id, spv::Op type,
                                       ValidationState_t& vstate) {
  std::vector<uint32_t> members;
  for (auto id : getStructMembers(struct_id, vstate)) {
    if (type == vstate.FindDef(id)->opcode()) {
      members.push_back(id);
    }
  }
  return members;
}

// Returns whether the given structure is missing Offset decoration for any
// member. Handles also nested structures.
bool isMissingOffsetInStruct(uint32_t struct_id, ValidationState_t& vstate) {
  const auto* inst = vstate.FindDef(struct_id);
  std::vector<bool> hasOffset;
  std::vector<uint32_t> struct_members;
  if (inst->opcode() == spv::Op::OpTypeStruct) {
    // Check offsets of member decorations.
    struct_members = getStructMembers(struct_id, vstate);
    hasOffset.resize(struct_members.size(), false);

    for (auto& decoration : vstate.id_decorations(struct_id)) {
      if ((spv::Decoration::Offset == decoration.dec_type() ||
           spv::Decoration::OffsetIdEXT == decoration.dec_type()) &&
          Decoration::kInvalidMember != decoration.struct_member_index()) {
        // Offset 0xffffffff is not valid so ignore it for simplicity's sake.
        if (decoration.params()[0] == 0xffffffff) return true;
        hasOffset[decoration.struct_member_index()] = true;
      }
    }
  } else if (inst->opcode() == spv::Op::OpTypeArray ||
             inst->opcode() == spv::Op::OpTypeRuntimeArray) {
    hasOffset.resize(1, true);
    struct_members.push_back(inst->GetOperandAs<uint32_t>(1u));
  }
  // Look through nested structs (which may be in an array).
  bool nestedStructsMissingOffset = false;
  for (auto id : struct_members) {
    if (isMissingOffsetInStruct(id, vstate)) {
      nestedStructsMissingOffset = true;
      break;
    }
  }
  return nestedStructsMissingOffset ||
         !std::all_of(hasOffset.begin(), hasOffset.end(),
                      [](const bool b) { return b; });
}

// Rounds x up to the next alignment. Assumes alignment is a power of two.
uint32_t align(uint32_t x, uint32_t alignment) {
  return (x + alignment - 1) & ~(alignment - 1);
}

// Returns base alignment of struct member. If |roundUp| is true, also
// ensure that structs, arrays, and matrices are aligned at least to a
// multiple of 16 bytes.  (That is, when roundUp is true, this function
// returns the *extended* alignment as it's called by the Vulkan spec.)
uint32_t getBaseAlignment(uint32_t member_id, bool roundUp,
                          const LayoutConstraints& inherited,
                          MemberConstraints& constraints,
                          ValidationState_t& vstate) {
  const auto inst = vstate.FindDef(member_id);
  const auto& words = inst->words();
  // Minimal alignment is byte-aligned.
  uint32_t baseAlignment = 1;
  switch (inst->opcode()) {
    case spv::Op::OpTypeSampledImage:
    case spv::Op::OpTypeSampler:
    case spv::Op::OpTypeImage:
      if (vstate.HasCapability(spv::Capability::BindlessTextureNV))
        return vstate.samplerimage_variable_address_mode() / 8;
      // SPV_EXT_descriptor_heap provides a way to access opaque images, we
      // assume alignment is validated at runtime as it is determined by the
      // client API
      if (vstate.HasCapability(spv::Capability::DescriptorHeapEXT)) return 1;
      assert(0);
      return 0;
    case spv::Op::OpTypeInt:
    case spv::Op::OpTypeFloat:
      baseAlignment = words[2] / 8;
      break;
    case spv::Op::OpTypeVector: {
      const auto componentId = words[2];
      const auto numComponents = words[3];
      const auto componentAlignment = getBaseAlignment(
          componentId, roundUp, inherited, constraints, vstate);
      baseAlignment =
          componentAlignment *
          ((numComponents == 3 || numComponents > 4) ? 4 : numComponents);
      break;
    }
    case spv::Op::OpTypeVectorIdEXT: {
      const auto componentId = words[2];
      const auto numComponents = vstate.GetDimension(inst->id());
      assert(numComponents != 0);
      const auto componentAlignment = getBaseAlignment(
          componentId, roundUp, inherited, constraints, vstate);
      baseAlignment =
          componentAlignment *
          ((numComponents == 3 || numComponents > 4) ? 4 : numComponents);
      break;
    }
    case spv::Op::OpTypeMatrix: {
      const auto column_type = words[2];
      if (inherited.majorness == kColumnMajor) {
        baseAlignment = getBaseAlignment(column_type, roundUp, inherited,
                                         constraints, vstate);
      } else {
        // A row-major matrix of C columns has a base alignment equal to the
        // base alignment of a vector of C matrix components.
        const auto num_columns = words[3];
        const auto component_inst = vstate.FindDef(column_type);
        const auto component_id = component_inst->words()[2];
        const auto componentAlignment = getBaseAlignment(
            component_id, roundUp, inherited, constraints, vstate);
        baseAlignment =
            componentAlignment * (num_columns == 3 ? 4 : num_columns);
      }
      if (roundUp) baseAlignment = align(baseAlignment, 16u);
    } break;
    case spv::Op::OpTypeArray:
    case spv::Op::OpTypeRuntimeArray:
      baseAlignment =
          getBaseAlignment(words[2], roundUp, inherited, constraints, vstate);
      if (roundUp) baseAlignment = align(baseAlignment, 16u);
      break;
    case spv::Op::OpTypeStruct: {
      const auto members = getStructMembers(member_id, vstate);
      for (uint32_t memberIdx = 0, numMembers = uint32_t(members.size());
           memberIdx < numMembers; ++memberIdx) {
        const auto id = members[memberIdx];
        const auto& constraint =
            constraints[std::make_pair(member_id, memberIdx)];
        baseAlignment = std::max(
            baseAlignment,
            getBaseAlignment(id, roundUp, constraint, constraints, vstate));
      }
      if (roundUp) baseAlignment = align(baseAlignment, 16u);
      break;
    }
    case spv::Op::OpTypePointer:
    case spv::Op::OpTypeUntypedPointerKHR:
      baseAlignment = vstate.pointer_size_and_alignment();
      break;
    default:
      assert(0);
      break;
  }

  return baseAlignment;
}

// Returns scalar alignment of a type.
uint32_t getScalarAlignment(uint32_t type_id, ValidationState_t& vstate) {
  const auto inst = vstate.FindDef(type_id);
  const auto& words = inst->words();
  switch (inst->opcode()) {
    case spv::Op::OpTypeSampledImage:
    case spv::Op::OpTypeSampler:
    case spv::Op::OpTypeImage:
      if (vstate.HasCapability(spv::Capability::BindlessTextureNV))
        return vstate.samplerimage_variable_address_mode() / 8;
      // SPV_EXT_descriptor_heap provides a way to access opaque images, we
      // assume alignment is validated at runtime as it is determined by the
      // client API
      if (vstate.HasCapability(spv::Capability::DescriptorHeapEXT)) return 1;
      assert(0);
      return 0;
    case spv::Op::OpTypeInt:
    case spv::Op::OpTypeFloat:
      return words[2] / 8;
    case spv::Op::OpTypeVector:
    case spv::Op::OpTypeVectorIdEXT:
    case spv::Op::OpTypeMatrix:
    case spv::Op::OpTypeArray:
    case spv::Op::OpTypeRuntimeArray: {
      const auto compositeMemberTypeId = words[2];
      return getScalarAlignment(compositeMemberTypeId, vstate);
    }
    case spv::Op::OpTypeStruct: {
      const auto members = getStructMembers(type_id, vstate);
      uint32_t max_member_alignment = 1;
      for (uint32_t memberIdx = 0, numMembers = uint32_t(members.size());
           memberIdx < numMembers; ++memberIdx) {
        const auto id = members[memberIdx];
        uint32_t member_alignment = getScalarAlignment(id, vstate);
        if (member_alignment > max_member_alignment) {
          max_member_alignment = member_alignment;
        }
      }
      return max_member_alignment;
    } break;
    case spv::Op::OpTypePointer:
    case spv::Op::OpTypeUntypedPointerKHR:
      return vstate.pointer_size_and_alignment();
    default:
      assert(0);
      break;
  }

  return 1;
}

// Returns size of a struct member. Doesn't include padding at the end of struct
// or array.  Assumes that in the struct case, all members have offsets.
uint32_t getSize(uint32_t member_id, const LayoutConstraints& inherited,
                 MemberConstraints& constraints, ValidationState_t& vstate) {
  const auto inst = vstate.FindDef(member_id);
  const auto& words = inst->words();
  switch (inst->opcode()) {
    case spv::Op::OpTypeSampledImage:
    case spv::Op::OpTypeSampler:
    case spv::Op::OpTypeImage:
      if (vstate.HasCapability(spv::Capability::BindlessTextureNV))
        return vstate.samplerimage_variable_address_mode() / 8;
      // SPV_EXT_descriptor_heap provides a way to access opaque images, we
      // assume alignment is validated at runtime as it is determined by the
      // client API
      if (vstate.HasCapability(spv::Capability::DescriptorHeapEXT)) return 1;
      assert(0);
      return 0;
    case spv::Op::OpTypeInt:
    case spv::Op::OpTypeFloat:
      return words[2] / 8;
    case spv::Op::OpTypeVector: {
      const auto componentId = words[2];
      const auto numComponents = words[3];
      const auto componentSize =
          getSize(componentId, inherited, constraints, vstate);
      const auto size = componentSize * numComponents;
      return size;
    }
    case spv::Op::OpTypeVectorIdEXT: {
      const auto componentId = words[2];
      const auto numComponents = vstate.GetDimension(inst->id());
      assert(numComponents != 0);
      const auto componentSize =
          getSize(componentId, inherited, constraints, vstate);
      const auto size = componentSize * numComponents;
      return size;
    }
    case spv::Op::OpTypeArray: {
      const auto sizeInst = vstate.FindDef(words[3]);
      if (spvOpcodeIsSpecConstant(sizeInst->opcode())) return 0;
      assert(spv::Op::OpConstant == sizeInst->opcode());
      const uint32_t num_elem = sizeInst->words()[3];
      const uint32_t elem_type = words[2];
      const uint32_t elem_size =
          getSize(elem_type, inherited, constraints, vstate);
      // Account for gaps due to alignments in the first N-1 elements,
      // then add the size of the last element.
      const auto size =
          (num_elem - 1) * GetArrayStride(member_id, vstate) + elem_size;
      return size;
    }
    case spv::Op::OpTypeRuntimeArray:
      return 0;
    case spv::Op::OpTypeMatrix: {
      const auto num_columns = words[3];
      if (inherited.majorness == kColumnMajor) {
        return num_columns * inherited.matrix_stride;
      } else {
        // Row major case.
        const auto column_type = words[2];
        const auto component_inst = vstate.FindDef(column_type);
        const auto num_rows = component_inst->words()[3];
        const auto scalar_elem_type = component_inst->words()[2];
        const uint32_t scalar_elem_size =
            getSize(scalar_elem_type, inherited, constraints, vstate);
        return (num_rows - 1) * inherited.matrix_stride +
               num_columns * scalar_elem_size;
      }
    }
    case spv::Op::OpTypeStruct: {
      const auto& members = getStructMembers(member_id, vstate);
      if (members.empty()) return 0;
      const auto lastIdx = uint32_t(members.size() - 1);
      const auto& lastMember = members.back();
      uint32_t offset = 0xffffffff;
      // Find the offset of the last element and add the size.
      auto member_decorations =
          vstate.id_member_decorations(member_id, lastIdx);
      for (auto decoration = member_decorations.begin;
           decoration != member_decorations.end; ++decoration) {
        assert(decoration->struct_member_index() == (int)lastIdx);
        if (spv::Decoration::Offset == decoration->dec_type()) {
          offset = decoration->params()[0];
        }
      }
      // This check depends on the fact that all members have offsets.  This
      // has been checked earlier in the flow.
      assert(offset != 0xffffffff);
      const auto& constraint = constraints[std::make_pair(lastMember, lastIdx)];
      return offset + getSize(lastMember, constraint, constraints, vstate);
    }
    case spv::Op::OpTypePointer:
    case spv::Op::OpTypeUntypedPointerKHR:
      return vstate.pointer_size_and_alignment();
    default:
      assert(0);
      return 0;
  }
}

// A member is defined to improperly straddle if either of the following are
// true:
// - It is a vector with total size less than or equal to 16 bytes, and has
// Offset decorations placing its first byte at F and its last byte at L, where
// floor(F / 16) != floor(L / 16).
// - It is a vector with total size greater than 16 bytes and has its Offset
// decorations placing its first byte at a non-integer multiple of 16.
bool hasImproperStraddle(uint32_t id, uint32_t offset,
                         const LayoutConstraints& inherited,
                         MemberConstraints& constraints,
                         ValidationState_t& vstate) {
  const auto size = getSize(id, inherited, constraints, vstate);
  const auto F = offset;
  const auto L = offset + size - 1;
  if (size <= 16) {
    if ((F >> 4) != (L >> 4)) return true;
  } else {
    if (F % 16 != 0) return true;
  }
  return false;
}

// Returns true if |offset| satsifies an alignment to |alignment|.  In the case
// of |alignment| of zero, the |offset| must also be zero.
bool IsAlignedTo(uint32_t offset, uint32_t alignment) {
  if (alignment == 0) return offset == 0;
  return 0 == (offset % alignment);
}

// Returns SPV_SUCCESS if the given struct satisfies standard layout rules for
// Block or BufferBlocks in Vulkan.  Otherwise emits a diagnostic and returns
// something other than SPV_SUCCESS.  Matrices inherit the specified column
// or row major-ness.
spv_result_t checkLayout(uint32_t struct_id, spv::StorageClass storage_class,
                         const char* decoration_str, bool blockRules,
                         bool scalar_block_layout, uint32_t incoming_offset,
                         MemberConstraints& constraints,
                         ValidationState_t& vstate) {
  if (vstate.options()->skip_block_layout) return SPV_SUCCESS;

  // blockRules are the same as bufferBlock rules if the uniform buffer
  // standard layout extension is being used.
  if (vstate.options()->uniform_buffer_standard_layout) blockRules = false;

  // Relaxed layout and scalar layout can both be in effect at the same time.
  // For example, relaxed layout is implied by Vulkan 1.1.  But scalar layout
  // is more permissive than relaxed layout.
  const bool relaxed_block_layout = vstate.IsRelaxedBlockLayout();

  auto fail = [&vstate, struct_id, storage_class, decoration_str, blockRules,
               relaxed_block_layout,
               scalar_block_layout](uint32_t member_idx) -> DiagnosticStream {
    DiagnosticStream ds = std::move(
        vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(struct_id))
        << "Structure id " << struct_id << " decorated as " << decoration_str
        << " for variable in " << StorageClassToString(storage_class)
        << " storage class must follow "
        << (scalar_block_layout
                ? "scalar "
                : (relaxed_block_layout ? "relaxed " : "standard "))
        << (blockRules ? "uniform buffer" : "storage buffer")
        << " layout rules: member " << member_idx << " ");
    return ds;
  };

  // People often use spirv-val from Vulkan Validation Layers, it ends up
  // mapping the various block layout rules from the enabled feature. This
  // offers a hint to help the user understand possbily why things are not
  // working when the shader itself "seems" valid, but just was a lack of adding
  // a supported feature
  auto extra = [&vstate, scalar_block_layout, storage_class,
                relaxed_block_layout, blockRules]() {
    if (!scalar_block_layout) {
      if (storage_class == spv::StorageClass::Workgroup) {
        return vstate.MissingFeature(
            "workgroupMemoryExplicitLayoutScalarBlockLayout feature",
            "--workgroup-scalar-block-layout", true);
      } else if (!relaxed_block_layout) {
        return vstate.MissingFeature("VK_KHR_relaxed_block_layout extension",
                                     "--relax-block-layout", true);
      } else if (blockRules) {
        return vstate.MissingFeature("uniformBufferStandardLayout feature",
                                     "--uniform-buffer-standard-layout", true);
      } else {
        return vstate.MissingFeature("scalarBlockLayout feature",
                                     "--scalar-block-layout", true);
      }
    }
    return std::string("");
  };

  // If we are checking the layout of untyped pointers or physical storage
  // buffer pointers, we may not actually have a struct here. Instead, pretend
  // we have a struct with a single member at offset 0.
  const auto& struct_type = vstate.FindDef(struct_id);
  std::vector<uint32_t> members;
  if (struct_type->opcode() == spv::Op::OpTypeStruct) {
    members = getStructMembers(struct_id, vstate);
  } else {
    members.push_back(struct_id);
  }

  // To check for member overlaps, we want to traverse the members in
  // offset order.
  struct MemberOffsetPair {
    uint32_t member;
    uint32_t offset;
  };
  std::vector<MemberOffsetPair> member_offsets;

  // With untyped pointers or physical storage buffers, we might be checking
  // layouts that do not originate from a structure.
  if (struct_type->opcode() == spv::Op::OpTypeStruct) {
    member_offsets.reserve(members.size());
    for (uint32_t memberIdx = 0, numMembers = uint32_t(members.size());
         memberIdx < numMembers; memberIdx++) {
      uint32_t offset = 0xffffffff;
      auto member_decorations =
          vstate.id_member_decorations(struct_id, memberIdx);
      for (auto decoration = member_decorations.begin;
           decoration != member_decorations.end; ++decoration) {
        assert(decoration->struct_member_index() == (int)memberIdx);
        switch (decoration->dec_type()) {
          case spv::Decoration::Offset:
            offset = decoration->params()[0];
            break;
          default:
            break;
        }
      }
      member_offsets.push_back(
          MemberOffsetPair{memberIdx, incoming_offset + offset});
    }
    std::stable_sort(
        member_offsets.begin(), member_offsets.end(),
        [](const MemberOffsetPair& lhs, const MemberOffsetPair& rhs) {
          return lhs.offset < rhs.offset;
        });
  } else {
    member_offsets.push_back({0, 0});
  }

  // Now scan from lowest offset to highest offset.
  uint32_t nextValidOffset = 0;
  for (size_t ordered_member_idx = 0;
       ordered_member_idx < member_offsets.size(); ordered_member_idx++) {
    const auto& member_offset = member_offsets[ordered_member_idx];
    const auto memberIdx = member_offset.member;
    const auto offset = member_offset.offset;
    auto id = members[member_offset.member];
    const LayoutConstraints& constraint =
        constraints[std::make_pair(struct_id, uint32_t(memberIdx))];
    // Scalar layout takes precedence because it's more permissive, and implying
    // an alignment that divides evenly into the alignment that would otherwise
    // be used.
    const auto alignment =
        scalar_block_layout
            ? getScalarAlignment(id, vstate)
            : getBaseAlignment(id, blockRules, constraint, constraints, vstate);
    const auto inst = vstate.FindDef(id);
    const auto opcode = inst->opcode();
    const auto size = getSize(id, constraint, constraints, vstate);
    // Check offset.
    if (offset == 0xffffffff)
      return fail(memberIdx) << "is missing an Offset decoration" << extra();

    if (opcode == spv::Op::OpTypeRuntimeArray &&
        ordered_member_idx != member_offsets.size() - 1) {
      return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(struct_id))
             << vstate.VkErrorID(4680) << "Structure id " << struct_id
             << " has a runtime array at offset " << offset
             << ", but other members at larger offsets";
    }

    if (!scalar_block_layout && relaxed_block_layout &&
        (opcode == spv::Op::OpTypeVector ||
         opcode == spv::Op::OpTypeVectorIdEXT)) {
      // In relaxed block layout, the vector offset must be aligned to the
      // vector's scalar element type.
      const auto componentId = inst->words()[2];
      const auto scalar_alignment = getScalarAlignment(componentId, vstate);
      if (!IsAlignedTo(offset, scalar_alignment)) {
        return fail(memberIdx) << "at offset " << offset
                               << " is not aligned to scalar element size "
                               << scalar_alignment << extra();
      }
    } else {
      // Without relaxed block layout, the offset must be divisible by the
      // alignment requirement.
      if (!IsAlignedTo(offset, alignment)) {
        return fail(memberIdx) << "at offset " << offset
                               << " is not aligned to " << alignment << extra();
      }
    }
    if (offset < nextValidOffset)
      return fail(memberIdx) << "at offset " << offset
                             << " overlaps previous member ending at offset "
                             << nextValidOffset - 1 << extra();
    if (!scalar_block_layout && relaxed_block_layout) {
      // Check improper straddle of vectors.
      if ((spv::Op::OpTypeVector == opcode ||
           spv::Op::OpTypeVectorIdEXT == opcode) &&
          hasImproperStraddle(id, offset, constraint, constraints, vstate))
        return fail(memberIdx)
               << "is an improperly straddling vector at offset " << offset
               << extra();
    }
    // Check struct members recursively.
    spv_result_t recursive_status = SPV_SUCCESS;
    if (spv::Op::OpTypeStruct == opcode &&
        SPV_SUCCESS != (recursive_status = checkLayout(
                            id, storage_class, decoration_str, blockRules,
                            scalar_block_layout, offset, constraints, vstate)))
      return recursive_status;
    // Check matrix stride.
    if (spv::Op::OpTypeMatrix == opcode) {
      const auto stride = constraint.matrix_stride;
      if (!IsAlignedTo(stride, alignment)) {
        return fail(memberIdx)
               << "is a matrix with stride " << stride
               << " not satisfying alignment to " << alignment << extra();
      }
    }

    // Check arrays and runtime arrays recursively.
    auto array_inst = inst;
    auto array_alignment = alignment;
    while (array_inst->opcode() == spv::Op::OpTypeArray ||
           array_inst->opcode() == spv::Op::OpTypeRuntimeArray) {
      const auto typeId = array_inst->word(2);
      const auto element_inst = vstate.FindDef(typeId);
      // Check array stride.
      uint32_t array_stride = 0;
      for (auto& decoration : vstate.id_decorations(array_inst->id())) {
        if (spv::Decoration::ArrayStride == decoration.dec_type()) {
          array_stride = decoration.params()[0];
          if (array_stride == 0) {
            return fail(memberIdx)
                   << "contains an array with stride 0" << extra();
          }
          if (!IsAlignedTo(array_stride, array_alignment))
            return fail(memberIdx)
                   << "contains an array with stride " << decoration.params()[0]
                   << " not satisfying alignment to " << alignment << extra();
        }
      }

      bool is_int32 = false;
      bool is_const = false;
      uint32_t num_elements = 0;
      if (array_inst->opcode() == spv::Op::OpTypeArray) {
        std::tie(is_int32, is_const, num_elements) =
            vstate.EvalInt32IfConst(array_inst->word(3));
      }
      num_elements = std::max(1u, num_elements);
      // Check each element recursively if it is a struct. There is a
      // limitation to this check if the array size is a spec constant or is a
      // runtime array then we will only check a single element. This means
      // some improper straddles might be missed.
      if (spv::Op::OpTypeStruct == element_inst->opcode()) {
        std::vector<bool> seen(16, false);
        for (uint32_t i = 0; i < num_elements; ++i) {
          uint32_t next_offset = i * array_stride + offset;
          // Stop checking if offsets repeat in terms of 16-byte multiples.
          if (seen[next_offset % 16]) {
            break;
          }

          if (SPV_SUCCESS !=
              (recursive_status = checkLayout(
                   typeId, storage_class, decoration_str, blockRules,
                   scalar_block_layout, next_offset, constraints, vstate)))
            return recursive_status;

          seen[next_offset % 16] = true;
        }
      } else if (spv::Op::OpTypeMatrix == element_inst->opcode()) {
        // Matrix stride would be on the array element in the struct.
        const auto stride = constraint.matrix_stride;
        if (!IsAlignedTo(stride, alignment)) {
          return fail(memberIdx)
                 << "is a matrix with stride " << stride
                 << " not satisfying alignment to " << alignment << extra();
        }
      }

      // Proceed to the element in case it is an array.
      array_inst = element_inst;
      array_alignment = scalar_block_layout
                            ? getScalarAlignment(array_inst->id(), vstate)
                            : getBaseAlignment(array_inst->id(), blockRules,
                                               constraint, constraints, vstate);

      const auto element_size =
          getSize(element_inst->id(), constraint, constraints, vstate);
      if (element_size > array_stride) {
        return fail(memberIdx)
               << "contains an array with stride " << array_stride
               << ", but with an element size of " << element_size << extra();
      }
    }
    nextValidOffset = offset + size;
    if (!scalar_block_layout &&
        (spv::Op::OpTypeArray == opcode || spv::Op::OpTypeStruct == opcode)) {
      // Non-scalar block layout rules don't permit anything in the padding of
      // a struct or array.
      nextValidOffset = align(nextValidOffset, alignment);
    }
  }
  return SPV_SUCCESS;
}

// Returns true if variable or structure id has given decoration. Handles also
// nested structures.
bool hasDecoration(uint32_t id, spv::Decoration decoration,
                   ValidationState_t& vstate) {
  for (auto& dec : vstate.id_decorations(id)) {
    if (decoration == dec.dec_type()) return true;
  }
  if (spv::Op::OpTypeStruct != vstate.FindDef(id)->opcode()) {
    return false;
  }
  for (auto member_id : getStructMembers(id, spv::Op::OpTypeStruct, vstate)) {
    if (hasDecoration(member_id, decoration, vstate)) {
      return true;
    }
  }
  return false;
}

// Returns true if all ids of given type have a specified decoration.
bool checkForRequiredDecoration(uint32_t struct_id,
                                std::function<bool(spv::Decoration)> checker,
                                spv::Op type, ValidationState_t& vstate) {
  const auto& members = getStructMembers(struct_id, vstate);
  for (size_t memberIdx = 0; memberIdx < members.size(); memberIdx++) {
    auto id = members[memberIdx];
    if (type == spv::Op::OpTypeMatrix) {
      // Matrix decorations also apply to arrays of matrices.
      auto memberInst = vstate.FindDef(id);
      while (memberInst->opcode() == spv::Op::OpTypeArray ||
             memberInst->opcode() == spv::Op::OpTypeRuntimeArray) {
        memberInst = vstate.FindDef(memberInst->GetOperandAs<uint32_t>(1u));
      }
      id = memberInst->id();
    }
    if (type != vstate.FindDef(id)->opcode()) continue;
    bool found = false;
    for (auto& dec : vstate.id_decorations(id)) {
      if (checker(dec.dec_type())) found = true;
    }
    for (auto& dec : vstate.id_decorations(struct_id)) {
      if (checker(dec.dec_type()) &&
          (int)memberIdx == dec.struct_member_index()) {
        found = true;
      }
    }
    if (!found) {
      return false;
    }
  }
  for (auto id : getStructMembers(struct_id, spv::Op::OpTypeStruct, vstate)) {
    if (!checkForRequiredDecoration(id, checker, type, vstate)) {
      return false;
    }
  }
  return true;
}

spv_result_t CheckLinkageAttrOfFunctions(ValidationState_t& vstate) {
  for (const auto& function : vstate.functions()) {
    if (function.block_count() == 0u) {
      // A function declaration (an OpFunction with no basic blocks), must have
      // a Linkage Attributes Decoration with the Import Linkage Type.
      if (!hasImportLinkageAttribute(function.id(), vstate)) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(function.id()))
               << "Function declaration (id " << function.id()
               << ") must have a LinkageAttributes decoration with the Import "
                  "Linkage type.";
      }
    } else {
      if (hasImportLinkageAttribute(function.id(), vstate)) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(function.id()))
               << "Function definition (id " << function.id()
               << ") may not be decorated with Import Linkage type.";
      }
    }
  }
  return SPV_SUCCESS;
}

// Checks whether an imported variable is initialized by this module.
spv_result_t CheckImportedVariableInitialization(ValidationState_t& vstate) {
  // According the SPIR-V Spec 2.16.1, it is illegal to initialize an imported
  // variable. This means that a module-scope OpVariable with initialization
  // value cannot be marked with the Import Linkage Type (import type id = 1).
  for (auto global_var_id : vstate.global_vars()) {
    // Initializer <id> is an optional argument for OpVariable. If initializer
    // <id> is present, the instruction will have 5 words.
    auto variable_instr = vstate.FindDef(global_var_id);
    if (variable_instr->words().size() == 5u &&
        hasImportLinkageAttribute(global_var_id, vstate)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, variable_instr)
             << "A module-scope OpVariable with initialization value "
                "cannot be marked with the Import Linkage Type.";
    }
  }
  return SPV_SUCCESS;
}

// Checks whether a builtin variable is valid.
spv_result_t CheckBuiltInVariable(uint32_t var_id, ValidationState_t& vstate) {
  const auto& decorations = vstate.id_decorations(var_id);
  for (const auto& d : decorations) {
    if (spvIsVulkanEnv(vstate.context()->target_env)) {
      if (d.dec_type() == spv::Decoration::Location ||
          d.dec_type() == spv::Decoration::Component) {
        return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
               << vstate.VkErrorID(4915) << "A BuiltIn variable (id " << var_id
               << ") cannot have any Location or Component decorations";
      }
    }
  }
  return SPV_SUCCESS;
}

// Checks whether proper decorations have been applied to the entry points.
spv_result_t CheckDecorationsOfEntryPoints(ValidationState_t& vstate) {
  for (uint32_t entry_point : vstate.entry_points()) {
    const auto& descs = vstate.entry_point_descriptions(entry_point);
    int num_builtin_block_inputs = 0;
    int num_builtin_block_outputs = 0;
    int num_workgroup_variables = 0;
    int num_workgroup_variables_with_block = 0;
    int num_workgroup_variables_with_aliased = 0;
    bool has_task_payload = false;
    for (const auto& desc : descs) {
      std::unordered_set<Instruction*> seen_vars;
      std::unordered_set<spv::BuiltIn> input_var_builtin;
      std::unordered_set<spv::BuiltIn> output_var_builtin;
      for (auto interface : desc.interfaces) {
        Instruction* var_instr = vstate.FindDef(interface);
        if (!var_instr ||
            (spv::Op::OpVariable != var_instr->opcode() &&
             spv::Op::OpUntypedVariableKHR != var_instr->opcode())) {
          return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                 << "Interfaces passed to OpEntryPoint must be variables. "
                    "Found Op"
                 << spvOpcodeString(var_instr->opcode()) << ".";
        }
        const bool untyped_pointers =
            var_instr->opcode() == spv::Op::OpUntypedVariableKHR;
        const auto sc_index = 2u;
        const spv::StorageClass storage_class =
            var_instr->GetOperandAs<spv::StorageClass>(sc_index);
        if (vstate.version() >= SPV_SPIRV_VERSION_WORD(1, 4)) {
          // SPV_EXT_mesh_shader, at most one task payload is permitted
          // per entry point
          if (storage_class == spv::StorageClass::TaskPayloadWorkgroupEXT) {
            if (has_task_payload) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << "There can be at most one "
                        "OpVariable with storage "
                        "class TaskPayloadWorkgroupEXT associated with "
                        "an OpEntryPoint";
            }
            has_task_payload = true;
          }

          // Starting in 1.4, OpEntryPoint must list all global variables
          // it statically uses and those interfaces must be unique.
          if (storage_class == spv::StorageClass::Function) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                   << "In SPIR-V 1.4 or later, OpEntryPoint interfaces should "
                      "only list global "
                      "variables";
          }

          if (!seen_vars.insert(var_instr).second) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                   << "In SPIR-V 1.4 or later, non-unique OpEntryPoint "
                      "interface "
                   << vstate.getIdName(interface) << " is disallowed";
          }
        } else {
          if (storage_class != spv::StorageClass::Input &&
              storage_class != spv::StorageClass::Output) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                   << "In SPIR-V 1.3 or earlier, OpEntryPoint interfaces must "
                      "be OpVariables with "
                      "Storage Class of Input(1) or Output(3). Found Storage "
                      "Class "
                   << uint32_t(storage_class) << " for Entry Point id "
                   << entry_point << ".";
          }
        }

        // Descriptor heap's base variables have no data type in declaration.
        if (untyped_pointers && var_instr->words().size() < 5 &&
            vstate.IsDescriptorHeapBaseVariable(var_instr))
          continue;

        // It is guaranteed (by validator ID checks) that ptr_instr is
        // OpTypePointer. Word 3 of this instruction is the type being pointed
        // to. For untyped variables, the pointee type comes from the data type
        // operand.
        const uint32_t type_id =
            untyped_pointers ? var_instr->word(4)
                             : vstate.FindDef(var_instr->word(1))->word(3);
        Instruction* type_instr = vstate.FindDef(type_id);
        const bool is_struct =
            type_instr && spv::Op::OpTypeStruct == type_instr->opcode();

        // Search all Built-in (on the variable or the struct)
        bool has_built_in = false;
        for (auto& dec :
             vstate.id_decorations(is_struct ? type_id : interface)) {
          if (dec.dec_type() != spv::Decoration::BuiltIn) continue;
          has_built_in = true;

          if (!spvIsVulkanEnv(vstate.context()->target_env)) continue;

          const spv::BuiltIn builtin = dec.builtin();
          if (storage_class == spv::StorageClass::Input) {
            if (!input_var_builtin.insert(builtin).second) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << vstate.VkErrorID(9658)
                     << "OpEntryPoint contains duplicate input variables "
                        "with "
                     << vstate.grammar().lookupOperandName(
                            SPV_OPERAND_TYPE_BUILT_IN, (uint32_t)builtin)
                     << " builtin";
            }
          }
          if (storage_class == spv::StorageClass::Output) {
            if (!output_var_builtin.insert(builtin).second) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << vstate.VkErrorID(9659)
                     << "OpEntryPoint contains duplicate output variables "
                        "with "
                     << vstate.grammar().lookupOperandName(
                            SPV_OPERAND_TYPE_BUILT_IN, (uint32_t)builtin)
                     << " builtin";
            }
          }
        }

        if (has_built_in) {
          if (auto error = CheckBuiltInVariable(interface, vstate))
            return error;

          if (is_struct) {
            if (!isBlock(type_id, vstate)) {
              return vstate.diag(SPV_ERROR_INVALID_DATA,
                                 vstate.FindDef(type_id))
                     << vstate.VkErrorID(4919)
                     << "Interface struct has no Block decoration but has "
                        "BuiltIn members. "
                        "Location decorations must be used on each member of "
                        "OpVariable with a structure type that is a block not "
                        "decorated with Location.";
            }
            if (storage_class == spv::StorageClass::Input)
              ++num_builtin_block_inputs;
            if (storage_class == spv::StorageClass::Output)
              ++num_builtin_block_outputs;
            if (num_builtin_block_inputs > 1 || num_builtin_block_outputs > 1)
              break;
          }
        }

        if (storage_class == spv::StorageClass::Workgroup) {
          ++num_workgroup_variables;
          if (type_instr) {
            if (spv::Op::OpTypeStruct == type_instr->opcode()) {
              if (hasDecoration(type_id, spv::Decoration::Block, vstate)) {
                ++num_workgroup_variables_with_block;
              } else if (untyped_pointers &&
                         vstate.HasCapability(spv::Capability::Shader)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                       << "Untyped workgroup variables in shaders must be "
                          "block decorated";
              }
              if (hasDecoration(var_instr->id(), spv::Decoration::Aliased,
                                vstate))
                ++num_workgroup_variables_with_aliased;
            } else if (untyped_pointers &&
                       vstate.HasCapability(spv::Capability::Shader)) {
              return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << "Untyped workgroup variables in shaders must be block "
                        "decorated structs";
            }
          }
        }

        if (spvIsVulkanEnv(vstate.context()->target_env)) {
          const auto* models = vstate.GetExecutionModels(entry_point);
          const bool has_frag =
              models->find(spv::ExecutionModel::Fragment) != models->end();
          const bool has_vert =
              models->find(spv::ExecutionModel::Vertex) != models->end();
          for (const auto& decoration :
               vstate.id_decorations(var_instr->id())) {
            if (decoration == spv::Decoration::Flat ||
                decoration == spv::Decoration::NoPerspective ||
                decoration == spv::Decoration::Sample ||
                decoration == spv::Decoration::Centroid) {
              // VUID 04670 already validates these decorations are input/output
              if (storage_class == spv::StorageClass::Input &&
                  (models->size() > 1 || has_vert)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                       << vstate.VkErrorID(6202)
                       << vstate.SpvDecorationString(decoration.dec_type())
                       << " decorated variable must not be used in vertex "
                          "execution model as an Input storage class for Entry "
                          "Point id "
                       << entry_point << ".";
              } else if (storage_class == spv::StorageClass::Output &&
                         (models->size() > 1 || has_frag)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                       << vstate.VkErrorID(6201)
                       << vstate.SpvDecorationString(decoration.dec_type())
                       << " decorated variable must not be used in fragment "
                          "execution model as an Output storage class for "
                          "Entry Point id "
                       << entry_point << ".";
              }
            }
          }

          const bool has_flat =
              hasDecoration(var_instr->id(), spv::Decoration::Flat, vstate);
          if (has_frag && storage_class == spv::StorageClass::Input &&
              !has_flat &&
              (vstate.IsFloatScalarType(type_id, 64) ||
               vstate.IsIntScalarOrVectorType(type_id))) {
            return vstate.diag(SPV_ERROR_INVALID_ID, var_instr)
                     << vstate.VkErrorID(4744)
                     << "Fragment OpEntryPoint operand "
                     << interface << " with Input interfaces with integer or "
                                     "float type must have a Flat decoration "
                                     "for Entry Point id "
                     << entry_point << ".";
          }
        }
      }
      if (num_builtin_block_inputs > 1 || num_builtin_block_outputs > 1) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(entry_point))
               << "There must be at most one object per Storage Class that can "
                  "contain a structure type containing members decorated with "
                  "BuiltIn, consumed per entry-point. Entry Point id "
               << entry_point << " does not meet this requirement.";
      }
      // The LinkageAttributes Decoration cannot be applied to functions
      // targeted by an OpEntryPoint instruction
      for (auto& decoration : vstate.id_decorations(entry_point)) {
        if (spv::Decoration::LinkageAttributes == decoration.dec_type()) {
          const std::string linkage_name =
              spvtools::utils::MakeString(decoration.params());
          return vstate.diag(SPV_ERROR_INVALID_BINARY,
                             vstate.FindDef(entry_point))
                 << "The LinkageAttributes Decoration (Linkage name: "
                 << linkage_name << ") cannot be applied to function id "
                 << entry_point
                 << " because it is targeted by an OpEntryPoint instruction.";
        }
      }

      const bool workgroup_blocks_allowed = vstate.HasCapability(
          spv::Capability::WorkgroupMemoryExplicitLayoutKHR);
      if (workgroup_blocks_allowed &&
          !vstate.HasCapability(spv::Capability::UntypedPointersKHR) &&
          num_workgroup_variables > 0 &&
          num_workgroup_variables_with_block > 0) {
        if (num_workgroup_variables != num_workgroup_variables_with_block) {
          return vstate.diag(SPV_ERROR_INVALID_BINARY,
                             vstate.FindDef(entry_point))
                 << "When declaring WorkgroupMemoryExplicitLayoutKHR, "
                    "either all or none of the Workgroup Storage Class "
                    "variables "
                    "in the entry point interface must point to struct types "
                    "decorated with Block (unless the "
                    "UntypedPointersKHR capability is declared).  "
                    "Entry point id "
                 << entry_point << " does not meet this requirement.";
        }
        if (num_workgroup_variables_with_block > 1 &&
            num_workgroup_variables_with_block !=
                num_workgroup_variables_with_aliased) {
          return vstate.diag(SPV_ERROR_INVALID_BINARY,
                             vstate.FindDef(entry_point))
                 << "When declaring WorkgroupMemoryExplicitLayoutKHR, "
                    "if more than one Workgroup Storage Class variable in "
                    "the entry point interface point to a type decorated "
                    "with Block, all of them must be decorated with Aliased "
                    "(unless the UntypedPointerWorkgroupKHR capability is "
                    "declared). Entry point id "
                 << entry_point << " does not meet this requirement.";
        }
      } else if (!workgroup_blocks_allowed &&
                 num_workgroup_variables_with_block > 0) {
        return vstate.diag(SPV_ERROR_INVALID_BINARY,
                           vstate.FindDef(entry_point))
               << "Workgroup Storage Class variables can't be decorated with "
                  "Block unless declaring the WorkgroupMemoryExplicitLayoutKHR "
                  "capability.";
      }
    }
  }
  return SPV_SUCCESS;
}

// Load |constraints| with all the member constraints for structs contained
// within the given array type.
void ComputeMemberConstraintsForArray(MemberConstraints* constraints,
                                      uint32_t array_id,
                                      const LayoutConstraints& inherited,
                                      ValidationState_t& vstate);

// Load |constraints| with all the member constraints for the given struct,
// and all its contained structs.
void ComputeMemberConstraintsForStruct(MemberConstraints* constraints,
                                       uint32_t struct_id,
                                       const LayoutConstraints& inherited,
                                       ValidationState_t& vstate) {
  assert(constraints);
  const auto& members = getStructMembers(struct_id, vstate);
  for (uint32_t memberIdx = 0, numMembers = uint32_t(members.size());
       memberIdx < numMembers; memberIdx++) {
    LayoutConstraints& constraint =
        (*constraints)[std::make_pair(struct_id, memberIdx)];
    constraint = inherited;
    auto member_decorations =
        vstate.id_member_decorations(struct_id, memberIdx);
    for (auto decoration = member_decorations.begin;
         decoration != member_decorations.end; ++decoration) {
      assert(decoration->struct_member_index() == (int)memberIdx);
      switch (decoration->dec_type()) {
        case spv::Decoration::RowMajor:
          constraint.majorness = kRowMajor;
          break;
        case spv::Decoration::ColMajor:
          constraint.majorness = kColumnMajor;
          break;
        case spv::Decoration::MatrixStride:
          constraint.matrix_stride = decoration->params()[0];
          break;
        default:
          break;
      }
    }

    // Now recurse
    auto member_type_id = members[memberIdx];
    const auto member_type_inst = vstate.FindDef(member_type_id);
    const auto opcode = member_type_inst->opcode();
    switch (opcode) {
      case spv::Op::OpTypeArray:
      case spv::Op::OpTypeRuntimeArray:
        ComputeMemberConstraintsForArray(constraints, member_type_id, inherited,
                                         vstate);
        break;
      case spv::Op::OpTypeStruct:
        ComputeMemberConstraintsForStruct(constraints, member_type_id,
                                          inherited, vstate);
        break;
      default:
        break;
    }
  }
}

void ComputeMemberConstraintsForArray(MemberConstraints* constraints,
                                      uint32_t array_id,
                                      const LayoutConstraints& inherited,
                                      ValidationState_t& vstate) {
  assert(constraints);
  auto elem_type_id = vstate.FindDef(array_id)->words()[2];
  const auto elem_type_inst = vstate.FindDef(elem_type_id);
  const auto opcode = elem_type_inst->opcode();
  switch (opcode) {
    case spv::Op::OpTypeArray:
    case spv::Op::OpTypeRuntimeArray:
      ComputeMemberConstraintsForArray(constraints, elem_type_id, inherited,
                                       vstate);
      break;
    case spv::Op::OpTypeStruct:
      ComputeMemberConstraintsForStruct(constraints, elem_type_id, inherited,
                                        vstate);
      break;
    default:
      break;
  }
}

spv_result_t CheckDecorationsOfVariables(ValidationState_t& vstate) {
  if (!spvIsVulkanEnv(vstate.context()->target_env)) {
    return SPV_SUCCESS;
  }
  for (const auto& inst : vstate.ordered_instructions()) {
    if ((spv::Op::OpVariable == inst.opcode()) ||
        (spv::Op::OpUntypedVariableKHR == inst.opcode())) {
      const auto var_id = inst.id();
      const auto storageClass = inst.GetOperandAs<spv::StorageClass>(2);
      const bool uniform = storageClass == spv::StorageClass::Uniform;
      const bool uniform_constant =
          storageClass == spv::StorageClass::UniformConstant;
      const bool storage_buffer =
          storageClass == spv::StorageClass::StorageBuffer;

      const char* sc_str = uniform            ? "Uniform"
                           : uniform_constant ? "UniformConstant"
                                              : "StorageBuffer";
      // Check variables in the UniformConstant, StorageBuffer, and Uniform
      // storage classes are decorated with DescriptorSet and Binding
      // (VUID-06677).
      if (uniform_constant || storage_buffer || uniform) {
        if (vstate.IsDescriptorHeapBaseVariable(&inst)) {
          continue;
        }
        // Skip validation if the variable is not used and we're looking
        // at a module coming from HLSL that has not been legalized yet.
        if (vstate.options()->before_hlsl_legalization &&
            vstate.EntryPointReferences(var_id).empty()) {
          continue;
        }
        if (!hasDecoration(var_id, spv::Decoration::DescriptorSet, vstate)) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                 << vstate.VkErrorID(6677) << sc_str << " id '" << var_id
                 << "' is missing DescriptorSet decoration.\n"
                 << "From Vulkan spec:\n"
                 << "These variables must have DescriptorSet and Binding "
                    "decorations specified";
        }
        if (!hasDecoration(var_id, spv::Decoration::Binding, vstate)) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                 << vstate.VkErrorID(6677) << sc_str << " id '" << var_id
                 << "' is missing Binding decoration.\n"
                 << "From Vulkan spec:\n"
                 << "These variables must have DescriptorSet and Binding "
                    "decorations specified";
        }
      }
    }
  }
  return SPV_SUCCESS;
}

spv_result_t CheckDecorationsOfBuffers(ValidationState_t& vstate) {
  // Set of entry points that are known to use a push constant.
  std::unordered_set<uint32_t> uses_push_constant;
  for (const auto& inst : vstate.ordered_instructions()) {
    const auto& words = inst.words();
    auto type_id = inst.type_id();
    const Instruction* type_inst = vstate.FindDef(type_id);
    bool scalar_block_layout = false;
    MemberConstraints constraints;
    if (spv::Op::OpVariable == inst.opcode() ||
        spv::Op::OpUntypedVariableKHR == inst.opcode()) {
      const bool untyped_pointer =
          inst.opcode() == spv::Op::OpUntypedVariableKHR;
      const auto var_id = inst.id();
      // For storage class / decoration combinations, see Vulkan 14.5.4 "Offset
      // and Stride Assignment".
      const auto storageClassVal = words[3];
      const auto storageClass = spv::StorageClass(storageClassVal);
      const bool uniform = storageClass == spv::StorageClass::Uniform;
      const bool push_constant =
          storageClass == spv::StorageClass::PushConstant;
      const bool storage_buffer =
          storageClass == spv::StorageClass::StorageBuffer;

      if (spvIsVulkanEnv(vstate.context()->target_env)) {
        // Vulkan: There must be no more than one PushConstant block per entry
        // point.
        if (push_constant &&
            !(vstate.HasCapability(spv::Capability::PushConstantBanksNV))) {
          auto entry_points = vstate.EntryPointReferences(var_id);
          for (auto ep_id : entry_points) {
            const bool already_used = !uses_push_constant.insert(ep_id).second;
            if (already_used) {
              return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                     << vstate.VkErrorID(6674) << "Entry point id '" << ep_id
                     << "' uses more than one PushConstant interface.\n"
                     << "From Vulkan spec:\n"
                     << "There must be no more than one push constant block "
                     << "statically used per shader entry point.";
            }
          }
        }
      }

      if (spvIsOpenGLEnv(vstate.context()->target_env)) {
        bool has_block = hasDecoration(var_id, spv::Decoration::Block, vstate);
        bool has_buffer_block =
            hasDecoration(var_id, spv::Decoration::BufferBlock, vstate);
        if ((uniform && (has_block || has_buffer_block)) ||
            (storage_buffer && has_block)) {
          auto entry_points = vstate.EntryPointReferences(var_id);
          if (!entry_points.empty() &&
              !hasDecoration(var_id, spv::Decoration::Binding, vstate)) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                   << StorageClassToString(storageClass) << " id '" << var_id
                   << "' is missing Binding decoration.\n"
                   << "From ARB_gl_spirv extension:\n"
                   << "Uniform and shader storage block variables must "
                   << "also be decorated with a *Binding*.";
          }
        }
      }

      const bool phys_storage_buffer =
          storageClass == spv::StorageClass::PhysicalStorageBuffer;
      const bool workgroup =
          storageClass == spv::StorageClass::Workgroup &&
          vstate.HasCapability(
              spv::Capability::WorkgroupMemoryExplicitLayoutKHR);

      if (spvIsVulkanEnv(vstate.context()->target_env) &&
          inst.opcode() == spv::Op::OpUntypedVariableKHR &&
          storageClass != spv::StorageClass::UniformConstant &&
          vstate.IsDescriptorHeapBaseVariable(&inst)) {
        if (vstate.IsBuiltin(inst.id(), spv::BuiltIn::ResourceHeapEXT)) {
          return vstate.diag(SPV_ERROR_INVALID_DATA, &inst)
                 << vstate.VkErrorID(11241)
                 << "The variable decorated with ResourceHeapEXT must be "
                 << "declared using the UniformConstant storage class.";
        }
        if (vstate.IsBuiltin(inst.id(), spv::BuiltIn::SamplerHeapEXT)) {
          return vstate.diag(SPV_ERROR_INVALID_DATA, &inst)
                 << vstate.VkErrorID(11239)
                 << "The variable decorated with SamplerHeapEXT must be "
                 << "declared using the UniformConstant storage class.";
        }
      }
      if (uniform || push_constant || storage_buffer || phys_storage_buffer ||
          workgroup) {
        const auto ptrInst = vstate.FindDef(words[1]);
        assert(spv::Op::OpTypePointer == ptrInst->opcode() ||
               spv::Op::OpTypeUntypedPointerKHR == ptrInst->opcode());
        auto id = untyped_pointer ? (words.size() > 4 ? words[4] : 0)
                                  : ptrInst->words()[3];
        if (id != 0) {
          auto id_inst = vstate.FindDef(id);
          // Jump through one level of arraying.
          if (!workgroup &&
              (id_inst->opcode() == spv::Op::OpTypeArray ||
               id_inst->opcode() == spv::Op::OpTypeRuntimeArray)) {
            id = id_inst->GetOperandAs<uint32_t>(1u);
            id_inst = vstate.FindDef(id);
          }
          // Struct requirement is checked on variables so just move on here.
          if (spv::Op::OpTypeStruct != id_inst->opcode()) continue;
          ComputeMemberConstraintsForStruct(&constraints, id,
                                            LayoutConstraints(), vstate);
        }

        if (spvIsVulkanEnv(vstate.context()->target_env)) {
          const bool block = hasDecoration(id, spv::Decoration::Block, vstate);
          const bool buffer_block =
              hasDecoration(id, spv::Decoration::BufferBlock, vstate);
          if (storage_buffer && buffer_block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(var_id))
                   << vstate.VkErrorID(6675) << "Storage buffer id '" << var_id
                   << " In Vulkan, BufferBlock is disallowed on variables in "
                      "the StorageBuffer storage class";
          }
          // Vulkan: Check Block decoration for PushConstant, Uniform
          // and StorageBuffer variables. Uniform can also use BufferBlock.
          if (push_constant && !block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                   << vstate.VkErrorID(6675) << "PushConstant id '" << id
                   << "' is missing Block decoration.\n"
                   << "From Vulkan spec:\n"
                   << "Such variables must be identified with a Block "
                      "decoration";
          }
          if (storage_buffer && !block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                   << vstate.VkErrorID(6675) << "StorageBuffer id '" << id
                   << "' is missing Block decoration.\n"
                   << "From Vulkan spec:\n"
                   << "Such variables must be identified with a Block "
                      "decoration";
          }
          if (uniform && !block && !buffer_block) {
            return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                   << vstate.VkErrorID(6676) << "Uniform id '" << id
                   << "' is missing Block or BufferBlock decoration.\n"
                   << "From Vulkan spec:\n"
                   << "Such variables must be identified with a Block or "
                      "BufferBlock decoration";
          }
        }

        if (id != 0) {
          for (const auto& dec : vstate.id_decorations(id)) {
            const bool blockDeco = spv::Decoration::Block == dec.dec_type();
            const bool bufferDeco =
                spv::Decoration::BufferBlock == dec.dec_type();
            const bool blockRules = uniform && blockDeco;
            const bool bufferRules = (uniform && bufferDeco) ||
                                     ((push_constant || storage_buffer ||
                                       phys_storage_buffer || workgroup) &&
                                      blockDeco);
            if (uniform && blockDeco) {
              vstate.RegisterPointerToUniformBlock(ptrInst->id());
              vstate.RegisterStructForUniformBlock(id);
            }
            if ((uniform && bufferDeco) ||
                ((storage_buffer || phys_storage_buffer) && blockDeco)) {
              vstate.RegisterPointerToStorageBuffer(ptrInst->id());
              vstate.RegisterStructForStorageBuffer(id);
            }

            if (blockRules || bufferRules) {
              const char* deco_str = blockDeco ? "Block" : "BufferBlock";
              spv_result_t recursive_status = SPV_SUCCESS;
              scalar_block_layout =
                  workgroup ? vstate.options()->workgroup_scalar_block_layout
                            : vstate.options()->scalar_block_layout;

              if (isMissingOffsetInStruct(id, vstate)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                       << "Structure id " << id << " decorated as " << deco_str
                       << " must be explicitly laid out with Offset "
                          "decorations.";
              }

              if (!checkForRequiredDecoration(
                      id,
                      [](spv::Decoration d) {
                        return d == spv::Decoration::ArrayStride ||
                               d == spv::Decoration::ArrayStrideIdEXT;
                      },
                      spv::Op::OpTypeArray, vstate)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                       << "Structure id " << id << " decorated as " << deco_str
                       << " must be explicitly laid out with ArrayStride or "
                          "ArrayStrideIdEXT "
                          "decorations.";
              }

              if (!checkForRequiredDecoration(
                      id,
                      [](spv::Decoration d) {
                        return d == spv::Decoration::MatrixStride;
                      },
                      spv::Op::OpTypeMatrix, vstate)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                       << "Structure id " << id << " decorated as " << deco_str
                       << " must be explicitly laid out with MatrixStride "
                          "decorations.";
              }

              if (!checkForRequiredDecoration(
                      id,
                      [](spv::Decoration d) {
                        return d == spv::Decoration::RowMajor ||
                               d == spv::Decoration::ColMajor;
                      },
                      spv::Op::OpTypeMatrix, vstate)) {
                return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                       << "Structure id " << id << " decorated as " << deco_str
                       << " must be explicitly laid out with RowMajor or "
                          "ColMajor decorations.";
              }

              if (spvIsVulkanEnv(vstate.context()->target_env)) {
                if (blockRules &&
                    (SPV_SUCCESS !=
                     (recursive_status = checkLayout(
                          id, storageClass, deco_str, true, scalar_block_layout,
                          0, constraints, vstate)))) {
                  return recursive_status;
                } else if (bufferRules &&
                           (SPV_SUCCESS != (recursive_status = checkLayout(
                                                id, storageClass, deco_str,
                                                false, scalar_block_layout, 0,
                                                constraints, vstate)))) {
                  return recursive_status;
                }
              }
            }
          }
        }
      }
    } else if (type_inst && type_inst->opcode() == spv::Op::OpTypePointer &&
               type_inst->GetOperandAs<spv::StorageClass>(1u) ==
                   spv::StorageClass::PhysicalStorageBuffer) {
      const bool buffer = true;
      const auto pointee_type_id = type_inst->GetOperandAs<uint32_t>(2u);
      const auto* data_type_inst = vstate.FindDef(pointee_type_id);
      scalar_block_layout = vstate.options()->scalar_block_layout;
      if (data_type_inst->opcode() == spv::Op::OpTypeStruct) {
        ComputeMemberConstraintsForStruct(&constraints, pointee_type_id,
                                          LayoutConstraints(), vstate);
      }
      if (auto res = checkLayout(
              pointee_type_id, spv::StorageClass::PhysicalStorageBuffer,
              "Block", !buffer, scalar_block_layout, 0, constraints, vstate)) {
        return res;
      }
    } else if (vstate.HasCapability(spv::Capability::UntypedPointersKHR) &&
               spvIsVulkanEnv(vstate.context()->target_env)) {
      // Untyped variables are checked above. Here we check that instructions
      // using an untyped pointer have a valid layout.
      uint32_t ptr_ty_id = 0;
      uint32_t data_type_id = 0;
      switch (inst.opcode()) {
        case spv::Op::OpUntypedAccessChainKHR:
        case spv::Op::OpUntypedInBoundsAccessChainKHR:
        case spv::Op::OpUntypedPtrAccessChainKHR:
        case spv::Op::OpUntypedInBoundsPtrAccessChainKHR:
          ptr_ty_id = inst.type_id();
          data_type_id = inst.GetOperandAs<uint32_t>(2);
          break;
        case spv::Op::OpLoad:
          if (vstate.GetIdOpcode(vstate.GetOperandTypeId(&inst, 2)) ==
              spv::Op::OpTypeUntypedPointerKHR) {
            const auto ptr_id = inst.GetOperandAs<uint32_t>(2);
            ptr_ty_id = vstate.FindDef(ptr_id)->type_id();
            data_type_id = inst.type_id();
          }
          break;
        case spv::Op::OpStore:
          if (vstate.GetIdOpcode(vstate.GetOperandTypeId(&inst, 0)) ==
              spv::Op::OpTypeUntypedPointerKHR) {
            const auto ptr_id = inst.GetOperandAs<uint32_t>(0);
            ptr_ty_id = vstate.FindDef(ptr_id)->type_id();
            data_type_id = vstate.GetOperandTypeId(&inst, 1);
          }
          break;
        case spv::Op::OpUntypedArrayLengthKHR:
          ptr_ty_id = vstate.FindDef(inst.GetOperandAs<uint32_t>(3))->type_id();
          data_type_id = inst.GetOperandAs<uint32_t>(2);
          break;
        default:
          break;
      }

      if (ptr_ty_id == 0 || data_type_id == 0) {
        // Not an untyped pointer.
        continue;
      }

      const auto sc =
          vstate.FindDef(ptr_ty_id)->GetOperandAs<spv::StorageClass>(1);

      auto data_type = vstate.FindDef(data_type_id);
      scalar_block_layout =
          sc == spv::StorageClass::Workgroup
              ? vstate.options()->workgroup_scalar_block_layout
              : vstate.options()->scalar_block_layout;

      // If the data type is an array that contains a Block- or
      // BufferBlock-decorated struct, then use the struct for layout checks
      // instead of the array. In this case, the array represents a descriptor
      // array which should not have an explicit layout.
      if (data_type->opcode() == spv::Op::OpTypeArray ||
          data_type->opcode() == spv::Op::OpTypeRuntimeArray) {
        const auto ele_type =
            vstate.FindDef(data_type->GetOperandAs<uint32_t>(1u));
        if (ele_type->opcode() == spv::Op::OpTypeStruct &&
            (vstate.HasDecoration(ele_type->id(), spv::Decoration::Block) ||
             vstate.HasDecoration(ele_type->id(),
                                  spv::Decoration::BufferBlock))) {
          data_type = ele_type;
          data_type_id = ele_type->id();
        }
      }

      // Assume uniform storage class uses block rules unless we see a
      // BufferBlock decorated struct in the data type.
      bool bufferRules = sc == spv::StorageClass::Uniform ? false : true;
      if (data_type->opcode() == spv::Op::OpTypeStruct) {
        if (sc == spv::StorageClass::Uniform) {
          bufferRules =
              vstate.HasDecoration(data_type_id, spv::Decoration::BufferBlock);
        }
        ComputeMemberConstraintsForStruct(&constraints, data_type_id,
                                          LayoutConstraints(), vstate);
      }
      const char* deco_str =
          bufferRules
              ? (sc == spv::StorageClass::Uniform ? "BufferBlock" : "Block")
              : "Block";

      if (!vstate.IsDescriptorHeapBaseVariable(&inst)) {
        if (auto result =
                checkLayout(data_type_id, sc, deco_str, !bufferRules,
                            scalar_block_layout, 0, constraints, vstate)) {
          return result;
        }
      }
    }
  }
  return SPV_SUCCESS;
}

// Returns true if |decoration| cannot be applied to the same id more than once.
bool AtMostOncePerId(spv::Decoration decoration) {
  return decoration != spv::Decoration::UserSemantic &&
         decoration != spv::Decoration::FuncParamAttr;
}

// Returns true if |decoration| cannot be applied to the same member more than
// once.
bool AtMostOncePerMember(spv::Decoration decoration) {
  return decoration != spv::Decoration::UserSemantic;
}

spv_result_t CheckDecorationsCompatibility(ValidationState_t& vstate) {
  using PerIDKey = std::tuple<spv::Decoration, uint32_t>;
  using PerMemberKey = std::tuple<spv::Decoration, uint32_t, uint32_t>;

  // An Array of pairs where the decorations in the pair cannot both be applied
  // to the same id.
  static const spv::Decoration mutually_exclusive_per_id[][2] = {
      {spv::Decoration::Block, spv::Decoration::BufferBlock},
      {spv::Decoration::Restrict, spv::Decoration::Aliased},
      {spv::Decoration::RestrictPointer, spv::Decoration::AliasedPointer}};
  static const auto num_mutually_exclusive_per_id_pairs =
      sizeof(mutually_exclusive_per_id) / (2 * sizeof(spv::Decoration));

  // An Array of pairs where the decorations in the pair cannot both be applied
  // to the same member.
  static const spv::Decoration mutually_exclusive_per_member[][2] = {
      {spv::Decoration::RowMajor, spv::Decoration::ColMajor},
      {spv::Decoration::Offset, spv::Decoration::OffsetIdEXT}};
  static const auto num_mutually_exclusive_per_mem_pairs =
      sizeof(mutually_exclusive_per_member) / (2 * sizeof(spv::Decoration));

  std::set<PerIDKey> seen_per_id;
  std::set<PerMemberKey> seen_per_member;

  for (const auto& inst : vstate.ordered_instructions()) {
    const auto& words = inst.words();
    if (spv::Op::OpDecorate == inst.opcode()) {
      const auto id = words[1];
      const auto dec_type = static_cast<spv::Decoration>(words[2]);
      const auto k = PerIDKey(dec_type, id);
      const auto already_used = !seen_per_id.insert(k).second;
      if (already_used && AtMostOncePerId(dec_type)) {
        return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
               << "ID '" << id << "' decorated with "
               << vstate.SpvDecorationString(dec_type)
               << " multiple times is not allowed.";
      }
      // Verify certain mutually exclusive decorations are not both applied on
      // an ID.
      for (uint32_t pair_idx = 0;
           pair_idx < num_mutually_exclusive_per_id_pairs; ++pair_idx) {
        spv::Decoration excl_dec_type = spv::Decoration::Max;
        if (mutually_exclusive_per_id[pair_idx][0] == dec_type) {
          excl_dec_type = mutually_exclusive_per_id[pair_idx][1];
        } else if (mutually_exclusive_per_id[pair_idx][1] == dec_type) {
          excl_dec_type = mutually_exclusive_per_id[pair_idx][0];
        } else {
          continue;
        }

        const auto excl_k = PerIDKey(excl_dec_type, id);
        if (seen_per_id.find(excl_k) != seen_per_id.end()) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                 << "ID '" << id << "' decorated with both "
                 << vstate.SpvDecorationString(dec_type) << " and "
                 << vstate.SpvDecorationString(excl_dec_type)
                 << " is not allowed.";
        }
      }
    } else if (spv::Op::OpMemberDecorate == inst.opcode() ||
               spv::Op::OpMemberDecorateIdEXT == inst.opcode()) {
      const auto id = words[1];
      const auto member_id = words[2];
      const auto dec_type = static_cast<spv::Decoration>(words[3]);
      const auto k = PerMemberKey(dec_type, id, member_id);
      const auto already_used = !seen_per_member.insert(k).second;
      if (already_used && AtMostOncePerMember(dec_type)) {
        return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
               << "ID '" << id << "', member '" << member_id
               << "' decorated with " << vstate.SpvDecorationString(dec_type)
               << " multiple times is not allowed.";
      }
      // Verify certain mutually exclusive decorations are not both applied on
      // a (ID, member) tuple.
      for (uint32_t pair_idx = 0;
           pair_idx < num_mutually_exclusive_per_mem_pairs; ++pair_idx) {
        spv::Decoration excl_dec_type = spv::Decoration::Max;
        if (mutually_exclusive_per_member[pair_idx][0] == dec_type) {
          excl_dec_type = mutually_exclusive_per_member[pair_idx][1];
        } else if (mutually_exclusive_per_member[pair_idx][1] == dec_type) {
          excl_dec_type = mutually_exclusive_per_member[pair_idx][0];
        } else {
          continue;
        }

        const auto excl_k = PerMemberKey(excl_dec_type, id, member_id);
        if (seen_per_member.find(excl_k) != seen_per_member.end()) {
          return vstate.diag(SPV_ERROR_INVALID_ID, vstate.FindDef(id))
                 << "ID '" << id << "', member '" << member_id
                 << "' decorated with both "
                 << vstate.SpvDecorationString(dec_type) << " and "
                 << vstate.SpvDecorationString(excl_dec_type)
                 << " is not allowed.";
        }
      }
    }
  }
  return SPV_SUCCESS;
}

spv_result_t CheckVulkanMemoryModelDeprecatedDecorations(
    ValidationState_t& vstate) {
  if (vstate.memory_model() != spv::MemoryModel::VulkanKHR) return SPV_SUCCESS;

  std::string msg;
  std::ostringstream str(msg);
  for (const auto& def : vstate.all_definitions()) {
    const auto inst = def.second;
    const auto id = inst->id();
    for (const auto& dec : vstate.id_decorations(id)) {
      const auto member = dec.struct_member_index();
      if (dec.dec_type() == spv::Decoration::Coherent ||
          dec.dec_type() == spv::Decoration::Volatile) {
        str << (dec.dec_type() == spv::Decoration::Coherent ? "Coherent"
                                                            : "Volatile");
        str << " decoration targeting " << vstate.getIdName(id);
        if (member != Decoration::kInvalidMember) {
          str << " (member index " << member << ")";
        }
        str << " is banned when using the Vulkan memory model.";
        return vstate.diag(SPV_ERROR_INVALID_ID, inst) << str.str();
      }
    }
  }
  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for FPRoundingMode
// decorations.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.
spv_result_t CheckFPRoundingModeForShaders(ValidationState_t& vstate,
                                           const Instruction& inst,
                                           const Decoration& decoration) {
  // Validates width-only conversion instruction for floating-point object
  // i.e., OpFConvert
  if (inst.opcode() != spv::Op::OpFConvert) {
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << "FPRoundingMode decoration can be applied only to a "
              "width-only conversion instruction for floating-point "
              "object.";
  }

  if (spvIsVulkanEnv(vstate.context()->target_env)) {
    const auto mode = spv::FPRoundingMode(decoration.params()[0]);
    if ((mode != spv::FPRoundingMode::RTE) &&
        (mode != spv::FPRoundingMode::RTZ)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << vstate.VkErrorID(4675)
             << "In Vulkan, the FPRoundingMode mode must only by RTE or RTZ.";
    }
  }

  // Validates Object operand of an OpStore
  for (const auto& use : inst.uses()) {
    const auto store = use.first;
    if (store->opcode() == spv::Op::OpFConvert) continue;
    if (spvOpcodeIsDebug(store->opcode())) continue;
    if (store->IsNonSemantic()) continue;
    if (spvOpcodeIsDecoration(store->opcode())) continue;
    if (store->opcode() != spv::Op::OpStore) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore.";
    }

    if (use.second != 2) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore.";
    }

    const auto ptr_inst = vstate.FindDef(store->GetOperandAs<uint32_t>(0));
    const auto ptr_type = vstate.FindDef(ptr_inst->GetOperandAs<uint32_t>(0));

    const auto half_float_id = ptr_type->GetOperandAs<uint32_t>(2);
    if (!vstate.IsFloatScalarOrVectorType(half_float_id) ||
        vstate.GetBitWidth(half_float_id) != 16) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore storing through a pointer "
                "to "
                "a 16-bit floating-point scalar or vector object.";
    }

    // Validates storage class of the pointer to the OpStore
    const auto storage = ptr_type->GetOperandAs<spv::StorageClass>(1);
    if (storage != spv::StorageClass::StorageBuffer &&
        storage != spv::StorageClass::Uniform &&
        storage != spv::StorageClass::PushConstant &&
        storage != spv::StorageClass::Input &&
        storage != spv::StorageClass::Output &&
        storage != spv::StorageClass::PhysicalStorageBuffer) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "FPRoundingMode decoration can be applied only to the "
                "Object operand of an OpStore in the StorageBuffer, "
                "PhysicalStorageBuffer, Uniform, PushConstant, Input, or "
                "Output Storage Classes.";
    }
  }
  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for the NonReadable or
// NonWritable
// decoration.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.  The |inst| parameter is the object being decorated.  This must
// be called after TypePass and AnnotateCheckDecorationsOfBuffers are called.
spv_result_t CheckNonReadableWritableDecorations(ValidationState_t& vstate,
                                                 const Instruction& inst,
                                                 const Decoration& decoration) {
  assert(inst.id() && "Parser ensures the target of the decoration has an ID");
  const bool is_non_writable =
      decoration.dec_type() == spv::Decoration::NonWritable;
  assert(is_non_writable ||
         decoration.dec_type() == spv::Decoration::NonReadable);

  if (decoration.struct_member_index() == Decoration::kInvalidMember) {
    // The target must be a memory object declaration.
    // First, it must be a variable or function parameter.
    const auto opcode = inst.opcode();
    const auto type_id = inst.type_id();
    if (opcode != spv::Op::OpVariable &&
        opcode != spv::Op::OpUntypedVariableKHR &&
        opcode != spv::Op::OpBufferPointerEXT &&
        opcode != spv::Op::OpFunctionParameter &&
        opcode != spv::Op::OpRawAccessChainNV) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of "
             << (is_non_writable ? "NonWritable" : "NonReadable")
             << " decoration must be a "
                "memory object "
                "declaration (a variable or a function parameter)";
    }
    const auto var_storage_class =
        opcode == spv::Op::OpVariable
            ? inst.GetOperandAs<spv::StorageClass>(2)
            : opcode == spv::Op::OpUntypedVariableKHR
                  ? inst.GetOperandAs<spv::StorageClass>(3)
                  : spv::StorageClass::Max;

    if (opcode == spv::Op::OpBufferPointerEXT) {
      auto result_type = vstate.FindDef(inst.type_id());
      auto sc = result_type->GetOperandAs<spv::StorageClass>(1);
      if (sc == spv::StorageClass::Uniform && is_non_writable) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << "Target of NonWritable decoration is invalid: "
               << "cannot be used to OpBufferPointerEXT "
               << "with Uniform storage class";
      }
      return SPV_SUCCESS;
    }

    if ((var_storage_class == spv::StorageClass::Function ||
         var_storage_class == spv::StorageClass::Private) &&
        vstate.features().nonwritable_var_in_function_or_private &&
        is_non_writable) {
      // New permitted feature in SPIR-V 1.4.
    } else if (var_storage_class == spv::StorageClass::TileAttachmentQCOM) {
    } else if (
        // It may point to a UBO, SSBO, storage image, or raw access chain.
        vstate.IsPointerToUniformBlock(type_id) ||
        vstate.IsPointerToStorageBuffer(type_id) ||
        vstate.IsPointerToStorageImage(type_id) ||
        vstate.IsPointerToTensor(type_id) ||
        opcode == spv::Op::OpRawAccessChainNV) {
    } else {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of "
             << (is_non_writable ? "NonWritable" : "NonReadable")
             << " decoration is invalid: "
                "must point to a "
                "storage image, tensor variable in UniformConstant storage "
                "class, uniform block, "
             << (vstate.features().nonwritable_var_in_function_or_private &&
                         is_non_writable
                     ? "storage buffer, or variable in Private or Function "
                       "storage class"
                     : "or storage buffer");
    }
  }

  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for Uniform or
// UniformId decorations. Otherwise emits a diagnostic and returns something
// other than SPV_SUCCESS. Assumes each decoration on a group has been
// propagated down to the group members.  The |inst| parameter is the object
// being decorated.
spv_result_t CheckUniformDecoration(ValidationState_t& vstate,
                                    const Instruction& inst,
                                    const Decoration& decoration) {
  const char* const dec_name = decoration.dec_type() == spv::Decoration::Uniform
                                   ? "Uniform"
                                   : "UniformId";

  // Uniform or UniformId must decorate an "object"
  //  - has a result ID
  //  - is an instantiation of a non-void type.  So it has a type ID, and that
  //  type is not void.

  // We already know the result ID is non-zero.

  if (inst.type_id() == 0) {
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << dec_name << " decoration applied to a non-object";
  }
  if (Instruction* type_inst = vstate.FindDef(inst.type_id())) {
    if (type_inst->opcode() == spv::Op::OpTypeVoid) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << dec_name << " decoration applied to a value with void type";
    }
  } else {
    // We might never get here because this would have been rejected earlier in
    // the flow.
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << dec_name << " decoration applied to an object with invalid type";
  }

  // Use of Uniform with OpDecorate is checked elsewhere.
  // Use of UniformId with OpDecorateId is checked elsewhere.

  if (decoration.dec_type() == spv::Decoration::UniformId) {
    assert(decoration.params().size() == 1 &&
           "Grammar ensures UniformId has one parameter");

    // The scope id is an execution scope.
    if (auto error =
            ValidateExecutionScope(vstate, &inst, decoration.params()[0]))
      return error;
  }

  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for NoSignedWrap or
// NoUnsignedWrap decorations. Otherwise emits a diagnostic and returns
// something other than SPV_SUCCESS. Assumes each decoration on a group has been
// propagated down to the group members.
spv_result_t CheckIntegerWrapDecoration(ValidationState_t& vstate,
                                        const Instruction& inst,
                                        const Decoration& decoration) {
  switch (inst.opcode()) {
    case spv::Op::OpIAdd:
    case spv::Op::OpISub:
    case spv::Op::OpIMul:
    case spv::Op::OpShiftLeftLogical:
    case spv::Op::OpSNegate:
      return SPV_SUCCESS;
    case spv::Op::OpExtInst:
    case spv::Op::OpExtInstWithForwardRefsKHR:
      // TODO(dneto): Only certain extended instructions allow these
      // decorations.  For now allow anything.
      return SPV_SUCCESS;
    default:
      break;
  }

  return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
         << (decoration.dec_type() == spv::Decoration::NoSignedWrap
                 ? "NoSignedWrap"
                 : "NoUnsignedWrap")
         << " decoration may not be applied to "
         << spvOpcodeString(inst.opcode());
}

// Returns SPV_SUCCESS if validation rules are satisfied for the Component
// decoration.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.
spv_result_t CheckComponentDecoration(ValidationState_t& vstate,
                                      const Instruction& inst,
                                      const Decoration& decoration) {
  assert(inst.id() && "Parser ensures the target of the decoration has an ID");
  assert(decoration.params().size() == 1 &&
         "Grammar ensures Component has one parameter");

  uint32_t type_id;
  if (decoration.struct_member_index() == Decoration::kInvalidMember) {
    // The target must be a memory object declaration.
    const auto opcode = inst.opcode();
    if (opcode != spv::Op::OpVariable &&
        opcode != spv::Op::OpFunctionParameter) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of Component decoration must be a memory object "
                "declaration (a variable or a function parameter)";
    }

    // Only valid for the Input and Output Storage Classes.
    const auto storage_class = opcode == spv::Op::OpVariable
                                   ? inst.GetOperandAs<spv::StorageClass>(2)
                                   : spv::StorageClass::Max;
    if (storage_class != spv::StorageClass::Input &&
        storage_class != spv::StorageClass::Output &&
        storage_class != spv::StorageClass::Max) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << "Target of Component decoration is invalid: must point to a "
                "Storage Class of Input(1) or Output(3). Found Storage "
                "Class "
             << uint32_t(storage_class);
    }

    type_id = inst.type_id();
    if (vstate.IsPointerType(type_id)) {
      const auto pointer = vstate.FindDef(type_id);
      type_id = pointer->GetOperandAs<uint32_t>(2);
    }
  } else {
    if (inst.opcode() != spv::Op::OpTypeStruct) {
      return vstate.diag(SPV_ERROR_INVALID_DATA, &inst)
             << "Attempted to get underlying data type via member index for "
                "non-struct type.";
    }
    type_id = inst.word(decoration.struct_member_index() + 2);
  }

  if (spvIsVulkanEnv(vstate.context()->target_env)) {
    // Strip the array, if present.
    while (vstate.GetIdOpcode(type_id) == spv::Op::OpTypeArray) {
      type_id = vstate.FindDef(type_id)->word(2u);
    }

    if (!vstate.IsIntScalarOrVectorType(type_id) &&
        !vstate.IsFloatScalarOrVectorType(type_id)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << vstate.VkErrorID(10583)
             << "Component decoration specified for type "
             << vstate.getIdName(type_id) << " that is not a scalar or vector";
    }

    const auto component = decoration.params()[0];
    if (component > 3) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << vstate.VkErrorID(4920)
             << "Component decoration value must not be greater than 3";
    }

    const auto dimension = vstate.GetDimension(type_id);
    const auto bit_width = vstate.GetBitWidth(type_id);
    if (bit_width == 16 || bit_width == 32) {
      const auto sum_component = component + dimension;
      if (sum_component > 4) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(4921)
               << "Sequence of components starting with " << component
               << " and ending with " << (sum_component - 1)
               << " gets larger than 3";
      }
    } else if (bit_width == 64) {
      if (dimension > 2) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(7703)
               << "Component decoration only allowed on 64-bit scalar and "
                  "2-component vector";
      }
      if (component == 1 || component == 3) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(4923)
               << "Component decoration value must not be 1 or 3 for 64-bit "
                  "data types";
      }
      // 64-bit is double per component dimension
      const auto sum_component = component + (2 * dimension);
      if (sum_component > 4) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(4922)
               << "Sequence of components starting with " << component
               << " and ending with " << (sum_component - 1)
               << " gets larger than 3";
      }
    }
  }

  return SPV_SUCCESS;
}

// Returns SPV_SUCCESS if validation rules are satisfied for the Block
// decoration.  Otherwise emits a diagnostic and returns something other than
// SPV_SUCCESS.
spv_result_t CheckBlockDecoration(ValidationState_t& vstate,
                                  const Instruction& inst,
                                  const Decoration& decoration) {
  assert(inst.id() && "Parser ensures the target of the decoration has an ID");
  if (inst.opcode() != spv::Op::OpTypeStruct) {
    const char* const dec_name = decoration.dec_type() == spv::Decoration::Block
                                     ? "Block"
                                     : "BufferBlock";
    return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
           << dec_name << " decoration on a non-struct type.";
  }
  return SPV_SUCCESS;
}

spv_result_t CheckLocationDecoration(ValidationState_t& vstate,
                                     const Instruction& inst,
                                     const Decoration& decoration) {
  if (inst.opcode() == spv::Op::OpVariable) return SPV_SUCCESS;

  if (decoration.struct_member_index() != Decoration::kInvalidMember &&
      inst.opcode() == spv::Op::OpTypeStruct) {
    return SPV_SUCCESS;
  }

  return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
         << "Location decoration can only be applied to a variable or member "
            "of a structure type";
}

spv_result_t CheckRelaxPrecisionDecoration(ValidationState_t& vstate,
                                           const Instruction& inst,
                                           const Decoration& decoration) {
  // This is not the most precise check, but the rules for RelaxPrecision are
  // very general, and it will be difficult to implement precisely.  For now,
  // I will only check for the cases that cause problems for the optimizer.
  if (!spvOpcodeGeneratesType(inst.opcode())) {
    return SPV_SUCCESS;
  }

  if (decoration.struct_member_index() != Decoration::kInvalidMember &&
      inst.opcode() == spv::Op::OpTypeStruct) {
    return SPV_SUCCESS;
  }
  return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
         << "RelaxPrecision decoration cannot be applied to a type";
}

#define PASS_OR_BAIL_AT_LINE(X, LINE)           \
  {                                             \
    spv_result_t e##LINE = (X);                 \
    if (e##LINE != SPV_SUCCESS) return e##LINE; \
  }                                             \
  static_assert(true, "require extra semicolon")
#define PASS_OR_BAIL(X) PASS_OR_BAIL_AT_LINE(X, __LINE__)

// Check rules for decorations where we start from the decoration rather
// than the decorated object.  Assumes each decoration on a group have been
// propagated down to the group members.
spv_result_t CheckDecorationsFromDecoration(ValidationState_t& vstate) {
  // Some rules are only checked for shaders.
  const bool is_shader = vstate.HasCapability(spv::Capability::Shader);

  for (const auto& kv : vstate.id_decorations()) {
    const uint32_t id = kv.first;
    const auto& decorations = kv.second;
    if (decorations.empty()) continue;

    const Instruction* inst = vstate.FindDef(id);
    assert(inst);

    // We assume the decorations applied to a decoration group have already
    // been propagated down to the group members.
    if (inst->opcode() == spv::Op::OpDecorationGroup) continue;

    for (const auto& decoration : decorations) {
      switch (decoration.dec_type()) {
        case spv::Decoration::Component:
          PASS_OR_BAIL(CheckComponentDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::FPRoundingMode:
          if (is_shader)
            PASS_OR_BAIL(
                CheckFPRoundingModeForShaders(vstate, *inst, decoration));
          break;
        case spv::Decoration::NonReadable:
        case spv::Decoration::NonWritable:
          PASS_OR_BAIL(
              CheckNonReadableWritableDecorations(vstate, *inst, decoration));
          break;
        case spv::Decoration::Uniform:
        case spv::Decoration::UniformId:
          PASS_OR_BAIL(CheckUniformDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::NoSignedWrap:
        case spv::Decoration::NoUnsignedWrap:
          PASS_OR_BAIL(CheckIntegerWrapDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::Block:
        case spv::Decoration::BufferBlock:
          PASS_OR_BAIL(CheckBlockDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::Location:
          PASS_OR_BAIL(CheckLocationDecoration(vstate, *inst, decoration));
          break;
        case spv::Decoration::RelaxedPrecision:
          PASS_OR_BAIL(
              CheckRelaxPrecisionDecoration(vstate, *inst, decoration));
          break;
        default:
          break;
      }
    }
  }
  return SPV_SUCCESS;
}

bool AllowsLayout(ValidationState_t& vstate, const spv::StorageClass sc) {
  switch (sc) {
    case spv::StorageClass::StorageBuffer:
    case spv::StorageClass::Uniform:
    case spv::StorageClass::PhysicalStorageBuffer:
    case spv::StorageClass::PushConstant:
      // Always explicitly laid out.
      return true;
    case spv::StorageClass::UniformConstant:
      return false;
    case spv::StorageClass::Workgroup:
      return vstate.HasCapability(
          spv::Capability::WorkgroupMemoryExplicitLayoutKHR);
    case spv::StorageClass::Function:
    case spv::StorageClass::Private:
      return vstate.version() <= SPV_SPIRV_VERSION_WORD(1, 4);
    case spv::StorageClass::Input:
    case spv::StorageClass::Output:
      // Block is used generally and mesh shaders use Offset.
      return true;
    default:
      // TODO: Some storage classes in ray tracing use explicit layout
      // decorations, but it is not well documented which. For now treat other
      // storage classes as allowed to be laid out. See Vulkan internal issue
      // 4192.
      return true;
  }
}

// Returns a decoration used to make it explicit
spv::Decoration UsesExplicitLayout(
    ValidationState_t& vstate, uint32_t type_id,
    std::unordered_map<uint32_t, spv::Decoration>& cache) {
  if (type_id == 0) {
    return spv::Decoration::Max;
  }

  if (cache.count(type_id)) {
    return cache[type_id];
  }

  spv::Decoration res = spv::Decoration::Max;
  const auto type_inst = vstate.FindDef(type_id);
  if (type_inst->opcode() == spv::Op::OpTypeStruct ||
      type_inst->opcode() == spv::Op::OpTypeArray ||
      type_inst->opcode() == spv::Op::OpTypeRuntimeArray ||
      type_inst->opcode() == spv::Op::OpTypePointer ||
      type_inst->opcode() == spv::Op::OpTypeUntypedPointerKHR) {
    const auto& id_decs = vstate.id_decorations();
    const auto iter = id_decs.find(type_id);
    if (iter != id_decs.end()) {
      bool allowLayoutDecorations = false;
      if (type_inst->opcode() == spv::Op::OpTypePointer ||
          type_inst->opcode() == spv::Op::OpTypeUntypedPointerKHR) {
        const auto sc = type_inst->GetOperandAs<spv::StorageClass>(1);
        allowLayoutDecorations = AllowsLayout(vstate, sc);
      }
      if (!allowLayoutDecorations) {
        for (const auto& d : iter->second) {
          const spv::Decoration dec = d.dec_type();
          if (dec == spv::Decoration::Block ||
              dec == spv::Decoration::BufferBlock ||
              dec == spv::Decoration::Offset ||
              dec == spv::Decoration::ArrayStride ||
              dec == spv::Decoration::MatrixStride) {
            res = dec;
            break;
          }
        }
      }
    }

    if (res == spv::Decoration::Max) {
      switch (type_inst->opcode()) {
        case spv::Op::OpTypeStruct:
          for (uint32_t i = 1;
               res == spv::Decoration::Max && i < type_inst->operands().size();
               i++) {
            res = UsesExplicitLayout(
                vstate, type_inst->GetOperandAs<uint32_t>(i), cache);
          }
          break;
        case spv::Op::OpTypeArray:
        case spv::Op::OpTypeRuntimeArray:
          res = UsesExplicitLayout(vstate, type_inst->GetOperandAs<uint32_t>(1),
                                   cache);
          break;
        case spv::Op::OpTypePointer: {
          const auto sc = type_inst->GetOperandAs<spv::StorageClass>(1);
          if (!AllowsLayout(vstate, sc)) {
            res = UsesExplicitLayout(
                vstate, type_inst->GetOperandAs<uint32_t>(2), cache);
          }
        }
        default:
          break;
      }
    }
  }

  cache[type_id] = res;
  return res;
}

spv_result_t CheckInvalidVulkanExplicitLayout(ValidationState_t& vstate) {
  if (!spvIsVulkanEnv(vstate.context()->target_env)) {
    return SPV_SUCCESS;
  }

  std::unordered_map<uint32_t, spv::Decoration> cache;
  for (const auto& inst : vstate.ordered_instructions()) {
    const auto type_id = inst.type_id();
    const auto type_inst = vstate.FindDef(type_id);

    spv::StorageClass sc = spv::StorageClass::Max;
    spv::Decoration layout_dec = spv::Decoration::Max;
    uint32_t fail_id = 0;
    uint32_t base_id = 0;
    // Variables are the main place to check for improper decorations, but some
    // untyped pointer instructions must also be checked since those types may
    // never be instantiated by a variable. Unlike verifying a valid layout,
    // physical storage buffer does not need checked here since it is always
    // explicitly laid out.
    switch (inst.opcode()) {
      case spv::Op::OpVariable:
      case spv::Op::OpUntypedVariableKHR: {
        sc = inst.GetOperandAs<spv::StorageClass>(2);
        auto check_id = type_id;
        base_id = inst.id();
        if (inst.opcode() == spv::Op::OpUntypedVariableKHR) {
          if (inst.operands().size() > 3) {
            check_id = inst.GetOperandAs<uint32_t>(3);
          }
        }
        if (!AllowsLayout(vstate, sc)) {
          layout_dec = UsesExplicitLayout(vstate, check_id, cache);
          if (layout_dec != spv::Decoration::Max) {
            fail_id = check_id;
          }
        }
        break;
      }
      case spv::Op::OpUntypedAccessChainKHR:
      case spv::Op::OpUntypedInBoundsAccessChainKHR:
      case spv::Op::OpUntypedPtrAccessChainKHR:
      case spv::Op::OpUntypedInBoundsPtrAccessChainKHR: {
        // Check both the base type and return type. The return type may have an
        // invalid array stride.
        sc = type_inst->GetOperandAs<spv::StorageClass>(1);
        base_id = vstate.FindDef(inst.GetOperandAs<uint32_t>(3))->id();
        if (!AllowsLayout(vstate, sc)) {
          const auto base_type_id = inst.GetOperandAs<uint32_t>(2);
          layout_dec = UsesExplicitLayout(vstate, base_type_id, cache);
          if (layout_dec != spv::Decoration::Max) {
            fail_id = base_type_id;
          } else {
            layout_dec = UsesExplicitLayout(vstate, type_id, cache);
            if (layout_dec != spv::Decoration::Max) {
              fail_id = type_id;
            }
          }
        }
        break;
      }
      case spv::Op::OpUntypedArrayLengthKHR: {
        // Check the data type.
        const auto ptr_ty_id =
            vstate.FindDef(inst.GetOperandAs<uint32_t>(3))->type_id();
        const auto ptr_ty = vstate.FindDef(ptr_ty_id);
        sc = ptr_ty->GetOperandAs<spv::StorageClass>(1);
        base_id = vstate.FindDef(inst.GetOperandAs<uint32_t>(3))->id();
        if (!AllowsLayout(vstate, sc)) {
          const auto base_type_id = inst.GetOperandAs<uint32_t>(2);
          layout_dec = UsesExplicitLayout(vstate, base_type_id, cache);
          if (layout_dec != spv::Decoration::Max) {
            fail_id = base_type_id;
          }
        }
        break;
      }
      case spv::Op::OpLoad: {
        const auto ptr_id = inst.GetOperandAs<uint32_t>(2);
        const auto ptr_type = vstate.FindDef(vstate.FindDef(ptr_id)->type_id());
        base_id = ptr_id;
        if (ptr_type->opcode() == spv::Op::OpTypeUntypedPointerKHR) {
          // For untyped pointers check the return type for an invalid layout.
          sc = ptr_type->GetOperandAs<spv::StorageClass>(1);
          if (!AllowsLayout(vstate, sc)) {
            layout_dec = UsesExplicitLayout(vstate, type_id, cache);
            if (layout_dec != spv::Decoration::Max) {
              fail_id = type_id;
            }
          }
        }
        break;
      }
      case spv::Op::OpStore: {
        const auto ptr_id = inst.GetOperandAs<uint32_t>(1);
        const auto ptr_type = vstate.FindDef(vstate.FindDef(ptr_id)->type_id());
        base_id = inst.GetOperandAs<uint32_t>(0);
        if (ptr_type->opcode() == spv::Op::OpTypeUntypedPointerKHR) {
          // For untyped pointers, check the type of the data operand for an
          // invalid layout.
          sc = ptr_type->GetOperandAs<spv::StorageClass>(1);
          if (!AllowsLayout(vstate, sc)) {
            const auto data_type_id = vstate.GetOperandTypeId(&inst, 1);
            layout_dec = UsesExplicitLayout(vstate, data_type_id, cache);
            if (layout_dec != spv::Decoration::Max) {
              fail_id = inst.GetOperandAs<uint32_t>(2);
            }
          }
        }
        break;
      }
      case spv::Op::OpBufferPointerEXT: {
        const auto ptr_id = inst.GetOperandAs<uint32_t>(1);
        const auto ptr_type = vstate.FindDef(vstate.FindDef(ptr_id)->type_id());
        // Check the type of the data operand for an invalid layout.
        sc = ptr_type->GetOperandAs<spv::StorageClass>(1);
        if (!AllowsLayout(vstate, sc) &&
            UsesExplicitLayout(vstate, type_id, cache) !=
                spv::Decoration::Max) {
          return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
                 << vstate.VkErrorID(11346)
                 << "The result type operand of OpBufferPointerEXT must have "
                 << "a Type operand that is explicitly laid out : "
                 << vstate.getIdName(type_id);
        } else if (sc != spv::StorageClass::StorageBuffer &&
                   sc != spv::StorageClass::Uniform) {
          return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
                 << "OpBufferPointerEXT's Result Type must be a pointer type "
                 << "with a Storage Class of Uniform or StorageBuffer.";
        }
        break;
      }
      default:
        break;
    }

    if (fail_id != 0 &&
        !vstate.IsDescriptorHeapBaseVariable(vstate.FindDef(base_id))) {
      return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
             << vstate.VkErrorID(10684)
             << "Invalid explicit layout decorations on type for operand "
             << vstate.getIdName(fail_id) << ", the "
             << spvtools::StorageClassToString(sc)
             << " storage class has a explicit layout from the "
             << vstate.SpvDecorationString(layout_dec) << " decoration.";
    }
  }

  return SPV_SUCCESS;
}

}  // namespace

spv_result_t ValidateDecorations(ValidationState_t& vstate) {
  if (auto error = CheckImportedVariableInitialization(vstate)) return error;
  if (auto error = CheckDecorationsOfEntryPoints(vstate)) return error;
  if (auto error = CheckDecorationsOfBuffers(vstate)) return error;
  if (auto error = CheckDecorationsOfVariables(vstate)) return error;
  if (auto error = CheckDecorationsCompatibility(vstate)) return error;
  if (auto error = CheckLinkageAttrOfFunctions(vstate)) return error;
  if (auto error = CheckVulkanMemoryModelDeprecatedDecorations(vstate))
    return error;
  if (auto error = CheckDecorationsFromDecoration(vstate)) return error;
  if (auto error = CheckInvalidVulkanExplicitLayout(vstate)) return error;
  return SPV_SUCCESS;
}

}  // namespace val
}  // namespace spvtools
