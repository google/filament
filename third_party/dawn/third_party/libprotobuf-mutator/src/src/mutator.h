// Copyright 2016 Google Inc. All rights reserved.
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

#ifndef SRC_MUTATOR_H_
#define SRC_MUTATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "port/protobuf.h"
#include "src/random.h"

namespace protobuf_mutator {

// Randomly makes incremental change in the given protobuf.
// Usage example:
//    protobuf_mutator::Mutator mutator(1);
//    MyMessage message;
//    message.ParseFromString(encoded_message);
//    mutator.Mutate(&message, 10000);
//
// Class implements very basic mutations of fields. E.g. it just flips bits for
// integers, floats and strings. Also it increases, decreases size of
// strings only by one. For better results users should override
// protobuf_mutator::Mutator::Mutate* methods with more useful logic, e.g. using
// library like libFuzzer.
class Mutator {
 public:
  // seed: value to initialize random number generator.
  Mutator() = default;
  virtual ~Mutator() = default;

  // Initialized internal random number generator.
  void Seed(uint32_t value);

  // message: message to mutate.
  // max_size_hint: approximate max ByteSize() of resulting message. Method does
  // not guarantee that real result will be strictly smaller than value. Caller
  // could repeat mutation if result was larger than expected.
  void Mutate(protobuf::Message* message, size_t max_size_hint);

  void CrossOver(const protobuf::Message& message1, protobuf::Message* message2,
                 size_t max_size_hint);

  // Makes message initialized and calls post processors to make it valid.
  void Fix(protobuf::Message* message);

  // Callback to postprocess mutations.
  // Implementation should use seed to initialize random number generators.
  using PostProcess =
      std::function<void(protobuf::Message* message, unsigned int seed)>;

  // Register callback which will be called after every message mutation.
  // In this callback fuzzer may adjust content of the message or mutate some
  // fields in some fuzzer specific way.
  void RegisterPostProcessor(const protobuf::Descriptor* desc,
                             PostProcess callback);

 protected:
  // TODO(vitalybuka): Consider to replace with single mutate (uint8_t*, size).
  virtual int32_t MutateInt32(int32_t value);
  virtual int64_t MutateInt64(int64_t value);
  virtual uint32_t MutateUInt32(uint32_t value);
  virtual uint64_t MutateUInt64(uint64_t value);
  virtual float MutateFloat(float value);
  virtual double MutateDouble(double value);
  virtual bool MutateBool(bool value);
  virtual size_t MutateEnum(size_t index, size_t item_count);
  virtual std::string MutateString(const std::string& value,
                                   int size_increase_hint);

  RandomEngine* random() { return &random_; }

 private:
  friend class FieldMutator;
  friend class TestMutator;
  bool MutateImpl(const std::vector<const protobuf::Message*>& sources,
                  const std::vector<protobuf::Message*>& messages,
                  bool copy_clone_only, int size_increase_hint);
  std::string MutateUtf8String(const std::string& value,
                               int size_increase_hint);
  bool IsInitialized(const protobuf::Message& message) const;
  bool keep_initialized_ = true;
  size_t random_to_default_ratio_ = 100;
  RandomEngine random_;
  using PostProcessors =
      std::unordered_multimap<const protobuf::Descriptor*, PostProcess>;
  PostProcessors post_processors_;
};

}  // namespace protobuf_mutator

#endif  // SRC_MUTATOR_H_
