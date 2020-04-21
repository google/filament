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
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_encoding_transform.h"
#include "draco/core/draco_test_base.h"

namespace {

class PredictionSchemeNormalOctahedronCanonicalizedTransformTest
    : public ::testing::Test {
 protected:
  typedef draco::PredictionSchemeNormalOctahedronCanonicalizedEncodingTransform<
      int32_t>
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

  void TestGetRotationCount(const Transform &transform, const Point2 &pred,
                            const int32_t &rot_dir) {
    const int32_t rotation_count = transform.GetRotationCount(pred);
    ASSERT_EQ(rot_dir, rotation_count);
  }

  void TestRotateRepresentation(const Transform &transform, const Point2 &org,
                                const Point2 &pred, const Point2 &rot_org,
                                const Point2 &rot_pred) {
    const int32_t rotation_count = transform.GetRotationCount(pred);
    const Point2 res_org = transform.RotatePoint(org, rotation_count);
    const Point2 res_pred = transform.RotatePoint(pred, rotation_count);
    ASSERT_EQ(rot_org[0], res_org[0]);
    ASSERT_EQ(rot_org[1], res_org[1]);
    ASSERT_EQ(rot_pred[0], res_pred[0]);
    ASSERT_EQ(rot_pred[1], res_pred[1]);
  }
};

TEST_F(PredictionSchemeNormalOctahedronCanonicalizedTransformTest, Init) {
  const Transform transform(15);
  ASSERT_TRUE(transform.AreCorrectionsPositive());
}

TEST_F(PredictionSchemeNormalOctahedronCanonicalizedTransformTest,
       IsInBottomLeft) {
  const Transform transform(15);
  ASSERT_TRUE(transform.IsInBottomLeft(Point2(0, 0)));
  ASSERT_TRUE(transform.IsInBottomLeft(Point2(-1, -1)));
  ASSERT_TRUE(transform.IsInBottomLeft(Point2(-7, -7)));

  ASSERT_FALSE(transform.IsInBottomLeft(Point2(1, 1)));
  ASSERT_FALSE(transform.IsInBottomLeft(Point2(7, 7)));
  ASSERT_FALSE(transform.IsInBottomLeft(Point2(-1, 1)));
  ASSERT_FALSE(transform.IsInBottomLeft(Point2(-7, 7)));
  ASSERT_FALSE(transform.IsInBottomLeft(Point2(1, -1)));
  ASSERT_FALSE(transform.IsInBottomLeft(Point2(7, -7)));
}

TEST_F(PredictionSchemeNormalOctahedronCanonicalizedTransformTest,
       GetRotationCount) {
  const Transform transform(15);
  TestGetRotationCount(transform, Point2(1, 2), 2);    // top right
  TestGetRotationCount(transform, Point2(-1, 2), 3);   // top left
  TestGetRotationCount(transform, Point2(1, -2), 1);   // bottom right
  TestGetRotationCount(transform, Point2(-1, -2), 0);  // bottom left
  TestGetRotationCount(transform, Point2(0, 2), 3);    // top left
  TestGetRotationCount(transform, Point2(0, -2), 1);   // bottom right
  TestGetRotationCount(transform, Point2(2, 0), 2);    // top right
  TestGetRotationCount(transform, Point2(-2, 0), 0);   // bottom left
  TestGetRotationCount(transform, Point2(0, 0), 0);    // bottom left
}

TEST_F(PredictionSchemeNormalOctahedronCanonicalizedTransformTest,
       RotateRepresentation) {
  const Transform transform(15);
  // p top left; shift clockwise by 3
  TestRotateRepresentation(transform, Point2(1, 2), Point2(-3, 1),
                           Point2(-2, 1), Point2(-1, -3));  // q top right
  TestRotateRepresentation(transform, Point2(-1, -2), Point2(-3, 1),
                           Point2(2, -1), Point2(-1, -3));  // q bottom left
  TestRotateRepresentation(transform, Point2(1, -2), Point2(-3, 1),
                           Point2(2, 1), Point2(-1, -3));  // q bottom right
  TestRotateRepresentation(transform, Point2(-1, 2), Point2(-3, 1),
                           Point2(-2, -1), Point2(-1, -3));  // q top left
  // p top right; shift clockwise by 2 (flip)
  TestRotateRepresentation(transform, Point2(1, 1), Point2(1, 3),
                           Point2(-1, -1), Point2(-1, -3));  // q top right
  TestRotateRepresentation(transform, Point2(-1, -2), Point2(1, 3),
                           Point2(1, 2), Point2(-1, -3));  // q bottom left
  TestRotateRepresentation(transform, Point2(-1, 2), Point2(1, 3),
                           Point2(1, -2), Point2(-1, -3));  // q top left
  TestRotateRepresentation(transform, Point2(1, -2), Point2(1, 3),
                           Point2(-1, 2), Point2(-1, -3));  // q bottom right
  // p bottom right; shift clockwise by 1
  TestRotateRepresentation(transform, Point2(1, 2), Point2(3, -1),
                           Point2(2, -1), Point2(-1, -3));  // q top right
  TestRotateRepresentation(transform, Point2(1, -2), Point2(3, -1),
                           Point2(-2, -1), Point2(-1, -3));  // q bottom right
  TestRotateRepresentation(transform, Point2(-1, -2), Point2(3, -1),
                           Point2(-2, 1), Point2(-1, -3));  // q bottom left
  TestRotateRepresentation(transform, Point2(-1, 2), Point2(3, -1),
                           Point2(2, 1), Point2(-1, -3));  // q top left
  // p bottom left; no change
  TestRotateRepresentation(transform, Point2(1, 2), Point2(-1, -3),
                           Point2(1, 2), Point2(-1, -3));  // q top right
  TestRotateRepresentation(transform, Point2(-1, 2), Point2(-1, -3),
                           Point2(-1, 2), Point2(-1, -3));  // q top left
  TestRotateRepresentation(transform, Point2(1, -2), Point2(-1, -3),
                           Point2(1, -2), Point2(-1, -3));  // q bottom right
  TestRotateRepresentation(transform, Point2(-1, -2), Point2(-1, -3),
                           Point2(-1, -2), Point2(-1, -3));  // q bottom left
}

TEST_F(PredictionSchemeNormalOctahedronCanonicalizedTransformTest,
       ComputeCorrection) {
  const Transform transform(15);
  TestComputeCorrection(transform, 0, 0, 0, 0, 0, 0);
  TestComputeCorrection(transform, 1, 1, 1, 1, 0, 0);
  // inside diamond; p top right
  TestComputeCorrection(transform, 3, 4, 1, 2, -2, -2);  // q top right
  TestComputeCorrection(transform, -3, 4, 1, 2, 4, -2);  // q top left
  TestComputeCorrection(transform, 3, -4, 1, 2, -2, 6);  // q bottom right
  TestComputeCorrection(transform, -3, -4, 1, 2, 4, 6);  // q bottom left
  // inside diamond; p top left
  TestComputeCorrection(transform, 3, 4, -1, 2, -2, 4);    // q top right
  TestComputeCorrection(transform, -3, 4, -1, 2, -2, -2);  // q top left
  TestComputeCorrection(transform, 3, -4, -1, 2, 6, 4);    // q bottom right
  TestComputeCorrection(transform, -3, -4, -1, 2, 6, -2);  // q bottom left
  // inside diamond; p bottom right
  TestComputeCorrection(transform, 3, 4, 1, -2, 6, -2);    // q top right
  TestComputeCorrection(transform, -3, 4, 1, -2, 6, 4);    // q top left
  TestComputeCorrection(transform, 3, -4, 1, -2, -2, -2);  // q bottom right
  TestComputeCorrection(transform, -3, -4, 1, -2, -2, 4);  // q bottom left
  // inside diamond; p bottom left
  TestComputeCorrection(transform, 3, 4, -1, -2, 4, 6);      // q top right
  TestComputeCorrection(transform, -3, 4, -1, -2, -2, 6);    // q top left
  TestComputeCorrection(transform, 3, -4, -1, -2, 4, -2);    // q bottom right
  TestComputeCorrection(transform, -3, -4, -1, -2, -2, -2);  // q bottom left
  // outside diamond; p top right
  TestComputeCorrection(transform, 1, 2, 5, 4, -2, -4);    // q top right
  TestComputeCorrection(transform, -1, 2, 5, 4, -7, -4);   // q top left
  TestComputeCorrection(transform, 1, -2, 5, 4, -2, -7);   // q bottom right
  TestComputeCorrection(transform, -1, -2, 5, 4, -7, -7);  // q bottom left
  // outside diamond; p top left
  TestComputeCorrection(transform, 1, 2, -5, 4, -4, -7);    // q top right
  TestComputeCorrection(transform, -1, 2, -5, 4, -4, -2);   // q top left
  TestComputeCorrection(transform, 1, -2, -5, 4, -7, -7);   // q bottom right
  TestComputeCorrection(transform, -1, -2, -5, 4, -7, -2);  // q bottom left
  // outside diamond; p bottom right
  TestComputeCorrection(transform, 1, 2, 5, -4, -7, -2);    // q top right
  TestComputeCorrection(transform, -1, 2, 5, -4, -7, -7);   // q top left
  TestComputeCorrection(transform, 1, -2, 5, -4, -4, -2);   // q bottom right
  TestComputeCorrection(transform, -1, -2, 5, -4, -4, -7);  // q bottom left
  // outside diamond; p bottom left
  TestComputeCorrection(transform, 1, 2, -5, -4, -7, -7);    // q top right
  TestComputeCorrection(transform, -1, 2, -5, -4, -2, -7);   // q top left
  TestComputeCorrection(transform, 1, -2, -5, -4, -7, -4);   // q bottom right
  TestComputeCorrection(transform, -1, -2, -5, -4, -2, -4);  // q bottom left

  TestComputeCorrection(transform, -1, -2, 7, 7, -5, -6);
  TestComputeCorrection(transform, 0, 0, 7, 7, 7, 7);
  TestComputeCorrection(transform, -1, -2, 0, -2, 0, 1);
}

TEST_F(PredictionSchemeNormalOctahedronCanonicalizedTransformTest, Interface) {
  const Transform transform(15);
  ASSERT_EQ(transform.max_quantized_value(), 15);
  ASSERT_EQ(transform.center_value(), 7);
  ASSERT_EQ(transform.quantization_bits(), 4);
}

}  // namespace
