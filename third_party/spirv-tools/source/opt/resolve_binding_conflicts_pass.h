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

#ifndef LIBSPIRV_OPT_RESOLVE_BINDING_CONFLICTS_PASS_H_
#define LIBSPIRV_OPT_RESOLVE_BINDING_CONFLICTS_PASS_H_

#include <unordered_map>
#include <utility>
#include <vector>

#include "source/diagnostic.h"
#include "source/opt/pass.h"
#include "source/util/small_vector.h"

namespace spvtools {
namespace opt {
class ResolveBindingConflictsPass : public Pass {
 public:
  virtual ~ResolveBindingConflictsPass() override = default;
  const char* name() const override { return "resolve-binding-conflicts"; }
  IRContext::Analysis GetPreservedAnalyses() override;
  Status Process() override;
};
}  // namespace opt
}  // namespace spvtools

#endif  // LIBSPIRV_OPT_RESOLVE_BINDING_CONFLICTS_PASS_H_
