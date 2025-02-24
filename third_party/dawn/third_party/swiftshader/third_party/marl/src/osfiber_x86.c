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

#if defined(__i386__)

#include "osfiber_asm_x86.h"

#include "marl/export.h"

MARL_EXPORT
void marl_fiber_trampoline(void (*target)(void*), void* arg) {
  target(arg);
}

MARL_EXPORT
void marl_fiber_set_target(struct marl_fiber_context* ctx,
                           void* stack,
                           uint32_t stack_size,
                           void (*target)(void*),
                           void* arg) {
  // The stack pointer needs to be 16-byte aligned when making a 'call'.
  // The 'call' instruction automatically pushes the return instruction to the
  // stack (4-bytes), before making the jump.
  // The marl_fiber_swap() assembly function does not use 'call', instead it
  // uses 'jmp', so we need to offset the ESP pointer by 4 bytes so that the
  // stack is still 16-byte aligned when the return target is stack-popped by
  // the callee.
  uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
  ctx->EIP = (uintptr_t)&marl_fiber_trampoline;
  ctx->ESP = (uintptr_t)&stack_top[-5];
  stack_top[-3] = (uintptr_t)arg;
  stack_top[-4] = (uintptr_t)target;
  stack_top[-5] = 0;  // No return target.
}

#endif  // defined(__i386__)
