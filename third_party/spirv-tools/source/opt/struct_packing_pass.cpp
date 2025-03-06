// Copyright (c) 2024 Epic Games, Inc.
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

#include "struct_packing_pass.h"

#include <algorithm>

#include "source/opt/instruction.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {

/*
Std140 packing rules from the original GLSL 140 specification (see
https://registry.khronos.org/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt)

When using the "std140" storage layout, structures will be laid out in
buffer storage with its members stored in monotonically increasing order
based on their location in the declaration. A structure and each
structure member have a base offset and a base alignment, from which an
aligned offset is computed by rounding the base offset up to a multiple of
the base alignment. The base offset of the first member of a structure is
taken from the aligned offset of the structure itself. The base offset of
all other structure members is derived by taking the offset of the last
basic machine unit consumed by the previous member and adding one. Each
structure member is stored in memory at its aligned offset. The members
of a top-level uniform block are laid out in buffer storage by treating
the uniform block as a structure with a base offset of zero.

(1) If the member is a scalar consuming <N> basic machine units, the
    base alignment is <N>.

(2) If the member is a two- or four-component vector with components
    consuming <N> basic machine units, the base alignment is 2<N> or
    4<N>, respectively.

(3) If the member is a three-component vector with components consuming
    <N> basic machine units, the base alignment is 4<N>.

(4) If the member is an array of scalars or vectors, the base alignment
    and array stride are set to match the base alignment of a single
    array element, according to rules (1), (2), and (3), and rounded up
    to the base alignment of a vec4. The array may have padding at the
    end; the base offset of the member following the array is rounded up
    to the next multiple of the base alignment.

(5) If the member is a column-major matrix with <C> columns and <R>
    rows, the matrix is stored identically to an array of <C> column
    vectors with <R> components each, according to rule (4).

(6) If the member is an array of <S> column-major matrices with <C>
    columns and <R> rows, the matrix is stored identically to a row of
    <S>*<C> column vectors with <R> components each, according to rule
    (4).

(7) If the member is a row-major matrix with <C> columns and <R> rows,
    the matrix is stored identically to an array of <R> row vectors
    with <C> components each, according to rule (4).

(8) If the member is an array of <S> row-major matrices with <C> columns
    and <R> rows, the matrix is stored identically to a row of <S>*<R>
    row vectors with <C> components each, according to rule (4).

(9) If the member is a structure, the base alignment of the structure is
    <N>, where <N> is the largest base alignment value of any of its
    members, and rounded up to the base alignment of a vec4. The
    individual members of this sub-structure are then assigned offsets
    by applying this set of rules recursively, where the base offset of
    the first member of the sub-structure is equal to the aligned offset
    of the structure. The structure may have padding at the end; the
    base offset of the member following the sub-structure is rounded up
    to the next multiple of the base alignment of the structure.

(10) If the member is an array of <S> structures, the <S> elements of
    the array are laid out in order, according to rule (9).
*/

static bool isPackingVec4Padded(StructPackingPass::PackingRules rules) {
  switch (rules) {
    case StructPackingPass::PackingRules::Std140:
    case StructPackingPass::PackingRules::Std140EnhancedLayout:
    case StructPackingPass::PackingRules::HlslCbuffer:
    case StructPackingPass::PackingRules::HlslCbufferPackOffset:
      return true;
    default:
      return false;
  }
}

static bool isPackingScalar(StructPackingPass::PackingRules rules) {
  switch (rules) {
    case StructPackingPass::PackingRules::Scalar:
    case StructPackingPass::PackingRules::ScalarEnhancedLayout:
      return true;
    default:
      return false;
  }
}

static bool isPackingHlsl(StructPackingPass::PackingRules rules) {
  switch (rules) {
    case StructPackingPass::PackingRules::HlslCbuffer:
    case StructPackingPass::PackingRules::HlslCbufferPackOffset:
      return true;
    default:
      return false;
  }
}

static uint32_t getPackedBaseSize(const analysis::Type& type) {
  switch (type.kind()) {
    case analysis::Type::kBool:
      return 1;
    case analysis::Type::kInteger:
      return type.AsInteger()->width() / 8;
    case analysis::Type::kFloat:
      return type.AsFloat()->width() / 8;
    case analysis::Type::kVector:
      return getPackedBaseSize(*type.AsVector()->element_type());
    case analysis::Type::kMatrix:
      return getPackedBaseSize(*type.AsMatrix()->element_type());
    default:
      break;  // we only expect bool, int, float, vec, and mat here
  }
  assert(0 && "Unrecognized type to get base size");
  return 0;
}

static uint32_t getScalarElementCount(const analysis::Type& type) {
  switch (type.kind()) {
    case analysis::Type::kVector:
      return type.AsVector()->element_count();
    case analysis::Type::kMatrix:
      return getScalarElementCount(*type.AsMatrix()->element_type());
    case analysis::Type::kStruct:
      assert(0 && "getScalarElementCount() does not recognized struct types");
      return 0;
    default:
      return 1;
  }
}

// Aligns the specified value to a multiple of alignment, whereas the
// alignment must be a power-of-two.
static uint32_t alignPow2(uint32_t value, uint32_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

void StructPackingPass::buildConstantsMap() {
  constantsMap_.clear();
  for (Instruction* instr : context()->module()->GetConstants()) {
    constantsMap_[instr->result_id()] = instr;
  }
}

uint32_t StructPackingPass::getPackedAlignment(
    const analysis::Type& type) const {
  switch (type.kind()) {
    case analysis::Type::kArray: {
      // Get alignment of base type and round up to minimum alignment
      const uint32_t minAlignment = isPackingVec4Padded(packingRules_) ? 16 : 1;
      return std::max<uint32_t>(
          minAlignment, getPackedAlignment(*type.AsArray()->element_type()));
    }
    case analysis::Type::kStruct: {
      // Rule 9. Struct alignment is maximum alignmnet of its members
      uint32_t alignment = 1;

      for (const analysis::Type* elementType :
           type.AsStruct()->element_types()) {
        alignment =
            std::max<uint32_t>(alignment, getPackedAlignment(*elementType));
      }

      if (isPackingVec4Padded(packingRules_))
        alignment = std::max<uint32_t>(alignment, 16u);

      return alignment;
    }
    default: {
      const uint32_t baseAlignment = getPackedBaseSize(type);

      // Scalar block layout always uses alignment for the most basic component
      if (isPackingScalar(packingRules_)) return baseAlignment;

      if (const analysis::Matrix* matrixType = type.AsMatrix()) {
        // Rule 5/7
        if (isPackingVec4Padded(packingRules_) ||
            matrixType->element_count() == 3)
          return baseAlignment * 4;
        else
          return baseAlignment * matrixType->element_count();
      } else if (const analysis::Vector* vectorType = type.AsVector()) {
        // Rule 1
        if (vectorType->element_count() == 1) return baseAlignment;

        // Rule 2
        if (vectorType->element_count() == 2 ||
            vectorType->element_count() == 4)
          return baseAlignment * vectorType->element_count();

        // Rule 3
        if (vectorType->element_count() == 3) return baseAlignment * 4;
      } else {
        // Rule 1
        return baseAlignment;
      }
    }
  }
  assert(0 && "Unrecognized type to get packed alignment");
  return 0;
}

static uint32_t getPadAlignment(const analysis::Type& type,
                                uint32_t packedAlignment) {
  // The next member following a struct member is aligned to the base alignment
  // of a previous struct member.
  return type.kind() == analysis::Type::kStruct ? packedAlignment : 1;
}

uint32_t StructPackingPass::getPackedSize(const analysis::Type& type) const {
  switch (type.kind()) {
    case analysis::Type::kArray: {
      if (const analysis::Array* arrayType = type.AsArray()) {
        uint32_t size =
            getPackedArrayStride(*arrayType) * getArrayLength(*arrayType);

        // For arrays of vector and matrices in HLSL, the last element has a
        // size depending on its vector/matrix size to allow packing other
        // vectors in the last element.
        const analysis::Type* arraySubType = arrayType->element_type();
        if (isPackingHlsl(packingRules_) &&
            arraySubType->kind() != analysis::Type::kStruct) {
          size -= (4 - getScalarElementCount(*arraySubType)) *
                  getPackedBaseSize(*arraySubType);
        }
        return size;
      }
      break;
    }
    case analysis::Type::kStruct: {
      uint32_t size = 0;
      uint32_t padAlignment = 1;
      for (const analysis::Type* memberType :
           type.AsStruct()->element_types()) {
        const uint32_t packedAlignment = getPackedAlignment(*memberType);
        const uint32_t alignment =
            std::max<uint32_t>(packedAlignment, padAlignment);
        padAlignment = getPadAlignment(*memberType, packedAlignment);
        size = alignPow2(size, alignment);
        size += getPackedSize(*memberType);
      }
      return size;
    }
    default: {
      const uint32_t baseAlignment = getPackedBaseSize(type);
      if (isPackingScalar(packingRules_)) {
        return getScalarElementCount(type) * baseAlignment;
      } else {
        uint32_t size = 0;
        if (const analysis::Matrix* matrixType = type.AsMatrix()) {
          const analysis::Vector* matrixSubType =
              matrixType->element_type()->AsVector();
          assert(matrixSubType != nullptr &&
                 "Matrix sub-type is expected to be a vector type");
          if (isPackingVec4Padded(packingRules_) ||
              matrixType->element_count() == 3)
            size = matrixSubType->element_count() * baseAlignment * 4;
          else
            size = matrixSubType->element_count() * baseAlignment *
                   matrixType->element_count();

          // For matrices in HLSL, the last element has a size depending on its
          // vector size to allow packing other vectors in the last element.
          if (isPackingHlsl(packingRules_)) {
            size -= (4 - matrixSubType->element_count()) *
                    getPackedBaseSize(*matrixSubType);
          }
        } else if (const analysis::Vector* vectorType = type.AsVector()) {
          size = vectorType->element_count() * baseAlignment;
        } else {
          size = baseAlignment;
        }
        return size;
      }
    }
  }
  assert(0 && "Unrecognized type to get packed size");
  return 0;
}

uint32_t StructPackingPass::getPackedArrayStride(
    const analysis::Array& arrayType) const {
  // Array stride is equal to aligned size of element type
  const uint32_t elementSize = getPackedSize(*arrayType.element_type());
  const uint32_t alignment = getPackedAlignment(arrayType);
  return alignPow2(elementSize, alignment);
}

uint32_t StructPackingPass::getArrayLength(
    const analysis::Array& arrayType) const {
  return getConstantInt(arrayType.LengthId());
}

uint32_t StructPackingPass::getConstantInt(spv::Id id) const {
  auto it = constantsMap_.find(id);
  assert(it != constantsMap_.end() &&
         "Failed to map SPIR-V instruction ID to constant value");
  [[maybe_unused]] const analysis::Type* constType =
      context()->get_type_mgr()->GetType(it->second->type_id());
  assert(constType != nullptr &&
         "Failed to map SPIR-V instruction result type to definition");
  assert(constType->kind() == analysis::Type::kInteger &&
         "Failed to map SPIR-V instruction result type to integer type");
  return it->second->GetOperand(2).words[0];
}

StructPackingPass::PackingRules StructPackingPass::ParsePackingRuleFromString(
    const std::string& s) {
  if (s == "std140") return PackingRules::Std140;
  if (s == "std140EnhancedLayout") return PackingRules::Std140EnhancedLayout;
  if (s == "std430") return PackingRules::Std430;
  if (s == "std430EnhancedLayout") return PackingRules::Std430EnhancedLayout;
  if (s == "hlslCbuffer") return PackingRules::HlslCbuffer;
  if (s == "hlslCbufferPackOffset") return PackingRules::HlslCbufferPackOffset;
  if (s == "scalar") return PackingRules::Scalar;
  if (s == "scalarEnhancedLayout") return PackingRules::ScalarEnhancedLayout;
  return PackingRules::Undefined;
}

StructPackingPass::StructPackingPass(const char* structToPack,
                                     PackingRules rules)
    : structToPack_{structToPack != nullptr ? structToPack : ""},
      packingRules_{rules} {}

Pass::Status StructPackingPass::Process() {
  if (packingRules_ == PackingRules::Undefined) {
    if (consumer()) {
      consumer()(SPV_MSG_ERROR, "", {0, 0, 0},
                 "Cannot pack struct with undefined rule");
    }
    return Status::Failure;
  }

  // Build Id-to-instruction map for easier access
  buildConstantsMap();

  // Find structure of interest
  const uint32_t structIdToPack = findStructIdByName(structToPack_.c_str());

  const Instruction* structDef =
      context()->get_def_use_mgr()->GetDef(structIdToPack);
  if (structDef == nullptr || structDef->opcode() != spv::Op::OpTypeStruct) {
    if (consumer()) {
      const std::string message =
          "Failed to find struct with name " + structToPack_;
      consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, message.c_str());
    }
    return Status::Failure;
  }

  // Find all struct member types
  std::vector<const analysis::Type*> structMemberTypes =
      findStructMemberTypes(*structDef);

  return assignStructMemberOffsets(structIdToPack, structMemberTypes);
}

uint32_t StructPackingPass::findStructIdByName(const char* structName) const {
  for (Instruction& instr : context()->module()->debugs2()) {
    if (instr.opcode() == spv::Op::OpName &&
        instr.GetOperand(1).AsString() == structName) {
      return instr.GetOperand(0).AsId();
    }
  }
  return 0;
}

std::vector<const analysis::Type*> StructPackingPass::findStructMemberTypes(
    const Instruction& structDef) const {
  // Found struct type to pack, now collect all types of its members
  assert(structDef.NumOperands() > 0 &&
         "Number of operands in OpTypeStruct instruction must not be zero");
  const uint32_t numMembers = structDef.NumOperands() - 1;
  std::vector<const analysis::Type*> structMemberTypes;
  structMemberTypes.resize(numMembers);
  for (uint32_t i = 0; i < numMembers; ++i) {
    const spv::Id memberTypeId = structDef.GetOperand(1 + i).AsId();
    if (const analysis::Type* memberType =
            context()->get_type_mgr()->GetType(memberTypeId)) {
      structMemberTypes[i] = memberType;
    }
  }
  return structMemberTypes;
}

Pass::Status StructPackingPass::assignStructMemberOffsets(
    uint32_t structIdToPack,
    const std::vector<const analysis::Type*>& structMemberTypes) {
  // Returns true if the specified instruction is a OpMemberDecorate for the
  // struct we're looking for with an offset decoration
  auto isMemberOffsetDecoration =
      [structIdToPack](const Instruction& instr) -> bool {
    return instr.opcode() == spv::Op::OpMemberDecorate &&
           instr.GetOperand(0).AsId() == structIdToPack &&
           static_cast<spv::Decoration>(instr.GetOperand(2).words[0]) ==
               spv::Decoration::Offset;
  };

  bool modified = false;

  // Find and re-assign all member offset decorations
  for (auto it = context()->module()->annotation_begin(),
            itEnd = context()->module()->annotation_end();
       it != itEnd; ++it) {
    if (isMemberOffsetDecoration(*it)) {
      // Found first member decoration with offset, we expect all other
      // offsets right after the first one
      uint32_t prevMemberIndex = 0;
      uint32_t currentOffset = 0;
      uint32_t padAlignment = 1;
      do {
        const uint32_t memberIndex = it->GetOperand(1).words[0];
        if (memberIndex < prevMemberIndex) {
          // Failure: we expect all members to appear in consecutive order
          return Status::Failure;
        }

        // Apply alignment rules to current offset
        const analysis::Type& memberType = *structMemberTypes[memberIndex];
        uint32_t packedAlignment = getPackedAlignment(memberType);
        uint32_t packedSize = getPackedSize(memberType);

        if (isPackingHlsl(packingRules_)) {
          // If a member crosses vec4 boundaries, alignment is size of vec4
          if (currentOffset / 16 != (currentOffset + packedSize - 1) / 16)
            packedAlignment = std::max<uint32_t>(packedAlignment, 16u);
        }

        const uint32_t alignment =
            std::max<uint32_t>(packedAlignment, padAlignment);
        currentOffset = alignPow2(currentOffset, alignment);
        padAlignment = getPadAlignment(memberType, packedAlignment);

        // Override packed offset in instruction
        if (it->GetOperand(3).words[0] < currentOffset) {
          // Failure: packing resulted in higher offset for member than
          // previously generated
          return Status::Failure;
        }

        it->GetOperand(3).words[0] = currentOffset;
        modified = true;

        // Move to next member
        ++it;
        prevMemberIndex = memberIndex;
        currentOffset += packedSize;
      } while (it != itEnd && isMemberOffsetDecoration(*it));

      // We're done with all decorations for the struct of interest
      break;
    }
  }

  return modified ? Status::SuccessWithChange : Status::SuccessWithoutChange;
}

}  // namespace opt
}  // namespace spvtools
