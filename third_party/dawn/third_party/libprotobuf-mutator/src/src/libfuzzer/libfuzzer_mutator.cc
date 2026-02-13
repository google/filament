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

#include "src/libfuzzer/libfuzzer_mutator.h"

#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
#include <sanitizer/msan_interface.h>
#endif
#endif
#include <string.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>

#include "port/protobuf.h"
#include "src/mutator.h"

// see compiler-rt/lib/sanitizer-common/sanitizer_internal_defs.h; usage same as
// SANITIZER_INTERFACE_WEAK_DEF with some functionality removed
#ifdef _MSC_VER
#if defined(_M_IX86) || defined(__i386__)
#define WIN_SYM_PREFIX "_"
#else
#define WIN_SYM_PREFIX
#endif

#define STRINGIFY_(A) #A
#define STRINGIFY(A) STRINGIFY_(A)

#define WEAK_DEFAULT_NAME(Name) Name##__def

// clang-format off
#define LIB_PROTO_MUTATOR_WEAK_DEF(ReturnType, Name, ...)     \
  __pragma(comment(linker, "/alternatename:"                  \
           WIN_SYM_PREFIX STRINGIFY(Name) "="                 \
           WIN_SYM_PREFIX STRINGIFY(WEAK_DEFAULT_NAME(Name))))\
  extern "C" ReturnType Name(__VA_ARGS__);                    \
  extern "C" ReturnType WEAK_DEFAULT_NAME(Name)(__VA_ARGS__)
// clang-format on
#else
#define LIB_PROTO_MUTATOR_WEAK_DEF(ReturnType, Name, ...) \
  extern "C" __attribute__((weak)) ReturnType Name(__VA_ARGS__)
#endif

LIB_PROTO_MUTATOR_WEAK_DEF(size_t, LLVMFuzzerMutate, uint8_t*, size_t, size_t) {
  return 0;
}

namespace protobuf_mutator {
namespace libfuzzer {

namespace {

template <class T>
T MutateValue(T v) {
  size_t size =
      LLVMFuzzerMutate(reinterpret_cast<uint8_t*>(&v), sizeof(v), sizeof(v));
  memset(reinterpret_cast<uint8_t*>(&v) + size, 0, sizeof(v) - size);
  // The value from LLVMFuzzerMutate needs to be treated as initialized.
#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
  __msan_unpoison(&v, sizeof(v));
#endif
#endif
  return v;
}

}  // namespace

int32_t Mutator::MutateInt32(int32_t value) { return MutateValue(value); }

int64_t Mutator::MutateInt64(int64_t value) { return MutateValue(value); }

uint32_t Mutator::MutateUInt32(uint32_t value) { return MutateValue(value); }

uint64_t Mutator::MutateUInt64(uint64_t value) { return MutateValue(value); }

float Mutator::MutateFloat(float value) { return MutateValue(value); }

double Mutator::MutateDouble(double value) { return MutateValue(value); }

std::string Mutator::MutateString(const std::string& value,
                                  int size_increase_hint) {
  // Randomly return empty strings as LLVMFuzzerMutate does not produce them.
  // Use uint16_t because on Windows, uniform_int_distribution does not support
  // any 8 bit types.
  if (!std::uniform_int_distribution<uint16_t>(0, 20)(*random())) return {};
  std::string result = value;
  int new_size = value.size() + size_increase_hint;
  result.resize(std::max(1, new_size));
  result.resize(LLVMFuzzerMutate(reinterpret_cast<uint8_t*>(&result[0]),
                                 value.size(), result.size()));
  // The value from LLVMFuzzerMutate needs to be treated as initialized.
#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
  __msan_unpoison(&result[0], result.size());
#endif
#endif
  return result;
}

}  // namespace libfuzzer
}  // namespace protobuf_mutator
