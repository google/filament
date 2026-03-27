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
#ifndef DRACO_IO_TEXTURE_IO_H_
#define DRACO_IO_TEXTURE_IO_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>
#include <string>
#include <vector>

#include "draco/core/draco_types.h"
#include "draco/core/status_or.h"
#include "draco/texture/texture.h"

namespace draco {

// Reads a texture from a file. Reads PNG, JPEG and WEBP texture files.
// Returns nullptr with an error status if the decoding failed.
StatusOr<std::unique_ptr<Texture>> ReadTextureFromFile(
    const std::string &file_name);

// Same as ReadTextureFromFile() but the texture data is parsed from a |buffer|.
StatusOr<std::unique_ptr<Texture>> ReadTextureFromBuffer(const uint8_t *buffer,
                                                         size_t buffer_size);
// Deprecated: |mime_type| is currently ignored and it is deducted automatically
// from the content of the |buffer|.
StatusOr<std::unique_ptr<Texture>> ReadTextureFromBuffer(
    const uint8_t *buffer, size_t buffer_size, const std::string &mime_type);

// Writes a texture into a file. Can write PNG, JPEG, WEBP, and KTX2 (with Basis
// compression) texture files depending on the extension specified in
// |file_name| and image format specified in |texture|. Note that images with
// Basis compression can only be saved to files in KTX2 format and not to files
// with "basis" extension. Returns an error status if the writing failed.
Status WriteTextureToFile(const std::string &file_name, const Texture &texture);

// Writes a |texture| into |buffer|.
Status WriteTextureToBuffer(const Texture &texture,
                            std::vector<uint8_t> *buffer);

// Returns the image format of an encoded texture stored in |buffer|.
// ImageFormat::NONE is returned for unknown image formats.
ImageFormat ImageFormatFromBuffer(const uint8_t *buffer, size_t buffer_size);

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_IO_TEXTURE_IO_H_
