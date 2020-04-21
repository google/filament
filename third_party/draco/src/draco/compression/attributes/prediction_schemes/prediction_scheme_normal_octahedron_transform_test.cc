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
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_encoding_transform.h"
#include "draco/core/draco_test_base.h"

namespace {

class PredictionSchemeNormalOctahedronTransformTest : public ::testing::Test {
 protected:
  typedef draco::PredictionSchemeNormalOctahedronEncodingTransform<int32_t>
      Transform;
  typedef Transform::Point2 Point2;

  void TestComputeCorrection(const Transform &transform, const int32_t &ox,
                             const int32_t &oy, const int32_t &px,
                             const int32_t &py, const int32_t &cx,
                             const int32_t &cy) {
    const int32_t o[2] = {ox + 7, oy + 7};
    const int32_t p[2] = {px + 7, py + 7};
    int32_t corr[2] = {500, 500};
    transform.ComputeCorrection(o, p, corr);
    ASSERT_EQ(corr[0], (cx + 15) % 15);
    ASSERT_EQ(corr[1], (cy + 15) % 15);
  }
};

TEST_F(PredictionSchemeNormalOctahedronTransformTest, Init) {
  const Transform transform(15);
  ASSERT_TRUE(transform.AreCorrectionsPositive());
}

TEST_F(PredictionSchemeNormalOctahedronTransformTest, ComputeCorrections) {
  const Transform transform(15);
  // checks inside diamond
  TestComputeCorrection(transform, 0, 0, 0, 0, 0, 0);
  TestComputeCorrection(transform, 1, 1, 1, 1, 0, 0);
  TestComputeCorrection(transform, 3, 4, 1, 1, 2, 3);
  TestComputeCorrection(transform, -1, -1, -1, -1, 0, 0);
  TestComputeCorrection(transform, -3, -4, -1, -1, -2, -3);
  // checks outside diamond
  TestComputeCorrection(transform, 4, 4, 4, 4, 0, 0);
  TestComputeCorrection(transform, 5, 6, 4, 4, -2, -1);
  TestComputeCorrection(transform, 3, 2, 4, 4, 2, 1);
  // checks on outer edges
  TestComputeCorrection(transform, 7, 7, 4, 4, -3, -3);
  TestComputeCorrection(transform, 6, 7, 4, 4, -3, -2);
  TestComputeCorrection(transform, -6, 7, 4, 4, -3, -2);
  TestComputeCorrection(transform, 7, 6, 4, 4, -2, -3);
  TestComputeCorrection(transform, 7, -6, 4, 4, -2, -3);
}

TEST_F(PredictionSchemeNormalOctahedronTransformTest, Interface) {
  const Transform transform(15);
  ASSERT_EQ(transform.max_quantized_value(), 15);
  ASSERT_EQ(transform.center_value(), 7);
  ASSERT_EQ(transform.quantization_bits(), 4);
}

}  // namespace
