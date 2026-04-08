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
#include "draco/texture/texture_library.h"

#include <unordered_map>

#ifdef DRACO_TRANSCODER_SUPPORTED

namespace draco {

void TextureLibrary::Copy(const TextureLibrary &src) {
  Clear();
  Append(src);
}

void TextureLibrary::Append(const TextureLibrary &src) {
  const size_t old_num_textures = textures_.size();
  textures_.resize(old_num_textures + src.textures_.size());
  for (int i = 0; i < src.textures_.size(); ++i) {
    textures_[old_num_textures + i] = std::unique_ptr<Texture>(new Texture());
    textures_[old_num_textures + i]->Copy(*src.textures_[i]);
  }
}

void TextureLibrary::Clear() { textures_.clear(); }

int TextureLibrary::PushTexture(std::unique_ptr<Texture> texture) {
  textures_.push_back(std::move(texture));
  return textures_.size() - 1;
}

std::unordered_map<const Texture *, int>
TextureLibrary::ComputeTextureToIndexMap() const {
  std::unordered_map<const Texture *, int> ret;
  for (int i = 0; i < textures_.size(); ++i) {
    ret[textures_[i].get()] = i;
  }
  return ret;
}

std::unique_ptr<Texture> TextureLibrary::RemoveTexture(int index) {
  std::unique_ptr<Texture> ret = std::move(textures_[index]);
  textures_.erase(textures_.begin() + index);
  return ret;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
