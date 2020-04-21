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
#ifndef DRACO_JAVASCRIPT_EMSCRIPTEN_ANIMATION_ENCODER_WEBIDL_WRAPPER_H_
#define DRACO_JAVASCRIPT_EMSCRIPTEN_ANIMATION_ENCODER_WEBIDL_WRAPPER_H_

#include <vector>

#include "draco/animation/keyframe_animation_encoder.h"
#include "draco/attributes/point_attribute.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/config/encoder_options.h"
#include "draco/compression/encode.h"

class DracoInt8Array {
 public:
  DracoInt8Array();
  int GetValue(int index) const;
  bool SetValues(const char *values, int count);

  size_t size() { return values_.size(); }

 private:
  std::vector<int> values_;
};

class AnimationBuilder {
 public:
  AnimationBuilder();

  bool SetTimestamps(draco::KeyframeAnimation *animation, long num_frames,
                     const float *timestamps);

  int AddKeyframes(draco::KeyframeAnimation *animation, long num_frames,
                   long num_components, const float *animation_data);
};

class AnimationEncoder {
 public:
  AnimationEncoder();

  void SetTimestampsQuantization(long quantization_bits);
  // TODO: Use expert encoder to set per attribute quantization.
  void SetKeyframesQuantization(long quantization_bits);
  int EncodeAnimationToDracoBuffer(draco::KeyframeAnimation *animation,
                                   DracoInt8Array *draco_buffer);

 private:
  draco::KeyframeAnimationEncoder encoder_;
  long timestamps_quantization_bits_;
  long keyframes_quantization_bits_;
  draco::EncoderOptions options_;
};

#endif  // DRACO_JAVASCRIPT_EMSCRIPTEN_ANIMATION_ENCODER_WEBIDL_WRAPPER_H_
