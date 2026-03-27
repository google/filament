// Copyright 2023 The Draco Authors.
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
#include "draco/metadata/property_attribute.h"

#include <memory>
#include <string>
#include <utility>

#include "draco/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(PropertyAttributeTest, TestPropertyDefaults) {
  // Test construction of an empty property attribute property.
  draco::PropertyAttribute::Property property;
  ASSERT_TRUE(property.GetName().empty());
  ASSERT_TRUE(property.GetAttributeName().empty());
}

TEST(PropertyAttributeTest, TestPropertyAttributeDefaults) {
  // Test construction of an empty property attribute.
  draco::PropertyAttribute attribute;
  ASSERT_TRUE(attribute.GetName().empty());
  ASSERT_TRUE(attribute.GetClass().empty());
  ASSERT_EQ(attribute.NumProperties(), 0);
}

TEST(PropertyAttributeTest, TestPropertySettersAndGetters) {
  // Test setter and getter methods of the property attribute property.
  draco::PropertyAttribute::Property property;
  property.SetName("The magnitude.");
  property.SetAttributeName("_MAGNITUDE");

  // Check that property members can be accessed via getters.
  ASSERT_EQ(property.GetName(), "The magnitude.");
  ASSERT_EQ(property.GetAttributeName(), "_MAGNITUDE");
}

TEST(PropertyAttributeTest, TestPropertyAttributeSettersAndGetters) {
  // Test setter and getter methods of the property attribute.
  draco::PropertyAttribute attribute;
  attribute.SetName("The movement.");
  attribute.SetClass("movement");
  {
    std::unique_ptr<draco::PropertyAttribute::Property> property(
        new draco::PropertyAttribute::Property());
    property->SetName("The magnitude.");
    property->SetAttributeName("_MAGNITUDE");
    ASSERT_EQ(attribute.AddProperty(std::move(property)), 0);
  }
  {
    std::unique_ptr<draco::PropertyAttribute::Property> property(
        new draco::PropertyAttribute::Property());
    property->SetName("The direction.");
    property->SetAttributeName("_DIRECTION");
    ASSERT_EQ(attribute.AddProperty(std::move(property)), 1);
  }

  // Check that property attribute members can be accessed via getters.
  ASSERT_EQ(attribute.GetName(), "The movement.");
  ASSERT_EQ(attribute.GetClass(), "movement");
  ASSERT_EQ(attribute.NumProperties(), 2);
  ASSERT_EQ(attribute.GetProperty(0).GetName(), "The magnitude.");
  ASSERT_EQ(attribute.GetProperty(0).GetAttributeName(), "_MAGNITUDE");
  ASSERT_EQ(attribute.GetProperty(1).GetName(), "The direction.");
  ASSERT_EQ(attribute.GetProperty(1).GetAttributeName(), "_DIRECTION");

  // Check that properties can be removed.
  attribute.RemoveProperty(0);
  ASSERT_EQ(attribute.NumProperties(), 1);
  ASSERT_EQ(attribute.GetProperty(0).GetName(), "The direction.");
  ASSERT_EQ(attribute.GetProperty(0).GetAttributeName(), "_DIRECTION");
  attribute.RemoveProperty(0);
  ASSERT_EQ(attribute.NumProperties(), 0);
}

TEST(PropertyAttributeTest, TestPropertyCopy) {
  // Test that property attribute property can be copied.
  draco::PropertyAttribute::Property property;
  property.SetName("The direction.");
  property.SetAttributeName("_DIRECTION");

  // Make a copy.
  draco::PropertyAttribute::Property copy;
  copy.Copy(property);

  // Check the copy.
  ASSERT_EQ(copy.GetName(), "The direction.");
  ASSERT_EQ(copy.GetAttributeName(), "_DIRECTION");
}

TEST(PropertyAttributeTest, TestPropertyAttributeCopy) {
  // Test that property attribute can be copied.
  draco::PropertyAttribute attribute;
  attribute.SetName("The movement.");
  attribute.SetClass("movement");
  {
    std::unique_ptr<draco::PropertyAttribute::Property> property(
        new draco::PropertyAttribute::Property());
    property->SetName("The magnitude.");
    property->SetAttributeName("_MAGNITUDE");
    ASSERT_EQ(attribute.AddProperty(std::move(property)), 0);
  }
  {
    std::unique_ptr<draco::PropertyAttribute::Property> property(
        new draco::PropertyAttribute::Property());
    property->SetName("The direction.");
    property->SetAttributeName("_DIRECTION");
    ASSERT_EQ(attribute.AddProperty(std::move(property)), 1);
  }

  // Make a copy.
  draco::PropertyAttribute copy;
  copy.Copy(attribute);

  // Check the copy.
  ASSERT_EQ(attribute.GetName(), "The movement.");
  ASSERT_EQ(attribute.GetClass(), "movement");
  ASSERT_EQ(attribute.NumProperties(), 2);
  ASSERT_EQ(attribute.GetProperty(0).GetName(), "The magnitude.");
  ASSERT_EQ(attribute.GetProperty(0).GetAttributeName(), "_MAGNITUDE");
  ASSERT_EQ(attribute.GetProperty(1).GetName(), "The direction.");
  ASSERT_EQ(attribute.GetProperty(1).GetAttributeName(), "_DIRECTION");
}

TEST(PropertyAttributeTest, TestPropertyCompare) {
  // Test comparison of two properties.
  typedef draco::PropertyAttribute::Property Property;
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
    a.SetName("The magnitude.");
    b.SetName("The direction.");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property objects with different attribute names.
    Property a;
    Property b;
    a.SetAttributeName("_MAGNITUDE");
    b.SetAttributeName("_DIRECTION");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

TEST(PropertyAttributeTest, TestPropertyAttributeCompare) {
  // Test comparison of two property attributes.
  typedef draco::PropertyAttribute PropertyAttribute;
  typedef draco::PropertyAttribute::Property Property;
  {
    // Compare the same property attribute object.
    PropertyAttribute a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two default property attributes.
    PropertyAttribute a;
    PropertyAttribute b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two property attributes with different names.
    PropertyAttribute a;
    PropertyAttribute b;
    a.SetName("The movement.");
    b.SetName("The reflection.");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property attributes with different classes.
    PropertyAttribute a;
    PropertyAttribute b;
    a.SetClass("movement");
    b.SetClass("reflection");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property attributes with identical properties.
    PropertyAttribute a;
    PropertyAttribute b;
    a.AddProperty(std::unique_ptr<Property>(new Property));
    b.AddProperty(std::unique_ptr<Property>(new Property));
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two property attributes with different number of properties.
    PropertyAttribute a;
    PropertyAttribute b;
    a.AddProperty(std::unique_ptr<Property>(new Property));
    b.AddProperty(std::unique_ptr<Property>(new Property));
    b.AddProperty(std::unique_ptr<Property>(new Property));
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two property attributes with different properties.
    PropertyAttribute a;
    PropertyAttribute b;
    std::unique_ptr<Property> p1(new Property);
    std::unique_ptr<Property> p2(new Property);
    p1->SetName("The magnitude.");
    p2->SetName("The direction.");
    a.AddProperty(std::move(p1));
    b.AddProperty(std::move(p2));
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
