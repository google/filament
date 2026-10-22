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
#include "draco/animation/animation.h"

#include "draco/core/draco_test_base.h"
#include "draco/draco_features.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED
TEST(AnimationTest, TestCopy) {
  // Test copying of animation data.
  draco::Animation src_anim;
  ASSERT_TRUE(src_anim.GetName().empty());
  src_anim.SetName("Walking");
  ASSERT_EQ(src_anim.GetName(), "Walking");

  std::unique_ptr<draco::AnimationSampler> src_sampler_0(
      new draco::AnimationSampler());
  src_sampler_0->interpolation_type =
      draco::AnimationSampler::SamplerInterpolation::CUBICSPLINE;
  std::unique_ptr<draco::AnimationSampler> src_sampler_1(
      new draco::AnimationSampler());
  src_sampler_1->Copy(*src_sampler_0);

  ASSERT_EQ(src_sampler_0->interpolation_type,
            src_sampler_1->interpolation_type);

  src_sampler_1->interpolation_type =
      draco::AnimationSampler::SamplerInterpolation::STEP;

  src_anim.AddSampler(std::move(src_sampler_0));
  src_anim.AddSampler(std::move(src_sampler_1));
  ASSERT_EQ(src_anim.NumSamplers(), 2);

  std::unique_ptr<draco::AnimationChannel> src_channel(
      new draco::AnimationChannel());
  src_channel->transformation_type =
      draco::AnimationChannel::ChannelTransformation::WEIGHTS;
  src_anim.AddChannel(std::move(src_channel));
  ASSERT_EQ(src_anim.NumChannels(), 1);

  draco::Animation dst_anim;
  dst_anim.Copy(src_anim);

  ASSERT_EQ(dst_anim.GetName(), src_anim.GetName());
  ASSERT_EQ(dst_anim.NumSamplers(), 2);
  ASSERT_EQ(dst_anim.NumChannels(), 1);

  ASSERT_EQ(dst_anim.GetSampler(0)->interpolation_type,
            src_anim.GetSampler(0)->interpolation_type);
  ASSERT_EQ(dst_anim.GetSampler(1)->interpolation_type,
            src_anim.GetSampler(1)->interpolation_type);
  ASSERT_EQ(dst_anim.GetChannel(0)->transformation_type,
            src_anim.GetChannel(0)->transformation_type);
}
#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
