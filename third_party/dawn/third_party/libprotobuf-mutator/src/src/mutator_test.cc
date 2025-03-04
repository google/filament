// Copyright 2016 Google Inc. All rights reserved.
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

#include "src/mutator.h"

#include <algorithm>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "port/gtest.h"
#include "src/binary_format.h"
#include "src/mutator_test_proto2.pb.h"
#include "src/mutator_test_proto3.pb.h"
#include "src/text_format.h"

namespace protobuf_mutator {

using protobuf::util::MessageDifferencer;
using testing::TestWithParam;
using testing::ValuesIn;

const char kMessages[] = R"(
  required_msg {}
  optional_msg {}
  repeated_msg {}
  repeated_msg {required_sint32: 56}
  repeated_msg {}
  repeated_msg {
    required_msg {}
    optional_msg {}
    repeated_msg {}
    repeated_msg { required_int32: 67 }
    repeated_msg {}
  }
  any {
    [type.googleapis.com/protobuf_mutator.Msg] {
      optional_msg {}
      repeated_msg {}
      any {
        [type.googleapis.com/protobuf_mutator.Msg3.SubMsg] {
          optional_int64: -5
        }
      }
    }
  }
)";

const char kMessagesProto3[] = R"(
  optional_msg {}
  repeated_msg {}
  repeated_msg {optional_sint32: 56}
  repeated_msg {}
  repeated_msg {
    optional_msg {}
    repeated_msg {}
    repeated_msg { optional_int32: 67 }
    repeated_msg {}
  }
  any {
    [type.googleapis.com/protobuf_mutator.Msg] {
      optional_msg {}
      repeated_msg {}
      any {
        [type.googleapis.com/protobuf_mutator.Msg3.SubMsg] {
          optional_int64: -5
        }
      }
    }
  }
)";

const char kRequiredFields[] = R"(
  required_double: 1.26685288449177e-313
  required_float: 5.9808638e-39
  required_int32: 67
  required_int64: 5285068
  required_uint32: 14486213
  required_uint64: 520229415
  required_sint32: 56
  required_sint64: -6057486163525532641
  required_fixed32: 8812173
  required_fixed64: 273731277756
  required_sfixed32: 43142
  required_sfixed64: 132
  required_bool: false
  required_string: "qwert"
  required_bytes: "asdf"
)";

const char kOptionalFields[] = R"(
  optional_double: 1.93177850152856e-314
  optional_float: 4.7397519e-41
  optional_int32: 40020
  optional_int64: 10
  optional_uint32: 40
  optional_uint64: 159
  optional_sint32: 44015
  optional_sint64: 17493625000076
  optional_fixed32: 193
  optional_fixed64: 8542688694448488723
  optional_sfixed32: 4926
  optional_sfixed64: 60
  optional_bool: true
  optional_string: "QWERT"
  optional_bytes: "ASDF"
  optional_enum: ENUM_5
)";

const char kRepeatedFields[] = R"(
  repeated_double: 1.93177850152856e-314
  repeated_double: 1.26685288449177e-313
  repeated_float: 4.7397519e-41
  repeated_float: 5.9808638e-39
  repeated_int32: 40020
  repeated_int32: 67
  repeated_int64: 10
  repeated_int64: 5285068
  repeated_uint32: 40
  repeated_uint32: 14486213
  repeated_uint64: 159
  repeated_uint64: 520229415
  repeated_sint32: 44015
  repeated_sint32: 56
  repeated_sint64: 17493625000076
  repeated_sint64: -6057486163525532641
  repeated_fixed32: 193
  repeated_fixed32: 8812173
  repeated_fixed64: 8542688694448488723
  repeated_fixed64: 273731277756
  repeated_sfixed32: 4926
  repeated_sfixed32: 43142
  repeated_sfixed64: 60
  repeated_sfixed64: 132
  repeated_bool: false
  repeated_bool: true
  repeated_string: "QWERT"
  repeated_string: "qwert"
  repeated_bytes: "ASDF"
  repeated_bytes: "asdf"
  repeated_enum: ENUM_5
  repeated_enum: ENUM_4
)";

const char kRequiredNestedFields[] = R"(
  required_int32: 123
  optional_msg {
    required_double: 1.26685288449177e-313
    required_float: 5.9808638e-39
    required_int32: 67
    required_int64: 5285068
    required_uint32: 14486213
    required_uint64: 520229415
    required_sint32: 56
    required_sint64: -6057486163525532641
    required_fixed32: 8812173
    required_fixed64: 273731277756
    required_sfixed32: 43142
    required_sfixed64: 132
    required_bool: false
    required_string: "qwert"
    required_bytes: "asdf"
  }
)";

const char kRequiredInAnyFields[] = R"(
  any {
    [type.googleapis.com/protobuf_mutator.Msg] {
      required_uint32: 14486213
      required_uint64: 520229415
      required_sint64: -6057486163525532641
      required_string: "qwert"
      required_bytes: "asdf"
    }
  }
)";

const char kOptionalNestedFields[] = R"(
  optional_int32: 123
  optional_msg {
    optional_double: 1.93177850152856e-314
    optional_float: 4.7397519e-41
    optional_int32: 40020
    optional_int64: 10
    optional_uint32: 40
    optional_uint64: 159
    optional_sint32: 44015
    optional_sint64: 17493625000076
    optional_fixed32: 193
    optional_fixed64: 8542688694448488723
    optional_sfixed32: 4926
    optional_sfixed64: 60
    optional_bool: true
    optional_string: "QWERT"
    optional_bytes: "ASDF"
    optional_enum: ENUM_5
  }
)";

const char kOptionalInAnyFields[] = R"(
  any {
    [type.googleapis.com/protobuf_mutator.Msg] {
      optional_uint32: 440
      optional_uint64: 1559
      optional_sint32: 440615
      optional_string: "XYZ"
      optional_enum: ENUM_4
    }
  }
)";

const char kRepeatedNestedFields[] = R"(
  optional_int32: 123
  optional_msg {
    repeated_double: 1.93177850152856e-314
    repeated_double: 1.26685288449177e-313
    repeated_float: 4.7397519e-41
    repeated_float: 5.9808638e-39
    repeated_int32: 40020
    repeated_int32: 67
    repeated_int64: 10
    repeated_int64: 5285068
    repeated_uint32: 40
    repeated_uint32: 14486213
    repeated_uint64: 159
    repeated_uint64: 520229415
    repeated_sint32: 44015
    repeated_sint32: 56
    repeated_sint64: 17493625000076
    repeated_sint64: -6057486163525532641
    repeated_fixed32: 193
    repeated_fixed32: 8812173
    repeated_fixed64: 8542688694448488723
    repeated_fixed64: 273731277756
    repeated_sfixed32: 4926
    repeated_sfixed32: 43142
    repeated_sfixed64: 60
    repeated_sfixed64: 132
    repeated_bool: false
    repeated_bool: true
    repeated_string: "QWERT"
    repeated_string: "qwert"
    repeated_bytes: "ASDF"
    repeated_bytes: "asdf"
    repeated_enum: ENUM_5
    repeated_enum: ENUM_4
  }
)";

const char kRepeatedInAnyFields[] = R"(
  any {
    [type.googleapis.com/protobuf_mutator.Msg] {
      repeated_double: 1.931778501556e-31
      repeated_double: 1.26685288449177e-31
      repeated_float: 4.739759e-41
      repeated_float: 5.98038e-39
      repeated_int32: 400201
      repeated_int32: 673
      repeated_int64: 104
      repeated_int64: 52850685
    }
  }
)";

const char kOptionalInDeepAnyFields[] = R"(
  any {
    [type.googleapis.com/protobuf_mutator.Msg] {
      any {
        [type.googleapis.com/protobuf_mutator.Msg] {
          any {
            [type.googleapis.com/protobuf_mutator.Msg] {
              optional_double: 1.9317850152856e-314
              optional_sint64: 1743625000076
              optional_string: "XYZ"
            }
          }
        }
      }
    }
  }
)";

const char kUnknownFieldInput[] = R"(
  optional_bool: true
  unknown_field: "test unknown field"
)";

const char kUnknownFieldExpected[] = R"(optional_bool: true
)";

class TestMutator : public Mutator {
 public:
  explicit TestMutator(bool keep_initialized,
                       size_t random_to_default_ratio = 0) {
    Seed(17);
    if (random_to_default_ratio)
      random_to_default_ratio_ = random_to_default_ratio;
    keep_initialized_ = keep_initialized;
  }

 private:
  RandomEngine random_;
};

class ReducedTestMutator : public TestMutator {
 public:
  ReducedTestMutator() : TestMutator(false, 4) {
    for (float i = 1000; i > 0.1; i /= 7) {
      values_.push_back(i);
      values_.push_back(-i);
    }
    values_.push_back(-1.0);
    values_.push_back(0.0);
    values_.push_back(1.0);
  }

 protected:
  int32_t MutateInt32(int32_t value) override { return GetRandomValue(); }
  int64_t MutateInt64(int64_t value) override { return GetRandomValue(); }
  uint32_t MutateUInt32(uint32_t value) override {
    return fabs(GetRandomValue());
  }
  uint64_t MutateUInt64(uint64_t value) override {
    return fabs(GetRandomValue());
  }
  float MutateFloat(float value) override { return GetRandomValue(); }
  double MutateDouble(double value) override { return GetRandomValue(); }
  std::string MutateString(const std::string& value,
                           int size_increase_hint) override {
    return strings_[std::uniform_int_distribution<>(
        0, strings_.size() - 1)(*random())];
  }

 private:
  float GetRandomValue() {
    return values_[std::uniform_int_distribution<>(
        0, values_.size() - 1)(*random())];
  }

  std::vector<float> values_;
  std::vector<std::string> strings_ = {
      "", "\001", "\000", "a", "b", "ab",
  };
};

std::vector<std::string> Split(const std::string& str) {
  std::istringstream iss(str);
  std::vector<std::string> result;
  for (std::string line; std::getline(iss, line, '\n');) result.push_back(line);
  return result;
}

using TestParams =
    std::tuple<const protobuf::Message*, const char*, size_t, std::string>;

template <class T>
std::vector<TestParams> GetFieldTestParams(
    const std::vector<const char*>& tests) {
  std::vector<TestParams> results;
  for (auto t : tests) {
    auto lines = Split(t);
    for (size_t i = 0; i != lines.size(); ++i) {
      if (lines[i].find(':') != std::string::npos)
        results.push_back(
            std::make_tuple(&T::default_instance(), t, i, lines[i]));
    }
  }
  return results;
}

template <class T>
std::vector<TestParams> GetMessageTestParams(
    const std::vector<const char*>& tests) {
  std::vector<TestParams> results;
  for (auto t : tests) {
    auto lines = Split(t);
    for (size_t i = 0; i != lines.size(); ++i) {
      if (lines[i].find("{}") != std::string::npos)
        results.push_back(
            std::make_tuple(&T::default_instance(), t, i, lines[i]));
    }
  }
  return results;
}

bool Mutate(const protobuf::Message& from, const protobuf::Message& to,
            int iterations = 100000) {
  EXPECT_FALSE(MessageDifferencer::Equals(from, to));
  ReducedTestMutator mutator;
  std::unique_ptr<protobuf::Message> message(from.New());
  EXPECT_FALSE(MessageDifferencer::Equals(from, to));
  for (int j = 0; j < iterations; ++j) {
    message->CopyFrom(from);
    mutator.Mutate(message.get(), 1500);
    if (MessageDifferencer::Equals(*message, to)) return true;
  }

  ADD_FAILURE() << "Failed to get from:\n"
                << from.DebugString() << "\nto:\n"
                << to.DebugString();
  return false;
}

bool CrossOver(const protobuf::Message& from, const protobuf::Message& with,
               const protobuf::Message& to, int iterations = 100000) {
  EXPECT_FALSE(MessageDifferencer::Equals(from, to));
  ReducedTestMutator mutator;
  std::unique_ptr<protobuf::Message> message(from.New());
  EXPECT_FALSE(MessageDifferencer::Equals(from, to));
  for (int j = 0; j < iterations; ++j) {
    message->CopyFrom(from);
    mutator.CrossOver(with, message.get(), 1000);
    if (MessageDifferencer::Equals(*message, to)) return true;
  }
  return false;
}

class MutatorTest : public TestWithParam<TestParams> {
 protected:
  void SetUp() override {
    m1_.reset(std::get<0>(GetParam())->New());
    m2_.reset(std::get<0>(GetParam())->New());
    text_ = std::get<1>(GetParam());
    line_ = std::get<2>(GetParam());
  }

  void LoadMessage(protobuf::Message* message) {
    EXPECT_TRUE(ParseTextMessage(text_, message));
  }

  void LoadWithoutLine(protobuf::Message* message) {
    std::ostringstream oss;
    auto lines = Split(text_);
    for (size_t i = 0; i != lines.size(); ++i) {
      if (i != line_) oss << lines[i] << '\n';
    }
    EXPECT_TRUE(ParseTextMessage(oss.str(), message));
  }

  void LoadWithChangedLine(protobuf::Message* message, int value) {
    auto lines = Split(text_);
    std::ostringstream oss;
    for (size_t i = 0; i != lines.size(); ++i) {
      if (i != line_) {
        oss << lines[i] << '\n';
      } else {
        std::string s = lines[i];
        s.resize(s.find(':') + 2);

        if (lines[i].back() == '\"') {
          // strings
          s += value ? "\"\\" + std::to_string(value) + "\"" : "\"\"";
        } else if (lines[i].back() == 'e') {
          // bools
          s += value ? "true" : "false";
        } else {
          s += std::to_string(value);
        }
        oss << s << '\n';
      }
    }
    EXPECT_TRUE(ParseTextMessage(oss.str(), message));
  }

  std::string text_;
  size_t line_;
  std::unique_ptr<protobuf::Message> m1_;
  std::unique_ptr<protobuf::Message> m2_;
};

// These tests are irrelevant for Proto3 as it has no required fields and
// insertion/deletion.

class MutatorFieldInsDelTest : public MutatorTest {};
INSTANTIATE_TEST_SUITE_P(Proto2, MutatorFieldInsDelTest,
                         ValuesIn(GetFieldTestParams<Msg>(
                             {kRequiredFields, kOptionalFields, kRepeatedFields,
                              kRequiredNestedFields, kRequiredInAnyFields,
                              kOptionalNestedFields, kOptionalInAnyFields,
                              kRepeatedNestedFields, kRepeatedInAnyFields,
                              kOptionalInDeepAnyFields})));

TEST_P(MutatorFieldInsDelTest, DeleteField) {
  LoadMessage(m1_.get());
  LoadWithoutLine(m2_.get());
  EXPECT_TRUE(Mutate(*m1_, *m2_));
}

INSTANTIATE_TEST_SUITE_P(Proto2, MutatorTest,
                         ValuesIn(GetFieldTestParams<Msg>(
                             {kRequiredFields, kOptionalFields, kRepeatedFields,
                              kRequiredNestedFields, kRequiredInAnyFields,
                              kOptionalNestedFields, kOptionalInAnyFields,
                              kRepeatedNestedFields, kRepeatedInAnyFields,
                              kOptionalInDeepAnyFields})));
INSTANTIATE_TEST_SUITE_P(Proto3, MutatorTest,
                         ValuesIn(GetFieldTestParams<Msg3>(
                             {kOptionalFields, kRepeatedFields,
                              kOptionalNestedFields, kOptionalInAnyFields,
                              kRepeatedNestedFields, kRepeatedInAnyFields,
                              kOptionalInDeepAnyFields})));

TEST_P(MutatorTest, Initialized) {
  LoadWithoutLine(m1_.get());
  TestMutator mutator(true);
  mutator.Mutate(m1_.get(), 1000);
  EXPECT_TRUE(m1_->IsInitialized());
}

TEST_P(MutatorTest, InsertField) {
  LoadWithoutLine(m1_.get());
  LoadWithChangedLine(m2_.get(), 1);
  EXPECT_TRUE(Mutate(*m1_, *m2_));
}

TEST_P(MutatorTest, ChangeField) {
  LoadWithChangedLine(m1_.get(), 0);
  LoadWithChangedLine(m2_.get(), 1);
  EXPECT_TRUE(Mutate(*m1_, *m2_, 1000000));
  EXPECT_TRUE(Mutate(*m2_, *m1_, 1000000));
}

TEST_P(MutatorTest, CrossOver) {
  LoadWithoutLine(m1_.get());
  LoadMessage(m2_.get());

  EXPECT_FALSE(MessageDifferencer::Equals(*m1_, *m2_));
  TestMutator mutator(false);

  EXPECT_TRUE(CrossOver(*m1_, *m2_, *m2_));
}

template <class Msg>
void RunCrossOver(const protobuf::Message& m1, const protobuf::Message& m2) {
  Msg from;
  from.add_repeated_msg()->CopyFrom(m1);
  from.add_repeated_msg()->CopyFrom(m2);
  from.mutable_repeated_msg(1)->add_repeated_string("repeated_string");

  Msg to;
  to.add_repeated_msg()->CopyFrom(m1);
  to.add_repeated_msg()->CopyFrom(m1);
  to.mutable_repeated_msg(1)->add_repeated_string("repeated_string");
  EXPECT_TRUE(CrossOver(from, from, to));
}

TEST_P(MutatorTest, CopyField) {
  LoadWithChangedLine(m1_.get(), 7);
  LoadWithChangedLine(m2_.get(), 0);

  if (m1_->GetDescriptor() == Msg::descriptor())
    RunCrossOver<Msg>(*m1_, *m2_);
  else
    RunCrossOver<Msg3>(*m1_, *m2_);
}

TEST_P(MutatorTest, CloneField) {
  LoadWithChangedLine(m1_.get(), 7);
  LoadWithoutLine(m2_.get());

  if (m1_->GetDescriptor() == Msg::descriptor())
    RunCrossOver<Msg>(*m1_, *m2_);
  else
    RunCrossOver<Msg3>(*m1_, *m2_);
}

class MutatorSingleFieldTest : public MutatorTest {};
template <typename T>
class MutatorTypedTest : public ::testing::Test {
 public:
  using Message = T;
};

using MutatorTypedTestTypes = testing::Types<Msg, Msg3>;
TYPED_TEST_SUITE(MutatorTypedTest, MutatorTypedTestTypes);

TYPED_TEST(MutatorTypedTest, FailedMutations) {
  TestMutator mutator(false);
  size_t crossovers = 0;
  for (int i = 0; i < 1000; ++i) {
    typename TestFixture::Message messages[2];
    typename TestFixture::Message tmp;
    for (int j = 0; j < 20; ++j) {
      for (auto& m : messages) {
        tmp.CopyFrom(m);
        mutator.Mutate(&m, 1000);
        // Mutate must not produce the same result.
        EXPECT_FALSE(MessageDifferencer::Equals(m, tmp));
      }
    }

    tmp.CopyFrom(messages[1]);
    mutator.CrossOver(messages[0], &tmp, 1000);
    if (MessageDifferencer::Equals(tmp, messages[1]) ||
        MessageDifferencer::Equals(tmp, messages[0]))
      ++crossovers;
  }

  // CrossOver may fail but very rare.
  EXPECT_LT(crossovers, 100u);
}

TYPED_TEST(MutatorTypedTest, RegisterPostProcessor) {
  std::set<std::string> top_mutations = {"0123456789abcdef",
                                         "abcdef0123456789"};
  TestMutator mutator(false);
  for (auto& v : top_mutations) {
    mutator.RegisterPostProcessor(
        TestFixture::Message::descriptor(),
        [=](protobuf::Message* message, unsigned int seed) {
          auto test_message =
              static_cast<typename TestFixture::Message*>(message);
          if (seed % 2) test_message->set_optional_string(v);
        });
  }

  std::set<int64_t> nested_mutations = {1234567, 567890};
  for (auto& v : nested_mutations) {
    mutator.RegisterPostProcessor(
        TestFixture::Message::SubMsg::descriptor(),
        [=](protobuf::Message* message, unsigned int seed) {
          auto test_message =
              static_cast<typename TestFixture::Message::SubMsg*>(message);
          if (seed % 2) test_message->set_optional_int64(v);
        });
  }

  bool regular_mutation = false;

  for (int j = 0; j < 100000; ++j) {
    // Include this field to increase the probability of mutation.
    typename TestFixture::Message message;
    message.set_optional_string("a");
    mutator.Mutate(&message, 1000);

    top_mutations.erase(message.optional_string());
    nested_mutations.erase(message.mutable_sub_message()->optional_int64());
    if (message.optional_string().empty()) regular_mutation = true;

    if (top_mutations.empty() && nested_mutations.empty() && regular_mutation)
      break;
  }

  EXPECT_TRUE(top_mutations.empty());
  EXPECT_TRUE(nested_mutations.empty());
  EXPECT_TRUE(regular_mutation);
}

TYPED_TEST(MutatorTypedTest, Serialization) {
  TestMutator mutator(false);
  for (int i = 0; i < 10000; ++i) {
    typename TestFixture::Message message;
    for (int j = 0; j < 5; ++j) {
      mutator.Mutate(&message, 1000);
      typename TestFixture::Message parsed;

      EXPECT_TRUE(ParseTextMessage(SaveMessageAsText(message), &parsed));
      EXPECT_TRUE(MessageDifferencer::Equals(parsed, message));

      EXPECT_TRUE(ParseBinaryMessage(SaveMessageAsBinary(message), &parsed));
      EXPECT_TRUE(MessageDifferencer::Equals(parsed, message));
    }
  }
}

TYPED_TEST(MutatorTypedTest, UnknownFieldTextFormat) {
  typename TestFixture::Message parsed;
  EXPECT_TRUE(ParseTextMessage(kUnknownFieldInput, &parsed));
  EXPECT_EQ(SaveMessageAsText(parsed), kUnknownFieldExpected);
}

TYPED_TEST(MutatorTypedTest, DeepRecursion) {
  typename TestFixture::Message message;
  typename TestFixture::Message* last = &message;
  for (int i = 0; i < 150; ++i) {
    last = last->mutable_optional_msg();
    std::string text = SaveMessageAsText(message);
    std::string binary = SaveMessageAsBinary(message);
    typename TestFixture::Message parsed;
    EXPECT_EQ(i < 100, ParseTextMessage(SaveMessageAsText(message), &parsed));
    EXPECT_EQ(i < 100,
              ParseBinaryMessage(SaveMessageAsBinary(message), &parsed));
  }
}

TYPED_TEST(MutatorTypedTest, EmptyMessage) {
  typename TestFixture::Message::EmptyMessage message;
  TestMutator mutator(false);
  for (int j = 0; j < 10000; ++j) mutator.Mutate(&message, 1000);
}

TYPED_TEST(MutatorTypedTest, Regressions) {
  typename TestFixture::Message::RegressionMessage message;
  TestMutator mutator(false);
  for (int j = 0; j < 10000; ++j) mutator.Mutate(&message, 1000);
}

TYPED_TEST(MutatorTypedTest, UsageExample) {
  typename TestFixture::Message::SmallMessage message;
  TestMutator mutator(false);

  // Test that we can generate all variation of the message.
  std::set<std::string> mutations;
  for (int j = 0; j < 1000; ++j) {
    mutator.Mutate(&message, 1000);
    std::string str = SaveMessageAsText(message);
    mutations.insert(str);
  }

  if (std::is_same<typename TestFixture::Message, Msg>::value) {
    // 3 states for boolean and 5 for enum, including missing fields.
    EXPECT_EQ(3u * 5u, mutations.size());
  } else {
    // 2 states for boolean and 4 for enum.
    EXPECT_EQ(2u * 4u, mutations.size());
  }
}

TYPED_TEST(MutatorTypedTest, Maps) {
  TestMutator mutator(true);

  typename TestFixture::Message::MapMessage message;
  for (int j = 0; j < 10000; ++j) mutator.Mutate(&message, 1000);
}

class MutatorMessagesTest : public MutatorTest {};
INSTANTIATE_TEST_SUITE_P(Proto2, MutatorMessagesTest,
                         ValuesIn(GetMessageTestParams<Msg>({kMessages})));
INSTANTIATE_TEST_SUITE_P(
    Proto3, MutatorMessagesTest,
    ValuesIn(GetMessageTestParams<Msg3>({kMessagesProto3})));

TEST_P(MutatorMessagesTest, DeletedMessage) {
  LoadMessage(m1_.get());
  LoadWithoutLine(m2_.get());
  EXPECT_TRUE(Mutate(*m1_, *m2_));
}

TEST_P(MutatorMessagesTest, InsertMessage) {
  LoadWithoutLine(m1_.get());
  LoadMessage(m2_.get());
  EXPECT_TRUE(Mutate(*m1_, *m2_));
}

class MutatorMessagesSizeTest : public TestWithParam<size_t> {};

static const size_t kMaxSizes[] = {100, 256, 777, 10101};
INSTANTIATE_TEST_SUITE_P(Proto, MutatorMessagesSizeTest, ValuesIn(kMaxSizes));

TEST_P(MutatorMessagesSizeTest, MaxSize) {
  TestMutator mutator(false);
  size_t over_sized_count = 0;
  Msg message;
  const size_t kMaxSize = GetParam();
  const int kIterations = 10000;
  for (int i = 0; i < kIterations; ++i) {
    mutator.Mutate(&message, kMaxSize);
    if (message.ByteSizeLong() > kMaxSize) ++over_sized_count;
    EXPECT_LT(message.ByteSizeLong(), 1.1 * kMaxSize);
  }
  EXPECT_LT(over_sized_count, kIterations * .1);
}

// TODO(vitalybuka): Special tests for oneof.

TEST(MutatorMessagesTest, NeverCopyUnknownEnum) {
  TestMutator mutator(false);
  for (int j = 0; j < 10000; ++j) {
    Msg3 message;
    message.set_optional_enum(Msg3::ENUM_5);
    message.add_repeated_enum(static_cast<Msg3::Enum>(100));
    mutator.Mutate(&message, 100);
    EXPECT_NE(message.optional_enum(), 100);
  }
}

}  // namespace protobuf_mutator
