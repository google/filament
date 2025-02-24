// Copyright 2023 The Marl Authors.
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

#if defined(__EMSCRIPTEN__)

#include "osfiber_emscripten.h"

#include "marl/export.h"


extern "C" {

MARL_EXPORT
void marl_fiber_trampoline(void (*target)(void*), void* arg) {
  target(arg);
}

MARL_EXPORT
void marl_main_fiber_init(marl_fiber_context* ctx) {
  emscripten_fiber_init_from_current_context(
          &ctx->context,
          ctx->asyncify_stack.data(),
          ctx->asyncify_stack.size());
}

MARL_EXPORT
void marl_fiber_set_target(marl_fiber_context* ctx,
                           void* stack,
                           uint32_t stack_size,
                           void (*target)(void*),
                           void* arg) {

  emscripten_fiber_init(
          &ctx->context,
          target,
          arg,
          stack,
          stack_size,
          ctx->asyncify_stack.data(),
          ctx->asyncify_stack.size());
}

MARL_EXPORT
extern void marl_fiber_swap(marl_fiber_context* from,
                            const marl_fiber_context* to) {
  emscripten_fiber_swap(&from->context, const_cast<emscripten_fiber_t*>(&to->context));
}
}
#endif  // defined(__EMSCRIPTEN__)
