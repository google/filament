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

#include "marl/containers.h"
#include "marl_test.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <cstddef>
#include <string>

class ContainersVectorTest : public WithoutBoundScheduler {};

TEST_F(ContainersVectorTest, Empty) {
  marl::containers::vector<std::string, 4> vector(allocator);
  ASSERT_EQ(vector.size(), size_t(0));
}

TEST_F(ContainersVectorTest, WithinFixedCapIndex) {
  marl::containers::vector<std::string, 4> vector(allocator);
  vector.resize(4);
  vector[0] = "A";
  vector[1] = "B";
  vector[2] = "C";
  vector[3] = "D";

  ASSERT_EQ(vector[0], "A");
  ASSERT_EQ(vector[1], "B");
  ASSERT_EQ(vector[2], "C");
  ASSERT_EQ(vector[3], "D");
}

TEST_F(ContainersVectorTest, BeyondFixedCapIndex) {
  marl::containers::vector<std::string, 1> vector(allocator);
  vector.resize(4);
  vector[0] = "A";
  vector[1] = "B";
  vector[2] = "C";
  vector[3] = "D";

  ASSERT_EQ(vector[0], "A");
  ASSERT_EQ(vector[1], "B");
  ASSERT_EQ(vector[2], "C");
  ASSERT_EQ(vector[3], "D");
}

TEST_F(ContainersVectorTest, WithinFixedCapPushPop) {
  marl::containers::vector<std::string, 4> vector(allocator);
  vector.push_back("A");
  vector.push_back("B");
  vector.push_back("C");
  vector.push_back("D");

  ASSERT_EQ(vector.size(), size_t(4));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(4));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "D");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(3));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(3));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "C");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(2));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(2));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "B");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(1));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(1));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "A");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(0));
}

TEST_F(ContainersVectorTest, BeyondFixedCapPushPop) {
  marl::containers::vector<std::string, 2> vector(allocator);
  vector.push_back("A");
  vector.push_back("B");
  vector.push_back("C");
  vector.push_back("D");

  ASSERT_EQ(vector.size(), size_t(4));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(4));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "D");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(3));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(3));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "C");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(2));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(2));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "B");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(1));
  ASSERT_EQ(vector.end() - vector.begin(), ptrdiff_t(1));

  ASSERT_EQ(vector.front(), "A");
  ASSERT_EQ(vector.back(), "A");
  vector.pop_back();
  ASSERT_EQ(vector.size(), size_t(0));
}

TEST_F(ContainersVectorTest, CopyConstruct) {
  marl::containers::vector<std::string, 4> vectorA(allocator);

  vectorA.resize(3);
  vectorA[0] = "A";
  vectorA[1] = "B";
  vectorA[2] = "C";

  marl::containers::vector<std::string, 4> vectorB(vectorA, allocator);
  ASSERT_EQ(vectorB.size(), size_t(3));
  ASSERT_EQ(vectorB[0], "A");
  ASSERT_EQ(vectorB[1], "B");
  ASSERT_EQ(vectorB[2], "C");
}

TEST_F(ContainersVectorTest, CopyConstructDifferentBaseCapacity) {
  marl::containers::vector<std::string, 4> vectorA(allocator);

  vectorA.resize(3);
  vectorA[0] = "A";
  vectorA[1] = "B";
  vectorA[2] = "C";

  marl::containers::vector<std::string, 2> vectorB(vectorA, allocator);
  ASSERT_EQ(vectorB.size(), size_t(3));
  ASSERT_EQ(vectorB[0], "A");
  ASSERT_EQ(vectorB[1], "B");
  ASSERT_EQ(vectorB[2], "C");
}

TEST_F(ContainersVectorTest, CopyAssignment) {
  marl::containers::vector<std::string, 4> vectorA(allocator);

  vectorA.resize(3);
  vectorA[0] = "A";
  vectorA[1] = "B";
  vectorA[2] = "C";

  marl::containers::vector<std::string, 4> vectorB(allocator);
  vectorB = vectorA;
  ASSERT_EQ(vectorB.size(), size_t(3));
  ASSERT_EQ(vectorB[0], "A");
  ASSERT_EQ(vectorB[1], "B");
  ASSERT_EQ(vectorB[2], "C");
}

TEST_F(ContainersVectorTest, CopyAssignmentDifferentBaseCapacity) {
  marl::containers::vector<std::string, 4> vectorA(allocator);

  vectorA.resize(3);
  vectorA[0] = "A";
  vectorA[1] = "B";
  vectorA[2] = "C";

  marl::containers::vector<std::string, 2> vectorB(allocator);
  vectorB = vectorA;
  ASSERT_EQ(vectorB.size(), size_t(3));
  ASSERT_EQ(vectorB[0], "A");
  ASSERT_EQ(vectorB[1], "B");
  ASSERT_EQ(vectorB[2], "C");
}

TEST_F(ContainersVectorTest, MoveConstruct) {
  marl::containers::vector<std::string, 4> vectorA(allocator);

  vectorA.resize(3);
  vectorA[0] = "A";
  vectorA[1] = "B";
  vectorA[2] = "C";

  marl::containers::vector<std::string, 2> vectorB(std::move(vectorA),
                                                   allocator);
  ASSERT_EQ(vectorB.size(), size_t(3));
  ASSERT_EQ(vectorB[0], "A");
  ASSERT_EQ(vectorB[1], "B");
  ASSERT_EQ(vectorB[2], "C");
}

TEST_F(ContainersVectorTest, Copy) {
  marl::containers::vector<std::string, 4> vectorA(allocator);
  marl::containers::vector<std::string, 2> vectorB(allocator);

  vectorA.resize(3);
  vectorA[0] = "A";
  vectorA[1] = "B";
  vectorA[2] = "C";

  vectorB.resize(1);
  vectorB[0] = "Z";

  vectorB = vectorA;
  ASSERT_EQ(vectorB.size(), size_t(3));
  ASSERT_EQ(vectorB[0], "A");
  ASSERT_EQ(vectorB[1], "B");
  ASSERT_EQ(vectorB[2], "C");
}

TEST_F(ContainersVectorTest, Move) {
  marl::containers::vector<std::string, 4> vectorA(allocator);
  marl::containers::vector<std::string, 2> vectorB(allocator);

  vectorA.resize(3);
  vectorA[0] = "A";
  vectorA[1] = "B";
  vectorA[2] = "C";

  vectorB.resize(1);
  vectorB[0] = "Z";

  vectorB = std::move(vectorA);
  ASSERT_EQ(vectorA.size(), size_t(0));
  ASSERT_EQ(vectorB.size(), size_t(3));
  ASSERT_EQ(vectorB[0], "A");
  ASSERT_EQ(vectorB[1], "B");
  ASSERT_EQ(vectorB[2], "C");
}

class ContainersListTest : public WithoutBoundScheduler {};

TEST_F(ContainersListTest, Empty) {
  marl::containers::list<std::string> list(allocator);
  ASSERT_EQ(list.size(), size_t(0));
}

TEST_F(ContainersListTest, EmplaceOne) {
  marl::containers::list<std::string> list(allocator);
  auto itEntry = list.emplace_front("hello world");
  ASSERT_EQ(*itEntry, "hello world");
  ASSERT_EQ(list.size(), size_t(1));
  auto it = list.begin();
  ASSERT_EQ(it, itEntry);
  ++it;
  ASSERT_EQ(it, list.end());
}

TEST_F(ContainersListTest, EmplaceThree) {
  marl::containers::list<std::string> list(allocator);
  auto itA = list.emplace_front("a");
  auto itB = list.emplace_front("b");
  auto itC = list.emplace_front("c");
  ASSERT_EQ(*itA, "a");
  ASSERT_EQ(*itB, "b");
  ASSERT_EQ(*itC, "c");
  ASSERT_EQ(list.size(), size_t(3));
  auto it = list.begin();
  ASSERT_EQ(it, itC);
  ++it;
  ASSERT_EQ(it, itB);
  ++it;
  ASSERT_EQ(it, itA);
  ++it;
  ASSERT_EQ(it, list.end());
}

TEST_F(ContainersListTest, EraseFront) {
  marl::containers::list<std::string> list(allocator);
  auto itA = list.emplace_front("a");
  auto itB = list.emplace_front("b");
  auto itC = list.emplace_front("c");
  list.erase(itC);
  ASSERT_EQ(list.size(), size_t(2));
  auto it = list.begin();
  ASSERT_EQ(it, itB);
  ++it;
  ASSERT_EQ(it, itA);
  ++it;
  ASSERT_EQ(it, list.end());
}

TEST_F(ContainersListTest, EraseBack) {
  marl::containers::list<std::string> list(allocator);
  auto itA = list.emplace_front("a");
  auto itB = list.emplace_front("b");
  auto itC = list.emplace_front("c");
  list.erase(itA);
  ASSERT_EQ(list.size(), size_t(2));
  auto it = list.begin();
  ASSERT_EQ(it, itC);
  ++it;
  ASSERT_EQ(it, itB);
  ++it;
  ASSERT_EQ(it, list.end());
}

TEST_F(ContainersListTest, EraseMid) {
  marl::containers::list<std::string> list(allocator);
  auto itA = list.emplace_front("a");
  auto itB = list.emplace_front("b");
  auto itC = list.emplace_front("c");
  list.erase(itB);
  ASSERT_EQ(list.size(), size_t(2));
  auto it = list.begin();
  ASSERT_EQ(it, itC);
  ++it;
  ASSERT_EQ(it, itA);
  ++it;
  ASSERT_EQ(it, list.end());
}

TEST_F(ContainersListTest, Grow) {
  marl::containers::list<std::string> list(allocator);
  for (int i = 0; i < 256; i++) {
    list.emplace_front(std::to_string(i));
  }
  ASSERT_EQ(list.size(), size_t(256));
}
