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
#include "draco/javascript/emscripten/animation_encoder_webidl_wrapper.h"

#include "draco/animation/keyframe_animation.h"
#include "draco/animation/keyframe_animation_encoder.h"

DracoInt8Array::DracoInt8Array() {}

int DracoInt8Array::GetValue(int index) const { return values_[index]; }

bool DracoInt8Array::SetValues(const char *values, int count) {
  values_.assign(values, values + count);
  return true;
}

AnimationBuilder::AnimationBuilder() {}

bool AnimationBuilder::SetTimestamps(draco::KeyframeAnimation *animation,
                                     long num_frames, const float *timestamps) {
  if (!animation || !timestamps) {
    return false;
  }
  std::vector<draco::KeyframeAnimation::TimestampType> timestamps_arr(
      timestamps, timestamps + num_frames);
  return animation->SetTimestamps(timestamps_arr);
}

int AnimationBuilder::AddKeyframes(draco::KeyframeAnimation *animation,
                                   long num_frames, long num_components,
                                   const float *animation_data) {
  if (!animation || !animation_data) {
    return -1;
  }
  std::vector<float> keyframes_arr(
      animation_data, animation_data + num_frames * num_components);
  return animation->AddKeyframes(draco::DT_FLOAT32, num_components,
                                 keyframes_arr);
}

AnimationEncoder::AnimationEncoder()
    : timestamps_quantization_bits_(-1),
      keyframes_quantization_bits_(-1),
      options_(draco::EncoderOptions::CreateDefaultOptions()) {}

void AnimationEncoder::SetTimestampsQuantization(long quantization_bits) {
  timestamps_quantization_bits_ = quantization_bits;
}

void AnimationEncoder::SetKeyframesQuantization(long quantization_bits) {
  keyframes_quantization_bits_ = quantization_bits;
}

int AnimationEncoder::EncodeAnimationToDracoBuffer(
    draco::KeyframeAnimation *animation, DracoInt8Array *draco_buffer) {
  if (!animation) {
    return 0;
  }
  draco::EncoderBuffer buffer;

  if (timestamps_quantization_bits_ > 0) {
    options_.SetAttributeInt(0, "quantization_bits",
                             timestamps_quantization_bits_);
  }
  if (keyframes_quantization_bits_ > 0) {
    for (int i = 1; i <= animation->num_animations(); ++i) {
      options_.SetAttributeInt(i, "quantization_bits",
                               keyframes_quantization_bits_);
    }
  }
  if (!encoder_.EncodeKeyframeAnimation(*animation, options_, &buffer).ok()) {
    return 0;
  }

  draco_buffer->SetValues(buffer.data(), buffer.size());
  return buffer.size();
}
