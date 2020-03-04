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

#include "draco/core/draco_test_base.h"
#include "draco/core/vector_d.h"

namespace draco {

class TriangleSoupMeshBuilderTest : public ::testing::Test {};

TEST_F(TriangleSoupMeshBuilderTest, CubeTest) {
  // This tests, verifies that the mesh builder constructs a valid cube out
  // of the provided triangle soup data.
  TriangleSoupMeshBuilder mb;
  mb.Start(12);
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

  std::unique_ptr<Mesh> mesh = mb.Finalize();
  ASSERT_NE(mesh, nullptr) << "Failed to build the cube mesh.";
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
  mb.SetPerFaceAttributeValueForFace(gen_att_id, FaceIndex(4), &bool_false);;

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
  EXPECT_EQ(mesh->num_faces(), 12) << "Unexpected number of faces.";
  EXPECT_EQ(mesh->GetAttributeElementType(gen_att_id), MESH_FACE_ATTRIBUTE)
      << "Unexpected attribute element type.";
}

}  // namespace draco
