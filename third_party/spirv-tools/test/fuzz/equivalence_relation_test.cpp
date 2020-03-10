// Copyright (c) 2019 Google LLC
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

#include <set>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "source/fuzz/equivalence_relation.h"

namespace spvtools {
namespace fuzz {
namespace {

struct UInt32Equals {
  bool operator()(const uint32_t* first, const uint32_t* second) const {
    return *first == *second;
  }
};

struct UInt32Hash {
  size_t operator()(const uint32_t* element) const {
    return static_cast<size_t>(*element);
  }
};

std::vector<uint32_t> ToUIntVector(
    const std::vector<const uint32_t*>& pointers) {
  std::vector<uint32_t> result;
  for (auto pointer : pointers) {
    result.push_back(*pointer);
  }
  return result;
}

TEST(EquivalenceRelationTest, BasicTest) {
  EquivalenceRelation<uint32_t, UInt32Hash, UInt32Equals> relation;
  ASSERT_TRUE(relation.GetAllKnownValues().empty());

  for (uint32_t element = 2; element < 80; element += 2) {
    relation.MakeEquivalent(0, element);
    relation.MakeEquivalent(element - 1, element + 1);
  }

  for (uint32_t element = 82; element < 100; element += 2) {
    relation.MakeEquivalent(80, element);
    relation.MakeEquivalent(element - 1, element + 1);
  }

  relation.MakeEquivalent(78, 80);

  std::vector<uint32_t> class1;
  for (uint32_t element = 0; element < 98; element += 2) {
    ASSERT_TRUE(relation.IsEquivalent(0, element));
    ASSERT_TRUE(relation.IsEquivalent(element, element + 2));
    class1.push_back(element);
  }
  class1.push_back(98);

  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(0)),
              testing::WhenSorted(class1));
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(4)),
              testing::WhenSorted(class1));
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(40)),
              testing::WhenSorted(class1));

  std::vector<uint32_t> class2;
  for (uint32_t element = 1; element < 79; element += 2) {
    ASSERT_TRUE(relation.IsEquivalent(1, element));
    ASSERT_TRUE(relation.IsEquivalent(element, element + 2));
    class2.push_back(element);
  }
  class2.push_back(79);
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(1)),
              testing::WhenSorted(class2));
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(11)),
              testing::WhenSorted(class2));
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(31)),
              testing::WhenSorted(class2));

  std::vector<uint32_t> class3;
  for (uint32_t element = 81; element < 99; element += 2) {
    ASSERT_TRUE(relation.IsEquivalent(81, element));
    ASSERT_TRUE(relation.IsEquivalent(element, element + 2));
    class3.push_back(element);
  }
  class3.push_back(99);
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(81)),
              testing::WhenSorted(class3));
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(91)),
              testing::WhenSorted(class3));
  ASSERT_THAT(ToUIntVector(relation.GetEquivalenceClass(99)),
              testing::WhenSorted(class3));

  bool first = true;
  std::vector<const uint32_t*> previous_class;
  for (auto representative : relation.GetEquivalenceClassRepresentatives()) {
    std::vector<const uint32_t*> current_class =
        relation.GetEquivalenceClass(*representative);
    ASSERT_TRUE(std::find(current_class.begin(), current_class.end(),
                          representative) != current_class.end());
    if (!first) {
      ASSERT_TRUE(std::find(previous_class.begin(), previous_class.end(),
                            representative) == previous_class.end());
    }
    previous_class = current_class;
    first = false;
  }
}

TEST(EquivalenceRelationTest, DeterministicEquivalenceClassOrder) {
  EquivalenceRelation<uint32_t, UInt32Hash, UInt32Equals> relation1;
  EquivalenceRelation<uint32_t, UInt32Hash, UInt32Equals> relation2;

  for (uint32_t i = 0; i < 1000; ++i) {
    if (i >= 10) {
      relation1.MakeEquivalent(i, i - 10);
      relation2.MakeEquivalent(i, i - 10);
    }
  }

  // We constructed the equivalence relations in the same way, so we would like
  // them to have identical representatives, and identically-ordered equivalence
  // classes per representative.
  ASSERT_THAT(ToUIntVector(relation1.GetEquivalenceClassRepresentatives()),
              ToUIntVector(relation2.GetEquivalenceClassRepresentatives()));
  for (auto representative : relation1.GetEquivalenceClassRepresentatives()) {
    ASSERT_THAT(ToUIntVector(relation1.GetEquivalenceClass(*representative)),
                ToUIntVector(relation2.GetEquivalenceClass(*representative)));
  }
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
