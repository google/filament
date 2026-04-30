// Copyright 2022 The Draco Authors.
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
#include "draco/scene/instance_array.h"

#include <limits>
#include <utility>

#include "draco/core/constants.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(InstanceArrayTest, TestInstance) {
  // Test construction of an empty draco::InstanceArray::Instance struct.
  const draco::InstanceArray::Instance instance;
  ASSERT_FALSE(instance.trs.TranslationSet());
  ASSERT_FALSE(instance.trs.RotationSet());
  ASSERT_FALSE(instance.trs.ScaleSet());
  ASSERT_FALSE(instance.trs.MatrixSet());
}

TEST(InstanceArrayTest, TestDefaults) {
  // Test construction of an empty draco::InstanceArray object.
  const draco::InstanceArray array;
  ASSERT_EQ(array.NumInstances(), 0);
}

TEST(InstanceArrayTest, TestAddInstance) {
  // Test population of draco::InstanceArray object with instances.
  draco::InstanceArray array;

  // Create an instance and set its transformation TRS vectors.
  const Eigen::Vector3d translation_0(1.0, 2.0, 3.0);
  const Eigen::Quaterniond rotation_0(4.0, 5.0, 6.0, 7.0);
  const Eigen::Vector3d scale_0(8.0, 9.0, 10.0);
  draco::InstanceArray::Instance instance_0;
  instance_0.trs.SetTranslation(translation_0);
  instance_0.trs.SetRotation(rotation_0);
  instance_0.trs.SetScale(scale_0);

  // Create another instance.
  const Eigen::Vector3d translation_1(1.1, 2.1, 3.1);
  const Eigen::Quaterniond rotation_1(4.1, 5.1, 6.1, 7.1);
  const Eigen::Vector3d scale_1(8.1, 9.1, 10.1);
  draco::InstanceArray::Instance instance_1;
  instance_1.trs.SetTranslation(translation_1);
  instance_1.trs.SetRotation(rotation_1);
  instance_1.trs.SetScale(scale_1);

  // Add two instances to instance array.
  DRACO_ASSERT_OK(array.AddInstance(instance_0));
  DRACO_ASSERT_OK(array.AddInstance(instance_1));

  // Check that the instances have been added.
  ASSERT_EQ(array.NumInstances(), 2);

  // Check transformation of the first instance.
  const draco::TrsMatrix &trs_0 = array.GetInstance(0).trs;
  ASSERT_TRUE(trs_0.TranslationSet());
  ASSERT_TRUE(trs_0.RotationSet());
  ASSERT_TRUE(trs_0.ScaleSet());
  ASSERT_FALSE(trs_0.MatrixSet());
  ASSERT_EQ(trs_0.Translation().value(), translation_0);
  ASSERT_EQ(trs_0.Rotation().value(), rotation_0);
  ASSERT_EQ(trs_0.Scale().value(), scale_0);

  // Check transformation of the second instance.
  const draco::TrsMatrix &trs_1 = array.GetInstance(1).trs;
  ASSERT_TRUE(trs_1.TranslationSet());
  ASSERT_TRUE(trs_1.RotationSet());
  ASSERT_TRUE(trs_1.ScaleSet());
  ASSERT_FALSE(trs_1.MatrixSet());
  ASSERT_EQ(trs_1.Translation().value(), translation_1);
  ASSERT_EQ(trs_1.Rotation().value(), rotation_1);
  ASSERT_EQ(trs_1.Scale().value(), scale_1);
}

TEST(InstanceArrayTest, TestAddInstanceWithoutTransform) {
  // Test that instance without any transformation can be added.
  draco::InstanceArray array;

  // Do not set any transformation.
  draco::InstanceArray::Instance instance;

  // Check that such instance can be added.
  DRACO_ASSERT_OK(array.AddInstance(instance));
}

TEST(InstanceArrayTest, TestAddInstanceWithoutScale) {
  // Test that instance without scale can be added.
  draco::InstanceArray array;

  // Set only instance translation and rotation.
  draco::InstanceArray::Instance instance;
  instance.trs.SetTranslation(Eigen::Vector3d(1.0, 2.0, 3.0));
  instance.trs.SetRotation(Eigen::Quaterniond(4.0, 5.0, 6.0, 7.0));

  // Check that such instance can be added.
  DRACO_ASSERT_OK(array.AddInstance(instance));
}

TEST(InstanceArrayTest, TestAddInstanceWithMatrixFails) {
  // Test that instance without scale cannot be added.
  draco::InstanceArray array;

  // Set TRS vectors, as well as the matrix.
  draco::InstanceArray::Instance instance;
  instance.trs.SetTranslation(Eigen::Vector3d(1.0, 2.0, 3.0));
  instance.trs.SetRotation(Eigen::Quaterniond(4.0, 5.0, 6.0, 7.0));
  instance.trs.SetScale(Eigen::Vector3d(8.0, 9.0, 10.0));
  // clang-format off
  Eigen::Matrix4d matrix;
  matrix <<  1.0,  2.0,  3.0,  4.0,
             5.0,  6.0,  7.0,  8.0,
             9.0, 10.0, 11.0, 12.0,
            13.0, 14.0, 15.0, 16.0;
  // clang-format on
  instance.trs.SetMatrix(matrix);

  // Check that such instance cannot be added.
  const draco::Status status = array.AddInstance(instance);
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.error_msg_string(), "Instance must have no matrix set.");
}

TEST(InstanceArrayTest, TestCopy) {
  // Test copying of draco::InstanceArray object.
  draco::InstanceArray array;

  // Create an instance and set its transformation TRS vectors.
  const Eigen::Vector3d translation_0(1.0, 2.0, 3.0);
  const Eigen::Quaterniond rotation_0(4.0, 5.0, 6.0, 7.0);
  const Eigen::Vector3d scale_0(8.0, 9.0, 10.0);
  draco::InstanceArray::Instance instance_0;
  instance_0.trs.SetTranslation(translation_0);
  instance_0.trs.SetRotation(rotation_0);
  instance_0.trs.SetScale(scale_0);

  // Create another instance.
  const Eigen::Vector3d translation_1(1.1, 2.1, 3.1);
  const Eigen::Quaterniond rotation_1(4.1, 5.1, 6.1, 7.1);
  const Eigen::Vector3d scale_1(8.1, 9.1, 10.1);
  draco::InstanceArray::Instance instance_1;
  instance_1.trs.SetTranslation(translation_1);
  instance_1.trs.SetRotation(rotation_1);
  instance_1.trs.SetScale(scale_1);

  // Add two instances to the instance array.
  DRACO_ASSERT_OK(array.AddInstance(instance_0));
  DRACO_ASSERT_OK(array.AddInstance(instance_1));

  // Create a copy of the populated instance array object.
  draco::InstanceArray copy;
  copy.Copy(array);

  // Check that the instances have been copied.
  ASSERT_EQ(copy.NumInstances(), 2);
  ASSERT_EQ(copy.GetInstance(0).trs, instance_0.trs);
  ASSERT_EQ(copy.GetInstance(1).trs, instance_1.trs);
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
