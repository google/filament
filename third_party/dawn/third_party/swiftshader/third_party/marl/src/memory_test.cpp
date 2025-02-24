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

#include "marl/memory.h"

#include "marl_test.h"

class AllocatorTest : public testing::Test {
 public:
  marl::Allocator* allocator = marl::Allocator::Default;
};

TEST_F(AllocatorTest, AlignedAllocate) {
  for (auto useGuards : {false, true}) {
    for (auto alignment : {1, 2, 4, 8, 16, 32, 64, 128}) {
      for (auto size : {1,   2,   3,   4,   5,   7,   8,   14,  16,  17,
                        31,  34,  50,  63,  64,  65,  100, 127, 128, 129,
                        200, 255, 256, 257, 500, 511, 512, 513}) {
        marl::Allocation::Request request;
        request.alignment = alignment;
        request.size = size;
        request.useGuards = useGuards;

        auto allocation = allocator->allocate(request);
        auto ptr = allocation.ptr;
        ASSERT_EQ(allocation.request.size, request.size);
        ASSERT_EQ(allocation.request.alignment, request.alignment);
        ASSERT_EQ(allocation.request.useGuards, request.useGuards);
        ASSERT_EQ(allocation.request.usage, request.usage);
        ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) & (alignment - 1), 0U);
        memset(ptr, 0,
               size);  // Check the memory was actually allocated.
        allocator->free(allocation);
      }
    }
  }
}

struct alignas(16) StructWith16ByteAlignment {
  uint8_t i;
  uint8_t padding[15];
};
struct alignas(32) StructWith32ByteAlignment {
  uint8_t i;
  uint8_t padding[31];
};
struct alignas(64) StructWith64ByteAlignment {
  uint8_t i;
  uint8_t padding[63];
};

TEST_F(AllocatorTest, Create) {
  auto s16 = allocator->create<StructWith16ByteAlignment>();
  auto s32 = allocator->create<StructWith32ByteAlignment>();
  auto s64 = allocator->create<StructWith64ByteAlignment>();
  ASSERT_EQ(alignof(StructWith16ByteAlignment), 16U);
  ASSERT_EQ(alignof(StructWith32ByteAlignment), 32U);
  ASSERT_EQ(alignof(StructWith64ByteAlignment), 64U);
  ASSERT_EQ(reinterpret_cast<uintptr_t>(s16) & 15U, 0U);
  ASSERT_EQ(reinterpret_cast<uintptr_t>(s32) & 31U, 0U);
  ASSERT_EQ(reinterpret_cast<uintptr_t>(s64) & 63U, 0U);
  allocator->destroy(s64);
  allocator->destroy(s32);
  allocator->destroy(s16);
}

#if GTEST_HAS_DEATH_TEST
TEST_F(AllocatorTest, Guards) {
  marl::Allocation::Request request;
  request.alignment = 16;
  request.size = 16;
  request.useGuards = true;
  auto alloc = allocator->allocate(request);
  auto ptr = reinterpret_cast<uint8_t*>(alloc.ptr);
  EXPECT_DEATH(ptr[-1] = 1, "");
  EXPECT_DEATH(ptr[marl::pageSize()] = 1, "");
}
#endif
