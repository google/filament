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
#include "draco/animation/animation.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

namespace draco {

void Animation::Copy(const Animation &src) {
  name_ = src.name_;
  channels_.clear();
  for (int i = 0; i < src.NumChannels(); ++i) {
    std::unique_ptr<AnimationChannel> new_channel(new AnimationChannel());
    new_channel->Copy(*src.GetChannel(i));
    channels_.push_back(std::move(new_channel));
  }

  samplers_.clear();
  for (int i = 0; i < src.NumSamplers(); ++i) {
    std::unique_ptr<AnimationSampler> new_sampler(new AnimationSampler());
    new_sampler->Copy(*src.GetSampler(i));
    samplers_.push_back(std::move(new_sampler));
  }

  node_animation_data_.clear();
  for (int i = 0; i < src.NumNodeAnimationData(); ++i) {
    std::unique_ptr<NodeAnimationData> new_data(new NodeAnimationData());
    new_data->Copy(*src.GetNodeAnimationData(i));
    node_animation_data_.push_back(std::move(new_data));
  }
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
