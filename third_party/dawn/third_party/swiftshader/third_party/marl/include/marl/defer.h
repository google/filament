// Copyright 2019 The Marl Authors.
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

#ifndef marl_defer_h
#define marl_defer_h

#include "finally.h"

namespace marl {

#define MARL_CONCAT_(a, b) a##b
#define MARL_CONCAT(a, b) MARL_CONCAT_(a, b)

// defer() is a macro to defer execution of a statement until the surrounding
// scope is closed and is typically used to perform cleanup logic once a
// function returns.
//
// Note: Unlike golang's defer(), the defer statement is executed when the
// surrounding *scope* is closed, not necessarily the function.
//
// Example usage:
//
//  void sayHelloWorld()
//  {
//      defer(printf("world\n"));
//      printf("hello ");
//  }
//
#define defer(x) \
  auto MARL_CONCAT(defer_, __LINE__) = marl::make_finally([&] { x; })

}  // namespace marl

#endif  // marl_defer_h
