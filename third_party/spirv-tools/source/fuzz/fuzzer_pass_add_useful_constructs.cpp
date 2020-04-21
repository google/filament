// Copyright (c) 2019 Google LLC
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

#include "source/fuzz/fuzzer_pass_add_useful_constructs.h"

#include "source/fuzz/transformation_add_constant_boolean.h"
#include "source/fuzz/transformation_add_constant_scalar.h"
#include "source/fuzz/transformation_add_type_boolean.h"
#include "source/fuzz/transformation_add_type_float.h"
#include "source/fuzz/transformation_add_type_int.h"
#include "source/fuzz/transformation_add_type_pointer.h"

namespace spvtools {
namespace fuzz {

FuzzerPassAddUsefulConstructs::FuzzerPassAddUsefulConstructs(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations) {}

FuzzerPassAddUsefulConstructs::~FuzzerPassAddUsefulConstructs() = default;

void FuzzerPassAddUsefulConstructs::MaybeAddIntConstant(
    uint32_t width, bool is_signed, std::vector<uint32_t> data) const {
  opt::analysis::Integer temp_int_type(width, is_signed);
  assert(GetIRContext()->get_type_mgr()->GetId(&temp_int_type) &&
         "int type should already be registered.");
  auto registered_int_type = GetIRContext()
                                 ->get_type_mgr()
                                 ->GetRegisteredType(&temp_int_type)
                                 ->AsInteger();
  auto int_type_id = GetIRContext()->get_type_mgr()->GetId(registered_int_type);
  assert(int_type_id &&
         "The relevant int type should have been added to the module already.");
  opt::analysis::IntConstant int_constant(registered_int_type, data);
  if (!GetIRContext()->get_constant_mgr()->FindConstant(&int_constant)) {
    TransformationAddConstantScalar add_constant_int =
        TransformationAddConstantScalar(GetFuzzerContext()->GetFreshId(),
                                        int_type_id, data);
    assert(add_constant_int.IsApplicable(GetIRContext(), *GetFactManager()) &&
           "Should be applicable by construction.");
    add_constant_int.Apply(GetIRContext(), GetFactManager());
    *GetTransformations()->add_transformation() = add_constant_int.ToMessage();
  }
}

void FuzzerPassAddUsefulConstructs::MaybeAddFloatConstant(
    uint32_t width, std::vector<uint32_t> data) const {
  opt::analysis::Float temp_float_type(width);
  assert(GetIRContext()->get_type_mgr()->GetId(&temp_float_type) &&
         "float type should already be registered.");
  auto registered_float_type = GetIRContext()
                                   ->get_type_mgr()
                                   ->GetRegisteredType(&temp_float_type)
                                   ->AsFloat();
  auto float_type_id =
      GetIRContext()->get_type_mgr()->GetId(registered_float_type);
  assert(
      float_type_id &&
      "The relevant float type should have been added to the module already.");
  opt::analysis::FloatConstant float_constant(registered_float_type, data);
  if (!GetIRContext()->get_constant_mgr()->FindConstant(&float_constant)) {
    TransformationAddConstantScalar add_constant_float =
        TransformationAddConstantScalar(GetFuzzerContext()->GetFreshId(),
                                        float_type_id, data);
    assert(add_constant_float.IsApplicable(GetIRContext(), *GetFactManager()) &&
           "Should be applicable by construction.");
    add_constant_float.Apply(GetIRContext(), GetFactManager());
    *GetTransformations()->add_transformation() =
        add_constant_float.ToMessage();
  }
}

void FuzzerPassAddUsefulConstructs::Apply() {
  {
    // Add boolean type if not present.
    opt::analysis::Bool temp_bool_type;
    if (!GetIRContext()->get_type_mgr()->GetId(&temp_bool_type)) {
      auto add_type_boolean =
          TransformationAddTypeBoolean(GetFuzzerContext()->GetFreshId());
      assert(add_type_boolean.IsApplicable(GetIRContext(), *GetFactManager()) &&
             "Should be applicable by construction.");
      add_type_boolean.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() =
          add_type_boolean.ToMessage();
    }
  }

  {
    // Add signed and unsigned 32-bit integer types if not present.
    for (auto is_signed : {true, false}) {
      opt::analysis::Integer temp_int_type(32, is_signed);
      if (!GetIRContext()->get_type_mgr()->GetId(&temp_int_type)) {
        TransformationAddTypeInt add_type_int = TransformationAddTypeInt(
            GetFuzzerContext()->GetFreshId(), 32, is_signed);
        assert(add_type_int.IsApplicable(GetIRContext(), *GetFactManager()) &&
               "Should be applicable by construction.");
        add_type_int.Apply(GetIRContext(), GetFactManager());
        *GetTransformations()->add_transformation() = add_type_int.ToMessage();
      }
    }
  }

  {
    // Add 32-bit float type if not present.
    opt::analysis::Float temp_float_type(32);
    if (!GetIRContext()->get_type_mgr()->GetId(&temp_float_type)) {
      TransformationAddTypeFloat add_type_float =
          TransformationAddTypeFloat(GetFuzzerContext()->GetFreshId(), 32);
      assert(add_type_float.IsApplicable(GetIRContext(), *GetFactManager()) &&
             "Should be applicable by construction.");
      add_type_float.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() = add_type_float.ToMessage();
    }
  }

  // Add boolean constants true and false if not present.
  opt::analysis::Bool temp_bool_type;
  auto bool_type = GetIRContext()
                       ->get_type_mgr()
                       ->GetRegisteredType(&temp_bool_type)
                       ->AsBool();
  for (auto boolean_value : {true, false}) {
    // Add OpConstantTrue/False if not already there.
    opt::analysis::BoolConstant bool_constant(bool_type, boolean_value);
    if (!GetIRContext()->get_constant_mgr()->FindConstant(&bool_constant)) {
      TransformationAddConstantBoolean add_constant_boolean(
          GetFuzzerContext()->GetFreshId(), boolean_value);
      assert(add_constant_boolean.IsApplicable(GetIRContext(),
                                               *GetFactManager()) &&
             "Should be applicable by construction.");
      add_constant_boolean.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() =
          add_constant_boolean.ToMessage();
    }
  }

  // Add signed and unsigned 32-bit integer constants 0 and 1 if not present.
  for (auto is_signed : {true, false}) {
    for (auto value : {0u, 1u}) {
      MaybeAddIntConstant(32, is_signed, {value});
    }
  }

  // Add 32-bit float constants 0.0 and 1.0 if not present.
  uint32_t uint_data[2];
  float float_data[2] = {0.0, 1.0};
  memcpy(uint_data, float_data, sizeof(float_data));
  for (unsigned int& datum : uint_data) {
    MaybeAddFloatConstant(32, {datum});
  }

  // For every known-to-be-constant uniform, make sure we have instructions
  // declaring:
  // - a pointer type with uniform storage class, whose pointee type is the type
  //   of the element
  // - a signed integer constant for each index required to access the element
  // - a constant for the constant value itself
  for (auto& fact_and_type_id :
       GetFactManager()->GetConstantUniformFactsAndTypes()) {
    uint32_t element_type_id = fact_and_type_id.second;
    assert(element_type_id);
    auto element_type =
        GetIRContext()->get_type_mgr()->GetType(element_type_id);
    assert(element_type &&
           "If the constant uniform fact is well-formed, the module must "
           "already have a declaration of the type for the uniform element.");
    opt::analysis::Pointer uniform_pointer(element_type,
                                           SpvStorageClassUniform);
    if (!GetIRContext()->get_type_mgr()->GetId(&uniform_pointer)) {
      auto add_pointer =
          TransformationAddTypePointer(GetFuzzerContext()->GetFreshId(),
                                       SpvStorageClassUniform, element_type_id);
      assert(add_pointer.IsApplicable(GetIRContext(), *GetFactManager()) &&
             "Should be applicable by construction.");
      add_pointer.Apply(GetIRContext(), GetFactManager());
      *GetTransformations()->add_transformation() = add_pointer.ToMessage();
    }
    std::vector<uint32_t> words;
    for (auto word : fact_and_type_id.first.constant_word()) {
      words.push_back(word);
    }
    // We get the element type again as the type manager may have been
    // invalidated since we last retrieved it.
    element_type = GetIRContext()->get_type_mgr()->GetType(element_type_id);
    if (element_type->AsInteger()) {
      MaybeAddIntConstant(element_type->AsInteger()->width(),
                          element_type->AsInteger()->IsSigned(), words);
    } else {
      assert(element_type->AsFloat() &&
             "Known uniform values must be integer or floating-point.");
      MaybeAddFloatConstant(element_type->AsFloat()->width(), words);
    }
    for (auto index :
         fact_and_type_id.first.uniform_buffer_element_descriptor().index()) {
      MaybeAddIntConstant(32, true, {index});
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
