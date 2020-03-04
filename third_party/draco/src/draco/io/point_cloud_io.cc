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
#include "draco/io/point_cloud_io.h"

#include "draco/io/file_utils.h"
#include "draco/io/obj_decoder.h"
#include "draco/io/parser_utils.h"
#include "draco/io/ply_decoder.h"

namespace draco {

StatusOr<std::unique_ptr<PointCloud>> ReadPointCloudFromFile(
    const std::string &file_name) {
  std::unique_ptr<PointCloud> pc(new PointCloud());
  // Analyze file extension.
  const std::string extension = parser::ToLower(
      file_name.size() >= 4 ? file_name.substr(file_name.size() - 4)
                            : file_name);
  if (extension == ".obj") {
    // Wavefront OBJ file format.
    ObjDecoder obj_decoder;
    const Status obj_status = obj_decoder.DecodeFromFile(file_name, pc.get());
    if (!obj_status.ok()) {
      return obj_status;
    }
    return std::move(pc);
  }
  if (extension == ".ply") {
    // Wavefront PLY file format.
    PlyDecoder ply_decoder;
    DRACO_RETURN_IF_ERROR(ply_decoder.DecodeFromFile(file_name, pc.get()));
    return std::move(pc);
  }

  std::vector<char> buffer;
  if (!ReadFileToBuffer(file_name, &buffer)) {
    return Status(Status::DRACO_ERROR, "Unable to read input file.");
  }
  DecoderBuffer decoder_buffer;
  decoder_buffer.Init(buffer.data(), buffer.size());
  Decoder decoder;
  auto status_or = decoder.DecodePointCloudFromBuffer(&decoder_buffer);
  return std::move(status_or).value();
}

}  // namespace draco
