// Copyright 2022 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco/metadata/property_table.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "draco/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(PropertyTableTest, TestPropertyDataDefaults) {
  // Test construction of an empty property data.
  draco::PropertyTable::Property::Data data;
  ASSERT_TRUE(data.data.empty());
  ASSERT_EQ(data.target, 0);
}

TEST(PropertyTableTest, TestPropertyDefaults) {
  // Test construction of an empty property table property.
  draco::PropertyTable::Property property;
  ASSERT_TRUE(property.GetName().empty());
  ASSERT_TRUE(property.GetData().data.empty());
  {
    const auto &offsets = property.GetArrayOffsets();
    ASSERT_TRUE(offsets.type.empty());
    ASSERT_TRUE(offsets.data.data.empty());
    ASSERT_EQ(offsets.data.target, 0);
  }
  {
    const auto &offsets = property.GetStringOffsets();
    ASSERT_TRUE(offsets.type.empty());
    ASSERT_TRUE(offsets.data.data.empty());
    ASSERT_EQ(offsets.data.target, 0);
  }
}

TEST(PropertyTableTest, TestPropertyTableDefaults) {
  // Test construction of an empty property table.
  draco::PropertyTable table;
  ASSERT_TRUE(table.GetName().empty());
  ASSERT_TRUE(table.GetClass().empty());
  ASSERT_EQ(table.GetCount(), 0);
  ASSERT_EQ(table.NumProperties(), 0);
}

TEST(PropertyTableTest, TestPropertySettersAndGetters) {
  // Test setter and getter methods of the property table property.
  draco::PropertyTable::Property property;
  property.SetName("Unfortunate Conflict Of Evidence");
  property.GetData().data.push_back(2);

  // Check that property members can be accessed via getters.
  ASSERT_EQ(property.GetName(), "Unfortunate Conflict Of Evidence");
  ASSERT_EQ(property.GetData().data.size(), 1);
  ASSERT_EQ(property.GetData().data[0], 2);
}

TEST(PropertyTableTest, TestPropertyTableSettersAndGetters) {
  // Test setter and getter methods of the property table.
  draco::PropertyTable table;
  table.SetName("Just Read The Instructions");
  table.SetClass("General Contact Unit");
  table.SetCount(456);
  {
    std::unique_ptr<draco::PropertyTable::Property> property(
        new draco::PropertyTable::Property());
    property->SetName("Determinist");
    ASSERT_EQ(table.AddProperty(std::move(property)), 0);
  }
  {
    std::unique_ptr<draco::PropertyTable::Property> property(
        new draco::PropertyTable::Property());
    property->SetName("Revisionist");
    ASSERT_EQ(table.AddProperty(std::move(property)), 1);
  }

  // Check that property table members can be accessed via getters.
  ASSERT_EQ(table.GetName(), "Just Read The Instructions");
  ASSERT_EQ(table.GetClass(), "General Contact Unit");
  ASSERT_EQ(table.GetCount(), 456);
  ASSERT_EQ(table.NumProperties(), 2);
  ASSERT_EQ(table.GetProperty(0).GetName(), "Determinist");
  ASSERT_EQ(table.GetProperty(1).GetName(), "Revisionist");

  // Check that proeprties can be removed.
  table.RemoveProperty(0);
  ASSERT_EQ(table.NumProperties(), 1);
  ASSERT_EQ(table.GetProperty(0).GetName(), "Revisionist");
  table.RemoveProperty(0);
  ASSERT_EQ(table.NumProperties(), 0);
}

TEST(PropertyTableTest, TestPropertyCopy) {
  // Test that property table property can be copied.
  draco::PropertyTable::Property property;
  property.SetName("Unfortunate Conflict Of Evidence");
  property.GetData().data.push_back(2);

  // Make a copy.
  draco::PropertyTable::Property copy;
  copy.Copy(property);

  // Check the copy.
  ASSERT_EQ(copy.GetName(), "Unfortunate Conflict Of Evidence");
  ASSERT_EQ(copy.GetData().data.size(), 1);
  ASSERT_EQ(copy.GetData().data[0], 2);
}

TEST(PropertyTableTest, TestPropertyTableCopy) {
  // Test that property table can be copied.
  draco::PropertyTable table;
  table.SetName("Just Read The Instructions");
  table.SetClass("General Contact Unit");
  table.SetCount(456);
  {
    std::unique_ptr<draco::PropertyTable::Property> property(
        new draco::PropertyTable::Property());
    property->SetName("Determinist");
    table.AddProperty(std::move(property));
  }
  {
    std::unique_ptr<draco::PropertyTable::Property> property(
        new draco::PropertyTable::Property());
    property->SetName("Revisionist");
    table.AddProperty(std::move(property));
  }

  // Make a copy.
  draco::PropertyTable copy;
  copy.Copy(table);

  // Check the copy.
  ASSERT_EQ(copy.GetName(), "Just Read The Instructions");
  ASSERT_EQ(copy.GetClass(), "General Contact Unit");
  ASSERT_EQ(copy.GetCount(), 456);
  ASSERT_EQ(copy.NumProperties(), 2);
  ASSERT_EQ(copy.GetProperty(0).GetName(), "Determinist");
  ASSERT_EQ(copy.GetProperty(1).GetName(), "Revisionist");
}

TEST(PropertyTableTest, TestPropertyDataCompare) {
  // Test comparison of two property data objects.
  typedef draco::PropertyTable::Property::Data Data;
  {
    // Compare the same data object.
    Data a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two default data objects.
    Data a;
    Data b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two data objects with different targets.
    Data a;
    Data b;
    a.target = 1;
    b.target = 2;
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two data objects with different data vectors.
    Data a;
    Data b;
    a.data = {1};
    a.data = {2};
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

TEST(PropertyTableTest, TestPropertyOffsets) {
  // Test comparison of two property offsets.
  typedef draco::PropertyTable::Property::Offsets Offsets;
  {
    // Compare the same offsets object.
    Offsets a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two default offsets objects.
    Offsets a;
    Offsets b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two offsets objects with different types.
    Offsets a;
    Offsets b;
    a.type = 1;
    b.type = 2;
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two offsets objects with different data objects.
    Offsets a;
    Offsets b;
    a.data.target = 1;
    b.data.target = 2;
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

TEST(PropertyTableTest, TestPropertyCompare) {
  // Test comparison of two properties.
  typedef draco::PropertyTable::Property Property;
  {
    // Compare the same property object.
    Property a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two default property objects.
    Property a;
    Property b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two property objects with different names.
    Property a;
    Property b;
    a.SetName("one");
    b.SetName("two");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property objects with different data.
    Property a;
    Property b;
    a.GetData().target = 1;
    b.GetData().target = 2;
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property objects with different array offsets.
    Property a;
    Property b;
    a.GetArrayOffsets().data.target = 1;
    b.GetArrayOffsets().data.target = 2;
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property objects with different string offsets.
    Property a;
    Property b;
    a.GetStringOffsets().data.target = 1;
    b.GetStringOffsets().data.target = 2;
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

TEST(PropertyTableTest, TestPropertyTableCompare) {
  // Test comparison of two property tables.
  typedef draco::PropertyTable PropertyTable;
  typedef draco::PropertyTable::Property Property;
  {
    // Compare the same property table object.
    PropertyTable a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two default property tables.
    PropertyTable a;
    PropertyTable b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two property tables with different names.
    PropertyTable a;
    PropertyTable b;
    a.SetName("one");
    b.SetName("two");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property tables with different classes.
    PropertyTable a;
    PropertyTable b;
    a.SetClass("one");
    b.SetClass("two");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property tables with different counts.
    PropertyTable a;
    PropertyTable b;
    a.SetCount(1);
    b.SetCount(2);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property tables with identical properties.
    PropertyTable a;
    PropertyTable b;
    a.AddProperty(std::unique_ptr<Property>(new Property));
    b.AddProperty(std::unique_ptr<Property>(new Property));
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two property tables with different number of properties.
    PropertyTable a;
    PropertyTable b;
    a.AddProperty(std::unique_ptr<Property>(new Property));
    b.AddProperty(std::unique_ptr<Property>(new Property));
    b.AddProperty(std::unique_ptr<Property>(new Property));
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property tables with different properties.
    PropertyTable a;
    PropertyTable b;
    std::unique_ptr<Property> p1(new Property);
    std::unique_ptr<Property> p2(new Property);
    p1->SetName("one");
    p2->SetName("two");
    a.AddProperty(std::move(p1));
    b.AddProperty(std::move(p2));
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

TEST(PropertyTableTest, EnodesAndDecodesOffsetBuffers) {
  {
    // Encoding an offset buffer from small integers that should fit in an 8 bit
    // integer.
    std::vector<uint64_t> sample_offsets = {0x5u, 0x21u, 0x7u, 0x32u, 0xffu};
    auto encoded_offsets =
        draco::PropertyTable::Property::Offsets::MakeFromInts(sample_offsets);
    ASSERT_EQ(encoded_offsets.data.data,
              std::vector<uint8_t>({0x5u, 0x21u, 0x7u, 0x32u, 0xffu}));
    ASSERT_EQ(encoded_offsets.type, "UINT8");

    DRACO_ASSIGN_OR_ASSERT(std::vector<uint64_t> decoded_offsets,
                           encoded_offsets.ParseToInts());
    ASSERT_EQ(decoded_offsets, sample_offsets);
  }
  {
    // Encoding an offset buffer from medium sized integers that should fit in a
    // 16 bit integer.
    std::vector<uint64_t> sample_offsets = {0x5u, 0x21u, 0xffffu};
    auto encoded_offsets =
        draco::PropertyTable::Property::Offsets::MakeFromInts(sample_offsets);
    ASSERT_EQ(encoded_offsets.data.data,
              std::vector<uint8_t>({0x5u, 0u, 0x21u, 0u, 0xffu, 0xffu}));
    ASSERT_EQ(encoded_offsets.type, "UINT16");

    DRACO_ASSIGN_OR_ASSERT(std::vector<uint64_t> decoded_offsets,
                           encoded_offsets.ParseToInts());
    ASSERT_EQ(decoded_offsets, sample_offsets);
  }
  {
    // Encoding an offset buffer from medium sized integers that should fit in a
    // 32 bit integer.
    std::vector<uint64_t> sample_offsets = {0x5u, 0x21u, 0xffffffffu};
    auto encoded_offsets =
        draco::PropertyTable::Property::Offsets::MakeFromInts(sample_offsets);
    ASSERT_EQ(encoded_offsets.data.data,
              std::vector<uint8_t>({0x5u, 0u, 0u, 0u, 0x21u, 0u, 0u, 0u, 0xffu,
                                    0xffu, 0xffu, 0xffu}));
    ASSERT_EQ(encoded_offsets.type, "UINT32");

    DRACO_ASSIGN_OR_ASSERT(std::vector<uint64_t> decoded_offsets,
                           encoded_offsets.ParseToInts());
    ASSERT_EQ(decoded_offsets, sample_offsets);
  }
  {
    // Encoding an offset buffer from large integers that won't fit in a 32 bit
    // integer.
    std::vector<uint64_t> sample_offsets = {0x5u, 0x21u, 0x100000000u};
    auto encoded_offsets =
        draco::PropertyTable::Property::Offsets::MakeFromInts(sample_offsets);
    ASSERT_EQ(encoded_offsets.data.data,
              std::vector<uint8_t>({0x5u,  0u, 0u, 0u, 0u, 0u, 0u, 0u,
                                    0x21u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
                                    0u,    0u, 0u, 0u, 1u, 0u, 0u, 0u}));
    ASSERT_EQ(encoded_offsets.type, "UINT64");

    DRACO_ASSIGN_OR_ASSERT(std::vector<uint64_t> decoded_offsets,
                           encoded_offsets.ParseToInts());
    ASSERT_EQ(decoded_offsets, sample_offsets);
  }
  {
    // Decoding a malformed buffer should return an error.
    draco::PropertyTable::Property::Offsets broken_offsets;
    broken_offsets.data.data = std::vector<uint8_t>({0, 0, 0, 0});
    broken_offsets.type = "BROKEN_TYPE";

    draco::StatusOr<std::vector<uint64_t>> decoded_offsets =
        broken_offsets.ParseToInts();
    ASSERT_FALSE(decoded_offsets.ok());
  }
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
