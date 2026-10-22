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
#include "draco/texture/texture_transform.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

namespace draco {

TextureTransform::TextureTransform() { Clear(); }

void TextureTransform::Clear() {
  offset_ = TextureTransform::GetDefaultOffset();
  rotation_ = TextureTransform::GetDefaultRotation();
  scale_ = TextureTransform::GetDefaultScale();
  tex_coord_ = TextureTransform::GetDefaultTexCoord();
}

void TextureTransform::Copy(const TextureTransform &src) {
  offset_ = src.offset_;
  rotation_ = src.rotation_;
  scale_ = src.scale_;
  tex_coord_ = src.tex_coord_;
}

bool TextureTransform::IsDefault(const TextureTransform &tt) {
  const TextureTransform defaults;
  if (tt == defaults) {
    return true;
  }
  return false;
}

bool TextureTransform::IsOffsetSet() const {
  return offset_ != TextureTransform::GetDefaultOffset();
}

bool TextureTransform::IsRotationSet() const {
  return rotation_ != TextureTransform::GetDefaultRotation();
}

bool TextureTransform::IsScaleSet() const {
  return scale_ != TextureTransform::GetDefaultScale();
}

bool TextureTransform::IsTexCoordSet() const {
  return tex_coord_ != TextureTransform::GetDefaultTexCoord();
}

bool TextureTransform::operator==(const TextureTransform &tt) const {
  if (tex_coord_ != tt.tex_coord_) {
    return false;
  }
  if (rotation_ != tt.rotation_) {
    return false;
  }
  if (offset_ != tt.offset_) {
    return false;
  }
  if (scale_ != tt.scale_) {
    return false;
  }
  return true;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
