// Copyright (c) 2026 Google Inc.
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
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "source/spirv_constant.h"
#include "source/spirv_target_env.h"
#include "source/util/hash_combine.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

enum class LayoutMode : uint8_t {
  // Vulkan scalar block rules
  kScalar,
  // Vulkan standard alignment rules (i.e. std430)
  kStandard,
  // Vulkan extended alignment rules (i.e. std140)
  kExtended,
};

std::ostream& operator<<(std::ostream& str, const LayoutMode& mode) {
  switch (mode) {
    case LayoutMode::kScalar:
      str << "scalar";
      break;
    case LayoutMode::kStandard:
      str << "standard";
      break;
    case LayoutMode::kExtended:
      str << "extended";
      break;
  }
  return str;
}

enum class LayoutRequirement : uint8_t {
  // Must be laid out
  kRequired,
  // Must not be laid out
  kProhibited,
  // Either laid or not
  kAllowed,
};

struct Impl {
  ValidationState_t& vstate;

  // Relevant information to describe a memory instruction for the purposes of
  // layout validation.
  struct MemoryReference {
    // The data type of the memory instruction (after stripping any descriptor
    // array).
    uint32_t type_id = 0;
    // The descriptor array type id (if there is one).
    uint32_t descriptor_array_id = 0;
    // The storage class for the memory instruction.
    spv::StorageClass storage_class;
    // The layout mode (only relevant if a layout is required).
    LayoutMode layout;
    // The layout requirement for the instruction.
    LayoutRequirement requirement;
    // Whether it is an untyped pointer base.
    bool untyped = false;
  };

  // Matrix constraints from a struct member to carry to the actual matrix type.
  struct MatrixConstraints {
    uint32_t stride = 0;
    bool col_major = true;
  };

  // Cache valid checks for types that should have no layout.
  std::unordered_set<uint32_t> no_layout_cache_;

  // Struct member info
  struct MemberInfo {
    // Structure member index (note: index + 1 is instruction index).
    uint32_t index;
    // Whether or not an Offset/OffsetIdEXT decoration is present.
    bool has_offset = false;
    // Offset value. Max uint32_t is used for no evaluation (e.g. spec
    // constant).
    uint32_t offset = std::numeric_limits<uint32_t>::max();
    // Whether or not RowMajor or ColMajor decoration is present.
    bool has_matrix = false;
    // Matrix constraints (stride and majorness).
    MatrixConstraints matrix_constraints;
  };

  // Cache of structure member information.
  std::unordered_map<uint32_t, std::vector<MemberInfo>> struct_members_;

  struct LayoutKey {
    uint32_t type_id = 0;
    LayoutMode layout;
    uint32_t incoming_offset = 0;
    MatrixConstraints matrix_constraints{};

    bool operator==(const LayoutKey& other) const {
      return type_id == other.type_id && layout == other.layout &&
             incoming_offset == other.incoming_offset &&
             matrix_constraints.stride == other.matrix_constraints.stride &&
             matrix_constraints.col_major == other.matrix_constraints.col_major;
    }
  };
  struct LayoutKeyHash {
    size_t operator()(const LayoutKey& key) const noexcept {
      return spvtools::utils::hash_combine(
          0, key.type_id, static_cast<uint32_t>(key.layout),
          key.incoming_offset, key.matrix_constraints.stride,
          key.matrix_constraints.col_major);
    }
  };

  // Caches valid results of CheckLayout calls.
  std::unordered_set<LayoutKey, LayoutKeyHash> layout_cache_;

  // Returns the layout requirements for `sc`.
  // Workgroup is expected to be explicitly laid out if `is_block` is true.
  // UniformConstant is expected to be explicitly laid out if `descriptor_heap`
  // is true.
  LayoutRequirement GetStorageClassRequirement(spv::StorageClass sc,
                                               bool is_block,
                                               bool descriptor_heap) {
    switch (sc) {
      case spv::StorageClass::Workgroup:
        return is_block ? LayoutRequirement::kRequired
                        : LayoutRequirement::kProhibited;
      case spv::StorageClass::StorageBuffer:
      case spv::StorageClass::Uniform:
      case spv::StorageClass::PushConstant:
      case spv::StorageClass::PhysicalStorageBuffer:
        return LayoutRequirement::kRequired;
      case spv::StorageClass::UniformConstant:
        return descriptor_heap ? LayoutRequirement::kRequired
                               : LayoutRequirement::kProhibited;
      case spv::StorageClass::Function:
      case spv::StorageClass::Private:
        return vstate.version() <= SPV_SPIRV_VERSION_WORD(1, 4)
                   ? LayoutRequirement::kAllowed
                   : LayoutRequirement::kProhibited;
      case spv::StorageClass::Input:
      case spv::StorageClass::Output:
        // Block is used generally and mesh shaders use Offset.
        // TODO: This is a little over permissive.
        return LayoutRequirement::kAllowed;
      default:
        // TODO: Some storage classes in ray tracing use explicit layout
        // decorations, but it is not well documented which. For now treat
        // other storage classes as allowed to be laid out. See Vulkan
        // internal issue 4192.
        return LayoutRequirement::kAllowed;
    }
  }

  // Returns the layout rules for `sc`.
  LayoutMode GetStorageClassLayout(spv::StorageClass sc, bool is_buffer_block) {
    switch (sc) {
      case spv::StorageClass::Workgroup:
        return vstate.options()->workgroup_scalar_block_layout
                   ? LayoutMode::kScalar
                   : LayoutMode::kStandard;
        break;
      case spv::StorageClass::StorageBuffer:
      case spv::StorageClass::PushConstant:
      case spv::StorageClass::UniformConstant:
      case spv::StorageClass::PhysicalStorageBuffer:
        return vstate.options()->scalar_block_layout ? LayoutMode::kScalar
                                                     : LayoutMode::kStandard;
        break;
      case spv::StorageClass::Uniform:
        return vstate.options()->scalar_block_layout
                   ? LayoutMode::kScalar
                   : ((is_buffer_block ||
                       vstate.options()->uniform_buffer_standard_layout)
                          ? LayoutMode::kStandard
                          : LayoutMode::kExtended);
        break;
      default:
        break;
    }
    return LayoutMode::kStandard;
  }

  // Returns true if `inst` is a memory reference instruction
  // Populates `reference` with the necessary information.
  //
  // The following are interesting memory references:
  // For typed pointers:
  //  * OpVariable
  //  * Memory instructions on PhysicalStorageBuffer
  //  * OpBufferPointerEXT
  // For untyped pointers:
  //  * All memory instructions
  bool GetMemoryReference(const Instruction* inst, MemoryReference* reference) {
    auto* type_inst = vstate.FindDef(inst->type_id());
    if (inst->opcode() == spv::Op::OpVariable) {
      auto sc = type_inst->GetOperandAs<spv::StorageClass>(1u);
      const bool is_descriptor_heap = vstate.IsDescriptorHeapBaseVariable(inst);
      reference->storage_class = sc;
      reference->type_id = type_inst->GetOperandAs<uint32_t>(2u);
      const bool is_block_decorated =
          vstate.GetIdOpcode(reference->type_id) == spv::Op::OpTypeStruct &&
          vstate.HasDecoration(reference->type_id, spv::Decoration::Block);
      reference->requirement = GetStorageClassRequirement(
          sc, is_block_decorated, is_descriptor_heap);

      // Unwrap the descriptor array.
      if (sc == spv::StorageClass::StorageBuffer ||
          sc == spv::StorageClass::Uniform ||
          sc == spv::StorageClass::UniformConstant) {
        const auto* data_type = vstate.FindDef(reference->type_id);
        if (data_type->opcode() == spv::Op::OpTypeArray ||
            data_type->opcode() == spv::Op::OpTypeRuntimeArray) {
          reference->descriptor_array_id = reference->type_id;
          reference->type_id = data_type->GetOperandAs<uint32_t>(1u);
        }
      }

      const bool buffer_block =
          vstate.GetIdOpcode(reference->type_id) == spv::Op::OpTypeStruct &&
          vstate.HasDecoration(reference->type_id,
                               spv::Decoration::BufferBlock);
      reference->layout = GetStorageClassLayout(sc, buffer_block);

      return true;
    } else if (type_inst && type_inst->opcode() == spv::Op::OpTypePointer &&
               type_inst->GetOperandAs<spv::StorageClass>(1u) ==
                   spv::StorageClass::PhysicalStorageBuffer) {
      reference->storage_class = spv::StorageClass::PhysicalStorageBuffer;
      reference->type_id = type_inst->GetOperandAs<uint32_t>(2u);
      reference->layout = GetStorageClassLayout(
          spv::StorageClass::PhysicalStorageBuffer, false);
      reference->requirement = LayoutRequirement::kRequired;

      return true;
    } else if (inst->opcode() == spv::Op::OpBufferPointerEXT &&
               type_inst->opcode() == spv::Op::OpTypePointer) {
      auto sc = type_inst->GetOperandAs<spv::StorageClass>(1u);
      reference->storage_class = sc;
      uint32_t pointee_ty_id = type_inst->GetOperandAs<uint32_t>(2u);
      const bool buffer_block =
          vstate.GetIdOpcode(pointee_ty_id) == spv::Op::OpTypeStruct &&
          vstate.HasDecoration(pointee_ty_id, spv::Decoration::BufferBlock);
      reference->layout = GetStorageClassLayout(sc, buffer_block);
      reference->requirement = LayoutRequirement::kRequired;
      reference->type_id = pointee_ty_id;

      return true;
    } else if (vstate.HasCapability(spv::Capability::UntypedPointersKHR) &&
               spvIsVulkanEnv(vstate.context()->target_env)) {
      uint32_t ptr_ty_id = 0;
      uint32_t data_ty_id = 0;
      switch (inst->opcode()) {
        case spv::Op::OpUntypedVariableKHR:
          if (inst->operands().size() > 3) {
            ptr_ty_id = inst->type_id();
            data_ty_id = inst->GetOperandAs<uint32_t>(3u);
          } else {
            return false;
          }
          break;
        case spv::Op::OpUntypedAccessChainKHR:
        case spv::Op::OpUntypedInBoundsAccessChainKHR:
        case spv::Op::OpUntypedPtrAccessChainKHR:
        case spv::Op::OpUntypedInBoundsPtrAccessChainKHR:
          ptr_ty_id = inst->type_id();
          data_ty_id = inst->GetOperandAs<uint32_t>(2);
          break;
        case spv::Op::OpLoad:
          if (vstate.GetIdOpcode(vstate.GetOperandTypeId(inst, 2)) ==
              spv::Op::OpTypeUntypedPointerKHR) {
            const auto ptr_id = inst->GetOperandAs<uint32_t>(2);
            ptr_ty_id = vstate.FindDef(ptr_id)->type_id();
            data_ty_id = inst->type_id();
          } else {
            return false;
          }
          break;
        case spv::Op::OpStore:
          if (vstate.GetIdOpcode(vstate.GetOperandTypeId(inst, 0)) ==
              spv::Op::OpTypeUntypedPointerKHR) {
            const auto ptr_id = inst->GetOperandAs<uint32_t>(0);
            ptr_ty_id = vstate.FindDef(ptr_id)->type_id();
            data_ty_id = vstate.GetOperandTypeId(inst, 1);
          } else {
            return false;
          }
          break;
        case spv::Op::OpUntypedArrayLengthKHR:
          ptr_ty_id =
              vstate.FindDef(inst->GetOperandAs<uint32_t>(3))->type_id();
          data_ty_id = inst->GetOperandAs<uint32_t>(2);
          break;
        default:
          return false;
      }

      // If the data type is an array that contains a Block- or
      // BufferBlock-decorated struct, then use the struct for layout checks
      // instead of the array. In this case, the array represents a descriptor
      // array which should not have an explicit layout.
      const auto* data_type = vstate.FindDef(data_ty_id);
      if (data_type->opcode() == spv::Op::OpTypeArray ||
          data_type->opcode() == spv::Op::OpTypeRuntimeArray) {
        uint32_t ele_ty_id = data_type->GetOperandAs<uint32_t>(1u);
        if (vstate.HasDecoration(ele_ty_id, spv::Decoration::Block) ||
            vstate.HasDecoration(ele_ty_id, spv::Decoration::BufferBlock)) {
          reference->descriptor_array_id = data_ty_id;
          data_ty_id = ele_ty_id;
        }
      }

      auto sc = vstate.FindDef(ptr_ty_id)->GetOperandAs<spv::StorageClass>(1u);
      reference->storage_class = sc;
      reference->type_id = data_ty_id;
      reference->requirement = LayoutRequirement::kRequired;
      reference->untyped = true;
      const bool buffer_block =
          vstate.GetIdOpcode(data_ty_id) == spv::Op::OpTypeStruct &&
          vstate.HasDecoration(data_ty_id, spv::Decoration::BufferBlock);
      reference->layout = GetStorageClassLayout(sc, buffer_block);

      return true;
    }

    // Not a memory reference.
    return false;
  }

  // Checks that no instruction in the type tree of `type_id` has any explicit
  // layout decorations.
  spv_result_t CheckNoLayout(const Instruction* inst, uint32_t type_id,
                             spv::StorageClass sc) {
    if (no_layout_cache_.count(type_id)) {
      return SPV_SUCCESS;
    }

    const auto* type_inst = vstate.FindDef(type_id);
    if (type_inst->opcode() == spv::Op::OpTypePointer) {
      // PhysicalStorageBuffer and variable pointers can have ArrayStride
      // decorations even if they are stored in a non-laid storage class (e.g.
      // Function).
      auto ptr_sc = type_inst->GetOperandAs<spv::StorageClass>(1u);
      if (GetStorageClassRequirement(ptr_sc, true, false) !=
          LayoutRequirement::kProhibited) {
        return SPV_SUCCESS;
      }
    }

    const auto& id_decs = vstate.id_decorations();
    const auto iter = id_decs.find(type_id);
    if (iter != id_decs.end()) {
      for (const auto& d : iter->second) {
        const spv::Decoration dec = d.dec_type();
        if (dec == spv::Decoration::Block ||
            dec == spv::Decoration::BufferBlock ||
            dec == spv::Decoration::Offset ||
            dec == spv::Decoration::OffsetIdEXT ||
            dec == spv::Decoration::ArrayStride ||
            dec == spv::Decoration::ArrayStrideIdEXT ||
            dec == spv::Decoration::MatrixStride ||
            dec == spv::Decoration::RowMajor ||
            dec == spv::Decoration::ColMajor) {
          return vstate.diag(SPV_ERROR_INVALID_ID, inst)
                 << vstate.VkErrorID(10684)
                 << "Invalid explicit layout decorations on type "
                 << vstate.getIdName(type_id) << ", the "
                 << spvtools::StorageClassToString(sc)
                 << " storage class has an explicit layout from the "
                 << vstate.SpvDecorationString(dec) << " decoration";
        }
      }
    }

    switch (type_inst->opcode()) {
      case spv::Op::OpTypeStruct:
        for (uint32_t i = 1; i < type_inst->operands().size(); i++) {
          if (auto error = CheckNoLayout(
                  inst, type_inst->GetOperandAs<uint32_t>(i), sc)) {
            return error;
          }
        }
        break;
      case spv::Op::OpTypeRuntimeArray:
      case spv::Op::OpTypeArray:
        if (auto error = CheckNoLayout(
                inst, type_inst->GetOperandAs<uint32_t>(1u), sc)) {
          return error;
        }
        break;
      case spv::Op::OpTypePointer: {
        auto ptr_sc = type_inst->GetOperandAs<spv::StorageClass>(1u);
        if (auto error = CheckNoLayout(
                inst, type_inst->GetOperandAs<uint32_t>(2u), ptr_sc)) {
          return error;
        }
        break;
      }
      default:
        break;
    }

    no_layout_cache_.insert(type_id);

    return SPV_SUCCESS;
  }

  // Returns true if type_id contains a matrix. Only looks through arrays.
  bool ContainsMatrix(uint32_t type_id) {
    const auto* type_inst = vstate.FindDef(type_id);
    switch (type_inst->opcode()) {
      case spv::Op::OpTypeMatrix:
        return true;
      case spv::Op::OpTypeArray:
      case spv::Op::OpTypeRuntimeArray:
        return ContainsMatrix(type_inst->GetOperandAs<uint32_t>(1u));
      default:
        break;
    }

    return false;
  }

  // Gets the value for an id-based layout decoration (e.g. ArrayStrideIdEXT and
  // OffsetIdEXT). Returns max uint32_t if the value cannot be evaluated (e.g.
  // spec constant).
  uint32_t GetIdDecorationValue(uint32_t id) {
    const auto* inst = vstate.FindDef(id);
    if (!spvOpcodeIsConstant(inst->opcode())) {
      return std::numeric_limits<uint32_t>::max();
    }
    uint64_t value = 0;
    if (vstate.EvalConstantValUint64(id, &value)) {
      return static_cast<uint32_t>(value);
    }
    return std::numeric_limits<uint32_t>::max();
  }

  // Returns whether an array stride decoration is present on the array and its
  // value.
  std::pair<bool, uint32_t> GetArrayStride(uint32_t array_id) {
    uint32_t stride = std::numeric_limits<uint32_t>::max();
    bool has_stride = false;
    for (auto& d : vstate.id_decorations(array_id)) {
      if (d.dec_type() == spv::Decoration::ArrayStride) {
        stride = d.params()[0];
        has_stride = true;
        break;
      } else if (d.dec_type() == spv::Decoration::ArrayStrideIdEXT) {
        stride = GetIdDecorationValue(d.params()[0]);
        has_stride = true;
        break;
      }
    }
    return std::make_pair(has_stride, stride);
  }

  // Gets the offset value from an offset decoration.
  uint32_t GetOffset(spv::Decoration dec, uint32_t param) {
    // param is a literal value
    if (dec == spv::Decoration::Offset) {
      return param;
    }
    // param is an id
    return GetIdDecorationValue(param);
  }

  // Returns true if value is aligned to align.
  bool IsAlignedTo(uint32_t value, uint32_t align) {
    if (align == 0) return value == 0;
    return (value % align) == 0;
  }

  // Rounds up value to next multiple of align.
  uint32_t AlignTo(uint32_t value, uint32_t align) {
    return (value + align - 1) & ~(align - 1);
  }

  // A member is defined to improperly straddle if either of the following are
  // true:
  // - It is a vector with total size less than or equal to 16 bytes, and has
  // Offset decorations placing its first byte at F and its last byte at L,
  // where floor(F / 16) != floor(L / 16).
  // - It is a vector with total size greater than 16 bytes and has its Offset
  // decorations placing its first byte at a non-integer multiple of 16.
  bool HasImproperStraddle(uint32_t offset, uint32_t size) {
    const auto F = offset;
    const auto L = offset + size - 1;
    if (size <= 16) {
      if ((F >> 4) != (L >> 4)) return true;
    } else {
      if (F % 16 != 0) return true;
    }
    return false;
  }

  // Returns the alignment for type_id for the given layout rules.
  uint32_t GetAlign(uint32_t type_id, LayoutMode mode,
                    const MatrixConstraints& matrix_constraints,
                    bool allow_relaxed = true) {
    const auto* type_inst = vstate.FindDef(type_id);
    uint32_t align = 1;
    switch (type_inst->opcode()) {
      case spv::Op::OpTypeSampledImage:
      case spv::Op::OpTypeSampler:
      case spv::Op::OpTypeImage:
        if (vstate.HasCapability(spv::Capability::BindlessTextureNV)) {
          return vstate.samplerimage_variable_address_mode() / 8;
        }
        if (type_inst->opcode() == spv::Op::OpTypeSampler) {
          return vstate.options()->sampler_descriptor_layout.alignment;
        }
        if (type_inst->opcode() == spv::Op::OpTypeImage) {
          return vstate.options()->image_descriptor_layout.alignment;
        }
        break;
      case spv::Op::OpTypeBufferEXT:
      case spv::Op::OpTypeAccelerationStructureKHR:
        return vstate.options()->buffer_descriptor_layout.alignment;
      case spv::Op::OpTypeInt:
      case spv::Op::OpTypeFloat:
        return type_inst->GetOperandAs<uint32_t>(1u) / 8;
      case spv::Op::OpTypeVector:
      case spv::Op::OpTypeVectorIdEXT: {
        const auto ele_id = type_inst->GetOperandAs<uint32_t>(1u);
        const auto num_eles = vstate.GetDimension(type_id);
        align = GetAlign(ele_id, mode, {});
        // Relaxed layout only applies to vectors as direct members of structs.
        // Arrays and matrices are not relaxed.
        if (mode == LayoutMode::kScalar ||
            (allow_relaxed && vstate.IsRelaxedBlockLayout())) {
          return align;
        }
        return align * ((num_eles == 3 || num_eles > 4) ? 4 : num_eles);
      }
      case spv::Op::OpTypeMatrix:
        if (mode == LayoutMode::kScalar) {
          const auto* vec_inst =
              vstate.FindDef(type_inst->GetOperandAs<uint32_t>(1u));
          const auto ele_id = vec_inst->GetOperandAs<uint32_t>(1u);
          return GetAlign(ele_id, mode, {});
        }

        if (matrix_constraints.col_major) {
          align = GetAlign(type_inst->GetOperandAs<uint32_t>(1u), mode, {},
                           /* allow_relaxed = */ false);
        } else {
          // A row-major matrix of C columns has a base alignment equal to the
          // base alignment of a vector of C matrix components.
          const auto num_cols = type_inst->GetOperandAs<uint32_t>(2u);
          const auto* col_inst =
              vstate.FindDef(type_inst->GetOperandAs<uint32_t>(1u));
          const auto ele_id = col_inst->GetOperandAs<uint32_t>(1u);
          align = GetAlign(ele_id, mode, {});
          // The equivalent vector may not exist so we replicate the vector rule
          // here.
          if (mode != LayoutMode::kScalar) {
            align = align * (num_cols == 3 ? 4 : num_cols);
          }
        }
        if (mode == LayoutMode::kExtended) {
          align = AlignTo(align, 16u);
        }
        return align;
      case spv::Op::OpTypeArray:
      case spv::Op::OpTypeRuntimeArray:
        align = GetAlign(type_inst->GetOperandAs<uint32_t>(1u), mode,
                         matrix_constraints, /* allow_relaxed = */ false);
        if (mode == LayoutMode::kExtended) {
          align = AlignTo(align, 16u);
        }
        return align;
      case spv::Op::OpTypeStruct:
        for (uint32_t i = 1; i < type_inst->operands().size(); i++) {
          const auto member_id = type_inst->GetOperandAs<uint32_t>(i);
          MatrixConstraints mat_constraints;
          GetMatrixConstraints(type_id, i - 1, &mat_constraints);
          align = std::max(align, GetAlign(member_id, mode, mat_constraints));
          if (mode == LayoutMode::kExtended) {
            align = AlignTo(align, 16u);
          }
        }
        return align;
      case spv::Op::OpTypePointer:
      case spv::Op::OpTypeUntypedPointerKHR:
        return vstate.pointer_size_and_alignment();
      default:
        break;
    }
    assert(0 && "unhandled type");
    return 1;
  }

  // Returns the size of the given type.
  uint32_t GetSize(uint32_t type_id,
                   const MatrixConstraints& matrix_constraints) {
    const auto* type_inst = vstate.FindDef(type_id);
    switch (type_inst->opcode()) {
      case spv::Op::OpTypeSampledImage:
      case spv::Op::OpTypeSampler:
      case spv::Op::OpTypeImage:
        if (vstate.HasCapability(spv::Capability::BindlessTextureNV)) {
          return vstate.samplerimage_variable_address_mode() / 8;
        }
        if (type_inst->opcode() == spv::Op::OpTypeSampler) {
          return vstate.options()->sampler_descriptor_layout.size;
        }
        if (type_inst->opcode() == spv::Op::OpTypeImage) {
          return vstate.options()->image_descriptor_layout.size;
        }
        break;
      case spv::Op::OpTypeBufferEXT:
      case spv::Op::OpTypeAccelerationStructureKHR:
        return vstate.options()->buffer_descriptor_layout.size;
      case spv::Op::OpTypeInt:
      case spv::Op::OpTypeFloat:
        return type_inst->GetOperandAs<uint32_t>(1u) / 8;
      case spv::Op::OpTypeVector:
      case spv::Op::OpTypeVectorIdEXT: {
        const auto ele_id = type_inst->GetOperandAs<uint32_t>(1u);
        const auto num_eles = vstate.GetDimension(type_id);
        return GetSize(ele_id, {}) * num_eles;
      }
      case spv::Op::OpTypeArray: {
        const auto count_id = type_inst->GetOperandAs<uint32_t>(2u);
        uint64_t count = 0;
        if (!vstate.EvalConstantValUint64(count_id, &count)) {
          return 0;
        }
        const auto [has_stride, stride] = GetArrayStride(type_id);
        const auto ele_size =
            GetSize(type_inst->GetOperandAs<uint32_t>(1u), matrix_constraints);
        // uint32 max is a marker for unevaluatable.
        if (stride == std::numeric_limits<uint32_t>::max()) {
          return ele_size;
        }
        return (static_cast<uint32_t>(count) - 1) * stride + ele_size;
      }
      case spv::Op::OpTypeRuntimeArray:
        return 0;
      case spv::Op::OpTypeMatrix: {
        const auto num_cols = type_inst->GetOperandAs<uint32_t>(2u);
        if (matrix_constraints.col_major) {
          return num_cols * matrix_constraints.stride;
        } else {
          const auto* col_inst =
              vstate.FindDef(type_inst->GetOperandAs<uint32_t>(1u));
          const auto ele_id = col_inst->GetOperandAs<uint32_t>(1u);
          const auto num_rows = col_inst->GetOperandAs<uint32_t>(2u);
          return (num_rows - 1) * matrix_constraints.stride +
                 num_cols * GetSize(ele_id, {});
        }
      }
      case spv::Op::OpTypeStruct: {
        const auto& members = GetStructMembers(type_id);
        if (members.empty()) return 0;
        const auto& last = members.back();
        if (last.offset == std::numeric_limits<uint32_t>::max()) {
          return 0;
        }
        return last.offset +
               GetSize(type_inst->GetOperandAs<uint32_t>(last.index + 1),
                       last.matrix_constraints);
      }
      case spv::Op::OpTypePointer:
      case spv::Op::OpTypeUntypedPointerKHR:
        return vstate.pointer_size_and_alignment();
      default:
        break;
    }
    assert(0 && "unhandled type");
    return 0;
  }

  // Returns true if type_id has a matrix and populates matrix_constraints.
  bool GetMatrixConstraints(uint32_t type_id, uint32_t index,
                            MatrixConstraints* matrix_constraints) {
    bool has_matrix = false;
    auto member_decorations = vstate.id_member_decorations(type_id, index);
    for (auto decoration = member_decorations.begin;
         decoration != member_decorations.end; ++decoration) {
      if (decoration->dec_type() == spv::Decoration::ColMajor ||
          decoration->dec_type() == spv::Decoration::RowMajor) {
        has_matrix = true;
        matrix_constraints->col_major =
            decoration->dec_type() == spv::Decoration::ColMajor;
      }
      if (decoration->dec_type() == spv::Decoration::MatrixStride) {
        matrix_constraints->stride = decoration->params()[0];
      }
    }
    return has_matrix;
  }

  // Gathers (and caches) structure members and their decorations.
  const std::vector<MemberInfo>& GetStructMembers(uint32_t type_id) {
    if (struct_members_.count(type_id)) {
      return struct_members_[type_id];
    }

    const auto* type_inst = vstate.FindDef(type_id);
    std::vector<MemberInfo> member_info;
    member_info.reserve(type_inst->operands().size() - 1);
    for (uint32_t i = 1; i < type_inst->operands().size(); i++) {
      auto member_idx = i - 1;
      auto member_decorations =
          vstate.id_member_decorations(type_id, member_idx);
      member_info.push_back(MemberInfo{member_idx});
      auto& member = member_info.back();
      member.has_matrix =
          GetMatrixConstraints(type_id, member_idx, &member.matrix_constraints);
      for (auto decoration = member_decorations.begin;
           decoration != member_decorations.end; ++decoration) {
        switch (decoration->dec_type()) {
          case spv::Decoration::Offset:
          case spv::Decoration::OffsetIdEXT:
            member.has_offset = true;
            member.offset =
                GetOffset(decoration->dec_type(), decoration->params()[0]);
            break;
          default:
            break;
        }
      }
    }
    // Sort by offset value.
    std::stable_sort(member_info.begin(), member_info.end(),
                     [](const MemberInfo& lhs, const MemberInfo& rhs) {
                       return lhs.offset < rhs.offset;
                     });
    struct_members_[type_id] = std::move(member_info);
    return struct_members_[type_id];
  }

  // Returns some common messaging to improve diagnostics.
  std::string CommonError(const Instruction* inst,
                          spv::StorageClass storage_class, LayoutMode mode) {
    std::string s;
    std::ostringstream str(s);
    str << " Instantiated via " << vstate.getIdName(inst->id()) << " in the "
        << spvtools::StorageClassToString(storage_class)
        << " storage class using " << mode << " layout rules.";
    if (mode != LayoutMode::kScalar) {
      if (storage_class == spv::StorageClass::Workgroup) {
        str << vstate.MissingFeature(
            "workgroupMemoryExplicitLayoutScalarBlockLayout feature",
            "--workgroup-scalar-block-layout", true);
      } else if (!vstate.IsRelaxedBlockLayout()) {
        str << vstate.MissingFeature("VK_KHR_relaxed_block_layout extension",
                                     "--relax-block-layout", true);
      } else if (storage_class == spv::StorageClass::Uniform &&
                 mode != LayoutMode::kStandard) {
        str << vstate.MissingFeature("uniformBufferStandardLayout feature",
                                     "--uniform-buffer-standard-layout", true);
      } else {
        str << vstate.MissingFeature("scalarBlockLayout feature",
                                     "--scalar-block-layout", true);
      }
    }
    return str.str();
  }

  // Checks struct layouts
  // * Each member must
  //   * Have an offset decoration
  //   * If the member is a matrix or array(s) of matrices
  //     * Must have majorness and matrix stride decorations
  //   * If member is a runtime array, it must be last by offset
  //   * Offset must be aligned
  //   * Total offset must be aligned
  //   * Offset + size < next elements offset
  //   * If member is a vector, it must have a valid straddle
  //   * The member has a valid layout
  spv_result_t CheckStructLayout(const Instruction* inst, uint32_t type_id,
                                 spv::StorageClass storage_class,
                                 LayoutMode mode, uint32_t incoming_offset,
                                 const MatrixConstraints& matrix_constraints) {
    const auto* type_inst = vstate.FindDef(type_id);
    const auto& member_info = GetStructMembers(type_id);
    for (auto& member : member_info) {
      const auto member_id =
          type_inst->GetOperandAs<uint32_t>(member.index + 1);
      if (!member.has_offset) {
        return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
               << "Structure member " << member.index
               << " must be explicitly laid out with Offset or OffsetIdEXT "
                  "decorations."
               << CommonError(inst, storage_class, mode);
      }
      if (ContainsMatrix(member_id)) {
        if (!member.has_matrix) {
          return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
                 << "Structure member " << member.index
                 << " containing a matrix must be explicitly laid out "
                    "with RowMajor or ColMajor decorations."
                 << CommonError(inst, storage_class, mode);
        } else if (member.matrix_constraints.stride == 0) {
          return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
                 << "Structure member " << member.index
                 << " containing a matrix must be explicitly laid out "
                    "with MatrixStride decorations."
                 << CommonError(inst, storage_class, mode);
        }
      }
    }

    uint32_t next_offset = 0;
    uint32_t ordered_index = 0;
    for (const auto& member : member_info) {
      auto member_idx = member.index;
      auto offset = member.offset;
      // We have no information
      if (offset == std::numeric_limits<uint32_t>::max()) {
        continue;
      }
      auto member_id = type_inst->GetOperandAs<uint32_t>(member_idx + 1);
      auto member_inst = vstate.FindDef(member_id);
      if (member_inst->opcode() == spv::Op::OpTypeRuntimeArray &&
          ordered_index != member_info.size() - 1) {
        return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
               << vstate.VkErrorID(4680)
               << "Structure has a runtime array at offset " << offset
               << ", but other members at larger offsets."
               << CommonError(inst, storage_class, mode);
      }
      ordered_index++;

      MatrixConstraints mat_constraints = matrix_constraints;
      if (member.has_matrix) {
        mat_constraints = member.matrix_constraints;
      }
      uint32_t align = GetAlign(member_id, mode, mat_constraints);
      if (!IsAlignedTo(offset, align)) {
        return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
               << "Structure member " << member_idx << " at offset " << offset
               << " is not aligned to " << align << "."
               << CommonError(inst, storage_class, mode);
      }
      if (!IsAlignedTo(offset + incoming_offset, align)) {
        return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
               << "Structure member " << member_idx << " at offset " << offset
               << " plus incoming offset " << incoming_offset
               << " is not aligned to " << align << "."
               << CommonError(inst, storage_class, mode);
      }
      if (offset < next_offset) {
        return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
               << "Structure member " << member_idx << " at offset " << offset
               << " overlaps previous member ending at offset "
               << next_offset - 1 << "."
               << CommonError(inst, storage_class, mode);
      }
      if (mode != LayoutMode::kScalar && vstate.IsRelaxedBlockLayout()) {
        if (member_inst->opcode() == spv::Op::OpTypeVector ||
            member_inst->opcode() == spv::Op::OpTypeVectorIdEXT) {
          uint32_t size = GetSize(member_id, {});
          if (HasImproperStraddle(incoming_offset + offset, size)) {
            return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
                   << "Structure member " << member_idx
                   << ": Vector has improper straddle due to offset "
                   << incoming_offset + offset << "."
                   << CommonError(inst, storage_class, mode);
          }
        }
      }
      if (auto error = CheckLayout(inst, member_id, storage_class, mode,
                                   incoming_offset + offset, mat_constraints)) {
        return error;
      }

      uint32_t size = GetSize(member_id, mat_constraints);
      next_offset = size + offset;
      if (mode != LayoutMode::kScalar &&
          (member_inst->opcode() == spv::Op::OpTypeArray ||
           member_inst->opcode() == spv::Op::OpTypeStruct)) {
        next_offset = AlignTo(next_offset, align);
      }
    }

    return SPV_SUCCESS;
  }

  // Checks array layouts
  // * Arrays must have stride decoration
  // * Stride must be non-zero
  // * Stride must be aligned
  // * Stride must be greater or equal to element size
  // * Elements have valid layout
  spv_result_t CheckArrayLayout(const Instruction* inst, uint32_t type_id,
                                spv::StorageClass storage_class,
                                LayoutMode mode, uint32_t incoming_offset,
                                const MatrixConstraints& matrix_constraints) {
    const auto* type_inst = vstate.FindDef(type_id);
    auto [has_stride, stride] = GetArrayStride(type_id);
    if (!has_stride) {
      return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
             << "Array must be explicitly laid out with ArrayStride or "
                "ArrayStrideIdEXT decorations."
             << CommonError(inst, storage_class, mode);
    }
    uint32_t ele_id = type_inst->GetOperandAs<uint32_t>(1u);
    uint32_t ele_size = GetSize(ele_id, matrix_constraints);
    uint32_t align = GetAlign(type_id, mode, matrix_constraints);
    if (stride == 0) {
      return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
             << "Array must not have a stride of 0."
             << CommonError(inst, storage_class, mode);
    }

    // uint32 max stride is unevaluatable (e.g. spec constant).
    if (stride == std::numeric_limits<uint32_t>::max()) {
      return SPV_SUCCESS;
    }

    if (!IsAlignedTo(stride, align)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
             << "Array stride " << stride << " must satisfy alignment " << align
             << "." << CommonError(inst, storage_class, mode);
    }
    if (stride != std::numeric_limits<uint32_t>::max() && stride < ele_size) {
      return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
             << "Array stride " << stride
             << " is smaller than element type size " << ele_size << "."
             << CommonError(inst, storage_class, mode);
    }

    uint32_t num_elements = 0;
    if (type_inst->opcode() == spv::Op::OpTypeArray) {
      uint64_t count = 0;
      if (vstate.EvalConstantValUint64(type_inst->GetOperandAs<uint32_t>(2u),
                                       &count)) {
        num_elements = static_cast<uint32_t>(count);
      }
    }
    num_elements = std::max(1u, num_elements);
    std::vector<bool> seen(16, false);
    for (uint32_t i = 0; i < num_elements; ++i) {
      uint32_t next_offset = i * stride + incoming_offset;
      // Stop checking if offsets repeat in terms of 16-byte multiples.
      if (seen[next_offset % 16]) {
        break;
      }

      if (auto error = CheckLayout(inst, ele_id, storage_class, mode,
                                   next_offset, matrix_constraints)) {
        return error;
      }

      seen[next_offset % 16] = true;
    }
    return SPV_SUCCESS;
  }

  // Checks the layout matrices
  // * Stride must be a multiple of align
  // * Stride must be greater or equal to minor size
  spv_result_t CheckMatrixLayout(const Instruction* inst, uint32_t type_id,
                                 spv::StorageClass storage_class,
                                 LayoutMode mode,
                                 const MatrixConstraints& matrix_constraints) {
    // We already checked that any struct containing a matrix has a non-zero
    // stride so if we have 0 stride here then it will come from the result
    // of an access chain or other instruction. Other rules are meant to
    // catch any misuse we can skip it here.
    if (matrix_constraints.stride == 0) {
      return SPV_SUCCESS;
    }
    const auto* type_inst = vstate.FindDef(type_id);
    const auto align = GetAlign(type_id, mode, matrix_constraints);
    if (!IsAlignedTo(matrix_constraints.stride, align)) {
      return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
             << "Matrix with a stride " << matrix_constraints.stride
             << " not satisfying alignment to " << align << "."
             << CommonError(inst, storage_class, mode) << "\n";
    }
    const auto ele_id = type_inst->GetOperandAs<uint32_t>(1u);
    uint32_t size = 0;
    if (matrix_constraints.col_major) {
      size = GetSize(ele_id, {});
    } else {
      // Element size is # cols * ele size.
      const auto* ele_inst = vstate.FindDef(ele_id);
      const auto scalar_id = ele_inst->GetOperandAs<uint32_t>(1u);
      size = GetSize(scalar_id, {}) * type_inst->GetOperandAs<uint32_t>(2u);
    }
    if (matrix_constraints.stride < size) {
      return vstate.diag(SPV_ERROR_INVALID_ID, type_inst)
             << "Matrix stride " << matrix_constraints.stride
             << " is smaller than column size " << size << "."
             << CommonError(inst, storage_class, mode);
    }
    return SPV_SUCCESS;
  }

  // Returns true if type_id satisfies the given layout rules.
  spv_result_t CheckLayout(const Instruction* inst, uint32_t type_id,
                           spv::StorageClass storage_class, LayoutMode mode,
                           uint32_t incoming_offset,
                           const MatrixConstraints& matrix_constraints) {
    if (vstate.options()->skip_block_layout) {
      return SPV_SUCCESS;
    }

    LayoutKey key{type_id, mode, incoming_offset, matrix_constraints};
    if (layout_cache_.count(key)) {
      return SPV_SUCCESS;
    }

    const auto* type_inst = vstate.FindDef(type_id);
    switch (type_inst->opcode()) {
      case spv::Op::OpTypeStruct:
        if (auto error =
                CheckStructLayout(inst, type_id, storage_class, mode,
                                  incoming_offset, matrix_constraints)) {
          return error;
        }
        break;
      case spv::Op::OpTypeArray:
      case spv::Op::OpTypeRuntimeArray:
        if (auto error =
                CheckArrayLayout(inst, type_id, storage_class, mode,
                                 incoming_offset, matrix_constraints)) {
          return error;
        }
        break;
      case spv::Op::OpTypeMatrix:
        if (auto error = CheckMatrixLayout(inst, type_id, storage_class, mode,
                                           matrix_constraints)) {
          return error;
        }
        break;
      default:
        break;
    }

    layout_cache_.insert(key);

    return SPV_SUCCESS;
  }

  // Performs a single pass over the IR and for each memory reference determines
  // what validation is necessary.
  spv_result_t Run() {
    if (!spvIsVulkanEnv(vstate.context()->target_env)) {
      return SPV_SUCCESS;
    }

    for (const auto& inst : vstate.ordered_instructions()) {
      MemoryReference reference;
      if (!GetMemoryReference(&inst, &reference)) {
        continue;
      }

      // Descriptor arrays shouldn't have a stride. Check for that here since
      // most descriptors require a layout.
      if (reference.descriptor_array_id != 0) {
        const bool array_stride = vstate.HasDecoration(
            reference.descriptor_array_id, spv::Decoration::ArrayStride);
        const bool array_stride_id = vstate.HasDecoration(
            reference.descriptor_array_id, spv::Decoration::ArrayStrideIdEXT);
        if (array_stride || array_stride_id) {
          return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
                 << vstate.VkErrorID(10684)
                 << "Invalid explicit layout decorations on type "
                 << vstate.getIdName(reference.descriptor_array_id) << ", the "
                 << spvtools::StorageClassToString(reference.storage_class)
                 << " storage class has an explicit layout from the "
                 << vstate.SpvDecorationString(
                        array_stride ? spv::Decoration::ArrayStride
                                     : spv::Decoration::ArrayStrideIdEXT)
                 << " decoration";
        }
      }

      // Untyped pointers require a layout. Workgroup variables must be blocks
      // to have a layout.
      if (reference.untyped &&
          reference.storage_class == spv::StorageClass::Workgroup &&
          inst.opcode() == spv::Op::OpUntypedVariableKHR &&
          (vstate.GetIdOpcode(reference.type_id) != spv::Op::OpTypeStruct ||
           !vstate.HasDecoration(reference.type_id, spv::Decoration::Block))) {
        return vstate.diag(SPV_ERROR_INVALID_ID, &inst)
               << vstate.VkErrorID(10684)
               << "Untyped variables in Workgroup storage class must be "
                  "block-decorated structs";
      }

      if (reference.requirement == LayoutRequirement::kRequired) {
        if (auto error =
                CheckLayout(&inst, reference.type_id, reference.storage_class,
                            reference.layout, 0, {})) {
          return error;
        }
      } else if (reference.requirement == LayoutRequirement::kProhibited) {
        if (auto error = CheckNoLayout(&inst, reference.type_id,
                                       reference.storage_class)) {
          return error;
        }
      }
    }
    return SPV_SUCCESS;
  }
};

}  // namespace

spv_result_t ValidateExplicitLayout(ValidationState_t& vstate) {
  return Impl{vstate}.Run();
}

}  // namespace val
}  // namespace spvtools
