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
#include "marl/memory.h"

#include <functional>
#include <memory>

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

namespace marl {

class OSFiber {
 public:
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
  static inline void WINAPI run(void* self);
  LPVOID fiber = nullptr;
  bool isFiberFromThread = false;
  std::function<void()> target;
};

OSFiber::~OSFiber() {
  if (fiber != nullptr) {
    if (isFiberFromThread) {
      ConvertFiberToThread();
    } else {
      DeleteFiber(fiber);
    }
  }
}

Allocator::unique_ptr<OSFiber> OSFiber::createFiberFromCurrentThread(
    Allocator* allocator) {
  auto out = allocator->make_unique<OSFiber>();
  out->fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
  out->isFiberFromThread = true;
  MARL_ASSERT(out->fiber != nullptr,
              "ConvertThreadToFiberEx() failed with error 0x%x",
              int(GetLastError()));
  return out;
}

Allocator::unique_ptr<OSFiber> OSFiber::createFiber(
    Allocator* allocator,
    size_t stackSize,
    const std::function<void()>& func) {
  auto out = allocator->make_unique<OSFiber>();
  // stackSize is rounded up to the system's allocation granularity (typically
  // 64 KB).
  out->fiber = CreateFiberEx(stackSize - 1, stackSize, FIBER_FLAG_FLOAT_SWITCH,
                             &OSFiber::run, out.get());
  out->target = func;
  MARL_ASSERT(out->fiber != nullptr, "CreateFiberEx() failed with error 0x%x",
              int(GetLastError()));
  return out;
}

void OSFiber::switchTo(OSFiber* to) {
  SwitchToFiber(to->fiber);
}

void WINAPI OSFiber::run(void* self) {
  std::function<void()> func;
  std::swap(func, reinterpret_cast<OSFiber*>(self)->target);
  func();
}

}  // namespace marl
