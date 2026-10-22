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
#include "draco/compression/mesh/mesh_encoder.h"

namespace draco {

MeshEncoder::MeshEncoder() : mesh_(nullptr), num_encoded_faces_(0) {}

void MeshEncoder::SetMesh(const Mesh &m) {
  mesh_ = &m;
  SetPointCloud(m);
}

Status MeshEncoder::EncodeGeometryData() {
  DRACO_RETURN_IF_ERROR(EncodeConnectivity());
  if (options()->GetGlobalBool("store_number_of_encoded_faces", false)) {
    ComputeNumberOfEncodedFaces();
  }
  return OkStatus();
}

}  // namespace draco
