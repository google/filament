// Copyright 2022 The Draco Authors.
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
#include "draco/mesh/mesh_features.h"

#include <memory>
#include <vector>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/texture/texture_map.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(MeshFeaturesTest, TestDefaults) {
  // Test construction of an empty feature ID set.
  draco::MeshFeatures mesh_features;
  ASSERT_TRUE(mesh_features.GetLabel().empty());
  ASSERT_EQ(mesh_features.GetFeatureCount(), 0);
  ASSERT_EQ(mesh_features.GetNullFeatureId(), -1);
  ASSERT_EQ(mesh_features.GetAttributeIndex(), -1);
  ASSERT_EQ(mesh_features.GetPropertyTableIndex(), -1);
  ASSERT_TRUE(mesh_features.GetTextureChannels().empty());
  ASSERT_EQ(mesh_features.GetTextureMap().texture(), nullptr);
  ASSERT_EQ(mesh_features.GetTextureMap().type(), draco::TextureMap::GENERIC);
}

TEST(MeshFeaturesTest, TestSettersAndGetters) {
  // Test setter and getter methods of the feature ID set.
  draco::MeshFeatures mesh_features;
  mesh_features.SetLabel("continent");
  mesh_features.SetFeatureCount(8);
  mesh_features.SetNullFeatureId(0);
  mesh_features.SetAttributeIndex(2);
  mesh_features.SetPropertyTableIndex(10);
  std::vector<int> channels = {2, 3};
  mesh_features.SetTextureChannels({2, 3});
  draco::TextureMap texture_map;
  texture_map.SetProperties(draco::TextureMap::GENERIC, 1);
  std::unique_ptr<draco::Texture> texture(new draco::Texture());
  texture_map.SetTexture(texture.get());
  mesh_features.SetTextureMap(texture_map);

  // Check that mesh feature set properties can be accessed via getters.
  ASSERT_EQ(mesh_features.GetLabel(), "continent");
  ASSERT_EQ(mesh_features.GetFeatureCount(), 8);
  ASSERT_EQ(mesh_features.GetNullFeatureId(), 0);
  ASSERT_EQ(mesh_features.GetAttributeIndex(), 2);
  ASSERT_EQ(mesh_features.GetPropertyTableIndex(), 10);
  ASSERT_EQ(mesh_features.GetTextureChannels(), channels);
  ASSERT_EQ(mesh_features.GetTextureMap().texture(), texture.get());
  ASSERT_EQ(mesh_features.GetTextureMap().type(), draco::TextureMap::GENERIC);
}

TEST(MeshFeaturesTest, TestCopy) {
  // Test that feature ID set can be copied.
  draco::MeshFeatures mesh_features;
  mesh_features.SetLabel("continent");
  mesh_features.SetFeatureCount(8);
  mesh_features.SetNullFeatureId(0);
  mesh_features.SetAttributeIndex(2);
  mesh_features.SetPropertyTableIndex(10);
  std::vector<int> channels = {2, 3};
  mesh_features.SetTextureChannels({2, 3});
  std::unique_ptr<draco::Texture> texture(new draco::Texture());
  mesh_features.SetTextureMap(texture.get(), 1);

  // Make a copy.
  draco::MeshFeatures copy;
  copy.Copy(mesh_features);

  // Check the copy.
  ASSERT_EQ(copy.GetLabel(), "continent");
  ASSERT_EQ(copy.GetFeatureCount(), 8);
  ASSERT_EQ(copy.GetNullFeatureId(), 0);
  ASSERT_EQ(copy.GetAttributeIndex(), 2);
  ASSERT_EQ(copy.GetPropertyTableIndex(), 10);
  ASSERT_EQ(copy.GetTextureChannels(), channels);
  ASSERT_EQ(copy.GetTextureMap().texture(), texture.get());
  ASSERT_EQ(copy.GetTextureMap().type(), draco::TextureMap::GENERIC);
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
