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
#include "draco/animation/keyframe_animation_decoder.h"
#include "draco/animation/keyframe_animation_encoder.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {

class KeyframeAnimationEncodingTest : public ::testing::Test {
 protected:
  KeyframeAnimationEncodingTest() {}

  bool CreateAndAddTimestamps(int32_t num_frames) {
    timestamps_.resize(num_frames);
    for (int i = 0; i < timestamps_.size(); ++i)
      timestamps_[i] = static_cast<draco::KeyframeAnimation::TimestampType>(i);
    return keyframe_animation_.SetTimestamps(timestamps_);
  }

  int32_t CreateAndAddAnimationData(int32_t num_frames,
                                    uint32_t num_components) {
    // Create and add animation data with.
    animation_data_.resize(num_frames * num_components);
    for (int i = 0; i < animation_data_.size(); ++i)
      animation_data_[i] = static_cast<float>(i);
    return keyframe_animation_.AddKeyframes(draco::DT_FLOAT32, num_components,
                                            animation_data_);
  }

  template <int num_components_t>
  void CompareAnimationData(const KeyframeAnimation &animation0,
                            const KeyframeAnimation &animation1,
                            bool quantized) {
    ASSERT_EQ(animation0.num_frames(), animation1.num_frames());
    ASSERT_EQ(animation0.num_animations(), animation1.num_animations());

    if (quantized) {
      // TODO(hemmer) : Add test for stable quantization.
      // Quantization will result in slightly different values.
      // Skip comparing values.
      return;
    }

    // Compare time stamp.
    const auto timestamp_att0 = animation0.timestamps();
    const auto timestamp_att1 = animation0.timestamps();
    for (int i = 0; i < animation0.num_frames(); ++i) {
      std::array<float, 1> att_value0;
      std::array<float, 1> att_value1;
      ASSERT_TRUE((timestamp_att0->GetValue<float, 1>(
          draco::AttributeValueIndex(i), &att_value0)));
      ASSERT_TRUE((timestamp_att1->GetValue<float, 1>(
          draco::AttributeValueIndex(i), &att_value1)));
      ASSERT_FLOAT_EQ(att_value0[0], att_value1[0]);
    }

    for (int animation_id = 1; animation_id < animation0.num_animations();
         ++animation_id) {
      // Compare keyframe data.
      const auto keyframe_att0 = animation0.keyframes(animation_id);
      const auto keyframe_att1 = animation1.keyframes(animation_id);
      ASSERT_EQ(keyframe_att0->num_components(),
                keyframe_att1->num_components());
      for (int i = 0; i < animation0.num_frames(); ++i) {
        std::array<float, num_components_t> att_value0;
        std::array<float, num_components_t> att_value1;
        ASSERT_TRUE((keyframe_att0->GetValue<float, num_components_t>(
            draco::AttributeValueIndex(i), &att_value0)));
        ASSERT_TRUE((keyframe_att1->GetValue<float, num_components_t>(
            draco::AttributeValueIndex(i), &att_value1)));
        for (int j = 0; j < att_value0.size(); ++j) {
          ASSERT_FLOAT_EQ(att_value0[j], att_value1[j]);
        }
      }
    }
  }

  template <int num_components_t>
  void TestKeyframeAnimationEncoding() {
    TestKeyframeAnimationEncoding<num_components_t>(false);
  }

  template <int num_components_t>
  void TestKeyframeAnimationEncoding(bool quantized) {
    // Encode animation class.
    draco::EncoderBuffer buffer;
    draco::KeyframeAnimationEncoder encoder;
    EncoderOptions options = EncoderOptions::CreateDefaultOptions();
    if (quantized) {
      // Set quantization for timestamps.
      options.SetAttributeInt(0, "quantization_bits", 20);
      // Set quantization for keyframes.
      for (int i = 1; i <= keyframe_animation_.num_animations(); ++i) {
        options.SetAttributeInt(i, "quantization_bits", 20);
      }
    }

    ASSERT_TRUE(
        encoder.EncodeKeyframeAnimation(keyframe_animation_, options, &buffer)
            .ok());

    draco::DecoderBuffer dec_decoder;
    draco::KeyframeAnimationDecoder decoder;
    DecoderBuffer dec_buffer;
    dec_buffer.Init(buffer.data(), buffer.size());

    // Decode animation class.
    std::unique_ptr<KeyframeAnimation> decoded_animation(
        new KeyframeAnimation());
    DecoderOptions dec_options;
    ASSERT_TRUE(
        decoder.Decode(dec_options, &dec_buffer, decoded_animation.get()).ok());

    // Verify if animation before and after compression is identical.
    CompareAnimationData<num_components_t>(keyframe_animation_,
                                           *decoded_animation, quantized);
  }

  draco::KeyframeAnimation keyframe_animation_;
  std::vector<draco::KeyframeAnimation::TimestampType> timestamps_;
  std::vector<float> animation_data_;
};

TEST_F(KeyframeAnimationEncodingTest, OneComponent) {
  const int num_frames = 1;
  ASSERT_TRUE(CreateAndAddTimestamps(num_frames));
  ASSERT_EQ(CreateAndAddAnimationData(num_frames, 1), 1);
  TestKeyframeAnimationEncoding<1>();
}

TEST_F(KeyframeAnimationEncodingTest, ManyComponents) {
  const int num_frames = 100;
  ASSERT_TRUE(CreateAndAddTimestamps(num_frames));
  ASSERT_EQ(CreateAndAddAnimationData(num_frames, 100), 1);
  TestKeyframeAnimationEncoding<100>();
}

TEST_F(KeyframeAnimationEncodingTest, ManyComponentsWithQuantization) {
  const int num_frames = 100;
  ASSERT_TRUE(CreateAndAddTimestamps(num_frames));
  ASSERT_EQ(CreateAndAddAnimationData(num_frames, 4), 1);
  // Test compression with quantization.
  TestKeyframeAnimationEncoding<4>(true);
}

TEST_F(KeyframeAnimationEncodingTest, MultipleAnimations) {
  const int num_frames = 5;
  ASSERT_TRUE(CreateAndAddTimestamps(num_frames));
  ASSERT_EQ(CreateAndAddAnimationData(num_frames, 3), 1);
  ASSERT_EQ(CreateAndAddAnimationData(num_frames, 3), 2);
  TestKeyframeAnimationEncoding<3>();
}

}  // namespace draco
