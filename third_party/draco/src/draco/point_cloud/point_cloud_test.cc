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

#include <string>
#include <utility>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/metadata/geometry_metadata.h"

namespace {

class PointCloudTest : public ::testing::Test {
 protected:
  PointCloudTest() {}
};

#ifdef DRACO_TRANSCODER_SUPPORTED
TEST_F(PointCloudTest, PointCloudCopy) {
  // Tests that we can copy a point cloud.
  std::unique_ptr<draco::PointCloud> pc =
      draco::ReadPointCloudFromTestFile("pc_kd_color.drc");
  ASSERT_NE(pc, nullptr);

  // Add metadata to the point cloud.
  std::unique_ptr<draco::GeometryMetadata> metadata(
      new draco::GeometryMetadata());
  metadata->AddEntryInt("speed", 1050);
  metadata->AddEntryString("code", "YT-1300f");

  // Add attribute metadata.
  std::unique_ptr<draco::AttributeMetadata> a_metadata(
      new draco::AttributeMetadata());
  a_metadata->set_att_unique_id(pc->attribute(0)->unique_id());
  a_metadata->AddEntryInt("attribute_test", 3);
  metadata->AddAttributeMetadata(std::move(a_metadata));
  pc->AddMetadata(std::move(metadata));

  // Create a copy of the point cloud.
  draco::PointCloud pc_copy;
  pc_copy.Copy(*pc);

  // Check the point cloud data.
  ASSERT_EQ(pc->num_points(), pc_copy.num_points());
  ASSERT_EQ(pc->num_attributes(), pc_copy.num_attributes());
  for (int i = 0; i < pc->num_attributes(); ++i) {
    ASSERT_EQ(pc->attribute(i)->attribute_type(),
              pc_copy.attribute(i)->attribute_type());
  }

  // Check the point cloud metadata.
  int speed;
  std::string code;
  ASSERT_NE(pc->GetMetadata(), nullptr);
  ASSERT_TRUE(pc->GetMetadata()->GetEntryInt("speed", &speed));
  ASSERT_TRUE(pc->GetMetadata()->GetEntryString("code", &code));
  ASSERT_EQ(speed, 1050);
  ASSERT_EQ(code, "YT-1300f");

  const auto *const att_metadata_copy =
      pc->GetMetadata()->GetAttributeMetadataByUniqueId(0);
  ASSERT_NE(att_metadata_copy, nullptr);
  int att_test;
  ASSERT_TRUE(att_metadata_copy->GetEntryInt("attribute_test", &att_test));
  ASSERT_EQ(att_test, 3);
}

TEST_F(PointCloudTest, TestCompressionSettings) {
  // Tests compression settings of a point cloud.
  draco::PointCloud pc;

  // Check that compression is disabled and compression settings are default.
  ASSERT_FALSE(pc.IsCompressionEnabled());
  const draco::DracoCompressionOptions default_compression_options;
  ASSERT_EQ(pc.GetCompressionOptions(), default_compression_options);

  // Check that compression options can be set without enabling compression.
  draco::DracoCompressionOptions compression_options;
  compression_options.quantization_bits_normal = 12;
  pc.SetCompressionOptions(compression_options);
  ASSERT_EQ(pc.GetCompressionOptions(), compression_options);
  ASSERT_FALSE(pc.IsCompressionEnabled());

  // Check that compression can be enabled.
  pc.SetCompressionEnabled(true);
  ASSERT_TRUE(pc.IsCompressionEnabled());

  // Check that individual compression options can be updated.
  pc.GetCompressionOptions().compression_level++;
  pc.GetCompressionOptions().compression_level--;

  // Check that compression settings can be copied.
  draco::PointCloud pc_copy;
  pc_copy.Copy(pc);
  ASSERT_TRUE(pc_copy.IsCompressionEnabled());
  ASSERT_EQ(pc_copy.GetCompressionOptions(), compression_options);
}

TEST_F(PointCloudTest, TestGetNamedAttributeByName) {
  draco::PointCloud pc;
  // Test whether we can get named attributes by name.
  constexpr auto kPosition = draco::GeometryAttribute::POSITION;
  constexpr auto kGeneric = draco::GeometryAttribute::GENERIC;
  draco::GeometryAttribute pos_att;
  draco::GeometryAttribute gen_att0;
  draco::GeometryAttribute gen_att1;
  pos_att.Init(kPosition, nullptr, 3, draco::DT_FLOAT32, false, 12, 0);
  gen_att0.Init(kGeneric, nullptr, 3, draco::DT_FLOAT32, false, 12, 0);
  gen_att1.Init(kGeneric, nullptr, 3, draco::DT_FLOAT32, false, 12, 0);
  pos_att.set_name("Zero");
  gen_att0.set_name("Zero");
  gen_att1.set_name("One");

  // Add one position, and two generic attributes.
  pc.AddAttribute(pos_att, false, 0);
  pc.AddAttribute(gen_att0, false, 0);
  pc.AddAttribute(gen_att1, false, 0);

  // Check added attributes.
  ASSERT_EQ(pc.attribute(0)->attribute_type(), kPosition);
  ASSERT_EQ(pc.attribute(1)->attribute_type(), kGeneric);
  ASSERT_EQ(pc.attribute(2)->attribute_type(), kGeneric);
  ASSERT_EQ(pc.attribute(0)->name(), "Zero");
  ASSERT_EQ(pc.attribute(1)->name(), "Zero");
  ASSERT_EQ(pc.attribute(2)->name(), "One");

  // Check that we can get correct attributes by name.
  ASSERT_EQ(pc.GetNamedAttributeByName(kPosition, "Zero"), pc.attribute(0));
  ASSERT_EQ(pc.GetNamedAttributeByName(kGeneric, "Zero"), pc.attribute(1));
  ASSERT_EQ(pc.GetNamedAttributeByName(kGeneric, "One"), pc.attribute(2));
}
#endif

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
