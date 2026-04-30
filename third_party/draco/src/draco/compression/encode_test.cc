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
#include "draco/io/file_utils.h"
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
      DRACO_ASSIGN_OR_ASSERT(auto decoded_mesh,
                             decoder.DecodeMeshFromBuffer(&decoder_buffer));
      ASSERT_NE(decoded_mesh, nullptr);
      ASSERT_EQ(decoded_mesh->num_points(), encoder.num_encoded_points());
      ASSERT_EQ(decoded_mesh->num_faces(), encoder.num_encoded_faces());
    } else {
      DRACO_ASSIGN_OR_ASSERT(
          auto decoded_pc, decoder.DecodePointCloudFromBuffer(&decoder_buffer));
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
  DRACO_ASSERT_OK(encoder.EncodePointCloudToBuffer(*pc, &buffer));
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
  DRACO_ASSERT_OK(encoder.EncodePointCloudToBuffer(*pc, &buffer));
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
  DRACO_ASSERT_OK(encoder.EncodePointCloudToBuffer(*pc, &buffer));
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
  DRACO_ASSERT_OK(encoder.EncodePointCloudToBuffer(*pc, &buffer));
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

  DRACO_ASSERT_OK(encoder.EncodeMeshToBuffer(*mesh, &buffer));
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

#ifdef DRACO_TRANSCODER_SUPPORTED
TEST_F(EncodeTest, TestDracoCompressionOptions) {
  // This test verifies that we can set the encoder's compression options via
  // draco::Mesh's compression options.
  const auto mesh = draco::ReadMeshFromTestFile("test_nm.obj");
  ASSERT_NE(mesh, nullptr);

  // First set compression level and quantization manually.
  draco::Encoder encoder_manual;
  draco::EncoderBuffer buffer_manual;
  encoder_manual.SetAttributeQuantization(draco::GeometryAttribute::POSITION,
                                          8);
  encoder_manual.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 7);
  encoder_manual.SetSpeedOptions(4, 4);

  DRACO_ASSERT_OK(encoder_manual.EncodeMeshToBuffer(*mesh, &buffer_manual));

  // Now do the same with options provided via DracoCompressionOptions.
  draco::DracoCompressionOptions compression_options;
  compression_options.compression_level = 6;
  compression_options.quantization_position.SetQuantizationBits(8);
  compression_options.quantization_bits_normal = 7;
  mesh->SetCompressionOptions(compression_options);
  mesh->SetCompressionEnabled(true);

  draco::Encoder encoder_auto;
  draco::EncoderBuffer buffer_auto;
  DRACO_ASSERT_OK(encoder_auto.EncodeMeshToBuffer(*mesh, &buffer_auto));

  // Ensure that both encoders produce the same result.
  ASSERT_EQ(buffer_manual.size(), buffer_auto.size());

  // Now change some of the mesh's compression settings and ensure the
  // compression changes as well.
  compression_options.compression_level = 7;
  mesh->SetCompressionOptions(compression_options);
  buffer_auto.Clear();
  DRACO_ASSERT_OK(encoder_auto.EncodeMeshToBuffer(*mesh, &buffer_auto));
  ASSERT_NE(buffer_manual.size(), buffer_auto.size());

  // Check that |mesh| compression options do not override the encoder options.
  mesh->GetCompressionOptions().compression_level = 10;
  mesh->GetCompressionOptions().quantization_position.SetQuantizationBits(10);
  mesh->GetCompressionOptions().quantization_bits_normal = 10;
  draco::EncoderBuffer buffer;
  DRACO_ASSERT_OK(encoder_manual.EncodeMeshToBuffer(*mesh, &buffer));
  ASSERT_EQ(buffer.size(), buffer_manual.size());
}

TEST_F(EncodeTest, TestDracoCompressionOptionsManualOverride) {
  // This test verifies that we can use encoder's option to override compression
  // options provided in draco::Mesh's compression options.
  const auto mesh = draco::ReadMeshFromTestFile("test_nm.obj");
  ASSERT_NE(mesh, nullptr);

  // Set some compression options.
  draco::DracoCompressionOptions compression_options;
  compression_options.compression_level = 6;
  compression_options.quantization_position.SetQuantizationBits(8);
  compression_options.quantization_bits_normal = 7;
  mesh->SetCompressionOptions(compression_options);
  mesh->SetCompressionEnabled(true);

  draco::Encoder encoder;
  draco::EncoderBuffer buffer_no_override;
  DRACO_ASSERT_OK(encoder.EncodeMeshToBuffer(*mesh, &buffer_no_override));

  // Now override some options and ensure the compression is different.
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 5);
  draco::EncoderBuffer buffer_with_override;
  DRACO_ASSERT_OK(encoder.EncodeMeshToBuffer(*mesh, &buffer_with_override));
  ASSERT_LT(buffer_with_override.size(), buffer_no_override.size());
}

TEST_F(EncodeTest, TestDracoCompressionOptionsGridQuantization) {
  // Test verifies that we can set position quantization via grid spacing.

  // 1x1x1 cube.
  const auto mesh = draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);
  mesh->SetCompressionEnabled(true);

  // Set grid quantization for positions.
  draco::DracoCompressionOptions compression_options;
  // This should result in 10x10x10 quantization.
  compression_options.quantization_position.SetGrid(0.1);
  mesh->SetCompressionOptions(compression_options);

  draco::ExpertEncoder encoder(*mesh);
  draco::EncoderBuffer buffer;
  DRACO_ASSERT_OK(encoder.EncodeToBuffer(&buffer));

  // The grid options should be reflected in the |encoder|. Check that the
  // computed values are correct.
  const int pos_att_id =
      mesh->GetNamedAttributeId(draco::GeometryAttribute::POSITION);
  draco::Vector3f origin;
  encoder.options().GetAttributeVector(pos_att_id, "quantization_origin", 3,
                                       &origin[0]);
  ASSERT_EQ(origin, draco::Vector3f(0.f, 0.f, 0.f));

  // We need 4 quantization bits (for 10 values).
  ASSERT_EQ(
      encoder.options().GetAttributeInt(pos_att_id, "quantization_bits", -1),
      4);

  // The quantization range should be ((1 << quantization_bits) - 1) * spacing.
  ASSERT_NEAR(encoder.options().GetAttributeFloat(pos_att_id,
                                                  "quantization_range", 0.f),
              15.f * 0.1f, 1e-6f);
}

TEST_F(EncodeTest, TestPointCloudGridQuantization) {
  // Test verifies that we can set position quantization via grid spacing for a
  // point cloud.
  const auto pc = draco::ReadPointCloudFromTestFile("cube_att.obj");
  ASSERT_NE(pc, nullptr);
  const int pos_att_id =
      pc->GetNamedAttributeId(draco::GeometryAttribute::POSITION);

  draco::ExpertEncoder encoder(*pc);
  encoder.SetAttributeGridQuantization(*pc, pos_att_id, 0.15);
  draco::EncoderBuffer buffer;
  DRACO_ASSERT_OK(encoder.EncodeToBuffer(&buffer));

  // The grid options should be reflected in the |encoder|. Check that the
  // computed values are correct.
  draco::Vector3f origin;
  encoder.options().GetAttributeVector(pos_att_id, "quantization_origin", 3,
                                       &origin[0]);
  ASSERT_EQ(origin, draco::Vector3f(0.f, 0.f, 0.f));

  // We need 3 quantization bits for 8 grid values [0.00, 0.15, ...,1.05].
  ASSERT_EQ(
      encoder.options().GetAttributeInt(pos_att_id, "quantization_bits", -1),
      3);

  // The quantization range should be ((1 << quantization_bits) - 1) * spacing.
  ASSERT_NEAR(encoder.options().GetAttributeFloat(pos_att_id,
                                                  "quantization_range", 0.f),
              1.05f, 1e-6f);
}

TEST_F(EncodeTest, TestPointCloudGridQuantizationFromCompressionOptions) {
  // Test verifies that we can set position quantization via grid spacing for a
  // point cloud using DracoCompressionOptions.
  const auto pc = draco::ReadPointCloudFromTestFile("cube_att.obj");
  ASSERT_NE(pc, nullptr);
  pc->SetCompressionEnabled(true);

  // Set grid quantization for positions.
  draco::DracoCompressionOptions compression_options;
  // This should result in 10x10x10 quantization.
  compression_options.quantization_position.SetGrid(0.15);
  pc->SetCompressionOptions(compression_options);

  draco::ExpertEncoder encoder(*pc);
  draco::EncoderBuffer buffer;
  DRACO_ASSERT_OK(encoder.EncodeToBuffer(&buffer));

  // The grid options should be reflected in the |encoder|. Check that the
  // computed values are correct.
  const int pos_att_id =
      pc->GetNamedAttributeId(draco::GeometryAttribute::POSITION);
  draco::Vector3f origin;
  encoder.options().GetAttributeVector(pos_att_id, "quantization_origin", 3,
                                       &origin[0]);
  ASSERT_EQ(origin, draco::Vector3f(0.f, 0.f, 0.f));

  // We need 3 quantization bits for 8 grid values [0.00, 0.15, ...,1.05].
  ASSERT_EQ(
      encoder.options().GetAttributeInt(pos_att_id, "quantization_bits", -1),
      3);

  // The quantization range should be ((1 << quantization_bits) - 1) * spacing.
  ASSERT_NEAR(encoder.options().GetAttributeFloat(pos_att_id,
                                                  "quantization_range", 0.f),
              1.05f, 1e-6f);
}

TEST_F(EncodeTest, TestDracoCompressionOptionsGridQuantizationWithOffset) {
  // Test verifies that we can set position quantization via grid spacing when
  // the geometry is not perfectly aligned with the quantization grid.

  // 1x1x1 cube.
  const auto mesh = draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  // Move all positions a bit.
  auto *pos_att = mesh->attribute(
      mesh->GetNamedAttributeId(draco::GeometryAttribute::POSITION));
  for (draco::AttributeValueIndex avi(0); avi < pos_att->size(); ++avi) {
    draco::Vector3f pos;
    pos_att->GetValue(avi, &pos[0]);
    pos = pos + draco::Vector3f(-0.55f, 0.65f, 10.75f);
    pos_att->SetAttributeValue(avi, &pos[0]);
  }

  mesh->SetCompressionEnabled(true);

  // Set grid quantization for positions.
  draco::DracoCompressionOptions compression_options;
  // This should result in 16x16x16 quantization if the grid was perfectly
  // aligned but since it is not we should expect 17 or 18 values per component.
  compression_options.quantization_position.SetGrid(0.0625f);
  mesh->SetCompressionOptions(compression_options);

  draco::ExpertEncoder encoder(*mesh);
  draco::EncoderBuffer buffer;
  DRACO_ASSERT_OK(encoder.EncodeToBuffer(&buffer));

  // The grid options should be reflected in the |encoder|. Check that the
  // computed values are correct.
  const int pos_att_id =
      mesh->GetNamedAttributeId(draco::GeometryAttribute::POSITION);
  draco::Vector3f origin;
  encoder.options().GetAttributeVector(pos_att_id, "quantization_origin", 3,
                                       &origin[0]);
  // The origin is the first lower value on the quantization grid for each
  // component of the mesh.
  ASSERT_EQ(origin, draco::Vector3f(-0.5625f, 0.625f, 10.75f));

  // We need 5 quantization bits (for 17-18 values).
  ASSERT_EQ(
      encoder.options().GetAttributeInt(pos_att_id, "quantization_bits", -1),
      5);

  // The quantization range should be ((1 << quantization_bits) - 1) * spacing.
  ASSERT_NEAR(encoder.options().GetAttributeFloat(pos_att_id,
                                                  "quantization_range", 0.f),
              31.f * 0.0625f, 1e-6f);
}
#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
