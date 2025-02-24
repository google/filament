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

#if !defined(_XOPEN_SOURCE)
// This must come before other #includes, otherwise we'll end up with ucontext_t
// definition mismatches, leading to memory corruption hilarity.
#define _XOPEN_SOURCE
#endif  //  !defined(_XOPEN_SOURCE)

#include "marl/debug.h"
#include "marl/memory.h"

#include <functional>
#include <memory>

#include <ucontext.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif  // defined(__clang__)

namespace marl {

class OSFiber {
 public:
  inline OSFiber(Allocator*);
  inline ~OSFiber();

  // createFiberFromCurrentThread() returns a fiber created from the current
  // thread.
  static inline Allocator::unique_ptr<OSFiber> createFiberFromCurrentThread(
      Allocator* allocator);

  // createFiber() returns a new fiber with the given stack size that will
  // call func when switched to. func() must end by switching back to another
  // fiber, and must not return.
  static inline Allocator::unique_ptr<OSFiber> createFiber(
      Allocator* allocator,
      size_t stackSize,
      const std::function<void()>& func);

  // switchTo() immediately switches execution to the given fiber.
  // switchTo() must be called on the currently executing fiber.
  inline void switchTo(OSFiber*);

 private:
  Allocator* allocator;
  ucontext_t context;
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
  getcontext(&out->context);
  return out;
}

Allocator::unique_ptr<OSFiber> OSFiber::createFiber(
    Allocator* allocator,
    size_t stackSize,
    const std::function<void()>& func) {
  union Args {
    OSFiber* self;
    struct {
      int a;
      int b;
    };
  };

  struct Target {
    static void Main(int a, int b) {
      Args u;
      u.a = a;
      u.b = b;
      u.self->target();
    }
  };

  Allocation::Request request;
  request.size = stackSize;
  request.alignment = 16;
  request.usage = Allocation::Usage::Stack;
#if MARL_USE_FIBER_STACK_GUARDS
  request.useGuards = true;
#endif

  auto out = allocator->make_unique<OSFiber>(allocator);
  out->context = {};
  out->stack = allocator->allocate(request);
  out->target = func;

  auto res = getcontext(&out->context);
  (void)res;
  MARL_ASSERT(res == 0, "getcontext() returned %d", int(res));
  out->context.uc_stack.ss_sp = out->stack.ptr;
  out->context.uc_stack.ss_size = stackSize;
  out->context.uc_link = nullptr;

  Args args{};
  args.self = out.get();
  makecontext(&out->context, reinterpret_cast<void (*)()>(&Target::Main), 2,
              args.a, args.b);

  return out;
}

void OSFiber::switchTo(OSFiber* fiber) {
  auto res = swapcontext(&context, &fiber->context);
  (void)res;
  MARL_ASSERT(res == 0, "swapcontext() returned %d", int(res));
}

}  // namespace marl

#if defined(__clang__)
#pragma clang diagnostic pop
#endif  // defined(__clang__)
