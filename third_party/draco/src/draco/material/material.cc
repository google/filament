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

namespace draco {

Material::Material() : Material(nullptr) {}

Material::Material(TextureLibrary *texture_library)
    : texture_library_(texture_library) {
  Clear();
}

void Material::Copy(const Material &src) {
  name_ = src.name_;
  color_factor_ = src.color_factor_;
  metallic_factor_ = src.metallic_factor_;
  roughness_factor_ = src.roughness_factor_;
  emissive_factor_ = src.emissive_factor_;
  transparency_mode_ = src.transparency_mode_;
  alpha_cutoff_ = src.alpha_cutoff_;
  double_sided_ = src.double_sided_;
  normal_texture_scale_ = src.normal_texture_scale_;

  // Copy properties of material extensions.
  unlit_ = src.unlit_;
  has_sheen_ = src.has_sheen_;
  sheen_color_factor_ = src.sheen_color_factor_;
  sheen_roughness_factor_ = src.sheen_roughness_factor_;
  has_transmission_ = src.has_transmission_;
  transmission_factor_ = src.transmission_factor_;
  has_clearcoat_ = src.has_clearcoat_;
  clearcoat_factor_ = src.clearcoat_factor_;
  clearcoat_roughness_factor_ = src.clearcoat_roughness_factor_;
  has_volume_ = src.has_volume_;
  thickness_factor_ = src.thickness_factor_;
  attenuation_distance_ = src.attenuation_distance_;
  attenuation_color_ = src.attenuation_color_;
  has_ior_ = src.has_ior_;
  ior_ = src.ior_;
  has_specular_ = src.has_specular_;
  specular_factor_ = src.specular_factor_;
  specular_color_factor_ = src.specular_color_factor_;

  // Copy texture maps.
  texture_map_type_to_index_map_ = src.texture_map_type_to_index_map_;
  texture_maps_.resize(src.texture_maps_.size());
  for (int i = 0; i < texture_maps_.size(); ++i) {
    texture_maps_[i] = std::unique_ptr<TextureMap>(new TextureMap());
    texture_maps_[i]->Copy(*src.texture_maps_[i]);
  }
}

void Material::Clear() {
  ClearTextureMaps();

  // Defaults correspond to the GLTF 2.0 spec.
  name_.clear();
  color_factor_ = Vector4f(1.f, 1.f, 1.f, 1.f);
  metallic_factor_ = 1.f;
  roughness_factor_ = 1.f;
  emissive_factor_ = Vector3f(0.f, 0.f, 0.f);
  transparency_mode_ = TRANSPARENCY_OPAQUE;
  alpha_cutoff_ = 0.5f;
  double_sided_ = false;
  normal_texture_scale_ = 1.0f;

  // Clear properties of material extensions to glTF 2.0 spec defaults.
  unlit_ = false;
  has_sheen_ = false;
  sheen_color_factor_ = Vector3f(0.f, 0.f, 0.f);
  sheen_roughness_factor_ = 0.f;
  has_transmission_ = false;
  transmission_factor_ = 0.f;
  has_clearcoat_ = false;
  clearcoat_factor_ = 0.f;
  clearcoat_roughness_factor_ = 0.f;
  has_volume_ = false;
  thickness_factor_ = 0.f;
  attenuation_distance_ = std::numeric_limits<float>::max();  // Infinity.
  attenuation_color_ = Vector3f(1.f, 1.f, 1.f);
  has_ior_ = false;
  ior_ = 1.5f;
  has_specular_ = false;
  specular_factor_ = 1.f;
  specular_color_factor_ = Vector3f(1.f, 1.f, 1.f);
}

void Material::ClearTextureMaps() {
  texture_maps_.clear();
  texture_map_type_to_index_map_.clear();
}

void Material::SetTextureMap(TextureMap &&texture_map) {
  std::unique_ptr<TextureMap> new_texture_map(new TextureMap);
  *new_texture_map = std::move(texture_map);
  SetTextureMap(std::move(new_texture_map));
}

void Material::SetTextureMap(std::unique_ptr<TextureMap> texture_map) {
  const TextureMap::Type type = texture_map->type();
  const auto it = texture_map_type_to_index_map_.find(type);
  // Only one texture of a given type is allowed to exist.
  if (it == texture_map_type_to_index_map_.end()) {
    texture_maps_.push_back(std::move(texture_map));
    texture_map_type_to_index_map_[type] = texture_maps_.size() - 1;
  } else {
    texture_maps_[it->second] = std::move(texture_map);
  }
}

void Material::SetTextureMap(std::unique_ptr<Texture> texture,
                             TextureMap::Type texture_map_type,
                             int tex_coord_index) {
  SetTextureMap(std::move(texture), texture_map_type,
                TextureMap::WrappingMode(TextureMap::CLAMP_TO_EDGE),
                tex_coord_index);
}

void Material::SetTextureMap(std::unique_ptr<Texture> texture,
                             TextureMap::Type texture_map_type,
                             TextureMap::WrappingMode wrapping_mode,
                             int tex_coord_index) {
  std::unique_ptr<TextureMap> texture_map(new TextureMap);
  texture_map->SetProperties(texture_map_type, wrapping_mode, tex_coord_index);

  if (texture_library_) {
    texture_map->SetTexture(texture.get());
    texture_library_->PushTexture(std::move(texture));
  } else {
    texture_map->SetTexture(std::move(texture));
  }
  SetTextureMap(std::move(texture_map));
}

Status Material::SetTextureMap(Texture *texture,
                               TextureMap::Type texture_map_type,
                               int tex_coord_index) {
  return SetTextureMap(texture, texture_map_type,
                       TextureMap::WrappingMode(TextureMap::CLAMP_TO_EDGE),
                       TextureMap::UNSPECIFIED, TextureMap::UNSPECIFIED,
                       tex_coord_index);
}

Status Material::SetTextureMap(Texture *texture,
                               TextureMap::Type texture_map_type,
                               TextureMap::WrappingMode wrapping_mode,
                               int tex_coord_index) {
  std::unique_ptr<TextureMap> texture_map(new TextureMap);
  return SetTextureMap(std::move(texture_map), texture, texture_map_type,
                       wrapping_mode, TextureMap::UNSPECIFIED,
                       TextureMap::UNSPECIFIED, tex_coord_index);
}

Status Material::SetTextureMap(Texture *texture,
                               TextureMap::Type texture_map_type,
                               TextureMap::WrappingMode wrapping_mode,
                               TextureMap::FilterType min_filter,
                               TextureMap::FilterType mag_filter,
                               int tex_coord_index) {
  std::unique_ptr<TextureMap> texture_map(new TextureMap);
  return SetTextureMap(std::move(texture_map), texture, texture_map_type,
                       wrapping_mode, min_filter, mag_filter, tex_coord_index);
}

Status Material::SetTextureMap(Texture *texture,
                               TextureMap::Type texture_map_type,
                               TextureMap::WrappingMode wrapping_mode,
                               TextureMap::FilterType min_filter,
                               TextureMap::FilterType mag_filter,
                               const TextureTransform &transform,
                               int tex_coord_index) {
  std::unique_ptr<TextureMap> texture_map(new TextureMap);
  texture_map->SetTransform(transform);
  return SetTextureMap(std::move(texture_map), texture, texture_map_type,
                       wrapping_mode, min_filter, mag_filter, tex_coord_index);
}

Status Material::SetTextureMap(std::unique_ptr<TextureMap> texture_map,
                               Texture *texture,
                               TextureMap::Type texture_map_type,
                               TextureMap::WrappingMode wrapping_mode,
                               TextureMap::FilterType min_filter,
                               TextureMap::FilterType mag_filter,
                               int tex_coord_index) {
  if (!IsTextureOwned(*texture)) {
    return Status(Status::DRACO_ERROR,
                  "Provided texture is not owned by the material.");
  }
  texture_map->SetProperties(texture_map_type, wrapping_mode, tex_coord_index,
                             min_filter, mag_filter);
  texture_map->SetTexture(texture);
  SetTextureMap(std::move(texture_map));
  return OkStatus();
}

bool Material::IsTextureOwned(const Texture &texture) {
  if (texture_library_) {
    // Ensure the texture is owned by the texture library.
    for (int ti = 0; ti < texture_library_->NumTextures(); ++ti) {
      if (texture_library_->GetTexture(ti) == &texture) {
        return true;
      }
    }
    return false;
  }
  // Else we need to check every texture map of this material.
  for (int ti = 0; ti < NumTextureMaps(); ++ti) {
    if (GetTextureMapByIndex(ti)->texture() == &texture) {
      return true;
    }
  }
  return false;
}

std::unique_ptr<TextureMap> Material::RemoveTextureMapByIndex(int index) {
  if (index < 0 || index >= texture_maps_.size()) {
    return nullptr;
  }
  std::unique_ptr<TextureMap> ret = std::move(texture_maps_[index]);
  texture_maps_.erase(texture_maps_.begin() + index);
  // A texture map was removed and we need to update
  // |texture_map_type_to_index_map_| to reflect the changes.
  for (int i = index; i < texture_maps_.size(); ++i) {
    texture_map_type_to_index_map_[texture_maps_[i]->type()] = i;
  }
  // Delete the removed texture map type.
  texture_map_type_to_index_map_.erase(
      texture_map_type_to_index_map_.find(ret->type()));
  return ret;
}

std::unique_ptr<TextureMap> Material::RemoveTextureMapByType(
    TextureMap::Type texture_type) {
  const auto it = texture_map_type_to_index_map_.find(texture_type);
  if (it == texture_map_type_to_index_map_.end()) {
    return nullptr;
  }
  return RemoveTextureMapByIndex(it->second);
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
