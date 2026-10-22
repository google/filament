// Copyright 2019 The Draco Authors.
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
#include "draco/io/scene_io.h"

#include <string>

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/io/file_utils.h"
#include "draco/io/gltf_decoder.h"
#include "draco/io/gltf_encoder.h"
#include "draco/io/obj_encoder.h"
#include "draco/io/ply_encoder.h"

namespace draco {

enum SceneFileFormat { UNKNOWN, GLTF, USD, PLY, OBJ };

SceneFileFormat GetSceneFileFormat(const std::string &file_name) {
  const std::string extension = LowercaseFileExtension(file_name);
  if (extension == "gltf" || extension == "glb") {
    return GLTF;
  }
  if (extension == "usd" || extension == "usda" || extension == "usdc" ||
      extension == "usdz") {
    return USD;
  }
  if (extension == "obj") {
    return OBJ;
  }
  if (extension == "ply") {
    return PLY;
  }
  return UNKNOWN;
}

StatusOr<std::unique_ptr<Scene>> ReadSceneFromFile(
    const std::string &file_name) {
  return ReadSceneFromFile(file_name, nullptr);
}

StatusOr<std::unique_ptr<Scene>> ReadSceneFromFile(
    const std::string &file_name, std::vector<std::string> *scene_files) {
  std::unique_ptr<Scene> scene(new Scene());
  switch (GetSceneFileFormat(file_name)) {
    case GLTF: {
      GltfDecoder decoder;
      return decoder.DecodeFromFileToScene(file_name, scene_files);
    }
    case USD: {
      return Status(Status::DRACO_ERROR, "USD is not supported yet.");
    }
    default: {
      return Status(Status::DRACO_ERROR, "Unknown input file format.");
    }
  }
}

Status WriteSceneToFile(const std::string &file_name, const Scene &scene) {
  Options options;
  return WriteSceneToFile(file_name, scene, options);
}

Status WriteSceneToFile(const std::string &file_name, const Scene &scene,
                        const Options &options) {
  const std::string extension = LowercaseFileExtension(file_name);
  std::string folder_path;
  std::string out_file_name;
  draco::SplitPath(file_name, &folder_path, &out_file_name);
  const auto format = GetSceneFileFormat(file_name);
  switch (format) {
    case GLTF: {
      GltfEncoder encoder;
      if (!encoder.EncodeToFile(scene, file_name, folder_path)) {
        return Status(Status::DRACO_ERROR, "Failed to encode the scene.");
      }
      return OkStatus();
    }
    case USD: {
      return Status(Status::DRACO_ERROR, "USD is not supported yet.");
    }
    case PLY:
    case OBJ: {
      // Convert the scene to mesh and save the scene as a mesh. For now we do
      // that by converting the scene to GLB and decoding the GLB into a mesh.
      GltfEncoder gltf_encoder;
      EncoderBuffer buffer;
      DRACO_RETURN_IF_ERROR(gltf_encoder.EncodeToBuffer(scene, &buffer));
      GltfDecoder gltf_decoder;
      DecoderBuffer dec_buffer;
      dec_buffer.Init(buffer.data(), buffer.size());
      DRACO_ASSIGN_OR_RETURN(auto mesh,
                             gltf_decoder.DecodeFromBuffer(&dec_buffer));
      if (format == PLY) {
        PlyEncoder ply_encoder;
        if (!ply_encoder.EncodeToFile(*mesh, file_name)) {
          return ErrorStatus("Failed to encode the scene as PLY.");
        }
      }
      if (format == OBJ) {
        ObjEncoder obj_encoder;
        if (!obj_encoder.EncodeToFile(*mesh, file_name)) {
          return ErrorStatus("Failed to encode the scene as OBJ.");
        }
      }
      return OkStatus();
    }
    default: {
      return Status(Status::DRACO_ERROR, "Unknown output file format.");
    }
  }
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
