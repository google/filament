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

#include "marl_test.h"

#include "marl/memory.h"
#include "marl/pool.h"
#include "marl/waitgroup.h"

TEST_P(WithBoundScheduler, UnboundedPool_ConstructDestruct) {
  marl::UnboundedPool<int> pool;
}

TEST_P(WithBoundScheduler, BoundedPool_ConstructDestruct) {
  marl::BoundedPool<int, 10> pool;
}

TEST_P(WithBoundScheduler, UnboundedPoolLoan_GetNull) {
  marl::UnboundedPool<int>::Loan loan;
  ASSERT_EQ(loan.get(), nullptr);
}

TEST_P(WithBoundScheduler, BoundedPoolLoan_GetNull) {
  marl::BoundedPool<int, 10>::Loan loan;
  ASSERT_EQ(loan.get(), nullptr);
}

TEST_P(WithBoundScheduler, UnboundedPool_Borrow) {
  marl::UnboundedPool<int> pool;
  for (int i = 0; i < 100; i++) {
    pool.borrow();
  }
}

TEST_P(WithBoundScheduler, UnboundedPool_ConcurrentBorrow) {
  marl::UnboundedPool<int> pool;
  constexpr int iterations = 10000;
  marl::WaitGroup wg(iterations);
  for (int i = 0; i < iterations; i++) {
    marl::schedule([=] {
      pool.borrow();
      wg.done();
    });
  }
  wg.wait();
}

TEST_P(WithBoundScheduler, BoundedPool_Borrow) {
  marl::BoundedPool<int, 100> pool;
  for (int i = 0; i < 100; i++) {
    pool.borrow();
  }
}

TEST_P(WithBoundScheduler, BoundedPool_ConcurrentBorrow) {
  marl::BoundedPool<int, 10> pool;
  constexpr int iterations = 10000;
  marl::WaitGroup wg(iterations);
  for (int i = 0; i < iterations; i++) {
    marl::schedule([=] {
      pool.borrow();
      wg.done();
    });
  }
  wg.wait();
}

struct CtorDtorCounter {
  CtorDtorCounter() { ctor_count++; }
  ~CtorDtorCounter() { dtor_count++; }
  static void reset() {
    ctor_count = 0;
    dtor_count = 0;
  }
  static int ctor_count;
  static int dtor_count;
};

int CtorDtorCounter::ctor_count = -1;
int CtorDtorCounter::dtor_count = -1;

TEST_P(WithBoundScheduler, UnboundedPool_PolicyReconstruct) {
  CtorDtorCounter::reset();
  marl::UnboundedPool<CtorDtorCounter, marl::PoolPolicy::Reconstruct> pool;
  ASSERT_EQ(CtorDtorCounter::ctor_count, 0);
  ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
  {
    auto loan = pool.borrow();
    ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
  }
  ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
  ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
  {
    auto loan = pool.borrow();
    ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
  }
  ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
  ASSERT_EQ(CtorDtorCounter::dtor_count, 2);
}

TEST_P(WithBoundScheduler, BoundedPool_PolicyReconstruct) {
  CtorDtorCounter::reset();
  marl::BoundedPool<CtorDtorCounter, 10, marl::PoolPolicy::Reconstruct> pool;
  ASSERT_EQ(CtorDtorCounter::ctor_count, 0);
  ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
  {
    auto loan = pool.borrow();
    ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
  }
  ASSERT_EQ(CtorDtorCounter::ctor_count, 1);
  ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
  {
    auto loan = pool.borrow();
    ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 1);
  }
  ASSERT_EQ(CtorDtorCounter::ctor_count, 2);
  ASSERT_EQ(CtorDtorCounter::dtor_count, 2);
}

TEST_P(WithBoundScheduler, UnboundedPool_PolicyPreserve) {
  CtorDtorCounter::reset();
  {
    marl::UnboundedPool<CtorDtorCounter, marl::PoolPolicy::Preserve> pool;
    int ctor_count;
    {
      auto loan = pool.borrow();
      ASSERT_NE(CtorDtorCounter::ctor_count, 0);
      ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
      ctor_count = CtorDtorCounter::ctor_count;
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    {
      auto loan = pool.borrow();
      ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
      ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
  }
  ASSERT_EQ(CtorDtorCounter::ctor_count, CtorDtorCounter::dtor_count);
}

TEST_P(WithBoundScheduler, BoundedPool_PolicyPreserve) {
  CtorDtorCounter::reset();
  {
    marl::BoundedPool<CtorDtorCounter, 10, marl::PoolPolicy::Preserve> pool;
    int ctor_count;
    {
      auto loan = pool.borrow();
      ASSERT_NE(CtorDtorCounter::ctor_count, 0);
      ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
      ctor_count = CtorDtorCounter::ctor_count;
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    {
      auto loan = pool.borrow();
      ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
      ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
    }
    ASSERT_EQ(CtorDtorCounter::ctor_count, ctor_count);
    ASSERT_EQ(CtorDtorCounter::dtor_count, 0);
  }
  ASSERT_EQ(CtorDtorCounter::ctor_count, CtorDtorCounter::dtor_count);
}

struct alignas(64) StructWithAlignment {
  uint8_t i;
  uint8_t padding[63];
};

TEST_P(WithBoundScheduler, BoundedPool_AlignedTypes) {
  marl::BoundedPool<StructWithAlignment, 100> pool;
  for (int i = 0; i < 100; i++) {
    auto loan = pool.borrow();
    ASSERT_EQ(reinterpret_cast<uintptr_t>(&loan->i) &
                  (alignof(StructWithAlignment) - 1),
              0U);
  }
}

TEST_P(WithBoundScheduler, UnboundedPool_AlignedTypes) {
  marl::UnboundedPool<StructWithAlignment> pool;
  for (int i = 0; i < 100; i++) {
    auto loan = pool.borrow();
    ASSERT_EQ(reinterpret_cast<uintptr_t>(&loan->i) &
                  (alignof(StructWithAlignment) - 1),
              0U);
  }
}
