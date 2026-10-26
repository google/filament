/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/PagedArenaBitset.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace utils;

template<typename T>
class EntitySetTest : public ::testing::Test {
protected:
    EntitySetTest() = default;
    ~EntitySetTest() override = default;
};

using Implementations = ::testing::Types<PagedArenaBitset>;
TYPED_TEST_SUITE(EntitySetTest, Implementations);

TYPED_TEST(EntitySetTest, Basic) {
    TypeParam set;
    EXPECT_TRUE(set.empty());

    set.add(10);
    EXPECT_FALSE(set.empty());
    EXPECT_EQ(set.size(), 1);
    EXPECT_TRUE(set[10]);

    bool found = false;
    set.forEachSetBit([&found](uint32_t const bit) {
        if (bit == 10) found = true;
    });
    EXPECT_TRUE(found);

    set.remove(10);
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
    EXPECT_FALSE(set[10]);
}

TYPED_TEST(EntitySetTest, Intersection) {
    TypeParam set1;
    TypeParam set2;

    set1.add(10);
    set1.add(20);
    set1.add(30);

    set2.add(20);
    set2.add(30);
    set2.add(40);

    set1.intersect(set2);

    std::vector<uint32_t> bits;
    set1.forEachSetBit([&bits](uint32_t const bit) {
        bits.push_back(bit);
    });

    std::sort(bits.begin(), bits.end());

    EXPECT_EQ(bits.size(), 2);
    EXPECT_EQ(bits[0], 20);
    EXPECT_EQ(bits[1], 30);
}

TYPED_TEST(EntitySetTest, UnsortedRemoval) {
    TypeParam set;
    set.add(30);
    set.add(10);
    set.add(20);
    set.add(10); // Duplicate

    set.remove(10); // Remove 10

    std::vector<uint32_t> bits;
    set.forEachSetBit([&bits](uint32_t const bit) {
        bits.push_back(bit);
    });

    std::sort(bits.begin(), bits.end());

    EXPECT_EQ(bits.size(), 2);
    EXPECT_EQ(bits[0], 20);
    EXPECT_EQ(bits[1], 30);
}

TYPED_TEST(EntitySetTest, IntersectionNonMember) {
    TypeParam set1;
    TypeParam set2;

    set1.add(10);
    set1.add(20);

    set2.add(20);
    set2.add(30);

    TypeParam const result = TypeParam::intersect(set1, set2);

    std::vector<uint32_t> bits;
    result.forEachSetBit([&bits](uint32_t const bit) {
        bits.push_back(bit);
    });

    EXPECT_EQ(bits.size(), 1);
    EXPECT_EQ(bits[0], 20);
}

TYPED_TEST(EntitySetTest, IntersectMove) {
    TypeParam set1;
    TypeParam set2;

    set1.add(10);
    set1.add(20);

    set2.add(20);
    set2.add(30);

    TypeParam result;
    set1.intersect(set2);
    std::swap(result, set1);

    EXPECT_TRUE(set1.empty()); // Because it was swapped
    EXPECT_FALSE(result.empty());

    std::vector<uint32_t> bits;
    result.forEachSetBit([&bits](uint32_t const bit) {
        bits.push_back(bit);
    });

    EXPECT_EQ(bits.size(), 1);
    EXPECT_EQ(bits[0], 20);
}

TYPED_TEST(EntitySetTest, Difference) {
    TypeParam set1;
    TypeParam set2;

    set1.add(10);
    set1.add(20);
    set1.add(30);

    set2.add(20);
    set2.add(30);
    set2.add(40);

    set1.difference(set2);

    std::vector<uint32_t> bits;
    set1.forEachSetBit([&bits](uint32_t const bit) {
        bits.push_back(bit);
    });

    EXPECT_EQ(bits.size(), 1);
    EXPECT_EQ(bits[0], 10);
}

TYPED_TEST(EntitySetTest, DifferenceNonMember) {
    TypeParam set1;
    TypeParam set2;

    set1.add(10);
    set1.add(20);

    set2.add(20);
    set2.add(30);

    TypeParam result = TypeParam::difference(set1, set2);

    std::vector<uint32_t> bits;
    result.forEachSetBit([&bits](uint32_t const bit) {
        bits.push_back(bit);
    });

    EXPECT_EQ(bits.size(), 1);
    EXPECT_EQ(bits[0], 10);

    // Verify set1 and set2 are not changed
    EXPECT_TRUE(set1[10]);
    EXPECT_TRUE(set1[20]);
    EXPECT_TRUE(set2[20]);
    EXPECT_TRUE(set2[30]);
}

TYPED_TEST(EntitySetTest, EarlyBreak) {
    TypeParam set;
    set.add(10);
    set.add(20);
    set.add(30);

    size_t count = 0;
    set.forEachSetBit([&count](uint32_t) {
        count++;
        return false; // Break immediately
    });

    EXPECT_EQ(count, 1);
}

TYPED_TEST(EntitySetTest, Iteration) {
    TypeParam bs;
    bs.add(1);
    bs.add(64);
    bs.add(127);

    std::vector<uint32_t> bits;
    bs.forEachSetBit([&bits](uint32_t const bit) {
        bits.push_back(bit);
    });

    std::sort(bits.begin(), bits.end());

    EXPECT_EQ(bits.size(), 3);
    EXPECT_EQ(bits[0], 1);
    EXPECT_EQ(bits[1], 64);
    EXPECT_EQ(bits[2], 127);
}

TEST(PagedArenaBitsetTest, ExtractTo) {
    PagedArenaBitset bs;
    bs.add(1);
    bs.add(64);
    bs.add(127);

    std::vector<uint32_t> bits;
    bs.extractTo(bits);

    EXPECT_EQ(bits.size(), 3);
    EXPECT_EQ(bits[0], 1);
    EXPECT_EQ(bits[1], 64);
    EXPECT_EQ(bits[2], 127);
}

TEST(PagedArenaBitsetTest, IntersectMulti) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;
    PagedArenaBitset bs3;

    bs1.add(1);
    bs1.add(64);
    bs1.add(127);

    bs2.add(64);
    bs2.add(127);
    bs2.add(255);

    bs3.add(127);
    bs3.add(255);
    bs3.add(511);

    // 2 arguments
    PagedArenaBitset const res2 = PagedArenaBitset::intersect(bs1, bs2);
    EXPECT_EQ(res2.size(), 2);
    EXPECT_TRUE(res2[64]);
    EXPECT_TRUE(res2[127]);

    // 3 arguments
    PagedArenaBitset const res3 = PagedArenaBitset::intersect(bs1, bs2, bs3);
    EXPECT_EQ(res3.size(), 1);
    EXPECT_TRUE(res3[127]);
}

TEST(PagedArenaBitsetTest, IntersectMultiEmpty) {
    PagedArenaBitset bs1;
    PagedArenaBitset const bs2;

    bs1.add(1);

    PagedArenaBitset const res = PagedArenaBitset::intersect(bs1, bs2);
    EXPECT_TRUE(res.empty());
}

TEST(PagedArenaBitsetTest, Defragment) {
    PagedArenaBitset bs;
    bs.add(10); // Page 0
    bs.add(5000); // Page 1 (since 5000 > 4096!)

    EXPECT_EQ(bs.size(), 2);

    // Remove bits from page 1!
    bs.remove(5000);

    EXPECT_EQ(bs.size(), 1);

    // Now page 1 is a ghost page!
    // Let's defragment!
    bs.defragment();

    EXPECT_EQ(bs.size(), 1);
    EXPECT_TRUE(bs[10]);
    EXPECT_FALSE(bs[5000]);
}

TEST(PagedArenaBitsetTest, LargeIndices) {
    PagedArenaBitset bs;
    uint32_t const largeIndex = (1U << 25) - 1; // Limit suggested by user!
    bs.add(largeIndex);
    EXPECT_TRUE(bs[largeIndex]);
    EXPECT_EQ(bs.size(), 1);

    bs.remove(largeIndex);
    EXPECT_FALSE(bs[largeIndex]);
    EXPECT_TRUE(bs.empty());
}

TEST(PagedArenaBitsetTest, DifferenceEdgeCases) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    bs1.add(10);
    bs1.add(20);

    // Subtract from itself!
    PagedArenaBitset res1 = PagedArenaBitset::difference(bs1, bs1);
    EXPECT_TRUE(res1.empty());

    // Subtract empty set!
    PagedArenaBitset res2 = PagedArenaBitset::difference(bs1, bs2);
    EXPECT_EQ(res2.size(), 2);
    EXPECT_TRUE(res2[10]);
    EXPECT_TRUE(res2[20]);

    // Subtract superset!
    bs2.add(10);
    bs2.add(20);
    bs2.add(30);
    PagedArenaBitset res3 = PagedArenaBitset::difference(bs1, bs2);
    EXPECT_TRUE(res3.empty());
}

TEST(PagedArenaBitsetTest, Clear) {
    PagedArenaBitset bs;
    bs.add(10);
    bs.add(20);
    EXPECT_FALSE(bs.empty());
    EXPECT_EQ(bs.size(), 2);

    bs.clear();
    EXPECT_TRUE(bs.empty());
    EXPECT_EQ(bs.size(), 0);
    EXPECT_FALSE(bs[10]);
}

TEST(PagedArenaBitsetTest, Clone) {
    PagedArenaBitset bs1;
    bs1.add(10);
    bs1.add(20);

    PagedArenaBitset bs2 = bs1.clone();
    EXPECT_EQ(bs2.size(), 2);
    EXPECT_TRUE(bs2[10]);
    EXPECT_TRUE(bs2[20]);

    bs2.add(30);
    EXPECT_FALSE(bs1[30]); // Verify deep copy!
}

TEST(PagedArenaBitsetTest, IntersectSize) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;
    PagedArenaBitset bs3;

    bs1.add(1);
    bs1.add(64);
    bs1.add(127);

    bs2.add(64);
    bs2.add(127);
    bs2.add(255);

    bs3.add(127);
    bs3.add(255);
    bs3.add(511);

    // 2 arguments
    uint32_t size2 = PagedArenaBitset::intersectSize(bs1, bs2);
    EXPECT_EQ(size2, 2);

    // 3 arguments
    uint32_t size3 = PagedArenaBitset::intersectSize(bs1, bs2, bs3);
    EXPECT_EQ(size3, 1);
}

TEST(PagedArenaBitsetTest, SelfIntersection) {
    PagedArenaBitset bs;
    bs.add(10);
    bs.add(20);

    bs.intersect(bs);
    EXPECT_EQ(bs.size(), 2);
    EXPECT_TRUE(bs[10]);
    EXPECT_TRUE(bs[20]);
}

TEST(PagedArenaBitsetTest, MaxIndex) {
    PagedArenaBitset bs;
    uint32_t const maxIndex = (1U << 27) - 1;
    bs.add(maxIndex);
    EXPECT_TRUE(bs[maxIndex]);
    EXPECT_EQ(bs.size(), 1);

    bs.remove(maxIndex);
    EXPECT_FALSE(bs[maxIndex]);
    EXPECT_TRUE(bs.empty());
}

TEST(PagedArenaBitsetTest, IsSubset) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    // Empty sets!
    EXPECT_TRUE(PagedArenaBitset::isSubset(bs1, bs2));

    bs1.add(10);
    bs1.add(20);

    // bs1 is NOT subset of empty bs2!
    EXPECT_FALSE(PagedArenaBitset::isSubset(bs1, bs2));

    // Empty bs2 is subset of bs1!
    EXPECT_TRUE(PagedArenaBitset::isSubset(bs2, bs1));

    bs2.add(10);
    bs2.add(20);

    // Equal sets!
    EXPECT_TRUE(PagedArenaBitset::isSubset(bs1, bs2));
    EXPECT_TRUE(PagedArenaBitset::isSubset(bs2, bs1));

    bs2.add(30);

    // bs1 is subset of bs2!
    EXPECT_TRUE(PagedArenaBitset::isSubset(bs1, bs2));

    // bs2 is NOT subset of bs1!
    EXPECT_FALSE(PagedArenaBitset::isSubset(bs2, bs1));

    bs1.add(40);

    // Disjoint parts!
    EXPECT_FALSE(PagedArenaBitset::isSubset(bs1, bs2));
}

TEST(PagedArenaBitsetTest, IsStrictSubset) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    // Both empty! Not strict subset!
    EXPECT_FALSE(PagedArenaBitset::isStrictSubset(bs1, bs2));

    bs1.add(10);
    bs1.add(20);

    // Empty bs2 is strict subset of non-empty bs1!
    EXPECT_TRUE(PagedArenaBitset::isStrictSubset(bs2, bs1));

    bs2.add(10);
    bs2.add(20);

    // Equal sets! Not strict subset!
    EXPECT_FALSE(PagedArenaBitset::isStrictSubset(bs1, bs2));
    EXPECT_FALSE(PagedArenaBitset::isStrictSubset(bs2, bs1));

    bs2.add(30);

    // bs1 is strict subset of bs2!
    EXPECT_TRUE(PagedArenaBitset::isStrictSubset(bs1, bs2));

    // bs2 is NOT strict subset of bs1!
    EXPECT_FALSE(PagedArenaBitset::isStrictSubset(bs2, bs1));
}

TEST(PagedArenaBitsetTest, Merge) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    bs1.add(10);
    bs1.add(20);

    bs2.add(20);
    bs2.add(30);

    // In-place merge!
    PagedArenaBitset bs3 = bs1.clone();
    bs3.merge(bs2);
    EXPECT_EQ(bs3.size(), 3);
    EXPECT_TRUE(bs3[10]);
    EXPECT_TRUE(bs3[20]);
    EXPECT_TRUE(bs3[30]);

    // Static merge!
    PagedArenaBitset res = PagedArenaBitset::merge(bs1, bs2);
    EXPECT_EQ(res.size(), 3);
    EXPECT_TRUE(res[10]);
    EXPECT_TRUE(res[20]);
    EXPECT_TRUE(res[30]);

    // Variadic merge!
    PagedArenaBitset bs4;
    bs4.add(40);
    PagedArenaBitset res4 = PagedArenaBitset::merge(bs1, bs2, bs4);
    EXPECT_EQ(res4.size(), 4);
    EXPECT_TRUE(res4[10]);
    EXPECT_TRUE(res4[20]);
    EXPECT_TRUE(res4[30]);
    EXPECT_TRUE(res4[40]);
}

TEST(PagedArenaBitsetTest, MergeSize) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    bs1.add(10);
    bs1.add(20);

    bs2.add(20);
    bs2.add(30);

    EXPECT_EQ(PagedArenaBitset::mergeSize(bs1, bs2), 3);

    PagedArenaBitset bs3;
    bs3.add(40);
    EXPECT_EQ(PagedArenaBitset::mergeSize(bs1, bs2, bs3), 4);
}

TEST(PagedArenaBitsetTest, MergeGhostBitBug) {
    PagedArenaBitset a;
    PagedArenaBitset b;
    PagedArenaBitset c;

    a.add(100);
    a.remove(100); // A has ghost page at 0!

    // B and C are empty!

    PagedArenaBitset result;
    // Use 3 arguments to force variadic path
    PagedArenaBitset::merge(&result, a, b, c);

    // If the bug exists, result claims to have bits but is empty!
    // And forEachSetBit might crash!

    size_t count = 0;
    result.forEachSetBit([&](uint32_t bit) {
        count++;
    });

    EXPECT_EQ(count, 0);
    EXPECT_EQ(result.size(), 0);
}

// The tests below cover stale directory entries. clear() and the static operations (which call
// out->clear()) reset the masks but not the directory, and copyFrom() copies the directory
// verbatim. Only the masks say which pages exist; reading a stale entry instead aliases two
// pages onto the same arena slot, or indexes past the end of the arena.

static std::vector<uint32_t> toVector(PagedArenaBitset const& bs) {
    std::vector<uint32_t> bits;
    bs.extractTo(bits);
    return bits;
}

TEST(PagedArenaBitsetTest, MergeAfterClearIgnoresStaleDirectory) {
    PagedArenaBitset a;
    a.add(5000);    // page 1 -> arena[0]
    a.clear();      // the directory entry for page 1 still says arena[0]
    a.add(10);      // page 0 -> arena[0]

    PagedArenaBitset b;
    b.add(5001);    // page 1

    a.merge(b);
    EXPECT_EQ(toVector(a), (std::vector<uint32_t>{ 10, 5001 }));
    EXPECT_EQ(a.size(), 2);
}

TEST(PagedArenaBitsetTest, MergeAfterCopyFromIgnoresStaleDirectory) {
    // This is the sequence FScene uses: the pending set is cleared every frame, then copied
    // into an empty set, which is then merged with another set.
    PagedArenaBitset pending;
    pending.add(4200);  // page 1 -> arena[0]
    pending.clear();
    pending.add(100);   // page 0 -> arena[0]

    PagedArenaBitset destroyed;
    destroyed.merge(pending);   // empty, so this is copyFrom(pending)

    PagedArenaBitset removed;
    removed.add(4300);  // page 1
    destroyed.merge(removed);

    EXPECT_EQ(toVector(destroyed), (std::vector<uint32_t>{ 100, 4300 }));
    EXPECT_EQ(destroyed.size(), 2);
}

TEST(PagedArenaBitsetTest, MergeStaleDirectoryPastEndOfArena) {
    PagedArenaBitset pending;
    pending.add(100);   // page 0 -> arena[0]
    pending.add(5000);  // page 1 -> arena[1]
    pending.clear();
    pending.add(200);   // page 0 -> arena[0]; the entry for page 1 still says arena[1]

    PagedArenaBitset destroyed;
    destroyed.merge(pending);   // copyFrom(): the arena has one page, the stale entry is copied

    PagedArenaBitset removed;
    removed.add(4300);  // page 1
    destroyed.merge(removed);   // used to write arena[1], one page past the end

    // A new page reuses arena[1]; with the bug it wiped 4300 and aliased page 1 onto page 2.
    destroyed.add(9000);
    EXPECT_EQ(toVector(destroyed), (std::vector<uint32_t>{ 200, 4300, 9000 }));
    EXPECT_EQ(destroyed.size(), 3);
}

TEST(PagedArenaBitsetTest, MergeAfterStaticOperationIgnoresStaleDirectory) {
    PagedArenaBitset a;
    a.add(5000);    // page 1
    PagedArenaBitset b;
    b.add(5000);

    PagedArenaBitset out;
    PagedArenaBitset::intersect(&out, a, b);    // page 1 -> arena[0]
    PagedArenaBitset c;
    c.add(10);      // page 0
    PagedArenaBitset::intersect(&out, c, c);    // clears out, page 0 -> arena[0]

    PagedArenaBitset d;
    d.add(5001);    // page 1
    out.merge(d);
    EXPECT_EQ(toVector(out), (std::vector<uint32_t>{ 10, 5001 }));
}

TEST(PagedArenaBitsetTest, MergeSizeIgnoresStaleDirectory) {
    PagedArenaBitset a;
    a.add(5000);    // page 1 -> arena[0]
    a.clear();
    a.add(100);     // page 0 -> arena[0]

    PagedArenaBitset b;
    b.add(4300);    // page 1

    EXPECT_EQ(PagedArenaBitset::mergeSize(a, b), 2);
    EXPECT_EQ(PagedArenaBitset::mergeSize(b, a), 2);
}

TEST(PagedArenaBitsetTest, StaticMergeIgnoresStaleDirectory) {
    PagedArenaBitset a;
    a.add(5000);    // page 1 -> arena[0]
    a.clear();
    a.add(100);     // page 0 -> arena[0]

    PagedArenaBitset b;
    b.add(4300);    // page 1

    EXPECT_EQ(toVector(PagedArenaBitset::merge(a, b)), (std::vector<uint32_t>{ 100, 4300 }));
    EXPECT_EQ(toVector(PagedArenaBitset::merge(b, a)), (std::vector<uint32_t>{ 100, 4300 }));
}

TEST(PagedArenaBitsetTest, CopyFromDoesNotKeepFreePages) {
    PagedArenaBitset a;
    a.add(10);      // page 0 -> arena[0]
    a.add(5000);    // page 1 -> arena[1]
    PagedArenaBitset drop;
    drop.add(5000);
    a.difference(drop);     // page 1 becomes empty, so arena[1] goes on a's free list

    PagedArenaBitset src;
    src.add(20);    // page 0 -> arena[0]
    src.add(4200);  // page 1 -> arena[1]
    a.copyFrom(src);

    // A new page must not reuse arena[1], which now holds page 1.
    a.add(9000);
    EXPECT_EQ(toVector(a), (std::vector<uint32_t>{ 20, 4200, 9000 }));
    EXPECT_EQ(a.size(), 3);
}

TEST(PagedArenaBitsetTest, Swap) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    bs1.add(10);
    bs1.add(20);

    bs2.add(30);

    // Member swap
    bs1.swap(bs2);
    EXPECT_EQ(bs1.size(), 1);
    EXPECT_TRUE(bs1[30]);
    EXPECT_FALSE(bs1[10]);

    EXPECT_EQ(bs2.size(), 2);
    EXPECT_TRUE(bs2[10]);
    EXPECT_TRUE(bs2[20]);

    // Free function swap
    using std::swap;
    swap(bs1, bs2);
    EXPECT_EQ(bs1.size(), 2);
    EXPECT_TRUE(bs1[10]);
    EXPECT_TRUE(bs1[20]);

    EXPECT_EQ(bs2.size(), 1);
    EXPECT_TRUE(bs2[30]);
}

TEST(PagedArenaBitsetTest, IsSubsetGhostPageBug) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    // Add a shared valid bit so subset is not empty
    bs1.add(10);
    bs2.add(10);

    // Force a ghost page in bs1 at a distinct location
    bs1.add(10000);
    bs1.remove(10000);

    // bs1 has [10], bs2 has [10].
    // bs1 is mathematically a subset of bs2.
    EXPECT_TRUE(PagedArenaBitset::isSubset(bs1, bs2));
}

TEST(PagedArenaBitsetTest, FetchAddRemoveReturnValues) {
    PagedArenaBitset bs;

    // fetchAdd on brand new bit should return false
    EXPECT_FALSE(bs.fetchAdd(42));
    // fetchAdd on duplicate bit should return true
    EXPECT_TRUE(bs.fetchAdd(42));

    EXPECT_EQ(bs.size(), 1);

    // fetchRemove on active bit should return true
    EXPECT_TRUE(bs.fetchRemove(42));
    // fetchRemove on dead bit should return false
    EXPECT_FALSE(bs.fetchRemove(42));

    EXPECT_TRUE(bs.empty());
}

TEST(PagedArenaBitsetTest, MemberQueryOperators) {
    PagedArenaBitset bs1;
    PagedArenaBitset bs2;

    // Both empty sets boundary check
    EXPECT_TRUE(bs1.isSubsetOf(bs2));
    EXPECT_FALSE(bs1.isStrictSubsetOf(bs2));
    EXPECT_TRUE(bs1.isSupersetOf(bs2));
    EXPECT_FALSE(bs1.isStrictSupersetOf(bs2));

    bs1.add(10);
    // Non-empty bs1 vs empty bs2
    EXPECT_FALSE(bs1.isSubsetOf(bs2));
    EXPECT_FALSE(bs1.isStrictSubsetOf(bs2));
    EXPECT_TRUE(bs1.isSupersetOf(bs2));
    EXPECT_TRUE(bs1.isStrictSupersetOf(bs2));

    // Empty bs2 vs non-empty bs1
    EXPECT_TRUE(bs2.isSubsetOf(bs1));
    EXPECT_TRUE(bs2.isStrictSubsetOf(bs1));
    EXPECT_FALSE(bs2.isSupersetOf(bs1));
    EXPECT_FALSE(bs2.isStrictSupersetOf(bs1));

    bs1.add(20);
    bs2.add(10);
    bs2.add(20);

    // Identical sets
    EXPECT_TRUE(bs1.isSubsetOf(bs2));
    EXPECT_FALSE(bs1.isStrictSubsetOf(bs2));
    EXPECT_TRUE(bs1.isSupersetOf(bs2));
    EXPECT_FALSE(bs1.isStrictSupersetOf(bs2));

    // bs1 is subset of bs2
    bs2.add(30);
    EXPECT_TRUE(bs1.isSubsetOf(bs2));
    EXPECT_TRUE(bs1.isStrictSubsetOf(bs2));
    EXPECT_FALSE(bs1.isSupersetOf(bs2));
    EXPECT_FALSE(bs1.isStrictSupersetOf(bs2));

    // bs2 is superset of bs1
    EXPECT_FALSE(bs2.isSubsetOf(bs1));
    EXPECT_FALSE(bs2.isStrictSubsetOf(bs1));
    EXPECT_TRUE(bs2.isSupersetOf(bs1));
    EXPECT_TRUE(bs2.isStrictSupersetOf(bs1));
}

TEST(PagedArenaBitsetTest, MoveConstructorAndAssignment) {
    PagedArenaBitset source;
    source.add(100);
    source.add(200);

    // Move Constructor
    PagedArenaBitset target(std::move(source));
    EXPECT_EQ(target.size(), 2);
    EXPECT_TRUE(target[100]);
    EXPECT_TRUE(target[200]);

    // Move Assignment
    PagedArenaBitset assignTarget;
    assignTarget.add(50);
    assignTarget = std::move(target);

    EXPECT_EQ(assignTarget.size(), 2);
    EXPECT_TRUE(assignTarget[100]);
    EXPECT_TRUE(assignTarget[200]);
    EXPECT_FALSE(assignTarget[50]); // Old data should be dropped
}

TEST(PagedArenaBitsetTest, DefragmentEdgeCases) {
    PagedArenaBitset emptyBs;
    // Should be completely safe on empty sets
    emptyBs.defragment();
    EXPECT_TRUE(emptyBs.empty());

    PagedArenaBitset bs;
    bs.add(10);    // Page 0
    bs.add(5000);  // Page 1
    bs.add(10000); // Page 2

    bs.remove(5000);  // Page 1 ghost
    bs.remove(10000); // Page 2 ghost

    // Defragmenting multiple consecutive ghosts
    bs.defragment();
    EXPECT_EQ(bs.size(), 1);
    EXPECT_TRUE(bs[10]);
}

TEST(PagedArenaBitsetTest, ExchangeAndSelfMove) {
    // Test self move-assignment
    PagedArenaBitset selfMoveBs;
    selfMoveBs.add(42);

    // Force self-move via reference to avoid compiler warnings
    PagedArenaBitset& ref = selfMoveBs;
    selfMoveBs = std::move(ref);

    EXPECT_EQ(selfMoveBs.size(), 1);
    EXPECT_TRUE(selfMoveBs[42]);
}

TEST(PagedArenaBitsetTest, CopyFrom) {
    PagedArenaBitset source;
    source.add(10);
    source.add(5000);

    PagedArenaBitset target;
    target.add(20);

    // Perform copyFrom
    target.copyFrom(source);
    EXPECT_EQ(target.size(), 2);
    EXPECT_TRUE(target[10]);
    EXPECT_TRUE(target[5000]);
    EXPECT_FALSE(target[20]); // Old target data should be wiped

    // Source should be unaffected
    EXPECT_EQ(source.size(), 2);
    EXPECT_TRUE(source[10]);
    EXPECT_TRUE(source[5000]);

    // Self-copy safety check
    target.copyFrom(target);
    EXPECT_EQ(target.size(), 2);
    EXPECT_TRUE(target[10]);
    EXPECT_TRUE(target[5000]);
}

TEST(PagedArenaBitsetTest, PopSetBits) {
    PagedArenaBitset bitset;
    for (uint32_t i = 0; i < 10; ++i) {
        bitset.add(i * 10);
    }
    EXPECT_EQ(bitset.size(), 10);

    uint32_t processedCount = 0;
    bitset.popSetBits([&](uint32_t bit) {
        processedCount++;
        if (processedCount == 5) {
            return false; // Stop
        }
        return true; // Pop
    });

    EXPECT_EQ(processedCount, 5);
    EXPECT_EQ(bitset.size(), 6);

    uint32_t remainderCount = 0;
    bitset.popSetBits([&](uint32_t bit) {
        remainderCount++;
        return true;
    });

    EXPECT_EQ(remainderCount, 6);
    EXPECT_TRUE(bitset.empty());
}

TEST(PagedArenaBitsetTest, BatchAddSliceAndInvariants) {
    auto verifyEquivalent = [](PagedArenaBitset const& actual, PagedArenaBitset const& expected) {
        EXPECT_EQ(actual.size(), expected.size());
        EXPECT_EQ(actual.empty(), expected.empty());
        EXPECT_TRUE(actual.isSubsetOf(expected));
        EXPECT_TRUE(expected.isSubsetOf(actual));

        std::vector<uint32_t> actualBits;
        actual.forEachSetBit([&](uint32_t const b) {
            EXPECT_TRUE(actual[b]);
            actualBits.push_back(b);
        });
        std::vector<uint32_t> expectedBits;
        expected.forEachSetBit([&](uint32_t const b) {
            expectedBits.push_back(b);
        });
        EXPECT_EQ(actualBits, expectedBits);
        EXPECT_EQ(actualBits.size(), actual.size());
    };

    // 1. Empty slice on fresh and populated bitsets (must not create ghost bits or alter size)
    {
        PagedArenaBitset bs;
        bs.add(Slice<const uint32_t>{});
        EXPECT_TRUE(bs.empty());
        EXPECT_EQ(bs.size(), 0u);
        uint32_t traversed = 0;
        bs.forEachSetBit([&](uint32_t) { traversed++; });
        EXPECT_EQ(traversed, 0u);

        bs.add(42u);
        bs.add(Slice<const uint32_t>{});
        EXPECT_EQ(bs.size(), 1u);
        EXPECT_TRUE(bs[42u]);
    }

    // 2. Same-page cache hits, boundary words/bits (0, 63, 64, 4095), and intra-batch duplicates
    {
        std::vector<uint32_t> const batch = {
            0u, 1u, 63u, 64u, 65u, 127u, 128u, 2048u, 4095u,
            // Duplicates within the same batch and same/different words
            0u, 63u, 64u, 4095u, 1u, 2048u
        };
        PagedArenaBitset batchBs;
        batchBs.add({ batch.data(), batch.size() });

        PagedArenaBitset scalarBs;
        for (uint32_t const id : batch) {
            scalarBs.add(id);
        }
        verifyEquivalent(batchBs, scalarBs);
    }

    // 3. Multi-page & alternating-page stress across all 8 L0 Master Mask words (2^27 domain),
    // forcing multiple mArena vector reallocations and cachedDirIdx thrashing + hits
    {
        std::vector<uint32_t> batch;
        // Span all 8 master words (each master word covers 64 * 64 * 4096 = 16,777,216 indices)
        for (uint32_t m = 0; m < 8; ++m) {
            uint32_t const base = m * (1u << 24);
            // Multiple pages per master word, multiple words per page
            for (uint32_t p = 0; p < 40; ++p) {
                uint32_t const pageBase = base + p * 4096u;
                batch.push_back(pageBase + 0u);
                batch.push_back(pageBase + 63u);
                batch.push_back(pageBase + 64u + (p % 64u));
                batch.push_back(pageBase + 4095u);
            }
        }
        // Interleave alternating pages and duplicates to test cachedDirIdx transitions
        for (size_t i = 0, n = batch.size(); i < n; i += 7) {
            batch.push_back(batch[i]);
            batch.push_back(batch[n - 1 - i]);
        }
        // Include highest valid index in the 2^27 domain
        batch.push_back((1u << PagedArenaBitset::DOMAIN_BITS) - 1u);

        PagedArenaBitset batchBs;
        batchBs.add({ batch.data(), batch.size() });

        PagedArenaBitset scalarBs;
        for (uint32_t const id : batch) {
            scalarBs.add(id);
        }
        verifyEquivalent(batchBs, scalarBs);

        // Also verify defragment() and intersect() invariants on the batch-constructed bitset
        batchBs.defragment();
        verifyEquivalent(batchBs, scalarBs);
    }

    // 4. The Garbage Contract: Recycled pages from mFreePages must be cleanly zeroed
    // before partial writes in add(Slice), and overlapping pre-existing bits must not double-count
    {
        PagedArenaBitset bs;
        PagedArenaBitset toRemove;
        // Allocate and heavily populate 16 pages
        std::vector<uint32_t> initial;
        for (uint32_t p = 0; p < 16; ++p) {
            for (uint32_t w = 0; w < 64; ++w) {
                uint32_t const idx = p * 4096u + w * 64u + (w % 64u);
                initial.push_back(idx);
                if (p < 12) {
                    toRemove.add(idx);
                }
            }
        }
        bs.add({ initial.data(), initial.size() });
        // Free pages 0..11 into mFreePages via difference()
        bs.difference(toRemove);

        // Now add a batch that re-allocates from mFreePages (pages 0..11), touches surviving
        // pages (12..15) with both existing and new bits, and allocates brand new pages (16..20)
        std::vector<uint32_t> secondBatch;
        for (uint32_t p = 0; p <= 20; ++p) {
            secondBatch.push_back(p * 4096u + 7u);
            secondBatch.push_back(p * 4096u + 0u); // Overlaps existing bit on pages 12..15
        }
        bs.add({ secondBatch.data(), secondBatch.size() });

        PagedArenaBitset expected;
        for (uint32_t const id : initial) {
            if (!toRemove[id]) {
                expected.add(id);
            }
        }
        for (uint32_t const id : secondBatch) {
            expected.add(id);
        }
        verifyEquivalent(bs, expected);
    }
}

TEST(PagedArenaBitsetTest, BatchAddFilteredSlice) {
    PagedArenaBitset filter;
    filter.add(10u);
    filter.add(4096u + 5u);
    filter.add(8192u + 77u);

    std::vector<uint32_t> const batch = {
        3u, 10u, 99u, 4096u + 5u, 4096u + 6u, 8192u + 77u, 16384u + 1u, 10u
    };

    PagedArenaBitset bs;
    bs.add({ batch.data(), batch.size() }, &filter);

    EXPECT_EQ(bs.size(), 3u);
    EXPECT_TRUE(bs[10u]);
    EXPECT_TRUE(bs[4096u + 5u]);
    EXPECT_TRUE(bs[8192u + 77u]);
    EXPECT_FALSE(bs[3u]);
    EXPECT_FALSE(bs[99u]);
    EXPECT_FALSE(bs[4096u + 6u]);
    EXPECT_FALSE(bs[16384u + 1u]);

    // Empty filter is a no-op
    PagedArenaBitset emptyFilter;
    PagedArenaBitset bsEmpty;
    bsEmpty.add({ batch.data(), batch.size() }, &emptyFilter);
    EXPECT_TRUE(bsEmpty.empty());
}

TEST(PagedArenaBitsetTest, OperatorSubscriptInvariants) {
    PagedArenaBitset bs;
    constexpr uint32_t kMaxIndex = (1u << PagedArenaBitset::DOMAIN_BITS) - 1u;

    // Unallocated pages across the entire domain return false
    EXPECT_FALSE(bs[0u]);
    EXPECT_FALSE(bs[63u]);
    EXPECT_FALSE(bs[64u]);
    EXPECT_FALSE(bs[4095u]);
    EXPECT_FALSE(bs[4096u]);
    EXPECT_FALSE(bs[kMaxIndex]);

    // Boundary bits within pages and words
    std::vector<uint32_t> const testBits = {
        0u, 1u, 63u, 64u, 65u, 4095u, 4096u, (1u << 20) + 123u, kMaxIndex
    };
    for (uint32_t const b : testBits) {
        bs.add(b);
    }

    for (uint32_t const b : testBits) {
        EXPECT_TRUE(bs[b]);
    }
    // Neighboring unset bits in allocated pages and unallocated pages
    EXPECT_FALSE(bs[2u]);
    EXPECT_FALSE(bs[62u]);
    EXPECT_FALSE(bs[66u]);
    EXPECT_FALSE(bs[4094u]);
    EXPECT_FALSE(bs[4097u]);
    EXPECT_FALSE(bs[kMaxIndex - 1u]);

    // Removing a bit creating a ghost page still returns false
    bs.remove(4096u);
    EXPECT_FALSE(bs[4096u]);

    // Clearing resets summary masks (while leaving stale directory entries) and must return false
    bs.clear();
    for (uint32_t const b : testBits) {
        EXPECT_FALSE(bs[b]);
    }
}
