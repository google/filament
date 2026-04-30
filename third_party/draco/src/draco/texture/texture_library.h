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
#ifndef DRACO_TEXTURE_TEXTURE_LIBRARY_H_
#define DRACO_TEXTURE_TEXTURE_LIBRARY_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>
#include <unordered_map>
#include <vector>

#include "draco/texture/texture.h"

namespace draco {

// Container class for storing draco::Texture objects in an indexed list.
class TextureLibrary {
 public:
  // Copies textures from the source library to this library. Order of the
  // copied textures is preserved.
  void Copy(const TextureLibrary &src);

  // Appends all textures from the source library to this library. All textures
  // are copied over.
  void Append(const TextureLibrary &src);

  // Removes all textures from the library.
  void Clear();

  // Pushes a new texture into the library. Returns an index of the newly
  // inserted texture.
  int PushTexture(std::unique_ptr<Texture> texture);

  size_t NumTextures() const { return textures_.size(); }

  Texture *GetTexture(int index) { return textures_[index].get(); }
  const Texture *GetTexture(int index) const { return textures_[index].get(); }

  // Returns a map from texture pointer to texture index for all textures.
  std::unordered_map<const Texture *, int> ComputeTextureToIndexMap() const;

  // Removes and returns a texture from the library. The returned texture can be
  // either used by the caller or ignored in which case it would be
  // automatically deleted.
  std::unique_ptr<Texture> RemoveTexture(int index);

 private:
  std::vector<std::unique_ptr<Texture>> textures_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_TEXTURE_TEXTURE_LIBRARY_H_
