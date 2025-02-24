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

#include "osfiber.h"

#include "marl_test.h"

namespace {

// A custom, small stack size for the fibers in these tests.
// Note: Stack sizes less than 16KB may cause issues on some platforms.
// See: https://github.com/google/marl/issues/201
constexpr size_t fiberStackSize = 16 * 1024;

}  // anonymous namespace

TEST_F(WithoutBoundScheduler, OSFiber) {
  std::string str;
  auto main = marl::OSFiber::createFiberFromCurrentThread(allocator);
  marl::Allocator::unique_ptr<marl::OSFiber> fiberA, fiberB, fiberC;
  fiberC = marl::OSFiber::createFiber(allocator, fiberStackSize, [&] {
    str += "C";
    fiberC->switchTo(fiberB.get());
  });
  fiberB = marl::OSFiber::createFiber(allocator, fiberStackSize, [&] {
    str += "B";
    fiberB->switchTo(fiberA.get());
  });
  fiberA = marl::OSFiber::createFiber(allocator, fiberStackSize, [&] {
    str += "A";
    fiberA->switchTo(main.get());
  });

  main->switchTo(fiberC.get());

  ASSERT_EQ(str, "CBA");
}

TEST_F(WithoutBoundScheduler, StackAlignment) {
  uintptr_t address = 0;

  struct alignas(16) AlignTo16Bytes {
    uint64_t a, b;
  };

  auto main = marl::OSFiber::createFiberFromCurrentThread(allocator);
  marl::Allocator::unique_ptr<marl::OSFiber> fiber;
  fiber = marl::OSFiber::createFiber(allocator, fiberStackSize, [&] {
    AlignTo16Bytes stack_var;

    address = reinterpret_cast<uintptr_t>(&stack_var);

    fiber->switchTo(main.get());
  });

  main->switchTo(fiber.get());

  ASSERT_TRUE((address & 15) == 0)
      << "Stack variable had unaligned address: 0x" << std::hex << address;
}
