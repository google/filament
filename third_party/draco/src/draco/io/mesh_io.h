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
#ifndef DRACO_MESH_MESH_IO_H_
#define DRACO_MESH_MESH_IO_H_

#include "draco/compression/config/compression_shared.h"
#include "draco/compression/decode.h"
#include "draco/compression/expert_encode.h"
#include "draco/core/options.h"

namespace draco {

template <typename OutStreamT>
OutStreamT WriteMeshIntoStream(const Mesh *mesh, OutStreamT &&os,
                               MeshEncoderMethod method,
                               const EncoderOptions &options) {
  EncoderBuffer buffer;
  EncoderOptions local_options = options;
  ExpertEncoder encoder(*mesh);
  encoder.Reset(local_options);
  encoder.SetEncodingMethod(method);
  if (!encoder.EncodeToBuffer(&buffer).ok()) {
    os.setstate(std::ios_base::badbit);
    return os;
  }

  os.write(static_cast<const char *>(buffer.data()), buffer.size());

  return os;
}

template <typename OutStreamT>
OutStreamT WriteMeshIntoStream(const Mesh *mesh, OutStreamT &&os,
                               MeshEncoderMethod method) {
  const EncoderOptions options = EncoderOptions::CreateDefaultOptions();
  return WriteMeshIntoStream(mesh, os, method, options);
}

template <typename OutStreamT>
OutStreamT &WriteMeshIntoStream(const Mesh *mesh, OutStreamT &&os) {
  return WriteMeshIntoStream(mesh, os, MESH_EDGEBREAKER_ENCODING);
}

template <typename InStreamT>
InStreamT &ReadMeshFromStream(std::unique_ptr<Mesh> *mesh, InStreamT &&is) {
  // Determine size of stream and write into a vector
  const auto start_pos = is.tellg();
  is.seekg(0, std::ios::end);
  const std::streampos is_size = is.tellg() - start_pos;
  is.seekg(start_pos);
  std::vector<char> data(is_size);
  is.read(&data[0], is_size);

  // Create a mesh from that data.
  DecoderBuffer buffer;
  buffer.Init(&data[0], data.size());
  Decoder decoder;
  auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
  *mesh = std::move(statusor).value();
  if (!statusor.ok() || *mesh == nullptr) {
    is.setstate(std::ios_base::badbit);
  }

  return is;
}

// Reads a mesh from a file. The function automatically chooses the correct
// decoder based on the extension of the files. Currently, .obj and .ply files
// are supported. Other file extensions are processed by the default
// draco::MeshDecoder.
// Returns nullptr with an error status if the decoding failed.
StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(const std::string &file_name);

// Reads a mesh from a file. The function does the same thing as the previous
// one except using metadata to encode additional information when
// |use_metadata| is set to true.
// Returns nullptr with an error status if the decoding failed.
StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(const std::string &file_name,
                                                 bool use_metadata);

// Reads a mesh from a file. Reading is configured with |options|:
// use_metadata  : Read obj file info like material names and object names into
// metadata. Default is false.
// The second form returns the files associated with the mesh via the
// |mesh_files| argument.
// Returns nullptr with an error status if the decoding failed.
StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(const std::string &file_name,
                                                 const Options &options);
StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(
    const std::string &file_name, const Options &options,
    std::vector<std::string> *mesh_files);

}  // namespace draco

#endif  // DRACO_MESH_MESH_IO_H_
