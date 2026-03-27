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
#ifndef DRACO_MATERIAL_MATERIAL_LIBRARY_H_
#define DRACO_MATERIAL_MATERIAL_LIBRARY_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <map>
#include <memory>
#include <string>

#include "draco/material/material.h"
#include "draco/texture/texture_library.h"

namespace draco {

// MaterialLibrary holds an array of materials that are applied to a single
// model.
class MaterialLibrary {
 public:
  MaterialLibrary() = default;

  // Copies the |src| into this instance.
  void Copy(const MaterialLibrary &src);

  // Appends materials from the |src| library to this library. All materials
  // and textures are copied over.
  void Append(const MaterialLibrary &src);

  // Deletes all materials from the material library.
  void Clear();

  // The number of materials stored in the library. All materials are stored
  // with indices <0, num_materials() - 1>.
  size_t NumMaterials() const { return materials_.size(); }

  // Returns a material with a given index or nullptr if the index is not valid.
  const Material *GetMaterial(int index) const {
    if (index < 0 || index >= materials_.size()) {
      return nullptr;
    }
    return materials_[index].get();
  }

  // Returns a mutable pointer to a given material. If the material with the
  // specified |index| does not exist, it is automatically created.
  Material *MutableMaterial(int index);

  // Removes a material with a given index and returns it. Caller can ignore the
  // returned value, in which case the material will be automatically deleted.
  // Index of all subsequent materials will be decremented by one.
  std::unique_ptr<Material> RemoveMaterial(int index);

  const TextureLibrary &GetTextureLibrary() const { return texture_library_; }
  TextureLibrary &MutableTextureLibrary() { return texture_library_; }

  // Removes all textures that are not referenced by a TextureMap from the
  // texture library.
  void RemoveUnusedTextures();

  // Returns a map between each TextureMap object and associated texture index
  // in the texture |library|.
  std::map<TextureMap *, int> ComputeTextureMapToTextureIndexMapping(
      const TextureLibrary &library) const;

  // Creates a named materials variant and returns its index.
  int AddMaterialsVariant(const std::string &name) {
    materials_variants_names_.push_back(name);
    return materials_variants_names_.size() - 1;
  }

  // Returns the number of materials variants.
  int NumMaterialsVariants() const { return materials_variants_names_.size(); }

  // Returns the name of a materials variant.
  const std::string &GetMaterialsVariantName(int index) const {
    return materials_variants_names_[index];
  }

 private:
  std::vector<std::unique_ptr<Material>> materials_;
  std::vector<std::string> materials_variants_names_;

  // Container for storing all textures used by materials of this library.
  TextureLibrary texture_library_;
};

}  // namespace draco

#endif  // DRACO_MATERIAL_MATERIAL_LIBRARY_H_
#endif  // DRACO_TRANSCODER_SUPPORTED
