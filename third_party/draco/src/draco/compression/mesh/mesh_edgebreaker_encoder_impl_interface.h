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
#ifndef DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_IMPL_INTERFACE_H_
#define DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_IMPL_INTERFACE_H_

#include "draco/compression/attributes/mesh_attribute_indices_encoding_data.h"
#include "draco/mesh/corner_table.h"
#include "draco/mesh/mesh_attribute_corner_table.h"

namespace draco {

// Forward declaration is necessary here to avoid circular dependencies.
class MeshEdgebreakerEncoder;

// Abstract interface used by MeshEdgebreakerEncoder to interact with the actual
// implementation of the edgebreaker method. The implementations are in general
// specializations of a template class MeshEdgebreakerEncoderImpl where the
// template arguments control encoding of the connectivity data. Because the
// choice of the implementation is done in run-time, we need to hide it behind
// the abstract interface MeshEdgebreakerEncoderImplInterface.
class MeshEdgebreakerEncoderImplInterface {
 public:
  virtual ~MeshEdgebreakerEncoderImplInterface() = default;
  virtual bool Init(MeshEdgebreakerEncoder *encoder) = 0;

  virtual const MeshAttributeCornerTable *GetAttributeCornerTable(
      int att_id) const = 0;
  virtual const MeshAttributeIndicesEncodingData *GetAttributeEncodingData(
      int att_id) const = 0;
  virtual bool GenerateAttributesEncoder(int32_t att_id) = 0;
  virtual bool EncodeAttributesEncoderIdentifier(int32_t att_encoder_id) = 0;
  virtual Status EncodeConnectivity() = 0;

  // Returns corner table of the encoded mesh.
  virtual const CornerTable *GetCornerTable() const = 0;

  // Returns true if a given face has been already encoded.
  virtual bool IsFaceEncoded(FaceIndex fi) const = 0;

  virtual MeshEdgebreakerEncoder *GetEncoder() const = 0;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_IMPL_INTERFACE_H_
