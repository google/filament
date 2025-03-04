// Copyright 2019 Google Inc. All rights reserved.
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

#include "port/gtest.h"
#include "port/protobuf.h"
#include "src/libfuzzer/libfuzzer_macro.h"
#include "src/mutator_test_proto2.pb.h"

using protobuf_mutator::protobuf::util::MessageDifferencer;
using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::Ref;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::StrictMock;

static class MockFuzzer* mock_fuzzer;

class MockFuzzer {
 public:
  MockFuzzer() { mock_fuzzer = this; }
  ~MockFuzzer() { mock_fuzzer = nullptr; }
  MOCK_METHOD(void, PostProcess,
              (protobuf_mutator::Msg * message, unsigned int seed));
  MOCK_METHOD(void, TestOneInput, (const protobuf_mutator::Msg& message));
};

protobuf_mutator::libfuzzer::PostProcessorRegistration<protobuf_mutator::Msg>
    reg = {[](protobuf_mutator::Msg* message, unsigned int seed) {
      mock_fuzzer->PostProcess(message, seed);
    }};

DEFINE_TEXT_PROTO_FUZZER(const protobuf_mutator::Msg& message) {
  mock_fuzzer->TestOneInput(message);
}

MATCHER_P(IsMessageEq, msg, "") {
  return MessageDifferencer::Equals(arg, msg.get());
}
MATCHER(IsInitialized, "") { return arg.IsInitialized(); }

TEST(LibFuzzerTest, LLVMFuzzerTestOneInput) {
  unsigned int seed = 0;
  testing::StrictMock<MockFuzzer> mock;
  protobuf_mutator::Msg msg;
  EXPECT_CALL(mock, PostProcess(_, _))
      .WillOnce(DoAll(SaveArgPointee<0>(&msg), SaveArg<1>(&seed)));
  EXPECT_CALL(
      mock, TestOneInput(AllOf(IsMessageEq(std::cref(msg)), IsInitialized())));
  LLVMFuzzerTestOneInput((const uint8_t*)"", 0);

  EXPECT_CALL(mock, PostProcess(_, seed)).WillOnce(SaveArgPointee<0>(&msg));
  EXPECT_CALL(
      mock, TestOneInput(AllOf(IsMessageEq(std::cref(msg)), IsInitialized())));
  LLVMFuzzerTestOneInput((const uint8_t*)"", 0);
}

TEST(LibFuzzerTest, LLVMFuzzerCustomMutator) {
  testing::StrictMock<MockFuzzer> mock;
  protobuf_mutator::Msg msg;
  EXPECT_CALL(mock, PostProcess(_, _)).WillOnce(SaveArgPointee<0>(&msg));
  EXPECT_CALL(
      mock, TestOneInput(AllOf(IsMessageEq(std::cref(msg)), IsInitialized())));

  uint8_t buff[1024] = {};
  size_t size = LLVMFuzzerCustomMutator(buff, 0, sizeof(buff), 5);
  ASSERT_GT(size, 0U);
  LLVMFuzzerTestOneInput(buff, size);
}

TEST(LibFuzzerTest, LLVMFuzzerCustomCrossOver) {
  testing::StrictMock<MockFuzzer> mock;
  protobuf_mutator::Msg msg;
  EXPECT_CALL(mock, PostProcess(_, _)).WillOnce(SaveArgPointee<0>(&msg));
  EXPECT_CALL(
      mock, TestOneInput(AllOf(IsMessageEq(std::cref(msg)), IsInitialized())));

  uint8_t buff1[1024] = {};
  uint8_t buff2[1024] = {};
  uint8_t buff3[1024] = {};
  size_t size =
      LLVMFuzzerCustomCrossOver(buff1, 0, buff2, 0, buff3, sizeof(buff3), 6);
  ASSERT_GT(size, 0U);
  LLVMFuzzerTestOneInput(buff3, size);
}
