// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/utils/memory/bump_allocator.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using BumpAllocatorTest = testing::Test;

TEST_F(BumpAllocatorTest, AllocationSizes) {
    BumpAllocator allocator;
    for (size_t n : {1u, 0x10u, 0x100u, 0x1000u, 0x10000u, 0x100000u,  //
                     2u, 0x34u, 0x567u, 0x8912u, 0x34567u, 0x891234u}) {
        auto ptr = allocator.Allocate(n);
        memset(ptr, 0x42, n);
    }
}

TEST_F(BumpAllocatorTest, AllocationSizesAroundBlockSize) {
    for (size_t n : {
             BumpAllocator::kDefaultBlockDataSize - sizeof(void*),
             BumpAllocator::kDefaultBlockDataSize - 4,
             BumpAllocator::kDefaultBlockDataSize - 1,
             BumpAllocator::kDefaultBlockDataSize,
             BumpAllocator::kDefaultBlockDataSize + 1,
             BumpAllocator::kDefaultBlockDataSize + 4,
             BumpAllocator::kDefaultBlockDataSize + sizeof(void*),
         }) {
        BumpAllocator allocator;
        auto* ptr = allocator.Allocate(n);
        memset(ptr, 0x42, n);
    }
}

TEST_F(BumpAllocatorTest, Count) {
    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        BumpAllocator allocator;
        EXPECT_EQ(allocator.Count(), 0u);
        for (size_t i = 0; i < n; i++) {
            allocator.Allocate(5);
        }
        EXPECT_EQ(allocator.Count(), n);
    }
}

TEST_F(BumpAllocatorTest, MoveConstruct) {
    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        BumpAllocator allocator_a;
        for (size_t i = 0; i < n; i++) {
            allocator_a.Allocate(5);
        }
        EXPECT_EQ(allocator_a.Count(), n);

        BumpAllocator allocator_b{std::move(allocator_a)};
        EXPECT_EQ(allocator_b.Count(), n);
    }
}

}  // namespace
}  // namespace tint
