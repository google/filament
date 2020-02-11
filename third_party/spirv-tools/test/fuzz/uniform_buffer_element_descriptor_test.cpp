// Copyright (c) 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/fuzz/uniform_buffer_element_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

TEST(UniformBufferElementDescriptorTest, TestEquality) {
  // Test that equality works as expected for various buffer element
  // descriptors.

  protobufs::UniformBufferElementDescriptor descriptor1 =
      MakeUniformBufferElementDescriptor(0, 0, {1, 2, 3});
  protobufs::UniformBufferElementDescriptor descriptor2 =
      MakeUniformBufferElementDescriptor(0, 0, {1, 2, 3});
  protobufs::UniformBufferElementDescriptor descriptor3 =
      MakeUniformBufferElementDescriptor(0, 1, {1, 2, 3});
  protobufs::UniformBufferElementDescriptor descriptor4 =
      MakeUniformBufferElementDescriptor(1, 0, {1, 2, 3});
  protobufs::UniformBufferElementDescriptor descriptor5 =
      MakeUniformBufferElementDescriptor(1, 1, {1, 2, 3});
  protobufs::UniformBufferElementDescriptor descriptor6 =
      MakeUniformBufferElementDescriptor(0, 0, {1, 2, 4});
  protobufs::UniformBufferElementDescriptor descriptor7 =
      MakeUniformBufferElementDescriptor(0, 0, {1, 2});

  ASSERT_TRUE(
      UniformBufferElementDescriptorEquals()(&descriptor1, &descriptor1));
  ASSERT_TRUE(
      UniformBufferElementDescriptorEquals()(&descriptor1, &descriptor2));
  ASSERT_TRUE(
      UniformBufferElementDescriptorEquals()(&descriptor3, &descriptor3));
  ASSERT_TRUE(
      UniformBufferElementDescriptorEquals()(&descriptor4, &descriptor4));
  ASSERT_TRUE(
      UniformBufferElementDescriptorEquals()(&descriptor5, &descriptor5));
  ASSERT_TRUE(
      UniformBufferElementDescriptorEquals()(&descriptor6, &descriptor6));
  ASSERT_TRUE(
      UniformBufferElementDescriptorEquals()(&descriptor7, &descriptor7));

  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor1, &descriptor3));
  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor3, &descriptor1));

  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor1, &descriptor4));
  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor4, &descriptor1));

  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor1, &descriptor5));
  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor5, &descriptor1));

  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor1, &descriptor6));
  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor6, &descriptor1));

  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor1, &descriptor7));
  ASSERT_FALSE(
      UniformBufferElementDescriptorEquals()(&descriptor7, &descriptor1));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
