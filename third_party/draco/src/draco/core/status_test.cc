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
#include "draco/core/status.h"

#include <sstream>

#include "draco/core/draco_test_base.h"

namespace {

class StatusTest : public ::testing::Test {
 protected:
  StatusTest() {}
};

TEST_F(StatusTest, TestStatusOutput) {
  // Tests that the Status can be stored in a provided std::ostream.
  const draco::Status status(draco::Status::DRACO_ERROR, "Error msg.");
  ASSERT_EQ(status.code(), draco::Status::DRACO_ERROR);
  ASSERT_EQ(status.code_string(), "DRACO_ERROR");

  std::stringstream str;
  str << status;
  ASSERT_EQ(str.str(), "Error msg.");

  const draco::Status status2 = draco::ErrorStatus("Error msg2.");
  ASSERT_EQ(status2.code(), draco::Status::DRACO_ERROR);
  ASSERT_EQ(status2.error_msg_string(), "Error msg2.");
  ASSERT_EQ(status2.code_string(), "DRACO_ERROR");
  ASSERT_EQ(status2.code_and_error_string(), "DRACO_ERROR: Error msg2.");
}

}  // namespace
