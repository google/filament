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

#ifndef SOURCE_FUZZ_FACT_MANAGER_H_
#define SOURCE_FUZZ_FACT_MANAGER_H_

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/opt/constants.h"

namespace spvtools {
namespace fuzz {

// Keeps track of facts about the module being transformed on which the fuzzing
// process can depend. Some initial facts can be provided, for example about
// guarantees on the values of inputs to SPIR-V entry points. Transformations
// may then rely on these facts, can add further facts that they establish.
// Facts are intended to be simple properties that either cannot be deduced from
// the module (such as properties that are guaranteed to hold for entry point
// inputs), or that are established by transformations, likely to be useful for
// future transformations, and not completely trivial to deduce straight from
// the module.
class FactManager {
 public:
  FactManager();

  ~FactManager();

  // Adds all the facts from |facts|, checking them for validity with respect to
  // |context|.  Warnings about invalid facts are communicated via
  // |message_consumer|; such facts are otherwise ignored.
  void AddFacts(const MessageConsumer& message_consumer,
                const protobufs::FactSequence& facts, opt::IRContext* context);

  // Checks the fact for validity with respect to |context|.  Returns false,
  // with no side effects, if the fact is invalid.  Otherwise adds |fact| to the
  // fact manager.
  bool AddFact(const protobufs::Fact& fact, opt::IRContext* context);

  // The fact manager is responsible for managing a few distinct categories of
  // facts. In principle there could be different fact managers for each kind
  // of fact, but in practice providing one 'go to' place for facts is
  // convenient.  To keep some separation, the public methods of the fact
  // manager should be grouped according to the kind of fact to which they
  // relate.

  //==============================
  // Querying facts about uniform constants

  // Provides the distinct type ids for which at least one  "constant ==
  // uniform element" fact is known.
  std::vector<uint32_t> GetTypesForWhichUniformValuesAreKnown() const;

  // Provides distinct constant ids with type |type_id| for which at least one
  // "constant == uniform element" fact is known.  If multiple identically-
  // valued constants are relevant, only one will appear in the sequence.
  std::vector<uint32_t> GetConstantsAvailableFromUniformsForType(
      opt::IRContext* ir_context, uint32_t type_id) const;

  // Provides details of all uniform elements that are known to be equal to the
  // constant associated with |constant_id| in |ir_context|.
  const std::vector<protobufs::UniformBufferElementDescriptor>
  GetUniformDescriptorsForConstant(opt::IRContext* ir_context,
                                   uint32_t constant_id) const;

  // Returns the id of a constant whose value is known to match that of
  // |uniform_descriptor|, and whose type matches the type of the uniform
  // element.  If multiple such constant is exist, the one that is returned
  // is arbitrary.  Returns 0 if no such constant id exists.
  uint32_t GetConstantFromUniformDescriptor(
      opt::IRContext* context,
      const protobufs::UniformBufferElementDescriptor& uniform_descriptor)
      const;

  // Returns all "constant == uniform element" facts known to the fact
  // manager, pairing each fact with id of the type that is associated with
  // both the constant and the uniform element.
  const std::vector<std::pair<protobufs::FactConstantUniform, uint32_t>>&
  GetConstantUniformFactsAndTypes() const;

  // End of uniform constant facts
  //==============================

  //==============================
  // Querying facts about id synonyms

  // Returns every id for which a fact of the form "this id is synonymous
  // with this piece of data" is known.
  const std::set<uint32_t>& GetIdsForWhichSynonymsAreKnown() const;

  // Requires that at least one synonym for |id| is known, and returns the
  // sequence of all known synonyms.
  const std::vector<protobufs::DataDescriptor>& GetSynonymsForId(
      uint32_t id) const;

  // End of id synonym facts
  //==============================

 private:
  // For each distinct kind of fact to be managed, we use a separate opaque
  // struct type.

  struct ConstantUniformFacts;  // Opaque struct for holding data about uniform
                                // buffer elements.
  std::unique_ptr<ConstantUniformFacts>
      uniform_constant_facts_;  // Unique pointer to internal data.

  struct IdSynonymFacts;  // Opaque struct for holding data about id synonyms.
  std::unique_ptr<IdSynonymFacts>
      id_synonym_facts_;  // Unique pointer to internal data.
};

}  // namespace fuzz
}  // namespace spvtools

#endif  // SOURCE_FUZZ_FACT_MANAGER_H_
