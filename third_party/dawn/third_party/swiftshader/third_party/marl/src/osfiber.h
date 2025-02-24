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

#include "marl/sanitizers.h"

#ifndef MARL_USE_FIBER_STACK_GUARDS
#if !defined(NDEBUG) && !MARL_ADDRESS_SANITIZER_ENABLED
#define MARL_USE_FIBER_STACK_GUARDS 1
#else
#define MARL_USE_FIBER_STACK_GUARDS 0
#endif
#endif  // MARL_USE_FIBER_STACK_GUARDS

#if MARL_USE_FIBER_STACK_GUARDS && MARL_ADDRESS_SANITIZER_ENABLED
#warning "ASAN can raise spurious failures when using mmap() allocated stacks"
#endif

#if defined(_WIN32)
#include "osfiber_windows.h"
#elif defined(MARL_FIBERS_USE_UCONTEXT)
#include "osfiber_ucontext.h"
#else
#include "osfiber_asm.h"
#endif
