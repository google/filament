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
#include "draco/mesh/mesh_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace {

// Compare normal vector rotated by |angle| around the x-axis.
void CompareRotatedNormals(const draco::Mesh &mesh_0, const draco::Mesh &mesh_1,
                           float angle) {
  const draco::PointAttribute *const norm_att_0 =
      mesh_0.GetNamedAttribute(draco::GeometryAttribute::NORMAL);
  const draco::PointAttribute *const norm_att_1 =
      mesh_1.GetNamedAttribute(draco::GeometryAttribute::NORMAL);
  ASSERT_EQ(norm_att_0->size(), norm_att_1->size());
  for (draco::AttributeValueIndex avi(0); avi < norm_att_0->size(); ++avi) {
    Eigen::Vector3f norm_0, norm_1;
    norm_att_0->GetValue(avi, norm_0.data());
    norm_att_1->GetValue(avi, norm_1.data());

    // Project the normals into yz plane
    norm_0[0] = 0.f;
    norm_1[0] = 0.f;

    if (norm_0.squaredNorm() < 1e-6f) {
      // Normal pointing towards X. Make sure the rotated normal is about the
      // same.
      ASSERT_NEAR(norm_1.squaredNorm(), 0.f, 1e-6f);
      continue;
    }

    // Ensure the angle between the normals is as expected.
    norm_0.normalize();
    norm_1.normalize();
    const float norm_angle =
        std::atan2(norm_0.cross(norm_1).norm(), norm_0.dot(norm_1));
    ASSERT_NEAR(std::abs(norm_angle), angle, 1e-6f);
  }
}

TEST(MeshUtilsTest, TestTransform) {
  auto mesh = draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  draco::Mesh transformed_mesh;
  transformed_mesh.Copy(*mesh);
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  draco::MeshUtils::TransformMesh(transform, &transformed_mesh);

  // Rotate the mesh by 45 deg around the x-axis.
  transform.block<3, 3>(0, 0) =
      Eigen::Quaterniond(
          Eigen::AngleAxisd(M_PI / 4.f, Eigen::Vector3d::UnitX()))
          .normalized()
          .toRotationMatrix();
  draco::MeshUtils::TransformMesh(transform, &transformed_mesh);
  CompareRotatedNormals(*mesh, transformed_mesh, M_PI / 4.f);

  // Now rotate the cube back.
  transform.block<3, 3>(0, 0) =
      Eigen::Quaterniond(
          Eigen::AngleAxisd(-M_PI / 4.f, Eigen::Vector3d::UnitX()))
          .normalized()
          .toRotationMatrix();

  draco::MeshUtils::TransformMesh(transform, &transformed_mesh);
  CompareRotatedNormals(*mesh, transformed_mesh, 0.f);
}

TEST(MeshUtilsTest, TestTextureUvFlips) {
  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  // Check that FlipTextureUvValues() only works on texture coordinates.
  draco::PointAttribute *att = mesh->attribute(0);
  ASSERT_EQ(att->attribute_type(), draco::GeometryAttribute::POSITION);
  ASSERT_FALSE(draco::MeshUtils::FlipTextureUvValues(false, true, att));

  att = mesh->attribute(1);
  ASSERT_EQ(att->attribute_type(), draco::GeometryAttribute::TEX_COORD);

  // Get the values and flip the V values.
  std::vector<std::array<float, 2>> check_uv_values;
  check_uv_values.resize(att->size());
  for (draco::AttributeValueIndex avi(0); avi < att->size(); ++avi) {
    att->GetValue<float, 2>(avi, &check_uv_values[avi.value()]);
    check_uv_values[avi.value()][1] = 1.0 - check_uv_values[avi.value()][1];
  }

  ASSERT_TRUE(draco::MeshUtils::FlipTextureUvValues(false, true, att));

  std::array<float, 2> value;
  for (draco::AttributeValueIndex avi(0); avi < att->size(); ++avi) {
    att->GetValue<float, 2>(avi, &value);
    ASSERT_EQ(value[0], check_uv_values[avi.value()][0]);
    ASSERT_EQ(value[1], check_uv_values[avi.value()][1]);
  }

  // Flip the U values.
  for (int i = 0; i < check_uv_values.size(); ++i) {
    check_uv_values[i][0] = 1.0 - check_uv_values[i][0];
  }

  ASSERT_TRUE(draco::MeshUtils::FlipTextureUvValues(true, false, att));

  for (draco::AttributeValueIndex avi(0); avi < att->size(); ++avi) {
    att->GetValue<float, 2>(avi, &value);
    ASSERT_EQ(value[0], check_uv_values[avi.value()][0]);
    ASSERT_EQ(value[1], check_uv_values[avi.value()][1]);
  }
}

// Tests counting degenerate values for positions and texture coordinates for
// both scene and mesh.
TEST(MeshUtilsTest, CountDegenerateValuesLantern) {
  int degenerate_positions_scene = 0;
  int degenerate_tex_coords_scene = 0;
  std::unique_ptr<draco::Scene> scene =
      draco::ReadSceneFromTestFile("Lantern/glTF/Lantern.gltf");
  ASSERT_NE(scene, nullptr);

  for (int mgi = 0; mgi < scene->NumMeshGroups(); ++mgi) {
    const draco::MeshGroup *const mesh_group =
        scene->GetMeshGroup(draco::MeshGroupIndex(mgi));
    ASSERT_NE(mesh_group, nullptr);

    for (int mi = 0; mi < mesh_group->NumMeshInstances(); ++mi) {
      const draco::MeshIndex mesh_index =
          mesh_group->GetMeshInstance(mi).mesh_index;
      const draco::Mesh &m = scene->GetMesh(mesh_index);

      for (int i = 0; i < m.num_attributes(); ++i) {
        const draco::PointAttribute *const att = m.attribute(i);
        ASSERT_NE(att, nullptr);

        if (att->attribute_type() == draco::GeometryAttribute::Type::POSITION) {
          degenerate_positions_scene +=
              draco::MeshUtils::CountDegenerateFaces(m, i);
        } else if (att->attribute_type() ==
                   draco::GeometryAttribute::Type::TEX_COORD) {
          degenerate_tex_coords_scene +=
              draco::MeshUtils::CountDegenerateFaces(m, i);
        }
      }
    }
  }
  EXPECT_EQ(degenerate_positions_scene, 0);
  EXPECT_EQ(degenerate_tex_coords_scene, 2);

  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("Lantern/glTF/Lantern.gltf");
  ASSERT_NE(mesh, nullptr);
  for (int i = 0; i < mesh->num_attributes(); ++i) {
    const draco::PointAttribute *const att = mesh->attribute(i);
    ASSERT_NE(att, nullptr);
    if (att->attribute_type() == draco::GeometryAttribute::Type::POSITION) {
      EXPECT_EQ(draco::MeshUtils::CountDegenerateFaces(*mesh, i),
                degenerate_positions_scene);
    } else if (att->attribute_type() ==
               draco::GeometryAttribute::Type::TEX_COORD) {
      EXPECT_EQ(draco::MeshUtils::CountDegenerateFaces(*mesh, i),
                degenerate_tex_coords_scene);
    }
  }
}

// Tests finding the lowest quantization bits for the texture coordinate in a
// mesh.
TEST(MeshUtilsTest, FindLowsetTextureQuantizationLanternMesh) {
  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("Lantern/glTF/Lantern.gltf");
  ASSERT_NE(mesh, nullptr);

  const int pos_quantization_bits = 11;
  const draco::PointAttribute *const pos_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::Type::POSITION, 0);
  ASSERT_NE(pos_att, nullptr);

  const draco::PointAttribute *const tex_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::Type::TEX_COORD, 0);
  ASSERT_NE(tex_att, nullptr);

  // Tests target no quantization returns no quantization.
  const int target_no_quantization_bits = 0;
  DRACO_ASSIGN_OR_ASSERT(const int no_quantization_bits,
                         draco::MeshUtils::FindLowestTextureQuantization(
                             *mesh, *pos_att, pos_quantization_bits, *tex_att,
                             target_no_quantization_bits));
  ASSERT_EQ(no_quantization_bits, 0);

  // Test failures.
  const int out_of_range_low = -1;
  const auto statusor_low = draco::MeshUtils::FindLowestTextureQuantization(
      *mesh, *pos_att, pos_quantization_bits, *tex_att, out_of_range_low);
  ASSERT_FALSE(statusor_low.ok());

  const int out_of_range_high = 30;
  const auto statusor_high = draco::MeshUtils::FindLowestTextureQuantization(
      *mesh, *pos_att, pos_quantization_bits, *tex_att, out_of_range_high);
  ASSERT_FALSE(statusor_high.ok());

  // Tests finding the lowest quantization bits for the texture coordinate.
  const int target_bits = 6;
  DRACO_ASSIGN_OR_ASSERT(
      const int lowest_bits,
      draco::MeshUtils::FindLowestTextureQuantization(
          *mesh, *pos_att, pos_quantization_bits, *tex_att, target_bits));
  ASSERT_EQ(lowest_bits, 14);
}

// Tests finding the lowest quantization bits for the texture coordinates for
// the three meshes in the scene.
TEST(MeshUtilsTest, FindLowsetTextureQuantizationLanternScene) {
  std::unique_ptr<draco::Scene> scene =
      draco::ReadSceneFromTestFile("Lantern/glTF/Lantern.gltf");
  ASSERT_NE(scene, nullptr);

  const std::vector<int> expected_mesh_quantization_bits{11, 8, 14};
  for (int mi = 0; mi < scene->NumMeshes(); ++mi) {
    const draco::Mesh &mesh = scene->GetMesh(draco::MeshIndex(mi));

    const int pos_quantization_bits = 11;
    const draco::PointAttribute *const pos_att =
        mesh.GetNamedAttribute(draco::GeometryAttribute::Type::POSITION, 0);
    ASSERT_NE(pos_att, nullptr);

    const draco::PointAttribute *const tex_att =
        mesh.GetNamedAttribute(draco::GeometryAttribute::Type::TEX_COORD, 0);
    ASSERT_NE(tex_att, nullptr);

    const int target_bits = 8;
    DRACO_ASSIGN_OR_ASSERT(
        const int lowest_bits,
        draco::MeshUtils::FindLowestTextureQuantization(
            mesh, *pos_att, pos_quantization_bits, *tex_att, target_bits));
    ASSERT_EQ(lowest_bits, expected_mesh_quantization_bits[mi]);
  }
}

TEST(MeshUtilsTest, CheckAutoGeneratedTangents) {
  // Test verifies that MeshUtils::HasAutoGeneratedTangents works as intended.
  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("sphere_no_tangents.gltf");
  ASSERT_NE(mesh, nullptr);

  ASSERT_TRUE(draco::MeshUtils::HasAutoGeneratedTangents(*mesh));
}

TEST(MeshUtilsTest, CheckMergeMetadata) {
  // Test verifies that we can merge metadata using MeshUtils::MergeMetadata().
  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("sphere_no_tangents.gltf");
  ASSERT_NE(mesh, nullptr);

  std::unique_ptr<draco::Mesh> other_mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");

  ASSERT_NE(mesh->GetMetadata(), nullptr);
  // One attribute metadata (for the tangent attribute) and no other entries.
  ASSERT_EQ(mesh->GetMetadata()->attribute_metadatas().size(), 1);
  ASSERT_EQ(mesh->GetMetadata()->num_entries(), 0);

  // No metadata at the other attribute.
  ASSERT_EQ(other_mesh->GetMetadata(), nullptr);

  // First try to merge |other_mesh| metadata to |mesh|. This shouldn't do
  // anything.
  draco::MeshUtils::MergeMetadata(*other_mesh, mesh.get());
  ASSERT_EQ(mesh->GetMetadata()->attribute_metadatas().size(), 1);
  ASSERT_EQ(mesh->GetMetadata()->num_entries(), 0);

  // Merge |mesh| metadata to |other_mesh|. This will create empty metadata but
  // not any attribute metadata because |other_mesh| doesn't have the tangent
  // attribute.
  draco::MeshUtils::MergeMetadata(*mesh, other_mesh.get());
  ASSERT_NE(other_mesh->GetMetadata(), nullptr);
  ASSERT_EQ(other_mesh->GetMetadata()->attribute_metadatas().size(), 0);
  ASSERT_EQ(other_mesh->GetMetadata()->num_entries(), 0);
  ASSERT_FALSE(draco::MeshUtils::HasAutoGeneratedTangents(*other_mesh));

  // Add dummy tangent attribute to the |other_mesh|.
  std::unique_ptr<draco::PointAttribute> tang_att(new draco::PointAttribute());
  draco::PointAttribute *const tang_att_ptr = tang_att.get();
  tang_att->set_attribute_type(draco::GeometryAttribute::TANGENT);
  other_mesh->AddAttribute(std::move(tang_att));

  // Merge |mesh| metadata to |other_mesh|. This time the tangent metadata
  // should be copied over.
  draco::MeshUtils::MergeMetadata(*mesh, other_mesh.get());
  ASSERT_NE(other_mesh->GetMetadata(), nullptr);
  ASSERT_EQ(other_mesh->GetMetadata()->attribute_metadatas().size(), 1);
  ASSERT_EQ(other_mesh->GetMetadata()->num_entries(), 0);
  ASSERT_NE(other_mesh->GetMetadata()->GetAttributeMetadataByUniqueId(
                tang_att_ptr->unique_id()),
            nullptr);
  ASSERT_TRUE(draco::MeshUtils::HasAutoGeneratedTangents(*other_mesh));

  // Now add some entries to the geometry metadata and merge again.
  mesh->metadata()->AddEntryInt("test_int_0", 0);
  mesh->metadata()->AddEntryInt("test_int_1", 1);
  mesh->metadata()->AddEntryInt("test_int_shared", 2);
  other_mesh->metadata()->AddEntryInt("test_int_shared", 3);

  // "test_int_0" and "test_int_1" should be copied over while
  // "test_entry_shared" should stay unchanged.
  draco::MeshUtils::MergeMetadata(*mesh, other_mesh.get());
  ASSERT_NE(other_mesh->GetMetadata(), nullptr);
  // Attribute metadata should stay unchanged.
  ASSERT_EQ(other_mesh->GetMetadata()->attribute_metadatas().size(), 1);
  ASSERT_NE(other_mesh->GetMetadata()->GetAttributeMetadataByUniqueId(
                tang_att_ptr->unique_id()),
            nullptr);
  ASSERT_EQ(other_mesh->GetMetadata()
                ->GetAttributeMetadataByUniqueId(tang_att_ptr->unique_id())
                ->num_entries(),
            1);

  // Check the geometry metadata entries.
  ASSERT_EQ(other_mesh->GetMetadata()->num_entries(), 3);
  int metadata_value;
  ASSERT_TRUE(
      other_mesh->GetMetadata()->GetEntryInt("test_int_0", &metadata_value));
  ASSERT_EQ(metadata_value, 0);
  ASSERT_TRUE(
      other_mesh->GetMetadata()->GetEntryInt("test_int_1", &metadata_value));
  ASSERT_EQ(metadata_value, 1);

  // The shared entry should have an unchanged value.
  ASSERT_TRUE(other_mesh->GetMetadata()->GetEntryInt("test_int_shared",
                                                     &metadata_value));
  ASSERT_EQ(metadata_value, 3);
}

TEST(MeshUtilsTest, RemoveUnusedMeshFeatures) {
  // Test verifies that MeshUtils::RemoveUnusedMeshFeatures works as intended.
  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("BoxesMeta/glTF/BoxesMeta.gltf");
  ASSERT_NE(mesh, nullptr);

  // The input mesh should have five mesh features and two features textures.
  ASSERT_EQ(mesh->NumMeshFeatures(), 5);
  ASSERT_EQ(mesh->GetNonMaterialTextureLibrary().NumTextures(), 2);

  // All of those features and textures should be used so calling the method
  // below shouldn't do anything.
  draco::MeshUtils::RemoveUnusedMeshFeatures(mesh.get());
  ASSERT_EQ(mesh->NumMeshFeatures(), 5);
  ASSERT_EQ(mesh->GetNonMaterialTextureLibrary().NumTextures(), 2);

  // Now remove material 1 that is mapped to first two mesh features.
  draco::PointAttribute *mat_att = mesh->attribute(
      mesh->GetNamedAttributeId(draco::GeometryAttribute::MATERIAL));

  // This basically remaps all faces from material 1 to material 0.
  uint32_t mat_index = 0;
  mat_att->SetAttributeValue(draco::AttributeValueIndex(1), &mat_index);

  // Try to remove the mesh features again.
  draco::MeshUtils::RemoveUnusedMeshFeatures(mesh.get());

  // Three of the mesh features should have been removed as well as one mesh
  // features texture.
  ASSERT_EQ(mesh->NumMeshFeatures(), 2);
  ASSERT_EQ(mesh->GetNonMaterialTextureLibrary().NumTextures(), 1);

  // Ensure the remaining mesh features are mapped to the correct material.
  for (draco::MeshFeaturesIndex mfi(0); mfi < mesh->NumMeshFeatures(); ++mfi) {
    ASSERT_EQ(mesh->NumMeshFeaturesMaterialMasks(mfi), 1);
    ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(mfi, 0), 0);
  }
}

TEST(MeshUtilsTest, RemoveUnusedPropertyAttributesIndices) {
  // Test verifies that MeshUtils::RemoveUnusedPropertyAttributesIndices works
  // as intended.
  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("BoxesMeta/glTF/BoxesMeta.gltf");
  ASSERT_NE(mesh, nullptr);

  // The input mesh should have two property attributes indices.
  ASSERT_EQ(mesh->NumPropertyAttributesIndices(), 2);
  ASSERT_EQ(mesh->GetPropertyAttributesIndex(0), 0);
  ASSERT_EQ(mesh->GetPropertyAttributesIndex(1), 1);
  ASSERT_EQ(mesh->NumPropertyAttributesIndexMaterialMasks(0), 1);
  ASSERT_EQ(mesh->NumPropertyAttributesIndexMaterialMasks(1), 1);
  ASSERT_EQ(mesh->GetPropertyAttributesIndexMaterialMask(0, 0), 0);
  ASSERT_EQ(mesh->GetPropertyAttributesIndexMaterialMask(1, 0), 1);

  // Both indices should be used so calling the method below shouldn't do
  // anything.
  draco::MeshUtils::RemoveUnusedPropertyAttributesIndices(mesh.get());
  ASSERT_EQ(mesh->NumPropertyAttributesIndices(), 2);

  // Now remove material 1 that is mapped to second property attributes index.
  draco::PointAttribute *mat_att = mesh->attribute(
      mesh->GetNamedAttributeId(draco::GeometryAttribute::MATERIAL));

  // This basically remaps all faces from material 1 to material 0.
  uint32_t mat_index = 0;
  mat_att->SetAttributeValue(draco::AttributeValueIndex(1), &mat_index);

  // Try to remove the property attributes indices again.
  draco::MeshUtils::RemoveUnusedPropertyAttributesIndices(mesh.get());

  // One of the property attributes indices should have been removed.
  ASSERT_EQ(mesh->NumPropertyAttributesIndices(), 1);

  // Ensure the remaining property attributes index is mapped to the correct
  // material.
  ASSERT_EQ(mesh->NumPropertyAttributesIndexMaterialMasks(0), 1);
  ASSERT_EQ(mesh->GetPropertyAttributesIndexMaterialMask(0, 0), 0);
}

}  // namespace

#endif  // DRACO_TRANSCODER_SUPPORTED
