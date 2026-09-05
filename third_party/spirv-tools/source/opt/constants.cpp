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

#include "source/opt/constants.h"

#include <vector>

#include "source/opt/ir_context.h"

namespace spvtools {
namespace opt {
namespace analysis {

float Constant::GetFloat() const {
  assert(type()->AsFloat() != nullptr && type()->AsFloat()->width() == 32);

  if (const FloatConstant* fc = AsFloatConstant()) {
    return fc->GetFloatValue();
  } else {
    assert(AsNullConstant() && "Must be a floating point constant.");
    return 0.0f;
  }
}

double Constant::GetDouble() const {
  assert(type()->AsFloat() != nullptr && type()->AsFloat()->width() == 64);

  if (const FloatConstant* fc = AsFloatConstant()) {
    return fc->GetDoubleValue();
  } else {
    assert(AsNullConstant() && "Must be a floating point constant.");
    return 0.0;
  }
}

double Constant::GetValueAsDouble() const {
  assert(type()->AsFloat() != nullptr);
  if (type()->AsFloat()->width() == 32) {
    return GetFloat();
  } else {
    assert(type()->AsFloat()->width() == 64);
    return GetDouble();
  }
}

uint32_t Constant::GetU32() const {
  assert(type()->AsInteger() != nullptr);
  assert(type()->AsInteger()->width() == 32);

  if (const IntConstant* ic = AsIntConstant()) {
    return ic->GetU32BitValue();
  } else {
    assert(AsNullConstant() && "Must be an integer constant.");
    return 0u;
  }
}

uint64_t Constant::GetU64() const {
  assert(type()->AsInteger() != nullptr);
  assert(type()->AsInteger()->width() == 64);

  if (const IntConstant* ic = AsIntConstant()) {
    return ic->GetU64BitValue();
  } else {
    assert(AsNullConstant() && "Must be an integer constant.");
    return 0u;
  }
}

int32_t Constant::GetS32() const {
  assert(type()->AsInteger() != nullptr);
  assert(type()->AsInteger()->width() == 32);

  if (const IntConstant* ic = AsIntConstant()) {
    return ic->GetS32BitValue();
  } else {
    assert(AsNullConstant() && "Must be an integer constant.");
    return 0;
  }
}

int64_t Constant::GetS64() const {
  assert(type()->AsInteger() != nullptr);
  assert(type()->AsInteger()->width() == 64);

  if (const IntConstant* ic = AsIntConstant()) {
    return ic->GetS64BitValue();
  } else {
    assert(AsNullConstant() && "Must be an integer constant.");
    return 0;
  }
}

uint64_t Constant::GetZeroExtendedValue() const {
  const auto* int_type = type()->AsInteger();
  assert(int_type != nullptr);
  const auto width = int_type->width();
  assert(width <= 64);

  uint64_t value = 0;
  if (const IntConstant* ic = AsIntConstant()) {
    if (width <= 32) {
      value = ic->GetU32BitValue();
    } else {
      value = ic->GetU64BitValue();
    }
  } else {
    assert(AsNullConstant() && "Must be an integer constant.");
  }
  return value;
}

int64_t Constant::GetSignExtendedValue() const {
  const auto* int_type = type()->AsInteger();
  assert(int_type != nullptr);
  const auto width = int_type->width();
  assert(width <= 64);

  int64_t value = 0;
  if (const IntConstant* ic = AsIntConstant()) {
    if (width <= 32) {
      // Let the C++ compiler do the sign extension.
      value = int64_t(ic->GetS32BitValue());
    } else {
      value = ic->GetS64BitValue();
    }
  } else {
    assert(AsNullConstant() && "Must be an integer constant.");
  }
  return value;
}

ConstantManager::ConstantManager(IRContext* ctx) : ctx_(ctx) {
  // Populate the constant table with values from constant declarations in the
  // module.  The values of each OpConstant declaration is the identity
  // assignment (i.e., each constant is its own value).
  for (const auto& inst : ctx_->module()->GetConstants()) {
    MapInst(inst);
  }
}

Type* ConstantManager::GetType(const Instruction* inst) const {
  return context()->get_type_mgr()->GetType(inst->type_id());
}

std::vector<const Constant*> ConstantManager::GetOperandConstants(
    const Instruction* inst) const {
  std::vector<const Constant*> constants;
  constants.reserve(inst->NumInOperands());
  for (uint32_t i = 0; i < inst->NumInOperands(); i++) {
    const Operand* operand = &inst->GetInOperand(i);
    if (operand->type != SPV_OPERAND_TYPE_ID) {
      constants.push_back(nullptr);
    } else {
      uint32_t id = operand->words[0];
      const analysis::Constant* constant = FindDeclaredConstant(id);
      constants.push_back(constant);
    }
  }
  return constants;
}

uint32_t ConstantManager::FindDeclaredConstant(const Constant* c,
                                               uint32_t type_id) const {
  c = FindConstant(c);
  if (c == nullptr) {
    return 0;
  }

  for (auto range = const_val_to_id_.equal_range(c);
       range.first != range.second; ++range.first) {
    Instruction* const_def =
        context()->get_def_use_mgr()->GetDef(range.first->second);
    if (type_id == 0 || const_def->type_id() == type_id) {
      return range.first->second;
    }
  }
  return 0;
}

std::vector<const Constant*> ConstantManager::GetConstantsFromIds(
    const std::vector<uint32_t>& ids) const {
  std::vector<const Constant*> constants;
  for (uint32_t id : ids) {
    if (const Constant* c = FindDeclaredConstant(id)) {
      constants.push_back(c);
    } else {
      return {};
    }
  }
  return constants;
}

Instruction* ConstantManager::BuildInstructionAndAddToModule(
    const Constant* new_const, Module::inst_iterator* pos, uint32_t type_id) {
  // TODO(1841): Handle id overflow.
  uint32_t new_id = context()->TakeNextId();
  if (new_id == 0) {
    return nullptr;
  }

  auto new_inst = CreateInstruction(new_id, new_const, type_id);
  if (!new_inst) {
    return nullptr;
  }
  auto* new_inst_ptr = new_inst.get();
  *pos = pos->InsertBefore(std::move(new_inst));
  ++(*pos);
  if (context()->AreAnalysesValid(IRContext::Analysis::kAnalysisDefUse))
    context()->get_def_use_mgr()->AnalyzeInstDefUse(new_inst_ptr);
  MapConstantToInst(new_const, new_inst_ptr);
  return new_inst_ptr;
}

Instruction* ConstantManager::GetDefiningInstruction(
    const Constant* c, uint32_t type_id, Module::inst_iterator* pos) {
  uint32_t decl_id = FindDeclaredConstant(c, type_id);
  if (decl_id == 0) {
    auto iter = context()->types_values_end();
    if (pos == nullptr) pos = &iter;
    return BuildInstructionAndAddToModule(c, pos, type_id);
  } else {
    auto def = context()->get_def_use_mgr()->GetDef(decl_id);
    assert(def != nullptr);
    assert((type_id == 0 || def->type_id() == type_id) &&
           "This constant already has an instruction with a different type.");
    return def;
  }
}

std::unique_ptr<Constant> ConstantManager::CreateConstant(
    const Type* type, const std::vector<uint32_t>& literal_words_or_ids) const {
  if (literal_words_or_ids.size() == 0) {
    // Constant declared with OpConstantNull
    return MakeUnique<NullConstant>(type);
  } else if (auto* bt = type->AsBool()) {
    assert(literal_words_or_ids.size() == 1 &&
           "Bool constant should be declared with one operand");
    return MakeUnique<BoolConstant>(bt, literal_words_or_ids.front());
  } else if (auto* it = type->AsInteger()) {
    return MakeUnique<IntConstant>(it, literal_words_or_ids);
  } else if (auto* ft = type->AsFloat()) {
    return MakeUnique<FloatConstant>(ft, literal_words_or_ids);
  } else if (auto* vt = type->AsVector()) {
    auto components = GetConstantsFromIds(literal_words_or_ids);
    if (components.empty()) return nullptr;
    // All components of VectorConstant must be of type Bool, Integer or Float.
    if (!std::all_of(components.begin(), components.end(),
                     [](const Constant* c) {
                       if (c->type()->AsBool() || c->type()->AsInteger() ||
                           c->type()->AsFloat()) {
                         return true;
                       } else {
                         return false;
                       }
                     }))
      return nullptr;
    // All components of VectorConstant must be in the same type.
    const auto* component_type = components.front()->type();
    if (!std::all_of(components.begin(), components.end(),
                     [&component_type](const Constant* c) {
                       if (c->type() == component_type) return true;
                       return false;
                     }))
      return nullptr;
    return MakeUnique<VectorConstant>(vt, components);
  } else if (auto* mt = type->AsMatrix()) {
    auto components = GetConstantsFromIds(literal_words_or_ids);
    if (components.empty()) return nullptr;
    return MakeUnique<MatrixConstant>(mt, components);
  } else if (auto* st = type->AsStruct()) {
    auto components = GetConstantsFromIds(literal_words_or_ids);
    if (components.empty()) return nullptr;
    return MakeUnique<StructConstant>(st, components);
  } else if (auto* at = type->AsArray()) {
    auto components = GetConstantsFromIds(literal_words_or_ids);
    if (components.empty()) return nullptr;
    return MakeUnique<ArrayConstant>(at, components);
  } else if (auto* tt = type->AsTensorARM()) {
    auto components = GetConstantsFromIds(literal_words_or_ids);
    if (components.empty()) return nullptr;
    return MakeUnique<TensorConstant>(tt, components);
  } else {
    return nullptr;
  }
}

const Constant* ConstantManager::GetConstantFromInst(const Instruction* inst) {
  std::vector<uint32_t> literal_words_or_ids;

  // Collect the constant defining literals or component ids.
  for (uint32_t i = 0; i < inst->NumInOperands(); i++) {
    literal_words_or_ids.insert(literal_words_or_ids.end(),
                                inst->GetInOperand(i).words.begin(),
                                inst->GetInOperand(i).words.end());
  }

  const Type* type = GetType(inst);

  switch (inst->opcode()) {
    // OpConstant{True|False} have the value embedded in the opcode. So they
    // are not handled by the for-loop above. Here we add the value explicitly.
    case spv::Op::OpConstantTrue:
      literal_words_or_ids.push_back(true);
      break;
    case spv::Op::OpConstantFalse:
      literal_words_or_ids.push_back(false);
      break;
    case spv::Op::OpConstantNull:
    case spv::Op::OpConstant:
    case spv::Op::OpConstantComposite:
    case spv::Op::OpSpecConstantComposite:
      break;
    // Replicated composite constant instructions have a single operand for the
    // value. We need to replicate it as many times as there are components.
    case spv::Op::OpConstantCompositeReplicateEXT:
    case spv::Op::OpSpecConstantCompositeReplicateEXT: {
      uint32_t value = literal_words_or_ids[0];
      uint64_t component_count = type->NumberOfComponents();
      if (const auto* tensor_type = type->AsTensorARM()) {
        // TensorARM shape is stored as an ID, so resolve the outer
        // dimension here instead of using Type::NumberOfComponents().
        if (!tensor_type->is_shaped()) {
          return nullptr;
        }
        const auto* shape_inst =
            context()->get_def_use_mgr()->GetDef(tensor_type->shape_id());
        if (!shape_inst) {
          return nullptr;
        }
        const auto* shape_cst = GetConstantFromInst(shape_inst);
        if (!shape_cst || !shape_cst->AsArrayConstant()) {
          return nullptr;
        }
        const auto& shape_components =
            shape_cst->AsArrayConstant()->GetComponents();
        if (shape_components.empty()) {
          return nullptr;
        }
        const auto* outer_dim_cst = shape_components.front()->AsIntConstant();
        if (!outer_dim_cst) {
          return nullptr;
        }
        component_count = outer_dim_cst->GetZeroExtendedValue();
      }
      literal_words_or_ids.assign(static_cast<size_t>(component_count), value);
      break;
    }
    default:
      return nullptr;
  }

  return GetConstant(type, literal_words_or_ids);
}

std::unique_ptr<Instruction> ConstantManager::CreateInstruction(
    uint32_t id, const Constant* c, uint32_t type_id) const {
  uint32_t type =
      (type_id == 0) ? context()->get_type_mgr()->GetId(c->type()) : type_id;
  if (c->AsNullConstant()) {
    return MakeUnique<Instruction>(context(), spv::Op::OpConstantNull, type, id,
                                   std::initializer_list<Operand>{});
  } else if (const BoolConstant* bc = c->AsBoolConstant()) {
    return MakeUnique<Instruction>(
        context(),
        bc->value() ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse, type,
        id, std::initializer_list<Operand>{});
  } else if (const IntConstant* ic = c->AsIntConstant()) {
    return MakeUnique<Instruction>(
        context(), spv::Op::OpConstant, type, id,
        std::initializer_list<Operand>{
            Operand(spv_operand_type_t::SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER,
                    ic->words())});
  } else if (const FloatConstant* fc = c->AsFloatConstant()) {
    return MakeUnique<Instruction>(
        context(), spv::Op::OpConstant, type, id,
        std::initializer_list<Operand>{
            Operand(spv_operand_type_t::SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER,
                    fc->words())});
  } else if (const CompositeConstant* cc = c->AsCompositeConstant()) {
    return CreateCompositeInstruction(id, cc, type_id);
  } else {
    return nullptr;
  }
}

std::unique_ptr<Instruction> ConstantManager::CreateCompositeInstruction(
    uint32_t result_id, const CompositeConstant* cc, uint32_t type_id) const {
  std::vector<Operand> operands;
  Instruction* type_inst = context()->get_def_use_mgr()->GetDef(type_id);
  uint32_t component_index = 0;
  for (const Constant* component_const : cc->GetComponents()) {
    uint32_t component_type_id = 0;
    if (type_inst && type_inst->opcode() == spv::Op::OpTypeStruct) {
      component_type_id = type_inst->GetSingleWordInOperand(component_index);
    } else if (type_inst && type_inst->opcode() == spv::Op::OpTypeArray) {
      component_type_id = type_inst->GetSingleWordInOperand(0);
    } else if (type_inst && type_inst->opcode() == spv::Op::OpTypeTensorARM) {
      component_type_id =
          context()->get_type_mgr()->GetId(component_const->type());
      if (component_type_id == 0) {
        return nullptr;
      }
    }
    uint32_t id = FindDeclaredConstant(component_const, component_type_id);

    if (id == 0) {
      // Cannot get the id of the component constant, while all components
      // should have been added to the module prior to the composite constant.
      // Cannot create OpConstantComposite instruction in this case.
      return nullptr;
    }
    operands.emplace_back(spv_operand_type_t::SPV_OPERAND_TYPE_ID,
                          std::initializer_list<uint32_t>{id});
    component_index++;
  }
  uint32_t type =
      (type_id == 0) ? context()->get_type_mgr()->GetId(cc->type()) : type_id;
  return MakeUnique<Instruction>(context(), spv::Op::OpConstantComposite, type,
                                 result_id, std::move(operands));
}

const Constant* ConstantManager::GetConstant(
    const Type* type, const std::vector<uint32_t>& literal_words_or_ids) {
  auto cst = CreateConstant(type, literal_words_or_ids);
  return cst ? RegisterConstant(std::move(cst)) : nullptr;
}

const Constant* ConstantManager::GetNullCompositeConstant(const Type* type) {
  std::vector<uint32_t> literal_words_or_id;

  if (type->AsVector()) {
    const Type* element_type = type->AsVector()->element_type();
    const uint32_t null_id = GetNullConstId(element_type);
    const uint32_t element_count = type->AsVector()->element_count();
    for (uint32_t i = 0; i < element_count; i++) {
      literal_words_or_id.push_back(null_id);
    }
  } else if (type->AsMatrix()) {
    const Type* element_type = type->AsMatrix()->element_type();
    const uint32_t null_id = GetNullConstId(element_type);
    const uint32_t element_count = type->AsMatrix()->element_count();
    for (uint32_t i = 0; i < element_count; i++) {
      literal_words_or_id.push_back(null_id);
    }
  } else if (type->AsStruct()) {
    // TODO (sfricke-lunarg) add proper struct support
    return nullptr;
  } else if (type->AsArray()) {
    const Type* element_type = type->AsArray()->element_type();
    const uint32_t null_id = GetNullConstId(element_type);
    assert(type->AsArray()->length_info().words[0] ==
               analysis::Array::LengthInfo::kConstant &&
           "unexpected array length");
    const uint32_t element_count = type->AsArray()->length_info().words[0];
    for (uint32_t i = 0; i < element_count; i++) {
      literal_words_or_id.push_back(null_id);
    }
  } else if (type->AsTensorARM()) {
    auto ttype = type->AsTensorARM();
    assert(ttype->is_shaped() && "Only shaped tensors are composites");
    const auto* shape_inst =
        context()->get_def_use_mgr()->GetDef(ttype->shape_id());
    if (!shape_inst) {
      return nullptr;
    }
    const auto* shape_cst = GetConstantFromInst(shape_inst);
    if (!shape_cst || !shape_cst->AsArrayConstant()) {
      return nullptr;
    }
    const auto& shape_components =
        shape_cst->AsArrayConstant()->GetComponents();
    if (shape_components.empty()) {
      return nullptr;
    }
    const auto* outer_dim_cst = shape_components.front()->AsIntConstant();
    if (!outer_dim_cst) {
      return nullptr;
    }
    const uint64_t element_count = outer_dim_cst->GetZeroExtendedValue();
    std::vector<const Constant*> components;
    components.reserve(static_cast<size_t>(element_count));

    if (shape_components.size() == 1) {
      const Constant* element_null = GetConstant(ttype->element_type(), {});
      for (uint64_t i = 0; i < element_count; i++) {
        components.push_back(element_null);
      }
      return RegisterConstant(
          MakeUnique<TensorConstant>(ttype, std::move(components)));
    }

    const Constant* rank_cst = nullptr;
    if (ttype->rank_id() != 0) {
      const auto* rank_inst =
          context()->get_def_use_mgr()->GetDef(ttype->rank_id());
      if (rank_inst) {
        rank_cst = GetConstantFromInst(rank_inst);
      }
    }
    uint64_t rank_value = shape_components.size();
    if (rank_cst && rank_cst->AsIntConstant()) {
      rank_value = rank_cst->AsIntConstant()->GetZeroExtendedValue();
    }
    if (rank_value <= 1) {
      const Constant* element_null = GetConstant(ttype->element_type(), {});
      for (uint64_t i = 0; i < element_count; i++) {
        components.push_back(element_null);
      }
      return RegisterConstant(
          MakeUnique<TensorConstant>(ttype, std::move(components)));
    }

    const uint64_t inner_rank = rank_value - 1;
    const auto* rank_int_type =
        rank_cst ? rank_cst->type()->AsInteger() : nullptr;
    if (!rank_int_type) {
      rank_int_type = outer_dim_cst->type()->AsInteger();
    }
    if (!rank_int_type) {
      return nullptr;
    }
    const Constant* inner_rank_cst =
        GenerateIntegerConstant(rank_int_type, inner_rank);
    const uint32_t inner_rank_id =
        GetDefiningInstruction(inner_rank_cst)->result_id();

    const Type* shape_elem_type = shape_components[1]->type();
    const auto* shape_elem_int = shape_elem_type->AsInteger();
    if (!shape_elem_int) {
      return nullptr;
    }
    const Constant* inner_shape_len_cst =
        GenerateIntegerConstant(shape_elem_int, inner_rank);
    const uint32_t inner_shape_len_id =
        GetDefiningInstruction(inner_shape_len_cst)->result_id();
    Array::LengthInfo inner_shape_len_info{
        inner_shape_len_id,
        {Array::LengthInfo::kConstant, static_cast<uint32_t>(inner_rank)}};
    Array inner_shape_type(shape_elem_type, inner_shape_len_info);
    const Type* inner_shape_reg_type =
        context()->get_type_mgr()->GetRegisteredType(&inner_shape_type);
    if (!inner_shape_reg_type) {
      return nullptr;
    }
    std::vector<uint32_t> inner_shape_ids;
    inner_shape_ids.reserve(shape_components.size() - 1);
    for (size_t i = 1; i < shape_components.size(); ++i) {
      inner_shape_ids.push_back(
          GetDefiningInstruction(shape_components[i])->result_id());
    }
    const Constant* inner_shape_cst =
        GetConstant(inner_shape_reg_type, inner_shape_ids);
    const uint32_t inner_shape_id =
        GetDefiningInstruction(inner_shape_cst)->result_id();

    TensorARM inner_tensor_type(ttype->element_type(), inner_rank_id,
                                inner_shape_id);
    const Type* inner_tensor_reg_type =
        context()->get_type_mgr()->GetRegisteredType(&inner_tensor_type);
    if (!inner_tensor_reg_type) {
      return nullptr;
    }
    const Constant* inner_null_tensor =
        GetNullCompositeConstant(inner_tensor_reg_type);
    if (!inner_null_tensor) {
      return nullptr;
    }
    for (uint64_t i = 0; i < element_count; i++) {
      components.push_back(inner_null_tensor);
    }
    return RegisterConstant(
        MakeUnique<TensorConstant>(ttype, std::move(components)));
  } else {
    return nullptr;
  }

  return GetConstant(type, literal_words_or_id);
}

const Constant* ConstantManager::GetNumericVectorConstantWithWords(
    const Vector* type, const std::vector<uint32_t>& literal_words) {
  const auto* element_type = type->element_type();
  uint32_t words_per_element = 0;
  if (const auto* float_type = element_type->AsFloat())
    words_per_element = float_type->width() / 32;
  else if (const auto* int_type = element_type->AsInteger())
    words_per_element = int_type->width() / 32;
  else if (element_type->AsBool() != nullptr)
    words_per_element = 1;

  if (words_per_element != 1 && words_per_element != 2) return nullptr;

  if (words_per_element * type->element_count() !=
      static_cast<uint32_t>(literal_words.size())) {
    return nullptr;
  }

  std::vector<uint32_t> element_ids;
  for (uint32_t i = 0; i < type->element_count(); ++i) {
    auto first_word = literal_words.begin() + (words_per_element * i);
    std::vector<uint32_t> const_data(first_word,
                                     first_word + words_per_element);
    const analysis::Constant* element_constant =
        GetConstant(element_type, const_data);
    auto element_id = GetDefiningInstruction(element_constant)->result_id();
    element_ids.push_back(element_id);
  }

  return GetConstant(type, element_ids);
}

uint32_t ConstantManager::GetFloatConstId(float val) {
  const Constant* c = GetFloatConst(val);
  Instruction* inst = GetDefiningInstruction(c);
  if (inst == nullptr) return 0;
  return inst->result_id();
}

const Constant* ConstantManager::GetFloatConst(float val) {
  Type* float_type = context()->get_type_mgr()->GetFloatType();
  utils::FloatProxy<float> v(val);
  const Constant* c = GetConstant(float_type, v.GetWords());
  return c;
}

uint32_t ConstantManager::GetDoubleConstId(double val) {
  const Constant* c = GetDoubleConst(val);
  Instruction* inst = GetDefiningInstruction(c);
  if (inst == nullptr) return 0;
  return inst->result_id();
}

const Constant* ConstantManager::GetDoubleConst(double val) {
  Type* float_type = context()->get_type_mgr()->GetDoubleType();
  utils::FloatProxy<double> v(val);
  const Constant* c = GetConstant(float_type, v.GetWords());
  return c;
}

uint32_t ConstantManager::GetSIntConstId(int32_t val) {
  Type* sint_type = context()->get_type_mgr()->GetSIntType();
  const Constant* c = GetConstant(sint_type, {static_cast<uint32_t>(val)});
  return GetDefiningInstruction(c)->result_id();
}

const Constant* ConstantManager::GetIntConst(uint64_t val, int32_t bitWidth,
                                             bool isSigned) {
  Type* int_type = context()->get_type_mgr()->GetIntType(bitWidth, isSigned);

  if (isSigned) {
    // Sign extend the value.
    int32_t num_of_bit_to_ignore = 64 - bitWidth;
    val = static_cast<int64_t>(val << num_of_bit_to_ignore) >>
          num_of_bit_to_ignore;
  } else if (bitWidth < 64) {
    // Clear the upper bit that are not used.
    uint64_t mask = ((1ull << bitWidth) - 1);
    val &= mask;
  }

  if (bitWidth <= 32) {
    return GetConstant(int_type, {static_cast<uint32_t>(val)});
  }

  // If the value is more than 32-bit, we need to split the operands into two
  // 32-bit integers.
  return GetConstant(
      int_type, {static_cast<uint32_t>(val), static_cast<uint32_t>(val >> 32)});
}

uint32_t ConstantManager::GetUIntConstId(uint32_t val) {
  Type* uint_type = context()->get_type_mgr()->GetUIntType();
  const Constant* c = GetConstant(uint_type, {val});
  return GetDefiningInstruction(c)->result_id();
}

uint32_t ConstantManager::GetNullConstId(const Type* type) {
  const Constant* c = GetConstant(type, {});
  return GetDefiningInstruction(c)->result_id();
}

const Constant* ConstantManager::GenerateIntegerConstant(
    const analysis::Integer* integer_type, uint64_t result) {
  assert(integer_type != nullptr);

  std::vector<uint32_t> words;
  if (integer_type->width() == 64) {
    // In the 64-bit case, two words are needed to represent the value.
    words = {static_cast<uint32_t>(result),
             static_cast<uint32_t>(result >> 32)};
  } else {
    // In all other cases, only a single word is needed.
    assert(integer_type->width() <= 32);
    if (integer_type->IsSigned()) {
      result = utils::SignExtendValue(result, integer_type->width());
    } else {
      result = utils::ZeroExtendValue(result, integer_type->width());
    }
    words = {static_cast<uint32_t>(result)};
  }
  return GetConstant(integer_type, words);
}

std::vector<const analysis::Constant*> Constant::GetVectorComponents(
    analysis::ConstantManager* const_mgr) const {
  std::vector<const analysis::Constant*> components;
  const analysis::VectorConstant* a = this->AsVectorConstant();
  const analysis::Vector* vector_type = this->type()->AsVector();
  assert(vector_type != nullptr);
  if (a != nullptr) {
    for (uint32_t i = 0; i < vector_type->element_count(); ++i) {
      components.push_back(a->GetComponents()[i]);
    }
  } else {
    const analysis::Type* element_type = vector_type->element_type();
    const analysis::Constant* element_null_const =
        const_mgr->GetConstant(element_type, {});
    for (uint32_t i = 0; i < vector_type->element_count(); ++i) {
      components.push_back(element_null_const);
    }
  }
  return components;
}

}  // namespace analysis
}  // namespace opt
}  // namespace spvtools
