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
#ifndef DRACO_SCENE_TRS_MATRIX_H_
#define DRACO_SCENE_TRS_MATRIX_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "Eigen/Geometry"
#include "draco/core/status_or.h"

namespace draco {

// This class is used to store one or more of a translation, rotation, scale
// vectors or a transformation matrix.
class TrsMatrix {
 public:
  TrsMatrix()
      : matrix_(Eigen::Matrix4d::Identity()),
        translation_(0.0, 0.0, 0.0),
        rotation_(1.0, 0.0, 0.0, 0.0),  // (w, x, y, z)
        scale_(1.0, 1.0, 1.0),
        matrix_set_(false),
        translation_set_(false),
        rotation_set_(false),
        scale_set_(false) {}

  void Copy(const TrsMatrix &tm);

  TrsMatrix &SetMatrix(const Eigen::Matrix4d &matrix) {
    matrix_ = matrix;
    matrix_set_ = true;
    return *this;
  }
  bool MatrixSet() const { return matrix_set_; }
  StatusOr<const Eigen::Matrix4d> Matrix() const {
    if (!matrix_set_) {
      return Status(Status::DRACO_ERROR, "Matrix is not set.");
    }
    return matrix_;
  }

  TrsMatrix &SetTranslation(const Eigen::Vector3d &translation) {
    translation_ = translation;
    translation_set_ = true;
    return *this;
  }
  bool TranslationSet() const { return translation_set_; }
  StatusOr<const Eigen::Vector3d> Translation() const {
    if (!translation_set_) {
      return Status(Status::DRACO_ERROR, "Translation is not set.");
    }
    return translation_;
  }

  TrsMatrix &SetRotation(const Eigen::Quaterniond &rotation) {
    rotation_ = rotation;
    rotation_set_ = true;
    return *this;
  }
  bool RotationSet() const { return rotation_set_; }
  StatusOr<const Eigen::Quaterniond> Rotation() const {
    if (!rotation_set_) {
      return Status(Status::DRACO_ERROR, "Rotation is not set.");
    }
    return rotation_;
  }

  TrsMatrix &SetScale(const Eigen::Vector3d &scale) {
    scale_ = scale;
    scale_set_ = true;
    return *this;
  }
  bool ScaleSet() const { return scale_set_; }
  StatusOr<const Eigen::Vector3d> Scale() const {
    if (!scale_set_) {
      return Status(Status::DRACO_ERROR, "Scale is not set.");
    }
    return scale_;
  }

  // Returns true if the matrix is not set or if matrix is set and is equal to
  // identity.
  bool IsMatrixIdentity() const;

  // Returns true if matrix is set and only the translation elements may differ
  // from identity. Returns false if matrix is not set.
  bool IsMatrixTranslationOnly() const;

  // Returns transformation matrix if it has been set. Otherwise, computes
  // transformation matrix from TRS vectors and returns it.
  Eigen::Matrix4d ComputeTransformationMatrix() const;

  // Returns a boolean indicating whether any of the transforms have been set.
  // Can be used to check whether this object represents a default transform.
  bool TransformSet() const {
    return matrix_set_ || translation_set_ || rotation_set_ || scale_set_;
  }

  bool operator==(const TrsMatrix &trs_matrix) const;

 private:
  Eigen::Matrix4d matrix_;
  Eigen::Vector3d translation_;
  Eigen::Quaterniond rotation_;
  Eigen::Vector3d scale_;
  bool matrix_set_;
  bool translation_set_;
  bool rotation_set_;
  bool scale_set_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_SCENE_TRS_MATRIX_H_
