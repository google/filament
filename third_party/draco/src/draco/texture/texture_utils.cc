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
#include "draco/texture/texture_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "draco/core/status.h"
#include "draco/core/status_or.h"
#include "draco/io/file_utils.h"
#include "draco/io/image_compression_options.h"
#include "draco/texture/texture.h"
#include "draco/texture/texture_map.h"

namespace draco {

std::string TextureUtils::GetTargetStem(const Texture &texture) {
  // Return stem of the source image if there is one.
  if (!texture.source_image().filename().empty()) {
    const std::string &full_path = texture.source_image().filename();
    std::string folder_path;
    std::string filename;
    SplitPath(full_path, &folder_path, &filename);
    return RemoveFileExtension(filename);
  }

  // Return an empty stem.
  return "";
}

std::string TextureUtils::GetOrGenerateTargetStem(const Texture &texture,
                                                  int index,
                                                  const std::string &suffix) {
  // Return target stem from |texture| if there is one.
  const std::string name = GetTargetStem(texture);
  if (!name.empty()) {
    return name;
  }

  // Return target stem generated from |index| and |suffix|.
  return "Texture" + std::to_string(index) + suffix;
}

ImageFormat TextureUtils::GetTargetFormat(const Texture &texture) {
  // Return format based on source image mime type.
  return GetSourceFormat(texture);
}

std::string TextureUtils::GetTargetExtension(const Texture &texture) {
  return GetExtension(GetTargetFormat(texture));
}

std::string TextureUtils::GetTargetMimeType(const Texture &texture) {
  const ImageFormat format = GetTargetFormat(texture);
  if (format == ImageFormat::NONE) {
    // Unknown format, try to re-use mime type stored in the source_image.
    // This is mostly useful if users need to handle image formats not directly
    // supported by the Draco library.
    if (!texture.source_image().mime_type().empty()) {
      return texture.source_image().mime_type();
    } else if (!texture.source_image().filename().empty()) {
      // Try to set mime type based on the extension of the filename.
      const std::string extension =
          LowercaseFileExtension(texture.source_image().filename());
      if (!extension.empty()) {
        return "image/" + extension;
      }
    }
  }
  return GetMimeType(format);
}

std::string TextureUtils::GetMimeType(ImageFormat image_format) {
  switch (image_format) {
    case ImageFormat::PNG:
      return "image/png";
    case ImageFormat::JPEG:
      return "image/jpeg";
    case ImageFormat::BASIS:
      return "image/ktx2";
    case ImageFormat::WEBP:
      return "image/webp";
    case ImageFormat::NONE:
    default:
      return "";
  }
}

ImageFormat TextureUtils::GetSourceFormat(const Texture &texture) {
  // Try to get the extension based on source image mime type.
  std::string extension =
      LowercaseMimeTypeExtension(texture.source_image().mime_type());
  if (extension.empty() && !texture.source_image().filename().empty()) {
    // Try to get the extension from the source image filename.
    extension = LowercaseFileExtension(texture.source_image().filename());
  }
  if (extension.empty()) {
    // Default to png.
    extension = "png";
  }
  return GetFormat(extension);
}

ImageFormat TextureUtils::GetFormat(const std::string &extension) {
  if (extension == "png") {
    return ImageFormat::PNG;
  } else if (extension == "jpg" || extension == "jpeg") {
    return ImageFormat::JPEG;
  } else if (extension == "basis" || extension == "ktx2") {
    return ImageFormat::BASIS;
  } else if (extension == "webp") {
    return ImageFormat::WEBP;
  }
  return ImageFormat::NONE;
}

std::string TextureUtils::GetExtension(ImageFormat format) {
  switch (format) {
    case ImageFormat::PNG:
      return "png";
    case ImageFormat::JPEG:
      return "jpg";
    case ImageFormat::BASIS:
      return "ktx2";
    case ImageFormat::WEBP:
      return "webp";
    case ImageFormat::NONE:
    default:
      return "";
  }
}

int TextureUtils::ComputeRequiredNumChannels(
    const Texture &texture, const MaterialLibrary &material_library) {
  // TODO(vytyaz): Consider a case where |texture| is not only used in OMR but
  // also in other texture map types.
  const auto mr_textures = TextureUtils::FindTextures(
      TextureMap::METALLIC_ROUGHNESS, &material_library);
  if (std::find(mr_textures.begin(), mr_textures.end(), &texture) ==
      mr_textures.end()) {
    // Occlusion-only texture.
    return 1;
  }
  // Occlusion-metallic-roughness texture.
  return 3;
}

std::vector<const Texture *> TextureUtils::FindTextures(
    const TextureMap::Type texture_type,
    const MaterialLibrary *material_library) {
  // Find textures with no duplicates.
  std::unordered_set<const Texture *> textures;
  for (int i = 0; i < material_library->NumMaterials(); ++i) {
    const TextureMap *const texture_map =
        material_library->GetMaterial(i)->GetTextureMapByType(texture_type);
    if (texture_map != nullptr && texture_map->texture() != nullptr) {
      textures.insert(texture_map->texture());
    }
  }

  // Return the textures as a vector.
  std::vector<const Texture *> result;
  result.insert(result.end(), textures.begin(), textures.end());
  return result;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
