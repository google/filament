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
#include "draco/metadata/structural_metadata_schema.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "draco/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(StructuralMetadataSchemaTest, TestSchemaDefaults) {
  // Test construction of an empty schema.
  draco::StructuralMetadataSchema schema;
  ASSERT_TRUE(schema.Empty());
  ASSERT_EQ(schema.json.GetName(), "schema");
  ASSERT_EQ(schema.json.GetType(),
            draco::StructuralMetadataSchema::Object::OBJECT);
  ASSERT_TRUE(schema.json.GetObjects().empty());
  ASSERT_TRUE(schema.json.GetArray().empty());
  ASSERT_TRUE(schema.json.GetString().empty());
  ASSERT_EQ(schema.json.GetInteger(), 0);
  ASSERT_FALSE(schema.json.GetBoolean());
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectDefaultConstructor) {
  // Test construction of an empty schema object.
  draco::StructuralMetadataSchema::Object object;
  ASSERT_TRUE(object.GetName().empty());
  ASSERT_EQ(object.GetType(), draco::StructuralMetadataSchema::Object::OBJECT);
  ASSERT_TRUE(object.GetObjects().empty());
  ASSERT_TRUE(object.GetArray().empty());
  ASSERT_TRUE(object.GetString().empty());
  ASSERT_EQ(object.GetInteger(), 0);
  ASSERT_FALSE(object.GetBoolean());
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectNamedConstructor) {
  // Test construction of a named schema object.
  draco::StructuralMetadataSchema::Object object("Flexible Demeanour");
  ASSERT_EQ(object.GetName(), "Flexible Demeanour");
  ASSERT_EQ(object.GetType(), draco::StructuralMetadataSchema::Object::OBJECT);
  ASSERT_TRUE(object.GetObjects().empty());
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectStringConstructor) {
  // Test construction of schema object storing a string.
  draco::StructuralMetadataSchema::Object object("Flexible Demeanour", "GCU");
  ASSERT_EQ(object.GetName(), "Flexible Demeanour");
  ASSERT_EQ(object.GetType(), draco::StructuralMetadataSchema::Object::STRING);
  ASSERT_EQ(object.GetString(), "GCU");
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectIntegerConstructor) {
  // Test construction of schema object storing an integer.
  draco::StructuralMetadataSchema::Object object("Flexible Demeanour", 12);
  ASSERT_EQ(object.GetName(), "Flexible Demeanour");
  ASSERT_EQ(object.GetType(), draco::StructuralMetadataSchema::Object::INTEGER);
  ASSERT_EQ(object.GetInteger(), 12);
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectBooleanConstructor) {
  // Test construction of schema object storing a boolean.
  draco::StructuralMetadataSchema::Object object("Flexible Demeanour", true);
  ASSERT_EQ(object.GetName(), "Flexible Demeanour");
  ASSERT_EQ(object.GetType(), draco::StructuralMetadataSchema::Object::BOOLEAN);
  ASSERT_TRUE(object.GetBoolean());
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectSettersAndGetters) {
  // Test value setters and getters of schema object.
  typedef draco::StructuralMetadataSchema::Object Object;
  Object object;
  ASSERT_EQ(object.GetType(), Object::OBJECT);

  object.SetArray().push_back(Object("entry", 12));
  ASSERT_EQ(object.GetType(), Object::ARRAY);
  ASSERT_EQ(object.GetArray().size(), 1);
  ASSERT_EQ(object.GetArray()[0].GetName(), "entry");
  ASSERT_EQ(object.GetArray()[0].GetInteger(), 12);

  object.SetObjects().push_back(Object("object", 9));
  ASSERT_EQ(object.GetType(), Object::OBJECT);
  ASSERT_EQ(object.GetObjects().size(), 1);
  ASSERT_EQ(object.GetObjects()[0].GetName(), "object");
  ASSERT_EQ(object.GetObjects()[0].GetInteger(), 9);

  object.SetString("matter");
  ASSERT_EQ(object.GetType(), Object::STRING);
  ASSERT_EQ(object.GetString(), "matter");

  object.SetInteger(5);
  ASSERT_EQ(object.GetType(), Object::INTEGER);
  ASSERT_EQ(object.GetInteger(), 5);

  object.SetBoolean(true);
  ASSERT_EQ(object.GetType(), Object::BOOLEAN);
  ASSERT_EQ(object.GetBoolean(), true);
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectLookupByName) {
  // Test the GetObjectByName() getter.
  typedef draco::StructuralMetadataSchema::Object Object;
  Object object;
  ASSERT_EQ(object.GetType(), Object::OBJECT);

  auto &objects = object.SetObjects();

  objects.push_back(Object("object1", 1));
  objects.push_back(Object("object2", "two"));

  Object object3("object3");
  object3.SetObjects().push_back(Object("child_object", "child"));
  objects.push_back(object3);

  ASSERT_EQ(object.GetObjectByName("child_object"), nullptr);

  ASSERT_NE(object.GetObjectByName("object1"), nullptr);
  ASSERT_EQ(object.GetObjectByName("object1")->GetInteger(), 1);

  ASSERT_NE(object.GetObjectByName("object2"), nullptr);
  ASSERT_EQ(object.GetObjectByName("object2")->GetString(), "two");

  ASSERT_NE(object.GetObjectByName("object3"), nullptr);
  ASSERT_NE(object.GetObjectByName("object3")->GetObjectByName("child_object"),
            nullptr);
  ASSERT_EQ(object.GetObjectByName("object3")
                ->GetObjectByName("child_object")
                ->GetString(),
            "child");
}

TEST(StructuralMetadataSchemaTest, TestSchemaCompare) {
  typedef draco::StructuralMetadataSchema Schema;
  // Test comparison of two schema objects.
  {
    // Compare the same empty schema object.
    Schema a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two empty schema objects.
    Schema a;
    Schema b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two schema objects with different JSON objects.
    Schema a;
    Schema b;
    a.json.SetBoolean(true);
    b.json.SetBoolean(false);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

TEST(StructuralMetadataSchemaTest, TestSchemaObjectCompare) {
  // Test comparison of two schema JSON objects.
  typedef draco::StructuralMetadataSchema::Object Object;
  {
    // Compare the same object.
    Object a;
    ASSERT_TRUE(a == a);
    ASSERT_FALSE(a != a);
  }
  {
    // Compare two default objects.
    Object a;
    Object b;
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two objects with different names.
    Object a("one");
    Object b("two");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two objects with different types.
    Object a;
    Object b;
    a.SetInteger(1);
    b.SetString("one");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two identical string-type objects.
    Object a;
    Object b;
    a.SetString("one");
    b.SetString("one");
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two different string-type objects.
    Object a;
    Object b;
    a.SetString("one");
    b.SetString("two");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two identical integer-type objects.
    Object a;
    Object b;
    a.SetInteger(1);
    b.SetInteger(1);
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two different integer-type objects.
    Object a;
    Object b;
    a.SetInteger(1);
    b.SetInteger(2);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two identical boolean-type objects.
    Object a;
    Object b;
    a.SetBoolean(true);
    b.SetBoolean(true);
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two different boolean-type objects.
    Object a;
    Object b;
    a.SetBoolean(true);
    b.SetBoolean(false);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two identical object-type objects.
    Object a;
    Object b;
    a.SetObjects().emplace_back("one");
    b.SetObjects().emplace_back("one");
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two different object-type objects.
    Object a;
    Object b;
    a.SetObjects().emplace_back("one");
    b.SetObjects().emplace_back("two");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two object-type objects with different counts.
    Object a;
    Object b;
    a.SetObjects().emplace_back("one");
    b.SetObjects().emplace_back("one");
    b.SetObjects().emplace_back("two");
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two identical array-type objects.
    Object a;
    Object b;
    a.SetArray().emplace_back("", 1);
    b.SetArray().emplace_back("", 1);
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
  }
  {
    // Compare two different array-type objects.
    Object a;
    Object b;
    a.SetArray().emplace_back("", 1);
    b.SetArray().emplace_back("", 2);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
  {
    // Compare two array-type objects with different counts.
    Object a;
    Object b;
    a.SetArray().emplace_back("", 1);
    b.SetArray().emplace_back("", 1);
    b.SetArray().emplace_back("", 2);
    ASSERT_FALSE(a == b);
    ASSERT_TRUE(a != b);
  }
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
