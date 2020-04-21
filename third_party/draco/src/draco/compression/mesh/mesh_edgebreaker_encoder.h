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
#ifndef DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_H_
#define DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_H_

#include <unordered_map>

#include "draco/compression/mesh/mesh_edgebreaker_encoder_impl_interface.h"
#include "draco/compression/mesh/mesh_edgebreaker_shared.h"
#include "draco/compression/mesh/mesh_encoder.h"
#include "draco/mesh/corner_table.h"

namespace draco {

// Class implements the edge breaker geometry compression method as described
// in "3D Compression Made Simple: Edgebreaker on a Corner-Table" by Rossignac
// at al.'01. http://www.cc.gatech.edu/~jarek/papers/CornerTableSMI.pdf
class MeshEdgebreakerEncoder : public MeshEncoder {
 public:
  MeshEdgebreakerEncoder();

  const CornerTable *GetCornerTable() const override {
    return impl_->GetCornerTable();
  }

  const MeshAttributeCornerTable *GetAttributeCornerTable(
      int att_id) const override {
    return impl_->GetAttributeCornerTable(att_id);
  }

  const MeshAttributeIndicesEncodingData *GetAttributeEncodingData(
      int att_id) const override {
    return impl_->GetAttributeEncodingData(att_id);
  }

  uint8_t GetEncodingMethod() const override {
    return MESH_EDGEBREAKER_ENCODING;
  }

 protected:
  bool InitializeEncoder() override;
  Status EncodeConnectivity() override;
  bool GenerateAttributesEncoder(int32_t att_id) override;
  bool EncodeAttributesEncoderIdentifier(int32_t att_encoder_id) override;
  void ComputeNumberOfEncodedPoints() override;
  void ComputeNumberOfEncodedFaces() override;

 private:
  // The actual implementation of the edge breaker method. The implementations
  // are in general specializations of a template class
  // MeshEdgebreakerEncoderImpl where the template arguments control encoding
  // of the connectivity data. The actual implementation is selected in this
  // class based on the provided encoding options. Because this choice is done
  // in run-time, the actual implementation has to be hidden behind the
  // abstract interface MeshEdgebreakerEncoderImplInterface.
  std::unique_ptr<MeshEdgebreakerEncoderImplInterface> impl_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_H_
