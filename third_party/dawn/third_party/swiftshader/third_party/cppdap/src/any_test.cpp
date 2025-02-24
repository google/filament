// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dap/any.h"
#include "dap/typeof.h"
#include "dap/types.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dap {

struct AnyTestObject {
  dap::integer i;
  dap::number n;
};

DAP_STRUCT_TYPEINFO(AnyTestObject,
                    "AnyTestObject",
                    DAP_FIELD(i, "i"),
                    DAP_FIELD(n, "n"));

inline bool operator==(const AnyTestObject& a, const AnyTestObject& b) {
  return a.i == b.i && a.n == b.n;
}

}  // namespace dap

namespace {

template <typename T>
struct TestValue {};

template <>
struct TestValue<dap::integer> {
  static const dap::integer value;
};
template <>
struct TestValue<dap::boolean> {
  static const dap::boolean value;
};
template <>
struct TestValue<dap::number> {
  static const dap::number value;
};
template <>
struct TestValue<dap::string> {
  static const dap::string value;
};
template <>
struct TestValue<dap::array<dap::string>> {
  static const dap::array<dap::string> value;
};
template <>
struct TestValue<dap::AnyTestObject> {
  static const dap::AnyTestObject value;
};

const dap::integer TestValue<dap::integer>::value = 20;
const dap::boolean TestValue<dap::boolean>::value = true;
const dap::number TestValue<dap::number>::value = 123.45;
const dap::string TestValue<dap::string>::value = "hello world";
const dap::array<dap::string> TestValue<dap::array<dap::string>>::value = {
    "one", "two", "three"};
const dap::AnyTestObject TestValue<dap::AnyTestObject>::value = {10, 20.30};

}  // namespace

TEST(Any, EmptyConstruct) {
  dap::any any;
  ASSERT_TRUE(any.is<dap::null>());
  ASSERT_FALSE(any.is<dap::boolean>());
  ASSERT_FALSE(any.is<dap::integer>());
  ASSERT_FALSE(any.is<dap::number>());
  ASSERT_FALSE(any.is<dap::object>());
  ASSERT_FALSE(any.is<dap::string>());
  ASSERT_FALSE(any.is<dap::array<dap::integer>>());
  ASSERT_FALSE(any.is<dap::AnyTestObject>());
}

TEST(Any, Boolean) {
  dap::any any(dap::boolean(true));
  ASSERT_TRUE(any.is<dap::boolean>());
  ASSERT_EQ(any.get<dap::boolean>(), dap::boolean(true));
}

TEST(Any, Integer) {
  dap::any any(dap::integer(10));
  ASSERT_TRUE(any.is<dap::integer>());
  ASSERT_EQ(any.get<dap::integer>(), dap::integer(10));
}

TEST(Any, Number) {
  dap::any any(dap::number(123.0f));
  ASSERT_TRUE(any.is<dap::number>());
  ASSERT_EQ(any.get<dap::number>(), dap::number(123.0f));
}

TEST(Any, String) {
  dap::any any(dap::string("hello world"));
  ASSERT_TRUE(any.is<dap::string>());
  ASSERT_EQ(any.get<dap::string>(), dap::string("hello world"));
}

TEST(Any, Array) {
  using array = dap::array<dap::integer>;
  dap::any any(array({10, 20, 30}));
  ASSERT_TRUE(any.is<array>());
  ASSERT_EQ(any.get<array>(), array({10, 20, 30}));
}

TEST(Any, Object) {
  dap::object o;
  o["one"] = dap::integer(1);
  o["two"] = dap::integer(2);
  o["three"] = dap::integer(3);
  dap::any any(o);
  ASSERT_TRUE(any.is<dap::object>());
  if (any.is<dap::object>()) {
    auto got = any.get<dap::object>();
    ASSERT_EQ(got.size(), 3U);
    ASSERT_EQ(got.count("one"), 1U);
    ASSERT_EQ(got.count("two"), 1U);
    ASSERT_EQ(got.count("three"), 1U);
    ASSERT_TRUE(got["one"].is<dap::integer>());
    ASSERT_TRUE(got["two"].is<dap::integer>());
    ASSERT_TRUE(got["three"].is<dap::integer>());
    ASSERT_EQ(got["one"].get<dap::integer>(), dap::integer(1));
    ASSERT_EQ(got["two"].get<dap::integer>(), dap::integer(2));
    ASSERT_EQ(got["three"].get<dap::integer>(), dap::integer(3));
  }
}

TEST(Any, TestObject) {
  dap::any any(dap::AnyTestObject{5, 3.0});
  ASSERT_TRUE(any.is<dap::AnyTestObject>());
  ASSERT_EQ(any.get<dap::AnyTestObject>().i, 5);
  ASSERT_EQ(any.get<dap::AnyTestObject>().n, 3.0);
}

template <typename T>
class AnyT : public ::testing::Test {
 protected:
  void check(const dap::any& any, const T& expect) {
    ASSERT_EQ(any.is<dap::integer>(), (std::is_same<T, dap::integer>::value));
    ASSERT_EQ(any.is<dap::boolean>(), (std::is_same<T, dap::boolean>::value));
    ASSERT_EQ(any.is<dap::number>(), (std::is_same<T, dap::number>::value));
    ASSERT_EQ(any.is<dap::string>(), (std::is_same<T, dap::string>::value));
    ASSERT_EQ(any.is<dap::array<dap::string>>(),
              (std::is_same<T, dap::array<dap::string>>::value));
    ASSERT_EQ(any.is<dap::AnyTestObject>(),
              (std::is_same<T, dap::AnyTestObject>::value));

    ASSERT_EQ(any.get<T>(), expect);
  }
};
TYPED_TEST_SUITE_P(AnyT);

TYPED_TEST_P(AnyT, CopyConstruct) {
  auto val = TestValue<TypeParam>::value;
  dap::any any(val);
  this->check(any, val);
}

TYPED_TEST_P(AnyT, MoveConstruct) {
  auto val = TestValue<TypeParam>::value;
  dap::any any(std::move(val));
  this->check(any, val);
}

TYPED_TEST_P(AnyT, Assign) {
  auto val = TestValue<TypeParam>::value;
  dap::any any;
  any = val;
  this->check(any, val);
}

TYPED_TEST_P(AnyT, MoveAssign) {
  auto val = TestValue<TypeParam>::value;
  dap::any any;
  any = std::move(val);
  this->check(any, val);
}

TYPED_TEST_P(AnyT, RepeatedAssign) {
  dap::string str = "hello world";
  auto val = TestValue<TypeParam>::value;
  dap::any any;
  any = str;
  any = val;
  this->check(any, val);
}

TYPED_TEST_P(AnyT, RepeatedMoveAssign) {
  dap::string str = "hello world";
  auto val = TestValue<TypeParam>::value;
  dap::any any;
  any = std::move(str);
  any = std::move(val);
  this->check(any, val);
}

REGISTER_TYPED_TEST_SUITE_P(AnyT,
                            CopyConstruct,
                            MoveConstruct,
                            Assign,
                            MoveAssign,
                            RepeatedAssign,
                            RepeatedMoveAssign);

using AnyTypes = ::testing::Types<dap::integer,
                                  dap::boolean,
                                  dap::number,
                                  dap::string,
                                  dap::array<dap::string>,
                                  dap::AnyTestObject>;
INSTANTIATE_TYPED_TEST_SUITE_P(T, AnyT, AnyTypes);

TEST(Any, Reset) {
  dap::any any(dap::integer(10));
  ASSERT_TRUE(any.is<dap::integer>());
  any.reset();
  ASSERT_FALSE(any.is<dap::integer>());
}
