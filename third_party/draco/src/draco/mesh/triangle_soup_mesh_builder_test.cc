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
#include "draco/mesh/triangle_soup_mesh_builder.h"

#include <cstdint>
#include <memory>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/core/vector_d.h"

namespace draco {

class TriangleSoupMeshBuilderTest : public ::testing::Test {};

TEST_F(TriangleSoupMeshBuilderTest, CubeTest) {
  // This tests, verifies that the mesh builder constructs a valid cube out
  // of the provided triangle soup data.
  TriangleSoupMeshBuilder mb;
  mb.Start(12);
#ifdef DRACO_TRANSCODER_SUPPORTED
  mb.SetName("Cube");
#endif
  const int pos_att_id =
      mb.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  // clang-format off
  // Front face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(0),
                               Vector3f(0.f, 0.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(0.f, 1.f, 0.f).data());
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(1),
                               Vector3f(0.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data());

  // Back face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(2),
                               Vector3f(0.f, 1.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data());
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(3),
                               Vector3f(1.f, 1.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 1.f, 1.f).data());

  // Top face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(4),
                               Vector3f(0.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data(),
                               Vector3f(0.f, 1.f, 1.f).data());
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(5),
                               Vector3f(0.f, 1.f, 1.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 1.f, 1.f).data());

  // Bottom face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(6),
                               Vector3f(0.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(0.f, 0.f, 0.f).data());
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(7),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data());

  // Right face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(8),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data());
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(9),
                               Vector3f(1.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 1.f, 1.f).data());

  // Left face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(10),
                               Vector3f(0.f, 1.f, 0.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 0.f, 0.f).data());
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(11),
                               Vector3f(0.f, 1.f, 1.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 1.f, 0.f).data());
  // clang-format on
#ifdef DRACO_TRANSCODER_SUPPORTED
  mb.SetAttributeName(pos_att_id, "Bob");
#endif

  std::unique_ptr<Mesh> mesh = mb.Finalize();
  ASSERT_NE(mesh, nullptr) << "Failed to build the cube mesh.";
#ifdef DRACO_TRANSCODER_SUPPORTED
  EXPECT_EQ(mesh->GetName(), "Cube");
  EXPECT_EQ(mesh->attribute(pos_att_id)->name(), "Bob");
#endif
  EXPECT_EQ(mesh->num_points(), 8) << "Unexpected number of vertices.";
  EXPECT_EQ(mesh->num_faces(), 12) << "Unexpected number of faces.";
}

TEST_F(TriangleSoupMeshBuilderTest, TestPerFaceAttribs) {
  // This tests, verifies that the mesh builder constructs a valid cube with
  // per face Boolean attributes.
  TriangleSoupMeshBuilder mb;
  mb.Start(12);
  const int pos_att_id =
      mb.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  const int gen_att_id =
      mb.AddAttribute(GeometryAttribute::GENERIC, 1, DT_BOOL);
  uint8_t bool_true = 1;
  uint8_t bool_false = 0;
  // clang-format off
  // Front face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(0),
                               Vector3f(0.f, 0.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(0.f, 1.f, 0.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(0), &bool_false);

  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(1),
                               Vector3f(0.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(1), &bool_true);

  // Back face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(2),
                               Vector3f(0.f, 1.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(2), &bool_true);

  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(3),
                               Vector3f(1.f, 1.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 1.f, 1.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(3), &bool_true);

  // Top face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(4),
                               Vector3f(0.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data(),
                               Vector3f(0.f, 1.f, 1.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(4), &bool_false);

  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(5),
                               Vector3f(0.f, 1.f, 1.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 1.f, 1.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(5), &bool_false);

  // Bottom face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(6),
                               Vector3f(0.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(0.f, 0.f, 0.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(6), &bool_true);

  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(7),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(7), &bool_true);

  // Right face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(8),
                               Vector3f(1.f, 0.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 1.f, 0.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(8), &bool_false);

  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(9),
                               Vector3f(1.f, 1.f, 0.f).data(),
                               Vector3f(1.f, 0.f, 1.f).data(),
                               Vector3f(1.f, 1.f, 1.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(9), &bool_true);

  // Left face.
  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(10),
                               Vector3f(0.f, 1.f, 0.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 0.f, 0.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(10), &bool_true);

  mb.SetAttributeValuesForFace(pos_att_id, FaceIndex(11),
                               Vector3f(0.f, 1.f, 1.f).data(),
                               Vector3f(0.f, 0.f, 1.f).data(),
                               Vector3f(0.f, 1.f, 0.f).data());
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(11), &bool_false);
  // clang-format on

  std::unique_ptr<Mesh> mesh = mb.Finalize();
  ASSERT_NE(mesh, nullptr) << "Failed to build the cube mesh.";
#ifdef DRACO_TRANSCODER_SUPPORTED
  EXPECT_TRUE(mesh->GetName().empty());
#endif
  EXPECT_EQ(mesh->num_faces(), 12) << "Unexpected number of faces.";
  EXPECT_EQ(mesh->GetAttributeElementType(gen_att_id), MESH_FACE_ATTRIBUTE)
      << "Unexpected attribute element type.";
}

TEST_F(TriangleSoupMeshBuilderTest, PropagatesAttributeUniqueIds) {
  // This test verifies that TriangleSoupMeshBuilder correctly applies
  // unique IDs to attributes.
  TriangleSoupMeshBuilder mb;
  mb.Start(1);
  const int pos_att_id =
      mb.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  mb.SetAttributeValuesForFace(
      pos_att_id, FaceIndex(0), Vector3f(0.f, 0.f, 0.f).data(),
      Vector3f(1.f, 0.f, 0.f).data(), Vector3f(0.f, 1.f, 0.f).data());
  mb.SetAttributeUniqueId(pos_att_id, 1234);
  std::unique_ptr<Mesh> mesh = mb.Finalize();
  ASSERT_NE(mesh, nullptr);
  ASSERT_EQ(mesh->GetAttributeByUniqueId(1234), mesh->attribute(pos_att_id));
}

#ifdef DRACO_TRANSCODER_SUPPORTED
TEST_F(TriangleSoupMeshBuilderTest, NormalizedColor) {
  // This tests, verifies that the mesh builder constructs a valid model with
  // normalized integer colors using floating points as input.
  TriangleSoupMeshBuilder mb;
  mb.Start(2);
  const int pos_att_id =
      mb.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  const int color_att_id =
      mb.AddAttribute(GeometryAttribute::COLOR, 3, DT_UINT8, true);

  mb.SetAttributeValuesForFace(
      pos_att_id, FaceIndex(0), Vector3f(0.f, 0.f, 0.f).data(),
      Vector3f(1.f, 0.f, 0.f).data(), Vector3f(0.f, 1.f, 0.f).data());
  DRACO_ASSERT_OK(mb.ConvertAndSetAttributeValuesForFace(
      color_att_id, FaceIndex(0), 4, Vector4f(0.f, 0.f, 0.f, 1.f).data(),
      Vector4f(1.f, 1.f, 1.f, 1.f).data(),
      Vector4f(0.5f, 0.5f, 0.5f, 1.f).data()));
  mb.SetAttributeValuesForFace(
      pos_att_id, FaceIndex(1), Vector3f(0.f, 1.f, 0.f).data(),
      Vector3f(1.f, 0.f, 0.f).data(), Vector3f(1.f, 1.f, 0.f).data());

  DRACO_ASSERT_OK(mb.ConvertAndSetAttributeValuesForFace(
      color_att_id, FaceIndex(1), 4, Vector4f(0.5f, 0.5f, 0.5f, 1.f).data(),
      Vector4f(1.f, 1.f, 1.f, 1.f).data(),
      Vector4f(0.25f, 0.0f, 1.f, 1.f).data()));

  std::unique_ptr<Mesh> mesh = mb.Finalize();
  ASSERT_NE(mesh, nullptr) << "Failed to build the test mesh.";

  EXPECT_EQ(mesh->num_points(), 4) << "Unexpected number of vertices.";
  EXPECT_EQ(mesh->num_faces(), 2) << "Unexpected number of faces.";

  const auto *col_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::COLOR);
  ASSERT_NE(col_att, nullptr) << "Missing color attribute.";
  ASSERT_EQ(col_att->size(), 4);

  // All colors should be in range 0-255.
  uint8_t max_val = 0, min_val = 255;
  for (draco::AttributeValueIndex avi(0); avi < col_att->size(); ++avi) {
    VectorD<uint8_t, 3> cval;
    col_att->GetValue(avi, &cval);
    const uint8_t max = cval.MaxCoeff();
    const uint8_t min = cval.MinCoeff();
    if (max > max_val) {
      max_val = max;
    }
    if (min < min_val) {
      min_val = min;
    }
  }
  ASSERT_EQ(max_val, 255);
  ASSERT_EQ(min_val, 0);
}
#endif

}  // namespace draco
