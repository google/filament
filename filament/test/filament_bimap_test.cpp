/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <gtest/gtest.h>

#include "../src/Bimap.h"

#include <string>
#include <vector>

using namespace filament;

// Test basic insertion, lookup by key and value, and emptiness.
TEST(BimapTest, InsertAndFind) {
    Bimap<int, std::string> bimap;

    EXPECT_TRUE(bimap.empty());

    bimap.insert(10, "ten");
    bimap.insert(20, "twenty");

    EXPECT_FALSE(bimap.empty());

    // Test forward lookup (Key -> Value)
    auto f_it_10 = bimap.find(10);
    ASSERT_NE(f_it_10, bimap.end());
    EXPECT_EQ(*f_it_10->first.pKey, 10);
    EXPECT_EQ(f_it_10->second, "ten");

    auto f_it_20 = bimap.find(20);
    ASSERT_NE(f_it_20, bimap.end());
    EXPECT_EQ(*f_it_20->first.pKey, 20);
    EXPECT_EQ(f_it_20->second, "twenty");

    // Test backward lookup (Value -> Key)
    auto b_it_ten = bimap.findValue("ten");
    ASSERT_NE(b_it_ten, bimap.endValue());
    EXPECT_EQ(b_it_ten->first, "ten");
    EXPECT_EQ(*b_it_ten->second.pKey, 10);

    auto b_it_twenty = bimap.findValue("twenty");
    ASSERT_NE(b_it_twenty, bimap.endValue());
    EXPECT_EQ(b_it_twenty->first, "twenty");
    EXPECT_EQ(*b_it_twenty->second.pKey, 20);
}

// Test finding elements that do not exist in the map.
TEST(BimapTest, FindNonExistent) {
    Bimap<int, std::string> bimap;
    bimap.insert(10, "ten");

    // Finding a non-existent key should return the end iterator.
    EXPECT_EQ(bimap.find(999), bimap.end());

    // Finding a non-existent value should return the value-end iterator.
    EXPECT_EQ(bimap.findValue("nine-ninety-nine"), bimap.endValue());
}

// A test structure to track construction/destruction to verify correct memory management.
struct LifecycleKey {
    int id;
    static inline int constructions = 0;
    static inline int destructions = 0;

    explicit LifecycleKey(int i) : id(i) { constructions++; }
    LifecycleKey(const LifecycleKey& other) : id(other.id) { constructions++; }
    LifecycleKey(LifecycleKey&&) = delete; // Bimap uses copy-construction
    ~LifecycleKey() { destructions++; }

    bool operator==(const LifecycleKey& rhs) const { return id == rhs.id; }

    static void reset() {
        constructions = 0;
        destructions = 0;
    }
};

// Specialize std::hash for the test key.
namespace std {
template <> struct hash<LifecycleKey> {
    size_t operator()(const LifecycleKey& k) const { return hash<int>()(k.id); }
};
}

TEST(BimapTest, KeyLifecycleOnClear) {
    LifecycleKey::reset();
    {
        Bimap<LifecycleKey, int> bimap;
        bimap.insert(LifecycleKey(10), 100);
        bimap.insert(LifecycleKey(20), 200);

        // 2 on stack, 2 copied into bimap
        ASSERT_EQ(LifecycleKey::constructions, 4);
        // 2 on stack are destroyed after insert() calls
        ASSERT_EQ(LifecycleKey::destructions, 2);

        bimap.clear();
        EXPECT_EQ(LifecycleKey::destructions, 4);
        EXPECT_TRUE(bimap.empty());
    } // Bimap destructor runs on an empty map
    EXPECT_EQ(LifecycleKey::constructions, LifecycleKey::destructions);
}

// A test fixture to provide a pre-populated Bimap for each test case.
class BimapEraseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Insert some initial data before each test runs.
        bimap.insert(10, "ten");
        bimap.insert(20, "twenty");
        bimap.insert(30, "thirty");
    }

    // Verifies that an element is completely gone from the bimap.
    void verifyElementIsErased(int key, const std::string& value) {
        // Check both forward and backward lookups fail.
        EXPECT_EQ(bimap.find(key), bimap.end());
        EXPECT_EQ(bimap.findValue(value), bimap.endValue());
    }

    // Verifies that an element still exists and is consistent.
    void verifyElementExists(int key, const std::string& value) {
        auto f_it = bimap.find(key);
        ASSERT_NE(f_it, bimap.end());
        EXPECT_EQ(f_it->second, value);

        auto b_it = bimap.findValue(value);
        ASSERT_NE(b_it, bimap.endValue());
        EXPECT_EQ(*(b_it->second.pKey), key);
    }

    Bimap<int, std::string> bimap;
};

// Test case for `bool erase(Key const& key)`
TEST_F(BimapEraseTest, EraseByKey) {
    bool result = bimap.erase(20);

    EXPECT_TRUE(result);
    verifyElementIsErased(20, "twenty");

    // Ensure other elements were not affected.
    verifyElementExists(10, "ten");
    verifyElementExists(30, "thirty");
}

TEST_F(BimapEraseTest, EraseByKeyNonExistent) {
    bool result = bimap.erase(999);

    EXPECT_FALSE(result);

    // Ensure no elements were affected.
    verifyElementExists(10, "ten");
    verifyElementExists(20, "twenty");
    verifyElementExists(30, "thirty");
}

// Test case for `void erase(typename ForwardMap::const_iterator it)`
TEST_F(BimapEraseTest, EraseByForwardIterator) {
    auto it_to_erase = bimap.find(30);
    ASSERT_NE(it_to_erase, bimap.end());

    bimap.erase(it_to_erase);

    verifyElementIsErased(30, "thirty");

    // Ensure other elements were not affected.
    verifyElementExists(10, "ten");
    verifyElementExists(20, "twenty");
}

// Test case for `void erase(typename BackwardMap::const_iterator it)`
TEST_F(BimapEraseTest, EraseByBackwardIterator) {
    auto it_to_erase = bimap.findValue("ten");
    ASSERT_NE(it_to_erase, bimap.endValue());

    bimap.erase(it_to_erase);

    verifyElementIsErased(10, "ten");

    // Ensure other elements were not affected.
    verifyElementExists(20, "twenty");
    verifyElementExists(30, "thirty");
}

TEST(BimapEraseLifecycleTest, EraseCallsKeyDestructor) {
    LifecycleKey::reset();
    {
        Bimap<LifecycleKey, int> bimap;

        // Insert three items.
        bimap.insert(LifecycleKey(1), 100);
        bimap.insert(LifecycleKey(2), 200);
        bimap.insert(LifecycleKey(3), 300);
        // 3 on stack (destroyed) + 3 in bimap
        ASSERT_EQ(LifecycleKey::constructions, 6);
        ASSERT_EQ(LifecycleKey::destructions, 3);

        // Erase one item by key. This should trigger two destructor calls.
        bimap.erase(LifecycleKey(1));
        EXPECT_EQ(LifecycleKey::destructions, 5);

        // Erase the second item with a forward iterator (Two destructions)
        auto f_it = bimap.find(LifecycleKey(2));
        ASSERT_NE(f_it, bimap.end());
        bimap.erase(f_it);
        EXPECT_EQ(LifecycleKey::destructions, 7);

        // Erase the third item with a backward iterator.
        auto b_it = bimap.findValue(300);
        ASSERT_NE(b_it, bimap.endValue());
        bimap.erase(b_it);
        EXPECT_EQ(LifecycleKey::destructions, 8);

        EXPECT_TRUE(bimap.empty());
    }
    // Final check: All constructions must be matched by destructions.
    EXPECT_EQ(LifecycleKey::constructions, LifecycleKey::destructions);
}