// Copyright 2022 The Draco Authors.
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
#ifndef DRACO_SCENE_LIGHT_H_
#define DRACO_SCENE_LIGHT_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <string>

#include "draco/core/vector_d.h"

namespace draco {

// Describes a light in a scene according to the KHR_lights_punctual extension.
class Light {
 public:
  enum Type { DIRECTIONAL, POINT, SPOT };

  Light();

  void Copy(const Light &light);

  // Name.
  void SetName(const std::string &name) { name_ = name; }
  const std::string &GetName() const { return name_; }

  // Color.
  void SetColor(const Vector3f &color) { color_ = color; }
  const Vector3f &GetColor() const { return color_; }

  // Intensity.
  void SetIntensity(double intensity) { intensity_ = intensity; }
  double GetIntensity() const { return intensity_; }

  // Type.
  void SetType(Type type) { type_ = type; }
  Type GetType() const { return type_; }

  // Range.
  void SetRange(double range) { range_ = range; }
  double GetRange() const { return range_; }

  // Inner cone angle.
  void SetInnerConeAngle(double angle) { inner_cone_angle_ = angle; }
  double GetInnerConeAngle() const { return inner_cone_angle_; }

  // Outer cone angle.
  void SetOuterConeAngle(double angle) { outer_cone_angle_ = angle; }
  double GetOuterConeAngle() const { return outer_cone_angle_; }

 private:
  std::string name_;
  Vector3f color_;
  double intensity_;
  Type type_;

  // The range is only applicable to lights with Type::POINT or Type::SPOT.
  double range_;

  // The cone angles are only applicable to lights with Type::SPOT.
  double inner_cone_angle_;
  double outer_cone_angle_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_SCENE_LIGHT_H_
