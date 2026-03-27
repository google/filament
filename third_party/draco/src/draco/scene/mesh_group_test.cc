// Copyright 2020 The Draco Authors.
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
#include "draco/scene/mesh_group.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/scene/scene_indices.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

using draco::MeshGroup;
using draco::MeshIndex;

// Test helper method generates materials variants mappings based on a |seed|.
std::vector<MeshGroup::MaterialsVariantsMapping> MakeMappings(int seed) {
  MeshGroup::MaterialsVariantsMapping a(10 * seed + 0, {seed + 0, seed + 1});
  MeshGroup::MaterialsVariantsMapping b(10 * seed + 1, {seed + 2, seed + 3});
  return {a, b};
}

TEST(MeshGroupTest, TestMeshInstanceTwoArgumentConstructor) {
  MeshGroup::MeshInstance instance(MeshIndex(2), 3);
  ASSERT_EQ(instance.mesh_index, MeshIndex(2));
  ASSERT_EQ(instance.material_index, 3);
  ASSERT_EQ(instance.materials_variants_mappings.size(), 0);
}

TEST(MeshGroupTest, TestMeshInstanceThreeArgumentConstructor) {
  const auto mappings = MakeMappings(4);
  MeshGroup::MeshInstance instance(MeshIndex(2), 3, mappings);
  ASSERT_EQ(instance.mesh_index, MeshIndex(2));
  ASSERT_EQ(instance.material_index, 3);
  ASSERT_EQ(instance.materials_variants_mappings, mappings);
}

TEST(MeshGroupTest, TestMeshInstanceEqualsOperator) {
  MeshGroup::MeshInstance instance_a(MeshIndex(2), 3, MakeMappings(4));
  MeshGroup::MeshInstance instance_b(MeshIndex(2), 3, MakeMappings(4));
  ASSERT_TRUE(instance_a == instance_b);
  ASSERT_FALSE(instance_a != instance_b);

  MeshGroup::MeshInstance instance_c(MeshIndex(1), 3, MakeMappings(4));
  MeshGroup::MeshInstance instance_d(MeshIndex(2), 1, MakeMappings(4));
  MeshGroup::MeshInstance instance_e(MeshIndex(2), 3, MakeMappings(1));
  ASSERT_FALSE(instance_a == instance_c);
  ASSERT_FALSE(instance_a == instance_d);
  ASSERT_FALSE(instance_a == instance_e);
  ASSERT_TRUE(instance_a != instance_c);
  ASSERT_TRUE(instance_a != instance_d);
  ASSERT_TRUE(instance_a != instance_e);
}

TEST(MeshGroupTest, TestRemoveMeshInstanceWithNoOccurrences) {
  // Test that no mesh instances are removed from mesh group when removing the
  // instances by the base mesh index that is not in the mesh group.

  // Create test mesh group.
  MeshGroup mesh_group;
  mesh_group.AddMeshInstance({MeshIndex(1), 0, {}});
  mesh_group.AddMeshInstance({MeshIndex(3), 0, {}});

  // Try to remove mesh that is not in the mesh group.
  mesh_group.RemoveMeshInstances(MeshIndex(2));

  // Check result.
  ASSERT_EQ(mesh_group.NumMeshInstances(), 2);
  ASSERT_EQ(mesh_group.GetMeshInstance(0).mesh_index, MeshIndex(1));
  ASSERT_EQ(mesh_group.GetMeshInstance(1).mesh_index, MeshIndex(3));
}

TEST(MeshGroupTest, TestRemoveTheOnlyMeshInstance) {
  // Test that the only mesh instance can be removed from mesh group.

  // Create test mesh group.
  MeshGroup mesh_group;
  MeshGroup::MaterialsVariantsMapping mapping(70, {0, 1});
  mesh_group.AddMeshInstance({MeshIndex(7), 70, {mapping}});

  // Remove a mesh instance.
  mesh_group.RemoveMeshInstances(MeshIndex(7));

  // Check result.
  ASSERT_EQ(mesh_group.NumMeshInstances(), 0);
}

TEST(MeshGroupTest, TestRemoveOneMeshInstances) {
  // Test a mesh instance can be removed from mesh group.

  // Create test mesh group.
  MeshGroup mesh_group;
  mesh_group.AddMeshInstance({MeshIndex(1), 0, {}});
  mesh_group.AddMeshInstance({MeshIndex(3), 0, {}});
  mesh_group.AddMeshInstance({MeshIndex(5), 0, {}});
  mesh_group.AddMeshInstance({MeshIndex(7), 0, {}});

  // Remove a mesh.
  mesh_group.RemoveMeshInstances(MeshIndex(3));

  // Check result.
  ASSERT_EQ(mesh_group.NumMeshInstances(), 3);
  ASSERT_EQ(mesh_group.GetMeshInstance(0).mesh_index, MeshIndex(1));
  ASSERT_EQ(mesh_group.GetMeshInstance(1).mesh_index, MeshIndex(5));
  ASSERT_EQ(mesh_group.GetMeshInstance(2).mesh_index, MeshIndex(7));
}

TEST(MeshGroupTest, TestRemoveThreeMeshInstances) {
  // Test that multiple mesh instances can be removed from a mesh group.

  // Create test mesh group.
  MeshGroup mesh_group;
  mesh_group.AddMeshInstance({MeshIndex(1), 10, MakeMappings(1)});
  mesh_group.AddMeshInstance({MeshIndex(3), 30, MakeMappings(3)});
  mesh_group.AddMeshInstance({MeshIndex(5), 50, MakeMappings(5)});
  mesh_group.AddMeshInstance({MeshIndex(1), 10, MakeMappings(1)});
  mesh_group.AddMeshInstance({MeshIndex(7), 70, MakeMappings(7)});
  mesh_group.AddMeshInstance({MeshIndex(1), 10, MakeMappings(1)});

  // Remove mesh instances.
  mesh_group.RemoveMeshInstances(MeshIndex(1));

  // Check result.
  ASSERT_EQ(mesh_group.NumMeshInstances(), 3);
  const MeshGroup::MeshInstance mi0 = mesh_group.GetMeshInstance(0);
  const MeshGroup::MeshInstance mi1 = mesh_group.GetMeshInstance(1);
  const MeshGroup::MeshInstance mi2 = mesh_group.GetMeshInstance(2);
  ASSERT_EQ(mi0.mesh_index, MeshIndex(3));
  ASSERT_EQ(mi1.mesh_index, MeshIndex(5));
  ASSERT_EQ(mi2.mesh_index, MeshIndex(7));
  ASSERT_EQ(mi0.material_index, 30);
  ASSERT_EQ(mi1.material_index, 50);
  ASSERT_EQ(mi2.material_index, 70);
  ASSERT_EQ(mi0.materials_variants_mappings, MakeMappings(3));
  ASSERT_EQ(mi1.materials_variants_mappings, MakeMappings(5));
  ASSERT_EQ(mi2.materials_variants_mappings, MakeMappings(7));
}

TEST(MeshGroupTest, TestCopy) {
  // Tests that a mesh group can be copied.

  // Create test mesh group.
  MeshGroup mesh_group;
  mesh_group.SetName("Mesh-1-3-5-7");
  mesh_group.AddMeshInstance({MeshIndex(1), 10, MakeMappings(1)});
  mesh_group.AddMeshInstance({MeshIndex(3), 30, MakeMappings(3)});
  mesh_group.AddMeshInstance({MeshIndex(5), 50, MakeMappings(5)});
  mesh_group.AddMeshInstance({MeshIndex(7), 70, MakeMappings(7)});

  // Verify source MeshGroup.
  ASSERT_EQ(mesh_group.GetName(), "Mesh-1-3-5-7");
  ASSERT_EQ(mesh_group.NumMeshInstances(), 4);
  const MeshGroup::MeshInstance mi0 = mesh_group.GetMeshInstance(0);
  const MeshGroup::MeshInstance mi1 = mesh_group.GetMeshInstance(1);
  const MeshGroup::MeshInstance mi2 = mesh_group.GetMeshInstance(2);
  const MeshGroup::MeshInstance mi3 = mesh_group.GetMeshInstance(3);
  ASSERT_EQ(mi0.mesh_index, MeshIndex(1));
  ASSERT_EQ(mi1.mesh_index, MeshIndex(3));
  ASSERT_EQ(mi2.mesh_index, MeshIndex(5));
  ASSERT_EQ(mi3.mesh_index, MeshIndex(7));
  ASSERT_EQ(mi0.material_index, 10);
  ASSERT_EQ(mi1.material_index, 30);
  ASSERT_EQ(mi2.material_index, 50);
  ASSERT_EQ(mi3.material_index, 70);
  ASSERT_EQ(mi0.materials_variants_mappings, MakeMappings(1));
  ASSERT_EQ(mi1.materials_variants_mappings, MakeMappings(3));
  ASSERT_EQ(mi2.materials_variants_mappings, MakeMappings(5));
  ASSERT_EQ(mi3.materials_variants_mappings, MakeMappings(7));

  MeshGroup copy;
  copy.Copy(mesh_group);

  // Verify that Copy worked.
  ASSERT_EQ(mesh_group.GetName(), copy.GetName());
  ASSERT_EQ(mesh_group.NumMeshInstances(), copy.NumMeshInstances());
  ASSERT_EQ(mesh_group.GetMeshInstance(0), copy.GetMeshInstance(0));
  ASSERT_EQ(mesh_group.GetMeshInstance(1), copy.GetMeshInstance(1));
  ASSERT_EQ(mesh_group.GetMeshInstance(2), copy.GetMeshInstance(2));
  ASSERT_EQ(mesh_group.GetMeshInstance(3), copy.GetMeshInstance(3));
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
