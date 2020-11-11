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

#include "source/fuzz/transformation_add_type_vector.h"

#include "fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddTypeVector::TransformationAddTypeVector(
    const spvtools::fuzz::protobufs::TransformationAddTypeVector& message)
    : message_(message) {}

TransformationAddTypeVector::TransformationAddTypeVector(
    uint32_t fresh_id, uint32_t component_type_id, uint32_t component_count) {
  message_.set_fresh_id(fresh_id);
  message_.set_component_type_id(component_type_id);
  message_.set_component_count(component_count);
}

bool TransformationAddTypeVector::IsApplicable(
    opt::IRContext* ir_context, const TransformationContext& /*unused*/) const {
  if (!fuzzerutil::IsFreshId(ir_context, message_.fresh_id())) {
    return false;
  }
  auto component_type =
      ir_context->get_type_mgr()->GetType(message_.component_type_id());
  if (!component_type) {
    return false;
  }
  return component_type->AsBool() || component_type->AsFloat() ||
         component_type->AsInteger();
}

void TransformationAddTypeVector::Apply(
    opt::IRContext* ir_context, TransformationContext* /*unused*/) const {
  fuzzerutil::AddVectorType(ir_context, message_.fresh_id(),
                            message_.component_type_id(),
                            message_.component_count());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  ir_context->InvalidateAnalysesExceptFor(
      opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddTypeVector::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_type_vector() = message_;
  return result;
}

std::unordered_set<uint32_t> TransformationAddTypeVector::GetFreshIds() const {
  return {message_.fresh_id()};
}

}  // namespace fuzz
}  // namespace spvtools
