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
#include "draco/scene/instance_array.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

#include <utility>

namespace draco {

void InstanceArray::Copy(const InstanceArray &other) {
  instances_.resize(other.instances_.size());
  for (int i = 0; i < instances_.size(); i++) {
    instances_[i].trs.Copy(other.instances_[i].trs);
  }
}

Status InstanceArray::AddInstance(const Instance &instance) {
  // Check that the |instance.trs| does not have the transformation matrix set,
  // because the EXT_mesh_gpu_instancing glTF extension dictates that only the
  // individual TRS vectors are stored.
  if (instance.trs.MatrixSet()) {
    return ErrorStatus("Instance must have no matrix set.");
  }

  // Move the |instance| to the end of the instances vector.
  instances_.push_back(instance);
  return OkStatus();
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
