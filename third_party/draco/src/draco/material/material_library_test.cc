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
#include "draco/material/material_library.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED
TEST(MaterialLibraryTest, TestMaterials) {
  // Test verifies that we can modify materials in a library.
  draco::MaterialLibrary library;
  ASSERT_EQ(library.NumMaterials(), 0);

  // Add a new material to the library.
  const draco::Material *const new_mat = library.MutableMaterial(0);
  ASSERT_NE(new_mat, nullptr);
  ASSERT_EQ(library.NumMaterials(), 1);

  const draco::Material *const new_mat2 = library.MutableMaterial(2);
  ASSERT_NE(new_mat2, nullptr);
  ASSERT_EQ(library.NumMaterials(), 3);
  ASSERT_EQ(library.GetMaterial(2), new_mat2);

  // Ensure that even though we call mutable_material multiple times, it does
  // not increase the number of materials associated to the library.
  for (int i = 0; i < library.NumMaterials(); ++i) {
    ASSERT_NE(library.MutableMaterial(i), nullptr);
  }
  ASSERT_EQ(library.NumMaterials(), 3);

  // Check that material variants can be added and cleared.
  library.AddMaterialsVariant("Milk Truck");
  library.AddMaterialsVariant("Ice Cream Truck");
  ASSERT_EQ(library.NumMaterialsVariants(), 2);
  ASSERT_EQ(library.GetMaterialsVariantName(0), "Milk Truck");
  ASSERT_EQ(library.GetMaterialsVariantName(1), "Ice Cream Truck");

  library.Clear();
  ASSERT_EQ(library.NumMaterials(), 0);
  ASSERT_EQ(library.NumMaterialsVariants(), 0);
}

TEST(MaterialLibraryTest, TestMaterialsCopy) {
  // Test verifies that we can copy a material library.
  draco::MaterialLibrary library;
  library.MutableMaterial(0)->SetMetallicFactor(2.4f);
  library.MutableMaterial(3)->SetRoughnessFactor(1.2f);
  library.AddMaterialsVariant("Milk Truck");
  library.AddMaterialsVariant("Ice Cream Truck");

  draco::MaterialLibrary new_library;
  new_library.Copy(library);
  ASSERT_EQ(library.NumMaterials(), new_library.NumMaterials());
  ASSERT_EQ(library.GetMaterial(0)->GetMetallicFactor(),
            new_library.GetMaterial(0)->GetMetallicFactor());
  ASSERT_EQ(library.GetMaterial(3)->GetRoughnessFactor(),
            new_library.GetMaterial(3)->GetRoughnessFactor());
  ASSERT_EQ(new_library.NumMaterialsVariants(), 2);
  ASSERT_EQ(new_library.GetMaterialsVariantName(0), "Milk Truck");
  ASSERT_EQ(new_library.GetMaterialsVariantName(1), "Ice Cream Truck");
}

TEST(MaterialLibraryTest, TestTextureLibrary) {
  // Tests that texture library is properly updated when we add new textures
  // to a material belonging to the material library.
  std::unique_ptr<draco::Texture> texture_0(new draco::Texture());
  std::unique_ptr<draco::Texture> texture_1(new draco::Texture());

  draco::MaterialLibrary library;
  library.MutableMaterial(0)->SetTextureMap(std::move(texture_0),
                                            draco::TextureMap::COLOR, 0);
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 1);
  library.MutableMaterial(3)->SetTextureMap(std::move(texture_1),
                                            draco::TextureMap::COLOR, 0);
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 2);
}

TEST(MaterialLibraryTest, RemoveUnusedTextures) {
  // Test verifies that we can remove unusued textures from the material
  // library.
  draco::MaterialLibrary library;

  // Create dummy textures.
  std::unique_ptr<draco::Texture> texture_0(new draco::Texture());
  std::unique_ptr<draco::Texture> texture_1(new draco::Texture());
  std::unique_ptr<draco::Texture> texture_2(new draco::Texture());

  // Add them to the materials of the library.
  library.MutableMaterial(0)->SetTextureMap(std::move(texture_0),
                                            draco::TextureMap::COLOR, 0);
  library.MutableMaterial(0)->SetTextureMap(
      std::move(texture_1), draco::TextureMap::METALLIC_ROUGHNESS, 0);
  library.MutableMaterial(1)->SetTextureMap(std::move(texture_2),
                                            draco::TextureMap::COLOR, 0);

  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 3);

  library.RemoveUnusedTextures();
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 3);

  // Remove texture map from a material.
  library.MutableMaterial(0)->RemoveTextureMapByType(
      draco::TextureMap::METALLIC_ROUGHNESS);
  library.RemoveUnusedTextures();
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 2);

  library.MutableMaterial(1)->RemoveTextureMapByType(draco::TextureMap::COLOR);
  library.RemoveUnusedTextures();
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 1);

  library.MutableMaterial(0)->RemoveTextureMapByType(draco::TextureMap::COLOR);
  library.RemoveUnusedTextures();
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 0);
}

TEST(MaterialLibraryTest, RemoveMaterial) {
  // Tests that we can safely remove materials from the material library.
  draco::MaterialLibrary library;
  library.MutableMaterial(0)->SetMetallicFactor(0.f);
  library.MutableMaterial(1)->SetMetallicFactor(1.f);
  library.MutableMaterial(2)->SetMetallicFactor(2.f);
  library.MutableMaterial(3)->SetMetallicFactor(3.f);

  ASSERT_EQ(library.NumMaterials(), 4);

  ASSERT_EQ(library.RemoveMaterial(0)->GetMetallicFactor(), 0.f);
  ASSERT_EQ(library.NumMaterials(), 3);

  ASSERT_EQ(library.RemoveMaterial(1)->GetMetallicFactor(), 2.f);
  ASSERT_EQ(library.NumMaterials(), 2);

  ASSERT_EQ(library.RemoveMaterial(1)->GetMetallicFactor(), 3.f);
  ASSERT_EQ(library.NumMaterials(), 1);

  ASSERT_EQ(library.RemoveMaterial(0)->GetMetallicFactor(), 1.f);
  ASSERT_EQ(library.NumMaterials(), 0);
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
