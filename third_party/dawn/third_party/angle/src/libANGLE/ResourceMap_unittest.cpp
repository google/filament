//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourceMap_unittest:
//   Unit tests for the ResourceMap template class.
//

#include <gtest/gtest.h>
#include <map>

#include "libANGLE/ResourceMap.h"

using namespace gl;

namespace gl
{
template <>
inline GLuint GetIDValue(int id)
{
    return id;
}
template <>
inline GLuint GetIDValue(unsigned int id)
{
    return id;
}
}  // namespace gl

namespace
{
// The resourceMap class uses a lock for "unsigned int" types to support this unit test.
using LocklessType = int;
using LockedType   = unsigned int;

template <typename T>
void AssignAndErase()
{
    constexpr size_t kSize = 300;
    ResourceMap<size_t, T> resourceMap;
    std::vector<size_t> objects(kSize, 1);
    for (size_t index = 0; index < kSize; ++index)
    {
        resourceMap.assign(index + 1, &objects[index]);
    }

    for (size_t index = 0; index < kSize; ++index)
    {
        size_t *found = nullptr;
        ASSERT_TRUE(resourceMap.erase(index + 1, &found));
        ASSERT_EQ(&objects[index], found);
    }

    ASSERT_TRUE(UnsafeResourceMapIter(resourceMap).empty());
}

// Tests assigning slots in the map and then deleting elements.
TEST(ResourceMapTest, AssignAndEraseLockless)
{
    AssignAndErase<LocklessType>();
}
// Tests assigning slots in the map and then deleting elements.
TEST(ResourceMapTest, AssignAndEraseLocked)
{
    AssignAndErase<LockedType>();
}

template <typename T>
void AssignAndClear()
{
    constexpr size_t kSize = 280;
    ResourceMap<size_t, T> resourceMap;
    std::vector<size_t> objects(kSize, 1);
    for (size_t index = 0; index < kSize; ++index)
    {
        resourceMap.assign(index + 1, &objects[index]);
    }

    resourceMap.clear();
    ASSERT_TRUE(UnsafeResourceMapIter(resourceMap).empty());
}

// Tests assigning slots in the map and then using clear() to free it.
TEST(ResourceMapTest, AssignAndClearLockless)
{
    AssignAndClear<LocklessType>();
}
// Tests assigning slots in the map and then using clear() to free it.
TEST(ResourceMapTest, AssignAndClearLocked)
{
    AssignAndClear<LockedType>();
}

template <typename T>
void BigGrowth()
{
    constexpr size_t kSize = 8;

    ResourceMap<size_t, T> resourceMap;
    std::vector<size_t> objects;

    for (size_t index = 0; index < kSize; ++index)
    {
        objects.push_back(index);
    }

    // Assign a large value.
    constexpr size_t kLargeIndex = 128;
    objects.push_back(kLargeIndex);

    for (size_t &object : objects)
    {
        resourceMap.assign(object, &object);
    }

    for (size_t object : objects)
    {
        size_t *found = nullptr;
        ASSERT_TRUE(resourceMap.erase(object, &found));
        ASSERT_EQ(object, *found);
    }

    ASSERT_TRUE(UnsafeResourceMapIter(resourceMap).empty());
}

// Tests growing a map more than double the size.
TEST(ResourceMapTest, BigGrowthLockless)
{
    BigGrowth<LocklessType>();
}
// Tests growing a map more than double the size.
TEST(ResourceMapTest, BigGrowthLocked)
{
    BigGrowth<LockedType>();
}

template <typename T>
void QueryUnassigned()
{
    constexpr size_t kSize        = 8;
    constexpr T kZero             = 0;
    constexpr T kIdInFlatRange    = 10;
    constexpr T kIdOutOfFlatRange = 500;

    ResourceMap<size_t, T> resourceMap;
    std::vector<size_t> objects;

    for (size_t index = 0; index < kSize; ++index)
    {
        objects.push_back(index);
    }

    ASSERT_FALSE(resourceMap.contains(kZero));
    ASSERT_EQ(nullptr, resourceMap.query(kZero));
    ASSERT_FALSE(resourceMap.contains(kIdOutOfFlatRange));
    ASSERT_EQ(nullptr, resourceMap.query(kIdOutOfFlatRange));

    for (size_t &object : objects)
    {
        resourceMap.assign(object, &object);
    }

    ASSERT_FALSE(UnsafeResourceMapIter(resourceMap).empty());

    for (size_t &object : objects)
    {
        ASSERT_TRUE(resourceMap.contains(object));
        ASSERT_EQ(&object, resourceMap.query(object));
    }

    ASSERT_FALSE(resourceMap.contains(kIdInFlatRange));
    ASSERT_EQ(nullptr, resourceMap.query(kIdInFlatRange));
    ASSERT_FALSE(resourceMap.contains(kIdOutOfFlatRange));
    ASSERT_EQ(nullptr, resourceMap.query(kIdOutOfFlatRange));

    for (size_t object : objects)
    {
        size_t *found = nullptr;
        ASSERT_TRUE(resourceMap.erase(object, &found));
        ASSERT_EQ(object, *found);
    }

    ASSERT_TRUE(UnsafeResourceMapIter(resourceMap).empty());

    ASSERT_FALSE(resourceMap.contains(kZero));
    ASSERT_EQ(nullptr, resourceMap.query(kZero));
    ASSERT_FALSE(resourceMap.contains(kIdOutOfFlatRange));
    ASSERT_EQ(nullptr, resourceMap.query(kIdOutOfFlatRange));
}

// Tests querying unassigned or erased values.
TEST(ResourceMapTest, QueryUnassignedLockless)
{
    QueryUnassigned<LocklessType>();
}
// Tests querying unassigned or erased values.
TEST(ResourceMapTest, QueryUnassignedLocked)
{
    QueryUnassigned<LockedType>();
}

void ConcurrentAccess(size_t iterations, size_t idCycleSize)
{
    if (std::is_same_v<ResourceMapMutex, angle::NoOpMutex>)
    {
        GTEST_SKIP() << "Test skipped: Locking is disabled in build.";
    }

    constexpr size_t kThreadCount = 13;

    ResourceMap<size_t, LockedType> resourceMap;

    std::array<std::thread, kThreadCount> threads;
    std::array<std::map<LockedType, size_t>, kThreadCount> insertedIds;

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i] = std::thread([&, i]() {
            // Each thread manipulates a different set of ids.  The resource map guarantees that the
            // data structure itself is thread-safe, not accesses to the same id.
            for (size_t j = 0; j < iterations; ++j)
            {
                const LockedType id = (j % (idCycleSize / kThreadCount)) * kThreadCount + i;

                ASSERT_LE(id, 0xFFFFu);
                ASSERT_LE(j, 0xFFFFu);
                const size_t value = id | j << 16;

                size_t *valuePtr = reinterpret_cast<size_t *>(value);

                const size_t *queryResult = resourceMap.query(id);
                const bool containsResult = resourceMap.contains(id);

                const bool expectContains = insertedIds[i].count(id) > 0;
                if (expectContains)
                {
                    EXPECT_TRUE(containsResult);
                    const LockedType queryResultInt =
                        static_cast<LockedType>(reinterpret_cast<size_t>(queryResult) & 0xFFFF);
                    const size_t queryResultIteration = reinterpret_cast<size_t>(queryResult) >> 16;
                    EXPECT_EQ(queryResultInt, id);
                    EXPECT_LT(queryResultIteration, j);

                    size_t *erasedValue = nullptr;
                    const bool erased   = resourceMap.erase(id, &erasedValue);

                    EXPECT_TRUE(erased);
                    EXPECT_EQ(erasedValue, queryResult);

                    insertedIds[i].erase(id);
                }
                else
                {
                    EXPECT_FALSE(containsResult);
                    EXPECT_EQ(queryResult, nullptr);

                    resourceMap.assign(id, valuePtr);
                    EXPECT_TRUE(resourceMap.contains(id));

                    ASSERT_TRUE(insertedIds[i].count(id) == 0);
                    insertedIds[i][id] = value;
                }
            }
        });
    }

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i].join();
    }

    // Verify that every value that is expected to be there is actually there
    std::map<size_t, size_t> allIds;
    size_t allIdsPrevSize = 0;

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        // Merge all the sets together.  The sets are disjoint, which is verified by the ASSERT_EQ.
        allIds.insert(insertedIds[i].begin(), insertedIds[i].end());
        ASSERT_EQ(allIds.size(), allIdsPrevSize + insertedIds[i].size());
        allIdsPrevSize = allIds.size();

        // Make sure every id that is expected to be there is actually there.
        for (auto &idValue : insertedIds[i])
        {
            EXPECT_TRUE(resourceMap.contains(idValue.first));
            EXPECT_EQ(resourceMap.query(idValue.first), reinterpret_cast<size_t *>(idValue.second));
        }
    }

    // Verify that every value that is NOT expected to be there isn't actually there
    for (auto &idValue : UnsafeResourceMapIter(resourceMap))
    {
        EXPECT_TRUE(allIds.count(idValue.first) == 1);
        EXPECT_EQ(idValue.second, reinterpret_cast<size_t *>(allIds[idValue.first]));
    }

    resourceMap.clear();
}

// Tests that concurrent access to thread-safe resource maps works for small ids that are mostly in
// the flat map range.
TEST(ResourceMapTest, ConcurrentAccessSmallIds)
{
    ConcurrentAccess(50'000, 128);
}
// Tests that concurrent access to thread-safe resource maps works for a wider range of ids.
TEST(ResourceMapTest, ConcurrentAccessLargeIds)
{
    ConcurrentAccess(10'000, 20'000);
}
}  // anonymous namespace
