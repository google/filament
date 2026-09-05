// Copyright 2015 Google Inc. All rights reserved.
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

#ifndef BENCHMARK_TYPES_H_
#define BENCHMARK_TYPES_H_

#include <stdint.h>

#include <functional>
#include <memory>
#include <string>

#include "benchmark/export.h"

namespace benchmark {

namespace internal {
#if (__cplusplus < 201402L || (defined(_MSC_VER) && _MSVC_LANG < 201402L))
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#else
using ::std::make_unique;
#endif
}  // namespace internal

class BenchmarkReporter;
class State;

using IterationCount = int64_t;

using callback_function = std::function<void(const benchmark::State&)>;

enum TimeUnit { kNanosecond, kMicrosecond, kMillisecond, kSecond };

}  // namespace benchmark

#endif  // BENCHMARK_TYPES_H_
