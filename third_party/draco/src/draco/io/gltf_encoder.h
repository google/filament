// Copyright 2018 The Draco Authors.
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
#ifndef DRACO_IO_GLTF_ENCODER_H_
#define DRACO_IO_GLTF_ENCODER_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "draco/core/encoder_buffer.h"
#include "draco/io/file_writer_factory.h"
#include "draco/io/file_writer_interface.h"
#include "draco/io/texture_io.h"
#include "draco/mesh/mesh.h"
#include "draco/scene/scene.h"

namespace draco {

// Class for encoding draco::Mesh into the glTF file format.
class GltfEncoder {
 public:
  // Types of output modes for the glTF data encoder. |COMPACT| will output
  // required and non-default glTF data. |VERBOSE| will output required and
  // default glTF data as well as readable JSON even when the output is saved in
  // a glTF-Binary file.
  enum OutputType { COMPACT, VERBOSE };

  GltfEncoder();

  // Encodes the geometry and saves it into a file. Returns false when either
  // the encoding failed or when the file couldn't be opened.
  template <typename T>
  bool EncodeToFile(const T &geometry, const std::string &file_name,
                    const std::string &base_dir);

  // Saves |geometry| into glTF 2.0 format. |filename| is the name of the
  // glTF file. The glTF bin file (if needed) will be named stem(|filename|) +
  // “.bin”. The other files (if needed) will be saved to basedir(|filename|).
  // If |filename| has the extension "glb" then |filename| will be written as a
  // glTF-Binary file. Otherwise |filename| will be written as non-binary glTF
  // file.
  template <typename T>
  Status EncodeFile(const T &geometry, const std::string &filename);

  // Saves |geometry| into glTF 2.0 format. |filename| is the name of the
  // glTF file. |bin_filename| is the name of the glTF bin file. The other
  // files (if needed) will be saved to basedir(|filename|). |bin_filename| will
  // be ignored if output is glTF-Binary.
  template <typename T>
  Status EncodeFile(const T &geometry, const std::string &filename,
                    const std::string &bin_filename);

  // Saves |geometry| into glTF 2.0 format. |filename| is the name of the
  // glTF file. |bin_filename| is the name of the glTF bin file. The other
  // files will be saved to |resource_dir|. |bin_filename| and |resource_dir|
  // will be ignored if output is glTF-Binary.
  template <typename T>
  Status EncodeFile(const T &geometry, const std::string &filename,
                    const std::string &bin_filename,
                    const std::string &resource_dir);

  // Encodes |geometry| to |out_buffer| in glTF 2.0 GLB format.
  template <typename T>
  Status EncodeToBuffer(const T &geometry, EncoderBuffer *out_buffer);

  void set_output_type(OutputType type) { output_type_ = type; }
  OutputType output_type() const { return output_type_; }

  void set_copyright(const std::string &copyright) { copyright_ = copyright; }
  std::string copyright() const { return copyright_; }

  // The name of the attribute metadata that contains the glTF attribute
  // name. For application-specific generic attributes, if the metadata for
  // an attribute contains this key, then the value will be used as the
  // encoded attribute name in the output GLTF.
  static const char kDracoMetadataGltfAttributeName[];

 private:
  // Encodes the mesh or the point cloud into a buffer.
  Status EncodeToBuffer(const Mesh &mesh, class GltfAsset *gltf_asset,
                        EncoderBuffer *out_buffer);
  Status EncodeToBuffer(const Scene &scene, class GltfAsset *gltf_asset,
                        EncoderBuffer *out_buffer);

  // Sets appropriate Json writer mode based on the provided |gltf_asset|
  // options.
  static void SetJsonWriterMode(class GltfAsset *gltf_asset);

  // Writes the ".gltf" and associted files. |gltf_asset| holds the glTF data.
  // |buffer| is the encoded glTF json data. |filename| is the name of the
  // ".gltf" file. |bin_filename| is the name of the glTF bin file. The other
  // files will be saved to |resource_dir|.
  Status WriteGltfFiles(const class GltfAsset &gltf_asset,
                        const EncoderBuffer &buffer,
                        const std::string &filename,
                        const std::string &bin_filename,
                        const std::string &resource_dir);

  // Writes the ".glb" file. |gltf_asset| holds the glTF data. |json_data| is
  // the encoded glTF json data. |filename| is the name of the ".glb" file.
  Status WriteGlbFile(const class GltfAsset &gltf_asset,
                      const EncoderBuffer &json_data,
                      const std::string &filename);

  // Creates GLB file chunks and passes them to |process_chunk| function for
  // processing. |gltf_asset| holds the glTF data. |json_data| is the encoded
  // glTF json data.
  Status ProcessGlbFileChunks(
      const class GltfAsset &gltf_asset, const EncoderBuffer &json_data,
      const std::function<Status(const EncoderBuffer &)> &process_chunk) const;

  EncoderBuffer *out_buffer_;
  OutputType output_type_;
  std::string copyright_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_IO_GLTF_ENCODER_H_
