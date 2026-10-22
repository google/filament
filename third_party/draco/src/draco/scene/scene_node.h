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
#ifdef DRACO_TRANSCODER_SUPPORTED
#ifndef DRACO_SCENE_SCENE_NODE_H_
#define DRACO_SCENE_SCENE_NODE_H_

#include "draco/scene/scene_indices.h"
#include "draco/scene/trs_matrix.h"

namespace draco {

// This class is used to create a scene hierarchy from meshes in their local
// space transformed into scene space.
class SceneNode {
 public:
  SceneNode()
      : mesh_group_index_(-1),
        skin_index_(-1),
        light_index_(-1),
        instance_array_index_(-1) {}

  void Copy(const SceneNode &sn) {
    name_ = sn.name_;
    trs_matrix_.Copy(sn.trs_matrix_);
    mesh_group_index_ = sn.mesh_group_index_;
    skin_index_ = sn.skin_index_;
    parents_ = sn.parents_;
    children_ = sn.children_;
    light_index_ = sn.light_index_;
    instance_array_index_ = sn.instance_array_index_;
  }

  // Sets a name.
  void SetName(const std::string &name) { name_ = name; }

  // Returns the name.
  const std::string &GetName() const { return name_; }

  // Set transformation from mesh local space to scene space.
  void SetTrsMatrix(const TrsMatrix &trsm) { trs_matrix_.Copy(trsm); }
  const TrsMatrix &GetTrsMatrix() const { return trs_matrix_; }

  // Set the index to the mesh group in the scene.
  void SetMeshGroupIndex(MeshGroupIndex index) { mesh_group_index_ = index; }
  MeshGroupIndex GetMeshGroupIndex() const { return mesh_group_index_; }

  // Set the index to the skin in the scene.
  void SetSkinIndex(SkinIndex index) { skin_index_ = index; }
  SkinIndex GetSkinIndex() const { return skin_index_; }

  // Set the index to the light in the scene.
  void SetLightIndex(LightIndex index) { light_index_ = index; }
  LightIndex GetLightIndex() const { return light_index_; }

  // Set the index to the mesh group instance array in the scene. Note that
  // according to EXT_mesh_gpu_instancing glTF extension there is no defined
  // behavior for a node with instance array and without a mesh group.
  void SetInstanceArrayIndex(InstanceArrayIndex index) {
    instance_array_index_ = index;
  }
  InstanceArrayIndex GetInstanceArrayIndex() const {
    return instance_array_index_;
  }

  // Functions to set and get zero or more parent nodes of this node.
  SceneNodeIndex Parent(int index) const { return parents_[index]; }
  const std::vector<SceneNodeIndex> &Parents() const { return parents_; }
  void AddParentIndex(SceneNodeIndex index) { parents_.push_back(index); }
  int NumParents() const { return parents_.size(); }
  void RemoveAllParents() { parents_.clear(); }

  // Functions to set and get zero or more child nodes of this node.
  SceneNodeIndex Child(int index) const { return children_[index]; }
  const std::vector<SceneNodeIndex> &Children() const { return children_; }
  void AddChildIndex(SceneNodeIndex index) { children_.push_back(index); }
  int NumChildren() const { return children_.size(); }
  void RemoveAllChildren() { children_.clear(); }

 private:
  std::string name_;
  TrsMatrix trs_matrix_;
  draco::MeshGroupIndex mesh_group_index_;
  draco::SkinIndex skin_index_;
  std::vector<SceneNodeIndex> parents_;
  std::vector<SceneNodeIndex> children_;
  LightIndex light_index_;
  InstanceArrayIndex instance_array_index_;
};

}  // namespace draco

#endif  // DRACO_SCENE_SCENE_NODE_H_
#endif  // DRACO_TRANSCODER_SUPPORTED
