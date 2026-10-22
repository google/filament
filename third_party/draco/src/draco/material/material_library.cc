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

#ifdef DRACO_TRANSCODER_SUPPORTED
namespace draco {

void MaterialLibrary::Copy(const MaterialLibrary &src) {
  Clear();
  Append(src);
}

void MaterialLibrary::Append(const MaterialLibrary &src) {
  const size_t old_num_materials = materials_.size();
  materials_.resize(old_num_materials + src.materials_.size());
  for (int i = 0; i < src.materials_.size(); ++i) {
    materials_[old_num_materials + i] =
        std::unique_ptr<Material>(new Material(&texture_library_));
    materials_[old_num_materials + i]->Copy(*src.materials_[i]);
  }

  const size_t old_num_textures = texture_library_.NumTextures();
  texture_library_.Append(src.texture_library_);
  for (int i = 0; i < src.materials_variants_names_.size(); i++) {
    materials_variants_names_.push_back(src.materials_variants_names_[i]);
  }

  // Remap all texture maps to the textures in the new texture library.

  // First gather mapping between texture maps and textures in the old material
  // library.
  const auto texture_map_to_index =
      ComputeTextureMapToTextureIndexMapping(src.texture_library_);

  // Remap all texture maps to textures stored in the new texture library.
  for (auto it = texture_map_to_index.begin(); it != texture_map_to_index.end();
       ++it) {
    TextureMap *const texture_map = it->first;
    const int texture_index = old_num_textures + it->second;
    texture_map->SetTexture(texture_library_.GetTexture(texture_index));
  }
}

std::unique_ptr<Material> MaterialLibrary::RemoveMaterial(int index) {
  std::unique_ptr<Material> ret = std::move(materials_[index]);
  materials_.erase(materials_.begin() + index);
  return ret;
}

void MaterialLibrary::RemoveUnusedTextures() {
  const auto texture_map_to_index =
      ComputeTextureMapToTextureIndexMapping(texture_library_);

  // Mark which textures are used.
  std::vector<bool> is_texture_used(texture_library_.NumTextures(), false);
  for (auto it = texture_map_to_index.begin(); it != texture_map_to_index.end();
       ++it) {
    is_texture_used[it->second] = true;
  }

  // Remove all textures that are not used (from backwards to avoid updating
  // entries in the |is_texture_used| vector).
  for (int i = texture_library_.NumTextures() - 1; i >= 0; --i) {
    if (!is_texture_used[i]) {
      texture_library_.RemoveTexture(i);
    }
  }
}

std::map<TextureMap *, int>
MaterialLibrary::ComputeTextureMapToTextureIndexMapping(
    const TextureLibrary &library) const {
  std::map<TextureMap *, int> map_to_index;
  for (int mi = 0; mi < materials_.size(); ++mi) {
    for (int ti = 0; ti < materials_[mi]->NumTextureMaps(); ++ti) {
      TextureMap *const texture_map = materials_[mi]->GetTextureMapByIndex(ti);
      for (int tli = 0; tli < library.NumTextures(); ++tli) {
        if (library.GetTexture(tli) != texture_map->texture()) {
          continue;
        }
        map_to_index[texture_map] = tli;
        break;
      }
    }
  }
  return map_to_index;
}

void MaterialLibrary::Clear() {
  materials_.clear();
  texture_library_.Clear();
  materials_variants_names_.clear();
}

Material *MaterialLibrary::MutableMaterial(int index) {
  if (index < 0) {
    return nullptr;
  }
  if (materials_.size() <= index) {
    const int old_size = materials_.size();
    materials_.resize(index + 1);
    // Ensure all newly created materials are valid.
    for (int i = old_size; i < index + 1; ++i) {
      materials_[i] =
          std::unique_ptr<Material>(new Material(&texture_library_));
    }
  }
  return materials_[index].get();
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
