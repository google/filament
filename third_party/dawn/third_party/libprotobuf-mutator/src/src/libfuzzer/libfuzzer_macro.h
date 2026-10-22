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

#ifndef SRC_LIBFUZZER_LIBFUZZER_MACRO_H_
#define SRC_LIBFUZZER_LIBFUZZER_MACRO_H_

#include <stddef.h>

#include <cstdint>
#include <functional>
#include <type_traits>

#include "port/protobuf.h"

// Defines custom mutator, crossover and test functions using default
// serialization format. Default is text.
#define DEFINE_PROTO_FUZZER(arg) DEFINE_TEXT_PROTO_FUZZER(arg)
// Defines custom mutator, crossover and test functions using text
// serialization. This format is more convenient to read.
#define DEFINE_TEXT_PROTO_FUZZER(arg) DEFINE_PROTO_FUZZER_IMPL(false, arg)
// Defines custom mutator, crossover and test functions using binary
// serialization. This makes mutations faster. However often test function is
// significantly slower than mutator, so fuzzing rate may stay unchanged.
#define DEFINE_BINARY_PROTO_FUZZER(arg) DEFINE_PROTO_FUZZER_IMPL(true, arg)

// Registers the callback as a potential mutation performed on the parent
// message of a field. This must be called inside an initialization code block.
// libFuzzer suggests putting one-time-initialization in a function used to
// initialize a static variable inside the fuzzer target. For example:
//
// static bool Modify(
//     SomeMessage* message /* Fix or additionally modify the message */,
//     unsigned int seed /* If random generator is needed use this seed */) {
//   ...
// }
//
// DEFINE_PROTO_FUZZER(const SomeMessage& msg) {
//   static PostProcessorRegistration reg(&Modify);
// }

// Implementation of macros above.
#define DEFINE_CUSTOM_PROTO_MUTATOR_IMPL(use_binary, Proto)                    \
  extern "C" size_t LLVMFuzzerCustomMutator(                                   \
      uint8_t* data, size_t size, size_t max_size, unsigned int seed) {        \
    using protobuf_mutator::libfuzzer::CustomProtoMutator;                     \
    Proto input;                                                               \
    return CustomProtoMutator(use_binary, data, size, max_size, seed, &input); \
  }

#define DEFINE_CUSTOM_PROTO_CROSSOVER_IMPL(use_binary, Proto)                 \
  extern "C" size_t LLVMFuzzerCustomCrossOver(                                \
      const uint8_t* data1, size_t size1, const uint8_t* data2, size_t size2, \
      uint8_t* out, size_t max_out_size, unsigned int seed) {                 \
    using protobuf_mutator::libfuzzer::CustomProtoCrossOver;                  \
    Proto input1;                                                             \
    Proto input2;                                                             \
    return CustomProtoCrossOver(use_binary, data1, size1, data2, size2, out,  \
                                max_out_size, seed, &input1, &input2);        \
  }

#define DEFINE_TEST_ONE_PROTO_INPUT_IMPL(use_binary, Proto)                 \
  extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) { \
    using protobuf_mutator::libfuzzer::LoadProtoInput;                      \
    Proto input;                                                            \
    if (LoadProtoInput(use_binary, data, size, &input))                     \
      TestOneProtoInput(input);                                             \
    return 0;                                                               \
  }

#define DEFINE_POST_PROCESS_PROTO_MUTATION_IMPL(Proto) \
  using PostProcessorRegistration =                    \
      protobuf_mutator::libfuzzer::PostProcessorRegistration<Proto>;

#define DEFINE_PROTO_FUZZER_IMPL(use_binary, arg)                 \
  static void TestOneProtoInput(arg);                             \
  using FuzzerProtoType =                                         \
      protobuf_mutator::libfuzzer::macro_internal::GetFirstParam< \
          decltype(&TestOneProtoInput)>::type;                    \
  DEFINE_CUSTOM_PROTO_MUTATOR_IMPL(use_binary, FuzzerProtoType)   \
  DEFINE_CUSTOM_PROTO_CROSSOVER_IMPL(use_binary, FuzzerProtoType) \
  DEFINE_TEST_ONE_PROTO_INPUT_IMPL(use_binary, FuzzerProtoType)   \
  DEFINE_POST_PROCESS_PROTO_MUTATION_IMPL(FuzzerProtoType)        \
  static void TestOneProtoInput(arg)

namespace protobuf_mutator {
namespace libfuzzer {

size_t CustomProtoMutator(bool binary, uint8_t* data, size_t size,
                          size_t max_size, unsigned int seed,
                          protobuf::Message* input);
size_t CustomProtoCrossOver(bool binary, const uint8_t* data1, size_t size1,
                            const uint8_t* data2, size_t size2, uint8_t* out,
                            size_t max_out_size, unsigned int seed,
                            protobuf::Message* input1,
                            protobuf::Message* input2);
bool LoadProtoInput(bool binary, const uint8_t* data, size_t size,
                    protobuf::Message* input);

void RegisterPostProcessor(
    const protobuf::Descriptor* desc,
    std::function<void(protobuf::Message* message, unsigned int seed)>
        callback);

template <class Proto>
struct PostProcessorRegistration {
  PostProcessorRegistration(
      const std::function<void(Proto* message, unsigned int seed)>& callback) {
    RegisterPostProcessor(
        Proto::descriptor(),
        [callback](protobuf::Message* message, unsigned int seed) {
          callback(protobuf::DownCastMessage<Proto>(message), seed);
        });
  }
};

namespace macro_internal {

template <typename T>
struct GetFirstParam;

template <class Arg>
struct GetFirstParam<void (*)(Arg)> {
  using type = typename std::remove_const<
      typename std::remove_reference<Arg>::type>::type;
};

}  // namespace macro_internal

}  // namespace libfuzzer
}  // namespace protobuf_mutator

#endif  // SRC_LIBFUZZER_LIBFUZZER_MACRO_H_
