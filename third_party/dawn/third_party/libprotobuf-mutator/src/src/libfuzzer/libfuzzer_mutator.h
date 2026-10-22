// Copyright 2017 Google Inc. All rights reserved.
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

#ifndef SRC_LIBFUZZER_LIBFUZZER_MUTATOR_H_
#define SRC_LIBFUZZER_LIBFUZZER_MUTATOR_H_

#include <string>

#include "src/mutator.h"

namespace protobuf_mutator {
namespace libfuzzer {

// Overrides protobuf_mutator::Mutator::Mutate* methods with implementation
// which uses libFuzzer library. protobuf_mutator::Mutator has very basic
// implementation of this methods.
class Mutator : public protobuf_mutator::Mutator {
 public:
  using protobuf_mutator::Mutator::Mutator;

 protected:
  int32_t MutateInt32(int32_t value) override;
  int64_t MutateInt64(int64_t value) override;
  uint32_t MutateUInt32(uint32_t value) override;
  uint64_t MutateUInt64(uint64_t value) override;
  float MutateFloat(float value) override;
  double MutateDouble(double value) override;
  std::string MutateString(const std::string& value,
                           int size_increase_hint) override;
};

}  // namespace libfuzzer
}  // namespace protobuf_mutator

#endif  // SRC_LIBFUZZER_LIBFUZZER_MUTATOR_H_
