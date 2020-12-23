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

#include "source/fuzz/pass_management/repeated_pass_manager.h"

namespace spvtools {
namespace fuzz {

RepeatedPassManager::RepeatedPassManager(FuzzerContext* fuzzer_context,
                                         RepeatedPassInstances* pass_instances)
    : fuzzer_context_(fuzzer_context), pass_instances_(pass_instances) {}

RepeatedPassManager::~RepeatedPassManager() = default;

}  // namespace fuzz
}  // namespace spvtools
