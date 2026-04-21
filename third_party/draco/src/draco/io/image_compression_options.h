// Copyright 2020 The Draco Authors.
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
#ifndef DRACO_IO_IMAGE_COMPRESSION_OPTIONS_H_
#define DRACO_IO_IMAGE_COMPRESSION_OPTIONS_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <string>

namespace draco {

// Enum defining image compression formats.
enum class ImageFormat {
  NONE,
  PNG,
  JPEG,
  BASIS,
  WEBP,
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_IO_IMAGE_COMPRESSION_OPTIONS_H_
