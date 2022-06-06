// Copyright (c) 2021 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <list>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "source/util/pooled_linked_list.h"

namespace spvtools {
namespace utils {
namespace {

using PooledLinkedListTest = ::testing::Test;

template <typename T>
static std::vector<T> ToVector(const PooledLinkedList<T>& list) {
  std::vector<T> vec;
  for (auto it = list.begin(); it != list.end(); ++it) {
    vec.push_back(*it);
  }
  return vec;
}

template <typename T>
static void AppendVector(PooledLinkedList<T>& list, const std::vector<T>& vec) {
  for (const T& t : vec) {
    list.push_back(t);
  }
}

TEST(PooledLinkedListTest, Empty) {
  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll(&pool);
  EXPECT_TRUE(ll.empty());

  ll.push_back(1u);
  EXPECT_TRUE(!ll.empty());
}

TEST(PooledLinkedListTest, Iterator) {
  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll(&pool);

  EXPECT_EQ(ll.begin(), ll.end());

  ll.push_back(1);
  EXPECT_NE(ll.begin(), ll.end());

  auto it = ll.begin();
  EXPECT_EQ(*it, 1);
  ++it;
  EXPECT_EQ(it, ll.end());
}

TEST(PooledLinkedListTest, Iterator_algorithms) {
  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll(&pool);

  AppendVector(ll, {3, 2, 0, 1});
  EXPECT_EQ(std::distance(ll.begin(), ll.end()), 4);
  EXPECT_EQ(*std::min_element(ll.begin(), ll.end()), 0);
  EXPECT_EQ(*std::max_element(ll.begin(), ll.end()), 3);
}

TEST(PooledLinkedListTest, FrontBack) {
  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll(&pool);

  ll.push_back(1);
  EXPECT_EQ(ll.front(), 1);
  EXPECT_EQ(ll.back(), 1);

  ll.push_back(2);
  EXPECT_EQ(ll.front(), 1);
  EXPECT_EQ(ll.back(), 2);
}

TEST(PooledLinkedListTest, PushBack) {
  const std::vector<uint32_t> vec = {1, 2, 3, 4, 5, 6};

  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll(&pool);

  AppendVector(ll, vec);
  EXPECT_EQ(vec, ToVector(ll));
}

TEST(PooledLinkedListTest, RemoveFirst) {
  const std::vector<uint32_t> vec = {1, 2, 3, 4, 5, 6};

  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll(&pool);

  EXPECT_FALSE(ll.remove_first(0));
  AppendVector(ll, vec);
  EXPECT_FALSE(ll.remove_first(0));

  std::vector<uint32_t> tmp = vec;
  while (!tmp.empty()) {
    size_t mid = tmp.size() / 2;
    uint32_t elt = tmp[mid];
    tmp.erase(tmp.begin() + mid);

    EXPECT_TRUE(ll.remove_first(elt));
    EXPECT_FALSE(ll.remove_first(elt));
    EXPECT_EQ(tmp, ToVector(ll));
  }
  EXPECT_TRUE(ll.empty());
}

TEST(PooledLinkedListTest, RemoveFirst_Duplicates) {
  const std::vector<uint32_t> vec = {3, 1, 2, 3, 3, 3, 3, 4, 3, 5, 3, 6, 3};

  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll(&pool);
  AppendVector(ll, vec);

  std::vector<uint32_t> tmp = vec;
  while (!tmp.empty()) {
    size_t mid = tmp.size() / 2;
    uint32_t elt = tmp[mid];
    tmp.erase(std::find(tmp.begin(), tmp.end(), elt));

    EXPECT_TRUE(ll.remove_first(elt));
    EXPECT_EQ(tmp, ToVector(ll));
  }
  EXPECT_TRUE(ll.empty());
}

TEST(PooledLinkedList, MoveTo) {
  const std::vector<uint32_t> vec = {1, 2, 3, 4, 5, 6};

  PooledLinkedListNodes<uint32_t> pool;
  PooledLinkedList<uint32_t> ll1(&pool);
  PooledLinkedList<uint32_t> ll2(&pool);
  PooledLinkedList<uint32_t> ll3(&pool);

  AppendVector(ll1, vec);
  AppendVector(ll2, vec);
  AppendVector(ll3, vec);
  EXPECT_EQ(pool.total_nodes(), vec.size() * 3);
  EXPECT_EQ(pool.total_nodes(), vec.size() * 3);
  EXPECT_EQ(pool.free_nodes(), 0);

  // Move two lists to the new pool
  PooledLinkedListNodes<uint32_t> pool_new;
  ll1.move_nodes(&pool_new);
  ll2.move_nodes(&pool_new);

  // Moved nodes should belong to new pool
  EXPECT_EQ(ll1.pool(), &pool_new);
  EXPECT_EQ(ll2.pool(), &pool_new);

  // Old pool should be smaller & have free nodes.
  EXPECT_EQ(pool.used_nodes(), vec.size());
  EXPECT_EQ(pool.free_nodes(), vec.size() * 2);

  // New pool should be sized exactly and no free nodes.
  EXPECT_EQ(pool_new.total_nodes(), vec.size() * 2);
  EXPECT_EQ(pool_new.used_nodes(), vec.size() * 2);
  EXPECT_EQ(pool_new.free_nodes(), 0);

  // All lists should be preserved
  EXPECT_EQ(ToVector(ll1), vec);
  EXPECT_EQ(ToVector(ll2), vec);
  EXPECT_EQ(ToVector(ll3), vec);
}

}  // namespace
}  // namespace utils
}  // namespace spvtools
