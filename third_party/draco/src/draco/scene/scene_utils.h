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
#ifndef DRACO_SCENE_SCENE_UTILS_H_
#define DRACO_SCENE_SCENE_UTILS_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/attributes/geometry_attribute.h"
#include "draco/scene/scene.h"

namespace draco {

// Helper class containing various utility functions operating on draco::Scene.
class SceneUtils {
 public:
  // Helper struct holding instanced meshes and their transformations.
  struct MeshInstance {
    // Index of the parent mesh in the draco::Scene.
    MeshIndex mesh_index;
    // Index of the node in the draco::Scene.
    SceneNodeIndex scene_node_index;
    // Index of the mesh in the mesh group.
    int mesh_group_mesh_index;
    // Transform of the instance from the mesh local space to the global space
    // of the scene.
    Eigen::Matrix4d transform;
  };

  // Computes all mesh instances in the |scene|.
  static IndexTypeVector<MeshInstanceIndex, MeshInstance> ComputeAllInstances(
      const Scene &scene);

  // Computes all mesh instances in the node hierarchy originating from
  // |node_index|. All instance transformations will be relative to the source
  // node. That is transformation of parent nodes will not be included.
  static IndexTypeVector<MeshInstanceIndex, MeshInstance>
  ComputeAllInstancesFromNode(const Scene &scene, SceneNodeIndex node_index);

  // Computes global transform matrix of a |scene| node given by its |index|.
  static Eigen::Matrix4d ComputeGlobalNodeTransform(const Scene &scene,
                                                    SceneNodeIndex index);

  // Returns a vector of mesh instance counts for all base meshes.
  static IndexTypeVector<MeshIndex, int> NumMeshInstances(const Scene &scene);

  // Returns the material index of the given |instance| or -1 if the mesh
  // |instance| has a default material.
  static int GetMeshInstanceMaterialIndex(const Scene &scene,
                                          const MeshInstance &instance);

  // Returns the total number of faces on all base meshes of the scene (not
  // counting instances).
  static int NumFacesOnBaseMeshes(const Scene &scene);

  // Returns the total number of faces on all meshes of the scenes, including
  // all instances of the same mesh.
  static int NumFacesOnInstancedMeshes(const Scene &scene);

  // Returns the total number of points on all base meshes of the scene (not
  // counting instances).
  static int NumPointsOnBaseMeshes(const Scene &scene);

  // Returns the total number of points on all meshes of the scenes, including
  // all instances of the same mesh.
  static int NumPointsOnInstancedMeshes(const Scene &scene);

  // Returns the total number of attribute entries on all base meshes of the
  // scene (not counting instances) for the first attribute of |att_type|.
  static int NumAttEntriesOnBaseMeshes(const Scene &scene,
                                       GeometryAttribute::Type att_type);

  // Returns the total number of attribute ent on all meshes of the scenes,
  // including all instances of the same mesh for the first attribute of
  // |att_type|.
  static int NumAttEntriesOnInstancedMeshes(const Scene &scene,
                                            GeometryAttribute::Type att_type);

  // Returns the bounding box of the scene.
  static BoundingBox ComputeBoundingBox(const Scene &scene);

  // Returns the bounding box of a mesh instance.
  static BoundingBox ComputeMeshInstanceBoundingBox(
      const Scene &scene, const MeshInstance &instance);

  // Prints info about input and simplified scenes.
  static void PrintInfo(const Scene &input, const Scene &simplified,
                        bool verbose);

  // Converts a draco::Mesh into a draco::Scene. If the passed-in `mesh` has
  // multiple materials, the returned scene will contain multiple meshes, one
  // for each of the source mesh's materials; if `mesh` has no material, one
  // will be created for it.
  //
  // By default, |MeshToScene| will attempt to deduplicate vertices if the mesh
  // has multiple materials. This means lower memory usage and smaller output
  // glTFs after encoding. However, for very large meshes, this may become an
  // expensive operation. If that becomes an issue, you might want to consider
  // disabling deduplication by setting |deduplicate_vertices| to false. Note
  // that at this moment, disabling deduplication works ONLY for point clouds.
  static StatusOr<std::unique_ptr<Scene>> MeshToScene(
      std::unique_ptr<Mesh> mesh, bool deduplicate_vertices = true);

  // Creates a mesh according to mesh |instance| in |scene|. Error is returned
  // if there is no corresponding base mesh in the |scene| or the base mesh has
  // no valid positions.
  static StatusOr<std::unique_ptr<Mesh>> InstantiateMesh(
      const Scene &scene, const MeshInstance &instance);

  // Cleans up a |scene| by removing unused base meshes, unused and empty mesh
  // groups, unused materials, unused texture coordinates and unused scene
  // nodes. The actual behavior of the cleanup operation can be controller via
  // the user provided |options|.
  struct CleanupOptions {
    bool remove_invalid_mesh_instances = true;
    bool remove_unused_mesh_groups = true;
    bool remove_unused_meshes = true;
    bool remove_unused_nodes = false;
    bool remove_unused_tex_coords = false;
    bool remove_unused_materials = true;
  };
  static void Cleanup(Scene *scene);
  static void Cleanup(Scene *scene, const CleanupOptions &options);

  // Removes mesh |instances| from |scene|.
  static void RemoveMeshInstances(const std::vector<MeshInstance> &instances,
                                  Scene *scene);

  // Removes duplicate mesh groups that have the same name and that contain
  // exactly the same meshes and materials.
  static void DeduplicateMeshGroups(Scene *scene);

  // Enables geometry compression and sets compression |options| to all meshes
  // in the |scene|. If |options| is nullptr then geometry compression is
  // disabled for all meshes in the |scene|.
  static void SetDracoCompressionOptions(const DracoCompressionOptions *options,
                                         Scene *scene);

  // Returns true if geometry compression is eabled for any of |scene| meshes.
  static bool IsDracoCompressionEnabled(const Scene &scene);

  // Returns a single tranformation matrix for each base mesh of the |scene|
  // corresponding to the instance with the maximum scale.
  static IndexTypeVector<MeshIndex, Eigen::Matrix4d>
  FindLargestBaseMeshTransforms(const Scene &scene);
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_SCENE_SCENE_UTILS_H_
