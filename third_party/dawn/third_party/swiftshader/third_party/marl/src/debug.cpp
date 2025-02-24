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

#include "marl/debug.h"
#include "marl/scheduler.h"

#include <cstdarg>
#include <cstdlib>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace marl {

void fatal(const char* msg, ...) {
  va_list vararg;
  va_start(vararg, msg);
  vfprintf(stderr, msg, vararg);
  va_end(vararg);
  abort();
}

void warn(const char* msg, ...) {
  va_list vararg;
  va_start(vararg, msg);
  vfprintf(stdout, msg, vararg);
  va_end(vararg);
}

void assert_has_bound_scheduler(const char* feature) {
  (void)feature;  // unreferenced parameter
  MARL_ASSERT(Scheduler::get() != nullptr,
              "%s requires a marl::Scheduler to be bound", feature);
}

}  // namespace marl
