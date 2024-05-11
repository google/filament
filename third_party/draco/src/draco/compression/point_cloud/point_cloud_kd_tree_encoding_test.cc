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
#include "draco/compression/point_cloud/point_cloud_kd_tree_decoder.h"
#include "draco/compression/point_cloud/point_cloud_kd_tree_encoder.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/core/vector_d.h"
#include "draco/io/obj_decoder.h"
#include "draco/point_cloud/point_cloud_builder.h"

namespace draco {

class PointCloudKdTreeEncodingTest : public ::testing::Test {
 protected:
  void ComparePointClouds(const PointCloud &p0, const PointCloud &p1) const {
    ASSERT_EQ(p0.num_points(), p1.num_points());
    ASSERT_EQ(p0.num_attributes(), p1.num_attributes());
    // Currently works only with one attribute.
    ASSERT_EQ(p0.num_attributes(), p1.num_attributes());
    for (auto index = 0; index < p0.num_attributes(); index += 1) {
      ASSERT_EQ(p0.attribute(index)->num_components(),
                p1.attribute(index)->num_components());
      std::vector<double> points_0, points_1;
      std::vector<double> att_entry_0(p0.attribute(index)->num_components());
      std::vector<double> att_entry_1(p0.attribute(index)->num_components());
      for (PointIndex i(0); i < p0.num_points(); ++i) {
        p0.attribute(index)->ConvertValue(p0.attribute(index)->mapped_index(i),
                                          &att_entry_0[0]);
        p1.attribute(index)->ConvertValue(p1.attribute(index)->mapped_index(i),
                                          &att_entry_1[0]);
        for (int d = 0; d < p0.attribute(index)->num_components(); ++d) {
          points_0.push_back(att_entry_0[d]);
          points_1.push_back(att_entry_1[d]);
        }
      }
      // To compare the point clouds we sort points components from both inputs
      // separately, and then we compare all matching coordinates one by one.
      // TODO(ostava): Note that this is not guaranteed to work for quantized
      // point clouds because the order of points may actually change because
      // of the quantization. The test should be make more robust to handle such
      // case.
      std::sort(points_0.begin(), points_0.end());
      std::sort(points_1.begin(), points_1.end());
      for (uint32_t i = 0; i < points_0.size(); ++i) {
        ASSERT_LE(std::fabs(points_0[i] - points_1[i]), 1e-2);
      }
    }
  }

  void TestKdTreeEncoding(const PointCloud &pc) {
    EncoderBuffer buffer;
    PointCloudKdTreeEncoder encoder;
    EncoderOptions options = EncoderOptions::CreateDefaultOptions();
    options.SetGlobalInt("quantization_bits", 16);
    for (int compression_level = 0; compression_level <= 6;
         ++compression_level) {
      options.SetSpeed(10 - compression_level, 10 - compression_level);
      encoder.SetPointCloud(pc);
      ASSERT_TRUE(encoder.Encode(options, &buffer).ok());

      DecoderBuffer dec_buffer;
      dec_buffer.Init(buffer.data(), buffer.size());
      PointCloudKdTreeDecoder decoder;

      std::unique_ptr<PointCloud> out_pc(new PointCloud());
      DecoderOptions dec_options;
      ASSERT_TRUE(decoder.Decode(dec_options, &dec_buffer, out_pc.get()).ok());

      ComparePointClouds(pc, *out_pc);
    }
  }

  void TestFloatEncoding(const std::string &file_name) {
    std::unique_ptr<PointCloud> pc = ReadPointCloudFromTestFile(file_name);
    ASSERT_NE(pc, nullptr);

    TestKdTreeEncoding(*pc);
  }
};

TEST_F(PointCloudKdTreeEncodingTest, TestFloatKdTreeEncoding) {
  TestFloatEncoding("cube_subd.obj");
}

TEST_F(PointCloudKdTreeEncodingTest, TestIntKdTreeEncoding) {
  constexpr int num_points = 120;
  std::vector<std::array<uint32_t, 3>> points(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 3> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127);
    pos[1] = 13 * ((i * 3) % 321);
    pos[2] = 29 * ((i * 19) % 450);
    points[i] = pos;
  }

  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id, PointIndex(i),
                                      &(points[i.value()])[0]);
  }
  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

// test higher dimensions with more attributes
TEST_F(PointCloudKdTreeEncodingTest, TestIntKdTreeEncodingHigherDimension) {
  constexpr int num_points = 120;
  std::vector<std::array<uint32_t, 3>> points3(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 3> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127);
    pos[1] = 13 * ((i * 3) % 321);
    pos[2] = 29 * ((i * 19) % 450);
    points3[i] = pos;
  }
  std::vector<std::array<uint32_t, 2>> points2(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 2> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 1;
    pos[1] = 13 * ((i * 3) % 321) + 1;
    points2[i] = pos;
  }
  std::vector<std::array<uint32_t, 1>> points1(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 1> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 11;
    points1[i] = pos;
  }

  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id3 =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id3, PointIndex(i),
                                      &(points3[i.value()])[0]);
  }
  const int att_id2 =
      builder.AddAttribute(GeometryAttribute::POSITION, 2, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id2, PointIndex(i),
                                      &(points2[i.value()])[0]);
  }
  const int att_id1 =
      builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id1, PointIndex(i),
                                      &(points1[i.value()])[0]);
  }

  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

// Test 16 and 8 bit encoding.
TEST_F(PointCloudKdTreeEncodingTest,
       TestIntKdTreeEncodingHigherDimensionVariedTypes) {
  constexpr int num_points = 120;
  std::vector<std::array<uint32_t, 3>> points3(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 3> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127);
    pos[1] = 13 * ((i * 3) % 321);
    pos[2] = 29 * ((i * 19) % 450);
    points3[i] = pos;
  }
  std::vector<std::array<uint16_t, 2>> points2(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint16_t, 2> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 1;
    pos[1] = 13 * ((i * 3) % 321) + 1;
    points2[i] = pos;
  }
  std::vector<std::array<uint8_t, 1>> points1(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint8_t, 1> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 11;
    points1[i] = pos;
  }

  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id3 =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id3, PointIndex(i),
                                      &(points3[i.value()])[0]);
  }
  const int att_id2 =
      builder.AddAttribute(GeometryAttribute::POSITION, 2, DT_UINT16);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id2, PointIndex(i),
                                      &(points2[i.value()])[0]);
  }
  const int att_id1 =
      builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_UINT8);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id1, PointIndex(i),
                                      &(points1[i.value()])[0]);
  }

  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

// Test 16 only encoding for one attribute.
TEST_F(PointCloudKdTreeEncodingTest, TestIntKdTreeEncoding16Bit) {
  constexpr int num_points = 120;
  std::vector<std::array<uint16_t, 3>> points3(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint16_t, 3> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127);
    pos[1] = 13 * ((i * 3) % 321);
    pos[2] = 29 * ((i * 19) % 450);
    points3[i] = pos;
  }

  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id3 =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_UINT16);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id3, PointIndex(i),
                                      &(points3[i.value()])[0]);
  }

  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

// Test 16 and 8 bit encoding with size bigger than 32bit encoding.
TEST_F(PointCloudKdTreeEncodingTest,
       TestIntKdTreeEncodingHigherDimensionVariedTypesBig16BitEncoding) {
  constexpr int num_points = 120;
  std::vector<std::array<uint32_t, 3>> points3(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 3> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127);
    pos[1] = 13 * ((i * 3) % 321);
    pos[2] = 29 * ((i * 19) % 450);
    points3[i] = pos;
  }
  // The total size of the 16bit encoding must be bigger than the total size of
  // the 32bit encoding.
  std::vector<std::array<uint16_t, 7>> points7(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint16_t, 7> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 1;
    pos[1] = 13 * ((i * 3) % 321) + 1;
    pos[2] = pos[0] + 13;
    pos[3] = pos[2] + 13;
    pos[4] = pos[3] + 13;
    pos[5] = pos[4] + 13;
    pos[6] = pos[5] + 13;
    points7[i] = pos;
  }
  std::vector<std::array<uint8_t, 1>> points1(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint8_t, 1> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 11;
    points1[i] = pos;
  }

  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id3 =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id3, PointIndex(i),
                                      &(points3[i.value()])[0]);
  }
  const int att_id2 =
      builder.AddAttribute(GeometryAttribute::POSITION, 7, DT_UINT16);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id2, PointIndex(i),
                                      &(points7[i.value()])[0]);
  }
  const int att_id1 =
      builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_UINT8);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id1, PointIndex(i),
                                      &(points1[i.value()])[0]);
  }

  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

// Test encoding of quantized values.
TEST_F(PointCloudKdTreeEncodingTest,
       TestIntKdTreeEncodingHigherDimensionFloatTypes) {
  constexpr int num_points = 130;
  std::vector<std::array<uint32_t, 3>> points3(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 3> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 125);
    pos[1] = 13 * ((i * 3) % 334);
    pos[2] = 29 * ((i * 19) % 470);
    points3[i] = pos;
  }
  std::vector<std::array<float, 2>> points_float(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<float, 2> pos;
    // Generate some pseudo-random points.
    pos[0] = static_cast<float>(8 * ((i * 7) % 127) + 1) / 2.5f;
    pos[1] = static_cast<float>(13 * ((i * 3) % 321) + 1) / 3.2f;
    points_float[i] = pos;
  }

  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id3 =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id3, PointIndex(i),
                                      &(points3[i.value()])[0]);
  }
  const int att_id_float =
      builder.AddAttribute(GeometryAttribute::GENERIC, 2, DT_FLOAT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id_float, PointIndex(i),
                                      &(points_float[i.value()])[0]);
  }

  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

// Test encoding of signed integer values
TEST_F(PointCloudKdTreeEncodingTest, TestIntKdTreeEncodingSignedTypes) {
  constexpr int num_points = 120;
  std::vector<std::array<uint32_t, 3>> points3(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, 3> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127);
    pos[1] = 13 * ((i * 3) % 321);
    pos[2] = 29 * ((i * 19) % 450);
    points3[i] = pos;
  }
  std::vector<std::array<int32_t, 2>> points2(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<int32_t, 2> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 1;
    if (i % 3 == 0) {
      pos[0] = -pos[0];
    }
    pos[1] = 13 * ((i * 3) % 321) + 1;
    points2[i] = pos;
  }
  std::vector<std::array<int16_t, 1>> points1(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<int16_t, 1> pos;
    // Generate some pseudo-random points.
    pos[0] = 8 * ((i * 7) % 127) + 11;
    if (i % 5 == 0) {
      pos[0] = -pos[0];
    }
    points1[i] = pos;
  }

  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id3 =
      builder.AddAttribute(GeometryAttribute::POSITION, 3, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id3, PointIndex(i),
                                      &(points3[i.value()])[0]);
  }
  const int att_id2 =
      builder.AddAttribute(GeometryAttribute::POSITION, 2, DT_INT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id2, PointIndex(i),
                                      &(points2[i.value()])[0]);
  }

  const int att_id1 =
      builder.AddAttribute(GeometryAttribute::GENERIC, 1, DT_INT16);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id1, PointIndex(i),
                                      &(points1[i.value()])[0]);
  }

  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

// Test encoding of integer point clouds with > 16 dimensions.
TEST_F(PointCloudKdTreeEncodingTest, TestIntKdTreeEncodingHighDimensional) {
  constexpr int num_points = 120;
  constexpr int num_dims = 42;
  std::vector<std::array<uint32_t, num_dims>> points(num_points);
  for (int i = 0; i < num_points; ++i) {
    std::array<uint32_t, num_dims> pos;
    // Generate some pseudo-random points.
    for (int d = 0; d < num_dims; ++d) {
      pos[d] = 8 * ((i + d) * (7 + (d % 4)) % (127 + d % 3));
    }
    points[i] = pos;
  }
  PointCloudBuilder builder;
  builder.Start(num_points);
  const int att_id =
      builder.AddAttribute(GeometryAttribute::POSITION, num_dims, DT_UINT32);
  for (PointIndex i(0); i < num_points; ++i) {
    builder.SetAttributeValueForPoint(att_id, PointIndex(i),
                                      &(points[i.value()])[0]);
  }

  std::unique_ptr<PointCloud> pc = builder.Finalize(false);
  ASSERT_NE(pc, nullptr);

  TestKdTreeEncoding(*pc);
}

}  // namespace draco
