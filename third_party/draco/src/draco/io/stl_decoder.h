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
#ifndef DRACO_IO_STL_DECODER_H_
#define DRACO_IO_STL_DECODER_H_

#include <string>

#include "draco/core/decoder_buffer.h"
#include "draco/core/status.h"
#include "draco/core/status_or.h"
#include "draco/draco_features.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Decodes an STL file into draco::Mesh (or draco::PointCloud if the
// connectivity data is not needed).
class StlDecoder {
 public:
  StatusOr<std::unique_ptr<Mesh>> DecodeFromFile(const std::string &file_name);
  StatusOr<std::unique_ptr<Mesh>> DecodeFromBuffer(DecoderBuffer *buffer);
};

}  // namespace draco

#endif  // DRACO_IO_STL_DECODER_H_
