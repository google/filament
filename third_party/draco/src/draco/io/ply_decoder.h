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
#ifndef DRACO_IO_PLY_DECODER_H_
#define DRACO_IO_PLY_DECODER_H_

#include <string>

#include "draco/core/decoder_buffer.h"
#include "draco/core/status.h"
#include "draco/draco_features.h"
#include "draco/io/ply_reader.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Decodes a PLY file into draco::Mesh (or draco::PointCloud if the
// connectivity data is not needed).
// TODO(b/34330853): The current implementation assumes that the input vertices
// are defined with x, y, z properties. The decoder also reads uint8 red, green,
// blue, alpha color information, float32 defined as nx, ny, nz properties, but
// all other attributes are ignored for now.
class PlyDecoder {
 public:
  PlyDecoder();

  // Decodes an obj file stored in the input file.
  Status DecodeFromFile(const std::string &file_name, Mesh *out_mesh);
  Status DecodeFromFile(const std::string &file_name,
                        PointCloud *out_point_cloud);

  Status DecodeFromBuffer(DecoderBuffer *buffer, Mesh *out_mesh);
  Status DecodeFromBuffer(DecoderBuffer *buffer, PointCloud *out_point_cloud);

 protected:
  Status DecodeInternal();
  DecoderBuffer *buffer() { return &buffer_; }

 private:
  Status DecodeFaceData(const PlyElement *face_element);
  Status DecodeVertexData(const PlyElement *vertex_element);

  template <typename DataTypeT>
  bool ReadPropertiesToAttribute(
      const std::vector<const PlyProperty *> &properties,
      PointAttribute *attribute, int num_vertices);

  DecoderBuffer buffer_;

  // Data structure that stores the decoded data. |out_point_cloud_| must be
  // always set but |out_mesh_| is optional.
  Mesh *out_mesh_;
  PointCloud *out_point_cloud_;
};

}  // namespace draco

#endif  // DRACO_IO_PLY_DECODER_H_
