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
#include "draco/io/ply_decoder.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {

class PlyDecoderTest : public ::testing::Test {
 protected:
  template <class Geometry>
  std::unique_ptr<Geometry> DecodePly(const std::string &file_name) const {
    const std::string path = GetTestFileFullPath(file_name);
    PlyDecoder decoder;
    std::unique_ptr<Geometry> geometry(new Geometry());
    Status status = decoder.DecodeFromFile(path, geometry.get());
    if (!status.ok()) {
      LOG(ERROR) << "Failed to decode " << file_name << ": " << status;
      return nullptr;
    }
    return geometry;
  }

  void test_decoding(const std::string &file_name, int num_faces,
                     uint32_t num_points, std::unique_ptr<Mesh> *out_mesh) {
    // Don't test mesh decoding when the input is point cloud.
    if (num_faces > 0) {
      std::unique_ptr<Mesh> mesh(DecodePly<Mesh>(file_name));
      ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;
      ASSERT_EQ(mesh->num_faces(), num_faces);
      if (out_mesh) {
        *out_mesh = std::move(mesh);
      }
    }

    const std::unique_ptr<PointCloud> pc(DecodePly<PointCloud>(file_name));
    ASSERT_NE(pc, nullptr) << "Failed to load test model " << file_name;
    ASSERT_EQ(pc->num_points(), num_points);
  }
  void test_decoding(const std::string &file_name) {
    const std::unique_ptr<Mesh> mesh(DecodePly<Mesh>(file_name));
    ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;
    ASSERT_GT(mesh->num_faces(), 0);

    const std::unique_ptr<PointCloud> pc(DecodePly<PointCloud>(file_name));
    ASSERT_NE(pc, nullptr) << "Failed to load test model " << file_name;
    ASSERT_GT(pc->num_points(), 0);
  }
};

TEST_F(PlyDecoderTest, TestPlyDecoding) {
  const std::string file_name = "test_pos_color.ply";
  test_decoding(file_name, 224, 114, nullptr);
}

TEST_F(PlyDecoderTest, TestPlyNormals) {
  const std::string file_name = "cube_att.ply";
  std::unique_ptr<Mesh> mesh;
  test_decoding(file_name, 12, 3 * 8, &mesh);
  ASSERT_NE(mesh, nullptr);
  const int att_id = mesh->GetNamedAttributeId(GeometryAttribute::NORMAL);
  ASSERT_GE(att_id, 0);
  const PointAttribute *const att = mesh->attribute(att_id);
  ASSERT_EQ(att->size(), 6);  // 6 unique normal values.
}

TEST_F(PlyDecoderTest, TestPlyDecodingAll) {
  // test if we can read all ply that are currently in test folder.
  test_decoding("bun_zipper.ply");
  // test_decoding("cube_att.ply"); // tested
  test_decoding("test_extra_whitespace.ply");
  test_decoding("test_more_datatypes.ply");
  test_decoding("test_pos_color_ascii.ply");
  test_decoding("int_point_cloud.ply", 0, 16, nullptr);
  // test_decoding("test_pos_color.ply"); // tested
  test_decoding("cube_quads.ply");
  test_decoding("Box.ply");
}

}  // namespace draco
