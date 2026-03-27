// Copyright 2018 The Draco Authors.
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
#include "draco/mesh/mesh.h"

#include <memory>
#include <utility>
#include <vector>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/compression/draco_compression_options.h"
#include "draco/material/material_utils.h"
#include "draco/mesh/mesh_are_equivalent.h"
#include "draco/mesh/mesh_features.h"
#include "draco/mesh/mesh_utils.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"
#endif  // DRACO_TRANSCODER_SUPPORTED

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED
// Tests naming of a mesh.
TEST(MeshTest, MeshName) {
  draco::Mesh mesh;
  ASSERT_TRUE(mesh.GetName().empty());
  mesh.SetName("Bob");
  ASSERT_EQ(mesh.GetName(), "Bob");
}

// Tests copying of a mesh.
TEST(MeshTest, MeshCopy) {
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);
  draco::Mesh mesh_copy;
  mesh_copy.Copy(*mesh);
  draco::MeshAreEquivalent eq;
  ASSERT_TRUE(eq(*mesh, mesh_copy));
}

// Tests that we can copy a mesh to a different mesh that already contains some
// data.
TEST(MeshTest, MeshCopyToExistingMesh) {
  const std::unique_ptr<draco::Mesh> mesh_0 =
      draco::ReadMeshFromTestFile("cube_att.obj");
  const std::unique_ptr<draco::Mesh> mesh_1 =
      draco::ReadMeshFromTestFile("test_nm.obj");
  ASSERT_NE(mesh_0, nullptr);
  ASSERT_NE(mesh_1, nullptr);
  draco::MeshAreEquivalent eq;
  ASSERT_FALSE(eq(*mesh_0, *mesh_1));

  mesh_1->Copy(*mesh_0);
  ASSERT_TRUE(eq(*mesh_0, *mesh_1));
}

// Tests that we can remove unused materials from a mesh.
TEST(MeshTest, RemoveUnusedMaterials) {
  // Input mesh has 29 materials defined in the source file but only 7 are
  // actually used.
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("mat_test.obj");
  ASSERT_NE(mesh, nullptr);

  const draco::PointAttribute *const mat_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::MATERIAL);
  ASSERT_NE(mat_att, nullptr);
  ASSERT_EQ(mat_att->size(), 29);

  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), mat_att->size());

  // Get materials on all faces.
  std::vector<const draco::Material *> face_materials(mesh->num_faces(),
                                                      nullptr);
  for (draco::FaceIndex fi(0); fi < mesh->num_faces(); ++fi) {
    uint32_t mat_index = 0;
    mat_att->GetMappedValue(mesh->face(fi)[0], &mat_index);
    face_materials[fi.value()] =
        mesh->GetMaterialLibrary().GetMaterial(mat_index);
  }

  mesh->RemoveUnusedMaterials();

  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), 7);

  // Ensure the material attribute contains material indices in the valid range.
  for (draco::AttributeValueIndex avi(0); avi < mat_att->size(); ++avi) {
    uint32_t mat_index = 0;
    mat_att->GetValue(avi, &mat_index);
    ASSERT_LT(mat_index, mesh->GetMaterialLibrary().NumMaterials());
  }

  // Ensure all materials are still the same for all faces.
  for (draco::FaceIndex fi(0); fi < mesh->num_faces(); ++fi) {
    uint32_t mat_index = 0;
    mat_att->GetMappedValue(mesh->face(fi)[0], &mat_index);
    ASSERT_EQ(mesh->GetMaterialLibrary().GetMaterial(mat_index),
              face_materials[fi.value()]);
  }
}

TEST(MeshTest, RemoveUnusedMaterialsOnPointClud) {
  // Input mesh has 29 materials defined in the source file but only 7 are
  // actually used. Same as above test but we remove all faces and treat the
  // model as a point cloud.
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("mat_test.obj");
  ASSERT_NE(mesh, nullptr);

  // Make it a point cloud.
  mesh->SetNumFaces(0);

  const draco::PointAttribute *const mat_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::MATERIAL);
  ASSERT_NE(mat_att, nullptr);
  ASSERT_EQ(mat_att->size(), 29);

  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), mat_att->size());

  // Get materials on all points.
  std::vector<const draco::Material *> point_materials(mesh->num_points(),
                                                       nullptr);
  for (draco::PointIndex pi(0); pi < mesh->num_points(); ++pi) {
    uint32_t mat_index = 0;
    mat_att->GetMappedValue(pi, &mat_index);
    point_materials[pi.value()] =
        mesh->GetMaterialLibrary().GetMaterial(mat_index);
  }

  mesh->RemoveUnusedMaterials();

  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), 7);

  // Ensure the material attribute contains material indices in the valid range.
  for (draco::AttributeValueIndex avi(0); avi < mat_att->size(); ++avi) {
    uint32_t mat_index = 0;
    mat_att->GetValue(avi, &mat_index);
    ASSERT_LT(mat_index, mesh->GetMaterialLibrary().NumMaterials());
  }

  // Ensure all materials are still the same for all points.
  for (draco::PointIndex pi(0); pi < mesh->num_points(); ++pi) {
    uint32_t mat_index = 0;
    mat_att->GetMappedValue(pi, &mat_index);
    ASSERT_EQ(mesh->GetMaterialLibrary().GetMaterial(mat_index),
              point_materials[pi.value()]);
  }
}

TEST(MeshTest, RemoveUnusedMaterialsNoIndices) {
  // The same as above but we actually want to remove only materials and not
  // material indices. Therefore we should end up with the same number of
  // materials as source but all unused materials should be "default".
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("mat_test.obj");
  ASSERT_NE(mesh, nullptr);

  const draco::PointAttribute *const mat_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::MATERIAL);
  ASSERT_NE(mat_att, nullptr);
  ASSERT_EQ(mat_att->size(), 29);

  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), mat_att->size());

  // Do not remove unused material indices.
  mesh->RemoveUnusedMaterials(false);

  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), 29);

  // Gether which materials were actually used and check that all remaining
  // materials are "default".
  std::vector<bool> is_mat_used(mesh->GetMaterialLibrary().NumMaterials(),
                                false);
  for (draco::AttributeValueIndex avi(0); avi < mat_att->size(); ++avi) {
    uint32_t mat_index = 0;
    mat_att->GetValue(avi, &mat_index);
    is_mat_used[mat_index] = true;
  }

  for (int mi = 0; mi < mesh->GetMaterialLibrary().NumMaterials(); ++mi) {
    if (!is_mat_used[mi]) {
      ASSERT_TRUE(draco::MaterialUtils::AreMaterialsEquivalent(
          *mesh->GetMaterialLibrary().GetMaterial(mi), draco::Material()));
    }
  }
}

TEST(MeshTest, TestAddNewAttributeWithConnectivity) {
  // Tests that we can add new attributes with arbitrary connectivity to an
  // existing mesh.

  // Create a simple quad. See corner indices of the quad on the figure below:
  //
  //  *-------*
  //  |2\3   5|
  //  |  \    |
  //  |   \   |
  //  |    \  |
  //  |     \4|
  //  |0    1\|
  //  *-------*
  //
  draco::TriangleSoupMeshBuilder mb;
  mb.Start(2);
  mb.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
  mb.SetAttributeValuesForFace(
      0, draco::FaceIndex(0), draco::Vector3f(0, 0, 0).data(),
      draco::Vector3f(1, 0, 0).data(), draco::Vector3f(1, 1, 0).data());
  mb.SetAttributeValuesForFace(
      0, draco::FaceIndex(1), draco::Vector3f(1, 1, 0).data(),
      draco::Vector3f(1, 0, 0).data(), draco::Vector3f(1, 1, 1).data());
  std::unique_ptr<draco::Mesh> mesh = mb.Finalize();
  ASSERT_NE(mesh, nullptr);
  ASSERT_EQ(mesh->num_points(), 4);
  ASSERT_EQ(mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION)->size(),
            4);

  // Create a simple attribute that has a constant value on every corner.
  std::unique_ptr<draco::PointAttribute> pa(new draco::PointAttribute());
  pa->Init(draco::GeometryAttribute::GENERIC, 1 /*One components*/,
           draco::DT_UINT8, false, 1);
  uint8_t val = 10;
  pa->SetAttributeValue(draco::AttributeValueIndex(0), &val);

  // Map all corners to the same value.
  draco::IndexTypeVector<draco::CornerIndex, draco::AttributeValueIndex>
      corner_to_point(6, draco::AttributeValueIndex(0));

  // Adding this attribute to the mesh should not increase the number of points.
  const int new_att_id_0 =
      mesh->AddAttributeWithConnectivity(std::move(pa), corner_to_point);

  ASSERT_EQ(mesh->num_attributes(), 2);
  ASSERT_EQ(mesh->num_points(), 4);

  const draco::PointAttribute *const new_att_0 = mesh->attribute(new_att_id_0);
  ASSERT_NE(new_att_0, nullptr);

  // All points of the mesh should be mapped to the same attribute value.
  for (draco::PointIndex pi(0); pi < mesh->num_points(); ++pi) {
    uint8_t att_val = 0;
    new_att_0->GetMappedValue(pi, &att_val);
    ASSERT_EQ(att_val, 10);
  }

  // Add a new attribute with two values and different connectivity.
  pa = std::unique_ptr<draco::PointAttribute>(new draco::PointAttribute());
  pa->Init(draco::GeometryAttribute::GENERIC, 1 /*One components*/,
           draco::DT_UINT8, false, 2);
  val = 11;
  pa->SetAttributeValue(draco::AttributeValueIndex(0), &val);
  val = 12;
  pa->SetAttributeValue(draco::AttributeValueIndex(1), &val);

  // Map all corners to the value index 0 except for corner 1 that is mapped to
  // value index 1. This should result in a new point being created on either
  // corner 1 or corner 4 (see figure at the beginning of this test).
  corner_to_point.assign(6, draco::AttributeValueIndex(0));
  corner_to_point[draco::CornerIndex(1)] = draco::AttributeValueIndex(1);

  const int new_att_id_1 =
      mesh->AddAttributeWithConnectivity(std::move(pa), corner_to_point);

  ASSERT_EQ(mesh->num_attributes(), 3);

  // One new point should have been created by adding the new attribute.
  ASSERT_EQ(mesh->num_points(), 5);

  const draco::PointAttribute *const new_att_1 = mesh->attribute(new_att_id_1);
  ASSERT_NE(new_att_1, nullptr);
  ASSERT_TRUE(mesh->CornerToPointId(1) == draco::PointIndex(4) ||
              mesh->CornerToPointId(4) == draco::PointIndex(4));

  new_att_1->GetMappedValue(mesh->CornerToPointId(1), &val);
  ASSERT_EQ(val, 12);

  new_att_1->GetMappedValue(mesh->CornerToPointId(4), &val);
  ASSERT_EQ(val, 11);

  // Ensure the attribute values of the remaining attributes are well defined
  // on the new point.
  draco::Vector3f pos;
  mesh->attribute(0)->GetMappedValue(draco::PointIndex(4), &pos[0]);
  ASSERT_EQ(pos, draco::Vector3f(1, 0, 0));

  new_att_0->GetMappedValue(draco::PointIndex(4), &val);
  ASSERT_EQ(val, 10);

  new_att_0->GetMappedValue(mesh->CornerToPointId(1), &val);
  ASSERT_EQ(val, 10);
  new_att_0->GetMappedValue(mesh->CornerToPointId(4), &val);
  ASSERT_EQ(val, 10);
}

TEST(MeshTest, TestAddNewAttributeWithConnectivityWithIsolatedVertices) {
  // Tests that we can add a new attribute with connectivity to a mesh that
  // contains isolated vertices.
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("isolated_vertices.ply");
  ASSERT_NE(mesh, nullptr);
  const draco::PointAttribute *const pos_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
  ASSERT_NE(pos_att, nullptr);
  ASSERT_TRUE(pos_att->is_mapping_identity());
  ASSERT_EQ(pos_att->size(), 5);
  ASSERT_EQ(mesh->num_points(), 5);
  ASSERT_EQ(mesh->num_faces(), 2);

  // Add a new attribute with two values (one for each face).
  auto pa = std::unique_ptr<draco::PointAttribute>(new draco::PointAttribute());
  pa->Init(draco::GeometryAttribute::GENERIC, 1 /*One component*/,
           draco::DT_UINT8, false, 2);
  uint8_t val = 11;
  pa->SetAttributeValue(draco::AttributeValueIndex(0), &val);
  val = 12;
  pa->SetAttributeValue(draco::AttributeValueIndex(1), &val);

  draco::IndexTypeVector<draco::CornerIndex, draco::AttributeValueIndex>
      corner_to_point(6, draco::AttributeValueIndex(0));
  // All corners on the second face are mapped to the value 1.
  for (draco::CornerIndex ci(3); ci < 6; ++ci) {
    corner_to_point[ci] = draco::AttributeValueIndex(1);
  }

  const draco::PointAttribute *const pa_raw = pa.get();
  mesh->AddAttributeWithConnectivity(std::move(pa), corner_to_point);

  // Two new point should have been added.
  ASSERT_EQ(mesh->num_points(), 7);

  for (draco::PointIndex pi(0); pi < mesh->num_points(); ++pi) {
    ASSERT_NE(pa_raw->mapped_index(pi), draco::kInvalidAttributeValueIndex);
    ASSERT_NE(pos_att->mapped_index(pi), draco::kInvalidAttributeValueIndex);
  }
}

TEST(MeshTest, TestAddPerVertexAttribute) {
  std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");

  ASSERT_NE(mesh, nullptr);
  const draco::PointAttribute *const pos_att =
      mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
  ASSERT_NE(pos_att, nullptr);

  // The input mesh should have 8 spatial vertices.
  ASSERT_EQ(pos_att->size(), 8);

  // Add a new scalar attribute where each value corresponds to the position
  // value index (vertex).
  std::unique_ptr<draco::PointAttribute> pa(new draco::PointAttribute());
  pa->Init(draco::GeometryAttribute::GENERIC, /* scalar */ 1, draco::DT_FLOAT32,
           false, /* one value per position value */ 8);

  // Set the value for the new attribute.
  for (draco::AttributeValueIndex avi(0); avi < 8; ++avi) {
    const float att_value = avi.value();
    pa->SetAttributeValue(avi, &att_value);
  }

  // Add the attribute to the existing mesh.
  const int new_att_id = mesh->AddPerVertexAttribute(std::move(pa));
  ASSERT_NE(new_att_id, -1);

  // Make sure all the attribute values are set correctly for every point of the
  // mesh.
  for (draco::PointIndex pi(0); pi < mesh->num_points(); ++pi) {
    const draco::AttributeValueIndex pos_avi = pos_att->mapped_index(pi);
    const draco::AttributeValueIndex new_att_avi =
        mesh->attribute(new_att_id)->mapped_index(pi);
    ASSERT_EQ(pos_avi, new_att_avi);

    float new_att_value;
    mesh->attribute(new_att_id)->GetValue(new_att_avi, &new_att_value);
    ASSERT_EQ(new_att_value, new_att_avi.value());
  }
}

TEST(MeshTest, TestRemovalOfIsolatedPoints) {
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("isolated_vertices.ply");

  draco::Mesh mesh_copy;
  mesh_copy.Copy(*mesh);

  ASSERT_EQ(mesh_copy.num_points(), 5);
  mesh_copy.RemoveIsolatedPoints();
  ASSERT_EQ(mesh_copy.num_points(), 4);

  draco::MeshAreEquivalent eq;
  ASSERT_TRUE(eq(*mesh, mesh_copy));
}

TEST(MeshTest, TestCompressionSettings) {
  // Tests compression settings of a mesh.
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  // Check that compression is disabled and compression settings are default.
  ASSERT_FALSE(mesh->IsCompressionEnabled());
  const draco::DracoCompressionOptions default_compression_options;
  ASSERT_EQ(mesh->GetCompressionOptions(), default_compression_options);

  // Check that compression options can be set without enabling compression.
  draco::DracoCompressionOptions compression_options;
  compression_options.quantization_bits_normal = 12;
  mesh->SetCompressionOptions(compression_options);
  ASSERT_EQ(mesh->GetCompressionOptions(), compression_options);
  ASSERT_FALSE(mesh->IsCompressionEnabled());

  // Check that compression can be enabled.
  mesh->SetCompressionEnabled(true);
  ASSERT_TRUE(mesh->IsCompressionEnabled());

  // Check that individual compression options can be updated.
  mesh->GetCompressionOptions().compression_level++;
  mesh->GetCompressionOptions().compression_level--;

  // Check that compression settings can be copied.
  draco::Mesh mesh_copy;
  mesh_copy.Copy(*mesh);
  ASSERT_TRUE(mesh_copy.IsCompressionEnabled());
  ASSERT_EQ(mesh_copy.GetCompressionOptions(), compression_options);
}

// Tests adding and removing of mesh features to a mesh.
TEST(MeshTest, TestMeshFeatures) {
  // Create a mesh with two feature ID sets.
  draco::Mesh mesh;
  ASSERT_EQ(mesh.NumMeshFeatures(), 0);
  std::unique_ptr<draco::MeshFeatures> oceans(new draco::MeshFeatures());
  std::unique_ptr<draco::MeshFeatures> continents(new draco::MeshFeatures());
  oceans->SetLabel("oceans");
  continents->SetLabel("continents");
  const draco::MeshFeaturesIndex index_0 =
      mesh.AddMeshFeatures(std::move(oceans));
  const draco::MeshFeaturesIndex index_1 =
      mesh.AddMeshFeatures(std::move(continents));
  ASSERT_EQ(index_0, draco::MeshFeaturesIndex(0));
  ASSERT_EQ(index_1, draco::MeshFeaturesIndex(1));

  // Check that the mesh has two feature ID sets.
  ASSERT_EQ(mesh.NumMeshFeatures(), 2);
  ASSERT_EQ(mesh.GetMeshFeatures(index_0).GetLabel(), "oceans");
  ASSERT_EQ(mesh.GetMeshFeatures(index_1).GetLabel(), "continents");

  // Remove one feature ID set and check the remaining feature ID set.
  mesh.RemoveMeshFeatures(draco::MeshFeaturesIndex(1));
  ASSERT_EQ(mesh.NumMeshFeatures(), 1);
  ASSERT_EQ(mesh.GetMeshFeatures(draco::MeshFeaturesIndex(0)).GetLabel(),
            "oceans");

  // Remove the remaining feature ID set and check that no sets remain.
  mesh.RemoveMeshFeatures(draco::MeshFeaturesIndex(0));
  ASSERT_EQ(mesh.NumMeshFeatures(), 0);
}

// Tests copying of a mesh with feature ID sets.
TEST(MeshTest, MeshCopyWithMeshFeatures) {
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  // Add two textures to the non-material texture library of the mesh.
  std::unique_ptr<draco::Texture> texture0(new draco::Texture());
  std::unique_ptr<draco::Texture> texture1(new draco::Texture());
  texture0->Resize(128, 128);
  texture1->Resize(256, 256);
  texture0->FillImage(draco::RGBA(100, 0, 0, 0));
  texture1->FillImage(draco::RGBA(200, 0, 0, 0));
  draco::TextureLibrary &library = mesh->GetNonMaterialTextureLibrary();
  library.PushTexture(std::move(texture0));
  library.PushTexture(std::move(texture1));

  // Add feature ID set referring to an attribute.
  const draco::MeshFeaturesIndex index_0 = mesh->AddMeshFeatures(
      std::unique_ptr<draco::MeshFeatures>(new draco::MeshFeatures()));
  mesh->GetMeshFeatures(index_0).SetLabel("planet");
  mesh->GetMeshFeatures(index_0).SetFeatureCount(2);
  mesh->GetMeshFeatures(index_0).SetAttributeIndex(1);

  // Add feature ID set referring to texture at index 0.
  const draco::MeshFeaturesIndex index_1 = mesh->AddMeshFeatures(
      std::unique_ptr<draco::MeshFeatures>(new draco::MeshFeatures()));
  mesh->GetMeshFeatures(index_1).SetLabel("continents");
  mesh->GetMeshFeatures(index_1).SetFeatureCount(7);
  mesh->GetMeshFeatures(index_1).GetTextureMap().SetTexture(
      library.GetTexture(0));

  // Add feature ID set referring to a texture at index 1.
  const draco::MeshFeaturesIndex index_2 = mesh->AddMeshFeatures(
      std::unique_ptr<draco::MeshFeatures>(new draco::MeshFeatures()));
  mesh->GetMeshFeatures(index_2).SetLabel("oceans");
  mesh->GetMeshFeatures(index_2).SetFeatureCount(5);
  mesh->GetMeshFeatures(index_2).GetTextureMap().SetTexture(
      library.GetTexture(1));

  // Check mesh feature ID set texture pointers.
  ASSERT_EQ(library.NumTextures(), 2);
  ASSERT_EQ(mesh->NumMeshFeatures(), 3);
  ASSERT_EQ(mesh->GetMeshFeatures(index_0).GetTextureMap().texture(), nullptr);
  ASSERT_EQ(mesh->GetMeshFeatures(index_1).GetTextureMap().texture(),
            library.GetTexture(0));
  ASSERT_EQ(mesh->GetMeshFeatures(index_2).GetTextureMap().texture(),
            library.GetTexture(1));

  // Copy the mesh.
  draco::Mesh mesh_copy;
  mesh_copy.Copy(*mesh);

  // Check that the meshes are equivalent.
  draco::MeshAreEquivalent eq;
  ASSERT_TRUE(eq(*mesh, mesh_copy));

  // Also check that the texture pointers have been updated correctly.
  const draco::TextureLibrary &library_copy =
      mesh_copy.GetNonMaterialTextureLibrary();
  ASSERT_EQ(library_copy.NumTextures(), 2);
  ASSERT_EQ(mesh_copy.NumMeshFeatures(), 3);
  ASSERT_EQ(mesh_copy.GetMeshFeatures(index_0).GetTextureMap().texture(),
            nullptr);
  ASSERT_EQ(mesh_copy.GetMeshFeatures(index_1).GetTextureMap().texture(),
            library_copy.GetTexture(0));
  ASSERT_EQ(mesh_copy.GetMeshFeatures(index_2).GetTextureMap().texture(),
            library_copy.GetTexture(1));
}

// Tests that mesh features are updated properly after a mesh attribute is
// deleted.
TEST(MeshTest, TestMeshFeaturesAttributeDeletion) {
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  // Add feature ID set referring to an attribute.
  const draco::MeshFeaturesIndex index_0 = mesh->AddMeshFeatures(
      std::unique_ptr<draco::MeshFeatures>(new draco::MeshFeatures()));
  mesh->GetMeshFeatures(index_0).SetLabel("planet");
  mesh->GetMeshFeatures(index_0).SetFeatureCount(2);
  mesh->GetMeshFeatures(index_0).SetAttributeIndex(1);

  // Delete mesh attribute 0. This should update attribute index associated with
  // mesh features |index_0| by one (to 0).
  ASSERT_EQ(mesh->GetMeshFeatures(index_0).GetAttributeIndex(), 1);
  mesh->DeleteAttribute(0);
  ASSERT_EQ(mesh->GetMeshFeatures(index_0).GetAttributeIndex(), 0);

  // Delete the new mesh attribute 0 and the mesh features |index_0| should not
  // be associated with any attribute anymore.
  mesh->DeleteAttribute(0);
  ASSERT_EQ(mesh->GetMeshFeatures(index_0).GetAttributeIndex(), -1);
}

// Tests that we can identify which attributes are used by mesh features.
TEST(MeshTest, TestAttributeUsedByMeshFeatures) {
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  // Add feature ID set referring to an attribute.
  const draco::MeshFeaturesIndex index_0 = mesh->AddMeshFeatures(
      std::unique_ptr<draco::MeshFeatures>(new draco::MeshFeatures()));
  mesh->GetMeshFeatures(index_0).SetLabel("planet");
  mesh->GetMeshFeatures(index_0).SetFeatureCount(2);
  mesh->GetMeshFeatures(index_0).SetAttributeIndex(1);

  // Ensure we can tell that attribute 1 is used by mesh features.
  ASSERT_TRUE(mesh->IsAttributeUsedByMeshFeatures(1));

  // Attribute 0 should not be used by mesh features.
  ASSERT_FALSE(mesh->IsAttributeUsedByMeshFeatures(0));

  // If the mesh features is deleted, attribute 1 should not be used by mesh
  // features any more.
  mesh->DeleteAttribute(1);
  ASSERT_FALSE(mesh->IsAttributeUsedByMeshFeatures(1));
}

// Tests copying of a mesh with structural metadata.
TEST(MeshTest, TestCopyWithStructuralMetadata) {
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr);

  // Add structural metadata to the mesh.
  draco::StructuralMetadataSchema schema;
  schema.json.SetString("Data");
  mesh->GetStructuralMetadata().SetSchema(schema);
  mesh->AddPropertyAttributesIndex(0);
  mesh->AddPropertyAttributesIndex(1);

  // Copy the mesh.
  draco::Mesh copy;
  copy.Copy(*mesh);

  // Check that the structural metadata has been copied.
  ASSERT_EQ(copy.GetStructuralMetadata().GetSchema().json.GetString(), "Data");
  ASSERT_EQ(copy.NumPropertyAttributesIndices(), 2);
  ASSERT_EQ(copy.GetPropertyAttributesIndex(0), 0);
  ASSERT_EQ(copy.GetPropertyAttributesIndex(1), 1);

  // Check that property attributes index can be removed.
  copy.RemovePropertyAttributesIndex(0);
  ASSERT_EQ(copy.NumPropertyAttributesIndices(), 1);
  ASSERT_EQ(copy.GetPropertyAttributesIndex(0), 1);
}

// Tests removing of unused materials for a mesh with mesh features and property
// attributes indices.
TEST(MeshTest,
     RemoveUnusedMaterialsWithMeshFeaturesAndPropertyAttributesIndices) {
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("BoxesMeta/glTF/BoxesMeta.gltf");
  ASSERT_NE(mesh, nullptr);

  // Input has five mesh features, two associated with material 0 and three with
  // material 1.
  ASSERT_EQ(mesh->NumMeshFeatures(), 5);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(0), 0),
            0);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(1), 0),
            0);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(2), 0),
            1);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(3), 0),
            1);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(4), 0),
            1);

  // Input has two property attributes, one associated with each of the two
  // materials.
  ASSERT_EQ(mesh->NumPropertyAttributesIndices(), 2);
  ASSERT_EQ(mesh->GetPropertyAttributesIndexMaterialMask(0, 0), 0);
  ASSERT_EQ(mesh->GetPropertyAttributesIndexMaterialMask(1, 0), 1);

  // Remove material 0.
  draco::PointAttribute *mat_att = mesh->attribute(
      mesh->GetNamedAttributeId(draco::GeometryAttribute::MATERIAL));
  // Map mat value 0 to 1.
  uint32_t new_mat_index = 1;
  mat_att->SetAttributeValue(draco::AttributeValueIndex(0), &new_mat_index);

  // This should not do anything because we still have the material 0 referenced
  // by mesh features 0 and 1, as well as by property attributes at index 0.
  mesh->RemoveUnusedMaterials();

  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), 2);
  ASSERT_EQ(mesh->NumMeshFeatures(), 5);
  ASSERT_EQ(mesh->NumPropertyAttributesIndices(), 2);

  // Now remove unused mesh features (should be 0 and 1) and property attributes
  // indices (should be 0).
  DRACO_ASSERT_OK(draco::MeshUtils::RemoveUnusedMeshFeatures(mesh.get()));
  DRACO_ASSERT_OK(
      draco::MeshUtils::RemoveUnusedPropertyAttributesIndices(mesh.get()));

  // All remaining mesh features should be still mapped to material 1.
  ASSERT_EQ(mesh->NumMeshFeatures(), 3);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(0), 0),
            1);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(1), 0),
            1);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(2), 0),
            1);

  // Remaining property attributes index should be still mapped to material 1.
  ASSERT_EQ(mesh->NumPropertyAttributesIndices(), 1);
  ASSERT_EQ(mesh->GetPropertyAttributesIndexMaterialMask(0, 0), 1);

  // Now remove the unused materials (0).
  mesh->RemoveUnusedMaterials();

  // Only one material should be remaining.
  ASSERT_EQ(mesh->GetMaterialLibrary().NumMaterials(), 1);

  // All the mesh features should now be mapped to material 0.
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(0), 0),
            0);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(1), 0),
            0);
  ASSERT_EQ(mesh->GetMeshFeaturesMaterialMask(draco::MeshFeaturesIndex(2), 0),
            0);

  // Property attributes index should now be mapped to material 0.
  ASSERT_EQ(mesh->GetPropertyAttributesIndexMaterialMask(0, 0), 0);
}

// Tests that when we remove mesh features from a mesh, the associated vertex
// attributes and textures are not removed.
TEST(MeshTest, TestDeleteMeshFeatures) {
  // The loaded mesh has several vertex attributes and textures used by the
  // mesh features.
  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("BoxesMeta/glTF/BoxesMeta.gltf");
  ASSERT_NE(mesh, nullptr);
  ASSERT_GT(mesh->NumMeshFeatures(), 0);

  draco::Mesh mesh_copy;
  mesh_copy.Copy(*mesh);

  // Delete all mesh features from the copy and ensure all vertex attributes
  // and textures stay the same.
  while (mesh_copy.NumMeshFeatures() > 0) {
    mesh_copy.RemoveMeshFeatures(draco::MeshFeaturesIndex(0));
  }
  ASSERT_EQ(mesh_copy.NumMeshFeatures(), 0);
  ASSERT_EQ(mesh_copy.num_attributes(), mesh->num_attributes());
  ASSERT_EQ(mesh_copy.GetNonMaterialTextureLibrary().NumTextures(),
            mesh->GetNonMaterialTextureLibrary().NumTextures());
}
#endif  // DRACO_TRANSCODER_SUPPORTED

// Test bounding box.
TEST(MeshTest, TestMeshBoundingBox) {
  const draco::Vector3f max_pt(1, 1, 1);
  const draco::Vector3f min_pt(0, 0, 0);

  const std::unique_ptr<draco::Mesh> mesh =
      draco::ReadMeshFromTestFile("cube_att.obj");
  ASSERT_NE(mesh, nullptr) << "Failed in Loading: "
                           << "cube_att.obj";
  const draco::BoundingBox bounding_box = mesh->ComputeBoundingBox();

  EXPECT_EQ(max_pt[0], bounding_box.GetMaxPoint()[0]);
  EXPECT_EQ(max_pt[1], bounding_box.GetMaxPoint()[1]);
  EXPECT_EQ(max_pt[2], bounding_box.GetMaxPoint()[2]);

  EXPECT_EQ(min_pt[0], bounding_box.GetMinPoint()[0]);
  EXPECT_EQ(min_pt[1], bounding_box.GetMinPoint()[1]);
  EXPECT_EQ(min_pt[2], bounding_box.GetMinPoint()[2]);
}

}  // namespace
