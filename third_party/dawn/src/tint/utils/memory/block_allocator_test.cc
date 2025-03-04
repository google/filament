// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/memory/block_allocator.h"

#include <vector>

#include "gtest/gtest.h"

namespace tint {
namespace {

struct LifetimeCounter {
    explicit LifetimeCounter(size_t* count) : count_(count) { (*count)++; }
    ~LifetimeCounter() { (*count_)--; }

    size_t* const count_;
};

using BlockAllocatorTest = testing::Test;

TEST_F(BlockAllocatorTest, Empty) {
    using Allocator = BlockAllocator<int>;

    Allocator allocator;

    EXPECT_EQ(allocator.Count(), 0u);
    for (int* i : allocator.Objects()) {
        (void)i;
        if ((true)) {  // Workaround for "error: loop will run at most once"
            FAIL() << "BlockAllocator should be empty";
        }
    }
    for (const int* i : static_cast<const Allocator&>(allocator).Objects()) {
        (void)i;
        if ((true)) {  // Workaround for "error: loop will run at most once"
            FAIL() << "BlockAllocator should be empty";
        }
    }
}

TEST_F(BlockAllocatorTest, Count) {
    using Allocator = BlockAllocator<int>;

    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        Allocator allocator;
        EXPECT_EQ(allocator.Count(), 0u);
        for (size_t i = 0; i < n; i++) {
            allocator.Create(123);
        }
        EXPECT_EQ(allocator.Count(), n);
    }
}

TEST_F(BlockAllocatorTest, ObjectLifetime) {
    using Allocator = BlockAllocator<LifetimeCounter>;

    size_t count = 0;
    {
        Allocator allocator;
        EXPECT_EQ(count, 0u);
        allocator.Create(&count);
        EXPECT_EQ(count, 1u);
        allocator.Create(&count);
        EXPECT_EQ(count, 2u);
        allocator.Create(&count);
        EXPECT_EQ(count, 3u);
    }
    EXPECT_EQ(count, 0u);
}

TEST_F(BlockAllocatorTest, MoveConstruct) {
    using Allocator = BlockAllocator<LifetimeCounter>;

    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        size_t count = 0;
        {
            Allocator allocator_a;
            for (size_t i = 0; i < n; i++) {
                allocator_a.Create(&count);
            }
            EXPECT_EQ(count, n);
            EXPECT_EQ(allocator_a.Count(), n);

            Allocator allocator_b{std::move(allocator_a)};
            EXPECT_EQ(count, n);
            EXPECT_EQ(allocator_b.Count(), n);
        }

        EXPECT_EQ(count, 0u);
    }
}

TEST_F(BlockAllocatorTest, MoveAssign) {
    using Allocator = BlockAllocator<LifetimeCounter>;

    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        size_t count_a = 0;
        size_t count_b = 0;

        {
            Allocator allocator_a;
            for (size_t i = 0; i < n; i++) {
                allocator_a.Create(&count_a);
            }
            EXPECT_EQ(count_a, n);
            EXPECT_EQ(allocator_a.Count(), n);

            Allocator allocator_b;
            for (size_t i = 0; i < n; i++) {
                allocator_b.Create(&count_b);
            }
            EXPECT_EQ(count_b, n);
            EXPECT_EQ(allocator_b.Count(), n);

            allocator_b = std::move(allocator_a);
            EXPECT_EQ(count_a, n);
            EXPECT_EQ(count_b, 0u);
            EXPECT_EQ(allocator_b.Count(), n);
        }

        EXPECT_EQ(count_a, 0u);
        EXPECT_EQ(count_b, 0u);
    }
}

TEST_F(BlockAllocatorTest, ObjectOrder) {
    using Allocator = BlockAllocator<int>;

    Allocator allocator;
    constexpr int N = 10000;
    for (int i = 0; i < N; i++) {
        allocator.Create(i);
    }

    {
        int i = 0;
        for (int* p : allocator.Objects()) {
            EXPECT_EQ(*p, i);
            i++;
        }
        EXPECT_EQ(i, N);
    }
    {
        int i = 0;
        for (const int* p : static_cast<const Allocator&>(allocator).Objects()) {
            EXPECT_EQ(*p, i);
            i++;
        }
        EXPECT_EQ(i, N);
    }
}

TEST_F(BlockAllocatorTest, AddWhileIterating) {
    using Allocator = BlockAllocator<size_t>;

    Allocator allocator;
    for (int i = 0; i < 20; i++) {
        allocator.Create(allocator.Count());

        std::vector<size_t*> seen;
        for (auto* j : allocator.Objects()) {
            if (*j % 3 == 0) {
                allocator.Create(allocator.Count());
            }
            seen.push_back(j);
        }

        // Check that iteration-while-adding saw the same list of objects as an
        // iteration-without-adding.
        size_t n = 0;
        for (auto* obj : allocator.Objects()) {
            ASSERT_TRUE(n < seen.size());
            EXPECT_EQ(seen[n++], obj);
        }
    }
}

}  // namespace
}  // namespace tint
