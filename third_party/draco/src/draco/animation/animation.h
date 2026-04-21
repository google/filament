// Copyright 2019 The Draco Authors.
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
#ifndef DRACO_ANIMATION_ANIMATION_H_
#define DRACO_ANIMATION_ANIMATION_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>
#include <vector>

#include "draco/animation/node_animation_data.h"
#include "draco/core/status.h"

namespace draco {

// Struct to hold information about an animation's sampler.
struct AnimationSampler {
  enum class SamplerInterpolation { LINEAR, STEP, CUBICSPLINE };

  static std::string InterpolationToString(SamplerInterpolation value) {
    switch (value) {
      case SamplerInterpolation::STEP:
        return "STEP";
      case SamplerInterpolation::CUBICSPLINE:
        return "CUBICSPLINE";
      default:
        return "LINEAR";
    }
  }

  AnimationSampler()
      : input_index(-1),
        interpolation_type(SamplerInterpolation::LINEAR),
        output_index(-1) {}

  void Copy(const AnimationSampler &src) {
    input_index = src.input_index;
    interpolation_type = src.interpolation_type;
    output_index = src.output_index;
  }

  int input_index;
  SamplerInterpolation interpolation_type;
  int output_index;
};

// Struct to hold information about an animation's channel.
struct AnimationChannel {
  enum class ChannelTransformation { TRANSLATION, ROTATION, SCALE, WEIGHTS };

  static std::string TransformationToString(ChannelTransformation value) {
    switch (value) {
      case ChannelTransformation::ROTATION:
        return "rotation";
      case ChannelTransformation::SCALE:
        return "scale";
      case ChannelTransformation::WEIGHTS:
        return "weights";
      default:
        return "translation";
    }
  }

  AnimationChannel()
      : target_index(-1),
        transformation_type(ChannelTransformation::TRANSLATION),
        sampler_index(-1) {}

  void Copy(const AnimationChannel &src) {
    target_index = src.target_index;
    transformation_type = src.transformation_type;
    sampler_index = src.sampler_index;
  }

  int target_index;
  ChannelTransformation transformation_type;
  int sampler_index;
};

// This class is used to hold data and information of glTF animations.
class Animation {
 public:
  Animation() {}

  void Copy(const Animation &src);

  const std::string &GetName() const { return name_; }
  void SetName(const std::string &name) { name_ = name; }

  // Returns the number of channels in an animation.
  int NumChannels() const { return channels_.size(); }
  // Returns the number of samplers in an animation.
  int NumSamplers() const { return samplers_.size(); }
  // Returns the number of accessors in an animation.
  int NumNodeAnimationData() const { return node_animation_data_.size(); }

  // Returns a channel in the animation.
  AnimationChannel *GetChannel(int index) { return channels_[index].get(); }
  const AnimationChannel *GetChannel(int index) const {
    return channels_[index].get();
  }
  // Returns a sampler in the animation.
  AnimationSampler *GetSampler(int index) { return samplers_[index].get(); }
  const AnimationSampler *GetSampler(int index) const {
    return samplers_[index].get();
  }
  // Returns an accessor in the animation.
  NodeAnimationData *GetNodeAnimationData(int index) {
    return node_animation_data_[index].get();
  }
  const NodeAnimationData *GetNodeAnimationData(int index) const {
    return node_animation_data_[index].get();
  }

  void AddNodeAnimationData(
      std::unique_ptr<NodeAnimationData> node_animation_data) {
    node_animation_data_.push_back(std::move(node_animation_data));
  }
  void AddSampler(std::unique_ptr<AnimationSampler> sampler) {
    samplers_.push_back(std::move(sampler));
  }
  void AddChannel(std::unique_ptr<AnimationChannel> channel) {
    channels_.push_back(std::move(channel));
  }

 private:
  std::string name_;
  std::vector<std::unique_ptr<AnimationSampler>> samplers_;
  std::vector<std::unique_ptr<AnimationChannel>> channels_;
  std::vector<std::unique_ptr<NodeAnimationData>> node_animation_data_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_ANIMATION_ANIMATION_H_
