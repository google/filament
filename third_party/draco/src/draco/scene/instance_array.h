// Copyright 2022 The Draco Authors.
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
#ifndef DRACO_SCENE_INSTANCE_ARRAY_H_
#define DRACO_SCENE_INSTANCE_ARRAY_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <string>
#include <vector>

#include "draco/core/vector_d.h"
#include "draco/scene/trs_matrix.h"

namespace draco {

// Describes a mesh group instancing array that includes TRS transformation
// for multiple instance positions and possibly other custom instance attributes
// (not yet supported), following the EXT_mesh_gpu_instancing glTF extension.
class InstanceArray {
 public:
  struct Instance {
    // Translation, rotation, and scale vectors.
    TrsMatrix trs;
    // TODO(vytyaz): Support custom instance attributes, e.g., _ID, _COLOR, etc.
  };

  InstanceArray() = default;

  void Copy(const InstanceArray &other);

  // Adds one |instance| into this mesh group instance array where the
  // |instance.trs| may have optional translation, rotation, and scale set.
  Status AddInstance(const Instance &instance);

  // Returns the count of instances in this mesh group instance array.
  int NumInstances() const { return instances_.size(); }

  // Returns an instance from this mesh group instance array.
  const Instance &GetInstance(int index) const { return instances_[index]; }

 private:
  std::vector<Instance> instances_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_SCENE_INSTANCE_ARRAY_H_
