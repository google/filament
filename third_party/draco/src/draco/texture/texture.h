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
#ifndef DRACO_TEXTURE_TEXTURE_H_
#define DRACO_TEXTURE_TEXTURE_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>
#include <vector>

#include "draco/io/image_compression_options.h"
#include "draco/texture/source_image.h"

namespace draco {

// Texture class storing the source image data.
class Texture {
 public:
  void Copy(const Texture &other) { source_image_.Copy(other.source_image_); }

  void set_source_image(const SourceImage &image) { source_image_.Copy(image); }
  const SourceImage &source_image() const { return source_image_; }
  SourceImage &source_image() { return source_image_; }

 private:
  // If set this is the image that this texture is based from.
  SourceImage source_image_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_TEXTURE_TEXTURE_H_
