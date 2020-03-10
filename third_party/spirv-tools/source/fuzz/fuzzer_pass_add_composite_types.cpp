// Copyright (c) 2020 Google LLC
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

#include "source/fuzz/fuzzer_pass_add_composite_types.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/transformation_add_type_array.h"
#include "source/fuzz/transformation_add_type_struct.h"

namespace spvtools {
namespace fuzz {

FuzzerPassAddCompositeTypes::FuzzerPassAddCompositeTypes(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassAddCompositeTypes::~FuzzerPassAddCompositeTypes() = default;

void FuzzerPassAddCompositeTypes::Apply() {
  MaybeAddMissingVectorTypes();
  MaybeAddMissingMatrixTypes();

  // Randomly interleave between adding struct and array composite types
  while (GetFuzzerContext()->ChoosePercentage(
      GetFuzzerContext()->GetChanceOfAddingArrayOrStructType())) {
    if (GetFuzzerContext()->ChoosePercentage(
            GetFuzzerContext()->GetChanceOfChoosingStructTypeVsArrayType())) {
      AddNewStructType();
    } else {
      AddNewArrayType();
    }
  }
}

void FuzzerPassAddCompositeTypes::MaybeAddMissingVectorTypes() {
  // Functions to lazily supply scalar base types on demand if we decide to
  // create vectors with the relevant base types.
  std::function<uint32_t()> bool_type_supplier = [this]() -> uint32_t {
    return FindOrCreateBoolType();
  };
  std::function<uint32_t()> float_type_supplier = [this]() -> uint32_t {
    return FindOrCreate32BitFloatType();
  };
  std::function<uint32_t()> int_type_supplier = [this]() -> uint32_t {
    return FindOrCreate32BitIntegerType(true);
  };
  std::function<uint32_t()> uint_type_supplier = [this]() -> uint32_t {
    return FindOrCreate32BitIntegerType(false);
  };

  // Consider each of the base types with which we can make vectors.
  for (auto& base_type_supplier : {bool_type_supplier, float_type_supplier,
                                   int_type_supplier, uint_type_supplier}) {
    // Consider each valid vector size.
    for (uint32_t size = 2; size <= 4; size++) {
      // Randomly decide whether to create (if it does not already exist) a
      // vector with this size and base type.
      if (GetFuzzerContext()->ChoosePercentage(
              GetFuzzerContext()->GetChanceOfAddingVectorType())) {
        FindOrCreateVectorType(base_type_supplier(), size);
      }
    }
  }
}

void FuzzerPassAddCompositeTypes::MaybeAddMissingMatrixTypes() {
  // Consider every valid matrix dimension.
  for (uint32_t columns = 2; columns <= 4; columns++) {
    for (uint32_t rows = 2; rows <= 4; rows++) {
      // Randomly decide whether to create (if it does not already exist) a
      // matrix with these dimensions.  As matrices can only have floating-point
      // base type, we do not need to consider multiple base types as in the
      // case for vectors.
      if (GetFuzzerContext()->ChoosePercentage(
              GetFuzzerContext()->GetChanceOfAddingMatrixType())) {
        FindOrCreateMatrixType(columns, rows);
      }
    }
  }
}

void FuzzerPassAddCompositeTypes::AddNewArrayType() {
  ApplyTransformation(TransformationAddTypeArray(
      GetFuzzerContext()->GetFreshId(), ChooseScalarOrCompositeType(),
      FindOrCreate32BitIntegerConstant(
          GetFuzzerContext()->GetRandomSizeForNewArray(), false)));
}

void FuzzerPassAddCompositeTypes::AddNewStructType() {
  std::vector<uint32_t> field_type_ids;
  do {
    field_type_ids.push_back(ChooseScalarOrCompositeType());
  } while (GetFuzzerContext()->ChoosePercentage(
      GetFuzzerContext()->GetChanceOfAddingAnotherStructField()));
  ApplyTransformation(TransformationAddTypeStruct(
      GetFuzzerContext()->GetFreshId(), field_type_ids));
}

uint32_t FuzzerPassAddCompositeTypes::ChooseScalarOrCompositeType() {
  // Gather up all the possibly-relevant types.
  std::vector<uint32_t> candidates;
  for (auto& inst : GetIRContext()->types_values()) {
    switch (inst.opcode()) {
      case SpvOpTypeArray:
      case SpvOpTypeBool:
      case SpvOpTypeFloat:
      case SpvOpTypeInt:
      case SpvOpTypeMatrix:
      case SpvOpTypeStruct:
      case SpvOpTypeVector:
        candidates.push_back(inst.result_id());
        break;
      default:
        break;
    }
  }
  assert(!candidates.empty() &&
         "This function should only be called if there is at least one scalar "
         "or composite type available.");
  // Return one of these types at random.
  return candidates[GetFuzzerContext()->RandomIndex(candidates)];
}

}  // namespace fuzz
}  // namespace spvtools
