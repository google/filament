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
#include "draco/scene/trs_matrix.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(TrsMatrixTest, TestIsMatrixIdentity) {
  draco::TrsMatrix trs;
  ASSERT_EQ(trs.MatrixSet(), false);
  ASSERT_EQ(trs.IsMatrixIdentity(), true);

  // clang-format off
  Eigen::Matrix4d matrix;
  matrix << 1,  2,  3,  4,
            5,  6,  7,  8,
            9, 10, 11, 12,
           13, 14, 15, 16;
  // clang-format on
  trs.SetMatrix(matrix);
  ASSERT_EQ(trs.MatrixSet(), true);
  ASSERT_EQ(trs.IsMatrixIdentity(), false);

  trs.SetMatrix(Eigen::Matrix4d::Identity());
  ASSERT_EQ(trs.MatrixSet(), true);
  ASSERT_EQ(trs.IsMatrixIdentity(), true);
}

TEST(TrsMatrixTest, TestIsMatrixTranslationOnly) {
  draco::TrsMatrix trs;
  ASSERT_EQ(trs.MatrixSet(), false);
  ASSERT_EQ(trs.IsMatrixTranslationOnly(), false);

  trs.SetMatrix(Eigen::Matrix4d::Identity());
  ASSERT_EQ(trs.MatrixSet(), true);
  ASSERT_EQ(trs.IsMatrixTranslationOnly(), true);

  // clang-format off
  Eigen::Matrix4d matrix;
  matrix << 1,  2,  3,  4,
            5,  6,  7,  8,
            9, 10, 11, 12,
           13, 14, 15, 16;
  // clang-format on
  trs.SetMatrix(matrix);
  ASSERT_EQ(trs.MatrixSet(), true);
  ASSERT_EQ(trs.IsMatrixTranslationOnly(), false);

  // clang-format off
  Eigen::Matrix4d translation;
  translation << 1, 0, 0, 1,
                 0, 1, 0, 2,
                 0, 0, 1, 3,
                 0, 0, 0, 1;
  // clang-format on
  trs.SetMatrix(translation);
  ASSERT_EQ(trs.MatrixSet(), true);
  ASSERT_EQ(trs.IsMatrixTranslationOnly(), true);
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
