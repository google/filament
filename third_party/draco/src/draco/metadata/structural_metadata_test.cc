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
#include "draco/metadata/structural_metadata.h"

#include <memory>
#include <utility>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(StructuralMetadataTest, TestCopy) {
  // Tests copying of structural metadata.
  draco::StructuralMetadata structural_metadata;

  // Add property table schema to structural metadata.
  draco::StructuralMetadataSchema schema;
  schema.json.SetString("Culture");
  structural_metadata.SetSchema(schema);

  // Add property table to structural metadata.
  std::unique_ptr<draco::PropertyTable> table(new draco::PropertyTable());
  table->SetName("Just Read The Instructions");
  table->SetClass("General Contact Unit");
  table->SetCount(456);
  {
    std::unique_ptr<draco::PropertyTable::Property> property(
        new draco::PropertyTable::Property());
    property->SetName("Determinist");
    table->AddProperty(std::move(property));
  }
  {
    std::unique_ptr<draco::PropertyTable::Property> property(
        new draco::PropertyTable::Property());
    property->SetName("Revisionist");
    table->AddProperty(std::move(property));
  }
  ASSERT_EQ(structural_metadata.AddPropertyTable(std::move(table)), 0);

  // Copy the structural metadata.
  draco::StructuralMetadata copy;
  copy.Copy(structural_metadata);

  // Check that the structural metadata property table schema has been copied.
  ASSERT_EQ(copy.GetSchema().json.GetString(), "Culture");

  // Check that the structural metadata property table has been copied.
  ASSERT_EQ(copy.NumPropertyTables(), 1);
  ASSERT_EQ(copy.GetPropertyTable(0).GetName(), "Just Read The Instructions");
  ASSERT_EQ(copy.GetPropertyTable(0).GetClass(), "General Contact Unit");
  ASSERT_EQ(copy.GetPropertyTable(0).GetCount(), 456);
  ASSERT_EQ(copy.GetPropertyTable(0).NumProperties(), 2);
  ASSERT_EQ(copy.GetPropertyTable(0).GetProperty(0).GetName(), "Determinist");
  ASSERT_EQ(copy.GetPropertyTable(0).GetProperty(1).GetName(), "Revisionist");
}

TEST(StructuralMetadataTest, TestPropertyTables) {
  // Tests adding and removing of property tables to structural metadata.
  draco::StructuralMetadata structural_metadata;

  // Check that property tables can be added.
  {
    std::unique_ptr<draco::PropertyTable> table(new draco::PropertyTable());
    table->SetName("Just Read The Instructions");
    ASSERT_EQ(structural_metadata.AddPropertyTable(std::move(table)), 0);
  }
  {
    std::unique_ptr<draco::PropertyTable> table(new draco::PropertyTable());
    table->SetName("So Much For Subtlety");
    ASSERT_EQ(structural_metadata.AddPropertyTable(std::move(table)), 1);
  }
  {
    std::unique_ptr<draco::PropertyTable> table(new draco::PropertyTable());
    table->SetName("Of Course I Still Love You");
    ASSERT_EQ(structural_metadata.AddPropertyTable(std::move(table)), 2);
  }
  draco::StructuralMetadata &sm = structural_metadata;

  // Check that the property tables can be removed.
  ASSERT_EQ(sm.NumPropertyTables(), 3);
  ASSERT_EQ(sm.GetPropertyTable(0).GetName(), "Just Read The Instructions");
  ASSERT_EQ(sm.GetPropertyTable(1).GetName(), "So Much For Subtlety");
  ASSERT_EQ(sm.GetPropertyTable(2).GetName(), "Of Course I Still Love You");

  sm.RemovePropertyTable(1);
  ASSERT_EQ(sm.NumPropertyTables(), 2);
  ASSERT_EQ(sm.GetPropertyTable(0).GetName(), "Just Read The Instructions");
  ASSERT_EQ(sm.GetPropertyTable(1).GetName(), "Of Course I Still Love You");

  sm.RemovePropertyTable(1);
  ASSERT_EQ(sm.NumPropertyTables(), 1);
  ASSERT_EQ(sm.GetPropertyTable(0).GetName(), "Just Read The Instructions");

  sm.RemovePropertyTable(0);
  ASSERT_EQ(sm.NumPropertyTables(), 0);
}

TEST(StructuralMetadataTest, TestCompare) {
  // Test comparison of two structural metadata objects.
  typedef draco::PropertyTable PropertyTable;
  typedef draco::PropertyAttribute PropertyAttribute;
  {
    // Compare the same structural metadata object.
    draco::StructuralMetadata a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two identical structural metadata objects.
    draco::StructuralMetadata a;
    draco::StructuralMetadata b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two structural metadata objects with different schemas.
    draco::StructuralMetadata a;
    draco::StructuralMetadata b;
    draco::StructuralMetadataSchema s1;
    draco::StructuralMetadataSchema s2;
    s1.json.SetString("one");
    s2.json.SetString("two");
    a.SetSchema(s1);
    b.SetSchema(s2);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two objects with different number of property tables.
    draco::StructuralMetadata a;
    draco::StructuralMetadata b;
    a.AddPropertyTable(std::unique_ptr<PropertyTable>(new PropertyTable()));
    b.AddPropertyTable(std::unique_ptr<PropertyTable>(new PropertyTable()));
    b.AddPropertyTable(std::unique_ptr<PropertyTable>(new PropertyTable()));
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two objects with identical property tables.
    draco::StructuralMetadata a;
    draco::StructuralMetadata b;
    auto p1 = std::unique_ptr<PropertyTable>(new PropertyTable());
    auto p2 = std::unique_ptr<PropertyTable>(new PropertyTable());
    p1->SetName("one");
    p2->SetName("one");
    a.AddPropertyTable(std::move(p1));
    b.AddPropertyTable(std::move(p2));
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two objects with different property tables.
    draco::StructuralMetadata a;
    draco::StructuralMetadata b;
    auto p1 = std::unique_ptr<PropertyTable>(new PropertyTable());
    auto p2 = std::unique_ptr<PropertyTable>(new PropertyTable());
    p1->SetName("one");
    p2->SetName("two");
    a.AddPropertyTable(std::move(p1));
    b.AddPropertyTable(std::move(p2));
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two objects with identical property attributes.
    draco::StructuralMetadata a;
    draco::StructuralMetadata b;
    auto p1 = std::unique_ptr<PropertyAttribute>(new PropertyAttribute());
    auto p2 = std::unique_ptr<PropertyAttribute>(new PropertyAttribute());
    p1->SetName("one");
    p2->SetName("one");
    a.AddPropertyAttribute(std::move(p1));
    b.AddPropertyAttribute(std::move(p2));
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two objects with identical property attributes.
    draco::StructuralMetadata a;
    draco::StructuralMetadata b;
    auto p1 = std::unique_ptr<PropertyAttribute>(new PropertyAttribute());
    auto p2 = std::unique_ptr<PropertyAttribute>(new PropertyAttribute());
    p1->SetName("one");
    p2->SetName("two");
    a.AddPropertyAttribute(std::move(p1));
    b.AddPropertyAttribute(std::move(p2));
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
