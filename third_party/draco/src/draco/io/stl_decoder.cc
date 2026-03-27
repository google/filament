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
#include "draco/io/stl_decoder.h"

#include <string>

#include "draco/core/macros.h"
#include "draco/core/status.h"
#include "draco/core/status_or.h"
#include "draco/io/file_utils.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"

namespace draco {

StatusOr<std::unique_ptr<Mesh>> StlDecoder::DecodeFromFile(
    const std::string &file_name) {
  std::vector<char> data;
  if (!ReadFileToBuffer(file_name, &data)) {
    return Status(Status::IO_ERROR, "Unable to read input file.");
  }
  DecoderBuffer buffer;
  buffer.Init(data.data(), data.size());
  return DecodeFromBuffer(&buffer);
}

StatusOr<std::unique_ptr<Mesh>> StlDecoder::DecodeFromBuffer(
    DecoderBuffer *buffer) {
  if (!strncmp(buffer->data_head(), "solid ", 6)) {
    return Status(Status::IO_ERROR,
                  "Currently only binary STL files are supported.");
  }
  buffer->Advance(80);
  uint32_t face_count;
  buffer->Decode(&face_count, 4);

  TriangleSoupMeshBuilder builder;
  builder.Start(face_count);

  const int32_t pos_att_id =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  const int32_t norm_att_id =
      builder.AddAttribute(GeometryAttribute::NORMAL, 3, DT_FLOAT32);

  for (uint32_t i = 0; i < face_count; i++) {
    float data[48];
    buffer->Decode(data, 48);
    uint16_t unused;
    buffer->Decode(&unused, 2);

    builder.SetPerFaceAttributeValueForFace(
        norm_att_id, draco::FaceIndex(i),
        draco::Vector3f(data[0], data[1], data[2]).data());

    builder.SetAttributeValuesForFace(
        pos_att_id, draco::FaceIndex(i),
        draco::Vector3f(data[3], data[4], data[5]).data(),
        draco::Vector3f(data[6], data[7], data[8]).data(),
        draco::Vector3f(data[9], data[10], data[11]).data());
  }

  std::unique_ptr<Mesh> mesh = builder.Finalize();
  return mesh;
}

}  // namespace draco
