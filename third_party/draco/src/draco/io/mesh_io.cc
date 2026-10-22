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
#include "draco/io/file_writer_interface.h"
#include "draco/io/obj_decoder.h"
#include "draco/io/ply_decoder.h"
#include "draco/io/stl_decoder.h"
#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/compression/draco_compression_options.h"
#include "draco/compression/encode.h"
#include "draco/io/gltf_decoder.h"
#include "draco/io/gltf_encoder.h"
#include "draco/io/obj_encoder.h"
#include "draco/io/ply_encoder.h"
#endif

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
  if (extension != "gltf" && extension != "obj" && mesh_files) {
    // The GLTF/OBJ decoder will fill |mesh_files|, but for other file types we
    // set the root file here to avoid duplicating code.
    mesh_files->push_back(file_name);
  }
  if (extension == "obj") {
    // Wavefront OBJ file format.
    ObjDecoder obj_decoder;
    obj_decoder.set_use_metadata(options.GetBool("use_metadata", false));
    obj_decoder.set_preserve_polygons(options.GetBool("preserve_polygons"));
    const Status obj_status =
        obj_decoder.DecodeFromFile(file_name, mesh.get(), mesh_files);
    if (!obj_status.ok()) {
      return obj_status;
    }
    return std::move(mesh);
  }
  if (extension == "ply") {
    // Stanford PLY file format.
    PlyDecoder ply_decoder;
    DRACO_RETURN_IF_ERROR(ply_decoder.DecodeFromFile(file_name, mesh.get()));
    return std::move(mesh);
  }
  if (extension == "stl") {
    // STL file format.
    StlDecoder stl_decoder;
    return stl_decoder.DecodeFromFile(file_name);
  }
#ifdef DRACO_TRANSCODER_SUPPORTED
  if (extension == "gltf" || extension == "glb") {
    GltfDecoder gltf_decoder;
    return gltf_decoder.DecodeFromFile(file_name, mesh_files);
  }
#endif

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
