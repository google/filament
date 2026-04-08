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
#include "draco/scene/scene.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/core/status.h"
#include "draco/mesh/mesh_are_equivalent.h"
#include "draco/scene/scene_are_equivalent.h"
#include "draco/scene/scene_indices.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

// Helper method for adding mesh group GPU instancing to the milk truck scene.
draco::Status AddGpuInstancingToMilkTruck(draco::Scene *scene) {
  // Create an instance and set its transformation TRS vectors.
  draco::InstanceArray::Instance instance_0;
  instance_0.trs.SetTranslation(Eigen::Vector3d(1.0, 2.0, 3.0));
  instance_0.trs.SetRotation(Eigen::Quaterniond(4.0, 5.0, 6.0, 7.0));
  instance_0.trs.SetScale(Eigen::Vector3d(8.0, 9.0, 10.0));

  // Create another instance.
  draco::InstanceArray::Instance instance_1;
  instance_1.trs.SetTranslation(Eigen::Vector3d(1.1, 2.1, 3.1));
  instance_1.trs.SetRotation(Eigen::Quaterniond(4.1, 5.1, 6.1, 7.1));
  instance_1.trs.SetScale(Eigen::Vector3d(8.1, 9.1, 10.1));

  // Add an empty GPU instancing object to the scene.
  const draco::InstanceArrayIndex index = scene->AddInstanceArray();
  draco::InstanceArray *gpu_instancing = scene->GetInstanceArray(index);

  // Add two instances to the GPU instancing object stored in the scene.
  DRACO_RETURN_IF_ERROR(gpu_instancing->AddInstance(instance_0));
  DRACO_RETURN_IF_ERROR(gpu_instancing->AddInstance(instance_1));

  // Assign the GPU instancing object to two mesh groups in two scene nodes.
  scene->GetNode(draco::SceneNodeIndex(2))->SetInstanceArrayIndex(index);
  scene->GetNode(draco::SceneNodeIndex(4))->SetInstanceArrayIndex(index);

  return draco::OkStatus();
}

TEST(SceneTest, TestCopy) {
  // Test copying of scene data.
  auto src_scene =
      draco::ReadSceneFromTestFile("CesiumMilkTruck/glTF/CesiumMilkTruck.gltf");
  ASSERT_NE(src_scene, nullptr);

  // Add GPU instancing to the scene for testing.
  DRACO_ASSERT_OK(AddGpuInstancingToMilkTruck(src_scene.get()));
  ASSERT_EQ(src_scene->NumInstanceArrays(), 1);
  ASSERT_EQ(src_scene->NumNodes(), 5);
  ASSERT_EQ(
      src_scene->GetNode(draco::SceneNodeIndex(0))->GetInstanceArrayIndex(),
      draco::kInvalidInstanceArrayIndex);
  ASSERT_EQ(
      src_scene->GetNode(draco::SceneNodeIndex(1))->GetInstanceArrayIndex(),
      draco::kInvalidInstanceArrayIndex);
  ASSERT_EQ(
      src_scene->GetNode(draco::SceneNodeIndex(2))->GetInstanceArrayIndex(),
      draco::InstanceArrayIndex(0));
  ASSERT_EQ(
      src_scene->GetNode(draco::SceneNodeIndex(3))->GetInstanceArrayIndex(),
      draco::kInvalidInstanceArrayIndex);
  ASSERT_EQ(
      src_scene->GetNode(draco::SceneNodeIndex(4))->GetInstanceArrayIndex(),
      draco::InstanceArrayIndex(0));

  // Make a copy of the scene.
  draco::Scene dst_scene;
  dst_scene.Copy(*src_scene);

  ASSERT_EQ(src_scene->NumMeshes(), dst_scene.NumMeshes());
  ASSERT_EQ(src_scene->NumMeshGroups(), dst_scene.NumMeshGroups());
  ASSERT_EQ(src_scene->NumNodes(), dst_scene.NumNodes());
  ASSERT_EQ(src_scene->NumAnimations(), dst_scene.NumAnimations());
  ASSERT_EQ(src_scene->NumSkins(), dst_scene.NumSkins());
  ASSERT_EQ(src_scene->NumLights(), dst_scene.NumLights());
  ASSERT_EQ(src_scene->NumInstanceArrays(), dst_scene.NumInstanceArrays());

  for (draco::MeshIndex i(0); i < src_scene->NumMeshes(); ++i) {
    draco::MeshAreEquivalent eq;
    ASSERT_TRUE(eq(src_scene->GetMesh(i), dst_scene.GetMesh(i)));
  }
  for (draco::MeshGroupIndex i(0); i < src_scene->NumMeshGroups(); ++i) {
    ASSERT_EQ(src_scene->GetMeshGroup(i)->NumMeshInstances(),
              dst_scene.GetMeshGroup(i)->NumMeshInstances());
    for (int j = 0; j < src_scene->GetMeshGroup(i)->NumMeshInstances(); ++j) {
      ASSERT_EQ(src_scene->GetMeshGroup(i)->GetMeshInstance(j).mesh_index,
                dst_scene.GetMeshGroup(i)->GetMeshInstance(j).mesh_index);
      ASSERT_EQ(src_scene->GetMeshGroup(i)->GetMeshInstance(j).material_index,
                dst_scene.GetMeshGroup(i)->GetMeshInstance(j).material_index);
      ASSERT_EQ(src_scene->GetMeshGroup(i)
                    ->GetMeshInstance(j)
                    .materials_variants_mappings.size(),
                dst_scene.GetMeshGroup(i)
                    ->GetMeshInstance(j)
                    .materials_variants_mappings.size());
    }
  }
  for (draco::SceneNodeIndex i(0); i < src_scene->NumNodes(); ++i) {
    ASSERT_EQ(src_scene->GetNode(i)->NumParents(),
              dst_scene.GetNode(i)->NumParents());
    for (int j = 0; j < src_scene->GetNode(i)->NumParents(); ++j) {
      ASSERT_EQ(src_scene->GetNode(i)->Parent(j),
                dst_scene.GetNode(i)->Parent(j));
    }
    ASSERT_EQ(src_scene->GetNode(i)->NumChildren(),
              dst_scene.GetNode(i)->NumChildren());
    for (int j = 0; j < src_scene->GetNode(i)->NumChildren(); ++j) {
      ASSERT_EQ(src_scene->GetNode(i)->Child(j),
                dst_scene.GetNode(i)->Child(j));
    }
    ASSERT_EQ(src_scene->GetNode(i)->GetMeshGroupIndex(),
              dst_scene.GetNode(i)->GetMeshGroupIndex());
    ASSERT_EQ(src_scene->GetNode(i)->GetSkinIndex(),
              dst_scene.GetNode(i)->GetSkinIndex());
    ASSERT_EQ(src_scene->GetNode(i)->GetLightIndex(),
              dst_scene.GetNode(i)->GetLightIndex());
    ASSERT_EQ(src_scene->GetNode(i)->GetInstanceArrayIndex(),
              dst_scene.GetNode(i)->GetInstanceArrayIndex());
  }
}

TEST(SceneTest, TestRemoveMesh) {
  // Test that a base mesh can be removed from scene.
  auto src_scene_ptr =
      draco::ReadSceneFromTestFile("CesiumMilkTruck/glTF/CesiumMilkTruck.gltf");
  ASSERT_NE(src_scene_ptr, nullptr);
  const draco::Scene &src_scene = *src_scene_ptr;

  // Copy scene.
  draco::Scene dst_scene;
  dst_scene.Copy(src_scene);
  ASSERT_EQ(dst_scene.NumMeshes(), 4);
  draco::MeshAreEquivalent eq;
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(0)),
                 src_scene.GetMesh(draco::MeshIndex(0))));
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(1)),
                 src_scene.GetMesh(draco::MeshIndex(1))));
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(2)),
                 src_scene.GetMesh(draco::MeshIndex(2))));
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(3)),
                 src_scene.GetMesh(draco::MeshIndex(3))));

  // Remove base mesh from scene.
  dst_scene.RemoveMesh(draco::MeshIndex(2));
  ASSERT_EQ(dst_scene.NumMeshes(), 3);
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(0)),
                 src_scene.GetMesh(draco::MeshIndex(0))));
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(1)),
                 src_scene.GetMesh(draco::MeshIndex(1))));
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(2)),
                 src_scene.GetMesh(draco::MeshIndex(3))));

  // Remove another base mesh from scene.
  dst_scene.RemoveMesh(draco::MeshIndex(1));
  ASSERT_EQ(dst_scene.NumMeshes(), 2);
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(0)),
                 src_scene.GetMesh(draco::MeshIndex(0))));
  ASSERT_TRUE(eq(dst_scene.GetMesh(draco::MeshIndex(1)),
                 src_scene.GetMesh(draco::MeshIndex(3))));
}

TEST(SceneTest, TestRemoveMeshGroup) {
  // Test that a mesh group can be removed from scene.
  auto src_scene_ptr =
      draco::ReadSceneFromTestFile("CesiumMilkTruck/glTF/CesiumMilkTruck.gltf");
  ASSERT_NE(src_scene_ptr, nullptr);
  const draco::Scene &src_scene = *src_scene_ptr;

  // Copy scene.
  draco::Scene dst_scene;
  dst_scene.Copy(src_scene);
  ASSERT_EQ(dst_scene.NumMeshGroups(), 2);
  ASSERT_EQ(dst_scene.NumNodes(), 5);
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(0))->GetMeshGroupIndex(),
            draco::MeshGroupIndex(0));
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(2))->GetMeshGroupIndex(),
            draco::MeshGroupIndex(1));
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(4))->GetMeshGroupIndex(),
            draco::MeshGroupIndex(1));

  // Remove mesh group from scene.
  dst_scene.RemoveMeshGroup(draco::MeshGroupIndex(0));
  ASSERT_EQ(dst_scene.NumMeshGroups(), 1);
  ASSERT_EQ(dst_scene.NumNodes(), 5);
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(0))->GetMeshGroupIndex(),
            draco::kInvalidMeshGroupIndex);
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(2))->GetMeshGroupIndex(),
            draco::MeshGroupIndex(0));
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(4))->GetMeshGroupIndex(),
            draco::MeshGroupIndex(0));

  // Remove another mesh group from scene.
  dst_scene.RemoveMeshGroup(draco::MeshGroupIndex(0));
  ASSERT_EQ(dst_scene.NumMeshGroups(), 0);
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(0))->GetMeshGroupIndex(),
            draco::kInvalidMeshGroupIndex);
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(2))->GetMeshGroupIndex(),
            draco::kInvalidMeshGroupIndex);
  ASSERT_EQ(dst_scene.GetNode(draco::SceneNodeIndex(4))->GetMeshGroupIndex(),
            draco::kInvalidMeshGroupIndex);
}

void CheckMeshMaterials(const draco::Scene &scene,
                        const std::vector<int> &expected_material_indices) {
  ASSERT_EQ(scene.NumMeshes(), expected_material_indices.size());
  std::vector<int> scene_material_indices;
  for (draco::MeshGroupIndex i(0); i < scene.NumMeshGroups(); i++) {
    const auto mg = scene.GetMeshGroup(i);
    for (int mi = 0; mi < mg->NumMeshInstances(); ++mi) {
      scene_material_indices.push_back(mg->GetMeshInstance(mi).material_index);
    }
  }
  ASSERT_EQ(scene_material_indices, expected_material_indices);
}

TEST(SceneTest, TestRemoveMaterial) {
  // Test that materials can be removed from a scene.
  auto src_scene_ptr =
      draco::ReadSceneFromTestFile("CesiumMilkTruck/glTF/CesiumMilkTruck.gltf");
  ASSERT_NE(src_scene_ptr, nullptr);
  const draco::Scene &src_scene = *src_scene_ptr;
  ASSERT_EQ(src_scene.GetMaterialLibrary().NumMaterials(), 4);
  CheckMeshMaterials(src_scene, {0, 1, 2, 3});

  // Copy scene.
  draco::Scene dst_scene;
  dst_scene.Copy(src_scene);

  // Check that referenced material cannot be removed from the scene.
  ASSERT_FALSE(dst_scene.RemoveMaterial(2).ok());

  // Copy scene again, since failed material removal corrupts the scene.
  dst_scene.Copy(src_scene);

  // Remove base mesh from scene. Material at index 2 becomes unreferenced.
  DRACO_ASSERT_OK(dst_scene.RemoveMesh(draco::MeshIndex(2)));
  ASSERT_EQ(dst_scene.GetMaterialLibrary().NumMaterials(), 4);
  CheckMeshMaterials(dst_scene, {0, 1, 3});

  // Check that unreferenced material can be removed from the scene.
  DRACO_ASSERT_OK(dst_scene.RemoveMaterial(2));
  ASSERT_EQ(dst_scene.GetMaterialLibrary().NumMaterials(), 3);
  CheckMeshMaterials(dst_scene, {0, 1, 2});

  // Check that material cannot be removed when material index is out of range.
  ASSERT_FALSE(dst_scene.RemoveMaterial(-1).ok());
  ASSERT_FALSE(dst_scene.RemoveMaterial(3).ok());
}

TEST(SceneTest, TestCopyWithStructuralMetadata) {
  // Tests copying of a scene with structural metadata.
  auto scene_ptr =
      draco::ReadSceneFromTestFile("CesiumMilkTruck/glTF/CesiumMilkTruck.gltf");
  ASSERT_NE(scene_ptr, nullptr);
  draco::Scene &scene = *scene_ptr;

  // Add structural metadata to the scene.
  draco::StructuralMetadataSchema schema;
  schema.json.SetString("Data");
  scene.GetStructuralMetadata().SetSchema(schema);

  // Copy the scene.
  draco::Scene copy;
  copy.Copy(scene);

  // Check that the structural metadata has been copied.
  ASSERT_EQ(copy.GetStructuralMetadata().GetSchema().json.GetString(), "Data");
}

TEST(SceneTest, TestCopyWithMetadata) {
  // Tests copying of a scene with general metadata.
  auto scene_ptr =
      draco::ReadSceneFromTestFile("CesiumMilkTruck/glTF/CesiumMilkTruck.gltf");
  ASSERT_NE(scene_ptr, nullptr);
  draco::Scene &scene = *scene_ptr;

  // Add metadata to the scene.
  scene.GetMetadata().AddEntryString("test_name", "test_value");
  scene.GetMetadata().AddEntryInt("test_int", 101);

  // Copy the scene.
  draco::Scene copy;
  copy.Copy(scene);

  // Check that the metadata has been copied.
  std::string string_val;
  int int_val;
  copy.GetMetadata().GetEntryString("test_name", &string_val);
  copy.GetMetadata().GetEntryInt("test_int", &int_val);
  ASSERT_EQ(string_val, "test_value");
  ASSERT_EQ(int_val, 101);
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
