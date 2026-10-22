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
#include "draco/compression/config/decoder_options.h"

#include "draco/core/draco_test_base.h"

namespace {

class DecoderOptionsTest : public ::testing::Test {
 protected:
  DecoderOptionsTest() {}
};

TEST_F(DecoderOptionsTest, TestOptions) {
  // This test verifies that we can update global and attribute options of the
  // DecoderOptions class instance.
  draco::DecoderOptions options;
  options.SetGlobalInt("test", 3);
  ASSERT_EQ(options.GetGlobalInt("test", -1), 3);

  options.SetAttributeInt(draco::GeometryAttribute::POSITION, "test", 1);
  options.SetAttributeInt(draco::GeometryAttribute::GENERIC, "test", 2);
  ASSERT_EQ(
      options.GetAttributeInt(draco::GeometryAttribute::TEX_COORD, "test", -1),
      3);
  ASSERT_EQ(
      options.GetAttributeInt(draco::GeometryAttribute::POSITION, "test", -1),
      1);
  ASSERT_EQ(
      options.GetAttributeInt(draco::GeometryAttribute::GENERIC, "test", -1),
      2);
}

TEST_F(DecoderOptionsTest, TestAttributeOptionsAccessors) {
  // This test verifies that we can query options stored in DecoderOptions
  // class instance.
  draco::DecoderOptions options;
  options.SetGlobalInt("test", 1);
  options.SetAttributeInt(draco::GeometryAttribute::POSITION, "test", 2);
  options.SetAttributeInt(draco::GeometryAttribute::TEX_COORD, "test", 3);

  ASSERT_EQ(
      options.GetAttributeInt(draco::GeometryAttribute::POSITION, "test", -1),
      2);
  ASSERT_EQ(
      options.GetAttributeInt(draco::GeometryAttribute::POSITION, "test2", -1),
      -1);
  ASSERT_EQ(
      options.GetAttributeInt(draco::GeometryAttribute::TEX_COORD, "test", -1),
      3);
  ASSERT_EQ(
      options.GetAttributeInt(draco::GeometryAttribute::NORMAL, "test", -1), 1);
}

}  // namespace
