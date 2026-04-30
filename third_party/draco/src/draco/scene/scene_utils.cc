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

#include "draco/scene/scene_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "draco/core/draco_index_type_vector.h"
#include "draco/core/hash_utils.h"
#include "draco/core/vector_d.h"
#include "draco/mesh/mesh_splitter.h"
#include "draco/mesh/mesh_utils.h"
#include "draco/scene/scene_indices.h"
#include "draco/texture/texture_utils.h"

namespace draco {

IndexTypeVector<MeshInstanceIndex, SceneUtils::MeshInstance>
SceneUtils::ComputeAllInstances(const Scene &scene) {
  IndexTypeVector<MeshInstanceIndex, MeshInstance> instances;
  for (int i = 0; i < scene.NumRootNodes(); ++i) {
    const auto node_instances =
        ComputeAllInstancesFromNode(scene, scene.GetRootNodeIndex(i));
    const size_t old_size = instances.size();
    instances.resize(instances.size() + node_instances.size());
    for (MeshInstanceIndex mii(0); mii < node_instances.size(); ++mii) {
      instances[mii + old_size] = node_instances[mii];
    }
  }
  return instances;
}

IndexTypeVector<MeshInstanceIndex, SceneUtils::MeshInstance>
SceneUtils::ComputeAllInstancesFromNode(const Scene &scene,
                                        SceneNodeIndex node_index) {
  IndexTypeVector<MeshInstanceIndex, MeshInstance> instances;

  // Traverse the scene assuming multiple root nodes.
  const Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();

  struct Node {
    const SceneNodeIndex scene_node_index;
    Eigen::Matrix4d transform;
  };
  std::vector<Node> nodes;
  nodes.push_back({node_index, transform});

  while (!nodes.empty()) {
    const Node node = nodes.back();
    nodes.pop_back();
    const SceneNode &scene_node = *scene.GetNode(node.scene_node_index);
    const Eigen::Matrix4d combined_transform =
        node.transform *
        scene_node.GetTrsMatrix().ComputeTransformationMatrix();

    // Create instances from node meshes.
    const MeshGroupIndex mesh_group_index = scene_node.GetMeshGroupIndex();
    if (mesh_group_index != kInvalidMeshGroupIndex) {
      const MeshGroup &mesh_group = *scene.GetMeshGroup(mesh_group_index);
      for (int i = 0; i < mesh_group.NumMeshInstances(); i++) {
        const MeshIndex mesh_index = mesh_group.GetMeshInstance(i).mesh_index;
        if (mesh_index != kInvalidMeshIndex) {
          instances.push_back(
              {mesh_index, node.scene_node_index, i, combined_transform});
        }
      }
    }

    // Traverse children nodes.
    for (int i = 0; i < scene_node.NumChildren(); i++) {
      nodes.push_back({scene_node.Child(i), combined_transform});
    }
  }
  return instances;
}

Eigen::Matrix4d SceneUtils::ComputeGlobalNodeTransform(const Scene &scene,
                                                       SceneNodeIndex index) {
  Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
  while (index != kInvalidSceneNodeIndex) {
    const SceneNode *const node = scene.GetNode(index);
    transform = node->GetTrsMatrix().ComputeTransformationMatrix() * transform;
    index = node->NumParents() == 1 ? node->Parent(0) : kInvalidSceneNodeIndex;
  }
  return transform;
}

IndexTypeVector<MeshIndex, int> SceneUtils::NumMeshInstances(
    const Scene &scene) {
  const auto instances = ComputeAllInstances(scene);
  IndexTypeVector<MeshIndex, int> num_mesh_instances(scene.NumMeshes(), 0);
  for (MeshInstanceIndex i(0); i < instances.size(); i++) {
    const MeshInstance &instance = instances[i];
    num_mesh_instances[instance.mesh_index]++;
  }
  return num_mesh_instances;
}

int SceneUtils::GetMeshInstanceMaterialIndex(const Scene &scene,
                                             const MeshInstance &instance) {
  const auto *const node = scene.GetNode(instance.scene_node_index);
  return scene.GetMeshGroup(node->GetMeshGroupIndex())
      ->GetMeshInstance(instance.mesh_group_mesh_index)
      .material_index;
}

int SceneUtils::NumFacesOnBaseMeshes(const Scene &scene) {
  int num_faces = 0;
  for (MeshIndex i(0); i < scene.NumMeshes(); ++i) {
    num_faces += scene.GetMesh(i).num_faces();
  }
  return num_faces;
}

int SceneUtils::NumFacesOnInstancedMeshes(const Scene &scene) {
  const auto instances = ComputeAllInstances(scene);
  int num_faces = 0;
  for (MeshInstanceIndex i(0); i < instances.size(); i++) {
    const MeshInstance &instance = instances[i];
    num_faces += scene.GetMesh(instance.mesh_index).num_faces();
  }
  return num_faces;
}

int SceneUtils::NumPointsOnBaseMeshes(const Scene &scene) {
  int num_points = 0;
  for (MeshIndex i(0); i < scene.NumMeshes(); ++i) {
    num_points += scene.GetMesh(i).num_points();
  }
  return num_points;
}

int SceneUtils::NumPointsOnInstancedMeshes(const Scene &scene) {
  const auto instances = ComputeAllInstances(scene);
  int num_points = 0;
  for (MeshInstanceIndex i(0); i < instances.size(); i++) {
    const MeshInstance &instance = instances[i];
    num_points += scene.GetMesh(instance.mesh_index).num_points();
  }
  return num_points;
}

int SceneUtils::NumAttEntriesOnBaseMeshes(const Scene &scene,
                                          GeometryAttribute::Type att_type) {
  int num_att_entries = 0;
  for (MeshIndex i(0); i < scene.NumMeshes(); ++i) {
    const Mesh &mesh = scene.GetMesh(i);
    const PointAttribute *att = mesh.GetNamedAttribute(att_type);
    if (att != nullptr) {
      num_att_entries += att->size();
    }
  }
  return num_att_entries;
}

int SceneUtils::NumAttEntriesOnInstancedMeshes(
    const Scene &scene, GeometryAttribute::Type att_type) {
  const auto instances = ComputeAllInstances(scene);
  int num_att_entries = 0;
  for (MeshInstanceIndex i(0); i < instances.size(); i++) {
    const MeshInstance &instance = instances[i];
    const Mesh &mesh = scene.GetMesh(instance.mesh_index);
    const PointAttribute *att = mesh.GetNamedAttribute(att_type);
    if (att != nullptr) {
      num_att_entries += att->size();
    }
  }
  return num_att_entries;
}

BoundingBox SceneUtils::ComputeBoundingBox(const Scene &scene) {
  // Compute bounding box that includes all scene mesh instances.
  const auto instances = ComputeAllInstances(scene);
  BoundingBox scene_bbox;
  for (MeshInstanceIndex i(0); i < instances.size(); i++) {
    const MeshInstance &instance = instances[i];
    const BoundingBox mesh_bbox =
        ComputeMeshInstanceBoundingBox(scene, instance);
    scene_bbox.Update(mesh_bbox);
  }
  return scene_bbox;
}

BoundingBox SceneUtils::ComputeMeshInstanceBoundingBox(
    const Scene &scene, const MeshInstance &instance) {
  const Mesh &mesh = scene.GetMesh(instance.mesh_index);
  BoundingBox mesh_bbox;
  auto pc_att = mesh.GetNamedAttribute(GeometryAttribute::POSITION);
  Eigen::Vector4d position;
  position[3] = 1.0;
  for (AttributeValueIndex i(0); i < pc_att->size(); ++i) {
    pc_att->ConvertValue<double>(i, &position[0]);
    const Eigen::Vector4d transformed = instance.transform * position;
    mesh_bbox.Update({static_cast<float>(transformed[0]),
                      static_cast<float>(transformed[1]),
                      static_cast<float>(transformed[2])});
  }
  return mesh_bbox;
}

namespace {

// Updates texture pointers in mesh features of |mesh| to texture pointers
// stored in |new_texture_library|. |texture_to_index_map| stores texture
// indices of the old texture pointers within |mesh|.
void UpdateMeshFeaturesTexturesOnMesh(
    const std::unordered_map<const Texture *, int> &texture_to_index_map,
    TextureLibrary *new_texture_library, Mesh *mesh) {
  for (MeshFeaturesIndex mfi(0); mfi < mesh->NumMeshFeatures(); ++mfi) {
    mesh->UpdateMeshFeaturesTexturePointer(
        texture_to_index_map, new_texture_library, &mesh->GetMeshFeatures(mfi));
  }
}

}  // namespace

StatusOr<std::unique_ptr<Scene>> SceneUtils::MeshToScene(
    std::unique_ptr<Mesh> mesh, bool deduplicate_vertices) {
  const size_t num_mesh_materials = mesh->GetMaterialLibrary().NumMaterials();
  std::unique_ptr<Scene> scene(new Scene());
  if (num_mesh_materials > 0) {
    scene->GetMaterialLibrary().Copy(mesh->GetMaterialLibrary());
    mesh->GetMaterialLibrary().Clear();
  } else {
    // Create a default material for the scene.
    scene->GetMaterialLibrary().MutableMaterial(0);
  }

  // Copy structural metadata.
  scene->GetStructuralMetadata().Copy(mesh->GetStructuralMetadata());

  // Copy mesh feature textures.
  scene->GetNonMaterialTextureLibrary().Copy(
      mesh->GetNonMaterialTextureLibrary());

  const auto old_texture_to_index_map =
      mesh->GetNonMaterialTextureLibrary().ComputeTextureToIndexMap();

  const SceneNodeIndex scene_node_index = scene->AddNode();
  SceneNode *const scene_node = scene->GetNode(scene_node_index);
  const MeshGroupIndex mesh_group_index = scene->AddMeshGroup();
  MeshGroup *const mesh_group = scene->GetMeshGroup(mesh_group_index);

  if (num_mesh_materials <= 1) {
    const MeshIndex mesh_index = scene->AddMesh(std::move(mesh));
    if (mesh_index == kInvalidMeshIndex) {
      // No idea whether this can happen.  It's not covered by any unit test.
      return Status(Status::DRACO_ERROR, "Could not add Draco mesh to scene.");
    }
    mesh_group->AddMeshInstance({mesh_index, 0, {}});

    UpdateMeshFeaturesTexturesOnMesh(old_texture_to_index_map,
                                     &scene->GetNonMaterialTextureLibrary(),
                                     &scene->GetMesh(mesh_index));

    // The non-material texture library is now in the scene. The non-material
    // texture library of the mesh must be cleared, because mesh features may
    // contain texture pointers invalid for this non-material texture library.
    scene->GetMesh(mesh_index).GetNonMaterialTextureLibrary().Clear();
  } else {
    const int32_t mat_att_id =
        mesh->GetNamedAttributeId(GeometryAttribute::MATERIAL);
    if (mat_att_id == -1) {
      // Probably dead code, not covered by any unit test.
      return Status(Status::DRACO_ERROR,
                    "Internal error in MeshToScene: "
                    "GetNamedAttributeId(MATERIAL) returned -1");
    }
    const PointAttribute *const mat_att =
        mesh->GetNamedAttribute(GeometryAttribute::MATERIAL);
    if (mat_att == nullptr) {
      // Probably dead code, not covered by any unit test.
      return Status(Status::DRACO_ERROR,
                    "Internal error in MeshToScene: "
                    "GetNamedAttribute(MATERIAL) returned nullptr");
    }

    MeshSplitter splitter;
    splitter.SetDeduplicateVertices(deduplicate_vertices);
    DRACO_ASSIGN_OR_RETURN(MeshSplitter::MeshVector split_meshes,
                           splitter.SplitMesh(*mesh, mat_att_id));
    // Note: cannot clear mesh here, since mat_att points into it.
    for (size_t i = 0; i < split_meshes.size(); ++i) {
      if (split_meshes[i] == nullptr) {
        // Probably dead code, not covered by any unit test.
        continue;
      }
      const MeshIndex mesh_index = scene->AddMesh(std::move(split_meshes[i]));
      if (mesh_index == kInvalidMeshIndex) {
        // No idea whether this can happen.  It's not covered by any unit test.
        return Status(Status::DRACO_ERROR,
                      "Could not add Draco mesh to scene.");
      }

      int material_index = 0;
      mat_att->GetValue(AttributeValueIndex(i), &material_index);
      mesh_group->AddMeshInstance({mesh_index, material_index, {}});

      // Copy over mesh features that were associated with the |material_index|.
      Mesh &scene_mesh = scene->GetMesh(mesh_index);
      Mesh::CopyMeshFeaturesForMaterial(*mesh, &scene_mesh, material_index);

      // Update mesh features attribute indices if needed.
      for (MeshFeaturesIndex mfi(0); mfi < scene_mesh.NumMeshFeatures();
           ++mfi) {
        auto &mesh_features = scene_mesh.GetMeshFeatures(mfi);
        if (mesh_features.GetAttributeIndex() != -1) {
          mesh_features.SetAttributeIndex(splitter.GetSplitMeshAttributeIndex(
              mesh_features.GetAttributeIndex()));
        }
      }

      UpdateMeshFeaturesTexturesOnMesh(old_texture_to_index_map,
                                       &scene->GetNonMaterialTextureLibrary(),
                                       &scene_mesh);

      // Copy over property attibutes indices that were associated with the
      // |material_index|.
      Mesh::CopyPropertyAttributesIndicesForMaterial(*mesh, &scene_mesh,
                                                     material_index);

      // The non-material texture library is now in the scene. The non-material
      // texture library of the mesh must be cleared, because mesh features may
      // contain texture pointers invalid for this non-material texture library.
      scene_mesh.GetNonMaterialTextureLibrary().Clear();
    }
  }

  scene_node->SetMeshGroupIndex(mesh_group_index);
  scene->AddRootNodeIndex(scene_node_index);
  return scene;
}

void SceneUtils::PrintInfo(const Scene &input, const Scene &simplified,
                           bool verbose) {
  struct Printer {
    Printer(const Scene &input, const Scene &simplified)
        : input(input), simplified(simplified), print_instanced_info(false) {
      // Info about the instanced meshes is printed if some of the meshes have
      // multiple instances and also if the number of base meshes has changed.
      auto input_instances = SceneUtils::NumMeshInstances(input);
      auto simplified_instances = SceneUtils::NumMeshInstances(simplified);
      if (input_instances.size() != simplified_instances.size()) {
        print_instanced_info = true;
        return;
      }
      for (MeshIndex i(0); i < input_instances.size(); i++) {
        if (input_instances[i] != 1 || simplified_instances[i] != 1) {
          print_instanced_info = true;
          return;
        }
      }
    }

    void PrintInfoHeader() const {
      printf("\n");
      printf("%21s |   geometry:         base", "");
      if (print_instanced_info) {
        printf("    instanced");
      }
      printf("\n");
    }

    void PrintInfoRow(const std::string &label, int count_input_base,
                      int count_input_instanced, int count_simplified_base,
                      int count_simplified_instanced) const {
      // Do not clutter the printout with empty info.
      if (count_input_base == 0 && count_input_instanced == 0) {
        return;
      }
      printf("  ----------------------------------------------");
      if (print_instanced_info) {
        printf("-------------");
      }
      printf("\n");
      printf("%21s |      input: %12d", label.c_str(), count_input_base);
      if (print_instanced_info) {
        printf(" %12d", count_input_instanced);
      }
      printf("\n");
      printf("%21s | simplified: %12d", "", count_simplified_base);
      if (print_instanced_info) {
        printf(" %12d", count_simplified_instanced);
      }
      printf("\n");
    }

    void PrintAttInfoRow(const std::string &label, const draco::Scene &input,
                         const draco::Scene &simplified,
                         draco::GeometryAttribute::Type att_type) const {
      PrintInfoRow(label, NumAttEntriesOnBaseMeshes(input, att_type),
                   NumAttEntriesOnInstancedMeshes(input, att_type),
                   NumAttEntriesOnBaseMeshes(simplified, att_type),
                   NumAttEntriesOnInstancedMeshes(simplified, att_type));
    }

    const Scene &input;
    const Scene &simplified;
    bool print_instanced_info;
  };

  // Print information about input and simplified scenes.
  const Printer printer(input, simplified);
  printer.PrintInfoHeader();
  if (verbose) {
    const int num_meshes_input_base = input.NumMeshes();
    const int num_meshes_simplified_base = simplified.NumMeshes();
    const int num_meshes_input_instanced = ComputeAllInstances(input).size();
    const int num_meshes_simplified_instanced =
        ComputeAllInstances(simplified).size();
    printer.PrintInfoRow("Number of meshes", num_meshes_input_base,
                         num_meshes_input_instanced, num_meshes_simplified_base,
                         num_meshes_simplified_instanced);
  }
  printer.PrintInfoRow("Number of faces", NumFacesOnBaseMeshes(input),
                       NumFacesOnInstancedMeshes(input),
                       NumFacesOnBaseMeshes(simplified),
                       NumFacesOnInstancedMeshes(simplified));
  if (verbose) {
    printer.PrintInfoRow("Number of points", NumPointsOnBaseMeshes(input),
                         NumPointsOnInstancedMeshes(input),
                         NumPointsOnBaseMeshes(simplified),
                         NumPointsOnInstancedMeshes(simplified));
    printer.PrintAttInfoRow("Number of positions", input, simplified,
                            draco::GeometryAttribute::POSITION);
    printer.PrintAttInfoRow("Number of normals", input, simplified,
                            draco::GeometryAttribute::NORMAL);
    printer.PrintAttInfoRow("Number of colors", input, simplified,
                            draco::GeometryAttribute::COLOR);
    printer.PrintInfoRow("Number of materials",
                         input.GetMaterialLibrary().NumMaterials(),
                         simplified.GetMaterialLibrary().NumMaterials(),
                         input.GetMaterialLibrary().NumMaterials(),
                         simplified.GetMaterialLibrary().NumMaterials());
  }
}

StatusOr<std::unique_ptr<Mesh>> SceneUtils::InstantiateMesh(
    const Scene &scene, const MeshInstance &instance) {
  // Check if the |scene| has base mesh corresponding to mesh |instance|.
  if (scene.NumMeshes() <= instance.mesh_index.value()) {
    Status(Status::DRACO_ERROR, "Scene has no corresponding base mesh.");
  }

  // Check that mesh has valid positions.
  const Mesh &base_mesh = scene.GetMesh(instance.mesh_index);
  const int32_t pos_id =
      base_mesh.GetNamedAttributeId(GeometryAttribute::POSITION);
  const PointAttribute *const pos_att = base_mesh.attribute(pos_id);
  if (pos_att == nullptr) {
    return Status(Status::DRACO_ERROR, "Mesh has no positions.");
  }
  if (pos_att->data_type() != DT_FLOAT32 || pos_att->num_components() != 3) {
    return Status(Status::DRACO_ERROR, "Mesh has invalid positions.");
  }

  // Copy the base mesh from |scene|.
  std::unique_ptr<Mesh> mesh(new Mesh());
  mesh->Copy(base_mesh);

  // Apply transformation to mesh unless transformation is identity.
  if (instance.transform != Eigen::Matrix4d::Identity()) {
    MeshUtils::TransformMesh(instance.transform, mesh.get());
  }
  return mesh;
}

namespace {

// Helper class for deleting unused nodes from the scene.
class SceneUnusedNodeRemover {
 public:
  // Removes unused nodes from the |scene|.
  void RemoveUnusedNodes(Scene *scene) {
    // Finds all unused nodes and initializes |node_map_| that maps old node
    // indices to new node indices.
    const int num_unused_nodes = FindUnusedNodes(*scene);
    if (num_unused_nodes == 0) {
      return;  // All nodes are used.
    }

    // Update indices of all scene elements accounting for nodes that are going
    // to be removed from the scene.
    UpdateNodeIndices(scene);
    RemoveUnusedNodesFromScene(scene);
  }

 private:
  // Returns the number of unused nodes.
  int FindUnusedNodes(const Scene &scene) {
    // First all nodes are considered unused (mapped to invalid index).
    // Initially if a node is used, we just map it to its own index. The final
    // mapping will be updated once we know all used nodes.
    node_map_.resize(scene.NumNodes(), kInvalidSceneNodeIndex);
    for (SceneNodeIndex sni(0); sni < scene.NumNodes(); ++sni) {
      // If the scene node has a valid mesh group, mark it as used.
      if (scene.GetNode(sni)->GetMeshGroupIndex() != kInvalidMeshGroupIndex) {
        node_map_[sni] = sni;
      }
    }

    // Preserve nodes used by animations.
    for (AnimationIndex i(0); i < scene.NumAnimations(); i++) {
      const Animation &animation = *scene.GetAnimation(i);
      for (int channel_i = 0; channel_i < animation.NumChannels();
           channel_i++) {
        const SceneNodeIndex node_index(
            animation.GetChannel(channel_i)->target_index);
        node_map_[node_index] = node_index;
      }
    }
    for (SkinIndex i(0); i < scene.NumSkins(); i++) {
      const Skin &skin = *scene.GetSkin(i);
      for (int j = 0; j < skin.NumJoints(); j++) {
        const SceneNodeIndex node_index = skin.GetJoint(j);
        node_map_[node_index] = node_index;
      }
      const SceneNodeIndex root_index = skin.GetJointRoot();
      if (root_index != kInvalidSceneNodeIndex) {
        node_map_[root_index] = root_index;
      }
    }

    // Ensure that "unused" nodes with used child nodes are marked as used
    // (a node can't be deleted as long as it has a used child node).
    for (int r = 0; r < scene.NumRootNodes(); ++r) {
      UpdateUsedNodesFromSceneGraph(scene, scene.GetRootNodeIndex(r));
    }

    // All used / unused nodes are known. Find new indices for all scene nodes.
    int num_valid_nodes = 0;
    for (SceneNodeIndex sni(0); sni < scene.NumNodes(); ++sni) {
      if (node_map_[sni] != kInvalidSceneNodeIndex) {
        node_map_[sni] = SceneNodeIndex(num_valid_nodes++);
      }
    }
    // Return the number of nodes that were unused.
    return scene.NumNodes() - num_valid_nodes;
  }

  // Recursively traverse node |sni| and mark it as used as long as it has a
  // used child node. The function returns true when |sni| is a used node.
  bool UpdateUsedNodesFromSceneGraph(const Scene &scene, SceneNodeIndex sni) {
    const auto &node = scene.GetNode(sni);
    bool is_any_child_node_used = false;
    for (int c = 0; c < node->NumChildren(); ++c) {
      const SceneNodeIndex cni = node->Child(c);
      // Check if the child node is used.
      const bool is_c_used = UpdateUsedNodesFromSceneGraph(scene, cni);
      if (is_c_used) {
        is_any_child_node_used = true;
      }
    }
    if (is_any_child_node_used) {
      // The node must be used even if it was previously marked as unused.
      node_map_[sni] = sni;
    }
    // Returns whether this node is used or not.
    return node_map_[sni] != kInvalidSceneNodeIndex;
  }

  // Remaps existing node indices at various scene elements to new node indices
  // defined by |node_map_|.
  void UpdateNodeIndices(Scene *scene) const {
    // Update node indices on child / parent nodes.
    std::vector<SceneNodeIndex> indices;
    for (SceneNodeIndex sni(0); sni < scene->NumNodes(); ++sni) {
      indices = scene->GetNode(sni)->Children();
      scene->GetNode(sni)->RemoveAllChildren();
      for (int j = 0; j < indices.size(); ++j) {
        const SceneNodeIndex new_sni = node_map_[indices[j]];
        if (new_sni != kInvalidSceneNodeIndex) {
          scene->GetNode(sni)->AddChildIndex(new_sni);
        }
      }
      indices = scene->GetNode(sni)->Parents();
      scene->GetNode(sni)->RemoveAllParents();
      for (int j = 0; j < indices.size(); ++j) {
        const SceneNodeIndex new_sni = node_map_[indices[j]];
        if (new_sni != kInvalidSceneNodeIndex) {
          scene->GetNode(sni)->AddParentIndex(new_sni);
        }
      }
    }

    // Update root node indices.
    indices = scene->GetRootNodeIndices();
    scene->RemoveAllRootNodeIndices();
    for (int ri = 0; ri < indices.size(); ++ri) {
      const SceneNodeIndex new_rni = node_map_[indices[ri]];
      if (new_rni != kInvalidSceneNodeIndex) {
        scene->AddRootNodeIndex(new_rni);
      }
    }

    // Update node indices used by animations.
    for (AnimationIndex i(0); i < scene->NumAnimations(); i++) {
      Animation &animation = *scene->GetAnimation(i);
      for (int i = 0; i < animation.NumChannels(); i++) {
        const SceneNodeIndex node_index(animation.GetChannel(i)->target_index);
        animation.GetChannel(i)->target_index = node_map_[node_index].value();
      }
    }
    for (SkinIndex i(0); i < scene->NumSkins(); i++) {
      Skin &skin = *scene->GetSkin(i);
      for (int j = 0; j < skin.NumJoints(); j++) {
        const SceneNodeIndex node_index = skin.GetJoint(j);
        skin.GetJoint(j) = node_map_[node_index];
      }
      const SceneNodeIndex root_index = skin.GetJointRoot();
      if (root_index != kInvalidSceneNodeIndex) {
        skin.SetJointRoot(node_map_[root_index]);
      }
    }
  }

  // Removes all unused nodes from the scene.
  void RemoveUnusedNodesFromScene(Scene *scene) const {
    int num_valid_nodes = 0;
    // Copy over nodes to their new position in the nodes array.
    for (SceneNodeIndex sni(0); sni < scene->NumNodes(); ++sni) {
      const SceneNodeIndex new_sni = node_map_[sni];
      if (new_sni == kInvalidSceneNodeIndex) {
        continue;
      }
      num_valid_nodes++;
      if (sni != new_sni) {
        // Copy over the |sni| node to the new location (|new_sni| is lower than
        // |sni|).
        scene->GetNode(new_sni)->Copy(*scene->GetNode(sni));
      }
    }
    // Resize the nodes in the scene to account for the unused ones. This will
    // delete all unused nodes.
    scene->ResizeNodes(num_valid_nodes);
  }

  IndexTypeVector<SceneNodeIndex, SceneNodeIndex> node_map_;
};

}  // namespace

void SceneUtils::Cleanup(Scene *scene) { Cleanup(scene, CleanupOptions()); }

void SceneUtils::Cleanup(Scene *scene, const CleanupOptions &options) {
  // Remove invalid mesh indices from mesh groups.
  if (options.remove_invalid_mesh_instances) {
    for (MeshGroupIndex i(0); i < scene->NumMeshGroups(); i++) {
      scene->GetMeshGroup(i)->RemoveMeshInstances(kInvalidMeshIndex);
    }
  }

  // Find references to mesh groups.
  std::vector<bool> is_mesh_group_referenced(scene->NumMeshGroups(), false);
  for (SceneNodeIndex i(0); i < scene->NumNodes(); i++) {
    const SceneNode &node = *scene->GetNode(i);
    const MeshGroupIndex mesh_group_index = node.GetMeshGroupIndex();
    if (mesh_group_index != kInvalidMeshGroupIndex) {
      is_mesh_group_referenced[mesh_group_index.value()] = true;
    }
  }

  // Find references to base meshes from referenced mesh groups and find mesh
  // groups that have no valid references to base meshes.
  std::vector<bool> is_base_mesh_referenced(scene->NumMeshes(), false);
  std::vector<bool> is_mesh_group_empty(scene->NumMeshGroups(), false);
  for (MeshGroupIndex i(0); i < scene->NumMeshGroups(); i++) {
    if (!is_mesh_group_referenced[i.value()]) {
      continue;
    }
    const MeshGroup &mesh_group = *scene->GetMeshGroup(i);
    bool mesh_group_is_empty = true;
    for (int j = 0; j < mesh_group.NumMeshInstances(); j++) {
      const MeshIndex mesh_index = mesh_group.GetMeshInstance(j).mesh_index;
      mesh_group_is_empty = false;
      is_base_mesh_referenced[mesh_index.value()] = true;
    }
    if (mesh_group_is_empty) {
      is_mesh_group_empty[i.value()] = true;
    }
  }

  if (options.remove_unused_meshes) {
    // Remove base meshes with no references to them.
    for (int i = scene->NumMeshes() - 1; i >= 0; i--) {
      const MeshIndex mi(i);
      if (!is_base_mesh_referenced[mi.value()]) {
        scene->RemoveMesh(mi);
      }
    }
  }

  if (options.remove_unused_mesh_groups) {
    // Remove empty mesh groups with no geometry or no references to them.
    for (int i = scene->NumMeshGroups() - 1; i >= 0; i--) {
      const MeshGroupIndex mgi(i);
      if (is_mesh_group_empty[mgi.value()] ||
          !is_mesh_group_referenced[mgi.value()]) {
        scene->RemoveMeshGroup(mgi);
      }
    }
  }

  // Find materials that reference a texture.
  MaterialLibrary &material_library = scene->GetMaterialLibrary();
  std::vector<bool> materials_with_textures(material_library.NumMaterials(),
                                            false);
  for (int i = 0; i < material_library.NumMaterials(); ++i) {
    if (material_library.GetMaterial(i)->NumTextureMaps() > 0) {
      materials_with_textures[i] = true;
    }
  }

  // Maps material index to a set of meshes that use that material.
  std::vector<std::unordered_set<MeshIndex>> material_meshes(
      material_library.NumMaterials());

  // Maps mesh index to a set of materials used by that mesh.
  IndexTypeVector<MeshIndex, std::unordered_set<int>> mesh_materials(
      scene->NumMeshes());

  // Maps mesh index to a set of tex coord indices referenced by materials.
  IndexTypeVector<MeshIndex, std::unordered_set<int>> tex_coord_referenced(
      scene->NumMeshes());

  // Populate the maps that will be used to remove unused texture coordinates.
  for (int mgi = 0; mgi < scene->NumMeshGroups(); ++mgi) {
    const MeshGroup *const mesh_group =
        scene->GetMeshGroup(MeshGroupIndex(mgi));
    for (int mi = 0; mi < mesh_group->NumMeshInstances(); ++mi) {
      const MeshIndex mesh_index = mesh_group->GetMeshInstance(mi).mesh_index;
      const int material_index = mesh_group->GetMeshInstance(mi).material_index;
      if (material_index == -1) {
        continue;
      }

      // Populate mesh-material mapping.
      material_meshes[material_index].insert(mesh_index);
      mesh_materials[mesh_index].insert(material_index);

      // Populate texture coordinate indices referenced by material textures.
      const auto material = material_library.GetMaterial(material_index);
      for (int i = 0; i < material->NumTextureMaps(); i++) {
        const TextureMap *const texture_map = material->GetTextureMapByIndex(i);
        const int tex_coord_index = texture_map->tex_coord_index();
        tex_coord_referenced[mesh_index].insert(tex_coord_index);
      }
    }
  }

  // From each mesh, remove texture coordinate attributes that are not
  // referenced by any materials and decrement texture coordinate indices in
  // texture maps of the mesh materials accordingly.
  if (options.remove_unused_tex_coords) {
    for (MeshIndex mi(0); mi < scene->NumMeshes(); ++mi) {
      // Do not remove unreferenced texture coordinates when the mesh materials
      // are used by any other meshes to avoid corrupting those other meshes.
      // TODO(vytyaz): Consider removing this limitation.
      bool remove_tex_coord = true;
      for (const int material_index : mesh_materials[mi]) {
        if (material_meshes[material_index].size() != 1) {
          // Materials of this mesh are used by other meshes.
          remove_tex_coord = false;
          break;
        }
      }
      if (!remove_tex_coord) {
        continue;
      }

      // Remove unreferenced texture coordinate sets from this mesh.
      Mesh &mesh = scene->GetMesh(mi);
      const int tex_coord_count =
          mesh.NumNamedAttributes(GeometryAttribute::TEX_COORD);
      for (int tci = tex_coord_count - 1; tci >= 0; tci--) {
        if (tex_coord_referenced[mi].count(tci) != 0) {
          // Texture coordinate set is referenced.
          continue;
        }
        mesh.DeleteAttribute(
            mesh.GetNamedAttributeId(GeometryAttribute::TEX_COORD, tci));

        // Decrement texture coordinate indices in all materials of this mesh.
        for (const int material_index : mesh_materials[mi]) {
          auto material = material_library.MutableMaterial(material_index);
          for (int i = 0; i < material->NumTextureMaps(); i++) {
            auto texture_map = material->GetTextureMapByIndex(i);
            // Decrement the indices that are greater than the removed index.
            if (texture_map->tex_coord_index() > tci) {
              texture_map->SetProperties(texture_map->type(),
                                         texture_map->tex_coord_index() - 1);
            }
          }
        }
      }
    }
  }

  if (options.remove_unused_materials) {
    // Remove materials that are not used by any mesh.
    for (int i = material_library.NumMaterials() - 1; i >= 0; --i) {
      if (material_meshes[i].empty()) {
        // Material |i| is not used.
        scene->RemoveMaterial(i);
      }
    }
  }

  if (options.remove_unused_nodes) {
    SceneUnusedNodeRemover node_remover;
    node_remover.RemoveUnusedNodes(scene);
  }
}

void SceneUtils::RemoveMeshInstances(const std::vector<MeshInstance> &instances,
                                     Scene *scene) {
  // Remove mesh instances from the scene.
  for (const SceneUtils::MeshInstance &instance : instances) {
    const MeshGroupIndex mgi =
        scene->GetNode(instance.scene_node_index)->GetMeshGroupIndex();

    // Create a new mesh group with removed instance (we can't just delete the
    // instance from the mesh group directly, because the same mesh group may
    // be used by multiple scene nodes).
    const MeshGroupIndex new_mesh_group_index = scene->AddMeshGroup();
    MeshGroup &new_mesh_group = *scene->GetMeshGroup(new_mesh_group_index);

    new_mesh_group.Copy(*scene->GetMeshGroup(mgi));
    new_mesh_group.RemoveMeshInstances(instance.mesh_index);

    // Assign the new mesh group to the scene node. Unused mesh groups will be
    // automatically removed later during a scene cleanup operation.
    scene->GetNode(instance.scene_node_index)
        ->SetMeshGroupIndex(new_mesh_group_index);
  }

  // Remove duplicate mesh groups that may have been created during the instance
  // removal process.
  DeduplicateMeshGroups(scene);
}

void SceneUtils::DeduplicateMeshGroups(Scene *scene) {
  if (scene->NumMeshGroups() <= 1) {
    return;
  }

  // Signature of a mesh group used for detecting duplicates.
  struct MeshGroupSignature {
    const MeshGroupIndex mesh_group_index;
    const MeshGroup &mesh_group;
    MeshGroupSignature(MeshGroupIndex mgi, const MeshGroup &mesh_group)
        : mesh_group_index(mgi), mesh_group(mesh_group) {}

    bool operator==(const MeshGroupSignature &signature) const {
      if (mesh_group.GetName() != signature.mesh_group.GetName()) {
        return false;
      }
      if (mesh_group.NumMeshInstances() !=
          signature.mesh_group.NumMeshInstances()) {
        return false;
      }
      // TODO(ostava): We may consider sorting meshes within a mesh group to
      // make the order of meshes irrelevant. This should be done only for
      // meshes with opaque materials though, because for transparent
      // geometries, the order matters.
      for (int i = 0; i < mesh_group.NumMeshInstances(); ++i) {
        if (mesh_group.GetMeshInstance(i) !=
            signature.mesh_group.GetMeshInstance(i)) {
          return false;
        }
      }
      return true;
    }
    struct Hash {
      size_t operator()(const MeshGroupSignature &signature) const {
        size_t hash = 79;  // Magic number.
        const MeshGroup &group = signature.mesh_group;
        hash = HashCombine(group.GetName(), hash);
        hash = HashCombine(group.NumMeshInstances(), hash);
        for (int i = 0; i < group.NumMeshInstances(); ++i) {
          const MeshGroup::MeshInstance &instance = group.GetMeshInstance(i);
          hash = HashCombine(instance.mesh_index, hash);
          hash = HashCombine(instance.material_index, hash);
          hash = HashCombine(instance.materials_variants_mappings.size(), hash);
          for (const MeshGroup::MaterialsVariantsMapping &mapping :
               instance.materials_variants_mappings) {
            hash = HashCombine(mapping.material, hash);
            hash = HashCombine(mapping.variants.size(), hash);
            for (const int &variant : mapping.variants) {
              hash = HashCombine(variant, hash);
            }
          }
        }
        return hash;
      }
    };
  };

  // Set holding unique mesh groups.
  std::unordered_set<MeshGroupSignature, MeshGroupSignature::Hash>
      unique_mesh_groups;
  IndexTypeVector<MeshGroupIndex, MeshGroupIndex> parent_mesh_group(
      scene->NumMeshGroups());
  for (MeshGroupIndex mgi(0); mgi < scene->NumMeshGroups(); ++mgi) {
    const MeshGroup *mg = scene->GetMeshGroup(mgi);
    const MeshGroupSignature signature(mgi, *mg);
    auto it = unique_mesh_groups.find(signature);
    if (it != unique_mesh_groups.end()) {
      parent_mesh_group[mgi] = it->mesh_group_index;
    } else {
      parent_mesh_group[mgi] = kInvalidMeshGroupIndex;
      unique_mesh_groups.insert(signature);
    }
  }

  // Go over all nodes and update mesh groups if needed.
  for (SceneNodeIndex sni(0); sni < scene->NumNodes(); ++sni) {
    const MeshGroupIndex mgi = scene->GetNode(sni)->GetMeshGroupIndex();
    if (mgi == kInvalidMeshGroupIndex ||
        parent_mesh_group[mgi] == kInvalidMeshGroupIndex) {
      continue;  // Nothing to update.
    }
    scene->GetNode(sni)->SetMeshGroupIndex(parent_mesh_group[mgi]);
  }

  // Remove any unused mesh groups.
  Cleanup(scene);
}

void SceneUtils::SetDracoCompressionOptions(
    const DracoCompressionOptions *options, Scene *scene) {
  for (MeshIndex i(0); i < scene->NumMeshes(); ++i) {
    Mesh &mesh = scene->GetMesh(i);
    if (options == nullptr) {
      mesh.SetCompressionEnabled(false);
    } else {
      mesh.SetCompressionEnabled(true);
      mesh.SetCompressionOptions(*options);
    }
  }
}

bool SceneUtils::IsDracoCompressionEnabled(const Scene &scene) {
  for (MeshIndex i(0); i < scene.NumMeshes(); ++i) {
    if (scene.GetMesh(i).IsCompressionEnabled()) {
      return true;
    }
  }
  return false;
}

IndexTypeVector<MeshIndex, Eigen::Matrix4d>
SceneUtils::FindLargestBaseMeshTransforms(const Scene &scene) {
  IndexTypeVector<MeshIndex, Eigen::Matrix4d> transforms(
      scene.NumMeshes(), Eigen::Matrix4d::Identity());

  // In case a mesh has multiple instances we want to use the instance with
  // the largest scale.
  IndexTypeVector<MeshIndex, float> transform_scale(scene.NumMeshes(), 0.f);

  const auto instances = SceneUtils::ComputeAllInstances(scene);
  for (MeshInstanceIndex i(0); i < instances.size(); ++i) {
    const auto &instance = instances[i];

    // Compute the scale of the transform.
    const Vector3f scale_vec(instance.transform.col(0).norm(),
                             instance.transform.col(1).norm(),
                             instance.transform.col(2).norm());

    // In our framework we support uniform scale only. For now, just take the
    // maximum scale across all axes.
    // TODO(ostava): Investigate how to properly support non-uniform scaling.
    const float max_scale = scale_vec.MaxCoeff();

    if (transform_scale[instance.mesh_index] < max_scale) {
      transform_scale[instance.mesh_index] = max_scale;
      transforms[instance.mesh_index] = instance.transform;
    }
  }

  return transforms;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
