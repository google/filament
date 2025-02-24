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

#if defined(__aarch64__)

#include <stddef.h>

#include "osfiber_asm_aarch64.h"

#include "marl/export.h"

MARL_EXPORT
void marl_fiber_trampoline(void (*target)(void*), void* arg) {
  target(arg);
}

// __attribute__((weak)) doesn't work on MacOS.
#if defined(linux) || defined(__linux) || defined(__linux__)
// This is needed for HWSAan runtimes that don't have this commit:
// https://reviews.llvm.org/D149228.
__attribute__((weak)) void __hwasan_tag_memory(const volatile void *p,
    unsigned char tag, size_t size);
__attribute((weak)) void *__hwasan_tag_pointer(const volatile void *p,
    unsigned char tag);
#endif

MARL_EXPORT
void marl_fiber_set_target(struct marl_fiber_context* ctx,
                           void* stack,
                           uint32_t stack_size,
                           void (*target)(void*),
                           void* arg) {

#if defined(linux) || defined(__linux) || defined(__linux__)
  if (__hwasan_tag_memory && __hwasan_tag_pointer) {
    stack = __hwasan_tag_pointer(stack, 0);
    __hwasan_tag_memory(stack, 0, stack_size);
  }
#endif
  uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
  ctx->LR = (uintptr_t)&marl_fiber_trampoline;
  ctx->r0 = (uintptr_t)target;
  ctx->r1 = (uintptr_t)arg;
  ctx->SP = ((uintptr_t)stack_top) & ~(uintptr_t)15;
}

#endif  // defined(__aarch64__)
