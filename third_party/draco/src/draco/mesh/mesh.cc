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
#include "draco/mesh/mesh.h"

#include <array>

namespace draco {

// Shortcut for typed conditionals.
template <bool B, class T, class F>
using conditional_t = typename std::conditional<B, T, F>::type;

Mesh::Mesh() {}

#ifdef DRACO_ATTRIBUTE_INDICES_DEDUPLICATION_SUPPORTED
void Mesh::ApplyPointIdDeduplication(
    const IndexTypeVector<PointIndex, PointIndex> &id_map,
    const std::vector<PointIndex> &unique_point_ids) {
  PointCloud::ApplyPointIdDeduplication(id_map, unique_point_ids);
  for (FaceIndex f(0); f < num_faces(); ++f) {
    for (int32_t c = 0; c < 3; ++c) {
      faces_[f][c] = id_map[faces_[f][c]];
    }
  }
}
#endif

}  // namespace draco
