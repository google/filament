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
#include "draco/material/material.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/core/draco_test_utils.h"
#include "draco/io/texture_io.h"

namespace {

TEST(MaterialTest, TestMaterialAccess) {
  // Tests that we can set and get material properties.
  draco::Material material;

  material.SetName("Superalloy");
  ASSERT_EQ(material.GetName(), "Superalloy");
  material.SetColorFactor(draco::Vector4f(1.f, 0.2f, 0.1f, 0.9f));
  ASSERT_EQ(material.GetColorFactor(), draco::Vector4f(1.f, 0.2f, 0.1f, 0.9f));
  material.SetMetallicFactor(0.3f);
  ASSERT_EQ(material.GetMetallicFactor(), 0.3f);
  material.SetRoughnessFactor(0.2f);
  ASSERT_EQ(material.GetRoughnessFactor(), 0.2f);
  material.SetEmissiveFactor(draco::Vector3f(0.2f, 0.f, 0.1f));
  ASSERT_EQ(material.GetEmissiveFactor(), draco::Vector3f(0.2f, 0.f, 0.1f));

  // Set and check the properties of material extensions.
  material.SetUnlit(true);
  ASSERT_TRUE(material.GetUnlit());
  material.SetHasSheen(true);
  ASSERT_TRUE(material.HasSheen());
  material.SetSheenColorFactor(draco::Vector3f(0.4f, 0.2f, 0.8f));
  ASSERT_EQ(material.GetSheenColorFactor(), draco::Vector3f(0.4f, 0.2f, 0.8f));
  material.SetSheenRoughnessFactor(0.428f);
  ASSERT_EQ(material.GetSheenRoughnessFactor(), 0.428f);
  material.SetHasTransmission(true);
  ASSERT_TRUE(material.HasTransmission());
  material.SetTransmissionFactor(0.5f);
  ASSERT_EQ(material.GetTransmissionFactor(), 0.5f);
  material.SetHasClearcoat(true);
  ASSERT_TRUE(material.HasClearcoat());
  material.SetClearcoatFactor(0.6f);
  ASSERT_EQ(material.GetClearcoatFactor(), 0.6f);
  material.SetClearcoatRoughnessFactor(0.7f);
  ASSERT_EQ(material.GetClearcoatRoughnessFactor(), 0.7f);
  material.SetHasVolume(true);
  ASSERT_TRUE(material.HasVolume());
  material.SetThicknessFactor(0.8f);
  ASSERT_EQ(material.GetThicknessFactor(), 0.8f);
  material.SetAttenuationDistance(0.9f);
  ASSERT_EQ(material.GetAttenuationDistance(), 0.9f);
  material.SetAttenuationColor(draco::Vector3f(0.2f, 0.5f, 0.8f));
  ASSERT_EQ(material.GetAttenuationColor(), draco::Vector3f(0.2f, 0.5f, 0.8f));
  material.SetHasIor(true);
  ASSERT_TRUE(material.HasIor());
  material.SetIor(1.1f);
  ASSERT_EQ(material.GetIor(), 1.1f);
  material.SetHasSpecular(true);
  ASSERT_TRUE(material.HasSpecular());
  material.SetSpecularFactor(0.01f);
  ASSERT_EQ(material.GetSpecularFactor(), 0.01f);
  material.SetSpecularColorFactor(draco::Vector3f(0.4f, 1.f, 1.f));
  ASSERT_EQ(material.GetSpecularColorFactor(), draco::Vector3f(0.4f, 1.f, 1.f));

  ASSERT_EQ(material.GetTextureMapByType(draco::TextureMap::COLOR), nullptr);
  ASSERT_EQ(material.NumTextureMaps(), 0);

  std::unique_ptr<draco::Texture> texture =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();
  ASSERT_NE(texture, nullptr);

  material.SetTextureMap(std::move(texture), draco::TextureMap::COLOR, 0);

  ASSERT_NE(material.GetTextureMapByType(draco::TextureMap::COLOR), nullptr);
  ASSERT_EQ(material.NumTextureMaps(), 1);
  ASSERT_EQ(material.GetTextureMapByIndex(0),
            material.GetTextureMapByType(draco::TextureMap::COLOR));

  std::unique_ptr<draco::Texture> texture2 =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();
  ASSERT_NE(texture2, nullptr);
  material.SetTextureMap(std::move(texture2), draco::TextureMap::EMISSIVE, 1);

  ASSERT_NE(material.GetTextureMapByType(draco::TextureMap::EMISSIVE), nullptr);
  ASSERT_EQ(material.GetTextureMapByType(draco::TextureMap::EMISSIVE)
                ->tex_coord_index(),
            1);
  ASSERT_EQ(material.NumTextureMaps(), 2);

  // Try to add the emissive texture one more time. This should replace the
  // previous instance of the emissive texture on the material.
  std::unique_ptr<draco::Texture> texture3 =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();
  ASSERT_NE(texture3, nullptr);
  material.SetTextureMap(std::move(texture2), draco::TextureMap::EMISSIVE, 2);
  ASSERT_EQ(material.NumTextureMaps(), 2);
  ASSERT_EQ(material.GetTextureMapByType(draco::TextureMap::EMISSIVE)
                ->tex_coord_index(),
            2);

  std::unique_ptr<draco::TextureMap> texture_map4(new draco::TextureMap());
  std::unique_ptr<draco::Texture> texture4 =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();
  material.SetTextureMap(std::move(texture4), draco::TextureMap::ROUGHNESS, 0);
  ASSERT_EQ(material.NumTextureMaps(), 3);
  ASSERT_NE(material.GetTextureMapByType(draco::TextureMap::ROUGHNESS),
            nullptr);

  material.SetTransparencyMode(draco::Material::TRANSPARENCY_BLEND);
  ASSERT_EQ(material.GetTransparencyMode(),
            draco::Material::TRANSPARENCY_BLEND);
  material.SetAlphaCutoff(0.2f);
  ASSERT_EQ(material.GetAlphaCutoff(), 0.2f);
  material.SetNormalTextureScale(0.75f);
  ASSERT_EQ(material.GetNormalTextureScale(), 0.75f);

  material.ClearTextureMaps();
  ASSERT_EQ(material.NumTextureMaps(), 0);
  ASSERT_EQ(material.GetTextureMapByType(draco::TextureMap::COLOR), nullptr);

  // Metallic factor should be unchanged.
  ASSERT_EQ(material.GetMetallicFactor(), 0.3f);

  material.Clear();
  // Metallic factor should be reset to default.
  ASSERT_NE(material.GetMetallicFactor(), 0.3f);

  ASSERT_EQ(material.GetDoubleSided(), false);
  material.SetDoubleSided(true);
  ASSERT_EQ(material.GetDoubleSided(), true);
}

TEST(MaterialTest, TestMaterialCopy) {
  draco::Material material;
  material.SetName("Antimatter");
  material.SetColorFactor(draco::Vector4f(0.3f, 0.2f, 0.4f, 0.9f));
  material.SetMetallicFactor(0.2f);
  material.SetRoughnessFactor(0.4f);
  material.SetEmissiveFactor(draco::Vector3f(0.3f, 0.1f, 0.2f));
  material.SetTransparencyMode(draco::Material::TRANSPARENCY_MASK);
  material.SetAlphaCutoff(0.25f);
  material.SetDoubleSided(true);
  material.SetNormalTextureScale(0.75f);

  // Set the properties of material extensions.
  material.SetUnlit(true);
  material.SetHasSheen(true);
  material.SetSheenColorFactor(draco::Vector3f(0.4f, 0.2f, 0.8f));
  material.SetSheenRoughnessFactor(0.428f);
  material.SetHasTransmission(true);
  material.SetTransmissionFactor(0.5f);
  material.SetHasClearcoat(true);
  material.SetClearcoatFactor(0.6f);
  material.SetClearcoatRoughnessFactor(0.7f);
  material.SetHasVolume(true);
  material.SetThicknessFactor(0.8f);
  material.SetAttenuationDistance(0.9f);
  material.SetAttenuationColor(draco::Vector3f(0.2f, 0.5f, 0.8f));
  material.SetHasIor(true);
  material.SetIor(1.1f);
  material.SetHasSpecular(true);
  material.SetSpecularFactor(0.01f);
  material.SetSpecularColorFactor(draco::Vector3f(0.4f, 1.f, 1.f));

  std::unique_ptr<draco::Texture> texture =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();
  ASSERT_NE(texture, nullptr);
  material.SetTextureMap(std::move(texture), draco::TextureMap::EMISSIVE, 2);

  draco::Material new_material;
  new_material.Copy(material);

  ASSERT_EQ(material.GetName(), new_material.GetName());
  ASSERT_EQ(material.GetColorFactor(), new_material.GetColorFactor());
  ASSERT_EQ(material.GetMetallicFactor(), new_material.GetMetallicFactor());
  ASSERT_EQ(material.GetRoughnessFactor(), new_material.GetRoughnessFactor());
  ASSERT_EQ(material.GetEmissiveFactor(), new_material.GetEmissiveFactor());
  ASSERT_EQ(material.GetTransparencyMode(), new_material.GetTransparencyMode());
  ASSERT_EQ(material.GetAlphaCutoff(), new_material.GetAlphaCutoff());
  ASSERT_EQ(material.GetDoubleSided(), new_material.GetDoubleSided());
  ASSERT_EQ(material.GetNormalTextureScale(),
            new_material.GetNormalTextureScale());

  // Check that the properties of material extensions have been copied.
  ASSERT_EQ(material.GetUnlit(), new_material.GetUnlit());
  ASSERT_EQ(material.HasSheen(), new_material.HasSheen());
  ASSERT_EQ(material.GetSheenColorFactor(), new_material.GetSheenColorFactor());
  ASSERT_EQ(material.GetSheenRoughnessFactor(),
            new_material.GetSheenRoughnessFactor());
  ASSERT_TRUE(material.HasTransmission());
  ASSERT_EQ(material.GetTransmissionFactor(),
            new_material.GetTransmissionFactor());
  ASSERT_TRUE(material.HasClearcoat());
  ASSERT_EQ(material.GetClearcoatFactor(), new_material.GetClearcoatFactor());
  ASSERT_EQ(material.GetClearcoatRoughnessFactor(),
            new_material.GetClearcoatRoughnessFactor());
  ASSERT_TRUE(material.HasVolume());
  ASSERT_EQ(material.GetThicknessFactor(), new_material.GetThicknessFactor());
  ASSERT_EQ(material.GetAttenuationDistance(),
            new_material.GetAttenuationDistance());
  ASSERT_EQ(material.GetAttenuationColor(), new_material.GetAttenuationColor());
  ASSERT_TRUE(material.HasIor());
  ASSERT_EQ(material.GetIor(), new_material.GetIor());
  ASSERT_TRUE(material.HasSpecular());
  ASSERT_EQ(material.GetSpecularFactor(), new_material.GetSpecularFactor());
  ASSERT_EQ(material.GetSpecularColorFactor(),
            new_material.GetSpecularColorFactor());

  for (int i = 0; i < draco::TextureMap::TEXTURE_TYPES_COUNT; ++i) {
    const draco::TextureMap::Type texture_map_type =
        static_cast<draco::TextureMap::Type>(i);
    if (material.GetTextureMapByType(texture_map_type) == nullptr) {
      ASSERT_EQ(new_material.GetTextureMapByType(texture_map_type), nullptr);
      continue;
    }
    if (material.GetTextureMapByType(texture_map_type)->texture() == nullptr) {
      ASSERT_EQ(new_material.GetTextureMapByType(texture_map_type)->texture(),
                nullptr);
    } else {
      ASSERT_NE(new_material.GetTextureMapByType(texture_map_type)->texture(),
                nullptr);
      ASSERT_EQ(
          material.GetTextureMapByType(texture_map_type)->tex_coord_index(),
          new_material.GetTextureMapByType(texture_map_type)
              ->tex_coord_index());
    }
  }

  ASSERT_EQ(material.NumTextureMaps(), new_material.NumTextureMaps());
  for (int i = 0; i < material.NumTextureMaps(); ++i) {
    const draco::TextureMap *const tm0 = material.GetTextureMapByIndex(i);
    const draco::TextureMap *const tm1 = new_material.GetTextureMapByIndex(i);
    ASSERT_NE(tm0, nullptr);
    ASSERT_NE(tm1, nullptr);
    ASSERT_EQ(tm0->type(), tm1->type());
  }
}

TEST(MaterialTest, RemoveTextureMap) {
  // Tests that we can remove existing texture maps from a material.
  draco::Material material;

  // Add some dummy textures to the material.
  std::unique_ptr<draco::Texture> texture =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();
  ASSERT_NE(texture, nullptr);
  material.SetTextureMap(std::move(texture), draco::TextureMap::COLOR, 0);

  std::unique_ptr<draco::Texture> texture_2 =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();

  material.SetTextureMap(std::move(texture), draco::TextureMap::EMISSIVE, 0);

  ASSERT_EQ(material.NumTextureMaps(), 2);

  // Try to delete the color texture.
  std::unique_ptr<draco::TextureMap> removed_texture =
      material.RemoveTextureMapByType(draco::TextureMap::COLOR);
  ASSERT_NE(removed_texture, nullptr);
  ASSERT_EQ(removed_texture->type(), draco::TextureMap::COLOR);
  ASSERT_EQ(material.NumTextureMaps(), 1);
  ASSERT_NE(material.GetTextureMapByType(draco::TextureMap::EMISSIVE), nullptr);
  ASSERT_EQ(material.GetTextureMapByIndex(0)->type(),
            draco::TextureMap::EMISSIVE);
  ASSERT_EQ(material.GetTextureMapByType(draco::TextureMap::COLOR), nullptr);

  removed_texture = material.RemoveTextureMapByIndex(0);
  ASSERT_NE(removed_texture, nullptr);
  ASSERT_EQ(removed_texture->type(), draco::TextureMap::EMISSIVE);
  ASSERT_EQ(material.NumTextureMaps(), 0);
  ASSERT_EQ(material.GetTextureMapByType(draco::TextureMap::EMISSIVE), nullptr);
}

TEST(MaterialTest, SharedTexture) {
  // Tests adding shared textures.
  draco::Material material;

  // Add some dummy textures to the material.
  std::unique_ptr<draco::Texture> texture =
      draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
          .value();
  ASSERT_NE(texture, nullptr);
  draco::Texture *texture_raw = texture.get();
  material.SetTextureMap(std::move(texture), draco::TextureMap::COLOR, 0);

  DRACO_ASSERT_OK(
      material.SetTextureMap(texture_raw, draco::TextureMap::EMISSIVE, 0));

  ASSERT_EQ(material.NumTextureMaps(), 2);

  // Read a new texture.
  texture = draco::ReadTextureFromFile(draco::GetTestFileFullPath("test.png"))
                .value();
  // Texture is not owned by the material so we expect a failure.
  ASSERT_FALSE(
      material
          .SetTextureMap(texture.get(), draco::TextureMap::AMBIENT_OCCLUSION, 0)
          .ok());
}

}  // namespace
#endif  // DRACO_TRANSCODER_SUPPORTED
