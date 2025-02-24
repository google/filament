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

#ifndef marl_parallelize_h
#define marl_parallelize_h

#include "scheduler.h"
#include "waitgroup.h"

namespace marl {

namespace detail {

MARL_NO_EXPORT inline void parallelizeChain(WaitGroup&) {}

template <typename F, typename... L>
MARL_NO_EXPORT inline void parallelizeChain(WaitGroup& wg, F&& f, L&&... l) {
  schedule([=] {
    f();
    wg.done();
  });
  parallelizeChain(wg, std::forward<L>(l)...);
}

}  // namespace detail

// parallelize() invokes all the function parameters, potentially concurrently,
// and waits for them all to complete before returning.
//
// Each function must take no parameters.
//
// parallelize() does the following:
//   (1) Schedules the function parameters in the parameter pack fn.
//   (2) Calls f0 on the current thread.
//   (3) Once f0 returns, waits for the scheduled functions in fn to all
//   complete.
// As the fn functions are scheduled before running f0, it is recommended to
// pass the function that'll take the most time as the first argument. That way
// you'll be more likely to avoid the cost of a fiber switch.
template <typename F0, typename... FN>
MARL_NO_EXPORT inline void parallelize(F0&& f0, FN&&... fn) {
  WaitGroup wg(sizeof...(FN));
  // Schedule all the functions in fn.
  detail::parallelizeChain(wg, std::forward<FN>(fn)...);
  // While we wait for fn to complete, run the first function on this thread.
  f0();
  wg.wait();
}

}  // namespace marl

#endif  // marl_parallelize_h
