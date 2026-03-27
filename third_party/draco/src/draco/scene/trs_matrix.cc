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

#ifdef DRACO_TRANSCODER_SUPPORTED

namespace draco {

void TrsMatrix::Copy(const TrsMatrix &tm) {
  matrix_ = tm.matrix_;
  translation_ = tm.translation_;
  rotation_ = tm.rotation_;
  scale_ = tm.scale_;
  matrix_set_ = tm.matrix_set_;
  translation_set_ = tm.translation_set_;
  rotation_set_ = tm.rotation_set_;
  scale_set_ = tm.scale_set_;
}

Eigen::Matrix4d TrsMatrix::ComputeTransformationMatrix() const {
  // Return transformation matrix if it has been set.
  if (matrix_set_) {
    return matrix_;
  }

  // Populate translation matrix.
  Eigen::Matrix4d translation_matrix = Eigen::Matrix4d::Identity();
  translation_matrix(0, 3) = translation_[0];
  translation_matrix(1, 3) = translation_[1];
  translation_matrix(2, 3) = translation_[2];

  // Populate rotation matrix using rotation quaternion.
  Eigen::Matrix3d rotation_matrix_3 = rotation_.normalized().toRotationMatrix();

  // Convert the 3x3 matrix to a 4x4 matrix that can be multiplied with the
  // other TRS matrices.
  Eigen::Matrix4d rotation_matrix = Eigen::Matrix4d::Identity();
  rotation_matrix.block<3, 3>(0, 0) = rotation_matrix_3;

  // Populate scale matrix.
  const Eigen::Matrix4d scale_matrix(
      Eigen::Vector4d(scale_.x(), scale_.y(), scale_.z(), 1.0).asDiagonal());

  // Return transformation matrix computed by combining TRS matrices.
  return translation_matrix * rotation_matrix * scale_matrix;
}

bool TrsMatrix::IsMatrixIdentity() const {
  if (!matrix_set_) {
    return true;
  }
  return matrix_ == Eigen::Matrix4d::Identity();
}

bool TrsMatrix::IsMatrixTranslationOnly() const {
  if (!matrix_set_) {
    return false;
  }
  Eigen::Matrix4d translation_check = matrix_;
  translation_check(0, 3) = 0.0;
  translation_check(1, 3) = 0.0;
  translation_check(2, 3) = 0.0;
  return translation_check == Eigen::Matrix4d::Identity();
}

bool TrsMatrix::operator==(const TrsMatrix &trs_matrix) const {
  if (matrix_set_ != trs_matrix.matrix_set_ ||
      translation_set_ != trs_matrix.translation_set_ ||
      rotation_set_ != trs_matrix.rotation_set_ ||
      scale_set_ != trs_matrix.scale_set_) {
    return false;
  }
  if (matrix_set_ && matrix_ != trs_matrix.matrix_) {
    return false;
  }
  if (translation_set_ && translation_ != trs_matrix.translation_) {
    return false;
  }
  if (rotation_set_ && rotation_ != trs_matrix.rotation_) {
    return false;
  }
  if (scale_set_ && scale_set_ != trs_matrix.scale_set_) {
    return false;
  }
  return true;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
