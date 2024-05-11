// Copyright 2016 The Draco Authors.
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
#include "draco/point_cloud/point_cloud_builder.h"

#include "draco/core/draco_test_base.h"

namespace draco {

class PointCloudBuilderTest : public ::testing::Test {
 protected:
  // Test data.
  // clang-format off
  std::vector<float> pos_data_ = {10.f, 0.f, 1.f,
                                  11.f, 1.f, 2.f,
                                  12.f, 2.f, 8.f,
                                  13.f, 4.f, 7.f,
                                  14.f, 5.f, 6.f,
                                  15.f, 6.f, 5.f,
                                  16.f, 1.f, 3.f,
                                  17.f, 1.f, 2.f,
                                  11.f, 1.f, 2.f,
                                  10.f, 0.f, 1.f};
  std::vector<int16_t> intensity_data_ = {100,
                                          200,
                                          500,
                                          700,
                                          400,
                                          400,
                                          400,
                                          100,
                                          100,
                                          100};
  // clang-format on
};

TEST_F(PointCloudBuilderTest, IndividualTest_NoDedup) {
  // This test verifies that PointCloudBuilder can construct point cloud using
  // SetAttributeValueForPoint API without deduplication.
  PointCloudBuilder builder;
  builder.Start(10);
  const int pos_att_id =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  const int intensity_att_id =
      builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_INT16);
  for (PointIndex i(0); i < 10; ++i) {
    builder.SetAttributeValueForPoint(pos_att_id, i,
                                      pos_data_.data() + 3 * i.value());
    builder.SetAttributeValueForPoint(intensity_att_id, i,
                                      intensity_data_.data() + i.value());
  }
  std::unique_ptr<PointCloud> res = builder.Finalize(false);
  ASSERT_TRUE(res != nullptr);
  ASSERT_EQ(res->num_points(), 10);
}

TEST_F(PointCloudBuilderTest, IndividualTest_Dedup) {
  // This test verifies that PointCloudBuilder can construct point cloud using
  // SetAttributeValueForPoint API with deduplication.
  PointCloudBuilder builder;
  builder.Start(10);
  const int pos_att_id =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  const int intensity_att_id =
      builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_INT16);
  for (PointIndex i(0); i < 10; ++i) {
    builder.SetAttributeValueForPoint(pos_att_id, i,
                                      pos_data_.data() + 3 * i.value());
    builder.SetAttributeValueForPoint(intensity_att_id, i,
                                      intensity_data_.data() + i.value());
  }
  std::unique_ptr<PointCloud> res = builder.Finalize(true);
  ASSERT_TRUE(res != nullptr);
  ASSERT_EQ(res->num_points(), 9);
}

TEST_F(PointCloudBuilderTest, BatchTest) {
  // This test verifies that PointCloudBuilder can construct point cloud using
  // SetAttributeValuesForAllPoints API.
  PointCloudBuilder builder;
  builder.Start(10);
  const int pos_att_id =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
  const int intensity_att_id =
      builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_INT16);
  builder.SetAttributeValuesForAllPoints(pos_att_id, pos_data_.data(), 0);
  builder.SetAttributeValuesForAllPoints(intensity_att_id,
                                         intensity_data_.data(), 0);
  std::unique_ptr<PointCloud> res = builder.Finalize(false);
  ASSERT_TRUE(res != nullptr);
  ASSERT_EQ(res->num_points(), 10);
  for (PointIndex i(0); i < 10; ++i) {
    float pos_val[3];
    res->attribute(pos_att_id)->GetMappedValue(i, pos_val);
    for (int c = 0; c < 3; ++c) {
      ASSERT_EQ(pos_val[c], pos_data_[3 * i.value() + c]);
    }
    int16_t int_val;
    res->attribute(intensity_att_id)->GetMappedValue(i, &int_val);
    ASSERT_EQ(intensity_data_[i.value()], int_val);
  }
}

TEST_F(PointCloudBuilderTest, MultiUse) {
  // This test verifies that PointCloudBuilder can be used multiple times
  PointCloudBuilder builder;
  {
    builder.Start(10);
    const int pos_att_id =
        builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
    const int intensity_att_id =
        builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_INT16);
    builder.SetAttributeValuesForAllPoints(pos_att_id, pos_data_.data(), 0);
    builder.SetAttributeValuesForAllPoints(intensity_att_id,
                                           intensity_data_.data(), 0);
    std::unique_ptr<PointCloud> res = builder.Finalize(false);
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->num_points(), 10);
    for (PointIndex i(0); i < 10; ++i) {
      float pos_val[3];
      res->attribute(pos_att_id)->GetMappedValue(i, pos_val);
      for (int c = 0; c < 3; ++c) {
        ASSERT_EQ(pos_val[c], pos_data_[3 * i.value() + c]);
      }
      int16_t int_val;
      res->attribute(intensity_att_id)->GetMappedValue(i, &int_val);
      ASSERT_EQ(intensity_data_[i.value()], int_val);
    }
  }

  {
    // Use only a sub-set of data (offsetted to avoid possible reuse of old
    // data).
    builder.Start(4);
    const int pos_att_id =
        builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_FLOAT32);
    const int intensity_att_id =
        builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_INT16);
    constexpr int offset = 5;
    builder.SetAttributeValuesForAllPoints(pos_att_id,
                                           pos_data_.data() + 3 * offset, 0);
    builder.SetAttributeValuesForAllPoints(intensity_att_id,
                                           intensity_data_.data() + offset, 0);
    std::unique_ptr<PointCloud> res = builder.Finalize(false);
    ASSERT_TRUE(res != nullptr);
    ASSERT_EQ(res->num_points(), 4);
    for (PointIndex i(0); i < 4; ++i) {
      float pos_val[3];
      res->attribute(pos_att_id)->GetMappedValue(i, pos_val);
      for (int c = 0; c < 3; ++c) {
        ASSERT_EQ(pos_val[c], pos_data_[3 * (i.value() + offset) + c]);
      }
      int16_t int_val;
      res->attribute(intensity_att_id)->GetMappedValue(i, &int_val);
      ASSERT_EQ(intensity_data_[i.value() + offset], int_val);
    }
  }
}

}  // namespace draco
