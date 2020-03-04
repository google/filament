// Copyright 2016 The Draco Authors.
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
#ifndef DRACO_MESH_MESH_ARE_EQUIVALENT_H_
#define DRACO_MESH_MESH_ARE_EQUIVALENT_H_

#include "draco/core/vector_d.h"
#include "draco/mesh/mesh.h"

// This file defines a functor to compare two meshes for equivalency up
// to permutation of the vertices.
namespace draco {

// A functor to compare two meshes for equivalency up to permutation of the
// vertices.
class MeshAreEquivalent {
 public:
  // Returns true if both meshes are equivalent up to permutation of
  // the internal order of vertices. This includes all attributes.
  bool operator()(const Mesh &mesh0, const Mesh &mesh1);

 private:
  // Internal type to keep overview.
  struct MeshInfo {
    explicit MeshInfo(const Mesh &in_mesh) : mesh(in_mesh) {}
    const Mesh &mesh;
    std::vector<FaceIndex> ordered_index_of_face;
    IndexTypeVector<FaceIndex, int> corner_index_of_smallest_vertex;
  };

  // Prepare functor for actual comparison.
  void Init(const Mesh &mesh0, const Mesh &mesh1);

  // Get position as Vector3f of corner c of face f.
  static Vector3f GetPosition(const Mesh &mesh, FaceIndex f, int32_t c);
  // Internal helper function mostly for debugging.
  void PrintPosition(const Mesh &mesh, FaceIndex f, int32_t c);
  // Get the corner index of the lex smallest vertex of face f.
  static int32_t ComputeCornerIndexOfSmallestPointXYZ(const Mesh &mesh,
                                                      FaceIndex f);

  // Less compare functor for two faces (represented by their indices)
  // with respect to their lex order.
  struct FaceIndexLess {
    explicit FaceIndexLess(const MeshInfo &in_mesh_info)
        : mesh_info(in_mesh_info) {}
    bool operator()(FaceIndex f0, FaceIndex f1) const;
    const MeshInfo &mesh_info;
  };

  void InitCornerIndexOfSmallestPointXYZ();
  void InitOrderedFaceIndex();

  std::vector<MeshInfo> mesh_infos_;
  int32_t num_faces_;
};

}  // namespace draco

#endif  // DRACO_MESH_MESH_ARE_EQUIVALENT_H_
