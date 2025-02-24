// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef marl_blocking_call_h
#define marl_blocking_call_h

#include "export.h"
#include "scheduler.h"
#include "waitgroup.h"

#include <thread>
#include <type_traits>
#include <utility>

namespace marl {
namespace detail {

template <typename RETURN_TYPE>
class OnNewThread {
 public:
  template <typename F, typename... Args>
  MARL_NO_EXPORT inline static RETURN_TYPE call(F&& f, Args&&... args) {
    RETURN_TYPE result;
    WaitGroup wg(1);
    auto scheduler = Scheduler::get();
    auto thread = std::thread(
        [&, wg](Args&&... args) {
          if (scheduler != nullptr) {
            scheduler->bind();
          }
          result = f(std::forward<Args>(args)...);
          if (scheduler != nullptr) {
            Scheduler::unbind();
          }
          wg.done();
        },
        std::forward<Args>(args)...);
    wg.wait();
    thread.join();
    return result;
  }
};

template <>
class OnNewThread<void> {
 public:
  template <typename F, typename... Args>
  MARL_NO_EXPORT inline static void call(F&& f, Args&&... args) {
    WaitGroup wg(1);
    auto scheduler = Scheduler::get();
    auto thread = std::thread(
        [&, wg](Args&&... args) {
          if (scheduler != nullptr) {
            scheduler->bind();
          }
          f(std::forward<Args>(args)...);
          if (scheduler != nullptr) {
            Scheduler::unbind();
          }
          wg.done();
        },
        std::forward<Args>(args)...);
    wg.wait();
    thread.join();
  }
};

}  // namespace detail

// blocking_call() calls the function F on a new thread, yielding this fiber
// to execute other tasks until F has returned.
//
// Example:
//
//  void runABlockingFunctionOnATask()
//  {
//      // Schedule a task that calls a blocking, non-yielding function.
//      marl::schedule([=] {
//          // call_blocking_function() may block indefinitely.
//          // Ensure this call does not block other tasks from running.
//          auto result = marl::blocking_call(call_blocking_function);
//          // call_blocking_function() has now returned.
//          // result holds the return value of the blocking function call.
//      });
//  }
template <typename F, typename... Args>
MARL_NO_EXPORT auto inline blocking_call(F&& f, Args&&... args)
    -> decltype(f(args...)) {
  return detail::OnNewThread<decltype(f(args...))>::call(
      std::forward<F>(f), std::forward<Args>(args)...);
}

}  // namespace marl

#endif  // marl_blocking_call_h
