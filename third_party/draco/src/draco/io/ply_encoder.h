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
#ifndef DRACO_IO_PLY_ENCODER_H_
#define DRACO_IO_PLY_ENCODER_H_

#include "draco/core/encoder_buffer.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Class for encoding draco::Mesh or draco::PointCloud into the PLY file format.
class PlyEncoder {
 public:
  PlyEncoder();

  // Encodes the mesh or a point cloud  and saves it into a file.
  // Returns false when either the encoding failed or when the file couldn't be
  // opened.
  bool EncodeToFile(const PointCloud &pc, const std::string &file_name);
  bool EncodeToFile(const Mesh &mesh, const std::string &file_name);

  // Encodes the mesh or the point cloud into a buffer.
  bool EncodeToBuffer(const PointCloud &pc, EncoderBuffer *out_buffer);
  bool EncodeToBuffer(const Mesh &mesh, EncoderBuffer *out_buffer);

 protected:
  bool EncodeInternal();
  EncoderBuffer *buffer() const { return out_buffer_; }
  bool ExitAndCleanup(bool return_value);

 private:
  const char *GetAttributeDataType(int attribute);

  EncoderBuffer *out_buffer_;

  const PointCloud *in_point_cloud_;
  const Mesh *in_mesh_;
};

}  // namespace draco

#endif  // DRACO_IO_PLY_ENCODER_H_
