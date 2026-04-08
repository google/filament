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
#ifndef DRACO_SCENE_MESH_GROUP_H_
#define DRACO_SCENE_MESH_GROUP_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <string>
#include <vector>

#include "draco/core/macros.h"
#include "draco/scene/scene_indices.h"

namespace draco {

// This class is used to hold ordered mesh instances that refer to one or more
// base meshes, materials, and materials variants mappings.
class MeshGroup {
 public:
  // Stores a mapping from material index to materials variant indices. Each
  // mesh instance may have multiple such mappings associated with it. See glTF
  // extension KHR_materials_variants for more details.
  struct MaterialsVariantsMapping {
    MaterialsVariantsMapping() = delete;
    MaterialsVariantsMapping(int material, const std::vector<int> &variants)
        : material(material), variants(variants) {}
    bool operator==(const MaterialsVariantsMapping &other) const {
      if (material != other.material) {
        return false;
      }
      if (variants != other.variants) {
        return false;
      }
      return true;
    }
    bool operator!=(const MaterialsVariantsMapping &other) const {
      return !(*this == other);
    }
    int material;
    std::vector<int> variants;
  };

  // Describes mesh instance stored in a mesh group, including base mesh index,
  // material index, and materials variants mappings.
  struct MeshInstance {
    MeshInstance() = delete;
    MeshInstance(MeshIndex mesh_index, int material_index)
        : MeshInstance(mesh_index, material_index, {}) {}
    MeshInstance(MeshIndex mesh_index, int material_index,
                 const std::vector<MaterialsVariantsMapping>
                     &materials_variants_mappings)
        : mesh_index(mesh_index),
          material_index(material_index),
          materials_variants_mappings(materials_variants_mappings) {}
    bool operator==(const MeshInstance &other) const {
      if (mesh_index != other.mesh_index) {
        return false;
      }
      if (material_index != other.material_index) {
        return false;
      }
      if (materials_variants_mappings.size() !=
          other.materials_variants_mappings.size()) {
        return false;
      }
      if (materials_variants_mappings != other.materials_variants_mappings) {
        return false;
      }
      return true;
    }
    bool operator!=(const MeshInstance &other) const {
      return !(*this == other);
    }
    MeshIndex mesh_index;
    int material_index;
    std::vector<MaterialsVariantsMapping> materials_variants_mappings;
  };

  MeshGroup() {}

  void Copy(const MeshGroup &mg) {
    name_ = mg.name_;
    mesh_instances_ = mg.mesh_instances_;
  }

  const std::string &GetName() const { return name_; }
  void SetName(const std::string &name) { name_ = name; }

  void AddMeshInstance(const MeshInstance &instance) {
    mesh_instances_.push_back(instance);
  }

  void SetMeshInstance(int index, const MeshInstance &instance) {
    mesh_instances_[index] = instance;
  }

  const MeshInstance &GetMeshInstance(int index) const {
    return mesh_instances_[index];
  }

  MeshInstance &GetMeshInstance(int index) { return mesh_instances_[index]; }

  int NumMeshInstances() const { return mesh_instances_.size(); }

  // Removes all mesh instances referring to base mesh at |mesh_index|.
  void RemoveMeshInstances(MeshIndex mesh_index) {
    int i = 0;
    while (i != mesh_instances_.size()) {
      if (mesh_instances_[i].mesh_index == mesh_index) {
        mesh_instances_.erase(mesh_instances_.begin() + i);
      } else {
        i++;
      }
    }
  }

 private:
  std::string name_;
  std::vector<MeshInstance> mesh_instances_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_SCENE_MESH_GROUP_H_
