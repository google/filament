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
#ifndef DRACO_TEXTURE_TEXTURE_TRANSFORM_H_
#define DRACO_TEXTURE_TEXTURE_TRANSFORM_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <array>
#include <memory>

namespace draco {

// Class to hold texture transformations. Parameters are based on the glTF 2.0
// extension KHR_texture_transform:
// https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_texture_transform
class TextureTransform {
 public:
  TextureTransform();

  // Resets the values back to defaults.
  void Clear();

  // Copies texture transform data from the |src| texture transform to this
  // texture transform.
  void Copy(const TextureTransform &src);

  // Returns true if |tt| contains all default values.
  static bool IsDefault(const TextureTransform &tt);

  bool IsOffsetSet() const;
  bool IsRotationSet() const;
  bool IsScaleSet() const;
  bool IsTexCoordSet() const;

  void set_offset(const std::array<double, 2> &offset) { offset_ = offset; }
  const std::array<double, 2> &offset() const { return offset_; }
  void set_scale(const std::array<double, 2> &scale) { scale_ = scale; }
  const std::array<double, 2> &scale() const { return scale_; }

  void set_rotation(double rotation) { rotation_ = rotation; }
  double rotation() const { return rotation_; }
  void set_tex_coord(int tex_coord) { tex_coord_ = tex_coord; }
  int tex_coord() const { return tex_coord_; }

  bool operator==(const TextureTransform &tt) const;

 private:
  static std::array<double, 2> GetDefaultOffset() { return {0.0, 0.0}; }
  static float GetDefaultRotation() { return 0.0; }
  static std::array<double, 2> GetDefaultScale() { return {0.0, 0.0}; }
  static int GetDefaultTexCoord() { return -1; }

  std::array<double, 2> offset_;
  double rotation_;
  std::array<double, 2> scale_;
  int tex_coord_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_TEXTURE_TEXTURE_TRANSFORM_H_
