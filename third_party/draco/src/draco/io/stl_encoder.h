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
#ifndef DRACO_IO_STL_ENCODER_H_
#define DRACO_IO_STL_ENCODER_H_

#include <string>

#include "draco/core/encoder_buffer.h"
#include "draco/draco_features.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Class for encoding draco::Mesh into the STL file format.
class StlEncoder {
 public:
  StlEncoder();

  // Encodes the mesh and saves it into a file.
  // Returns false when either the encoding failed or when the file couldn't be
  // opened.
  Status EncodeToFile(const Mesh &mesh, const std::string &file_name);

  // Encodes the mesh into a buffer.
  Status EncodeToBuffer(const Mesh &mesh, EncoderBuffer *out_buffer);

 protected:
  Status EncodeInternal();
  EncoderBuffer *buffer() const { return out_buffer_; }

 private:
  EncoderBuffer *out_buffer_;

  const PointCloud *in_point_cloud_;
  const Mesh *in_mesh_;
};

}  // namespace draco

#endif  // DRACO_IO_STL_ENCODER_H_
