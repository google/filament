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

// Minimal assembly implementations of fiber context switching for Unix-based
// platforms.
//
// Note: Unlike makecontext, swapcontext or the Windows fiber APIs, these
// assembly implementations *do not* save or restore signal masks,
// floating-point control or status registers, FS and GS segment registers,
// thread-local storage state nor any SIMD registers. This should not be a
// problem as the marl scheduler requires fibers to be executed on the same
// thread throughout their lifetime.

#if defined(__x86_64__)
#include "osfiber_asm_x64.h"
#elif defined(__i386__)
#include "osfiber_asm_x86.h"
#elif defined(__aarch64__)
#include "osfiber_asm_aarch64.h"
#elif defined(__arm__)
#include "osfiber_asm_arm.h"
#elif defined(__powerpc64__)
#include "osfiber_asm_ppc64.h"
#elif defined(__mips__) && _MIPS_SIM == _ABI64
#include "osfiber_asm_mips64.h"
#elif defined(__riscv) && __riscv_xlen == 64
#include "osfiber_asm_rv64.h"
#elif defined(__loongarch__) && _LOONGARCH_SIM == _ABILP64
#include "osfiber_asm_loongarch64.h"
#elif defined(__EMSCRIPTEN__)
#include "osfiber_emscripten.h"
#else
#error "Unsupported target"
#endif

#include "marl/export.h"
#include "marl/memory.h"

#include <functional>
#include <memory>

extern "C" {

#if defined(__EMSCRIPTEN__)
MARL_EXPORT
void marl_main_fiber_init(marl_fiber_context* ctx);
#else
MARL_EXPORT
inline void marl_main_fiber_init(marl_fiber_context*) {}
#endif
MARL_EXPORT
extern void marl_fiber_set_target(marl_fiber_context*,
                                  void* stack,
                                  uint32_t stack_size,
                                  void (*target)(void*),
                                  void* arg);
MARL_EXPORT
extern void marl_fiber_swap(marl_fiber_context* from,
                            const marl_fiber_context* to);

}  // extern "C"

namespace marl {

class OSFiber {
 public:
  inline OSFiber(Allocator*);
  inline ~OSFiber();

  // createFiberFromCurrentThread() returns a fiber created from the current
  // thread.
  MARL_NO_EXPORT static inline Allocator::unique_ptr<OSFiber>
  createFiberFromCurrentThread(Allocator* allocator);

  // createFiber() returns a new fiber with the given stack size that will
  // call func when switched to. func() must end by switching back to another
  // fiber, and must not return.
  MARL_NO_EXPORT static inline Allocator::unique_ptr<OSFiber> createFiber(
      Allocator* allocator,
      size_t stackSize,
      const std::function<void()>& func);

  // switchTo() immediately switches execution to the given fiber.
  // switchTo() must be called on the currently executing fiber.
  MARL_NO_EXPORT inline void switchTo(OSFiber*);

 private:
  MARL_NO_EXPORT
  static inline void run(OSFiber* self);

  Allocator* allocator;
  marl_fiber_context context;
  std::function<void()> target;
  Allocation stack;
};

OSFiber::OSFiber(Allocator* allocator) : allocator(allocator) {}

OSFiber::~OSFiber() {
  if (stack.ptr != nullptr) {
    allocator->free(stack);
  }
}

Allocator::unique_ptr<OSFiber> OSFiber::createFiberFromCurrentThread(
    Allocator* allocator) {
  auto out = allocator->make_unique<OSFiber>(allocator);
  out->context = {};
  marl_main_fiber_init(&out->context);
  return out;
}

Allocator::unique_ptr<OSFiber> OSFiber::createFiber(
    Allocator* allocator,
    size_t stackSize,
    const std::function<void()>& func) {
  Allocation::Request request;
  request.size = stackSize;
  request.alignment = 16;
  request.usage = Allocation::Usage::Stack;
#if MARL_USE_FIBER_STACK_GUARDS
  request.useGuards = true;
#endif

  auto out = allocator->make_unique<OSFiber>(allocator);
  out->context = {};
  out->target = func;
  out->stack = allocator->allocate(request);
  marl_fiber_set_target(
      &out->context, out->stack.ptr, static_cast<uint32_t>(stackSize),
      reinterpret_cast<void (*)(void*)>(&OSFiber::run), out.get());
  return out;
}

void OSFiber::run(OSFiber* self) {
  self->target();
}

void OSFiber::switchTo(OSFiber* fiber) {
  marl_fiber_swap(&context, &fiber->context);
}

}  // namespace marl
