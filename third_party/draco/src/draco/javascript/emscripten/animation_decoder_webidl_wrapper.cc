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
#include "draco/javascript/emscripten/animation_decoder_webidl_wrapper.h"

#include <vector>

#include "draco/compression/decode.h"
#include "draco/mesh/mesh.h"
#include "draco/mesh/mesh_stripifier.h"

using draco::DecoderBuffer;
using draco::PointAttribute;
using draco::Status;

DracoFloat32Array::DracoFloat32Array() {}

float DracoFloat32Array::GetValue(int index) const { return values_[index]; }

bool DracoFloat32Array::SetValues(const float *values, int count) {
  if (values) {
    values_.assign(values, values + count);
  } else {
    values_.resize(count);
  }
  return true;
}

AnimationDecoder::AnimationDecoder() {}

// Decodes animation data from the provided buffer.
const draco::Status *AnimationDecoder::DecodeBufferToKeyframeAnimation(
    draco::DecoderBuffer *in_buffer, draco::KeyframeAnimation *animation) {
  draco::DecoderOptions dec_options;
  last_status_ = decoder_.Decode(dec_options, in_buffer, animation);
  return &last_status_;
}

bool AnimationDecoder::GetTimestamps(const draco::KeyframeAnimation &animation,
                                     DracoFloat32Array *timestamp) {
  if (!timestamp) {
    return false;
  }
  const int num_frames = animation.num_frames();
  const draco::PointAttribute *timestamp_att = animation.timestamps();
  // Timestamp attribute has only 1 component, so the number of components is
  // equal to the number of frames.
  timestamp->SetValues(nullptr, num_frames);
  int entry_id = 0;
  float timestamp_value = -1.0;
  for (draco::PointIndex i(0); i < num_frames; ++i) {
    const draco::AttributeValueIndex val_index = timestamp_att->mapped_index(i);
    if (!timestamp_att->ConvertValue<float>(val_index, &timestamp_value)) {
      return false;
    }
    timestamp->SetValue(entry_id++, timestamp_value);
  }
  return true;
}

bool AnimationDecoder::GetKeyframes(const draco::KeyframeAnimation &animation,
                                    int keyframes_id,
                                    DracoFloat32Array *animation_data) {
  const int num_frames = animation.num_frames();
  // Get animation data.
  const draco::PointAttribute *animation_data_att =
      animation.keyframes(keyframes_id);
  if (!animation_data_att) {
    return false;
  }

  const int components = animation_data_att->num_components();
  const int num_entries = num_frames * components;
  const int kMaxAttributeFloatValues = 4;

  std::vector<float> values(components, -1.0);
  int entry_id = 0;
  animation_data->SetValues(nullptr, num_entries);
  for (draco::PointIndex i(0); i < num_frames; ++i) {
    const draco::AttributeValueIndex val_index =
        animation_data_att->mapped_index(i);
    if (!animation_data_att->ConvertValue<float>(val_index, &values[0])) {
      return false;
    }
    for (int j = 0; j < components; ++j) {
      animation_data->SetValue(entry_id++, values[j]);
    }
  }
  return true;
}
