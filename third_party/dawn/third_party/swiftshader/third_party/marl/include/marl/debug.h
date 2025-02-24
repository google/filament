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

#ifndef marl_debug_h
#define marl_debug_h

#include "export.h"

#if !defined(MARL_DEBUG_ENABLED)
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#define MARL_DEBUG_ENABLED 1
#else
#define MARL_DEBUG_ENABLED 0
#endif
#endif

namespace marl {

MARL_EXPORT
void fatal(const char* msg, ...);

MARL_EXPORT
void warn(const char* msg, ...);

MARL_EXPORT
void assert_has_bound_scheduler(const char* feature);

#if MARL_DEBUG_ENABLED
#define MARL_FATAL(msg, ...) marl::fatal(msg "\n", ##__VA_ARGS__);
#define MARL_ASSERT(cond, msg, ...)              \
  do {                                           \
    if (!(cond)) {                               \
      MARL_FATAL("ASSERT: " msg, ##__VA_ARGS__); \
    }                                            \
  } while (false);
#define MARL_ASSERT_HAS_BOUND_SCHEDULER(feature) \
  assert_has_bound_scheduler(feature);
#define MARL_UNREACHABLE() MARL_FATAL("UNREACHABLE");
#define MARL_WARN(msg, ...) marl::warn("WARNING: " msg "\n", ##__VA_ARGS__);
#else
#define MARL_FATAL(msg, ...)
#define MARL_ASSERT(cond, msg, ...)
#define MARL_ASSERT_HAS_BOUND_SCHEDULER(feature)
#define MARL_UNREACHABLE()
#define MARL_WARN(msg, ...)
#endif

}  // namespace marl

#endif  // marl_debug_h
