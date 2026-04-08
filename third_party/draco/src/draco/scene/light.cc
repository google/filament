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
#include "draco/scene/light.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

#include <limits>

#include "draco/core/constants.h"

namespace draco {

Light::Light()
    : color_(1.0f, 1.0f, 1.0f),
      intensity_(1.0),
      type_(POINT),
      range_(std::numeric_limits<float>::max()),  // Infinity.
      inner_cone_angle_(0.0),
      outer_cone_angle_(DRACO_PI / 4.0) {}

void Light::Copy(const Light &light) {
  name_ = light.name_;
  color_ = light.color_;
  intensity_ = light.intensity_;
  type_ = light.type_;
  range_ = light.range_;
  inner_cone_angle_ = light.inner_cone_angle_;
  outer_cone_angle_ = light.outer_cone_angle_;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
