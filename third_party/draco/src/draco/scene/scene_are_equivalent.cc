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
#include "draco/scene/scene_are_equivalent.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/mesh/mesh_are_equivalent.h"

namespace draco {

bool SceneAreEquivalent::operator()(const Scene &scene0, const Scene &scene1) {
  // Check scene component sizes.
  if (scene0.NumAnimations() != scene1.NumAnimations()) {
    return false;
  }
  if (scene0.NumMeshGroups() != scene1.NumMeshGroups()) {
    return false;
  }
  if (scene0.NumSkins() != scene1.NumSkins()) {
    return false;
  }

  // Check equivalence of each mesh.
  if (scene0.NumMeshes() != scene1.NumMeshes()) {
    return false;
  }
  for (MeshIndex i(0); i < scene0.NumMeshes(); i++) {
    if (!AreEquivalent(scene0.GetMesh(i), scene1.GetMesh(i))) {
      return false;
    }
  }

  // Check eqiuvalence of each node.
  if (scene0.NumNodes() != scene1.NumNodes()) {
    return false;
  }
  for (SceneNodeIndex i(0); i < scene0.NumNodes(); i++) {
    if (!AreEquivalent(*scene0.GetNode(i), *scene1.GetNode(i))) {
      return false;
    }
  }

  // Check non-material texture library sizes.
  if (scene0.GetNonMaterialTextureLibrary().NumTextures() !=
      scene1.GetNonMaterialTextureLibrary().NumTextures()) {
    return false;
  }

  // TODO(vytyaz): Check remaining scene properties like animations and skins.
  return true;
}

bool SceneAreEquivalent::AreEquivalent(const Mesh &mesh0, const Mesh &mesh1) {
  MeshAreEquivalent eq;
  return eq(mesh0, mesh1);
}

bool SceneAreEquivalent::AreEquivalent(const SceneNode &node0,
                                       const SceneNode &node1) {
  // Check equivalence of node indices.
  if (node0.GetMeshGroupIndex() != node1.GetMeshGroupIndex()) {
    return false;
  }
  if (node0.GetSkinIndex() != node1.GetSkinIndex()) {
    return false;
  }

  // Check equivalence of node transformations.
  if (node0.GetTrsMatrix().ComputeTransformationMatrix() !=
      node1.GetTrsMatrix().ComputeTransformationMatrix()) {
    return false;
  }

  // Check equivalence of node children.
  if (node0.NumChildren() != node1.NumChildren()) {
    return false;
  }
  for (int i = 0; i < node0.NumChildren(); i++) {
    if (node0.Child(i) != node1.Child(i)) {
      return false;
    }
  }

  // Check equivalence of node parents.
  if (node0.NumParents() != node1.NumParents()) {
    return false;
  }
  for (int i = 0; i < node0.NumParents(); i++) {
    if (node0.Parent(i) != node1.Parent(i)) {
      return false;
    }
  }
  return true;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
