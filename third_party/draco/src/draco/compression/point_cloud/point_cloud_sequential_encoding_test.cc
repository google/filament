// Copyright 2016 The Draco Authors.
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
#include "draco/compression/point_cloud/point_cloud_sequential_decoder.h"
#include "draco/compression/point_cloud/point_cloud_sequential_encoder.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/obj_decoder.h"

namespace draco {

class PointCloudSequentialEncodingTest : public ::testing::Test {
 protected:
  std::unique_ptr<PointCloud> EncodeAndDecodePointCloud(const PointCloud *pc) {
    EncoderBuffer buffer;
    PointCloudSequentialEncoder encoder;
    EncoderOptions options = EncoderOptions::CreateDefaultOptions();
    encoder.SetPointCloud(*pc);
    if (!encoder.Encode(options, &buffer).ok()) {
      return nullptr;
    }

    DecoderBuffer dec_buffer;
    dec_buffer.Init(buffer.data(), buffer.size());
    PointCloudSequentialDecoder decoder;

    std::unique_ptr<PointCloud> out_pc(new PointCloud());
    DecoderOptions dec_options;
    if (!decoder.Decode(dec_options, &dec_buffer, out_pc.get()).ok()) {
      return nullptr;
    }
    return out_pc;
  }

  void TestEncoding(const std::string &file_name) {
    std::unique_ptr<PointCloud> pc = ReadPointCloudFromTestFile(file_name);
    ASSERT_NE(pc, nullptr);

    std::unique_ptr<PointCloud> decoded_pc =
        EncodeAndDecodePointCloud(pc.get());
    ASSERT_NE(decoded_pc.get(), nullptr);
    ASSERT_EQ(decoded_pc->num_points(), pc->num_points());
  }
};

TEST_F(PointCloudSequentialEncodingTest, DoesEncodeAndDecode) {
  TestEncoding("test_nm.obj");
}

TEST_F(PointCloudSequentialEncodingTest, EncodingPointCloudWithMetadata) {
  std::unique_ptr<PointCloud> pc = ReadPointCloudFromTestFile("test_nm.obj");
  ASSERT_NE(pc, nullptr);
  // Add metadata to point cloud.
  std::unique_ptr<GeometryMetadata> metadata =
      std::unique_ptr<GeometryMetadata>(new GeometryMetadata());
  const uint32_t pos_att_id =
      pc->GetNamedAttributeId(GeometryAttribute::POSITION);
  std::unique_ptr<AttributeMetadata> pos_metadata =
      std::unique_ptr<AttributeMetadata>(new AttributeMetadata());
  pos_metadata->AddEntryString("name", "position");
  pc->AddAttributeMetadata(pos_att_id, std::move(pos_metadata));

  std::unique_ptr<PointCloud> decoded_pc = EncodeAndDecodePointCloud(pc.get());
  ASSERT_NE(decoded_pc.get(), nullptr);

  const GeometryMetadata *const pc_metadata = decoded_pc->GetMetadata();
  ASSERT_NE(pc_metadata, nullptr);
  // Test getting attribute metadata by id.
  ASSERT_NE(pc->GetAttributeMetadataByAttributeId(pos_att_id), nullptr);
  // Test getting attribute metadata by entry name value pair.
  const AttributeMetadata *const requested_att_metadata =
      pc_metadata->GetAttributeMetadataByStringEntry("name", "position");
  ASSERT_NE(requested_att_metadata, nullptr);
  ASSERT_EQ(requested_att_metadata->att_unique_id(),
            pc->attribute(pos_att_id)->unique_id());
}

// TODO(ostava): Test the reusability of a single instance of the encoder and
// decoder class.

}  // namespace draco
