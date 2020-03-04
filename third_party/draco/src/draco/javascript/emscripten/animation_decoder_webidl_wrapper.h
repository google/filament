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
#ifndef DRACO_JAVASCRIPT_EMSCRITPEN_DECODER_WEBIDL_WRAPPER_H_
#define DRACO_JAVASCRIPT_EMSCRITPEN_DECODER_WEBIDL_WRAPPER_H_

#include <vector>

#include "draco/animation/keyframe_animation_decoder.h"
#include "draco/attributes/attribute_transform_type.h"
#include "draco/attributes/point_attribute.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/decode.h"
#include "draco/core/decoder_buffer.h"

typedef draco::AttributeTransformType draco_AttributeTransformType;
typedef draco::GeometryAttribute draco_GeometryAttribute;
typedef draco_GeometryAttribute::Type draco_GeometryAttribute_Type;
typedef draco::EncodedGeometryType draco_EncodedGeometryType;
typedef draco::Status draco_Status;
typedef draco::Status::Code draco_StatusCode;

class DracoFloat32Array {
 public:
  DracoFloat32Array();
  float GetValue(int index) const;

  // In case |values| is nullptr, the data is allocated but not initialized.
  bool SetValues(const float *values, int count);

  // Directly sets a value for a specific index. The array has to be already
  // allocated at this point (using SetValues() method).
  void SetValue(int index, float val) { values_[index] = val; }

  int size() const { return values_.size(); }

 private:
  std::vector<float> values_;
};

// Class used by emscripten WebIDL Binder [1] to wrap calls to decode animation
// data.
class AnimationDecoder {
 public:
  AnimationDecoder();

  // Decodes animation data from the provided buffer.
  const draco::Status *DecodeBufferToKeyframeAnimation(
      draco::DecoderBuffer *in_buffer, draco::KeyframeAnimation *animation);

  static bool GetTimestamps(const draco::KeyframeAnimation &animation,
                            DracoFloat32Array *timestamp);

  static bool GetKeyframes(const draco::KeyframeAnimation &animation,
                           int keyframes_id, DracoFloat32Array *animation_data);

 private:
  draco::KeyframeAnimationDecoder decoder_;
  draco::Status last_status_;
};

#endif  // DRACO_JAVASCRIPT_EMSCRITPEN_DECODER_WEBIDL_WRAPPER_H_
