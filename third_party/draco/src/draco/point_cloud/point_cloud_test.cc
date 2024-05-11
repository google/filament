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
#include "draco/point_cloud/point_cloud.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/metadata/geometry_metadata.h"

namespace {

class PointCloudTest : public ::testing::Test {
 protected:
  PointCloudTest() {}
};

TEST_F(PointCloudTest, TestAttributeDeletion) {
  draco::PointCloud pc;
  // Test whether we can correctly delete an attribute from a point cloud.
  // Create some attributes for the point cloud.
  draco::GeometryAttribute pos_att;
  pos_att.Init(draco::GeometryAttribute::POSITION, nullptr, 3,
               draco::DT_FLOAT32, false, 12, 0);
  draco::GeometryAttribute norm_att;
  norm_att.Init(draco::GeometryAttribute::NORMAL, nullptr, 3, draco::DT_FLOAT32,
                false, 12, 0);
  draco::GeometryAttribute gen_att;
  gen_att.Init(draco::GeometryAttribute::GENERIC, nullptr, 3, draco::DT_FLOAT32,
               false, 12, 0);

  // Add one position, two normal and two generic attributes.
  pc.AddAttribute(pos_att, false, 0);
  pc.AddAttribute(gen_att, false, 0);
  pc.AddAttribute(norm_att, false, 0);
  pc.AddAttribute(gen_att, false, 0);
  pc.AddAttribute(norm_att, false, 0);

  ASSERT_EQ(pc.num_attributes(), 5);
  ASSERT_EQ(pc.attribute(0)->attribute_type(),
            draco::GeometryAttribute::POSITION);
  ASSERT_EQ(pc.attribute(3)->attribute_type(),
            draco::GeometryAttribute::GENERIC);

  // Delete generic attribute.
  pc.DeleteAttribute(1);
  ASSERT_EQ(pc.num_attributes(), 4);
  ASSERT_EQ(pc.attribute(1)->attribute_type(),
            draco::GeometryAttribute::NORMAL);
  ASSERT_EQ(pc.NumNamedAttributes(draco::GeometryAttribute::NORMAL), 2);
  ASSERT_EQ(pc.GetNamedAttributeId(draco::GeometryAttribute::NORMAL, 1), 3);

  // Delete the first normal attribute.
  pc.DeleteAttribute(1);
  ASSERT_EQ(pc.num_attributes(), 3);
  ASSERT_EQ(pc.attribute(1)->attribute_type(),
            draco::GeometryAttribute::GENERIC);
  ASSERT_EQ(pc.NumNamedAttributes(draco::GeometryAttribute::NORMAL), 1);
  ASSERT_EQ(pc.GetNamedAttributeId(draco::GeometryAttribute::NORMAL, 0), 2);
}

TEST_F(PointCloudTest, TestPointCloudWithMetadata) {
  draco::PointCloud pc;
  std::unique_ptr<draco::GeometryMetadata> metadata =
      std::unique_ptr<draco::GeometryMetadata>(new draco::GeometryMetadata());

  // Add a position attribute metadata.
  draco::GeometryAttribute pos_att;
  pos_att.Init(draco::GeometryAttribute::POSITION, nullptr, 3,
               draco::DT_FLOAT32, false, 12, 0);
  const uint32_t pos_att_id = pc.AddAttribute(pos_att, false, 0);
  ASSERT_EQ(pos_att_id, 0);
  std::unique_ptr<draco::AttributeMetadata> pos_metadata =
      std::unique_ptr<draco::AttributeMetadata>(new draco::AttributeMetadata());
  pos_metadata->AddEntryString("name", "position");
  pc.AddAttributeMetadata(pos_att_id, std::move(pos_metadata));
  const draco::GeometryMetadata *pc_metadata = pc.GetMetadata();
  ASSERT_NE(pc_metadata, nullptr);
  // Add a generic material attribute metadata.
  draco::GeometryAttribute material_att;
  material_att.Init(draco::GeometryAttribute::GENERIC, nullptr, 3,
                    draco::DT_FLOAT32, false, 12, 0);
  const uint32_t material_att_id = pc.AddAttribute(material_att, false, 0);
  ASSERT_EQ(material_att_id, 1);
  std::unique_ptr<draco::AttributeMetadata> material_metadata =
      std::unique_ptr<draco::AttributeMetadata>(new draco::AttributeMetadata());
  material_metadata->AddEntryString("name", "material");
  // The material attribute has id of 1 now.
  pc.AddAttributeMetadata(material_att_id, std::move(material_metadata));

  // Test if the attribute metadata is correctly added.
  const draco::AttributeMetadata *requested_pos_metadata =
      pc.GetAttributeMetadataByStringEntry("name", "position");
  ASSERT_NE(requested_pos_metadata, nullptr);
  const draco::AttributeMetadata *requested_mat_metadata =
      pc.GetAttributeMetadataByStringEntry("name", "material");
  ASSERT_NE(requested_mat_metadata, nullptr);

  // Attribute id should be preserved.
  ASSERT_EQ(
      pc.GetAttributeIdByUniqueId(requested_pos_metadata->att_unique_id()), 0);
  ASSERT_EQ(
      pc.GetAttributeIdByUniqueId(requested_mat_metadata->att_unique_id()), 1);

  // Test deleting attribute with metadata.
  pc.DeleteAttribute(pos_att_id);
  ASSERT_EQ(pc.GetAttributeMetadataByStringEntry("name", "position"), nullptr);

  requested_mat_metadata =
      pc.GetAttributeMetadataByStringEntry("name", "material");
  // The unique id should not be changed.
  ASSERT_EQ(requested_mat_metadata->att_unique_id(), 1);
  // Now position attribute is removed, material attribute should have the
  // attribute id of 0.
  ASSERT_EQ(
      pc.GetAttributeIdByUniqueId(requested_mat_metadata->att_unique_id()), 0);
  // Should be able to get metadata using the current attribute id.
  // Attribute id of material attribute is changed from 1 to 0.
  ASSERT_NE(pc.GetAttributeMetadataByAttributeId(0), nullptr);
}

}  // namespace
