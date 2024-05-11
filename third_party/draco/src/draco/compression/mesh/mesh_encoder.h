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
#ifndef DRACO_COMPRESSION_MESH_MESH_ENCODER_H_
#define DRACO_COMPRESSION_MESH_MESH_ENCODER_H_

#include "draco/compression/attributes/mesh_attribute_indices_encoding_data.h"
#include "draco/compression/point_cloud/point_cloud_encoder.h"
#include "draco/mesh/mesh.h"
#include "draco/mesh/mesh_attribute_corner_table.h"

namespace draco {

// Abstract base class for all mesh encoders. It provides some basic
// functionality that's shared between different encoders.
class MeshEncoder : public PointCloudEncoder {
 public:
  MeshEncoder();

  // Sets the mesh that is going be encoded. Must be called before the Encode()
  // method.
  void SetMesh(const Mesh &m);

  EncodedGeometryType GetGeometryType() const override {
    return TRIANGULAR_MESH;
  }

  // Returns the number of faces that were encoded during the last Encode().
  // function call. Valid only if "store_number_of_encoded_faces" flag was set
  // in the provided EncoderOptions.
  size_t num_encoded_faces() const { return num_encoded_faces_; }

  // Returns the base connectivity of the encoded mesh (or nullptr if it is not
  // initialized).
  virtual const CornerTable *GetCornerTable() const { return nullptr; }

  // Returns the attribute connectivity data or nullptr if it does not exist.
  virtual const MeshAttributeCornerTable *GetAttributeCornerTable(
      int /* att_id */) const {
    return nullptr;
  }

  // Returns the encoding data for a given attribute or nullptr when the data
  // does not exist.
  virtual const MeshAttributeIndicesEncodingData *GetAttributeEncodingData(
      int /* att_id */) const {
    return nullptr;
  }

  const Mesh *mesh() const { return mesh_; }

 protected:
  Status EncodeGeometryData() override;

  // Needs to be implemented by the derived classes.
  virtual Status EncodeConnectivity() = 0;

  // Computes and sets the num_encoded_faces_ for the encoder.
  virtual void ComputeNumberOfEncodedFaces() = 0;

  void set_mesh(const Mesh *mesh) { mesh_ = mesh; }
  void set_num_encoded_faces(size_t num_faces) {
    num_encoded_faces_ = num_faces;
  }

 private:
  const Mesh *mesh_;
  size_t num_encoded_faces_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_ENCODER_H_
