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
#include "draco/animation/keyframe_animation.h"

#include "draco/core/draco_test_base.h"

namespace {

class KeyframeAnimationTest : public ::testing::Test {
 protected:
  KeyframeAnimationTest() {}

  bool CreateAndAddTimestamps(int32_t num_frames) {
    timestamps_.resize(num_frames);
    for (int i = 0; i < timestamps_.size(); ++i) {
      timestamps_[i] = static_cast<draco::KeyframeAnimation::TimestampType>(i);
    }
    return keyframe_animation_.SetTimestamps(timestamps_);
  }

  int32_t CreateAndAddAnimationData(int32_t num_frames,
                                    uint32_t num_components) {
    // Create and add animation data with.
    animation_data_.resize(num_frames * num_components);
    for (int i = 0; i < animation_data_.size(); ++i) {
      animation_data_[i] = static_cast<float>(i);
    }
    return keyframe_animation_.AddKeyframes(draco::DT_FLOAT32, num_components,
                                            animation_data_);
  }

  template <int num_components_t>
  void CompareAnimationData() {
    // Compare time stamp.
    const auto timestamp_att = keyframe_animation_.timestamps();
    for (int i = 0; i < timestamps_.size(); ++i) {
      std::array<float, 1> att_value;
      ASSERT_TRUE((timestamp_att->GetValue<float, 1>(
          draco::AttributeValueIndex(i), &att_value)));
      ASSERT_FLOAT_EQ(att_value[0], i);
    }

    // Compare keyframe data.
    const auto keyframe_att = keyframe_animation_.keyframes(1);
    for (int i = 0; i < animation_data_.size() / num_components_t; ++i) {
      std::array<float, num_components_t> att_value;
      ASSERT_TRUE((keyframe_att->GetValue<float, num_components_t>(
          draco::AttributeValueIndex(i), &att_value)));
      for (int j = 0; j < num_components_t; ++j) {
        ASSERT_FLOAT_EQ(att_value[j], i * num_components_t + j);
      }
    }
  }

  template <int num_components_t>
  void TestKeyframeAnimation(int32_t num_frames) {
    ASSERT_TRUE(CreateAndAddTimestamps(num_frames));
    ASSERT_EQ(CreateAndAddAnimationData(num_frames, num_components_t), 1);
    CompareAnimationData<num_components_t>();
  }

  draco::KeyframeAnimation keyframe_animation_;
  std::vector<draco::KeyframeAnimation::TimestampType> timestamps_;
  std::vector<float> animation_data_;
};

// Test animation with 1 component and 10 frames.
TEST_F(KeyframeAnimationTest, OneComponent) { TestKeyframeAnimation<1>(10); }

// Test animation with 4 component and 10 frames.
TEST_F(KeyframeAnimationTest, FourComponent) { TestKeyframeAnimation<4>(10); }

// Test adding animation data before timestamp.
TEST_F(KeyframeAnimationTest, AddingAnimationFirst) {
  ASSERT_EQ(CreateAndAddAnimationData(5, 1), 1);
  ASSERT_TRUE(CreateAndAddTimestamps(5));
}

// Test adding timestamp more than once.
TEST_F(KeyframeAnimationTest, ErrorAddingTimestampsTwice) {
  ASSERT_TRUE(CreateAndAddTimestamps(5));
  ASSERT_FALSE(CreateAndAddTimestamps(5));
}
// Test animation with multiple animation data.
TEST_F(KeyframeAnimationTest, MultipleAnimationData) {
  const int num_frames = 5;
  ASSERT_TRUE(CreateAndAddTimestamps(num_frames));
  ASSERT_EQ(CreateAndAddAnimationData(num_frames, 1), 1);
  ASSERT_EQ(CreateAndAddAnimationData(num_frames, 2), 2);
}

}  // namespace
