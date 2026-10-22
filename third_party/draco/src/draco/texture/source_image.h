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
#ifndef DRACO_TEXTURE_SOURCE_IMAGE_H_
#define DRACO_TEXTURE_SOURCE_IMAGE_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <cstdint>
#include <string>
#include <vector>

#include "draco/core/status.h"

namespace draco {

// This class is used to hold the encoded and decoded data and characteristics
// for an image. In order for the image to contain "valid" encoded data, either
// the |filename_| must point to a valid image file or the |mime_type_| and
// |encoded_data_| must contain valid image data.
class SourceImage {
 public:
  SourceImage() {}

  // No copy constructors.
  SourceImage(const SourceImage &) = delete;
  SourceImage &operator=(const SourceImage &) = delete;
  // No move constructors.
  SourceImage(SourceImage &&) = delete;
  SourceImage &operator=(SourceImage &&) = delete;

  void Copy(const SourceImage &src);

  // Sets the name of the source image file.
  void set_filename(const std::string &filename) { filename_ = filename; }
  const std::string &filename() const { return filename_; }

  void set_mime_type(const std::string &mime_type) { mime_type_ = mime_type; }
  const std::string &mime_type() const { return mime_type_; }

  std::vector<uint8_t> &MutableEncodedData() { return encoded_data_; }
  const std::vector<uint8_t> &encoded_data() const { return encoded_data_; }

 private:
  // The filename of the image. This field can be empty as long as |mime_type_|
  // and |encoded_data_| is not empty.
  std::string filename_;

  // The mimetype of the |encoded_data_|.
  std::string mime_type_;

  // The encoded data of the image. This field can be empty as long as
  // |filename_| is not empty.
  std::vector<uint8_t> encoded_data_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_TEXTURE_SOURCE_IMAGE_H_
