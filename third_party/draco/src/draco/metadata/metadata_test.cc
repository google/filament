// Copyright 2017 The Draco Authors.
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
#include "draco/metadata/metadata.h"

#include <memory>
#include <string>

#include "draco/core/draco_test_base.h"
#include "draco/metadata/geometry_metadata.h"

namespace {

class MetadataTest : public ::testing::Test {
 protected:
  MetadataTest() {}

  draco::Metadata metadata;
  draco::GeometryMetadata geometry_metadata;
};

TEST_F(MetadataTest, TestRemoveEntry) {
  metadata.AddEntryInt("int", 100);
  metadata.RemoveEntry("int");
  int32_t int_value = 0;
  ASSERT_FALSE(metadata.GetEntryInt("int", &int_value));
}

TEST_F(MetadataTest, TestSingleEntry) {
  metadata.AddEntryInt("int", 100);
  int32_t int_value = 0;
  ASSERT_TRUE(metadata.GetEntryInt("int", &int_value));
  ASSERT_EQ(int_value, 100);

  metadata.AddEntryDouble("double", 1.234);
  double double_value = 0.0;
  ASSERT_TRUE(metadata.GetEntryDouble("double", &double_value));
  ASSERT_EQ(double_value, 1.234);
}

TEST_F(MetadataTest, TestWriteOverEntry) {
  metadata.AddEntryInt("int", 100);
  metadata.AddEntryInt("int", 200);
  int32_t int_value = 0;
  ASSERT_TRUE(metadata.GetEntryInt("int", &int_value));
  ASSERT_EQ(int_value, 200);
}

TEST_F(MetadataTest, TestArrayEntry) {
  std::vector<int32_t> int_array({1, 2, 3});
  metadata.AddEntryIntArray("int_array", int_array);
  std::vector<int32_t> return_int_array;
  ASSERT_TRUE(metadata.GetEntryIntArray("int_array", &return_int_array));
  ASSERT_EQ(return_int_array.size(), 3);
  ASSERT_EQ(return_int_array[0], 1);
  ASSERT_EQ(return_int_array[1], 2);
  ASSERT_EQ(return_int_array[2], 3);

  std::vector<double> double_array({0.1, 0.2, 0.3});
  metadata.AddEntryDoubleArray("double_array", double_array);
  std::vector<double> return_double_array;
  ASSERT_TRUE(
      metadata.GetEntryDoubleArray("double_array", &return_double_array));
  ASSERT_EQ(return_double_array.size(), 3);
  ASSERT_EQ(return_double_array[0], 0.1);
  ASSERT_EQ(return_double_array[1], 0.2);
  ASSERT_EQ(return_double_array[2], 0.3);
}

TEST_F(MetadataTest, TestStringEntry) {
  const std::string entry_value = "test string entry";
  metadata.AddEntryString("string", entry_value);
  std::string return_value;
  ASSERT_TRUE(metadata.GetEntryString("string", &return_value));
  ASSERT_EQ(entry_value.size(), return_value.size());
  ASSERT_EQ(entry_value, return_value);
}

TEST_F(MetadataTest, TestBinaryEntry) {
  const std::vector<uint8_t> binarydata({0x1, 0x2, 0x3, 0x4});
  metadata.AddEntryBinary("binary_data", binarydata);
  std::vector<uint8_t> return_binarydata;
  ASSERT_TRUE(metadata.GetEntryBinary("binary_data", &return_binarydata));
  ASSERT_EQ(binarydata.size(), return_binarydata.size());
  for (int i = 0; i < binarydata.size(); ++i) {
    ASSERT_EQ(binarydata[i], return_binarydata[i]);
  }
}

TEST_F(MetadataTest, TestNestedMetadata) {
  std::unique_ptr<draco::Metadata> sub_metadata =
      std::unique_ptr<draco::Metadata>(new draco::Metadata());
  sub_metadata->AddEntryInt("int", 100);

  metadata.AddSubMetadata("sub0", std::move(sub_metadata));
  const auto sub_metadata_ptr = metadata.sub_metadata("sub0");
  ASSERT_NE(sub_metadata_ptr, nullptr);

  int32_t int_value = 0;
  ASSERT_TRUE(sub_metadata_ptr->GetEntryInt("int", &int_value));
  ASSERT_EQ(int_value, 100);

  sub_metadata_ptr->AddEntryInt("new_entry", 20);
  ASSERT_TRUE(sub_metadata_ptr->GetEntryInt("new_entry", &int_value));
  ASSERT_EQ(int_value, 20);
}

TEST_F(MetadataTest, TestHardCopyMetadata) {
  metadata.AddEntryInt("int", 100);
  std::unique_ptr<draco::Metadata> sub_metadata =
      std::unique_ptr<draco::Metadata>(new draco::Metadata());
  sub_metadata->AddEntryInt("int", 200);
  metadata.AddSubMetadata("sub0", std::move(sub_metadata));

  draco::Metadata copied_metadata(metadata);

  int32_t int_value = 0;
  ASSERT_TRUE(copied_metadata.GetEntryInt("int", &int_value));
  ASSERT_EQ(int_value, 100);

  const auto sub_metadata_ptr = copied_metadata.GetSubMetadata("sub0");
  ASSERT_NE(sub_metadata_ptr, nullptr);

  int32_t sub_int_value = 0;
  ASSERT_TRUE(sub_metadata_ptr->GetEntryInt("int", &sub_int_value));
  ASSERT_EQ(sub_int_value, 200);
}

TEST_F(MetadataTest, TestGeometryMetadata) {
  std::unique_ptr<draco::AttributeMetadata> att_metadata =
      std::unique_ptr<draco::AttributeMetadata>(new draco::AttributeMetadata());
  att_metadata->set_att_unique_id(10);
  att_metadata->AddEntryInt("int", 100);
  att_metadata->AddEntryString("name", "pos");

  ASSERT_FALSE(geometry_metadata.AddAttributeMetadata(nullptr));
  ASSERT_TRUE(geometry_metadata.AddAttributeMetadata(std::move(att_metadata)));

  ASSERT_NE(geometry_metadata.GetAttributeMetadataByUniqueId(10), nullptr);
  ASSERT_EQ(geometry_metadata.GetAttributeMetadataByUniqueId(1), nullptr);

  const draco::AttributeMetadata *requested_att_metadata =
      geometry_metadata.GetAttributeMetadataByStringEntry("name", "pos");
  ASSERT_NE(requested_att_metadata, nullptr);
  ASSERT_EQ(
      geometry_metadata.GetAttributeMetadataByStringEntry("name", "not_exists"),
      nullptr);
}

}  // namespace
