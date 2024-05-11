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
// The encoder compresses all attribute values using an order preserving
// attribute encoder (that can still support quantization, prediction schemes,
// and other features).
// The mesh connectivity data can be encoded using two modes that are controlled
// using a global encoder options flag called "compress_connectivity"
// 1. When "compress_connectivity" == true:
//      All point ids are first delta coded and then compressed using an entropy
//      coding.
// 2. When "compress_connectivity" == false:
//      All point ids are encoded directly using either 8, 16, or 32 bits per
//      value based on the maximum point id value.

#ifndef DRACO_COMPRESSION_MESH_MESH_SEQUENTIAL_ENCODER_H_
#define DRACO_COMPRESSION_MESH_MESH_SEQUENTIAL_ENCODER_H_

#include "draco/compression/mesh/mesh_encoder.h"

namespace draco {

// Class that encodes mesh data using a simple binary representation of mesh's
// connectivity and geometry.
// TODO(ostava): Use a better name.
class MeshSequentialEncoder : public MeshEncoder {
 public:
  MeshSequentialEncoder();
  uint8_t GetEncodingMethod() const override {
    return MESH_SEQUENTIAL_ENCODING;
  }

 protected:
  Status EncodeConnectivity() override;
  bool GenerateAttributesEncoder(int32_t att_id) override;
  void ComputeNumberOfEncodedPoints() override;
  void ComputeNumberOfEncodedFaces() override;

 private:
  // Returns false on error.
  bool CompressAndEncodeIndices();
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_SEQUENTIAL_ENCODER_H_
