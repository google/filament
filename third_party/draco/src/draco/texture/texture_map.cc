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
#include "draco/texture/texture_map.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

namespace draco {

TextureMap::TextureMap()
    : type_(TextureMap::GENERIC),
      wrapping_mode_(CLAMP_TO_EDGE),
      tex_coord_index_(-1),
      min_filter_(UNSPECIFIED),
      mag_filter_(UNSPECIFIED),
      texture_(nullptr) {}

void TextureMap::Copy(const TextureMap &src) {
  type_ = src.type_;
  wrapping_mode_ = src.wrapping_mode_;
  tex_coord_index_ = src.tex_coord_index_;
  min_filter_ = src.min_filter_;
  mag_filter_ = src.mag_filter_;
  if (src.owned_texture_ == nullptr) {
    owned_texture_ = nullptr;
    texture_ = src.texture_;
  } else {
    std::unique_ptr<Texture> new_texture(new Texture());
    new_texture->Copy(*src.owned_texture_);
    owned_texture_ = std::move(new_texture);
    texture_ = owned_texture_.get();
  }
  texture_transform_.Copy(src.texture_transform_);
}

void TextureMap::SetProperties(Type type) {
  SetProperties(type, WrappingMode(CLAMP_TO_EDGE), 0);
}

void TextureMap::SetProperties(TextureMap::Type type, int tex_coord_index) {
  SetProperties(type, WrappingMode(CLAMP_TO_EDGE), tex_coord_index);
}

void TextureMap::SetProperties(Type type, WrappingMode wrapping_mode,
                               int tex_coord_index) {
  SetProperties(type, wrapping_mode, tex_coord_index, UNSPECIFIED, UNSPECIFIED);
}

void TextureMap::SetProperties(Type type, WrappingMode wrapping_mode,
                               int tex_coord_index, FilterType min_filter,
                               FilterType mag_filter) {
  type_ = type;
  wrapping_mode_ = wrapping_mode;
  tex_coord_index_ = tex_coord_index;
  min_filter_ = min_filter;
  mag_filter_ = mag_filter;
}

void TextureMap::SetTexture(std::unique_ptr<Texture> texture) {
  owned_texture_ = std::move(texture);
  texture_ = owned_texture_.get();
}

void TextureMap::SetTexture(Texture *texture) {
  owned_texture_ = nullptr;
  texture_ = texture;
}

void TextureMap::SetTransform(const TextureTransform &transform) {
  texture_transform_.Copy(transform);
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
