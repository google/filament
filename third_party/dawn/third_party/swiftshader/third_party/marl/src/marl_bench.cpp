// Copyright 2020 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "marl_bench.h"

#include "marl/sanitizers.h"

int main(int argc, char** argv) {
#if MARL_ADDRESS_SANITIZER_ENABLED
  printf(
      "***WARNING*** Marl built with address sanitizer enabled. "
      "Timings will be affected\n");
#endif
#if MARL_MEMORY_SANITIZER_ENABLED
  printf(
      "***WARNING*** Marl built with memory sanitizer enabled. "
      "Timings will be affected\n");
#endif
#if MARL_THREAD_SANITIZER_ENABLED
  printf(
      "***WARNING*** Marl built with thread sanitizer enabled. "
      "Timings will be affected\n");
#endif
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  return 0;
}

uint32_t Schedule::doSomeWork(uint32_t x) {
  uint32_t q = x;
  for (uint32_t i = 0; i < 100000; i++) {
    x = (x << 4) | x;
    x = x | 0x1020;
    x = (x >> 2) & q;
  }
  return x;
}
