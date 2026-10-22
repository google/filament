// Copyright 2017 Google Inc. All rights reserved.
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

#include "src/utf8_fix.h"

#include "port/gtest.h"
#include "port/protobuf.h"

namespace protobuf_mutator {

class FixUtf8StringTest : public ::testing::TestWithParam<int> {
 public:
  bool IsStructurallyValid(const std::string& s) {
    using protobuf::internal::WireFormatLite;
    return WireFormatLite::VerifyUtf8String(s.data(), s.length(),
                                            WireFormatLite::PARSE, "");
  }
};

TEST_F(FixUtf8StringTest, IsStructurallyValid) {
  EXPECT_TRUE(IsStructurallyValid(""));
  EXPECT_TRUE(IsStructurallyValid("abc"));
  EXPECT_TRUE(IsStructurallyValid("\xC2\xA2"));
  EXPECT_TRUE(IsStructurallyValid("\xE2\x82\xAC"));
  EXPECT_TRUE(IsStructurallyValid("\xF0\x90\x8D\x88"));
  EXPECT_FALSE(IsStructurallyValid("\xFF\xFF\xFF\xFF"));
  EXPECT_FALSE(IsStructurallyValid("\xFF\x8F"));
  EXPECT_FALSE(IsStructurallyValid("\x3F\xBF"));
}

INSTANTIATE_TEST_SUITE_P(Size, FixUtf8StringTest, ::testing::Range(0, 10));

TEST_P(FixUtf8StringTest, FixUtf8String) {
  RandomEngine random(GetParam());
  std::uniform_int_distribution<> random8(0, 0xFF);

  std::string str(random8(random), 0);
  for (uint32_t run = 0; run < 10000; ++run) {
    for (size_t i = 0; i < str.size(); ++i) str[i] = random8(random);
    std::string fixed = str;
    FixUtf8String(&fixed, &random);
    if (IsStructurallyValid(str)) {
      EXPECT_EQ(str, fixed);
    } else {
      EXPECT_TRUE(IsStructurallyValid(fixed));
    }
  }
}

}  // namespace protobuf_mutator
