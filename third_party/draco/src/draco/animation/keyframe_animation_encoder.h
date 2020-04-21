// Copyright 2017 The Draco Authors.
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
#ifndef DRACO_ANIMATION_KEYFRAME_ANIMATION_ENCODER_H_
#define DRACO_ANIMATION_KEYFRAME_ANIMATION_ENCODER_H_

#include "draco/animation/keyframe_animation.h"
#include "draco/compression/point_cloud/point_cloud_sequential_encoder.h"

namespace draco {

// Class for encoding keyframe animation. It takes KeyframeAnimation as a
// PointCloud and compress it. It's mostly a wrapper around PointCloudEncoder so
// that the animation module could be separated from geometry compression when
// exposed to developers.
class KeyframeAnimationEncoder : private PointCloudSequentialEncoder {
 public:
  KeyframeAnimationEncoder();

  // Encode an animation to a buffer.
  Status EncodeKeyframeAnimation(const KeyframeAnimation &animation,
                                 const EncoderOptions &options,
                                 EncoderBuffer *out_buffer);
};

}  // namespace draco

#endif  // DRACO_ANIMATION_KEYFRAME_ANIMATION_ENCODER_H_
