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

#include "draco/compression/encode.h"

#include <cinttypes>
#include <fstream>
#include <sstream>

#include "draco/attributes/attribute_quantization_transform.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/decode.h"
#include "draco/compression/expert_encode.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/core/vector_d.h"
#include "draco/io/obj_decoder.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"
#include "draco/point_cloud/point_cloud_builder.h"

namespace {

class EncodeTest : public ::testing::Test {
 protected:
  EncodeTest() {}
  std::unique_ptr<draco::Mesh> CreateTestMesh() const {
    draco::TriangleSoupMeshBuilder mesh_builder;

    // Create a simple mesh with one face.
    mesh_builder.Start(1);

    // Add one position attribute and two texture coordinate attributes.
    const int32_t pos_att_id = mesh_builder.AddAttribute(
        draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
    const int32_t tex_att_id_0 = mesh_builder.AddAttribute(
        draco::GeometryAttribute::TEX_COORD, 2, draco::DT_FLOAT32);
    const int32_t tex_att_id_1 = mesh_builder.AddAttribute(
        draco::GeometryAttribute::TEX_COORD, 2, draco::DT_FLOAT32);

    // Initialize the attribute values.
    mesh_builder.SetAttributeValuesForFace(
        pos_att_id, draco::FaceIndex(0), draco::Vector3f(0.f, 0.f, 0.f).data(),
        draco::Vector3f(1.f, 0.f, 0.f).data(),
        draco::Vector3f(1.f, 1.f, 0.f).data());
    mesh_builder.SetAttributeValuesForFace(
        tex_att_id_0, draco::FaceIndex(0), draco::Vector2f(0.f, 0.f).data(),
        draco::Vector2f(1.f, 0.f).data(), draco::Vector2f(1.f, 1.f).data());
    mesh_builder.SetAttributeValuesForFace(
        tex_att_id_1, draco::FaceIndex(0), draco::Vector2f(0.f, 0.f).data(),
        draco::Vector2f(1.f, 0.f).data(), draco::Vector2f(1.f, 1.f).data());

    return mesh_builder.Finalize();
  }

  std::unique_ptr<draco::PointCloud> CreateTestPointCloud() const {
    draco::PointCloudBuilder pc_builder;

    constexpr int kNumPoints = 100;
    constexpr int kNumGenAttCoords0 = 4;
    constexpr int kNumGenAttCoords1 = 6;
    pc_builder.Start(kNumPoints);

    // Add one position attribute and two generic attributes.
    const int32_t pos_att_id = pc_builder.AddAttribute(
        draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
    const int32_t gen_att_id_0 = pc_builder.AddAttribute(
        draco::GeometryAttribute::GENERIC, kNumGenAttCoords0, draco::DT_UINT32);
    const int32_t gen_att_id_1 = pc_builder.AddAttribute(
        draco::GeometryAttribute::GENERIC, kNumGenAttCoords1, draco::DT_UINT8);

    std::vector<uint32_t> gen_att_data_0(kNumGenAttCoords0);
    std::vector<uint32_t> gen_att_data_1(kNumGenAttCoords1);

    // Initialize the attribute values.
    for (draco::PointIndex i(0); i < kNumPoints; ++i) {
      const float pos_coord = static_cast<float>(i.value());
      pc_builder.SetAttributeValueForPoint(
          pos_att_id, i,
          draco::Vector3f(pos_coord, -pos_coord, pos_coord).data());

      for (int j = 0; j < kNumGenAttCoords0; ++j) {
        gen_att_data_0[j] = i.value();
      }
      pc_builder.SetAttributeValueForPoint(gen_att_id_0, i,
                                           gen_att_data_0.data());

      for (int j = 0; j < kNumGenAttCoords1; ++j) {
        gen_att_data_1[j] = -i.value();
      }
      pc_builder.SetAttributeValueForPoint(gen_att_id_1, i,
                                           gen_att_data_1.data());
    }
    return pc_builder.Finalize(false);
  }

  std::unique_ptr<draco::PointCloud> CreateTestPointCloudPosNorm() const {
    draco::PointCloudBuilder pc_builder;

    constexpr int kNumPoints = 20;
    pc_builder.Start(kNumPoints);

    // Add one position attribute and a normal attribute.
    const int32_t pos_att_id = pc_builder.AddAttribute(
        draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
    const int32_t norm_att_id = pc_builder.AddAttribute(
        draco::GeometryAttribute::NORMAL, 3, draco::DT_FLOAT32);

    // Initialize the attribute values.
    for (draco::PointIndex i(0); i < kNumPoints; ++i) {
      const float pos_coord = static_cast<float>(i.value());
      pc_builder.SetAttributeValueForPoint(
          pos_att_id, i,
          draco::Vector3f(pos_coord, -pos_coord, pos_coord).data());

      // Pseudo-random normal.
      draco::Vector3f norm(pos_coord * 2.f, pos_coord - 2.f, pos_coord * 3.f);
      norm.Normalize();
      pc_builder.SetAttributeValueForPoint(norm_att_id, i, norm.data());
    }

    return pc_builder.Finalize(false);
  }

  int GetQuantizationBitsFromAttribute(const draco::PointAttribute *att) const {
    if (att == nullptr) {
      return -1;
    }
    draco::AttributeQuantizationTransform transform;
    if (!transform.InitFromAttribute(*att)) {
      return -1;
    }
    return transform.quantization_bits();
  }

  void VerifyNumQuantizationBits(const draco::EncoderBuffer &buffer,
                                 int pos_quantization,
                                 int tex_coord_0_quantization,
                                 int tex_coord_1_quantization) const {
    draco::Decoder decoder;

    // Skip the dequantization for the attributes which will allow us to get
    // the number of quantization bits used during encoding.
    decoder.SetSkipAttributeTransform(draco::GeometryAttribute::POSITION);
    decoder.SetSkipAttributeTransform(draco::GeometryAttribute::TEX_COORD);

    draco::DecoderBuffer in_buffer;
    in_buffer.Init(buffer.data(), buffer.size());
    auto mesh = decoder.DecodeMeshFromBuffer(&in_buffer).value();
    ASSERT_NE(mesh, nullptr);
    ASSERT_EQ(GetQuantizationBitsFromAttribute(mesh->attribute(0)),
              pos_quantization);
    ASSERT_EQ(GetQuantizationBitsFromAttribute(mesh->attribute(1)),
              tex_coord_0_quantization);
    ASSERT_EQ(GetQuantizationBitsFromAttribute(mesh->attribute(2)),
              tex_coord_1_quantization);
  }

  // Tests that the encoder returns the correct number of encoded points and
  // faces for a given mesh or point cloud.
  void TestNumberOfEncodedEntries(const std::string &file_name,
                                  int32_t encoding_method) {
    std::unique_ptr<draco::PointCloud> geometry;
    draco::Mesh *mesh = nullptr;

    if (encoding_method == draco::MESH_EDGEBREAKER_ENCODING ||
        encoding_method == draco::MESH_SEQUENTIAL_ENCODING) {
      std::unique_ptr<draco::Mesh> mesh_tmp =
          draco::ReadMeshFromTestFile(file_name);
      mesh = mesh_tmp.get();
      if (!mesh->DeduplicateAttributeValues()) {
        return;
      }
      mesh->DeduplicatePointIds();
      geometry = std::move(mesh_tmp);
    } else {
      geometry = draco::ReadPointCloudFromTestFile(file_name);
    }
    ASSERT_NE(mesh, nullptr);

    draco::Encoder encoder;
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 12);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 10);

    encoder.SetEncodingMethod(encoding_method);

    encoder.SetTrackEncodedProperties(true);

    draco::EncoderBuffer buffer;
    if (mesh) {
      encoder.EncodeMeshToBuffer(*mesh, &buffer);
    } else {
      encoder.EncodePointCloudToBuffer(*geometry, &buffer);
    }

    // Ensure the logged number of encoded points and faces matches the number
    // we get from the decoder.

    draco::DecoderBuffer decoder_buffer;
    decoder_buffer.Init(buffer.data(), buffer.size());
    draco::Decoder decoder;

    if (mesh) {
      auto maybe_mesh = decoder.DecodeMeshFromBuffer(&decoder_buffer);
      ASSERT_TRUE(maybe_mesh.ok());
      auto decoded_mesh = std::move(maybe_mesh).value();
      ASSERT_NE(decoded_mesh, nullptr);
      ASSERT_EQ(decoded_mesh->num_points(), encoder.num_encoded_points());
      ASSERT_EQ(decoded_mesh->num_faces(), encoder.num_encoded_faces());
    } else {
      auto maybe_pc = decoder.DecodePointCloudFromBuffer(&decoder_buffer);
      ASSERT_TRUE(maybe_pc.ok());
      auto decoded_pc = std::move(maybe_pc).value();
      ASSERT_EQ(decoded_pc->num_points(), encoder.num_encoded_points());
    }
  }
};

TEST_F(EncodeTest, TestExpertEncoderQuantization) {
  // This test verifies that the expert encoder can quantize individual
  // attributes even if they have the same type.
  auto mesh = CreateTestMesh();
  ASSERT_NE(mesh, nullptr);

  draco::ExpertEncoder encoder(*mesh);
  encoder.SetAttributeQuantization(0, 16);  // Position quantization.
  encoder.SetAttributeQuantization(1, 15);  // Tex-coord 0 quantization.
  encoder.SetAttributeQuantization(2, 14);  // Tex-coord 1 quantization.

  draco::EncoderBuffer buffer;
  encoder.EncodeToBuffer(&buffer);
  VerifyNumQuantizationBits(buffer, 16, 15, 14);
}

TEST_F(EncodeTest, TestEncoderQuantization) {
  // This test verifies that Encoder applies the same quantization to all
  // attributes of the same type.
  auto mesh = CreateTestMesh();
  ASSERT_NE(mesh, nullptr);

  draco::Encoder encoder;
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 16);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 15);

  draco::EncoderBuffer buffer;
  encoder.EncodeMeshToBuffer(*mesh, &buffer);
  VerifyNumQuantizationBits(buffer, 16, 15, 15);
}

TEST_F(EncodeTest, TestLinesObj) {
  // This test verifies that Encoder can encode file that contains only line
  // segments (that are ignored).
  std::unique_ptr<draco::Mesh> mesh(
      draco::ReadMeshFromTestFile("test_lines.obj"));
  ASSERT_NE(mesh, nullptr);
  ASSERT_EQ(mesh->num_faces(), 0);
  std::unique_ptr<draco::PointCloud> pc(
      draco::ReadPointCloudFromTestFile("test_lines.obj"));
  ASSERT_NE(pc, nullptr);

  draco::Encoder encoder;
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 16);

  draco::EncoderBuffer buffer;
  ASSERT_TRUE(encoder.EncodePointCloudToBuffer(*pc, &buffer).ok());
}

TEST_F(EncodeTest, TestQuantizedInfinity) {
  // This test verifies that Encoder fails to encode point cloud when requesting
  // quantization of attribute that contains infinity values.
  std::unique_ptr<draco::PointCloud> pc(
      draco::ReadPointCloudFromTestFile("float_inf_point_cloud.ply"));
  ASSERT_NE(pc, nullptr);

  {
    draco::Encoder encoder;
    encoder.SetEncodingMethod(draco::POINT_CLOUD_SEQUENTIAL_ENCODING);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 11);

    draco::EncoderBuffer buffer;
    ASSERT_FALSE(encoder.EncodePointCloudToBuffer(*pc, &buffer).ok());
  }

  {
    draco::Encoder encoder;
    encoder.SetEncodingMethod(draco::POINT_CLOUD_KD_TREE_ENCODING);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 11);

    draco::EncoderBuffer buffer;
    ASSERT_FALSE(encoder.EncodePointCloudToBuffer(*pc, &buffer).ok());
  }
}

TEST_F(EncodeTest, TestUnquantizedInfinity) {
  // This test verifies that Encoder can successfully encode point cloud when
  // not requesting quantization of attribute that contains infinity values.
  std::unique_ptr<draco::PointCloud> pc(
      draco::ReadPointCloudFromTestFile("float_inf_point_cloud.ply"));
  ASSERT_NE(pc, nullptr);

  // Note that the KD tree encoding method is not applicable to float values.
  draco::Encoder encoder;
  encoder.SetEncodingMethod(draco::POINT_CLOUD_SEQUENTIAL_ENCODING);

  draco::EncoderBuffer buffer;
  ASSERT_TRUE(encoder.EncodePointCloudToBuffer(*pc, &buffer).ok());
}

TEST_F(EncodeTest, TestQuantizedAndUnquantizedAttributes) {
  // This test verifies that Encoder can successfully encode point cloud with
  // two float attribiutes - one quantized and another unquantized. The encoder
  // defaults to sequential encoding in this case.
  std::unique_ptr<draco::PointCloud> pc(
      draco::ReadPointCloudFromTestFile("float_two_att_point_cloud.ply"));
  ASSERT_NE(pc, nullptr);

  draco::Encoder encoder;
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 11);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 0);
  draco::EncoderBuffer buffer;
  ASSERT_TRUE(encoder.EncodePointCloudToBuffer(*pc, &buffer).ok());
}

TEST_F(EncodeTest, TestKdTreeEncoding) {
  // This test verifies that the API can successfully encode a point cloud
  // defined by several attributes using the kd tree method.
  std::unique_ptr<draco::PointCloud> pc = CreateTestPointCloud();
  ASSERT_NE(pc, nullptr);

  draco::EncoderBuffer buffer;
  draco::Encoder encoder;
  encoder.SetEncodingMethod(draco::POINT_CLOUD_KD_TREE_ENCODING);
  // First try it without quantizing positions which should fail.
  ASSERT_FALSE(encoder.EncodePointCloudToBuffer(*pc, &buffer).ok());

  // Now set quantization for the position attribute which should make
  // the encoder happy.
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 16);
  ASSERT_TRUE(encoder.EncodePointCloudToBuffer(*pc, &buffer).ok());
}

TEST_F(EncodeTest, TestTrackingOfNumberOfEncodedEntries) {
  TestNumberOfEncodedEntries("deg_faces.obj", draco::MESH_EDGEBREAKER_ENCODING);
  TestNumberOfEncodedEntries("deg_faces.obj", draco::MESH_SEQUENTIAL_ENCODING);
  TestNumberOfEncodedEntries("cube_att.obj", draco::MESH_EDGEBREAKER_ENCODING);
  TestNumberOfEncodedEntries("test_nm.obj", draco::MESH_EDGEBREAKER_ENCODING);
  TestNumberOfEncodedEntries("test_nm.obj", draco::MESH_SEQUENTIAL_ENCODING);
  TestNumberOfEncodedEntries("cube_subd.obj",
                             draco::POINT_CLOUD_KD_TREE_ENCODING);
  TestNumberOfEncodedEntries("cube_subd.obj",
                             draco::POINT_CLOUD_SEQUENTIAL_ENCODING);
}

TEST_F(EncodeTest, TestTrackingOfNumberOfEncodedEntriesNotSet) {
  // Tests that when tracing of encoded properties is disabled, the returned
  // number of encoded faces and points is 0.
  std::unique_ptr<draco::Mesh> mesh(
      draco::ReadMeshFromTestFile("cube_att.obj"));
  ASSERT_NE(mesh, nullptr);

  draco::EncoderBuffer buffer;
  draco::Encoder encoder;

  ASSERT_TRUE(encoder.EncodeMeshToBuffer(*mesh, &buffer).ok());
  ASSERT_EQ(encoder.num_encoded_points(), 0);
  ASSERT_EQ(encoder.num_encoded_faces(), 0);
}

TEST_F(EncodeTest, TestNoPosQuantizationNormalCoding) {
  // Tests that we can encode and decode a file with quantized normals but
  // non-quantized positions.
  const auto mesh = draco::ReadMeshFromTestFile("test_nm.obj");
  ASSERT_NE(mesh, nullptr);

  // The mesh should have positions and normals.
  ASSERT_NE(mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION),
            nullptr);
  ASSERT_NE(mesh->GetNamedAttribute(draco::GeometryAttribute::NORMAL), nullptr);

  draco::EncoderBuffer buffer;
  draco::Encoder encoder;
  // No quantization for positions.
  encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 8);

  DRACO_ASSERT_OK(encoder.EncodeMeshToBuffer(*mesh, &buffer));

  draco::Decoder decoder;

  draco::DecoderBuffer in_buffer;
  in_buffer.Init(buffer.data(), buffer.size());
  const auto decoded_mesh = decoder.DecodeMeshFromBuffer(&in_buffer).value();
  ASSERT_NE(decoded_mesh, nullptr);
}

}  // namespace
