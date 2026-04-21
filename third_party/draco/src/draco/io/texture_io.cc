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
#include "draco/io/texture_io.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

#include <algorithm>
#include <cstring>

#include "draco/io/file_utils.h"
#include "draco/texture/texture_utils.h"

namespace draco {

namespace {

StatusOr<std::unique_ptr<Texture>> CreateDracoTextureInternal(
    const std::vector<uint8_t> &image_data, SourceImage *out_source_image) {
  std::unique_ptr<Texture> draco_texture(new Texture());
  const auto format =
      ImageFormatFromBuffer(image_data.data(), image_data.size());
  out_source_image->MutableEncodedData() = image_data;
  out_source_image->set_mime_type(TextureUtils::GetMimeType(format));
  return std::move(draco_texture);
}

}  // namespace

ImageFormat ImageFormatFromBuffer(const uint8_t *buffer, size_t buffer_size) {
  if (buffer_size > 4) {
    // These bytes are the Start of Image (SOI) and End of Image (EOI) markers
    // in a JPEG data stream.
    const std::array<uint8_t, 2> kJpegSOIMarker = {0xFF, 0xD8};
    const std::array<uint8_t, 2> kJpegEOIMarker = {0xFF, 0xD9};

    if (!memcmp(buffer, kJpegSOIMarker.data(), kJpegSOIMarker.size())) {
      // Look for the last occurence of the end marker (allow trailing bytes).
      if (std::find_end(buffer, buffer + buffer_size, kJpegEOIMarker.begin(),
                        kJpegEOIMarker.end()) != buffer + buffer_size) {
        return ImageFormat::JPEG;
      }
    }
  }

  if (buffer_size > 2) {
    // For Binomial Basis format input the stream always begins with
    // the signature 'B' * 256 + 's', or 0x4273.
    const std::array<uint8_t, 2> kBasisSignature = {0x42, 0x73};
    if (!memcmp(buffer, kBasisSignature.data(), kBasisSignature.size())) {
      return ImageFormat::BASIS;
    }
  }

  if (buffer_size > 4) {
    // For Binomial Basis/KTX2 format input the stream begins with 0xab 0x4b
    // 0x54 0x58.
    const std::array<uint8_t, 4> kKtx2Signature = {0xab, 0x4b, 0x54, 0x58};
    if (!memcmp(buffer, kKtx2Signature.data(), kKtx2Signature.size())) {
      return ImageFormat::BASIS;
    }
  }

  if (buffer_size > 8) {
    // The first eight bytes of a PNG stream always contain these values:
    const std::array<uint8_t, 8> kPngSignature = {0x89, 0x50, 0x4e, 0x47,
                                                  0x0d, 0x0a, 0x1a, 0x0a};
    if (!memcmp(buffer, kPngSignature.data(), kPngSignature.size())) {
      return ImageFormat::PNG;
    }
  }

  if (buffer_size > 12) {
    // The WebP signature bytes are: RIFF 0 0 0 0 WEBP. The 0's are where WebP
    // size information is encoded in the stream, but the check here just looks
    // for RIFF and WEBP.
    const std::array<uint8_t, 4> kRIFF = {0x52, 0x49, 0x46, 0x46};
    const std::array<uint8_t, 4> kWEBP = {0x57, 0x45, 0x42, 0x50};

    if (!memcmp(buffer, kRIFF.data(), kRIFF.size()) &&
        !memcmp(buffer + 8, kWEBP.data(), kWEBP.size())) {
      return ImageFormat::WEBP;
    }
  }

  return ImageFormat::NONE;
}

StatusOr<std::unique_ptr<Texture>> ReadTextureFromFile(
    const std::string &file_name) {
  std::vector<uint8_t> image_data;
  if (!ReadFileToBuffer(file_name, &image_data)) {
    return Status(Status::IO_ERROR, "Unable to read input texture file.");
  }

  SourceImage source_image;
  DRACO_ASSIGN_OR_RETURN(auto texture,
                         CreateDracoTextureInternal(image_data, &source_image));
  source_image.set_filename(file_name);
  if (source_image.mime_type().empty()) {
    // Try to set mime type from extension if we were not able to detect it
    // automatically.
    const std::string extension = LowercaseFileExtension(file_name);
    const std::string mime_type =
        "image/" + (extension == "jpg" ? "jpeg" : extension);
    source_image.set_mime_type(mime_type);
  }
  texture->set_source_image(source_image);
  return texture;
}

StatusOr<std::unique_ptr<Texture>> ReadTextureFromBuffer(const uint8_t *buffer,
                                                         size_t buffer_size) {
  SourceImage source_image;
  std::vector<uint8_t> image_data(buffer, buffer + buffer_size);
  DRACO_ASSIGN_OR_RETURN(auto texture,
                         CreateDracoTextureInternal(image_data, &source_image));
  texture->set_source_image(source_image);
  return texture;
}

StatusOr<std::unique_ptr<Texture>> ReadTextureFromBuffer(
    const uint8_t *buffer, size_t buffer_size, const std::string &mime_type) {
  return ReadTextureFromBuffer(buffer, buffer_size);
}

Status WriteTextureToFile(const std::string &file_name,
                          const Texture &texture) {
  std::vector<uint8_t> buffer;
  DRACO_RETURN_IF_ERROR(WriteTextureToBuffer(texture, &buffer));

  if (!WriteBufferToFile(buffer.data(), buffer.size(), file_name)) {
    return Status(Status::DRACO_ERROR, "Failed to write image.");
  }

  return OkStatus();
}

Status WriteTextureToBuffer(const Texture &texture,
                            std::vector<uint8_t> *buffer) {
  // Copy data from the encoded source image if possible, otherwise load the
  // data from the source file.
  if (!texture.source_image().encoded_data().empty()) {
    *buffer = texture.source_image().encoded_data();
  } else if (!texture.source_image().filename().empty()) {
    if (!ReadFileToBuffer(texture.source_image().filename(), buffer)) {
      return Status(Status::IO_ERROR, "Unable to read input texture file.");
    }
  } else {
    return Status(Status::DRACO_ERROR, "Invalid source data for the texture.");
  }
  return OkStatus();
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
