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
#include "draco/mesh/mesh_are_equivalent.h"

#include <sstream>
#include <utility>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/mesh_io.h"
#include "draco/io/obj_decoder.h"
#include "draco/mesh/mesh.h"

namespace draco {

class MeshAreEquivalentTest : public ::testing::Test {};

TEST_F(MeshAreEquivalentTest, TestOnIndenticalMesh) {
  const std::string file_name = "test_nm.obj";
  const std::unique_ptr<Mesh> mesh(ReadMeshFromTestFile(file_name));
  ASSERT_NE(mesh, nullptr) << "Failed to load test model." << file_name;

#ifdef DRACO_TRANSCODER_SUPPORTED
  // Add mesh feature ID set to the mesh.
  std::unique_ptr<MeshFeatures> mesh_features(new MeshFeatures());
  mesh->AddMeshFeatures(std::move(mesh_features));
#endif

  // Check that mesh is equivalent to itself.
  MeshAreEquivalent equiv;
  ASSERT_TRUE(equiv(*mesh, *mesh));
}

TEST_F(MeshAreEquivalentTest, TestPermutedOneFace) {
  const std::string file_name_0 = "one_face_123.obj";
  const std::string file_name_1 = "one_face_312.obj";
  const std::string file_name_2 = "one_face_321.obj";
  const std::unique_ptr<Mesh> mesh_0(ReadMeshFromTestFile(file_name_0));
  const std::unique_ptr<Mesh> mesh_1(ReadMeshFromTestFile(file_name_1));
  const std::unique_ptr<Mesh> mesh_2(ReadMeshFromTestFile(file_name_2));
  ASSERT_NE(mesh_0, nullptr) << "Failed to load test model." << file_name_0;
  ASSERT_NE(mesh_1, nullptr) << "Failed to load test model." << file_name_1;
  ASSERT_NE(mesh_2, nullptr) << "Failed to load test model." << file_name_2;
  MeshAreEquivalent equiv;
  ASSERT_TRUE(equiv(*mesh_0, *mesh_0));
  ASSERT_TRUE(equiv(*mesh_0, *mesh_1));   // Face rotated.
  ASSERT_FALSE(equiv(*mesh_0, *mesh_2));  // Face inverted.
}

TEST_F(MeshAreEquivalentTest, TestPermutedTwoFaces) {
  const std::string file_name_0 = "two_faces_123.obj";
  const std::string file_name_1 = "two_faces_312.obj";
  const std::unique_ptr<Mesh> mesh_0(ReadMeshFromTestFile(file_name_0));
  const std::unique_ptr<Mesh> mesh_1(ReadMeshFromTestFile(file_name_1));
  ASSERT_NE(mesh_0, nullptr) << "Failed to load test model." << file_name_0;
  ASSERT_NE(mesh_1, nullptr) << "Failed to load test model." << file_name_1;
  MeshAreEquivalent equiv;
  ASSERT_TRUE(equiv(*mesh_0, *mesh_0));
  ASSERT_TRUE(equiv(*mesh_1, *mesh_1));
  ASSERT_TRUE(equiv(*mesh_0, *mesh_1));
}

TEST_F(MeshAreEquivalentTest, TestPermutedThreeFaces) {
  const std::string file_name_0 = "three_faces_123.obj";
  const std::string file_name_1 = "three_faces_312.obj";
  const std::unique_ptr<Mesh> mesh_0(ReadMeshFromTestFile(file_name_0));
  const std::unique_ptr<Mesh> mesh_1(ReadMeshFromTestFile(file_name_1));
  ASSERT_NE(mesh_0, nullptr) << "Failed to load test model." << file_name_0;
  ASSERT_NE(mesh_1, nullptr) << "Failed to load test model." << file_name_1;
  MeshAreEquivalent equiv;
  ASSERT_TRUE(equiv(*mesh_0, *mesh_0));
  ASSERT_TRUE(equiv(*mesh_1, *mesh_1));
  ASSERT_TRUE(equiv(*mesh_0, *mesh_1));
}

// This test checks that the edgebreaker algorithm does not change the mesh up
// to the order of faces and vertices.
TEST_F(MeshAreEquivalentTest, TestOnBigMesh) {
  const std::string file_name = "test_nm.obj";
  const std::unique_ptr<Mesh> mesh0(ReadMeshFromTestFile(file_name));
  ASSERT_NE(mesh0, nullptr) << "Failed to load test model." << file_name;

  std::unique_ptr<Mesh> mesh1;
  std::stringstream ss;
  WriteMeshIntoStream(mesh0.get(), ss, MESH_EDGEBREAKER_ENCODING);
  ReadMeshFromStream(&mesh1, ss);
  ASSERT_TRUE(ss.good()) << "Mesh IO failed.";

  MeshAreEquivalent equiv;
  ASSERT_TRUE(equiv(*mesh0, *mesh0));
  ASSERT_TRUE(equiv(*mesh1, *mesh1));
  ASSERT_TRUE(equiv(*mesh0, *mesh1));
}

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST_F(MeshAreEquivalentTest, TestMeshFeatures) {
  const std::string file_name = "test_nm.obj";
  const std::unique_ptr<Mesh> mesh0(ReadMeshFromTestFile(file_name));
  const std::unique_ptr<Mesh> mesh1(ReadMeshFromTestFile(file_name));
  ASSERT_NE(mesh0, nullptr);
  ASSERT_NE(mesh1, nullptr);

  // Add identical mesh feature ID sets to meshes.
  mesh0->AddMeshFeatures(std::unique_ptr<MeshFeatures>(new MeshFeatures()));
  mesh1->AddMeshFeatures(std::unique_ptr<MeshFeatures>(new MeshFeatures()));

  // Empty feature sets should match.
  MeshAreEquivalent equiv;
  ASSERT_TRUE(equiv(*mesh0, *mesh1));

  // Make mesh features different and check that the meshes are not equivalent.
  mesh0->GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(5);
  mesh1->GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(6);
  ASSERT_FALSE(equiv(*mesh0, *mesh1));

  // Make mesh features identical and check that the meshes are equivalent.
  mesh0->GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(1);
  mesh1->GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(1);
  ASSERT_TRUE(equiv(*mesh0, *mesh1));
}
#endif  // DRACO_TRANSCODER_SUPPORTED
}  // namespace draco
