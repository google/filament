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
#ifndef DRACO_TEXTURE_TEXTURE_UTILS_H_
#define DRACO_TEXTURE_TEXTURE_UTILS_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/core/status.h"
#include "draco/core/status_or.h"
#include "draco/io/file_utils.h"
#include "draco/material/material_library.h"
#include "draco/texture/texture_library.h"
#include "draco/texture/texture_map.h"

namespace draco {

// Helper class implementing various utilities operating on draco::Texture.
class TextureUtils {
 public:
  // Returns |texture| image stem (file basename without extension) based on the
  // source image filename or an empty string when source image is not set.
  static std::string GetTargetStem(const Texture &texture);

  // Returns |texture| image stem (file basename without extension) based on the
  // source image filename or a name generated from |index| and |suffix| like
  // "Texture5_BaseColor" when source image is not set.
  static std::string GetOrGenerateTargetStem(const Texture &texture, int index,
                                             const std::string &suffix);

  // Returns |texture| format based on compression settings, the source image
  // mime type or the source image filename.
  static ImageFormat GetTargetFormat(const Texture &texture);

  // Returns |texture| image file extension based on compression settings, the
  // source image mime type or the source image filename.
  static std::string GetTargetExtension(const Texture &texture);

  // Returns mime type string based on |texture| compression settings, source
  // image mime type or the source image filename.
  static std::string GetTargetMimeType(const Texture &texture);

  // Returns mime type string for a given |image_format|.
  static std::string GetMimeType(ImageFormat image_format);

  // Returns |texture| format based on source image mime type or the source
  // image filename.
  static ImageFormat GetSourceFormat(const Texture &texture);

  // Returns image format corresponding to a given image file |extension|. NONE
  // is returned when |extension| is empty or unknown.
  static ImageFormat GetFormat(const std::string &extension);

  // Returns image file extension corresponding to a given image |format|. Empty
  // extension is returned when the |format| is NONE.
  static std::string GetExtension(ImageFormat format);

  // Returns the number of channels required for encoding a |texture| from a
  // given |material_library|, taking into account texture opacity and assuming
  // that occlusion and metallic-roughness texture maps may share a texture.
  // TODO(vytyaz): Move this and FindTextures() to MaterialLibrary class.
  static int ComputeRequiredNumChannels(
      const Texture &texture, const MaterialLibrary &material_library);

  static std::vector<const Texture *> FindTextures(
      const TextureMap::Type texture_type,
      const MaterialLibrary *material_library);
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_TEXTURE_TEXTURE_UTILS_H_
