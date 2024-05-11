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
#include "draco/io/mesh_io.h"

#include <fstream>
#include <string>

#include "draco/io/file_utils.h"
#include "draco/io/obj_decoder.h"
#include "draco/io/ply_decoder.h"

namespace draco {

StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(const std::string &file_name) {
  const Options options;
  return ReadMeshFromFile(file_name, options, nullptr);
}

StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(const std::string &file_name,
                                                 bool use_metadata) {
  Options options;
  options.SetBool("use_metadata", use_metadata);
  return ReadMeshFromFile(file_name, options, nullptr);
}

StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(const std::string &file_name,
                                                 const Options &options) {
  return ReadMeshFromFile(file_name, options, nullptr);
}

StatusOr<std::unique_ptr<Mesh>> ReadMeshFromFile(
    const std::string &file_name, const Options &options,
    std::vector<std::string> *mesh_files) {
  std::unique_ptr<Mesh> mesh(new Mesh());
  // Analyze file extension.
  const std::string extension = LowercaseFileExtension(file_name);
  if (extension != "gltf" && mesh_files) {
    // The GLTF decoder will fill |mesh_files|, but for other file types we set
    // the root file here to avoid duplicating code.
    mesh_files->push_back(file_name);
  }
  if (extension == "obj") {
    // Wavefront OBJ file format.
    ObjDecoder obj_decoder;
    obj_decoder.set_use_metadata(options.GetBool("use_metadata", false));
    const Status obj_status = obj_decoder.DecodeFromFile(file_name, mesh.get());
    if (!obj_status.ok()) {
      return obj_status;
    }
    return std::move(mesh);
  }
  if (extension == "ply") {
    // Wavefront PLY file format.
    PlyDecoder ply_decoder;
    DRACO_RETURN_IF_ERROR(ply_decoder.DecodeFromFile(file_name, mesh.get()));
    return std::move(mesh);
  }

  // Otherwise not an obj file. Assume the file was encoded with one of the
  // draco encoding methods.
  std::vector<char> file_data;
  if (!ReadFileToBuffer(file_name, &file_data)) {
    return Status(Status::DRACO_ERROR, "Unable to read input file.");
  }
  DecoderBuffer buffer;
  buffer.Init(file_data.data(), file_data.size());
  Decoder decoder;
  auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
  if (!statusor.ok() || statusor.value() == nullptr) {
    return Status(Status::DRACO_ERROR, "Error decoding input.");
  }
  return std::move(statusor).value();
}

}  // namespace draco
