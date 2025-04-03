// Copyright 2020 The Dawn & Tint Authors
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

#include <set>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/common/SlabAllocator.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

struct Foo : public PlacementAllocated {
    explicit Foo(int value) : value(value) {}

    int value;
};

struct alignas(256) AlignedFoo : public Foo {
    using Foo::Foo;
};

// Test that a slab allocator of a single object works.
TEST(SlabAllocatorTests, Single) {
    SlabAllocator<Foo> allocator(1 * sizeof(Foo));

    Foo* obj = allocator.Allocate(4);
    EXPECT_EQ(obj->value, 4);

    allocator.Deallocate(obj);
}

// Allocate multiple objects and check their data is correct.
TEST(SlabAllocatorTests, AllocateSequential) {
    // Check small alignment
    {
        SlabAllocator<Foo> allocator(5 * sizeof(Foo));

        std::vector<Foo*> objects;
        for (int i = 0; i < 10; ++i) {
            auto* ptr = allocator.Allocate(i);
            EXPECT_TRUE(std::find(objects.begin(), objects.end(), ptr) == objects.end());
            objects.push_back(ptr);
        }

        for (int i = 0; i < 10; ++i) {
            // Check that the value is correct and hasn't been trampled.
            EXPECT_EQ(objects[i]->value, i);

            // Check that the alignment is correct.
            EXPECT_TRUE(IsPtrAligned(objects[i], alignof(Foo)));
        }

        // Deallocate all of the objects.
        for (Foo* object : objects) {
            allocator.Deallocate(object);
        }
    }

    // Check large alignment
    {
        SlabAllocator<AlignedFoo> allocator(9 * sizeof(AlignedFoo));

        std::vector<AlignedFoo*> objects;
        for (int i = 0; i < 21; ++i) {
            auto* ptr = allocator.Allocate(i);
            EXPECT_TRUE(std::find(objects.begin(), objects.end(), ptr) == objects.end());
            objects.push_back(ptr);
        }

        for (int i = 0; i < 21; ++i) {
            // Check that the value is correct and hasn't been trampled.
            EXPECT_EQ(objects[i]->value, i);

            // Check that the alignment is correct.
            EXPECT_TRUE(IsPtrAligned(objects[i], 256));
        }

        // Deallocate all of the objects.
        for (AlignedFoo* object : objects) {
            allocator.Deallocate(object);
        }
    }
}

// Test that when reallocating a number of objects <= pool size, all memory is reused.
TEST(SlabAllocatorTests, ReusesFreedMemory) {
    SlabAllocator<Foo> allocator(17 * sizeof(Foo));

    // Allocate a number of objects.
    std::set<Foo*> objects;
    for (int i = 0; i < 17; ++i) {
        EXPECT_TRUE(objects.insert(allocator.Allocate(i)).second);
    }

    // Deallocate all of the objects.
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }

    std::set<Foo*> reallocatedObjects;
    // Allocate objects again. All of the pointers should be the same.
    for (int i = 0; i < 17; ++i) {
        Foo* ptr = allocator.Allocate(i);
        EXPECT_TRUE(reallocatedObjects.insert(ptr).second);
        EXPECT_TRUE(std::find(objects.begin(), objects.end(), ptr) != objects.end());
    }

    // Deallocate all of the objects.
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }
}

// Test DeleteEmptySlabs() when all slabs are empty.
TEST(SlabAllocatorTests, DeleteAllSlabs) {
    SlabAllocator<Foo> allocator(5 * sizeof(Foo));

    // Allocate a number of objects.
    std::set<Foo*> objects;
    for (int i = 0; i < 11; ++i) {
        EXPECT_TRUE(objects.insert(allocator.Allocate(i)).second);
    }
    EXPECT_EQ(allocator.CountAllocatedSlabsForTesting(), 3u);

    // Deallocate all of the objects.
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }

    // Allocate and deallocate one slab full of objects so both available and recycled lists are
    // populated.
    objects.clear();
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(objects.insert(allocator.Allocate(i)).second);
    }
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }
    EXPECT_EQ(allocator.CountAllocatedSlabsForTesting(), 3u);

    allocator.DeleteEmptySlabs();
    EXPECT_EQ(allocator.CountAllocatedSlabsForTesting(), 0u);
}

// Test DeleteEmptySlabs() when one slab is empty and one slab has allocations.
TEST(SlabAllocatorTests, DeleteSomeSlabs) {
    SlabAllocator<Foo> allocator(5 * sizeof(Foo));

    // Allocate a number of objects.
    std::set<Foo*> objects;
    for (int i = 0; i < 6; ++i) {
        EXPECT_TRUE(objects.insert(allocator.Allocate(i)).second);
    }

    // Deallocate all of the objects.
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }

    // Allocate a new object so one slab still has an allocation.
    Foo* object = allocator.Allocate(6);
    EXPECT_EQ(allocator.CountAllocatedSlabsForTesting(), 2u);

    allocator.DeleteEmptySlabs();
    EXPECT_EQ(allocator.CountAllocatedSlabsForTesting(), 1u);

    allocator.Deallocate(object);
}

// Test many allocations and deallocations. Meant to catch corner cases with partially
// empty slabs.
TEST(SlabAllocatorTests, AllocateDeallocateMany) {
    SlabAllocator<Foo> allocator(17 * sizeof(Foo));

    std::set<Foo*> objects;
    std::set<Foo*> set3;
    std::set<Foo*> set7;

    // Allocate many objects.
    for (uint32_t i = 0; i < 800; ++i) {
        Foo* object = allocator.Allocate(i);
        EXPECT_TRUE(objects.insert(object).second);

        if (i % 3 == 0) {
            set3.insert(object);
        } else if (i % 7 == 0) {
            set7.insert(object);
        }
    }

    // Deallocate every 3rd object.
    for (Foo* object : set3) {
        allocator.Deallocate(object);
        objects.erase(object);
    }

    // Allocate many more objects
    for (uint32_t i = 0; i < 800; ++i) {
        Foo* object = allocator.Allocate(i);
        EXPECT_TRUE(objects.insert(object).second);

        if (i % 7 == 0) {
            set7.insert(object);
        }
    }

    // Deallocate every 7th object from the first and second rounds of allocation.
    for (Foo* object : set7) {
        allocator.Deallocate(object);
        objects.erase(object);
    }

    // Allocate objects again
    for (uint32_t i = 0; i < 800; ++i) {
        Foo* object = allocator.Allocate(i);
        EXPECT_TRUE(objects.insert(object).second);
    }

    // Deallocate the rest of the objects
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }
}

}  // anonymous namespace
}  // namespace dawn
