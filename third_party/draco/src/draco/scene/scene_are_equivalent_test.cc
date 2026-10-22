// Copyright 2019 The Draco Authors.
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
#include "draco/scene/scene_are_equivalent.h"

#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/scene_io.h"
#include "draco/scene/scene.h"

namespace draco {

#ifdef DRACO_TRANSCODER_SUPPORTED
class SceneAreEquivalentTest : public ::testing::Test {};

TEST_F(SceneAreEquivalentTest, TestOnIndenticalScenes) {
  const std::string file_name = "CesiumMilkTruck/glTF/CesiumMilkTruck.gltf";
  const std::unique_ptr<Scene> scene(ReadSceneFromTestFile(file_name));
  ASSERT_NE(scene, nullptr) << "Failed to load test scene: " << file_name;

  // Add mesh feature ID set to a scene mesh.
  std::unique_ptr<MeshFeatures> mesh_features(new MeshFeatures());
  scene->GetMesh(MeshIndex(2)).AddMeshFeatures(std::move(mesh_features));

  SceneAreEquivalent equiv;
  ASSERT_TRUE(equiv(*scene, *scene));
}

TEST_F(SceneAreEquivalentTest, TestOnDifferentScenes) {
  const std::string file_name0 = "CesiumMilkTruck/glTF/CesiumMilkTruck.gltf";
  const std::string file_name1 = "Lantern/glTF/Lantern.gltf";
  const std::unique_ptr<Scene> scene0(ReadSceneFromTestFile(file_name0));
  const std::unique_ptr<Scene> scene1(ReadSceneFromTestFile(file_name1));
  ASSERT_NE(scene0, nullptr) << "Failed to load test scene: " << file_name0;
  ASSERT_NE(scene1, nullptr) << "Failed to load test scene: " << file_name1;
  SceneAreEquivalent equiv;
  ASSERT_FALSE(equiv(*scene0, *scene1));
}

TEST_F(SceneAreEquivalentTest, TestMeshFeatures) {
  const std::string file_name = "CesiumMilkTruck/glTF/CesiumMilkTruck.gltf";
  const std::unique_ptr<Scene> scene0(ReadSceneFromTestFile(file_name));
  const std::unique_ptr<Scene> scene1(ReadSceneFromTestFile(file_name));
  ASSERT_NE(scene0, nullptr);
  ASSERT_NE(scene1, nullptr);

  // Add identical mesh feature ID sets to mesh at index 0.
  Mesh &mesh0 = scene0->GetMesh(MeshIndex(0));
  Mesh &mesh1 = scene1->GetMesh(MeshIndex(0));
  mesh0.AddMeshFeatures(std::unique_ptr<MeshFeatures>(new MeshFeatures()));
  mesh1.AddMeshFeatures(std::unique_ptr<MeshFeatures>(new MeshFeatures()));

  // Empty feature sets should match.
  SceneAreEquivalent equiv;
  ASSERT_TRUE(equiv(*scene0, *scene1));

  // Make mesh features different and check that the meshes are not equivalent.
  mesh0.GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(5);
  mesh1.GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(6);
  ASSERT_FALSE(equiv(*scene0, *scene1));

  // Make mesh features identical and check that the meshes are equivalent.
  mesh0.GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(1);
  mesh1.GetMeshFeatures(MeshFeaturesIndex(0)).SetFeatureCount(1);
  ASSERT_TRUE(equiv(*scene0, *scene1));
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace draco
