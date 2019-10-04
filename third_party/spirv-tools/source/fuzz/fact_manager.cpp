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

#include "source/fuzz/fact_manager.h"

#include <sstream>

#include "source/fuzz/uniform_buffer_element_descriptor.h"
#include "source/opt/ir_context.h"

namespace spvtools {
namespace fuzz {

namespace {

std::string ToString(const protobufs::Fact& fact) {
  assert(fact.fact_case() == protobufs::Fact::kConstantUniformFact &&
         "Right now this is the only fact.");
  std::stringstream stream;
  stream << "("
         << fact.constant_uniform_fact()
                .uniform_buffer_element_descriptor()
                .descriptor_set()
         << ", "
         << fact.constant_uniform_fact()
                .uniform_buffer_element_descriptor()
                .binding()
         << ")[";

  bool first = true;
  for (auto index : fact.constant_uniform_fact()
                        .uniform_buffer_element_descriptor()
                        .index()) {
    if (first) {
      first = false;
    } else {
      stream << ", ";
    }
    stream << index;
  }

  stream << "] == [";

  first = true;
  for (auto constant_word : fact.constant_uniform_fact().constant_word()) {
    if (first) {
      first = false;
    } else {
      stream << ", ";
    }
    stream << constant_word;
  }

  stream << "]";
  return stream.str();
}

}  // namespace

//=======================
// Constant uniform facts

// The purpose of this struct is to group the fields and data used to represent
// facts about uniform constants.
struct FactManager::ConstantUniformFacts {
  // See method in FactManager which delegates to this method.
  bool AddFact(const protobufs::FactConstantUniform& fact,
               opt::IRContext* context);

  // See method in FactManager which delegates to this method.
  std::vector<uint32_t> GetConstantsAvailableFromUniformsForType(
      opt::IRContext* ir_context, uint32_t type_id) const;

  // See method in FactManager which delegates to this method.
  const std::vector<protobufs::UniformBufferElementDescriptor>
  GetUniformDescriptorsForConstant(opt::IRContext* ir_context,
                                   uint32_t constant_id) const;

  // See method in FactManager which delegates to this method.
  uint32_t GetConstantFromUniformDescriptor(
      opt::IRContext* context,
      const protobufs::UniformBufferElementDescriptor& uniform_descriptor)
      const;

  // See method in FactManager which delegates to this method.
  std::vector<uint32_t> GetTypesForWhichUniformValuesAreKnown() const;

  // Returns true if and only if the words associated with
  // |constant_instruction| exactly match the words for the constant associated
  // with |constant_uniform_fact|.
  bool DataMatches(
      const opt::Instruction& constant_instruction,
      const protobufs::FactConstantUniform& constant_uniform_fact) const;

  // Yields the constant words associated with |constant_uniform_fact|.
  std::vector<uint32_t> GetConstantWords(
      const protobufs::FactConstantUniform& constant_uniform_fact) const;

  // Yields the id of a constant of type |type_id| whose data matches the
  // constant data in |constant_uniform_fact|, or 0 if no such constant is
  // declared.
  uint32_t GetConstantId(
      opt::IRContext* context,
      const protobufs::FactConstantUniform& constant_uniform_fact,
      uint32_t type_id) const;

  // Checks that the width of a floating-point constant is supported, and that
  // the constant is finite.
  bool FloatingPointValueIsSuitable(const protobufs::FactConstantUniform& fact,
                                    uint32_t width) const;

  std::vector<std::pair<protobufs::FactConstantUniform, uint32_t>>
      facts_and_type_ids;
};

uint32_t FactManager::ConstantUniformFacts::GetConstantId(
    opt::IRContext* context,
    const protobufs::FactConstantUniform& constant_uniform_fact,
    uint32_t type_id) const {
  auto type = context->get_type_mgr()->GetType(type_id);
  assert(type != nullptr && "Unknown type id.");
  auto constant = context->get_constant_mgr()->GetConstant(
      type, GetConstantWords(constant_uniform_fact));
  return context->get_constant_mgr()->FindDeclaredConstant(constant, type_id);
}

std::vector<uint32_t> FactManager::ConstantUniformFacts::GetConstantWords(
    const protobufs::FactConstantUniform& constant_uniform_fact) const {
  std::vector<uint32_t> result;
  for (auto constant_word : constant_uniform_fact.constant_word()) {
    result.push_back(constant_word);
  }
  return result;
}

bool FactManager::ConstantUniformFacts::DataMatches(
    const opt::Instruction& constant_instruction,
    const protobufs::FactConstantUniform& constant_uniform_fact) const {
  assert(constant_instruction.opcode() == SpvOpConstant);
  std::vector<uint32_t> data_in_constant;
  for (uint32_t i = 0; i < constant_instruction.NumInOperands(); i++) {
    data_in_constant.push_back(constant_instruction.GetSingleWordInOperand(i));
  }
  return data_in_constant == GetConstantWords(constant_uniform_fact);
}

std::vector<uint32_t>
FactManager::ConstantUniformFacts::GetConstantsAvailableFromUniformsForType(
    opt::IRContext* ir_context, uint32_t type_id) const {
  std::vector<uint32_t> result;
  std::set<uint32_t> already_seen;
  for (auto& fact_and_type_id : facts_and_type_ids) {
    if (fact_and_type_id.second != type_id) {
      continue;
    }
    if (auto constant_id =
            GetConstantId(ir_context, fact_and_type_id.first, type_id)) {
      if (already_seen.find(constant_id) == already_seen.end()) {
        result.push_back(constant_id);
        already_seen.insert(constant_id);
      }
    }
  }
  return result;
}

const std::vector<protobufs::UniformBufferElementDescriptor>
FactManager::ConstantUniformFacts::GetUniformDescriptorsForConstant(
    opt::IRContext* ir_context, uint32_t constant_id) const {
  std::vector<protobufs::UniformBufferElementDescriptor> result;
  auto constant_inst = ir_context->get_def_use_mgr()->GetDef(constant_id);
  assert(constant_inst->opcode() == SpvOpConstant &&
         "The given id must be that of a constant");
  auto type_id = constant_inst->type_id();
  for (auto& fact_and_type_id : facts_and_type_ids) {
    if (fact_and_type_id.second != type_id) {
      continue;
    }
    if (DataMatches(*constant_inst, fact_and_type_id.first)) {
      result.emplace_back(
          fact_and_type_id.first.uniform_buffer_element_descriptor());
    }
  }
  return result;
}

uint32_t FactManager::ConstantUniformFacts::GetConstantFromUniformDescriptor(
    opt::IRContext* context,
    const protobufs::UniformBufferElementDescriptor& uniform_descriptor) const {
  // Consider each fact.
  for (auto& fact_and_type : facts_and_type_ids) {
    // Check whether the uniform descriptor associated with the fact matches
    // |uniform_descriptor|.
    if (UniformBufferElementDescriptorEquals()(
            &uniform_descriptor,
            &fact_and_type.first.uniform_buffer_element_descriptor())) {
      return GetConstantId(context, fact_and_type.first, fact_and_type.second);
    }
  }
  // No fact associated with the given uniform descriptor was found.
  return 0;
}

std::vector<uint32_t>
FactManager::ConstantUniformFacts::GetTypesForWhichUniformValuesAreKnown()
    const {
  std::vector<uint32_t> result;
  for (auto& fact_and_type : facts_and_type_ids) {
    if (std::find(result.begin(), result.end(), fact_and_type.second) ==
        result.end()) {
      result.push_back(fact_and_type.second);
    }
  }
  return result;
}

bool FactManager::ConstantUniformFacts::FloatingPointValueIsSuitable(
    const protobufs::FactConstantUniform& fact, uint32_t width) const {
  const uint32_t kFloatWidth = 32;
  const uint32_t kDoubleWidth = 64;
  if (width != kFloatWidth && width != kDoubleWidth) {
    // Only 32- and 64-bit floating-point types are handled.
    return false;
  }
  std::vector<uint32_t> words = GetConstantWords(fact);
  if (width == 32) {
    float value;
    memcpy(&value, words.data(), sizeof(float));
    if (!std::isfinite(value)) {
      return false;
    }
  } else {
    double value;
    memcpy(&value, words.data(), sizeof(double));
    if (!std::isfinite(value)) {
      return false;
    }
  }
  return true;
}

bool FactManager::ConstantUniformFacts::AddFact(
    const protobufs::FactConstantUniform& fact, opt::IRContext* context) {
  // Try to find a unique instruction that declares a variable such that the
  // variable is decorated with the descriptor set and binding associated with
  // the constant uniform fact.
  opt::Instruction* uniform_variable = FindUniformVariable(
      fact.uniform_buffer_element_descriptor(), context, true);

  if (!uniform_variable) {
    return false;
  }

  assert(SpvOpVariable == uniform_variable->opcode());
  assert(SpvStorageClassUniform == uniform_variable->GetSingleWordInOperand(0));

  auto should_be_uniform_pointer_type =
      context->get_type_mgr()->GetType(uniform_variable->type_id());
  if (!should_be_uniform_pointer_type->AsPointer()) {
    return false;
  }
  if (should_be_uniform_pointer_type->AsPointer()->storage_class() !=
      SpvStorageClassUniform) {
    return false;
  }
  auto should_be_uniform_pointer_instruction =
      context->get_def_use_mgr()->GetDef(uniform_variable->type_id());
  auto element_type =
      should_be_uniform_pointer_instruction->GetSingleWordInOperand(1);

  for (auto index : fact.uniform_buffer_element_descriptor().index()) {
    auto should_be_composite_type =
        context->get_def_use_mgr()->GetDef(element_type);
    if (SpvOpTypeStruct == should_be_composite_type->opcode()) {
      if (index >= should_be_composite_type->NumInOperands()) {
        return false;
      }
      element_type = should_be_composite_type->GetSingleWordInOperand(index);
    } else if (SpvOpTypeArray == should_be_composite_type->opcode()) {
      auto array_length_constant =
          context->get_constant_mgr()
              ->GetConstantFromInst(context->get_def_use_mgr()->GetDef(
                  should_be_composite_type->GetSingleWordInOperand(1)))
              ->AsIntConstant();
      if (array_length_constant->words().size() != 1) {
        return false;
      }
      auto array_length = array_length_constant->GetU32();
      if (index >= array_length) {
        return false;
      }
      element_type = should_be_composite_type->GetSingleWordInOperand(0);
    } else if (SpvOpTypeVector == should_be_composite_type->opcode()) {
      auto vector_length = should_be_composite_type->GetSingleWordInOperand(1);
      if (index >= vector_length) {
        return false;
      }
      element_type = should_be_composite_type->GetSingleWordInOperand(0);
    } else {
      return false;
    }
  }
  auto final_element_type = context->get_type_mgr()->GetType(element_type);
  if (!(final_element_type->AsFloat() || final_element_type->AsInteger())) {
    return false;
  }
  auto width = final_element_type->AsFloat()
                   ? final_element_type->AsFloat()->width()
                   : final_element_type->AsInteger()->width();

  if (final_element_type->AsFloat() &&
      !FloatingPointValueIsSuitable(fact, width)) {
    return false;
  }

  auto required_words = (width + 32 - 1) / 32;
  if (static_cast<uint32_t>(fact.constant_word().size()) != required_words) {
    return false;
  }
  facts_and_type_ids.emplace_back(
      std::pair<protobufs::FactConstantUniform, uint32_t>(fact, element_type));
  return true;
}

// End of uniform constant facts
//==============================

//==============================
// Id synonym facts

// The purpose of this struct is to group the fields and data used to represent
// facts about id synonyms.
struct FactManager::IdSynonymFacts {
  // See method in FactManager which delegates to this method.
  void AddFact(const protobufs::FactIdSynonym& fact);

  // A record of all the synonyms that are available.
  std::map<uint32_t, std::vector<protobufs::DataDescriptor>> synonyms;

  // The set of keys to the above map; useful if you just want to know which ids
  // have synonyms.
  std::set<uint32_t> ids_with_synonyms;
};

void FactManager::IdSynonymFacts::AddFact(
    const protobufs::FactIdSynonym& fact) {
  if (synonyms.count(fact.id()) == 0) {
    assert(ids_with_synonyms.count(fact.id()) == 0);
    ids_with_synonyms.insert(fact.id());
    synonyms[fact.id()] = std::vector<protobufs::DataDescriptor>();
  }
  assert(ids_with_synonyms.count(fact.id()) == 1);
  synonyms[fact.id()].push_back(fact.data_descriptor());
}

// End of id synonym facts
//==============================

FactManager::FactManager()
    : uniform_constant_facts_(MakeUnique<ConstantUniformFacts>()),
      id_synonym_facts_(MakeUnique<IdSynonymFacts>()) {}

FactManager::~FactManager() = default;

void FactManager::AddFacts(const MessageConsumer& message_consumer,
                           const protobufs::FactSequence& initial_facts,
                           opt::IRContext* context) {
  for (auto& fact : initial_facts.fact()) {
    if (!AddFact(fact, context)) {
      message_consumer(
          SPV_MSG_WARNING, nullptr, {},
          ("Invalid fact " + ToString(fact) + " ignored.").c_str());
    }
  }
}

bool FactManager::AddFact(const spvtools::fuzz::protobufs::Fact& fact,
                          spvtools::opt::IRContext* context) {
  switch (fact.fact_case()) {
    case protobufs::Fact::kConstantUniformFact:
      return uniform_constant_facts_->AddFact(fact.constant_uniform_fact(),
                                              context);
    case protobufs::Fact::kIdSynonymFact:
      id_synonym_facts_->AddFact(fact.id_synonym_fact());
      return true;
    default:
      assert(false && "Unknown fact type.");
      return false;
  }
}

std::vector<uint32_t> FactManager::GetConstantsAvailableFromUniformsForType(
    opt::IRContext* ir_context, uint32_t type_id) const {
  return uniform_constant_facts_->GetConstantsAvailableFromUniformsForType(
      ir_context, type_id);
}

const std::vector<protobufs::UniformBufferElementDescriptor>
FactManager::GetUniformDescriptorsForConstant(opt::IRContext* ir_context,
                                              uint32_t constant_id) const {
  return uniform_constant_facts_->GetUniformDescriptorsForConstant(ir_context,
                                                                   constant_id);
}

uint32_t FactManager::GetConstantFromUniformDescriptor(
    opt::IRContext* context,
    const protobufs::UniformBufferElementDescriptor& uniform_descriptor) const {
  return uniform_constant_facts_->GetConstantFromUniformDescriptor(
      context, uniform_descriptor);
}

std::vector<uint32_t> FactManager::GetTypesForWhichUniformValuesAreKnown()
    const {
  return uniform_constant_facts_->GetTypesForWhichUniformValuesAreKnown();
}

const std::vector<std::pair<protobufs::FactConstantUniform, uint32_t>>&
FactManager::GetConstantUniformFactsAndTypes() const {
  return uniform_constant_facts_->facts_and_type_ids;
}

const std::set<uint32_t>& FactManager::GetIdsForWhichSynonymsAreKnown() const {
  return id_synonym_facts_->ids_with_synonyms;
}

const std::vector<protobufs::DataDescriptor>& FactManager::GetSynonymsForId(
    uint32_t id) const {
  return id_synonym_facts_->synonyms.at(id);
}

}  // namespace fuzz
}  // namespace spvtools
