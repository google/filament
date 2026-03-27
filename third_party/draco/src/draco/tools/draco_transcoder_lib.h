// Copyright 2021 The Draco Authors.
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
#ifndef DRACO_TOOLS_DRACO_TRANSCODER_LIB_H_
#define DRACO_TOOLS_DRACO_TRANSCODER_LIB_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <string>

#include "draco/compression/draco_compression_options.h"
#include "draco/core/options.h"
#include "draco/io/gltf_encoder.h"
#include "draco/io/image_compression_options.h"

namespace draco {

// Struct to hold Draco transcoding options.
struct DracoTranscodingOptions {
  DracoTranscodingOptions() {}

  // Options used when geometry compression optimization is disabled.
  DracoCompressionOptions geometry;
};

// Class that supports input of glTF (and some simple USD) files, encodes
// them with Draco compression, and outputs glTF Draco compressed files.
//
// glTF supported extensions:
//  Input and Output:
//    KHR_draco_mesh_compression. http://shortn/_L5tPQqdwWf
//    KHR_materials_unlit. http://shortn/_3eaDLoIGam
//    KHR_texture_transform. http://shortn/_PORWgVTEe8
//
// glTF unsupported features:
//  Input and Output:
//    Morph targets. http://shortn/_zE5DLw8a9B
//    Sparse accessors. http://shortn/_h3FwbzQl4f
//    KHR_lights_punctual. http://shortn/_nzGk80wKtK
//    KHR_materials_pbrSpecularGlossiness. http://shortn/_iz0VC6dIKe
//    All vendor extensions.
class DracoTranscoder {
 public:
  struct FileOptions {
    std::string input_filename;   // Must be non-empty.
    std::string output_filename;  // Must be non-empty.
    std::string output_bin_filename = "";
    std::string output_resource_directory = "";
  };

  DracoTranscoder();

  // Creates a DracoTranscoder object. |options| sets the compression options
  // used in the Encode function.
  static StatusOr<std::unique_ptr<DracoTranscoder>> Create(
      const DracoTranscodingOptions &options);

  // Deprecated.
  // TODO(fgalligan): Remove when function is not being used anymore.
  static StatusOr<std::unique_ptr<DracoTranscoder>> Create(
      const DracoCompressionOptions &options);

  // Encodes the input with Draco compression using the compression options
  // passed in the Create function. The recommended use case is to create a
  // transcoder once and call Transcode for multiple files.
  Status Transcode(const FileOptions &file_options);

 private:
  // Read scene from file.
  Status ReadScene(const FileOptions &file_options);

  // Write scene to file.
  Status WriteScene(const FileOptions &file_options);

  // Apply compression settings to the scene.
  Status CompressScene();

 private:
  GltfEncoder gltf_encoder_;

  // The scene being transcoded.
  std::unique_ptr<Scene> scene_;

  // Copy of the transcoding options passed into the Create function.
  DracoTranscodingOptions transcoding_options_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_TOOLS_DRACO_TRANSCODER_LIB_H_
